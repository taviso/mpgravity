/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: codepg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.8  2008/09/19 14:51:15  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

/**********************************************************************************
Copyright (c) 2003, Albert M. Choy
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
* Neither the name of Microplanet, Inc. nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************/

#include <stdafx.h>
#include "codepg.h"
#include "qprint.h"
#include "superstl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static int CP_encode_QP(bool fHeader, CString & strIn, CString & strOut);

// From the Server to the UI ( Inbound )
//
//    Translate bytes to Unicode and then use 'WideCharToMultiByte' to get to the computers
//    active code page.
//
//
// From the UI to the Server
//
//    Take the multibyte charset convert to Unicode, then figure out what ISO-charset will
//    hold it.  Then label message with that charset.
//

int CP_Inbound(const CString & strCharsetIn, LPCTSTR data, int len, CString & strOutput)
{
	// RLW : Charset extraction bug fixed - thanks to anonymous contributor for the report + patch

	// Find charset text ('=' not included because it could be surrounded by space chars)
	int nIndex = strCharsetIn.Find("charset");

	if (nIndex == -1)
		return 1;

	// Get rid of anything before 'charset'
	CString str = strCharsetIn.Mid(nIndex + 7);
	// get rid of anything after the next ';'
	str = str.SpanExcluding(";");
	// Clean the remainder up (remove spaces, '=', quotes...)
	str.Trim();
	str.TrimLeft("=");
	str.Trim();
	str.Trim('"');
	
	if (0 == str.Compare("us-ascii"))
		return 1; // do no processing

	GravCharset * pCharset = gsCharMaster.findByName(str);

	if (NULL == pCharset)
		return 1; // Not a charset we recognise

	return CP_Util_Inbound(pCharset, data, len, strOutput);
}

int CP_Util_Inbound(GravCharset * pCharset, LPCTSTR data, int len, CString & strOutput)
{
	ASSERT(NULL != pCharset);

	std::vector<WCHAR> vWideChars;

	// convert to Unicode
	// this is a virt function now. For UTF-8 we'll probably have fewer characters afterwards
	pCharset->Translate(data, len, vWideChars);
	len = vWideChars.size();

	// convert to something viable in the user's code page
	int    cchMB = len + 16;
	int    retries = 0;

try_again:
	LPTSTR pMultiByte = strOutput.GetBuffer (cchMB);

	// this returns the count of translated characters
	int nRet = WideCharToMultiByte(CP_ACP,
		0,       // flags,
		&vWideChars[0],
		len,
		pMultiByte,
		cchMB,
		NULL,
		NULL);

	strOutput.ReleaseBuffer(nRet);

	if (0 == nRet)
	{
		DWORD err = GetLastError();

		switch (err)
		{
		case ERROR_INSUFFICIENT_BUFFER:

			TRACE("INSUFFICIENT_BUFFER\n");

			cchMB = ((1024 + cchMB) * 3) / 2;

			if (++retries < 5)
				goto try_again;
			break;

		case ERROR_INVALID_FLAGS:
			TRACE("Invalid flags \n");
			break;

		case ERROR_INVALID_PARAMETER:
			TRACE("INVALID_PARAM \n");
			break;
		}
	}

	return 0;
}

// ========================================================================
// Returns 0 for success.
// fSend8Bit -  True,  don't use QP, just send 8bit iso-8859-N
//              False, convert to iso, then use QP
//
int CP_Outbound(bool              fHeaderLine,
				const CString &   strIn,
				int				   iSendCharset,
				BOOL				   fSend8Bit,
				CString &			strOut,          // really contains bytes
				ECP_OutputType &  eOutput)
{
	// go from codepage  to Unicode
	int len = strIn.GetLength();

	vector<WCHAR> vWideChars(len + 1);

	// this accounts for the user's codepage. We go from some codepage to WideChars
	int nCount = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED,
		(LPCTSTR) strIn,
		len,
		&vWideChars[0],
		len);

	// go from Unicode to some byte value
	GravCharset* pCharset = gsCharMaster.findById(iSendCharset);
	if (NULL == pCharset)
	{
		strOut = strIn;
		return 0;
	}

	std::vector<BYTE> outBytes;
	// tells us if we encounted an 8bit character
	BOOL preprocessForQP  = (fHeaderLine && !fSend8Bit);
	bool f8BitOutput = pCharset->TranslateUnicodeToBytes(preprocessForQP, nCount, &vWideChars[0], outBytes);

	// clear vWideChars
	vector<WCHAR>().swap(vWideChars);

	// transfer back to 'strOut'
	if (!outBytes.empty())
	{
		int sz = outBytes.size();
		LPTSTR pOut = strOut.GetBuffer(sz);
		CopyMemory(pOut, &outBytes[0], sz);
		strOut.ReleaseBuffer(sz);

		// clear outBytes
		vector<BYTE>().swap(outBytes);
	}

	// short circuit. If ok to send 8 bit, skip QP encoding
	if (fSend8Bit)
	{
		if (f8BitOutput)
			eOutput = kCP_8BIT;
		else
			eOutput = kCP_7BIT;
		return 0;
	}

	if (false == f8BitOutput)
	{
		eOutput = kCP_7BIT;
		return 0;
	}

	// change 8bit iso to QP
	eOutput = kCP_QP;

	CString strFinal;

	CP_encode_QP(fHeaderLine, strOut, strFinal);

	strOut = strFinal;   // return strOut

	return 0;
}

int CP_encode_QP (bool fHeaderLine, CString & strIn, CString & strOut)
{
	LPTSTR pData = (LPTSTR)(LPCTSTR) strIn;

	int len = strIn.GetLength();
	istrstream in_stream(pData, len);   // input stream (from a string)

	CMemFile sFile;

	DWORD  dwOutBytes = 0;
	DWORD  dwOutLines = 0;

	DWORD  dwRet = 0;

	if (fHeaderLine)
	{
		dwRet = Encode_8bit_QuotedPrintable(in_stream,
			len,
			999000000,
			sFile,
			dwOutBytes,
			dwOutLines,
			32000000);  // infinite line length
	}
	else
	{
		dwRet = Encode_8bit_QuotedPrintable(in_stream,
			len,
			999000000,
			sFile,
			dwOutBytes,
			dwOutLines);
	}

	dwRet = sFile.GetLength();

	LPTSTR pFinal = strOut.GetBuffer(int(dwRet));

	sFile.SeekToBegin();
	sFile.Read(pFinal, dwRet);
	strOut.ReleaseBuffer(int(dwRet));

	return 0;
}
