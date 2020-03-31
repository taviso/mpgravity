/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tras.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

///////////////////////////////////////////////////////////////////////////
//
//  The RAS dll may not be installed on the users system. So we can't
//  implicitly link to it.  This object handles the explicit linking:
//   LoadLibrary(), GetProcAddr(), junk like that
//
#pragma once

#include <ras.h>
#include <rasdlg.h>

class TRasException {};
class TRasGetProcAddrFailed {};

class TRas
{
public:
	TRas();
	~TRas();

	BOOL Initialize();

	DWORD RasCreatePhonebookEntry(HWND hwnd, LPTSTR pPhoneBook);

	DWORD RasDial(LPRASDIALEXTENSIONS pDE,
		LPTSTR              phonebook,
		LPRASDIALPARAMS     pDP,
		DWORD               notifier,
		LPVOID              pNotifier,
		LPHRASCONN          phRasConn);

	DWORD RasEditPhonebookEntry(HWND hwnd, LPTSTR pPhoneBook, LPTSTR entry);

	DWORD RasEnumConnections(LPRASCONN pRasConn, LPDWORD lpcb, LPDWORD lpcConn);

	DWORD RasEnumEntries(LPTSTR r1, LPTSTR pPhoneBook, LPRASENTRYNAME pEntry,
		LPDWORD lpcb, LPDWORD lpcEntries);

	DWORD RasGetConnectStatus(HRASCONN hRasConn, LPRASCONNSTATUS pStatus);

	DWORD RasGetEntryDialParams(LPTSTR pPhoneBook,
		LPRASDIALPARAMS pDialParams,
		LPBOOL pfPassword);

	DWORD RasGetErrorString(UINT errVal, LPTSTR errorString, DWORD bufSize);

	DWORD RasHangUp(HRASCONN hRasConn);

	DWORD RasSetEntryDialParams(LPTSTR pBook, LPRASDIALPARAMS pDP, BOOL fRemovePwd);

	BOOL  RasNTDialDlg (LPTSTR pBook, LPCTSTR pEntry, LPTSTR pPhoneNum, LPRASDIALDLG psInfo);

protected:
	FARPROC get_proc_address(LPCTSTR name);

	HINSTANCE m_hLib;
	HINSTANCE m_hDlgLib;
};

typedef DWORD (WINAPI *P_CREATEPHONEBOOKENTRY)(HWND, LPTSTR);

typedef int (WINAPI *P_RASDIAL)(LPRASDIALEXTENSIONS,
								LPTSTR,
								LPRASDIALPARAMS,
								DWORD,
								LPVOID,
								LPHRASCONN);

typedef DWORD (WINAPI *P_RASEDITPHONEBOOKENTRY)
(HWND hwnd, LPTSTR pPhoneBook, LPTSTR entry);

typedef DWORD (WINAPI *P_RASENUMCONNECTIONS)
(LPRASCONN pRasConn, LPDWORD lpcb, LPDWORD lpcConn);

typedef DWORD (WINAPI *P_RASENUMENTRIES)
(LPTSTR r1, LPTSTR pPhoneBook, LPRASENTRYNAME pEntry, LPDWORD lpcb, LPDWORD lpcEntries);

typedef DWORD (WINAPI *P_RASGETCONNECTSTATUS)
(HRASCONN hRasConn, LPRASCONNSTATUS pStatus);

typedef DWORD (WINAPI *P_RASGETENTRYDIALPARAMS)(LPTSTR pPhoneBook,
												LPRASDIALPARAMS pDialParams,
												LPBOOL pfPassword);
typedef DWORD (WINAPI *P_RASGETERRORSTRING)
(UINT errVal, LPTSTR errorString, DWORD bufSize);

typedef DWORD (WINAPI *P_RASHANGUP)(HRASCONN hRasConn);

typedef DWORD (WINAPI *P_RASSETENTRYDIALPARAMS)
(LPTSTR pBook, LPRASDIALPARAMS pDP, BOOL fRemovePwd);

typedef BOOL (WINAPI  *P_RASDIALDLG)
(LPTSTR pBook, LPTSTR pEntry, LPTSTR pPhoneNum, LPRASDIALDLG psInfo);
