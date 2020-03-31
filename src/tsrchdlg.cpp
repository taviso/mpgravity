/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsrchdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:19  richard_wood
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

// tsrchdlg.cpp : implementation file
//

#include "stdafx.h"
#include <stdlib.h>
#include "news.h"
#include "tsrchdlg.h"
#include "custmsg.h"
#include "globals.h"
#include "names.h"
#include "article.h"             // TArticleHeader
#include "tsearch.h"
#include "article.h"
#include "tglobopt.h"
#include "arttext.h"
#include "rtfspt.h"
#include "utilsize.h"
#include "newsview.h"
#include "nglist.h"
#include "gotoart.h"
#include "tnews3md.h"
#include "artdisp.h"
#include "fetchart.h"            // PUMP_EXCEPTION error code
#include "ngutil.h"
#include "fileutil.h"
#include "topexcp.h"             // TopLevelException()
#include "genutil.h"             // FetchBody()
#include "rgui.h"                // SaveUtilDlg(), ...
#include "rgswit.h"              // GetSearchGotoLoad ()
#include "idxlst.h"              // TArticleIndexList
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "nglist.h"              // TNewsGroupUseLock
#include "timeutil.h"
#include "srchlvw.h"
#include "vfilter.h"             // TViewFilter

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// static data member to hold window class name
LPCTSTR TSearchDialog::s_winClassName = NULL;

// static data member to hold last search text
CString TSearchDialog::s_strMRUText;

