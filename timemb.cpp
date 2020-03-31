/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: timemb.cpp,v $
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
#include "timemb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Thread-local storage variable initialized by TimeMsgBox_MessageBox
//   and possibly changed by TimeMsgBox_TimerProc
static BOOL __declspec(thread) gt_fTimeMsgBox_Timedout = FALSE;

// ------------------------------------------------------------------------
static void WINAPI
TimeMsgBox_TimerProc (HWND /*hwnd*/, UINT /*uMsg*/, UINT /*idEvent*/, 
   DWORD /*dwTime*/)
{
   // if we get here, the user has not responded to the message box in
   //   the specified time.

   // Change the thread-local storage variable to indicate a timeout
   gt_fTimeMsgBox_Timedout = TRUE;

   // Force the message box to terminate.  The message box must be the
   // active window for this thread
   EndDialog (GetActiveWindow(), 0);
}

// ------------------------------------------------------------------------
//  Creates a message box that destroys itself if the user doesn't respond
//   to it in the specified time.
int WINAPI TimeMsgBox_MessageBox(HWND hwndOwner, LPCTSTR pszText,
   LPCTSTR pszCaption, UINT fuStyle, int nTimeout)
{

   int nResult, nTimerID;

   // Assume the user is going to respond in time
   gt_fTimeMsgBox_Timedout = FALSE;

   // use a callback function, so that MessageBoxEx's modal loop calls
   //   our TimerProc function.
   nTimerID = SetTimer(NULL, 0, nTimeout, TimeMsgBox_TimerProc);

   // display the message box to the User
   nResult = MessageBox(hwndOwner, pszText, pszCaption, fuStyle);

   // Kill the timer so we are not accidentally called when
   //  the message box is not visible
   KillTimer(NULL, nTimerID);

   // If the thread-local variable is TRUE, then we timed out.
   return (gt_fTimeMsgBox_Timedout ? ID_CUST_TIMEOUT : nResult);
}

