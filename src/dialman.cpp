/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: dialman.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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

// TDialupManager - class for managing a dialup connection for Gravity

#include "stdafx.h"
#include "mplib.h"
#include "globals.h"
#include "server.h"
#include "dialman.h"
#include "critsect.h"
#include "fileutil.h"
#include "resource.h"
#include "genutil.h"       // GetOS
#include "timemb.h"        // TimeMsgBox_MessageBox
#include "servcp.h"        // TServerCountedPtr
#include "raserror.h"      // ERROR_NO_CONNECTION
#include "rasdlg.h"        // TRasDialDlg  our timer redial dlg

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TDialupManager dialMgr;  // a global

P_RASSTATUSFUNC gpfnStatus = 0;

// prototypes;
static UINT GetRasConnStateStringID( RASCONNSTATE rasconn );

static HWND hWaitDlg;

static HWND ghWndOutput = 0;   // set within Connect
static UINT gmsgOutput  = 0;   // set within Connect

BOOL   postSuccess ()
{
	PostMessage (ghWndOutput, gmsgOutput, 0, 0);
	return TRUE;
}

BOOL   postFailure (DWORD dwError)
{
	// stuff error code into wParam
	PostMessage (ghWndOutput, gmsgOutput, (WPARAM) dwError, 0);
	return TRUE;
}

// prototype
void WINAPI ras_process_event (UINT         uMsg,
							   RASCONNSTATE rasconnstate,
							   DWORD        dwError);

/////////////////////////////////////////////////////////////////////////////
// TDialupManager ctor - Construct a dialup manager for Gravity
/////////////////////////////////////////////////////////////////////////////

TDialupManager::TDialupManager()

{
	m_fRasInstalled      = FALSE;
	m_hConnection        = 0;
	m_fWeCreated         = FALSE;
	m_fCancelled         = FALSE;

	m_ePseudoState       = kNotConnected;

	m_byRedials          = 0;

	m_hCriticalSection = &m_CriticalSection;

	m_fAskedQuestionAlready = false;

	InitializeCriticalSection (m_hCriticalSection);
}

/////////////////////////////////////////////////////////////////////////////
// loads RAS dll
/////////////////////////////////////////////////////////////////////////////

BOOL TDialupManager::Initialize ()
{
	m_fRasInstalled = m_rasLib.Initialize ();
	return m_fRasInstalled;
}

/////////////////////////////////////////////////////////////////////////////
// TDialupManager dtor
/////////////////////////////////////////////////////////////////////////////

TDialupManager::~TDialupManager ()
{
	DeleteCriticalSection (m_hCriticalSection);
}

/////////////////////////////////////////////////////////////////////////////
// ShowRasError - Show a RAS error to the user.
/////////////////////////////////////////////////////////////////////////////

void TDialupManager::ShowRasError (bool fDuringInit, DWORD errorNum)

{
	CString strIntro;

	if (fDuringInit)
		strIntro.LoadString (IDS_ERR_DIALUP_EST);
	else
		strIntro.LoadString (IDS_ERR_DIALUP_TERM);

	TCHAR    errorBuff[512];
	CString  errorString;

	if (0 != m_rasLib.RasGetErrorString (errorNum,
		errorBuff, sizeof (errorBuff)))
		errorBuff[0] = 0;

	errorString.Format ("%s - %s.", (LPCTSTR)strIntro, errorBuff);
	NewsMessageBox (TGlobalDef::kMainWnd,  errorString, MB_OK | MB_ICONEXCLAMATION);
}

/////////////////////////////////////////////////////////////////////////////
// Connect - Take the steps to create a dialup networking connection.
//
// Notes:      1) If ForceConnect is FALSE, then we just return true,
//                since we assume that the connection has been established.
//             2) If PromptBeforeConnecting is set, we prompt before
//                attempting to make the dialup connection.
//             3) If UseExistingConnection, we check to see if the
//                DUN connection exists by enumerating over the current
//                connections, and if found, using it.  Otherwise
//                we proceed to build a connection.
/////////////////////////////////////////////////////////////////////////////

BOOL TDialupManager::Connect (P_RASSTATUSFUNC pStatusFunc, HWND hWnd, UINT msg)

