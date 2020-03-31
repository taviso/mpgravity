/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsview.cpp,v $
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
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.6  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.5  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.4  2008/10/15 23:30:23  richard_wood
/*  Fixed bug in EMail reply dialog, if a ReplyTo header was present, the email field in the dialog picked up the address from the previous email sent.
/*
/*  Revision 1.3  2008/09/19 14:51:35  richard_wood
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

// Newsview.cpp : implementation of the CNewsView class
//
//  4-18-96 amc  When an article is destroyed by the DEL key, the Flatview
//               and the thread view update each other.  I am no longer
//               using the odb callback stuff
//
#include "stdafx.h"
#include "News.h"

#include "Newsdoc.h"
#include "Newsview.h"
#include "mainfrm.h"

#include "globals.h"
#include "hints.h"
#include "hourglas.h"
#include "tmutex.h"

#include "posttmpl.h"

#include "custmsg.h"
#include "tglobopt.h"

#include "pobject.h"
#include "names.h"

#include "tasker.h"

#include "nggenpg.h"
#include "ngoverpg.h"            // TNewsGroupOverrideOptions
#include "ngfltpg.h"             // TNewsGroupFilterPage
#include "tsigpage.h"            // TSigPage (per newsgroup sigs)
#include "warndlg.h"
#include "gotoart.h"
#include "ngpurg.h"
#include "ngutil.h"

#include "tnews3md.h"            // TNews3MDIChildWnd
#include "mlayout.h"             // TMdiLayout
#include "artview.h"             // TArticleFormView
#include "statunit.h"            // TStatusUnit
#include "thrdlvw.h"             // TThreadListView
#include "utilrout.h"
#include "rgbkgrnd.h"
#include "rgwarn.h"
#include "rgfont.h"
#include "rgui.h"
#include "rgswit.h"
#include "log.h"
#include "utilsize.h"
//#include "hctldrag.h"
#include "rules.h"               // DoRuleSubstitution()
#include "TAskFol.h"             // how to handle "followup-to: poster"
#include "fileutil.h"            // UseProgramPath
#include "genutil.h"             // GetThreadView()
#include "licutil.h"
#include "nglist.h"              // TNewsGroupUseLock
#include "server.h"
#include "newsdb.h"              // gpStore
#include "vfilter.h"             // TViewFilter
#include "utilerr.h"
#include "servcp.h"              // TServerCountedPtr
#include "limithdr.h"            // CPromptLimitHeaders
#include "usrdisp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions * gpGlobalOptions;
extern TNewsTasker* gpTasker;

const UINT CNewsView::idCaptionTimer = 32100;
const UINT CNewsView::idSelchangeTimer = 32101;

enum MessageType {MESSAGE_TYPE_BUG, MESSAGE_TYPE_SUGGESTION,
MESSAGE_TYPE_SEND_TO_FRIEND};
MessageType giMessageType;       // what type of message are we composing?

#define FLATHDR_ID      9001
#define SCROLL_BMP_CX   15

#define  BASE_CLASS   CListView

/////////////////////////////////////////////////////////////////////////////
// CNewsView

IMPLEMENT_DYNCREATE(CNewsView, CListView)

BEGIN_MESSAGE_MAP(CNewsView, BASE_CLASS)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(IDR_NGPOPUP_OPEN, OnNgpopupOpen)
	ON_COMMAND(IDR_NGPOPUP_UNSUBSCRIBE, OnNewsgroupUnsubscribe)
	ON_COMMAND(IDR_NGPOPUP_CATCHUPALLARTICLES, OnNgpopupCatchupallarticles)
	ON_COMMAND(IDC_NGPOPUP_MANUAL_RULE, OnNgpopupManualRule)
	ON_COMMAND(ID_NEWSGROUP_POSTARTICLE, OnNewsgroupPostarticle)
	ON_COMMAND(ID_ARTICLE_FOLLOWUP, OnArticleFollowup)
	ON_COMMAND(ID_ARTICLE_REPLYBYMAIL, OnArticleReplybymail)
	ON_COMMAND(ID_ARTICLE_MAILTOFRIEND, OnArticleForward)
	ON_COMMAND(ID_FORWARD_SELECTED, OnForwardSelectedArticles)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_REPLYBYMAIL, OnUpdateArticleReplybymail)
	ON_UPDATE_COMMAND_UI(ID_FORWARD_SELECTED, OnUpdateForwardSelectedArticles)
//	ON_UPDATE_COMMAND_UI(ID_HELP_SENDBUGREPORT, OnUpdateHelpSendBugReport)
//	ON_UPDATE_COMMAND_UI(ID_HELP_SENDPRODUCTSUGGESTION, OnUpdateHelpSendSuggestion)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateSendToFriend)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_FOLLOWUP, OnUpdateArticleFollowup)
	ON_UPDATE_COMMAND_UI(ID_NEWSGROUP_POSTARTICLE, OnUpdateNewsgroupPostarticle)
	ON_COMMAND(IDR_NGPOPUP_PROPERTIES, OnNgpopupProperties)
//	ON_COMMAND(ID_HELP_SENDBUGREPORT, OnHelpSendBugReport)
//	ON_COMMAND(ID_HELP_SENDPRODUCTSUGGESTION, OnHelpSendSuggestion)
	ON_COMMAND(ID_FILE_SEND_MAIL, OnSendToFriend)
	ON_WM_TIMER()
	ON_MESSAGE (WMU_NEWSVIEW_GOTOARTICLE, GotoArticle)
	ON_MESSAGE (WMU_NEWSVIEW_PROCESS_MAILTO, ProcessMailTo)
	ON_MESSAGE (WMC_DISPLAY_ARTCOUNT, OnDisplayArtcount)
	ON_COMMAND           (ID_ARTICLE_SAVE_AS, OnSaveToFile)
	ON_UPDATE_COMMAND_UI (ID_ARTICLE_SAVE_AS, OnUpdateSaveToFile)
	ON_COMMAND (ID_THREAD_CHANGETHREADSTATUSTO_READ, OnThreadChangeToRead)
	ON_UPDATE_COMMAND_UI (IDR_NGPOPUP_CATCHUPALLARTICLES, OnUpdateNewsgroupCatchup)
	ON_UPDATE_COMMAND_UI (IDR_NGPOPUP_UNSUBSCRIBE, OnUpdateNewsgroupUnsubscribe)
	ON_UPDATE_COMMAND_UI (IDR_NGPOPUP_PROPERTIES, OnUpdateNewsgroupProperties)
	ON_UPDATE_COMMAND_UI (ID_THREAD_CHANGETHREADSTATUSTO_READ, OnUpdateThreadChangeToRead)
	ON_UPDATE_COMMAND_UI (IDC_VIEW_BINARY, OnUpdateDisable)
	ON_COMMAND(ID_CMD_TABAROUND, OnCmdTabaround)
	ON_COMMAND(ID_CMD_TABBACK, OnCmdTabBack)
	ON_COMMAND(ID_GETHEADERS_ALLGROUPS, OnGetheadersAllGroups)
	ON_UPDATE_COMMAND_UI(ID_GETHEADERS_ALLGROUPS, OnUpdateGetheadersAllGroups)
	ON_COMMAND(ID_GETHEADERS_MGROUPS, OnGetHeadersMultiGroup)
	ON_UPDATE_COMMAND_UI(ID_GETHEADERS_MGROUPS, OnUpdateGetHeadersMultiGroup)
	ON_COMMAND(ID_GETHEADER_LIMITED, OnGetheaderLimited)
	ON_UPDATE_COMMAND_UI(ID_GETHEADER_LIMITED, OnUpdateGetheaderLimited)
	ON_COMMAND(WMC_DISPLAYALL_ARTCOUNT, OnDisplayAllArticleCounts)
	ON_MESSAGE(WMU_MODE1_HDRS_DONE, OnMode1HdrsDone)
	ON_MESSAGE(WMU_NGROUP_HDRS_DONE, OnNewsgroupHdrsDone)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_MESSAGE(WMU_ERROR_FROM_SERVER, OnErrorFromServer)
	ON_MESSAGE(WMU_SELCHANGE_OPEN, OnSelChangeOpen)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_DELETE_SELECTED, OnUpdateDeleteSelected)
	ON_UPDATE_COMMAND_UI(IDC_NGPOPUP_MANUAL_RULE, OnUpdateNgpopupManualRule)
	ON_UPDATE_COMMAND_UI(IDR_NGPOPUP_OPEN, OnUpdateNgpopupOpen)
	ON_UPDATE_COMMAND_UI(ID_KEEP_SAMPLED, OnUpdateKeepSampled)
	ON_COMMAND(ID_KEEP_SAMPLED, OnKeepSampled)
	ON_UPDATE_COMMAND_UI(ID_KEEP_ALL_SAMPLED, OnUpdateKeepAllSampled)
	ON_COMMAND(ID_KEEP_ALL_SAMPLED, OnKeepAllSampled)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_POST_SELECTED, OnUpdatePostSelected)
	ON_COMMAND(ID_POST_SELECTED, OnPostSelected)
	ON_COMMAND(ID_ARTICLE_DELETE_SELECTED, OnArticleDeleteSelected)
	ON_COMMAND(ID_VERIFY_HDRS, OnVerifyLocalHeaders)
	ON_COMMAND(ID_HELP_RESYNC_STATI, OnHelpResyncStati)
	ON_COMMAND(ID_NEWSGROUP_PINFILTER, OnNewsgroupPinfilter)
	ON_UPDATE_COMMAND_UI(ID_NEWSGROUP_PINFILTER, OnUpdateNewsgroupPinfilter)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_MORE, OnUpdateArticleMore)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDisplayInfo)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_WM_MOUSEWHEEL()
	ON_UPDATE_COMMAND_UI(IDM_GETTAGGED_FORGROUPS, OnUpdateGettaggedForgroups)
	ON_COMMAND(IDM_GETTAGGED_FORGROUPS, OnGettaggedForgroups)
	ON_WM_VKEYTOITEM()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_THREADLIST_REFRESH, OnRefreshCurrentNewsgroup)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_NV_FORWARD_SELECTED, OnForwardSelectedArticles)
	ON_UPDATE_COMMAND_UI(ID_THREADLIST_REPLYBYMAIL, OnUpdateArticleReplybymail)
	ON_UPDATE_COMMAND_UI(ID_THREADLIST_POSTFOLLOWUP, OnUpdateArticleFollowup)
	ON_UPDATE_COMMAND_UI(ID_THREADLIST_POSTNEWARTICLE, OnUpdateNewsgroupPostarticle)
	ON_UPDATE_COMMAND_UI (IDC_DECODE, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI (IDC_MANUAL_DECODE, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI(ID_VERIFY_HDRS, OnUpdateGetHeadersMultiGroup)
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedText)
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
// utility function
void FreeGroupIDPairs (CPtrArray* pVec)
{
	int tot = pVec->GetSize();
	for (--tot; tot >= 0; --tot)
		delete ((TGroupIDPair*) pVec->GetAt(tot));
}

/////////////////////////////////////////////////////////////////////////////
// CNewsView construction/destruction

CNewsView::CNewsView()
{
	InitializeCriticalSection(&m_dbCritSect);
	iLocalOpen = 0;

	m_ngPopupMenu.LoadMenu (IDR_NGPOPUP);

	m_imageList.Create (IDB_SCROLL,
		SCROLL_BMP_CX, // width of frame
		1,              // gro factor
		RGB(255,0,255)  // transparent color (hot purple)
		);

	// setup the 1st overlay image
	VERIFY(m_imageList.SetOverlayImage (6, 1));

	m_curNewsgroupID = 0;
	m_pBrowsePaneHeader = 0;
	m_pBrowsePaneText = 0;

	m_fLoadFreshData = TRUE;

	m_hCaptionTimer = 0;
	m_hSelchangeTimer = 0;

	m_GotoArticle.m_articleNumber = -1;
	m_fPinFilter = FALSE;

	m_fTrackZoom = false;
	m_iOneClickInterference = 0;
}

/////////////////////////////////////////////////////////////////////////////
CNewsView::~CNewsView()
{
	if (m_pBrowsePaneText)
	{
		delete m_pBrowsePaneText;
		m_pBrowsePaneText = 0;
	}

	DeleteCriticalSection(&m_dbCritSect);
}

/////////////////////////////////////////////////////////////////////////////
// CNewsView drawing

void CNewsView::OnDraw(CDC* pDC)
{
	CNewsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CNewsView printing

BOOL CNewsView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CNewsView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CNewsView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

#if defined(OLAY_CRAP)
/////////////////////////////////////////////////////////////////////////////
// OLE Server support

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the server (not the container) causes the deactivation.
void CNewsView::OnCancelEditSrvr()
{
	GetDocument()->OnDeactivateUI(FALSE);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewsView diagnostics

#ifdef _DEBUG
void CNewsView::AssertValid() const
{
	BASE_CLASS::AssertValid();
}

void CNewsView::Dump(CDumpContext& dc) const
{
	BASE_CLASS::Dump(dc);
}

CNewsDoc* CNewsView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNewsDoc)));
	return (CNewsDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNewsView message handlers

int CNewsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	LogString("Newsview start create");
	if (BASE_CLASS::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetupFont ();

	GetListCtrl().SetImageList (&m_imageList, LVSIL_SMALL);

	GetListCtrl().SetCallbackMask (LVIS_OVERLAYMASK);

	// Strip CS_HREDRAW and CS_VREDRAW. We don't need to repaint everytime we
	//  are sized.  (Tip from 'PC Magazine' May 6,1997)
	DWORD dwStyle = GetClassLong (m_hWnd, GCL_STYLE);
	SetClassLong (m_hWnd, GCL_STYLE, dwStyle & ~(CS_HREDRAW | CS_VREDRAW));

	// OnInitialUpdate will Setup the columns in the header ctrl
	//   after the size has settled down.

	// read from registry
	m_fPinFilter = gpGlobalOptions->GetRegUI()->GetPinFilter ();

	((CMainFrame*) AfxGetMainWnd())->UpdatePinFilter ( m_fPinFilter );

	LogString("Newsview end create");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CNewsView::OnSize(UINT nType, int cx, int cy)
{
	BASE_CLASS::OnSize(nType, cx, cy);

	Resize ( cx, cy );
}

// called from OnSize, and OnInitialUpdate
void CNewsView::Resize(int cx, int cy)
{
}

// ------------------------------------------------------------------------
//
void CNewsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (VIEWHINT_SERVER_SWITCH == lHint)
		return;

	if (VIEWHINT_UNZOOM == lHint)
	{
		handle_zoom (false, pHint);
		return;
	}

	if (VIEWHINT_ZOOM == lHint)
	{
		handle_zoom (true, pHint);
		return;
	}

	if (VIEWHINT_SHOWARTICLE == lHint)
		return;

	if (VIEWHINT_SHOWGROUP == lHint)
		return;

	if (VIEWHINT_SHOWGROUP_NOSEL == lHint)
		return;

	if (VIEWHINT_EMPTY == lHint)
	{
		EmptyBrowsePointers ();
		GetDocument()->EmptyArticleStatusChange();
		sync_caption();
		return;
	}

	// if an article's status changes it means nothing to us
	if (VIEWHINT_STATUS_CHANGE == lHint ||
		VIEWHINT_ERASE_OLDTHREADS == lHint)
		return;

	//if (VIEWHINT_NEWSVIEW_UNSUBSCRIBE == lHint)
	//   {
	//   Unsubscribing ((TNewsGroup*) pHint);
	//   return;
	//   }

	// remember selected group-id
	LONG iSelGroupID = GetSelectedGroupID();

	CListCtrl & lc = GetListCtrl();
	lc.DeleteAllItems ();

	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vNewsGroups);

	int count = vNewsGroups->GetSize();

	for (int j = 0; j < count; ++j)
	{
		TNewsGroup* pNG = vNewsGroups[j];

		AddStringWithData ( pNG->GetBestname(), pNG->m_GroupID);
	}

	// restore selection
	if (iSelGroupID)
	{
		if (0 != SetSelectedGroupID (iSelGroupID))
			SetOneSelection (0);
	}
}

// 5-8-96  change to fetch on zero
void CNewsView::OnNgpopupOpen()
{
	OpenNewsgroup( kFetchOnZero, kPreferredFilter );
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateNgpopupOpen(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsExactlyOneNewsgroupSelected());
}

// ------------------------------------------------------------------------
// Returns -1 for failure
int  CNewsView::GetSelectedIndex ()
{
	CListCtrl & lc = GetListCtrl();
	POSITION pos = lc.GetFirstSelectedItemPosition ();
	if (pos)
		return lc.GetNextSelectedItem (pos);

	int iMark = lc.GetSelectionMark ();
	if (-1 == iMark)
		return -1;

	return iMark;
}

// ------------------------------------------------------------------------
// Returns positive for success, 0 for failure
LONG CNewsView::GetSelectedGroupID ()
{
	int idx = GetSelectedIndex ();
	if (-1 == idx)
		return 0;

	return (LONG) GetListCtrl().GetItemData (idx);
}

// ------------------------------------------------------------------------
// Input: a group id
// Set the listbox selection to the newsgroup that has that id
// Returns: 0 for success, non-zero for error
int CNewsView::SetSelectedGroupID (int iGroupID)
{
	CListCtrl & lc = GetListCtrl();

	int idx = lc.GetItemCount();
	for (--idx; idx >= 0; --idx)
	{
		if (iGroupID == (LONG) lc.GetItemData(idx))
		{
			SetOneSelection (idx);
			return 0;
		}
	}
	return 1;
}

// ------------------------------------------------------------------------
void CNewsView::OnNgpopupManualRule ()
{
	AfxGetMainWnd ()->PostMessage (WM_COMMAND, ID_OPTIONS_MANUAL_RULE);
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateNgpopupManualRule(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsOneOrMoreNewsgroupSelected ());
}

// ------------------------------------------------------------------------
void CNewsView::OnInitialUpdate()
{
	CListCtrl & lc = GetListCtrl();

	LogString("newsview start initial update");
	CNewsApp* pNewsApp = (CNewsApp*) AfxGetApp();
	pNewsApp->SetGlobalNewsDoc( GetDocument() );
	CNewsDoc::m_pDoc = GetDocument();

	CRect rct; GetClientRect( &rct );

	// load the widths from the registry
	SetupReportColumns (rct.Width());

	lc.SetExtendedStyle (LVS_EX_FULLROWSELECT);

	// the header control is too tall unless we do this
	Resize( rct.Width(), rct.Height() );

	// setup columns before initial fill
	BASE_CLASS::OnInitialUpdate();

	// setup tool tips

	VERIFY(m_sToolTips.Create (this, TTS_ALWAYSTIP));
	VERIFY(m_sToolTips.AddTool (this, LPSTR_TEXTCALLBACK));
	m_sToolTips.SetMaxTipWidth  (SHRT_MAX);
	m_sToolTips.SetDelayTime (TTDT_AUTOPOP, 10000);    // go away after 10 secs
	m_sToolTips.SetDelayTime (TTDT_INITIAL, 500);
	m_sToolTips.SetDelayTime (TTDT_RESHOW, 1000);

	// at this point the splash screen is gone and we are pretty much,
	//   up and running and willing to load the VCR file from the cmdline
	CWnd::FromHandle(ghwndMainFrame)->PostMessage (WMU_READYTO_RUN);

	int lastGID = gpGlobalOptions->GetRegUI()->GetLastGroupID();

	if (lastGID < 0)
	{
		if (lc.GetItemCount() > 0)
			lastGID = (int) lc.GetItemData( 0 );
	}

	TServerCountedPtr cpNewsServer;

	// find it
	bool fFoundValidGroup = false;

	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, lastGID, &fUseLock, pNG);
		if (fUseLock)
		{
			fFoundValidGroup = true;

			// don't wait for mode-1 to load
			if (TNewsGroup::kNothing == UtilGetStorageOption (pNG))
				fFoundValidGroup = false;
		}
	}

	if (fFoundValidGroup)
	{
		int tot = lc.GetItemCount();
		for (--tot; tot >= 0; --tot)
		{
			if (lastGID == (int) lc.GetItemData( tot ))
			{

				if (gpGlobalOptions->GetRegUI()->GetOneClickGroup())
				{
					SetOneSelection (tot);

					// LVN_ITEMCHANGED notification does the rest....
				}
				else
				{
					SetOneSelection (tot);
					OpenNewsgroup( kOpenNormal, kPreferredFilter, lastGID );
				}

				break;
			}
		}
	}

	LogString("newsview end initial update");
}

///////////////////////////////////////////////////////////////////////////
void CNewsView::SetupReportColumns (int iParentWidth)
{
	TRegUI* pRegUI = gpGlobalOptions->GetRegUI();
	int riWidths[3];

	if (0 != pRegUI->LoadUtilHeaders("NGPane", riWidths, ELEM(riWidths)))
	{
		int avgChar = LOWORD(GetDialogBaseUnits());
		riWidths[1] = avgChar * 6;
		riWidths[2] = avgChar * 6;
		int remainder = iParentWidth -riWidths[1] -riWidths[2]
		-GetSystemMetrics(SM_CXVSCROLL);
		riWidths[0] = max(remainder, avgChar*6);
	}

	CString strNewsgroups; strNewsgroups.LoadString (IDS_HEADER_NEWSGROUPS);
	CString strLocal;      strLocal.LoadString (IDS_HEADER_LOCAL);
	CString strServer;     strServer.LoadString (IDS_HEADER_SERVER);

	CListCtrl & lc = GetListCtrl();
	lc.InsertColumn (0, strNewsgroups, LVCFMT_LEFT,  riWidths[0], 0);
	lc.InsertColumn (1, strLocal,      LVCFMT_RIGHT, riWidths[1], 1);
	lc.InsertColumn (2, strServer,     LVCFMT_RIGHT, riWidths[2], 2);
}

///////////////////////////////////////////////////////////////////////////
// CloseCurrentNewsgroup - this was factored out of OpenNewsgroup and
//                         made public so that it could be used by
//                         CMainFrame::OnDatePurgeAll
///////////////////////////////////////////////////////////////////////////

void CNewsView::CloseCurrentNewsgroup ()
{
	BOOL  fViewsEmptied = FALSE;
	fViewsEmptied = ShutdownOldNewsgroup ();

	gpUIMemory->SetLastGroup ( m_curNewsgroupID );

	// the intent of this function is that there will be no
	// current group afterwards...

	SetCurNewsGroupID (0);

	if (!fViewsEmptied)
	{
		EmptyBrowsePointers ();

		// clear out TreeView, FlatView, ArtView
		GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);
	}
}

///////////////////////////////////////////////////////////////////////////
// OpenNewsgroup
//    eOpenMode     - kFetchOnZero if you want to Try to GetHeaders
//    iInputGroupID - pass in a groupID, or function will use the cur Sel
//
// Changes:
//    4-09-96  For the Dying NG, Empty the View and then Close it.
//             also call ::Empty to clean out the thread list
//
//    8-01-97  return 0 if loaded, 1 if started download, -1 for error
//
int  CNewsView::OpenNewsgroup(EOpenMode eMode, EUseFilter eFilter, int iInputGroupID/*=-1*/)
{
	int  iFilterRC = 0;
	BOOL fViewsEmptied = FALSE;
	LONG newGroupID;
	if (iInputGroupID > 0)
		newGroupID = iInputGroupID;
	else
	{
		newGroupID = GetSelectedGroupID ();
		if (NULL == newGroupID)
			return -1;
	}

	fViewsEmptied = ShutdownOldNewsgroup ();

	SetCurNewsGroupID( newGroupID );

	// set title on mdi-child
	if (0 == m_hCaptionTimer)
		m_hCaptionTimer = SetTimer (idCaptionTimer, 250, NULL);

	if (!fViewsEmptied)
	{
		EmptyBrowsePointers ();
		// clear out TreeView, FlatView, ArtView
		GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);
		fViewsEmptied = TRUE;
	}

	TServerCountedPtr cpNewsServer;
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);

		if (fUseLock)
		{
			// open this guy for business
			protected_open ( pNG, TRUE );

			if (kPreferredFilter == eFilter && (FALSE==m_fPinFilter))
			{
				// newsgroup may have a default filter. hand off to the Filterbar
				int iWhatFilter = GetPreferredFilter ( pNG );

				iFilterRC = ((CMainFrame*)AfxGetMainWnd())->SelectFilter ( iWhatFilter );
			}
			else
			{
				// if we are using GotoArticle, skip the preferred filter, use current
				// if filters are pinned. Use current
			}

			// if the newsgrp is devoid of new articles, then Try downloading some

			if ((kFetchOnZero == eMode) &&
				(TNewsGroup::kHeadersOnly == UtilGetStorageOption (pNG) ||
				TNewsGroup::kStoreBodies == UtilGetStorageOption (pNG)))
			{
				pNG->UpdateFilterCount (true);

				TViewFilter * pVF = TNewsGroup::GetpCurViewFilter();
				if (0 == pVF)
					return -1;
				TStatusUnit::ETriad eTri = pVF->GetNew();

				if (TStatusUnit::kYes == eTri)
				{
					int iLocalNew, iServerNew, iLocalTotal;
					iLocalNew = iServerNew = 100;
					pNG->FormatNewArticles ( iLocalNew, iServerNew, iLocalTotal );
					if (0 == iLocalNew)
					{
						// there's nothing in the DB to load - hit the server
						GetHeadersOneGroup ();
						return 1;
					}
				}
				if (TStatusUnit::kMaybe == eTri)
				{
					// they are viewing both New and Read
					if (0 == pNG->HdrRangeCount())
					{
						// there's absolutely nothing in the newgroup - hit server
						GetHeadersOneGroup ();
						return 1;
					}
				}
			}
			// when re-creating a new layout, we just use the old data.
			if (m_fLoadFreshData) {

				// when in mode 1, do all rule substitution before loading articles
				// NOTE: This must be done before the hourglass icon is displayed
				// (below)
				if (pNG->GetStorageOption () == TNewsGroup::kNothing)
					ResetAndPerformRuleSubstitution (NULL /* psHeader */, pNG);

				// tell rules that an hourglass icon is going to appear, and not to
				// bring up any dialogs (it will queue them and display them later)
				RulesHourglassStart ();

				// user waits....
				{
					CHourglass wait = this;
					pNG->LoadArticles(FALSE);
				}

				// now that the hourglass icon is gone, we can let rules display any
				// notification dialogs
				RulesHourglassEnd ();
			}
		}
	}

	// fill thread view
	please_update_views();

	// allow the layout manager to see this action
	CFrameWnd * pDadFrame = GetParentFrame();
	if (pDadFrame)
		pDadFrame->SendMessage (WMU_USER_ACTION, TGlobalDef::kActionOpenGroup);

	if (iFilterRC)
		AfxMessageBox (IDS_GRP_FILTERGONE);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Return TRUE if the views have been Emptied
