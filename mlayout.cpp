/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mlayout.cpp,v $
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
/*  Revision 1.4  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:31  richard_wood
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
#include "News.h"
#include "mplib.h"
#include "globals.h"          // gpUIMemory

#include "mlayout.h"

#include "thrdlvw.h"
#include "artview.h"          // FormView
#include "uimem.h"
#include "tglobopt.h"
#include "rglaymdi.h"
#include "rgswit.h"
#include "CompFrm.h" // TComposeFrame

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

extern BYTE gfFirstUse;

TMdiLayout::TMdiLayout(int count)
{
	m_fFreemem = FALSE;

	m_pwndSplit1  = 0;
	m_pwndSplit2  = 0;
	if (count >= 1)
		m_pwndSplit1 = new TSplitterWnd;
	if (count >= 2)
		m_pwndSplit2 = new TSplitterWnd;

	pre_create_view();
}

TMdiLayout::~TMdiLayout()
{
	if (m_fFreemem)
	{
		if (m_pwndSplit1)
			m_pwndSplit1->DestroyWindow();

		// since split2 is an offshoot of Split1 it may be dead already
		if (m_pwndSplit2 && IsWindow(m_pwndSplit2->m_hWnd))
			m_pwndSplit2->DestroyWindow();
	}
}

void TMdiLayout::pre_create_view()
{
	m_pInitNewsView = 0;
	m_pInitThreadView = 0;
	m_pInitArtView = 0;
}

BOOL TMdiLayout::make_a_view (
							  CSplitterWnd* splitWnd,
							  SIZE& size,
							  CCreateContext* pContext,
							  int iWhich,
							  int row,
							  int col
							  )
{
	TUIMemory::EPaneType ePane =
		gpGlobalOptions->GetRegLayoutMdi()->m_vPanes[iWhich];

	BOOL ret;
	switch (ePane)
	{
	case TUIMemory::PANE_NEWSGROUPS:
		//ret = splitWnd->CreateView(row, col, pContext->m_pNewViewClass, size, pContext);
		ret = splitWnd->CreateView(row, col, RUNTIME_CLASS(CNewsView), size, pContext);
		m_pInitNewsView = (CNewsView*) splitWnd->GetPane(row, col);
		break;

	case TUIMemory::PANE_THREADVIEW:
		ret = splitWnd->CreateView(row, col, RUNTIME_CLASS(TThreadListView), size, pContext);
		m_pInitThreadView = (TThreadListView*) splitWnd->GetPane(row, col);
		break;

	case TUIMemory::PANE_ARTVIEW:
		ret = splitWnd->CreateView(row, col, RUNTIME_CLASS(TArticleFormView), size, pContext);
		m_pInitArtView = (TArticleFormView*) splitWnd->GetPane(row,col);
		break;
	}
	return ret;
}

void TMdiLayout::three_pane_linkup(void)
{
	ASSERT(m_pInitNewsView);
	ASSERT(m_pInitThreadView );
	ASSERT( m_pInitArtView );

	// link up the threadView to the newsView (back-pointer)
	m_pInitThreadView->SetNewsView ( m_pInitNewsView );
	m_pInitThreadView->SetArtView ( m_pInitArtView );

	m_pInitArtView->SetTitleView ( m_pInitThreadView );
}

int TMdiLayout::Save(TRegLayoutMdi* pLayMdi)
{
	ASSERT(0);
	return 0;
}

void TMdiLayout::transfer_pane(int iWhichSplitter, int row, int col, RECT& rct)
{
	SIZE aSize;
	if (1 == iWhichSplitter)
	{
		m_pwndSplit1->my_get_info(row,col,aSize);
	}
	else if (2 == iWhichSplitter)
	{
		m_pwndSplit2->my_get_info(row,col,aSize);
	}
	rct.top = rct.left = 0;
	rct.bottom = aSize.cy;
	rct.right  = aSize.cx;
}

void TMdiLayout::fill_pane_defsize(SIZE* pSZ, int Count)
{
	for (--Count; Count >= 0; --Count, pSZ++) {
		pSZ->cx = PANE_CX_DEF;
		pSZ->cy = PANE_CY_DEF;
	}
}

//////////////////////////////////////////////////////////////////////
// Called from the mdi-child window
//  see if any the views handles the message
BOOL TMdiLayout::OnCmdMsg(
						  UINT nID,
						  int nCode,
						  void* pExtra,
						  AFX_CMDHANDLERINFO* pHandlerInfo,
						  CView* pActiveView,
						  int    iMRUTitleView
						  )
{
#if defined(_DEBUG)
	if (nID == ID_ARTICLE_DELETE_SELECTED)
	{
		int jj = 0;
		jj++;             // set breakpoint here
	}
#endif

	// Active view has 1st crack
	if (pActiveView)
	{
		if (pActiveView == m_pInitNewsView &&
			m_pInitNewsView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
		if (pActiveView == m_pInitThreadView &&
			m_pInitThreadView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
		if (pActiveView == m_pInitArtView &&
			m_pInitArtView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	// check against the active view.(which already had a crack at it)
	if (m_pInitNewsView && (m_pInitNewsView != pActiveView) &&
		m_pInitNewsView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// if the artview has focus, then the MRU titleview has the honor
	if (m_pInitThreadView && (m_pInitThreadView != pActiveView) &&
		m_pInitThreadView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	if (m_pInitArtView && (m_pInitArtView != pActiveView) &&
		m_pInitArtView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return FALSE;
}

//-------------------------------------------------------------------------
//

void TMdiLayout::ApplyNewNGFont()
{
	ASSERT(m_pInitNewsView);
	m_pInitNewsView->ApplyNewFont();
}

void TMdiLayout::ApplyNewTreeFont()
{
	ASSERT(m_pInitThreadView);
	m_pInitThreadView->ApplyNewFont();
}

void TMdiLayout::ApplyViewTemplate()
{
	if (m_pInitArtView)
		m_pInitArtView->ApplyViewTemplate();
}

// 0 = newsview   2 = artview
// 1 = thrdlvw
int TMdiLayout::NextPane(int iCurPane, BOOL fForward)
{
	TUIMemory::EPaneType eCurPane;
	int i;
	// convert from int to ENUM
	switch (iCurPane)
	{
	case 0:
		eCurPane = TUIMemory::PANE_NEWSGROUPS;
		break;
	case 1:
		eCurPane = TUIMemory::PANE_THREADVIEW;
		break;
	case 2:
		eCurPane = TUIMemory::PANE_ARTVIEW;
		break;
	}

	TUIMemory::EMdiLayout layout;
	layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;
	int panecount = gpUIMemory->LayoutPaneCount(layout);
	ASSERT(iCurPane >= 0 && iCurPane < panecount);

	// search
	for (i = 0; i < panecount; i++)
	{
		if (eCurPane == gpGlobalOptions->GetRegLayoutMdi()->m_vPanes[i])
			break;
	}
	// increment index
	i += (fForward) ? 1 : (panecount-1);
	i = i % panecount;

	// and find the ENUM in the array
	eCurPane = gpGlobalOptions->GetRegLayoutMdi()->m_vPanes[i];

	// convert from ENUM to int
	switch (eCurPane)
	{
	case TUIMemory::PANE_NEWSGROUPS:
		i = 0;
		break;
	case TUIMemory::PANE_THREADVIEW:
		i = 1;
		break;
	case TUIMemory::PANE_ARTVIEW:
		i = 2;
		break;
	}
	ASSERT((i >= 0) && (i<3));
	return i;
}

//-------------------------------------------------------------------------
//
void TMdiLayout::SetActiveView(CFrameWnd* pFrame, int iFocusWnd)
{
	CView* pView = get_view_from_code (iFocusWnd);
	if (pView && ::IsWindow(pView->m_hWnd))
		pFrame->SetActiveView ( pView );
}

// -------------------------------------------------------------------------
CView* TMdiLayout::get_view_from_code (int iFocusWnd)
{
	CView* pView;
	switch (iFocusWnd)
	{
	case 0:
		pView = m_pInitNewsView;
		break;
	case 1:
		pView = GetThreadView();
		break;
	case 2:
		pView = GetArtFormView();
		break;
	default:
		throw(new TException (IDS_ERR_FOCUS, kFatal));
	}

	return pView;
}

// -------------------------------------------------------------------------
// public function , override a virtual.
// GOAL:  Zoom or UnZoom a given pane
//  fIsVisible indicates whether view will be visible after unzooming
int TMdiLayout::Zoom (int iFocus, BOOL fZoom, BOOL fIsVisible /* =TRUE */)
{
	CView * pView = get_view_from_code (iFocus);

	if (fZoom)
	{
		TSplitterWnd * pSplit = 0;
		POINT pt;
		BOOL fFound = FALSE;
		if (m_pwndSplit1)
		{
			fFound = m_pwndSplit1->HaveView (pView, &pt);
			pSplit = m_pwndSplit1;
		}
		if (!fFound && m_pwndSplit2)
		{
			fFound = m_pwndSplit2->HaveView (pView, &pt);
			pSplit = m_pwndSplit2;
		}
		ASSERT(fFound);

		pSplit->ZoomView (pView, pt);

		// hide the rest
		m_pwndSplit1->ShowWindow (SW_HIDE);
	}
	else
	{
		if (m_pwndSplit1 && m_pwndSplit1->IsZoomed())
		{
			m_pwndSplit1->UnZoomView (fIsVisible);
		}
		else if (m_pwndSplit2 && m_pwndSplit2->IsZoomed())
		{
			m_pwndSplit2->UnZoomView (fIsVisible);
		}

		// Show everybody
		m_pwndSplit1->ShowWindow ( SW_SHOW );
	}

	return 0;
}

