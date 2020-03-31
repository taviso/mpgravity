/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thrdlvw.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.9  2009/02/17 19:31:03  richard_wood
/*  Tidied up classes
/*
/*  Revision 1.8  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.7  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/10/03 08:21:06  richard_wood
/*  Tidying up code and comments.
/*
/*  Revision 1.4  2008/09/19 14:52:05  richard_wood
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

// thrdlvw.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "thrdlvw.h"
#include "thredlst.h"
#include "newsgrp.h"
#include "article.h"

#include "globals.h"
#include "tmutex.h"

#include "hints.h"
#include "tasker.h"
#include "hourglas.h"
#include "custmsg.h"
#include "artclean.h"

#include "artview.h"

#include "coding.h"     // Encode/Decode
#include "dirpick.h"

#include "tvwtmpl.h"

#include "tpoptmpl.h"
#include "fileutil.h"
#include "tpopfrm.h"

#include "tglobopt.h"
#include "statchg.h"
#include "warndlg.h"
#include "rgwarn.h"
#include "rglaymdi.h"
#include "names.h"
#include "artnode.h"
#include "fetchart.h"
#include "utilsize.h"
#include "genutil.h"             // MsgResource(), EnsureVisibleSelection()
#include "rgui.h"
#include "ngutil.h"
#include "autodrw.h"             // TAutoDraw
#include "nglist.h"              // TNewsGroupUseLock
#include "server.h"              // TNewsServer
#include "vfilter.h"             // TViewFilter
#include "decoding.h"            // PriorityDecodeCallback()
#include "navigdef.h"            // fnNavigate_Fire_Event()

extern TGlobalOptions * gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const int THREADLISTVIEW_LBXID = 100;

static int fnChangeThreadStatus(TArticleHeader* pHdr, TNewsGroup *pNG,
								 DWORD cookie);
static int fnChangePileStatus(TArticleHeader* pHdr, TNewsGroup *pNG,
							   DWORD cookie);
static int fnSaveArticleCallback(TArticleHeader* pHdr, int fSelect,
								 TNewsGroup* pNG, DWORD cookie);
static int fnCountNonLocalCallback(TArticleHeader* pHdr, int fSelect,
								   TNewsGroup* pNG,DWORD cookie);

typedef struct tagCHGSTAT
{
	TStatusUnit::EStatus eStatus;
	BOOL                 fOn;
	int                  iApplyCount;
	TThreadListView*     pSelf;
} CHGSTAT, *LPCHGSTAT;

#define HIERLBX_ID  789
#define HIERLBX_HDR 788
#define LISTCTRL_ID 790

/////////////////////////////////////////////////////////////////////////////
// TThreadListView

IMPLEMENT_DYNCREATE(TThreadListView, TTitleView)

TThreadListView::TThreadListView()
{
	m_fCollapsed = FALSE;
	m_fTrackZoomed = false;
	m_fTestCurrentNavItem = false;
}

TThreadListView::~TThreadListView()
{
}

BEGIN_MESSAGE_MAP(TThreadListView, TTitleView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_ARTICLE_SKIPNEXTUNREAD,         OnArticleSkipNextUnread)
	ON_COMMAND(ID_ARTICLE_SKIPNEXTUNREADINTHREAD, OnArticleSkipNextUnreadInThread)
	ON_COMMAND(ID_ARTICLE_SKIPNEXTUNREADLOCAL,    OnArticleSkipNextUnreadLocal)
	ON_COMMAND(ID_ARTICLE_VIEWNEXTUNREAD,         OnArticleViewNextUnread)
	ON_COMMAND(ID_ARTICLE_VIEWNEXTUNREADINTHREAD, OnArticleViewNextUnreadInThread)
	ON_COMMAND(ID_ARTICLE_VIEWNEXTUNREADLOCAL,    OnArticleViewNextUnreadLocal)
	ON_COMMAND(ID_ARTICLE_VIEWNEXTLOCAL, OnArticleViewNextlocal)
	ON_COMMAND(ID_ARTICLE_NEXT, OnArticleNext)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_NEXT, OnUpdateArticleNext)
	ON_COMMAND(ID_ARTICLE_PREVIOUS, OnArticlePrevious)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_PREVIOUS, OnUpdateArticlePrevious)
	ON_COMMAND(ID_ARTICLE_MARKREAD, OnArticleMarkread)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_MARKREAD, EnableIfSelectionExists)
	ON_COMMAND(ID_THREAD_NEXT, OnThreadNext)
	ON_COMMAND(ID_THREAD_PREVIOUS, OnThreadPrevious)
	ON_COMMAND(ID_THREAD_KILLTHREAD, OnThreadKillthread)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_THREAD_NEXT, OnUpdateThreadNext)
	ON_UPDATE_COMMAND_UI(ID_THREAD_PREVIOUS, OnUpdateThreadPrevious)
	ON_UPDATE_COMMAND_UI (ID_THREAD_KILLTHREAD, OnUpdateKillThread)
	ON_COMMAND(ID_FORWARD_SELECTED, OnForwardSelected)
	ON_UPDATE_COMMAND_UI(ID_THREADLIST_SEARCH, OnUpdateThreadlistSearch)
	ON_COMMAND(ID_CMD_TABAROUND, OnCmdTabaround)
	ON_COMMAND(ID_CMD_TABBACK, OnCmdTabBack)
	ON_MESSAGE(WMU_THRDLVW_RESIZE, OnFinalResize)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_THREAD_EXPANDCOLLAPSE, OnThreadExpandCollapse)
	ON_UPDATE_COMMAND_UI(ID_THREAD_EXPANDCOLLAPSE, OnUpdateThreadExpandCollapse)
	ON_COMMAND(ID_NEWSGROUP_SORT, OnNewsgroupSort)
	ON_UPDATE_COMMAND_UI(ID_NEWSGROUP_SORT, OnUpdateNewsgroupSort)
	ON_COMMAND(ID_PRIORITY_DECODE, OnPriorityDecode)
	ON_COMMAND(IDC_DECODE, OnDecode)
	ON_UPDATE_COMMAND_UI(IDC_DECODE, SelectionOK)
	ON_COMMAND(ID_DECODE_TO, OnDecodeTo)
	ON_UPDATE_COMMAND_UI(ID_DECODE_TO, SelectionOK)
	ON_COMMAND(ID_NAVIGATE_SKIPDOWN, OnNavigateSkipdown)
	ON_COMMAND(ID_NAVIGATE_SKIPDOWN_NOEXP, OnNavigateSkipdownNoExpand)
	ON_COMMAND(ID_EXPAND_THREAD, OnExpandThread)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_THREAD, OnUpdateExpandThread)
	ON_COMMAND(ID_COLLAPSE_THREAD, OnCollapseThread)
	ON_UPDATE_COMMAND_UI(ID_COLLAPSE_THREAD, OnUpdateCollapseThread)
	ON_UPDATE_COMMAND_UI(ID_PRIORITY_DECODE, EnableIfSelectionExists)
	ON_UPDATE_COMMAND_UI(IDC_DECODE, EnableIfSelectionExists)
	ON_UPDATE_COMMAND_UI(ID_DECODE_TO, EnableIfSelectionExists)
	ON_COMMAND(ID_DISPLAY_RAW_ARTICLE, OnDisplayRawArticle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TThreadListView drawing
void TThreadListView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// TThreadListView diagnostics
#ifdef _DEBUG
void TThreadListView::AssertValid() const
{
	TTitleView::AssertValid();
}

void TThreadListView::Dump(CDumpContext& dc) const
{
	TTitleView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TThreadListView message handlers

void TThreadListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// setup widths now that the size has stopped changing
	if (m_header.EmptySize())
	{
		RECT rctClient;

		GetClientRect (&rctClient);
		m_header.LoadSettings ( rctClient.right );

		ASSERT(m_hlbx.getColumnCount() > 0);
	}

	switch (lHint)
	{
	case VIEWHINT_SHOWGROUP_NOSEL:
		// fill the pane, don't do any selection manipulation
		FillPane();
		break;

	case VIEWHINT_SHOWGROUP:
		{
			bool fResetSelection = false;
			CWnd * pFocus = GetFocus();
			if (pFocus && pFocus->m_hWnd == m_hlbx.m_hWnd)
				fResetSelection = true;

			FillPane();

			// if the lbx had the focus (before we emptied it), reset the selection.
			int tot = m_hlbx.GetCount();
			if (fResetSelection)
			{
				if (tot > 0)
				{
					int iCaret = m_hlbx.GetCaretIndex ();
					if (-1 == iCaret)
						iCaret = 0;
					m_hlbx.SetOneSelection ( iCaret );
				}
			}
			else
			{
				if (tot > 0)
					m_hlbx.SetOneSelection (0);
			}
		}
		break;

	case VIEWHINT_SAVESEL:
		filter_save_selection (pHint);
		break;

	case VIEWHINT_REFILLTREE:
		RefillTree (NULL == pHint ? true : false);
		break;

	case VIEWHINT_STATUS_CHANGE:
		{
			try
			{
				TStatusChg * pStatusChg = (TStatusChg *) pHint;
				RepaintArticle ( pStatusChg->m_msgNumber );
			}
			catch(...)
			{
				// catch everything
			}
		}
		break;

	case VIEWHINT_EMPTY:
		EmptyView();
		break;

	case VIEWHINT_GOTOARTICLE:
		{
			TGotoArticle * pGoto = (TGotoArticle *) pHint;

			SelectAndShowArticle (pGoto->m_articleNumber, pGoto->m_byLoad,
				(BOOL) pGoto -> m_byRemember);

			// generally you need this to actually transfer focus from the
			//  modeless srchdialog to the thread pane + main app window.
			SetLbxFocus ();
		}
		break;

	case VIEWHINT_ZOOM:
		handle_zoom(TRUE, pHint);
		break;

	case VIEWHINT_UNZOOM:
		handle_zoom(FALSE, pHint);
		break;

	case VIEWHINT_SERVER_SWITCH:
		m_hlbx.ServerSwitch ();
		break;
	}
}

int TThreadListView::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (TTitleView::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL  fCreated;
	CRect rct(0,0, 200, 200);

	DWORD style = LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_SHOWSELALWAYS |
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS ;

	m_hlbx.Create (style,
		rct,
		this,
		HIERLBX_ID);

	CHeaderCtrl * pHC = m_hlbx.GetHeaderCtrl();
	if (pHC)
	{
		fCreated = m_header.Attach ( pHC->m_hWnd );
	}
	else
	{
		CWnd * pChild = m_hlbx.GetWindow (GW_CHILD);

		ASSERT(pChild);

		if (NULL == pChild)
			return -1;

		fCreated = m_header.Attach ( pChild->m_hWnd );
	}

	if (!fCreated)
		return -1;

	m_header.m_pParentThreadView = this;

	m_hlbx.m_pHeader = &m_header;
	m_hlbx.m_parentThreadView = this;

	return 0;
}

void TThreadListView::OnSize(UINT nType, int cx, int cy)
{
	TTitleView::OnSize(nType, cx, cy);

	resize (cx, cy);
}

//-------------------------------------------------------------------------
//
void TThreadListView::FillPane(bool fSort /* =true */,
							   bool fResetMetrics /* =false*/)
{
	if (m_header.IsThreaded())
	{
		if (fSort)
			update_ascend_from_filter ();

		FillTree (fSort, fResetMetrics);
	}
	else
	{
		TTreeHeaderCtrl::EColumn eCol = m_header.GetSortColumn();
		UseFlatList(eCol, m_header.IsSortAscend());
	}
}