//
BOOL CNewsView::ShutdownOldNewsgroup (void)
{
	// We can't shut down a group if the servers not alive
	if (!IsActiveServer())
		return TRUE;

	// save off status of old?
	BOOL fViewsEmptied = FALSE;

	TServerCountedPtr cpNewsServer;
	LONG oldGroupID = m_curNewsgroupID;
	BOOL fUseLock;
	TNewsGroup* pOld = 0;
	TNewsGroupUseLock useLock(cpNewsServer, oldGroupID, &fUseLock, pOld);

	if (fUseLock)
	{
		TUserDisplay_UIPane sAutoDraw("Clear list...");

		if (!fViewsEmptied)
		{
			// clear the threadview while the pointers are Valid
			EmptyBrowsePointers ();
			GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);
			fViewsEmptied = TRUE;
		}

		//      if (pOld->GetDirty())
		pOld->TextRangeSave();

		// Try closing the newsgroup
		protected_open (pOld, FALSE);

		// this is new (4/9) Empty the threadlist

		pOld->Empty();

		gpUserDisplay->SetCountFilter ( 0 );
		gpUserDisplay->SetCountTotal ( 0 );

	}
	return fViewsEmptied;
}

void CNewsView::please_update_views(void)
{
	TUserDisplay_UIPane sAutoDraw("Loading list...");

	// perform a broadcast directed at the threadlist view
	GetDocument()->UpdateAllViews(this, VIEWHINT_SHOWGROUP);
}

//-------------------------------------------------------------------------
// msg handler
void CNewsView::OnRefreshCurrentNewsgroup ()
{
	RefreshCurrentNewsgroup ( );
}

//-------------------------------------------------------------------------
// Called by filter buttons, tmanrule.cpp
void CNewsView::RefreshCurrentNewsgroup (bool fMoveSelection /* = true */)
{
	TServerCountedPtr cpNewsServer;
	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);

	if (fUseLock)
	{
		CWaitCursor  wait;
		TSaveSelHint sSelHint;

		if (fMoveSelection)
		{
			// Try to restore selection after change (do this before Emptying)
			GetDocument()->UpdateAllViews (this, VIEWHINT_SAVESEL, &sSelHint);
		}

		if (true)
		{
			TUserDisplay_UIPane sAutoDraw("Emptying");

			GetDocument()->UpdateAllViews (NULL, VIEWHINT_EMPTY);
		}

		// do not empty before reloading
		pNG->ReloadArticles (FALSE);

		TUserDisplay_UIPane sAutoDraw("Loading...");

		if (fMoveSelection)
		{
			// perform a broadcast directed at the threadlist view
			GetDocument()->UpdateAllViews (this, VIEWHINT_SHOWGROUP);

			// restore selection
			GetDocument()->UpdateAllViews (this, VIEWHINT_SAVESEL, &sSelHint);
		}
		else
		{
			// perform a broadcast directed at the threadlist view
			GetDocument()->UpdateAllViews (this, VIEWHINT_SHOWGROUP_NOSEL);
		}

	}
}

//-------------------------------------------------------------------------
void CNewsView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	//TRACE0("caught WM_CONTEXTMENU\n");
	CPoint anchor(point);

	// weird coords if menu summoned with Shift-F10
	if (-1 == anchor.x && -1 == anchor.y)
	{
		// do the best we can - top left corner of listbox
		anchor.x = anchor.y = 2;
		this->ClientToScreen( &anchor );
	}
	context_menu( anchor );
}

