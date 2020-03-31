/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutlthrd.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:21  richard_wood
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

// tutlthrd.cpp -- TUtilityThread abstract class.  Derive from this class for
// worker threads that process job queues

#include "stdafx.h"                 // standard junk
#include "tutlthrd.h"               // this file's prototypes
#include "tutljob.h"                // TUtilityJob, ...
#include "resource.h"               // IDS_*
#include "tdecjob.h"                // TDecodeJob
#include "mplib.h"                  // TPath
#include "globals.h"                // gpStore, TNewsServer
#include "topexcp.h"                // TopLevelException()
#include "server.h"                 // TNewsServer
#include "tutlmsg.h"                // ID_CURRENT_CHANGE, ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(TUtilityThread, CWinThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
// TUtilityThread -- constructor
TUtilityThread::TUtilityThread()
: m_fSaveJobs(false)
{
	// param 1: NULL means "no security"
	// param 2: TRUE means "manual reset"
	// param 3: FALSE means "start out unsignaled"
	// param 4: NULL means "no name"
	m_hKillRequest = CreateEvent (NULL, TRUE, FALSE, NULL);
	m_hKilled = CreateEvent (NULL, TRUE, FALSE, NULL);
	m_hEventJobReady = CreateEvent (NULL, TRUE, FALSE, NULL);
	m_pCurrentJob = 0;
	m_bImBeingKilled = FALSE;
	m_bPaused = FALSE;
}

// -------------------------------------------------------------------------
// TUtilityThread -- destructor
TUtilityThread::~TUtilityThread()
{
	CloseHandle (m_hEventJobReady);
	CloseHandle (m_hKilled);
	CloseHandle (m_hKillRequest);
}

// -------------------------------------------------------------------------
// DeallocateList -- deallocates memory held by an object list
void TUtilityThread::DeallocateList (TObBucket &obBucket)
{
	obBucket.Lock ();
	POSITION iPos = obBucket.GetHeadPosition ();
	while (!obBucket.IsEmpty ()) {
		void *pJob = obBucket.RemoveHead ();
		ASSERT (pJob);
		DeallocateJob (pJob);
	}
	obBucket.Unlock ();
}

// -------------------------------------------------------------------------
// InitInstance -- called when the thread starts up... main thread loop is
// in here
BOOL TUtilityThread::InitInstance()
{
	void *pJob;                   // job being processed
	DWORD dwWaitRet;
	HANDLE rHandles[2];

	try {
		bool   fRequeueJob;

		// these 2 imply re-queueing
		CString strNoConn; strNoConn.LoadString (IDS_ERR_UTIL_NO_CONN);
		CString strLowSpace; strLowSpace.LoadString (IDS_ERR_LOW_SPACE);

		// read in any jobs left over from the last run
		SaveOrRetrieveJobs (kRETRIEVE);

		rHandles[0] = m_hKillRequest;
		rHandles[1] = m_hEventJobReady;
		while (1) {
			dwWaitRet = WaitForMultipleObjects (2, rHandles, FALSE /* fWaitAll */,
				INFINITE);
			if (dwWaitRet == 0)
				break;

			if (m_bPaused)
				// sleep so we don't use up too much CPU time
				Sleep (500);

			// service a single job, then loop back
			// 'GetJob' removes pJob from wait-queue
			if (!m_bPaused && (0 != (pJob = GetJob ())))
			{
				m_pCurrentJob = pJob;   // remember the current job

				fRequeueJob = ServiceJob (pJob);      // basically calls DoYourJob

				/***
				bLastJobOK = TRUE;
				if (((TUtilityJob *) pJob) -> Status () == strNoConn)
				bLastJobOK = FALSE;
				if (((TUtilityJob *) pJob) -> Status () == strLowSpace);
				bLastJobOK = FALSE;
				****/

				// if this last job failed because the connection terminated, pause
				// the queue, and put the failed job back in the wait queue
				if (fRequeueJob && !m_bPaused) {
					Pause ();
					AddJob (m_pCurrentJob, TRUE /* bTop */);
					((TUtilityJob *) m_pCurrentJob) -> JobKickedBackToWaitQueue ();
					m_pCurrentJob = 0;
					PostToMonitorDialog (ID_CURRENT_CHANGE);
				}
				else
					AddJobToCompletedList ();
			}

			// allow MFC to destroy Temp Objects
			CWinThread::OnIdle (1);
		}
	}
	catch (TException *Except)
	{
		Except->PushError (IDS_ERR_UTILITY_THREAD, kFatal);
		Except->Display ();
		TopLevelException (kThreadUtility);
		TException *ex = new TException(*Except);
		Except->Delete();
		throw(ex);
		return FALSE;
	}
	catch(...)
	{
		TopLevelException (kThreadUtility);
		throw;
		return FALSE;
	}

	// fail... system code will call ExitInstance()
	return FALSE;
}

