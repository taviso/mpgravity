/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecthrd.h,v $
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

// tdecthrd.h -- decoding thread

#pragma once

#include "tutlthrd.h"         // TUtilityThread
#include "tobserv.h"          // TObserver watches TSubject
#include "stldeclare.h"       // VEC_HDRS

class TArticleHeader;
class TArtNode;
class TArticleText;
class TDecodeJob;
class TUtilityJobs;

// -------------------------------------------------------------------------
class TDecodeThread : public TUtilityThread, public TObserver
{
public:
   TDecodeThread ();
   ~TDecodeThread ();

   void AddJob (TDecodeJob *psDecodeJob,
                VEC_HDRS * pVecHdrs,
                BOOL bEliminateDuplicate = TRUE,
                BOOL bPriority = FALSE);
   void *NewJob ();
   void CopyVisualFields (void *pSourceJob, void *pDestJob);
   void CancelJob (void *pJob);
   int ArticleFullPath (LONG lGroupID, LONG lArtNum, CString &strPath);
   void KillAndWaitTillDead ();
   void AddJobToCompletedList ();
   virtual void UpdateObserver ();
   void SetJobLaunch (int iID);
   void MarkArticlesRead (int iID);

protected:
   virtual void PersistJobs (TNewsServer * pServer);

private:

   // return true to re-queue the job
   bool ServiceJob (void *pJob);
   void DeallocateJob (void *pJob);
   void InsertCompletedJob (TDecodeJob *pJob);
   CDialog*  MonitorDialog (); // dialog that may be monitoring us
   LPCRITICAL_SECTION GetMonitorCriticalSection ();
   TUtilityJobs*  LockServerJobList (TNewsServer*& rpServer);
};

extern TDecodeThread *gpDecodeThread;  // global pointer to the decode thread
extern CDialog *gpsDecodeDialog;       // decode-monitor dialog
extern CRITICAL_SECTION gcritDecodeDialog; // protect decode-monitor dialog

void DecodeTaskerCallback (int iStatus, TArticleHeader *pHdr,
   TArticleText *pText);
