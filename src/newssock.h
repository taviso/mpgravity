/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newssock.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:35  richard_wood
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

#include "mpsokerr.h"
#include "superstl.h"
#include "sockssl.h"

class TMemorySpot;
class TSockTrack;
class TErrorList;

typedef void (*P_LINEPROGRESSFUNC)(int linecount, DWORD dwCookie);

class TDirectSocket {
public:
   static BOOL TDirectSocket::InitInstance ();

public:
   ~TDirectSocket();

   int    Connect(TErrorList * pErrList, LPCTSTR hostAddress);

   int    ReadNum (TErrorList * pErrList, int & wrd);
   int    ReadLine(TErrorList * pErrList, CString & str);

   // read until <PERIOD><CRLF>
   int    ReadAnswer (TErrorList * pErrList,
                      TMemorySpot* pSpot,
                      bool fHandleDoublePeriods,
                      P_LINEPROGRESSFUNC pCB = NULL,
                      DWORD dwMagicCookie = 0);

   int    Write (TErrorList * pErrList, LPCSTR str, int bytes);

   //  appends CRLF to data
   int    WriteLine (TErrorList * pErrList, LPCSTR buf, int bytes);

	void   CancelBlockingCall(void);

   virtual void   SetPort (int port) {m_PortNumber = port;}
   virtual int    GetPort () {return m_PortNumber;}

   virtual void SetSecure (bool fEnable) { m_bSecure = fEnable; }
   virtual bool GetSecure () { return m_bSecure; }

   int    GetLocalIPAddress(CString & strIPAddress);

   // returns true if there was any communication failure
   bool   TransmitError () { return m_bSendRecvError; }

   int  AsyncQuit (TErrorList * pErrList);

   const CString & getDottedAddr ()  { return m_strDebugDottedAddr; }

protected:
   // protected constructor
   TDirectSocket(FARPROC pBlockingHook, HANDLE hStopEvent, bool * pfProcessJobs);

   int  sub_connect (LPCSTR hostAddress, char* pbuf, int buflen);

   int      m_PortNumber;
   WSADATA  wsaData;
   struct   sockaddr_in m_saSocketAddr;
   SOCKET   m_DataSocket;
   vector<TCHAR> m_vDataBuffer;
	vector<TCHAR>::iterator it;
   BOOL     m_bAborted;
   BOOL     m_bSecure;

   PSEC_LAYER m_SecurityLayer;

   // if we encounter a send or recv error, then we shouldn't send QUIT
   bool     m_bSendRecvError;
   int      m_iLastStatus;

   FARPROC  m_pBlockingHook;
   HANDLE   m_hStopEvent;
   bool  *  m_pfProcessJobs;

   static BYTE m_fRegisterClass;
   TSockTrack* m_pTrack;

   CString  m_strDebugDottedAddr;

private:
   int  recv_error(TErrorList *pErrList, int iBytesRead, DWORD dwLastError);
   int  send_error(TErrorList *pErrList, DWORD dwLastError);
   int  fill_buffer_EL();   // requires that m_psErrList be set
   void free_socket ();

private:
   int  myReadChar_EL ();

   TErrorList * m_psErrList;
   char        m_c;
};

/////////////////////////////////////////////////////////////////////////////
// TNewsSocket - 1 channel to a NNTP news server.
//
/////////////////////////////////////////////////////////////////////////////
class TNewsSocket : public TDirectSocket {
public:
   TNewsSocket(int port, FARPROC pBlockingHook, HANDLE hStopEvent,
                  bool * pfProcessJobs, bool bSecure);

   ~TNewsSocket(void);

protected:
   // 100% of functionality from the TBufferedSocket

   bool * m_pfProcessJobs;
};

/////////////////////////////////////////////////////////////////////////////
// TSMTPSocket - 1 channel to a SMTP server.
//
/////////////////////////////////////////////////////////////////////////////
class TSMTPSocket : public TDirectSocket {
public:
   TSMTPSocket(FARPROC pBlockingHook = NULL);
   ~TSMTPSocket(void);

protected:
   // 100% of functionality from the TBufferedSocket
};

/**************************************************************************
I could have an hierarchy
    TBigSocket
        TNewsSocket inherits from BigSocket
        TSmtpSocket inherits from BigSocket

**************************************************************************/
