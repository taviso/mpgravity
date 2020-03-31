/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecthrd.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:00  richard_wood
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

// tdecthrd.cpp -- decoding thread

#include "stdafx.h"              // precompiled header
#include "tdecthrd.h"            // this file's prototypes
#include "tdecjob.h"             // TDecodeJob
#include "tsubjct.h"             // TSubject
#include "tsubjctx.h"            // SUBJECT_CONNECTED
#include "tglobopt.h"            // gpGlobalOptions
#include "rgswit.h"              // GetRegSwitch()
#include "server.h"              // TNewsServer
#include "tutlmsg.h"             // ID_FINISHED_INSERT, ...
#include "genutil.h"             // GetStartupDir()
#include "gallery.h"             // GetGalleryWindow()
#include "ipcgal.h"              // IPC_Gallery_QueueChange

TDecodeThread *gpDecodeThread = NULL; // global decoding thread object
CDialog *gpsDecodeDialog = NULL; // decode-monitor dialog
CRITICAL_SECTION gcritDecodeDialog; // protect decode-monitor dialog

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TDecodeThread::TDecodeThread ()
{
	InitializeCriticalSection (&gcritDecodeDialog);
}

// -------------------------------------------------------------------------
TDecodeThread::~TDecodeThread ()
{
	DeleteCriticalSection (&gcritDecodeDialog);
}

// -------------------------------------------------------------------------
void *TDecodeThread::NewJob ()
{
	return new TDecodeJob;
}

// -------------------------------------------------------------------------
// KillAndWaitTillDead -- if option to delete decoded files on exit is on,
// do it here
void TDecodeThread::KillAndWaitTillDead ()
{
	if (gpGlobalOptions -> GetRegSwitch () -> GetDeleteOnExit ()) {
		m_obCompleted.Lock ();
		POSITION iPos = m_obCompleted.GetHeadPosition ();
		while (iPos) {
			TDecodeJob *pJob = (TDecodeJob *) m_obCompleted.GetNext (iPos);
			DeleteFile (pJob -> FirstFilename ());
		}
		m_obCompleted.Unlock ();
	}

	TUtilityThread::KillAndWaitTillDead ();
}

// -------------------------------------------------------------------------
// DeallocateJob -- deallocates a job's object... called by the utility-thread
// class
void TDecodeThread::DeallocateJob (void *pJob)
{
	delete (TDecodeJob *) pJob;
}

// -------------------------------------------------------------------------
// ServiceJob -- service a decode job... called by TUtilityThread's fetch-
// service loop
//
// return true to re-queue the job
bool TDecodeThread::ServiceJob (void *pJob)
{
	return ((TDecodeJob *) pJob) -> DoYourJob (m_hKillRequest);
}

// -------------------------------------------------------------------------
// CopyVisualFields -- copies visual fields of a job of our job-type to a job
// owned by the user
void TDecodeThread::CopyVisualFields (void *pSourceJob, void *pDestJob)
{
	TDecodeJob *pSource = (TDecodeJob *) pSourceJob;
	TDecodeJob *pDest = (TDecodeJob *) pDestJob;

	pDest -> CopyVisualFields (pSource);
}

// -------------------------------------------------------------------------
// GetMonitorCriticalSection -- fetch critical section that protects
// MonitorDialog
LPCRITICAL_SECTION TDecodeThread::GetMonitorCriticalSection ()
{
	return &gcritDecodeDialog;
}

// -------------------------------------------------------------------------
// MonitorDialog -- return the job control dialog
CDialog * TDecodeThread::MonitorDialog ()
{
	return gpsDecodeDialog;
}

// -------------------------------------------------------------------------
// CancelJob -- tells a decode-job that it's being cancelled
void TDecodeThread::CancelJob (void *pJob)
{
	((TDecodeJob *) pJob) -> JobBeingCancelled ();
}

