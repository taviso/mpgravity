/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgserv.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:45  richard_wood
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

#include "rgbase.h"
#include "tglobdef.h"
#include "appversn.h"      // different Configurations

class TRegServer : public TRegistryBase {
public:
   typedef struct {
      int strID;
      TGlobalDef::EGroupSchedule       m_eGrpSched;
   } GRPSYNC;

private:
   enum ERegServerFlags { kPostOK    = 0x80000000,
                          kXOverXRef = 0x40000000 };

public:
   TRegServer();
   ~TRegServer();
   int Load(LPCTSTR server);
   int Save(LPCTSTR server);

   const CString& GetNewsServerName () {return m_newsServerName;}

   void SetNewsServerName (LPCTSTR serverName){m_newsServerName = serverName;}

   const CString & GetServerDatabasePath () {return m_serverDatabasePath;}
   void SetServerDatabasePath (LPCTSTR path) {
      m_serverDatabasePath = path;
      }

   const CString& GetNewsServerAddress () {
      if (m_newsServerAddr.IsEmpty())
         return GetNewsServerName ();
      else
         return m_newsServerAddr;
      }

   void SetNewsServerAddress (const CString & addr) { m_newsServerAddr = addr; }

   LONG GetNewsServerPort () {return m_newsServerPort;}
   void SetNewsServerPort (LONG port) {m_newsServerPort = port;}

   const CString & GetSmtpServer () {return m_smtpServer;}
   void  SetSmtpServer (LPCTSTR smtpServer) {m_smtpServer = smtpServer;}

   LONG GetSmtpServerPort () {return m_smtpServerPort;}
   void SetSmtpServerPort (LONG port) {m_smtpServerPort = port;}

   const CString & GetFullName () {return m_fullName;}
   void SetFullName (LPCTSTR fullName) {m_fullName = fullName;}

   const CString & GetEmailAddress () {return m_emailAddress;}
   void SetEmailAddress (LPCTSTR address) {m_emailAddress = address;}

   const CString & GetEmailAddressForPost () {return m_emailAddressForPost;}
   void SetEmailAddressForPost (LPCTSTR address) {m_emailAddressForPost = address;}

   const CString&  GetReplyTo () { return m_replyTo; }
   void            SetReplyTo (LPCTSTR rplyto) { m_replyTo = rplyto; }

   const CString & GetOrganization () {return m_organization;}
   void SetOrganization (LPCTSTR org) {m_organization = org;}

   BOOL  GetConnectAtStartup () {return m_fConnectAtStartup;}
   void  SetConnectAtStartup (BOOL fConnect) {m_fConnectAtStartup = fConnect;}

   TGlobalDef::EGroupSchedule GetGroupSchedule () {return m_kGroupSchedule;}
   void SetGroupSchedule (TGlobalDef::EGroupSchedule kSchedule) {
      m_kGroupSchedule = kSchedule;
      }

   const CString & GetAccountName () {return m_accountName;}
   void SetAccountName (LPCTSTR name) {m_accountName = name;}

   const CString & GetAccountPassword () {return m_accountPassword;}
   void SetAccountPassword (LPCTSTR pwd) {m_accountPassword = pwd;}

   int GetNewsCheckInterval () {return m_newsCheckInterval;}
   void SetNewsCheckInterval (int interval) {m_newsCheckInterval = interval;}

   BOOL GetDisplayIncomingGroups () {return m_fDisplayIncomingGroups;}
   void SetDisplayIncomingGroups (BOOL fDisplay) {
      m_fDisplayIncomingGroups = fDisplay;
      }

   int  GetGoBackArtcount();
   void SetGoBackArtcount(int count);

   BOOL GetSendKeepAliveMsgs() { return m_fSendKeepAliveMsgs; }
   void SetSendKeepAliveMsgs(BOOL fKeepAlive);

