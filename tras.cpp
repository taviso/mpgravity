/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tras.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:16  richard_wood
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
#include <ras.h>
#include "tras.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRas::TRas()
{
   m_hLib = 0;
}

BOOL TRas::Initialize()
{
   m_hLib = LoadLibrary ("RASAPI32.DLL");

   // only found on NT.
   m_hDlgLib = LoadLibrary ("RASDLG.DLL");

   // Assume that RAS is not installed if the DLL is missing

   return m_hLib ? TRUE : FALSE;
}

TRas::~TRas()
{
   if (m_hLib)
      FreeLibrary( m_hLib );
   if (m_hDlgLib)
      FreeLibrary( m_hDlgLib );
}

FARPROC TRas::get_proc_address(LPCTSTR name)
{
   if (0 == m_hLib)
      throw(new TRasException);

   FARPROC fp = GetProcAddress (m_hLib, name);
   if (0 == fp)
      throw(new TRasGetProcAddrFailed);
   return fp;
}

///////////////////////////////////////////////////////////////////////////
//
DWORD TRas::RasCreatePhonebookEntry(HWND hwnd, LPTSTR pPhoneBook)
{
   FARPROC fp = get_proc_address ("RasCreatePhonebookEntryA");
   P_CREATEPHONEBOOKENTRY proc = (P_CREATEPHONEBOOKENTRY) fp;
   return proc( hwnd, pPhoneBook );
}

///////////////////////////////////////////////////////////////////////////
//
//
DWORD TRas::RasDial(
LPRASDIALEXTENSIONS pDE,
LPTSTR              phonebook,
LPRASDIALPARAMS     pDP,
DWORD               notifier,
LPVOID              pNotifier,
LPHRASCONN          phRasConn)
{
   FARPROC fp = get_proc_address ("RasDialA");

   P_RASDIAL proc = (P_RASDIAL) fp;
   return proc(pDE, phonebook, pDP, notifier, pNotifier, phRasConn);
}

///////////////////////////////////////////////////////////////////////////
//
DWORD TRas::RasEditPhonebookEntry(HWND hwnd, LPTSTR pPhonebook, LPTSTR entry)
{
   FARPROC fp = get_proc_address ("RasEditPhonebookEntryA");
   P_RASEDITPHONEBOOKENTRY proc = (P_RASEDITPHONEBOOKENTRY) fp;
   return proc(hwnd, pPhonebook, entry);
}

///////////////////////////////////////////////////////////////////////////
//
DWORD TRas::RasEnumConnections(LPRASCONN pRasConn, LPDWORD lpcb,
                               LPDWORD lpcConn)
{
   FARPROC fp = get_proc_address ("RasEnumConnectionsA");
   P_RASENUMCONNECTIONS proc = (P_RASENUMCONNECTIONS) fp;
   return proc(pRasConn, lpcb, lpcConn);
}

DWORD
TRas::RasEnumEntries(LPTSTR r1, LPTSTR pPhoneBook, LPRASENTRYNAME pEntry,
                        LPDWORD lpcb, LPDWORD lpcEntries)
{
   FARPROC fp = get_proc_address ("RasEnumEntriesA");
   P_RASENUMENTRIES proc = (P_RASENUMENTRIES) fp;
   return proc(r1, pPhoneBook, pEntry, lpcb, lpcEntries);
}

///////////////////////////////////////////////////////////////////////////
//
DWORD TRas::RasGetConnectStatus(HRASCONN hRasConn, LPRASCONNSTATUS pStatus)
{
   FARPROC fp = get_proc_address ("RasGetConnectStatusA");
   P_RASGETCONNECTSTATUS proc = (P_RASGETCONNECTSTATUS) fp;
   return proc(hRasConn, pStatus);
}

///////////////////////////////////////////////////////////////////////////
//
DWORD TRas::RasGetEntryDialParams(
   LPTSTR pPhoneBook, LPRASDIALPARAMS pDialParams, LPBOOL pfPassword)
{
   FARPROC fp = get_proc_address ("RasGetEntryDialParamsA");

   P_RASGETENTRYDIALPARAMS proc = (P_RASGETENTRYDIALPARAMS) fp;
   return proc(pPhoneBook, pDialParams, pfPassword);
}

///////////////////////////////////////////////////////////////////////////
//
//
DWORD TRas::RasGetErrorString(UINT errVal, LPTSTR errorString, DWORD bufSize)
{
   FARPROC fp = get_proc_address ("RasGetErrorStringA");
   P_RASGETERRORSTRING proc = (P_RASGETERRORSTRING)fp;

   return proc(errVal, errorString, bufSize);
}

///////////////////////////////////////////////////////////////////////////
DWORD TRas::RasHangUp(HRASCONN hRasConn)
{
   FARPROC fp = get_proc_address ("RasHangUpA");
   P_RASHANGUP proc = (P_RASHANGUP) fp;
   return proc(hRasConn);
}

///////////////////////////////////////////////////////////////////////////
DWORD TRas::RasSetEntryDialParams(LPTSTR pBook, LPRASDIALPARAMS pDP, BOOL fRemovePwd)
{
   FARPROC fp = get_proc_address ("RasSetEntryDialParamsA");
   P_RASSETENTRYDIALPARAMS proc = (P_RASSETENTRYDIALPARAMS) fp;
   return proc(pBook, pDP, fRemovePwd);
}

///////////////////////////////////////////////////////////////////////////
BOOL TRas::RasNTDialDlg (LPTSTR       pBook,
                         LPCTSTR      pEntry,
                         LPTSTR       pPhoneNum,
                         LPRASDIALDLG psInfo)
{
   FARPROC fp = GetProcAddress (m_hDlgLib, "RasDialDlgA");

   P_RASDIALDLG proc = (P_RASDIALDLG) fp;

   return proc (pBook, (LPTSTR)pEntry, pPhoneNum, psInfo);
}

