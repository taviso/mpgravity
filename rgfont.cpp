/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgfont.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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
#include "rgfont.h"
#include "regutil.h"
#include "sysclr.h"
#include "gdiutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TSystem gSystem;

//-------------------------------------------------------------------------
TRegFonts::TRegFonts()
{
	default_values();
}
TRegFonts::~TRegFonts()
{
}

//-------------------------------------------------------------------------
int TRegFonts::Load()
{
	return TRegistryBase::Load ( GetGravityRegKey()+"Fonts" );
}

//-------------------------------------------------------------------------
int TRegFonts::Save()
{
	return TRegistryBase::Save ( GetGravityRegKey()+"Fonts" );
}

//-------------------------------------------------------------------------
void TRegFonts::read()
{
	LONG lRet;
	lRet = rgReadLogfont("Tree",     &m_treeFont, m_treePtSize);
	lRet = rgReadLogfont("Newsgroup",&m_ngFont,   m_ngPtSize);
	lRet = rgReadLogfont("Post",     &m_postFont, m_postPtSize);
	BYTE buf[80];
	try {
		lRet = rgReadRGB("TreeColor", buf, sizeof(buf), m_clrTree);
	}
	catch (TException *pE) {pE->Delete();}
	try {
		lRet = rgReadRGB("NGColor", buf, sizeof(buf), m_clrNewsgroup);
	}
	catch (TException *pE) {pE->Delete();}
	try
	{
		lRet = rgReadRGB("NewColor", buf, sizeof buf, m_clrNewArticle);
	}
	catch (TException *pE) {pE->Delete();}
	try
	{
		lRet = rgReadRGB("QuotedRGB", buf, sizeof buf, m_clrQuotedText);
	}
	catch (TException *pE) {pE->Delete();}

	LOGFONT lfQuotedText;
	if (ERROR_SUCCESS==rgReadLogfont("QuotedText", &lfQuotedText, m_QuotedTextPtSize))
		CopyMemory(&m_lfQuotedText, &lfQuotedText, sizeof m_lfQuotedText);

	try
	{
		lRet = rgReadRGB("SignatureRGB", buf, sizeof buf, m_clrSignatureText);
	}
	catch (TException *pE) {pE->Delete();}

	LOGFONT lfSignatureText;
	if (ERROR_SUCCESS==rgReadLogfont("SignatureText", &lfSignatureText, m_SignatureTextPtSize))
		CopyMemory(&m_lfSignatureText, &lfSignatureText, sizeof m_lfSignatureText);

	try
	{
		rgReadString("QuoteChars", buf, sizeof buf, m_QuoteChars);
	}
	catch (TException *pE) {pE->Delete();}

	rgReadLogfont ("Print", &m_printFont, m_iPrintPointSize);
	if (!m_printFont.lfFaceName [0]) {  // set defaults
		_tcscpy (m_printFont.lfFaceName, _T("MS Sans Serif"));
		m_printFont.lfHeight = -16; // just a good guess... seems bad to leave at 0
	}
	try { rgReadRGB ("PrintColor", buf, sizeof (buf), m_dwPrintColor); }
	catch (TException *pE) {pE->Delete();}
}

//-------------------------------------------------------------------------
void TRegFonts::write()
{
	LONG lRet;
	lRet = rgWriteLogfont("Tree",      &m_treeFont, m_treePtSize);
	lRet = rgWriteLogfont("Newsgroup", &m_ngFont,   m_ngPtSize);
	lRet = rgWriteLogfont("Post",      &m_postFont, m_postPtSize);
	lRet = rgWriteRGB("TreeColor",    m_clrTree);
	lRet = rgWriteRGB("NGColor",      m_clrNewsgroup);
	lRet = rgWriteRGB("NewColor",     m_clrNewArticle);
	lRet = rgWriteRGB("QuotedRGB",    m_clrQuotedText);
	lRet = rgWriteLogfont("QuotedText", &m_lfQuotedText, m_QuotedTextPtSize);
	lRet = rgWriteRGB("SignatureRGB",    m_clrSignatureText);
	lRet = rgWriteLogfont("SignatureText", &m_lfSignatureText, m_SignatureTextPtSize);
	lRet = rgWriteString("QuoteChars", m_QuoteChars);
	lRet = rgWriteLogfont ("Print", &m_printFont, m_iPrintPointSize);
	lRet = rgWriteRGB ("PrintColor", m_dwPrintColor);
}

