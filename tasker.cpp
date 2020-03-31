/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tasker.cpp,v $
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
/*  Revision 1.4  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:58  richard_wood
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

// tasker.cpp : implementation file
//
//  4-23-96  amc Separate the User-Getting-Headers from the program getting
//               headers.  If the user does it, reload the cur Newsgroup
//

#include "stdafx.h"
#include "tasker.h"

#include "declare.h"
#include "tmutex.h"
#include "newspump.h"
#include "tscribe.h"
#include "globals.h"
#include "arttext.h"

#include "tscribe.h"

#include "smtp.h"

#include "artclean.h"
#include "fileutil.h"
#include "tglobopt.h"

#include "log.h"
#include "pobject.h"
#include "names.h"

#include "bits.h"

#include "ngutil.h"
#include "evtlog.h"

#include "tprnthrd.h"            // gpPrintThread
#include "tdecthrd.h"            // gpDecodeThread
#include "tpendec.h"             // gsChart
#include "rules.h"
#include "custmsg.h"
#include "enumjob.h"
#include "rgconn.h"
#include "topexcp.h"                // TopLevelException()
#include "rgsys.h"
#include "autoprio.h"
#include "utilrout.h"      // PostMainWndMsg
#include "nglist.h"        // TNewsGroupUseLock
#include "server.h"        // TNewsServer
#include "servcp.h"        // TServerCountedPtr
#include "usrdisp.h"       // TUserDisplay
#include "utilerr.h"
#include "newsdoc.h"       // GetPumpObserver

#include "strext.h"        // TStringEx
#include "coding.h"
#include "online.h"        // TOnlineFlag
#include "genutil.h"       // CopyCStringList
#include "rgswit.h"

#define PERFMON_STATS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

LONG  gl_Pump_Busy ;
LONG  gl_Emergency_Busy;
LONG  gl_Scribe_Busy;
LONG  gl_Smtp_Sending;

extern TGlobalOptions * gpGlobalOptions;
extern HWND ghwndMainFrame;
static BYTE gbyCancelPrio;

static const int TASKER_IDLE_MILLISEC = 10000;

// April 29, 2002  - made these globals, so the critsect is never deleted/invalid
TPumpJobs  gsPumpJobs1;
TPumpJobs  gsPumpJobs2;

/////////////////////////////////////////////////////////////////////////////
// TNewsTasker

IMPLEMENT_DYNCREATE(TNewsTasker, CHumbleThread)

#define new DEBUG_NEW

TNewsTasker::TNewsTasker (void)
: m_LastGetHdrs4GroupsTime(CTime::GetCurrentTime()),
m_LastSaveNGListTime(m_LastGetHdrs4GroupsTime)
{
	m_instanceRet = 666;
	m_fTaskerRunning = FALSE;

	m_KillRequest = CreateEvent(NULL, TRUE, FALSE, NULL);   // Off by default
	m_Killed      = CreateEvent(NULL, TRUE, FALSE, NULL);   // Off by default
	m_hEventResReady = CreateEvent(NULL, TRUE, FALSE, NULL);   // Off by default
	m_hMutexResBucket = CreateMutex(NULL,    // no security
		NULL,              // not owned
		"MutexForResults");   // name

	m_hmtxTasker = CreateMutex (NULL, NULL, NULL);
	m_outboxCompactMutex = CreateMutex (NULL, NULL, NULL);
	m_hEventMail = CreateEvent (NULL,
		FALSE,   // Try autoreset
		FALSE,   // Non-signaled
		NULL);

	// the scribe resets this when its queue is backed up
	m_hScribeUndercapacity = CreateEvent (NULL,
		TRUE,   // fManual
		TRUE,   // start out signaled
		NULL);

	m_pPumpLink1 = 0;
	m_pPumpLink2 = 0;

	// make these ptrs to a global struct, so we never have deletion problems.
	m_pJobsPump1 = &gsPumpJobs1;
	m_pJobsPump2 = &gsPumpJobs2;

	start_thread_scribe();

#if defined(PERFMON_STATS)
	TPump::PreparePerformanceMonitoring (&m_hPumpPerfmonData);
#endif

	TRegSystem* pRegSys = gpGlobalOptions->GetRegSystem();

	// start the print thread
	ASSERT (!gpPrintThread);
	gpPrintThread = new TPrintThread;
	gpPrintThread -> CreateThread( CREATE_SUSPENDED );
	gpPrintThread -> SetThreadPriority( pRegSys->GetPrio(TRegSystem::kPrint) );
	gpPrintThread -> ResumeThread();

	// start the decode thread
	ASSERT (!gpDecodeThread);
	gpDecodeThread = new TDecodeThread;
	gpDecodeThread -> CreateThread( CREATE_SUSPENDED );
	gpDecodeThread -> SetThreadPriority( pRegSys->GetPrio(TRegSystem::kDecode) );
	gpDecodeThread -> ResumeThread();

	m_timeLastSaveDecodeJobs = CTime::GetCurrentTime();
}

///////////////////////////////////////////////////////////////////////////
//
TNewsTasker::~TNewsTasker()
{
	CloseHandle (m_hScribeUndercapacity);

	CloseHandle (m_KillRequest);
	CloseHandle (m_Killed);

	CloseHandle (m_hmtxTasker);

	CloseHandle (m_hEventMail);

	CloseHandle (m_outboxCompactMutex);

	CloseHandle (m_hEventResReady);
	CloseHandle (m_hMutexResBucket);

	m_pJobsPump1->cleanup_queue();
	m_pJobsPump1 = NULL;
	m_pJobsPump2->cleanup_queue();
	m_pJobsPump2 = NULL;

	delete_thread_scribe ();

	//TRACE0("Pump:: scribe is freed\n");
}

///////////////////////////////////////////////////////////////////////
static void AnnounceSendFailure ()
{
	CString str;
	str.LoadString (IDS_ERR_SENDING_MESSAGE);

	auto_prio booster(auto_prio::kNormal);
	NewsMessageBox (TGlobalDef::kMainWnd, str);
}

// --------------------------------------------------------------------------
int  TNewsTasker::StartPingCycle(void)
{
	TServerCountedPtr cpNewsServer;

	// Sort them, so we download in the same order as
	//   the CNewsView display
	CStringArray vNames;
	cpNewsServer->GetSubscribedArray().GetSortedRetrieveArray ( false, &vNames );

	// vNames now contains names of newsgroups, ordered by nicknames

	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	BOOL fNormalValidated = FALSE;
	if (validate_pump(1, &fNormalValidated)) // reject if busy
		return 1;

	CStringList * pNameList = new CStringList;

	for (int i = 0; i < vNames.GetSize(); ++i)
		pNameList->AddTail (vNames[i]);

	m_pJobsPump1->AddJob ( new TPumpJobPingMultiGroup(pNameList) );

	return 0;
}

// --------------------------------------------------------------------------
//
int TNewsTasker::StartRetrieveGroup (EGetAction eAction, const CString & ngName)
{
	if (gl_Pump_Busy)
		return 1;

	return ForceRetrieveGroup (eAction, ngName);
}

// --------------------------------------------------------------------------
// Called From:
//          CNewsDoc::OnGetHeadersAllGroups
//
// EGetAction  { kActionUser, kActionProgram }
//
int TNewsTasker::ForceRetrieveGroup (EGetAction eAction, const CString & ngName)
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	BOOL fNormalValidated = FALSE;
	if (validate_pump(1, &fNormalValidated)) // reject if busy
		return 1;

	RetrieveOneGroup ( ngName, TRUE, 0, eAction );

	return 0;
}

//-------------------------------------------------------------------------
//  Multi version of ForceRetrieveGroup
int TNewsTasker::StartRetrieveCycle (EGetAction eAction)
{
	TServerCountedPtr cpNewsServer;

	// Sort them, so we download in the same order as
	//   the CNewsView display
	CStringArray vNames;

	// this will exclude groups that are InActive aka disabled
	cpNewsServer->GetSubscribedArray().GetSortedRetrieveArray ( true, &vNames );

	// vNames now contains names of newsgroups, ordered by nicknames

	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	BOOL fNormalValidated = FALSE;
	if (validate_pump(1, &fNormalValidated)) // reject if busy
		return 1;

	for (int i = 0; i < vNames.GetSize(); i++)
		RetrieveOneGroup ( vNames[i], TRUE, 0, eAction );

	return 0;
}

//-------------------------------------------------------------------------
//
int TNewsTasker::PingList (const CStringList& lstNames)
{
	int ret;
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	BOOL fNormalValidated;
	if ((ret = validate_pump(1, &fNormalValidated)) != 0)  // ok if busy
		return ret;

	// make a copy
	CStringList * pCopy = new CStringList;

	// copy whole list
	CopyCStringList (*pCopy, lstNames);

	m_pJobsPump1->AddJob ( new TPumpJobPingMultiGroup (pCopy) );

	return 0;
}

//-------------------------------------------------------------------------
int TNewsTasker::AddJob (TPumpJob * pJob)
{
	int ret;
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	BOOL fNormalValidated;
	if ((ret = validate_pump(1, &fNormalValidated)) != 0)  // ok if busy
		return ret;

	if (!IsConnected())
		return -10;

	m_pJobsPump1->AddJob ( pJob );
	return 0;
}

//-------------------------------------------------------------------------
// caller gets lock on pump ptr
// iWhich - 0 means check normal, then e-pump
//          1 = normal,
//          2 = emergency
//          3 = check e-pump, then norm
int TNewsTasker::validate_pump(int iWhich, BOOL * pfNormalValid)
{
	switch (iWhich)
	{
	case 0:
		if (m_pPumpLink1)
		{
			*pfNormalValid = TRUE;
			return 0;
		}
		if (m_pPumpLink2)
		{
			*pfNormalValid = FALSE;
			return 0;
		}
		//TRACE0("Pump is null - can't proceed\n");
		return -1;

	case 1:
		if (m_pPumpLink1)
			return 0;
		break;

	case 2:
		if (m_pPumpLink2)
			return 0;
		break;

	case 3:
		if (m_pPumpLink2)
		{
			*pfNormalValid = FALSE;
			return 0;
		}
		if (m_pPumpLink1)
		{
			*pfNormalValid = TRUE;
			return 0;
		}
		//TRACE0("Pump is null - can't proceed\n");
		return -1;
	}

	//TRACE0("Pump is null - can't proceed\n");

	return -1;
}

///////////////////////////////////////////////////////////////////////////
//
BOOL TNewsTasker::RetrieveOneGroup (LPCTSTR    groupName,
									BOOL       fGetAll,
									int        iHdrsLimit,
									EGetAction eGetAction)
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	TPumpJobEnterGroup* pJob = NULL;

	// if the user started this action, show group when done
	TPumpJob::EPumpJobDone eDone = (kActionUser == eGetAction)
		? TPumpJob::kDoneShow
		: TPumpJob::kDoneNothing;

	// investigate status of this group
	pJob = new TPumpJobEnterGroup(groupName, eDone, fGetAll, iHdrsLimit);

	m_pJobsPump1->AddJob ( pJob );
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//
BOOL TNewsTasker::InitInstance()
{
	m_bAutoDelete = 0;

	HANDLE rHandles[3];
	rHandles[0] = m_KillRequest;
	rHandles[1] = m_hEventResReady;
	rHandles[2] = m_hEventMail;

	DWORD         dwWaitRet;

	try
	{
		// Kill request -       quit
		// Result Ready -       process result
		// 2sec elapsed -       check time elapsed, may poll newsgroups again

		for (;;)
		{
			dwWaitRet = WaitForMultipleObjects(
				sizeof(rHandles)/sizeof(rHandles[0]),
				rHandles,
				FALSE,                      // fWaitAll
				TASKER_IDLE_MILLISEC);      // pause seconds

			if (WAIT_FAILED == dwWaitRet)
			{
				break;
			}
			else if (dwWaitRet - WAIT_OBJECT_0 == 0)    // kill request
			{
				m_instanceRet = 1;
				break;
			}
			else if (WAIT_TIMEOUT == dwWaitRet)
			{
				SingleStep();
			}
			else if (dwWaitRet - WAIT_OBJECT_0 == 1)   // result arrived
			{
				ProcessResult();
			}
			else if (dwWaitRet - WAIT_OBJECT_0 == 2)   // smtp result arrived
			{
				Process_SMTP_Result();
			}
		}

		// system code will call delete::
		return FALSE;
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_TASKER_MAIN_LOOP, kInfo);
		except->Display();
		TopLevelException (kThreadTasker);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
		return FALSE;
	}
	catch (CException* pCExc)
	{
		pCExc->ReportError();
		pCExc->Delete();
		TopLevelException (kThreadTasker);
		throw;
		return FALSE;
	}
	catch(...)
	{
		ASSERT(0);
		TopLevelException (kThreadTasker);
		throw;
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////
//
void TNewsTasker::Delete(void)
{
	CHumbleThread::Delete();

	// cancel the pumps
	Disconnect ();

	end_thread_scribe ();

#if defined(PERFMON_STATS)
	TPump::ShutdownPerformanceMonitoring (m_hPumpPerfmonData);
#endif

	// this is the 2nd swipe.  1st call is really from CMainFrame::OnClose
	KillPrintAndDecodeThreads ();

	//TRACE0("Tasker: ok, i'm dead\n");
	SetEvent ( m_Killed );     // Admit that I'm dead.
}

///////////////////////////////////////////////////////////////////////////
//  called from CMainFrame::OnClose
void TNewsTasker::KillPrintAndDecodeThreads ()
{
	if (gpPrintThread)
	{
		TPrintThread * pTemp = gpPrintThread;
		gpPrintThread = NULL;

		pTemp -> KillAndWaitTillDead ();
		// the print-thread is auto-deleting, so we don't have to call delete...
		// just make sure its pointer is null
	}

	if (gpDecodeThread)
	{
		TDecodeThread * pTemp = gpDecodeThread;
		gpDecodeThread = NULL;

		pTemp -> KillAndWaitTillDead ();
		// the decode-thread is auto-deleting, so we don't have to call delete...
		// just make sure its pointer is null
	}
}

///////////////////////////////////////////////////////////////////////////
//
int TNewsTasker::ExitInstance()
{
	CHumbleThread::ExitInstance();
	return m_instanceRet;
}

BEGIN_MESSAGE_MAP(TNewsTasker, CHumbleThread)
	// NOTE - the ClassWizard will add and remove mapping macros here.

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TNewsTasker message handlers

/////////////////////////////////////////////////////////////////////////////
// See if there are any timely tasks to do.  We come here
//   every TASKER_IDLE_MILLISEC
//
void TNewsTasker::SingleStep()
{
	try
	{
		// no work done yet
		if (DoAllOutgoing())
		{
			return;  // work done
		}

		periodic_saveng_cycle ();
		periodic_retr_cycle ();

		periodic_save_decode_jobs ();

		// let MFC clean out temp object maps
		CHumbleThread::OnIdle (1);
	}
	catch(...)
	{
		ASSERT(0);
		throw;
	}
}

// ------------------------------------------------------------------------
// created   March 22, 2003
int TNewsTasker::periodic_save_decode_jobs ()
{
	TRegSwitch* pSwitch = gpGlobalOptions->GetRegSwitch();

	if (pSwitch->GetAutoSaveDecodeJobs ())
	{
		CTime     now  = CTime::GetCurrentTime();
		CTimeSpan span =  now - m_timeLastSaveDecodeJobs;

		if (span.GetTotalMinutes() >= pSwitch->GetAutoSaveDecodeJobsInterval ())
		{
			m_timeLastSaveDecodeJobs = now;

			if (gpDecodeThread)
				gpDecodeThread->FlagSaveJobs();
		}
	}
	return 0;
}

// ------------------------------------------------------------------------
int TNewsTasker::periodic_retr_cycle ()
{
	TServerCountedPtr cpNewsServer;
	try
	{
		BOOL fAuto = cpNewsServer->GetAutomaticCycle ();

		if (fAuto)
		{
			int iInterval = cpNewsServer->GetNewsCheckInterval();

			// example: every 4 minutes add the groups
			CTime       now  = CTime::GetCurrentTime();
			CTimeSpan   span = now - m_LastGetHdrs4GroupsTime;

			if (span.GetTotalMinutes() >= iInterval)
			{
				int pumpJobs = 0;
				TMutex mtx_mgr(m_hmtxTasker);  // get lock on the Pump ptr.

				// make sure pump has not been deleted
				if (0 == m_pPumpLink1)
					return 0;

				// don't overwhelm the pump
				// don't ask for another overview while one is being downloaded
				m_pJobsPump1->GetJobCount(pumpJobs);
				if ((0 == pumpJobs) &&
					(0 == StartRetrieveCycle(kActionProgram)))
				{
					// transfer
					m_LastGetHdrs4GroupsTime = now;
				}
			}
		}
		return 0;
	}
	catch(...)
	{
		ASSERT(0);
		throw;
		return 1;
	}
}

// ------------------------------------------------------------------------
int TNewsTasker::periodic_saveng_cycle ()
{
	try
	{
		TServerCountedPtr cpNewsServer;
		int iInterval = 60; //  minutes

		CTime     now  = CTime::GetCurrentTime();
		CTimeSpan span = now - m_LastSaveNGListTime;

		if (span.GetTotalMinutes() >= iInterval)
		{
			// save these out occasionally to keep things consistent
			cpNewsServer -> SaveDirtybitGroups ();
			cpNewsServer -> SaveReadRange ();

			m_LastSaveNGListTime = CTime::GetCurrentTime();
		}
		return 0;
	}
	catch(...)
	{
		ASSERT(0);
		throw;
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////
//
void TNewsTasker::AddResult (TPumpJob * pResult)
{
	TMutex mgr(m_hMutexResBucket);

	m_Results.AddTail ( pResult );
	SetEvent ( m_hEventResReady );
}

///////////////////////////////////////////////////////////////////////////
//
TPumpJob* TNewsTasker::GetResult (void)
{
	TMutex mgr(m_hMutexResBucket);
	if (1 == m_Results.GetCount())
		ResetEvent (m_hEventResReady);
	return m_Results.RemoveHead();
}

///////////////////////////////////////////////////////////////////////////
//
int TNewsTasker::RegisterGroup(const CString& groupName)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
int TNewsTasker::UnregisterGroup (TNewsGroup* pGrp)
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	// kill pumpjobs, scribejobs
	m_pJobsPump1->CancelNewsgroup ( pGrp->GetName() );

	m_pJobsPump2->CancelNewsgroup ( pGrp->GetName() );

	m_pScribe->CancelNewsgroup ( pGrp->GetName() );

	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
void TNewsTasker::ProcessResult()
{
	try
	{
		TPumpJob* pResult = GetResult ();

		if (0 == pResult)
			return;
		switch (pResult->GetType())
		{
		case (TPumpJob::kJobPingGroup):
			// Q: should we do this in a batch?
			calculate_new_articles(pResult);
			break;

		case (TPumpJob::kJobGroup):
			try
			{
				ResultExpandGroup(pResult);
			}
			catch(...)
			{
				ASSERT(0);
				throw;
			}
			break;

		case (TPumpJob::kJobPost):
			MarkArticleAsPosted( pResult );
			break;

		case (TPumpJob::kJobConnect):
			HandleConnectResult( pResult );
			break;

		default:
			ASSERT(0);  // unknown case

		}
		delete pResult;
	}
	catch(...)
	{
		ASSERT(0);
		throw;
	}
}

// ------------------------------------------------------------------------
// A Range has come back from the Pump
//
// 12-03-96  amc Organized it so that the UseLock and the "m_hmtxTasker"
//                 are separate.
void TNewsTasker::ResultExpandGroup(TPumpJob* pBaseResult)
{
	TServerCountedPtr cpNewsServer;

	//TRACE0(" >>>>>  Enter ExpandGroup\n");

	TPumpJobEnterGroup * pResult = (TPumpJobEnterGroup*) pBaseResult;

	if (pResult->IsOK() == FALSE)
	{
		// UI thread may be blocking. Release it.
		if (pResult->Release ())
		{
			// pump shows error now
		}

		// send back an 'event' for the VCR window
		PostMainWndMsg (WMU_VCR_GROUPDONE);

		return;
	}

	const CString & newsGroup = pResult->GetGroup();
	POINT ptServer;
	int   iGrpLo, iGrpHi;
	pResult->GetGroupRange( iGrpLo, iGrpHi );
	ptServer.x = iGrpLo; ptServer.y = iGrpHi;

	CString msg;
	msg.Format("Server: %s has range %d-%d", LPCTSTR(newsGroup), ptServer.x, ptServer.y);
	LogString ( msg );
	msg.Empty();

	TRangeSet* pLumper = 0;
	TRangeSet* pPing   = 0;
	int  iGroupID;
	TNewsGroup::EStorageOption option = TNewsGroup::kHeadersOnly;

	// Step 1 - Get information out of the newsgroup object
	{
		BOOL fUseLock;
		TNewsGroup * pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, newsGroup, &fUseLock, pNG);

		if (FALSE == fUseLock)
		{
			ASSERT(0);

			// send back an 'event' for the VCR window
			PostMainWndMsg (WMU_VCR_GROUPDONE);

			return;
		}
		int iObjectLow, iObjectHigh;  // current state of NG object
		pNG->GetServerBounds (iObjectLow, iObjectHigh);

		iGroupID = pNG->m_GroupID;

		// trim the lower bound and upper bound (use a rangeset for clumping)
		pLumper = new TRangeSet;

		// range set to see what hdrs are on disk, but expired on newsserver
		pPing   = new TRangeSet;

		// figure out what to request
		pNG->adjust_bound (pResult->m_fGetAll,
			pResult->m_iHdrLimit,
			ptServer, pLumper, pPing);

		option = UtilGetStorageOption(pNG);

		// see if we can avoid work
		if (TNewsGroup::kHeadersOnly == option &&
			(iObjectLow == iGrpLo) &&
			(iObjectHigh == iGrpHi) &&
			(0 == pLumper->RangeCount()))
		{
			// a) Mode-3 may have the headers, but still needs to get Bodies
			//    so check for Mode-2 only.
			//
			// b) numbers from Server match numbers from Object
			// c) nothing to request in pLumper
			delete pLumper;
			delete pPing;

			// send back an 'event' for the VCR window
			PostMainWndMsg (WMU_VCR_GROUPDONE);
			// do nothing
			return ;
		}
		// unlock useLock
	}

	// Step 2 - Get lock on the Pump ptr.
	TMutex mtx_mgr(m_hmtxTasker);

	// we might be shutting down
	if (!IsConnected())
		return;

	TPumpJobs* pSelPumpJobs = m_pJobsPump1;

	try
	{
		InterlockedIncrement ( &gl_Pump_Busy );

		TPumpJob * pPumpJob;
		if (TNewsGroup::kNothing == option)
		{
			/*
			delete pPing;
			BOOL fAllowConnect;
			if (m_pPumpLink1 && SecondIsConnected(&fAllowConnect))
			pSelPumpJobs = m_pJobsPump2;
			pPumpJob = new TPumpJobOverviewUI ( newsGroup, iGroupID,
			iGrpLo,
			cpNewsServer->GetExpirePhase (),
			pLumper );
			*/
		}
		else
		{
			pPumpJob = new TPumpJobOverview ( newsGroup,
				iGroupID,
				iGrpLo,
				pLumper,
				cpNewsServer->GetExpirePhase (),
				pPing,
				pResult->GetDisposition() );
		}
		pSelPumpJobs->AddJob ( pPumpJob );

	} // Try-clause
	catch(...)
	{
		InterlockedDecrement ( &gl_Pump_Busy );
		ASSERT(0);
		throw;
	}
	InterlockedDecrement ( &gl_Pump_Busy );

	//TRACE0(" <<<<< Exit ExpandGroup\n");
} // ExpandGroup