{
	ghWndOutput = hWnd;
	gmsgOutput  = msg;

	m_fAskedQuestionAlready = false;

	TServerCountedPtr cpNewsServer;  // smart pointer

	// act dumb
	if (!m_fRasInstalled)
	{
		return postSuccess ();
	}

	if (!IsActiveServer())
		throw(new TException (IDS_ERR_NO_SERVER_SET_UP, kError));

	// don't let more than one person Try to connect at once...

	if (TDialupManager::kConnecting == m_ePseudoState)
	{
		//TRACE0("we are connecting already...\n");
		return FALSE;
	}

	// if we're already connected, just return TRUE

	const CString & strDialupName = cpNewsServer->GetConnectionName();

	HRASCONN hFound = 0;

	if (FindConnectionByName (kFindOurs, strDialupName, hFound))
	{
		//TRACE0("we are Already connected...\n");

		TEnterCSection cs(m_hCriticalSection);
		m_hConnection = hFound;
		m_ePseudoState = kConnected;

		return postSuccess ();
	}

	if (!cpNewsServer->GetForceConnection() && !cpNewsServer->GetForceDisconnect ())
	{
		// we're not responsible for any RAS work.  or this is a LAN connection
		return postSuccess ();
	}

	BOOL fRet;

	// if we're not told to force a connection, then we just
	// tell whoever is calling us the connection has been
	// established

	if (!cpNewsServer->GetForceConnection())
	{
		TEnterCSection ds(m_hCriticalSection);

		FindConnectionByName(kFindAny, "", m_hConnection);

		return postSuccess ();
	}

	if (strDialupName.IsEmpty())
	{
		// "You have not selected a dialup connection to use."
		NewsMessageBox (TGlobalDef::kMainWnd, IDS_DIALUP_NOCONNECTION, MB_OK | MB_ICONSTOP);
		return TRUE;
	}

	if (cpNewsServer->GetUseExistingConnection())
	{
		// iterate over the existing connections and use
		// the existing one if one is there...
		HRASCONN hRasConn;

		if (FindConnectionByName (kFindOurs, strDialupName, hRasConn))
		{
			//TRACE0("Found an existing connection\n");

			// we found the connection that the user specified, so
			// we set that we connected and return TRUE to the user...
			{
				TEnterCSection cs(m_hCriticalSection);
				m_hConnection = hRasConn;
			}

			return postSuccess ();
		}
	}

	LPCTSTR pszConnectionName = strDialupName;

	if (cpNewsServer->GetPromptBeforeConnecting())
	{
		CString  connectionPrompt;
		AfxFormatString1 (connectionPrompt, IDS_DIALUP_CONNECT1, pszConnectionName);

		// "Connect to %s now?"
		if (IDNO == NewsMessageBox(TGlobalDef::kMainWnd,
			connectionPrompt,
			MB_YESNO | MB_ICONQUESTION))
		{
			return FALSE;
		}
	}

	// get to the nitty gritty
	gpfnStatus = pStatusFunc;
	fRet = MakeConnection (pszConnectionName);
	return fRet;
}