// ------------------------------------------------------------------------
//
void CNewsView::context_menu(CPoint & ptScreen)
{
	CListCtrl & lc = GetListCtrl();

	// multiple selection is OK for everything except 'Properties...'
	if (lc.GetSelectedCount () <= 0)
		return;

	// get the group name so we can do the op...
	int idx = GetSelectedIndex ();

	if (idx < 0)
		return;

	TServerCountedPtr cpNewsServer;

	LONG id = (LONG) lc.GetItemData( idx );
	BOOL fUseLock;
	TNewsGroup * pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, id, &fUseLock, pNG);
	if (!fUseLock)
		return;

	CMenu * pTrackMenu = m_ngPopupMenu.GetSubMenu ( 0 );

	ProbeCmdUI (this, pTrackMenu);
	pTrackMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this);
}

// ------------------------------------------------------------------------
void CNewsView::UnsubscribeGroups (const CPtrArray &vec)
{
	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

	for (int i = 0; i < vec.GetSize(); i++)
	{
		TGroupIDPair * pPair = static_cast<TGroupIDPair*>(vec[i]);
		bool fDemoted = false;  // TRUE if lbx-string is deleted

		{
			BOOL fUseLock = FALSE;
			TNewsGroup* pNG;
			TNewsGroupUseLock useLock(cpNewsServer, pPair->id, &fUseLock, pNG);

			// kill any queued jobs
			if (fUseLock)
				fDemoted = demote_newsgroup ( pNG );
		}

		// Try to get destroy lock on it
		if (0 == vNewsGroups.ReserveGroup (pPair->id))
		{
			cpNewsServer->UnsubscribeGroup (pPair->id);
		}
		else
		{
			// yikes. add string back into listbox.
			if (fDemoted)
				AddStringWithData (pPair->ngName, pPair->id);

			CString pattern; pattern.LoadString (IDS_ERR_GROUP_IN_USE);
			CString msg; msg.Format (pattern, (LPCTSTR) pPair->ngName);
			NewsMessageBox (this, msg, MB_OK | MB_ICONSTOP);
		}
	}

	// find new selection, open next newsgroup etc...
	finish_unsubscribe();
}

// ------------------------------------------------------------------------
void CNewsView::DoUnsubscribe (BOOL bAlwaysAsk)
{
	CPtrArray vec;   // hold data from _prelim function

	// confirm unsubscribe
	if (unsubscribe_prelim (&vec, bAlwaysAsk))
		return;

	CWaitCursor cursor;
	UnsubscribeGroups (vec);
	FreeGroupIDPairs (&vec);
}

// ------------------------------------------------------------------------
void CNewsView::OnNewsgroupUnsubscribe()
{
	DoUnsubscribe (FALSE /* bAlwaysAsk */);
}

// ------------------------------------------------------------------------
void CNewsView::OnArticleDeleteSelected()
{
	DoUnsubscribe (TRUE /* bAlwaysAsk */);
}

// ------------------------------------------------------------------------
// TRUE indicates the lbx line was deleted
bool CNewsView::demote_newsgroup ( TNewsGroup* pNG )
{
	extern TNewsTasker* gpTasker;
	CListCtrl & lc = GetListCtrl();

	int total = lc.GetItemCount();
	for (int i = 0; i < total; ++i)
	{
		if (pNG->m_GroupID == (int)lc.GetItemData(i))
		{
			// kill any queued jobs.
			gpTasker->UnregisterGroup ( pNG );

			// if we are killing the current newsgroup, open some
			//   other group
			if (pNG->m_GroupID == m_curNewsgroupID)
			{
				m_curNewsgroupID = 0;
			}

			lc.DeleteItem ( i );

			// figure out what to do with the selection
			int newsel;
			if (i == total - 1)
				newsel = i - 1;
			else
				newsel = i;

			if (newsel >= 0)
				SetOneSelection (newsel);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//  4-24-96 amc  Write changes to disk, reset title to Null
// 11-01-96 amc  Clear out views when going from Mode2(dead)->Mode1
int CNewsView::finish_unsubscribe (void)
{
	int         total;
	BOOL        fEmptySync = TRUE;

	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
	total = vNewsGroups->GetSize();

	if (total > 0)
	{
		BOOL fOpenGroup = FALSE;
		LONG newGroupID;

		// Pull in newly selected newsgroup
		//   iff we killed the current newsgroup
		if (0==m_curNewsgroupID)
		{
			// However... don't pull a newsgroup in
			//   if it means doing a Mode-1 suck
			{
				newGroupID = GetSelectedGroupID ();
				if (NULL == newGroupID)
					return 0;

				BOOL fUseLock;
				TNewsGroup* pNG = 0;
				TNewsGroupUseLock useLock(cpNewsServer, newGroupID, &fUseLock, pNG);

				if (!fUseLock)
					return 0;

				if (TNewsGroup::kHeadersOnly == UtilGetStorageOption(pNG) ||
					TNewsGroup::kStoreBodies == UtilGetStorageOption(pNG))
					fOpenGroup = TRUE;
			}
			if (fOpenGroup)
			{
				fEmptySync = FALSE;
				OpenNewsgroup(kOpenNormal, kPreferredFilter, newGroupID);
			}
		}
	}

	if (fEmptySync)
	{
		// empty all views
		GetDocument()->UpdateAllViews ( NULL, VIEWHINT_EMPTY );
		sync_caption ();        // "no newsgroup selected"
	}

	return 0;
}

// ------------------------------------------------------------------------
// gets info on multiselect
int CNewsView::multisel_get (CPtrArray * pVec, int* piSel)
{
	CListCtrl & lc = GetListCtrl();

	int sel = lc.GetSelectedCount();
	if (sel <= 0)
		return 1;

	int* pIdx = new int[sel];
	auto_ptr<int> pDeleterIdx(pIdx);

	// get selected items
	int n = 0;
	POSITION pos = lc.GetFirstSelectedItemPosition ();
	while (pos)
		pIdx[n++] = lc.GetNextSelectedItem (pos);

	// get the names and the group-ids

	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

	// get the each ng name and release

	for (int i = 0; i < sel; ++i)
	{
		int groupId = lc.GetItemData (pIdx[i]);
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock  useLock(cpNewsServer, groupId, &fUseLock, pNG);
		if (fUseLock)
			pVec->Add ( new TGroupIDPair(groupId, pNG->GetBestname()) );
	}

	*piSel = sel;
	return 0;
}

// ------------------------------------------------------------------------
// unsubscribe_prelim - Returns 0 for success. 1 for stop
//  Confirm that the user wants to unsubscribe from multiple newsgroups
//  Name and id pairs are returned in the PtrArray. Uses nickname if
//   it is set.
int CNewsView::unsubscribe_prelim (CPtrArray *pVec, BOOL bAlwaysAsk)
{
	int sel = 0;

	if (multisel_get (pVec, &sel))
		return 1;

	int nProtectedArticles = 0;

	TServerCountedPtr cpNewsServer;
	for (int i = 0; i < pVec->GetSize(); i++)
	{
		TGroupIDPair * pPair = static_cast<TGroupIDPair*>(pVec->GetAt(i));
		{
			BOOL fUseLock = FALSE;
			TNewsGroup* pNG;
			TNewsGroupUseLock useLock(cpNewsServer, pPair->id, &fUseLock, pNG);

			if (fUseLock)
			{
				int nProt = 0;
				pNG->StatusCountProtected ( nProt );
				nProtectedArticles += nProt;
			}
		}
	}

	// see if they really want to kill these groups...
	if (gpGlobalOptions->WarnOnUnsubscribe() || bAlwaysAsk || (nProtectedArticles > 0))
	{
		CString msg;
		CString strProtected;
		if (1 == sel)
		{
			if (pVec->GetSize () != 1)
				return 1;

			AfxFormatString1 (msg, IDS_WARNING_UNSUBSCRIBE,
				((TGroupIDPair*)pVec->GetAt(0))->ngName);
		}
		else
		{
			char szCount[10];
			_itoa (sel, szCount, 10);
			AfxFormatString1 (msg, IDS_WARN_UNSUBSCRIBE_MANY1, szCount);
		}

		if (nProtectedArticles > 0)
		{
			CString strProtected;
			strProtected.Format (IDS_WARNING_PROTCOUNT1, nProtectedArticles);
			msg += strProtected;
		}

		BOOL  fDisableWarning = FALSE;
		if (WarnWithCBX (msg,
			&fDisableWarning,
			NULL /* pParentWnd */,
			FALSE /* iNotifyOnly */,
			FALSE /* bDefaultToNo */,
			bAlwaysAsk /* bDisableCheckbox */))
		{
			if (fDisableWarning)
			{
				gpGlobalOptions->SetWarnOnUnsubscribe(FALSE);
				TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
				pRegWarning->Save();
			}

			return 0; // it's OK to go on
		}
		else
		{
			// free intermediate data
			FreeGroupIDPairs (pVec);
			return 1;
		}
	}

	return 0;  // it's OK to go on
}

/* ----------------------------------------------------------------
Case 1: You are in Mode 1, and you are viewing New & Read
messages. Catching up means marking things Read; After catching
up, you do not want to reload the same headers over your
PPP link (see Bug #68 part 2)
11-26-96 amc  User can move to next group after catchup on this one
------------------------------------------------------------------- */
void CNewsView::OnNgpopupCatchupallarticles()
{
	// holds a collection of  (groupID, name) pairs
	CPtrArray vec;

	// get info on multisel and allow cancellation
	if (catchup_prelim (&vec))
		return;

	int tot = vec.GetSize();
	for (int i = 0; i < tot; i++)
	{
		BOOL fMoveNext = (i == tot-1);

		TGroupIDPair* pPair = static_cast<TGroupIDPair*>(vec[i]);
		CatchUp_Helper ( pPair->id, fMoveNext );
	}

	// free intermediate data
	FreeGroupIDPairs (&vec);
}

// ------------------------------------------------------------------------
// Called from :
//       OnGetheaderGroup
//       OnNgpopupCatchupallarticles
void CNewsView::CatchUp_Helper (int iGroupID, BOOL fAllowMoveNext)
{
	TServerCountedPtr cpNewsServer;
	TNewsGroup * pNG = 0;
	BOOL fUseLock;
	// reacquire lock
	TNewsGroupUseLock useLock(cpNewsServer, iGroupID, &fUseLock, pNG);

	if (!fUseLock)
		return;

	BOOL fViewing = (pNG->m_GroupID == m_curNewsgroupID);
	if (TNewsGroup::kNothing == UtilGetStorageOption ( pNG ))
	{
		// Since this is Mode 1, avoid going out to the server again

		// marks everything as Read
		pNG->CatchUpArticles();

		OnDisplayArtcount(pNG->m_GroupID, 0);

		if (fViewing)
		{
			int iNextGrpId = -1;
			int iNextIdx;
			// see if we are moving onward
			if (fAllowMoveNext &&
				gpGlobalOptions->GetRegSwitch()->GetCatchUpLoadNext() &&
				validate_next_newsgroup(&iNextGrpId, &iNextIdx))
			{
				// clear out
				GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);

				// goto next ng
				AddOneClickGroupInterference();
				SetOneSelection (iNextIdx);
				OpenNewsgroup (kOpenNormal, kPreferredFilter, iNextGrpId);
			}
			else
			{
				TViewFilter * pFilter = TNewsGroup::GetpCurViewFilter ();
				if (0 == pFilter)
					return;

				TStatusUnit::ETriad eTri = pFilter->GetNew ();
				switch (eTri)
				{
				case TStatusUnit::kMaybe:
				case TStatusUnit::kNo:
					// this will show the result (all articles marked as Read)

					GetDocument()->UpdateAllViews(this, VIEWHINT_SHOWGROUP);
					break;

				case TStatusUnit::kYes:
					GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);
					break;
				}
			}
		}
		else
		{
			// catchup on a group that we are not viewing. No movement
		}

	}
	else
	{
		// this is Mode 2 or Mode 3

		if (fViewing)
			// empty out TreeView, FlatView, ArtView
			GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);

		pNG->CatchUpArticles();

		// newsview should redraw count of new articles (0)
		OnDisplayArtcount(pNG->m_GroupID, 0);

		if (fViewing)
		{
			int iNextGrpId = -1;
			int iNextIdx;
			// see if we are moving onward
			if (fAllowMoveNext &&
				gpGlobalOptions->GetRegSwitch()->GetCatchUpLoadNext() &&
				validate_next_newsgroup(&iNextGrpId, &iNextIdx))
			{
				EmptyBrowsePointers ();

				// goto next ng
				AddOneClickGroupInterference();
				SetOneSelection (iNextIdx);
				OpenNewsgroup (CNewsView::kOpenNormal, kPreferredFilter, iNextGrpId);
			}
			else
			{
				EmptyBrowsePointers ();

				// reload & update the display
				newsview_reload(pNG);
			}
		}
	}
}

// -----------------------------------------------------------------------
//  Called from OnNgpopupCatchupallarticles
void CNewsView::newsview_reload(TNewsGroup* pNG)
{
	ASSERT(pNG);
	CWaitCursor wait;
	if (pNG)
	{
		SetCurNewsGroupID( pNG->m_GroupID );

		// TRUE - empty thread list first, then load
		pNG->ReloadArticles(TRUE);

		// perform a broadcast directed at the threadlist view
		GetDocument()->UpdateAllViews(this, VIEWHINT_SHOWGROUP);
	}
}

// --------------------------------------------------------------
int CNewsView::catchup_prelim (CPtrArray* pVec)
{
	int sel = 0;
	if (multisel_get (pVec, &sel))
		return 1;

	// see if they really want to catchup on these groups...

	if (gpGlobalOptions->WarnOnCatchup())
	{
		CString msg;

		if (1 == sel)
		{
			if (pVec->GetSize () != 1)
				return 1;

			AfxFormatString1 (msg, IDS_WARNING_CATCHUP,
				((TGroupIDPair*)pVec->GetAt(0))->ngName);
		}
		else
		{
			char szCount[10];
			_itoa (sel, szCount, 10);
			AfxFormatString1 (msg, IDS_WARN_CATCHUP_MANY1, szCount);
		}

		BOOL  fDisableWarning = FALSE;
		if (WarnWithCBX (msg, &fDisableWarning))
		{
			if (fDisableWarning)
			{
				gpGlobalOptions->SetWarnOnCatchup(FALSE);
				TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
				pRegWarning->Save();
			}
		}
		else
		{
			// free intermediate data
			FreeGroupIDPairs (pVec);
			return 1;
		}
	}

	return 0; // it's OK to go on
}

// --------------------------------------------------------------
void CNewsView::OnUpdateNewsgroupPostarticle(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsNewsgroupDisplayed());
}

// --------------------------------------------------------------
// OnNewsgroupPostarticle -- post an article to the open newsgroup
void CNewsView::OnNewsgroupPostarticle ()
{
	if (!check_server_posting_allowed())
		return;

	ASSERT (m_curNewsgroupID);
	TPostTemplate *pTemplate = gptrApp->GetPostTemplate ();
	pTemplate->m_iFlags = TPT_TO_NEWSGROUP | TPT_CANCEL_WARNING_ID | TPT_POST;
	pTemplate->m_NewsGroupID = GetCurNewsGroupID();
	pTemplate->m_iCancelWarningID = IDS_WARNING_POSTCANCEL;
	pTemplate->Launch (GetCurNewsGroupID());
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdatePostSelected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsOneOrMoreNewsgroupSelected ());
}

