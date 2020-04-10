/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: servopts.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:50  richard_wood
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

// servopts.cpp
//
#include "stdafx.h"
#include "News.h"

#include "globals.h"

// Dialog boxes
#include "tconnpg.h"
#include "toperpg.h"             // TOperationsPage
#include "tglobopt.h"
#include "newsgrp.h"
#include "tsetpg.h"
#include "tserverspage.h"           // server setup PropertyPage

#include "optshet.h"
#include "custmsg.h"

#include "tidarray.h"            // SetMessageIDsSaved()

#include "dialuppg.h"
#include "dialman.h"
#include "rgmgr.h"
#include "servchng.h"
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "newsdb.h"              // TNewsDB
#include "genutil.h"             // MsgResource(), ...
#include "newsrcpg.h"            // TOptionsNewsrcDlg
#include "newsrc.h"              // GetDefaultNewsrc()
#include "servpost.h"            // server posting stuff...
#include "srvoptg.h"             // group options
#include "newsview.h"
#include "hints.h"               //  VIEWHINT_EMPTY

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


extern BOOL  gfShutdownRename;

extern TServerChangeDlg::RenameChoice HandleServerNameChange (LPCTSTR oldName, LPCTSTR newName);

/////////////////////////////////////////////////////////////////////////////
// Server Related Options
/////////////////////////////////////////////////////////////////////////////