/////////////////////////////////////////////////////////////////////////////
// Disconnect - Handle disconnecting from the dialup connection we may
//              or may not have started.  This is controlled by two variables
//              that can be set in the server - ForceDisconnect and
//              DisconnectIfWeOpened.  The method will also show a prompt
//              if PromptBeforeDisconnecting is set at the server.
//
//  fAutoDisconnect - indicates that Gravity is disconnecting automatically.
//              Don't let the MessageBoxes halt the shutdown
/////////////////////////////////////////////////////////////////////////////
BOOL TDialupManager::Disconnect(BOOL fAutoDisconnect)
{
	DWORD             dwRC;
	TServerCountedPtr cpNewsServer;  // smart pointer

	//TRACE ("%s : %d, thread ID : %d\n", __FILE__, __LINE__, AfxGetThread ());
	if (FALSE == m_fRasInstalled)
		return TRUE;

	if (!IsActiveServer ())
		throw(new TException (IDS_ERR_NO_SERVER_SET_UP, kError));

	if (!cpNewsServer->GetForceDisconnect())
	{
		// reset to clean state
		set_vars_init ();
		return TRUE;
	}

	// if they only want us to disconnect sessions we created, then return
	if (cpNewsServer->GetDisconnectIfWeOpened ())
	{
		if (!m_fWeCreated)
		{
			// reset to clean state
			set_vars_init ();
			return TRUE;
		}
	}

	if (kConnecting == m_ePseudoState)
	{
		// amc - it's valid to Try to abort a RAS session that is
		// connecting. Proceed
	}
	else if (0 == m_hConnection)
	{
		set_vars_init ();
		return TRUE;
	}

	try
	{
		// keep from asking the disconnect Question more than once.
		// this is weird since you might get asked 3 times:
		//     1) disconnect
		//     2) mainfrm OnClose
		//     3) ExitInstance

		if (m_fAskedQuestionAlready)
		{
			set_vars_init ();
			return FALSE;
		}

		if (cpNewsServer->GetPromptBeforeDisconnecting() && !m_fAskedQuestionAlready)
		{
			int      nMsgRet;
			CString  promptString;

			AfxFormatString1(promptString, IDS_DIALUP_ASKDISCONNECT1,
				cpNewsServer->GetConnectionName ());

			UINT uBoxFlags = MB_YESNO | MB_ICONQUESTION;
			if (fAutoDisconnect)
			{
				// Timeout after 60 seconds - return IDTIMEOUT
				nMsgRet = NewsMessageBoxTimeout (60, NULL, promptString, uBoxFlags);
			}
			else
				nMsgRet = NewsMessageBox(TGlobalDef::kMainWnd, promptString, uBoxFlags);

			m_fAskedQuestionAlready = true;

			if (IDNO == nMsgRet)
			{
				set_vars_init ();
				return FALSE;
			}
		}
	}
	catch(...)
	{
		LINE_TRACE();
		throw;
	}

	// go ahead and close the connection....
	try
	{
		// 2-26-98.  Even if the server has dropped us,
		//   (FindConnectionByName==FALSE)
		//   call ras hangup on the m_hConnection handle.  It seems to
		//   help ras come back to a clean initial state.

		if (kConnecting == m_ePseudoState)
			m_fCancelled = TRUE;

		{
			TEnterCSection cs(m_hCriticalSection);

			dwRC = 0;

			ASSERT(m_hConnection);

			//TRACE0("In Disconnect() - call RasHangUp\n");

			if (m_hConnection)
			{
				dwRC = m_rasLib.RasHangUp (m_hConnection);
			}

			if (0 == dwRC)
			{
				// wait for handle to become invalid
				wait_after_hangup (m_hConnection);

				// set vars to indicate cancelled
				set_vars_init ();

				return TRUE;
			}
		}

		// deal with error

		set_vars_init ();

		// it's silly to say "Can't hang up - connection was dropped"
		if (ERROR_NO_CONNECTION != dwRC)
		{
			ShowRasError (false, dwRC);
		}

		return FALSE;

	}
	catch(...)
	{
		LINE_TRACE();
		throw;
	}
}

// -------------------------------------------------------------------------
// set_vars_init -- reset variables to a clean state
void TDialupManager::set_vars_init()
{
	TEnterCSection cs(m_hCriticalSection);

	m_fWeCreated  = FALSE;
	m_fCancelled  = FALSE;
	m_hConnection = NULL;
	m_ePseudoState = kNotConnected;
}

