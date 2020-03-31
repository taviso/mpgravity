/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: splitter.cpp,v $
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
/*  Revision 1.4  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:53  richard_wood
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

// splitter.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "splitter.h"
#include "hints.h"         // VIEWHINT_UNZOOM

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TSplitterWnd

TSplitterWnd::TSplitterWnd()
{
	m_byZoomed = FALSE;
}

TSplitterWnd::~TSplitterWnd()
{
}

BEGIN_MESSAGE_MAP(TSplitterWnd, CSplitterWnd)
	// NOTE - the ClassWizard will add and remove mapping macros here.

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSplitterWnd message handlers

void TSplitterWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;	
	CSplitterWnd::PostNcDestroy();
}

void TSplitterWnd::my_get_info(int row, int col, SIZE& sz)
{
	int imax;
	int n;
	GetRowInfo(row, n, imax);  // get height of row
	sz.cy = n;
	GetColumnInfo(col, n, imax);  // get width of column
	sz.cx = n;
}

void TSplitterWnd::my_set_info(int row, int col, RECT& rct, SIZE& sz)
{
	sz.cx = rct.right;
	sz.cy = rct.bottom;
	SetRowInfo(row, sz.cy, 10 /* minimum */);
	SetColumnInfo(col, sz.cx, 10 /* minimum */);
}

BOOL TSplitterWnd::HaveView (const CView * pView, POINT * ppt)
{
	int iRowTot = GetRowCount();
	int iColTot = GetColumnCount();

	for (int r = 0; r < iRowTot; ++r)
	{
		for (int c = 0; c < iColTot; ++c)
			if (pView->m_hWnd == GetPane(r, c)->m_hWnd)
			{
				ppt->x = r;
				ppt->y = c;
				return TRUE;
			}
	}
	return FALSE;
}

//-------------------------------------------------------------------------
//
int  TSplitterWnd::ZoomView (CView * pView, POINT & pt)
{
	CWnd *pDadFrame = pView->GetParentFrame ();

	// attach directly to MDI child
	pView->SetParent(pDadFrame);

	// keep splitter window whole and happy with a dummy window
	RECT sRct; ZeroMemory(&sRct, sizeof(sRct));

	CWnd *pDummy = new CWnd;
	pDummy->Create("STATIC", "TEMP", WS_CHILD | WS_VISIBLE, sRct,
		this, pView->GetDlgCtrlID());

	ASSERT(::IsWindow(pDummy->m_hWnd));

	pDadFrame->GetClientRect(&sRct);

	pView->SetWindowPos(NULL, 0, 0, sRct.right, sRct.bottom, SWP_NOZORDER);

	m_pTravellingView = pView;
	m_byZoomed = TRUE;

	// notify the view that it is being Zoomed.  Mostly so the
	//  thread pane can use the Zoom column width settings
	CDocument * pDoc = m_pTravellingView->GetDocument();
	if (pDoc)
	{
		pDoc->UpdateAllViews(NULL, VIEWHINT_ZOOM, pView);
	}

	return 0;
}

//-------------------------------------------------------------------------
// If we are unzooming a window and it is visible, it is
//  slightly different than when we unzoom a window only to zoom a sibling.
int  TSplitterWnd::UnZoomView (BOOL fWindowIsVisible)
{
	// this is the inverse of IdFromRowCol
	int n = m_pTravellingView->GetDlgCtrlID();
	n -= AFX_IDW_PANE_FIRST;
	int row = n / 16;
	int col = n % 16;

	// kill the place holder window
	CWnd * pImpostor = GetPane (row, col);
	pImpostor->DestroyWindow ();
	delete pImpostor;

	// i'm back
	m_pTravellingView -> SetParent ( this );

	// do some voodoo stuff
	RecalcLayout ();

	// notify the view that it is being unzoomed
	CDocument * pDoc = m_pTravellingView->GetDocument();
	if (pDoc)
	{
		// this is derived from CObject
		TUnZoomHintInfo sInfo(fWindowIsVisible, m_pTravellingView);

		// last arg is a ptr to a CObject
		pDoc->UpdateAllViews (NULL, VIEWHINT_UNZOOM, &sInfo);
	}

	m_pTravellingView = NULL;
	m_byZoomed = FALSE;

	return 0;
}

//-------------------------------------------------------------------------
// the mdi child is resizing
void TSplitterWnd::ResizeZoomPane (int cx, int cy)
{
	ASSERT(m_byZoomed);
	ASSERT(m_pTravellingView);

	m_pTravellingView->MoveWindow(0, 0, cx, cy);
}