//-------------------------------------------------------------------------
// the mdi child is resizing
void TMdiLayout::OnSize(UINT nType, int cx, int cy)
{
	if (SIZE_MINIMIZED == nType)
		return;

	if (m_pwndSplit2 && m_pwndSplit2->IsZoomed())
		m_pwndSplit2->ResizeZoomPane(cx, cy);
	else if (m_pwndSplit1 && m_pwndSplit1->IsZoomed())
		m_pwndSplit1->ResizeZoomPane(cx, cy);
}

//-------------------------------------------------------------------------
BOOL TMdiLayout::IsZoomed ()
{
	return (m_pwndSplit1 && m_pwndSplit1->IsZoomed()) ||
		(m_pwndSplit2 && m_pwndSplit2->IsZoomed());
}

//-------------------------------------------------------------------------
//
void TMdiLayout::UserActionArrange(TGlobalDef::EUserAction eAction,
								   CFrameWnd * pFrame,
								   int * piFocus,
								   int   iNewFocus)
{
	if (FALSE == IsZoomed())
	{
		switch (eAction)
		{
		case TGlobalDef::kActionOpenArticle:

			if ( gpGlobalOptions->GetRegSwitch()->GetReadPutFocusToArtpane() )
			{
				// v2.20   mimic agent: put focus into the article pane
				//   from there the user can navigate around with Skip, or
				//   single-key-read
				//
				// If the active window is a compose window, do NOT
				// make the article view the active view.
				//
				// If a compose window is the active window it means the user
				// is following up an article, and if we adjust the active view
				// here we will hide the compose view - very annoying!
				CWnd *pActive = AfxGetApp()->GetMainWnd()->GetActiveWindow();
				if (!pActive || !pActive->IsKindOf(RUNTIME_CLASS(TComposeFrame)))
					SetActiveView (pFrame, 2);
			}
			else
			{
				// leave focus in the thread pane, like Gravity 2.1
			}
			break;
		}
		return;
	}

	switch (eAction)
	{
	case TGlobalDef::kActionOpenGroup:
		// click on NewsGroup Pane => ThreadPane
		ZoomTo ( pFrame, piFocus, 1 );

		if (m_pInitThreadView->GetCount() > 0)
			m_pInitThreadView->SetSel (0, true);  // select index 0

		break;

	case TGlobalDef::kActionOpenArticle:
		// click on ThreadPane => Show the article pane
		ZoomTo ( pFrame, piFocus, 2 );
		break;

	case TGlobalDef::kActionTabAround:
		ZoomTo ( pFrame, piFocus, iNewFocus );
		break;
	}
}

