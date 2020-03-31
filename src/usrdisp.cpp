/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: usrdisp.cpp,v $
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

///////////////////////////////////////////////////////////////////////
// BlueSky idea: should MAINFRM have a thread that paints the statusbar
//               the statusbar would never halt even when the main thread
//               is loading a fat newsgroup.

#include "stdafx.h"
#include "usrdisp.h"
#include "custmsg.h"
#include "critsect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TUserDisplay* gpUserDisplay;

static const int SENDMSG_TIMEOUT = 500;

TUserDisplay::TUserDisplay()
{
   InitializeCriticalSection (&m_critSect);
   m_iCursorCount = 0;
   m_iAutoRefreshStack = 0;
   m_sNorm.reset ();
   m_sPrio.reset ();
   m_fShowPrioString   = FALSE;
}

TUserDisplay::~TUserDisplay()
{
   DeleteCriticalSection (&m_critSect);
}

// 9-25-95 this is the CDialogBar that supports the real TStatusBarCtrl
void TUserDisplay::SetWindow(HWND hWndMainFrame, HWND hWndStatus)
{
   m_hwndMainFrame = hWndMainFrame;
   m_hWndStatus = hWndStatus;
}

/**************************************************************************
All these messages get posted to MAINFRM.CPP (and its statusbar)
**************************************************************************/

void TUserDisplay::SetText(const CString& rText, BOOL fPrio)
{
   TEnterCSection cs_mgr (&m_critSect);
   if (fPrio)
      {
      m_fShowPrioString = TRUE;
      m_sPrio.m_text = rText;
      }
   else
      m_sNorm.m_text = rText;
}

void TUserDisplay::SetText(UINT stringID, BOOL fPrio)
{
   CString msg;
   msg.LoadString (stringID);
   SetText (msg, fPrio);     // utilize other form
}

// -------------------------------------------------------------------------
void TUserDisplay::GetText( CString & rText, BOOL fPrio)
{
   TEnterCSection cs_mgr (&m_critSect);

   if (fPrio)
      rText = m_sPrio.m_text;
   else
      rText = m_sNorm.m_text;
}

// -------------------------------------------------------------------------
void TUserDisplay::SetRange(int low, int high, BOOL fPrio)
{
   TEnterCSection cs_mgr (&m_critSect);
   if (fPrio)
      {
      m_fShowPrioString = TRUE;

      m_sPrio.setRange (low, high);
      }
   else
      {
      m_sNorm.setRange (low, high);
      }
}

void TUserDisplay::StepIt(BOOL fDraw, BOOL fPriority, int incr)
{
   TEnterCSection cs_mgr (&m_critSect);
   if (fPriority)
      m_sPrio.m_iCurrent += incr;
   else
      m_sNorm.m_iCurrent += incr;

   if (fDraw)
      SendToStatuswnd ( WMU_PROGRESS_STEP, fDraw );
}

void TUserDisplay::SetPos (int pos, BOOL fDraw, BOOL fPrio)
{
   TEnterCSection cs_mgr (&m_critSect);
   if (fPrio)
      m_sPrio.setPos (pos);
   else
      m_sNorm.setPos (pos);

   if (fDraw)
      SendToStatuswnd (WMU_PROGRESS_POS, fDraw);
}

///////////////////////////////////////////////////////////////////////////
//  4-29-96  use SendMessageTimeout to avoid deadlocks
//
void TUserDisplay::Done(BOOL fPrio)
{
   {
   TEnterCSection cs_mgr (&m_critSect);
   if (fPrio)
      {
      m_fShowPrioString = FALSE;
      m_sPrio.reset ();
      }
   else
      {
      m_sNorm.reset ();
      }
   }

   // use these lines to erase any remaining text
   SendToStatuswnd (WMU_PROGRESS_END, 0);
}

// Hint taken from MSJ July 95 Q&A Win32.  SendMessageTimeout
void TUserDisplay::AddActiveCursor()
{
   TEnterCSection cs_mgr (&m_critSect);

   // we registered our own class - CMainFrame::PreCreateWindow()

   ++m_iCursorCount;
   if (1 == m_iCursorCount)
      SendMessage (m_hwndMainFrame, WMU_NONBLOCKING_CURSOR, TRUE, 0L);
}

void TUserDisplay::RemoveActiveCursor()
{
   TEnterCSection cs_mgr (&m_critSect);
   --m_iCursorCount;

   if (0 == m_iCursorCount)
      SendMessage (m_hwndMainFrame, WMU_NONBLOCKING_CURSOR, FALSE, 0L);
}

