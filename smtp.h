/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: smtp.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:51  richard_wood
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

#include "article.h"

class TSMTPSocket;    // forward reference
class TNewsServer;
class TErrorList;

typedef enum {
               kInitial = 3,
               kOK,
               kMTA_Unavailable,
               kConnectFailure,
               kHelloFailure,
               kFromFailure,
               kRecipFailure,
               kDataFailure,
               kFinalFailure,
               kRecipCCFailure,
               kRecipBCCFailure,
               kWinsockFailure
             } ESmtpError;

class TEmailMsgBind
{
public:
   TEmailMsgBind();
   TEmailMsgBind(TEmailHeader* pHdr, TEmailText* pText);

   ~TEmailMsgBind();
   TEmailMsgBind& operator= (const TEmailMsgBind& rhs);

   void ReleaseOwnership() { m_pHdr = 0; m_pText = 0; }

public:
   TEmailHeader* m_pHdr;
   TEmailText*   m_pText;
   ESmtpError    m_error;
   int           m_ret;
   CString       m_errString;
private:
   TEmailMsgBind( const TEmailMsgBind& src ) {}
};

class TEmailMsgPool {
public:
   TEmailMsgPool(void);
   ~TEmailMsgPool(void);

   void Reset(void);
   void AddMsg(TEmailHeader* pHdr, TEmailText* pText);

public:
   CArray<TEmailMsgBind, TEmailMsgBind&> m_bindArray;

   // event to say "I'm done or Dead"
   HANDLE      m_hEventDone;

   BOOL        m_Completed;
   ESmtpError  m_eError;         // indicates stage or reason
   int         m_serverCode;     // actual error from smtp server
   CString     m_strServerAck;   // error response string from server

   CString     m_smtpServer;     // for connect call
   CString     m_domain;         // for HELO smtp command
};

class TSMTPMailer {
public:
   TSMTPMailer( TErrorList * pErrList, TSMTPSocket* pSoc);
   ~TSMTPMailer(void);

   // returns 0 for success
   int SendOneMessage ( TNewsServer* pServer,
                        TEmailHeader* pHdr, TEmailText* pText,
                        ESmtpError* peError,
                        int* pCode, CString* pErrString,
                        bool & fSent );

   int Hello (const CString& mailhost, int& code, CString& strHELOanswer);
   int EHLO  (const CString& mailhost, int& code, CString& strHELOanswer,
                          CString & strAuthLine);

   int TSMTPMailer::CanSupportLogin (const CString & strAuthline);
   int TSMTPMailer::Login (
      CString & usr,
      CString & pwd,
      CString & strAuthLine,
      int & code,
      CString & ack)         ;

protected:
   void VoidThisTransaction(void);
   int  GetAnswer (int& code, CString& response, CString * pstrAuthLine = 0);

   void make_clean_email_addr(const CString& emailAddr, CString& rClean);
   void make_bracket_addr(const CString& addr, CString& rBracket);
   void write_file(CFile* pFile);

protected:
   TSMTPSocket* m_pSoc;
   char buf[512];

   TErrorList * m_pErrList;
};

// Worker thread function
UINT SMTP_Mailer(LPVOID pVoid);
