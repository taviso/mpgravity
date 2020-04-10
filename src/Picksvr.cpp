/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: Picksvr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.7  2010/04/20 21:04:55  richard_wood
/*  Updated splash screen component so it works properly.
/*  Fixed crash from new splash screen.
/*  Updated setver setup dialogs, changed into a Wizard with a lot more verification and "user helpfulness".
/*
/*  Revision 1.6  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.5  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.4  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.3  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:07  richard_wood
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

/////////////////////////////////////////////////////////////////////////////
// picksvr.cpp - routines for picking news servers...
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <io.h>
#include <stdlib.h>

#include "news.h"

#include "mainfrm.h"
#include "Newsdoc.h"
#include "Newsview.h"
#include "mdichild.h"
#include "tnews3md.h"

#include "confstr.h"

#include "globals.h"

#include "tasker.h"
#include "tmutex.h"

#include "posttmpl.h"               // TPostTemplate

#include "tvwdoc.h"
#include "tvwtmpl.h"

#include "artview.h"

#include "tpopfrm.h"
#include "tpoptmpl.h"

#include "custmsg.h"
// Dialog boxes
#include "tdlgtmpv.h"               //   a property page
//#include "tconnpg.h"
#include "tmailpg.h"
#include "tsigpg.h"
#include "twastepg.h"
#include "tglobopt.h"
#include "tpostpg.h"
#include "replypg.h"
#include "decodpg.h"
#include "encodpg.h"

#include "fileutil.h"

#include "tidarray.h"            // RetrieveMessageIDs(), ...

#include "log.h"
#include <winreg.h>
#include "regutil.h"
#include "tdbdlg.h"
#include "direct.h"
#include "servpick.h"
#include "rgsys.h"
#include "rgurl.h"
#include "rgui.h"
#include "newsdb.h"           // gpStore
#include "tsetpg.h"           // human setup PropertyPage
#include "tserverspage.h"     // server setup PropertyPage
#include "picksvr.h"
#include "servcp.h"

#include "vcrrun.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GetServerProfile - Get the values corresponding to a single news server
//                    from the user.
/////////////////////////////////////////////////////////////////////////////
BOOL GetServerProfile(TSetupPage &pgSetupHuman, TServersPage &pgSetupServers, bool &bImport)
{
	bImport = false;
	// Get the defaults
	T_MSFT_DEFAULTS  sMSFT;

	GetDefaultConnectionSettings ( sMSFT );

	pgSetupHuman.m_fullname = sMSFT.strFullName;
	pgSetupHuman.m_organization = sMSFT.strOrganization ;

	while (TRUE)
	{
		CPropertySheet sSheet(IDS_GRAVITY_SERVSETUP);

		sSheet.AddPage ( &pgSetupHuman );
		sSheet.AddPage ( &pgSetupServers );

		sSheet.SetWizardMode();

		int rc = sSheet.DoModal();
		if (IDCANCEL == rc)
			return FALSE;
		else if (IDC_BUTTON_IMPORT == rc)
		{
			bImport = true;
			return TRUE;
		}

		if (pgSetupServers.m_newsServer.IsEmpty())
			AfxMessageBox(IDS_SERVER_BLANK);
		else
			return TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CheckConnectionSettings - Check to see if a news server has been set up.
//                           If not, put up a dialog to get the configuration
//                           information.
/////////////////////////////////////////////////////////////////////////////
BOOL CheckConnectionSettings(TNewsServer ** pServer)
{
#ifdef OLD_FUNCTION
	// NOTE: this is an old function. If you use it, add handling for
	// the email address override stuff

	TSetupPage dlgSetupServer;

	// 1) if no news server exists, pull stuff from the registry,

	if (*pServer)
	{
		LogString ("CheckConnectionSettings 1");
		dlgSetupServer.m_newsServer     = (*pServer)->GetNewsServerName ();
		dlgSetupServer.m_emailAddress   = (*pServer)->GetEmailAddress ();
		dlgSetupServer.m_fullname       = (*pServer)->GetFullName ();
		dlgSetupServer.m_smtpServer     = (*pServer)->GetSmtpServer ();
		dlgSetupServer.m_organization   = (*pServer)->GetOrganization ();
		dlgSetupServer.m_iLogonStyle    = (*pServer)->GetLogonStyle ();
		dlgSetupServer.m_authName       = (*pServer)->GetAccountName ();
		dlgSetupServer.m_authPass       = (*pServer)->GetAccountPassword ();
		LogString ("CheckConnectionSettings 2");
		dlgSetupServer.m_iPortNNTP      = (*pServer)->GetNewsServerPort();
		dlgSetupServer.m_iPortSMTP      = (*pServer)->GetSmtpServerPort();
	}
	else
	{
		// pull in registry values
		LogString ("CheckConnectionSettings 3");
		GetDefaultConnectionSettings (dlgSetupServer);
	}

	while (TRUE)
	{
		int rc;

		CPropertySheet sSheet(IDS_GRAVITY_SERVSETUP);

		sSheet.AddPage ( &dlgSetupServer );

		rc = sSheet.DoModal();
		if (IDCANCEL == rc)
			return FALSE;
		if (dlgSetupServer.m_newsServer.IsEmpty())
			AfxMessageBox (IDS_SERVER_BLANK);
		else
			break;
	}

	LogString ("CheckConnectionSettings 5");
	*pServer = gpStore->CreateServer (dlgSetupServer.m_newsServer);
	LogString ("CheckConnectionSettings 6");
	(*pServer)->Open ();
	(*pServer)->SetSmtpServer ( dlgSetupServer.m_smtpServer );
	(*pServer)->SetEmailAddress ( dlgSetupServer.m_emailAddress );
	(*pServer)->SetOrganization ( dlgSetupServer.m_organization );
	(*pServer)->SetFullName ( dlgSetupServer.m_fullname );
	(*pServer)->SetLogonStyle (dlgSetupServer.m_iLogonStyle);
	(*pServer)->SetAccountName (dlgSetupServer.m_authName);
	(*pServer)->SetAccountPassword (dlgSetupServer.m_authPass);
	(*pServer)->SetNewsServerPort (dlgSetupServer.m_iPortNNTP);
	(*pServer)->SetSmtpServerPort (dlgSetupServer.m_iPortSMTP);
	LogString ("CheckConnectionSettings 7");
	(*pServer)->SaveSettings();
#endif

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void GetDefaultConnectionSettings (T_MSFT_DEFAULTS & sMSFT)
{
	BOOL     fKeyOpen = FALSE;
	HKEY     regKey;
	TCHAR    temp[200];
	DWORD    len;
	DWORD    type;
	CString  strOsKey;

	if (gdwPlatform == VER_PLATFORM_WIN32_NT)
		strOsKey = "Software\\Microsoft\\Windows NT\\CurrentVersion";
	else
		strOsKey = "Software\\Microsoft\\Windows\\CurrentVersion";

	// we can take some well known information out of the registry
	if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE,
		strOsKey,
		DWORD(0),
		KEY_READ,
		&regKey))
	{
		// get the value for the RegisteredOwner
		if (ERROR_SUCCESS == RegQueryValueEx (regKey,
			"RegisteredOwner",
			0,
			&type,
			(LPBYTE) temp,
			&len))
		{
			sMSFT.strFullName = temp;
			// If the value retrieved is "Microsoft", set to blank
			if (!sMSFT.strFullName.CompareNoCase("microsoft"))
				sMSFT.strFullName = "";
		}

		// get the value for the RegisteredOrganization
		len = sizeof (temp);

		if (ERROR_SUCCESS == RegQueryValueEx (regKey,
			"RegisteredOrganization",
			0,
			&type,
			(LPBYTE) temp,
			&len))
		{
			sMSFT.strOrganization = temp;
			// If the value retrieved is "Microsoft", set to blank
			if (!sMSFT.strOrganization.CompareNoCase("microsoft"))
				sMSFT.strOrganization = "";
		}

		RegCloseKey (regKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool  GetMRUServerNameFromRegistry (CString & server)
{
	// check if there is a current

	CString  strValue;
	HKEY     hkServers;
	TCHAR    temp[200];
	DWORD    len;
	DWORD    type;
	bool     fDone = false;

	if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_CURRENT_USER,
		GetGravityRegKey()+"Servers",
		DWORD(0),
		KEY_QUERY_VALUE,
		&hkServers))
	{
		// get the value for the MRU directly
		len = sizeof (temp);
		LogString ("GetMRUServerNameFromRegistry");
		if (ERROR_SUCCESS == RegQueryValueEx (hkServers,
			"MRU",
			LPDWORD (0),
			&type,
			(LPBYTE) temp,
			&len))
		{
			strValue = temp;
			if (strValue.GetLength())
			{
				server = strValue;
				fDone = true;
			}
		}

		RegCloseKey (hkServers);

	} // key opened OK

	return fDone;
}

/////////////////////////////////////////////////////////////////////////////
// GetCurrentServerName -- get the value of the MRU key.  If nothing is
//    there, use the name of the first defined server.
BOOL  GetCurrentServerName(TNewsDB* pDB, CString &server)
{
	bool fDone = GetMRUServerNameFromRegistry(server);

	if (fDone)
	{
		// double check that there really is a server ptr to be had
		if (pDB->GetServerByName(server))
			return TRUE;
	}

	// use first server in database
	TNewsServer * pOneServer = pDB->GetFirstServer();
	if (pOneServer)
	{
		server = pOneServer->GetNewsServerName();
		ASSERT(!server.IsEmpty());
		SetCurrentServer(server);
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// GetServerPointer - Gets a pointer to the server that has
//                    a specific name.
/////////////////////////////////////////////////////////////////////////////
TNewsServer * GetServerPointer (const CString & server)
{
	TServerIterator it(gpStore);
	TNewsServer *pServer;

	// TODO: Add extra initialization here
	while (NULL != (pServer = it.Next()))
	{
		if (0 == server.CompareNoCase (pServer->GetNewsServerName()))
			return pServer;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void SetCurrentServer (const CString & mruServer)
{
	HKEY  hKey;

	// create the base storage key

	if (ERROR_SUCCESS == UtilRegOpenKey (GetGravityRegKey()+"Servers", &hKey))
	{
		LogString ("SetCurrentServer 1");
		RegSetValueEx (hKey,
			"MRU",
			DWORD(0),
			REG_SZ,
			(BYTE *) LPCTSTR(mruServer),
			mruServer.GetLength() + 1);
		LogString ("SetCurrentServer 2");
		RegCloseKey (hKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CreateServer - Create a news server based on dialog values supplied by
//                the user, and close it.
/////////////////////////////////////////////////////////////////////////////
TNewsServer * fnCreateServer (TSetupPage & dlgHuman, TServersPage & dlgServer)
{
	try
	{
		TNewsServer *pServer = 0;
		pServer = gpStore->CreateServer (dlgServer.m_newsServer);

		if (!pServer)
			return FALSE;

		pServer->Open ();
		pServer->SetNewsServerAddress ( dlgServer.m_strNNTPAddress);
		pServer->SetSmtpServer ( dlgServer.m_smtpServer );
		pServer->SetEmailAddress (dlgHuman.m_emailAddress );
		pServer->SetOrganization ( dlgHuman.m_organization );
		pServer->SetFullName ( dlgHuman.m_fullname );
		pServer->SetLogonStyle ( dlgServer.m_iLogonStyle_NTP );
		pServer->SetAccountName (dlgServer.m_authName_NTP);
		pServer->SetAccountPassword (dlgServer.m_authPass_NTP);
		pServer->SetNewsServerPort (dlgServer.m_iPortNNTP);
		pServer->SetConnectSecurely(dlgServer.m_fConnectSecurely);
		pServer->SetSmtpServerPort (dlgServer.m_iPortSMTP);
		pServer->SetEmailAddressForPost (dlgHuman.m_strMailOverride);

		pServer->SetSmtpLogonStyle (dlgServer.m_iLogonStyle_STP );
		pServer->SetSmtpLogonUser (dlgServer.m_authName_STP);
		pServer->SetSmtpLogonPass (dlgServer.m_authPass_STP);

		// must call this  again since we changed stuff - this gets
		// put in the registry...

		pServer->SaveSettings();
		pServer->Close ();

		// at this point, the server is in the global list of servers,
		// and is ready for business by InitServer, but is not open

		return pServer;
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_FNCREATESERVER, kFatal);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
		return NULL;
	}
}

static int giServerMenuCount = 0;

/////////////////////////////////////////////////////////////////////////////
// EmptyServerMenu - Empty the server menu...
/////////////////////////////////////////////////////////////////////////////
void EmptyServerMenu ()
{
	// we only remove servers after we've been
	// through BuildServerMenu

	if (giServerMenuCount)
	{
		CMenu *pServerMenu;
		CMenu* pTopMenu = AfxGetMainWnd()->GetMenu();

		int iPos;
		for (iPos = pTopMenu->GetMenuItemCount()-1; iPos >= 0; iPos--)
		{
			CMenu* pMenu = pTopMenu->GetSubMenu(iPos);
			if (pMenu && pMenu->GetMenuItemID(0) == ID_FILE_SERVER_CONNECT)
			{
				pServerMenu = pMenu;
				break;
			}
		}

		UINT count = pServerMenu->GetMenuItemCount();

		for (UINT i = giServerMenuCount; i < count; i++)
			pServerMenu->DeleteMenu (giServerMenuCount, MF_BYPOSITION);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BuildServerMenu - Adds the list of available servers to the server menu.
/////////////////////////////////////////////////////////////////////////////
void BuildServerMenu ()
{
	CMenu *pServerMenu;
	CMenu* pTopMenu = AfxGetMainWnd()->GetMenu();

	int iPos;
	for (iPos = pTopMenu->GetMenuItemCount()-1; iPos >= 0; iPos--)
	{
		CMenu* pMenu = pTopMenu->GetSubMenu(iPos);
		if (pMenu && pMenu->GetMenuItemID(0) == ID_FILE_SERVER_CONNECT)
		{
			pServerMenu = pMenu;
			break;
		}
	}

	if (!giServerMenuCount)
	{
		giServerMenuCount = pServerMenu->GetMenuItemCount();
	}

	CString strActiveServer;  GetCountedActiveServerName (strActiveServer);

	// build a sorted list of server names
	CStringList rstrServers;
	TServerIterator it(gpStore);
	TNewsServer *pServer;
	while (0 != (pServer = it.Next()))
	{
		CString strName = pServer -> GetNewsServerName ();

		// insert the server name into the list at the proper sorted position
		POSITION pos = rstrServers.GetHeadPosition ();
		POSITION origPos = pos;
		BOOL bBefore = FALSE;
		while (pos) {
			origPos = pos;
			CString &strExisting = rstrServers.GetNext (pos);
			if (strName.CompareNoCase (strExisting) < 0) {
				bBefore = TRUE;
				break;
			}
		}
		if (bBefore)
			rstrServers.InsertBefore (origPos, strName);
		else
			rstrServers.InsertAfter (origPos, strName);
	}

	UINT  flags;
	int   item = 0;
	POSITION pos = rstrServers.GetHeadPosition ();
	while (pos) {
		CString &strName = rstrServers.GetNext (pos);
		flags  = MF_STRING|MF_ENABLED;
		if (strActiveServer == strName)
			flags |= MF_CHECKED;
		pServerMenu->AppendMenu(flags, ID_SERVER_SERVER1 + item, strName);
		item++;
	}

	AfxGetMainWnd()->DrawMenuBar();
}

/////////////////////////////////////////////////////////////////////////////
// GetServersDownloadInfo -- Get list of servers and list of groups for each
//
/////////////////////////////////////////////////////////////////////////////
int GetServersDownloadInfo (ListDownloadInfo & lst)
{
	TServerCountedPtr cpNewsServer;

	// Get list of server names
	TServerIterator it(gpStore);
	TNewsServer *pServer;

	while (0 != (pServer = it.Next()))
	{
		CString strName = pServer -> GetNewsServerName ();

		TServerDownloadInfo * pInfo = new TServerDownloadInfo (strName);
		pServer->GetSubscribedNames ( pInfo->m_lstGroups );

		// insert server name into the list at the proper sorted position
		POSITION pos = lst.GetHeadPosition ();
		POSITION origPos = pos;
		bool     fBefore = false;

		while (pos)
		{
			origPos = pos;

			TServerDownloadInfo * pInfoExist = lst.GetNext (pos);

			if (strName.CompareNoCase (pInfoExist->m_strServerName) < 0)
			{
				fBefore = true;
				break;
			}
		}

		if (fBefore)
			lst.InsertBefore (origPos, pInfo);
		else
			lst.InsertAfter  (origPos, pInfo);
	}

	return 0;
}