///////////////////////////////////////////////////////////////////////////
//  4-10-96  View should not invalidate, just the LBX. (hdrctrl needn't paint)
//
void TThreadListView::FillTree (bool fSort /* =true */,
								bool fResetMetrics /*f*/)
{
	TServerCountedPtr cpNewsServer;

	m_fCollapsed = gpGlobalOptions->GetRegUI()->GetShowThreadsCollapsed();
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup * pNG = 0;
	{
		TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
		if (!fUseLock)
			return;

		TLockDraw  sAuto(&m_hlbx, FALSE);

		m_hlbx.DeleteAll();

		TThreadList * pThreadList = pNG->GetpThreadList();

		if (fSort)
			pThreadList->UpdateSort (m_header.IsSortAscend(), fResetMetrics);

		pThreadList->FillTree ( &m_hlbx, m_fCollapsed );

		pNG->UpdateFilterCount (true);
	}

	// Since we are changing newsgroups, we should clear the
	//   article in both the ArticleView Pane and the Full window
	GetArtView()->Clear();
}

// --------------------------------------------------------------------------
// Move the selection down -- but open any branches we come across
//
int TThreadListView::NextItem(bool fExpandBranches /*=true*/)
{
	int sel = m_hlbx.GetFirstSelection();
	if (LB_ERR == sel)
		return LB_ERR;

	// Open any branches as we traverse down  8-26-96 amc
	//  but this is only if we are NOT in a flat sort. In a flat sort,
	//  everything is here anyway.
	if (fExpandBranches)
	{
		TArtNode * pNode = m_hlbx.GetpNode ( sel );
		if (pNode && m_header.IsThreaded())
		{
			TArticleHeader * pHdr = pNode->GetpRandomHeader();
			if (pHdr)
			{
				int uniq = pHdr->GetArticleNumber();
				if (!m_hlbx.IsOpened(uniq))
					m_hlbx.OpenBranch (sel);
			}
		}
	}
	if (sel == m_hlbx.GetCount())
		return LB_ERR;

	return sel + 1;
}

int TThreadListView::PrevItem()
{
	int sel = m_hlbx.GetFirstSelection();
	if (LB_ERR == sel)
		return LB_ERR;

	if (sel == 0)
		return LB_ERR;

	return sel - 1;
}

void TThreadListView::NextArticle(BOOL fNext)
{
	int nextIdx;

	if (fNext)
	{
		nextIdx = this->NextItem();

		if (nextIdx >= m_hlbx.GetCount())
		{
			int iOutGID, iOutArtNum;

			int stat = QueryToEngine (kQueryNext, false, iOutGID, iOutArtNum);
			if (1 == stat)   // valid article in next group
			{
				JumpSecondPass (GetNewsView(),
					kQueryNext,
					iOutGID,
					true,
					false);
			}
		}

		else if (nextIdx != LB_ERR && nextIdx >= 0)
			NextArticleIndex ( nextIdx );
	}
	else
	{
		nextIdx = this->PrevItem();

		if (nextIdx != LB_ERR &&
			nextIdx >= 0 &&
			nextIdx < m_hlbx.GetCount())
		{
			NextArticleIndex ( nextIdx );
		}
	}
}

void TThreadListView::NextArticleIndex(int idx, BOOL bRemember /* = TRUE */)
{
	if (idx < 0 || idx >= m_hlbx.GetCount())
	{
		ASSERT(0);
		return ;
	}

	TPersist822Header* pHdr;
	TArtNode* pNode;

	pNode = m_hlbx.GetpNode ( idx );
	pHdr = pNode->GetpRandomHeader();
	if ( pHdr )
	{
		// last arg is a magic cookie sent back to RedrawItem()
		// LoadFullArticle (pNewsGrp, pHdr, DWORD(pNode), bRemember);
		DisplayArticle (pHdr, pNode, bRemember);
		m_hlbx.SetOneSelection ( idx );
	}
}