static UINT BASED_CODE search_indicators[] =
{
	ID_SEPARATOR,           // status line indicator
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TSearchDialog dialog

TSearchDialog::TSearchDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TSearchDialog::IDD, pParent)
{
	m_currGroup = _T("");
	m_currGroupID=0;
	m_stopEvent=0;
	m_workerThread=0;
	m_hwndRTF=0;

	m_fSearchHeaders = FALSE;
	m_fSearchBodies = FALSE;
	m_searchFor = _T("");
	m_fSearchThisNewsgroup = -1;
	m_fLocalOnly = FALSE;
	m_fRE = FALSE;

	m_stopEvent = 0;
	m_pResultFrame = 0;

	// due to server switching, we no longer use g-p-NewsServer. We got
	//  our copy.  See OnInitDialog
	m_pNewsServer = 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TAB1, m_myTabs);
	DDX_Check(pDX, IDC_SEARCH_INCLUDEHEADERS, m_fSearchHeaders);
	DDX_Check(pDX, IDC_SEARCH_INCLUDEBODIES, m_fSearchBodies);
	DDX_Text(pDX, IDC_SEARCH_SEARCHFOR, m_searchFor);
	DDX_Radio(pDX, IDC_SEARCH_THIS_NEWSGROUP, m_fSearchThisNewsgroup);
	DDX_Check(pDX, IDC_SEARCH_LOCAL_ONLY, m_fLocalOnly);
	DDX_Check(pDX, IDC_SEARCH_RE, m_fRE);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(TSearchDialog, CDialog)
	ON_BN_CLICKED(IDC_SEARCH_SEARCHNOW, OnSearchSearchNow)
	ON_BN_CLICKED(IDC_SEARCH_STOP, OnSearchStop)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_SEARCH_RESULTS, OnDblclkSearchResults)
	ON_NOTIFY(NM_CLICK, IDC_SEARCH_RESULTS, OnClkSearchResults)
	ON_BN_CLICKED(IDC_SEARCH_JUMP, OnSearchJump)
	ON_NOTIFY(LVN_DELETEITEM, IDC_SEARCH_RESULTS, OnDeleteitemSearchResults)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTabSelchange)
	ON_BN_CLICKED(IDC_SEARCH_GOTOSEL, OnSearchOptionsGoto)
	ON_MESSAGE (WMU_SEARCH_THREADTALK, OnThreadTalk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TSearchDialog message handlers
BOOL TSearchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CButton *pButton = (CButton *) GetDlgItem (IDC_SEARCH_FROM);

	pButton->SetCheck (1);
	pButton = (CButton *) GetDlgItem (IDC_SEARCH_SUBJECT);
	pButton->SetCheck (1);
	pButton = (CButton *) GetDlgItem (IDC_SEARCH_INCLUDEHEADERS);
	pButton->SetCheck (0);
	pButton = (CButton *) GetDlgItem (IDC_SEARCH_INCLUDEBODIES);
	pButton->SetCheck (0);
	pButton = (CButton *) GetDlgItem (IDC_SEARCH_THIS_NEWSGROUP);
	pButton->SetCheck (1);
	pButton = (CButton *) GetDlgItem (IDC_SEARCH_STOP);
	pButton->EnableWindow(FALSE);

	// Setup the tab control

	TC_ITEM sTabCtrlItem;
	sTabCtrlItem.mask = TCIF_TEXT;
	CString str; str.LoadString (IDS_DEFINE_SEARCH);
	sTabCtrlItem.pszText = (LPTSTR) (LPCTSTR) str;
	m_myTabs.InsertItem ( 0, &sTabCtrlItem );

	str.LoadString (IDS_OPTIONS);
	sTabCtrlItem.pszText = (LPTSTR) (LPCTSTR) str;
	m_myTabs.InsertItem ( 1, &sTabCtrlItem );

	// Tab page number 2
	pButton = (CButton *) GetDlgItem (IDC_SEARCH_GOTOSEL);
	pButton->SetCheck (gpGlobalOptions->GetRegSwitch()->GetSearchGotoLoad());

	// set dialog pos
	CString strSizePos = gpGlobalOptions->GetRegUI ()->GetSearchSizePos ();
	int dx, dy, x, y;
	if (!DecomposeSizePos (strSizePos, dx, dy, x, y))
		SetWindowPos (NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// set dialog size
	int iLoadRet;
	int iCY, iCX = 0;
	BOOL bMax, bMin;
	TRegUI *pRegUI = gpGlobalOptions->GetRegUI ();
	iLoadRet = pRegUI->LoadUtilDlg ("Search", iCX, iCY, bMax, bMin);
	if (0 == iLoadRet) {
		SetWindowPos (NULL, 0, 0, iCX, iCY, SWP_NOZORDER | SWP_NOMOVE);
		if (bMax)
			ShowWindow (SW_SHOWMAXIMIZED);

		// - ignore restore to minimized !
		//if (bMin)
		//   ShowWindow (SW_SHOWMINIMIZED);
	}

	// setup the width, so the default header widths look ok
	RECT rctClient;
	GetWindowRect (&rctClient);

	// I would like a SplitterWindow, but it needs a CFrameWnd. So make one
	int dwBaseUnit = GetDialogBaseUnits ();
	RECT crctResult;
	crctResult.left = LOWORD(dwBaseUnit)/2;
	crctResult.right = (rctClient.right - rctClient.left) - LOWORD(dwBaseUnit)/2;
	crctResult.top = 0; crctResult.bottom = 160;
	m_pResultFrame = new TSearchFrame;

	// A lot of the good stuff happens in OnCreateClient
	VERIFY (m_pResultFrame->Create (NULL, "SrchResFrm",
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, crctResult, this));

	// I want your notification messges.
	m_pResultFrame->GetpListView()->SetOwner(this);

	// I want to position (wake up) the embedded frame window.
	on_size ( SIZE_RESTORED,
		rctClient.right - rctClient.left,
		rctClient.bottom - rctClient.top );

	// image lists ????

	// create a status bar...

	// set the parts for the status bar
	int parts[2];
	CStatusBarCtrl *pBar = (CStatusBarCtrl *) GetDlgItem (IDC_SEARCH_STATUSBAR);
	if (pBar)
	{
		CRect statusRect;
		pBar->GetWindowRect (&statusRect);
		ScreenToClient (&statusRect);
		parts[0] = statusRect.right / 2;
		parts[1] = -1;
		pBar->SetParts (2, parts);
	}

	CRect rc;
	pBar->GetRect (1, &rc);

	m_progress.Create (WS_CHILD | WS_VISIBLE | PBS_SMOOTH, rc, pBar, 1);
	m_progress.SetRange (0, 100);
	m_progress.SetStep (1);

	CString & rLastTime = TSearchDialog::s_strMRUText;
	if (!rLastTime.IsEmpty())
		GetDlgItem (IDC_SEARCH_SEARCHFOR)->SetWindowText ( rLastTime );

	// get it and lock it.
	m_pNewsServer = GetCountedActiveServer ();
	m_pNewsServer->AddRef ();

	// setup the correct icon - done in PreCreateWindow

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnSearchSearchNow()
{
	// first, clear previous search results
	m_pResultFrame->GetResultsCtrl().DeleteAllItems();

	m_bPreserveStatus = FALSE;

	// gather up all the options and stuff into a structure
	// for the thread
	SEARCH_THREAD_PARMS * pParms = new SEARCH_THREAD_PARMS;
	pParms->m_timeFormat = gpGlobalOptions->GetRegUI()->GetDateFormatStr();

	m_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// pass this down to our worker-bee
	pParms->m_stopEvent  = m_stopEvent;
	pParms->m_hWnd       = m_hWnd;

	// get the dialog parameters
	GetDialogParms(pParms);

	if (pParms->m_searchFor.IsEmpty())
	{
		NewsMessageBox(this, IDS_ERR_NO_EXPR);
		return;
	}

	// we set up the group ID and name because this dialog is modal
	// and the search can be re-run after the user switches from one
	// group to another...

	if (!pParms->m_fSearchAll)
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		pParms->m_groupID = GetNewsView()->GetCurNewsGroupID();

		{
			TNewsGroupUseLock useLock(m_pNewsServer, pParms->m_groupID, &fUseLock, pNG);
			if (!fUseLock)
			{
				delete pParms;
				NewsMessageBox(this, IDS_ERR_NO_GROUP_OPEN);
				return;
			}
			pParms->m_newsgroup = pNG->GetName();
		}
	}

	pParms->m_pNewsServer = m_pNewsServer;

	// enable/disable the correct controls
	EnableDisable(TRUE);

	// fire off thread to do search and stuff the COwnerDrawListView
	// pParms is delegated to the thread - it will delete
	// the structure before it returns ...
	CWinThread *pThread = AfxBeginThread((AFX_THREADPROC)BackgroundSearchThread, (LPVOID) pParms);
	m_workerThread = (HANDLE) pThread->m_nThreadID;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::SetProgress (TProgressMessage *pProgress)
{
	CStatusBarCtrl *pCtrl = (CStatusBarCtrl *) GetDlgItem (IDC_SEARCH_STATUSBAR);
	static DWORD last = 0;

	DWORD percChange;

	if (pProgress->m_group.Compare(pCtrl->GetText (0)) != 0)
		pCtrl->SetText (pProgress->m_group, 0, 0);

	if (pProgress->m_total)
	{
		if (pProgress->m_curr == pProgress->m_total)
			m_progress.SetPos (100);
		else if (last != pProgress->m_curr)
		{
			percChange = ((pProgress->m_curr - last) * 100)/(pProgress->m_total);
			if (percChange >= 1)
			{
				last = pProgress->m_curr;
				m_progress.SetPos ((pProgress->m_curr * 100)/pProgress->m_total);
			}
		}
	}
	else
		m_progress.SetPos(0);

	delete pProgress;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
LRESULT TSearchDialog::OnThreadTalk (WPARAM wParam, LPARAM  lParam)
{
	switch (wParam)
	{
		// either way here, we have the same work to do
	case kSearchAborted:
	case kThreadDead:
	case kSearchFinished:
		// close the handle for the search
		CloseHandle (m_stopEvent);
		m_stopEvent = 0;
		m_workerThread = 0;

		// clear status line
		if (!m_bPreserveStatus)
			SetProgress ( new TProgressMessage() );

		// re-enable all of the disabled controls, disable
		// the stop button

		EnableDisable (FALSE);

		if (kSearchAborted == wParam)
			NewsMessageBox (this, IDS_SEARCH_ABORTED); // ????
		GetDlgItem (IDC_SEARCH_SEARCHFOR)->SetFocus();
		break;
	case kSearchProgress:
		SetProgress ((TProgressMessage *) lParam);
		break;
	case kSearchAddResult:
		AddResult ((SEARCH_ARTICLE_RESULT *) lParam);
		break;
	case kPreserveStatus:
		m_bPreserveStatus = TRUE;
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// ArticleMatches -- returns 0 on success, non-zero on error
//                   pfRet contains the match return code
//
//  1-07-98  amc  Changed to return an integer + Bool
int ArticleMatches(TSearch *pSearch, TNewsGroup *pGroup, TArticleHeader *pArt,
					SEARCH_THREAD_PARMS *pParms, BOOL *pfRet)
{
	try
	{
		//TSearch sSearch;

		//sSearch.SetPattern(pParms->m_searchFor, FALSE /* fCaseSensitive */,
		//	pParms->m_fRE ? TSearch::RE : TSearch::NON_RE);
		int iDummy;
		DWORD dwDummy;

		*pfRet = FALSE;

		if (pParms->m_fSearchFrom)
		{
			if (pSearch->Search(LPCTSTR(pArt->GetFrom()), iDummy, dwDummy, _tcslen(pArt->GetFrom())))
			{
				*pfRet = TRUE;
				return 0;
			}
		}
		if (pParms->m_fSearchSubject)
		{
			if (pSearch->Search(LPCTSTR(pArt->GetSubject()), iDummy, dwDummy, _tcslen(pArt->GetSubject())))
			{
				*pfRet = TRUE;
				return 0;
			}
		}

		*pfRet = FALSE;
		if (pParms->m_fSearchBodies || pParms->m_fSearchHeaders)
		{
			TError sErrorRet;
			CPoint ptPartID(0,0);
			TArticleText *psBody = 0;
			pGroup->Open();

			int iFetchResult = fnFetchBody(sErrorRet,
				pGroup, 
				pArt, 
				psBody, 
				ptPartID,
				FALSE /* bMarkAsRead */, 
				!pParms->m_fLocalOnly /* bTryFromNewsfeed */);
			pGroup->Close();
			switch (iFetchResult)
			{
			case 0:
				{
					CString text;
					if (pParms->m_fSearchHeaders && pParms->m_fSearchBodies)
						text = psBody->GetHeader() + "\n" + psBody->GetBody();
					else if (pParms->m_fSearchHeaders)
						text = psBody->GetHeader();
					else
						text = psBody->GetBody();
					*pfRet = pSearch->Search(text, iDummy, dwDummy, text.GetLength());
				}
				break;

			case PUMP_EXCEPTION:
			case PUMP_SHUTDOWN:
			case PUMP_GROUP_UNSUBSCRIBED:
			case PUMP_USER_STOP:
				// sounds pretty serious - stop the search  -amc
				SetEvent(pParms->m_stopEvent);
				break;

			case PUMP_NOT_CONNECTED:
				{
					// stop the search
					SetEvent(pParms->m_stopEvent);

					// display error message
					TProgressMessage *pProgress  = new TProgressMessage;
					pProgress->m_group.LoadString (IDS_BODY_FAIL_NOT_CONNECTED);
					SendMessage(pParms->m_hWnd,
						WMU_SEARCH_THREADTALK,
						(WPARAM) TSearchDialog::kSearchProgress,
						(LPARAM) pProgress);

					SendMessage(pParms->m_hWnd,
						WMU_SEARCH_THREADTALK,
						(WPARAM) TSearchDialog::kPreserveStatus, 0);
				}
				break;

			default:
				*pfRet = FALSE;
				break;
			}

			// our responsibility to delete the fetched body
			delete psBody;
		}

		// function ran OK.
		return 0;
	}
	catch (TException *except)
	{
		// SetPattern can throw
		except->Display();
		except->Delete();

		// big time error
		return 1;
	}
	catch(...)
	{
		// all other exceptions
		return 1;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TSearchDialog::EThreadMessage SearchNewsGroup (
	LONG groupID,
	SEARCH_THREAD_PARMS *pParms)
{
	TArticleHeader   *pArt;
	CString           timeString;
	DWORD             dwResult;
	int bLoadedGroup = FALSE;
	TSearchDialog::EThreadMessage iReturn = TSearchDialog::kSearchFinished;

	TNewsGroup *pGroup = NULL;
	BOOL fUseLock;
	TNewsGroupUseLock useLock(pParms->m_pNewsServer, groupID, &fUseLock, pGroup);

	if (!fUseLock)
		return iReturn;

	TAutoClose sCloser(pGroup);            // auto-open, auto-close

	BOOL bGroupLocked = FALSE;

	// The newsgroup fills the index.  Afterwards we only work with the
	//  index. We can release the read lock on the NewsGroup object.

	POSITION     pos;
	TArticleIndexList sIndexList;          // unordered set of articles

	// if this newsgroup's thread-list is already constructed, and the
	// current filter set is what we want (VIEW ALL), use it. Otherwise,
	// make our own threadlist

	TViewFilter * pFilter = TNewsGroup::GetpCurViewFilter();
	if (0 == pFilter)
		return iReturn;

	if (pGroup->FlatListLength() && pFilter->IsViewAll())
	{
		pGroup->ReadLock();
		pGroup->CreateArticleIndex(&sIndexList);
		pGroup->UnlockRead();
	}
	else
	{
		// may want to put up a background-hourglass cursor here...

		TProgressMessage *pProgress  = new TProgressMessage;
		CString temp;

		temp.LoadString(IDS_LOADING_HEADERS_FOR);
		temp += " ";
		temp += pGroup->GetName();
		pProgress->m_group = temp;
		SendMessage (pParms->m_hWnd,
			WMU_SEARCH_THREADTALK,
			(WPARAM) TSearchDialog::kSearchProgress,
			(LPARAM) pProgress);

		// ad-hoc filter contains all
		TViewFilter sFilter(0);

		// make our own thread list
		pGroup->LoadForArticleIndex(&sFilter,              // filter
			FALSE,                 // fCreateStati
			!pParms->m_fLocalOnly, // fHitServer
			&sIndexList);
	}

	TSearch sSearch;

	sSearch.SetPattern(pParms->m_searchFor, FALSE /* fCaseSensitive */,
		pParms->m_fRE ? TSearch::RE : TSearch::NON_RE);

	pos = sIndexList->GetHeadPosition();
	int count = 0;
	while ( pos )
	{
		pArt = sIndexList->GetNext(pos);
		// make sure the window is still there
		if (IsWindow(pParms->m_hWnd))
		{
			TProgressMessage *pProgress  = new TProgressMessage;
			pProgress->m_group = pParms->m_newsgroup;
			pProgress->m_curr = ++count;
			pProgress->m_total = sIndexList->GetCount();
			SendMessage (pParms->m_hWnd,
				WMU_SEARCH_THREADTALK,
				(WPARAM) TSearchDialog::kSearchProgress,
				(LPARAM) pProgress);
		}

		// make sure the window is still there
		if (IsWindow(pParms->m_hWnd))
		{
			BOOL fMatch = FALSE;
			if (0 != ArticleMatches(&sSearch, pGroup, pArt, pParms, &fMatch))
			{
				// massive error, end the loop
				break;
			}
			else if (fMatch)
			{
				// send the result back to the dialog...
				SEARCH_ARTICLE_RESULT *pResult = new SEARCH_ARTICLE_RESULT;
				CString  custDate;

				pResult->m_from = pArt->GetFrom();
				pResult->m_subject = pArt->GetSubject();
				pResult->m_newsgroup = pParms->m_newsgroup;

				pResult->m_groupID = pParms->m_groupID;
				pResult->m_header = new TArticleHeader;
				*pResult->m_header = *pArt;
				CTime local;
				GetLocalTime(pResult->m_header->GetTime(), local);
				GetCustomFormat(pParms->m_timeFormat, local, pResult->m_date);

				SendMessage(pParms->m_hWnd,
					WMU_SEARCH_THREADTALK,
					(WPARAM) TSearchDialog::kSearchAddResult,
					(LPARAM) pResult);
			}
		}

		dwResult = WaitForSingleObject(pParms->m_stopEvent, 0);
		if (dwResult == WAIT_OBJECT_0)
		{
			pArt = 0;
			iReturn = TSearchDialog::kThreadDead;
			break;
		}
		pArt = 0;
	} // while loop

	TProgressMessage *pProgress  = new TProgressMessage;
	pProgress->m_group = pParms->m_newsgroup;
	pProgress->m_curr = pProgress->m_total = sIndexList->GetCount();
	SendMessage(pParms->m_hWnd,
		WMU_SEARCH_THREADTALK,
		(WPARAM) TSearchDialog::kSearchProgress,
		(LPARAM) pProgress);

	return iReturn;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
UINT BackgroundSearchThread(LPVOID pParam)
{
	try
	{
		SEARCH_THREAD_PARMS  *        pParms = (SEARCH_THREAD_PARMS *) pParam;
		BOOL                          fDone = FALSE;
		BOOL                          fSearchHeaders = pParms->m_fSearchHeaders;
		BOOL                          fSearchBodies = pParms->m_fSearchBodies;

		CArray<TGroupNameID, TGroupNameID&> groupList;
		TSearchDialog::EThreadMessage kResult;

		if (pParms->m_fSearchAll)
		{
			if (!GetGroupList(pParms->m_pNewsServer, groupList))
			{
				// empty the group list
				groupList.RemoveAll();

				// ???? post transaction abort message
				::PostMessage(pParms->m_hWnd,
					WMU_SEARCH_THREADTALK,
					(WPARAM) TSearchDialog::kSearchAborted,
					(LPARAM) 0);
				delete pParms;
				return FALSE;
			}
		}

		if (pParms->m_fSearchAll)
		{
			for (int i = 0; i < groupList.GetSize (); i++)
			{
				pParms->m_newsgroup = groupList[i].m_name;
				pParms->m_groupID = groupList[i].m_groupID;
				pParms->m_fSearchHeaders = fSearchHeaders;
				pParms->m_fSearchBodies = fSearchBodies;
				if (TSearchDialog::kSearchFinished !=
					(kResult = SearchNewsGroup(groupList[i].m_groupID, pParms)))
				{
					// remove the list of groups
					groupList.RemoveAll();

					// delete the delegated parameters
					::PostMessage(pParms->m_hWnd,
						WMU_SEARCH_THREADTALK,
						(WPARAM) kResult,
						(LPARAM) 0);
					delete pParms;
					return FALSE;
				}
			}
		}
		else
		{
			if (TSearchDialog::kSearchFinished !=
				(kResult = SearchNewsGroup(pParms->m_groupID, pParms)))
			{
				// remove the list of groups
				groupList.RemoveAll();

				// delete the delegated parameters
				::PostMessage(pParms->m_hWnd,
					WMU_SEARCH_THREADTALK,
					(WPARAM) kResult,
					(LPARAM) 0);
				delete pParms;
				return FALSE;
			}
		}

		// make sure the window is still there
		if (IsWindow (pParms->m_hWnd))
		{
			// we finished, let the dialog know...
			TProgressMessage *pProgress  = new TProgressMessage;
			pProgress->m_group.LoadString(IDS_SEARCH_COMPLETE);
			SendMessage(pParms->m_hWnd,
				WMU_SEARCH_THREADTALK,
				(WPARAM) TSearchDialog::kSearchProgress,
				(LPARAM) pProgress);

			SendMessage(pParms->m_hWnd,
				WMU_SEARCH_THREADTALK,
				(WPARAM) TSearchDialog::kSearchFinished,
				(LPARAM) 0);
		}

		delete pParms;
		groupList.RemoveAll();
		return TRUE;
	}
	catch (TException *Except)
	{
		Except->PushError(IDS_ERR_SEARCH_THREAD, kError);
		TException *e = new TException(*Except);
		Except->Display();
		Except->Delete();
		TopLevelException(kThreadSearch);
		throw(e);
		return FALSE;
	}
	catch(...)
	{
		TopLevelException(kThreadSearch);
		throw;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnSearchStop()
{
	// set the event to stop the thread
	SetEvent (m_stopEvent);

	// we'll wait for a message from the thread
	// before we do anything else ... it will
	// be handled in OnThreadTalk
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnDestroy()
{
	// save window size
	TRegUI *pRegUI = gpGlobalOptions->GetRegUI ();
	WINDOWPLACEMENT sPlacement;  ZeroMemory(&sPlacement, sizeof sPlacement);
	sPlacement.length = sizeof sPlacement;
	GetWindowPlacement ( &sPlacement );
	BOOL bZoomed = (sPlacement.showCmd == SW_SHOWMAXIMIZED);
	BOOL bIconic = (sPlacement.showCmd == SW_SHOWMINIMIZED);
	RECT & rNorm = sPlacement.rcNormalPosition;
	pRegUI->SaveUtilDlg ("Search", (int) (rNorm.right - rNorm.left),
		(int) (rNorm.bottom - rNorm.top), bZoomed, bIconic);

	// save window pos
	CRect sRect;
	GetWindowRect (&sRect);
	CString strSizePos = ComposeSizePos (0, 0, sRect.left, sRect.top);
	gpGlobalOptions->GetRegUI ()->SetSearchSizePos (strSizePos);

	m_pResultFrame->DestroyWindow ();
	m_pResultFrame = 0;

	CDialog::OnDestroy();
}

// ------------------------------------------------------------------------
// OnClose
//
// 8-6-97  release m_pNewsServer reference
void TSearchDialog::OnClose()
{
	if (m_stopEvent)
	{
		SetEvent (m_stopEvent);
		WaitForSingleObject (m_workerThread, INFINITE);
	}

	// remember the text, for the next invocation of the dialog
	//  this is a class-static string, so it will survive
	CString strTest;
	GetDlgItem (IDC_SEARCH_SEARCHFOR)->GetWindowText ( strTest );
	if (!strTest.IsEmpty())
		s_strMRUText = strTest;

	DestroyWindow();
	if (m_stopEvent)
	{
		CloseHandle (m_stopEvent);
		m_stopEvent = 0;
	}

	m_pNewsServer->Release ();
	m_pNewsServer = 0;
}

// ------------------------------------------------------------------------
TRichEd * TSearchDialog::GetRichEdit ()
{
	return &m_pResultFrame->GetViewPane ();
}

/////////////////////////////////////////////////////////////////////////////
// EnableDisable - Based on whether we're currently searching or not
//                 enable and disable the appropriate controls.
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::EnableDisable (BOOL   fRunning)
{
	GetDlgItem (IDC_SEARCH_RE)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_SEARCHNOW)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_SEARCHFOR)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_FROM)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_SUBJECT)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_INCLUDEHEADERS)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_INCLUDEBODIES)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_SEARCHNOW)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_JUMP)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_THIS_NEWSGROUP)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_ALL_NEWSGROUPS)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_LOCAL_ONLY)->EnableWindow (!fRunning);
	GetDlgItem (IDC_SEARCH_STOP)->EnableWindow (fRunning);
}

