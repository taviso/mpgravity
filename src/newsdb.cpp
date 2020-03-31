/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsdb.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.8  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.7  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.6  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.5  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.4  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.3  2009/06/14 13:17:22  richard_wood
/*  Added side by side installation of Gravity.
/*  Adding (WORK IN PORGRESS!!!) DB export/import.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2008/09/19 14:51:33  richard_wood
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

// newsdb.cpp - News Database Object

#include "stdafx.h"
#include "resource.h"
#include "mplib.h"
#include "newsdb.h"
#include "server.h"
#include "fileutil.h"
#include "globals.h"             // gpStore

#include "regutil.h"
#include <io.h>
#include <direct.h>
#include "tdbdlg.h"
#include <winreg.h>
#include "ssutil.h"
#include "rules.h"               // Rules object
#include "rgpurg.h"
#include "tglobopt.h"
#include "tscoring.h"            // ScoreSets, ...
#include "tbozobin.h"            // TBozoList
#include "dbutil.h"

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TNewsDB Contructor                                                       +
/////////////////////////////////////////////////////////////////////////////
TNewsDB::TNewsDB()
{
	m_fOpen        = FALSE;
	m_minimumFree  = 5;
	m_pIStorage    = 0;
}

/////////////////////////////////////////////////////////////////////////////
// TNewsDB Destructor                                                       +
/////////////////////////////////////////////////////////////////////////////
TNewsDB::~TNewsDB()
{
	// go through the Servers and make sure all database
	// files are closed

	ASSERT(!m_fOpen);
	if (m_fOpen)
		Close();
}

/////////////////////////////////////////////////////////////////////////////
// CreateDBRegistryKeys - Create the initial registry keys for a database,  +
//                        including :
//
//               \HKEY_CURRENT_USER\Software\MicroPlanet\Gravity\Storage
//                   DatabasePath value
//                   MinimumFree value
//               \HKEY_CURRENT_USER\Software\MicroPlanet\Gravity\Servers
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::CreateDBRegistryKeys(LPCTSTR fullPath, DWORD minimumFree)
{
	HKEY  hKey;
	DWORD disposition;

	// create the base storage key

	if (ERROR_SUCCESS != UtilRegOpenKey(GetGravityRegKey()+"Storage", &hKey))
	{
		if (ERROR_SUCCESS != UtilRegCreateKey(GetGravityRegKey()+"Storage", &hKey, &disposition))
		{
			CString str; str.LoadString(IDS_ERR_CREATE_KEY);
			throw(new TDatabaseRegistryError(str, kFatal));
		}
	}

	// set the DatabasePath and MinimumFree variables

	if (ERROR_SUCCESS != RegSetValueEx(hKey,
		"DatabasePath",
		DWORD(0),
		REG_SZ,
		(BYTE *) fullPath,
		_tcslen(fullPath) + 1))
	{
		RegCloseKey(hKey);
		CString str; str.LoadString(IDS_ERR_SET_DBPATH);
		throw(new TDatabaseRegistryError(str, kFatal));
	}

	if (ERROR_SUCCESS != RegSetValueEx(hKey,
		"MinimumFree",
		DWORD(0),
		REG_DWORD,
		(BYTE *) &minimumFree,
		sizeof(DWORD)))
	{
		RegCloseKey(hKey);
		CString str; str.LoadString(IDS_ERR_SET_MINFREE);
		throw(new TDatabaseRegistryError(str, kFatal));
	}

	RegCloseKey(hKey);

	if (ERROR_SUCCESS != UtilRegOpenKey(GetGravityRegKey()+"Servers", &hKey))
	{
		if (ERROR_SUCCESS != UtilRegCreateKey(GetGravityRegKey()+"Servers",
			&hKey,
			&disposition))
		{
			CString str; str.LoadString(IDS_ERR_CREATE_SERVERS_KEY);
			throw(new TDatabaseRegistryError(str, kFatal));
		}
		RegCloseKey(hKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
// RemoveDBRegistryKeys - Remove all registry keys that were created by     +
//                        the database system.  This includes:
//
//                        1) The storage subkey
//                        2) The servers subkey
//
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::RemoveDBRegistryKeys()
{
	UtilRegDelKeyTree(HKEY_CURRENT_USER, GetGravityRegKey()+"Servers");
	UtilRegDelKeyTree(HKEY_CURRENT_USER, GetGravityRegKey()+"Storage");
}

/////////////////////////////////////////////////////////////////////////////
// GetDatabasePath - If the database exists, the path is in the registry.   +
/////////////////////////////////////////////////////////////////////////////
BOOL TNewsDB::GetDatabasePath(CString & databasePath)
{
	int      rc;
	HKEY     regKey;
	DWORD    type;
	TCHAR    temp[200];
	DWORD    dwLen = sizeof(temp);

	// make sure that the database path key is not already present
	rc = UtilRegOpenKey(GetGravityRegKey()+"Storage", &regKey);
	if (ERROR_SUCCESS == rc)
	{
		// get the value for the RegisteredOrganization
		if (ERROR_SUCCESS == RegQueryValueEx(regKey,
			"DatabasePath",
			LPDWORD(0),
			&type,
			(LPBYTE) temp,
			&dwLen))
		{
			// why are we trying to create a database that supposedly
			// already exists?
			RegCloseKey(regKey);
			databasePath = temp;
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Create - Create a news database, leave it open for processing.           +
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::Create(LPCTSTR dbPath)
{
	TPath    path;
	TCHAR    fullDatabasePath[512];
	LPTSTR   pFileName;
	DWORD    result;

	path += dbPath;

	if (Exists())
	{
		CString str; str.LoadString(IDS_ERR_DB_EXISTS);
		throw(new TDatabaseAlreadyExists(str, kFatal));
	}

	// check if the database directory exists, if not create it
	if (_access(path, 0) == -1)
	{
		CString str; str.LoadString(IDS_ERR_DB_DIR_EXIST);
		throw(new TNoBaseDirectory(str, kFatal));
	}

	path.AddBackSlash();
	path += GetGravityNewsDBDir();

	if (_access(path, 0) == -1)
	{
		if (!CreateDirectory(path, 0))
		{
			CString  temp;
			CString str; str.LoadString(IDS_ERR_CREATE_DB_DIRECTORY);
			temp.Format(str, path);
			throw(new TDatabaseCreateDirectory(temp, kFatal));
		}
	}
	else
	{
		// one is already there, check for options.dat
		TPath currPath;
		currPath += path;
		currPath.AddBackSlash();
		currPath += NEW_OPTIONS_FILENAME;

		if (TFileUtil::FileExist(currPath))
		{
			CString  formatString;
			CString str; str.LoadString(IDS_ERR_DB_EXISTS_USE_IT);
			formatString.Format(str, path);
			if (IDYES == AfxMessageBox(formatString, MB_YESNO))
			{
				m_fOpen = FALSE;
				result = GetFullPathName(path,
					sizeof(fullDatabasePath),
					fullDatabasePath,
					&pFileName);

				if (!result)
				{
					CString temp;
					CString str; str.LoadString(IDS_ERR_DB_PATH);
					temp.Format(str, fullDatabasePath);
					throw(new TBadDatabasePath(temp, kFatal));
				}
				CreateDBRegistryKeys(fullDatabasePath, m_minimumFree);
				Open(FALSE);
				return;
			}
			else
			{
				// only give the option of destroying the old one
				CString  dumpOldPrompt;

				CString str; str.LoadString(IDS_ERR_REMOVE_DB);
				dumpOldPrompt.Format(str, path);
				if (IDNO == AfxMessageBox(dumpOldPrompt, MB_YESNO))
				{
					AfxMessageBox(IDS_ERR_USE_OR_REPLACE);
					throw(new TException(IDS_ERR_CREATE_DB, kError));
				}
				else
				{
					TrashDirectory(path);
					RemoveDBRegistryKeys();
					if(!CreateDirectory(path, 0))
					{
						CString temp;
						CString str; str.LoadString(IDS_ERR_CREATE_DIRECTORY);
						temp.Format(str, path);
						throw(new TDatabaseCreateDirectory(temp, kFatal));
					}
				}
			}
		}
	}

	result = GetFullPathName(path,
		sizeof(fullDatabasePath),
		fullDatabasePath,
		&pFileName);

	if (!result)
	{
		CString temp;
		CString str; str.LoadString(IDS_ERR_DB_PATH);
		temp.Format(str, fullDatabasePath);
		throw(new TBadDatabasePath(temp, kFatal));
	}

	// for the following routines, the database has to be open...
	m_databasePath = fullDatabasePath;
	m_fOpen = TRUE;

	// create the rules file.  The list of rules is embedded in the database and
	// is currently empty
	if (m_spRules->IsEmpty())
		m_spRules->CreateInitialRules();
	SaveRules();

	// create the score-sets file
	SaveScoreSets();

	SaveScoreColors();

	CreateDefaultViewFilters();
	SaveViewFilters();

	// save thread-action lists (ignore, watch)
	SaveThreadActions();

	// create the global TIDArray file
	SaveIDArray();

	// create bozo list
	SaveBozoList();

	// read any hints from Install script and create the global options file...
	m_spGlobalOptions->PreLoadFromRegistry();
	SaveGlobalOptions();

	// store the minimum size needed in the registry
	// add the registry keys : storage, DatabasePath, empty servers key

	CreateDBRegistryKeys(fullDatabasePath, m_minimumFree);
}

/////////////////////////////////////////////////////////////////////////////
// CheckConversion - Checks to see if a conversion is necessary
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::CheckConversion(BOOL fLoadGlobalOptions)
{
	HRESULT     hr;
	TPath       oldSettingsPath(m_databasePath, GLOBALS_FILE);
	TPath       newPath(m_databasePath, NEW_OPTIONS_FILENAME);
	CFileStatus status;

	if (CFile::GetStatus(newPath, status))
		return;

	WCHAR wideString[256];

	FormWideString(oldSettingsPath, wideString, sizeof(wideString));

	hr = StgOpenStorage(wideString,
		NULL,
		STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		NULL,
		0,
		&m_pIStorage);

	if (FAILED(hr))
	{
		CString str; str.LoadString(IDS_ERR_OPEN_SETTINGS);
		throw(new TErrorOpeningGlobalsFile(str, kError));
	}

	// load the global options
	OldLoadGlobalOptions(fLoadGlobalOptions);

	// load our global TIDArray
	OldLoadIDArray();

	// load thread-action lists (ignore, watch)
	OldLoadThreadActions();

	// load bozo bin
	OldLoadBozoList();

	// get the list of server objects into memory
	LoadServerDefinitions();

	CheckServerRename();

	// we're open and ready to go...
	m_fOpen = TRUE;

	// get the rules into memory -- the rule system wants the database to already
	// be open
	OldLoadRules();
	if (m_spRules->IsEmpty())
		m_spRules->CreateInitialRules();

	OldLoadScoreSets();

	OldLoadScoreColors();

	m_spIgnoreList->MakeDirty();
	m_spWatchList->MakeDirty();

	Close(); // will save out in new format!

	// close  the global settings file

	m_pIStorage->Release();
	m_pIStorage = 0;

	// remove "Global Settings" file
	TPath settings;

	settings = m_databasePath;
	settings.AddBackSlash();
	settings += GLOBALS_FILE;

	//try
	//{
	//	// CFile::Remove (settings);
	//}
	//catch(...)
	//{
	//	// error removing global settings... ????
	//}

	AfxMessageBox(IDS_GLOBAL_SETTINGS_CONVERTED);
}

/////////////////////////////////////////////////////////////////////////////
// Open - Open a news database for processing.                              +
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::Open(BOOL fLoadGlobalOptions)
{
	if (m_fOpen)
		return;

	// make sure the database has been created...
	if (!Exists())
	{
		CString str; str.LoadString(IDS_ERR_DB_EXIST);
		throw(new TDatabaseDoesNotExist(str, kFatal));
	}

	// get the database path, where everything will begin
	GetDatabasePath(m_databasePath);

	// if no options.dat file exists, we need to convert to a new version

	CheckConversion(fLoadGlobalOptions);

	// load the global options
	NewLoadGlobalOptions(fLoadGlobalOptions);

	// load our global TIDArray
	NewLoadIDArray();

	// load thread-action lists(ignore, watch)
	NewLoadThreadActions();

	// load bozo bin
	NewLoadBozoList();

	// get the list of server objects into memory
	LoadServerDefinitions();

	CheckServerRename();

	// we're open and ready to go...
	m_fOpen = TRUE;

	// get the rules into memory -- the rule system wants the database to already
	// be open
	NewLoadRules();
	if (m_spRules->IsEmpty())
		m_spRules->CreateInitialRules();

	NewLoadScoreSets();

	NewLoadScoreColors();

	LoadViewFilters();
}

/////////////////////////////////////////////////////////////////////////////
// Destroy - Destroy an entire database, including the whole directory      +
//           structure and all of the registry keys.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::Destroy()
{
	CString  path;

	// smash the directory, smash the files, and remove the registry keys...
	if (!GetDatabasePath(path))
	{
		CString str; str.LoadString(IDS_ERR_DB_EXIST);
		throw(new TDatabaseDoesNotExist(str, kFatal));
	}

	TrashDirectory(path);

	RemoveDBRegistryKeys();
}

/////////////////////////////////////////////////////////////////////////////
// Close - Close the database - all files for all servers.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::Close()
{
	if (!m_fOpen)
		return;

	// save thread-action lists(ignore, watch)
	SaveThreadActions();

	// save bozo bin
	SaveBozoList();

	// save our global TIDArray
	SaveIDArray();

	// save rules in case one has disabled itself or has updated its last-fired
	// time
	SaveRules();

	// save score-sets in case a score's last-seen date has been updated
	SaveScoreSets();

	// save the score colors

	SaveScoreColors();

	// save global options

	SaveGlobalOptions();

	// save view filters

	SaveViewFilters();

	// loop through the servers and close and destroy them...
	// note that we have to delete each server because we
	// don't provide a Destruct elements call (because
	// we need to be able to rename a server, and there is
	// no easy mechanism for doing this with a CMap

	TServerIterator   it(this);
	TNewsServer *     pServer;

	// ???? do we somehow have to lock something here?

	while ((pServer = it.Next()) != 0)
	{
		if (pServer->IsOpen())
			pServer->Close();
		delete pServer;
	}

	m_servers.RemoveAll();

	// commit and close the structured storage file...

	m_fOpen = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Compact - Compact the entire database.  If the InPlace flag is used
//           don't use any temporary files.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::Compact(BOOL fInPlace)
{
	// First, the native state of everything but the bodies, is compacted, so...
	// just bop through all of the servers and compact the bodies.

	// loop through the servers and close them...
	TServerIterator it(this);
	TNewsServer *pServer;

	// ???? do we somehow have to lock something here?

	while ((pServer = it.Next()) != 0)
		pServer->Compact(fInPlace);
}

/////////////////////////////////////////////////////////////////////////////
// GetStats - Get statistics for the whole database.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::GetDBStats(TNewsDBStats *pStats)
{
	ZeroMemory((void *) pStats, sizeof(*pStats));

	// loop through the servers and accumulate their statistics

	TServerIterator   it(this);
	TNewsServer *     pServer;
	TNewsDBStats      serverStats;

	// ???? lock what?

	while ((pServer = it.Next()) != 0)
	{
		pServer->GetDBStats(&serverStats);
		pStats->SetCurrentSize(pStats->GetCurrentSize() +
			serverStats.GetCurrentSize());
		pStats->SetCompressibleSize(pStats->GetCompressibleSize() +
			serverStats.GetCompressibleSize());
		if (serverStats.GetExtraSpaceNeeded() > pStats->GetExtraSpaceNeeded())
			pStats->SetExtraSpaceNeeded(serverStats.GetExtraSpaceNeeded());
	}
}

/////////////////////////////////////////////////////////////////////////////
// TNewsDBStats::CompactingWorthWhile - Determines whether
/////////////////////////////////////////////////////////////////////////////
BOOL TNewsDBStats::CompactingWorthWhile()
{
	int difference = m_currentSize - m_compressibleSize;

	if ((difference >= gpGlobalOptions->GetRegPurge()->GetSkipAmount() * 1024) ||
		((difference*100)/m_currentSize >= gpGlobalOptions->GetRegPurge()->GetSkipPercent()))
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// GenServerRegKey - Generates a server registry key of the form Server1,
//                   Server2, etc.. based on what is already there.
/////////////////////////////////////////////////////////////////////////////
CString TNewsDB::GenServerRegKey()
{
	CString  key;
	CString  server;
	CString  strFmt = GetGravityRegKey()+"Servers";
	int      serverNum;
	HKEY     regKey;

	strFmt += "\\%s";

	for (serverNum = 1;; serverNum++)
	{
		server.Format("server%d", serverNum);

		key.Format((LPCTSTR)strFmt, server);

		// key should look like     Software\Microplanet\Gravity\Servers\server7

		if (ERROR_SUCCESS == UtilRegOpenKey(key, &regKey))
			RegCloseKey(regKey);
		else
			break;
	}

	return server;
}

/////////////////////////////////////////////////////////////////////////////
// ServerExist - Is there a server with the given name already defined?     +
/////////////////////////////////////////////////////////////////////////////
bool TNewsDB::AnyServers()
{
	return (m_servers.IsEmpty() == FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CreateServer - Create a news server, including registry keys and the     +
//                directory structure and files that are global to the
//                server like the article outbox.
/////////////////////////////////////////////////////////////////////////////
TNewsServer *TNewsDB::CreateServer(LPCTSTR server)
{
	TNewsServer *pNewsServer = new TNewsServer;
	TPath        path;
	TPath        encodedServer;
	bool         fInMap = false;

	// we can't create a server if we're not open
	if (!m_fOpen)
	{
		CString str; str.LoadString(IDS_ERR_CREATE_FILES);
		throw(new TDatabaseNotOpen(str, kFatal));
	}

	// assign the name
	pNewsServer->SetRegistryKey(GenServerRegKey());
	pNewsServer->SetNewsServerName(server);

	// generate a directory name that is legal...
	encodedServer = GenFileName(server);

	// create the directory
	path += m_databasePath;
	path.AddBackSlash();
	path += encodedServer;

	// save the registry keys

	pNewsServer->SetServerDatabasePath(path);

	// create the server directory
	if (_access(path, 0) == -1)
	{
		if (!CreateDirectory(path, 0))
		{
			CString temp;
			CString str; str.LoadString(IDS_ERR_CREATE_SERVER_DIR);
			temp.Format(str, path);
			throw(new TDatabaseCreateDirectory(temp, kFatal));
		}

		// this is a dependency thing  -amc
		m_servers.SetAt(server, pNewsServer);
		fInMap = true;

		// create all of the blank database files for the server
		pNewsServer->CreateDatabaseFiles();
	}

	pNewsServer->SaveSettings();

	if (!fInMap)
	{
		// stick the new server in the list of servers...
		m_servers.SetAt(server, pNewsServer);
	}

	return pNewsServer;
}

/////////////////////////////////////////////////////////////////////////////
// RemoveServer - Remove an entire news server and all of its               +
//                keys and files.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::RemoveServer(LPCTSTR   server)
{
	TNewsServer *pServer;
	CString      serverDBPath;

	// ???? should have lock around list of servers

	if(!m_servers.Lookup(server, pServer))
	{
		CString str; str.LoadString(IDS_ERR_SERVER_NOT_PRESENT);
		throw(new TServerDoesNotExist(str, kFatal));
	}

	if (!pServer->GetNewsServerName().IsEmpty())
		serverDBPath = pServer->GetServerDatabasePath();

	// make sure all database files are closed
	if (pServer->IsOpen())
		pServer->Close();

	// remove the registry key for the server
	pServer->RemoveRegistryKeys();

	VERIFY(m_servers.RemoveKey(server));
	delete pServer;

	// smash all of the database files
	if (serverDBPath.GetLength())
		if (!TrashDirectory(serverDBPath))
		{
			CString str;
			str.Format(IDS_ERR_REMOVE_DB_FILES, serverDBPath);
			AfxMessageBox(str, MB_OK);
		}
}

/////////////////////////////////////////////////////////////////////////////
// ServerExist - Is there a server with the given name already defined?     +
/////////////////////////////////////////////////////////////////////////////
BOOL TNewsDB::ServerExist(LPCTSTR  serverName)
{
	TNewsServer *pServer;

	return (m_servers.Lookup(serverName, pServer));
}

/////////////////////////////////////////////////////////////////////////////
// GetServerByName - Access a news server pointer
TNewsServer * TNewsDB::GetServerByName(const CString& serverName)
{
	TNewsServer *pServer = 0;

	if (m_fOpen == FALSE) return NULL;

	VERIFY(m_servers.Lookup(serverName, pServer));
	return pServer;
}

/////////////////////////////////////////////////////////////////////////////
// OpenNewsServer - Open a news server for processing.  The close function
//                  is a member of the TNewsServer object.
/////////////////////////////////////////////////////////////////////////////
TNewsServer *TNewsDB::OpenNewsServer(LPCTSTR name)
{
	// make sure that the server exists in our list of servers

	if (!ServerExist(name))
	{
		CString temp;
		CString str; str.LoadString(IDS_ERR_SERVER_EXIST);
		temp.Format(str, name);
		throw(new TServerDoesNotExist(temp, kFatal));
	}

	TNewsServer *pServer;

	// see if it is already open...
	m_servers.Lookup(name, pServer);

	// ask the server to open itself up...
	return (pServer->Open());
}

/////////////////////////////////////////////////////////////////////////////
// GetFirstServer - Get the first (and in release 1, the only) server in the
//                  server list.
/////////////////////////////////////////////////////////////////////////////
TNewsServer * TNewsDB::GetFirstServer()
{
	TServerIterator it(this);
	TNewsServer * pServer;

	pServer = it.Next();

	return pServer;
}

/////////////////////////////////////////////////////////////////////////////
// SaveRules - Save the rules back out.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveRules()
{
	if (!m_fOpen)
	{
		CString str; str.LoadString(IDS_ERR_SAVING_RULES);
		throw(new TDatabaseNotOpen(str, kFatal));
	}

	m_spRules->ReadLock();
	m_spRules.Save(m_databasePath, NEW_RULES_FILENAME);
	m_spRules->UnlockRead();
}

/////////////////////////////////////////////////////////////////////////////
// SaveScoreSets - Save the scores back out.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveScoreSets()
{
	if (!m_fOpen)
	{
		CString str; str.LoadString(IDS_ERR_SAVING_SCORES);
		throw(new TDatabaseNotOpen(str, kFatal));
	}

	m_spScoreSets->ReadLock();
	m_spScoreSets.Save(m_databasePath, NEW_SCORES_FILENAME);
	m_spScoreSets->UnlockRead();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveScoreColors()
{
	if (!m_fOpen)
	{
		CString str; str.LoadString(IDS_ERR_SAVING_SCORES);
		throw(new TDatabaseNotOpen(str, kFatal));
	}

	CObject *rpPtrArray [6];
	for (int i = 0; i < 5; i++)
		rpPtrArray [i] = &grScoreColors [i];
	rpPtrArray [5] = 0;
	DBSave(0, m_databasePath, NEW_SCORE_COLORS_FILENAME, rpPtrArray);
}

/////////////////////////////////////////////////////////////////////////////
// GetRulesList - Get a pointer to the list of rules...
/////////////////////////////////////////////////////////////////////////////
Rules *TNewsDB::GetRuleList()
{
	if (!m_fOpen)
	{
		CString str; str.LoadString(IDS_ERR_GETTING_RULE_LIST);
		throw(new TDatabaseNotOpen(str, kFatal));
	}

	return m_spRules.get();
}

/////////////////////////////////////////////////////////////////////////////
// GetScoreSets - Get a pointer to the score-sets...
/////////////////////////////////////////////////////////////////////////////
ScoreSets *TNewsDB::GetScoreSets()
{
	if (!m_fOpen)
	{
		CString str; str.LoadString(IDS_ERR_GETTING_SCORE_SETS);
		throw(new TDatabaseNotOpen(str, kFatal));
	}

	return m_spScoreSets.get();
}

/////////////////////////////////////////////////////////////////////////////
// GenFileName - Take a group name and generate a file name...              +
/////////////////////////////////////////////////////////////////////////////
CString TNewsDB::GenFileName(LPCTSTR groupName)
{
	ASSERT(groupName);
	return TPath::GenLegalFileName(groupName);
}

/////////////////////////////////////////////////////////////////////////////
// SaveGlobalOptions - Save the global options out to a file.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveGlobalOptions()
{
	// real data structs like SigList, and ArticlePane layout must
	//   be stored in the database...
	m_spGlobalOptions.Save(m_databasePath, NEW_OPTIONS_FILENAME);

	// other string like data can be saved in the registry
	m_spGlobalOptions->SaveToRegistry();
}

/////////////////////////////////////////////////////////////////////////////
// Exists - The path for the database exists in the registry - i.e. it      +
//          was created succesfully, and one of "Global Settings" or
//          "options.dat" exists (old or new format database).
/////////////////////////////////////////////////////////////////////////////
BOOL TNewsDB::Exists()
{
	CString  path;
	if (!GetDatabasePath(path))
		return FALSE;

	path += '\\';
	path += GLOBALS_FILE;

	if (TFileUtil::FileExist(path))
		return TRUE;

	GetDatabasePath(path);
	path += '\\';
	path += NEW_OPTIONS_FILENAME;

	if (TFileUtil::FileExist(path))
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// TServerIterator Constructor - Construct an iterator to iterate over the  +
//                               servers in this database.
/////////////////////////////////////////////////////////////////////////////
TServerIterator::TServerIterator(TNewsDB *pDB)
{
	m_currPos = pDB->m_servers.GetStartPosition();
	m_pDB = pDB;
}

/////////////////////////////////////////////////////////////////////////////
// TServerIterator Destructor                                               +
/////////////////////////////////////////////////////////////////////////////
TServerIterator::~TServerIterator()
{
	// nothing to do...
}

/////////////////////////////////////////////////////////////////////////////
// TServerIterator::Next - Get the next server in the iteration.            +
/////////////////////////////////////////////////////////////////////////////
TNewsServer * TServerIterator::Next()
{
	CString  name;
	TNewsServer *pServer;

	if (m_currPos == NULL)
		return 0;

	m_pDB->m_servers.GetNextAssoc(m_currPos, name, pServer);
	return pServer;
}

/////////////////////////////////////////////////////////////////////////////
// OldLoadGlobalOptions - Load in the global options from the global options
//                     file.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadGlobalOptions(BOOL fLoadRegistrySettings)
{
	LoadGlobalObject(m_spGlobalOptions.get(), OPTIONS_FILENAME);
	if (fLoadRegistrySettings)
		m_spGlobalOptions->LoadFromRegistry();
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadGlobalOptions - Load in the global options from the global options
//                        file.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadGlobalOptions(BOOL fLoadRegistrySettings)
{

	// member func of template class
	m_spGlobalOptions.SafeLoad(false, m_databasePath, NEW_OPTIONS_FILENAME);

	if (fLoadRegistrySettings)
		m_spGlobalOptions->LoadFromRegistry();
}

/////////////////////////////////////////////////////////////////////////////
// OldLoadRules - Load the rule set in from the rules file.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadRules()
{
	m_spRules->WriteLock();
	LoadGlobalObject(m_spRules.get(), RULES_FILENAME);
	m_spRules->UnlockWrite();
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadRules - Load the rule set in from the rules file.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadRules()
{
	m_spRules->WriteLock();

	m_spRules.SafeLoad(false, m_databasePath, NEW_RULES_FILENAME);

	m_spRules->UnlockWrite();
}

/////////////////////////////////////////////////////////////////////////////
// OldLoadScoreSets - Load the score sets
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadScoreSets()
{
	m_spScoreSets->WriteLock();
	try {
		LoadGlobalObject(m_spScoreSets.get(), SCORES_FILENAME);
	}
	catch(...)
	{
		// no problem... probably stream doesn't exist yet
	}
	m_spScoreSets->UnlockWrite();
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadScoreSets - Load the score sets
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadScoreSets()
{
	m_spScoreSets->WriteLock();
	m_spScoreSets.SafeLoad(true, m_databasePath, NEW_SCORES_FILENAME);
	m_spScoreSets->UnlockWrite();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadScoreColors()
{
	try
	{
		CObject *rpPtrArray [6];
		for (int i = 0; i < 5; i++)
			rpPtrArray [i] = &grScoreColors [i];
		rpPtrArray [5] = 0;
		LoadGlobalObject(0, SCORE_COLORS_FILENAME, rpPtrArray);
	}
	catch(...)
	{
		// no problem... probably stream doesn't exist yet
	}
}

/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadScoreColors()
{
	try {
		CObject *rpPtrArray [6];
		for (int i = 0; i < 5; i++)
			rpPtrArray [i] = &grScoreColors [i];
		rpPtrArray [5] = 0;

		if (!DBLoad(true, 0, m_databasePath, NEW_SCORE_COLORS_FILENAME, rpPtrArray))
		{
			// error and recovery ????
		}
	}
	catch(...) {
		// no problem... probably stream doesn't exist yet
	}
}

/////////////////////////////////////////////////////////////////////////////
// OldLoadIDArray - Load the global TIDArray object
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadIDArray()
{
	LoadGlobalObject(m_spIdArray.get(), IDARRAY_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadIDArray - Load the global TIDArray object
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadIDArray()
{
	m_spIdArray.SafeLoad(false, m_databasePath, NEW_IDARRAY_FILENAME);
	// not much error recovery we can do
}

/////////////////////////////////////////////////////////////////////////////
// SaveIDArray - Save the global TIDArray object
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveIDArray()
{
	m_spIdArray.Save(m_databasePath, NEW_IDARRAY_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
// OldLoadThreadActions - Load the global thread action objects(watch, ignore)
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadThreadActions()
{
	try {
		LoadGlobalObject(m_spIgnoreList.get(), IGNORE_FILENAME);
	}
	catch (TSSErrorOpening *pE)
	{
		// no problem... probably first time running this version (this stream
		// didn't exist before)
		pE->Delete();
	}

	try {
		LoadGlobalObject(m_spWatchList.get(), WATCH_FILENAME);
	}
	catch (TSSErrorOpening *pE)
	{
		// no problem... probably first time running this version (this stream
		// didn't exist before)
		pE->Delete();
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadThreadActions - Load the global thread action objects (watch, ignore)
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadThreadActions()
{
	m_spIgnoreList.SafeLoad(true, m_databasePath, NEW_IGNORE_FILENAME);

	m_spWatchList.SafeLoad(true, m_databasePath, NEW_WATCH_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
void TNewsDB::OldLoadBozoList()
{
	try {
		LoadGlobalObject(m_spBozoList.get(), BOZO_FILENAME);
	}
	catch (TSSErrorOpening *pE)
	{
		// no problem... probably first time running this version (this stream
		// didn't exist before)
		pE->Delete();
	}
}

/////////////////////////////////////////////////////////////////////////////
void TNewsDB::NewLoadBozoList()
{
	m_spBozoList.SafeLoad(true, m_databasePath, NEW_BOZO_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
// SaveThreadActions - Save the global thread action objects (watch, ignore)
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveThreadActions()
{
	if(m_spIgnoreList->Dirty())
		m_spIgnoreList.Save(m_databasePath, NEW_IGNORE_FILENAME);

	if (m_spWatchList->Dirty())
		m_spWatchList.Save(m_databasePath, NEW_WATCH_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
void TNewsDB::SaveBozoList()
{
	m_spBozoList.Save(m_databasePath, NEW_BOZO_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
// ExpireThreadActions - performs expiration for watch, ignore
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::ExpireThreadActions()
{
	m_spIgnoreList->Expire();
	m_spWatchList->Expire();
}

/////////////////////////////////////////////////////////////////////////////
// LoadServerDefinitions - Load the server definitions in from the registry,
//                         creating the map of servers.  Note that none of
//                         these servers will be open yet.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::LoadServerDefinitions()
{
	HKEY           serversKey;
	LONG           rc;
	FILETIME       ft;
	TCHAR          rcServerName [512];
	DWORD          len;
	TNewsServer  * pNewsServer;
	int            i = 0;

	// open the servers key
	if (ERROR_SUCCESS != UtilRegOpenKey(GetGravityRegKey()+"Servers",
		&serversKey,
		KEY_ENUMERATE_SUB_KEYS))
	{
		CString str; str.LoadString(IDS_ERR_OPENING_SERVERS_KEY);
		throw(new TDatabaseRegistryError(str, kFatal));
	}

	// make sure the server map is empty
	m_servers.RemoveAll();

	// loop over the servers and load them in, adding them to our
	while (TRUE)
	{
		len = sizeof(rcServerName);
		rc = RegEnumKeyEx(serversKey,    // open key
			DWORD(i++),   // index of entry
			rcServerName, // buffer for subkey
			&len,         // length of subkey
			NULL,         // reserved, must be null
			NULL,         // buffer for class name
			NULL,         // size of class name buffer
			&ft);         // returned file time

		if (rc != ERROR_SUCCESS &&
			rc != ERROR_MORE_DATA)
			break;

		// there's a possibility that there will be a key called "Rename"
		// for handling key renames...
		if (_tcsstr(rcServerName, "server") == rcServerName)
		{
			pNewsServer = new TNewsServer;

			pNewsServer->SetRegistryKey(rcServerName);
			pNewsServer->Load(rcServerName);

			TNewsServer * pOld = NULL;

			// clear out the slot in the CMap    5-1-97 amc
			if (m_servers.Lookup(pNewsServer->GetNewsServerName(), pOld))
				delete pOld;

			m_servers.SetAt(pNewsServer->GetNewsServerName(), pNewsServer);
		}
	}

	RegCloseKey(serversKey);
}

/////////////////////////////////////////////////////////////////////////////
// LoadGlobalObject - Load an object from the global settings compound file.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::LoadGlobalObject(CObject *   pOb,
								LPCTSTR     name,
								CObject *   pObArray[])
{
	LoadSSObject(m_pIStorage, pOb, name, pObArray);
}

/////////////////////////////////////////////////////////////////////////////
// CheckServerRename - Check to see if there is a registry key indicating
//                     that a server should be renamed.  If so, then
//                     rename there server.
/////////////////////////////////////////////////////////////////////////////
void TNewsDB::CheckServerRename()
{
	HKEY           hRenameKey;
	CString        renameKey;
	TCHAR          rcOldName[256];
	TCHAR          rcNewName[256];
	BOOL           fRemoveAndCreate;
	DWORD          reserved;
	DWORD          type;
	DWORD          size;
	TNewsServer *  pServer;

	renameKey = GetGravityRegKey()+"Servers";
	renameKey += "\\Rename";

	if (ERROR_SUCCESS != UtilRegOpenKey(renameKey, &hRenameKey))
		return;

	// get our values...

	reserved = 0;
	type = REG_SZ;
	size = sizeof(rcOldName);
	if (ERROR_SUCCESS != RegQueryValueEx(hRenameKey,
		OLD_SERVER_NAME,
		NULL,
		&type,
		(LPBYTE) rcOldName,
		&size))
	{
		CString temp;
		CString str; str.LoadString(IDS_ERR_READING_SETTING);
		temp.Format(str, rcOldName);
		throw(new TDatabaseRegistryError(temp, kFatal));
	}

	size = sizeof(rcNewName);
	if (ERROR_SUCCESS != RegQueryValueEx(hRenameKey,
		NEW_SERVER_NAME,
		NULL,
		&type,
		(LPBYTE) rcNewName,
		&size))
	{
		CString temp;
		CString str; str.LoadString(IDS_ERR_READING_SETTING);
		temp.Format(str, rcNewName);
		throw(new TDatabaseRegistryError(temp, kFatal));
	}

	type = REG_DWORD;
	size = sizeof(fRemoveAndCreate);

	if (ERROR_SUCCESS != RegQueryValueEx(hRenameKey,
		REMOVE_AND_CREATE,
		NULL,
		&type,
		(LPBYTE) &fRemoveAndCreate,
		&size))
	{
		CString str; str.LoadString(IDS_ERR_READING_REMOVE_CREATE);
		throw(new TDatabaseRegistryError(str, kFatal));
	}

	// find the old server name...

	if (m_servers.Lookup(rcOldName, pServer))
	{
		// take it out of the map right away with the old key
		m_servers.RemoveKey(rcOldName);

		// calculate the new server database path...

		pServer->SetNewsServerName(rcNewName);

		TPath encodedServer;
		TPath path;
		TPath oldPath;

		oldPath = pServer->GetServerDatabasePath();

		// generate a directory name that is legal...
		encodedServer = GenFileName(rcNewName);

		// create the directory
		path += m_databasePath;
		path.AddBackSlash();
		path += encodedServer;

		pServer->SetServerDatabasePath(path);

		// if the we're removing and creating, whack the directory
		// and create the new server database, otherwise,
		// we just rename the directory...
		if (fRemoveAndCreate)
		{
			TrashDirectory(oldPath);

			// create the server directory
			if (_access(path, 0) == -1)
			{
				if (!CreateDirectory(path, 0))
				{
					CString temp;
					CString str; str.LoadString(IDS_ERR_CREATING_DIR);
					temp.Format(str, path);
					throw(new TDatabaseCreateDirectory(temp, kFatal));
				}

				// ??????????????? - these two calls weren't necessary in the
				//                   old world, but the outbox files can't
				//                   create themselves if the server is
				//                   not in the server map.  I don't think
				//                   there is any harm in this happening
				//                   again in the code below.  This
				//                   means for a couple of seconds the
				//                   server is in the map prior to it being
				//                   ready, but I don't think it is possible
				//                   for it to screw up.  We'll see.

				// change the server name and save the settings for the server...
				pServer->SaveSettings();

				// add the server back into the server map...
				m_servers.SetAt(rcNewName, pServer);

				// create all of the blank database files for the server
				pServer->CreateDatabaseFiles();
			}

			// reset the time when the group list was retrieved...
			extern CTime gTimeAncient;
			pServer->SetNewsGroupCheckTime(gTimeAncient);
		}
		else
			MoveFile(oldPath, path);

		// change the server name and save the settings for the server...
		pServer->SaveSettings();

		// add the server back into the server map...
		m_servers.SetAt(rcNewName, pServer);
	}

	// remove the rename key
	if ((ERROR_SUCCESS != RegCloseKey(hRenameKey)) ||
		(ERROR_SUCCESS != RegDeleteKey(HKEY_CURRENT_USER, renameKey)))
	{
		CString str; str.LoadString(IDS_ERR_REMOVING_RENAME_KEY);
		throw(new TDatabaseRegistryError(str, kError));
	}
}

/////////////////////////////////////////////////////////////////////////////
//  LoadViewFilters -- read from file 'Filters.DAT'
int TNewsDB::LoadViewFilters()
{
	bool fOK;

	// preserve the original filters.dat file (in case folks want to switch back to
	//  Super Gravity)

	TPath filterFile(m_databasePath, NEW_FILTER_V2_FILENAME);
	CFileStatus fs;
	CString fileName;

	// load the version 2 file if it exists
	if (CFile::GetStatus(filterFile, fs))
		fileName = NEW_FILTER_V2_FILENAME;
	else
		fileName = NEW_FILTER_V1_FILENAME;

	fOK = m_spAllFilters.SafeLoad(false,             // report missing as status-ok
		m_databasePath,
		fileName);
	ASSERT(fOK);
	return fOK ? 0 : 1;
}

/////////////////////////////////////////////////////////////////////////////
//  SaveViewFilters -- save to file 'Filters.DAT'
//
int TNewsDB::SaveViewFilters()
{
	bool fOK = m_spAllFilters.Save(m_databasePath, NEW_FILTER_V2_FILENAME);
	ASSERT(fOK);

	return fOK ? 0 : 1;
}

/////////////////////////////////////////////////////////////////////////////
int TNewsDB::CreateDefaultViewFilters()
{
	m_spAllFilters->DefaultSet();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void TRuleIterator::CommonConstruct(Rules &sRules, LockType iLockType)
{
	m_psRules = &sRules;
	m_iPosition = m_psRules->GetHeadPosition();
	iType = iLockType;
	if (iType == ReadLock)
		m_psRules->ReadLock();
	if (iType == WriteLock)
		m_psRules->WriteLock();
}

/////////////////////////////////////////////////////////////////////////////
// TRuleIterator -- iterate over the rules in a rule collection
TRuleIterator::TRuleIterator(Rules &sRules, LockType iLockType)
{
	CommonConstruct(sRules, iLockType);
}

/////////////////////////////////////////////////////////////////////////////
// TRuleIterator -- iterate over the rules in the main news database
TRuleIterator::TRuleIterator(LockType iLockType)
{
	CommonConstruct(*(gpStore->GetRuleList()), iLockType);
}

/////////////////////////////////////////////////////////////////////////////
TRuleIterator::~TRuleIterator()
{
	if (iType == ReadLock)
		m_psRules->UnlockRead();
	if (iType == WriteLock)
		m_psRules->UnlockWrite();
}

/////////////////////////////////////////////////////////////////////////////
// Next - gets the next rule in the rule collection
Rule *TRuleIterator::Next()
{
	if (!m_iPosition)
		return 0;
	return m_psRules->GetNext(m_iPosition);
}

// Save all external files.
void TNewsDB::SaveData()
{
	// save thread-action lists(ignore, watch)
	SaveThreadActions();

	// save bozo bin
	SaveBozoList();

	// save our global TIDArray
	SaveIDArray();

	// save rules in case one has disabled itself or has updated its last-fired
	// time
	SaveRules();

	// save score-sets in case a score's last-seen date has been updated
	SaveScoreSets();

	// save the score colors

	SaveScoreColors();

	// save global options

	SaveGlobalOptions();

	// save view filters

	SaveViewFilters();
}