// -------------------------------------------------------------------------
// ExitInstance -- called when InitInstance fails (returns FALSE)
int TUtilityThread::ExitInstance()
{
	// save any leftover jobs for the next time the program is run
	SaveOrRetrieveJobs (kSAVE);

	// deallocate memory in our queues
	DeallocateList (m_Waiting);
	DeallocateList (m_obCompleted);
	if (m_pCurrentJob)
		DeallocateJob (m_pCurrentJob);

	//TRACE0 ("utility thread: ok, i'm dead\n");
	SetEvent (m_hKilled);
	CWinThread::ExitInstance ();

	return 0;
}

// -------------------------------------------------------------------------
// AddJob -- adds a job to the utility thread's queue
void TUtilityThread::AddJob (void *pJob, BOOL bTop /* = FALSE */)
{
	POSITION iPos;

	m_Waiting.Lock ();
	iPos = bTop ?
		m_Waiting.AddHead ((CObject *) pJob) :
	m_Waiting.AddTail ((CObject *) pJob);
	m_Waiting.Unlock ();

	SetEvent (m_hEventJobReady);

	// send a message to the monitor-dialog, if it's present
	PostToMonitorDialog (bTop ? ID_WAITING_INSERT_AT_TOP : ID_WAITING_INSERT,
		((TUtilityJob *) pJob) -> m_iID, (LONG) iPos);
}

// -------------------------------------------------------------------------
// GetJob -- gets a job from the utility thread's queue.  Returns a pointer
// to the job's object or NULL if no job is available
void *TUtilityThread::GetJob ()
{
	m_Waiting.Lock ();

	// check whether the queue is already empty... may have been modified
	// since we last got a job
	if (m_Waiting.IsEmpty ()) {
		ResetEvent (m_hEventJobReady);
		m_Waiting.Unlock ();
		return 0;
	}

	POSITION iPos = m_Waiting.GetHeadPosition ();
	void *pJob = m_Waiting.RemoveHead();
	if (m_Waiting.IsEmpty ())
		ResetEvent (m_hEventJobReady);

	m_Waiting.Unlock ();

	// send a message to the monitor-dialog, if it's present
	PostToMonitorDialog (ID_WAITING_DELETE, 0,
		(LPARAM) ((TUtilityJob *) pJob) -> m_iID);

	return pJob;
}

// -------------------------------------------------------------------------
// GetBucketJob -- gets a particular job from a particular bucket.
// NOTE: The bucket must already be locked (this function does not lock it)
POSITION TUtilityThread::GetBucketJob (TObBucket *pBucket, void *&pJob,
									   int iID)
{
	POSITION iPos = pBucket -> GetTailPosition ();
	POSITION iOldPos;

	while (iPos) {
		iOldPos = iPos;
		pJob = pBucket -> GetPrev (iPos);
		if (pJob && ((TUtilityJob *) pJob) -> m_iID == iID) {
			return iOldPos;
		}
	}

	return 0;
}