// ------------------------------------------------------------------------
void TUserDisplay::AutoRefresh(BOOL fOn)
{
   TEnterCSection cs_mgr (&m_critSect);

   if (fOn)
      ++m_iAutoRefreshStack;
   else if (m_iAutoRefreshStack > 0)
      --m_iAutoRefreshStack;
}

BOOL TUserDisplay::IsAutoRefresh()
{
   return m_iAutoRefreshStack > 0;
}

// ------------------------------------------------------------------------
// Get all the relevant draw information in one fell swoop
BOOL TUserDisplay::GetAllCurrentInfo (LPUSRDISP_INFO pDI)
{
   ASSERT(pDI);

   TEnterCSection cs_mgr (&m_critSect);

   pDI->m_fPrio = m_fShowPrioString;

   pDI->m_ptFilter = m_ptFilter;

   if (m_fShowPrioString)
      {
      pDI->m_iLow = m_sPrio.m_iLow;
      pDI->m_iHigh = m_sPrio.m_iHigh;
      pDI->m_iCurrent = m_sPrio.m_iCurrent;
      pDI->m_text = m_sPrio.m_text;
      }
   else
      {
      pDI->m_iLow = m_sNorm.m_iLow;
      pDI->m_iHigh = m_sNorm.m_iHigh;
      pDI->m_iCurrent = m_sNorm.m_iCurrent;
      pDI->m_text = m_sNorm.m_text;
      }

   pDI->m_uiStatus = m_strUIStatus;

   return TRUE;
}

// ------------------------------------------------------------------------
// private
void TUserDisplay::SendToStatuswnd (UINT message, WPARAM fDraw)
{
   DWORD dwResult;

   if (!::IsWindow (m_hWndStatus))
      return;

   if (!SendMessageTimeout(m_hWndStatus, message, fDraw, 0L,
                           SMTO_NORMAL, SENDMSG_TIMEOUT /*msecs*/, &dwResult))

      {
      PostMessage (m_hWndStatus, message, fDraw, 0L);
      }
}

// ------------------------------------------------------------------------
void TUserDisplay::SetUIStatus (const CString & rText)
{
   TEnterCSection cs_mgr (&m_critSect);

   m_strUIStatus = rText;
}

// ------------------------------------------------------------------------
void TUserDisplay::ClearUIStatus (void)
{
   TEnterCSection cs_mgr (&m_critSect);

   m_strUIStatus.Empty();
}

// ------------------------------------------------------------------------
void TUserDisplay::SetCountFilter (int iCountFilter)
{
   TEnterCSection cs_mgr (&m_critSect);

   m_ptFilter.x = iCountFilter;
}

// ------------------------------------------------------------------------
void TUserDisplay::SetCountTotal  (int iCountTotal)
{
   TEnterCSection cs_mgr (&m_critSect);

   m_ptFilter.y  = iCountTotal;
}

/////////////////////////////////////////////////////////////////
//
TUserDisplay_Auto::TUserDisplay_Auto(
   TUserDisplay_Auto::EAction   eAction,
   TUserDisplay_Auto::EPriority ePriority
)
   : m_eAction(eAction), m_ePriority(ePriority)
{
   gpUserDisplay->AutoRefresh(TRUE);
}

TUserDisplay_Auto::~TUserDisplay_Auto()
{
   gpUserDisplay->AutoRefresh(FALSE);

   ASSERT(kClearDisplay == m_eAction);   // only choice available

   if (kPriority == m_ePriority)
      gpUserDisplay->Done (TRUE);
   else
      gpUserDisplay->Done (FALSE);
}

// ------------------------------------------------------------
TUserDisplay_UIPane::TUserDisplay_UIPane(const CString & status)
{
   gpUserDisplay->SetUIStatus ( status );
}
TUserDisplay_UIPane::~TUserDisplay_UIPane()
{
   gpUserDisplay->ClearUIStatus ();
}

// ------------------------------------------------------------
TUserDisplay_Reset::TUserDisplay_Reset(const CString & status, BOOL fPrio)
      : m_fPrio(fPrio),  m_str(status)
      {

      }

TUserDisplay_Reset::~TUserDisplay_Reset()
      {
      // essentially, pop the stack
      gpUserDisplay->SetText (m_str, m_fPrio);
      }

