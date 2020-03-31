/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: srchlvw.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:54  richard_wood
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

// srchlvw.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "srchlvw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TSearchListView

IMPLEMENT_DYNCREATE(TSearchListView, CView)

TSearchListView::TSearchListView()
{
}

TSearchListView::~TSearchListView()
{
}

BEGIN_MESSAGE_MAP(TSearchListView, CView)
		ON_WM_CREATE()
	ON_WM_SIZE()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSearchListView drawing

void TSearchListView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// TSearchListView diagnostics

#ifdef _DEBUG
void TSearchListView::AssertValid() const
{
	CView::AssertValid();
}

void TSearchListView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TSearchListView message handlers
void TSearchListView::Initialize ()
{
   //CListCtrl & rList = GetListCtrl ();

   // change the CListCtrl to our full owner-drawn version
   //m_results.SubclassWindow ( rList.m_hWnd );

	// Setup the columns of the List Control
   RECT  rct;
   GetParent()->GetParent()->GetClientRect (&rct);

   CString temp;
   temp.LoadString (IDS_TMPL_FROM);
   m_results.InsertColumn (0, temp, LVCFMT_LEFT, 
                           max((rct.right / 5) - 10, 0),
                           1);

   temp.LoadString (IDS_TMPL_SUBJECT);
   m_results.InsertColumn (1, temp, LVCFMT_LEFT, 
                           max((rct.right*2/5) - 10, 0),
                           2);

   temp.LoadString (IDS_TMPL_DATE);
   m_results.InsertColumn (2, temp, LVCFMT_LEFT, rct.right/10, 3);

   temp.LoadString (IDS_DL_GROUP);
   VERIFY (-1 != m_results.InsertColumn (3, temp, LVCFMT_LEFT,
                   rct.right*3/10, 4));
}

int TSearchListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
   RECT rct; ZeroMemory (&rct, sizeof rct);

   VERIFY (
	m_results.Create (LVS_ALIGNTOP | LVS_REPORT |
                        LVS_OWNERDRAWFIXED | LVS_NOSORTHEADER |
                        WS_VISIBLE,
                     rct,
                     this,
                     IDC_SEARCH_RESULTS) );
	
	return 0;
}

void TSearchListView::OnSize(UINT nType, int cx, int cy)
{
   CView::OnSize(nType, cx, cy);
   	
   if (::IsWindow (m_results.m_hWnd))
      m_results.MoveWindow (0, 0, cx, cy);
	
}

BOOL TSearchListView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// re-route to tsrchdlg.cpp
	return (BOOL) GetOwner()->SendMessage (WM_NOTIFY, wParam, lParam);	

	//return CView::OnNotify(wParam, lParam, pResult);
}
