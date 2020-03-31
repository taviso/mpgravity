/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: custview.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:20  richard_wood
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
#include "custview.h"
#include "resource.h"
#include "pobject.h"
#include "gdiutil.h"
#include "superstl.h"            // istrstream, ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(TArticleItemView, CObject, 1);
IMPLEMENT_SERIAL(TCustomArticleView, CObject, 1);

// this constructor is used by CArray
TArticleItemView::TArticleItemView()
{
	ZeroMemory (&m_lf, sizeof(m_lf));
	m_PointSize = 0;
	m_color = 0;
	m_itemID = 0;
}

TArticleItemView::TArticleItemView(int stringID, BYTE r, BYTE g, BYTE b)
{
	m_itemID = stringID;
	InitLogfont ( &m_lf );

	// this is (tenths of a point)
	m_PointSize = NEWS32_BASE_FONT_PTSIZE * 10;

	m_color = RGB(r,g,b);  // black = RGB(0,0,0)
}

/// global function
void TArticleItemView::InitLogfont(LOGFONT * pLF)
{
	HWND desktop = GetDesktopWindow();
	HDC  hdc = GetDC( desktop );

	setupSansSerifFont ( NEWS32_BASE_FONT_PTSIZE, hdc, pLF );

	ReleaseDC ( desktop, hdc );
}

TArticleItemView::~TArticleItemView(void)
{
}

BOOL TArticleItemView::SetNewFont (LPLOGFONT plf, int PointSize)
{
	m_PointSize = PointSize;
	CopyMemory (&m_lf, plf, sizeof(m_lf));
	return TRUE;
}

void TArticleItemView::Serialize (CArchive & ar)
{
	CObject::Serialize ( ar );
	if ( ar.IsStoring() )
	{
		ar.Write (&m_lf, sizeof(m_lf));
		ar << LONG(m_itemID);
		ar << m_color;
		ar << m_PointSize;
	}
	else
	{
		ar.Read (&m_lf, sizeof(m_lf));
		LONG tmp;
		ar >> tmp;   m_itemID = tmp;
		ar >> m_color;
		ar >> m_PointSize;
	}
}

TArticleItemView& TArticleItemView::operator= (const TArticleItemView& rhs)
{
	if (this == &rhs)
		return *this;

	m_itemID = rhs.m_itemID;
	m_color  = rhs.m_color;
	m_PointSize = rhs.m_PointSize;

	CopyMemory(&m_lf, &rhs.m_lf, sizeof(m_lf));

	return *this;
}

// copy ctor
TArticleItemView::TArticleItemView (const TArticleItemView& src)
{
	m_itemID = src.m_itemID;
	m_color  = src.m_color;
	m_PointSize = src.m_PointSize;

	CopyMemory(&m_lf, &src.m_lf, sizeof(m_lf));
}



// copy constructor
TArticleItemViewUI::TArticleItemViewUI(const TArticleItemViewUI& src)
	: text(src.text), m_item(src.m_item)
{
	m_pFont = new CFont;
	m_pFont->CreateFontIndirect ( &m_item.m_lf );
}

// normal constructor
TArticleItemViewUI::TArticleItemViewUI(const CString& str, int stringID)
	: text(str), m_item(stringID,0,0,0)
{
	m_pFont = new CFont;
	m_pFont->CreateFontIndirect ( &m_item.m_lf );
}

// takes the substructure as an initializer
TArticleItemViewUI::TArticleItemViewUI(const TArticleItemView& item)
	: m_item(item)
{
	text.LoadString ( m_item.GetStringID() );

	m_pFont = new CFont;

	BOOL ret;
	ret = m_pFont->CreateFontIndirect ( &m_item.m_lf );

	ASSERT(ret);
}

// Assignment
TArticleItemViewUI& TArticleItemViewUI::operator= (const TArticleItemViewUI& rhs)
{
	if (this == &rhs)
		return *this;

	text = rhs.text;
	m_item = rhs.m_item;
	ClearFont();

	m_pFont = new CFont;
	m_pFont->CreateFontIndirect ( &m_item.m_lf );

	return *this;
}

TArticleItemViewUI::~TArticleItemViewUI()
{
	ClearFont();
}

void TArticleItemViewUI::ClearFont(void)
{
	delete m_pFont;
	m_pFont = 0;
}

BOOL TArticleItemViewUI::SetNewFont (LPLOGFONT plf, int PointSize)
{
	m_item.SetNewFont (plf, PointSize);
	ClearFont();
	m_pFont = new CFont;
	m_pFont->CreateFontIndirect ( &m_item.m_lf );
	return TRUE;
}


template<> void AFXAPI
SerializeElements <TArticleItemView>(CArchive& ar, TArticleItemView* pItemView, int nCount)
{
	for (int i=0; i < nCount; ++i, ++pItemView)
		pItemView->Serialize ( ar );
}




TCustomArticleView::TCustomArticleView()
{
	AddDefaultItems();
}

TCustomArticleView& TCustomArticleView::operator=(const TCustomArticleView& rhs)
{
	int count = rhs.m_set.GetSize();
	m_set.SetSize(count);

	for (int i = 0; i < count; ++i)
	{
		m_set.ElementAt(i) = rhs.m_set.GetAt(i) ;
	}

	return *this;
}

void TCustomArticleView::Serialize (CArchive & ar)
{
	PObject::Serialize ( ar );

	m_set.Serialize (ar);
}


void TCustomArticleView::AddDefaultItems()
{
	int sub_red, sub_grn, sub_blu;
	int frm_red, frm_grn, frm_blu;
	int rpt[3];                      // replyto
	int ng[3];
	int flw[3];                      // followup to

	// Good Netkeeping states Followup-To and Reply-To should
	//  be shown if they differ. Skip them for now

	CString data;
	data.LoadString ( IDS_SUBJECTFROM_RGB );
	istrstream iss((LPTSTR) (LPCTSTR) data);
	iss >> sub_red >> sub_grn >> sub_blu
		>> frm_red >> frm_grn >> frm_blu
		>> rpt[0]  >> rpt[1]  >> rpt[2]
		>> ng[0]   >> ng[1]   >> ng[2]
		>> flw[0]  >> flw[1]  >> flw[2];

	TArticleItemView itemSubject(IDS_TMPL_SUBJECT,
		(BYTE)sub_red, (BYTE)sub_grn, (BYTE)sub_blu);
	TArticleItemView itemFrom(IDS_TMPL_FROM,
		(BYTE)frm_red, (BYTE)frm_grn, (BYTE)frm_blu);
	TArticleItemView itemReplyTo(IDS_TMPL_REPLYTO,
		(BYTE) rpt[0], (BYTE) rpt[1], (BYTE) rpt[2]);
	TArticleItemView itemNG(IDS_TMPL_NGROUP,
		(BYTE) ng[0], (BYTE) ng[1], (BYTE) ng[2]);
	TArticleItemView itemFol(IDS_TMPL_FOLLOWUP,
		(BYTE) flw[0], (BYTE) flw[1], (BYTE) flw[2]);

	TArticleItemView itemBlank(IDS_TMPL_BLANK, 0,0,0);
	TArticleItemView itemBody(IDS_TMPL_BODY, 0,0,0);

	Add ( itemSubject );
	Add ( itemFrom );
	Add ( itemReplyTo );
	Add ( itemNG );
	Add ( itemFol );
	Add ( itemBlank );
	Add ( itemBody );
}