///////////////////////////////////////////////////////////////////////////
// DisplayArticle --
int  TThreadListView::DisplayArticle(
									 TPersist822Header* p822Hdr,
									 LPVOID             pVoid,
									 BOOL               bRemember,
									 BOOL               bCheckMaxLines
									 )
{
	TServerCountedPtr cpNewsServer;
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup* pNewsGrp = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNewsGrp);

	if (!fUseLock)
		return -1;

	if ( p822Hdr )
	{
		int iMaxLines = gpGlobalOptions->GetRegUI()->GetMaxLines();
		if (bCheckMaxLines && (p822Hdr->GetLines() > iMaxLines))
		{
			switch (gpGlobalOptions->GetRegUI()->GetMaxLinesCmd())
			{
			case MAXLINES_OPEN:
				return LoadFullArticle (pNewsGrp, p822Hdr, DWORD(pVoid), bRemember );
				break;
			case MAXLINES_DECODE:
				PostMessage (WM_COMMAND, IDC_DECODE);
				break;
			case MAXLINES_VIEW:
				PostMessage (WM_COMMAND, IDC_VIEW_BINARY);
				break;
			case MAXLINES_IGNORE:
				MessageBeep(MB_OK);
				break;
			}
			return 0;
		}
		else
			return LoadFullArticle (pNewsGrp, p822Hdr, DWORD(pVoid), bRemember );
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////
// 'N' is a real accelerator
void TThreadListView::OnArticleNext()
{
	// TODO: Add your command handler code here
	NextArticle(TRUE);
	CenterListboxSelection(&m_hlbx, TRUE);
}

void TThreadListView::OnUpdateArticleNext(CCmdUI* pCmdUI)
{
	// this can jump to next group now
	pCmdUI->Enable ( /* IsNextArticle() */ TRUE );
}

BOOL TThreadListView::IsNextArticle(void)
{
	int sel = m_hlbx.GetFirstSelection();

	if (LB_ERR == sel)
		return FALSE;

	int tot = m_hlbx.GetCount();

	if (sel < tot - 1)
		return TRUE;
	else if (sel == tot - 1)
	{
		// this is the Last listbox item; But it might be a collapsed item.
		TArtNode * pNode = m_hlbx.GetpNode ( sel );
		if (pNode)
		{
			TArticleHeader * pHdr = pNode->GetpRandomHeader();
			if (pHdr)
			{
				int uniq = pHdr->GetArticleNumber();
				if (!m_hlbx.IsOpened(uniq))
					return TRUE;
			}
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
// 'P' is a real accelerator
void TThreadListView::OnArticlePrevious()
{
	NextArticle(FALSE);
	CenterListboxSelection(&m_hlbx, FALSE /* fMovingUp */);
}

void TThreadListView::OnUpdateArticlePrevious(CCmdUI* pCmdUI)
{
	BOOL fOn = TRUE;
	// if we are the root we can't go back anymore
	int idx = m_hlbx.GetFirstSelection();
	if (LB_ERR == idx || 0 == idx)
		fOn = FALSE;

	pCmdUI->Enable( fOn );
}

// override a virtual function
//  dwRedrawCookie should be an artNode;
void TThreadListView::MarkItemRead_and_redraw ( DWORD dwRedrawCookie )
{
	TArtNode * pNode = (TArtNode*) dwRedrawCookie;

	STLMapAHdr::iterator it = pNode->GetArtsItBegin();

	for (; it != pNode->GetArtsItEnd(); it++)
	{
		TArticleHeader * pHdr = (*it);

		if (pHdr)
		{
			// wotta useful func!
			Util_ArticleMarkread ( (TArtNode*) dwRedrawCookie, pHdr );
		}
	}
}

// override a virtual function
void TThreadListView::RedrawItem ( DWORD dwRedrawCookie )
{
	m_hlbx.RepaintItem ( (LPVOID) dwRedrawCookie );
}

/////////////////////////////////////////////////////////////////////////////
// ApplyToArticles -- calls a function once for each article in this view.
// If any of the function invocations returns nonzero, return ourselves
// with that error code immediately. Otherwise, return 0
int  TThreadListView::ApplyToArticles (
									   int  (*pfn) (TArticleHeader *, int, TNewsGroup *, DWORD),
									   BOOL fRepaint,            /* FALSE */
									   DWORD dwMagicCookie,      // pass this back into the pfn
									   BOOL fBackwards
									   )
{
	TServerCountedPtr cpNewsServer;
	LONG gid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	// we are operating on Selected items
	int tot = m_hlbx.GetCount();

	for (int i = 0; i < tot; ++i)
	{
		int n = i;
		if (fBackwards)
			n = tot - 1 - i;
		TArticleHeader* pHdr = (TArticleHeader*) m_hlbx.Getp_Header ( n );
		if (pHdr)
		{
			int stat = pfn (pHdr,
				m_hlbx.GetSel(n),     // fSelected
				pNG, dwMagicCookie );
			if (0 != stat)
				return stat;
			if (fRepaint)
				m_hlbx.RepaintIndex( n );
		}
	}

	return 0;
}

// ------------------------------------------------------------------------
// ApplyToSelectedItems - it's more efficient, a refinement of the above
//                        created 9-01-96
int  TThreadListView::ApplyToSelectedItems (
	int  (*pfn) (TArticleHeader *, TNewsGroup *, DWORD dwCookie),
	BOOL fRepaint /* = FALSE */,
	DWORD dwMagicCookie /* = 0 */,
	BOOL fBackwards /* = FALSE */)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	// we are operating on Selected items
	int tot = m_hlbx.GetCount();

	for (int i = 0; i < tot; ++i)
	{
		int n = i;
		if (fBackwards)
			n = tot - 1 - i;

		if (m_hlbx.GetSel(n))
		{
			TArticleHeader* pHdr = static_cast<TArticleHeader*>(m_hlbx.Getp_Header (n));

			if (pHdr)
			{
				// utilitize callback ptr
				int stat = pfn (pHdr, pNG, dwMagicCookie);

				if (0 != stat)
					return stat;
				if (fRepaint)
					m_hlbx.RepaintIndex( n );
			}
		}
	}

	return 0;
}

// ------------------------------------------------------------------------
// ApplyToSelectedNodes - pass nodes to callback function
//                        created 7-26-01
//
//  the magic cookie should be a ptr to  DecodeArticleCallbackData
//
int  TThreadListView::ApplyToSelectedNodes (
	int  (*pfn) (TArtNode *, TArticleHeader *, TNewsGroup *, DWORD dwCookie),
	BOOL fRepaint       /* = FALSE */,
	DWORD dwMagicCookie /* = 0 */)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	// we are operating on Selected items

	int iStartFrom = -1;
	int idx;

	while  ((idx = m_hlbx.GetNextItem (iStartFrom, LVNI_SELECTED)) != -1)
	{
		TArtNode * pNode =  m_hlbx.GetpNode (idx);

		if (pNode)
		{
			// utilitize callback ptr
			int stat = pfn (pNode, pNode->GetpRandomHeader (), pNG, dwMagicCookie);

			if (0 != stat)
				return stat;

			if (fRepaint)
				m_hlbx.RepaintIndex( idx );
		}

		iStartFrom = idx;
	}

	return 0;
}

void TThreadListView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: Add your specialized code here and/or call the base class
	if (bActivate)
	{
		SetLbxFocus ();
		// let the mdichild window know that we (index 1) have the focus
		((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->PostMessage ( WMU_CHILD_FOCUS, 0, 1L );

		// default processing would put focus on the ThreadListView,
		//   so just exit.
		return;
	}

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

///////////////////////////////////////////////////////////////////////////
void TThreadListView::EnableIfSelectionExists(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (LB_ERR != m_hlbx.GetFirstSelection());
}

void TThreadListView::OnArticleMarkread()
{
	int idx = m_hlbx.GetFirstSelection();
	if (LB_ERR == idx)
		return;

	Util_ArticleMarkIndexRead ( idx );
}

void TThreadListView::Util_ArticleMarkread( TArtNode* pArtNode )
{
	if (pArtNode)
	{
		STLMapAHdr::iterator it = pArtNode->GetArtsItBegin();

		for (; it != pArtNode->GetArtsItEnd(); it++)
		{
			TArticleHeader * pHdr = (*it);

			if ( pHdr )
				Util_ArticleMarkread ( pArtNode, pHdr );
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//
void TThreadListView::Util_ArticleMarkread(
	TArtNode*   pNode,
	TPersist822Header* pHdr)
{
	Util_ArticleNGread ( pHdr->CastToArticleHeader() );
	// mark this article as read
	m_hlbx.RepaintItem ( pNode );
}

///////////////////////////////////////////////////////////////////////////
//
//
void TThreadListView::Util_ArticleMarkIndexRead(int index)
{
	TPersist822Header * pHdr = m_hlbx.Getp_Header(index);
	if (0 == pHdr)
		return;
	TServerCountedPtr cpNewsServer;

	CNewsView*  pNewsView = GetNewsView();
	LONG gid = pNewsView->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup* pNewsGrp  = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNewsGrp);
	if (fUseLock)
	{
		int artInt = pHdr->GetNumber();
		// ReadRangeAdd changes the status bit
		pNewsGrp->ReadRangeAdd ( pHdr->CastToArticleHeader() );
		pNewsGrp->SetDirty();
	}
	// mark this article as read
	m_hlbx.RepaintIndex ( index );
}

/////////////////////////////////////////////////////////////////////
//  move to the first article in the next thread
//    Try this completely via the UI
void TThreadListView::OnThreadNext()
{
	TUIMemory::EMdiLayout layout;
	layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;
	if (3 == gpUIMemory->LayoutPaneCount(layout))
		Util_ThreadNext ( TRUE );
	else
	{
		// four pane layouts are flatview oriented, and don't use NEXT-THREAD
	}
}

/////////////////////////////////////////////////////////////////////
// move to the first article in the prev thread
void TThreadListView::OnThreadPrevious()
{
	TUIMemory::EMdiLayout layout;
	layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;
	if (3 == gpUIMemory->LayoutPaneCount(layout))
		Util_ThreadNext ( FALSE );
	else
	{
		// four pane layouts are flatview oriented, and don't use PREV-THREAD
	}
}

// -------------------------------------------------------------------------
void TThreadListView::Util_ThreadNext (BOOL fForward)
{
	int idx = m_hlbx.GetFirstSelection();
	if (LB_ERR == idx || idx < 0)
		return;

	int rootIdx;
	ASSERT(idx >= 0);
	if (!m_hlbx.GetThreadRoot(idx, rootIdx))
		return;

	if (!m_hlbx.GetNextThread(rootIdx, idx, fForward))
		return;

	NextArticleIndex ( idx );
}

// -------------------------------------------------------------------------
BOOL TThreadListView::IsAThreadSelected ()
{
	return (LB_ERR == m_hlbx.GetFirstSelection()) ? FALSE : TRUE ;
}

// -------------------------------------------------------------------------
void TThreadListView::OnUpdateKillThread (CCmdUI* pCmdUI)
{
	pCmdUI -> Enable (IsAThreadSelected ());
}

// -------------------------------------------------------------------------
void TThreadListView::OnThreadKillthread()
{
	int idx = m_hlbx.GetFirstSelection();

	if (LB_ERR == idx)
		return;

	// see if they really want to kill this thread...
	if (!confirmKillThread())
		return;

	SetThreadStatus (TStatusUnit::kNew, FALSE);

	int iNumSelected = this -> GetSelCount();
	if (iNumSelected > 0)
	{
		int iTopIndex = 0;  this -> GetSelItems (1, &iTopIndex);
		m_hlbx.SetOneSelection ( iTopIndex );
	}

	// fancy navigation
	int   iThrdOff, iThrdOn, iSortOff, iSortOn;

	gpGlobalOptions->GetRegUI()->GetNavigKillThrd (iThrdOff, iThrdOn, iSortOff, iSortOn);

	// before we do the refresh, figure out where we should land
	bool fViewArticle = false;
	EJmpQuery eQuery = fnNavigate_GetQuery (iThrdOff, iThrdOn, iSortOff, iSortOn, &fViewArticle);

	int iOutGID = 0;
	int iOutArtNum = 0;

	int nQueryRet = this -> QueryToEngine ( eQuery,
		false,
		iOutGID,
		iOutArtNum );

	FilterOutReadThreads ();

	// go to the landing area
	switch (nQueryRet)
	{
	case -1:
		break;

	case 0:
		SelectAndShowArticle ( iOutArtNum, fViewArticle );
		break;

	case 1:
		JumpSecondPass ( GetNewsView (), eQuery, iOutGID, fViewArticle, false /* limitToPile */ );
		break;
	}
}

void TThreadListView::OnDestroy()
{
	TTitleView::OnDestroy();

	// remember widths of the tree header control
	m_header.SaveSettings( m_fTrackZoomed );

	m_header.Detach ();
}

///////////////////////////////////////////////////////////////////////////
// used by the tasker to flush out Headers with Expired Bodies
//
void
TThreadListView::ArticleStatusCallback(const CString & group,
									   const CString & articleNumber,
									   TStatusUnit::EStatus kOldStatus,
									   TStatusUnit::EStatus kNewStatus)
{
	TRACE3("%s number %s - status is now %s",
		LPCTSTR(group), LPCTSTR(articleNumber),
		kNewStatus == TStatusUnit::kNew ? "new" : "other");
}

// -------------------------------------------------------------------------
// an override
BOOL TThreadListView::GetHdr_for_SelectedItem(TPersist822Header* & rpHdr)
{
	rpHdr = 0;

	int idx = m_hlbx.GetFirstSelection();
	if (LB_ERR == idx)
		return FALSE;

	TPersist822Header* p822Header = m_hlbx.Getp_Header ( idx );
	if (NULL == p822Header)
		return FALSE;

	rpHdr = p822Header;
	return TRUE;
}

// -------------------------------------------------------------------------
// GetFirstSelectedHeader -- returns 0 for success, non-0 for failure
int TThreadListView::GetFirstSelectedHeader (TPersist822Header *&pHeader)
{
	if (!GetHdr_for_SelectedItem (pHeader))
		return 1;
	return 0;
}

// -------------------------------------------------------------------------
// Modified 12-14-95
// Modified 10-29-96  -  expand correct branch, then goto it.
void TThreadListView::SelectAndShowArticle (
	int    iArticleNumber,
	BOOL   fLoad,
	BOOL   fRemember /* = TRUE */)
{
	const int & key = iArticleNumber;
	int index = -1;
	BOOL fValidIndex = FALSE;
	bool fParentResult;

	if ( IsThreaded () )
	{
		// The pane may show many expanded branches, but an individual
		//   branch may be collapsed

		if (m_hlbx.FindCollapsed (key, &index, &fParentResult))
		{
			if (fParentResult)
			{
				TPersist822Header* p822Hdr = m_hlbx.Getp_Header (index);
				if (p822Hdr)
				{
					if (m_hlbx.IsOpened (p822Hdr->GetNumber()))
					{
						// this cleans up the branch, next we open it completely
						m_hlbx.CloseBranch (index);
					}

					// open branch that contains the art we want
					m_hlbx.OpenBranch (index);
				}
			}

			int idxAncestor = index;
			if (m_hlbx.FindFrom (idxAncestor, key, &index))
				fValidIndex = TRUE;
			// index is setup
		}
	}
	else
	{
		// not collapsed, do a straight search
		if (m_hlbx.Find (iArticleNumber, &index))
		{
			ASSERT(index >= 0);
			// index is setup
			fValidIndex = TRUE;
		}
	}

	if (fValidIndex)
	{
		// user has choice to load the article or just Select it.
		if (fLoad)
			NextArticleIndex ( index, fRemember);
		else
		{
			m_hlbx.SetOneSelection ( index );
		}

		if (IsPaneZoomed())
		{
			// less destructive
			AfxGetMainWnd()->BringWindowToTop ();
		}
		else
		{
			// fix for build 736 10/6/97.
			//  1) We do want a hard-core SetFocus if we want to change
			//      from the Search modeless-dlg to the main app window.
			//      This is done by the calling function.
			//
			//  2) We do not want to set focus if the user is
			//      zoomed into the Article Pane, for instance.
			//
			//  3) 10-25-98  Generally, we don't want to change focus
			//      around the panes. Let focus remain in the ThreadPane, or
			//      ArtPane!  (Accelerator for Next Unread Body, etc)
			//

			/*  SetLbxFocus (); 10/25/98 -amc*/
		}

		CenterListboxSelection (&m_hlbx, TRUE);

		TPersist822Header* p822Hdr = m_hlbx.Getp_Header (index);

		if (p822Hdr)
			OnHLbxSelChange ( (TArticleHeader *) p822Hdr);
	}

	return;
}

// -------------------------------------------------------------------------
BOOL TThreadListView::IsArticleInThreadPane (LONG lArticle)
{
	const int &key = lArticle;
	int index = -1;
	BOOL fValidIndex = FALSE;
	bool fParentResult;

	if (m_fCollapsed)
	{
		if (m_hlbx.FindCollapsed (key, &index, &fParentResult))
		{
			if (fParentResult)
			{
				TPersist822Header* p822Hdr = m_hlbx.Getp_Header (index);
				if (p822Hdr)
				{
					if (m_hlbx.IsOpened (p822Hdr->GetNumber()))
					{
						// this cleans up the branch, next we open it completely
						m_hlbx.CloseBranch (index);
					}

					// open branch that contains the art we want
					m_hlbx.OpenBranch (index);
				}
			}

			int idxAncestor = index;
			if (m_hlbx.FindFrom (idxAncestor, key, &index))
				fValidIndex = TRUE;
			// index is setup
		}
	}
	else
	{
		// not collapsed, do a straight search
		if (m_hlbx.Find (lArticle, &index))
		{
			ASSERT(index >= 0);
			// index is setup
			fValidIndex = TRUE;
		}
	}

	return fValidIndex;
}

void TThreadListView::EmptyView()
{
	if (m_hlbx.GetCount() > 0)
	{
		TAutoDraw autoDraw(&m_hlbx, TRUE);
		m_hlbx.DeleteAll();
	}
}

void TThreadListView::RedrawItem_Artint (int artInt)
{
	int idx;
	if (m_hlbx.Find (artInt, &idx))
		m_hlbx.RepaintIndex ( idx );
}

// -------------------------------------------------------------------------
//  Follow the Ascend / descend setting from the tree header ctrl
void TThreadListView::UpdateSortFromTreeheader (void)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup * pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);

	// we may arrange threads ascending (oldest at top) or
	//    descending (newest at top)
	if (fUseLock)
		pNG->GetpThreadList()->UpdateSort (m_header.IsSortAscend());
}

// -------------------------------------------------------------------------
// UpdateThreadMoves -- used for OnUpdateThreadNext() and OnUpdateThreadPrevious()
void TThreadListView::UpdateThreadMoves (CCmdUI *pCmdUI, BOOL fForward)
{
	TUIMemory::EMdiLayout layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;

	int idx = m_hlbx.GetFirstSelection();

	// for 4-pane layout... always turn next-thread/prev-thread off
	if (4 == gpUIMemory->LayoutPaneCount(layout) || LB_ERR == idx || (idx < 0))
	{
		pCmdUI -> Enable (FALSE);
		return;
	}

	int rootIdx;
	BOOL fEnable = FALSE;
	// must not be in Flat mode
	if ( IsThreaded() &&
		m_hlbx.GetThreadRoot(idx, rootIdx) &&
		m_hlbx.GetNextThread(rootIdx, idx, fForward) )
		fEnable = TRUE;

	pCmdUI -> Enable ( fEnable );
}

// -------------------------------------------------------------------------
void TThreadListView::OnUpdateThreadNext(CCmdUI* pCmdUI)
{
	UpdateThreadMoves (pCmdUI, TRUE);
}

// -------------------------------------------------------------------------
void TThreadListView::OnUpdateThreadPrevious(CCmdUI* pCmdUI)
{
	UpdateThreadMoves (pCmdUI, FALSE);
}

void TThreadListView::OnInitialUpdate()
{
	CRect rctClient;

	GetClientRect (&rctClient);

	// setup widths now that the size has stopped changing
	if (m_header.EmptySize())
	{
		m_header.LoadSettings ( rctClient.Width() );
	}

	PostMessage ( WMU_THRDLVW_RESIZE );

	TTitleView::OnInitialUpdate();
}

void TThreadListView::Followup(TArtNode* pNode)
{
	if (pNode)
	{
		// pass in the article,  pass in the redraw Cookie
		if (0 == DisplayArticle (pNode->GetpRandomHeader(), pNode))
			AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ARTICLE_FOLLOWUP);
	}
}

void TThreadListView::ReplyByMail(TArtNode* pNode)
{
	if (pNode)
	{
		if (0 == DisplayArticle (pNode->GetpRandomHeader(), pNode))
			AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ARTICLE_REPLYBYMAIL);
	}
}

//-------------------------------------------------------------------------
//
void TThreadListView::ChangeStatusUtil(TStatusUnit::EStatus eStatus,
									   BOOL                 fOn)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return;

	MAP_NODES stlMapNodes;

	// this function deals with collapsed branches
	int iFirstIdx = engine_of_selection (&stlMapNodes, FALSE /* fClose */);

	if (LB_ERR == iFirstIdx)
		return;

	MAP_NODES::iterator itN = stlMapNodes.begin();

	for (; itN != stlMapNodes.end(); itN++)
	{
		TArtNode * pNode = (*itN).second;

		STLMapAHdr::iterator itH = pNode->GetArtsItBegin();

		for (; itH != pNode->GetArtsItEnd(); itH++)
		{
			TArticleHeader * pHdr = *itH;

			pNG->StatusBitSet ( pHdr->GetNumber(), eStatus, fOn );

			if (eStatus == TStatusUnit::kNew && !fOn)
				pNG->ReadRangeAdd (pHdr);
			else if (eStatus == TStatusUnit::kNew && fOn)
				pNG->ReadRangeSubtract (pHdr, KThreadList::kHandleCrossPost);
		}
	}

	pNG->SetDirty();

	// If status is changing New->Read,  or Read->New
	if (TStatusUnit::kNew == eStatus)
	{
		// update the count in the newsview
		GetNewsView()->PostMessage (WMC_DISPLAY_ARTCOUNT, gid);
	}
}

///////////////////////////////////////////////////////////////////////////
// Processes the DEL key (an accelerator)
void TThreadListView::DeleteArticlesNow()
{
	TServerCountedPtr cpNewsServer;
	int selCnt;
	LONG gid = GetNewsView()->GetCurNewsGroupID();
	TNewsGroup* pNG = 0;
	BOOL fUseLock;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return;

	if ((selCnt= m_hlbx.GetSelCount()) <= 0)
		return;

	MAP_NODES  stlMapNodes;

	// gather
	int idxFirstSel = engine_of_selection ( &stlMapNodes, FALSE /* fClose */ );

	// Step #2 : note - Tagged articles are exempt from DEL key
	//                  Permanent arts  are exempt from DEL key
	TPersistentTags & rTags = cpNewsServer->GetPersistentTags ();

	QUE_NODES   stlQueNodes;

	MAP_NODES::iterator itN = stlMapNodes.begin();

	for (; itN != stlMapNodes.end(); itN++)
	{
		TArtNode * pNode = (*itN).second;

		bool fWholeNodeEraseable = true;

		STLMapAHdr::iterator itH = pNode->GetArtsItBegin();

		for (; itH != pNode->GetArtsItEnd(); itH++)
		{
			TArticleHeader * pSelected = *itH;

			int iArtInt = pSelected->GetNumber();

			if (rTags.FindTag (gid, iArtInt))
			{
				// you escaped this time!
				fWholeNodeEraseable = false;
				break;
			}
			else if (pNG->IsStatusBitOn (iArtInt, TStatusUnit::kPermanent))
			{
				// you escaped this time!
				fWholeNodeEraseable = false;
				break;
			}
		}

		if (fWholeNodeEraseable)
			stlQueNodes.push_back ( pNode );
	}

	// Step #3 : destroy objects
	engine_of_destruction ( &stlQueNodes, pNG);

	// Step #2 was complex. Rather than manipulate the listbox with
	//   microsurgery, just reload everything. THIS Could be better
	// Suppose there is a collapsed branch.  Some nodes are kPermanent.
	//  You select it and delete it.  You can delete the selected lbx line,
	//  but then you have to add in the kPermanent nodes.
	EmptyView ();
	FillPane  (true  /*sort*/, true /*resetmetrics*/);

	int i;
	// set cur selection
	if ((i=m_hlbx.GetCount()) > 0)
	{
		m_hlbx.SetOneSelection((idxFirstSel >= i) ? i-1 : idxFirstSel);
		CenterListboxSelection (&m_hlbx, TRUE);
	}
	GetNewsView()->PostMessage (WMC_DISPLAY_ARTCOUNT, pNG->m_GroupID);
}

////////////////////////////////////////////////////////////////////////
// things to do:
//   - destroy article hdr from DB
//   - destroy article bod from DB
//   - remove StatusUnit from status Vector
//   - remove from TextRangeSet
//   - remove from HdrRangeSet
//   - remove items from the UI listbox
//   - reset selection
//   - update NewsView UI count
//   - mark items as closed in hierarchy listbox
// Question: what about collapsed branches?
//           what about flatview?

static void smartAdd(MAP_NODES * pstlNodeMap, TArtNode* pNode)
{
	int key  = pNode->GetArticleNumber ();

	if (pstlNodeMap->end() == pstlNodeMap->find (key))
		pstlNodeMap->insert (MAP_NODES::value_type (key, pNode) );
}

static void smartAddKids(MAP_NODES * pstlNodeMap, TArtNode* pNode)
{
	// add msg_id of self

	smartAdd ( pstlNodeMap, pNode );

	STLMapANode::iterator  it  = pNode->GetKidsItBegin ();
	STLMapANode::iterator  itE = pNode->GetKidsItEnd ();

	// add msg_id of kids

	for (; it != itE; it++)
	{
		TArtNode * pKid = *it;
		smartAddKids ( pstlNodeMap, pKid );
	}
}

//-------------------------------------------------------------------------
// extract the article headers
// this function deals with collapsed branches
int  TThreadListView::engine_of_selection(MAP_NODES * pstlNodeMap, BOOL fClose)
{
	int i, idx, artInt;
	int selCnt = m_hlbx.GetSelCount();

	if (selCnt <= 0)
		return LB_ERR;

	int * pIdx = new int[selCnt];
	auto_ptr<int> sIdxDeleter(pIdx);

	m_hlbx.GetSelItems (selCnt, pIdx);

	for (i = selCnt - 1; i >= 0; --i)
	{
		idx = pIdx[i];

		TArtNode * pNode = m_hlbx.GetpNode ( idx );
		if (pNode)
		{
			artInt = pNode->GetArticleNumber();
			if (FALSE == IsThreaded())
			{
				// flat mode - just add yourself
				smartAdd ( pstlNodeMap, pNode );
			}
			else if (m_hlbx.IsOpened( artInt ))
			{
				smartAdd ( pstlNodeMap, pNode );
			}
			else
			{
				// this is collapsed! Add self and all children.
				smartAddKids ( pstlNodeMap, pNode );
			}
		}
	}

	if (fClose)
	{
		TAutoDraw sAutoDraw(&m_hlbx, TRUE);

		// remove stuff from Listbox, these are the visible items!
		for (i = selCnt - 1; i >= 0; --i)
		{
			idx = pIdx[i];
			m_hlbx.CloseItem (idx);
			m_hlbx.DeleteString (idx);
		}
	}

	return pIdx[0];
}

///////////////////////////////////////////////////////////////////////////
//  Remember the artInts of the destroyed articles.
//
// 7-15-96 don't recursively kill children if we are in a flat-mode
void TThreadListView::engine_of_destruction(QUE_NODES * pstlQueNodes, TNewsGroup* pNG)
{
	int iHdrSetSize;

	CWaitCursor wait;

	TAutoDraw autoDraw(&m_hlbx, FALSE);      // Turn Redraw Off

	if ((iHdrSetSize = pstlQueNodes->size()) <= 0)
		return;

	// we may need to recalculate thread metrics
	pNG->GetpThreadList()->FlagMetricsClean ();

	// loop through all the items (even the hidden children)

	TArtHdrDestroyedInfo hdi(iHdrSetSize);
	QUE_NODES::iterator itN = pstlQueNodes->begin();

	QUE_HDRS  qHdrs;

	for (; itN != pstlQueNodes->end(); itN++)
	{
		TArtNode * pNode =  *itN;

		STLMapAHdr::iterator  itH  = pNode->GetArtsItBegin ();
		for (;  itH != pNode->GetArtsItEnd ();  itH++)
		{
			qHdrs.push_back ( *itH );
		}

		// unhook from ThreadList.  We will destroy the Header object.
		//  function destroys the artnode
		pNG->GetpThreadList()->RemoveArticle ( *itN, NULL, FALSE );

		// case 1 - from lbx - select one big node - delete 4 USENET arts + kill the ArtNode

		// case 2 - from rules - we are deleting 1 USENET art, and the ArtNode may be OK, or it
		//          may be empty

		// we need to gracefully unhook the article from the ArtNode, Thread ThreadPile
		//   and gracefully delete each if they are empty
		//
		//
	}

	// see if we are deleting the current browse article
	BOOL fBrowseKilled = FALSE;
	TPersist822Header * pBrowse822Hdr = GetNewsView()->GetBrowseHeader ();
	CString browseMsgID;
	if (pBrowse822Hdr)
		browseMsgID = pBrowse822Hdr->GetMessageID();

	// now remove from the database
	// load in headers to destroy some

	QUE_HDRS::iterator it;
	{
		// open the newsgroup
		TAutoClose sAutoClose( pNG );

		it = qHdrs.begin();
		for (; it != qHdrs.end(); it++)
		{
			TArticleHeader * pDead = *it;
			if (!browseMsgID.IsEmpty())
			{
				if (!fBrowseKilled)
					fBrowseKilled = (browseMsgID == pDead->GetMessageID());
			}

			TArticleCleaner::DestroyOne (pNG, pDead);
		}
	}

	autoDraw.SetRedrawOn();

	if (fBrowseKilled)
		EmptyBrowse_ClearArtPane ();

	it = qHdrs.begin();
	for (; it != qHdrs.end(); it++)
	{
		TArticleHeader * pDead = *it;

		// now you really are dead.
		delete pDead;
	}
}

//-------------------------------------------------------------------------
//
void TThreadListView::SetThreadStatus(TStatusUnit::EStatus eStatus, BOOL fOn)
{
	set_thread_statbit ( eStatus, fOn, TRUE /* fWholePile */);
}

//-------------------------------------------------------------------------
void TThreadListView::set_thread_statbit(TStatusUnit::EStatus eStatus,
										 BOOL fOn, BOOL fWholePile)
{
	TServerCountedPtr cpNewsServer;

	int iRet = 0;
	CHGSTAT chg;
	chg.eStatus     = eStatus;
	chg.fOn         = fOn;
	chg.iApplyCount = 0;
	chg.pSelf       = this;

	// look for CheckedBoxed articles
	iRet = ApplyToSelectedItems (fWholePile
		? fnChangePileStatus
		: fnChangeThreadStatus,
		TRUE,                  // fRepaint
		(DWORD) &chg);         //
	ASSERT(0 == iRet);

	// none found. Do the current selection
	if (0 == chg.iApplyCount)
	{
		ASSERT(0);  // should be something in PREMIA tree Ctrl
	}

	// save out the RangeSet in this newsgroup.  Gotta save them all
	cpNewsServer->SaveReadRange ();

	// update view if we are going New -> Read  or
	//   Read -> New
	if (TStatusUnit::kNew == eStatus)
	{
		CNewsView* pNV = GetNewsView();
		// update the count in the newsview
		pNV->PostMessage (WMC_DISPLAY_ARTCOUNT, pNV->GetCurNewsGroupID());
	}
}

//-------------------------------------------------------------------------
// fnChangeThreadStatus - c function used by ApplyToSelectedItems()
int fnChangeThreadStatus(TArticleHeader* pHdr,
						 TNewsGroup*     pNG,
						 DWORD           cookie)
{
	LPCHGSTAT pChg = (LPCHGSTAT) cookie;

	pNG->MarkThreadStatus (pHdr, pChg->eStatus, pChg->fOn);
	pChg->iApplyCount++;
	pNG->SetDirty();
	return 0;
}

//-------------------------------------------------------------------------
// fnChangePileStatus - c function used by ApplyToSelectedItems()
int fnChangePileStatus(TArticleHeader* pHdr,
					   TNewsGroup*     pNG,
					   DWORD           cookie)
{
	LPCHGSTAT pChg = (LPCHGSTAT) cookie;

	pNG->MarkPileStatus (pHdr, pChg->eStatus, pChg->fOn);
	pChg->iApplyCount++;
	pNG->SetDirty();
	return 0;
}

//////////////////////////////////////////////////////////////////////

typedef struct
{
	TNewsGroup* m_pNG;
	BOOL m_fFound;
} STAT_INSPECTOR;

void fnInspectStatus_TagNew(TArticleHeader* pArtHdr, void* pVoid)
{
	STAT_INSPECTOR* pSI = (STAT_INSPECTOR*) pVoid;
	try
	{
		if (pSI->m_pNG->IsStatusBitOn(pArtHdr->GetNumber(), TStatusUnit::kNew))
		{
			pSI->m_fFound = TRUE;
		}
	}
	catch (TException *pE)
	{
		pE->Delete();
		return;  // may have been destroyed
	}
}

void fnInspectStatus_TagOld(TArticleHeader* pArtHdr, void* pVoid)
{
	STAT_INSPECTOR* pSI = (STAT_INSPECTOR*) pVoid;
	try
	{
		if (!pSI->m_pNG->IsStatusBitOn(pArtHdr->GetNumber(), TStatusUnit::kNew))
		{
			pSI->m_fFound = TRUE;
		}
	}
	catch (TException *pE)
	{
		pE->Delete();
		return;  // may have been destroyed
	}
}

void fnInspectStatus_TagImportant(TArticleHeader* pArtHdr, void* pVoid)
{
	STAT_INSPECTOR* pSI = (STAT_INSPECTOR*) pVoid;
	try
	{
		if (pSI->m_pNG->IsStatusBitOn(pArtHdr->GetNumber(),
			TStatusUnit::kImportant))
		{
			pSI->m_fFound = TRUE;
		}
	}
	catch (TException *pE)
	{
		pE->Delete();
		return;  // may have been destroyed
	}
}

///////////////////////////////////////////////////////////////////
// remove a thread from the UI if all the articles in a thread are
// read
//
// sub-goal:  maintain selection
void TThreadListView::FilterOutReadThreads (void)
{
	filter_out_threads( kRead );

	m_fTestCurrentNavItem = true;   // special signal to FancyNavigation code
}

///////////////////////////////////////////////////////////////////
// remove a thread from the UI if all the articles in a thread are
// New
//
// sub-goal:  maintain selection
void TThreadListView::FilterOutNewThreads (void)
{
	filter_out_threads( kNew );
}

// -----------------------------------------------------------------
// remove a thread from the UI if all the articles in a thread are
//  normal importance
//
// sub-goal:  maintain selection
void TThreadListView::FilterOutNormalImpThreads (void)
{
	filter_out_threads( kNormalImp );
}

// -----------------------------------------------------------------
// remove a thread from the UI if all the articles in a thread are
// New
//
// sub-goal:  maintain selection
//
// Q: What is the relationship between the thread list and what
//     is displayed? (Assume the Filter is "Unread Articles" and you
//     are marking articles 'Read'
// A: In tree-mode, when a thread is completely 'Read' it can be
//     removed, since the thread as a whole doesn't match the filter.
// Q: What happens in flat-mode?
// A: hopefully the same thing?
//
void TThreadListView::filter_out_threads(ESeen eSeen)
{
	// if contents shown by filter are not affected by action, just exit
	if (!needs_filter_out (eSeen))
		return;

	TServerCountedPtr cpNewsServer;

	TRACE0("Start Erase old threads\n");

	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fLockUpdate;
	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);

	if (fUseLock)
	{
		// track how many lbx items above 'us' we have deleted.
		int idx = m_hlbx.GetFirstSelection();

		// do one or the other, but Try hi-tech version first
		fLockUpdate = m_hlbx.LockWindowUpdate();
		if (!fLockUpdate)
			m_hlbx.SetRedraw (FALSE);

		try
		{
			filter_out_threads_two (eSeen, pNG, &idx);
		}
		catch(...)
		{
			filter_out_threads_end (fLockUpdate);
			throw;
		}

		filter_out_threads_end (fLockUpdate);

		// set selection

		int iLbxMax = m_hlbx.GetCount() - 1;
		if (idx >= 0 && idx <= iLbxMax)
			m_hlbx.SetOneSelection (idx);
		else if (iLbxMax >= 0 && idx > iLbxMax)
			m_hlbx.SetOneSelection (iLbxMax);
	}
	TRACE0("end Erase old threads\n");
}

