/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: utilpump.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:24  richard_wood
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
#include "custmsg.h"
#include "utilpump.h"

#include "tutlmsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void PumpAUsefulMessageLITE (void)
{
   static int nCount = 0;

   if ((++nCount % 256) == 0)
      PumpAUsefulMessage ();
}

///////////////////////////////////////////////////////////////////////////
//
//  4-23-96 amc  Pass timer messages through also
void PumpAUsefulMessage(void)
{
   extern HWND ghwndStatusBar;

   MSG msg;
   // Paint messages are harmless
   if (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
      {
      ::DispatchMessage(&msg);
      return;
      }

   // Timer messages are harmless - (this is mainly for our statusbar)
   if (::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE))
      {
      ::DispatchMessage(&msg);
      return;
      }

   if (IsWindow(ghwndStatusBar))
      {
      if (::PeekMessage(&msg, ghwndStatusBar, NULL, NULL,
                                     PM_REMOVE))
         {
         ::DispatchMessage(&msg);
         return;
         }
      }

   if (::PeekMessage(&msg, NULL, FIRST_MODELESS_MESSAGE,
                                  LAST_MODELESS_MESSAGE, PM_REMOVE))
      {
      CWinApp* pApp = AfxGetApp();
      // this should route to all modeless dialogs too
      if (FALSE == pApp->PreTranslateMessage(&msg))
         ::DispatchMessage(&msg);
      return;
      }
}

// --------------------------------------------------------------
// used for the freaky splash screen, which needs WM_TIMER msgs
//
void PumpDialogMessages ()
{
   MSG msg;

   // Paint messages are harmless
   if (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
      {
      ::DispatchMessage(&msg);
      return;
      }

   // Timer messages are harmless - (this is mainly for our statusbar)
   if (::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE))
      {
      ::DispatchMessage(&msg);
      return;
      }

   if (::PeekMessage(&msg, NULL, FIRST_MODELESS_MESSAGE,
                                  LAST_MODELESS_MESSAGE, PM_REMOVE))
      {
      CWinApp* pApp = AfxGetApp();
      // this should route to all modeless dialogs too
      if (FALSE == pApp->PreTranslateMessage(&msg))
         ::DispatchMessage(&msg);
      return;
      }
}

