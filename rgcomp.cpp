/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgcomp.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.4  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.3  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:44  richard_wood
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
#include "resource.h"
#include "mplib.h"
#include "rgcomp.h"
#include "regutil.h"
#include "tidarray.h"            // SetMessageIDsSaved()
#include "globals.h"
#include "newsdb.h"              // gpStore
#include "server.h"              // TNewsServer
#include "genutil.h"             // CopyCStringList()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegCompose::ENCSYNC TRegCompose::m_EncMap[] =
{
	{_T("UUENCODE"), TGlobalDef::kUUENCODE},
	{_T("MIME"),    TGlobalDef::kMIME}
};

TRegCompose::TRegCompose()
{
	default_values();
	// Setup table that controls i/o
	m_itemCount = 22;
	m_pTable = new TableItem[m_itemCount];
	SetItem(m_pTable + 0, _T("IncOrigMail"), kREGBool, &m_fIncludeOriginalMail);
	SetItem(m_pTable + 1, _T("IncOrigFol"),  kREGBool, &m_fIncludeOriginalFollowup);
	SetItem(m_pTable + 2, _T("LimitOrig"),   kREGBool, &m_fLimitQuotedLines);
	SetItem(m_pTable + 3, _T("SepAttach"),   kREGBool, &m_fattachSeparateArt);

	SetItem(m_pTable + 4, _T("encSplitLen"),   kREGInt,  &m_encodeSplitLen);
	SetItem(m_pTable + 5, _T("WordWrap"),      kREGInt,  &m_WordWrap);
	SetItem(m_pTable + 6, _T("IDsToRemember"), kREGInt,  &m_IDsToRemember);

	SetItem(m_pTable + 7, _T("MxOrigLines"),  kREGLong, &m_maxQuotedLines);
	SetItem(m_pTable + 8, _T("MxEncLines"),   kREGLong, &m_maxEncodedLines);

	SetItem(m_pTable + 9, _T("folIntro"),     kREGString, &m_folIntro);
	SetItem(m_pTable +10, _T("rplIntro"),     kREGString, &m_rplyIntro);
	SetItem(m_pTable +11, _T("MIME-CTE"),     kREGString, &m_MIME_ContentTransferEncoding);
	SetItem(m_pTable +12, _T("MIME_typ"),     kREGString, &m_MIME_ContentType);
	SetItem(m_pTable +13, _T("Indent"),       kREGString, &m_IndentString);
	SetItem(m_pTable +14, _T("subjTemplate"), kREGString, &m_attachSubjectTemplate);
	SetItem(m_pTable +15, _T("Dist"),         kREGString, &m_distribution);
	SetItem(m_pTable +16, _T("ReplyTo"),      kREGString, &m_replyTo);
	SetItem(m_pTable +17, _T("CCIntro"),      kREGString, &m_CCIntro, TRUE);
	SetItem(m_pTable +18, _T("CustomHeaders"),kREGMultiString, &m_strCustomHeaders, TRUE);
	SetItem(m_pTable +19, _T("SendCharset"),  kREGLong,   &m_iSendCharset, TRUE);
	SetItem(m_pTable +20, _T("Send8Bit"),     kREGLong,   &m_fAllow8Bit, TRUE);
	SetItem(m_pTable +21, _T("Send8BitHdrs"), kREGLong,   &m_f8BitHdrs, TRUE);
}

TRegCompose::~TRegCompose()
{
	delete [] m_pTable;
}

static void CustomHeadersToString (CStringList &sHeaders, CString &str)
{
	str = "";
	POSITION pos = sHeaders.GetHeadPosition ();
	while (pos) {
		CString &strHeader = sHeaders.GetNext (pos);
		str += strHeader + "\r\n";
	}
}

static void CustomHeadersFromString (CStringList &sHeaders, CString &str)
{
	sHeaders.RemoveAll ();
	CString strTemp = str;
	while (!strTemp.IsEmpty ()) {
		int iPos = strTemp.Find (_T("\r\n"));
		BOOL bFoundLineFeed = (iPos != -1);
		if (!bFoundLineFeed)
			iPos = strTemp.GetLength ();
		sHeaders.AddTail (strTemp.Left (iPos));
		strTemp = strTemp.Right (strTemp.GetLength () - iPos);
		if (bFoundLineFeed)
			strTemp = strTemp.Right (strTemp.GetLength () - 2);
	}
}

int TRegCompose::Load()
{
	int RC = TRegistryBase::Load ( GetGravityRegKey()+_T("Compose") );
	CustomHeadersFromString (m_sCustomHeaders, m_strCustomHeaders);
	return RC;
}

int TRegCompose::Save()
{
	CustomHeadersToString (m_sCustomHeaders, m_strCustomHeaders);
	return TRegistryBase::Save ( GetGravityRegKey()+_T("Compose") );
}

void TRegCompose::read()
{
	BYTE buf[128];
	int  sz=sizeof(buf);
	LONG lRet = rgReadTable (m_itemCount, m_pTable);

	// notify the global TIDArray object of its new setting
	(gpStore -> GetIDArray ()).SetMessageIDsSaved ( (unsigned short) m_IDsToRemember);

	CString strEncoding;
	lRet = rgReadString(_T("encode"), buf, sz, strEncoding);
	int tot = sizeof(m_EncMap)/sizeof(m_EncMap[0]);
	for (--tot; tot >= 0; --tot)
	{
		if (0 == strEncoding.CompareNoCase( m_EncMap[tot].name ) ) {
			m_kEncodingType = m_EncMap[tot].m_encoding;
			break;
		}
	}
	if (tot < 0)
		throw(new TException(IDS_ERR_RANGE, kError));
}

void TRegCompose::write()
{
	LONG lRet;

	lRet = rgWriteTable (m_itemCount, m_pTable);

	int tot = sizeof(m_EncMap)/sizeof(m_EncMap[0]);
	for (--tot; tot >= 0; --tot)
	{
		if (m_EncMap[tot].m_encoding == m_kEncodingType) {
			lRet = rgWriteString (_T("encode"), m_EncMap[tot].name);
			break;
		}
	}
	if (tot < 0)
		throw(new TException(IDS_ERR_RANGE, kError));
}

void TRegCompose::default_values()
{
	m_kEncodingType               = TGlobalDef::kMIME;
	m_IndentString.LoadString(IDS_DEF_INDENT);
	m_WordWrap                    = 73;
	m_IDsToRemember               = 20;
	m_fIncludeOriginalMail        = TRUE;
	m_fIncludeOriginalFollowup    = TRUE;
	m_maxQuotedLines              = 50;
	m_attachSubjectTemplate.LoadString(IDS_DEF_SUBJTEMPLATE);
	m_maxEncodedLines             = 1;    // ? used ?
	m_fLimitQuotedLines           = TRUE;
	m_folIntro.LoadString(IDS_DEF_INTRO_FOLLOW);
	m_rplyIntro.LoadString(IDS_DEF_INTRO_REPLY);
	m_CCIntro.LoadString (IDS_DEF_INTRO_CC);
	m_MIME_ContentTransferEncoding.LoadString (IDS_ENCODE_BASE64);
	m_MIME_ContentType.LoadString(IDS_CONTENT_OCTET);
	m_encodeSplitLen              = 64;   // KBytes
	m_fattachSeparateArt          = TRUE;
	m_iSendCharset                = 15;
	m_fAllow8Bit				  = TRUE;
	m_f8BitHdrs                   = TRUE;
}

void TRegCompose::SetCustomHeaders (CStringList &sCustomHeaders)
{
	CopyCStringList (m_sCustomHeaders, sCustomHeaders);
}
