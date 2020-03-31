/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: dialman.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:21  richard_wood
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

#pragma once

#include "tras.h"                      // ras object

typedef void (*P_RASSTATUSFUNC)(int iStatusStringID);

/////////////////////////////////////////////////////////////////////
// TDialupManager - Manages connecting and disconnecting to the
//                  current News Server if the user has configured
//                  it thusly.
/////////////////////////////////////////////////////////////////////

class TDialupManager

{
public:
   enum EFindConn { kFindOurs, kFindAny };

   enum ECheckConnect { kNotConnected,
                        kConnected,
                        kNotInstalled,
                        kConnecting };

public:
   TDialupManager();
   ~TDialupManager ();

   // returns TRUE if Ras is installed
   BOOL Initialize ();
   BOOL IsRasInstalled () { return m_fRasInstalled; }

   BOOL Connect (P_RASSTATUSFUNC pStatusFunc, HWND hWnd, UINT msg);
   BOOL Disconnect (BOOL fAutoDisconnect);
   ECheckConnect GetDialupState ();

   BOOL CancelConnectRequest (DWORD timeout = 5000);

   BOOL TryingToConnect ();

   // Used by the dialup Page
   DWORD RasCreatePhonebookEntry(HWND hwnd, LPTSTR pPhoneBook);
   DWORD RasEditPhonebookEntry(HWND hwnd, LPTSTR pPhoneBook, LPTSTR entry);
   DWORD RasEnumEntries(LPTSTR r1, LPTSTR pPhoneBook, LPRASENTRYNAME pEntry,
                        LPDWORD lpcb, LPDWORD lpcEntries);
   DWORD RasGetEntryDialParams(LPTSTR pPhoneBook,
                               LPRASDIALPARAMS pDialParams,
                               LPBOOL pfPassword);
   DWORD RasSetEntryDialParams(LPTSTR pBook, LPRASDIALPARAMS pDP, BOOL fRemovePwd);

   // not really public
   void ShowRasError (bool fDuringInit, DWORD errorNum);
   void processEventsResult (bool fSuccess);
   void hangup_nowait ();

   bool incrementRedialCount (bool fResetToZero);  // returns true to redial

   bool ShowRedialDlg (CWnd * pAnchor);

protected:
   BOOL MakeConnection (LPCTSTR pConnectionName);
   BOOL FindConnectionByName(EFindConn eFind, const CString& dialupName,
                             HRASCONN & hConnection);

   void wait_after_hangup (HRASCONN hConnection);
   BOOL completion_status (HRASCONN hConnection, DWORD dwError);
   BOOL get_dial_parms (LPCTSTR pConnName, RASDIALPARAMS* prasParms);
   void hangup_and_wait (HRASCONN hConnection);
   void set_vars_cancelled ();
   void set_vars_init ();

private:
   HRASCONN            m_hConnection;

   BOOL                m_fWeCreated;

   // kConnected
   //  successful call to Connect turns this on.
   //  call to Disconnect turns this off
   //    may not correspond to actual RAS connection
   //    returned by FindConnectionByName

   // kConnecting means before calling RasDial and the return

   ECheckConnect     m_ePseudoState;

   BOOL              m_fCancelled;

   CRITICAL_SECTION  m_CriticalSection;
   LPCRITICAL_SECTION m_hCriticalSection;

   BOOL              m_fRasInstalled;
   TRas              m_rasLib;

   BYTE              m_byRedials;

   bool              m_fAskedQuestionAlready;   // Disconnect question
};