   int  GetKeepAliveMinutes();
   void SetKeepAliveMinutes(int iDelayMinutes);

   BOOL GetAutomaticCycle ();
   void SetAutomaticCycle (BOOL fCycle);

   LONG NextPostID ();
   LONG NextMailID ();

   void GetNewsGroupCheckTime(CTime& tm);
   void SetNewsGroupCheckTime(const CTime& tm);

   const CString & GetConnectionName () {return m_connectionName;}
   void  SetConnectionName (LPCTSTR name) {m_connectionName = name;}

   BOOL  GetForceConnection () {return m_fForceConnection;}
   void  SetForceConnection (BOOL fForce) {m_fForceConnection = fForce;}

   BOOL  GetUseExistingConnection () {return m_fUseExistingConnection;}
   void  SetUseExistingConnection (BOOL fUse) {m_fUseExistingConnection = fUse;}

   BOOL  GetPromptBeforeConnecting () {return m_fPromptBeforeConnecting;}
   void  SetPromptBeforeConnecting (BOOL fPrompt) {
      m_fPromptBeforeConnecting = fPrompt;
      }

   BOOL  GetPromptBeforeDisconnecting () {return m_fPromptBeforeDisconnecting;}
   void  SetPromptBeforeDisconnecting (BOOL fPrompt) {
      m_fPromptBeforeDisconnecting = fPrompt;
      }

   BOOL  GetDisconnectIfWeOpened () {
      return m_fDisconnectIfWeOpened;
      }

   void  SetDisconnectIfWeOpened (BOOL fDisconnect) {
      m_fDisconnectIfWeOpened = fDisconnect;
      }

   BOOL  GetForceDisconnect () {
      return m_fForceDisconnect;
      }

   void  SetForceDisconnect (BOOL fForce) {
      m_fForceDisconnect = fForce;
      }

   BOOL  GetPostingAllowed ();
   void  SetPostingAllowed (BOOL fPostAllowed);

   // stuff for disconnect automatically
   BOOL GetAutoDisconnect () const { return m_fAutoQuit; }
   void SetAutoDisconnect (BOOL fAutoDisconnect) { m_fAutoQuit = fAutoDisconnect; }

   int  GetAutoDisconnectMinutes () { return m_iAutoQuitMinutes; }
   void SetAutoDisconnectMinutes (int n) { m_iAutoQuitMinutes = n; }

   BOOL GetAutoRetrieveTagged () { return m_fAutoRetrieveTagged; }
   void SetAutoRetrieveTagged (BOOL fFetch) { m_fAutoRetrieveTagged = fFetch; }

   BOOL GetFullCrossPost () { return m_fFullCrossPost; }
   void SetFullCrossPost (BOOL fFull) { m_fFullCrossPost = fFull; }

   // if True, we check for expired articles and eliminate them
   bool GetExpirePhase () { return m_fExpirePhase ? true : false; }
   void SetExpirePhase (BOOL fExpire) { m_fExpirePhase = fExpire; }

   BOOL GetSubscribeDL () { return m_fSubscribeDL; }
   void SetSubscribeDL (BOOL fDL) { m_fSubscribeDL = fDL; }

   // 0 = manual, 1 = once per session, 2 = upon connecting
   int  GetUpdateGroupCount () { return m_iUpdateGroupCount; }
   void SetUpdateGroupCount (int i) { m_iUpdateGroupCount = i; }

   // newsrc settings
   BOOL GetImport () { return m_fImport; }
   void SetImport (BOOL fNew) { m_fImport = fNew; }
   BOOL GetExport () { return m_fExport; }
   void SetExport (BOOL fNew) { m_fExport = fNew; }
   BOOL GetExportSubscribedOnly () { return m_fExportSubscribedOnly; }
   void SetExportSubscribedOnly (BOOL fNew) { m_fExportSubscribedOnly = fNew; }
   const CString &GetImportFile () { return m_strImportFile; }
   void SetImportFile (LPCTSTR file) { m_strImportFile = file; }
   const CString &GetExportFile () { return m_strExportFile; }
   void SetExportFile (LPCTSTR file) { m_strExportFile = file; }

