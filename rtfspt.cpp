/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rtfspt.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:47  richard_wood
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

#include "stdafx.h"
#include "rtfspt.h"
#include "richedit.h"
#include "tglobopt.h"            // TGlobalOptions

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

DWORD CALLBACK RTFParagraphFinderCallback(
	DWORD    dwCookie,
	LPBYTE   pbBuff,
	LONG     cb,
	LONG*    pcb);

// ------------------------------------------------------------------------
// add a copy of this data into the array
void RTF_ParaVecAdd(CHARRANGE& in, PARAVEC* pVec)
{
	CHARRANGE* pElem = new CHARRANGE;
	pElem->cpMin = in.cpMin;
	pElem->cpMax = in.cpMax;

	pVec->Add ( pElem );
}

// special support for Quoting original text
void RTF_ParagraphFinder (CWnd* pWnd,
						  CHARRANGE& cr,
						  PARAVEC* pVec,
						  LPCTSTR  prefix)
{
	EDITSTREAM     sEditStream;   // defined by microsoft

	PARAFIND_STREAMIN_DATA sin;   // defined by me
	//sin.pPrefix = prefix;
	//sin.iPrefixLen = _tcslen(prefix);
	sin.iIndex  = 0;
	sin.iFoundCRLF = 0;

	sin.iStartIdx = min(cr.cpMin, cr.cpMax);
	sin.fBegun  = TRUE;  // assume first chunk is a paragraph
	sin.cLast   = 0;
	sin.pVec    = pVec;
	sin.fNewline= TRUE;

	sEditStream.dwCookie = (DWORD) &sin;
	sEditStream.dwError  = 0;
	sEditStream.pfnCallback = RTFParagraphFinderCallback;

	// select the area of interest
	CHARRANGE rite;
	rite.cpMin = min(cr.cpMin, cr.cpMax);
	rite.cpMax = max(cr.cpMin, cr.cpMax);
	pWnd->SendMessage(EM_EXSETSEL, 0, LPARAM(&rite));

	// stream out the selected area
	pWnd->SendMessage(EM_STREAMOUT,
		SF_TEXT | SFF_SELECTION,
		(LPARAM) &sEditStream);

	// last chunk is considered a paragraph
	if (sin.fBegun)
	{
		CHARRANGE para;
		para.cpMin = sin.iFoundCRLF + sin.iStartIdx+1;
		para.cpMax = rite.cpMax;
		if (para.cpMax > para.cpMin)
			RTF_ParaVecAdd(para, pVec);
	}

	// calling function must restore selection
}

DWORD CALLBACK RTFParagraphFinderCallback(
	DWORD    dwCookie,
	LPBYTE   pbBuff,           // buffer we read from
	LONG     cb,               // bytes to read
	LONG*    pcb)              // return bytes we read
{
	// my struct
	LPPARAFIND_STREAMIN_DATA pData = (LPPARAFIND_STREAMIN_DATA) dwCookie;

	// Code commented out below is inefficient, so replaced
	// with the C functions that do the scanning for us.

	// goal: look for paragraphs
	int nStart = 0, nEnd = 0;
	char *pStart = (char*)pbBuff, *pBuff = (char*)pbBuff, *pLast = NULL;

	while ((pBuff - pStart) < cb)
	{
		// Rememeber where we are
		pLast = pBuff;

		// Find the next '\n' in pbBuff
		pBuff = strchr((char*)(pStart+(pLast-pStart)), 0x0a);

		if (!pBuff)
		{
			// Not found one, might be one in next lot of text streamed
			// from rich edit control so save status for next lump...
			break;
		}
		else
		{
			// Store this newline pos so our caller can work out the last para.
			pData->iFoundCRLF = pData->iIndex + (pLast - pStart);

			// Found a newline. The previous char should be a carriage return
			if (pBuff != pStart)
			{
				// get previous character
				pData->cLast = (*(pBuff-1));
			}
			// Else pBuff is right at the start to use previous char stored last time around

			if (pData->cLast == 0x0d)
			{
				// work out start and end of this paragraph
				nStart = pLast - pStart;
				nEnd = pBuff - pStart;

				// pData->iIndex is the total of chars we've processed in calls to this func previously
				// pData->iStartIdx is the offset from the start
				// nDiff is the position in this lump
				//
				// Real starting pos is therefore:-
				// pData->iStartIdx + pData->iIndex + nStart
				// and end is at:-
				// pData->iStartIdx + pData->iIndex + nEnd
				CHARRANGE para;
				para.cpMin = pData->iStartIdx + pData->iIndex + nStart;
				para.cpMax = pData->iStartIdx + pData->iIndex + nEnd;

				RTF_ParaVecAdd ( para, pData->pVec );
				//TRACE("Para found (%d - %d) : '%s'\n", para.cpMin, para.cpMax,
				//	CString(&pStart[pData->iIndex + nStart], (para.cpMax - para.cpMin)+1));
			}
			// Move past the end of this para to the start of the next.
			pBuff++;
		}
	}

	// Store last available char
	pData->cLast = pbBuff[cb];
	// Store index we've checked upto
	pData->iIndex += cb;
	// Tell RTF we've eaten all its supplied and want more
	*pcb = cb;

	// return Zero to Continue
	return 0;


	//// goal: look for paragraphs
	//LONG lBytes = 0;
	//LPBYTE pBuff = pbBuff;

	//while (lBytes < cb)
	//{
	//	if (*pBuff == '\n' && pData->cLast == '\r')
	//	{
	//		if (TRUE  /*pData->fBegun*/)
	//		{
	//			CHARRANGE para;
	//			para.cpMin = (pData->iFoundCRLF) + pData->iStartIdx;
	//			para.cpMax = (pData->iIndex + 1) + pData->iStartIdx;

	//			RTF_ParaVecAdd ( para, pData->pVec );

	//			pData->iFoundCRLF = pData->iIndex + 1;
	//		}
	//		else
	//		{
	//			pData->fBegun = TRUE;
	//			pData->iFoundCRLF = pData->iIndex + 1;
	//		}
	//	}
	//	pData->cLast = *pBuff;
	//	++(pData->iIndex);
	//	++pBuff;
	//	++lBytes;
	//}
	//ASSERT( lBytes == cb);

	//*pcb = lBytes;

	//return 0;   // return Zero to Continue;
}