//-------------------------------------------------------------------------
// - protected function -
int TMdiLayout::ZoomTo(CFrameWnd * pFrame, int * piFocus, int iCodeOfNewView)
{
	BOOL fUpdateLocked = pFrame->LockWindowUpdate ();

	// unzoom whoever is zoomed
	Zoom (*piFocus,
		FALSE,   // zoom is false
		FALSE);  // fVisible.

	// ! window is not visible, because another window will be zoomed
	//  on top of it in the following lines of code.

	// forcibly move to some specified Pane
	*piFocus = iCodeOfNewView;
	Zoom (*piFocus, TRUE);

	if (fUpdateLocked)
		pFrame->UnlockWindowUpdate ();

	SetActiveView ( pFrame, *piFocus );
	return 0;
}

// ------------------------------------------------------------------------
// ZoomEscape -- public function. Returns 0 for success.
//
int TMdiLayout::ZoomEscape(CFrameWnd * pFrame, int * piFocus)
{
	// It's like popping the stack

	switch (*piFocus)
	{
	case 0:
		break;

	case 1:
		// go from Thread to NewsGroups
		ZoomTo ( pFrame, piFocus, 0 );
		break;
	case 2:
		// go from article to Thread
		ZoomTo ( pFrame, piFocus, 1 );
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	return 0;
}

// ------------------------------------------------------------------------
// Goal: I need to do something intelligent about the column widths
//       especially when a new layout changes the width of a pane
int TMdiLayout::RecalcPaneSizes(TRegLayoutMdi* pOrig)
{
	// Get the current settings. Pane roles and sizes
	TRegLayoutMdi sCur;
	TRegLayoutMdi * pGlobal = gpGlobalOptions->GetRegLayoutMdi();
	for (int n = 0; n < 3; ++n)
		sCur.m_vPanes[n] = pGlobal->m_vPanes[n];
	Save (&sCur);

	POINT ptNG;
	POINT ptTH;
	RECT* vpOrigRct[3];
	vpOrigRct[0] = &pOrig->m_rct1;
	vpOrigRct[1] = &pOrig->m_rct2;
	vpOrigRct[2] = &pOrig->m_rct3;

	RECT* vpCurRct[3];
	vpCurRct[0] = &sCur.m_rct1;
	vpCurRct[1] = &sCur.m_rct2;
	vpCurRct[2] = &sCur.m_rct3;

	// I have to Organize my data, into a useable format.
	for (int i = 0; i < 3; ++i)
	{
		int cxOrig = vpOrigRct[i]->right - vpOrigRct[i]->left;
		int cxCur  = vpCurRct[i]->right  - vpCurRct[i]->left;

		// originals go into X slot
		switch (pOrig->m_vPanes[i])
		{
		case TUIMemory::PANE_NEWSGROUPS:
			ptNG.x = cxOrig;
			break;
		case TUIMemory::PANE_THREADVIEW:
			ptTH.x = cxOrig;
			break;
		}

		// current width go into Y slot
		switch (sCur.m_vPanes[i])
		{
		case TUIMemory::PANE_NEWSGROUPS:
			ptNG.y = cxCur;
			break;

		case TUIMemory::PANE_THREADVIEW:
			ptTH.y = cxCur;
			break;
		}

	} // for loop

	// has width of the ThreadList pane shrunk?
	if (ptTH.x > ptTH.y)
		m_pInitThreadView->AdjustForSmallerWidth();
	return 0;
}

//-------------------------------------------------------------------------
//  B E G I N      D E R I V E D     C L A S S E S
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//
//  +-+-+
//  |1|2|
//  +-+-+
//  |3  |
//  +---+
TMdiLayout3_2Top::TMdiLayout3_2Top(CWnd* pWnd, CCreateContext* pContext)
: TMdiLayout(2)
{
	// primary a splitter with 2 row, 1 column
	if (!m_pwndSplit1->CreateStatic(pWnd, 2, 1))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// vert split
	if (!m_pwndSplit2->CreateStatic(
		m_pwndSplit1,
		1, 2,              // the 2nd split shall be 1row 2col
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		m_pwndSplit1->IdFromRowCol(0,0)
		))
	{
		TRACE0("Failed to create nested splitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// default heights
	CSize vSize[3];
	fill_pane_defsize(vSize, 3);

	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (pRegLay->m_fSizeInfo)
	{
		m_pwndSplit2->my_set_info(0,0, pRegLay->m_rct1, vSize[0]);
		m_pwndSplit2->my_set_info(0,1, pRegLay->m_rct2, vSize[1]);
		m_pwndSplit1->my_set_info(1,0, pRegLay->m_rct3, vSize[2]);

		m_pwndSplit1->SetRowInfo(0, vSize[0].cy, PANE_CY_MIN);
	}
	else
	{
		RECT rct; pWnd->GetClientRect(&rct);
		m_pwndSplit1->SetRowHeight(0, rct.bottom/2);
		m_pwndSplit1->SetRowHeight(1, rct.bottom/2);

		// If first time, then give more room to the treeView Pane
		int treeViewCX;
		if (gfFirstUse)
			vSize[0].cx = rct.right * 2 / 7;
		else
			vSize[0].cx = rct.right / 2;

		treeViewCX = rct.right - vSize[0].cx;
		vSize[1].cx = max(vSize[1].cx, treeViewCX);
	}
	// 2 top
	pre_create_view();
	make_a_view(m_pwndSplit2, vSize[0], pContext,0, 0, 0);
	make_a_view(m_pwndSplit2, vSize[1], pContext,1, 0, 1);
	make_a_view(m_pwndSplit1, vSize[2], pContext,2, 1, 0);

	three_pane_linkup ();
}

TMdiLayout3_2Top::~TMdiLayout3_2Top()
{}

int TMdiLayout3_2Top::Save(TRegLayoutMdi* pLayMdi)
{
	pLayMdi->m_fSizeInfo = TRUE;
	transfer_pane(2, 0,0, pLayMdi->m_rct1);
	transfer_pane(2, 0,1, pLayMdi->m_rct2);
	transfer_pane(1, 1,0, pLayMdi->m_rct3);
	return 0;
}

//-------------------------------------------------------------------------
//
TMdiLayout3_2Bottom::TMdiLayout3_2Bottom(CWnd* pWnd, CCreateContext* pContext)
: TMdiLayout(2)
{
	// primary a splitter with 2 row, 1 column
	if (!m_pwndSplit1->CreateStatic(pWnd, 2, 1))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// vert split
	if (!m_pwndSplit2->CreateStatic(
		m_pwndSplit1,
		1, 2,              // the 2nd split shall be 1row 2col
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		m_pwndSplit1->IdFromRowCol(1,0)
		))
	{
		TRACE0("Failed to create nested splitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// default heights
	CSize vSize[3];
	fill_pane_defsize(vSize, 3);

	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (pRegLay->m_fSizeInfo)
	{
		m_pwndSplit1->my_set_info(0,0, pRegLay->m_rct1, vSize[0]);
		m_pwndSplit2->my_set_info(0,0, pRegLay->m_rct2, vSize[1]);
		m_pwndSplit2->my_set_info(0,1, pRegLay->m_rct3, vSize[2]);

		m_pwndSplit1->SetRowInfo(1, vSize[1].cy, PANE_CY_MIN);
	}
	else
	{
		RECT rct; pWnd->GetClientRect(&rct);
		m_pwndSplit1->SetRowHeight(0, rct.bottom/2);
		m_pwndSplit1->SetRowHeight(1, rct.bottom/2);
		vSize[1].cx = rct.right/2;
	}

	// 2 bottom
	pre_create_view();
	make_a_view(m_pwndSplit1, vSize[0], pContext,0, 0, 0);
	make_a_view(m_pwndSplit2, vSize[1], pContext,1, 0, 0);
	make_a_view(m_pwndSplit2, vSize[2], pContext,2, 0, 1);

	three_pane_linkup ();
}

TMdiLayout3_2Bottom::~TMdiLayout3_2Bottom()
{}
int TMdiLayout3_2Bottom::Save(TRegLayoutMdi* pLayMdi)
{
	pLayMdi->m_fSizeInfo = TRUE;
	transfer_pane(1, 0,0, pLayMdi->m_rct1);
	transfer_pane(2, 0,0, pLayMdi->m_rct2);
	transfer_pane(2, 0,1, pLayMdi->m_rct3);
	return 0;
}

TMdiLayout3_2Right::TMdiLayout3_2Right(CWnd* pWnd, CCreateContext* pContext)
: TMdiLayout(2)
{
	// primary a splitter with 1 row, 2 column
	if (!m_pwndSplit1->CreateStatic(pWnd, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// vert split
	if (!m_pwndSplit2->CreateStatic(
		m_pwndSplit1,
		2, 1,              // the 2nd split shall be 2row 1col
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		m_pwndSplit1->IdFromRowCol(0,1)
		))
	{
		TRACE0("Failed to create nested splitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// default heights
	CSize vSize[3];
	fill_pane_defsize(vSize, 3);

	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (pRegLay->m_fSizeInfo)
	{
		m_pwndSplit1->my_set_info(0,0, pRegLay->m_rct1, vSize[0]);
		m_pwndSplit2->my_set_info(0,0, pRegLay->m_rct2, vSize[1]);
		m_pwndSplit2->my_set_info(1,0, pRegLay->m_rct3, vSize[2]);

		m_pwndSplit1->SetColumnInfo(1, vSize[1].cx, PANE_CX_MIN);
	}
	else
	{
		RECT rct; pWnd->GetClientRect(&rct);
		m_pwndSplit1->SetColWidth(0, rct.right/2);
		m_pwndSplit1->SetColWidth(1, rct.right/2);
		vSize[1].cy = rct.bottom/2;
	}
	// 2 right
	pre_create_view();
	make_a_view(m_pwndSplit1, vSize[0], pContext,0, 0, 0);
	make_a_view(m_pwndSplit2, vSize[1], pContext,1, 0, 0);
	make_a_view(m_pwndSplit2, vSize[2], pContext,2, 1, 0);

	three_pane_linkup ();
}

TMdiLayout3_2Right::~TMdiLayout3_2Right()
{}
int TMdiLayout3_2Right::Save(TRegLayoutMdi* pLayMdi)
{
	pLayMdi->m_fSizeInfo = TRUE;
	transfer_pane(1, 0,0, pLayMdi->m_rct1);
	transfer_pane(2, 0,0, pLayMdi->m_rct2);
	transfer_pane(2, 1,0, pLayMdi->m_rct3);
	return 0;
}

TMdiLayout3_2Left::TMdiLayout3_2Left(CWnd* pWnd, CCreateContext* pContext)
: TMdiLayout(2)
{
	// primary a splitter with 1 row, 2 column
	if (!m_pwndSplit1->CreateStatic(pWnd, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// vert split
	if (!m_pwndSplit2->CreateStatic(
		m_pwndSplit1,
		2, 1,              // the 2nd split shall be 2row 1col
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		m_pwndSplit1->IdFromRowCol(0,0)
		))
	{
		TRACE0("Failed to create nested splitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// default heights
	CSize vSize[3];
	fill_pane_defsize(vSize, 3);

	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (pRegLay->m_fSizeInfo)
	{
		m_pwndSplit2->my_set_info(0,0, pRegLay->m_rct1, vSize[0]);
		m_pwndSplit2->my_set_info(1,0, pRegLay->m_rct2, vSize[1]);
		m_pwndSplit1->my_set_info(0,1, pRegLay->m_rct3, vSize[2]);

		m_pwndSplit1->SetColumnInfo(0, vSize[1].cx, PANE_CX_MIN);
	}
	else
	{
		RECT rct; pWnd->GetClientRect(&rct);
		m_pwndSplit1->SetColWidth(0, rct.right/2);
		m_pwndSplit1->SetColWidth(1, rct.right/2);
		vSize[0].cy = rct.bottom/2;
	}
	// 2 right
	pre_create_view();
	make_a_view(m_pwndSplit2, vSize[0], pContext,0, 0, 0);
	make_a_view(m_pwndSplit2, vSize[1], pContext,1, 1, 0);
	make_a_view(m_pwndSplit1, vSize[2], pContext,2, 0, 1);

	three_pane_linkup ();
}

TMdiLayout3_2Left::~TMdiLayout3_2Left()
{}
int TMdiLayout3_2Left::Save(TRegLayoutMdi* pLayMdi)
{
	pLayMdi->m_fSizeInfo = TRUE;
	transfer_pane(2, 0,0, pLayMdi->m_rct1);
	transfer_pane(2, 1,0, pLayMdi->m_rct2);
	transfer_pane(1, 0,1, pLayMdi->m_rct3);
	return 0;
}

TMdiLayout3_3High::TMdiLayout3_3High(CWnd* pWnd, CCreateContext* pContext)
: TMdiLayout(1)
{
	// primary a splitter with 3 row, 1 column
	if (!m_pwndSplit1->CreateStatic(pWnd, 3, 1))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// default heights
	CSize vSize[3];
	fill_pane_defsize(vSize, 3);

	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (pRegLay->m_fSizeInfo)
	{
		m_pwndSplit1->my_set_info(0,0, pRegLay->m_rct1, vSize[0]);
		m_pwndSplit1->my_set_info(1,0, pRegLay->m_rct2, vSize[1]);
		m_pwndSplit1->my_set_info(2,0, pRegLay->m_rct3, vSize[2]);

		m_pwndSplit1->SetRowInfo(1, vSize[1].cy, PANE_CY_MIN);
	}
	else
	{
		RECT rct; pWnd->GetClientRect(&rct);
		m_pwndSplit1->SetRowHeight(0, rct.bottom/3);
		m_pwndSplit1->SetRowHeight(1, rct.bottom/3);
		m_pwndSplit1->SetRowHeight(2, rct.bottom/3);
	}
	// 3 high
	pre_create_view();
	make_a_view(m_pwndSplit1, vSize[0], pContext,0, 0, 0);
	make_a_view(m_pwndSplit1, vSize[1], pContext,1, 1, 0);
	make_a_view(m_pwndSplit1, vSize[2], pContext,2, 2, 0);

	three_pane_linkup ();
}

TMdiLayout3_3High::~TMdiLayout3_3High() {}
int TMdiLayout3_3High::Save(TRegLayoutMdi* pLayMdi)
{
	pLayMdi->m_fSizeInfo = TRUE;
	transfer_pane(1, 0,0, pLayMdi->m_rct1);
	transfer_pane(1, 1,0, pLayMdi->m_rct2);
	transfer_pane(1, 2,0, pLayMdi->m_rct3);
	return 0;
}

TMdiLayout3_3Wide::TMdiLayout3_3Wide(CWnd* pWnd, CCreateContext* pContext)
: TMdiLayout(1)
{
	// primary a splitter with 1 row, 3 column
	if (!m_pwndSplit1->CreateStatic(pWnd, 1, 3))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		throw(new TException(IDS_ERR_WND_CREATE, kError));
	}

	// default heights
	CSize vSize[3];
	fill_pane_defsize(vSize, 3);

	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (pRegLay->m_fSizeInfo)
	{
		m_pwndSplit1->my_set_info(0,0, pRegLay->m_rct1, vSize[0]);
		m_pwndSplit1->my_set_info(0,1, pRegLay->m_rct2, vSize[1]);
		m_pwndSplit1->my_set_info(0,2, pRegLay->m_rct3, vSize[2]);
	}
	else
	{
		RECT rct; pWnd->GetClientRect(&rct);
		m_pwndSplit1->SetColWidth(0, rct.right/3);
		m_pwndSplit1->SetColWidth(1, rct.right/3);
		m_pwndSplit1->SetColWidth(2, rct.right/3);
	}

	// 3 wide
	pre_create_view();
	make_a_view(m_pwndSplit1, vSize[0], pContext,0, 0, 0);
	make_a_view(m_pwndSplit1, vSize[1], pContext,1, 0, 1);
	make_a_view(m_pwndSplit1, vSize[2], pContext,2, 0, 2);

	three_pane_linkup ();
}

TMdiLayout3_3Wide::~TMdiLayout3_3Wide() {}
int TMdiLayout3_3Wide::Save(TRegLayoutMdi* pLayMdi)
{
	pLayMdi->m_fSizeInfo = TRUE;
	transfer_pane(1, 0,0, pLayMdi->m_rct1);
	transfer_pane(1, 0,1, pLayMdi->m_rct2);
	transfer_pane(1, 0,2, pLayMdi->m_rct3);
	return 0;
}