   void SetLastPropPage (int iIndex) {
      m_iPropPage = iIndex;
      }

   int GetLastPropPage () {
      return m_iPropPage;
      }

   // proxicom extension
   void GetIconModTime (CString* pStr) { *pStr = m_strIconModTime; }
   void SetIconModTime (const CString& str) { m_strIconModTime = str; }

   // 0 = unknown, 1 = Proxicom Web / NNTP Server
   int  ProxicomWebServer () { return m_iProxicomServer; }
   void SetProxicomWebServer (int n) { m_iProxicomServer = n; }

   bool XOverXRef ();
   void XOverXRef (bool fXRef);

   void SetLogonStyle (int iStyle) { m_iLogonStyle = iStyle; }
   int GetLogonStyle () { return m_iLogonStyle; }

   // limit headers for download?
   bool GetLimitHeaders()           { return m_fLimitHeaders ? true : false;  }
   void SetLimitHeaders(bool limit) { m_fLimitHeaders = limit; }

   // the max number of headers we will get
   LONG HeadersLimit ()       { return m_lHeadersLimit; }
   void HeadersLimit (LONG n) { m_lHeadersLimit = n; }

   // some German servers reject postings that have own msg-id
   BOOL GetGenerateOwnMsgID ()          { return m_fGenerateMsgID; }
   void SetGenerateOwnMsgID (BOOL gen)  { m_fGenerateMsgID = gen; }

   // multiple servers servicing one DNS address, creates msgs out of order
   BOOL GetNewsServerFarm   ()          { return m_fServerFarm; }
   void SetNewsServerFarm   (BOOL f)    { m_fServerFarm = f; }

   BOOL GetPurgeOutbox      ()          { return m_fPurgeOutbox; }
   void SetPurgeOutbox      (BOOL p)    { m_fPurgeOutbox = p; }

   int  GetPurgeOutboxDays  ()          { return m_iPurgeOutboxDays; }
   void SetPurgeOutboxDays  (int d)     { m_iPurgeOutboxDays = d; }

   BOOL GetRetrying()            { return m_fRetrying; }
   void SetRetrying(BOOL f)      { m_fRetrying = f; }

   int  GetRetryCount()          { return m_iRetryCount; }
   void SetRetryCount(int r)     { m_iRetryCount = r; }

   BOOL GetPausing()             { return m_fPausing; }
   void SetPausing(BOOL f)       { m_fPausing = f; }

   int  GetPauseCount()          { return m_iPauseCount; }
   void SetPauseCount(int p)     { m_iPauseCount = p; }

   BOOL GetModeReader ()         { return m_fModeReader; }
   void SetModeReader (BOOL f)   { m_fModeReader = f; }

   int  GetSmtpLogonStyle()      { return m_iSmtpLogonStyle; }
   void SetSmtpLogonStyle(int s) { m_iSmtpLogonStyle = s; }

   const CString & GetSmtpLogonUser() { return m_strSmtpLogonUser; }
   void  SetSmtpLogonUser(const CString & s) { m_strSmtpLogonUser = s; }

   const CString & GetSmtpLogonPass() { return m_strSmtpLogonPwd;  }
   void  SetSmtpLogonPass(const CString & s) { m_strSmtpLogonPwd = s; }

protected:
   CString m_newsServerName;        // friendly name for news server
   CString m_serverDatabasePath;    // path to the server database files
   CString m_newsServerAddr;        // address for NNTP server
   LONG    m_newsServerPort;        // port for news server
   CString m_smtpServer;            // host address for SMTP mail
   LONG    m_smtpServerPort;        // port for SMTP server
   CString m_fullName;              // ex: John Q. Public
   CString m_emailAddress;          // email address for user
   CString m_emailAddressForPost;   // email address when sending e-mail
   CString m_replyTo;
   CString m_organization;          // user organization, e.g. NASA