//-------------------------------------------------------------------------
// called from the dialog box
void WINAPI ras_process_event (UINT         uMsg,
							   RASCONNSTATE rasconnstate,
							   DWORD        dwError)
{
	// wait for 1 of three cases
	//  dwError is not zero - An Error occurred
	//  RASCS_Connected -
	//  someone called RasHangup
	if (dwError)
	{
		//TRACE("callback - (error) %d, %u\n", (int)rasconnstate, dwError);

		// clear status line
		if (gpfnStatus)
			gpfnStatus ( -1 );

		// both cases need to set flags to failure values
		dialMgr.processEventsResult (false);

		dialMgr.hangup_nowait ();

		// special handling for ERROR_LINE_BUSY, pass error up, so upper layer can redial
		if (ERROR_LINE_BUSY == dwError ||
			ERROR_VOICE_ANSWER == dwError)
		{
			postFailure (dwError);
			return ;
		}

		if (ERROR_USER_DISCONNECTION == dwError)
		{
			// user interrupted dialing themselves, so they don't need to see the error msg.
		}
		else
			dialMgr.ShowRasError (true, dwError);
		return;
	}

	if (gpfnStatus)
		gpfnStatus ( GetRasConnStateStringID(rasconnstate) );

	if (RASCS_DONE & rasconnstate)
	{
		dialMgr.processEventsResult (true);  // success
		postSuccess ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// TDialupManager::GetDialupState
//
// Returns:   ECheckConnect[kNotConnected, kConnected,
//                          kConnecting, kNotInstalled]
//
/////////////////////////////////////////////////////////////////////////////
TDialupManager::ECheckConnect TDialupManager::GetDialupState ()
{
	if (FALSE == m_fRasInstalled)
		return kNotInstalled;

	// don't use FindConnectionByName, use the pseudo var

	return m_ePseudoState;
}

/////////////////////////////////////////////////////////////////////////////
BOOL TDialupManager::MakeConnection (LPCTSTR pConnName)

{
	if (RUNNING_ON_WINNT == GetOS())
	{
		m_ePseudoState = kConnecting;

		RASDIALDLG  DlgInfo;

		ZeroMemory (&DlgInfo, sizeof(DlgInfo));

		DlgInfo.dwSize    = sizeof(DlgInfo);
		DlgInfo.hwndOwner = ghWndOutput;

		// returns 0 for ERROR or if the user Canceled

		BOOL fRet = m_rasLib.RasNTDialDlg (NULL,         // phonebook
			pConnName,    // phbook entry
			NULL,         // override phone number
			&DlgInfo);

		if (fRet)
		{
			TEnterCSection cs(m_hCriticalSection);

			FindConnectionByName (kFindOurs, pConnName, m_hConnection);

			m_ePseudoState = kConnected;
			m_fWeCreated  = TRUE;

			return postSuccess ();
		}
		else
		{
			m_ePseudoState = kNotConnected;

			if (ERROR_LINE_BUSY == DlgInfo.dwError)
				postFailure (DlgInfo.dwError);

			return FALSE;
		}
	}

	RASDIALPARAMS  rasParms;

	DWORD          dwRC;

	m_fCancelled = FALSE;

	// get the dial parameters from the registry...
	if (FALSE == get_dial_parms ( pConnName, &rasParms ))
		return FALSE;

	HRASCONN   hConnection = NULL;

	m_ePseudoState = kConnecting;
	m_fWeCreated  = TRUE;

	RASDIALEXTENSIONS extensions;
	ZeroMemory (&extensions, sizeof (extensions));
	extensions.dwSize = sizeof (extensions);
	extensions.dwfOptions |= RDEOPT_PausedStates;

	// dialog processes events via ras_process_event (in this module)
	dwRC = m_rasLib.RasDial (
		&extensions,         // RASDIALEXTENSIONS - we don't use them
		NULL,                // use the default phonebook
		&rasParms,           // parameters for RAS dialer
		(DWORD) 0 ,          // treat next param as type RasDialFunc
		ras_process_event,   // pass in our function ptr
		&hConnection);

	if (dwRC)
	{
		if (true)
		{
			TEnterCSection cs(m_hCriticalSection);

			hangup_and_wait ( hConnection );
			m_hConnection = NULL;
		}

		if (m_fCancelled)
		{
			m_fCancelled = FALSE; // reset to clean state
		}
		else
			ShowRasError (true, dwRC);

		m_ePseudoState = kNotConnected;
		m_fWeCreated  = FALSE;
		return FALSE;
	}
	else
	{
		TEnterCSection cs(m_hCriticalSection);
		m_hConnection = hConnection;
	}

	return TRUE;
}

DWORD TDialupManager::RasCreatePhonebookEntry(HWND hwnd, LPTSTR pPhoneBook)
{
	return m_rasLib.RasCreatePhonebookEntry(hwnd, pPhoneBook);
}

DWORD TDialupManager::RasEditPhonebookEntry(HWND hwnd, LPTSTR pPhoneBook, LPTSTR entry)
{
	return m_rasLib.RasEditPhonebookEntry(hwnd, pPhoneBook, entry);
}

DWORD TDialupManager::RasEnumEntries(LPTSTR r1, LPTSTR pPhoneBook, LPRASENTRYNAME pEntry,
									 LPDWORD lpcb, LPDWORD lpcEntries)
{
	return m_rasLib.RasEnumEntries(r1, pPhoneBook, pEntry, lpcb, lpcEntries);
}

DWORD
TDialupManager::RasGetEntryDialParams(LPTSTR pPhoneBook,
									  LPRASDIALPARAMS pDialParams,
									  LPBOOL pfPassword)
{
	return m_rasLib.RasGetEntryDialParams(pPhoneBook, pDialParams, pfPassword);
}

DWORD
TDialupManager::RasSetEntryDialParams(LPTSTR pBook,
									  LPRASDIALPARAMS pDP,
									  BOOL fRemovePwd)
{
	return m_rasLib.RasSetEntryDialParams(pBook, pDP, fRemovePwd);
}

//-------------------------------------------------------------------------
void TDialupManager::wait_after_hangup (HRASCONN hConnection)
{
	RASCONNSTATUS  rasStatus;
	DWORD          dwRC;
	CTime start = CTime::GetCurrentTime();
	do
	{
		ZeroMemory (&rasStatus, sizeof(rasStatus));
		rasStatus.dwSize = sizeof(rasStatus);

		dwRC = m_rasLib.RasGetConnectStatus (hConnection, &rasStatus);
		if (ERROR_INVALID_HANDLE == dwRC)
			break;
		Sleep (0);
	} while ((CTime::GetCurrentTime() - start).GetSeconds() < 15);
}

//-------------------------------------------------------------------------
BOOL TDialupManager::get_dial_parms (LPCTSTR        pConnName,
									 RASDIALPARAMS* prasParms)
{
	BOOL  fPassword = FALSE;
	DWORD dwRC;

	ZeroMemory (prasParms, sizeof(RASDIALPARAMS));
	prasParms->dwSize = sizeof(RASDIALPARAMS);

	if (sizeof(*prasParms) != sizeof(RASDIALPARAMS))
		AfxThrowMemoryException();

	_tcscpy(prasParms->szEntryName, pConnName);

	dwRC = m_rasLib.RasGetEntryDialParams(
		NULL,         // use default phonebook
		prasParms,    // pointer to parameters
		&fPassword);  // was password returned?
	if (0 != dwRC)
	{
		// "Error retrieving dialup networking parameters."
		NewsMessageBox (TGlobalDef::kMainWnd, IDS_DIALUP_ERRGETDIALPARAMS, MB_OK | MB_ICONSTOP);
		return FALSE;
	}
	return TRUE;
}

// ------------------------------------------------------------------------
//  PURPOSE: get the index to the corresponding string
//
//  PARAMETERS:
//    rasconn - ras connection state
//
//  RETURNS:
//    index into stringtable.
static UINT GetRasConnStateStringID( RASCONNSTATE rasconn )
{
	switch( rasconn )
	{
	case RASCS_OpenPort:
		return IDS_OPENPORT;
	case RASCS_PortOpened:
		return IDS_PORTOPENED;
	case RASCS_ConnectDevice:
		return IDS_CONNECTDEVICE;
	case RASCS_DeviceConnected:
		return IDS_DEVICECONNECTED;
	case RASCS_AllDevicesConnected:
		return IDS_ALLDEVICESCONNECTED;
	case RASCS_Authenticate:
		return IDS_AUTHENTICATE;
	case RASCS_AuthNotify:
		return IDS_AUTHNOTIFY;
	case RASCS_AuthRetry:
		return IDS_AUTHRETRY;
	case RASCS_AuthCallback:
		return IDS_AUTHCALLBACK;
	case RASCS_AuthChangePassword:
		return IDS_AUTHCHANGEPASSWORD;
	case RASCS_AuthProject:
		return IDS_AUTHPROJECT;
	case RASCS_AuthLinkSpeed:
		return IDS_AUTHLINKSPEED;
	case RASCS_AuthAck:
		return IDS_AUTHACK;
	case RASCS_ReAuthenticate:
		return IDS_REAUTHENTICATE;
	case RASCS_Authenticated:
		return IDS_AUTHENTICATED;
	case RASCS_PrepareForCallback:
		return IDS_PREPAREFORCALLBACK;
	case RASCS_WaitForModemReset:
		return IDS_WAITFORMODEMRESET;
	case RASCS_WaitForCallback:
		return IDS_WAITFORCALLBACK;
	case RASCS_StartAuthentication:
		return IDS_STARTAUTH;
	case RASCS_CallbackComplete:
		return IDS_CALLBACK_COMPLETE;
	case RASCS_LogonNetwork:
		return IDS_LOGON_NETWORK;
	case RASCS_SubEntryConnected:
		return IDS_SUBENTRYCONNECTED;
	case RASCS_Interactive:
		return IDS_INTERACTIVE;
	case RASCS_RetryAuthentication:
		return IDS_RETRYAUTHENTICATION;
	case RASCS_CallbackSetByCaller:
		return IDS_CALLBACKSETBYCALLER;
	case RASCS_PasswordExpired:
		return IDS_PASSWORDEXPIRED;
	case RASCS_Connected:
		return IDS_CONNECTED;
	case RASCS_Disconnected:
		return IDS_DISCONNECTED;
	default:
		return IDS_RAS_UNDEFINED_ERROR;
	}
}

//-------------------------------------------------------------------------
void TDialupManager::hangup_and_wait (HRASCONN hConnection)
{
	if (hConnection)
	{
		m_rasLib.RasHangUp ( hConnection );
		wait_after_hangup ( hConnection );
	}
}

/////////////////////////////////////////////////////////////////////////////
// FindConnectionByName - Locate a connection (in the set of current
//                        connections) by name
/////////////////////////////////////////////////////////////////////////////

BOOL TDialupManager::FindConnectionByName(
	EFindConn       eFind,
	const CString & dialupName,
	HRASCONN &      hConnection)

{
	RASCONN  connections[10];     // assume there are not more than 10 ras conns
	DWORD    cbSize;
	DWORD    numConnections;
	DWORD    dwRC;

	cbSize = sizeof (connections);

	connections[0].dwSize = sizeof (RASCONN);

	dwRC = m_rasLib.RasEnumConnections (connections,
		&cbSize,
		&numConnections);

	if (0 == dwRC)
	{
		for (int i = 0; i < int(numConnections); i++)
		{
			if (eFind == kFindOurs)
			{
				if (dialupName.CompareNoCase(connections[i].szEntryName) == 0)
				{
					hConnection = connections[i].hrasconn;
					return TRUE;
				}
			}
			else  // kFindAny
			{
				hConnection = connections[i].hrasconn;
				return TRUE;
			}
		}
	}

	hConnection = NULL;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void TDialupManager::processEventsResult (bool fSuccess)
{
	if (fSuccess)
	{
		m_ePseudoState = kConnected ;

		m_byRedials = 0;
	}
	else
	{
		m_ePseudoState = kNotConnected;
	}
}

/////////////////////////////////////////////////////////////////////////////
void TDialupManager::hangup_nowait ()
{
	TEnterCSection cs(m_hCriticalSection);

	if (m_hConnection)
	{
		// we are not hanging up a Valid connection, more like freeing the
		//    memory for this handle
		// come here if we get a busy signal
		m_rasLib.RasHangUp (m_hConnection);

		m_hConnection = 0;
	}
}

// returns true to redial
bool TDialupManager::incrementRedialCount (bool fResetToZero)
{
	BYTE n = m_byRedials + 1;

	if (n > 4 || fResetToZero)
	{
		// over the limit
		m_byRedials = 0;
		return false;
	}
	else
	{
		m_byRedials = n;
		return true;
	}
}

// ------------------------------------------------------------------------
// Returns true for "ok to redial"
bool TDialupManager::ShowRedialDlg (CWnd * pAnchor)
{
	TRasDialDlg  sDlg(pAnchor);

	sDlg.DoModal();

	return sDlg.getAnswer();
}