// -------------------------------------------------------------------------
TUtilityJobs*  TDecodeThread::LockServerJobList (TNewsServer*& rpServer)
{
	rpServer = GetCountedActiveServer ();
	rpServer->AddRef ();
	return rpServer -> GetDecodeJobs ();
}

// -------------------------------------------------------------------------
// ArticleFullPath -- takes an article number and searches for a completed-job
// containing the article.  If not found, returns non-zero.  If found, returns
// zero and fills in the decoded binary's full-path for the completed-job
int TDecodeThread::ArticleFullPath (LONG lGroupID, LONG lArtNum,
									CString &strPath)
{
	int iResult = 1;

	m_obCompleted.Lock ();
	POSITION iPos = m_obCompleted.GetHeadPosition ();
	TDecodeJob *pJob;
	while (iPos)
	{
		pJob = (TDecodeJob *) m_obCompleted.GetNext (iPos);

		if (!pJob -> ArticleFullPath (lGroupID, lArtNum, strPath))
		{
			iResult = 0;
			break;
		}
	}

	m_obCompleted.Unlock ();
	return iResult;
}

// -------------------------------------------------------------------------
// AddJob -- add new decode job... weed out new jobs whose prefix already
//    occurs in the waiting-job-list
//
// This work is still done by the main thread, not the decode thread
//
void TDecodeThread::AddJob (
							TDecodeJob *psDecodeJob,
							VEC_HDRS *  pVecHdrs,
							BOOL bEliminateDuplicate /* = TRUE */,
							BOOL bPriority /* = FALSE */)
{
	// compute the new job's prefix
	psDecodeJob -> ComputePrefix ();

	// zero out m_iNumParts... it shouldn't be initialized until
	// TDecodeJob::DoYourJob()
	psDecodeJob -> ZeroNumParts ();

	if (bEliminateDuplicate && psDecodeJob -> WasPartNumInSubject ())
	{
		// If the prefix for this job is the prefix for any job in the waiting
		// queue (or the current-job), don't add this job, just delete it.
		//
		// SPECIAL CASE: if the part number wasn't in the job's subject line,
		//               don't perform this duplicate elimination
		//               (remember to kill Tony)

		if (m_pCurrentJob &&
			psDecodeJob -> m_strPrefix ==
			((TDecodeJob *) m_pCurrentJob) -> m_strPrefix)
		{
			delete psDecodeJob;
			return;
		}

		m_Waiting.Lock ();
		POSITION iPos = m_Waiting.GetHeadPosition ();
		while (iPos)
		{
			TDecodeJob *pJob = (TDecodeJob *) m_Waiting.GetNext (iPos);
			if (psDecodeJob -> m_strPrefix == pJob -> m_strPrefix)
			{
				delete psDecodeJob;
				m_Waiting.Unlock ();
				return;
			}
		}
		m_Waiting.Unlock ();
	}

	// locate siblings while the newsgroup is present
	psDecodeJob -> PreprocessJob (pVecHdrs);

	TUtilityThread::AddJob (psDecodeJob, bPriority);
}

// ------------------------------------------------------------------------
// UpdateObserver -- flesh out TObserver interface.
void TDecodeThread::UpdateObserver ()
{
	ASSERT(m_pSubj);

	if (SUBJECT_CONNECTED == m_pSubj->GetSubjectState ())
	{
		// this data member is inherited from TUtilityThread

		if (gpGlobalOptions -> GetRegSwitch () -> GetResumePausedDecJob () &&
			IsPaused ())
		{
			// The app has successfully reconnected.  Toggle the pause, and
			//  continue to decode

			Pause ();
		}
	}
}

// -------------------------------------------------------------------------
void TDecodeThread::InsertCompletedJob (TDecodeJob *pJob)
{
	m_obCompleted.Lock ();
	POSITION iPos = m_obCompleted.AddTail (pJob);
	m_obCompleted.Unlock ();
	PostToMonitorDialog (ID_FINISHED_INSERT, pJob -> m_iID, (LONG) iPos);
}