// ------------------------------------------------------------------
// helper function for filter_out_threads. Returns TRUE if filter and
//  action indicate the UI might need adjusting.
//
BOOL TThreadListView::needs_filter_out (ESeen eSeen)
{
	TViewFilter * pFilter = TNewsGroup::GetpCurViewFilter ();
	if (0 == pFilter)
	{
		NewsMessageBox (this, IDS_ERR_NOVFLT);
		return FALSE;
	}
	TStatusUnit::ETriad eTri = pFilter->GetNew();
	if (kRead == eSeen)              // if we mark it as Read ...
	{
		// if we view read or both
		if (TStatusUnit::kNo == eTri || TStatusUnit::kMaybe == eTri)
			return FALSE;
	}
	else if (kNew == eSeen)          // if we mark it as New ...
	{
		// if we view new or both
		if (TStatusUnit::kYes == eTri || TStatusUnit::kMaybe == eTri)
			return FALSE;
	}
	else if (kNormalImp == eSeen)    // we mark it as Normal
	{
		eTri = pFilter->GetImp();
		// we are viewing normal or both
		if (TStatusUnit::kNo == eTri || TStatusUnit::kMaybe == eTri)
			return FALSE;
	}

	return TRUE;
}

// ------------------------------------------------------------------
// Note: the ui may show threads, or be in a flat mode
void TThreadListView::filter_out_threads_two (ESeen eSeen, TNewsGroup* pNG,
											  int* piSelIdx)
{
	if (m_header.IsThreaded())
	{
		// Loop through listbox items

		int i = m_hlbx.GetCount();
		for (--i; i >= 0; --i)
		{
			TThread* pThread = 0;
			if (filter_out_can_remove_thrd (i, pNG, eSeen, pThread))
			{
				// This thread should be taken out of UI, so ask
				//  hierlbx to remove this thread
				m_hlbx.RemoveThread (m_header.IsThreaded(), pThread, i, piSelIdx);

				// remove thread from threadlist
				pNG->GetpThreadList()->RemoveThread (pThread, TRUE);
			}
		}
	}
	else
	{
		// since whatever thread goes 'Poof' may delete randomly positioned
		// lbx items, we loop through the thredlist directly.

		CPtrList sDedList;

		TThreadList * pTL = pNG->GetpThreadList();

		std::vector<TThreadPile*>::iterator it = pTL->m_rPiles.begin();
		for ( ; it != pTL->m_rPiles.end(); ++it)
		{
			TThreadPile * pPile = *it;

			for (int k = 0; k < pPile->GetSize(); k++)
			{
				TThread * pThread = (TThread*) pPile->GetElem(k);

				if (filter_out_test_remove_thrd (pNG, eSeen, pThread))
				{
					m_hlbx.RemoveThread (FALSE,      // fTreeView
						pThread,
						0,          // dummy
						piSelIdx);
					sDedList.AddTail ( pThread );
				}

			} // inner
		} // outer

		// now we are OK to destroy

		while (!sDedList.IsEmpty())
		{
			TThread * pThread = (TThread*) sDedList.RemoveHead ();
			pTL->RemoveThread ( pThread, TRUE );
		}
	}

	pNG->UpdateFilterCount ();
}

