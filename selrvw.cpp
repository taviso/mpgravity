/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: selrvw.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 15:48:56  richard_wood
/*  Stopped the Undo buffer in the compose window from filling up with colour wrap operations.
/*  Undo does nothing at all for the moment (needs proper fix).
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:49  richard_wood
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

// selrvw.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "selrvw.h"
#include "rtfspt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TSelectRichEditView

IMPLEMENT_DYNCREATE(TSelectRichEditView, CRichEditView)

TSelectRichEditView::TSelectRichEditView()
{
	m_pRich = &GetRichEditCtrl();//NULL;
	m_fWordWrap = FALSE;
}

TSelectRichEditView::~TSelectRichEditView()
{
}

BEGIN_MESSAGE_MAP(TSelectRichEditView, CRichEditView)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSelectRichEditView drawing

void TSelectRichEditView::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();

	// this mimics TBaseFormView::OnInitialUpdate
	// for big messages, give the RTF more space for the user to type into
	m_pRich->SendMessage(EM_EXLIMITTEXT, 0, 300000);
}

void TSelectRichEditView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// TSelectRichEditView diagnostics

#ifdef _DEBUG
void TSelectRichEditView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void TSelectRichEditView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TSelectRichEditView message handlers

///////////////////////////////////////////////////////////////////////////
//
void TSelectRichEditView::SetWrapWidth(BOOL fWrap)
{
	m_fWordWrap = fWrap;
}

//////////////////////////////////////////////////////////////////////////
//  Find the limits of the paragraph that contains 'iCharPos'
//
int TSelectRichEditView::GetParagraphBounds(int iCharPos, CHARRANGE* pchrg)
{
	return RTF_GetParagraphBounds(m_pRich, iCharPos, pchrg);
}

//////////////////////////////////////////////////////////////////////////
//  Return TRUE if color is consistent throughout selection
//
BOOL TSelectRichEditView::IsSelectionColorConsistent(COLORREF color)
{
	CHARFORMAT cfm;
	cfm.cbSize = sizeof(cfm);
	cfm.dwMask = CFM_COLOR;

	// get character attributes of selection
	m_pRich->GetSelectionCharFormat ( cfm );

	// see is color is uniform throughout ...      and matches
	if ((cfm.dwMask & CFM_COLOR) && (color == cfm.crTextColor))
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////
//  Set the color of the current selection. Selection must already be set
//
BOOL TSelectRichEditView::SetSelectionColor(COLORREF color)
{
	CHARFORMAT cfm;
	// strictly speaking we shouldn't need to Zero it out. But it works!!
	ZeroMemory( &cfm, sizeof cfm );
	cfm.cbSize = sizeof(cfm);
	cfm.dwMask = CFM_COLOR;
	cfm.crTextColor = color;

	return m_pRich->SetSelectionCharFormat ( cfm );
}

///////////////////////////////////////////////////////////////////////////
// QueryAcceptData -- tell the framework that the user can't drag a file
// onto this view
HRESULT TSelectRichEditView::QueryAcceptData (LPDATAOBJECT lpdataobj,
											  CLIPFORMAT* lpcfFormat, DWORD dwReco, BOOL bReally, HGLOBAL hMetaPict)
{
	IDataObject *pObject = lpdataobj;
	IEnumFORMATETC *pEnum;
	HRESULT RC = pObject->EnumFormatEtc(DATADIR_GET, &pEnum);
	if (RC != S_OK)
		return S_FALSE;

	FORMATETC rsFormat [1];
	do
	{
		RC = pEnum -> Next (1, rsFormat, NULL);
		if (RC == S_OK && rsFormat [0].cfFormat == CF_HDROP)
			// user is trying to drop files into the rich-edit... this caused
			// a crash so we disallow it
			return S_FALSE;
	} while (RC == S_OK);

	return CRichEditView::QueryAcceptData (lpdataobj, lpcfFormat, dwReco,
		bReally, hMetaPict);
}
