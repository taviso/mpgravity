/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tprnthrd.h,v $
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

// tprnthrd.h -- printing thread

#pragma once

#include "tutlthrd.h"         // TUtilityThread

class TArticleHeader;
class TArticleText;
class TPrintJob;
class TUtilityJobs;

// -------------------------------------------------------------------------
class TPrintThread : public TUtilityThread
{
public:
	// constructor and destructor
	TPrintThread ();
	~TPrintThread ();

	// separate AddJob() here for type checking
	void AddJob (TPrintJob *psPrintJob) { TUtilityThread::AddJob (psPrintJob); }
	void *NewJob ();
	void CopyVisualFields (void *pSourceJob, void *pDestJob);
	void AddJobToCompletedList ();

private:
	bool ServiceJob (void *pJob);
	void DeallocateJob (void *pJob);
	CDialog *MonitorDialog (); // dialog that may be monitoring us
	LPCRITICAL_SECTION GetMonitorCriticalSection ();
	void CancelJob (void *pJob) { /* not needed for print jobs */ }
	TUtilityJobs* LockServerJobList (TNewsServer*& rpServer);
	void CleanUpDC (HDC hDC);
	void RemovedFromWaitingQueue (TUtilityJob *pJob);
};

extern TPrintThread *gpPrintThread; // global pointer to the print thread
extern CDialog *gpsPrintDialog;  // print-monitor dialog
extern CRITICAL_SECTION gcritPrintDialog; // protect print-monitor dialog

void PrintTaskerCallback (int iStatus, TArticleHeader *pHdr,
						  TArticleText *pText);