// ------------------------------------------------------------------
void TThreadListView::filter_out_threads_end (BOOL fUnlockUpdate)
{
	if (fUnlockUpdate)
		m_hlbx.UnlockWindowUpdate();
	else
	{
		m_hlbx.SetRedraw (TRUE);
		m_hlbx.Invalidate (NULL);  // repaint everything
	}
}

// ---------------------------------------------------------------------------
BOOL TThreadListView::filter_out_can_remove_thrd (int idx, TNewsGroup * pNG,
												  ESeen eSeen, TThread*& rpThread)
{
	BOOL fRemove = FALSE;
	try
	{
		TArtNode* pNode = m_hlbx.GetpNode(idx);

		// only analyze roots
		if (0 == pNode->m_depth)
		{
			TThread* pThread = 0;

			TArticleHeader* pArtHdr = pNode->GetpRandomHeader();
			if (NULL == pArtHdr)
				return FALSE;
			pNG->FindThread (pArtHdr, pThread);
			if (pThread)
			{
				if (filter_out_test_remove_thrd (pNG, eSeen, pThread))
				{
					rpThread = pThread;
					fRemove = TRUE;
				}
			}
		}
	}
	catch(...)
	{
	}
	return fRemove;
}

// ------------------------------------------------------------------------
//
BOOL TThreadListView::filter_out_test_remove_thrd (TNewsGroup* pNG,
												   ESeen eSeen, TThread* pThread)
{
	STAT_INSPECTOR  sSI;
	sSI.m_pNG = pNG;
	sSI.m_fFound = FALSE;

	pThread->OperateOnHeaders ( kRead == eSeen
		? fnInspectStatus_TagNew
		: (kNew == eSeen
		? fnInspectStatus_TagOld
		: fnInspectStatus_TagImportant),
		&sSI );
	// flag is still false, so thread is entirely Read
	return (!sSI.m_fFound);
}