DWORD TNewsTasker::ResumeTasker(void)
{
	DWORD ret;
	m_fTaskerRunning =   ( (ret = ResumeThread()) == 1 );
	return ret;
}

///////////////////////////////////////////////////////////////////////////
// If there is any outbound traffic, send them all at once
//
BOOL TNewsTasker::DoAllOutgoing()
{

	try
	{
		BOOL fWorkDone = FALSE;

		// don't send stuff when we are connectING...
		if (IsConnected())
		{
			//It is tempting to write:  while (CheckOutgoingPosts())  { }
			//But I worry about overloading the TNewsPump Queue with tons
			//	of UUEncoded JPG files.  It would be a real memory hog.

			//	As it is, I come here in two cases:
			//a) every 10 seconds
			//	b) an article is MarkedAsPosted, I come looking for more work

			// send all outbound articles
			if (CheckOutgoingPosts())
				fWorkDone = TRUE;

			// send all outbound emails
			if (CheckOutgoingEmail())
				fWorkDone = TRUE;
		}

		return fWorkDone;
	}
	catch(...)
	{
		ASSERT(0);
		throw;
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////
// If there are outbound articles, queue up a job for self.
//
BOOL TNewsTasker::CheckOutgoingPosts(void)
{
	TServerCountedPtr cpNewsServer;
	BOOL fWorkDone = FALSE;
	TOutbox *pOutbox = cpNewsServer->GetOutbox ();
	int artInt;

	if (!GetOutboxCompactLock (1000))
		return FALSE;

	if (0 == pOutbox)
		throw(new TException(IDS_INVALID_OUTBOX, kFatal));

	try
	{
		// Peek at the Status.  Don't really need to load headers via Open()

		if (pOutbox->PeekArticle(TStatusUnit::kOut, &artInt))
		{
			if (SendOutboundArticle ( artInt ) >= 0)
				fWorkDone = TRUE;
		}
	}
	catch(...)
	{
		ReleaseOutboxCompactLock ();
		throw;
		return FALSE;
	}

	ReleaseOutboxCompactLock ();
	return fWorkDone;
}

BOOL TNewsTasker::GetOutboxCompactLock (DWORD wait)
{
	DWORD dwResult = WaitForSingleObject (m_outboxCompactMutex, wait);
	if (WAIT_OBJECT_0 == dwResult)
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// ReleaseOutboxCompactLock - Release the mutex that is being held for compact
//                            or to prevent compacting...
void TNewsTasker::ReleaseOutboxCompactLock ()
{
	ReleaseMutex (m_outboxCompactMutex);
}

///////////////////////////////////////////////////////////////////////////
// Use the Emergency Pump to retreive the Body, and hand it
// back to the Client (the UI).
// Caller has the responsibility of calling 'Connect'
//
int  TNewsTasker::PriorityArticle(const CString & newsgroup,
								  LONG groupID,
								  BOOL fSaveBody,
								  TArticleHeader* pArtHdr,
								  TFetchArticle* pObjFetch)
{
	return priority_article_driver ( m_pJobsPump2,
		newsgroup,
		groupID,
		fSaveBody,
		pArtHdr,
		pObjFetch );
}

///////////////////////////////////////////////////////////////////////////
// Use the Normal Pump to retreive the Body, and hand it
//    back to the Client (the UI).
// Caller has the responsibility of calling 'Connect'
//
int  TNewsTasker::NonPriorityArticle(const CString & newsgroup,
									 LONG groupID,
									 BOOL fSaveBody,
									 TArticleHeader* pArtHdr,
									 TFetchArticle* pObjFetch)
{
	return priority_article_driver ( m_pJobsPump1,
		newsgroup,
		groupID,
		fSaveBody,
		pArtHdr,
		pObjFetch );
}

///////////////////////////////////////////////////////////////////////////
//  Use the passed in pump to retrieve a priority article
//
int TNewsTasker::priority_article_driver(
	TPumpJobs* pSelPump,
	const CString & newsgroup,
	LONG            groupID,
	BOOL            fSaveBody,
	TArticleHeader* pArtHdr,
	TFetchArticle*  pObjFetch)
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	if (0 == pSelPump)
		throw(new TException (IDS_ERR_SOCK_INVALID, kFatal));

	if (NULL == pObjFetch)
		throw(new TException(IDS_ERR_TASK_THREAD, kFatal));

	int artInt         = pArtHdr->GetNumber();
	int bodyLines      = pArtHdr->GetLines();
	CString strSubjectEmpty;

	// One special case: the normal pump may have retrieved it
	//   but the scribe hasn't written it yet.

	// the normal pump is in the middle of downloading it RIGHT NOW.

	DWORD dwJobFlags = 0;
	if (fSaveBody)
		dwJobFlags |= PJF_SEND_SCRIBE;

	CPoint ptPartID(0,0);
	TPumpJobArticle * pJob = new TPumpJobArticle (
		newsgroup,
		groupID,
		strSubjectEmpty,
		ptPartID,
		artInt,
		bodyLines,
		dwJobFlags,
		pArtHdr,
		pObjFetch);

	pSelPump->AddJob ( pJob );

	// put up the non-blocking cursor
	gpUserDisplay->AddActiveCursor ();

	return 0; // normal
}

///////////////////////////////////////////////////////////////////////////
//
BOOL TNewsTasker::CheckOutgoingEmail (void)
{
	TServerCountedPtr cpNewsServer;
	extern LONG gl_Outbox_Locked;
	BOOL fMessagePresent = FALSE;

	// 6-6-95 Added -amc
	if (!IsConnected())
		return FALSE;

	if (!GetOutboxCompactLock (1000))
		return FALSE;

	if (gl_Outbox_Locked || gl_Smtp_Sending)
	{
		ReleaseOutboxCompactLock ();
		return FALSE;
	}

	// Setup info struct for the SMTP thread
	if (!prepare_msgpool())
	{
		ReleaseOutboxCompactLock ();
		return FALSE;
	}

	CDWordArray vArtInt;
	TOutbox     *pOutbox = cpNewsServer->GetOutbox ();
	if (0 == pOutbox)
	{
		ReleaseOutboxCompactLock ();
		throw(new TException(IDS_INVALID_OUTBOX_OUTGOING, kFatal));
		return FALSE;
	}

	BOOL fOpenBox = FALSE;

	try
	{
		// We can peek at the status without! loading the Headers with Open().

		if (pOutbox->PeekEmails(TStatusUnit::kOut, &vArtInt))
		{
			// looks like there's a message
			pOutbox->Open ();
			fOpenBox = TRUE;

			int total = vArtInt.GetSize();
			for (int i = 0; i < total; ++i)
			{
				int artInt = vArtInt[i];
				TEmailHeader * pMailHdr = new TEmailHeader;
				TEmailText   * pMailTxt = new TEmailText;
				TPersist822Header *pHeader = pOutbox->GetHeader(artInt);
				if (pHeader)
				{
					*pMailHdr = *(pHeader->CastToEmailHeader());

					ASSERT(artInt == pMailHdr->GetNumber());

					pOutbox->LoadBody (artInt, pMailTxt);

					// queue them up
					m_MsgPool.AddMsg ( pMailHdr, pMailTxt );
					(*pOutbox)->Mark (artInt, TStatusUnit::kSending);
					fMessagePresent = TRUE;
				}
				else
					ASSERT(FALSE);
			}
			cpNewsServer->SaveOutboxState ();
		}

		if (fMessagePresent)
			thread_outgoing_email ( pOutbox, vArtInt );
	}
	catch(...)
	{
		m_MsgPool.Reset();  // release any messages we loaded
		if (fOpenBox)
			pOutbox->Close ();
		ReleaseOutboxCompactLock ();
		throw;
		return FALSE;
	}
	if (fOpenBox)
		pOutbox->Close ();
	ReleaseOutboxCompactLock ();
	return fMessagePresent;
}

///////////////////////////////////////////////////////////////////////////
//  Utility
//  Returns FALSE if there is no smtpserver set
//
BOOL TNewsTasker::prepare_msgpool()
{
	TServerCountedPtr cpNewsServer;
	m_MsgPool.Reset();
	CString strAddress;

	strAddress = cpNewsServer -> GetEmailAddress ();
	int iIndex = strAddress.Find ('@');
	if (iIndex == -1)
		// huh ?!?  What's wrong with this address?
		m_MsgPool.m_domain.LoadString (IDS_SMTP_DOMAIN);
	else {
		// not really what the SMTP RFC intended, but hey
		m_MsgPool.m_domain.LoadString (IDS_SMTP_DOMAIN);
		m_MsgPool.m_domain += ".";
		m_MsgPool.m_domain += strAddress.Mid (iIndex+1);
	}
	m_MsgPool.m_hEventDone = m_hEventMail;
	m_MsgPool.m_smtpServer = cpNewsServer -> GetSmtpServer ();

	m_MsgPool.m_smtpServer.TrimLeft();
	m_MsgPool.m_smtpServer.TrimRight();

	return (m_MsgPool.m_smtpServer.IsEmpty()) ? FALSE : TRUE;
}

///////////////////////////////////////////////////////////////////////////
//  Returns TRUE if smtp thread started
//  Note: if pump is not connected, assume that SMTP should not launch
BOOL TNewsTasker::thread_outgoing_email(TOutbox* pOutbox, CDWordArray& vArtInts)
{
	BOOL fStat = FALSE;

	// If the pump is connected, the RAS line is probably OK for SMTP xfer
	if (IsConnected())
	{
		// Start worker thread
		CWinThread * pSmtpThread = AfxBeginThread (
			(AFX_THREADPROC) SMTP_Mailer,
			&m_MsgPool,
			THREAD_PRIORITY_NORMAL,
			0,                 // stack size
			CREATE_SUSPENDED );
		if (pSmtpThread)
		{
			//TRACE0("starting SMTP thread!\n");
			InterlockedIncrement ( &gl_Smtp_Sending );
			pSmtpThread->ResumeThread();                  // let her rip
			fStat = TRUE;
		}
	}

	// either we are not connected or Thread didn't start
	if (fStat)
		return TRUE;
	else
	{
		TServerCountedPtr cpNewsServer;

		// undo the status change
		int tot = vArtInts.GetSize();
		for (int i = 0; i < tot; ++i)
		{
			int iEmailInt = vArtInts[i];
			(*pOutbox)->Mark (iEmailInt, TStatusUnit::kOut);
		}
		cpNewsServer->SaveOutboxState ();
		m_MsgPool.Reset();
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//
void TNewsTasker::Process_SMTP_Result()
{
	try
	{
		TServerCountedPtr cpNewsServer;
		// display transmission errors
		// -- bad messages go into "bad message" folder
		int total = m_MsgPool.m_bindArray.GetSize();
		int cGood, cBad;
		int j;

		TOutbox *pOutbox = cpNewsServer->GetOutbox ();

		// All articles get a new status (kSent || kSendErr)

		for (cGood = cBad = 0, j = 0; j < total; ++j) {
			TEmailMsgBind& rBind = m_MsgPool.m_bindArray.ElementAt(j);
			if (kOK == rBind.m_error) {
				(*pOutbox)->Mark (rBind.m_pHdr->GetNumber(), TStatusUnit::kSent);
				++cGood;
			}
			else {
				++cBad;
				(*pOutbox)->Mark (rBind.m_pHdr->GetNumber(), TStatusUnit::kSendErr);
			}
		}
		cpNewsServer->SaveOutboxState ();

		CString Good;
		Good.Format("%d", cGood);
		CString msg;
		if (0 == cBad)
			AfxFormatString1(msg, IDS_SMTP_RESULT1, Good);
		else
		{
			CString Bad;
			Bad.Format("%d", cBad);
			AfxFormatString2(msg, IDS_SMTP_RESULT2, Good, Bad);
			AnnounceSendFailure ();
		}

		// use post message, so we don't block on the UI.
		PostMainWndMsg (WMU_REFRESH_OUTBOX, 0);

		// REPORT MORE INFO here BEFORE RESETTING.
		log_smtp_result();

		// delete the message memory
		m_MsgPool.Reset();

		InterlockedDecrement( &gl_Smtp_Sending );

		// update the outbox dialog bar as well
		// use post message, so we don't block on the UI.
		PostMainWndMsg(WMU_REFRESH_OUTBOX_BAR, 0);
	}
	catch(...)
	{
		ASSERT(0);
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////
// start connection to the newsserver
// Returns:  0 for success
//           negative for error
//           positive for Busy
int TNewsTasker::Connect(LPCTSTR server)
{
	TServerCountedPtr cpNewsServer;

	// everything in the taglist is marked as Unsubmitted
	if (IsActiveServer())
		cpNewsServer->GetPersistentTags().InitializeSubmit();

	// start normal pump
	int ret = connect_driver(FALSE, server);

	return ret;
}

///////////////////////////////////////////////////////////////////////////
// start emergency pump connection to the newsserver
// Returns:  0 for success
//           negative for error
//           positive for Busy
int TNewsTasker::SecondConnect (LPCTSTR server)
{
	// this call is asynchronous
	return connect_driver (TRUE, server);
}

////////////////////////////////////////////////////////////////
// This is asynchronous
//
int TNewsTasker::connect_driver(BOOL fEmergency, LPCTSTR server)
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	TPumpExternalLink*& pGeneralLink = (fEmergency)
		? m_pPumpLink2 : m_pPumpLink1;

	TPumpJobs*& pGeneralJobs = (fEmergency)
		? m_pJobsPump2 : m_pJobsPump1;

	// it's actually the pump who frees this chunk of memory
	pGeneralLink = new TPumpExternalLink(this,
		server,
		pGeneralJobs,
		m_hScribeUndercapacity,
		fEmergency ? true : false);

	// A pump is a Subject watched by a TObserver
	if (!fEmergency)
	{
		pGeneralLink->AddObserver ( gpDecodeThread );

		pGeneralLink->AddObserver ( CNewsDoc::m_pDoc->GetPumpObserver() );

		pGeneralLink->AddObserver ( gpsOnlineFlag );
	}

	int iPrio = gpGlobalOptions->GetRegSystem()->GetPrio( fEmergency ?
		TRegSystem::kPumpE : TRegSystem::kPumpN );

	// add the 'connect job
	pGeneralJobs->AddJobToFront(new TPumpJobConnect (fEmergency));

	// RLWTODO 1 : we've created a new TPumpExternalLink and pass a pointer to it to the pump
	CWinThread * pThread = AfxBeginThread(TPump::ThreadFunction, (PVOID)pGeneralLink, iPrio);

	// RLWTODO 1a : killrequest handle should be available here?
	// used when disconnecting
	pGeneralLink->m_hPumpThread = pThread->m_hThread;

	while (false == pGeneralLink->m_fPumpCtorFinished)
		Sleep (50);

	//TRACE1("connection busy for %d\n", iPrio);

	// pLink = 0          :   initial state
	// pLink != 0,  !fConnected   : connecting
	// pLink != 0,  fConnected    : running
	return 0;
}

// ------------------------------------------------------------------------
//  drop connections to the newsserver
//    what is m_pPump  is null?
void TNewsTasker::Disconnect()
{
	TRACE("TNewsTasker::Disconnect >\n");
	disconnect_normal();
	disconnect_prio();
	TRACE("TNewsTasker::Disconnect <\n");
}

// ------------------------------------------------------------------------
void TNewsTasker::SecondDisconnect()
{
	TRACE("TNewsTasker::SecondDisconnect >\n");
	// disconnect the emergency pump
	disconnect_prio();
	TRACE("TNewsTasker::SecondDisconnect <\n");
}

///////////////////////////////////////////////////////////////////////////
//
//
void TNewsTasker::disconnect_normal()
{
	TRACE("TNewsTasker::disconnect_normal >\n");
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	disconnect_helper(m_pPumpLink1, m_pJobsPump1);
	TRACE("TNewsTasker::disconnect_normal <\n");
}

///////////////////////////////////////////////////////////////////////////
// disconnect_prio
//
void TNewsTasker::disconnect_prio()
{
	TRACE("TNewsTasker::disconnect_prio >\n");
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	disconnect_helper(m_pPumpLink2, m_pJobsPump2);
	TRACE("TNewsTasker::disconnect_prio <\n");
}

///////////////////////////////////////////////////////////////////////////
// caller must lock  'm_hmtxTasker'
void TNewsTasker::disconnect_helper(TPumpExternalLink * & rpLink, TPumpJobs * pPumpJobs)
{
	TRACE("TNewsTasker::disconnect_helper >\n");
	if (0 == rpLink)
	{
		TRACE("TNewsTasker::disconnect_helper : rpLink is NULL <\n");
		return;
	}

	HANDLE hPumpThread = rpLink->m_hPumpThread;
	bool   fEpump      = rpLink->m_fEmergencyPump;

	rpLink->m_fUserStop = true;

	// RLWTODO 4 : we set killrequest here.
	TRACE("TNewsTasker::disconnect_helper : Setting m_hKillRequest event\n");
	SetEvent(rpLink->m_hKillRequest);

	int n = 0;

	while (WAIT_TIMEOUT == WaitForSingleObject(hPumpThread, 100))
	{
		if (fEpump)
			TRACE("TNewsTasker::disconnect_helper : Waiting for Pump #2 to end %d\n", ++n);
		else
			TRACE("TNewsTasker::disconnect_helper : Waiting for Norm Pump to end %d\n", ++n);
	}

	TRACE("TNewsTasker::disconnect_helper : hPumpThread finished\n");

	// the pump will free the rpLink ptr.
	rpLink = NULL;

#if defined(_DEBUG)
	if (fEpump)
		TRACE("TNewsTasker::disconnect_helper : Tasker : Done waiting for Pump #2\n");
	else
		TRACE("TNewsTasker::disconnect_helper : Tasker : Done waiting for Normal Pump\n");
#endif

	// currently this weeds out 'kBigList
	pPumpJobs->OnDisconnect();
	TRACE("TNewsTasker::disconnect_helper <\n");
}

///////////////////////////////////////////////////////////////////////////
// This is called often so make sure we can time-out and abort.
//
// diligent callers will pass in boolean, to see if we are Connecting.
//
//
BOOL TNewsTasker::IsConnected (bool * pfConnecting /* =NULL */)
{
	try
	{
		if (pfConnecting)
			*pfConnecting = false;

		TMutex mtx_mgr(m_hmtxTasker, 500);   // get lock on the Pump ptr.

		if (0 == m_pPumpLink1)
			return FALSE;

		if (pfConnecting)
			*pfConnecting = true;

		return m_pPumpLink1 -> m_fConnected;
	}
	catch (TException *pE)
	{
		pE->Delete();
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////
// Actually query the state of the pump
//
BOOL TNewsTasker::SecondIsConnected(BOOL* pfContinue, BOOL* pfNull /* = NULL */)
{
	BOOL fOnline = FALSE;
	try
	{
		TMutex mtx_mgr(m_hmtxTasker, 500);   // get lock on the Pump ptr.

		if (0 == m_pPumpLink2)
		{
			if (pfNull) *pfNull = TRUE;
			*pfContinue = TRUE;
			return FALSE;
		}

		if (pfNull)
			*pfNull = FALSE;

		if (false == m_pPumpLink2->m_fConnected)
		{
			//TRACE("  SIC - pump is connecting\n");
			fOnline     = FALSE;
			*pfContinue = FALSE;
		}
		else
		{
			fOnline     = TRUE;
			*pfContinue = TRUE;
		}

		return fOnline;
	}
	catch (TException *pE)
	{
		pE->Delete();
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////
// Retrieve Tagged Articles
BOOL TNewsTasker::RetrieveArticle ( const CString& groupName,
								   LONG  groupID,
								   int artInt,
								   int iLineCount )
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	ASSERT(groupName.IsEmpty() == FALSE);

	if ( m_pPumpLink1 && m_pPumpLink1->m_fConnected )
	{
		m_pJobsPump1->RequestBody (groupName,
			groupID,
			artInt,
			iLineCount,
			FALSE,           // front of queue?
			PJF_TAG_JOB | PJF_SEND_SCRIBE);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////
// returns 0 for success
int TNewsTasker::EraseAllTagJobs ()
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.

	if ( m_pJobsPump1 )
	{
		// erase flag  PJF_TAG_JOB
		return m_pJobsPump1->EraseAllTagJobs ();
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////
// Scan for TArticleHeader::kOut
// return 0 on success,  -1 for error
int TNewsTasker::SendOutboundArticle ( int iArtInt )
{
	TServerCountedPtr cpNewsServer;
	BOOL fOutboxOpen = FALSE;
	TOutbox * pOutbox;
	CMemFile* pmemFile = 0;
	CStringList groups;
	int     iReturn = 0;
	bool fNewTopic = true;
	try
	{
		TArticleHeader* pHdr = 0;
		TArticleText    sText;
		pOutbox = cpNewsServer->GetOutbox ();

		pOutbox->Open();
		fOutboxOpen = TRUE;

		pHdr = (TArticleHeader*) pOutbox->GetHeader( iArtInt );
		if (NULL == pHdr)
		{
			// Should I remove it?

			// this is not good.
			pOutbox->Close ();
			return -1;
		}
		pHdr->GetNewsGroups ( &groups );

		ASSERT(iArtInt == pHdr->GetNumber());

		pOutbox->LoadBody ( iArtInt, &sText );

		pmemFile = new CMemFile;
		int iPrepareStatus = ArticlePreparePost ( pHdr, &sText, pmemFile,
			cpNewsServer->GetGenerateOwnMsgID(),
			&fNewTopic );

		pHdr = 0;  // we are done with you
		pOutbox->Close ();
		fOutboxOpen = FALSE;

		if (0 != iPrepareStatus)
		{
			// returns a String ID that describes error
			CString sEN;  sEN.LoadString(iPrepareStatus);
			CString str; str.LoadString (IDS_ERR_FORMATTING_POST);
			gpEventLog->AddError (TEventEntry::kTasker, str, sEN);

			// status of this article goes to 'Error'
			(*pOutbox)->Mark(iArtInt, TStatusUnit::kSendErr);
			cpNewsServer->SaveOutboxState ();

			delete pmemFile;
			pmemFile = 0;
			AnnounceSendFailure ();
		}
		else
		{
			TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
			if (m_pPumpLink1)
			{
				int iMarkRet;

				// status of this article goes to 'Sending'
				// note - we do this when we know the pump is OK
				iMarkRet = (*pOutbox)->Mark( iArtInt, TStatusUnit::kSending );

				if (0 != iMarkRet)
				{
					// someone else has gotten to this message already! It is already
					// marked as 'Sending'.
					delete pmemFile;
					pmemFile = 0;
					iReturn = -1;
				}
				else
				{
					cpNewsServer->SaveOutboxState ();

					TPumpJobPost * pJob = new TPumpJobPost ( iArtInt, pmemFile, groups.GetHead(),
						fNewTopic );

					m_pJobsPump1->AddJobToFront ( pJob );
					pmemFile = 0;
				}
			}
			else
			{
				delete pmemFile;
				pmemFile = 0;
				AnnounceSendFailure ();
			}
		}
	}
	catch (TException *pE )
	{
		if (fOutboxOpen)
			pOutbox->Close();
		delete pmemFile;
		// update the outbox dialog bar
		PostMainWndMsg(WMU_REFRESH_OUTBOX_BAR, 0);
		AnnounceSendFailure ();
		TException *ex = new TException(*pE);
		pE->Delete();
		throw(ex);
		return iReturn;
	}
	if (fOutboxOpen)
		pOutbox->Close();
	delete pmemFile;

	// update the outbox dialog bar
	PostMainWndMsg (WMU_REFRESH_OUTBOX_BAR, 0);

	return iReturn;
}

///////////////////////////////////////////////////////////////////////////
//
void TNewsTasker::MarkArticleAsPosted(TPumpJob* pBaseResult)
{
	try
	{
		// committed
		PostMainWndMsg (WMU_REFRESH_OUTBOX);
		PostMainWndMsg (WMU_REFRESH_OUTBOX_BAR);
	}
	catch (TException *pE)
	{
		ASSERT(0);
		TException *ex = new TException(*pE);
		pE->Delete();
		throw(ex);
		return;
	}

	//TRACE0("posted ok - Looking for more outbound traffic\n");
	DoAllOutgoing();
}

// ------------------------------------------------------------------------
// rules has analyzed the header and wants to throw this article away
//  (cancel download of the article text)
BOOL TNewsTasker::CancelArticleBody ( const CString& groupName, int artInt )
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	if (0 == m_pJobsPump1)
		return FALSE;
	return m_pJobsPump1->CancelArticleBody ( groupName, artInt );
}

// ------------------------------------------------------------------------
BOOL TNewsTasker::CancelRetrieveJob ( const CString& groupName, int artInt )
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	if (0 == m_pJobsPump1)
		return FALSE;
	return m_pJobsPump1->CancelRetrieveJob ( groupName, artInt );
}

// ------------------------------------------------------------------------
// GetStatusData -- query data from Pump objects, via the m_pAlias ptrs.
//    before the pump does cleanup on itself, the m_pAlias ptrs are zeroed out
void TNewsTasker::GetStatusData(int& iFetch)
{
	try
	{
		iFetch = 0;

		TMutex mtx_mgr(m_hmtxTasker, 500);

		int pumpJobs = 0;

		if (m_pJobsPump2)
		{
			// count the Queued jobs in Emergency Pump
			m_pJobsPump2->GetJobCount( pumpJobs );
			iFetch += pumpJobs;
		}

		if (m_pJobsPump1)
		{
			pumpJobs = 0;
			// count the Queued jobs in Normal Pump
			m_pJobsPump1->GetJobCount( pumpJobs );
			iFetch += pumpJobs;
		}

		// the current jobs
		iFetch += gl_Pump_Busy;      // covers both types of pumps
	}
	catch (TException *pE)
	{
		pE->Delete();
		// catch the mutex timeout
	}
}

// ------------------------------------------------------------------------
//
int  TNewsTasker::TaskerFetchBody (
								   const CString & groupName,
								   int groupID,
								   const CString & subject,
								   CPoint & ptPartID,
								   int artInt,
								   int iLines,
								   TFetchArticle* pobjFetch,
								   BOOL fPrioPump,
								   BOOL bMarkAsRead /* wuz TRUE */,
								   BOOL bEngage /* = TRUE */)    // Try engaging if not engaged
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	TPumpJobs* pSelectedJobs;
	int iTry;
	BOOL fRet;

	// if the caller doesn't want us to initiate a connection, make sure we're
	// already connected
	if (!bEngage && (!m_pPumpLink1))
		return -1;

	if (fPrioPump)
	{
		// connect on demand
		if ((iTry = engage_priopump ()) != 0)
			return iTry;
		pSelectedJobs = m_pJobsPump2;

		if (!m_pPumpLink2 || !m_pPumpLink2->m_fConnected)
			return -2;
	}
	else
	{
		// connect on demand
		if ((iTry = engage_normpump ()) != 0)
			return iTry;
		pSelectedJobs = m_pJobsPump1;
		if (!m_pPumpLink1 || !m_pPumpLink1->m_fConnected)
			return -2;
	}

	fRet = pSelectedJobs->FetchBody(groupName, groupID,
		subject, ptPartID,
		artInt, iLines, pobjFetch, bMarkAsRead);
	return fRet ? 0 : -3;
}

///////////////////////////////////////////////////////////////////////////
// User has clicked on this newsgroup
//   caller must check for storage option: NOTHING
// by default this is a kActionUser, which will reopen/refresh the newsgroup
//   after getting headers
BOOL TNewsTasker::PrioritizeNewsgroup (
									   LPCTSTR    groupName,
									   BOOL       fGetAll,
									   int        iHdrsLimit,
									   EGetAction eAction /* = TNewsTasker::kActionUser */ )
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	if (!IsConnected ())
		return FALSE;

	// ask pump to prioritize any jobs for this group
	BOOL fJobExists = m_pJobsPump1->QueueReOrder ( groupName );
	if (fJobExists)
	{
		// we moved a job to the Head of the queue
		return TRUE;
	}

	// rules --- reset any rule-based text-substitution
	ResetAllRuleSubstitution ();

	// nothing was found. do a fresh overview
	RetrieveOneGroup ( groupName, fGetAll, iHdrsLimit, eAction );
	return TRUE;
}

// ------------------------------------------------------------------------
// VerifyLocalHeaders --
int  TNewsTasker::VerifyLocalHeaders (TNewsGroup * pNG)
{
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	if (!IsConnected ())
		return 1;

	int iServerLo, iServerHi;
	pNG->GetServerBounds (iServerLo, iServerHi);

	TPumpJobPingArticles * pJob =
		new TPumpJobPingArticles(pNG->m_GroupID, pNG->GetName(),
		pNG->GetBestname(), iServerLo);

	pNG->CalculatePingSet (true, pJob->m_pPing);

	m_pJobsPump1->AddJob ( pJob );
	return 0;
}

static int smtp_errorid (ESmtpError eSmtpError)
{
	int idStage = 0;

	switch (eSmtpError)
	{
	case kMTA_Unavailable:        idStage = IDS_SMTP_NOMTA;        break;
	case kConnectFailure:         idStage = IDS_SMTP_CONNECT;      break;
	case kHelloFailure:           idStage = IDS_SMTP_HELLO;        break;
	case kFromFailure:            idStage = IDS_SMTP_FROM;         break;
	case kRecipFailure:           idStage = IDS_SMTP_RECIP;        break;
	case kDataFailure:            idStage = IDS_SMTP_DATA;         break;
	case kFinalFailure:           idStage = IDS_SMTP_FINAL;        break;
	case kRecipCCFailure:         idStage = IDS_SMTP_FAILCC;       break;
	case kRecipBCCFailure:        idStage = IDS_SMTP_FAILBCC;      break;
	case kWinsockFailure:         idStage = IDS_SMTP_WINSOCKERR;   break;
	}
	return idStage;
}

void TNewsTasker::log_smtp_result()
{
	CString stage;
	int     idStage = smtp_errorid (m_MsgPool.m_eError);
	if (idStage)
	{
		// report Big errors with the system
		CString err; err.LoadString (IDS_SMTP_DESCFAIL);
		stage.LoadString ( idStage );

		// if we have a response string from the server, show it
		if (!m_MsgPool.m_strServerAck.IsEmpty())
		{
			stage += "\n";
			stage += m_MsgPool.m_strServerAck;
		}
		gpEventLog->AddError ( TEventEntry::kTasker,  err, stage );
		return;
	}
	int tot = m_MsgPool.m_bindArray.GetSize();
	for (int i = 0; i < tot; i++)
	{
		TEmailMsgBind& rBind = m_MsgPool.m_bindArray.ElementAt(i);
		idStage = smtp_errorid( rBind.m_error );
		if (idStage)
		{
			CString err; err.LoadString (IDS_SMTP_DESCFAILITEM);
			stage.LoadString (idStage);
			CString details;
			CString str; str.LoadString (IDS_ERR_SERVER);
			details.Format (str,
				(LPCTSTR) stage,
				(LPCTSTR) rBind.m_errString);
			gpEventLog->AddError(TEventEntry::kTasker, err, details);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// for the CmdUI Update
//
BOOL TNewsTasker::CanRetrieveCycle(void)
{
	try
	{
		// get lock on the Pump ptr.
		TMutex mtx_mgr(m_hmtxTasker, 100);

		if (NULL == m_pPumpLink1)
			return FALSE;

		// Pump is busy - cant shovel groups
		if (gl_Pump_Busy)
			return FALSE;
		return TRUE;
	}
	catch (TException *pE)
	{
		pE->Delete();
		// just return TRUE
		return TRUE;
	}
}

///////////////////////////////////////////////////////////////////////////
//  THREAD_PRIORITY_BELOW_NORMAL
//  THREAD_PRIORITY_ABOVE_NORMAL
//  THREAD_PRIORITY_HIGHEST
void TNewsTasker::start_thread_scribe ()
{
	TServerCountedPtr cpNewsServer;     // smart pointer

	TRegSystem* pRegSys = gpGlobalOptions->GetRegSystem();

	CSingleLock mtx_mgr(&m_mtxProtectsScribe, TRUE);

	m_pScribe = new TNewsScribe ( m_hScribeUndercapacity, cpNewsServer );
	m_pScribe->CreateThread ( CREATE_SUSPENDED );
	m_pScribe->SetThreadPriority ( pRegSys->GetPrio(TRegSystem::kScribe) );
	m_pScribe->ResumeScribe ();
}

void TNewsTasker::end_thread_scribe ()
{
	CSingleLock mtx_mgr(&m_mtxProtectsScribe, TRUE);
	if (m_pScribe && m_pScribe->IsRunning())
	{
		// Ask the NewsScribe to shutdown
		SetEvent (m_pScribe->m_KillRequest);
		WaitForSingleObject (m_pScribe->m_Killed, INFINITE);
	}
}

void TNewsTasker::delete_thread_scribe ()
{
	CSingleLock mtx_mgr(&m_mtxProtectsScribe, TRUE);

	delete m_pScribe;
	m_pScribe = 0;
}

void TNewsTasker::AddScribeJob (TScribePrep * pPrep)
{
	CSingleLock mtx_mgr(&m_mtxProtectsScribe, TRUE);
	m_pScribe->AddPrep (pPrep);
}

BOOL TNewsTasker::NormalPumpBusy ()
{
	return (gl_Pump_Busy > 0);
}

BOOL TNewsTasker::EmergencyPumpBusy ()
{
	return (gl_Emergency_Busy > 0);
}

// ------------------------------------------------------------------------
// Called from the UI thread
void TNewsTasker::StopEmergencyPump()
{
	TRACE("TNewsTasker::StopEmergencyPump >\n");
	disconnect_prio();

	//TRACE0("Stop emergency pump - boom1\n");

	// the newspump will throw an exception

	gbyCancelPrio = TRUE;
	TRACE("TNewsTasker::StopEmergencyPump <\n");
}

///////////////////////////////////////////////////////////////////////////
// make sure the Normal pump is up and ready for business
//
// Returns: 0 for success
int TNewsTasker::engage_normpump ()
{
	TServerCountedPtr cpNewsServer;
	TMutex mtx_mgr(m_hmtxTasker);   // get lock on the Pump ptr.
	BOOL fConnected;
	int  iTry;

	fConnected = IsConnected ();
	if (fConnected)
		return 0;         // everything is fine

	//gpUserDisplay->AddActiveCursor ();
	iTry = Connect ( cpNewsServer->GetNewsServerAddress() );
	mtx_mgr.Release();
	//gpUserDisplay->RemoveActiveCursor ();

	if (0 != iTry)
		return -1;
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// make sure the emergency pump is up and ready for business
// This must be synchronous
//
// Returns: 0 for success
int TNewsTasker::engage_priopump ()
{
	TServerCountedPtr cpNewsServer;
	int iLoop = 0;                      // just so we don't loop forever
	int iSleepTime = 0;
	BOOL fConnected, fContinueConnect;
	BOOL fNullPump;

	while (iLoop++ < 1000 && iSleepTime < 60000)
	{
		fContinueConnect = FALSE;
		fConnected = SecondIsConnected ( &fContinueConnect, &fNullPump );
		if (fConnected)
		{
			//TRACE("EPP - Case 1 - online\n");
			return 0;         // we are online!
		}

		if (fContinueConnect)
		{
			if (!fNullPump)
			{
				// delete the Pump carcass
				disconnect_prio();
			}

			//TRACE("EPP - starting 2nd pump\n");

			// this call is asynchronous
			LPCTSTR sn = cpNewsServer->GetNewsServerAddress();
			int iTry = SecondConnect ( sn );

			if (0 == iTry)          // function success
			{
				// connection in progress, keep waiting
			}
			else if (iTry < 0)      // error - exit
			{
				//TRACE("EPP - Case 2.3 - wait on async connect\n");
				return -1;
			}
			else
			{                    // busy - wait
				//TRACE("EPP - Case 2.4 - connecting? disconnecting?\n");
				Sleep (50);
				iSleepTime += 50;
			}
		}
		else  // fContinueConnect == false
		{
			// 5-2-96  Pump is either connecting or shutting down. I think
			//         I will pause and then check again.
			//TRACE("EPP - Case 3 - pause\n");
			Sleep (50);
			iSleepTime += 50;
		}
	}
	//TRACE("EPP - Case 4 - leave loop (-1)\n");
	return -1;
}

// ------------------------------------------------------------------------
BOOL TNewsTasker::HandleConnectResult (TPumpJob* & pResult)
{
	TPumpJobConnect* pJob = (TPumpJobConnect*) pResult;

	// Connected and all is well
	if (pJob->m_fSuccess)
	{

	}
	else
	{
		bool fReport     = false;

		if (pJob->m_fDeath)   // pump aborted after running
		{
			fReport  = false;
		}
		else if (pJob->m_fUserCancel)  // no need to report anything
		{
			fReport  = false;
		}
		else          // connection failed
		{
			fReport  = true;
		}

		if (fReport)
			ReportConnectError (pResult);
	}
	return TRUE;
}

// ------------------------------------------------------------------------
void TNewsTasker::ReportConnectError(TPumpJob* & pBaseJob)
{
	TPumpJobConnect* pJob = (TPumpJobConnect*) pBaseJob;

	if (pJob->m_fSuccess)
		return;

	// transfer ownership of ptr to UI
	PostMessage (ghwndMainFrame, WMU_SHOW_CONNECT_RESULT, 0,
		(LPARAM) (void*) pJob);

	// we don't own this ptr
	pBaseJob = 0;
}

///////////////////////////////////////////////////////////////////////////
//  calling function will delete the job
//
void TNewsTasker::calculate_new_articles(TPumpJob* pBaseJob)
{
	try
	{
		TServerCountedPtr cpNewsServer;
		TNewsGroup* pNG = 0;
		int srv_lo, srv_hi;
		TPumpJobPingGroup* pJob = (TPumpJobPingGroup*) pBaseJob;

		// group no longer exists?
		if (pJob->IsOK())
		{
			pJob->GetGroupRange(srv_lo, srv_hi);
			BOOL fUseLock;
			TNewsGroupUseLock useLock(cpNewsServer, pJob->GetGroup(), &fUseLock, pNG);
			if (fUseLock)
				pNG->SetServerBounds(srv_lo, srv_hi);
		}
		else
		{

		}

		// update the Server-New column
		if (pJob->fRefreshUI)
			::PostMessage(ghwndMainFrame, WM_COMMAND, WMC_DISPLAYALL_ARTCOUNT, 0);

	}
	catch(...)
	{
		ASSERT(0);
		throw;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//
int TNewsTasker::RetrieveNewsgroups (BOOL fGetAll,
									 WPARAM wParam,   // black box
									 CTime& since)
{
	if (0 == m_pPumpLink1)
		throw(new TException(IDS_ERR_PUMP_NOT_STARTED, kFatal));

	TPumpJobBigList* pJob = new TPumpJobBigList(!fGetAll,
		wParam, since);
	m_pJobsPump1->AddJob ( pJob );
	return 0;
}

//-------------------------------------------------------------------------
BOOL TNewsTasker::IsSendingEmail ()
{
	return gl_Smtp_Sending;
}