//-------------------------------------------------------------------------
void TRegFonts::default_values()
{
	m_treePtSize = m_ngPtSize = m_iPrintPointSize = NEWS32_BASE_FONT_PTSIZE;
	ZeroMemory(&m_treeFont, sizeof(LOGFONT));
	ZeroMemory(&m_ngFont,   sizeof(LOGFONT));
	ZeroMemory(&m_postFont, sizeof(LOGFONT));
	ZeroMemory(&m_printFont, sizeof(LOGFONT));

	HWND desktop = GetDesktopWindow();
	HDC  hdc = GetDC( desktop );

	setupCourierFont ( NEWS32_BASE_FONT_PTSIZE, hdc, &m_postFont );

	// font for Quoted Text when Viewing - Posting uses a fixed-width font
	setupSansSerifFont ( NEWS32_BASE_FONT_PTSIZE, hdc, &m_lfQuotedText );
	m_QuotedTextPtSize = NEWS32_BASE_FONT_PTSIZE * 10;

	setupSansSerifFont ( NEWS32_BASE_FONT_PTSIZE, hdc, &m_lfSignatureText );
	m_SignatureTextPtSize = NEWS32_BASE_FONT_PTSIZE * 10;

	// note that RGB(64,128,128) does not match a font color
	m_clrQuotedText = RGB(0,128,128);

	ReleaseDC (desktop, hdc);

	m_postPtSize = NEWS32_BASE_FONT_PTSIZE * 10;   // this is pointSize * 10

	m_clrNewsgroup = m_clrTree = gSystem.WindowText();
	m_clrNewArticle = RGB(128,128,128);

	m_QuoteChars = ">:|}";

	m_dwPrintColor = RGB (0, 0, 0);  // black is default color for printing
}

//-------------------------------------------------------------------------
const LOGFONT* TRegFonts::GetTreeFont ()
{
	return &m_treeFont;
}

//-------------------------------------------------------------------------
void  TRegFonts::SetTreeFont (const LOGFONT* pLF)
{
	CopyMemory(&m_treeFont, pLF, sizeof(m_treeFont));
}

//-------------------------------------------------------------------------
const LOGFONT* TRegFonts::GetNewsGroupFont ()
{
	return &m_ngFont;
}

//-------------------------------------------------------------------------
void  TRegFonts::SetNewsGroupFont (const LOGFONT* pLF)
{
	CopyMemory(&m_ngFont, pLF, sizeof(m_ngFont));
}

//-------------------------------------------------------------------------
const LOGFONT* TRegFonts::GetQuotedTextFont ()
{
	return &m_lfQuotedText;
}

//-------------------------------------------------------------------------
void  TRegFonts::SetQuotedTextFont (const LOGFONT* pLF)
{
	CopyMemory( &m_lfQuotedText, pLF, sizeof(m_lfQuotedText));
}

//-------------------------------------------------------------------------
const LOGFONT* TRegFonts::GetSignatureTextFont ()
{
	return &m_lfSignatureText;
}

//-------------------------------------------------------------------------
void  TRegFonts::SetSignatureTextFont (const LOGFONT* pLF)
{
	CopyMemory( &m_lfSignatureText, pLF, sizeof(m_lfSignatureText));
}

//-------------------------------------------------------------------------
const LOGFONT* TRegFonts::GetPrintFont ()
{
	return &m_printFont;
}

//-------------------------------------------------------------------------
void TRegFonts::SetPrintFont (const LOGFONT* pLF)
{
	CopyMemory(&m_printFont, pLF, sizeof(m_treeFont));
}