// ------------------------------------------------------------------------
void CNewsView::OnPostSelected()
{
	CListCtrl & lc = GetListCtrl();

	// check if posting is allowed to this server
	if (!check_server_posting_allowed())
		return;

	// make template
	TPostTemplate *pTemplate = gptrApp->GetPostTemplate ();

	// get selected group IDs
	int iSel = lc.GetSelectedCount ();
	if (iSel <= 0)
	{
		ASSERT (0);
		return;
	}

	int * pIndices = new int [iSel];
	auto_ptr<int> pDeleterIndices (pIndices);

	// get selected items
	int n = 0;
	POSITION pos = lc.GetFirstSelectedItemPosition ();
	while (pos)
		pIndices[n++] = lc.GetNextSelectedItem (pos);

	bool fCurGroupInSelection = false;

	pTemplate->m_rNewsgroups.RemoveAll ();
	for (int i = 0; i < iSel; i++)
	{
		LONG gid = lc.GetItemData (pIndices [i]);
		if (m_curNewsgroupID == gid)
			fCurGroupInSelection = true;

		pTemplate->m_rNewsgroups.Add (gid);
	}

	// utilize Per-Group e-mail address override w.r.t. which Group?

	if (1 == iSel)
		pTemplate->m_NewsGroupID = lc.GetItemData (pIndices[0]);
	else
		pTemplate->m_NewsGroupID = (fCurGroupInSelection) ? m_curNewsgroupID : 0 ;

	// fill in rest of template and launch
	pTemplate->m_iFlags = TPT_TO_NEWSGROUPS | TPT_CANCEL_WARNING_ID | TPT_POST;
	pTemplate->m_iCancelWarningID = IDS_WARNING_POSTCANCEL;
	pTemplate->Launch (pTemplate->m_NewsGroupID);
}

// ------------------------------------------------------------------------
void CNewsView::OnArticleFollowup ()
{
	TServerCountedPtr cpNewsServer;
	CString followUps;
	TArticleText * pArtText = static_cast<TArticleText*>(m_pBrowsePaneText);

	// get article on demand if we need it.  I would like the TPostDoc to
	//   get the article, but the check for "followup-to: poster" already
	//   requires it early.

	if (true)
	{
		int  stat;
		BOOL fUseLock;
		TNewsGroup * pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);

		if (!fUseLock)
			return;

		BOOL fMarkAsRead = gpGlobalOptions->GetRegSwitch()->GetMarkReadFollowup();

		if (0 == pArtText)
		{
			TError  sErrorRet;
			CPoint ptPartID(0,0);

			TArticleText * pReturnArticleText = 0;
			stat = fnFetchBody (sErrorRet,
				pNG,
				(TArticleHeader *) m_pBrowsePaneHeader,
				pReturnArticleText,
				ptPartID,
				fMarkAsRead,  // mark-as-read
				TRUE);        // Try from newsfeed
			if (0 == stat)
			{
				SetBrowseText ( pReturnArticleText );
				pArtText = pReturnArticleText;
			}
			else
			{
				CString str; str.LoadString (IDS_ERR_COULD_NOT_RETRIEVE);
				NewsMessageBox ( this, str, MB_ICONSTOP | MB_OK );
				return;
			}
		}
		else
		{              // it's local
			TArticleHeader* pArtHdr = (TArticleHeader* ) m_pBrowsePaneHeader;

			if (pArtHdr && fMarkAsRead)
				pNG->ReadRangeAdd ( pArtHdr );
		}
	}

	BOOL fPostWithCC = FALSE;

	// check for "followup-to: poster"
	pArtText->GetFollowup ( followUps );
	followUps.TrimLeft();  followUps.TrimRight();

	// Son-of-1036 indicates exact match
	if ("poster" == followUps)
	{
		TAskFollowup dlg(this);

		dlg.DoModal();
		switch (dlg.m_eAction)
		{
		case TAskFollowup::kCancel:
			return;
		case TAskFollowup::kPostWithCC:
			fPostWithCC = TRUE;
			break;
		case TAskFollowup::kSendEmail:
			{
				OnArticleReplybymail ();
				return;
			}
		}
	}

	// Verify that server is "Posting Allowed".  This check is down here since
	//  Followup-To: poster can morph to an e-mail message

	if (!check_server_posting_allowed())
		return;

	ASSERT (m_curNewsgroupID);
	TPostTemplate *pTemplate = gptrApp->GetFollowTemplate ();
	pTemplate->m_iFlags =
		TPT_TO_NEWSGROUP |
		TPT_INSERT_ARTICLE |
		TPT_READ_ARTICLE_HEADER |
		TPT_CANCEL_WARNING_ID |
		TPT_FOLLOWUP |
		TPT_INIT_SUBJECT |
		TPT_USE_FOLLOWUP_INTRO |
		TPT_POST;

	if (fPostWithCC)
		pTemplate->m_iFlags |= TPT_POST_CC_AUTHOR;

	pTemplate->m_NewsGroupID = m_curNewsgroupID;
	pTemplate->m_iCancelWarningID = IDS_WARNING_FOLLOWUPCANCEL;
	pTemplate->m_strSubjPrefix.LoadString (IDS_RE);

	// lock the group while the article header and text are being accessed
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);
	if (!fUseLock)
		return;
	TSyncReadLock sync (pNG);

	// $$ MicroPlanet tech-support specific.  Turn on CC
	if (gpGlobalOptions->GetRegSwitch()->m_fUsePopup &&
		("support" == pNG->GetName() ||
		"suggest" == pNG->GetName() ||
		"regstudio" == pNG->GetName()))
		pTemplate->m_iFlags |= TPT_POST_CC_AUTHOR;

	pTemplate->m_pArtHdr = (TArticleHeader *) m_pBrowsePaneHeader;
	pTemplate->m_pArtText = (TArticleText *) m_pBrowsePaneText;
	pTemplate->m_strSubject =
		((TArticleHeader *) m_pBrowsePaneHeader)->GetSubject ();
	pTemplate->Launch (pTemplate->m_NewsGroupID);
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateArticleFollowup(CCmdUI* pCmdUI)
{
	// we must have a current article
	BOOL fEnable = FALSE;
	if (m_curNewsgroupID && m_pBrowsePaneHeader)
	{
		if (m_pBrowsePaneText)
			fEnable = TRUE;
		else
		{
			// the body is not here. But we can probably get it.
			fEnable = gpTasker->IsConnected();
		}
	}

	pCmdUI->Enable ( fEnable );
}

// ------------------------------------------------------------------------
void fnNewsView_ReplyTo (CString& strTo, CString& strFrom, TArticleText* pText)
{
	// use the 'Reply-To' field if present
	CString fldReplyTo;

	if (pText->GetReplyTo (fldReplyTo) && !fldReplyTo.IsEmpty())
	{
		// RLW : 16/10/08 : The effect of these #ifdefs and
		// DecodeElectronicAddress being commented out meant that
		// ReplyTo: was being ignored, and the email was getting
		// the email of the pervious email sent!!
		// So I have removed #ifdef and ASSERT...
		//#if defined(_DEBUG)
		//ASSERT(0);
		// RLW : 16/10/08 : removed commenting from this func...
		// this function will handle the rfc2047 encoding
		TArticleHeader::DecodeElectronicAddress (fldReplyTo, strTo);
		// RLW : 16/10/08 : removed #endif to implement correct behaviour.
		//#endif
	}
	else
		strTo = strFrom;
}

///////////////////////////////////////////////////////////////////////////
//  amc  4-2-97  Fix a deadlock problem. If you are replying to something
//               you haven't read, the Pump gets it on the fly and then
//                blocks on a WriteLock as it tries to mark the article
//                Read. Solution - release the ReadLock we acquired in this
//                function
void CNewsView::OnArticleReplybymail()
{
	TServerCountedPtr cpNewsServer;
	TPostTemplate *pTemplate = gptrApp->GetReplyTemplate ();
	pTemplate->m_iFlags =
		TPT_TO_STRING |
		TPT_INSERT_ARTICLE |
		TPT_READ_ARTICLE_HEADER |
		TPT_CANCEL_WARNING_ID |
		TPT_INIT_SUBJECT |
		TPT_USE_REPLY_INTRO |
		TPT_MAIL;
	pTemplate->m_iCancelWarningID = IDS_WARNING_REPLYCANCEL;
	pTemplate->m_strSubjPrefix.LoadString (IDS_RE);
	pTemplate->m_NewsGroupID = m_curNewsgroupID;

	// lock the group while the article header and text are being accessed
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);
	if (!fUseLock)
		return;

	// protects the header object
	pNG->ReadLock ();

	pTemplate->m_pArtHdr = (TArticleHeader *) m_pBrowsePaneHeader;

	// copy the From field
	CString strFrom  = pTemplate->m_pArtHdr->GetFrom ();

	pTemplate->m_strSubject = pTemplate->m_pArtHdr->GetSubject ();

	pTemplate->m_pArtText = (TArticleText *) m_pBrowsePaneText;

	if (pTemplate->m_pArtText)
	{
		pNG->UnlockRead ();
	}
	else
	{
		TError  sErrorRet;
		CPoint ptPartID(0,0);
		TArticleHeader tmpHdr(*pTemplate->m_pArtHdr);
		TArticleText * pReturnArticleText = 0;

		// fMarkRead will need a WriteLock !! Give it up.
		pNG->UnlockRead ();

		BOOL fMarkAsRead = gpGlobalOptions->GetRegSwitch()->GetMarkReadReply();

		int stat = fnFetchBody ( sErrorRet,
			pNG,
			&tmpHdr,
			pReturnArticleText,
			ptPartID,
			fMarkAsRead /* fMarkRead */,
			TRUE /* Try newsfeed */ );
		if (stat)
		{
			NewsMessageBox (this, IDS_ERR_COULD_NOT_RETRIEVE, MB_ICONSTOP | MB_OK);
			return;
		}

		SetBrowseText ( pReturnArticleText );
		pTemplate->m_pArtText = pReturnArticleText;
	}

	// setup the 'To:' field. Use the 'Reply-To' field if present
	fnNewsView_ReplyTo ( pTemplate->m_strTo, strFrom,
		pTemplate->m_pArtText );

	// proceed
	pTemplate->Launch (pTemplate->m_NewsGroupID);
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateArticleReplybymail(CCmdUI* pCmdUI)
{
	if (!IsActiveServer())
	{
		pCmdUI->Enable (FALSE);
		return;
	}

	TServerCountedPtr cpNewsServer;
	BOOL fArticleBody = m_pBrowsePaneText ? TRUE : gpTasker->IsConnected();
	pCmdUI->Enable (
		!cpNewsServer->GetSmtpServer ().IsEmpty () &&
		m_curNewsgroupID &&
		m_pBrowsePaneHeader &&
		fArticleBody
		);
}

////////////////////////////////////////////////////////////////
static BOOL gbForwardingSelected;   // forwarding all selected articles?
void CNewsView::OnArticleForward ()
{
	TServerCountedPtr cpNewsServer;
	TPostTemplate *pTemplate = gptrApp->GetForwardTemplate ();
	pTemplate->m_iFlags =
		TPT_CANCEL_WARNING_ID |
		TPT_MAIL;
	pTemplate->m_NewsGroupID = m_curNewsgroupID;
	pTemplate->m_iCancelWarningID = IDS_WARNING_FORWARDCANCEL;
	pTemplate->m_strSubjPrefix.LoadString (IDS_FWD);
	pTemplate->m_iFlags |=
		gbForwardingSelected ? (TPT_INSERT_SELECTED_ARTICLES | TPT_INIT_SUBJECT) :
		(TPT_READ_ARTICLE_HEADER | TPT_INSERT_ARTICLE | TPT_INIT_SUBJECT);

	// put this newsgroup's name in the template's "extra text file" field
	{
		BOOL fUseLock;
		TNewsGroup *pNG;
		TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);
		if (!fUseLock)
			return;
		pTemplate->m_strExtraTextFile = pNG->GetName ();
	}

	// when forwarding, don't wrap, don't quote, ignore the line limit, and
	// insert the special prefix message
	pTemplate->m_iFlags |= TPT_DONT_WRAP | TPT_DONT_QUOTE |
		TPT_IGNORE_LINE_LIMIT | TPT_USE_FORWARD_INTRO;

	if (!gbForwardingSelected) {
		// lock the group while the article header and text are being accessed
		BOOL fUseLock;
		TNewsGroup *pNG;
		TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);
		if (!fUseLock)
			return;
		TSyncReadLock sync (pNG);
		pTemplate->m_pArtHdr = (TArticleHeader *) m_pBrowsePaneHeader;
		pTemplate->m_pArtText = (TArticleText *) m_pBrowsePaneText;
		pTemplate->m_strSubject =
			((TArticleHeader *) m_pBrowsePaneHeader)->GetSubject ();
	}
	else {
		// get the subject from the first selected article
		TThreadListView *pView = GetThreadView ();
		ASSERT (pView);
		TArticleHeader *pHeader = NULL;
		pView->GetFirstSelectedHeader ((TPersist822Header *&) pHeader);
		ASSERT (pHeader);
		pTemplate->m_strSubject = pHeader->GetSubject ();
	}

	pTemplate->Launch (pTemplate->m_NewsGroupID);
	gbForwardingSelected = FALSE;
}

////////////////////////////////////////////////////////////////
void CNewsView::OnForwardSelectedArticles ()
{
	gbForwardingSelected = TRUE;
	OnArticleForward ();
}

void CNewsView::OnUpdateForwardSelectedArticles (CCmdUI* pCmdUI)
{
	TServerCountedPtr cpNewsServer;

	const CString & host = cpNewsServer->GetSmtpServer();
	if (!host.IsEmpty() && m_curNewsgroupID &&
		m_pBrowsePaneHeader && m_pBrowsePaneText)
		pCmdUI->Enable (TRUE);
	else
		pCmdUI->Enable (FALSE);
}

///////////////////////////////////////////////////
void CNewsView::SetBrowseText(TPersist822Text* pText)
{
	delete m_pBrowsePaneText;
	m_pBrowsePaneText = pText;
}

TPersist822Text * CNewsView::GetBrowseText(void)
{
	//ASSERT(m_pBrowsePaneArticle);
	return m_pBrowsePaneText;
}

///////////////////////////////////////////////////
void CNewsView::SetBrowseHeader(TPersist822Header* pHdr)
{
	m_pBrowsePaneHeader = pHdr;
}

TPersist822Header * CNewsView::GetBrowseHeader(void)
{
	//ASSERT(m_pBrowsePaneArticle);
	return m_pBrowsePaneHeader;
}

///////////////////////////////////////////////////
void CNewsView::SetCurNewsGroupID(LONG id)
{
	m_curNewsgroupID = id;
}

LONG CNewsView::GetCurNewsGroupID(void)
{
	return m_curNewsgroupID;
}

