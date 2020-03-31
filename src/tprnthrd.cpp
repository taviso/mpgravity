/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tprnthrd.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:15  richard_wood
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

// tprnthrd.cpp -- printing thread

#include "stdafx.h"              // precompiled header
#include "tprnthrd.h"            // this file's prototypes
#include "tprnjob.h"             // TPrintJob
#include "server.h"              // TNewsServer
#include "critsect.h"            // TEnterCSection
#include "tutlmsg.h"             // ID_FINISHED_INSERT, ...
#include "tglobopt.h"            // gpGlobalOptions
#include "rgswit.h"              // TRegSwitch

TPrintThread *gpPrintThread = NULL; // global printing thread object
CDialog *gpsPrintDialog = NULL;  // print-monitor dialog
CRITICAL_SECTION gcritPrintDialog; // protect print-monitor dialog

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TPrintThread::TPrintThread ()
{
   InitializeCriticalSection ( &gcritPrintDialog );
}

// -------------------------------------------------------------------------
TPrintThread::~TPrintThread ()
{
   DeleteCriticalSection ( &gcritPrintDialog );
}

// -------------------------------------------------------------------------
void *TPrintThread::NewJob ()
{
   return new TPrintJob;
}

// -------------------------------------------------------------------------
// DeallocateJob -- deallocates a job's object... called by the utility-thread
// class
void TPrintThread::DeallocateJob (void *pJob)
{
   delete (TPrintJob *) pJob;
}

// -------------------------------------------------------------------------
// ServiceJob -- service a print job... called by TUtilityThread's fetch-
// service loop
//
// return true to re-queue the job
bool TPrintThread::ServiceJob (void *pJob)
{
   return ((TPrintJob *) pJob) -> DoYourJob (m_hKillRequest);
}

// -------------------------------------------------------------------------
// CopyVisualFields -- copies visual fields of a job of our job-type to a job
// owned by the user
void TPrintThread::CopyVisualFields (void *pSourceJob, void *pDestJob)
{
   TPrintJob *pSource = (TPrintJob *) pSourceJob;
   TPrintJob *pDest = (TPrintJob *) pDestJob;

   pDest -> CopyVisualFields (pSource);
}

// -------------------------------------------------------------------------
// GetMonitorCriticalSection -- used in conjunction with MonitorDialog
LPCRITICAL_SECTION TPrintThread::GetMonitorCriticalSection ()
{
   return &gcritPrintDialog;
}

// -------------------------------------------------------------------------
// MonitorDialog -- return the job control dialog
CDialog * TPrintThread::MonitorDialog ()
{
   return gpsPrintDialog;
}

// -------------------------------------------------------------------------
TUtilityJobs*  TPrintThread::LockServerJobList (TNewsServer*& rpServer)
{
   rpServer = GetCountedActiveServer();
   rpServer->AddRef ();
   return (rpServer -> GetPrintJobs ());
}

// -------------------------------------------------------------------------
void TPrintThread::CleanUpDC (HDC hDC)
{
   // if there are no more waiting-jobs with this hDC, delete it
   BOOL bExist = FALSE;
   POSITION pos = m_Waiting.GetHeadPosition ();
   while (pos) {
      TPrintJob *pJob = (TPrintJob *) m_Waiting.GetNext (pos);
      if (pJob -> GetDC () == hDC) {
         bExist = TRUE;
         break;
         }
      }

   // check current-job too
   TPrintJob *pCurrent = (TPrintJob *) m_pCurrentJob;
   if (pCurrent)
      if (pCurrent -> GetDC () == hDC)
         bExist = TRUE;

   if (!bExist)
      DeleteDC (hDC);
}

// -------------------------------------------------------------------------
// AddJobToCompletedList -- add the job to the completed-job list
void TPrintThread::AddJobToCompletedList ()
{
   m_obCompleted.Lock ();
   POSITION iPos = m_obCompleted.AddTail ((CObject *) m_pCurrentJob);
   m_obCompleted.Unlock ();

   // send a message to the monitor-dialog, if it's present
   PostToMonitorDialog (ID_FINISHED_INSERT,
      ((TUtilityJob *) m_pCurrentJob) -> m_iID, (LONG) iPos);
   PostToMonitorDialog (ID_CURRENT_CHANGE);

   HDC hDC = ((TPrintJob *) m_pCurrentJob) -> GetDC ();
   m_pCurrentJob = 0;

   CleanUpDC (hDC);
}

// -------------------------------------------------------------------------
void TPrintThread::RemovedFromWaitingQueue (TUtilityJob *pJob)
{
   CleanUpDC (((TPrintJob *) pJob) -> GetDC ());
}