/////////////////////////////////////////////////////////////////////////////
// GetDialogParms - Get the current parameters from the dialog.
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::GetDialogParms (SEARCH_THREAD_PARMS  *pParms)
{
	CWnd *pWnd;

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_SEARCHFOR);
	((CEdit *) pWnd)->GetWindowText (pParms->m_searchFor);

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_FROM);
	pParms->m_fSearchFrom = ((CButton *) pWnd)->GetCheck();

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_SUBJECT);
	pParms->m_fSearchSubject = ((CButton *) pWnd)->GetCheck();

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_INCLUDEHEADERS);
	pParms->m_fSearchHeaders = ((CButton *) pWnd)->GetCheck();

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_INCLUDEBODIES);
	pParms->m_fSearchBodies = ((CButton *) pWnd)->GetCheck();

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_ALL_NEWSGROUPS);
	pParms->m_fSearchAll = ((CButton *) pWnd)->GetCheck();

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_LOCAL_ONLY);
	pParms->m_fLocalOnly = ((CButton *) pWnd)->GetCheck();

	pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_RE);
	pParms->m_fRE = ((CButton *) pWnd)->GetCheck();
}

/////////////////////////////////////////////////////////////////////////////
// AddResult - Add a result to the COwnerDrawListView
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::AddResult (SEARCH_ARTICLE_RESULT  *pResult)
{
	int index = 0;
	int count = 0;

	TResultItemData *pTempResult;

	CListCtrl & rResults = m_pResultFrame->GetResultsCtrl();
	count = rResults.GetItemCount ();

	for (index = 0; index < count; index++)
	{
		pTempResult = (TResultItemData *) rResults.GetItemData(index);
		if (pTempResult->m_header->GetTime() < pResult->m_header->GetTime())
			break;
	}

	TResultItemData *pData = new TResultItemData;

	pData->m_groupID = pResult->m_groupID;
	pData->m_header = pResult->m_header;

	rResults.InsertItem (LVIF_TEXT, index, pResult->m_from, 0, 0, -1, 0/*(LPARAM) pData*/);
	rResults.SetItemText (index, 1, LPTSTR(LPCTSTR(pResult->m_subject)));
	rResults.SetItemText (index, 2, LPTSTR(LPCTSTR(pResult->m_date)));
	rResults.SetItemText (index, 3, LPTSTR(LPCTSTR(pResult->m_newsgroup)));
	rResults.SetItemData (index, (DWORD) pData);
	delete pResult;
}