   BOOL    m_fConnectAtStartup;     // connect automatically at startup?
   TGlobalDef::EGroupSchedule    m_kGroupSchedule;     // get new groups when connect to server?
   CString m_accountName;           // user name at news server
   CString m_accountPassword;       // password at the news server
   CString m_passwordBuffer;        //  encryption buffer
   CString m_passwordBuffer2;       //  encryption buffer
   int     m_newsCheckInterval;     // how long between checking groups
   BOOL    m_fDisplayIncomingGroups;// display updates to allgroup list?
   DWORD   m_dwServerFlags;         // High bit indicates Posting Allowed
   BOOL    m_fPurgeOutbox;          // purge outbox entries?
   int     m_iPurgeOutboxDays;      // days before purging outbox entries

   int     m_iSmtpLogonStyle;
   CString m_strSmtpLogonUser;
   CString m_strSmtpLogonPwd;

protected:
   static GRPSYNC  m_grpMap[];

   void read();
   void write();
   void default_values();
   void upgrade();
   int  make_cipher_text (const CString & strPlain, CString& strCipher);
   int  make_plain_text (const CString & strCipher, CString& strPlain);

   int  m_iGoBack;                  // after subscribe, get 500 articles
   BOOL m_fSendKeepAliveMsgs;
   int  m_iKeepAliveMinutes;
   BOOL m_fAutomaticCycle;

   // start program state
   BOOL m_fGroupListExist;
   BOOL m_fMemoryExist;
   CTime m_NewsGroup_Check;

   // stuff for disconnect automatically
   BOOL m_fAutoQuit;                // automatic disconnect
   int  m_iAutoQuitMinutes;         // the Delay before automatic disconnect

   BOOL m_fAutoRetrieveTagged;      // get tagged articles after connecting
   int  m_iUpdateGroupCount;        // update counts in NewsView pane

   BOOL m_fFullCrossPost;           // retrieve crosspost info

   // dialup networking parameters

   CString  m_connectionName;          // which connection to use
   BOOL     m_fForceConnection;        // whether to force a connection
   BOOL     m_fUseExistingConnection;   // search list of current and use it
   BOOL     m_fPromptBeforeConnecting; // prompt before starting connection
   BOOL     m_fPromptBeforeDisconnecting; // prompt before disconnecting
   BOOL     m_fDisconnectIfWeOpened;      // disconnect only if we opened
   BOOL     m_fForceDisconnect;           // force disconnection

   BOOL    m_fImport;               // import newsrc on startup
   BOOL    m_fExport;               // export newsrc on shutdown
   BOOL    m_fExportSubscribedOnly; // export subscribed groups only
   CString m_strImportFile;         // export file name
   CString m_strExportFile;         // export file name

   int     m_iPropPage;             // last page accessed in prop sheet
   CString m_strIconModTime;        // Proxicom - last time icons were changed
   int     m_iProxicomServer;       // 0=unknown, 1=Yes, 2=definitely NO

   BOOL     m_fExpirePhase;         // true if we eliminate deadwood
   BOOL     m_fSubscribeDL;         // automatically DL hdrs after subscribe
   int      m_iLogonStyle;          // plaintext Password or SPA or nil

   BOOL    m_fLimitHeaders;         // limit header downloads to [N]
   LONG    m_lHeadersLimit;         //   number

   BOOL    m_fGenerateMsgID;        // generate msg-id when posting article?
   BOOL    m_fServerFarm;

   BOOL    m_fPausing;              // control 'relentless mode
   BOOL    m_fRetrying;             //
   int     m_iPauseCount;           //
   int     m_iRetryCount;           //

   BOOL    m_fModeReader;           // send 'mode reader' line
};
