/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newspump.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2009/08/27 15:29:22  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/18 22:05:02  richard_wood
/*  Refactored XOVER and XHDR commands so they fetch item data in batches of 300 (or less) if we want > 300 articles.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.6  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.5  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
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
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

// newspump.cpp : implementation file
//
//  download headers.  WinSock Error -
//               resubmit job
//

#include "stdafx.h"
#include <tchar.h>
#include "News.h"
#include "newspump.h"
#include "globals.h"
#include "server.h"                 // TNewsServer
#include "servcp.h"                 // TServerCountedPtr
#include "tmutex.h"
//#include "newsconn.h"
#include "tasker.h"
#include "tscribe.h"
#include "usrdisp.h"
#include "custmsg.h"
#include "fileutil.h"
#include "tglobopt.h"
#include "ecpcomm.h"
#include "rules.h"
#include "fetchart.h"
#include "timermgr.h"
#include "bits.h"
#include "evtlog.h"
#include "rgconn.h"
#include "enumjob.h"
#include "ngutil.h"
#include "autoprio.h"
#include "timeutil.h"
#include "rgswit.h"
#include "topexcp.h"                // TopLevelException()
#include "taglist.h"
#include "expire.h"                 // TExpiryData, TIntVector,
#include "tsubjctx.h"               // SUBJECT_::CONNECTED
#include "nglist.h"
#include "memspot.h"
#include "utilrout.h"               // PostMainWndMsg
#include <winperf.h>                // PerfMon stuff
#include "genutil.h"
#include "security.h"
#include "superstl.h"               // auto_ptr, ...
#include "utilerr.h"                // TYP_ERROR_ENTERGROUP
#include "tscoring.h"               // ScoreBody()
#include <sstream>
#include "utlmacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define PERFMON_STATS

extern TGlobalOptions* gpGlobalOptions;

PPERF_COUNTER_BLOCK pCounterBlock;

extern LONG  gl_Pump_Busy;                // in tasker.cpp
extern LONG  gl_Emergency_Busy;           // in tasker.cpp
extern BYTE  gfGettingNewsgroupList;

BOOL FAR WINAPI EmergencyPumpBlockingHook(void);
BOOL FAR WINAPI NormalPumpBlockingHook(void);

struct PUMPHOOKDATA
{
	HANDLE   hKillEvent;
	TPump *  psPump;
};

PUMPHOOKDATA   gsNormHookData;
PUMPHOOKDATA   gsPrioHookData;
TRETRY         vsRetry[2];          // a global , so it can outlive the pump

static int FindBlankLineIndex (CString& body, int& iEndHdr, int& iStartBody);

static void errorDialog (LPCSTR file, DWORD line, DWORD error, CString & strError);

#define ERRDLG(id,desc)  errorDialog(__FILE__, __LINE__, id, desc)

const static int kiPumpStatusBarRange = 45000;

UINT fnProcessNewsgroups (LPVOID pVoid);  // thread function

TPumpTimeKeeper  TPump::m_timeKeeper;     // track when to disconnect

/////////////////////////////////////////////////////////////////////////////
// C L A S S   T P u m p J o b s

/////////////////////////////////////////////////////////////////////////////
TPumpJobs::TPumpJobs()
{
	InitializeCriticalSection (&m_csProtect);

	m_bySacrifice  = 0xAC;
	m_hEventJobReady = CreateEvent ( NULL,       // no security
		TRUE,       // we want manual reset
		FALSE,      // start out unsignaled
		NULL );     // no name
}

/////////////////////////////////////////////////////////////////////////////
TPumpJobs::~TPumpJobs()
{
	m_bySacrifice = 0x00;
	cleanup_queue();

	CloseHandle (m_hEventJobReady);
	DeleteCriticalSection (&m_csProtect);
}

/////////////////////////////////////////////////////////////////////////////
// AddJob - add job to end of the queue. which is the normal case
void TPumpJobs::AddJob (TPumpJob * pJob)
{
	TEnterCSection sLock(&m_csProtect);

	m_Jobs.AddTail ( pJob );
	SetEvent ( m_hEventJobReady );
}

/////////////////////////////////////////////////////////////////////////////
// put Job at front of queue
void TPumpJobs::AddJobToFront (TPumpJob * pJob)
{
	TEnterCSection sLock(&m_csProtect);
	if (m_Jobs.IsEmpty())
		m_Jobs.AddHead ( pJob );
	else
	{
		// whatever you do, make sure you don't go ahead of the Connect Job!
		POSITION pos = m_Jobs.GetHeadPosition();
		TPumpJob * pHeadJob = m_Jobs.GetAt(pos);

		if (pHeadJob->GetType() == TPumpJob::kJobConnect)
		{
			// TRACE0("Saved by the bell!--->>>\n");
			m_Jobs.InsertAfter(pos, pJob);
		}
		else
			m_Jobs.InsertBefore(pos, pJob);
	}
	SetEvent ( m_hEventJobReady );
}

/////////////////////////////////////////////////////////////////////////////
// erase jobs with flag PJF_TAG_JOB
int TPumpJobs::EraseAllTagJobs ()
{
	bool fFound = false;
	TEnterCSection sLock(&m_csProtect);

	POSITION pos = m_Jobs.GetHeadPosition ();
	POSITION oldpos ;

	while (pos)
	{
		oldpos = pos;
		TPumpJob * pJob = m_Jobs.GetNext (pos);

		if (pJob->GetType() == TPumpJob::kJobArticle)
		{
			TPumpJobArticle * pJA = (TPumpJobArticle *) pJob;
			if (pJA->TestFlag (PJF_TAG_JOB))
			{
				m_Jobs.RemoveAt ( oldpos );
				delete pJA;
			}
		}
	}
	return 0; // success
}

/////////////////////////////////////////////////////////////////////////////
void TPumpJobs::GetJobCount (int& pumpJobs)
{
	pumpJobs = 0;

	if (m_bySacrifice != 0xAC)
		return;

	TEnterCSection sLock(&m_csProtect);
	pumpJobs = m_Jobs.GetCount();

#if defined(_DEBUG)
	if (pumpJobs > 0)
	{
		TPumpJob::EPumpJobType type = m_Jobs.GetHead()->GetType ();

	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
// when we disconnect, we need to weed out certain job types
//
void TPumpJobs::OnDisconnect ()
{
	TEnterCSection sLock(&m_csProtect);

	POSITION pos = m_Jobs.GetHeadPosition();
	POSITION prev;

	TPumpJob* pPumpJob;
	while (pos)
	{
		prev = pos;
		pPumpJob = m_Jobs.GetNext ( pos );

		if (TPumpJob::kJobBigList == pPumpJob->GetType ())
		{
			m_Jobs.RemoveAt (prev);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
TPumpJob * TPumpJobs::RemoveJob ()
{
	// if TPumpJobs has been deleted, this should throw
	//
	// basically, I want to test the waters before trying a fragile KERNEL function
	//  like EnterCriticalSection with an invalid critsect.
	if (m_bySacrifice != 0xAC)
		throw 1;

	TEnterCSection sLock(&m_csProtect);
	int n = m_Jobs.GetCount();
	if (0 == n)
	{
		ResetEvent ( m_hEventJobReady );
		return NULL;
	}
	else if (1 == n)
		ResetEvent ( m_hEventJobReady );
	return m_Jobs.RemoveHead ();
}

///////////////////////////////////////////////////////////////////////////
// RequestBody (Public) --  called by tasker to DL specific articles
//
//  note: the groupName is passed in as a CString, to take advantage of the
//           CString reference counting
//
void TPumpJobs::RequestBody (
							 const CString &   groupName,
							 LONG              groupID,
							 int               artInt,
							 int               iLines,
							 BOOL              fFrontOfQueue,
							 DWORD             dwFlags
							 )
{
	CString subject;
	CPoint  ptPartID(0,0);

	// order the Pump to download this message body
	TPumpJobArticle * pJob = new TPumpJobArticle(
		groupName,
		groupID,
		subject,
		ptPartID,
		artInt,
		iLines,
		dwFlags,
		NULL,
		NULL);

	if (fFrontOfQueue)
		AddJobToFront ( pJob );
	else
		AddJob ( pJob );
}

/////////////////////////////////////////////////////////////////////////////
// user is unsubscribing; kill pending jobs
int TPumpJobs::CancelNewsgroup(LPCTSTR groupName)
{
	int killCount = 0;

	TEnterCSection sLock(&m_csProtect);

	POSITION pos = m_Jobs.GetHeadPosition();
	POSITION prev;

	TPumpJob* pPumpJob;
	while (pos)
	{
		BOOL fKill = FALSE;
		prev = pos;
		pPumpJob = m_Jobs.GetNext ( pos );
		switch (pPumpJob->GetType())
		{
		case (TPumpJob::kJobGroup):
			{
				TPumpJobEnterGroup * pJobGroup = (TPumpJobEnterGroup*) pPumpJob;
				const CString& rName = pJobGroup->GetGroup();
				if (0 == rName.CompareNoCase(groupName))
					fKill = TRUE;
				break;
			}

		case (TPumpJob::kJobArticle):
			{
				TPumpJobArticle * pJob = (TPumpJobArticle*) pPumpJob;
				const CString& rName = pJob->GetGroup();
				if (0 == rName.CompareNoCase(groupName))
					fKill = TRUE;
				break;
			}

		case (TPumpJob::kJobOverview):
			TPumpJobOverview * pJobOverview = (TPumpJobOverview*) pPumpJob;
			const CString& rName = pJobOverview->GetGroup();
			if (0 == rName.CompareNoCase(groupName))
				fKill = TRUE;
			break;
		}

		if (fKill)
		{
			delete pPumpJob;
			m_Jobs.RemoveAt ( prev );
			++killCount;
		}
	} // while loop
	if (0 == m_Jobs.GetCount())
		ResetEvent(m_hEventJobReady);

	return killCount;
}

// ------------------------------------------------------------------------
// rules analyzes the header, and wants to kill the complete Article
//  chase down the Body

BOOL TPumpJobs::CancelArticleBody ( const CString& groupName, int artInt )
{
	return cancel_job_in_queue ( groupName, artInt, TPumpJob::kJobArticle );
}

// ------------------------------------------------------------------------
// case 2:  DEL key applied to tagged article that has been submitted.
BOOL TPumpJobs::CancelRetrieveJob ( const CString& groupName, int artInt )
{
	return cancel_job_in_queue ( groupName, artInt, TPumpJob::kJobArticle );
}

// ------------------------------------------------------------------------
//  only supports  TPumpJob::kJobArticle
//
BOOL TPumpJobs::cancel_job_in_queue (const CString& groupName, int artInt,
									 TPumpJob::EPumpJobType eType)
{
	ASSERT(TPumpJob::kJobArticle == eType);

	TEnterCSection sLock(&m_csProtect);
	POSITION oldPos;
	POSITION pos = m_Jobs.GetHeadPosition();
	TPumpJob* pPumpJob;
	while (pos)
	{
		oldPos = pos;
		pPumpJob = m_Jobs.GetNext ( pos );

		if (eType == pPumpJob->GetType())
		{
			switch (pPumpJob->GetType())
			{
			case (TPumpJob::kJobArticle):
				{
					TPumpJobArticle* pParts = (TPumpJobArticle*) pPumpJob;
					if (artInt == pParts->GetArtInt())
					{
						if (0==groupName.CompareNoCase( pParts->GetGroup() ))
						{
							m_Jobs.RemoveAt (oldPos);
							delete pParts;
							return TRUE;
						}
					}
				}
				break;
			default:
				ASSERT(FALSE);  // unhandled type
				break;
			}
		}
	}
	return FALSE;  // not found.  maybe already downloaded. hmmmm...
}

// ------------------------------------------------------------------------
// FetchBody -- Retrieve the Body for this Header.  Send both into CB function.
//     NoSave. This is called when a layer wants an article synchronously
BOOL TPumpJobs::FetchBody (
						   const CString&    groupName,
						   int               groupID,
						   const CString &   subject,
						   CPoint &           ptPartID,
						   int               artInt,
						   int               iLines,
						   TFetchArticle*    pFetchArticle,
						   BOOL              bMarkAsRead /* = TRUE */)
{
	ASSERT(pFetchArticle);
	ASSERT(groupName);

	DWORD dwJobFlags = 0;

	if (bMarkAsRead)
		dwJobFlags  |= PJF_MARK_READ;

	// order the Pump to download this message body
	TPumpJobArticle * pJob = new TPumpJobArticle(
		groupName,
		groupID,
		subject,
		ptPartID,
		artInt,
		iLines,
		dwJobFlags,
		NULL,         // pHeader
		pFetchArticle);

	AddJobToFront ( pJob );
	return TRUE;
}

// return True if we moved something
BOOL TPumpJobs::QueueReOrder (LPCTSTR groupName)
{
	TEnterCSection sLock(&m_csProtect);

	if (m_Jobs.IsEmpty())
		return FALSE;
	CTypedPtrList<CObList, TPumpJob*> SubJobs;
	POSITION oldPos;
	POSITION pos = m_Jobs.GetTailPosition();
	TPumpJob* pPumpJob;
	while (pos)
	{
		oldPos = pos;
		pPumpJob = m_Jobs.GetPrev ( pos );
		switch (pPumpJob->GetType())
		{
		case (TPumpJob::kJobArticle):
			{
				TPumpJobArticle* pParts = (TPumpJobArticle*) pPumpJob;
				if (0 == (pParts->GetGroup()).CompareNoCase(groupName))
				{
					SubJobs.AddHead ( pPumpJob );
					m_Jobs.RemoveAt ( oldPos );
				}
				break;
			}
		case (TPumpJob::kJobOverview):
			{
				TPumpJobOverview* pOver = (TPumpJobOverview*) pPumpJob;
				if (0 == (pOver->GetGroup()).CompareNoCase(groupName))
				{
					SubJobs.AddHead ( pPumpJob );
					m_Jobs.RemoveAt ( oldPos );
				}
				break;
			}
		} // switch
	} // while
	BOOL fEmpty = SubJobs.IsEmpty();

	// move the subgroup to the start
	m_Jobs.AddHead ( &SubJobs );

	return !fEmpty;
}

/////////////////////////////////////////////////////////////////////////////
// cleanup_queue
void TPumpJobs::cleanup_queue()
{
	TPumpJob* pJob;
	TEnterCSection sLock(&m_csProtect);

	while (m_Jobs.GetCount() > 0)
	{
		pJob = m_Jobs.RemoveHead ();
		delete pJob;
	}

	ResetEvent ( m_hEventJobReady );
}

/////////////////////////////////////////////////////////////////////////////
// TPumpExternalLink

TPumpExternalLink::TPumpExternalLink (TNewsTasker * pTasker,
									  const CString& strServer,
									  TPumpJobs * pJobs,
									  HANDLE         hScribeUnderCapacity,
									  bool           fEmergencyPump)
									  : m_pTasker(pTasker),  m_pJobs(pJobs), m_strServer(strServer)
{
	// note: send scribe jobs to pTasker

	m_iSocketError   = 0;
	m_iNNTPError     = 0;
	m_fUserStop      = false;

	// flag used when setting up  connection
	m_fPumpCtorFinished = false;

	m_hScribeUnderCapacity = hScribeUnderCapacity;
	m_fEmergencyPump       = fEmergencyPump;

	SetEvent (hScribeUnderCapacity);

	m_hKillRequest         = 0;
	m_fConnected           = false;  // set to true after password is accepted
}

// ------------------------------------------------------------------------
// Flesh out TSubject Interface
DWORD TPumpExternalLink::GetSubjectState ()
{
	// right now observers only care if we are totally fully connected
	return m_fConnected
		? SUBJECT_CONNECTED
		: SUBJECT_NO_INFO;
}

// ------------------------------------------------------------------------
TPumpExternalLink::~TPumpExternalLink()
{
	m_fConnected = false;

	// let observers know that we are disconnected
	Update_Observers ();

	RemoveAllObservers ();
}

/////////////////////////////////////////////////////////////////////////////
// C L A S S    T P u m p

/////////////////////////////////////////////////////////////////////////////
UINT TPump::ThreadFunction (LPVOID pParam)
{
	TPump thePump(pParam);

	UINT ret = thePump.Run ();

	// kludge : Try to ensure that cleanup happens before the
	//          entire app exits.  boost thread priority

	VERIFY( SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_ABOVE_NORMAL) );

	// automatic call to destructors

	return ret;
}

// ------------------------------------------------------------------------
// TPump ctor
TPump::TPump(LPVOID  pParam)
{
	m_pErrList = &m_sErrorList;

	m_pLink = (TPumpExternalLink *) pParam;

	// copy info ptrs
	m_fEmergencyPump = m_pLink->m_fEmergencyPump;
	m_pMasterTasker  = m_pLink->m_pTasker;

	// RLWTODO 2 : pump creates the kill request handle
	TRACE("TPump::TPump : Creating m_KillRequest event\n");
	// security, manual, init-state, name
	m_KillRequest = CreateEvent(NULL, TRUE, FALSE, NULL);   // Off by default

	m_fProcessJobs = true;

	if (m_fEmergencyPump)
	{
		gsPrioHookData.hKillEvent = m_KillRequest;
		gsPrioHookData.psPump = this;
	}
	else
	{
		gsNormHookData.hKillEvent = m_KillRequest;
		gsNormHookData.psPump = this;
	}

	// RLWTODO 3 : cpoies it back into the structure
	m_pLink->m_hKillRequest = m_KillRequest;

	TServerCountedPtr cpNewsServer;

	m_pFeeed  = new TNewsFeed ( cpNewsServer->GetNewsServerPort (),
		m_fEmergencyPump ? EmergencyPumpBlockingHook : NormalPumpBlockingHook,
		m_KillRequest,
		&m_fProcessJobs );

	if ( gpGlobalOptions->GetRegSwitch()->GetLogGroupCmds  ()  )
	{
		m_logFile = cpNewsServer->GetServerDatabasePath();
		m_logFile += "\\CMDGRP.TRC";
	}
	else
	{
		// m_logFile.Empty();
	}

	m_wServerCap = 0;

	m_fQuitNNTP = TRUE;

	// Does Server have XRef info embedded in the XOver info?
	//    this is from registry.
	if (cpNewsServer->XOverXRef())
		m_wServerCap |= kXRefinXOver;

	m_server = m_pLink->m_strServer;

	m_pLink->m_fPumpCtorFinished = true;
}