DWORD CALLBACK RTFStreamInCallback (
									DWORD    dwCookie,
									LPBYTE   pbBuff,
									LONG     cb,
									LONG*    pcb)
{
	LPSTREAMIN_DATA pData = (LPSTREAMIN_DATA) dwCookie;
	LPCTSTR ptr = pData->pStr + pData->index;
	LONG   byAdded = 0;

	if (pData->end)
		return *pcb = 0;

	int iRemain = pData->total - pData->index;
	if (iRemain >= cb)
	{
		// we have more data than the RTF is Asking for
		CopyMemory ( pbBuff,   // dest
			ptr,      // source
			cb );
		byAdded = cb;
		if (iRemain == cb)
			pData->end = TRUE;
	}
	else // iRemain < cb
	{
		CopyMemory ( pbBuff, ptr, iRemain );
		pData->end = TRUE;
		byAdded = iRemain;
	}

	*pcb = byAdded;        // return number of bytes added
	pData->index += byAdded;
	return 0;
}

// Flags are usually :
//    SF_TEXT  - replaces all contents
//    SF_TEXT | SF_SELECTION  - replaces current selection (eg APPEND)
void RTF_TextIn(CWnd * pWnd, LPCTSTR pText, WPARAM wFlags)
{
	if (!pWnd || !pWnd->m_hWnd)
		return;
	STREAMIN_DATA sin;               // defined by Alchoy
	EDITSTREAM sEditStream;          // defined by Microsoft

	sin.pStr  = pText;
	sin.index = 0;
	sin.total = _tcslen (pText);
	sin.end   = FALSE;

	sEditStream.dwCookie = (DWORD) &sin;
	sEditStream.dwError  = 0;
	sEditStream.pfnCallback = RTFStreamInCallback;
	pWnd->SendMessage(EM_STREAMIN, wFlags, (LPARAM) &sEditStream);
}

/**************************************************************************

**************************************************************************/

DWORD CALLBACK RTFStreamOutCallback (
									 DWORD    dwCookie,
									 LPBYTE   pbBuff,
									 LONG     cb,                 // number of BYTES
									 LONG*    pcb)
{
	LPSTREAMOUT_DATA pData = (LPSTREAMOUT_DATA) dwCookie;
	LONG count = cb;

	int curLen = pData->pString->GetLength();
	int newLen = curLen + (cb/sizeof(TCHAR));
	LPTSTR ptr = pData->pString->GetBuffer(newLen);
	CopyMemory ( ptr + curLen,   // dest
		pbBuff,         // src
		cb );           // number of bytes
	pData->pString->ReleaseBuffer (newLen);

	*pcb = cb;
	// success
	return 0;
}