/////////////////////////////////////////////////////////////////////
//
// override
int TThreadListView::vAnythingSelected()
{
	return m_hlbx.IsSelection();
}

/////////////////////////////////////////////////////////////////////
//
typedef struct tagTHRDSelAvl
{
	CNewsView * pNV;
	int         count;
} THRDSelAvl, * LPTHRDSelAvl;

BOOL TThreadListView::vSelectedArticlesAvailable(int& non_local_count)
{
	// have or is currently showing in view pane
	THRDSelAvl sSA;
	sSA.pNV = GetNewsView();
	sSA.count = 0;
	ApplyToArticles (fnCountNonLocalCallback, FALSE, (DWORD) &sSA);
	non_local_count = sSA.count;
	if (0 == sSA.count)
		return TRUE;  // yep, all articles are local.
	return FALSE;
}

static int  fnCountNonLocalCallback(TArticleHeader* pHdr,
									int fSelect,
									TNewsGroup* pNG,
									DWORD cookie)
{
	if (!fSelect)
		return 0;
	LPTHRDSelAvl pSA = (LPTHRDSelAvl) cookie;
	int artInt = pHdr->GetArticleNumber();
	if (FALSE == pNG->TextRangeHave(artInt))
	{
		TPersist822Text* pText = pSA->pNV->GetBrowseText();
		if (!pText)
			++pSA->count;
		else
		{
			if (artInt != pText->GetNumber())
				++pSA->count;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
//
// override

int  TThreadListView::vSaveSelectedArticles(const CString& fileName,
											CFile & file,
											BOOL fRetrieveNonLocal)
{
	CHourglass mgr = this;
	TITLSaveArt sSaveInfo;
	sSaveInfo.pFileName = &fileName;
	sSaveInfo.pFile     = &file;
	sSaveInfo.fRetrieve = fRetrieveNonLocal;
	sSaveInfo.pNV = GetNewsView();
	sSaveInfo.pSelf = this;
	sSaveInfo.missing = 0;
	sSaveInfo.fErrorShown = FALSE;
	int ret = ApplyToArticles (fnSaveArticleCallback,
		TRUE,     // uncheck when done
		(DWORD) &sSaveInfo);

	return ret;
}

static int  fnSaveArticleCallback(TArticleHeader* pHdr,
								  int fSelect,
								  TNewsGroup* pNG,
								  DWORD cookie)
{
	LPTITLSaveArt pSaveInfo = (LPTITLSaveArt) cookie;
	int ret = pSaveInfo->pSelf->SaveArticleDriver (pSaveInfo, pHdr, fSelect, pNG);
	return ret;
}

void TThreadListView::OnForwardSelected()
{
	GetNewsView()->SendMessage(WM_COMMAND, ID_NV_FORWARD_SELECTED);
}

BOOL TThreadListView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_hlbx.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	// do normal behavior
	return TTitleView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// ------------------------------------------------------------------------
void TThreadListView::OnUpdateThreadlistSearch(CCmdUI* pCmdUI)
{
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	pCmdUI->Enable (gid != 0);
}

// ------------------------------------------------------------------------
// Called from hierlbx.cpp
BOOL TThreadListView::GetArticleStatus(int artInt, WORD& wStatus, BOOL& fHaveBody)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup * pNG = 0;

	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return FALSE;

	return pNG->ArticleStatus ( artInt, wStatus, fHaveBody );
}

void TThreadListView::RepaintArticle (int artInt)
{
	m_hlbx.RepaintArticle ( artInt );
}

void TThreadListView::ApplyNewFont()
{
	m_hlbx.ApplyNewFont();
}

void TThreadListView::vCancelArticle ()
{
	int idx = m_hlbx.GetFirstSelection();
	TPersist822Header* p822Hdr = m_hlbx.Getp_Header (idx);
	if (0 == p822Hdr)
		return;

	doCancelArticle ( (TArticleHeader*) p822Hdr );
}

// this is a real accelerator
void TThreadListView::OnCmdTabaround()
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
		PostMessage (WMU_CHILD_TAB, FALSE, 1);
}

void TThreadListView::OnCmdTabBack()
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
		PostMessage (WMU_CHILD_TAB, TRUE, 1);
}

// ------------------------------------------------------------------------
BOOL TThreadListView::confirmKillThread()
{
	if (gpGlobalOptions->WarnOnMarkRead())
	{
		BOOL  fDisableWarning = FALSE;
		if (WarnWithCBX (IDS_WARNING_KILLTHREAD, &fDisableWarning))
		{
			if (fDisableWarning)
			{
				gpGlobalOptions->SetWarnOnMarkRead(FALSE);
				TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
				pRegWarning->Save();
			}
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return TRUE;
}

BOOL TThreadListView::ArticleMatchesContext ( TFetchArticle* pFetchArticle )
{
	// OK before we go any further, check some things out
	//   - is the newsgroup the same
	//   - is the article status compatible w/ the Filter Set

	LONG gid = GetNewsView()->GetCurNewsGroupID();

	LONG fetch_gid;
	int  fetch_artint;
	WORD fetch_hdrstatus;

	pFetchArticle->GetUIContext ( &fetch_gid, &fetch_artint, &fetch_hdrstatus );

	if (gid != fetch_gid)
		return FALSE;

	// do a reverse lookup in the listbox.  Checking the filter set
	// directly doesn't allow you to re-read old articles

	int dummy_index;
	return m_hlbx.Find (fetch_artint, &dummy_index);
}

///////////////////////////////////////////////////////////////////////////
// pText - pass in ArticleText (it has the CrossPost info)
//
BOOL TThreadListView::vMarkBrowseHeader (int artInt, TArticleText * pText)
{
	TPersist822Header* p822Hdr = 0;
	int index;

	if (!m_hlbx.Find (artInt, &index, &p822Hdr))
		return FALSE;

	TArticleHeader * pArtHdr = p822Hdr->CastToArticleHeader();

	pText->TransferXRef( pArtHdr );

	GetNewsView()->SetBrowseHeader ( p822Hdr );

	// mark item read in the newsgroup status...
	Util_ArticleNGread ( p822Hdr->CastToArticleHeader() );

	// and redraw
	m_hlbx.RepaintIndex ( index );
	return TRUE;
}

HWND TThreadListView::GetInternalLBX(void)
{
	return m_hlbx.m_hWnd;
}

///////////////////////////////////////////////////////////////////////////
//  reset the CHeaderCtrl based on a change in the Options dialog box
//
void TThreadListView::ResetHeader()
{
	CRect rct;
	GetClientRect(&rct);
	m_header.ResetSettings( rct.Width() );

	// redraw everything
	Invalidate();
}

// Posted from OnInitialUpdate
LRESULT TThreadListView::OnFinalResize(WPARAM wParam, LPARAM lParam)
{
	// lock the update, so we have 1 big Repaint at the end.
	BOOL fUpdateLock = LockWindowUpdate ();
	CRect rct;
	GetClientRect ( &rct );
	resize ( rct.Width(), rct.Height() );
	if (fUpdateLock)
		UnlockWindowUpdate ();
	return 0;
}

// ----------------------------------------------------------
void TThreadListView::resize(int cx, int cy)
{
	m_hlbx.MoveWindow (0, 0, cx, cy);
}

/////////////////////////////////////////////////////////////
// this is Edit Menu -> SelectAll
//  since this menuitem does something we must explicitly
//  disable Edit->Copy
void TThreadListView::OnEditSelectAll()
{
	m_hlbx.SetSel(-1, TRUE); // -1 means all items
}

void TThreadListView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (m_hlbx.GetCount() > 0);
}

// you can't copy stuff out of the ListBox
void TThreadListView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

// ---------------------------------------------------------------------------
void TThreadListView::OnThreadExpandCollapse()
{
	m_fCollapsed = !m_fCollapsed;
	m_hlbx.ExpandAll (!m_fCollapsed);
}

void TThreadListView::OnUpdateThreadExpandCollapse(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( update_expand_collapse() );
}

BOOL TThreadListView::update_expand_collapse()
{
	BOOL fEnable = FALSE;
	if (m_hlbx.GetCount() > 0 && IsThreaded())
		fEnable = TRUE;
	return fEnable;
}

//-------------------------------------------------------------------------
//
//
int TThreadListView::ResetContent(TTreeHeaderCtrl::EColumn eOldCol,
								  TTreeHeaderCtrl::EColumn eNewCol,
								  BOOL fOldSortAscending,
								  BOOL fNewSortAscending)
{
	TServerCountedPtr cpNewsServer;
	CWaitCursor wait;
	int ret;
	if (TTreeHeaderCtrl::kThread == eOldCol && TTreeHeaderCtrl::kThread == eNewCol)
	{
		// switch from/to flat
		ret = ChangeThreadDisplay(fOldSortAscending);   // fThread
	}
	else if (TTreeHeaderCtrl::kThread != eOldCol && TTreeHeaderCtrl::kThread == eNewCol)
	{
		ret = ChangeThreadDisplay(fOldSortAscending);
	}
	else if (eOldCol != eNewCol)
	{
		// column change (maybe sort change)
		ret = ChangeColumn (eNewCol, fNewSortAscending);
	}
	else if (fOldSortAscending != fNewSortAscending)
	{
		LONG gid = (GetNewsView())->GetCurNewsGroupID();
		BOOL fUseLock;
		TNewsGroup * pNG = 0;

		TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
		if (!fUseLock)
			return -10;

		// change sort order
		ret = ChangeSortOrder (*(pNG->GetpThreadList()), fNewSortAscending, TRUE);
	}
	return ret;
}

//-------------------------------------------------------------------------
int TThreadListView::ChangeThreadDisplay(BOOL fThread)
{
	int iSelCount = m_hlbx.GetSelCount();
	int idx = -1;
	if (1 == iSelCount)
		m_hlbx.GetSelItems(1, &idx);

	if (fThread)
	{
		// Going from Flat to Thread
		// get ancestor list, keep selection constant
		CPtrList list;
		TArtNode* pNode = 0;
		TArtNode* pDad = 0;
		if (idx >= 0)
		{
			pNode = m_hlbx.GetpNode(idx);
			while (pNode)
			{
				list.AddTail (pNode);   // build list of ancestors
				pNode = pNode->GetParent();
			}
		}

		FillTree();   // note - this clears the article pane.  Not optimal

		if (1 == iSelCount)
		{
			POSITION pos = list.GetHeadPosition();
			while (pos)
			{
				// one of the ancestors will get the selection
				pNode = (TArtNode*) list.GetNext(pos);
				if (m_hlbx.Find (pNode->GetpRandomHeader()->GetNumber(), &idx))
				{
					m_hlbx.SetOneSelection(idx);
					m_hlbx.SetTopIndex(idx);
					break;
				}
			}
		}
		return 0;
	}
	else
	{
		int artInt;
		int ret;
		if (idx >= 0)
			artInt = m_hlbx.GetpNode(idx)->GetpRandomHeader()->GetNumber();

		m_hlbx.NodeExpand( FALSE );  // close status for everything
		TTreeHeaderCtrl::EColumn eCol = m_header.GetSortColumn();

		ret = UseFlatList(eCol, fThread);

		if (idx >= 0)
		{
			if (m_hlbx.Find (artInt, &idx))
			{
				m_hlbx.SetOneSelection ( idx );
				m_hlbx.SetTopIndex ( idx );
			}
		}
		return ret;
	}
}

//-------------------------------------------------------------------------
int TThreadListView::UseFlatList(TTreeHeaderCtrl::EColumn eCol, BOOL fSortAscending)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fUseLock;
	int  ret;
	TNewsGroup * pNG = 0;

	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return -10;

	TThreadList& rList = *(pNG->GetpThreadList());

	// sort by correct column
	ret = SortFlatList(rList,  eCol);

	// jam into listbox
	ChangeSortOrder(rList, fSortAscending, FALSE);

	pNG->UpdateFilterCount (true);

	return 0;
}

