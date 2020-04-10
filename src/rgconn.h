/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgconn.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:44  richard_wood
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

//
//   ALL SETTINGS ARE NOW in RGSERV.CPP
//

#pragma once

#include "rgbase.h"
#include "tglobdef.h"

class TRegConnection : public TRegistryBase {
public:
   typedef struct {
      int strID;
      TGlobalDef::EGroupSchedule       m_eGrpSched;
   } GRPSYNC;

public:
   TRegConnection();
   ~TRegConnection();
   int Load();
   int Save();

   int  GetGoBackArtcount();
   void SetGoBackArtcount(int count);

   BOOL GetSendKeepAliveMsgs() { return m_fSendKeepAliveMsgs; }
   void SetSendKeepAliveMsgs(BOOL fKeepAlive);

   int  GetKeepAliveMinutes();
   void SetKeepAliveMinutes(int iDelayMinutes);

   BOOL GetAutomaticCycle ();
   void SetAutomaticCycle (BOOL fCycle);

   BOOL GetAutoDisconnect () const { return m_fAutoQuit; }
   void SetAutoDisconnect (BOOL fAutoDisconnect) { m_fAutoQuit = fAutoDisconnect; }

   int  GetAutoDisconnectMinutes () { return m_iAutoQuitMinutes; }
   void SetAutoDisconnectMinutes (int n) { m_iAutoQuitMinutes = n; }

public:
   CString m_newsServer;            // host name or IP address
   LONG    m_newsServerPort;        // port for news server
   BOOL    m_fConnectSecurely;      // Optionally connect via SSL
   CString m_smtpServer;            // host address for SMTP mail
   LONG    m_smtpServerPort;        // port for SMTP server
   CString m_fullName;              // ex: John Q. Public
   CString m_emailAddress;          // email address for user
   CString m_organization;          // user organization, e.g. NASA

   BOOL    m_fConnectAtStartup;     // connect automatically at startup?
   TGlobalDef::EGroupSchedule    m_kGroupSchedule;     // get new groups when connect to server?
   CString m_accountName;           // user name at news server
   CString m_accountPassword;       // password at the news server
   int     m_newsCheckInterval;     // how long between checking groups
   BOOL    m_fDisplayIncomingGroups;// display updates to allgroup list?
protected:
   static GRPSYNC  m_grpMap[];

   void read();
   void write();
   void default_values();
   int     m_iGoBack;               // after subscribe, get 500 articles
   BOOL m_fSendKeepAliveMsgs;
   int  m_iKeepAliveMinutes;
   BOOL m_fAutomaticCycle;
   BOOL m_fAutoQuit;                // automatic disconnect
   int  m_iAutoQuitMinutes;         // the Delay before automatic disconnect
};
