/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsdoc.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.4  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.6  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.5  2008/09/19 14:51:33  richard_wood
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

// Newsdoc.cpp : implementation of the CNewsDoc class
//   This document may represent a list of newsgroups
//

#include "stdafx.h"
#include "News.h"

#include "Newsdoc.h"
#include "srvritem.h"

#include "thredlst.h"

#include "globals.h"
#include "tsubscri.h"
#include "custview.h"
#include "arttext.h"
#include "tglobopt.h"
#include "custmsg.h"             // WMU_PANIC_KEY
#include "timeutil.h"
#include "ngstat.h"

#include "hints.h"
#include "statchg.h"
#include "kidsecgt.h"
#include "fileutil.h"            // fnNNTPError
#include "utilpump.h"
#include "msgid.h"
#include "server.h"
#include "tasker.h"
#include "getlist.h"
#include "rgswit.h"                    // TRegSwitch
#include "rgsys.h"                     // TRegSystem
#include "newsview.h"
#include "rules.h"                     // ResetAllRuleSubstitution ();
#include "utilerr.h"                   // TNNTPErrorDialog
#include "tsubjct.h"                   // TSubject  interface
#include "tsubjctx.h"
#include "genutil.h"                   // evil  GetNewsView()

//#include "myspell30\myspell.hxx"
#include "hunspell-1.2.8\src\hunspell\hunspell.hxx"
#include "mywords.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// class wide variable
CNewsDoc* CNewsDoc::m_pDoc;

extern TGlobalOptions* gpGlobalOptions;
extern TNewsTasker*    gpTasker;
extern BYTE gfFirstUse;
extern HWND ghwndMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CNewsDoc

IMPLEMENT_DYNCREATE(CNewsDoc, CDocument)

BEGIN_MESSAGE_MAP(CNewsDoc, CDocument)
		ON_COMMAND(ID_NEWSGROUP_SUBSCRIBE, OnNewsgroupSubscribe)
	ON_UPDATE_COMMAND_UI(ID_NEWSGROUP_SUBSCRIBE, OnUpdateNewsgroupSubscribe)
	ON_COMMAND(ID_NEWSGROUP_RECENT, OnNewsgroupRecent)
	ON_UPDATE_COMMAND_UI(ID_NEWSGROUP_RECENT, OnUpdateNewsgroupRecent)
	ON_COMMAND(ID_NEWSGROUP_GETALL, OnNewsgroupGetall)
	ON_UPDATE_COMMAND_UI(ID_NEWSGROUP_GETALL, OnUpdateNewsgroupGetall)
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewsDoc construction/destruction

// --------------------------------------------------------------------------
CNewsDoc::CNewsDoc()
{
	// TODO: add one-time construction code here

	InitializeCriticalSection (&m_csStatChg);
	m_iStatChangeCount = 0;

	m_fPanicMode = false;   // current state of Panic

	m_fPanicHookRunning = false;

	ConfigurePanicKey (gpGlobalOptions->GetRegSystem()->EnablePanicKey());

	m_pMS = NULL;
	m_pMyWords = new TMyWords();
}

// --------------------------------------------------------------------------
CNewsDoc::~CNewsDoc()
{
	// turn it off for Good.
	ConfigurePanicKey (false);

	DeleteCriticalSection (&m_csStatChg);

	delete m_pMyWords;
	delete m_pMS;
}

BOOL CNewsDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// 12-5-95  I hate that 'News1' title -amc
	SetTitle ("");
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CNewsDoc serialization

void CNewsDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNewsDoc diagnostics

#ifdef _DEBUG
void CNewsDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNewsDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNewsDoc commands

void CNewsDoc::OnNewsgroupSubscribe()
{
	CallSubscribeDlg(NULL);
}