// ------------------------------------------------------------------------
// You must catch all exceptions in destructors. Otherwise you terminate
//
TPump::~TPump()
{
	try
	{
		CloseHandle (m_KillRequest);

		FreeConnection ();

		// created by tasker, but I free the memory
		if (m_pLink)
			delete m_pLink;

		//TRACE0("Pump: destructor is finished\n");
	}
	catch(...)
	{
		// catch everything
	}
}

// -------------------------------------------------------------------------
// Start normal operation
UINT TPump::Run(void)
{
	TRACE("TPump::Run >\n");
	DWORD ret;
	try
	{
		for (;;)
		{
			ret = CustomizedWait();
			if (0 == ret)    // kill request
				break;

			// Process lots of jobs
			SingleStep();
		}
	}
	catch (TSocketException *except)
	{
		return run_exception(TRUE, except);
	}
	catch (TException *except)
	{
		return run_exception(FALSE, except);
	}
	catch (CException* pCExc)
	{
		pCExc->ReportError();
		pCExc->Delete();

		// top-level exception handling
		LINE_TRACE();
		TopLevelException (kThreadDownload);
		pump_lastrites();
		throw;
	}
	catch(...)
	{
		// top-level exception handling
		LINE_TRACE();

		TopLevelException (kThreadDownload);
		pump_lastrites();
		throw;
	}

	TRACE("TPump::Run : Thread finished, tidying up before exiting\n");

	// This last chunk.  Suppose the server disconnected us.  Normal pump catches
	// an exception and dies. But the Emergency pump thinks everything is OK.
	try
	{
		// this is a controlled shutdown. Quit the NNTP session
		if (m_pFeeed && m_fQuitNNTP)
			m_pFeeed->Quit( m_pErrList );

		// destroy newsfeed before ras-hangup
		FreeConnection ();
	}
	catch (TException *pE)
	{
		pE->Delete();
		// catch problems with Quit().
		LINE_TRACE();
		return pump_lastrites();
	}
	catch (CException *pE)
	{
		pE->Delete();
		// catch problems with Quit().
		LINE_TRACE();
		return pump_lastrites();
	}
	catch(...)
	{
		// top-level exception handling
		LINE_TRACE();
		TopLevelException (kThreadDownload);
		pump_lastrites();
		throw;
	}

	// RLWTODO 9 : a flagged killrequest should finally stop the pump thread and end up here
	// system code will call ExitInstance() && Delete()
	TRACE("TPump::Run <\n");
	return pump_lastrites();
}

// ------------------------------------------------------------------------
// ok this is the final gasp
BOOL TPump::pump_lastrites()
{
	return FALSE;
}

//-------------------------------------------------------------------------
// process TSocketExceptions & TExceptions here
//
int TPump::run_exception(BOOL fSocket, TException* pe)
{
	BOOL fKillRequest = (WAIT_OBJECT_0 == WaitForSingleObject(m_KillRequest, 0));

	// if the e-pump has a socket problem, let the normal pump continue on.
	if (fSocket && m_fEmergencyPump)
	{
		// if the user did not initiate this problem, display an error
		if (!fKillRequest)
		{
			CString strError;  strError.LoadString (IDS_ERR_CONN2);
			gpEventLog->AddError (TEventEntry::kPump, strError);
		}

		// go ahead and call delete
		return pump_lastrites();
	}

	// what's the matter?
	if (fKillRequest)
	{
		// we are shutting down. We must have done CancelBlockingCall.
		//TRACE0("TNewsPump::InitInstance shutting down\n");
		return pump_lastrites();  // go ahead and call delete
	}
	else
	{
		// this is a real error
		pe->Display();
		return pump_lastrites();  // go ahead and call delete
	}
}

// ------------------------------------------------------------------------
// Wait for
//      ScribeCanHandleMore && JobReady
//      Kill Request
//
int  TPump::CustomizedWait()
{
	DWORD  dwRet;

	if (false == m_fProcessJobs)
	{
		// since we had an error, we are just waiting around to be killed.

		dwRet = WaitForSingleObject (m_KillRequest, 60000);  // 60 seconds

		// kill request
		return 0;
	}

	TServerCountedPtr cpNewsServer;
	HANDLE rSetOne[2];
	HANDLE rSetTwo[2];
	BOOL   fKeepAlive = cpNewsServer->GetSendKeepAliveMsgs();

	rSetOne[0] = m_KillRequest;
	rSetOne[1] = m_pLink->m_hScribeUnderCapacity;

	rSetTwo[0] = m_KillRequest;
	rSetTwo[1] = m_pLink->m_pJobs->m_hEventJobReady;

	dwRet = WaitForMultipleObjects (2, rSetOne, FALSE, INFINITE);
	if (dwRet - WAIT_OBJECT_0 == 0)
	{
		// RLWTODO 6 : a flagged killrequest should end up here
		TRACE("TPump::CustomizedWait : Received m_KillRequest event (exit 1) <\n");
		// kill request
		return 0;
	}
	else
	{
		// scribe is ready to handle more data

		int   minutes = 1;
		const DWORD waitFor = 60000;  // one minute

		//   wait for a KillRequest, scribe-trouble or a New Job
		if (fKeepAlive)
		{
			minutes = cpNewsServer->GetKeepAliveMinutes();
		}
wait_more:

		dwRet = WaitForMultipleObjects (2, rSetTwo, FALSE, waitFor);
		if (dwRet - WAIT_OBJECT_0 == 0)
		{
			// RLWTODO 7 : or here
			TRACE("TPump::CustomizedWait : Received m_KillRequest event (exit 2) <\n");
			// kill request
			return 0;
		}
		else if (dwRet - WAIT_OBJECT_0 == 1)
		{
			// pump job is ready
			return 1;
		}
		else if (WAIT_TIMEOUT != dwRet)
		{
			// some error
			return 0;
		}
		else // (WAIT_TIMEOUT == dwRet)
		{
			// one minute has passed
			BOOL fJob = FALSE;

			// Only the normal pump alone initiates auto-disconnect
			//  - Enough time must have elapsed
			//  - the email thread must be idle
			if (!m_fEmergencyPump && m_timeKeeper.IsSignaled() &&
				!m_pMasterTasker->IsSendingEmail())
			{
				// start our own demise. fIdleDisconnect, fReconnect
				PostDisconnectMsg (true, false);
			}
			else
			{
				if (fKeepAlive && (--minutes <= 0))
				{
					fJob = TRUE;
					m_pLink->m_pJobs->AddJob (new TPumpJobHelp);
					minutes = cpNewsServer->GetKeepAliveMinutes();
				}
			}

//#if defined(_DEBUG)
//			TRACE1("%s pump - one minute cycle\n", m_fEmergencyPump ? "E" : "Norm");
//#endif
			if (fJob)
				return 2;

			goto wait_more;
		}
	}
}

// ------------------------------------------------------------------------
void TPump::SingleStep (void)
{
	TPumpJob * pOneJob = NULL;

	try
	{
		pOneJob = m_pLink->m_pJobs->RemoveJob();
	}
	catch (int)
	{
		// has m_pJobs been deleted ?? !!
		pOneJob = NULL;
	}

	if (NULL == pOneJob)
		return;

	LONG& busy_flag = m_fEmergencyPump ? gl_Emergency_Busy : gl_Pump_Busy;

	InterlockedIncrement ( &busy_flag );

	try
	{
		VERIFY(_CrtCheckMemory());

		SingleStepDispatch ( pOneJob );

		// allow MFC to free Temp maps, DLLS, etc...

		VERIFY(_CrtCheckMemory());
	}
	catch(...)
	{
		InterlockedDecrement ( &busy_flag );
		throw;
	}

	InterlockedDecrement ( &busy_flag );
}

// ------------------------------------------------------------------------
void TPump::SingleStepDispatch (TPumpJob * pOneJob)
{
	BOOL fLogActivity = TRUE;

	if (TPumpJob::kJobHelp == pOneJob->GetType())
		fLogActivity = FALSE;

	if (fLogActivity)
		m_timeKeeper.SetActivity ();

	switch (pOneJob->GetType())
	{
	case (TPumpJob::kJobPingMultiGroup):
		{
			// done.
			PingMultiGroup ( pOneJob );
			break;
		}

		//case (TPumpJob::kJobPingGroupMode1):
	case (TPumpJob::kJobPingGroup):
	case (TPumpJob::kJobGroup):
		{
			// done.
			CheckNewArticles ( pOneJob );
			break;
		}

	case (TPumpJob::kJobArticle):
		{
			//TRACE0("start DownloadArticle\n");
			// done.
			DownloadArticle ( pOneJob );
			break;
		}

	case (TPumpJob::kJobPost):
		{
			// done.
			PostOneMessage ( pOneJob );
			break;
		}

	case (TPumpJob::kJobOverview):
		{
			// not done!
			DownloadOverview ( pOneJob );
			break;
		}

	case (TPumpJob::kJobHelp):
		{
			// done.
			DoHelpCommand ( pOneJob );
			break;
		}

	case (TPumpJob::kJobConnect):
		{
			// not done!
			DoConnect ( pOneJob );
			break;
		}

	case (TPumpJob::kJobBigList):
		{
			// not done!
			DoGetNewsgroupList ( pOneJob );
			break;
		}
	case (TPumpJob::kJobPingArticles):
		{
			// done.
			DoPingArticles ( pOneJob );
			break;
		}
	} // switch

	if (fLogActivity)
		m_timeKeeper.SetActivity ();
}

// ------------------------------------------------------------------------
// CheckNewArticles - check newsgroup for new articles, send job data
//                    back to tasker.
//
// Returns 0 for success, 1 for error. (which means stop processing)
// Handles
//    TPumpJob::kJobPingGroupMode1
//    TPumpJob::kJobPingGroup
//    TPumpJob::kJobGroup
int  TPump::CheckNewArticles (TPumpJob * pBaseJob)
{
	FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);

	TPumpJobPingGroup * pJob = static_cast<TPumpJobPingGroup*>(pBaseJob);

//#ifdef _DEBUG
//	afxDump << "pump: Checking " << pJob->GetGroup() << "\n";
//#endif

	sSG.lpszNewsGroup = pJob->GetGroup();

	if (0 == m_pFeeed->SetGroup ( m_pErrList, &sSG ))
	{
		pJob->SetGroupResults ( sSG.fOK, sSG.iRet, sSG.first, sSG.last );

		// $$ reset this
		pJob->m_NewsGroup = sSG.strGroupName;
		m_CurrentGroup = pJob->GetGroup();

		m_pMasterTasker->AddResult ( pJob );

		return 0;     // success
	}

	DWORD   dwErrorID;
	CString strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	if (kClassWinsock == eErrorClass)
	{
		//  this is not a Job that is important enough to resubmit
		if (WSAEINTR == dwErrorID)
			PostDisconnectMsg (false, false);
		else
			// disconnect and reconnect
			PostDisconnectMsg (false, true);

		return 1;
	}

	if (kClassNNTP == eErrorClass)
	{
		TCHAR rcNum[10]; _itot(sSG.iRet, rcNum, 10);

		// no such news group
		CString desc; AfxFormatString1(desc, IDS_WARN_1NOSUCHNG, sSG.lpszNewsGroup);
		CString extDesc;
		sSG.strAck.TrimRight();
		AfxFormatString2(extDesc, IDS_WARN_2NOSUCHNGEXT, rcNum, (LPCTSTR) sSG.strAck);
		gpEventLog->AddWarning (TEventEntry::kPump, desc, extDesc);

		// present log window to user
		PostMainWndMsg (WM_COMMAND, ID_VIEW_EVENTLOG);

		m_CurrentGroup.Empty();
		pJob->SetGroupResults ( FALSE, sSG.iRet, 0, 0 );
		pJob->m_iRet = sSG.iRet;
		pJob->m_Ack  = sSG.strAck;

		m_pMasterTasker->AddResult ( pJob );

		return 0;  // continue processing
	}

	ERRDLG(dwErrorID, strError);

	return 0;
}

// ------------------------------------------------------------------------
// iSyncGroup -- changes newsgroup.  more of a utility function
// Returns 0 for success. Caller does the serious error handling
//
int  TPump::iSyncGroup (const CString& rGroup, FEED_SETGROUP * pSG)
{
	// must enter group
	pSG->fOK           = FALSE;
	pSG->lpszNewsGroup = rGroup;

	if (0 == m_CurrentGroup.CompareNoCase( rGroup ))
		return 0;

	if (0 == m_pFeeed->SetGroup ( m_pErrList, pSG ))
	{
		m_CurrentGroup = pSG->strGroupName;

		return 0;
	}
	else
		return 1;
}

////////////////////////////////////////////////////////////////////////////
// DownloadArticle -- returns 0 for success
//
int  TPump::DownloadArticle(TPumpJob * pBaseJob)
{
	auto_ptr<CObject> job_deleter(pBaseJob);
	TPumpJobArticle * pJob = (TPumpJobArticle *) pBaseJob;
	TArticleText * pText = 0;
	TUserDisplay_Auto refresher(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump
		? TUserDisplay_Auto::kPriority
		: TUserDisplay_Auto::kNormal);

	TServerCountedPtr cpNewsServer;
	TNewsGroup * pNG = 0;
	BOOL fUseLock;
	TNewsGroupUseLock useLock (cpNewsServer, pJob->GetGroupID(), &fUseLock, pNG);
	BOOL fOK = FALSE;

	if (!fUseLock)
	{
		TError sErr (kError, kClassUser, PUMP_GROUP_UNSUBSCRIBED);
		m_pErrList->PushError (sErr);
	}
	else
		do
		{
			//  check for unsubscribe
			//  enter Group
			//  setup status line
			if (0 != dl_body_start( pNG, pJob ))
				break;

			pText = new TArticleText;

			// sub-function for added readability
			if (download_body_util (pJob, pText))
				break;

			dl_body_success ( pNG, pJob, pText );

			pText = 0;
			return 0;

		} while (false);

		// handle errors here

		DWORD    dwErrorID;
		CString  strError;

		EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);
		switch (eErrorClass)
		{
		case kClassWinsock:
			{
				bool fResubmit;
				dl_body_fail_winsock (pJob, dwErrorID, strError, fResubmit);
				if (fResubmit)
					job_deleter.release();
				break;
			}

		case kClassUser:
			dl_body_fail_user (pJob, dwErrorID, strError);
			break;

		case kClassNNTP:
			dl_body_fail_nntp (pNG, pJob, dwErrorID, strError);
			break;

		default:
			ASSERT(0);
			break;
		}

		delete pText;

		return 1;
}

///////////////////////////////////////////////////////////////////////////
// download_body_util
//
//   returns 0 for success
//           1 for error
//
int  TPump::download_body_util(
							   TPumpJobArticle * pJob,
							   TArticleText *    pText )
{
	pText->SetNumber ( pJob->GetArtInt() );
	BOOL fOK = FALSE;
	CString& bod = pText->AccessBody();
	int iRetryCount = 0;
	CString  ack;
	int r;

ask_utwice:

	// use the article command to get the Full header and Body simultaneously
	BOOL fShowREDStatus = m_fEmergencyPump;

	r = m_pFeeed->Article (m_pErrList, pJob->GetArtInt(), bod, ack, fOK, pJob->GetBodyLines(),
		fShowREDStatus);
	if (r)
	{
		DWORD   dwErrorID;
		CString strError;
		EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

		if (eErrorClass == kClassNNTP && 412 == dwErrorID &&
			iRetryCount++ == 0)
		{
			// special processing for some stupid servers

			// no newsgroup has been selected
			m_CurrentGroup.Empty ();

			m_pErrList->ClearErrors ();

			FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);
			r = iSyncGroup (pJob->GetGroup(), &sSG);

			if (0 == r)
				goto ask_utwice;
			return 1;
		}
		else
		{
			if (FALSE == ack.IsEmpty())
				gpEventLog->AddError (TEventEntry::kPump, ack);

			return 1;
		}
	}

	// chop this article up

	int iEndHdr = 0;
	int iStartBody = 0;

	int ln = bod.GetLength();

	// separate the Header from the Body
	if (0 == FindBlankLineIndex (bod, iEndHdr, iStartBody))
	{
		LPTSTR both = bod.GetBuffer(ln);
		*(both + iEndHdr) = 0;       // cap off the header part

		pText->SetHeader(both);            // set the full header

		// want to shift the body part up.
		MoveMemory(both, both + iStartBody, (ln - iStartBody)*sizeof(TCHAR));
		bod.ReleaseBuffer( ln - iStartBody );
	}
	else
	{
		// Can't find blank line.  Treat this as all Header/Zero-Body

		LPTSTR both = bod.GetBuffer (ln);

		pText->SetHeader(both);

		bod.ReleaseBuffer (0);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//  do not delete the PumpJob. The caller will
//
//
//  custom errors:    kClassUser
//                    PUMP_ARTICLE_INDB
//
int TPump::dl_body_start (TNewsGroup * pNG, TPumpJobArticle*& pJob)
{
	// check status of article.  The emergency pump may have sneaked it in
	BOOL fHave = pNG->TextRangeHave (pJob->GetArtInt());

	// pre-check if other pump got it
	if (fHave)
	{
		TError sErr(kError, kClassUser, PUMP_ARTICLE_INDB);
		m_pErrList->PushError (sErr);

		return 1;
	}

	FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);

	if (iSyncGroup ( pJob->GetGroup(), &sSG ))
		return 1;

	CString status = pJob->FormatStatusString ();
	int     iLines = pJob->GetBodyLines();

	gpUserDisplay->SetText ( status, m_fEmergencyPump );
	gpUserDisplay->SetRange( 0, iLines, m_fEmergencyPump );

	return 0;
}