void CNewsView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: Add your specialized code here and/or call the base class
	if (bActivate)
	{
		SetFocus ();

		// let the mdichild window know that we have the focus
		((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->PostMessage ( WMU_CHILD_FOCUS, 0, 0L );
	}
	//  IMHO  this vvv is bullshit -al
	//BASE_CLASS::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// This is public, so the threadView can route msgs through us
BOOL CNewsView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	return BASE_CLASS::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

////////////////////////////////////////////////////////////////////
// Note: the Deregister also happens when we destroy & recreate the
//    layout configuration
void CNewsView::OnDestroy()
{
	// save off status before we shut down

	{
		if (IsActiveServer())
		{
			TServerCountedPtr cpNewsServer;
			BOOL fUseLock;
			TNewsGroup* pOld = 0;
			TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pOld);

			if (fUseLock)
			{
				if (pOld && (pOld->GetDirty()) )
					pOld->TextRangeSave();

				// close the current newsgroup
				if (pOld->IsOpen())
					pOld->Close();
			}
		}
	}

	SaveColumnSettings();

	// m_fPinFilter doesn't need to be written. it is saved immediately on Toggl

	BASE_CLASS::OnDestroy();
}

//-------------------------------------------------------------------------
// PropertyPage
void CNewsView::OnNgpopupProperties()
{
	int  idx = GetSelectedIndex ();
	if (idx < 0)
		return;

	LONG id = (LONG)GetListCtrl().GetItemData(idx);

	TServerCountedPtr cpNewsServer;
	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, id, &fUseLock, pNG);

	if (!fUseLock)
		return;

	TNewsGroup::EStorageOption eOldStorage = UtilGetStorageOption (pNG);

	TNewGroupGeneralOptions       pgGeneral;
	TPurgePage                    pgPurge;
	TNewsGroupOverrideOptions     pgOverride;
	TNewsGroupFilterPage          pgFilter;
	TSigPage                      pgSignature;

	pgGeneral.m_kStorageOption = pgGeneral.m_iOriginalMode = (int) eOldStorage;

	pNG->GetServerBounds ( pgGeneral.m_iServerLow, pgGeneral.m_iServerHi );
	int iHiRead = 0;
	if (pNG->GetHighestReadArtInt (&iHiRead))
		pgGeneral.m_iHighestArticleRead = iHiRead;

	pgGeneral.m_lowWater = pNG->GetLowwaterMark();
	pgGeneral.m_nickname = pNG->GetNickname ();
	pgGeneral.m_bSample = pNG->IsSampled ();
	pgGeneral.m_fInActive = pNG->IsActiveGroup() ? FALSE : TRUE ;

	//pgGeneral.m_fUseGlobalStorageOptions = pNG->UseGlobalStorageOptions ();
	pgGeneral.m_fCustomNGFont = FALSE;
	pgGeneral.m_strGroupName = pNG->GetName ();
	if (gpGlobalOptions->IsCustomNGFont())
	{
		CopyMemory (&pgGeneral.m_ngFont, gpGlobalOptions->GetNewsgroupFont(), sizeof(LOGFONT));
		pgGeneral.m_newsgroupColor = gpGlobalOptions->GetRegFonts()->GetNewsgroupFontColor();
		pgGeneral.m_fCustomNGFont   = TRUE;
	}
	else
		CopyMemory (&pgGeneral.m_ngFont, &gpGlobalOptions->m_defNGFont, sizeof(LOGFONT));

	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors ();

	pgGeneral.m_newsgroupBackground = pBackgrounds->GetNewsgroupBackground();
	pgGeneral.m_fDefaultBackground = gpGlobalOptions->GetRegSwitch()->IsDefaultNGBG();

	pgPurge.m_fOverride    = pNG->IsPurgeOverride();
	pgPurge.m_fPurgeRead   = UtilGetPurgeRead(pNG);
	pgPurge.m_iReadLimit   = UtilGetPurgeReadLimit(pNG);
	pgPurge.m_fPurgeUnread = UtilGetPurgeUnread(pNG);
	pgPurge.m_iUnreadLimit = UtilGetPurgeUnreadLimit(pNG);
	pgPurge.m_fOnHdr       = UtilGetPurgeOnHdrs(pNG);
	pgPurge.m_iDaysHdrPurge= UtilGetPurgeOnHdrsEvery(pNG);
	pgPurge.m_fShutdown    = UtilGetCompactOnExit(pNG);
	pgPurge.m_iShutCompact = UtilGetCompactOnExitEvery(pNG);

	pgOverride.m_bOverrideCustomHeaders = pNG->GetOverrideCustomHeaders ();
	pgOverride.m_bOverrideEmail = pNG->GetOverrideEmail ();
	pgOverride.m_bOverrideFullName = pNG->GetOverrideFullName ();
	pgOverride.m_bOverrideDecodeDir = pNG->GetOverrideDecodeDir ();
	pgOverride.m_fOverrideLimitHeaders = pNG->GetOverrideLimitHeaders ();
	pgOverride.m_strDecodeDir = pNG->GetDecodeDir ();
	pgOverride.m_strEmail = pNG->GetEmail ();
	pgOverride.m_strFullName = pNG->GetFullName ();
	CopyCStringList (pgOverride.m_sCustomHeaders, pNG->GetCustomHeaders ());
	pgOverride.m_iHeaderLimit = pNG->GetHeadersLimit ();

	// newsgroup filter page
	pgFilter.m_iFilterID = pNG->GetFilterID();
	pgFilter.m_fOverrideFilter = pgFilter.m_iFilterID != 0;

	// newsgroup signature page
	pgSignature.m_fCustomSig = pNG->GetUseSignature();
	pgSignature.m_strShortName = pNG->GetSigShortName();

	CPropertySheet newsgroupProperties(pNG->GetName ());
	int   iRC;

	newsgroupProperties.AddPage (&pgGeneral);
	newsgroupProperties.AddPage (&pgPurge);
	newsgroupProperties.AddPage (&pgOverride);
	newsgroupProperties.AddPage (&pgFilter);
	newsgroupProperties.AddPage (&pgSignature);
	iRC = newsgroupProperties.DoModal ();

	if (IDOK == iRC)
	{
#if defined(_DEBUG) && defined(VERBOSE)
		CString msg; msg.Format("SrHi= %d; SrLo= %d; HiRead=%d",
			pgGeneral.m_iServerHi,
			pgGeneral.m_iServerLow,
			pgGeneral.m_iHighestArticleRead);

		MessageBox (msg);
#endif

		pNG->SetNickname (pgGeneral.m_nickname);
		pNG->Sample (pgGeneral.m_bSample);

		sync_caption();

		if (pgGeneral.m_iHighestArticleRead != pgGeneral.m_iHighestArticleRead0)
		{
			// user has chosen to go back!
			int iLowWater = pNG->GetLowwaterMark ();

			if (pgGeneral.m_iHighestArticleRead < iLowWater)
				pNG->SetLowwaterMark (pgGeneral.m_iHighestArticleRead);

			pNG->ResetHighestArticleRead (pgGeneral.m_iHighestArticleRead,
				pgGeneral.m_iHighestArticleRead0);

			// if user changed the GoBack, then we should save this
			cpNewsServer->SaveReadRange ();
		}

		pNG->SetStorageOption (TNewsGroup::EStorageOption(pgGeneral.m_kStorageOption));

		pNG->SetUseGlobalStorageOptions (/*pgGeneral.m_fUseGlobalStorageOptions*/FALSE);

		gpGlobalOptions->CustomNGFont (pgGeneral.m_fCustomNGFont);
		gpGlobalOptions->GetRegSwitch()->SetDefaultNGBG(pgGeneral.m_fDefaultBackground);
		pBackgrounds->SetNewsgroupBackground(pgGeneral.m_newsgroupBackground);
		gpGlobalOptions->GetRegFonts()->SetNewsgroupFontColor( pgGeneral.m_newsgroupColor );
		if (pgGeneral.m_fCustomNGFont)
			gpGlobalOptions->SetNewsgroupFont ( &pgGeneral.m_ngFont );
		pNG->SetActiveGroup (!pgGeneral.m_fInActive);

		pNG->SetPurgeOverride(pgPurge.m_fOverride);
		pNG->SetPurgeRead(pgPurge.m_fPurgeRead);
		pNG->SetPurgeReadLimit(pgPurge.m_iReadLimit);
		pNG->SetPurgeUnread(pgPurge.m_fPurgeUnread);
		pNG->SetPurgeUnreadLimit(pgPurge.m_iUnreadLimit);
		pNG->SetPurgeOnHdrs(pgPurge.m_fOnHdr);
		pNG->SetPurgeOnHdrsEvery(pgPurge.m_iDaysHdrPurge);
		pNG->SetCompactOnExit(pgPurge.m_fShutdown);
		pNG->SetCompactOnExitEvery(pgPurge.m_iShutCompact);

		pNG->SetOverrideCustomHeaders (pgOverride.m_bOverrideCustomHeaders);
		pNG->SetOverrideEmail (pgOverride.m_bOverrideEmail);
		pNG->SetOverrideFullName (pgOverride.m_bOverrideFullName);
		pNG->SetOverrideDecodeDir (pgOverride.m_bOverrideDecodeDir);
		pNG->SetOverrideLimitHeaders (pgOverride.m_fOverrideLimitHeaders);

		pNG->SetDecodeDir (pgOverride.m_strDecodeDir);
		pNG->SetEmail (pgOverride.m_strEmail);
		pNG->SetFullName (pgOverride.m_strFullName);
		pNG->SetCustomHeaders (pgOverride.m_sCustomHeaders);
		pNG->SetHeadersLimit (pgOverride.m_iHeaderLimit);

		// destroy stuff if we transition from (Mode 2,3)->(Mode 1)
		if ((TNewsGroup::kHeadersOnly == eOldStorage ||
			TNewsGroup::kStoreBodies == eOldStorage) &&
			TNewsGroup::kNothing == UtilGetStorageOption (pNG))
		{
			if (pNG->m_GroupID == m_curNewsgroupID)
			{
				// probably lots of articles have been removed
				EmptyBrowsePointers ();
				GetDocument()->UpdateAllViews(this, VIEWHINT_EMPTY);

				// empty out thread list
				pNG->Empty ();
			}

			// destroy headers for which there are no bodies
			//  -- what do we have to lock down?
			pNG->StorageTransition (eOldStorage);

		}

		// associated filter
		if (pgFilter.m_fOverrideFilter)
			pNG->SetFilterID (pgFilter.m_iFilterID);
		else
			pNG->SetFilterID (0);

		pNG->SetUseSignature (pgSignature.m_fCustomSig);
		pNG->SetSigShortName (pgSignature.m_strShortName);

		// if user changed the GoBack, then we should save this
		//  "this is done above" cpNewsServer->SaveReadRange ();

		// make it last
		cpNewsServer->SaveIndividualGroup ( pNG );
		gpStore->SaveGlobalOptions ();

		CMDIChildWnd * pMDIChild;
		CMDIFrameWnd * pMDIFrame = (CMDIFrameWnd*) AfxGetMainWnd();

		// note this is only 1 of the mdi children. could be more.
		//  we aren't handling them.
		pMDIChild = pMDIFrame->MDIGetActive();
		// apply the new ng font
		pMDIChild->PostMessage ( WMU_NEWSVIEW_NEWFONT );

		GetListCtrl().DeleteItem (idx);

		// use name or nickname
		if (0==AddStringWithData (pNG->GetBestname(), pNG->m_GroupID, &idx))
			SetOneSelection (idx);

	} // IDOK == DoModal()
}

// -------------------------------------------------------------------------
static void MicroplanetDoesntLikeSpamBlockingAddresses ()
{
	TServerCountedPtr pServer;
	CString strAddress = pServer->GetEmailAddress ();
	strAddress.MakeUpper ();
	if (strAddress.Find ("SPAM") >= 0 ||
		strAddress.Find ("REMOVE") >= 0 ||
		strAddress.Find ("DELETE") >= 0)
		MsgResource (IDS_MICROPLANET_DOESNT_LIKE_SPAM);
}

// -------------------------------------------------------------------------
void CNewsView::ComposeMessage ()
{
	// if mailing to microplanet, check the from address for spam-blocking
	if (giMessageType == MESSAGE_TYPE_SUGGESTION ||
		giMessageType == MESSAGE_TYPE_BUG)
		MicroplanetDoesntLikeSpamBlockingAddresses ();

	TPostTemplate *pTemplate =
		giMessageType == MESSAGE_TYPE_BUG ? gptrApp->GetBugTemplate () :
		(giMessageType == MESSAGE_TYPE_SUGGESTION ?
		gptrApp->GetSuggestionTemplate () :
	gptrApp->GetSendToFriendTemplate ());
	pTemplate->m_strSubjPrefix = "";
	pTemplate->m_iFlags = TPT_CANCEL_WARNING_ID | TPT_INIT_SUBJECT | TPT_MAIL;

	switch (giMessageType) {
	  case MESSAGE_TYPE_BUG:
		  pTemplate->m_iFlags |= TPT_TO_STRING | TPT_INSERT_MACHINEINFO;
		  pTemplate->m_iCancelWarningID = IDS_WARNING_BUGCANCEL;
		  pTemplate->m_strSubject.LoadString (IDS_BUG_REPORT);
#if defined(KISS) || defined(KISS_TRIAL)
		  pTemplate->m_strTo.LoadString (IDS_SUPPORT_KISS);
#else
		  pTemplate->m_strTo.LoadString (IDS_SUPPORT_ADDR);
#endif
		  break;
	  case MESSAGE_TYPE_SUGGESTION:
		  pTemplate->m_iFlags |= TPT_TO_STRING;
		  pTemplate->m_iCancelWarningID = IDS_WARNING_SUGGESTCANCEL;
		  pTemplate->m_strSubject.LoadString (IDS_NEWS32_SUGGESTION);
		  pTemplate->m_strTo.LoadString (IDS_SUGGEST_ADDR);
		  break;
	  case MESSAGE_TYPE_SEND_TO_FRIEND:
		  {
			  TPath   appFilespec;
			  CString & infoFilename = pTemplate->m_strExtraTextFile;

			  pTemplate->m_iCancelWarningID = IDS_WARNING_MESSAGECANCEL;
			  pTemplate->m_strSubject.LoadString (IDS_NEWS32_INFORMATION);
			  pTemplate->m_iFlags |= TPT_INSERT_FILE;

			  // look in the App's directory
			  infoFilename.LoadString (IDS_INFO_FILE);
			  TFileUtil::UseProgramPath(infoFilename, appFilespec);
			  pTemplate->m_strExtraTextFile = appFilespec;
			  break;
		  }
	  default:
		  ASSERT (0);
		  break;
	}

	pTemplate->Launch ( 0 /* newsgroupID */);
}

// ------------------------------------------------------------------------
//void CNewsView::OnHelpSendBugReport()
//{
//	giMessageType = MESSAGE_TYPE_BUG;
//	ComposeMessage ();
//}

// ------------------------------------------------------------------------
//void CNewsView::OnUpdateHelpSendBugReport (CCmdUI* pCmdUI)
//{
//	TServerCountedPtr cpNewsServer;
//
//	const CString & host = cpNewsServer->GetSmtpServer();
//	if (!host.IsEmpty())
//		pCmdUI->Enable (TRUE);
//	else
//		pCmdUI->Enable (FALSE);
//}