// this is a public function
void CNewsDoc::CallSubscribeDlg(TGroupList * pFreshGroups)
{
	try
	{
		// Check password.  Don't let little kiddies access porn!
		BOOL fContinue = FALSE;
		const CString& subscribePwd = gpGlobalOptions->GetSubscribePassword();
		if (!subscribePwd.IsEmpty())
		{
			TKidSecurityGetPwd dlg(fContinue, subscribePwd, AfxGetMainWnd());
			dlg.DoModal();
		}
		else
			fContinue = TRUE;
		if (fContinue)
		{
			int dlgRet;
			TSubscribeDlg  dlg(NULL, pFreshGroups);

			dlg.pDoc = this;
			dlgRet = dlg.DoModal();
			if (gfFirstUse)
				gfFirstUse = FALSE;

			if (IDCANCEL != dlgRet)
			{
				// for the newly added NewsGroups, show count of articles
				//   on the server
				if (gpTasker->IsConnected())
				{
					gpTasker->PingList ( dlg.m_OutputGroupNames );

					TServerCountedPtr cpNewsServer;

					// automatically get the headers?
					if (cpNewsServer->GetSubscribeDL())
					{
						POSITION pos = dlg.m_OutputGroupNames.GetHeadPosition ();
						while (pos)
						{
							CString ngName = dlg.m_OutputGroupNames.GetNext (pos);
							if (!gpTasker->PrioritizeNewsgroup (ngName, TRUE, 0, TNewsTasker::kActionProgram))
								break;
						}
					}
				}
			}
		}
	}
	catch (TException *exc)
	{
		exc->PushError (IDS_ERR_SUBSCRIBING, kError);
		TException *ex = new TException(*exc);
		exc->Delete();
		throw(ex);
	}
	catch (CException* pExc)
	{
		pExc->Delete();
		throw(new TException(IDS_ERR_SUBSCRIBING, kError));
	}
}