// ------------------------------------------------------------------------
void TPump::dl_body_success (TNewsGroup * pNG, TPumpJobArticle*& pJob,
							 TArticleText* pText)
{
	TPrepBody* pBody = 0;      // job for the scribe

	TFetchArticle * pFetch = pJob->ReleaseFetchObject ();
	bool fTag              = pJob->TestFlag (PJF_TAG_JOB);

	if (pJob->TestFlag (PJF_MARK_READ))
		pNG->StatusMarkRead (pJob->GetArtInt());

	if (pJob->TestFlag (PJF_SEND_SCRIBE) && pFetch)
	{
		// make a copy for the UI
		TArticleText * pTxt2 = new TArticleText;
		*pTxt2 = *pText;

		// send to the APP.  app takes ownership of pFetch pointer
		pFetch->JobSuccess ( pTxt2 );
		pFetch = 0;

		// pass original off to the scribe
		pBody = new TPrepBody ( pJob->GetGroup(), pText,
			pJob->GetArtInt(),
			fTag ? kJobPartsTag : kJobPartsCache);

		pText = NULL; // now owned by pBody;

		ScribeJob ( pBody );
		return;
	}
	else if (pJob->TestFlag (PJF_SEND_SCRIBE) && NULL == pFetch)
	{
		// normal case goes straight to the scribe
		pBody = new TPrepBody  ( pJob->GetGroup(), pText,
			pJob->GetArtInt(),
			fTag ? kJobPartsTag : kJobFlagsNone);

		pText = NULL; // now owned by pBody;

		ScribeJob ( pBody );

		return;
	}
	else if (pFetch)
	{
		// we are not saving the body. Can give Original back to the UI

		// - we are skipping the scribe, but we gotta let rules
		//   have a crack at it

		TArticleHeader * pHeader = pJob->GetArticleHeader();

		if (pHeader)
		{
			EvalRulesHaveBody (pHeader, pText, pNG);
			EvalRulesHaveBody_Committed (pHeader, pText, pNG);

			// also, let the scoring system evaluate the body
			ScoreBody (pHeader, pText, pNG);
		}
		pFetch->JobSuccess ( pText );
	}

	return ;
}

// ------------------------------------------------------------------------
int TPump::dl_body_fail_winsock (
								 TPumpJobArticle * pJob,
								 DWORD             dwError,
								 CString &         strError,
								 bool    &         fResubmit )
{

	fResubmit = false;
	TFetchArticle * pFetch = pJob->ReleaseFetchObject ();

	if (WSAEINTR == dwError)
	{
		// interrupted by user

		if (pFetch)
			pFetch->JobFailed (kClassWinsock, dwError, strError);

		// do no action for
		//     PJF_SEND_UPPERLAYER
		//     PJF_SEND_SCRIBE
		//     PJF_TAG_JOB

		PostDisconnectMsg (false /* fIdleDisconnect */, false /* fReconnect */ );
	}
	else
	{
		TServerCountedPtr cpNewsServer;
		BOOL fRetry       = cpNewsServer->GetRetrying();
		int  iRetryCount  = cpNewsServer->GetRetryCount();

		bool fMyReconnect = true;

		// handles blocking and send_to_ui
		if (pFetch)
			pFetch->JobFailed (kClassWinsock, dwError, strError);

		else
		{
			//     PJF_TAG_JOB, PJF_SEND_SCRIBE
			TRETRY * psRetry = &vsRetry[0];
			if (m_fEmergencyPump)
				psRetry = &vsRetry[1];

			// see if we can rtry job
			if ( fRetry && psRetry->CanRetryJob ( (DWORD) pJob, iRetryCount ) )
			{

				// re-do async job
				resubmit_job ( pJob );     // #1
				fResubmit = true;

			}
			else
				fMyReconnect = false;
		}

		int iSleepSecs = 0;

		// disconnect and reconnect
		PostDisconnectMsg (false, fMyReconnect, iSleepSecs);
	}
	return 0;
}

// ------------------------------------------------------------------------
int TPump::dl_body_fail_user (
							  TPumpJobArticle * pJob,
							  DWORD             dwError,
							  CString &         strError)
{
	TServerCountedPtr cpNewsServer;

	// handle blocking, UI, tscribe
	TFetchArticle * pFetch = pJob->ReleaseFetchObject ();

	if (pFetch)
		pFetch->JobFailed (kClassUser, dwError, strError);   // $ ui should ignore

	// do nothing for  PJF_SEND_SCRIBE

	if (pJob->TestFlag (PJF_TAG_JOB))
	{
		TPersistentTags & rTags = cpNewsServer->GetPersistentTags();
		rTags.DeleteTagCancelJob (pJob->GetGroup(), pJob->GetGroupID(),
			pJob->GetArtInt(), FALSE);
		cpNewsServer->SavePersistentTags ();
	}

	return 0;
}

// ------------------------------------------------------------------------
int TPump::dl_body_fail_nntp (
							  TNewsGroup *      pNG,
							  TPumpJobArticle * pJob,
							  DWORD             dwError,
							  CString &         strError)
{
	// handle blocking, UI, tscribe
	TFetchArticle * pFetch = pJob->ReleaseFetchObject ();

	// release blocking thread, or
	//  return error to UI
	if (pFetch)
		pFetch->JobFailed (kClassNNTP, dwError, strError);

	if (pJob->TestFlag (PJF_SEND_SCRIBE))
		pNG->StatusBitSet (pJob->GetArtInt(), TStatusUnit::kSendErr, TRUE);  // pink-X

	if (pJob->TestFlag (PJF_TAG_JOB))
	{
		TServerCountedPtr cpNewsServer;

		TPersistentTags & rTags = cpNewsServer->GetPersistentTags();
		rTags.DeleteTagCancelJob (pJob->GetGroup(), pJob->GetGroupID(),
			pJob->GetArtInt(), FALSE);
		cpNewsServer->SavePersistentTags ();
	}
	return 0;
}

//-------------------------------------------------------------------------
int  TPump::PostOneMessage(TPumpJob * pBaseJob)
{
	CString post_ack;
	TPumpJobPost * pJob = (TPumpJobPost *) pBaseJob;

	BOOL fOK = FALSE;
	int  iMarkRet = 0;

	int  r = m_pFeeed->PostArticle (m_pErrList, pJob->GetFile(),
		pJob->m_groupName, fOK,
		post_ack);

	TServerCountedPtr cpNewsServer;

	TOutbox* pOutbox = cpNewsServer->GetOutbox ();
	LONG lArtInt;  pJob->GetJobID (lArtInt);

	pJob->SetPostResult ( fOK );

	if (0 == r)
	{
		if (pOutbox)
			iMarkRet = (*pOutbox)->Mark (lArtInt, TStatusUnit::kSent);

		// save outbox
		cpNewsServer->SaveOutboxState ();

		// pass back result, good or bad.
		m_pMasterTasker->AddResult ( pJob );

		return 0;      // success
	}

	DWORD   dwErrorID;
	CString strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	switch (eErrorClass)
	{
	case kClassWinsock:
		if (WSAEINTR == dwErrorID)
		{
			if (pOutbox)
				iMarkRet = (*pOutbox)->Mark (lArtInt, TStatusUnit::kSendErr);

			PostDisconnectMsg (false, false);
		}
		else
		{
			// back to waiting
			if (pOutbox)
				iMarkRet = (*pOutbox)->Mark (lArtInt, TStatusUnit::kOut);

			PostDisconnectMsg (false, true);
		}

		// save outbox
		cpNewsServer->SaveOutboxState ();

		// pass back result, good or bad.
		m_pMasterTasker->AddResult ( pJob );

		break;

	case kClassNNTP:
		{
			CString err;
			int iPostRet = _ttoi(post_ack);
			err.Format(IDS_ERR_POSTING_FAILED, (LPCTSTR) pJob->m_groupName,
				iPostRet);
			post_ack.TrimRight();
			gpEventLog->AddShowError (TEventEntry::kPump, err, post_ack);

			if (pOutbox)
				iMarkRet = (*pOutbox)->Mark (lArtInt, TStatusUnit::kSendErr);

			// save outbox
			cpNewsServer->SaveOutboxState ();

			// pass back result, good or bad.
			m_pMasterTasker->AddResult ( pJob );
		}
		break;

	default:
		ASSERT(0);
		break;

	} // switch

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// DownloadOverview -- download headers, get xpost info, expire articles
//
//
void TPump::DownloadOverview (TPumpJob * pBaseJob)
{
	TServerCountedPtr cpNewsServer;
	TPumpJobOverview * pJob = static_cast<TPumpJobOverview*>(pBaseJob);
	auto_ptr<CObject> deleter(pJob);
	int low, high, i, rangeCount;
	int grandTotal = 0;
	TRangeSet * pRangeSet;
	FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);
	BOOL fGetBodies = FALSE;
	DWORD    dwErrorID;
	CString  strError;

	int r;

	// Sync Group can be sloo-ow, so do status bar immediately

	// turns on automatic refreshing of statusbar - clear when destruct
	TUserDisplay_Auto oRefresher(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump
		? TUserDisplay_Auto::kPriority
		: TUserDisplay_Auto::kNormal);

	// we need more control over that status line for the X-Refs
	CString mesg1;
	if (pJob->GetGroup().IsEmpty())
		mesg1.Format(IDS_UTIL_ENTERGRP, "");
	else
		mesg1.Format(IDS_UTIL_ENTERGRP, LPCTSTR(pJob->GetGroup()));

	gpUserDisplay->SetText (mesg1, m_fEmergencyPump);
	gpUserDisplay->SetRange (0, kiPumpStatusBarRange, m_fEmergencyPump);

	// must enter group
	r = iSyncGroup (pJob->GetGroup(), &sSG);

	if (r)
	{
		EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

		switch (eErrorClass)
		{
		case kClassWinsock:
			{
				BOOL fRetry       = cpNewsServer->GetRetrying();
				int  iRetryMax    = cpNewsServer->GetRetryCount();

				bool fMyReconnect = true;

				TRETRY * psRetry = &vsRetry[0];
				if (m_fEmergencyPump)
					psRetry = &vsRetry[1];

				if ( fRetry && psRetry->CanRetryJob ( (DWORD) pJob, iRetryMax ))
				{

					resubmit_job (pJob); // #2
					deleter.release();   // we resubmit job, so don't delete ptr
				}
				else
					fMyReconnect = false;

				if (WSAEINTR==dwErrorID)
					fMyReconnect = false;

				int iSleepSecs = 0;

				PostDisconnectMsg (false, fMyReconnect, iSleepSecs);
				break;
			}
		case kClassNNTP:
			{
				// error should have been handled by EnterGroup
			}
		default:
			ASSERT(0);
			break;
		}

		return;
	}

	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, pJob->GetGroupID(), &fUseLock, pNG);

		if (!fUseLock)
			return;

		// values still not set
		if (-1 == sSG.first && -1 == sSG.last)
		{
			int sblow, sbhi;
			pNG->GetServerBounds(sblow, sbhi);
			sSG.first = sblow; sSG.last = sbhi; sSG.est = sbhi - sblow + 1;
		}

		// mode 3 newsgroups get the bodies after getting headers
		fGetBodies = (TNewsGroup::kStoreBodies == UtilGetStorageOption(pNG));
	}

	int iLumpTotal = pJob->GetRangeSet()->CountItems();
	int iPingTotal = 0;
	if (pJob->GetPingRangeSet())
		iPingTotal = pJob->GetPingRangeSet()->CountItems();

	grandTotal = iLumpTotal + iPingTotal;

	if (0 == grandTotal)
	{
		pJob->Abort();
		TExpiryData *pExpiry = new TExpiryData(!pJob->GetExpirePhase(),
			pJob->GetGroupID(),
			pJob->GetExpireLow());
		ExpireHeadersNotFound (pExpiry);

		// send back an 'event' for the VCR window
		PostMainWndMsg (WMU_VCR_GROUPDONE);

		return ;               // avoid divide-by-zero too!
	}

	if (pJob->GetGroup().IsEmpty())
		AfxFormatString1 (mesg1, IDS_DOWNLOAD_OVERVIEW, _T(""));
	else
		AfxFormatString1 (mesg1, IDS_DOWNLOAD_OVERVIEW, (LPCTSTR) pJob->GetGroup());

	gpUserDisplay->SetText (mesg1, m_fEmergencyPump);

	TArtHdrArray * pHdrArray = new TArtHdrArray;
	try
	{
		int iMul = 0;
		int iLogicalRange;
		int iLogicalPos = 0;
		if (0 == iLumpTotal)
		{
			iLogicalRange = kiPumpStatusBarRange;
			POINT ptLogical;
			ptLogical.x = 0;
			ptLogical.y = iLogicalRange;
			ping_articles( pJob->GetPingRangeSet(),
				pJob->GetExpirePhase(),
				pJob->GetGroupID(),
				pJob->GetExpireLow(),
				grandTotal, &ptLogical, sSG.est, FALSE );
		}
		else if (0 == iPingTotal || pJob->GetExpirePhase()==false)
		{
			MPBAR_INFO sBarInfo;

			sBarInfo.m_iLogPos = 0;
			sBarInfo.m_iTotalLogRange = iLogicalRange = kiPumpStatusBarRange;

			sBarInfo.m_nGrandTotal = iLumpTotal;

			pRangeSet = pJob->GetRangeSet();
			int nSubTotal = 0;
			for (rangeCount = pRangeSet->RangeCount(), i = 0; i < rangeCount; ++i)
			{
				pRangeSet->GetRange( i, low, high );

				// fill pHdrArray
				sBarInfo.m_nSubRangeLen = high - low + 1;
				sBarInfo.m_iLogSubRangeLen = MulDiv (iLogicalRange , sBarInfo.m_nSubRangeLen,
					iLumpTotal);

				if (straight_overview( pJob, low, high, &sBarInfo, pHdrArray ))
				{
					for (int k = 0; k < pHdrArray->GetSize(); k++)
						delete pHdrArray->GetAt(k);
					delete pHdrArray;
					return;
				}

				// track subtotal
				nSubTotal += sBarInfo.m_nSubRangeLen;

				// do math, check for overflow
				iMul = MulDiv (iLogicalRange, nSubTotal, iLumpTotal);

				// assignment, not increment
				if (-1 != iMul)
					sBarInfo.m_iLogPos = iMul;
			}

			// even if iPingTotal is zero, we should still call ping_articles.
			//  this takes care of the case when the NewsGroup object has
			//   articles 1-5 and the server now has 7-10. (We must kill 1-5)
			POINT ptLogical; ptLogical.x = 0; ptLogical.y = 0;
			ping_articles (pJob->GetPingRangeSet(),
				pJob->GetExpirePhase(),
				pJob->GetGroupID(),
				pJob->GetExpireLow(),
				grandTotal, &ptLogical, sSG.est, FALSE);
		}
		else
		{
			POINT ptLogical;
			MPBAR_INFO sBarInfo;

			// overview operates on half the range
			sBarInfo.m_iLogPos = 0;
			sBarInfo.m_iTotalLogRange = kiPumpStatusBarRange;
			sBarInfo.m_nGrandTotal = grandTotal;

			pRangeSet = pJob->GetRangeSet();
			rangeCount = pRangeSet->RangeCount();

			// grandTotal consists of arbitrary fraction M + N
			for (i = 0; i < rangeCount; ++i)
			{
				pRangeSet->GetRange( i, low, high );

				sBarInfo.m_nSubRangeLen = high - low + 1;
				sBarInfo.m_iLogSubRangeLen = MulDiv (sBarInfo.m_iTotalLogRange , sBarInfo.m_nSubRangeLen,
					grandTotal);

				if (straight_overview( pJob, low, high, &sBarInfo, pHdrArray ))
				{
					for (int k = 0; k < pHdrArray->GetSize(); k++)
						delete pHdrArray->GetAt(k);
					delete pHdrArray;
					return;
				}

				// do math, check for overflow
				iMul = MulDiv(sBarInfo.m_iTotalLogRange, high - low + 1, grandTotal);
				if (-1 != iMul)
					sBarInfo.m_iLogPos += iMul;

				// StatusBar: M  / (M+N) is done
			}

			// run the remainder of our work  (N)
			ptLogical.x = sBarInfo.m_iLogPos;
			ptLogical.y = kiPumpStatusBarRange - sBarInfo.m_iLogPos;
			ping_articles (pJob->GetPingRangeSet(),
				pJob->GetExpirePhase(),
				pJob->GetGroupID(),
				pJob->GetExpireLow(),
				grandTotal, &ptLogical, sSG.est, FALSE);
		}
	}
	catch(...)
	{
		deliver_headers_to_scribe (pJob, pHdrArray, fGetBodies);
		throw;
	}

	deliver_headers_to_scribe (pJob, pHdrArray, fGetBodies);
}

typedef struct
{
	CStringArray m_crossPost;
	CStringArray m_groups;
	CStringArray m_XIcon;
	CByteArray   m_vbyProtect;
} TYP_OVERVIEW_EXTRA, * PTYP_OVERVIEW_EXTRA;