// ------------------------------------------------------------------------
//void CNewsView::OnHelpSendSuggestion()
//{
//	giMessageType = MESSAGE_TYPE_SUGGESTION;
//	ComposeMessage ();
//}
//
//// ------------------------------------------------------------------------
//void CNewsView::OnUpdateHelpSendSuggestion (CCmdUI* pCmdUI)
//{
//	OnUpdateHelpSendBugReport(pCmdUI);
//}

// ------------------------------------------------------------------------
void CNewsView::OnSendToFriend()
{
	giMessageType = MESSAGE_TYPE_SEND_TO_FRIEND;
	ComposeMessage ();
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateSendToFriend (CCmdUI* pCmdUI)
{
	TServerCountedPtr cpNewsServer;

	const CString & host = cpNewsServer->GetSmtpServer();
	if (!host.IsEmpty())
		pCmdUI->Enable (TRUE);
	else
		pCmdUI->Enable (FALSE);
}

// ------------------------------------------------------------------------
// Keep trying to set the Caption on the mdichild
void CNewsView::OnTimer(UINT nIDEvent)
{
	if (idCaptionTimer == nIDEvent)
	{
		if (sync_caption())
		{
			KillTimer ( m_hCaptionTimer );
			m_hCaptionTimer = 0;
		}
	}
	else if (CNewsView::idSelchangeTimer == nIDEvent)
	{
		// this is related to 1-click open NG
		stop_selchange_timer ();
		//TRACE("SelchangeTimer popped\n");
		PostMessage (WMU_SELCHANGE_OPEN);
	}

	BASE_CLASS::OnTimer(nIDEvent);
}

///////////////////////////////////////////////////////////////////////////
// Get the name of the current NG and set the CDocument title.  This
// will set the caption on the MDI window.
//
BOOL CNewsView::sync_caption()
{
	TNewsGroup* pNG = 0;
	LONG id = m_curNewsgroupID;
	CString title;

	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vNewsGroups);

	if (0 == vNewsGroups->GetSize() || (0==id))
	{
		title.Empty ();
	}
	else
	{
		BOOL fUseLock;
		TNewsGroupUseLock useLock(cpNewsServer, id, &fUseLock, pNG);
		if (!fUseLock)
		{
			title.Empty ();
		}
		else
		{
			title = cpNewsServer->GetNewsServerName() + "/" + pNG->GetBestname();
		}
	}

	// Documents have titles too!
	GetDocument()->SetTitle ( title );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// GotoArticle - Jump to a specific group/article pair.
//
//  return 0 if loaded, 1 if started download
//
LRESULT CNewsView::GotoArticle (WPARAM wParam, LPARAM lParam)
{
	CListCtrl & lc = GetListCtrl ();
	TServerCountedPtr cpNewsServer;

	// find the newsgroup in the listbox, set selection, and
	// call OpenNewsgroup

	int iOpenRet = 0;
	TGotoArticle   *pGoto = (TGotoArticle *) lParam;
	BOOL fOpened = FALSE;
	int iRet = -1;

	int count = lc.GetItemCount ();

	for (int i = 0; i < count; i++)
	{
		if (pGoto->m_groupNumber == (int)lc.GetItemData (i))
		{
#if 1
			// the search dialog has logic to force a filter change (via
			//  WMU_FORCE_FILTER_CHANGE
#else
			// set the view filter to the least restrictive one... ???? don't need
			// to restrict filter if article is compatible with current filter set
			gpUIMemory->SetViewFilter (0);
#endif

			// ???? might need to optimize for newsgroup is current one somehow
			//      potentially by passing in the status of the message so
			//      that it can be compared against the current filter

			// if we need to jump to a different NG, or we are about to utilize
			//   the 'All Articles' filter, then Open the newsgroup
			if (m_curNewsgroupID != pGoto->m_groupNumber || pGoto->m_byOpenNG)
			{
				if (pGoto->m_byDownloadNG)
				{
					// this is useful for newsurl support. DL a newly subscribed NG.
					iRet = iOpenRet = OpenNewsgroup ( kFetchOnZero, kCurrentFilter, pGoto->m_groupNumber);
				}
				else
					iRet = iOpenRet = OpenNewsgroup ( kOpenNormal, kCurrentFilter, pGoto->m_groupNumber);
			}

			if (iRet >= 0)
			{
				this->AddOneClickGroupInterference ();
				SetOneSelection (i);
			}
			else
			{
				SetOneSelection (i);
			}
			break;
		}
	}

	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, m_curNewsgroupID, &fUseLock, pNG);

	if (fUseLock)
	{
		switch (UtilGetStorageOption ( pNG ))
		{
		case TNewsGroup::kNothing:
			m_GotoArticle = *pGoto;
			break;

		case TNewsGroup::kHeadersOnly:
		case TNewsGroup::kStoreBodies:
			EmptyBrowsePointers ();
			GetDocument()->UpdateAllViews(this, VIEWHINT_GOTOARTICLE, pGoto);
			break;

		default:
			ASSERT(0);
			break;
		}
	}
	return iOpenRet;
}

/////////////////////////////////////////////////////////////////////
// ProcessMailTo - Send mail to somebody.
/////////////////////////////////////////////////////////////////////
LRESULT CNewsView::ProcessMailTo (WPARAM wParam, LPARAM lParam)
{
	TPostTemplate *pTemplate = gptrApp->GetMailToTemplate ();
	pTemplate->m_iFlags = TPT_CANCEL_WARNING_ID | TPT_MAIL | TPT_TO_STRING;
	pTemplate->m_iCancelWarningID = IDS_WARNING_MESSAGECANCEL;
	pTemplate->m_strTo = (LPCTSTR) lParam;
	pTemplate->Launch (GetCurNewsGroupID());
	return FALSE;
}

// ---------------------------------------------------------------------
// handles WMU_NEWSVIEW_NEWFONT
void CNewsView::ApplyNewFont(void)
{
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors ();

	if (!gpGlobalOptions->GetRegSwitch()->IsDefaultNGBG())
	{
		CListCtrl &lc = GetListCtrl();
		lc.SetBkColor ( pBackgrounds->GetNewsgroupBackground() );
		lc.SetTextBkColor ( pBackgrounds->GetNewsgroupBackground() );
	}

	GetListCtrl().SetTextColor (
		gpGlobalOptions->GetRegFonts()->GetNewsgroupFontColor() );

	LOGFONT lfCurrent;

	// check against current font.
	CFont* pCurCFont = GetFont();
	if (pCurCFont)
		pCurCFont->GetObject (sizeof(LOGFONT), &lfCurrent);
	else
	{
		// we should always have an explicit font set.
		ASSERT(0);
	}

	const LOGFONT* plfFontNew;
	BOOL fCustom = gpGlobalOptions->IsCustomNGFont();
	if (fCustom)
		plfFontNew = gpGlobalOptions->GetNewsgroupFont();
	else
		plfFontNew = &gpGlobalOptions->m_defNGFont;

	// Only if the newfont is different, do we apply it.
	//   9-15-95 this may not work due to non-match on precision,
	//           clipping and fine points like that
	if (memcmp (plfFontNew, &lfCurrent, sizeof(LOGFONT)))
	{
		if (m_font.m_hObject)
		{
			m_font.DeleteObject ();  // i pray this does the detach
			m_font.m_hObject = NULL;
		}

		m_font.CreateFontIndirect ( plfFontNew );

		SetFont (&m_font, TRUE);
		// m_font is cleaned up by CFont destructor.
	}
}

LRESULT CNewsView::OnDisplayArtcount (WPARAM wParam, LPARAM lParam)
{
	int iGroupID = (int) wParam;

	if (iGroupID <= 0)
	{
		// redraw all
		this->Invalidate ();
	}
	else
	{
		// redraw to show the Total number of new articles
		RedrawByGroupID ( iGroupID );
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// The MDI window does the 4 way routing.  If we really do have the focus
//   save the article shown is the view pane.
void CNewsView::OnSaveToFile ()
{
	BOOL fMax;
	TNews3MDIChildWnd *pMDI = (TNews3MDIChildWnd *)
		((CMainFrame*) AfxGetMainWnd ())->MDIGetActive (&fMax);
	TMdiLayout *pLayout = pMDI->GetLayoutWnd ();
	TArticleFormView *pArtView = pLayout->GetArtFormView();
	pArtView->SendMessage (WM_COMMAND, ID_ARTICLE_SAVE_AS);
}

void CNewsView::OnUpdateSaveToFile (CCmdUI* pCmdUI)
{
	BOOL fValid = (GetBrowseHeader () && GetBrowseText ());
	pCmdUI->Enable (fValid);
}

// -------------------------------------------------------------------------
void CNewsView::OnUpdateThreadChangeToRead (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (FALSE);
}

// -------------------------------------------------------------------------
// this should always be disabled when the NewsView pane has focus
void CNewsView::OnThreadChangeToRead ()
{
	// ChangeThreadStatusTo (TStatusUnit::kNew, FALSE);
}

void CNewsView::EmptyBrowsePointers ()
{
	SetBrowseHeader (NULL);
	SetBrowseText (NULL);
}

// -------------------------------------------------------------------------
// a newsgroup's headers are displayed
BOOL CNewsView::IsNewsgroupDisplayed ()
{
	return m_curNewsgroupID ? TRUE : FALSE;
}

// -------------------------------------------------------------------------
// True if exactly one
bool CNewsView::IsExactlyOneNewsgroupSelected ()
{
	return GetListCtrl().GetSelectedCount() == 1;
}

// -------------------------------------------------------------------------
bool CNewsView::IsOneOrMoreNewsgroupSelected ()
{
	return GetListCtrl().GetSelectedCount() > 0;
}

// -------------------------------------------------------------------------
void CNewsView::OnUpdateNewsgroupUnsubscribe (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsOneOrMoreNewsgroupSelected ());
}

// -------------------------------------------------------------------------
//  4-19-96  amc Changed catchup alot. So I can use any selected NG.
void CNewsView::OnUpdateNewsgroupCatchup (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsOneOrMoreNewsgroupSelected ());
}

// -------------------------------------------------------------------------
void CNewsView::OnUpdateNewsgroupProperties (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsExactlyOneNewsgroupSelected ());
}

void CNewsView::OnUpdateDisable(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (FALSE);
}

// -------------------------------------------------------------------------
// this is a real accelerator
void CNewsView::OnCmdTabaround()
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
		PostMessage (WMU_CHILD_TAB, FALSE, 0);
}

// -------------------------------------------------------------------------
// this is a real accelerator
void CNewsView::OnCmdTabBack()
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
		PostMessage (WMU_CHILD_TAB, TRUE, 0);
}

///////////////////////////////////////////////////////////////////////////
// Get articles for all subscribed newsgroups
//
void CNewsView::OnGetheadersAllGroups()
{
	// get headers, (force the retrieve cycle,  fUserAction)
	CNewsDoc::DocGetHeadersAllGroups (true, true);
}

void CNewsView::OnUpdateGetheadersAllGroups(CCmdUI* pCmdUI)
{
	BOOL fEnable = FALSE;

	if (gpTasker)
		fEnable = gpTasker->CanRetrieveCycle ();

	pCmdUI->Enable ( fEnable );
}

///////////////////////////////////////////////////////////////////////////
// Get article headers for one group.  Function does NOT map to a menu-item
//
void CNewsView::GetHeadersOneGroup()
{
	if (0 == gpTasker)
		return;

	BOOL fCatchUp = gpGlobalOptions->GetRegSwitch()->GetCatchUpOnRetrieve();
	int iGroupId = GetSelectedGroupID ();

	if (LB_ERR != iGroupId)
		get_header_worker (iGroupId, fCatchUp, TRUE /* get all */, 0);
}

// ------------------------------------------------------------------------
// also used by ID_VERIFY_HDRS
void CNewsView::OnUpdateGetHeadersMultiGroup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (gpTasker ? IsOneOrMoreNewsgroupSelected () : false);
}

// ------------------------------------------------------------------------
// Request headers for 1 or more groups
void CNewsView::OnGetHeadersMultiGroup ()
{
	// call worker function
	get_header_multigroup_worker (TRUE /* fGetAllHdrs */, 0);
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateGetheaderLimited(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	OnUpdateGetHeadersMultiGroup( pCmdUI);
}

// ------------------------------------------------------------------------
// Get 'X' headers
void CNewsView::OnGetheaderLimited()
{
	CPromptLimitHeaders sDlg (AfxGetMainWnd());

	if (IDOK == sDlg.DoModal())
	{
		get_header_multigroup_worker (FALSE, sDlg.m_iCount);
	}
}

// ------------------------------------------------------------------------
int CNewsView::get_header_multigroup_worker (BOOL fGetAll, int iHdrsLimit)
{
	if (0 == gpTasker)
		return 1;

	BOOL fCatchUp = gpGlobalOptions->GetRegSwitch()->GetCatchUpOnRetrieve();
	CPtrArray vec;
	int iSelCount = 0;

	// get info on multi selection
	if (multisel_get (&vec, &iSelCount))
		return 1;

	for (int i = 0; i < vec.GetSize(); i++)
	{
		TGroupIDPair * pPair = static_cast<TGroupIDPair*>(vec[i]);

		get_header_worker (pPair->id, fCatchUp, fGetAll, iHdrsLimit);
	}

	FreeGroupIDPairs (&vec);
	return 0;
}

// ------------------------------------------------------------------------
//
int CNewsView::get_header_worker (int iGroupId,
								  BOOL fCatchUp,
								  BOOL fGetAll,
								  int  iHdrsLimit)
{
	if (LB_ERR == iGroupId)
		return 1;

	BOOL fConnected =  gpTasker->IsConnected();
	if (!fConnected)
		return 1;

	TServerCountedPtr cpNewsServer;

	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, iGroupId, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	if (fCatchUp)
	{
		CatchUp_Helper ( iGroupId, FALSE );
	}

	// before downloading headers Purge via Date criteria

	// Master Plan:
	//   purge
	//   get headers
	//   refresh ui.
	// If we are't connected then don't purge either  [2-12-97 amc]

	if (fConnected && pNG->NeedsPurge())
		pNG->PurgeByDate();

	gpTasker->PrioritizeNewsgroup ( pNG->GetName(), fGetAll, iHdrsLimit );

	return 0;
}

// ------------------------------------------------------------------------
// OnVerifyLocalHeaders -- see if all local headers still exist on the
//    server.  If any have been expired, remove them from the display (reload)
void CNewsView::OnVerifyLocalHeaders ()
{
	try
	{
		CPtrArray vec;
		int iSelCount = 0;

		// get info on multi selection
		if (multisel_get (&vec, &iSelCount))
			return;

		TServerCountedPtr cpNewsServer;

		for (int i = 0; i < vec.GetSize(); i++)
		{
			TGroupIDPair * pPair = static_cast<TGroupIDPair*>(vec[i]);
			BOOL fUseLock;
			TNewsGroup* pNG = 0;
			TNewsGroupUseLock useLock(cpNewsServer, pPair->id, &fUseLock, pNG);
			if (fUseLock && TNewsGroup::kNothing != UtilGetStorageOption (pNG))
				gpTasker->VerifyLocalHeaders (pNG);
		}

		FreeGroupIDPairs (&vec);
	}
	catch(...) { /* trap all errors */ }
}

void CNewsView::OnDisplayAllArticleCounts(void)
{
	// for listctrl redraw it all
	Invalidate ();
}

///////////////////////////////////////////////////////////////////////////
// Created 3-24-96
//   Once all the headers have been saved - re-open the newsgroup.
//    1 - this saves the user the effort of opening the NG himself
//    2 - after purging articles (during hdr retrieve) we sorta haveto
//        repaint, (some articles showing in the LBX may be gone)
//
LRESULT CNewsView::OnNewsgroupHdrsDone(WPARAM wParam, LPARAM lParam)
{
	TServerCountedPtr cpNewsServer;

	LONG grpID = (LONG) wParam;
	BOOL fUserAction = (BOOL) lParam;

	if (grpID == m_curNewsgroupID)
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, grpID, &fUseLock, pNG);
		if (fUseLock)
		{
			if (fUserAction)
				OpenNewsgroup (kOpenNormal, kPreferredFilter, grpID);
		}
	}
	return 0;
}