/////////////////////////////////////////////////////////////////////////////
//
void CNewsDoc::OnUpdateNewsgroupSubscribe(CCmdUI* pCmdUI)
{
	try
	{
		TServerCountedPtr cpNewsServer;
		BOOL fEnable = cpNewsServer->GetGroupListExist();
		pCmdUI->Enable ( fEnable );
	}
	catch(...)
	{
		// catch all
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// This is a pretty 'engine' like function, so I put it here. The newsdoc
// object is pretty lightweight right now.
//
int CNewsDoc::SaveToFile(TPersist822Header*  pBaseArt,
						 TPersist822Text *   pBaseText,
						 BOOL                fFullHdr,
						 CFile&           file)
{
	const char * gpchMessageSeparator =
		"\r\n------------------------------------------------------------\r\n\r\n";
	extern TGlobalOptions *gpGlobalOptions;

	TCustomArticleView& rCustomView = gpGlobalOptions->GetCustomView();
	int total = rCustomView.GetSize();

	if (total <= 0)
		return -2;

	if (fFullHdr)
	{
		// Full Header, and blank line
		const CString& strFullHdr = pBaseText->GetHeader ();
		file.Write (strFullHdr, strFullHdr.GetLength() );
		file.Write ("\r\n", 2);

		// body
		const CString& strBody = pBaseText->GetBody ();
		file.Write (strBody, strBody.GetLength());
	}
	else
	{
		CString field;

		int wField;
		for (int i = 0; i < total; ++i)
		{
			const TArticleItemView& rItem = rCustomView[i];
			wField = rItem.GetStringID();

			if (IDS_TMPL_BLANK == wField)
			{
				field = "\r\n";
			}
			else if (IDS_TMPL_BODY == wField)
			{
				ArticleCreateField ( pBaseArt, pBaseText, WORD(wField), FALSE, field );
			}
			else
			{
				if (FALSE == ArticleCreateField ( pBaseArt, pBaseText, WORD(wField), TRUE, field ))
					continue;
				field += "\r\n";
			}

			file.Write (field, field.GetLength());
		}
	}

	// RLW : Match rule "save" behaviour in that we create a seperator between saved articles.
	file.Write(gpchMessageSeparator, _tcslen(gpchMessageSeparator));

	return 0;
}

// UI thingy
void CNewsDoc::OnUpdateNewsgroupRecent(CCmdUI* pCmdUI)
{
	extern BYTE gfGettingNewsgroupList;
	// if not running, then enable it.
	// if running then disable it.
	pCmdUI->Enable (!gfGettingNewsgroupList);
}

///////////////////////////////////////////////////////////////////////////
// UtilRetrieveRecentNewsgroups -- now it sends a job to the pump
//
void CNewsDoc::UtilRetrieveRecentNewsgroups (
	bool fRequestGetEntireList,
	BOOL fShowSubscribeDialog,
	BOOL fPopupEmptyList /* ==FALSE */
	)
{
	TServerCountedPtr cpNewsServer;
	extern CTime gTimeAncient;

	// Force Connection here!!
	if (!gpTasker->IsConnected())
	{
		if (0 != gpTasker->Connect( cpNewsServer->GetNewsServerAddress() ) )
			return ;
	}

	CTime since;
	extern TGlobalOptions *gpGlobalOptions;

	// load start time from somewhere.
	cpNewsServer->GetNewsGroupCheckTime( since );

	BOOL fGetAll = FALSE;
	if (fRequestGetEntireList || (gTimeAncient.GetYear() == since.GetYear()))
		fGetAll = TRUE;

#if defined(_DEBUG)
	CString res1 = since.Format ("%m %d %Y %H:%M:%S");
	afxDump << "since " << res1 << "\n";
#endif

	WPARAM wParam = 0;
	if (fShowSubscribeDialog)
		wParam |= kGetListShowDialog;

	if (fPopupEmptyList)
		wParam |= kGetListPopupEmptyList;

	gpTasker->RetrieveNewsgroups(fGetAll,  // All?
		wParam, since);
}

// Get fresh newsgroups from the server
void CNewsDoc::OnNewsgroupRecent()
{
	UtilRetrieveRecentNewsgroups (false,   // force Get Entire List
		TRUE,    // show subscribe dialog
		TRUE);   // popup empty list
}

void CNewsDoc::AutoDelete (BOOL fOn)
{
	m_bAutoDelete = fOn;
}

//////////////////////////////////////////////////////////////////////////
void CNewsDoc::AddArticleStatusChange ( TStatusChg* pStatusChange )
{
	TEnterCSection sAuto(&m_csStatChg);

	m_lstStatChg.AddTail ( *pStatusChange );
	m_iStatChangeCount ++;
}

//////////////////////////////////////////////////////////////////////////
//  Propagates a change to the other panes (FlatListView vs ThreadListView)
//
BOOL CNewsDoc::DequeueArticleStatusChange (void)
{
	TStatusChg achg;

	if (true)
	{
		TEnterCSection sAuto(&m_csStatChg);

		if ((m_iStatChangeCount=m_lstStatChg.GetCount()) == 0)
			return FALSE;

		achg = m_lstStatChg.RemoveHead();
	}

	CNewsView * pNV = GetNewsView();

	// update only if the group matches
	if (pNV  &&  (pNV->GetCurNewsGroupID() == achg.m_lGroupID))
	{
		UpdateAllViews ( NULL, VIEWHINT_STATUS_CHANGE,
			&achg );                       // the Hint
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//  We can short-circuit this if we are emptying the views. Called from
//  CNewsView::OnUpdate(VIEWHINT_EMPTY)
void CNewsDoc::EmptyArticleStatusChange (void)
{
	TEnterCSection sAuto(&m_csStatChg);

	m_lstStatChg.RemoveAll();  // empty the list of Status Changes
	m_iStatChangeCount = 0;
}

void CNewsDoc::CancelMessage(
							 const CString& newsGroup,
							 const CString& oldmsgid,
							 const CString& oldsubject
							 )
{
	TServerCountedPtr cpNewsServer;
	ASSERT(!newsGroup.IsEmpty());
	ASSERT(!oldmsgid.IsEmpty());

	TArticleHeader * pHdr = new TArticleHeader;
	TArticleText   sBod;
	sBod.SetBody ( "Cancel\r\n" );
	CString subj;
	subj.Format("Cancel \"%s\"", (LPCTSTR)oldsubject);
	CString ctrlline;
	ctrlline.Format("cancel %s", (LPCTSTR)oldmsgid);
	int id = cpNewsServer->NextPostID();

	pHdr->SetNumber (id);
	sBod.SetNumber (id);

	CString strMyAddress = cpNewsServer->GetEmailAddress ();
	if (!cpNewsServer->GetEmailAddressForPost ().IsEmpty ())
		strMyAddress = cpNewsServer->GetEmailAddressForPost ();

	pHdr->SetFrom(strMyAddress);
	pHdr->SetSubject (subj);
	pHdr->AddNewsGroup ( newsGroup );
	pHdr->StampCurrentTime ();
	pHdr->SetLines ( sBod.GetBody() );

	pHdr->SetControl ( ctrlline );

	CString msg_id;
	GenerateMessageID ( cpNewsServer->GetEmailAddress(),
		cpNewsServer->GetNewsServerAddress(),
		id,
		msg_id );
	pHdr->SetMessageID ( msg_id );

	TOutbox *pOutbox = cpNewsServer->GetOutbox ();

	pOutbox->SaveArticle (pHdr, &sBod);
}

void CNewsDoc::OnNewsgroupGetall()
{
	TServerCountedPtr cpNewsServer;
	extern CTime gTimeAncient;

	// we want to get Everything, so move check time back to 1972
	cpNewsServer->SetNewsGroupCheckTime( gTimeAncient );

	UtilRetrieveRecentNewsgroups (true,    // force get entire list
		TRUE,    // show subscribe dialog
		FALSE);  // popup empty list
}

void CNewsDoc::OnUpdateNewsgroupGetall(CCmdUI* pCmdUI)
{
	extern BYTE gfGettingNewsgroupList;
	// if not running, then enable it.
	// if running then disable it.
	pCmdUI->Enable (!gfGettingNewsgroupList);
}

// ------------------------------------------------------------------------
// Class wide function
// History:  this function used to do (PURGE-ALL) (FETCH-ALL) but this
//           would look as if Gravity was hung if it had to purge 200 groups..
//           So now it does purge & fetch 1 by 1.
//
void CNewsDoc::DocGetHeadersAllGroups (bool fForceRetrieveCycle, bool fUserAction)
{
	if (!gpTasker || !gpTasker->IsConnected())
		return;

	CStringArray       vNames;
	TServerCountedPtr  cpNewsServer;
	BOOL fCatchUp = gpGlobalOptions->GetRegSwitch()->GetCatchUpOnRetrieve();
	bool fFirstRetrieve   = true;

	// Sort them, so we process them in the same order as the
	//     CNewsView display

	cpNewsServer->GetSubscribedArray().GetSortedRetrieveArray ( true, &vNames );

	// loop thru list.  For each group, purge and then retrieve headers

	for (int i = 0; i < vNames.GetSize(); i++)
	{
		CString ngName = vNames[i];

		BOOL         fUseLock = FALSE;
		TNewsGroup * pNG = 0;
		bool         fActiveGroup = false;

		if (true)
		{
			TNewsGroupUseLock useLock (cpNewsServer, ngName, &fUseLock, pNG);

			if (fUseLock)
			{
				fActiveGroup = pNG->IsActiveGroup ();
				if (fActiveGroup)
				{
					if (fCatchUp)
						pNG->CatchUpArticles ();

					if (pNG->NeedsPurge ())
					{
						pNG->PurgeByDate (true /* fWriteOutGroup */);
					}
				}
			}
		}

		if (fUseLock && fActiveGroup && gpTasker)
		{
			if (fFirstRetrieve)
			{
				// rules --- reset any rule-based text-substitution
				ResetAllRuleSubstitution ();

				fFirstRetrieve = false;
			}

			// send it down to tasker,
			if (fForceRetrieveCycle)
				gpTasker->ForceRetrieveGroup (fUserAction ? TNewsTasker::kActionUser : TNewsTasker::kActionProgram,
				ngName);
			else
				gpTasker->StartRetrieveGroup (fUserAction ? TNewsTasker::kActionUser : TNewsTasker::kActionProgram,
				ngName);

		}

	} // for
}

// ------------------------------------------------------------------------
// Returns: 0 for success, non-zero for error
int CNewsDoc::HandleNewsUrl (CString * & rpstrNewsURL)
{
	TServerCountedPtr cpNewsServer;

	CString token = rpstrNewsURL->Left(5);

	if (token.CompareNoCase("news:"))
		return 1;
	token = rpstrNewsURL->Mid(5);

	delete rpstrNewsURL;
	rpstrNewsURL = NULL;

	// this should be the newsgroup name
	token.TrimLeft(); token.TrimRight();
	if (token.Find('@') != -1)
	{
		// the url indicates a specific message-id
		return 0;
	}
	else if (token.Find('*') != -1)
	{
		// the url specifies all newsgroups.  do nothing
		return 0;
	}
	else
	{
		CString & newsGroup = token;
		BOOL fUseLock;
		int  iGroupID = -1;
		TNewsGroup* pNG = 0;

		// maybe it is already subscribed
		{
			TNewsGroupUseLock useLock(cpNewsServer, newsGroup, &fUseLock, pNG);
			if (fUseLock)
			{
				// it's already subscribed
				iGroupID = pNG->m_GroupID;
			}
		}

		// is the group in our Big List?
		if (-1 == iGroupID)
		{
			if (ForceSubscribe (newsGroup, iGroupID))
			{
				AfxMessageBox("GroupNotFound");

				// group not in our Big List
				return 1;
			}
		}

		// open this newsgroup then

		if (iGroupID < 0)
			return 1;

		POSITION pos = GetFirstViewPosition ();
		while (pos)
		{
			CView * pView = GetNextView (pos);

			// ok I admit this is not very clean

			if (pView->IsKindOf(RUNTIME_CLASS(CNewsView)))
			{
				int stat = ((CNewsView*)pView)->SetSelectedGroupID (iGroupID);
				if (0 == stat)
					((CNewsView*)pView)->OpenNewsgroupEx (CNewsView::kFetchOnZero);
				return 0;
			}
		}
		return 1;
	}
}

// ------------------------------------------------------------------------
int CNewsDoc::ForceSubscribe (const CString & newsGroup, int & iGroupID)
{
	TServerCountedPtr cpNewsServer;
	TGroupList * pAllGroups = 0;

	try
	{
		int idx;
		// does it exist in our big list?
		pAllGroups = cpNewsServer->LoadServerGroupList ();

		TGlobalDef::ENewsGroupType eType = TGlobalDef::kPostAllowed;
		WORD        wNumArticles = 0;

		if (pAllGroups->GroupExist (newsGroup, &idx))
		{
			CString myName;
			pAllGroups->GetItem(idx, myName, wNumArticles, eType);

			TNewsGroup * pNG = cpNewsServer->SubscribeGroup (myName, eType,
				STORAGE_MODE_DEFAULT, 300);  // $$$

			// get the changes into the ui
			SubscribeGroupUpdate ();

			iGroupID = pNG->m_GroupID;

			idx = 0;
		}
		else
		{
			idx = 1;
		}

		delete pAllGroups;
		return idx;
	}
	catch(...)
	{
		delete pAllGroups;
		throw;
		return 1;
	}
}

// ------------------------------------------------------------------------
//
void CNewsDoc::SubscribeGroupUpdate ()
{
	TServerCountedPtr cpNewsServer;

	// incremental add to the master list
	gptrApp->BuildNewsgroupList ( TRUE, cpNewsServer, gpTasker );

	// do the get document update all views thing
	UpdateAllViews (NULL, VIEWHINT_RELOAD_NEWSGROUPS);
}

// --------------------------------------------------------------------------
// called by ctor/dtor, and after the user resets Global options.
void CNewsDoc::ConfigurePanicKey (BOOL fEnabled)
{
}

// ------------------------------------------------------------------------
//
void TObserverWatchesPump::UpdateObserver ()
{
	extern HWND ghwndMainFrame;

	if (0 == gpTasker)
		return;

	if (m_pSubj->GetSubjectState () == SUBJECT_CONNECTED)
	{
		if (!::PostMessage (ghwndMainFrame, WMU_CONNECT_OK, 0, 0L))
		{
			DWORD dwError = GetLastError ();
			CString msg; msg.Format (IDS_ERR_POSTMSGFAILED, __FILE__,
				__LINE__, dwError);

			NewsMessageBoxTimeout (30, CWnd::FromHandle(ghwndMainFrame),
				msg, MB_OK | MB_ICONWARNING);
		}
	}
}

// ------------------------------------------------------------------------
//
void CNewsDoc::getGroup (const CString & groupName)
{
	// cuz I don't want to include rules.h into vcrrun.cpp

	ResetAllRuleSubstitution ();

	gpTasker->ForceRetrieveGroup (TNewsTasker::kActionProgram, groupName);
}

// ------------------------------------------------------------------------
// Displaying connect errors USED to be the Tasker's job, but it's better
// if the display is handled completely by the UI thread.
//
void CNewsDoc::DisplayConnectError (HWND     hWndMainFrame,
									bool     fRelease,
									LPARAM   lParam)
{
	TPumpJobConnect * pJob = (TPumpJobConnect *) (void*) lParam;

	if (fRelease)
	{
		// caller wants us to free memory, that's all
		delete pJob;
		return;
	}

	TServerCountedPtr cpNewsServer;

	if (kClassNNTP == pJob->m_eErrorClass)
	{
		CString strWhen;

		strWhen.Format (IDS_ERR_CONNECT_FAIL1,
			(LPCTSTR) cpNewsServer->GetNewsServerAddress());

		int code = atoi(pJob->m_errorMessage);

		if (code)
			fnNNTPError (strWhen, code, pJob->m_errorMessage);
		else
			NewsMessageBoxTimeout( 30, CWnd::FromHandle(ghwndMainFrame), pJob->m_errorMessage,
			MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);

		PostMessage (ghwndMainFrame, WM_COMMAND, ID_FILE_SERVER_RECONNECTDELAY, 0);
	}
	else if (pJob->m_errorMessage.GetLength())
	{
		NewsMessageBoxTimeout(30, CWnd::FromHandle(ghwndMainFrame), pJob->m_errorMessage,
			MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);

		PostMessage (ghwndMainFrame, WM_COMMAND, ID_FILE_SERVER_RECONNECTDELAY, 0);
	}
	else if (kClassWinsock == pJob->m_eErrorClass)
	{
		CString strWinSockError;
		int iExplainID = 0;

		iExplainID = GetWinsockErrorID (pJob->m_dwErrorID);

		if (iExplainID)
			strWinSockError.LoadString ( iExplainID );

		// show  Error, When, Reason  dialog box
		TNNTPErrorDialog sDlg (CWnd::FromHandle (ghwndMainFrame));

		if (cpNewsServer != NULL)
			sDlg.m_strWhen.Format (IDS_ERR_CONNECT_FAIL1, (LPCTSTR) cpNewsServer->GetNewsServerAddress());
		else
			sDlg.m_strWhen = "Unknown";

		sDlg.m_strReason = strWinSockError;

		sDlg.m_fTimer = true;
		sDlg.m_iSeconds = 40;
		sDlg.DoModal ();

		PostMessage (ghwndMainFrame, WM_COMMAND, ID_FILE_SERVER_RECONNECTDELAY, 0);
	}

	// if user cancelled do nothing

	delete pJob;
}

// ------------------------------------------------------------------------
// Interface to OpenOffice.org MySpell spelling library
//
// Returns 0 if word is OK
//         1 if word is bad, plus suggestions
//         2 initialization failure
//
int CNewsDoc::CheckSpelling_Word (const CString & testWord, std::list<std::string> & suggestList)
{
	if (testWord.IsEmpty())
		return 0;
	if (NULL == m_pMS)
	{
		CString installDir;
		GetInstallDir( installDir );

		CString dict = gpGlobalOptions->GetRegSystem()->GetSpellingDictionary();
		CString aff  = gpGlobalOptions->GetRegSystem()->GetSpellingAffinity();

		TPath   p0(installDir, "spell");

		TPath df(p0, dict);  // combine directory + filename
		TPath af(p0, aff);

		m_pMS = new Hunspell ( af, df );
//		m_pMS = new MySpell ( af, df );
	}

	if (gpGlobalOptions->GetRegSystem()->GetSpelling_IgnoreNumbers())
	{
		bool fFoundLetter = false;
		for (int i=0; i < testWord.GetLength(); i++)
		{
			TCHAR c = testWord[i];
			if (('0' <= c && c <= '9') || ('.' == c) || (',' == c))
			{
				// pseudo digit
			}
			else
			{
				fFoundLetter = true;
				break;
			}
		}
		if (false == fFoundLetter)
			return 0;
	}

	// it could be in the user's customized word list
	if (m_pMyWords && m_pMyWords->Lookup( testWord))
		return 0;

	int ret = m_pMS->spell( testWord );
	if (ret)
		return 0;

	char ** slst;
	int numSuggestions = m_pMS->suggest( &slst, testWord );

	if (numSuggestions > 0)
	{
		for (int i = 0; i < numSuggestions; i++)
		{
			suggestList.push_back( string(slst[i]) );
			free( slst[i] );
		}
		free (slst);
	}
	return 1;
}

