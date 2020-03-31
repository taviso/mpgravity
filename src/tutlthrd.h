/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutlthrd.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

// tutlthrd.h  -- TUtilityThread abstract class.  Derive from this class for
// worker threads that process job queues

// to derive from this class:
//  1. override AddJob() to perform type checking, and call
//     TUtilityThread::AddJob().  Make AddJob() public
//  2. define ServiceJob().  Make it private
//  3. define DeallocateJob().  Make it private
//  4. during ServiceJob(), call JobClass::SetStatus() as needed

#pragma once

#include "bucket.h"           // TObBucket

class TNewsServer;
class TUtilityJob;
class TUtilityJobs;

// -------------------------------------------------------------------------
class TUtilityThread : public CWinThread
{
public:
   virtual void KillAndWaitTillDead () {
      m_bImBeingKilled = TRUE;
      SetEvent (m_hKillRequest);
      WaitForSingleObject (m_hKilled, INFINITE);
      }
   POSITION GetHeadPosition () { return m_Waiting.GetHeadPosition (); }
   void *GetNext (POSITION &rPosition) { return m_Waiting.GetNext (rPosition); }
   void *GetCurrentJob () { return m_pCurrentJob; }
   POSITION GetCompletedHeadPosition () { return m_obCompleted.GetHeadPosition (); }
   void *GetCompletedNext (POSITION &rPosition) { return m_obCompleted.GetNext (rPosition); }
   void RemoveWaitingJob (int iID);
   void retryJob (int iID);
   void RemoveCompletedJob (int iID);
   void RenameCompletedJob (int iID, CString &strNewPath);
   void MoveUpWaitingJob (int iID);
   void MoveDownWaitingJob (int iID);
   void MoveWaitingJobToTop (int iID);
   void Pause ();
   BOOL IsPaused () { return m_bPaused; }
   TUtilityJob *GetWaitingJob (int iID);
   int GetWaitingJobID (long lGroupID, long lArticleNum);

   // function to enquire about specific members in the queues
   enum WhichView { FINISHED_VIEW, CURRENT_VIEW, QUEUED_VIEW };
   void GetViewMemberInfo (WhichView iWhichView, int iID, void *&pJob,
      POSITION &iPos);

   BOOL m_bImBeingKilled;        // TRUE if thread is being killed

   void FlagSaveJobs() { m_fSaveJobs=true; }

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TUtilityThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

protected:
	TUtilityThread ();         // protected constructor used by dynamic creation
	~TUtilityThread ();

   // return true if we should re-queue the job
   virtual bool ServiceJob (void *pJob) = 0; // must be defined by child class

   virtual void DeallocateJob (void *pJob) = 0; // must be defined by child class
   virtual void AddJob (void *pJob, BOOL bTop = FALSE);
   HANDLE m_hKillRequest;     // subclasses need to get at this...
   virtual void *NewJob () = 0;  // allocates a new object of the appropriate job-subtype
   virtual void CopyVisualFields (void *pSourceJob, void *pDestJob) = 0;
   virtual CDialog *MonitorDialog () = 0; // dialog that may be monitoring us
   virtual LPCRITICAL_SECTION GetMonitorCriticalSection () = 0;
   POSITION GetBucketJob (TObBucket *pBucket, void *&pJob, int iID);
   virtual void PostToMonitorDialog (int iMessage, WPARAM wParam = 0,
      LPARAM lParam = 0) ;
   virtual void CancelJob (void *pJob) = 0;
   virtual TUtilityJobs*  LockServerJobList (TNewsServer*& rpServer) = 0;
   virtual void FreeServerJobList (TNewsServer*& rpServer);
   virtual void AddJobToCompletedList () = 0;
   virtual void RemovedFromWaitingQueue (TUtilityJob *pJob) {}

   virtual BOOL OnIdle (LONG lCount);
   void   AutoSaveJobs();
   virtual void PersistJobs (TNewsServer * pServer) { }

protected:
   TObBucket m_Waiting;      // job queue
   TObBucket m_obCompleted;   // completed-job queue
   void *m_pCurrentJob;       // currently-running job
   BOOL m_bPaused;            // are we paused?

   bool m_fSaveJobs;

private:
   void *GetJob ();
   void DeallocateList (TObBucket &obBucket);
   enum ESaveOrRetrieve { kSAVE, kRETRIEVE };
   void SaveOrRetrieveJobs (ESaveOrRetrieve iType);
   void*  RemoveBucketJob (int iID, bool fFreeJob);

   HANDLE m_hKilled;          // tells killer that we're dead
   HANDLE m_hEventJobReady;   // for queue signaling

protected:
	//{{AFX_MSG(TUtilityThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