//-------------------------------------------------------------------------
//
LRESULT CNewsView::OnMode1HdrsDone(WPARAM wParam, LPARAM lParam)
{
	LONG grpID = (LONG) wParam;
	BOOL fOld  =  (lParam & 0x1) ? TRUE : FALSE;
	BOOL fNew  =  (lParam & 0x2) ? TRUE : FALSE;

	TServerCountedPtr cpNewsServer;

	if (grpID == m_curNewsgroupID)
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, grpID, &fUseLock, pNG);
		if (fUseLock)
		{
			if (TNewsGroup::kNothing == UtilGetStorageOption(pNG))
			{
				// mode 1 - Don't call OpenNewsgroup this would be an infinite loop

				pNG->Mode1_Thread (fOld, fNew);
				please_update_views ();

				// 'GOTO' article from Search Window
				if (m_GotoArticle.m_articleNumber >= 0)
				{
					EmptyBrowsePointers ();
					GetDocument()->UpdateAllViews(this, VIEWHINT_GOTOARTICLE,
						&m_GotoArticle);
					m_GotoArticle.m_articleNumber = -1;
				}
				return 0;
			}
		}
	}
	return 0;
}

// There are no objects in the newsview you can copy
void CNewsView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( FALSE );
}

// ------------------------------------------------------------------------
// subthread forces UI to show a message box
LRESULT CNewsView::OnErrorFromServer (WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam);
	if (wParam)
	{
		PTYP_ERROR_FROM_SERVER psErr = (PTYP_ERROR_FROM_SERVER)(void*)(wParam);

		// Response from Server: %s
		CString strReason;
		strReason.Format (IDS_UTIL_SERVRESP, psErr->iRet, LPCTSTR(psErr->serverString));

		TNNTPErrorDialog sDlg(this);

		sDlg.m_strWhen   = psErr->actionDesc;
		sDlg.m_strReason = strReason;

		// show fairly polished dlg box
		sDlg.DoModal ();

		delete psErr;
	}
	return TRUE;
}

void CNewsView::protected_open(TNewsGroup* pNG, BOOL fOpen)
{
	TEnterCSection enter(&m_dbCritSect);

	int iCount = 10;
	if (fOpen)
	{
		TUserDisplay_UIPane sAutoDraw("Open group");

		pNG->Open ();
		++ iLocalOpen;
	}
	else
	{
		TUserDisplay_UIPane sAutoDraw("Close group");

		ASSERT(iLocalOpen > 0);
		if (iLocalOpen > 0)
		{
			pNG->Close ();
			-- iLocalOpen;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Called from OnLbxSelChange(void)
LRESULT CNewsView::OnSelChangeOpen(WPARAM wParam, LPARAM lParam)
{
	//TRACE0("Start from SelChange Open\n");
	OpenNewsgroup( kFetchOnZero, kPreferredFilter );
	//TRACE0("Returning from SelChange Open\n");
	return 0;
}

//-------------------------------------------------------------------------
// return TRUE to continue, FALSE to abort
BOOL CNewsView::check_server_posting_allowed()
{
	TServerCountedPtr cpNewsServer;
	if (cpNewsServer->GetPostingAllowed())
		return TRUE;

	CString msg;
	AfxFormatString1(msg, IDS_WARN_SERVER_NOPOSTALLOWED,
		(LPCTSTR) cpNewsServer->GetNewsServerName());

	// are you sure you want to continue?
	return (IDYES == NewsMessageBox(this, msg, MB_YESNO | MB_ICONQUESTION));
}

//-------------------------------------------------------------------------
// validate_connection -- Suppose the normal pump is busy downloading a
//     binary.  As the user switches to a mode-1 NG, the emergency-pump
//     should be used, since the n-pump is busy.  Start it up and it should
//     be running when the ArticleBank needs it.  It would be GROSS to
//     move the connection code to when the ArticleBank calls
//     gpTasker->GetRangeFor()
BOOL CNewsView::validate_connection ()
{
	TServerCountedPtr cpNewsServer;

	if (FALSE == gpTasker->IsConnected())
		return TRUE;
	else
	{
		// ok we are connected
		if (FALSE == gpTasker->NormalPumpBusy())
			return TRUE;

		int  iTry;
		BOOL fConnected;
		BOOL fContinueConnect = FALSE;
		fConnected = gpTasker->SecondIsConnected ( &fContinueConnect );
		if (fConnected)
			return TRUE;
		else
		{
			// since the norm-pump is running, we should have permission to
			//  start the e-pump
			if (!fContinueConnect)
				return FALSE;

			iTry = gpTasker->SecondConnect ( cpNewsServer->GetNewsServerAddress() );
			if (0 != iTry)
				return FALSE;
			return TRUE;
		}
	}
}

// ------------------------------------------------------------------------
// used for CatchUp and Move to Next newsgroup.  Returns grpid.
BOOL CNewsView::validate_next_newsgroup (int * pGrpId, int * pIdx)
{
	TServerCountedPtr cpNewsServer;
	CListCtrl & lc =  GetListCtrl ();

	int iNext = -1;
	int tot = lc.GetItemCount();
	for (int i = 0; i < tot; ++i)
	{
		// hunt for a newsgroup we can open
		if (0 == iNext)
		{
			int iGrpID = (int) lc.GetItemData(i);

			BOOL fUseLock;
			TNewsGroup* pNG = 0;
			TNewsGroupUseLock useLock(cpNewsServer, iGrpID, &fUseLock, pNG);
			if (fUseLock)
			{
				if (TNewsGroup::kNothing == UtilGetStorageOption (pNG))
				{
					// if we are not connected keep going
					if (gpTasker->IsConnected())
					{
						*pGrpId = iGrpID;
						*pIdx = i;
						return TRUE;
					}
				}
				else
				{
					*pGrpId = iGrpID;
					*pIdx = i;
					return TRUE;
				}
			}

		}

		// find our current guy
		if (m_curNewsgroupID == (int) lc.GetItemData(i))
			iNext = 0;
	}
	return FALSE;
}

// ------------------------------------------------------------------------
// A real public func
void CNewsView::OpenNewsgroupEx(CNewsView::EOpenMode eMode)
{
	// open the selected one!
	OpenNewsgroup ( eMode, kPreferredFilter );
}

// ------------------------------------------------------------------------
int CNewsView::NumSelected ()
{
	return GetListCtrl().GetSelectedCount();
}

// ------------------------------------------------------------------------
// GetSelectedIDs -- takes a pointer to an integer array, and returns the
// group IDs for the selected groups
void CNewsView::GetSelectedIDs (int *piIDs, int iSize)
{
	// first, fill dest array with zeros
	int i = 0;
	for (i = 0; i < iSize; i++)
		piIDs [i] = 0;

	int iNumSelected = NumSelected ();
	if (iNumSelected <= 0)
		return;

	CListCtrl & lc = GetListCtrl();
	int * piIndices = new int [iNumSelected];
	auto_ptr <int> pIndicesDeleter(piIndices);

	int n = 0;
	POSITION pos = lc.GetFirstSelectedItemPosition ();
	while (pos)
		piIndices[n++] = lc.GetNextSelectedItem (pos);

	for (i = 0; i < iNumSelected && i < iSize; i++)
		piIDs [i] = lc.GetItemData (piIndices [i]);
}

// ------------------------------------------------------------------------
// OnUpdateKeepSampled -- enabled if any selected group is sampled
void CNewsView::OnUpdateKeepSampled(CCmdUI* pCmdUI)
{
	TServerCountedPtr cpNewsServer;
	BOOL bEnable = FALSE;

	CPtrArray sPairs; // holds a collection of (groupID, name) pairs
	int iNum = 0;
	if (multisel_get (&sPairs, &iNum)) {
		pCmdUI->Enable (FALSE);
		return;
	}

	iNum = sPairs.GetSize();
	for (int i = 0; i < iNum; i++) {

		// get pNG
		TGroupIDPair *pPair = static_cast <TGroupIDPair*> (sPairs [i]);
		TNewsGroup *pNG;
		BOOL fUseLock;
		TNewsGroupUseLock useLock (cpNewsServer, pPair->id, &fUseLock, pNG);
		if (!fUseLock)
			break;

		// check it
		if (pNG->IsSampled ()) {
			bEnable = TRUE;
			break;
		}
	}

	FreeGroupIDPairs (&sPairs);
	pCmdUI->Enable (bEnable);
}

// ------------------------------------------------------------------------
void CNewsView::OnKeepSampled()
{
	TServerCountedPtr cpNewsServer;
	CPtrArray sPairs; // holds a collection of (groupID, name) pairs
	int iNum = 0;
	if (multisel_get (&sPairs, &iNum))
		return;

	iNum = sPairs.GetSize();
	for (int i = 0; i < iNum; i++) {

		// get pNG
		TGroupIDPair *pPair = static_cast <TGroupIDPair*> (sPairs [i]);
		TNewsGroup *pNG;
		BOOL fUseLock;
		TNewsGroupUseLock useLock (cpNewsServer, pPair->id, &fUseLock, pNG);
		if (!fUseLock)
			break;

		// keep the group
		pNG->Sample (FALSE);
	}

	FreeGroupIDPairs (&sPairs);

	// redraw all
	Invalidate ();
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateKeepAllSampled(CCmdUI* pCmdUI)
{
	TServerCountedPtr cpNewsServer;
	BOOL bEnable = FALSE;
	TNewsGroupArray &vNewsGroups = cpNewsServer->GetSubscribedArray ();
	int iNum = vNewsGroups->GetSize ();
	for (int i = 0; i < iNum; i++) {
		TNewsGroup *pNG = vNewsGroups [i];
		if (pNG->IsSampled ())
			bEnable = TRUE;
	}
	pCmdUI->Enable (bEnable);
}

// ------------------------------------------------------------------------
void CNewsView::OnKeepAllSampled()
{
	TServerCountedPtr cpNewsServer;
	TNewsGroupArray &vNewsGroups = cpNewsServer->GetSubscribedArray ();
	int iNum = vNewsGroups->GetSize ();
	for (int i = 0; i < iNum; i++) {
		TNewsGroup *pNG = vNewsGroups [i];
		pNG->Sample (FALSE);
	}

	// redraw all
	Invalidate ();
}

// ------------------------------------------------------------------------
void CNewsView::OnEditSelectAll()
{
	GetListCtrl().SetItemState (-1, LVIS_SELECTED, LVIS_SELECTED);
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (GetListCtrl().GetItemCount () > 0);
}

// ------------------------------------------------------------------------
void CNewsView::EmptyListbox ()
{
	GetListCtrl().DeleteAllItems ();
}

// ------------------------------------------------------------------------
void CNewsView::OnUpdateDeleteSelected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsOneOrMoreNewsgroupSelected ());
}

// ---------------------------------------------------
void CNewsView::OnHelpResyncStati()
{
	//TRACE0("Resyncing...\n");

	TServerCountedPtr cpNewsServer;
	LONG nGID = GetCurNewsGroupID();

	BOOL fUseLock;
	TNewsGroup* pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, nGID, &fUseLock, pNG);
	if (!fUseLock)
		return;

	{
		TAutoClose sCloser(pNG);
		CWaitCursor cursor;

		pNG->ValidateStati ();
	}
	// redraw all
	Invalidate ();
}

// Pindown view filter
void CNewsView::OnNewsgroupPinfilter()
{
	m_fPinFilter = !m_fPinFilter;

	// save immediately
	gpGlobalOptions->GetRegUI()->SetPinFilter( m_fPinFilter );
	gpGlobalOptions->GetRegUI()->Save ();

	((CMainFrame*)AfxGetMainWnd())->UpdatePinFilter ( m_fPinFilter );
}

void CNewsView::OnUpdateNewsgroupPinfilter(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck ( m_fPinFilter );
}

// -------------------------------------------------------------------------
// Returns 0 for success.  On success return the group-id we found.
//
int CNewsView::GetNextGroup (EJmpQuery eQuery, int * pGroupID)
{
	CListCtrl & lc = GetListCtrl ();

	// depends on the 'sort' order, so we have to ask the UI. ick!

	int curGID = GetCurNewsGroupID();
	bool fFound = false;
	int nextI = 0;

	for (int i = 0; i < lc.GetItemCount(); i++)
	{
		if (lc.GetItemData (i) == curGID)
		{
			nextI = i + 1;
			break;
		}
	}

	if (nextI >= lc.GetItemCount() || 0==nextI)
		return 1;

	TServerCountedPtr cpNewsServer;        // smart pointer

	for (; nextI < lc.GetItemCount(); nextI++)
	{
		int gid = lc.GetItemData (nextI);

		BOOL fUseLock;
		TNewsGroup * pNG = 0;

		TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);

		if (pNG)
		{
			if (pNG->QueryExistArticle (eQuery))
			{
				*pGroupID = gid;
				return 0;
			}
		}
	}

	return 1;  // no matching group found
}

BOOL CNewsView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style &= ~(LVS_ICON | LVS_SMALLICON | LVS_LIST);
	cs.style |= LVS_REPORT | LVS_NOSORTHEADER | LVS_SORTASCENDING
		| LVS_SHOWSELALWAYS;

	return BASE_CLASS::PreCreateWindow(cs);
}

// ------------------------------------------------------------------------
int CNewsView::AddStringWithData (const CString & groupName,
								  LONG            lGroupID,
								  int *           pIdxAt /* =NULL */)
{
	CListCtrl & lc = GetListCtrl();

	int    idx;
	LVITEM lvi;  ZeroMemory (&lvi, sizeof(lvi));

	lvi.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem    = lc.GetItemCount();
	lvi.iSubItem = 0;
	lvi.iImage   = I_IMAGECALLBACK;
	lvi.pszText  = LPTSTR(LPCTSTR(groupName));
	//   lvi.state    |= INDEXTOOVERLAYMASK(1);
	//   lvi.stateMask = LVIS_OVERLAYMASK;
	lvi.lParam   = lGroupID;

	if (-1 == (idx = lc.InsertItem ( &lvi)))
		return 1; //fail

	// local count, server count
	lc.SetItemText (idx, 1, LPSTR_TEXTCALLBACK);
	lc.SetItemText (idx, 2, LPSTR_TEXTCALLBACK);

	if (pIdxAt)
		*pIdxAt = idx;
	return 0;   // success
}

