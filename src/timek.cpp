/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: timek.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:07  richard_wood
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

#include "stdafx.h"
#include "timek.h"
#include "critsect.h"
#include "server.h"           // TNewsServer
#include "servcp.h"           // TServerCountedPtr

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -----------------------------------------------------------------------
TTimeKeeper::TTimeKeeper()
{
   InitializeCriticalSection(&m_critSect);

   m_fEnabled = FALSE;
}

// -----------------------------------------------------------------------
TTimeKeeper::~TTimeKeeper()
{
   DeleteCriticalSection(&m_critSect);
}

// -----------------------------------------------------------------------
// enable countdown
void TTimeKeeper::Enable(BOOL fStartTiming)
{
   TEnterCSection sLock(&m_critSect);
   m_fEnabled = fStartTiming;

   m_lastActivity = CTime::GetCurrentTime();
}

// -----------------------------------------------------------------------
BOOL TTimeKeeper::IsEnabled(void)
{
   TEnterCSection sLock(&m_critSect);
   return m_fEnabled;
}

// -----------------------------------------------------------------------
// return TRUE if more than X minutes has elapsed
BOOL TTimeKeeper::Elapsed(int nMin)
{
   ASSERT(nMin >= 1);
   TEnterCSection sLock(&m_critSect);

   if (!m_fEnabled)
      return FALSE;

   CTimeSpan span = CTime::GetCurrentTime() - m_lastActivity;

   return (span.GetMinutes() >= nMin);
}

// -----------------------------------------------------------------------
// set the activity time
void TTimeKeeper::SetActivity(void)
{
   TEnterCSection sLock(&m_critSect);

   if (!m_fEnabled)
      return;

   m_lastActivity = CTime::GetCurrentTime();
}

// ------------------------------------------------------------------------
//
BOOL TPumpTimeKeeper::IsSignaled ()
{
   TServerCountedPtr cpNewsServer;

   if (!cpNewsServer->GetAutoDisconnect())
      return FALSE;

   // from base class
   return Elapsed ( cpNewsServer->GetAutoDisconnectMinutes() );
}