///////////////////////////////////////////////////////////////////////////
// straight_overview -- returns 0 for success
//
//
int  TPump::straight_overview(TPumpJob * pBaseJob, int low, int high,
							  MPBAR_INFO * psBarInfo,
							  TArtHdrArray* pHdrArray)
{
	psBarInfo->m_nPos = 0;          // reset for this subrange

	BOOL fOK = FALSE;
	TPumpJobOverview * pParentJob = (TPumpJobOverview*) pBaseJob;
	int serverRetVal = 0;
	int r;

	TServerCountedPtr psServer;
	BOOL fFullCrossPost = psServer->GetFullCrossPost ();

	// create a sub-job
	TPumpJobOverview * pJob;

	pJob = new TPumpJobOverview( pParentJob->GetGroup(),
		pParentJob->GetGroupID(),
		low, high );
	auto_ptr<TPumpJobOverview> job_deleter(pJob);   // I own this pointer

	if (m_wServerCap & kMustUseXHDR)
		return Use_XHDR_Instead (pJob, psBarInfo, fFullCrossPost, pHdrArray);

	CString followCmd;

	if (fFullCrossPost)
	{
		// Xover is 50%  since CP info is 25% + 25%
		psBarInfo->m_nDivide = 2;
	}
	else
	{
		// Xover is 100%
		psBarInfo->m_nDivide = 1;
	}

	// use the XOVER command on this range of articles. Ask for everything
	r = m_pFeeed->Overview (m_pErrList, low, high, pJob->GetList(),
		fOK, &serverRetVal, m_fEmergencyPump,
		psBarInfo);

	if (r)
	{
		DWORD   dwErrorID;
		CString strError;
		EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

		switch (eErrorClass)
		{
		case kClassNNTP:
			if (serverRetVal >= 500)  // command unimplemented
			{
				m_pErrList->ClearErrors();
				return Use_XHDR_Instead (pJob, psBarInfo, fFullCrossPost, pHdrArray);
			}
			else
				log_nntp_error (strError);
			break;
		case kClassWinsock:
			if (WSAEINTR == dwErrorID)
				PostDisconnectMsg (false, false);
			else
				PostDisconnectMsg (false, true);
			break;
		} // switch
		return 1;
	}

	// so far so good
	if (-1 != psBarInfo->m_iLogSubRangeLen)
		psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
		psBarInfo->m_nDivide;

	TYP_OVERVIEW_EXTRA sExtra;
	sExtra.m_crossPost.SetSize (high - low + 1);
	sExtra.m_groups.SetSize (high - low + 1);

	if (!fFullCrossPost)
		return process_overview_data(pJob, fFullCrossPost, &sExtra, pHdrArray);

	psBarInfo->m_nPos = 0;          // reset for this phase
	psBarInfo->m_nDivide = 4;

	r = get_xpost_info(pJob, low, high, &sExtra, psBarInfo);

	if (r)
		// cleanup pHdrArray
		return r;

	process_overview_data(pJob, fFullCrossPost, &sExtra, pHdrArray);

	// if this is an OverviewUI job then unblock the UI
	//pJob->Abort ();
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//  chop up the overview line.  Get the Cross Posting information.
//
int
TPump::process_overview_data(TPumpJobOverview* pBaseJob,
							 BOOL fCrossPost,
							 void * pExtraIn,
							 TArtHdrArray* pHdrArray)
{
	TServerCountedPtr cpNewsServer;
	TNewsGroup* pNG = 0;
	BOOL fGroupOpen = FALSE;
	int bufSz = 8192;
	LPTSTR pSubj = (LPTSTR) malloc(bufSz);
	if (0 == pSubj)
		AfxThrowMemoryException ();

	THeapBitmap * pAccount = 0;
	PTYP_OVERVIEW_EXTRA pExtra = (PTYP_OVERVIEW_EXTRA) pExtraIn;

	try
	{
		int req_low, req_hi;
		pBaseJob->GetGroupRange(req_low, req_hi);
		pAccount = new THeapBitmap(req_low, req_hi);
		// bit set to 1 means article OK
		//  tallies expired articles
		T2StringList & LineList = pBaseJob->GetList();

		CString fat;
		int  iArtInt;
		BOOL fComplete = FALSE;
		BOOL fUseLock;

		TNewsGroupUseLock useLock(cpNewsServer, pBaseJob->GetGroup(), &fUseLock, pNG);
		if (!fUseLock)
		{
			// suddenly unsubscribed?
			//TRACE1("HandleOverview - where is %s\n", LPCTSTR(pBaseJob->GetGroup()));
			delete pAccount;
			free (pSubj);
			return 1;
		}

		TArticleHeader * pArtHeader = 0;
		CString          strStatusMsg;

		int total = LineList.size();

		// processing %d headers for %s
		strStatusMsg.Format (IDS_STATUS_PROCESSING, total,
			LPCTSTR(pBaseJob->GetGroup()));
		CString oldStatusTxt;
		gpUserDisplay->GetText (oldStatusTxt, m_fEmergencyPump);

		TUserDisplay_Reset sStatusReset(oldStatusTxt, m_fEmergencyPump);

		gpUserDisplay->SetText (strStatusMsg, m_fEmergencyPump);

		for (int i = 0; i < total; ++i)
		{
			fComplete = overview_to_article ( LineList, fat, pSubj, bufSz,
				pArtHeader, iArtInt );
			if (!fComplete)
			{
				delete pArtHeader;
				pArtHeader = NULL;
			}
			else if (iArtInt < req_low || iArtInt > req_hi)
			{
				// weirdo case
				delete pArtHeader;
				pArtHeader = NULL;
			}
			else /* TRUE == fComplete */
			{
				iArtInt = pArtHeader->GetNumber();
				pAccount->SetBit( iArtInt );  // article has not expired!

				// did we retrieve full cross post info
				if (fCrossPost)
					add_all_xpost (pExtra->m_crossPost, req_low, iArtInt, fat,
					pExtra->m_groups,
					pSubj, bufSz, pArtHeader);

				catch_header (pNG, pHdrArray, pBaseJob->GetGroup(),
					iArtInt, pArtHeader);

			} // fComplete
		} // for loop, thru all lines

		TExpiryData * pExpiryData = new TExpiryData(TExpiryData::kMissing,
			true, // assume all present
			pNG->m_GroupID,
			pBaseJob->GetExpireLow());
		pExpiryData->m_ArraypBits.Add (pAccount);

		// sub function
		ExpireHeadersNotFound (pExpiryData);
		pAccount = 0;
	}
	catch(...)
	{
		delete pAccount;
		free (pSubj);

		throw;
		return 1;
	}
	delete pAccount;
	free (pSubj);

	return 0;
} // process_overview_data

///////////////////////////////////////////////////////////////////////////
BOOL TPump::overview_to_article (T2StringList & rList, CString & strFat,
								 LPTSTR & pSubj, int & bufSz, TArticleHeader*& rpArtHdr, int & iArtInt)
{
	BOOL fComplete = FALSE;
	rpArtHdr = NULL;

	// STL is a little weird.. I admit
	strFat = *(rList.begin());
	rList.pop_front ();

	int iIndex = strFat.Find ('\t');

	if (0 >= iIndex)
		return FALSE;
	int iFatLen = strFat.GetLength ();

	// do we need more room?
	if (iFatLen > bufSz)
		size_alloc ( pSubj, bufSz, iFatLen );

	LPTSTR fatbuf = strFat.GetBuffer ( iFatLen );

	fComplete = overview_make_art ( fatbuf, pSubj, bufSz, rpArtHdr);

	if (fComplete)
	{
		iArtInt = rpArtHdr->GetNumber ();
	}

	strFat.ReleaseBuffer ( iFatLen );

	return fComplete;
}

///////////////////////////////////////////////////////////////////////////
void TPump::size_alloc (LPTSTR & pSubj, int & bufSz, int iNewSize)
{
	LPTSTR pNewBuf = (LPTSTR) realloc (pSubj, iNewSize);
	if (0 == pNewBuf)
		realloc_error ( bufSz, iNewSize );
	else
	{
		bufSz = iNewSize;
		pSubj = pNewBuf;
	}
}

///////////////////////////////////////////////////////////////////////////
void TPump::realloc_error (int iOldSize, int iNewSize)
{
	CString final;
	final.Format (IDS_ERR_REALLOC, iOldSize, iNewSize);
	throw(new TException (final, kFatal));
}

///////////////////////////////////////////////////////////////////////////
//  Break this out to profile it
//
BOOL TPump::overview_make_art (LPTSTR line, LPTSTR pSubj, int bufSz,
							   TArticleHeader*& rpArtHdr)
{

	// Note  :  get() stops on the delimiter, but doesn't extract it
	//          getline() extracts delimiter, but doesn't store it in result
	//           buffer

	int iBytes = 0;
	int iLines = -10;
	TCHAR  rcFrom[4096];
	int iArtInt;
	BOOL fComplete = FALSE;
	TArticleHeader * pArtHeader = 0;

	rpArtHdr = NULL;

	// make a Stream!
	_tstrstream in(line);

	// article number       Borrow rcFrom as a buffer
	//  these 2 lines handle cases where instead of ArtInt<tab>
	//  we have ArtInt<spaces><tab>
	//
	in >> iArtInt;
	in.getline (rcFrom, sizeof(rcFrom), '\t');  // discard extra junk upto and including Tab

	// subject
	in.getline (pSubj, bufSz, '\t');

	// from
	rcFrom[0] = '\0';
	in.getline (rcFrom, sizeof(rcFrom), '\t');

	if ('\0' == rcFrom[0])
	{
		return FALSE;
	}

	pArtHeader = new TArticleHeader;
	pArtHeader->SetArticleNumber ( iArtInt );
	pArtHeader->SetQPFrom          ( rcFrom );

#if defined(_DEBUG)
	pArtHeader->SetQPSubject       ( pSubj , rcFrom );
#else
	pArtHeader->SetQPSubject       ( pSubj );
#endif

	// date
	in.getline(pSubj, bufSz, '\t');
	pArtHeader->SetDate( pSubj );

	// msgid
	in.getline(pSubj, bufSz, '\t');
	pArtHeader->SetMessageID(pSubj);

	// refs
	in.getline(pSubj, bufSz, '\t');
	pArtHeader->SetReferences(pSubj);

	// skip bytes, get lines
	in >> iBytes >> iLines;

	// if lines is not here, then the BYTES token was missing
	if (iLines < 0)
		iLines = iBytes;
	pArtHeader->SetLines( iLines );

	// checking for XRef information here at the end
	in >> ws;
	in.getline(pSubj, bufSz, _T(' '));
	if (0==_tcsicmp(pSubj, _T("Xref:")))
	{
		in >> ws;
		in.getline(pSubj, bufSz, _T('\r'));

		// store     servername [group:###]+
		pArtHeader->SetXRef ( pSubj );

		WORD wPrev = m_wServerCap;
		m_wServerCap |= kXRefinXOver;

		if (!(wPrev & kXRefinXOver))
		{
			TServerCountedPtr cpNewsServer;
			cpNewsServer->XOverXRef (true);

			// saves out to registry
			cpNewsServer->SaveSettings ();
		}
	}

	fComplete = TRUE;
	rpArtHdr = pArtHeader;
	return fComplete;
}

// ------------------------------------------------------------------------
void TPump::add_all_xpost (CStringArray& crossPost, int iLow, int iArtInt,
						   CString & fat, CStringArray& vNG, LPTSTR & pSubj, int & bufSz,
						   TArticleHeader * pHdr)
{
	int j = iArtInt - iLow;

	if (!(kXRefinXOver & m_wServerCap))
	{
		fat = crossPost[j];
		add_xpost_info ( fat, pSubj, bufSz, pHdr );
	}

	fat = vNG[j];
	int iSpace = fat.Find(' ');
	if (iSpace > 0)
		process_ng_line (fat, iSpace, pHdr);
}

// ------------------------------------------------------------------------
//
void TPump::add_xpost_info (CString & fat, LPTSTR & pSubj, int & bufSz,
							TArticleHeader * pHdr)
{
	int index;
	if (!fat.IsEmpty())
	{
		int iFatLen = fat.GetLength ();
		if (bufSz < iFatLen)
			size_alloc (pSubj, bufSz, iFatLen);

		LPTSTR fatbuf = fat.GetBuffer (iFatLen);

		_tstrstream in(fatbuf);

		// discard the article number
		in >> index;
		in >> ws;
		in.getline(pSubj, bufSz, '\r');

		// store the servername [group:###]+
		pHdr->SetXRef ( pSubj );

		fat.ReleaseBuffer (iFatLen);
	}
}

///////////////////////////////////////////////////////////////////////////
// take article object and dispatch it somewhere
void TPump::catch_header (TNewsGroup* pNG,
						  TArtHdrArray* pHdrArray, LPCTSTR lszGroupName,
						  int iArtInt,
						  TArticleHeader*& rpHdr)
{
	// article is not on disk. But is it already queued up?
	if (FALSE == pNG->TextRangeHave (iArtInt))
	{
		// scribe will get whole array
		pHdrArray->Add ( rpHdr );
	}
	else
	{
		delete rpHdr;
		rpHdr = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
//  This function is called if the server does not support XOVER
//
//  Returns TRUE if a job was queued for the scribe
int TPump::Use_XHDR_Instead(TPumpJobOverview*& pBaseJob,
							MPBAR_INFO * psBarInfo,
							BOOL fFullCrossPost,
							TArtHdrArray*  pHdrArray)     // fill this up with headers
{
	TServerCountedPtr cpNewsServer;
	ASSERT(0 == m_HeaderMap.GetCount());
	TPumpJobOverview * pJob = (TPumpJobOverview*) pBaseJob;
	int low, high, range;

	pJob->GetGroupRange ( low, high );
	range = high - low + 1;
	THeapBitmap* pAccount = new THeapBitmap(low, high);
	int r;

	int  iDenum = (fFullCrossPost) ? 8 : 6;
	try
	{
		do
		{
			int iFromCount;
			int iStatusBarPos = 0;
			CString output;
			CString cmdLine;
			FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);

			// Request FROM - DATE - REFERENCES - MSGID - SUBJECT - LINECOUNT
			//           XREF

			// 1  download bunch of FROM lines
			iFromCount = 0;

			psBarInfo->m_nDivide = iDenum;

			psBarInfo->m_nPos = 0;

			r = xhdr_command (pJob, low, high, "from", kXFrom, iFromCount, psBarInfo);

			if (r)
				return r;

			if (-1 != psBarInfo->m_iLogSubRangeLen)
				psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
				psBarInfo->m_nDivide;

			// sanity check
			if (0 == iFromCount)
			{
				// there is nothing in this range. don't bother trying
				// the other fields.  jump to the end.
				break;
			}

			// 2  bunch of DATE lines

			psBarInfo->m_nPos = 0;

			r = xhdr_command (pJob, low, high, "date", kXDate, iFromCount, psBarInfo);
			if (r)
				return r;

			if (-1 != psBarInfo->m_iLogSubRangeLen)
				psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
				psBarInfo->m_nDivide;

			// 3  bunch of REFERENCES lines

			psBarInfo->m_nPos = 0;
			r = xhdr_command (pJob, low, high, _T("references"), kXRefs, iFromCount, psBarInfo);
			if (r)
				return r;

			if (-1 != psBarInfo->m_iLogSubRangeLen)
				psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
				psBarInfo->m_nDivide;

			// 4  bunch of MessageID lines

			psBarInfo->m_nPos = 0;
			r = xhdr_command (pJob, low, high, "Message-ID", kXMsgID, iFromCount, psBarInfo);
			if (r)
				return r;

			if (-1 != psBarInfo->m_iLogSubRangeLen)
				psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
				psBarInfo->m_nDivide;

			// 5  bunch of Subject lines

			psBarInfo->m_nPos = 0;
			r = xhdr_command (pJob, low, high, "subject", kXSubject, iFromCount, psBarInfo);
			if (r)
				return r;

			if (-1 != psBarInfo->m_iLogSubRangeLen)
				psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
				psBarInfo->m_nDivide;

			// 6  bunch of line-count

			psBarInfo->m_nPos = 0;
			r = xhdr_command (pJob, low, high, "lines", kXLines, iFromCount, psBarInfo);
			if (r)
				return r;

			if (-1 != psBarInfo->m_iLogSubRangeLen)
				psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
				psBarInfo->m_nDivide;

			if (fFullCrossPost)
			{
				// 7  bunch of XREF lines
				psBarInfo->m_nPos = 0;

				r = xhdr_command (pJob, low, high, "xref", kXCrossPost, iFromCount,
					psBarInfo);

				if (r)
					return r;

				if (-1 != psBarInfo->m_iLogSubRangeLen)
					psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
					psBarInfo->m_nDivide;

				// 8 bunch of Newsgroup lines
				psBarInfo->m_nPos = 0;

				r = xhdr_command (pJob, low, high, "newsgroups", kXNewsgroups, iFromCount,
					psBarInfo);
				if (r)
					return r;

				if (-1 != psBarInfo->m_iLogSubRangeLen)
					psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
					psBarInfo->m_nDivide;
			}

			TArticleHeader* pArtHdr;
			int             artInt;

			BOOL fUseLock;
			TNewsGroup* pNG = 0;
			TNewsGroupUseLock useLock (cpNewsServer, pJob->GetGroup(), &fUseLock, pNG);

			try
			{
				POSITION pos = m_HeaderMap.GetStartPosition();
				while (pos)
				{
					m_HeaderMap.GetNextAssoc( pos, artInt, pArtHdr );
					m_HeaderMap.RemoveKey ( artInt );  // remove from map

					pAccount->SetBit( artInt );

					if (true)
					{
						// big package for scribe
						pHdrArray->Add(pArtHdr);
					}
				}
			}
			catch(...)
			{
				throw;
			}

		} while(FALSE);

		if (true)
		{
			TExpiryData * pED = new TExpiryData(TExpiryData::kMissing,
				!pJob->GetExpirePhase(),
				pJob->GetGroupID(),
				pJob->GetExpireLow());
			pED->m_ArraypBits.Add (pAccount);
			pAccount = 0;

			ExpireHeadersNotFound ( pED );
		}
	}
	catch (TException *pE)
	{
		delete pAccount;
		// free any remaining items
		freeHeaderMap();
		TException *ex = new TException(*pE);
		pE->Delete();
		throw(ex);
		return 0;
	}
	delete pAccount;
	m_HeaderMap.RemoveAll();

	return 0;
}

// ------------------------------------------------------------------------
// xhdr_command -- called to get article headers via multiple XHDR commands
//
int  TPump::xhdr_command (TPumpJobOverview * pJob,
						  int low, int high, LPCTSTR token,
						  EHeaderParts ePart, int & total,
						  MPBAR_INFO * psBarInfo)
{
	CString cmd, output;
	int r;

	// RLW - Modified so we do not ask for more than 300 articles in one go
	int nStart = low;

	total = 0;

	while (nStart <= high)
	{
		cmd.Format("XHDR %s %d-%d", token, nStart, (nStart + 300 < high) ? nStart + 300 : high);
		//TRACE("%s\n", cmd);
		
		if ((r = m_pFeeed->Direct(m_pErrList, cmd)) == 0)
		{
			while (true)
			{
				if (!m_pFeeed->DirectNext(m_pErrList, output))
				{
					if (output.GetLength() == 3 && output == ".\r\n")
						break;
					++total;
					StoreHeaderPart(ePart, output, psBarInfo);
				}
				else
				{
					// returns 0 to continue
					r = error_xhdr(cmd, pJob);
					if (r)
						return r;
				}
			}
		}
		else
		{
			// returns 0 to continue
			r = error_xhdr(cmd, pJob);
			if (r)
				return r;
		}

		nStart += 301;
	}
	//cmd.Format ("XHDR %s %d-%d", token, low, high);
	//TRACE("%s\n", cmd);
	//total = 0;
	//
	//if ((r = m_pFeeed->Direct(m_pErrList, cmd)) == 0)
	//{
	//	while (true)
	//	{
	//		if (!m_pFeeed->DirectNext(m_pErrList, output))
	//		{
	//			if (output.GetLength() == 3 && output == ".\r\n")
	//				break;
	//			++total;
	//			StoreHeaderPart(ePart, output, psBarInfo);
	//		}
	//		else
	//			return error_xhdr(cmd, pJob);
	//	}
	//}
	//else
	//{
	//	// returns 0 to continue
	//	r = error_xhdr(cmd, pJob);
	//	if (r)
	//		return r;
	//}

	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 to continue with calling function, non-zero to abort caller
int  TPump::error_xhdr (const CString & cmd, TPumpJobOverview * pJob)
{
	DWORD   dwErrorID;
	CString strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	switch (eErrorClass)
	{
	case kClassNNTP:
		{
			CString strMsg;
			strMsg.Format ("%s returned %s", LPCTSTR(cmd), LPCTSTR(strError));
			gpEventLog->AddError (TEventEntry::kPump, strMsg, strError);

			m_pErrList->ClearErrors();

			// caller should continue
			return 0;
		}

	case kClassWinsock:
		if (WSAEINTR == dwErrorID)
		{
			// no reconnect
			PostDisconnectMsg (false, false);
		}
		else
		{
			// fIdle, fReconnect
			PostDisconnectMsg (false, true);
		}

		// caller should abort
		return 1;

	default:
		ERRDLG(dwErrorID, strError);

		// caller should abort
		return 1;
	}

	return 1;
}

// free any remaining items in the header Map
void TPump::freeHeaderMap(void)
{
	//TRACE0("Calling freeHeaderMap for cleanup\n");
	TArticleHeader* pArtHdr;
	int             artInt;

	POSITION pos = m_HeaderMap.GetStartPosition();

	while (pos)
	{
		pArtHdr = 0;
		m_HeaderMap.GetNextAssoc ( pos, artInt, pArtHdr );
		delete pArtHdr;
	}
	m_HeaderMap.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////
// StoreHeaderPart
//
void TPump::StoreHeaderPart (TPump::EHeaderParts ePart,
							 CString& line, MPBAR_INFO * psBarInfo)
{
	ASSERT(line.GetLength() >= 2);
	static BYTE byDisplay = 0;

	TArticleHeader* pArtHdr;
	line.TrimRight();

	LPCTSTR ptr = line;
	int     idx = line.Find(' ');
	int artInt = _ttoi(ptr);

	if (WAIT_OBJECT_0 == WaitForSingleObject(m_KillRequest, 0))
		throw(new TException(IDS_ERR_XHDR_ABORT, kInfo));

	psBarInfo->m_nPos++;
	if (psBarInfo->m_iLogSubRangeLen != -1)
	{
		int fraction = MulDiv (psBarInfo->m_iLogSubRangeLen, psBarInfo->m_nPos,
			psBarInfo->m_nSubRangeLen);
		if (fraction != -1)
			fraction /= psBarInfo->m_nDivide;
		gpUserDisplay->SetPos (psBarInfo->m_iLogPos + fraction, FALSE, m_fEmergencyPump);
	}

	if (kXFrom == ePart)
	{
		TArticleHeader* pNewHdr = new TArticleHeader;
		// Take everything after the space
		if (idx > 0)
		{
			pNewHdr->SetQPFrom ( ptr + idx + 1 );
			pNewHdr->SetArticleNumber ( artInt );
			m_HeaderMap.SetAt ( artInt, pNewHdr );
		}
		return;
	}

	if (idx <= 0)
		return;

	if (kXRefs == ePart)
	{
		if (_tcscmp(ptr + idx + 1, _T("(none)")) && m_HeaderMap.Lookup ( artInt, pArtHdr ))
			pArtHdr->SetReferences ( ptr + idx + 1 );
		return;
	}

	if (kXCrossPost == ePart)
	{
		if (_tcscmp(ptr + idx + 1, _T("(none)")) && m_HeaderMap.Lookup(artInt, pArtHdr))
			pArtHdr->SetXRef( ptr + idx + 1 );
		return;
	}

	if (kXNewsgroups == ePart)
	{
		if (_tcscmp(ptr + idx + 1, _T("(none)")) && m_HeaderMap.Lookup(artInt, pArtHdr))
			process_ng_line ( line, idx + 1, pArtHdr );
		return;
	}

	if (!m_HeaderMap.Lookup ( artInt, pArtHdr ))
		return;

	switch (ePart)
	{
	case kXDate:
		pArtHdr->SetDate ( ptr + idx + 1 );
		break;

	case kXMsgID:
		pArtHdr->SetMessageID ( ptr + idx + 1 );
		break;

	case kXSubject:
		pArtHdr->SetQPSubject ( ptr + idx + 1 );
		break;

	case kXLines:
		{
			int n = _ttoi(ptr + idx + 1);
			pArtHdr->SetLines ( n );
		}
		break;

	default:
		throw(new TException(IDS_ERR_UNKNOWN_TYPE_STOREHEADERPART, kFatal));
	}
	return;
} // StoreHeaderPart

// ------------------------------------------------------------------------
// Make this job-neutral
int  TPump::ping_articles (
						   TRangeSet* prsPing,
						   BOOL fExpirePhase,
						   int  iGroupID,
						   int  iExpireLow,
						   int grandTotal,
						   LPPOINT pptLogical,
						   int iEstimate,
						   BOOL fRedraw)
{
	int iLogPos = pptLogical->x;
	int iLogicalRange = pptLogical->y;
	int r;

	// Turn bit On if the article is alive

	if (m_wServerCap & kMustUseXLines)
	{
		return ping_articles_lines (prsPing, fExpirePhase, iGroupID, iExpireLow,
			grandTotal, pptLogical, fRedraw);
	}

	// otherwise, we are gonna Try ListGroup command
	TExpiryData * pExpiryData = new TExpiryData (!fExpirePhase,
		iGroupID,
		iExpireLow);
	if (fRedraw) pExpiryData->m_fRedrawUI = true;
	TRangeSet* pRange = prsPing;

	bool fRangeValid = pRange && (pRange->RangeCount() > 0);
	if (false == fRangeValid)
		pExpiryData->SetAllPresent (true);

	// do nothing if nothing to ping
	if (fRangeValid && fExpirePhase)
	{
		// Try LISTGROUP
		CString cmd = "listgroup";
		CString result;

		r = m_pFeeed->Direct (m_pErrList, cmd);

		if (r)
		{
			DWORD   dwErrorID;
			CString strError;
			EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

			switch (eErrorClass)
			{
			case kClassNNTP:
				{
					m_pErrList->ClearErrors ();

					m_wServerCap |= kMustUseXLines;
					return ping_articles_lines (prsPing, fExpirePhase, iGroupID, iExpireLow,
						grandTotal, pptLogical, fRedraw);
				}
				break;
			case kClassWinsock:
				if (WSAEINTR == dwErrorID)
					PostDisconnectMsg (false, false);
				else
					PostDisconnectMsg (false, true);
				break;
			default:
				break;
			}  // switch

		} // error from LISTGROUP

		// real pos = n
		// real range = pptBound
		// log pos = iLogPos
		// log range = iLogicalRange

		while (true)
		{
			r = m_pFeeed->DirectNext (m_pErrList, result);

			if (r)
			{
				DWORD   dwErrorID;
				CString strError;
				EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

				switch (eErrorClass)
				{
				case kClassNNTP:
					break;
				case kClassWinsock:
					if (WSAEINTR == dwErrorID)
						PostDisconnectMsg (false, false);
					else
						PostDisconnectMsg (false, true);
					break;
				default:
					break;
				}  // switch
			}

			if (result == ".\r\n")
				break;

			int n = _ttoi(result);
			pExpiryData->Add(n);
			if (iEstimate > 0)
			{
				int iAdvanceLogPos = iLogicalRange * (n - iExpireLow) / iEstimate;
				gpUserDisplay->SetPos (iLogPos + iAdvanceLogPos, FALSE, m_fEmergencyPump);
			}
		} // read data from LISTGROUP
	}
	ExpireHeadersNotFound (pExpiryData);

	return 0;
}

// ------------------------------------------------------------------------
// check deadwood using 'XHDR LINES'
//  make this job-neutral
int  TPump::ping_articles_lines (
								 TRangeSet *       prsPing,
								 BOOL              fExpirePhase,
								 int               iGroupID,
								 int               iExpireLow,
								 int               grandTotal,
								 LPPOINT           pptLogical,
								 BOOL              fRedraw)
{
	int iLogPos = pptLogical->x;
	int iLogicalRange = pptLogical->y;
	int i, low, high, r;

	TExpiryData* pExpiryData = new TExpiryData(TExpiryData::kExpired,
		!fExpirePhase,
		iGroupID, iExpireLow);
	if (fRedraw) pExpiryData->m_fRedrawUI = true;

	CString cmd, result;
	TRangeSet* pRange = prsPing;

	bool fRangeValid = pRange && (pRange->RangeCount() > 0);
	if (false == fRangeValid)
		pExpiryData->SetAllPresent (true);

	if (pRange)
	{
		int rangeTot = pRange->RangeCount();
		int nPing = 0;
#if defined(_DEBUG)
		{
			CString strStatus;
			strStatus.Format("(debug) checking deadwood for NG");
			gpUserDisplay->SetText (strStatus, m_fEmergencyPump);
		}
		if (false==fExpirePhase)
			ASSERT(rangeTot==0);
#endif
		for (i = 0; i < rangeTot; ++i)
		{
			pRange->GetRange(i, low, high);
			THeapBitmap* pBits = new THeapBitmap(low, high);

			int iArtInt, iLineCount;
			// RLW - Modified so we do not ask for more than 300 items in one go
			int nStart = low;

			while (nStart < high)
			{
				cmd.Format("XHDR LINES %d-%d", nStart, (nStart + 300 < high) ? nStart + 300 : high);
				//TRACE("%s\n", cmd);

				if ((r = m_pFeeed->Direct(m_pErrList, cmd)) == 0)
				{
					while (true)
					{
						if ((r = m_pFeeed->DirectNext (m_pErrList, result)) == 0)
						{
							if (result == ".\r\n")
								break;

							int fields = _stscanf(result, _T("%d %d"), &iArtInt, &iLineCount);
							if ((2 == fields) && (iArtInt >= low && iArtInt <= high))
							{
								pBits->SetBit(iArtInt);
							}
							// (high-low+1)
							// ------------  =  % to advance
							// grandTotal

							int iLogAdv = iLogicalRange * ++nPing / grandTotal;
							// this is increment from baseline

							gpUserDisplay->SetPos(iLogPos + iLogAdv, FALSE, m_fEmergencyPump);
						}
						else
							return r;  // $$
					}
				}
				else
					return r;  // $$

				nStart += 301;
			}

			//cmd.Format("XHDR LINES %d-%d", low, high);
			//TRACE("%s\n", cmd);

			//int iArtInt, iLineCount;
			//if ((r = m_pFeeed->Direct(m_pErrList, cmd)) == 0)
			//{
			//	while (true)
			//	{
			//		if ((r = m_pFeeed->DirectNext (m_pErrList, result)) == 0)
			//		{
			//			if (result == ".\r\n")
			//				break;

			//			int fields = _stscanf(result, _T("%d %d"), &iArtInt, &iLineCount);
			//			if ((2 == fields) && (iArtInt >= low && iArtInt <= high))
			//			{
			//				pBits->SetBit(iArtInt);
			//			}
			//			// (high-low+1)
			//			// ------------  =  % to advance
			//			// grandTotal

			//			int iLogAdv = iLogicalRange * ++nPing / grandTotal;
			//			// this is increment from baseline

			//			gpUserDisplay->SetPos(iLogPos + iLogAdv, FALSE, m_fEmergencyPump);
			//		}
			//		else
			//			return r;  // $$
			//	}
			//}
			//else
			//	return r;  // $$

			pExpiryData->m_ArraypBits.Add (pBits);
		} // for
	} // if pRange
	ExpireHeadersNotFound (pExpiryData);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void TPump::ScribeJob(TScribePrep * pPrep)
{
	m_pMasterTasker->AddScribeJob ( pPrep );
}

///////////////////////////////////////////////////////////////////////////
void TPump::SaveHeader_and_Request(
								   const CString&    groupName,
								   LONG              groupID,
								   TArticleHeader*   pArtHdr,
								   int               artInt,
								   BOOL              fBody)
{
	// order scribe to write this
	TPrepHeader * pPrepHeader = new TPrepHeader(
		groupName,
		pArtHdr,
		artInt);
	ScribeJob ( pPrepHeader );

	ASSERT(fBody == TRUE || fBody == FALSE);

	if (fBody)
	{
		m_pLink->m_pJobs->RequestBody ( groupName, groupID,
			artInt, pArtHdr->GetLines(),
			false,  // front of queue
			PJF_SEND_SCRIBE);

	}
}

// ------------------------------------------------------------------------
// BatchSaveHeader_and_Request --
//
void TPump::BatchSaveHeader_and_Request(
										const CString &        groupName,
										LONG                   groupID,
										TArtHdrArray*          pHdrArray,
										BOOL                   fGetBody,
										TPumpJob::EPumpJobDone eJobDone)
{
	// the marker helps the TScribe finally bind the Hdr + Body

	/// we want to send the scribe 1 Big job.
	CDWordArray   ArtInts;
	CDWordArray   Lines;
	int arraySize = pHdrArray->GetSize();

	if (fGetBody)
	{
		ArtInts.SetSize(arraySize);
		Lines.SetSize(arraySize);
	}
	int j;
	int l, artint;
	for (j = 0; j < arraySize; ++j)
	{
		if (fGetBody)
		{
			TArticleHeader* pArtHdr = pHdrArray->GetAt(j);
			artint = pArtHdr->GetArticleNumber();
			l      = pArtHdr->GetLines();

			ArtInts.SetAt(j, artint);
			Lines.SetAt(j,   l);
		}
	}

	TScribePrep::EPrepDone eDone = (TPumpJob::kDoneShow == eJobDone)
		? TScribePrep::kDoneDisplay
		: TScribePrep::kDoneNothing;

	// send scribe this big job
	TPrepBatchHeader * pPrepBatchHeader = new TPrepBatchHeader(
		groupName,
		groupID,
		pHdrArray,
		eDone);
	ScribeJob ( pPrepBatchHeader );

	ASSERT(fGetBody == TRUE || fGetBody == FALSE);

	if (fGetBody)
	{
		for (j = 0; j < arraySize; j++)
		{
			m_pLink->m_pJobs->RequestBody ( groupName,
				groupID,
				int(ArtInts[j]),
				int(Lines[j]),
				false,
				PJF_SEND_SCRIBE );
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// This gets called repeatedly when we are blocking on a winsock call
//
BOOL FAR WINAPI NormalPumpBlockingHook(void)
{
//#if defined(_DEBUG) && defined(JUNKO)
//	CTime sNow = CTime::GetCurrentTime();
//
//	CString str = sNow.Format("%I:%M:%S %p\n");
//	//TRACE(LPCTSTR(str));
//#endif

	if (WAIT_OBJECT_0 == WaitForSingleObject(gsNormHookData.hKillEvent, 0))
	{
		gsNormHookData.psPump->NoSendQuit ();

		//TRACE("+++++++ called WSACancelBlockingCall ++++\n");
		WSACancelBlockingCall();
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
BOOL FAR WINAPI EmergencyPumpBlockingHook(void)
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(gsPrioHookData.hKillEvent, 0))
	{
		gsPrioHookData.psPump->NoSendQuit ();
		WSACancelBlockingCall();
	}
	return FALSE;
}

//-------------------------------------------------------------------------
// Returns 0 for success. caller does major error handling
//
int TPump::connect (CString & rstrWelcome)
{
	int stat = 0;

	// clear status line when done
	TUserDisplay_Auto sAuto(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump ?
		TUserDisplay_Auto::kPriority :
	TUserDisplay_Auto::kNormal);
	CString status_msg;
	status_msg.Format(IDS_ERR_CONNECTING_TO, m_server);
	gpUserDisplay->SetText( status_msg, m_fEmergencyPump );

	stat = m_pFeeed->Init ( m_pErrList, m_server, rstrWelcome );

	if (0 == stat)
		return 0;

	DWORD   dwErrorID;
	CString strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	if (true)
	{
		CString error;
		error.LoadString(IDS_ERR_CONNECT_TO);
		error += " " + m_server;

		CString details;
		details.Format ("%d %s", dwErrorID, strError);
		gpEventLog->AddError ( TEventEntry::kPump, error, details );
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////
// Returns 0 for success
// this used just as a 'keep alive' message.
//
int TPump::DoHelpCommand (TPumpJob* pBaseJob )
{
	auto_ptr<CObject> job_deleter(pBaseJob);

	// clear status line when done
	TUserDisplay_Auto sAuto(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump ?
		TUserDisplay_Auto::kPriority :
	TUserDisplay_Auto::kNormal);
	CString status_msg;
	status_msg.LoadString (IDS_HLPCMD_STATUS);

	gpUserDisplay->SetText( status_msg, m_fEmergencyPump );

	int     server_code = 999;
	int     iAnswer = 0;
	CString server_ack;

	if (m_pFeeed->WriteLine (m_pErrList, _T("help"), 4))
		goto help_error;  // error

	if (m_pFeeed->ReadLine (m_pErrList, server_ack))
		goto help_error;  // error

	server_code = _ttoi(LPCTSTR(server_ack));

	if ((server_code/100) == 1)
	{
		// read until  .\r\n

		do
		{
			status_msg.Format (IDS_HLPCMD_RESPONSE, ++iAnswer);
			gpUserDisplay->SetText( status_msg, m_fEmergencyPump );

			if (m_pFeeed->ReadLine (m_pErrList, server_ack))
				goto help_error;

		} while (!(3 == server_ack.GetLength() && '.' == server_ack[0]));

		return 0;
	}
	else
		return 0;

help_error:
	if (true)
	{
		DWORD       dwErrorID;
		CString     strError;
		EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

		switch (eErrorClass)
		{
		case kClassWinsock:
			if (WSAEINTR == dwErrorID)
				PostDisconnectMsg (false, false);
			else
				// fIdle, fReconnect
				PostDisconnectMsg (false, true);
			break;

		case kClassNNTP:
			m_pErrList->ClearErrors ();
			return 0;

		default:
			ASSERT(0);
			ERRDLG(dwErrorID, strError);
			break;
		} // switch

		return 1;
	}
}

// ------------------------------------------------------------------------
//   CString m_errorMessage;
//   int     m_iNNTPError;
void TPump::DoConnect(TPumpJob* pBaseJob)
{
	TServerCountedPtr cpNewsServer;
	int iWinSockLastError = 0;
	TPumpJobConnect* pJob = (TPumpJobConnect*) pBaseJob;
	CString welcome;
	int srvCode;
	BOOL fUserCancel = FALSE;
	int  r = 0;
	CString errorMsg;                // holds context "error authorizing passwd"

	// WinSock connect
	r = connect ( welcome );

	if (r)
	{
		connect_fail (pJob, fUserCancel, errorMsg );
		return;
	}

	// this is a class-wide thing
	m_timeKeeper.Enable (TRUE);
	m_timeKeeper.SetActivity ();

	// analyze the welcome message
	_tstrstream iss((LPTSTR)(LPCTSTR)welcome);
	iss >> srvCode;
	switch (srvCode/100)
	{
	case 1:        // help text
		break;

	case 2:        // server ready
		{
			// set things up with the Mode Reader command
			int r;

			// some servers don't like it, so Gravity has option to turn it off
			if (cpNewsServer->GetModeReader())
			{
				CString strMode = "mode reader";
				CString strModeAck;
				r = m_pFeeed->WriteLine (m_pErrList, strMode, strMode.GetLength());
				if (r)
				{
					connect_fail (pJob, fUserCancel, strMode);
					return ;
				}

				// read response
				r = m_pFeeed->ReadLine (m_pErrList, strModeAck);
				if (r)
				{
					connect_fail (pJob, fUserCancel, strMode);
					return ;
				}
			}

			if (!m_fEmergencyPump)
			{
				BOOL fPostingOK = (200 == srvCode || 205 == srvCode);
				if (cpNewsServer->GetPostingAllowed() != fPostingOK)
				{
					cpNewsServer->SetPostingAllowed (fPostingOK);
					cpNewsServer->SaveSettings ();
				}
			}

			TSPASession * pSPA = cpNewsServer->LockSPASession ();

			try
			{
				r = logon_style (srvCode, errorMsg,
					cpNewsServer->GetLogonStyle(),
					cpNewsServer->GetAccountName(),
					cpNewsServer->GetAccountPassword(),
					pSPA);
			}
			catch(...)
			{
				cpNewsServer->UnlockSPASession ();
				throw;
			}
			cpNewsServer->UnlockSPASession ();

			if (r)
			{
				connect_fail ( pJob, fUserCancel, errorMsg );
				return;
			}
		}
		break;

	case 4:
	case 5:        // error
		{
			TError sErr(welcome, kError, kClassNNTP);
			m_pErrList->PushError (sErr);

			connect_fail ( pJob, fUserCancel, errorMsg );
			return;
		}
	} // end switch

	// CONTINUE SETUP

	if (0 == AfterConnect (pJob))
	{

		// success!
		return ;
	}
	else
		connect_fail ( pJob, fUserCancel, errorMsg );
}

// ------------------------------------------------------------------------
// Returns 0 for success.  Uses m_pErrList to store errors
int TPump::logon_style (int iWelcomeCode,
						CString & errorMsg,
						int iLogonStyle,
						LPCTSTR usrname,
						LPCTSTR password, void * pVoidSPA)
{
	// clear status line when done
	TUserDisplay_Auto sAuto(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump ?
		TUserDisplay_Auto::kPriority :
	TUserDisplay_Auto::kNormal);
	CString status_msg;
	status_msg = "Authorizing with " + m_server;
	gpUserDisplay->SetText( status_msg, m_fEmergencyPump );

	int r;
	switch (iLogonStyle)
	{
	case 0:
		// no password required
		return 0;

	case 1:
		// plaintext authorization
		{
			CString line_name;
			CString ack;
			int authret;
			line_name.Format ("AUTHINFO user %s", usrname);

			r = m_pFeeed->WriteLine ( m_pErrList, line_name, line_name.GetLength() );
			if (r)
				return r;

			if (m_pFeeed->ReadLine ( m_pErrList, authret, ack ))
				return 1;

			if (authret >= 500)
			{
				CString rbuild; rbuild.Format ("%d %s", authret, LPCTSTR(ack));
				TError sError(rbuild, kError, kClassNNTP);
				m_pErrList->PushError (sError);

				errorMsg.LoadString (IDS_ERR_AUTHORIZING);
				return 1;
			}
			line_name.Format (_T("AUTHINFO pass %s"), password);

			r = m_pFeeed->WriteLine ( m_pErrList, line_name, line_name.GetLength() );
			if (r)
				return 1;

			r = m_pFeeed->ReadLine ( m_pErrList, authret, ack );
			if (r)
				return 1;

			if (authret < 200 || authret > 299)
			{
				CString rbuild; rbuild.Format (_T("%d %s"), authret, LPCTSTR(ack));

				TError sError(rbuild, kError, kClassNNTP);
				m_pErrList->PushError (sError);

				errorMsg.LoadString (IDS_ERR_AUTHORIZING_PASS);
				return 1;
			}
		}
		return 0;

	case 2:
		// Secure Password Authentication
		return CryptoLogin (pVoidSPA);

	default:
		ASSERT(0);  // unknown case
	}
	return 1;
}

// ------------------------------------------------------------------------
int TPump::connect_fail (TPumpJobConnect * pJob, BOOL fUserCancel,
						 CString & errorMsg)
{
	DWORD       dwErrorID;
	CString     strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	pJob->m_eErrorClass = eErrorClass;
	pJob->m_dwErrorID = dwErrorID;

	switch (eErrorClass)
	{
	case kClassWinsock:
		{
			switch (dwErrorID)
			{
			case WSAEINTR:
				// no reconnect, user interrupted
				PostDisconnectMsg (false, false);

				fUserCancel = TRUE;
				break;

			case WSAETIMEDOUT:
				// reconnect = yes
				PostDisconnectMsg (false, true);
				break;

			case WSAECONNREFUSED:
			case WSAENETDOWN:
			case WSAENOBUFS:
			case WSAEFAULT:
			case WSAHOST_NOT_FOUND:
			case WSANO_RECOVERY:
			case WSANO_DATA:
			case WSANOTINITIALISED:
			case WSATRY_AGAIN:
			case WSAECONNABORTED:
				// no reconnect,   we pretty much can't recover from this
				PostDisconnectMsg (false, false);
				break;

			default:
				// fIdle, fReconnect ( no reconnect )
				PostDisconnectMsg (false, false);
				break;
			}
		}
		break;

	case kClassExternal:
	case kClassNNTP:
		{
			if (!errorMsg.IsEmpty())
			{
				pJob->m_errorMessage.Format ("%s - Server responded: %s",
					LPCTSTR(errorMsg),
					LPCTSTR(strError));
			}
			else
				pJob->m_errorMessage = strError;

			// no reconnect
			PostDisconnectMsg (false, false);
		}
		break;

	default:
		pJob->m_errorMessage = strError;

		ERRDLG(dwErrorID, strError);
		PostDisconnectMsg (false, false);
		break;
	} // switch

	// Tasker does the actual reporting of the error
	// Notify tasker, he will clean up his ptrs to us

	pJob->m_fUserCancel = fUserCancel;

	m_pMasterTasker->AddResult ( pJob );

	return 1;
}

// ------------------------------------------------------------------------
// Do more stuff after connecting and sending password
int TPump::AfterConnect (TPumpJobConnect* pJob)
{
	// Gosh!  it worked!
	pJob->m_fSuccess = TRUE;
	m_pLink->m_fConnected = true;

	m_pMasterTasker->AddResult ( pJob );

	// TSubject Interface
	if (false == m_pLink->m_fEmergencyPump)
		m_pLink->Update_Observers ();

	return 0;
}

// ------------------------------------------------------------------------
class TGetListData
{
public:
	typedef CTypedPtrList<CPtrList, CString*> TStringPool;

	static const int BUFSIZE;     // size of free list
	TGetListData(TPumpJobBigList* pPumpJob, HANDLE hKillReq, BOOL fPrioPump);
	~TGetListData();

	CRITICAL_SECTION  critAll;
	CRITICAL_SECTION  critProtectFreeList;

	HANDLE      hEventGroupReady;
	HANDLE      hSemSpaceAvail;
	BOOL        fAllDone;
	BOOL        m_fPrioPump;
	TStringPool sBigList;
	TStringPool sFreeList;

	CTime       newCheck;
	TPumpJobBigList* m_pPumpJob;
	HANDLE      hEventReaderDone;
	HANDLE      hPumpKillRequest;
	int         iNTPRet;
	CString     strAck;
};

const int TGetListData::BUFSIZE = 250;

TGetListData::TGetListData(TPumpJobBigList* pPumpJob, HANDLE hKillReq,
						   BOOL fPrioPump)
						   : m_pPumpJob(pPumpJob), hPumpKillRequest(hKillReq), m_fPrioPump(fPrioPump)
{
	int i;
	InitializeCriticalSection(&critAll);
	InitializeCriticalSection(&critProtectFreeList);

	hEventReaderDone = CreateEvent (NULL, TRUE, FALSE, NULL);
	hEventGroupReady = CreateEvent ( NULL,       // no security
		TRUE,       // we want manual reset
		FALSE,      // start out unsignaled
		NULL );     // no name

	hSemSpaceAvail = CreateSemaphore (NULL,  // no security
		BUFSIZE,  // inital count
		BUFSIZE,  // max count
		NULL); // no name
	// make some empty strings
	for (i = 0; i < BUFSIZE; ++i)
		sFreeList.AddTail ( new CString );

	// clear out just in case
	while (!sBigList.IsEmpty())
		delete sBigList.RemoveHead();

	fAllDone = FALSE;

	newCheck = CTime::GetCurrentTime();
	iNTPRet = 0;
}

TGetListData::~TGetListData()
{
	// free the CString pointers
	while (!sFreeList.IsEmpty())
		delete sFreeList.RemoveHead();

	// clear out just in case
	while (!sBigList.IsEmpty())
		delete sBigList.RemoveHead();

	CloseHandle (hEventReaderDone);
	CloseHandle (hSemSpaceAvail);
	CloseHandle (hEventGroupReady);
	DeleteCriticalSection(&critProtectFreeList);
	DeleteCriticalSection(&critAll);
}

///////////////////////////////////////////////////////////////////////////
// Check KillRequest too
//
void TPump::DoGetNewsgroupList ( TPumpJob* pOneJob )
{
	CString* pLine = 0;
	TPumpJobBigList * pJob = static_cast<TPumpJobBigList*>(pOneJob);
	auto_ptr<CObject> job_deleter(pOneJob);

	TGetListData sGL(pJob, m_KillRequest, m_fEmergencyPump);

	// start the READER thread
	AfxBeginThread ((AFX_THREADPROC)fnProcessNewsgroups, (LPVOID) &sGL);

	// we are the WRITER thread
	try
	{
		auto_prio boost(auto_prio::kNormal);  // boost priority of Pump Thread
		int iInitRet; CString strNewAck;

		if (pJob->m_fRecent)
			iInitRet = m_pFeeed->Newgroups (m_pErrList, pJob->m_prevCheck, strNewAck);
		else
			iInitRet = m_pFeeed->StartList (m_pErrList, _T("LIST"), &sGL.iNTPRet, &sGL.strAck );

		if (0 == iInitRet)
		{
			// semaphore counts objects in free list
			WaitForSingleObject (sGL.hSemSpaceAvail, INFINITE);
			{
				TEnterCSection mgr(&sGL.critProtectFreeList);
				pLine = sGL.sFreeList.RemoveHead();
			}
			while (0 == m_pFeeed->NextListLine (m_pErrList, pLine ))
			{
				if (*pLine == ".\r\n")
					break;

				{
					TEnterCSection mgr(&sGL.critAll);
					ResetEvent ( sGL.hEventGroupReady );
					sGL.sBigList.AddTail ( pLine );
					SetEvent ( sGL.hEventGroupReady );
				}

				// get storage string from FreeList
				while (WAIT_TIMEOUT == WaitForSingleObject (sGL.hSemSpaceAvail, 200))
					TRACE0("wait on freelist\n");
				{
					TEnterCSection mgr(&sGL.critProtectFreeList);
					pLine = sGL.sFreeList.RemoveHead();
				}

				// peek at KillRequest
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_KillRequest, 0))
				{
					m_fQuitNNTP = FALSE;
					break;
				}
			}
			// No more data
			delete pLine;
			pLine = 0;

			{
				TEnterCSection mgr(&sGL.critAll);
				sGL.fAllDone = TRUE;
				// one last prod...
				SetEvent ( sGL.hEventGroupReady );
			}
		}
		else
		{
			TEnterCSection mgr(&sGL.critAll);
			sGL.fAllDone = TRUE;
			// one last prod...
			SetEvent ( sGL.hEventGroupReady );
		}
	}
	catch (...)
	{
		m_fQuitNNTP = FALSE;
		if (pLine)
		{
			delete pLine;
			pLine = 0;
		}
		{
			TEnterCSection mgr(&sGL.critAll);
			sGL.fAllDone = TRUE;
			// one last prod...
			SetEvent ( sGL.hEventGroupReady );
		}
		// Wait for the reader thread to finish, before we
		// destroy the goodies in 'sGL'
		WaitForSingleObject(sGL.hEventReaderDone, INFINITE);
		//TRACE0("Leaving DoGetNewsgroupList\n");
		throw;
	}

	// Wait for the reader thread to finish
	WaitForSingleObject(sGL.hEventReaderDone, INFINITE);
	//TRACE0("Leaving DoGetNewsgroupList\n");
}

///////////////////////////////////////////////////////////////////////////
//
//
static int freeCount;
static int lowWater;
static const int UPDATE_FREQUENCY = 20;

static void fnChangeStatusLine(int count, BOOL fPriority)
{
	if (0 == (count % UPDATE_FREQUENCY))
	{
		CString mesg;
		CString szCount;
#if !defined(_DEBUG)
		szCount.Format("%d", count);
#else
		szCount.Format("%d (lomark %d free %d)", count, lowWater, freeCount);
#endif
		AfxFormatString1(mesg, IDS_DOWNLOAD_GROUP, szCount);
		gpUserDisplay->SetText ( mesg , fPriority );
	}
}

enum ELineFormat {
	kSNNM,             // String Number Number Modifier
	kSM,               // String Modifier
	kLineError             // other crap
};

// -----------------------------------------------------------------------------------
ELineFormat  fnAnalyzeLine (CString * pLine,
							LPTSTR  pTok1,  LPTSTR pTok2, LPTSTR pTok3, LPTSTR pTok4)
{
	CString & rLine = *pLine;
	LPCTSTR    pInputLine = rLine;

	*pTok1 = *pTok2 = *pTok3 = *pTok4 = NULL;

	int nFound = _stscanf (pInputLine, _T("%s %s %s %s"), pTok1, pTok2, pTok3, pTok4);

	// test for String | Number | Number | Modifier

	if ((nFound >= 4) && isdigit(*pTok2) && isdigit(*pTok3) && !isdigit(*pTok4))
	{
		return  kSNNM;
	}

	// test for String | Modifier

	if ((2 == nFound) && !isdigit(*pTok2))
	{
		return kSM;
	}

	return kLineError;
}

//////////////////////////////////////////////////////////////////////////////
// 1-22-96  If unknown classification, then don't add it to our list
//          at all.
// Upon return rTrueName points to the name we should use
BOOL
fnModifyLine (CString* pLine, int idxSpace,
			  TGlobalDef::ENewsGroupType* eType,
			  LPTSTR pWorkingBuf, int iBufSize,
			  CString& strRename,
			  CString& rTrueName)
{
	TCHAR  rcTok1[9999];
	TCHAR  rcTok2[9999];
	TCHAR  rcTok4[9999];
	TCHAR  rcTok3[9999];
	BOOL fRet = TRUE;

	int ln = pLine->GetLength();
	if (ln < 3)
		return FALSE;  // just a CRLF?

	if (ln > iBufSize)
		return FALSE;

	ELineFormat ef = fnAnalyzeLine (pLine, rcTok1, rcTok2, rcTok3, rcTok4);
	switch (ef)
	{
	case kSNNM:
		pWorkingBuf = rcTok4;
		break;

	case kSM:
		pWorkingBuf = rcTok2;
		break;

	default:
		{
			CString str; str.LoadString (IDS_ERR_UNKNOWN_GROUP_TYPE);
			gpEventLog->AddInfo ( TEventEntry::kGeneral, str, *pLine );
			return FALSE;

			break;
		}
	}

	rTrueName = pLine->Left (idxSpace);

	// check the modifier

	switch (*pWorkingBuf)
	{
	case 'N':
	case 'n':
		*eType = TGlobalDef::kPostNotAllowed;
		break;
	case 'X':                      // this is disabled.
	case 'x':
		{
			return FALSE;
		}
	case 'Y':
	case 'y':
		*eType = TGlobalDef::kPostAllowed;
		break;

	case 'G':   // Hamster local group
	case 'g':
		*eType = TGlobalDef::kPostAllowed;
		break;

	default:
		{
			CString modToken(pWorkingBuf);
			modToken.MakeLower();

			// it might be '=' to indicate a renaming
			if ('=' == modToken[0])
			{
				*eType = TGlobalDef::kPostAllowed;
				strRename = modToken.Mid(1);

				if (strRename.IsEmpty())
				{
					CString str; str.LoadString (IDS_ERR_UNKNOWN_GROUP_TYPE);
					gpEventLog->AddInfo ( TEventEntry::kGeneral, str, *pLine );
					return FALSE;
				}

				// this is the new name
				rTrueName = strRename;
			}
			// it might be 'm' 'M' or "(Moderated)"
			else if ('m' == modToken[0] || modToken.Find(_T("moder")) != -1)
			{
				*eType = TGlobalDef::kPostModerated;
			}
			else
			{
				CString str; str.LoadString (IDS_ERR_UNKNOWN_GROUP_TYPE);
				gpEventLog->AddInfo ( TEventEntry::kGeneral, str, *pLine );
				return FALSE;
			}
		}
		break;
	}

	return fRet;
}

///////////////////////////////////////////////////////////////////
// Save Newsgroup List to disk
//
void fnSaveGroupResults (TGetListData * pGetList,
						 TGroupList * pGroups,
						 TGroupList * pFreshGroups,
						 int count,
						 const CTime & newCheckTime)
{
	TServerCountedPtr cpNewsServer;

	if (count > 0)
	{
		TCHAR buf[15];
		CString statusMsg;
		AfxFormatString1( statusMsg, IDS_SAVING_NGLIST, _itot(count, buf, 10) );
		gpUserDisplay->SetText ( statusMsg, pGetList->m_fPrioPump );
	}

	if (!pGetList->m_pPumpJob->m_fRecent)
	{
		// We are doing ALL groups
		cpNewsServer->SaveServerGroupList( pGroups );
	}
	else
	{  // Recent groups == Incremental Update
		TGroupList* pAllGroups = 0;
		// this is some heavy duty work, so might as well do it in
		//  this background thread

		// load current groups
		pAllGroups = cpNewsServer->LoadServerGroupList ();

		if (0 == pAllGroups)
			cpNewsServer->SaveServerGroupList( pFreshGroups );
		else
		{
			if (pFreshGroups->NumGroups() > 0)
			{
				// add in the fresh groups
				(*pAllGroups) += *pFreshGroups;

				// save the whole mess back out.
				cpNewsServer->SaveServerGroupList( pAllGroups );
			}
		}
		delete pAllGroups;
	}

	// write out the time
	cpNewsServer->SetNewsGroupCheckTime( newCheckTime );
	cpNewsServer->SaveSettings();
} // fnSaveGroupResults

///////////////////////////////////////////////////////////////////
// Read from the pool
//
UINT fnProcessNewsgroups (LPVOID pVoid)
{
	BOOL fKilled = FALSE;
	TGetListData* pGetList = (TGetListData*) pVoid;
	TGroupList allGroups;
	TGroupList * pFreshGroups = 0;
	CTime newCheckTime = CTime::GetCurrentTime();

	GetGmtTime ( newCheckTime );

	TGetListData::TStringPool& sBigList  = pGetList->sBigList;
	TGetListData::TStringPool& sFreeList = pGetList->sFreeList;

	if (pGetList->m_pPumpJob->m_fRecent)
		pFreshGroups = new TGroupList;

	gfGettingNewsgroupList = TRUE;

	int     iWorkingSize = 4096;
	TCHAR * pWorkingBuf = new TCHAR[iWorkingSize];
	auto_ptr<TCHAR> deleter(pWorkingBuf);

	// status bar will refresh on a timer interval
	TUserDisplay_Auto autoRefreshStatus(TUserDisplay_Auto::kClearDisplay,
		pGetList->m_fPrioPump
		? TUserDisplay_Auto::kPriority
		: TUserDisplay_Auto::kNormal);

	try
	{
		CString* pLine=0;
		BOOL  fEndReader = FALSE;
		DWORD dwWait;
		int   count = 0;
		lowWater = 500;
		HANDLE rEvents[2];
		rEvents[0] = pGetList->hEventGroupReady;
		rEvents[1] = pGetList->hPumpKillRequest;

		CString   strRename;

		for (;;)
		{
			dwWait = WaitForMultipleObjects(2, rEvents, FALSE, INFINITE);
			if (dwWait - WAIT_OBJECT_0 == 1)
			{
				fKilled = TRUE;
				break;
			}
			pLine = NULL;
			{
				TEnterCSection mgr(&pGetList->critAll);
				int size = sBigList.GetCount();
				if (size >= 1)
				{
					pLine = sBigList.RemoveHead();
					if (size == 1)
					{
						// the bucket is empty
						if (pGetList->fAllDone)
							fEndReader = TRUE;
						else
							ResetEvent (pGetList->hEventGroupReady);
					}
				}
				else if (size == 0 && pGetList->fAllDone)
					fEndReader = TRUE;
			} // critsect scope

			if (pLine)
			{
				// character indicates if Posting is Allowed [Yes|No|Moderated]
				int idxSpace = pLine->Find(' ');
				if (idxSpace > -1)
				{
					CString  sTrueName;
					TGlobalDef::ENewsGroupType eType;

					if (fnModifyLine ( pLine, idxSpace,
						&eType,
						pWorkingBuf, iWorkingSize,
						strRename, sTrueName ))
					{
#if defined(_DEBUG)
						if (sTrueName == "local.test.norsk")
							sTrueName = "local.test.n\xF8rsk";

#endif
						if (pGetList->m_pPumpJob->m_fRecent)
							pFreshGroups->AddGroup (sTrueName, WORD(0), eType);
						else
							allGroups.AddGroup (sTrueName, WORD(0), eType);
					}
					fnChangeStatusLine ( ++count, pGetList->m_fPrioPump );
				}
				{
					TEnterCSection pro(&pGetList->critProtectFreeList);
					sFreeList.AddHead ( pLine );
					freeCount = sFreeList.GetCount();
					if (freeCount < lowWater)
						lowWater = freeCount;

					// the free list is bigger
					ReleaseSemaphore (pGetList->hSemSpaceAvail, 1, NULL);
				}
			}

			if (fEndReader)
				break;
		} // for loop

		if (WAIT_OBJECT_0 == WaitForSingleObject(pGetList->hPumpKillRequest,0))
			fKilled = TRUE;

		if (!fKilled && (pGetList->iNTPRet < 300))
			fnSaveGroupResults ( pGetList, &allGroups, pFreshGroups,
			count,  newCheckTime );
	} // try-block
	catch (TException *exc)
	{
		SetEvent(pGetList->hEventReaderDone);
		gfGettingNewsgroupList = FALSE;

		delete pFreshGroups;
		exc->PushError (IDS_ERR_PROCESSING_GROUPS, kInfo);
		exc->PushError (IDS_ERR_MAYBE_CONTINUE, kFatal);
		exc->Display ();
		exc->Delete();

		return 0;
	}

	if (!fKilled)
	{
		LPT_GROUPLISTDONE pDone = new T_GROUPLISTDONE;
		pDone->pGroupList = pFreshGroups;
		pDone->iNTPRet = pGetList->iNTPRet;
		pDone->strAck  = pGetList->strAck;
		AfxGetMainWnd()->PostMessage(WMU_GETLIST_DONE,
			pGetList->m_pPumpJob->m_wParam,
			(LPARAM) pDone);
	}
	gfGettingNewsgroupList = FALSE;

	// calling thread waits for event
	// once this is unlocked, the m_pPumpJob will be destroyed
	SetEvent(pGetList->hEventReaderDone);

	return 0;
} // fnProcessNewsgroups

///////////////////////////////////////////////////////////////////////////
//  Take ownership of pHdrArray
//  Called from DownloadOverview()
//
int TPump::deliver_headers_to_scribe(TPumpJobOverview* pJob,
									 TArtHdrArray*& pHdrArray,
									 BOOL fGetBodies)
{
	if (pHdrArray->GetSize() == 0)
	{
		delete pHdrArray;
		pHdrArray = 0;

		// send back an 'event' for the VCR window
		PostMainWndMsg (WMU_VCR_GROUPDONE);

		return 0;
	}

	TScribePrep::EPrepDone eDone = (TPumpJob::kDoneShow == pJob->GetDisposition())
		? TScribePrep::kDoneDisplay
		: TScribePrep::kDoneNothing;

	TArtHdrArray * pMyPtr = pHdrArray;
	pHdrArray = 0;
	BatchSaveHeader_and_Request ( pJob->GetGroup(),
		pJob->GetGroupID(),
		pMyPtr,
		fGetBodies,
		pJob->GetDisposition() );

	return 0;
}

//-------------------------------------------------------------------------
// isolate each newsgroup name & add it to the Header
//
void TPump::process_ng_line(CString& line, int offset, TArticleHeader* pHdr)
{
	int    len = line.GetLength();
	LPTSTR lpszNG = line.GetBuffer(len) + offset;
	LPTSTR lpszFwd = 0;
	do {
		// advance over WS
		while (*lpszNG && (isspace(*lpszNG) || ',' == *lpszNG))
			++lpszNG;
		if (*lpszNG)
		{
			lpszFwd = lpszNG + 1;

			while (*lpszFwd && !isspace(*lpszFwd) && ',' != *lpszFwd)
				++lpszFwd;
			if (lpszFwd > lpszNG)
			{
				TCHAR tmp = *lpszFwd;
				// cap off a substring
				*lpszFwd = '\0';

				// check for "group1, , group2"
				if (lpszFwd > lpszNG + 1)
					pHdr->AddNewsGroup ( lpszNG );
				*lpszFwd = tmp;
				lpszNG = lpszFwd;
			}
		}
	} while (*lpszNG);
	line.ReleaseBuffer(len);
}

// -------------------------------------------------------------------------
// I want the newspump to treat this set as an atomic operation
void TPump::PingMultiGroup ( TPumpJob * pBaseJob )
{
	auto_ptr<CObject> job_deleter(pBaseJob);

	TPumpJobPingMultiGroup * pMulti =
		static_cast<TPumpJobPingMultiGroup *>(pBaseJob);

	// startup - shutdown status bar
	TUserDisplay_Auto refresher(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump
		? TUserDisplay_Auto::kPriority
		: TUserDisplay_Auto::kNormal);

	while (!pMulti->m_pGroupNames->IsEmpty())
	{
		CString groupName = pMulti->m_pGroupNames->RemoveHead ();

		// put name on status bar
		CString strStatus;
		AfxFormatString1 (strStatus, IDS_CHECKING_GROUP1, LPCTSTR(groupName));
		gpUserDisplay->SetText ( strStatus );

		FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);
		sSG.lpszNewsGroup = groupName;

		TPumpJobPingGroup* pChildJob = new TPumpJobPingGroup( groupName );

		if (0 == m_pFeeed->SetGroup ( m_pErrList, &sSG ))
		{
			pChildJob->fRefreshUI = pMulti->m_pGroupNames->IsEmpty();

			pChildJob->SetGroupResults ( sSG.fOK, sSG.iRet, sSG.first, sSG.last );

			m_CurrentGroup = sSG.strGroupName;
			pChildJob->m_NewsGroup = sSG.strGroupName;  // hard reset

			m_pMasterTasker->AddResult ( pChildJob );
		}
		else
		{
			DWORD   dwErrorID;
			CString strError;
			EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

			switch (eErrorClass)
			{
			case kClassWinsock:
				delete pChildJob;

				//  this is not Job that is important enough to resubmit
				if (WSAEINTR == dwErrorID)
					PostDisconnectMsg (false, false);
				else
					// disconnect and reconnect
					PostDisconnectMsg (false, true);
				break;

			case kClassNNTP:
				{
					TCHAR rcNum[10]; _itot(sSG.iRet, rcNum, 10);
					// no such news group
					CString desc; AfxFormatString1(desc, IDS_WARN_1NOSUCHNG, sSG.lpszNewsGroup);
					CString extDesc;

					AfxFormatString2(extDesc, IDS_WARN_2NOSUCHNGEXT, rcNum, (LPCTSTR) sSG.strAck);
					gpEventLog->AddWarning (TEventEntry::kPump, desc, extDesc);

					// present log window to user
					PostMainWndMsg (WM_COMMAND, ID_VIEW_EVENTLOG);

					m_CurrentGroup.Empty();
					pChildJob->SetGroupResults ( FALSE, sSG.iRet, 0, 0 );
					pChildJob->m_iRet = sSG.iRet;
					pChildJob->m_Ack  = sSG.strAck;
					m_pMasterTasker->AddResult ( pChildJob );

					break;
				}

			default:
				delete pChildJob;
				ERRDLG(dwErrorID, strError);
				break;

			} // switch
		} // else

	} // end while loop
}

// ---------------------------------------------------------------------------
void TPump::ExpireHeadersNotFound (TExpiryData*& pExpiryData)
{
	//TRACE0("Pump posting EXPIRE msg\n");
	extern HWND ghwndMainFrame;

	// pass off to UI thread. pass ownership
	::PostMessage (ghwndMainFrame, WMU_EXPIRE_ARTICLES, 0, (LPARAM)pExpiryData);

	pExpiryData = NULL;

	//TRACE0("Pump done posting EXPIRE msg\n");

	return;
}

// --------------------------------------------------------------------------
// FindBlankLineIndex -- find a line that is all whitespace. Return 0
//    on success.
static int FindBlankLineIndex (CString& body,
							   int& rEndHdr,
							   int& rStartBody)
{
	// start with simple search
	int n = body.Find(_T("\r\n\r\n"));

	if (n != -1)
	{
		rEndHdr = n + 2;
		rStartBody = n + 4;
		return 0;
	}

	// do a more involved search

	int ret = 1;
	int len = body.GetLength();
	TCHAR * pLine = new TCHAR[len+2];
	auto_ptr<TCHAR> sLineDeleter (pLine);

	LPTSTR pBuf = body.GetBuffer(len);

	// make a stream!

	_tstrstream in(pBuf);

	streampos pos = 0;

	while (in.getline (pLine, len+1))   // extracts \n
	{
		bool fEntirelyWS = true;
		LPTSTR pTravel = pLine;

		while (*pTravel)
		{
			if (!_istspace(*pTravel++))
			{
				fEntirelyWS=false;
				break;
			}
		}

		if (fEntirelyWS)
		{
			rEndHdr = pos;
			rStartBody = in.tellg();   // body should be right after This blankline

			ret = 0;
			break;
		}

		pos = in.tellg();
	}

	body.ReleaseBuffer(len);

	return ret;
}

#if defined(PERFMON_STATS)
// class wide function
BOOL TPump::PreparePerformanceMonitoring (LPHANDLE pHandle)
{
	HANDLE hMappedObject;
	TCHAR szMappedObjectName[] = TEXT("GRAVITY_COUNTER_MEMCHUNK");

	// initial value
	pCounterBlock = NULL;

	hMappedObject = CreateFileMapping ((HANDLE) 0xFFFFFFFF,
		NULL,
		PAGE_READWRITE,
		0,
		4096,
		szMappedObjectName);
	if (NULL == hMappedObject)
	{
		//TRACE1 ("Could not create mapped object for perfmon %x\n",
		//	GetLastError ());
		pCounterBlock = NULL;
	}
	else
	{
		// mapped object created okay
		//
		// map the section and assign the counter block pointer
		// to this section of memory
		pCounterBlock = (PPERF_COUNTER_BLOCK)
			MapViewOfFile (hMappedObject,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0);
		if (NULL == pCounterBlock)
		{
			//TRACE1 ("Failed to Map View of File %x\n", GetLastError());

			// cleanup
			CloseHandle (hMappedObject);

			return FALSE;
		}

		ZeroMemory (pCounterBlock, sizeof(DWORD) * 3);

		*pHandle = hMappedObject;
	}

	return TRUE;
}

// class wide function
BOOL TPump::ShutdownPerformanceMonitoring (HANDLE hMap)
{
	// undo MapViewOfFile
	if (pCounterBlock)
	{
		UnmapViewOfFile (pCounterBlock);
		pCounterBlock = NULL;
	}

	// undo CreateFileMapping
	CloseHandle (hMap);
	return TRUE;
}
#endif  // PERFMON_STATS

// ------------------------------------------------------------------------
// CryptoLogin -- Handle MSN login.  Returns 0 for success, or Pushes error
//
int TPump::CryptoLogin (void * pVoidSPA)
{
	CString errorMessage;
	BOOL done = FALSE;
	CString strStatusLine;
	strStatusLine.LoadString (IDS_SPA_STATUS);

	// clear status line when done
	TUserDisplay_Auto auto(TUserDisplay_Auto::kClearDisplay,
		m_fEmergencyPump ?
		TUserDisplay_Auto::kPriority :
	TUserDisplay_Auto::kNormal);

	gpUserDisplay->SetText ( strStatusLine, m_fEmergencyPump );

	TSPASession * pSPA = static_cast<TSPASession*>(pVoidSPA);
	if (m_fEmergencyPump)
		pSPA = &pSPA[1];        // mega super hack

	int authret = 0;
	CString cmd = "AUTHINFO GENERIC";
	CString ack;
	int channel = 0;
	CString strPackage;

	pSPA->m_fNewConversation = true;

	if (!pSPA->m_strPackage.IsEmpty())
		strPackage = pSPA->m_strPackage;
	else
	{
		if (m_pFeeed->WriteLine (m_pErrList, cmd, cmd.GetLength() ) ||
			m_pFeeed->ReadLine (m_pErrList, authret, ack ))
			return 1;

		CStringList strList;
		if (2 != authret/100)    // should be 281
		{
			// push error
			errorMessage.Format(IDS_SPA_ERROR2, authret, LPCTSTR(ack));
			TError sErr(errorMessage, kError, kClassNNTP);
			m_pErrList->PushError (sErr);

			// event log entry
			CString msg; msg.LoadString (IDS_SPA_ERROR);
			gpEventLog->AddError (TEventEntry::kPump, msg, errorMessage);
			return 1;
		}

		for (;;)                 // read list of packages
		{
			ack.Empty();
			if (m_pFeeed->ReadLine ( m_pErrList, ack ))
				return 1;
			if (ack[0] == '.')
				break;
			ack.TrimRight();
			strList.AddTail (ack);
		}
		if (strList.IsEmpty())
		{
			CString msg; msg.LoadString (IDS_SPA_ERROR);

			errorMessage.LoadString (IDS_SPA_ERROR_NOPACKS);

			gpEventLog->AddError (TEventEntry::kPump, msg, errorMessage);

			TError sErr (errorMessage, kError, kClassNNTP);
			m_pErrList->PushError (sErr);
			return 1;
		}

		bool fFoundMatchingPackage = false;
		while (!strList.IsEmpty())
		{
			strPackage = strList.RemoveHead();

			// see what we have
			if (0 == pSPA->InitPackage (strPackage))
			{
				fFoundMatchingPackage = true;
				break;
			}
		}

		if (false == fFoundMatchingPackage)
		{
			CString msg; msg.LoadString (IDS_SPA_ERROR);
			errorMessage.LoadString (IDS_SPA_ERROR_MATCHPACKS);
			gpEventLog->AddError (TEventEntry::kPump, msg, errorMessage);

			TError sErr (errorMessage, kError, kClassNNTP);
			m_pErrList->PushError (sErr);
			return 1;
		}
	}
	cmd.Format ("AUTHINFO GENERIC %s", LPCTSTR(strPackage));

	if (m_pFeeed->WriteLine (m_pErrList, cmd, cmd.GetLength()))
		return 1;

	ack.Empty();
	if (m_pFeeed->ReadLine (m_pErrList, authret, ack))
		return 1;

	if (3 != authret/100)
	{
		pSPA->TermPackage ();
		CString msg; msg.LoadString (IDS_SPA_ERROR);

		errorMessage.Format (IDS_SPA_ERROR2,
			authret, LPCTSTR(ack));

		gpEventLog->AddError (TEventEntry::kPump, msg, errorMessage);

		TError sErr(errorMessage, kError, kClassNNTP);
		m_pErrList->PushError (sErr);

		return 1;
	}

	CString strLeg1;
	DWORD   dwLegError = 0;

	if (0 != pSPA->CreateLeg1 (m_server, strLeg1, dwLegError))
	{
		pSPA->TermPackage ();

		// if we can't get an English description, just show Hex
		if (pSPA->FormatMessage (dwLegError, errorMessage))
			errorMessage.Format (IDS_SPA_HEXERROR1, dwLegError);

		TError sErr(errorMessage, kError, kClassExternal);
		m_pErrList->PushError (sErr);
		return 1;
	}

	cmd.Format ("AUTHINFO TRANSACT %s", LPCTSTR(strLeg1) );

	if (m_pFeeed->WriteLine (m_pErrList, cmd, cmd.GetLength()))
		return 1;

	// read  Leg2 from Server

	ack.Empty();
	if (m_pFeeed->ReadLine (m_pErrList, authret, ack))
		return 1;

	if (3 != authret/100)  // should be 381
	{
		pSPA->TermPackage ();
		errorMessage.Format (IDS_SPA_ERROR2, authret, LPCTSTR(ack));

		TError sErr(errorMessage, kError, kClassNNTP);
		m_pErrList->PushError (sErr);
		return 1;
	}

	CString strLeg3;
	if (0 != pSPA->CreateLeg3 (m_server, ack, strLeg3, dwLegError))
	{
		pSPA->TermPackage ();
		// if we can't get an English description, just show Hex
		if (pSPA->FormatMessage (dwLegError, errorMessage))
			errorMessage.Format (IDS_SPA_HEXERROR1, dwLegError);

		TError sErr(errorMessage, kError, kClassExternal);
		m_pErrList->PushError (sErr);
		return 1;
	}

	cmd.Format ("AUTHINFO TRANSACT %s", LPCTSTR(strLeg3));
	if (m_pFeeed->WriteLine (m_pErrList, cmd, cmd.GetLength()))
		return 1;

	authret = 0;
	ack.Empty();
	if (m_pFeeed->ReadLine (m_pErrList, authret, ack ))
		return 1;

	if (2 != authret / 100) {
		pSPA->TermPackage ();
		errorMessage.Format (IDS_SPA_ERROR2, authret, LPCTSTR(ack));

		TError sErr(errorMessage, kError, kClassNNTP);
		m_pErrList->PushError (sErr);

		return 1;
	}

	// success !
	return 0;
}

// ------------------------------------------------------------------------
//
// output: post message to ghwndMainFrame. data = what has expired.
int TPump::DoPingArticles (TPumpJob * pBaseJob)
{
	auto_ptr<CObject> job_deleter(pBaseJob);
	TPumpJobPingArticles * pPingJob = static_cast<TPumpJobPingArticles *>(pBaseJob);
	int r;

	FEED_SETGROUP sSG(m_logFile, m_fEmergencyPump);

	do
	{
		// switch into NG
		m_CurrentGroup.Empty();
		r = iSyncGroup (pPingJob->GroupName(), &sSG);

		if (r)
			break;

		pPingJob->m_iServerLow = sSG.first;

		POINT ptLogical;
		ptLogical.x = 0;
		ptLogical.y = kiPumpStatusBarRange;

		// turns on automatic refreshing of statusbar - clear when destruct
		TUserDisplay_Auto oRefresher(TUserDisplay_Auto::kClearDisplay,
			m_fEmergencyPump
			? TUserDisplay_Auto::kPriority
			: TUserDisplay_Auto::kNormal);
		CString statusBarText;
		statusBarText.Format ("Verifying local articles in %s", LPCTSTR(pPingJob->GroupName()));
		gpUserDisplay->SetText (statusBarText, m_fEmergencyPump);
		gpUserDisplay->SetRange (ptLogical.x, ptLogical.y, m_fEmergencyPump);

		// use this function, since it's already written
		r = ping_articles (pPingJob->m_pPing,
			TRUE /* fExpirePhase */,
			pPingJob->GetGroupID(),
			pPingJob->m_iServerLow,   // expire arts lower than X
			pPingJob->m_pPing->CountItems(),
			&ptLogical, sSG.est, TRUE);

		if (0 == r)
			return 0;
	} while (false);

	// handle errors here

	DWORD       dwErrorID;
	CString     strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	switch (eErrorClass)
	{
	case kClassNNTP:
		// should display error, since something is weird here  $$
		break;

	case kClassWinsock:
		if (WSAEINTR == dwErrorID)
			PostDisconnectMsg (false, false);
		else
			PostDisconnectMsg (false, true);
		break;

	default:
		ASSERT(0);
		break;
	}

	return 1;
}

// ------------------------------------------------------------------------
// FreeConnection --
int TPump::FreeConnection ()
{
	delete m_pFeeed;
	m_pFeeed = 0;

	return 0;
}

// ------------------------------------------------------------------------
EErrorClass TPump::GetErrorClass (DWORD & dwError, CString & desc)
{
	ASSERT(m_sErrorList.GetSize() >= 1);

	ESeverity   eSeverity;
	EErrorClass eClass;

	m_sErrorList.GetErrorByIndex (0, eSeverity, eClass, dwError, desc);

	return eClass;
}

// ------------------------------------------------------------------------
void TPump::PostDisconnectMsg (bool fIdleDisconnect, bool fReconnect,
							   int iSleepSecs /* =0 */)
{
	// stop doing jobs - this affects CustomizedWait
	m_fProcessJobs = false;

	PostMainWndMsg (WMU_INTERNAL_DISCONNECT,
		m_fEmergencyPump ? 1 : 0,
		fIdleDisconnect);

	// same ID as the menu item
	if (fReconnect)
	{
		// note  ID_FILE_SERVER_RECONNECT  is like ID_FILE_SERVER_CONNECT
		//    except that it keeps track of the retry attemps

		PostMainWndMsg (WM_COMMAND, ID_FILE_SERVER_RECONNECTDELAY);
	}

	// event log stuff
	CString strMsg;
	int     stringID;

	if (fReconnect)
		stringID = IDS_UTIL_RECONNECT;
	else if (fIdleDisconnect)
		stringID = IDS_UTIL_IDLEDCONN;
	else
		stringID = IDS_UTIL_DCONN;

	strMsg.LoadString (stringID);

	gpEventLog->AddInfo (TEventEntry::kPump, strMsg);
}

// ------------------------------------------------------------------------
// Last ditch dialog
static void errorDialog (LPCSTR file, DWORD line, DWORD error, CString & strError)
{
	CString msg;

	if (error)
		msg.Format (_T("Error in %s line %d. ErrorID: %d"), file, line, error);
	else
		msg.Format (_T("Error in %s line %d.\n\n%s"), file, line, LPCTSTR(strError));

	gpEventLog->AddShowError (TEventEntry::kPump,
		_T("Error"),
		msg);
}

// take job ptr and simply and resubmit it into the queue
int TPump::resubmit_job ( TPumpJob * pJob )
{
	m_pLink->m_pJobs->AddJobToFront ( pJob );

	return 0;
}

// ------------------------------------------------------------------------
int TPump::log_nntp_error (CString & ack)
{
	CString msg = "xover failed";

	gpEventLog->AddError (TEventEntry::kPump, msg, ack );
	return 0;
}

// ------------------------------------------------------------------------
// get_xpost_info -- returns 0 for success
int TPump::get_xpost_info (
						   TPumpJobOverview * pJob,
						   int low,
						   int high,
						   void * pVoid,
						   MPBAR_INFO * psBarInfo

						   )
{
	int      r;
	CString  command;
	CString  output;
	bool     fReadxref = true;
	//bool     fReadnewsgroups;

	TYP_OVERVIEW_EXTRA * psExtra = (TYP_OVERVIEW_EXTRA *) pVoid;

	if ( (m_wServerCap & kXRefinXOver) )
	{
		// we are skipping the 'xhdr xref' command

		if (psBarInfo->m_iLogSubRangeLen != -1)
		{
			int fraction = psBarInfo->m_iLogSubRangeLen;

			fraction /= psBarInfo->m_nDivide;

			// user feedback
			gpUserDisplay->SetPos (psBarInfo->m_iLogPos + fraction, FALSE, m_fEmergencyPump);
		}
	}
	else
	{
#if defined(_DEBUG)
		{
			CString statMsg;
			statMsg.Format ("(debug) getting XRefs for arts");
			gpUserDisplay->SetText (statMsg, m_fEmergencyPump);
		}
#endif

		// RLW - Modified so we do not ask for more than 300 articles in one go
		int nStart = low;

		while (nStart <= high)
		{
			command.Format("XHDR xref %d-%d", nStart, (nStart + 300 < high) ? nStart + 300 : high);
			//TRACE("%s\n", command);

			// request this block of xref lines
			if (!m_pFeeed->Direct(m_pErrList, command))
			{
				// Process the reply
				while (true)
				{
					// read cross-posting information (XREF lines)
					if (!m_pFeeed->DirectNext(m_pErrList, output))
					{
						if (output.GetLength() == 3 && output == _T(".\r\n"))
							break; // End of reply data

						if (-1 != output.Find(':'))
						{
							int ai = _ttoi(output);
							if (ai >= low && ai <= high)
								psExtra->m_crossPost[ai - low] = output;
							else
								ASSERT(0);
						}

						psBarInfo->m_nPos++;
						if (psBarInfo->m_iLogSubRangeLen != -1)
						{
							int fraction = MulDiv(psBarInfo->m_iLogSubRangeLen, psBarInfo->m_nPos, psBarInfo->m_nSubRangeLen );
							if (fraction != -1)
							{
								fraction /= psBarInfo->m_nDivide;

								// user feedback
								gpUserDisplay->SetPos(psBarInfo->m_iLogPos + fraction, FALSE, m_fEmergencyPump);
							}
						}
					}
					else // Error reading reply
					{
						r  =  error_xref(_T("Reading xrefs lines"));
						if (r)
							return r;
					}
				} // while (true)
			}
			else // Error sending command
			{
				// return 0 to continue
				if ((r = error_xref(command)) != 0)
					return r;
			}

			nStart += 301;
		} // while (nStart < high)
	} // test bitflag

	if (-1 != psBarInfo->m_iLogSubRangeLen)
		psBarInfo->m_iLogPos += psBarInfo->m_iLogSubRangeLen /
		psBarInfo->m_nDivide;

#if defined(_DEBUG)
	{
		CString statMsg;
		statMsg.Format ("(debug) Getting Newsgroups line for arts ");
		gpUserDisplay->SetText (statMsg, m_fEmergencyPump);
	}
#endif

	psBarInfo->m_nPos = 0;          // reset for this phase
	psBarInfo->m_nDivide = 4;

	// RLW - Modified so we do not ask for more than 300 items in one go
	int nStart = low;

	while (nStart < high)
	{
		command.Format("XHDR newsgroups %d-%d", nStart, (nStart + 300 < high) ? nStart + 300 : high);
		//TRACE("%s\n", command);

		if (!m_pFeeed->Direct (m_pErrList, command))
		{
			while (true)
			{
				if (!m_pFeeed->DirectNext (m_pErrList, output))
				{
					if (3 == output.GetLength() && output == _T(".\r\n"))
						break; // End of reply data

					int ai = _ttoi(output);
					if (ai >= low && ai <= high)
						psExtra->m_groups[ai - low] = output;
					else
						ASSERT(0);

					psBarInfo->m_nPos ++;
					if (psBarInfo->m_iLogSubRangeLen != -1)
					{
						int fraction = MulDiv (psBarInfo->m_iLogSubRangeLen, psBarInfo->m_nPos, psBarInfo->m_nSubRangeLen );
						if (fraction != -1)
						{
							fraction /= psBarInfo->m_nDivide;

							// user feedback
							gpUserDisplay->SetPos(psBarInfo->m_iLogPos + fraction, FALSE, m_fEmergencyPump);
						}
					}
				}
				else // Error receiving reply
				{
					r = error_xref(_T("Reading newsgroups lines"));
					if (r)
						return r;
				}
			} // while(true)
		}
		else // Error sending command
		{
			r = error_xref (command);
			if (r)
				return r;
		}

		nStart += 301;
	} // while (nStart < high)




	//command.Format("xhdr newsgroups %d-%d", low, high);
	//TRACE("%s\n", command);

	//r = m_pFeeed->Direct (m_pErrList, command);

	//if (r)
	//{
	//	fReadnewsgroups = false;
	//	r = error_xref (command);
	//	if (r)
	//		return r;
	//}
	//else
	//	fReadnewsgroups = true;

	//while (fReadnewsgroups)
	//{
	//	r = m_pFeeed->DirectNext (m_pErrList, output);

	//	if (r)
	//	{
	//		r = error_xref (_T("Reading newsgroups lines"));
	//		if (r)
	//			return r;
	//	}

	//	if (3 == output.GetLength() && output == _T(".\r\n"))
	//		break;

	//	int ai = _ttoi(output);
	//	if (ai >= low && ai <= high)
	//		psExtra->m_groups[ai - low] = output;
	//	else
	//		ASSERT(0);

	//	psBarInfo->m_nPos ++;
	//	if (psBarInfo->m_iLogSubRangeLen != -1)
	//	{
	//		int fraction = MulDiv (psBarInfo->m_iLogSubRangeLen, psBarInfo->m_nPos,
	//			psBarInfo->m_nSubRangeLen );
	//		if (fraction != -1)
	//		{
	//			fraction /= psBarInfo->m_nDivide;

	//			// user feedback
	//			gpUserDisplay->SetPos (psBarInfo->m_iLogPos + fraction, FALSE, m_fEmergencyPump);
	//		}
	//	}
	//} // while

	return 0;
}

// ------------------------------------------------------------------------
// return 0 for caller to continue with function
int TPump::error_xref (LPCTSTR cmd)
{
	DWORD   dwErrorID;
	CString strError;
	EErrorClass eErrorClass = GetErrorClass (dwErrorID, strError);

	switch (eErrorClass)
	{
	case kClassNNTP:
		{
			CString strMsg;
			strMsg.Format ("%s returned %s", cmd, LPCTSTR(strError));
			gpEventLog->AddError (TEventEntry::kPump, strMsg);

			m_pErrList->ClearErrors();
		}
		return 0;

	case kClassWinsock:
		if (WSAEINTR == dwErrorID)
		{
			// no reconnect
			PostDisconnectMsg (false, false);
		}
		else
			// fIdle, fReconnect
			PostDisconnectMsg (false, true);
		return 1;

	default:
		ERRDLG(dwErrorID, strError);
		break;
	}
	return 0;
}