// ------------------------------------------------------------------------
int CNewsView::SetOneSelection (int idx)
{
	CListCtrl & lc = GetListCtrl();

	// note:  in case the SELCHANGE notification is hooked, do not
	//      - turn off selection on all
	//      - turn on  selection on the index we want.  (It may cause a
	//          one-click group to re-open)

	for (int i = 0; i < lc.GetItemCount(); i++)
	{
		if (i == idx)
		{
			// select this guy
			lc.SetItemState (idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
		else
		{
			// deselect all
			lc.SetItemState (i, 0, LVIS_SELECTED | LVIS_FOCUSED);
		}
	}

	// this useful thing is provided by MFC
	lc.EnsureVisible (idx, TRUE /* fPartialOK */);

	// success
	return 0;
}

// ---------------------------------------------------------------------
// called from OnCreate
void CNewsView::SetupFont ()
{
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors ();

	COLORREF ngCR = pBackgrounds->GetNewsgroupBackground();

	if (!gpGlobalOptions->GetRegSwitch()->IsDefaultNGBG())
	{
		CListCtrl &lc = GetListCtrl();
		lc.SetBkColor ( pBackgrounds->GetNewsgroupBackground() );
		lc.SetTextBkColor ( pBackgrounds->GetNewsgroupBackground() );
	}

	ngCR = gpGlobalOptions->GetRegFonts()->GetNewsgroupFontColor();
	GetListCtrl().SetTextColor (ngCR);

	if (0 == gpGlobalOptions->m_defNGFont.lfFaceName[0])
	{
		// show this info in the config dialog box
		gpVariableFont->GetObject ( sizeof (LOGFONT), &gpGlobalOptions->m_defNGFont );
	}

	if (m_font.m_hObject)
	{
		m_font.DeleteObject ();  // i pray this does the detach
		m_font.m_hObject = NULL;
	}
	if (gpGlobalOptions->IsCustomNGFont())
		m_font.CreateFontIndirect ( gpGlobalOptions->GetNewsgroupFont() );
	else
	{
		// default to microplanet defined small font
		LOGFONT info;
		gpVariableFont->GetObject ( sizeof (info), &info );
		m_font.CreateFontIndirect ( &info );
	}

	SetFont ( &m_font );
}

// --------------------------------------------------------------------------
void CNewsView::OnGetDisplayInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO * pDispInfo = (LV_DISPINFO*)pNMHDR;

	if (!IsActiveServer())
	{
		*pResult = 0;
		return;
	}

	LVITEM & lvi = pDispInfo->item;

	TServerCountedPtr cpNewsServer;

	TNewsGroup * pNG = 0;
	BOOL         fUseLock;
	LONG         gid = LONG(GetListCtrl().GetItemData (lvi.iItem));
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);

	if (fUseLock)
	{
		if (lvi.mask & LVIF_IMAGE)
		{
			switch (pNG->GetStorageOption())
			{
			case TNewsGroup::kNothing:
				lvi.iImage = 0;
				break;
			case TNewsGroup::kHeadersOnly:
				lvi.iImage = 1;
				break;
			case TNewsGroup::kStoreBodies:
				lvi.iImage = 2;
				break;
			}

			if (pNG->IsSampled ())
				lvi.iImage += 3;
		}

		if (lvi.mask & LVIF_STATE)
		{
			if (!pNG->IsActiveGroup())
				lvi.state |= INDEXTOOVERLAYMASK(1);
		}

		if (lvi.mask & LVIF_TEXT)
		{
			int iLocalNew, iServerNew, nTotal;
			pNG->FormatNewArticles ( iLocalNew, iServerNew , nTotal);
			if (1 == lvi.iSubItem)
				wsprintf (lvi.pszText, "%d", iLocalNew);
			else if (2 == lvi.iSubItem)
				wsprintf (lvi.pszText, "%d", iServerNew);

			// data eventuall is displayed in status bar pane
			if (GetCurNewsGroupID() == gid)
				gpUserDisplay->SetCountTotal ( nTotal );

		}
	}
	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////
//  Depends on user preference - single click/selchange opens a newsgroup
//
void CNewsView::OnClick (NMHDR* pNMHDR, LRESULT* pResult)
{
	//TRACE0("On Click\n");
	if (gpGlobalOptions->GetRegUI()->GetOneClickGroup() &&
		(1 == GetListCtrl().GetSelectedCount()))
	{

		// turn off delayed activation. Actually this is futile, since
		//  NM_ITEMCHANGE will start the timer again
		stop_selchange_timer();

		AddOneClickGroupInterference();
		PostMessage (WMU_SELCHANGE_OPEN);
	}

	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////
void CNewsView::AddOneClickGroupInterference ()
{
	if (gpGlobalOptions->GetRegUI()->GetOneClickGroup())
	{
		m_iOneClickInterference ++;
	}
}

///////////////////////////////////////////////////////////////////////////
// Substitute for LBN_SELCHANGE,
//
void CNewsView::OnItemChanged (NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 1;

	LPNMLISTVIEW pnmlv = (LPNMLISTVIEW) pNMHDR;
	BOOL  fNewSelect = (pnmlv->uNewState & LVIS_SELECTED);
	BOOL  fOldSelect =  (pnmlv->uOldState & LVIS_SELECTED);

	if (  fNewSelect && !fOldSelect )
	{
		//TRACE0("OnItemChanged\n");
		if (gpGlobalOptions->GetRegUI()->GetOneClickGroup() &&
			(1 == GetListCtrl().GetSelectedCount()))
		{

			if (m_iOneClickInterference > 0)
			{
				*pResult = 0;

				// do not start timer!
				--m_iOneClickInterference;
			}
			else
			{
				// this will pop unless:
				//    a:  another selchange comes
				//    b:  this turns out to be a right click
				//
				start_selchange_timer ();

				//  OnTimer will call
				//   PostMessage (WMU_SELCHANGE_OPEN);

				// 5-25-96 Rather than calling OpenNewsgroup() directly,
				//  post a message to myself.  That way the processing
				//  can finish before we start the big fat OpenNewsgroup function

				*pResult = 0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CNewsView::OnReturn(NMHDR* pNMHDR, LRESULT* pResult)
{
	// We got an VK_RETURN, so open the newsgroup

	OpenNewsgroup ( kFetchOnZero, kPreferredFilter );

	*pResult = 0;
}

// ------------------------------------------------------------------------
void CNewsView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// seems to set selection on Right click

	// this is so we can right click on  a newsgroup, an put selection on it
	//  with out actually opening the newsgroup via 1-click
	stop_selchange_timer ();
	//TRACE("Rclik stopped selchange action\n");

	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////
//  Depends on user preference - double click opens a newsgroup
//
void CNewsView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (FALSE == gpGlobalOptions->GetRegUI()->GetOneClickGroup())
		OpenNewsgroup( kFetchOnZero, kPreferredFilter );

	*pResult = 0;
}

// ------------------------------------------------------------------------
void CNewsView::RedrawByGroupID (int iGroupID)
{
	CListCtrl & lc = GetListCtrl();
	RECT        rct;

	int n = lc.GetItemCount();
	for (--n; n >= 0; --n)
	{
		if (iGroupID == (int) lc.GetItemData (n))
		{
			lc.GetItemRect (n, &rct, LVIR_BOUNDS);

			// we only need to redraw the counters
			InvalidateRect (&rct);
			break;
		}
	}
}

// ------------------------------------------------------------------------
void CNewsView::SaveColumnSettings()
{
	SaveColumnSettingsTo (m_fTrackZoom ? "NGPaneZ" : "NGPane");
}

// ------------------------------------------------------------------------
void CNewsView::SaveColumnSettingsTo (const CString & strLabel)
{
	TRegUI* pRegUI = gpGlobalOptions->GetRegUI();
	int riWidths[3];

	riWidths[0] = GetListCtrl().GetColumnWidth (0);
	riWidths[1] = GetListCtrl().GetColumnWidth (1);
	riWidths[2] = GetListCtrl().GetColumnWidth (2);
	pRegUI->SaveUtilHeaders(strLabel, riWidths, ELEM(riWidths));
}

// ------------------------------------------------------------------------
// tnews3md.cpp does the real work. Each active view must have a
// handler for this
//
void CNewsView::OnUpdateArticleMore (CCmdUI* pCmdUI)
{
	CWnd* pMdiChild = ((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame();
	BOOL fOK = ((TNews3MDIChildWnd*) pMdiChild)->CanArticleMore();
	pCmdUI->Enable (fOK);
}

// ------------------------------------------------------------------------
BOOL CNewsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// let the mdi-child decide which pane will handle this
	CWnd* pMdiChild = ((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame();
	if (!pMdiChild)
		return FALSE;

	if (((TNews3MDIChildWnd*) pMdiChild)->handleMouseWheel(nFlags, zDelta, pt))
		return TRUE;
	else
		return handleMouseWheel (nFlags, zDelta, pt);
}

// ------------------------------------------------------------------------
// called from MDI child
BOOL CNewsView::handleMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return BASE_CLASS::OnMouseWheel (nFlags, zDelta, pt);
}

// ------------------------------------------------------------------------
// zoom mode has its own set of column widths
void CNewsView::handle_zoom (bool fZoom, CObject* pHint)
{
	int riWidths[3];
	bool fApply = false;
	TRegUI* pRegUI = gpGlobalOptions->GetRegUI();

	if (fZoom)
	{
		if (pHint == this)
		{
			// The roles are the same, but the widths can be different

			// save off normal
			SaveColumnSettings ();

			if (0 != pRegUI->LoadUtilHeaders("NGPaneZ", riWidths, ELEM(riWidths)))
				return;

			fApply = true;

			m_fTrackZoom = true;  // passive variable, as a convenienence
		}
	}
	else
	{
		TUnZoomHintInfo * pUnZoom = static_cast<TUnZoomHintInfo *>(pHint);

		if (pUnZoom->m_pTravelView == this)
		{
			m_fTrackZoom = false;

			// save zoomed
			SaveColumnSettingsTo ("NGPaneZ");

			// load Normal
			if (0 != pRegUI->LoadUtilHeaders("NGPane", riWidths, ELEM(riWidths)))
				return;

			fApply = true;
		}
	}
	if (fApply)
	{
		GetListCtrl().SetColumnWidth (0, riWidths[0]);
		GetListCtrl().SetColumnWidth (1, riWidths[1]);
		GetListCtrl().SetColumnWidth (2, riWidths[2]);
	}
}

// pCmdUI for ID_GETTAGGED_FOR_GROUPS
void CNewsView::OnUpdateGettaggedForgroups(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsOneOrMoreNewsgroupSelected ());
}

// ----------------------------------------------------------
// Retrieve Tagged articles only for the selected groups
void CNewsView::OnGettaggedForgroups()
{
	// holds a collection of  (groupID, name) pairs
	CPtrArray vecPairs;
	int sel = 0;

	// get info on multisel
	if (multisel_get (&vecPairs, &sel))
		return;

	int tot = vecPairs.GetSize();

	// build up CDWordArray that only contains GroupIDs
	CDWordArray vecIDs;
	for (int i = 0; i < tot; i++)
	{
		TGroupIDPair* pPair = static_cast<TGroupIDPair*>(vecPairs[i]);
		vecIDs.Add (pPair->id);
	}

	FreeGroupIDPairs (&vecPairs);

	TServerCountedPtr cpServer;

	cpServer->GetPersistentTags().RetrieveTagged (false, vecIDs);
}

// -------------------------------------------------------------------------
BOOL CNewsView::PreTranslateMessage(MSG* pMsg)
{
	if (::IsWindow (m_sToolTips.m_hWnd) && pMsg->hwnd == m_hWnd)
	{
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			m_sToolTips.RelayEvent (pMsg);
			break;
		}
	}

	return BASE_CLASS::PreTranslateMessage(pMsg);
}

// -------------------------------------------------------------------------
void CNewsView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!(nFlags & MK_LBUTTON) || 0 == GetListCtrl().GetSelectedCount ())
	{
		HandleToolTips (point);
		BASE_CLASS::OnMouseMove (nFlags, point);
		return;
	}

	BASE_CLASS::OnMouseMove(nFlags, point);
}

// -------------------------------------------------------------------------
BOOL CNewsView::OnNeedText (UINT, NMHDR *pNMHdr, LRESULT *)
{
	// make sure the cursor is in the client area, because the mainframe also
	// wants these messages to provide tooltips for the toolbar
	CPoint sCursorPoint;
	VERIFY (::GetCursorPos (&sCursorPoint));
	ScreenToClient (&sCursorPoint);

	CRect sClientRect;
	GetClientRect (&sClientRect);

	if (sClientRect.PtInRect (sCursorPoint))
	{
		TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *) pNMHdr;

		pTTT->lpszText = (LPTSTR)(LPCTSTR) m_strToolTip;

		return TRUE;
	}
	return FALSE;
}

// -------------------------------------------------------------------------
void CNewsView::HandleToolTips (CPoint &point)
{
	static bool fDisplayed = false;
	static int  iDisplayedIndex;

	if (!::IsWindow (m_sToolTips.m_hWnd))
		return;

	// get index of item under mouse

	int iIndex = GetListCtrl().HitTest ( point );

	if (fDisplayed)
	{
		if (iIndex != iDisplayedIndex)
		{
			m_sToolTips.Activate (FALSE);
			m_strToolTip.Empty();
			fDisplayed = false;
		}
	}

	if (!fDisplayed && iIndex >= 0)
	{
		try
		{
			TServerCountedPtr cpNewsServer;

			TNewsGroup * pNG = 0;
			BOOL         fUseLock;
			LONG         gid = LONG(GetListCtrl().GetItemData (iIndex));

			TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);

			if (fUseLock)
			{
				int nNewHdrs, nServer, nTotalHdrs;
				pNG->FormatNewArticles ( nNewHdrs, nServer, nTotalHdrs );
				m_strToolTip.Format ("%s\nLocal Unread:  %d\nLocal Total:  %d\nServer:  %d",
					GetListCtrl().GetItemText (iIndex, 0),
					nNewHdrs, nTotalHdrs, nServer );
			}
			else
				m_strToolTip = GetListCtrl().GetItemText (iIndex, 0);
		}
		catch(...)
		{
			m_strToolTip.Empty();
		}

		m_sToolTips.Activate (TRUE);

		iDisplayedIndex = iIndex;
		fDisplayed = true;
	}
}

// -------------------------------------------------------------------------
void CNewsView::start_selchange_timer ()
{
	stop_selchange_timer ();

	m_hSelchangeTimer = SetTimer (CNewsView::idSelchangeTimer, 333, NULL);
}

// -------------------------------------------------------------------------
void CNewsView::stop_selchange_timer ()
{
	// kill any timer currently running
	if (m_hSelchangeTimer)
		KillTimer (m_hSelchangeTimer);

	m_hSelchangeTimer = 0;
}

// -------------------------------------------------------------------------
int CNewsView::GetPreferredFilter (TNewsGroup * pNG)
{
	int iWhatFilter = pNG->GetFilterID();
	if (0 == iWhatFilter)
	{
		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
		iWhatFilter = pAllFilters->GetGlobalDefFilterID();
	}

	return iWhatFilter;
}