void CNewsApp::OnServerOptions()
{
   TServerCountedPtr cpNewsServer;

   CString              temp, captionString;
   temp.LoadString(IDS_CAPTION_OPTIONS);

   captionString.Format(temp, cpNewsServer->GetNewsServerName());

   TOptionsSheet        dlgOptions(captionString);
   TConnectionPage      pgConnection;

   TSetupPage           pgHuman;
   TServersPage         pgSetup;
   TDialupPage          pgDialup;
   TOperationPage       pgOperate;
   TServerPostPage      pgPosting;
   TOptionsNewsrcDlg    pgNewsrc;
   TServerGroups        pgGroups;

   extern TDialupManager dialMgr;

   BOOL fPromptForShutdown = FALSE;

   // human
   pgHuman.m_fullname       = cpNewsServer->GetFullName ();
   pgHuman.m_emailAddress   = cpNewsServer->GetEmailAddress ();
   pgHuman.m_organization   = cpNewsServer->GetOrganization ();
   pgHuman.m_strMailOverride= cpNewsServer->GetEmailAddressForPost ();

   // set up the ddx stuff with the global stuff we just read
   pgSetup.m_newsServer     = cpNewsServer->GetNewsServerName ();
   pgSetup.m_strNNTPAddress = cpNewsServer->GetNewsServerAddress ();
   pgSetup.m_iLogonStyle_NTP    = cpNewsServer->GetLogonStyle ();
   pgSetup.m_authName_NTP       = cpNewsServer->GetAccountName ();
   pgSetup.m_authPass_NTP       = cpNewsServer->GetAccountPassword ();

   pgSetup.m_smtpServer       = cpNewsServer->GetSmtpServer ();
   pgSetup.m_iLogonStyle_STP  = cpNewsServer->GetSmtpLogonStyle();
   pgSetup.m_authName_STP     = cpNewsServer->GetSmtpLogonUser();
   pgSetup.m_authPass_STP     = cpNewsServer->GetSmtpLogonPass();

   pgSetup.m_iPortSMTP      = cpNewsServer->GetSmtpServerPort ();
   pgSetup.m_iPortNNTP      = cpNewsServer->GetNewsServerPort ();

   pgSetup.m_fConnectSecurely = cpNewsServer->GetConnectSecurely();

   // connections page
   pgConnection.m_fConnectAtStartup = cpNewsServer->GetConnectAtStartup();
   pgConnection.m_fKeepAlive = cpNewsServer->GetSendKeepAliveMsgs();
   pgConnection.m_iKeepAliveMinutes = cpNewsServer->GetKeepAliveMinutes();
   pgConnection.m_fAutoQuit = cpNewsServer->GetAutoDisconnect();
   pgConnection.m_iAutoQuitMinutes = cpNewsServer->GetAutoDisconnectMinutes();
   pgConnection.m_fModeReader      = cpNewsServer->GetModeReader();

   pgConnection.m_fRetry      = cpNewsServer->GetRetrying();
   pgConnection.m_iRetryCount = cpNewsServer->GetRetryCount();
   pgConnection.m_fRetryPause = cpNewsServer->GetPausing();
   pgConnection.m_iPauseCount = cpNewsServer->GetPauseCount();

   // group page
   pgGroups.m_fNewgroupsOnConnect = (cpNewsServer->GetGroupSchedule ()
                                         == TGlobalDef::kGroupsUponConnecting);
   pgGroups.m_fDisplayNewGroups = cpNewsServer->GetDisplayIncomingGroups ();
   pgGroups.m_iUpdateServerCount = cpNewsServer->GetUpdateGroupCount();

   // operations page
   pgOperate.m_fAutoCycle = cpNewsServer->GetAutomaticCycle();
   pgOperate.m_iCycleMinutes = cpNewsServer->GetNewsCheckInterval();
   pgOperate.m_fFullCrossPost = cpNewsServer->GetFullCrossPost();
   pgOperate.m_fAutoGetTags = cpNewsServer->GetAutoRetrieveTagged();
   pgOperate.m_fVerifyLocal = cpNewsServer->GetExpirePhase ();
   pgOperate.m_fSubscribeDownload = cpNewsServer->GetSubscribeDL ();
   pgOperate.m_fLimitHeaders = cpNewsServer->GetLimitHeaders ();
   pgOperate.m_iHeaderLimit = cpNewsServer->HeadersLimit ();
   pgOperate.m_fNewsFarm    = cpNewsServer->GetNewsServerFarm ();

   // dialup page
   pgDialup.m_connectionName = cpNewsServer->GetConnectionName ();
   pgDialup.m_fForceConnection = cpNewsServer->GetForceConnection();
   // ???? pgDialup.m_dialupUserName = cpNewsServer->;
   pgDialup.m_fUseExistingConnection = cpNewsServer->GetUseExistingConnection();
   pgDialup.m_fPromptBeforeConnecting = cpNewsServer->GetPromptBeforeConnecting();
   pgDialup.m_fPromptBeforeDisconnecting = cpNewsServer->GetPromptBeforeDisconnecting();
   // ???? pgDialup.m_dialupPassword = cpNewsServer->;
   pgDialup.m_fDisconnectIfWeOpened = cpNewsServer->GetDisconnectIfWeOpened ();
   pgDialup.m_fForceDisconnect = cpNewsServer->GetForceDisconnect();

   pgNewsrc.m_bImport = cpNewsServer -> GetImport ();
   pgNewsrc.m_strImportFile = cpNewsServer -> GetImportFile ();
   pgNewsrc.m_bExport = cpNewsServer -> GetExport ();
   pgNewsrc.m_strExportFile = cpNewsServer -> GetExportFile ();
   pgNewsrc.m_bSubscribedOnly = cpNewsServer -> GetExportSubscribedOnly ();
   if (pgNewsrc.m_strImportFile.IsEmpty ())
      pgNewsrc.m_strImportFile = GetDefaultNewsrc ();
   if (pgNewsrc.m_strExportFile.IsEmpty ())
      pgNewsrc.m_strExportFile = GetDefaultNewsrc ();

   pgPosting.m_replyTo      = cpNewsServer->GetReplyTo();
   pgPosting.m_fGenMsgID    = cpNewsServer->GetGenerateOwnMsgID();

   //////////////////////////////////////
   // Add all pages to the property sheet

   dlgOptions.AddPage ( &pgHuman );          // human
   dlgOptions.AddPage ( &pgSetup );          // servers
   dlgOptions.AddPage ( &pgConnection );     // Connect

   // ??? - need to check for version of NT, if below 4.0
   //       then disable
   if (dialMgr.IsRasInstalled())
      dlgOptions.AddPage ( &pgDialup);          // Dial-up Networking

   dlgOptions.AddPage ( &pgGroups );         // Group options
   dlgOptions.AddPage ( &pgNewsrc );         // Newsrc
   dlgOptions.AddPage ( &pgOperate );        // Operation

   dlgOptions.AddPage (&pgPosting);          // posting options

   dlgOptions.m_iMRUPage = cpNewsServer->GetLastPropPage();
   dlgOptions.SetActivePage ( dlgOptions.m_iMRUPage );

   if (IDOK == dlgOptions.DoModal())
      {
      // copy the stuff back into our structure
      cpNewsServer->SetLastPropPage(dlgOptions.m_iMRUPage);

      if (pgSetup.m_newsServer.CompareNoCase(cpNewsServer->GetNewsServerName()))
         {
         switch (HandleServerNameChange (cpNewsServer->GetNewsServerName(),
                                 pgSetup.m_newsServer))
            {
            case TServerChangeDlg::kJustRename:
               cpNewsServer->SetServerRename (pgSetup.m_newsServer, FALSE);
               fPromptForShutdown = TRUE;
               break;
            case TServerChangeDlg::kRemoveOld:
               cpNewsServer->SetServerRename (pgSetup.m_newsServer, TRUE);
               fPromptForShutdown = TRUE;
               break;
            case TServerChangeDlg::kDoNothing:
               break;
            }
         }

      cpNewsServer->SetFullName (pgHuman.m_fullname);
      cpNewsServer->SetOrganization (pgHuman.m_organization);
      cpNewsServer->SetEmailAddress (pgHuman.m_emailAddress);
      cpNewsServer->SetEmailAddressForPost (pgHuman.m_strMailOverride);

      cpNewsServer->SetNewsServerAddress (pgSetup.m_strNNTPAddress);
      cpNewsServer->SetLogonStyle (pgSetup.m_iLogonStyle_NTP);
      cpNewsServer->SetAccountName (pgSetup.m_authName_NTP);
      cpNewsServer->SetAccountPassword (pgSetup.m_authPass_NTP);

      cpNewsServer->SetSmtpServer (pgSetup.m_smtpServer);
      cpNewsServer->SetSmtpLogonStyle (pgSetup.m_iLogonStyle_STP);
      cpNewsServer->SetSmtpLogonUser  (pgSetup.m_authName_STP);
      cpNewsServer->SetSmtpLogonPass  (pgSetup.m_authPass_STP);

      cpNewsServer->SetSmtpServerPort (pgSetup.m_iPortSMTP);
      cpNewsServer->SetNewsServerPort (pgSetup.m_iPortNNTP);

      cpNewsServer->SetConnectSecurely(pgSetup.m_fConnectSecurely);

      cpNewsServer->SetConnectAtStartup (pgConnection.m_fConnectAtStartup);
      cpNewsServer->SetSendKeepAliveMsgs(pgConnection.m_fKeepAlive);
      cpNewsServer->SetKeepAliveMinutes (pgConnection.m_iKeepAliveMinutes);
      cpNewsServer->SetAutoDisconnect (pgConnection.m_fAutoQuit);
      cpNewsServer->SetAutoDisconnectMinutes (pgConnection.m_iAutoQuitMinutes);
      cpNewsServer->SetModeReader (pgConnection.m_fModeReader);

      cpNewsServer->SetRetrying  ( pgConnection.m_fRetry      );
      cpNewsServer->SetRetryCount( pgConnection.m_iRetryCount );
      cpNewsServer->SetPausing   ( pgConnection.m_fRetryPause );
      cpNewsServer->SetPauseCount( pgConnection.m_iPauseCount );

      // Groups
      if (pgGroups.m_fNewgroupsOnConnect)
         cpNewsServer->SetGroupSchedule (TGlobalDef::kGroupsUponConnecting);
      else
         cpNewsServer->SetGroupSchedule (TGlobalDef::kGroupsOnDemand);

      cpNewsServer->SetDisplayIncomingGroups (pgGroups.m_fDisplayNewGroups);
      cpNewsServer->SetUpdateGroupCount (pgGroups.m_iUpdateServerCount);

      // Operations
      cpNewsServer->SetAutomaticCycle(pgOperate.m_fAutoCycle);
      cpNewsServer->SetNewsCheckInterval (pgOperate.m_iCycleMinutes);
      cpNewsServer->SetFullCrossPost(pgOperate.m_fFullCrossPost);
      cpNewsServer->SetAutoRetrieveTagged (pgOperate.m_fAutoGetTags);
      cpNewsServer->SetExpirePhase (pgOperate.m_fVerifyLocal);
      cpNewsServer->SetSubscribeDL (pgOperate.m_fSubscribeDownload);
      cpNewsServer->SetLimitHeaders (pgOperate.m_fLimitHeaders ? true : false);
      cpNewsServer->HeadersLimit (pgOperate.m_iHeaderLimit);
      cpNewsServer->SetNewsServerFarm (pgOperate.m_fNewsFarm);

      if (dialMgr.IsRasInstalled())
         {
         cpNewsServer->SetConnectionName (pgDialup.m_connectionName);
         cpNewsServer->SetForceConnection(pgDialup.m_fForceConnection);
         // ???? pgDialup.m_dialupUserName = cpNewsServer->;
         cpNewsServer->SetUseExistingConnection(pgDialup.m_fUseExistingConnection);
         cpNewsServer->SetPromptBeforeConnecting(pgDialup.m_fPromptBeforeConnecting);
         cpNewsServer->SetPromptBeforeDisconnecting(pgDialup.m_fPromptBeforeDisconnecting);
         // ???? pgDialup.m_dialupPassword = cpNewsServer->;
         cpNewsServer->SetDisconnectIfWeOpened (pgDialup.m_fDisconnectIfWeOpened);
         cpNewsServer->SetForceDisconnect(pgDialup.m_fForceDisconnect);
         }

      cpNewsServer -> SetImport (pgNewsrc.m_bImport);
      cpNewsServer -> SetImportFile (pgNewsrc.m_strImportFile);
      cpNewsServer -> SetExport (pgNewsrc.m_bExport);
      cpNewsServer -> SetExportFile (pgNewsrc.m_strExportFile);
      cpNewsServer -> SetExportSubscribedOnly (pgNewsrc.m_bSubscribedOnly);

      cpNewsServer->SetReplyTo( pgPosting.m_replyTo );
      cpNewsServer->SetGenerateOwnMsgID( pgPosting.m_fGenMsgID);

      gpStore->SaveGlobalOptions();
      cpNewsServer->SaveSettings();

      if (fPromptForShutdown)
         {
         if (GetNewsView())
            {
            GetNewsView()->GetDocument()->UpdateAllViews(NULL, VIEWHINT_EMPTY);
            GetNewsView()->CloseCurrentNewsgroup();
            }
         gptrApp->ExitServer ();
         gpStore->CheckServerRename();
         gptrApp->SwitchServer (cpNewsServer->GetNewsServerName());
         }
      }
}

