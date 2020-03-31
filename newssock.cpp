/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newssock.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.7  2009/02/19 11:24:17  richard_wood
/*  Re-enabled optimisations in classes newssock.cpp, rulesdlg.cpp, server.cpp and tdecjob.cpp
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/09/19 14:51:35  richard_wood
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
#include "newssock.h"
#include "mplib.h"
#include "memspot.h"
#include "evtlog.h"
#include "resource.h"
#include "autoprio.h"
#include "fileutil.h"         // GetWinsockErrorID()
#include "socktrak.h"
#include "tmsgbx.h"           // NewsMessageBox
#include "utlmacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define PERFMON_STATS

#if defined(PERFMON_STATS)
#include <winperf.h>          // Perfmon Stuff, PPERF_COUNTER_BLOCK, etc
#endif

#define new DEBUG_NEW
#define WM_GRAVSOCK_MSG    (WM_USER + 300)

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

// Turn off all optimizations - helps with catching Trumpet Winsock Crash
//#pragma optimize( "", off )

#if defined(PERFMON_STATS)
// Perfmon stuff
extern PPERF_COUNTER_BLOCK pCounterBlock;
#endif

//
//  Uses    kClassWinsock
//          kClassExternal
//

BYTE TDirectSocket::m_fRegisterClass = 0;
static const TCHAR grcSocketWndClassName[] = _T("MicroPlanetSocketWindow");

// can be enabled in ncmdline.cpp
bool gfLogSocketDataToFile = false;

LRESULT CALLBACK Proc_MicroPlanet_Socket_Window(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT * pcs = (CREATESTRUCT*) lParam;
			HANDLE h = *((HANDLE*) pcs->lpCreateParams);
			SetWindowLong (hwnd, 0, (LONG) h);
			break;
		}

	case WM_GRAVSOCK_MSG:
		{
			// store 'return-code'
			SetWindowLong (hwnd, sizeof HANDLE, (LONG) lParam);
			HANDLE h = (HANDLE)(GetWindowLong (hwnd, 0));
			SetEvent (h);
			break;
		}

	default:
		break;
	}

	return DefWindowProc (hwnd, uMsg, wParam, lParam);
}

BOOL TDirectSocket::InitInstance ()
{
	WNDCLASS wc;
	wc.style         = 0;
	wc.lpfnWndProc   = Proc_MicroPlanet_Socket_Window;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof (HANDLE) + sizeof(LPARAM);
	wc.hInstance     = AfxGetInstanceHandle ();
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = grcSocketWndClassName;

	ATOM atom = RegisterClass ( &wc );
	return  (atom) ? TRUE : FALSE;
}

extern TEventLog* gpEventLog;

// RFC 977 says port 119 is the NewsPort
const int kNEWS_PORT = 119;
const int kSMTP_PORT = 25;

/////////////////////////////////////////////////////////////////////////////
TDirectSocket::TDirectSocket(FARPROC pBlockingHook, HANDLE hStopEvent,
							 bool * pfProcessJobs)
							 : m_pBlockingHook(pBlockingHook), m_hStopEvent(hStopEvent),
							 m_pfProcessJobs(pfProcessJobs)
{
	m_DataSocket = INVALID_SOCKET;
	m_bAborted   = FALSE;
	m_bSendRecvError = false;
	m_PortNumber = 0xFF000;
	m_pTrack = 0;

	int iMaxSize = 8192 * 2;

	m_vDataBuffer.reserve(iMaxSize);

	it = m_vDataBuffer.begin();

	if (gfLogSocketDataToFile)
		m_pTrack = new TSockTrack;
		
	return;
}

/////////////////////////////////////////////////////////////////////////////
TDirectSocket::~TDirectSocket(void)
{
	try
	{
		// not interrupted by the User, nor fatal line error
		if (!m_bAborted && !m_bSendRecvError && WSAIsBlocking())
			WSACancelBlockingCall();

		free_socket ();

		if (m_pTrack)
		{
			delete m_pTrack;
			m_pTrack = NULL;
		}
	}
	catch(...)
	{
		// trap everything
	}
}

// ------------------------------------------------------------------------
// Returns 0 for success
//   other error codes listed in mpsokerr.h
int  TDirectSocket::Connect(TErrorList * pErrList, LPCTSTR hostAddress)
{
	LPCSTR pszHostAddress = 0;
#if defined(_UNICODE)	
	char rcHostAddress[1024];
	WideCharToMultiByte (CP_ACP, 0, hostAddress, _tcslen(hostAddress), rcHostAddress, sizeof(rcHostAddress), 0, 0);
	pszHostAddress = rcHostAddress;
#else
	pszHostAddress = hostAddress;
#endif

	ASSERT(m_PortNumber != 0xFF000);
	struct hostent* pHostEntry;

	char hostBuf[MAXGETHOSTSTRUCT];

	// install our blocking hook, so we can stop things like
	// 'gethostbyname'
	FARPROC pProc = NULL;
	if (m_pBlockingHook)
		pProc = WSASetBlockingHook (m_pBlockingHook);

	// initialize socket address structure
	ZeroMemory(&m_saSocketAddr, sizeof(m_saSocketAddr));

	m_saSocketAddr.sin_family=AF_INET;

	m_saSocketAddr.sin_port = htons((unsigned short)m_PortNumber);

	// mfc does it like this
	m_saSocketAddr.sin_addr.s_addr = inet_addr(pszHostAddress);
	if (m_saSocketAddr.sin_addr.s_addr==INADDR_NONE)
	{
		int iConnect = 0;
#if defined(_DEBUG)
		DWORD start = GetTickCount();
#endif

		// first, try blocking call
		try
		{
			pHostEntry = NULL;
			pHostEntry = gethostbyname(pszHostAddress);
		}
		catch(...)
		{
			NewsMessageBox (NULL, IDS_ERR_FATALGETHOSTBYNAME,
				MB_OK | MB_ICONSTOP);
			TError sError (IDS_ERR_FATALGETHOSTBYNAME, kError, kClassExternal);
			pErrList -> PushError (sError);
			return 1;
		}

		if (NULL == pHostEntry)
		{
			DWORD lastError = WSAGetLastError();
			if (WSAEINTR == lastError)
			{
				TError sError(kError, kClassWinsock, WSAEINTR);
				pErrList->PushError (sError);

				return MPSOCK_ERR_USERCANCEL;
			}

			// blocking called failed, Try non-blocking call
			//TRACE0("trying async gethostbyname\n");
			iConnect = sub_connect (pszHostAddress, hostBuf, sizeof hostBuf);
			if (0 == iConnect)
				pHostEntry = (struct hostent*) hostBuf;
		}

//#if defined(_DEBUG)
//		TRACE1("%d Ticks for gethostbyname\n", GetTickCount() - start);
//#endif

		if (0 == iConnect)
		{
			// pHostEntry = (struct hostent*) hostBuf;
			m_saSocketAddr.sin_addr.s_addr = ((LPIN_ADDR)pHostEntry->h_addr)->s_addr;

			m_strDebugDottedAddr = inet_ntoa ( m_saSocketAddr.sin_addr );
		}
		else
		{
			if (-1 == iConnect)
			{
				TError sError( IDS_ERR_SUBCONNECT, kError, kClassInternal);

				pErrList->PushError ( sError );
				return 1;
			}

			if (MPSOCK_ERR_USERCANCEL == iConnect)
			{
				TError sError(kError, kClassWinsock, WSAEINTR);
				pErrList->PushError (sError);

				return 1;
			}

			// just a winsock error

			TError sE1(kError, kClassWinsock, iConnect);
			pErrList->PushError (sE1);

			//TRACE1("can't get '%s' host entry\n",hostAddress);

			return MPSOCK_ERR_GETHOSTBYNAME;
		}
	}
	else
	{
		// user passed us a dotted address
		m_strDebugDottedAddr = hostAddress;
	}

	// allocate a socket
	try
	{
		m_DataSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	}
	catch(...)
	{
		TError sErr(IDS_ERR_CREATESOCKET, kError, kClassExternal);

		pErrList->PushError (sErr);
		return MPSOCK_ERR_SOCKET_FATAL;
	}

	if (INVALID_SOCKET == m_DataSocket)
	{
		DWORD dwError = WSAGetLastError();

		TError sErr(kError, kClassWinsock, dwError);
		pErrList->PushError (sErr);

		return MPSOCK_ERR_CREATESOCKET;
	}

	// connect the socket

	int  iRet = 0;

	if (connect(m_DataSocket,
		(struct sockaddr *)&m_saSocketAddr,
		sizeof(m_saSocketAddr))==SOCKET_ERROR)
	{
		DWORD dwError = WSAGetLastError();

		//TRACE1(" connect() failed with %d\n", dwError);

		TError sErr1(kError, kClassWinsock, dwError);
		pErrList->PushError (sErr1);

		iRet = MPSOCK_ERR_CONNECT;
	}
	else
	{
		BOOL fKeepAlive = TRUE;
		VERIFY (setsockopt(m_DataSocket, SOL_SOCKET, SO_KEEPALIVE,
			(const char*) &fKeepAlive, sizeof BOOL) == 0);
	}

	return iRet;
}

//-------------------------------------------------------------------------
// artificially call WSAAsyncGetHostByName
// returns   -1 for internal error
//            0 for success
//           winsock error
int TDirectSocket::sub_connect(LPCSTR hostAddress, LPSTR pbuf, int buflen)
{
	TRACE("TDirectSocket::sub_connect >\n");

	auto_prio sPrio(auto_prio::kNormal);

	extern HWND ghwndMainFrame;

	int  ret = -1;
	BOOL fWindowEvent = FALSE;
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (0 == hEvent)
	{
		TRACE("TDirectSocket::sub_connect : Error creating hEvent<\n");
		return ret;
	}

	HWND hWndSink = CreateWindow (grcSocketWndClassName,
		_T(""),
		WS_POPUP,
		0, 0, 0, 0,
		ghwndMainFrame,      // parent
		NULL,                // menu,
		AfxGetInstanceHandle (),
		&hEvent);
	if (0 == hWndSink)
	{
		CloseHandle (hEvent);
		TRACE("TDirectSocket::sub_connect : Error creating window <\n");
		return ret;
	}

	HANDLE hAsync = WSAAsyncGetHostByName (
		hWndSink, WM_GRAVSOCK_MSG, hostAddress,
		pbuf, buflen);
	if (0==hAsync)
	{
		ret = GetLastError ();
		CloseHandle (hEvent);
		DestroyWindow (hWndSink);
		TRACE("TDirectSocket::sub_connect : Error opening socket <\n");
		return ret;
	}
	else
	{
		TRACE("TDirectSocket::sub_connect : Starting loop\n");
		int iLoop  = 0;
		do
		{
			int iCount = 1;
			HANDLE vEvents[2];
			vEvents[0] = hEvent;
			if (m_hStopEvent)
			{
				vEvents[1] = m_hStopEvent;
				iCount = 2;
			}

			// MsgWaitForMultipleObjects did not work for me
			DWORD dwRet = WaitForMultipleObjects (iCount, vEvents, FALSE, 200);
			if (WAIT_TIMEOUT == dwRet)
			{
				iLoop ++;
				int iPKLoop = 0;
				//TRACE1("sub_connect Timeout %d\n", iLoop);
				MSG msg;
				while (PeekMessage (&msg, hWndSink, 0, 0, PM_NOREMOVE))
				{
					PeekMessage (&msg, hWndSink, 0, 0, PM_REMOVE);
					++iPKLoop;
					if (WM_QUIT == msg.message)
						goto jmp_quit;
					DispatchMessage( &msg );
				}
				//TRACE1("sub_connect peekd %d\n", iPKLoop);
			}
			else if (WAIT_OBJECT_0 == dwRet)
			{
				fWindowEvent = TRUE;
				break;
			}
			else if ((2==iCount) && (WAIT_OBJECT_0 + 1 == dwRet))
			{
				WSACancelAsyncRequest ( hAsync );
jmp_quit:
				// cancelled by user
				ret = MPSOCK_ERR_USERCANCEL;
				// RLWTODO 8 : a flagged killrequest should end up here
				TRACE("TDirectSocket::sub_connect : Received m_hStopEvent\n");
				break;
			}
		} while (TRUE);

		if (fWindowEvent)
		{
			DWORD dwRet = (DWORD) GetWindowLong(hWndSink, sizeof HANDLE);

			ret = WSAGETASYNCERROR(dwRet);
		}
	}

	DestroyWindow (hWndSink);
	CloseHandle (hEvent);

	return ret;
}

// 10-19-95    WSAcall removed. just use close
void TDirectSocket::CancelBlockingCall(void)
{
	// the blocking call will get WSAEINTR
	free_socket ();
	m_bAborted = TRUE;
}

//////////////////////////////////////////////////////////////////////////
void TDirectSocket::free_socket ()
{
	if (INVALID_SOCKET != m_DataSocket)
	{
		if (SOCKET_ERROR == closesocket(m_DataSocket))
			TRACE1("closesocket error: %d\n",WSAGetLastError());

		// socket is now invalid
		m_DataSocket = INVALID_SOCKET;
	}
}

// ------------------------------------------------------------------------
// Returns 0 for success
int TDirectSocket::ReadNum (TErrorList * pErrList, int& wrd)
{
	int   total = 0;
	BOOL  fInNumber = FALSE;
	int   ret = 0;

	// set this , so we don't have to pass it into myReadChar 1000 times
	m_psErrList = pErrList;

	// note - this reads 1 more character than it should
	for (;;)
	{
		ret = myReadChar_EL ();    // sets m_c
		if (ret)
			break;

		if (m_c >= '0' && m_c <= '9')
		{
			total *= 10 ;
			total += m_c - '0';
			fInNumber = TRUE;
		}
		else
			break;
	}
	wrd = total;

	return ret;
}

// ------------------------------------------------------------------------
// Returns 0 for success
// m_psErrorList must be set!
int  TDirectSocket::ReadLine (TErrorList * pErrList, CString & str)
{
	int len = 0;
	int chunk;

	// set this , so we don't have to pass it into myReadChar 1000 times
	m_psErrList = pErrList;

	while (true)
	{
		if (it == m_vDataBuffer.end())
		{
			// m_psErrorList must be set!
			if (fill_buffer_EL ())
				return 1;
		}
		vector<TCHAR>::iterator result = find(it, m_vDataBuffer.end(), '\n');

		if (m_vDataBuffer.end() == result)	// not found
		{
			chunk = result - it;
			LPTSTR psz = str.GetBuffer (len + chunk);
			CopyMemory(psz + len, &(*it), chunk);
			len += chunk;
			str.ReleaseBuffer (len);
			it = result;
		}
		else
		{
			result++;   // include the \n in the copy
			chunk = result - it;
			LPTSTR psz = str.GetBuffer (len + chunk);
			CopyMemory(psz + len, &(*it), chunk);
			str.ReleaseBuffer (len + chunk);
			it = result;
			return 0;
		}
	}

	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 for success
//
int TDirectSocket::ReadAnswer (
							   TErrorList  *      pErrList,
							   TMemorySpot *      pSpot,
							   bool               fHandleDoublePeriods,
							   P_LINEPROGRESSFUNC pCB,
							   DWORD              dwCookie)
{
	ASSERT(0 == pSpot->size());

	int  ln;
	int  totLineCount = 0;

	CString line;

	do
	{
		if (ReadLine (pErrList, line))
			return 1;

		ln = line.GetLength();
		LPCTSTR pBuf = line;

		if (3 == ln && ('.' == pBuf[0])
			&& ('\r' == pBuf[1])
			&& ('\n' == pBuf[2]))
		{
			break;
		}

		if (fHandleDoublePeriods && '.' == *pBuf)
		{
			// skip first period
			pSpot->Add (pBuf + 1, ln - 1);
		}
		else
			pSpot->Add (pBuf, ln);

		// callback updates the status line
		if (pCB)
			pCB (++totLineCount, dwCookie);

	} while (true);

	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 for success
int  TDirectSocket::WriteLine(TErrorList * pErrList, LPCSTR buf, int nBytesToWrite)
{
	ASSERT(m_DataSocket!=INVALID_SOCKET);
	int stat;

	stat = Write (pErrList, buf, nBytesToWrite);
	if (stat)
		return stat;

	stat = Write (pErrList, "\r\n", 2);
	if (stat)
		return stat;

	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 for success
int   TDirectSocket::Write(TErrorList * pErrList, LPCSTR buf, int nBytesToWrite)
{
	int nBytesLeft;
	int nWritten;
	int nToWrite;

	ASSERT(m_DataSocket!=INVALID_SOCKET);

	nBytesLeft = nBytesToWrite;

	while (nBytesLeft > 0)
	{
		if (nBytesLeft > 512)
			nToWrite = 512;
		else
			nToWrite = nBytesLeft;

		nWritten = send (m_DataSocket, buf, nToWrite, 0);

		if (SOCKET_ERROR == nWritten)
			return send_error (pErrList, WSAGetLastError ());

		else if (m_pTrack)
			m_pTrack->AppSend ((PBYTE)buf, nWritten);

		nBytesLeft -= nWritten;

		buf += nWritten;
	}

	return 0;
}

//-------------------------------------------------------------------------
static void append_readable_explanation (int iSocketError, CString & msg)
{
	int iExplainID = GetWinsockErrorID ( iSocketError );
	if (iExplainID)
	{
		CString strWSError;
		strWSError.LoadString (iExplainID);

		msg += "\n\n";
		msg += strWSError;
	}
}

//-------------------------------------------------------------------------
// Returns 1
//
int TDirectSocket::send_error (TErrorList * pErrList, DWORD dwLastError)
{
	m_bSendRecvError = true;

	CString msg; msg.Format(IDS_ERR_SOCK_SEND, dwLastError);

	CString str;
	str.LoadString (IDS_ERR_SOCKET);

	append_readable_explanation (dwLastError, msg);

	gpEventLog->AddError (TEventEntry::kGeneral, str, msg);

	TError sErr(kError, kClassWinsock, dwLastError);

	sErr.Description (msg);

	pErrList -> PushError (sErr);

	return 1;
}

//-------------------------------------------------------------------------
// Returns 1
int TDirectSocket::recv_error (TErrorList * pErrList,
							   int iBytesRead, DWORD dwLastError)
{
	CString msg;

	m_bSendRecvError = true;

	if (0 == iBytesRead)
	{
		msg.LoadString (IDS_CONN_CLOSED);
		//TRACE0("connection closed\n");

		TError sErr(msg, kError, kClassWinsock);
		pErrList -> PushError (sErr);
	}
	else
	{
		if (WSAEINTR == dwLastError)
		{
			m_bAborted = TRUE;

			msg = "Interrupted by gravity";
		}
		else
		{
			msg.Format(IDS_ERR_SOCK_RECEIVE, dwLastError);

			append_readable_explanation ( (int) dwLastError, msg );
		}

		TError sError(kError, kClassWinsock, dwLastError);

		sError.Description (msg);

		pErrList->PushError (sError);
	}

	// send entry to  event log
	CString str; str.LoadString (IDS_ERR_SOCKET);
	gpEventLog->AddError (TEventEntry::kGeneral, str, msg);

	return 1;
}

// ------------------------------------------------------------------------
// Returns character in member var  m_c
//
int TDirectSocket::myReadChar_EL ()
{
	if (it == m_vDataBuffer.end())
	{
		// m_psErrorList must be set!
		if (fill_buffer_EL ())
			return 1;
	}

	m_c = *it++;
	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 for success
int TDirectSocket::fill_buffer_EL ()
{
	char buf[16384];

	ULONG uSockCount = 0;
	int ret = ioctlsocket(m_DataSocket, FIONREAD, &uSockCount);
	if (SOCKET_ERROR == ret)
	{
		DWORD dwError = WSAGetLastError ();

		CString msg; msg.Format(IDS_ERR_SOCK_IORECEIVE, dwError);

		CString str; str.LoadString (IDS_ERR_SOCKET);

		gpEventLog->AddError (TEventEntry::kGeneral, str, msg);

		TError sError(kError, kClassWinsock, dwError);

		m_psErrList->PushError (sError);

		return (int) dwError;
	}
	else
	{
		if (0 == uSockCount)
			uSockCount = 1;
		uSockCount = min(uSockCount, sizeof(buf));

		// read
		ret = recv (m_DataSocket, buf, uSockCount, 0);
		if (SOCKET_ERROR == ret || 0 == ret)
		{
			return recv_error (m_psErrList, ret, WSAGetLastError () );
		}
		else if (m_pTrack)
			m_pTrack->AppRead ((PBYTE)buf, ret);
		else if ( WaitForSingleObject (m_hStopEvent, 0 ) == WAIT_OBJECT_0 )
		{
			if ( m_pfProcessJobs )
				*m_pfProcessJobs = false;
			return recv_error (m_psErrList, ret, WSAEINTR);
		}

		// I hate to add this extra copy, but using a std::vector allows me to use
		//   the find() function in ::ReadLine()		
		m_vDataBuffer.assign (&buf[0], &buf[ret]);

		ASSERT(m_vDataBuffer.size() == ret);
		it = m_vDataBuffer.begin();
#if defined(PERFMON_STATS)
		{
			PDWORD pdwCounter;      // perfmon counter to increment

			// increment Bytes counter
			pdwCounter = (PDWORD) pCounterBlock;
			(*pdwCounter) += ret;

			(pdwCounter[1])++;  // track calls to RECV
		}
#endif

		return 0;
	}
}

// ------------------------------------------------------------------------
// Returns 0 for success, 1 for error
int TDirectSocket::GetLocalIPAddress(CString & strIPAddress)
{
	struct hostent * pHostEntry = 0;
	char rcName[300];

	try
	{
		if (0 != gethostname (rcName, sizeof(rcName)))
			return 1;

		if ((pHostEntry = gethostbyname (rcName)) == 0)
			return 1;

		char * pDottedAddress = inet_ntoa ( *((LPIN_ADDR)pHostEntry->h_addr) );
		if (NULL == pDottedAddress)
			return 1;
#if defined (_UNICODE)
		TCHAR rcAddr[128];
		MultiByteToWideChar (CP_ACP, 0, pDottedAddress, strlen(pDottedAddress), rcAddr, ELEM(rcAddr));

		strIPAddress = rcAddr;
#else
		strIPAddress = pDottedAddress;
#endif
		return 0;
	}
	catch(...)
	{
		return 1;
	}
}

// ------------------------------------------------------------------------
// Very, very specific function
int  TDirectSocket::AsyncQuit (TErrorList * pErrList)
{
	int    ret = 1;

	if (INVALID_SOCKET == m_DataSocket)
		return 1;

	//  I would like to be nice and send the proper QUIT message,
	//    but I am not adamant about it. so use non-blocking mode

	ULONG mode = 1;

	// switch to non-blocking mode

	int r = ioctlsocket ( m_DataSocket, FIONBIO, &mode );

	if (SOCKET_ERROR == r)
		return 1;

	r = send ( m_DataSocket, "QUIT\r\n", 6, 0);

	if (6 == r)
	{
		ret = 0;

		if (true)
		{
			char  rcLine[1024];
			ZeroMemory ( rcLine, sizeof rcLine );

			// do a non-blocking read to suck up the '205 connection closed

			r = recv ( m_DataSocket, rcLine,  1024, 0 );
			//TRACE1("%s\n", rcLine);
		}
	}

	// back to blocking

	mode = 0;
	ioctlsocket ( m_DataSocket, FIONBIO, &mode );

	return ret;
}

////////////////////////////////////////////////////////

TNewsSocket::TNewsSocket(int port, FARPROC pBlockingHook, HANDLE hEvent,
						 bool * pfProcessJobs)
						 : TDirectSocket(pBlockingHook, hEvent, pfProcessJobs)
{
	m_PortNumber = port;
}

TNewsSocket::~TNewsSocket(void)
{
}

////////////////////////////////////////////////////////
TSMTPSocket::TSMTPSocket(FARPROC pBlockingHook)
: TDirectSocket(pBlockingHook, 0 /* stop event*/, 0 /* pfProcessJobs */)
{
	m_PortNumber = kSMTP_PORT;
}

TSMTPSocket::~TSMTPSocket(void)
{
}