/////////////////////////////////////////////////////////////////////////////
// Get the whole list of groups.
/////////////////////////////////////////////////////////////////////////////
BOOL GetGroupList (TNewsServer* pNewsServer, CArray<TGroupNameID, TGroupNameID &> & groupList)
{
	TGroupNameID   group;
	int            i;

	TNewsGroupArray& vNewsGroups = pNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr (vNewsGroups);

	for (i = 0; i < vNewsGroups->GetSize (); i++)
	{
		TNewsGroup * pNG = vNewsGroups[i];
		group.m_name      = pNG->GetName ();
		group.m_groupID   = pNG->m_GroupID;
		groupList.Add (group);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// load components in preparation for Posting
BOOL TSearchDialog::LoadArticleForDisplay (LONG             GroupID,
										   TArticleHeader*  pHdr,
										   TArticleText*&   pArtText)
{
	// load the newsgroup
	BOOL fUseLock;
	TNewsGroup *pGroup;
	TNewsGroupUseLock useLock(m_pNewsServer, GroupID, &fUseLock, pGroup);
	if (!fUseLock)
		return FALSE;

	TAutoClose sCloser (pGroup);     // auto open,  auto close

	TError  sErrorRet;
	CPoint  ptPartID(0,0);

	// load the body
	// NOTE (4/10/96) DO NOT Try to mark the article as read at this point...
	// that will cause a deadlock, at least the way things are implemented now

	BOOL bResult = !fnFetchBody (sErrorRet,
		pGroup,
		pHdr,
		pArtText,
		ptPartID,
		FALSE /* bMarkAsRead */,
		TRUE /* bTryFromNewsFeed*/);

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::DisplaySelectedArticle ()
{
	int index = GetCurSel ();
	if (index < 0)
		return;

	BeginWaitCursor ();

	CListCtrl & rResults = m_pResultFrame->GetResultsCtrl ();
	TResultItemData *pData = (TResultItemData *) rResults.GetItemData (index);
	TArticleHeader *pArtHdr = pData->m_header;
	TArticleText *pArtText;
	if (LoadArticleForDisplay (pData->m_groupID, pArtHdr, pArtText))
	{
		RichDisplayArticle (GetRichEdit(), pArtHdr, pArtText);

		// our responsibility to delete the loaded article's text
		delete pArtText;
	}
	else
		NewsMessageBox (this, IDS_ERR_LOAD_FOR_DISPLAY);

	EndWaitCursor ();
}

/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnDblclkSearchResults(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!gpGlobalOptions->GetRegUI()->GetOneClickArt())
		DisplaySelectedArticle ();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnClkSearchResults(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (gpGlobalOptions->GetRegUI()->GetOneClickArt())
		DisplaySelectedArticle ();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnSearchJump()
{
	int index = GetCurSel ();

	if (index < 0)
	{
		return;
	}

	CListCtrl *pCtrl = &(m_pResultFrame->GetResultsCtrl());

	TResultItemData *pData = (TResultItemData *) pCtrl->GetItemData (index);

	TGotoArticle sGoto;
	sGoto.m_articleNumber = pData->m_header->GetArticleNumber();
	sGoto.m_groupNumber   = pData->m_groupID;
	sGoto.m_byLoad        = BYTE(gpGlobalOptions->GetRegSwitch()->GetSearchGotoLoad ());
	sGoto.m_byOpenNG      = FALSE;
	BOOL fArtMatchesFilter = FALSE;

	// Get this newsgroup object

	{
		BOOL fUseLock;
		TNewsGroup * pNG = 0;
		TNewsGroupUseLock useLock(m_pNewsServer, sGoto.m_groupNumber, &fUseLock, pNG);
		if (!fUseLock)
			return;

		TViewFilter * pFilter = TNewsGroup::GetpCurViewFilter ();
		if (0 == pFilter)
			return;

		fArtMatchesFilter = pFilter->FilterMatch (sGoto.m_articleNumber,
			pNG,
			&m_pNewsServer->GetPersistentTags());
	}
	BOOL fForceFilterChange = FALSE;
	if (!fArtMatchesFilter)
	{
		if (IDCANCEL == NewsMessageBox(this, IDS_NOMATCHFILTER_CHANGE, MB_OKCANCEL))
			return;

		fForceFilterChange = TRUE;
	}

	CMDIFrameWnd *pMainWnd = static_cast<CMDIFrameWnd *> (AfxGetMainWnd ());

	if (fForceFilterChange)
	{
		if (0 != pMainWnd->SendMessage (WMU_FORCE_FILTER_CHANGE))
			return;
		// ask to reopen the group , to utilize new filter.
		sGoto.m_byOpenNG = TRUE;
	}
	TNews3MDIChildWnd *pChildWnd  = (TNews3MDIChildWnd *) pMainWnd->MDIGetActive();

	// jump to the article that has been selected
	pChildWnd->SendMessage (WMU_NEWSVIEW_GOTOARTICLE, 0, LPARAM(&sGoto));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int TSearchDialog::GetCurSel (void)
{
	CListCtrl & rResults = m_pResultFrame->GetResultsCtrl ();
	int total = rResults.GetItemCount();
	for (int i = 0; i < total; ++i)
	{
		if (rResults.GetItemState(i, LVIS_SELECTED))
			return i;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnDeleteitemSearchResults(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	delete (TResultItemData *)
		(m_pResultFrame->GetResultsCtrl ().GetItemData (pNMListView->iItem));
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	on_size (nType, cx, cy);
}

/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::on_size (UINT nType, int cx, int cy)
{
	// the status bar resizes itself according to its parent
	CWnd *pWnd = (CWnd *) GetDlgItem (IDC_SEARCH_STATUSBAR);
	if (pWnd)
	{
		pWnd->SendMessage (WM_SIZE, (WPARAM) 0, MAKELPARAM(cx, cy));

		// we want the results window and the article display
		// to be the same width as the status bar -
		CRect statusRect;

		// get position with respect to parent
		Utility_GetPosParent (this, pWnd, statusRect);

		CRect tabRect;
		Utility_GetPosParent (this, IDC_TAB1, tabRect);

		if (m_pResultFrame)
		{
			DWORD dwBaseUnit = GetDialogBaseUnits ();

			// Position the frame window, below the TabCtrl and above the Statusbar
			int x = statusRect.left + LOWORD(dwBaseUnit);
			int y = tabRect.bottom + HIWORD(dwBaseUnit) / 2;

			m_pResultFrame->SetWindowPos (NULL, x, y,
				max(statusRect.right - LOWORD(dwBaseUnit) - x, 0),
				max(statusRect.top - HIWORD(dwBaseUnit)/2 - y, 0),
				SWP_NOZORDER);
		}

		if (IsWindow (m_progress.m_hWnd))
		{
			RECT  paneRect;
			CStatusBarCtrl *pBar = (CStatusBarCtrl *) GetDlgItem (IDC_SEARCH_STATUSBAR);
			pBar->GetRect (1, &paneRect);
			m_progress.SetWindowPos (NULL, paneRect.left, 
				paneRect.top,
				paneRect.right - paneRect.left,
				paneRect.bottom - paneRect.top,
				SWP_NOZORDER);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TSearchDialog::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	// how small can the window get ?
	CWnd * pGotoButton = (CWnd *) GetDlgItem (IDC_SEARCH_JUMP);

	if (pGotoButton)
	{
		CRect  resultRect;
		RECT   clntRect;
		CRect  jumpRect;

		// this is the "Goto Article" button
		Utility_GetPosParent (this, IDC_SEARCH_JUMP, jumpRect);

		clntRect.top = clntRect.left = 0;
		clntRect.right = jumpRect.right + 4;
		clntRect.bottom = jumpRect.bottom + 45;

		AdjustWindowRect (&clntRect,
			WS_MINIMIZEBOX | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU |  WS_THICKFRAME,
			FALSE);

		lpMMI->ptMinTrackSize.x = clntRect.right - clntRect.left;
		lpMMI->ptMinTrackSize.y = clntRect.bottom - clntRect.top;
		CDialog::OnGetMinMaxInfo(lpMMI);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TSearchDialog::OnCancel( )
{
	OnClose ();
}

static void show_util (CWnd * pDlg, int * riID,  int n, BOOL fShow)
{
	for (int i = 0; i < n; ++i)
	{
		CWnd * pWnd = pDlg->GetDlgItem(riID[i]);
		ASSERT(pWnd);
		pWnd->EnableWindow(fShow);
		pWnd->ShowWindow(fShow ? SW_SHOW : SW_HIDE);
	}
}

void TSearchDialog::OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iTab = m_myTabs.GetCurSel();
	int riTab1[] = {
		IDC_SEARCH_SEARCHFORDESC,
		IDC_SEARCH_SEARCHFOR,
		IDC_SEARCH_RE,
		IDC_SEARCH_FROM,
		IDC_SEARCH_SUBJECT,
		IDC_SEARCH_INCLUDEHEADERS,
		IDC_SEARCH_INCLUDEBODIES,
		IDC_SEARCH_THIS_NEWSGROUP,
		IDC_SEARCH_ALL_NEWSGROUPS,
		IDC_SEARCH_LOCAL_ONLY
	};

	int riTab2[] = { IDC_SEARCH_GOTOSEL };

	switch (iTab)
	{
	case 0:
		show_util(this, riTab1, sizeof(riTab1)/sizeof(riTab1[0]), TRUE);
		show_util(this, riTab2, sizeof(riTab2)/sizeof(riTab2[0]), FALSE);
		GetDlgItem(IDC_SEARCH_SEARCHFOR)->SetFocus();
		break;

	case 1:
		show_util(this, riTab1, sizeof(riTab1)/sizeof(riTab1[0]), FALSE);
		show_util(this, riTab2, sizeof(riTab2)/sizeof(riTab2[0]), TRUE);
		GetDlgItem(IDC_SEARCH_GOTOSEL)->SetFocus();
		break;
	}
}

// ------------------------------------------------------------------------
//
void TSearchDialog::OnSearchOptionsGoto()
{
	TRegSwitch * pRS = gpGlobalOptions->GetRegSwitch ();

	pRS->SetSearchGotoLoad (IsDlgButtonChecked(IDC_SEARCH_GOTOSEL));

	// save immediately
	pRS->Save();
}

// ------------------------------------------------------------------------
// TResultItemData destructor
TResultItemData::~TResultItemData ()
{
	delete m_header;
}