// -------------------------------------------------------------------------
// RemoveWaitingJob -- removes a job from the waiting-job queue
void TUtilityThread::RemoveWaitingJob (int iID)
{
	m_Waiting.Lock ();

	void *pJob;
	POSITION rPosition = GetBucketJob (&m_Waiting, pJob, iID);
	if (!rPosition) {
		m_Waiting.Unlock ();
		return;
	}

	m_Waiting.RemoveAt (rPosition);
	m_Waiting.Unlock ();

	// inform subclasses
	RemovedFromWaitingQueue ((TUtilityJob *) pJob);

	// inform the job it's being cancelled
	CancelJob (pJob);

	// delete the job's memory as well
	DeallocateJob (pJob);

	// item has already been removed from list control if dialog is up
}

// -------------------------------------------------------------------------
TUtilityJob *TUtilityThread::GetWaitingJob (int iID)
{
	void *pJob;
	m_Waiting.Lock ();
	POSITION rPosition = GetBucketJob (&m_Waiting, pJob, iID);
	m_Waiting.Unlock ();
	if (!rPosition)
		return NULL;
	return (TUtilityJob *) pJob;
}

// -------------------------------------------------------------------------
// RenameCompletedJob -- changes the filename for a job from the completed-job
// queue
void TUtilityThread::RenameCompletedJob (int iID, CString &strNewPath)
{
	m_obCompleted.Lock ();

	void *pJob;
	POSITION rPosition = GetBucketJob (&m_obCompleted, pJob, iID);
	if (!rPosition) {
		m_obCompleted.Unlock ();
		return;
	}

	((TDecodeJob *) pJob) -> SetFilename (strNewPath);

	m_obCompleted.Unlock ();

	// send a message to the monitor-dialog, if it's present
	PostToMonitorDialog (ID_FINISHED_CHANGED, iID, (LPARAM) rPosition);
}

// -------------------------------------------------------------------------
void TUtilityThread::retryJob (int iID)
{
	void * pJob = RemoveBucketJob (iID, false);

	((TUtilityJob *) pJob) -> JobKickedBackToWaitQueue ();

	AddJob ( pJob );     // put job at end
}

// -------------------------------------------------------------------------
// RemoveCompletedJob -- removes a job from the completed-job queue
void TUtilityThread::RemoveCompletedJob (int iID)
{
	// call private function
	RemoveBucketJob (iID, true);
}

// -------------------------------------------------------------------------
// RemoveBucketJob -- Lookup job by the unique id and deallocate it.
void* TUtilityThread::RemoveBucketJob (int iID, bool fFreeJob)
{
	m_obCompleted.Lock ();

	void *pJob = 0;
	POSITION rPosition = GetBucketJob (&m_obCompleted, pJob, iID);
	if (rPosition)
	{
		m_obCompleted.RemoveAt (rPosition);
	}

	m_obCompleted.Unlock ();

	if (fFreeJob)
	{
		// delete the job's memory as well
		DeallocateJob (pJob);
		return NULL;
	}
	else
		return pJob;
}

// -------------------------------------------------------------------------
// MoveUpWaitingJob -- moves a waiting job up in the waiting-job queue
void TUtilityThread::MoveUpWaitingJob (int iID)
{
	m_Waiting.Lock ();

	void *pJob;
	POSITION rPosition = GetBucketJob (&m_Waiting, pJob, iID);
	if (!rPosition) {
		m_Waiting.Unlock ();
		return;
	}

	m_Waiting.MoveUp (rPosition);
	m_Waiting.Unlock ();

	// send a message to the monitor-dialog, if it's present
	PostToMonitorDialog (ID_WAITING_MOVE_UP, 0, (LPARAM) iID);
}