// -------------------------------------------------------------------------
// AddJobToCompletedList -- add the job to the completed-job list.  If there
// is already a job in the completed-job list with a matching prefix, modify
// its status... otherwise, add this job to the completed-job list
void TDecodeThread::AddJobToCompletedList ()
{
	TDecodeJob *pJob = (TDecodeJob *) m_pCurrentJob;
	m_pCurrentJob = 0;

	PostToMonitorDialog (ID_CURRENT_CHANGE);

	// do this only if there is a prefix. Also, forget about it if there were
	// multiple files decoded (each will get its own completed-job item)
	if (pJob -> m_strPrefix.GetLength () &&
		pJob -> Filenames ().GetCount () == 1) {

			// Try to find an existing completed-job with an identical prefix
			m_obCompleted.Lock ();
			POSITION iPos = m_obCompleted.GetHeadPosition ();
			POSITION iOldPos;
			TUtilityJob *pListJob;
			while (iPos) {
				iOldPos = iPos;
				pListJob = (TUtilityJob *) m_obCompleted.GetNext (iPos);
				if (pListJob && pListJob -> MergeJobsInResultPane (pJob)) {

					// if the existing list-job has completed successfully, don't
					// replace it. Just discard the new job
					CString strOK; strOK.LoadString (IDS_UTIL_OK);
					if (pListJob -> Status() == strOK) {
						delete pJob;
						m_obCompleted.Unlock ();
						return;
					}

					m_obCompleted.SetAt (iOldPos, pJob);
					pJob -> m_iID = pListJob -> m_iID;
					delete pListJob;
					PostToMonitorDialog (ID_FINISHED_CHANGED_NO_HILIGHT, pJob -> m_iID,
						(LONG) iOldPos);
					m_obCompleted.Unlock ();
					return;
				}
			}
			m_obCompleted.Unlock ();
	}

	// if there were multiple binaries, each gets it own completed-job
	const CStringList &rstrFilenames = pJob -> Filenames ();
	if (rstrFilenames.GetCount () < 2)
		InsertCompletedJob (pJob);
	else {
		POSITION pos = rstrFilenames.GetHeadPosition ();
		while (pos) {
			TDecodeJob *pNew = new TDecodeJob (pJob);
			const CString &strFilename = rstrFilenames.GetNext (pos);
			pNew -> SetFilename (strFilename);
			InsertCompletedJob (pNew);
		}
		delete pJob;
	}

	// notify gallery... check for gallery every 10 times to reduce overhead
	HWND hWnd = GetGalleryWindow ();
	if (hWnd)
	{
		int iWaiting   = m_Waiting.GetCount ();
		int iCompleted = m_obCompleted.GetCount ();

		IPC_Gallery_QueueChange (hWnd, iWaiting, iCompleted);
	}
}

// -------------------------------------------------------------------------
void TDecodeThread::SetJobLaunch (int iID)
{
	m_Waiting.Lock ();
	POSITION iPos = m_Waiting.GetHeadPosition ();
	while (iPos) {
		TDecodeJob *pJob = (TDecodeJob *) m_Waiting.GetNext (iPos);
		if (pJob -> m_iID == iID) {
			pJob -> SetLaunchViewer ();
			break;
		}
	}
	m_Waiting.Unlock ();
}

// -------------------------------------------------------------------------
// MarkArticlesRead -- mark job's articles as read
void TDecodeThread::MarkArticlesRead (int iID)
{
	m_obCompleted.Lock ();
	POSITION pos = m_obCompleted.GetHeadPosition ();
	while (pos) {
		TDecodeJob *pJob = (TDecodeJob *) m_obCompleted.GetNext (pos);
		if (iID == pJob -> m_iID) {
			pJob -> MarkArticlesRead ();
			break;
		}
	}
	m_obCompleted.Unlock ();
}

// -------------------------------------------------------------------------
// PersistJobs -- override a virt fun
void TDecodeThread::PersistJobs (TNewsServer * pServer)
{
	pServer -> SaveDecodeJobs ();
}