//-------------------------------------------------------------------------
int TThreadListView::SortFlatList(TThreadList& rList,
								  TTreeHeaderCtrl::EColumn eCol)
{
	TServerCountedPtr cpNewsServer;
	switch (eCol)
	{
	case TTreeHeaderCtrl::kFrom:
		rList.FromSortFlatlist();
		break;

	case TTreeHeaderCtrl::kIndicator:
		{
			LONG gid = (GetNewsView())->GetCurNewsGroupID();
			BOOL fUseLock;
			TNewsGroup * pNG = 0;

			TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
			if (!fUseLock)
				return -10;

			rList.IndicatorSortFlatlist(pNG);
		}
		break;

	case TTreeHeaderCtrl::kSubject:
		rList.SubjSortFlatlist();
		break;

	case TTreeHeaderCtrl::kLines:
		rList.LineSortFlatlist();
		break;

	case TTreeHeaderCtrl::kDate:
		rList.DateSortFlatlist();
		break;

	case TTreeHeaderCtrl::kScore:
		rList.ScoreSortFlatlist();
		break;

	default:
		ASSERT(0);
		break;
	}
	return 0;
}

//-------------------------------------------------------------------------
int TThreadListView::ChangeSortOrder(TThreadList& rList,
									 BOOL fSortAscending, BOOL fKeepSel)
{
	int lbxTot = m_hlbx.GetCount();
	int iListTot = rList.FlatListLength();
	int idx = -1;
	int iSelCount = -1;
	int artInt = -1;

	if (fKeepSel)
	{
		iSelCount = m_hlbx.GetSelCount();
		if (1 == iSelCount)
		{
			m_hlbx.GetSelItems(1, &idx);
			artInt = m_hlbx.GetpNode(idx)->GetpRandomHeader()->GetNumber();
		}
	}
	ASSERT(iListTot >= lbxTot);

	// turn off redraw
	TAutoDraw autoDraw(&m_hlbx, TRUE);

	// if we are sorting descending, just put it into the listbox "backwards"

	VEC_NODES::iterator it  = rList.GetFlatBegin();
	VEC_NODES::iterator itEnd = rList.GetFlatEnd();

	VEC_NODES::reverse_iterator itR  = rList.GetFlatRBegin();
	VEC_NODES::reverse_iterator itREnd = rList.GetFlatREnd();

	int i = 0;
	if (fSortAscending)
	{
		for (; it != itEnd; it++, i++)
		{
			if (i < lbxTot)
				m_hlbx.SetItemDataPtr ( i, *it );   // keep lbx string.  Just reset the ITEMDATA ptr
			else
				m_hlbx.AddString( (LPCTSTR) *it );  // add more lbx items
		}
	}
	else
	{
		for (; itR != itREnd; itR++, i++)
		{
			if (i < lbxTot)
				m_hlbx.SetItemDataPtr ( i, *itR );   // keep lbx string.  Just reset the ITEMDATA ptr
			else
				m_hlbx.AddString( (LPCTSTR) *itR );  // add more lbx items
		}
	}

	autoDraw.SetRedrawOn ();

	if (fKeepSel && idx >= 0)
	{
		if (m_hlbx.Find(artInt, &idx))
		{
			m_hlbx.SetOneSelection(idx);
			m_hlbx.SetTopIndex(idx);
		}
	}

	return 0;
}

//-------------------------------------------------------------------------
int TThreadListView::ChangeColumn(TTreeHeaderCtrl::EColumn eNewCol,
								  BOOL fNewSortAscending)
{
	TServerCountedPtr cpNewsServer;
	LONG gid = (GetNewsView())->GetCurNewsGroupID();
	BOOL fUseLock;
	int  ret;
	TNewsGroup * pNG = 0;

	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return -10;

	TThreadList& rList = *(pNG->GetpThreadList());

	ret = SortFlatList ( rList, eNewCol );

	ChangeSortOrder ( rList, fNewSortAscending, TRUE );
	return 0;
}

BOOL TThreadListView::IsThreaded(void)
{
	return m_header.IsThreaded();
}

void TThreadListView::OnNewsgroupSort()
{
	m_header.OnNewsgroupSort(this);
}

void TThreadListView::OnUpdateNewsgroupSort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_hlbx.GetCount() > 0);
}

//-------------------------------------------------------------------------
// Maintain selection through filter-button changes
BOOL TThreadListView::filter_save_selection (CObject * pObSel)
{
	if (NULL == pObSel)
	{
		ASSERT(0);
		return FALSE;
	}

	TSaveSelHint * pSel = (TSaveSelHint *) pObSel;

	BOOL fRet = FALSE;
	int iSel;
	if (false == pSel->m_fApply)
	{
		// toggle flag
		pSel->m_fApply = true;

		iSel = m_hlbx.GetFirstSelection();
		if (LB_ERR != iSel)
		{
			TPersist822Header* pP822Hdr = m_hlbx.Getp_Header(iSel);

			if (0 == pP822Hdr)
				return FALSE;

			// save both the ArtInt and the plain index
			pSel->m_iFilterSaveArtInt = pP822Hdr->GetNumber ();
			pSel->m_iIndex = iSel;
		}
	}
	else
	{
		// we are restoring
		if (-1 != pSel->m_iFilterSaveArtInt)
		{
			if (m_hlbx.Find (pSel->m_iFilterSaveArtInt, &iSel, NULL))
			{
				fRet = TRUE;
			}
		}

		if (!fRet)
		{
			int nTot = m_hlbx.GetCount();
			if (pSel->m_iIndex >= 0  && (pSel->m_iIndex < nTot))
			{
				iSel = pSel->m_iIndex;
				fRet = TRUE;
			}
			else if (nTot > 0  && pSel->m_iIndex >= nTot)
			{
				iSel = nTot - 1;
				fRet = TRUE;
			}
		}

		if (fRet)      // do it
		{
			m_hlbx.SetOneSelection (iSel);
			m_hlbx . EnsureVisibleSelection ();    // added 6-15-98
		}
	}
	return fRet;
}

//-------------------------------------------------------------------------
// before adding to the TagList, remove any articles that already have
//  local bodies
int TThreadListView::discard_local(MAP_NODES * pstlMapNodes, int * pgid,
								   CString* pGrpName, QUE_HDRS * pstlQueHdrs)
{
	TServerCountedPtr cpNewsServer;
	*pgid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, *pgid, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	TPersistentTags & rTags = cpNewsServer->GetPersistentTags();

	MAP_NODES::iterator itN =  pstlMapNodes->begin();
	for (; itN != pstlMapNodes->end(); itN++)
	{
		TArtNode * pNode = (*itN).second;

		STLMapAHdr::iterator itH =  pNode->GetArtsItBegin();
		for (; itH != pNode->GetArtsItEnd(); itH++ )
		{
			TArticleHeader * pHdr =  *itH;
			int artInt = pHdr->GetNumber();

			if (pNG->TextRangeHave ( artInt ))
			{
				// self-correct here
				if (rTags.FindTag ( *pgid, artInt ))
					rTags.DeleteTagCancelJob (pNG->GetName(), *pgid, artInt, FALSE);
			}
			else
				pstlQueHdrs->push_back (  *itH );
		}
	}
	*pGrpName = pNG->GetName ();
	return 0;
}

// ------------------------------------------------------------------------
// 11-06-96  amc  I want to make the ArtPane reflect the current listbox
//                selection.  That way, the 'F' accelerator doesn't become
//                ambiguous - followup to listbox selection or the ArtPane
//                contents.
int TThreadListView::OnHLbxSelChange(TArticleHeader * pHdr)
{
	TPersist822Header * p822Hdr = GetNewsView()->GetBrowseHeader();
	if (0 == p822Hdr)
		return ArtPaneSummarize ( pHdr );

	if (p822Hdr->GetNumber() == pHdr->GetNumber())
		return 0;

	// in base class
	return ArtPaneSummarize ( pHdr );
}

//-------------------------------------------------------------------------
int TThreadListView::ChangeArticle_Permanent()
{
	TServerCountedPtr cpNewsServer;
	MAP_NODES stlMapNodes;

	// get selected articles
	engine_of_selection (&stlMapNodes, FALSE);

	// access newsgroup
	int gid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	// object to open - close newsgroup
	TAutoClose sAutoClose(pNG);

	// see if any are non-local
	int iOnServer = 0;
	int id ;

	MAP_NODES::iterator itN = stlMapNodes.begin();

	STLMapAHdr::iterator  itH;

	for (; itN != stlMapNodes.end(); itN++)
	{
		TArtNode * pNode = (*itN).second;

		for (itH = pNode->GetArtsItBegin(); itH != pNode->GetArtsItEnd(); itH++)
		{
			TArticleHeader * pHdr = *itH;
			id = pHdr->GetNumber();

			if ( !pNG->TextRangeHave (id) )
				++iOnServer;
		}
	}

	BOOL fTagIt = FALSE;
	if (iOnServer > 0)
	{
		TCHAR szNum[15]; _itot(iOnServer, szNum, 10);
		CString msg; AfxFormatString1(msg, IDS_TAGPERMARTS1, szNum);
		fTagIt = (IDYES == NewsMessageBox(this, msg, MB_YESNO | MB_ICONQUESTION));
	}

	TPersistentTags & rTags = cpNewsServer->GetPersistentTags();

	itN = stlMapNodes.begin();
	for (; itN != stlMapNodes.end(); itN++)
	{
		TArtNode * pNode = (*itN).second;

		for (itH = pNode->GetArtsItBegin(); itH != pNode->GetArtsItEnd(); itH++)
		{
			TArticleHeader * pHdr =  (*itH);

			id = pHdr->GetNumber();

			// tag it for the User
			if ((iOnServer > 0) && !pNG->TextRangeHave(id) && fTagIt)
				rTags.AddTag (pNG, id, pHdr->GetLines(), NULL);

			pNG->StatusBitSet(id, TStatusUnit::kPermanent, TRUE);
		}
	}
	return 0;
}