void RTF_TextOut(CWnd* pWnd, CString*  pString, WPARAM wFlags)
{
	STREAMOUT_DATA sout;
	EDITSTREAM sEditStream;

	sout.pString  = pString;
	sout.end      = FALSE;

	sEditStream.dwCookie = (DWORD) &sout;
	sEditStream.dwError  = 0;
	sEditStream.pfnCallback = RTFStreamOutCallback;
	pWnd->SendMessage(EM_STREAMOUT, wFlags, (LPARAM) &sEditStream);
}

// OR in these styles with EM_SETEVENTMASK
//  used by mainfrm.cpp, friched.cpp
void RTF_EventMask (CWnd* pWnd, DWORD style)
{
	DWORD eventMask = pWnd->SendMessage (EM_GETEVENTMASK);
	eventMask |= style;
	pWnd->SendMessage (EM_SETEVENTMASK, 0, LPARAM(eventMask));
}

//////////////////////////////////////////////////////////////////////
// does the ROT13 substitution cipher
//
void RTF_TextRotate13(CString & strText)
{
	int len = strText.GetLength();
	LPTSTR pText = strText.GetBuffer(len);
	for (int i = 0; i < len; ++i,++pText)
	{
		TCHAR c = *pText;
		if (c >= 'a' && c <= 'm')
			c = TCHAR(c + 13);
		else if (c >= 'n' && c <= 'z')
			c = TCHAR(c - 13);
		else if (c >= 'A' && c <= 'M')
			c = TCHAR(c + 13);
		else if (c >= 'N' && c <= 'Z')
			c = TCHAR(c - 13);
		*pText = c;
	}
	strText.ReleaseBuffer(len);
}

//-------------------------------------------------------------------------
//
//
int RTF_GetParagraphBounds(CWnd* pRich, int iCharPos, CHARRANGE* pchrg)
{
	FINDTEXT  ft;
	TCHAR rcBuf[] = "\r\n";
	int  iBegin = 0;
	int  iEnd;
	int  ret;

	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = iCharPos;
	ft.lpstrText = rcBuf;

	// Starting from the top, keep searching for CRLF
	while (TRUE)
	{
		ret = pRich->SendMessage(EM_FINDTEXT, 0, (LPARAM)&ft);
		if (ret == -1)
			break;
		else
			iBegin = ret + 2;

		if (ret + 2 >= iCharPos)
			break;
		// go again
		ft.chrg.cpMin = ret + 2;
	}

	// ok we know the paragraph begins at iBegin

	ft.chrg.cpMin = iCharPos;
	ft.chrg.cpMax = iEnd = pRich->SendMessage(WM_GETTEXTLENGTH);

	// see where paragraph ends (EOF or CRLF)
	ret = pRich->SendMessage(EM_FINDTEXT, 0, (LPARAM) &ft);

	if (ret != -1)
		iEnd = ret;

	pchrg->cpMin = iBegin;
	pchrg->cpMax = iEnd;

	return 0;
}

//-------------------------------------------------------------------------
// search forward from 'iStartPos' and return the 0-based index where
//        'cKey' was found
//
int RTF_FindChar (CRichEditCtrl & sRich, int iStartPos, TCHAR  cKey,
				  BOOL & fEOF)
{
	FINDTEXTEX  ft;
	TCHAR     rcBuf[2];
	int       nLen =    sRich.GetTextLength();

	rcBuf[0] = cKey;
	rcBuf[1] = '\0';

	ft.chrg.cpMin = iStartPos;
	ft.chrg.cpMax = max (0,  nLen - 1);
	ft.lpstrText = rcBuf;

	long lRet = sRich.FindText (0, &ft);
	int  idxFound;

	if (-1 == lRet )
	{
		fEOF = TRUE;
		idxFound = ft.chrg.cpMax;
	}
	else
	{
		fEOF = FALSE;
		idxFound = (int) lRet;     
	}
	return idxFound;
}

//-------------------------------------------------------------------------
// Collect the ranges for each paragraph. Return data in CTypedPtrArray
//

int octalFindText(CWnd* pRich, FINDTEXT* pft)
{
	return pRich->SendMessage(EM_FINDTEXT, 0, (LPARAM) pft);
}

int RTF_CollectParagraphs(CWnd* pRich, CHARRANGE& chrg, PARAVEC* pVec)
{
	CHARRANGE saveSel ;
	pRich->SendMessage(EM_EXGETSEL, 0, (LPARAM) &saveSel);
	RTF_ParagraphFinder ( pRich, chrg, pVec, NULL );
	pRich->SendMessage(EM_EXSETSEL, 0, (LPARAM) &saveSel);
	return 0;
}
