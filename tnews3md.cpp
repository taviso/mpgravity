/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tnews3md.cpp,v $
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
/*  Revision 1.5  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/10/03 08:21:06  richard_wood
/*  Tidying up code and comments.
/*
/*  Revision 1.2  2008/09/19 14:52:11  richard_wood
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

// tnews3md.cpp : implementation file
//
// I think the model is that m_pLayout is a XINU-like lower layer.
//   We can call it's functions, but it can't call our functions

#include "stdafx.h"
#include "News.h"

#include "tnews3md.h"
#include "globals.h"
#include "mlayout.h"
#include "custmsg.h"
#include "newsdoc.h"
#include "newsview.h"
#include "tglobopt.h"
#include "readavc.h"

#include "thrdlvw.h"
#include "artview.h"
#include "hints.h"
#include "rglaymdi.h"
#include "rgswit.h"
#include "rgui.h"
#include "log.h"
#include "fetchart.h"
#include "usrdisp.h"
#include "genutil.h"             // MsgResource()
#include "navigdef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;
static BYTE gbyChangingLayout;

/////////////////////////////////////////////////////////////////////////////
// TNews3MDIChildWnd

IMPLEMENT_DYNCREATE(TNews3MDIChildWnd, CMDIChildWnd)

TNews3MDIChildWnd::TNews3MDIChildWnd()
{
	m_pReadAdvancer = new TReadAdvancer(this);
	m_iMRU_Titleview = 0;
	m_iFocusWnd = 0;
	m_pLayout = 0;
	m_byCheckZoom = 0;
}

TNews3MDIChildWnd::~TNews3MDIChildWnd()
{
	delete m_pReadAdvancer;
}

BEGIN_MESSAGE_MAP(TNews3MDIChildWnd, CMDIChildWnd)
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_MESSAGE(WMU_CHILD_FOCUS, OnChildFocus)
	ON_MESSAGE(WMU_CHILD_TAB, OnChildTab)
	ON_MESSAGE(WMU_TREEVIEW_NEWFONT,    OnNewTreeFont)
	ON_MESSAGE(WMU_NEWSVIEW_NEWFONT,    OnNewNGFont)
	ON_MESSAGE(WMU_NEWSMDI_NEWLAYOUT,   OnNewLayout)
	ON_MESSAGE(WMU_TREEVIEW_REFILL,     OnTreeviewRefill)
	ON_MESSAGE(WMU_NEWSVIEW_GOTOARTICLE, GotoArticle)
	ON_MESSAGE(WMU_NEWSVIEW_PROCESS_MAILTO, ProcessMailTo)
	ON_MESSAGE(WMU_ARTVIEW_NEWTEMPLATE, OnNewViewTemplate)
	ON_MESSAGE(WMU_NONBLOCKING_CURSOR, OnNonBlockingCursor)
	ON_MESSAGE(WMU_TREEVIEW_RESETHDR, OnResetTreeHeader)
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_CLOSE()
	ON_COMMAND(ID_ARTICLE_MORE, OnArticleMore)
	ON_MESSAGE(WMU_USER_ACTION, OnUserAction)
	ON_COMMAND(ID_ZOOM_PANE, OnZoomKey)
	ON_COMMAND(ID_ZOOM_ESCAPE, OnZoomEscape)
	ON_MESSAGE(WMU_MBUTTON_DOWN, OnMButtonDown)
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_VIEW_NEWSGROUPPANE, OnViewNewsgroupPane)
	ON_COMMAND(ID_VIEW_ARTICLEPANE, OnViewArticlePane)
	ON_COMMAND(ID_VIEW_THREADPANE, OnViewThreadPane)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TNews3MDIChildWnd message handlers

/////////////////////////////////////////////////////////////////////////////
//  4-23-96 amc On first run, save out a clean window layout.
//
BOOL TNews3MDIChildWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	DWORD style = ::GetWindowLong (m_hWnd, GWL_STYLE);

	style &= ~(WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

	::SetWindowLong (m_hWnd, GWL_STYLE, style);

	// ---------------

	BOOL ret;
	extern BYTE gfFirstUse;

	gbyChangingLayout ++;
	ret = build_new_layout (this, pContext);
	gbyChangingLayout --;

	// in case we crash, save a valid window arrangement
	if (gfFirstUse)
		save_layout();

	// we were zoomed?  this is true only for a short while during creation
	m_byCheckZoom = 1;

	return ret;
}

//-------------------------------------------------------------------------
//
BOOL
TNews3MDIChildWnd::build_new_layout(
									CWnd*             parent,
									CCreateContext*   pContext
									)
{
	BOOL fOK = TRUE;
	LogString ("mdi window start new layout");
	m_iFocusWnd = 0;
	m_iMRU_Titleview = 3;

	switch (gpGlobalOptions->GetRegLayoutMdi()->m_layout)
	{
	case TUIMemory::LAY3_2TOP:
		m_pLayout = new TMdiLayout3_2Top(this, pContext);
		m_iMRU_Titleview = 1;
		break;
	case TUIMemory::LAY3_2BOT:
		m_pLayout = new TMdiLayout3_2Bottom(this, pContext);
		m_iMRU_Titleview = 1;
		break;
	case TUIMemory::LAY3_2RIGHT:
		m_pLayout = new TMdiLayout3_2Right(this, pContext);
		m_iMRU_Titleview = 1;
		break;
	case TUIMemory::LAY3_2LEFT:
		m_pLayout = new TMdiLayout3_2Left(this, pContext);
		m_iMRU_Titleview = 1;
		break;
	case TUIMemory::LAY3_3HIGH:
		m_pLayout = new TMdiLayout3_3High(this, pContext);
		m_iMRU_Titleview = 1;
		break;
	case TUIMemory::LAY3_3WIDE:
		m_pLayout = new TMdiLayout3_3Wide(this, pContext);
		m_iMRU_Titleview = 1;
		break;

	default:
		ASSERT(0); break;
	}
	LogString ("mdi window end new layout");

	ASSERT(fOK);
	return fOK;
}

////////////////////////////////////////////////////////////////////
// Called when the layout is
// Normally the CView::~CView will call
//   CDocument::RemoveView
//     OnChangedViewList  - if (ViewList is Empty && m_fAutoDelete)
//       CloseDocument
//
// We don't want the document to die! temporarily disable AutoDelete
//
// WPARAM is a pointer, which we delete
LRESULT TNews3MDIChildWnd::OnNewLayout(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pLayout);

	// unzoom things
	if (m_pLayout->IsZoomed())
		OnZoomKey ();

	TRegLayoutMdi sOrig;
	m_pLayout->Save ( &sOrig );

	// destroy the old layout. (don't save window sizes)
	// delete the splitter windows
	m_pLayout->m_fFreemem = TRUE;

	// hold this document (while we flush the views)
	CNewsDoc* pNewsDoc = m_pLayout->m_pInitNewsView->GetDocument();

	pNewsDoc->AutoDelete(FALSE);
	gbyChangingLayout ++;

	SetActiveView(NULL, FALSE);
	delete m_pLayout;    // destroy the views
	m_pLayout = 0;
	pNewsDoc->AutoDelete(TRUE);

	CCreateContext createCtx;
	ZeroMemory(&createCtx, sizeof(createCtx));
	createCtx.m_pCurrentDoc = pNewsDoc;

	gpGlobalOptions->GetRegLayoutMdi()->m_fSizeInfo = FALSE;

	// create the new layout
	build_new_layout(this, &createCtx);
	gbyChangingLayout --;

	// though I am an MDI child window, I am also a frame
	//  steal a hint from WINFRM.CPP line 721.  This call is not in the
	//  online help.

	if (m_pLayout->m_pInitNewsView)
	{
		// skip reload from database
		m_pLayout->m_pInitNewsView->UseFreshData(FALSE);
		InitialUpdateFrame (pNewsDoc, TRUE);
		m_pLayout->m_pInitNewsView->UseFreshData(TRUE);
	}

	// from CFrameWnd
	RecalcLayout ();

	m_pLayout->RecalcPaneSizes ( &sOrig );

	// (we may crash) write out the fact that we have nothing but
	//   default sizing data
	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	pRegLay->DestroyPaneSizes();
	pRegLay->Save ();

	return 0;
}

void TNews3MDIChildWnd::OnSetFocus(CWnd* pOldWnd)
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	FocusOnOnePane();
}

BOOL TNews3MDIChildWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// our only concern is not a specific window position,
	//  but rather do we maximize the mdi-child
	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	if (FALSE == pRegLay->m_fSizeInfo)
	{
		// no info, so maximized by default
		cs.style |= (WS_MAXIMIZE | WS_VISIBLE);
	}
	else
	{
		if (pRegLay->m_place.showCmd & SW_SHOWMAXIMIZED)
			cs.style |= (WS_MAXIMIZE | WS_VISIBLE);
	}
	return CMDIChildWnd::PreCreateWindow(cs);
}

LRESULT TNews3MDIChildWnd::OnChildFocus(WPARAM wP, LPARAM lParam)
{
	ASSERT(lParam >= 0);
	//   TRACE1(" rec ChildFocus %d\n", lParam);

	// 0 == newsview        2==artformview
	// 1 == thrdlvw
	if (lParam == 1)
		m_iMRU_Titleview = lParam;

	m_iFocusWnd = lParam;
	return 0;
}

LRESULT TNews3MDIChildWnd::OnChildTab (WPARAM wParam, LPARAM lParam)
{
	int i = int(lParam);

	// wParam == shift key down means back-tab
	int nextWindow = m_pLayout->NextPane(i, wParam ? FALSE : TRUE);

	if (m_pLayout->IsZoomed())
	{
		m_pLayout->UserActionArrange (TGlobalDef::kActionTabAround, this,
			&m_iFocusWnd, nextWindow);
	}
	else
	{
		m_iFocusWnd = nextWindow;
		FocusOnOnePane();
	}
	return 0;
}

void TNews3MDIChildWnd::FocusOnOnePane()
{
	if (gbyChangingLayout > 0)
		return;

	if (0 == m_pLayout)
		return;

	// layout object handles it all
	m_pLayout->SetActiveView ( this, m_iFocusWnd );
}

void TNews3MDIChildWnd::OnSize(UINT nType, int cx, int cy)
{
	if (m_pLayout)
		m_pLayout->OnSize(nType, cx, cy);

	// this is useful if you want to go spelunking
	//  into MFC.
	CMDIChildWnd::OnSize(nType, cx, cy);
}

afx_msg LRESULT TNews3MDIChildWnd::OnNewTreeFont (WPARAM wParam, LPARAM lParam)
{
	m_pLayout->ApplyNewTreeFont();
	return 0;
}

afx_msg LRESULT TNews3MDIChildWnd::OnNewNGFont (WPARAM wParam, LPARAM lParam)
{
	m_pLayout->ApplyNewNGFont();
	return 0;
}

afx_msg LRESULT TNews3MDIChildWnd::OnTreeviewRefill (WPARAM wParam, LPARAM lParam)
{
	CDocument* pDoc = GetActiveDocument();
	if (pDoc)
	{
		// resort threads.  Subject or Date. then redisplay
		pDoc->UpdateAllViews(NULL, VIEWHINT_REFILLTREE,  0);
	}
	return 0;
}

BOOL TNews3MDIChildWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// pump through current view FIRST
	CView* pView = GetActiveView();

	// CView::OnCmdMsg is protected
	//if (pView != NULL && pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	// return TRUE;

	// try other views
	if ((m_pLayout != NULL) &&
		m_pLayout->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo, pView, m_iMRU_Titleview ))
		return TRUE;

	return CMDIChildWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// --------------------------------------------------------------------------
LRESULT TNews3MDIChildWnd::GotoArticle (WPARAM wParam, LPARAM lParam)
{
	ASSERT (m_pLayout->m_pInitNewsView);
	return
		m_pLayout->m_pInitNewsView->SendMessage (WMU_NEWSVIEW_GOTOARTICLE, wParam, lParam);
}

LRESULT TNews3MDIChildWnd::ProcessMailTo (WPARAM wParam, LPARAM lParam)

{
	ASSERT (m_pLayout->m_pInitNewsView);
	m_pLayout->m_pInitNewsView->SendMessage (WMU_NEWSVIEW_PROCESS_MAILTO, wParam, lParam);
	return 0;
}

afx_msg LRESULT TNews3MDIChildWnd::OnNewViewTemplate (WPARAM wParam, LPARAM lParam)
{
	m_pLayout->ApplyViewTemplate();
	return 0;
}

void TNews3MDIChildWnd::RedrawTitles(int groupID, int artInt, BOOL fStatusOnly)
{
	if (m_pLayout)
	{
		LONG gid = m_pLayout->m_pInitNewsView->GetCurNewsGroupID();
		if (groupID != gid)
			return;

		TThreadListView* pTree = m_pLayout->GetThreadView();
		if (pTree)
			pTree->RedrawItem_Artint(artInt);
	}
}

void TNews3MDIChildWnd::OnInitMenu(CMenu* pMenu)
{
	CMDIChildWnd::OnInitMenu(pMenu);
}

void TNews3MDIChildWnd::OnClose()
{
	// we only have 1 mdi window, ya can't close it. do nothing
	///CMDIChildWnd::OnClose();
}

void TNews3MDIChildWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CMDIChildWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	if (bSysMenu)
	{
		// hunt for and disable the CLOSE item.
		int tot = pPopupMenu->GetMenuItemCount();
		for (--tot; tot > 0; --tot)
		{
			if (SC_CLOSE == pPopupMenu->GetMenuItemID (tot))
			{
				pPopupMenu->EnableMenuItem (tot, MF_DISABLED | MF_GRAYED | MF_BYPOSITION);
				break;
			}
		}
	}
}

LRESULT
TNews3MDIChildWnd::Send_NewsView(UINT message, WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pLayout->m_pInitNewsView);
	return m_pLayout->m_pInitNewsView->SendMessage(message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
void TNews3MDIChildWnd::OnDestroy()
{
	save_layout ();

	// delete the splitter windows
	delete m_pLayout;
	m_pLayout = 0;

	CMDIChildWnd::OnDestroy();
}

///////////////////////////////////////////////////////////////////////////
// This function is called during WM_DESTROY & also the very first time we run
//
void TNews3MDIChildWnd::save_layout(void)
{
	// save size of the mdi-child
	TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
	GetWindowPlacement ( &pRegLay->m_place );

	// ask layout to save the size of the panes, into the ptr
	m_pLayout->Save(pRegLay);

	int iZoomCode;
	if (!m_pLayout->IsZoomed())
		iZoomCode  = 0;
	else
	{
		iZoomCode = m_iFocusWnd + 1;
		if (iZoomCode < 1 || iZoomCode > 3)
			iZoomCode = 2;

		// Arbitrary: I think coming back to a blank article pane is stupid
		if (iZoomCode == 3)
			iZoomCode = 2;
	}
	pRegLay->m_iZoomCode = iZoomCode;

	// write out
	pRegLay->Save();
}

///////////////////////////////////////////////////////////////////////////
// This handles the SpaceBar  action  (VK_SPACE is a true accelerator)
//
void TNews3MDIChildWnd::OnArticleMore()
{
	TArticleFormView* pArtFormView = m_pLayout->GetArtFormView();

	bool fBrowseText =  (m_pLayout->m_pInitNewsView->GetBrowseText() != NULL);
	bool fOpenSelected = false;

	if (!fBrowseText)
	{
		if (m_pLayout->GetThreadView()->OpenSelectedArticle() == 0)
			fOpenSelected = true;
	}

	if (false == fOpenSelected)
	{
		if (pArtFormView->CanRequestPageDown () && 0 == 
			pArtFormView->RequestPageDown () ) 
		{
			// empty
		}
		else
		{
			int iThrdOff, iThrdOn, iSortOff, iSortOn; 
			gpGlobalOptions->GetRegUI()->GetNavig1KeyRead (iThrdOff, iThrdOn, iSortOff, iSortOn); 

			fnNavigate_Fire_Event (iThrdOff, iThrdOn, iSortOff, iSortOn);
		}
	}

}

// ------------------------------------------------------------------------
// CanArticleMore -- utility function used by pCmdUIUpdate in ArtView
//                   and TitleView
//

BOOL TNews3MDIChildWnd::CanArticleMore()
{
	return TRUE;

#pragma message ("pCmdUI for SpaceBar ; needs work")

	// return m_pReadAdvancer->CanAdvance( m_pLayout->GetArtFormView());
}

// ------------------------------------------------------------------------
// called by the ReadAdvancer
BOOL TNews3MDIChildWnd::CanAdvanceNextTitle()
{
	if (1 == m_iMRU_Titleview)
		return m_pLayout->GetThreadView()->IsNextArticle();
	return FALSE;
}

// ------------------------------------------------------------------------
// AdvanceNextMessage --  called by ReadAdvancer
//                        Returns 0 for success.
//
int TNews3MDIChildWnd::AdvanceNextMessage ()
{
	int ret = 1;

	// change this from 'next' to 'next-unread-local'

	if (m_pLayout->GetThreadView())
		ret = m_pLayout->GetThreadView()->Nav_ViewNextUnreadLocal();
	return ret;
}

void TNews3MDIChildWnd::ProcessPumpArticle ( TFetchArticle * pFetchArticle )
{
	TUIMemory::EMdiLayout layout;
	layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;

	BOOL fMatch;

	if (pFetchArticle->GetSuccess())
	{
		fMatch = m_pLayout->GetThreadView()->ArticleMatchesContext ( pFetchArticle );

		if (!fMatch)
		{
			delete pFetchArticle->GetArtText();
			pFetchArticle->DestroySelf();
			TRACE0("UI Changed!  destroying fetched article.\n");

			gpUserDisplay->RemoveActiveCursor();
			return ;
		}
	}
	else
	{
		// if there is an error, pass it down, and let Pane report error
	}

	m_pLayout->GetThreadView()->ProcessPumpArticle ( pFetchArticle );
}

afx_msg LRESULT TNews3MDIChildWnd::OnNonBlockingCursor(WPARAM wParam, LPARAM lParam)
{
	TUIMemory::EMdiLayout layout;
	layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;
	m_pLayout->GetThreadView()->NonBlockingCursor(wParam);
	m_pLayout->GetArtFormView()->NonBlockingCursor(wParam);
	return 0;
}

afx_msg LRESULT TNews3MDIChildWnd::OnResetTreeHeader(WPARAM wParam, LPARAM lParam)
{
	if (m_pLayout->GetThreadView())
		m_pLayout->GetThreadView()->ResetHeader();
	return 0;
}

afx_msg LRESULT TNews3MDIChildWnd::OnUserAction(WPARAM wParam, LPARAM lParam)
{
	ASSERT(0 == lParam);

	TGlobalDef::EUserAction eAction = static_cast<TGlobalDef::EUserAction>(wParam);
	if (m_pLayout)
		m_pLayout->UserActionArrange ( eAction, this, &m_iFocusWnd, 0 );
	return 1;
}

//-------------------------------------------------------------------------
// toggle the zoom state
afx_msg void TNews3MDIChildWnd::OnZoomKey()
{
#ifdef LITE
	if (!m_pLayout -> IsZoomed ())
	{
		MsgResource (IDS_LITE_NO_ZOOM);
		return;
	}
#endif

	if (m_pLayout)
	{
		// zoom or unzoom this pane
		m_pLayout->Zoom(m_iFocusWnd, m_pLayout->IsZoomed() ? FALSE : TRUE);
	}
}

//-------------------------------------------------------------------------
afx_msg void TNews3MDIChildWnd::OnZoomEscape()
{
	if (0 == m_pLayout)
		return;

	if  (m_pLayout->IsZoomed())
		m_pLayout->ZoomEscape ( this, &m_iFocusWnd );
	else
	{
		// 10-27-98  we are not zoomed, but set focus (like popping stack)

		switch (m_iFocusWnd)
		{
		case 0:
			return;

		case 1:
			m_iFocusWnd = 0;
			SetActiveView (m_pLayout->m_pInitNewsView);
			return;

		case 2:
			m_iFocusWnd = 1;
			SetActiveView (m_pLayout->GetThreadView());
			return;
		}
	}
}

// ------------------------------------------------------------------------
// This message is passed on when any of the 3 panes gets a WM_MBUTTONDOWN
//   message
//
LRESULT TNews3MDIChildWnd::OnMButtonDown (WPARAM,LPARAM)
{
	if (gpGlobalOptions->GetRegSwitch()->GetMiddleButtonSKR())
		OnArticleMore ();
	return 0;
}

// ------------------------------------------------------------------------
BOOL TNews3MDIChildWnd::handleMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	if (0 == m_pLayout)
		return FALSE;

	if  (m_pLayout->IsZoomed())
		return FALSE;  // let original pane deal with it.

	// we must find out what item was clicked
	// Find out where the cursor was for this message
	DWORD dw=GetMessagePos();

	POINT  tpt;
	tpt.x = (short) LOWORD(dw);
	tpt.y = (short) HIWORD(dw);

	CRect  rctNGPane, rctThread, rctArtPane;

	m_pLayout-> m_pInitNewsView  -> GetWindowRect (&rctNGPane);

	// OK!
	if (rctNGPane.PtInRect (tpt))
		return m_pLayout->m_pInitNewsView->handleMouseWheel (nFlags, zDelta, pt);

	m_pLayout-> GetThreadView () -> GetWindowRect (&rctThread);
	if (rctThread.PtInRect (tpt))
		return m_pLayout->GetThreadView()->handleMouseWheel (nFlags, zDelta, pt);

	m_pLayout-> GetArtFormView ()-> GetWindowRect (&rctArtPane);
	if (rctArtPane.PtInRect (tpt))
		return m_pLayout->GetArtFormView()->handleMouseWheel (nFlags, zDelta, pt);

	return FALSE;
}

// ------------------------------------------------------------------------------
// After the views have gone through OnInitialUpdate, restore the zoom state
//
void TNews3MDIChildWnd::ActivateFrame(int nCmdShow)
{
	CMDIChildWnd::ActivateFrame(nCmdShow);

	if (m_byCheckZoom)  // flag set via OnCreateClient
	{  
		m_byCheckZoom = 0;

		TRegLayoutMdi* pRegLay = gpGlobalOptions->GetRegLayoutMdi();
		if (pRegLay->m_iZoomCode > 0)
		{
			m_iFocusWnd =  pRegLay->m_iZoomCode - 1;
			FocusOnOnePane ();

			//m_pLayout->SetActiveView (this, pRegLay->m_iZoomCode - 1);
			this->OnZoomKey ();
		}
	}
}

// ------------------------------------------------------------------------------
// SetAbsolutePane - Jump to a specific pane
// ------------------------------------------------------------------------------
void TNews3MDIChildWnd::SetAbsolutePane (int iPane)

{
	if (m_pLayout->IsZoomed())
	{
		if (iPane == m_iFocusWnd)
		{
			// do nothing
		}
		else
			m_pLayout->ZoomTo (this, &m_iFocusWnd, iPane);
	}
	else
	{
		m_iFocusWnd = iPane;
		m_pLayout->SetActiveView ( this, iPane );
	}
}

void TNews3MDIChildWnd::OnViewNewsgroupPane() 
{
	SetAbsolutePane (0);
}

void TNews3MDIChildWnd::OnViewArticlePane() 
{
	SetAbsolutePane (2);
}

void TNews3MDIChildWnd::OnViewThreadPane() 
{
	SetAbsolutePane (1);
}