//-------------------------------------------------------------------------
int TThreadListView::ChangeThread_Permanent()
{
	TServerCountedPtr cpNewsServer;
	TPileIDS sPileIDS;

	int i ;
	int selCnt = m_hlbx.GetSelCount();

	if (selCnt <= 0)
		return LB_ERR;

	// note this is a shallow list
	{
		int * pIdx = new int[selCnt];
		auto_ptr<int> sIdxDeleter(pIdx);

		m_hlbx.GetSelItems (selCnt, pIdx);

		for (i = 0; i < selCnt; ++i)
		{
			TPersist822Header * p822Hdr = m_hlbx.Getp_Header(pIdx[i]);
			if (p822Hdr)
			{
				sPileIDS.m_ArtIdsToFind.Add ( p822Hdr->GetNumber() );
			}
		}
	}

	int gid = GetNewsView()->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	TAutoClose sAutoClose(pNG);

	// get deep contents
	pNG->FindNPileContents ( &sPileIDS );

	// figure out if we have any articles that are non-local
	int iOnServer = 0;
	int id;

	int tot = sPileIDS.m_ArtIdsFound.GetCount();
	for (i = 0; i < tot; ++i)
	{
		id = sPileIDS.m_ArtIdsFound.GetAt(i);

		if ( !pNG->TextRangeHave( id ) )
			++iOnServer;
	}

	// Ask to tag the rest?
	if (iOnServer > 0)
	{
		TCHAR szNum[15]; _itot(iOnServer, szNum, 10);
		CString msg; AfxFormatString1(msg, IDS_TAGPERMARTS1, szNum);

		BOOL fTagIt = (IDYES == NewsMessageBox(this, msg, MB_YESNO | MB_ICONQUESTION));

		for (i = 0; i < tot; ++i)
		{
			id = sPileIDS.m_ArtIdsFound.GetAt(i);
			if (fTagIt && !pNG->TextRangeHave (id))
			{
				// oops. we may need the actual Header here.
				TArticleHeader* pHdr = pNG->GetHeader(id)->CastToArticleHeader();
				int iLines = pHdr->GetLines ();
				TArticleHeader * pInputHdr = 0;
				if (TNewsGroup::kNothing == UtilGetStorageOption(pNG))
					pInputHdr = new TArticleHeader(*pHdr);

				// tag it.
				cpNewsServer->GetPersistentTags().AddTag (pNG,
					id,
					iLines,
					pInputHdr);
			}

			// mark it perm
			pNG->StatusBitSet (id, TStatusUnit::kPermanent, TRUE);
		}
	}
	else
		SetThreadStatus (TStatusUnit::kPermanent, TRUE);

	return 0;
}

//-------------------------------------------------------------------------
// do whatever rearranging you gotta do. Mainly make sure selection is not
//   clipped.
// Implementation note: if we are unzooming, but Gravity is actually
//  switching to another zoomed view. then we don't want to move selection
void TThreadListView::handle_zoom(BOOL fZoom, CObject * pObj)
{
	if (fZoom)
	{
		if (static_cast<CView *>(pObj) == this)
		{
			m_header.OnZoom(true);
			m_fTrackZoomed = true;
		}
	}
	else
	{
		TUnZoomHintInfo * pInfo = static_cast<TUnZoomHintInfo *>(pObj);

		if (pInfo->m_pTravelView == this)
		{
			m_header.OnZoom(false);

			if (pInfo->m_byWindowIsVisible)
				m_hlbx.EnsureVisibleSelection();
			m_fTrackZoomed = false;
		}
	}
}

// ------------------------------------------------------------------------
// Skip
void TThreadListView::OnArticleSkipNextUnread()
{
	// call public func
	Nav_SkipNextUnread();
}

// ------------------------------------------------------------------------
// public func.  Returns 0 for success
int TThreadListView::Nav_SkipNextUnread()
{
	return JumpToEngine(kQueryNextUnread, false, false /* LimitToPile */);
}

// ------------------------------------------------------------------------
void TThreadListView::OnArticleViewNextUnread()
{
	JumpToEngine(kQueryNextUnread, true, false);
}

// ------------------------------------------------------------------------
// Skip
//  disabled when in Flat sort
void TThreadListView::OnArticleSkipNextUnreadInThread()
{
	JumpToEngine(kQueryNextUnreadInThread, false, true);  // fLimitToPile
}

// ------------------------------------------------------------------------
// View
//  disabled when in Flat sort
void TThreadListView::OnArticleViewNextUnreadInThread()
{
	JumpToEngine(kQueryNextUnreadInThread, true, true); // fLimitToPile
}

// ------------------------------------------------------------------------
// orphan
void TThreadListView::OnArticleViewNextlocal()
{
	JumpToEngine(kQueryNextLocal, false, false);  // fLimitToPile
}

// ------------------------------------------------------------------------
// Skip
void TThreadListView::OnArticleSkipNextUnreadLocal()
{
	JumpToEngine(kQueryNextUnreadLocal, false, false); // fLimitToPile
}

// ------------------------------------------------------------------------
//
void TThreadListView::OnArticleViewNextUnreadLocal()
{
	Nav_ViewNextUnreadLocal();
}

// -----------------------------------------------------------------------
// returns 0 for success
// used directly by tnews3md.cpp, he needs the return code, can't use
// WM_COMMAND
int TThreadListView::Nav_ViewNextUnreadLocal()
{
	return JumpToEngine(kQueryNextUnreadLocal, true, false);  // fLimitToPile
}

// ------------------------------------------------------------------------
int TThreadListView::GetTopIndex() const
{
	return m_hlbx.GetTopIndex();
}

// ------------------------------------------------------------------------
void TThreadListView::SetTopIndex(int iIndex)
{
	m_hlbx.SetTopIndex(iIndex);
}

// ------------------------------------------------------------------------
int TThreadListView::GetSelCount() const
{
	return m_hlbx.GetSelectedCount();
}

// ------------------------------------------------------------------------
int TThreadListView::SetSel(int iIndex, BOOL bSelect /* = TRUE */)
{
	return m_hlbx.SetSel(iIndex, bSelect);
}

// ------------------------------------------------------------------------
int TThreadListView::GetSelItems(int iMaxItems, int *piItems) const
{
	return m_hlbx.GetSelItems(iMaxItems, piItems);
}

// ------------------------------------------------------------------------
int TThreadListView::GetCount() const
{
	return m_hlbx.GetItemCount();
}

// ------------------------------------------------------------------------
void TThreadListView::SetLbxFocus()
{
	m_hlbx.SetFocus();
}

// ------------------------------------------------------------------------
// used by filterbar. caller must handle any redisplay/reloading of the pane
void TThreadListView::SortBy(TTreeHeaderCtrl::EColumn eCol, bool fAscending)
{
	m_header.SetSortAscend(fAscending);
	m_header.SetSortColumn(eCol);

	m_header.Invalidate();
}

// ------------------------------------------------------------------------
void TThreadListView::OnPriorityDecode()
{
	ApplyToSelectedNodes(PriorityDecodeCallback);
}

// ------------------------------------------------------------------------
void TThreadListView::OnDecode()
{
	ApplyToSelectedNodes(NormalDecodeCallback);
}

// ------------------------------------------------------------------------
// Allow user to choose a directory and then decode articles to that
//   dir.
void TThreadListView::OnDecodeTo()
{
	if (DirectoryPicker(&m_hlbx, &m_decodeCBdata.decodeDir))
	{
		ApplyToSelectedNodes(NormalDecodeCallback,
			FALSE,
			DWORD(&m_decodeCBdata));
	}
}

// ------------------------------------------------------------------------
void TThreadListView::RefillTree(bool fFromUpdate)
{
	if (fFromUpdate)
		update_ascend_from_filter();

	// Layouts with 3 panes fill the tree completely.
	// Layouts with 4 panes just show the family of the
	//    current article.  Waits for a current article
	TUIMemory::EMdiLayout layout;
	layout = gpGlobalOptions->GetRegLayoutMdi()->m_layout;
	if (3 == gpUIMemory->LayoutPaneCount(layout))
	{
		UpdateSortFromTreeheader();
		FillPane(false);
	}
	else
		EmptyView();
}

// ------------------------------------------------------------------------
//  This is odd.  The ascend setting comes originally from the
//    viewfilter.  Subsequently it can change due to column clicks or
//    the Sort dialog
void TThreadListView::update_ascend_from_filter()
{
	TViewFilter * pVF = TNewsGroup::GetpCurViewFilter();
	if (pVF)
	{
		BOOL fAscending = TViewFilter::SortAscending(pVF->SortCode());
		m_header.SetSortAscend(fAscending ? true : false);
	}
}

// ------------------------------------------------------------------------
// This is mainly for the GlobalOptions|Navigate .  This cmdID doesn't have
//    a corresponding menu-item  [ID_NAVIGATE_SKIPDOWN]
//
// I think this should just move the selection down
//
void TThreadListView::OnNavigateSkipdown()
{
	// note - this will open branches as we go

	int nextIdx = NextItem();

	if (nextIdx != LB_ERR && nextIdx >= 0 && nextIdx < m_hlbx.GetCount())
	{
		m_hlbx.SetOneSelection(nextIdx);
		CenterListboxSelection(&m_hlbx, TRUE);
	}
}

void TThreadListView::OnNavigateSkipdownNoExpand()
{
	int nextIdx = NextItem(false);  // do not expand

	if (nextIdx != LB_ERR && nextIdx >= 0 && nextIdx < m_hlbx.GetCount())
	{
		m_hlbx.SetOneSelection(nextIdx);
		CenterListboxSelection(&m_hlbx, TRUE);
	}
}

void TThreadListView::OnExpandThread()
{
	int sel = m_hlbx.GetFirstSelection();
	int root;

	if (LB_ERR == sel)
		return;

	if (m_hlbx.GetThreadRoot(sel, root))
		sel = root;

	m_hlbx.OpenBranch(sel);

	m_hlbx.SetSel(sel);

	m_hlbx.RepaintIndex(sel);
}

void TThreadListView::OnUpdateExpandThread(CCmdUI* pCmdUI)
{
	bool bExpanded, bOneSelected;

	GetExpandState(bExpanded, bOneSelected);

	pCmdUI->Enable(!bExpanded && bOneSelected);
}

void TThreadListView::OnCollapseThread()
{
	int sel = m_hlbx.GetFirstSelection();
	int root;

	if (LB_ERR == sel)
		return;

	if (m_hlbx.GetThreadRoot(sel, root))
		sel = root;

	m_hlbx.CloseBranch(sel);
	m_hlbx.SetOneSelection(sel);
}

void TThreadListView::OnUpdateCollapseThread(CCmdUI* pCmdUI)
{
	bool bOneSelected, bExpanded;

	GetExpandState(bExpanded, bOneSelected);
	pCmdUI->Enable(bExpanded && bOneSelected);
}

void TThreadListView::GetExpandState(bool & bExpanded, bool & bOneSelected)
{
	int selItems[2];
	int numSelected;
	int root;

	numSelected = m_hlbx.GetSelItems(2, selItems);
	bOneSelected = (numSelected == 1);

	if (!bOneSelected)
		return;

	int sel = m_hlbx.GetFirstSelection();

	if (m_hlbx.GetThreadRoot(sel, root))
		sel = root;

	TArtNode* pNode = m_hlbx.GetpNode(sel);

	int artInt = pNode->GetArticleNumber();

	// is this open already?
	bExpanded = m_hlbx.IsOpened(artInt) ? true : false;
}

// returns 0 for success
int TThreadListView::OpenSelectedArticle()
{
	int nIndex = 0;

	if (GetCount() <= 0)
		return 1;

	if (GetSelItems(1, &nIndex) != 1)
		return 1;

	return m_hlbx.OpenArticleByIndex(nIndex);
}

void TThreadListView::OnDisplayRawArticle()
{
	int nIndex = 0;

	if (GetCount() <= 0)
		return;

	if (GetSelItems(1, &nIndex) != 1)
		return;

	m_hlbx.OpenArticleByIndex(nIndex, FALSE);
}

void TThreadListView::SelectionOK(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_hlbx.IsSelection());
}