// -------------------------------------------------------------------------
// MoveWaitingJobToTop -- moves a waiting job up to the top of the waiting-job
// queue
void TUtilityThread::MoveWaitingJobToTop (int iID)
{
	m_Waiting.Lock ();

	void *pJob;
	POSITION rPosition = GetBucketJob (&m_Waiting, pJob, iID);
	if (!rPosition) {
		m_Waiting.Unlock ();
		return;
	}

	m_Waiting.RemoveAt (rPosition);
	PostToMonitorDialog (ID_WAITING_DELETE, 0, (LPARAM) iID);

	rPosition = m_Waiting.AddHead ((CObject *) pJob);
	PostToMonitorDialog (ID_WAITING_INSERT_AT_TOP,
		((TUtilityJob *) pJob) -> m_iID, (LONG) rPosition);

	m_Waiting.Unlock ();
}

// -------------------------------------------------------------------------
void TUtilityThread::Pause ()
{
	m_bPaused = !m_bPaused;

	// send 0 to show "pause", 1 to show "resume"
	PostToMonitorDialog (ID_PAUSE, m_bPaused ? 1 : 0);
}

// -------------------------------------------------------------------------
// MoveDownWaitingJob -- moves a waiting job up in the waiting-job queue
void TUtilityThread::MoveDownWaitingJob (int iID)
{
	m_Waiting.Lock ();

	void *pJob;
	POSITION rPosition = GetBucketJob (&m_Waiting, pJob, iID);
	if (!rPosition) {
		m_Waiting.Unlock ();
		return;
	}

	m_Waiting.MoveDown (rPosition);

	m_Waiting.Unlock ();

	// send a message to the monitor-dialog, if it's present
	PostToMonitorDialog (ID_WAITING_MOVE_DOWN, 0, (LPARAM) iID);
}

// -------------------------------------------------------------------------
// GetViewMemberInfo -- gives the calling dialog information about a specific
// job
void TUtilityThread::GetViewMemberInfo (WhichView iWhichView, int iID,
										void *&pJob, POSITION &iPos)
{
	// allocate a new object of the appropriate job-subtype
	pJob = NewJob ();
	if (!pJob)
		return;

	if (iWhichView == CURRENT_VIEW) {
		CopyVisualFields (m_pCurrentJob, pJob);
		return;
	}

	void *pListJob;
	TObBucket *pList =
		(iWhichView == FINISHED_VIEW ? &m_obCompleted : &m_Waiting);

	// the following didn't work. Sometimes by the time we get to this point, the
	// worker thread has already taken the job out of the waiting-job queue, so
	// iPos is an invalid value, which causes GetNext() to return an invalid
	// (but nonzero) pointer
#ifdef DIDNT_WORK
	// if we've been given a hint about the `pos' value, use it instead of
	// searching the entire list
	if (iPos) {
		pListJob = pList -> GetNext (iPos);
		if (((TUtilityJob *) pListJob) -> m_iID == iID) {
			CopyVisualFields (pListJob, pJob);
			return;
		}
	}
#endif

	// find the desired object within the bucket
	pList -> Lock ();
	POSITION iListPos;
	iListPos = pList -> GetHeadPosition ();
	POSITION iOldListPos;
	while (iListPos) {
		iOldListPos = iListPos;
		pListJob = pList -> GetNext (iListPos);
		if (((TUtilityJob *) pListJob) -> m_iID == iID) {
			CopyVisualFields (pListJob, pJob);
			iPos = iOldListPos;
			pList -> Unlock ();
			return;
		}
	}
	pList -> Unlock ();

	// not found
	delete (TUtilityJob *) pJob;
	pJob = NULL;
}

// -------------------------------------------------------------------------
// PostToMonitorDialog -- posts a message to a dialog that may be monitoring
// this thread
void TUtilityThread::PostToMonitorDialog (int iMessage, WPARAM wParam /* = 0 */,
										  LPARAM lParam /* = 0 */)
{
	// get exclusive access to dialog box
	LPCRITICAL_SECTION pCrit = GetMonitorCriticalSection ();
	EnterCriticalSection ( pCrit );

	CDialog *pMonitorDlg = MonitorDialog ();
	if (pMonitorDlg && ::IsWindow (pMonitorDlg -> m_hWnd))
		pMonitorDlg -> PostMessage (iMessage, wParam, lParam);

	// release it
	LeaveCriticalSection ( pCrit );
}

// -------------------------------------------------------------------------
// SaveOrRetrieveJobs -- save all pending jobs, or retrieve them
void TUtilityThread::SaveOrRetrieveJobs (ESaveOrRetrieve eType)
{
	TNewsServer *pServer =  0;
	CPtrList *psServerJobs = LockServerJobList (pServer);
	TUtilityJob *pJob;

	if (kSAVE == eType)
	{
		psServerJobs->RemoveAll ();

		// save the remaining waiting-jobs to the server object
		m_Waiting.Lock ();
		while (!m_Waiting.IsEmpty ())
		{
			pJob = (TUtilityJob *) m_Waiting.RemoveHead ();
			psServerJobs -> AddHead (pJob);
		}
		m_Waiting.Unlock ();
	}
	else
	{
		// get the list of saved jobs from the server object
		while (!psServerJobs->IsEmpty ())
		{
			pJob = (TUtilityJob *) psServerJobs->RemoveHead ();
			AddJob (pJob);
		}
	}
	FreeServerJobList (pServer);
}

// -------------------------------------------------------------------------
// OnIdle -- use this to implement autosave of the decode jobs
BOOL TUtilityThread::OnIdle (LONG lCount)
{
	CWinThread::OnIdle (lCount);

	if (m_fSaveJobs)
	{
		m_fSaveJobs = false;

		// Clone the jobs and save
		AutoSaveJobs ();
	}

	return FALSE;
}

// ------------------------------------------------------------------------
void TUtilityThread::AutoSaveJobs ()
{
	TNewsServer *pServer =  0;
	TUtilityJobs* psServerJobs = LockServerJobList (pServer); // virt fun
	TUtilityJob *pJob;
	TUtilityJob *pClone;
	bool fAbort = false;

	ASSERT(psServerJobs->IsEmpty());

	// make a copy of the waiting jobs
	m_Waiting.Lock ();

	POSITION pos = m_Waiting.GetHeadPosition() ;

	while (pos)
	{
		pJob = (TUtilityJob*) m_Waiting.GetNext (pos);

		if (pJob->CanClone() == false)
		{
			fAbort = true;
			break;
		}

		pClone = pJob->Clone ();
		if (pClone)
			psServerJobs -> AddHead (pClone);   // looks weird, eh? I guess it's a stack
	}

	m_Waiting.Unlock ();

	int tot = psServerJobs->GetCount();

	if (!fAbort)
	{
		// virt fn write to disk
		PersistJobs ( pServer );

		ASSERT(tot == psServerJobs->GetCount());

		// re-empty
		psServerJobs->delete_all();
	}

	FreeServerJobList (pServer);  // virt fun
}

// ------------------------------------------------------------------------
// FreeServerJobList -- undo refcount
void TUtilityThread::FreeServerJobList (TNewsServer*& rpServer)
{
	rpServer->Release ();
}

// ------------------------------------------------------------------------
// GetWaitingJobID -- returns the ID of the waiting job, or -1 for failure
int TUtilityThread::GetWaitingJobID (long lGroupID, long lArticleNum)
{
	int iResult = -1;

	m_Waiting.Lock ();
	POSITION pos = m_Waiting.GetHeadPosition ();
	while (pos) {
		TUtilityJob *pJob = (TUtilityJob *) m_Waiting.GetNext (pos);
		if (pJob -> GetGroupID () == lGroupID &&
			pJob -> GetArticleNumber () == lArticleNum) {
				iResult = pJob -> m_iID;
				break;
		}
	}
	m_Waiting.Unlock ();

	return iResult;
}

