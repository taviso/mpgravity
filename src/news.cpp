/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: news.cpp,v $
/*  Revision 1.3  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.2  2010/07/24 21:57:03  richard_wood
/*  Bug fixes for Win7 executing file ops out of order.
/*  V3.0.1 RC2
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.15  2010/04/20 21:04:55  richard_wood
/*  Updated splash screen component so it works properly.
/*  Fixed crash from new splash screen.
/*  Updated setver setup dialogs, changed into a Wizard with a lot more verification and "user helpfulness".
/*
/*  Revision 1.14  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.13  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.12  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.11  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.10  2009/07/26 18:35:43  richard_wood
/*  Changed back from ShellExecute to system as ShellExecute doesn't work from program files?
/*
/*  Revision 1.9  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.8  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.7  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.6  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.5  2009/06/14 20:47:36  richard_wood
/*  Vista/Win7 compatible.
/*  Multi user.
/*  Multi version side-by-side.
/*  Import/Export DB & settings.
/*  Updated credits file for the UFT CStdioFileEx and compression code.
/*
/*  Revision 1.4  2009/06/14 13:17:22  richard_wood
/*  Added side by side installation of Gravity.
/*  Adding (WORK IN PORGRESS!!!) DB export/import.
/*
/*  Revision 1.3  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.7  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.6  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.5  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.2  2008/09/19 14:51:33  richard_wood
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

// News.cpp : Defines the class behaviors for the application.
//

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
#include "tsetpg.h"                 // human  setup
#include "tserverspage.h"           // server setup PropertyPage
#include <ddeml.h>
#include "dialman.h"
#include "fileutil.h"
#include "grplist.h"
#include "tidarray.h"            // RetrieveMessageIDs(), ...
#include "hourglas.h"
#include "mpserial.h"
#include "licutil.h"
#include "utilsize.h"
#include "timeutil.h"
#include "tbozobin.h"            // TBozoBin
#include "timpword.h"            // TImpWord
#include "urlsppt.h"
#include "genutil.h"             // RegisterSimilarDialogClass, ...
#include "log.h"
#include <winreg.h>
#include "regutil.h"
#include "tdbdlg.h"
#include "direct.h"
#include "ncmdline.h"
#include "servpick.h"
#include "rgsys.h"
#include "rgurl.h"
#include "rgui.h"
#include "newsdb.h"           // gpStore
#include "newssock.h"         // TDirectSocket InitInstance
#include "netcfg.h"           // TNetConfiguration
#include "vfilter.h"          // TAllViewFilter
#include "thrdact.h"          // EditThreadActionList()
#include "rgxcopy.h"          // RenameBranchAnawaveToMicroplanet
#include "treehctl.h"         // ::Upgrade
#include "rgdir.h"            // TRegDirectory
#include "newsrc.h"           // ImportNewsrcUseDefaults(), ...
#include "picksvr.h"          // server picking related routines
#include "hints.h"            // for sending view hints to newsview
//#include "_analib.h"
#include "helpcode.h"
#include "utilpump.h"
#include "titlevw.h"          // TTitleView
#include "topexcp.h"          // TopLevelExceptionInit
#include "mywords.h"

#include "genutil.h"          // for debug purposes
#if defined(_DEBUG)
#include "thrdlvw.h"
#endif
#include "ipcgal.h"
#include <initguid.h>
#include "NEWS_i.c"
#include "GravityNewsgroup.h"
#include "8859x.h"

#include "StdioFileEx.h"

//#include "splash.h"
#include "SplashScreenEx.h"

#include "PngImage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


// Code fragment that loads png from disc

/*
			PngImage image;

			image.load(strImage);

			if (image.good())
			{
				psImageRef->m_Image.SetFromPNG(image);
				bLoaded = true;
			}
*/


CSplashScreenEx* gpGravitySplash = NULL;

TNewsTasker* gpTasker;
BYTE gfFirstUse = 0;
BYTE gfPostingOK = 0;

TGlobalOptions *gpGlobalOptions;
DWORD gidDdeServerInst = 0;
HSZ ghszServerName = 0;
HSZ ghszTopic = 0;

extern TDialupManager dialMgr;
HWND ghwndMainFrame;

DWORD gdwThreadId = 0;
int   giRegistryBuildNumber = 0;
static HANDLE ghSemSingleInstance = 0;

TLicenseSystem *gpLicenseDope = 0;

TNewsCommandLineInfo sCmdLineInfo;        // see the virtual function

unsigned short MemDefaultPoolBlockSizeFS = 32;

HDDEDATA CALLBACK DdeCallback(
							  UINT     uType,         // transaction type
							  UINT     uFmt,          // clipboard data format
							  HCONV    hConv,         // handle of conversation
							  HSZ      hsz1,          // handle of string
							  HSZ      hsz2,          // handle of string
							  HDDEDATA hdata,         // handle of global memory object
							  DWORD    dwData1,       // transaction-specific data
							  DWORD    dwData2        // transaction-specific data
							  );

static void UpdateURLPatterns(int iCurBuild, BOOL * pfSave);
static void UpdateBuild525RegistrySettings(int iCurBuild, BOOL * pfSave);
BOOL DeleteDirectory(const TCHAR* sPath);

#define NEWS_DB_OPEN             0x001
#define NEWS_WINSOCK_INIT        0x002
#define NEWS_MAINFRAME_CRITSECT  0x004

/////////////////////////////////////////////////////////////////////////////
// CNewsApp

BEGIN_MESSAGE_MAP(CNewsApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_OPTIONS_CONFIGURE, OnOptionsConfigure)
	ON_COMMAND(ID_OPTIONS_RULES, OnOptionsRules)
	ON_COMMAND(ID_OPTIONS_MANUAL_RULE, OnOptionsManualRule)
//	ON_COMMAND(ID_HELP_REGISTRATION, OnHelpRegistration)
	ON_COMMAND(ID_OPTIONS_EDIT_BOZO, OnOptionsEditBozo)
	ON_COMMAND(ID_OPTIONS_WORDS, OnOptionsWords)
	ON_COMMAND(ID_WATCH, OnWatch)
	ON_COMMAND(ID_IGNORE, OnIgnore)
	ON_COMMAND(ID_SERVER_ADDREMOVE, OnServerAddRemove)
	ON_COMMAND(ID_SERVER_OPTIONS, OnServerOptions)
	ON_COMMAND_RANGE(ID_SERVER_SERVER1, ID_SERVER_SERVER50, OnServerMenuItem)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SERVER_SERVER1, ID_SERVER_SERVER50, OnUpdateServerMenuItem)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_IMPORTDATABASE, OnImportdatabase)
	ON_COMMAND(ID_EXPORTDATABASE, OnExportdatabase)
	ON_COMMAND(ID_EXPORTNEWSSERVER, &CNewsApp::OnExportnewsserver)
END_MESSAGE_MAP()

typedef struct ANALIB_STRUCT
{
   WORD        wVersion;            // Major Version of Software
   WORD        wMinorVersion;       // Minor version of Software
   CString     strVersion;          // Version string
} ANALIB, *LPANALIB;

ANALIB gGravityVersion;

// true if we are not fully registered...
//BOOL gfInTrial = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CNewsApp construction

CNewsApp::CNewsApp()
{
	free((void *)m_pszHelpFilePath);
	m_pszHelpFilePath = _tcsdup(_T(GetStartupDir() + "gravity.chm"));
	EnableHtmlHelp();

	m_fCreateStore = FALSE;
	m_pViewFrame = NULL;
	m_pNewsDoc = NULL;
	m_InitFlags = 0;

	InitializeCriticalSection( &m_csIndirectFrame );
	m_hwndIndirectFrame = NULL;

	DWORD  dwVer = WINVER;

	dwVer = dwVer;
}

CNewsApp::~CNewsApp()
{
	if (gpGravitySplash)
	{
		if (::IsWindow(gpGravitySplash->m_hWnd))
		{
			gpGravitySplash->SetStyle(CSS_CENTERAPP);
			gpGravitySplash->Hide();
			gpGravitySplash->DestroyWindow();
		}
		delete gpGravitySplash;
		gpGravitySplash = NULL;
	}
	DeleteCriticalSection( &m_csIndirectFrame );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNewsApp object

CNewsApp theApp;
CNewsApp * gptrApp;

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.
static const CLSID BASED_CODE clsid =
{ 0xabf68600, 0x5a1b, 0x11ce, { 0xa7, 0xe5, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };

/////////////////////////////////////////////////////////////////////////////
// InitializeDocumentTemplates - Set up all that funky document template stuff
/////////////////////////////////////////////////////////////////////////////
CMultiDocTemplate * CNewsApp::InitializeDocumentTemplates()
{
	// Register the application's document templates.  Document templates
	// serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;

	LogString("Create multi doc templates");
	// This configuration has 3 panes
	pDocTemplate = new CMultiDocTemplate(
		IDR_NEWSTYPE, // IDR_SPLIT2TYPE,
		RUNTIME_CLASS(CNewsDoc),
		RUNTIME_CLASS(TNews3MDIChildWnd),    // custom frame has 2 splitter wnds
		RUNTIME_CLASS(CNewsView));

	m_pBrowserTemplate = pDocTemplate;

	// add 3 pane configuration
	AddDocTemplate(pDocTemplate);

	// bunch of templates for posting and mailing
	m_pPostTemplate = new TPostTemplate(IDR_POSTTYPE);
	AddDocTemplate(m_pPostTemplate);
	m_pFollowTemplate = new TPostTemplate(IDR_FOLLOWTYPE);
	AddDocTemplate(m_pFollowTemplate);
	m_pReplyTemplate = new TPostTemplate(IDR_REPLYTYPE);
	AddDocTemplate(m_pReplyTemplate);
	m_pForwardTemplate = new TPostTemplate(IDR_FORWARDTYPE);
	AddDocTemplate(m_pForwardTemplate);
	m_pBugTemplate = new TPostTemplate(IDR_BUGTYPE);
	AddDocTemplate(m_pBugTemplate);
	m_pSuggestionTemplate = new TPostTemplate(IDR_SUGGESTIONTYPE);
	AddDocTemplate(m_pSuggestionTemplate);
	m_pSendToFriendTemplate = new TPostTemplate(IDR_SENDTOFRIENDTYPE);
	AddDocTemplate(m_pSendToFriendTemplate);
	m_pMailToTemplate = new TPostTemplate(IDR_MAILTOTYPE);
	AddDocTemplate(m_pMailToTemplate);

	return pDocTemplate;
}

/////////////////////////////////////////////////////////////////////////////
// CreateDatabase - Create the database where it is supposed to go.
/////////////////////////////////////////////////////////////////////////////
void CreateDatabase(CString & path)
{
	BOOL  fDirExists = FALSE;
	LogString("CreateDatabase 1");
	if (_access(path, 6) == -1)
	{
		LogString("CreateDatabase 4");
		if (_mkdir(path))
			throw(new TException(IDS_ERR_CREATE_DB_DIR, kFatal));
	}

	// first, check to see if the database key is there, then
	// then see if a physical database is already there...
	LogString("CreateDatabase 5");
	if (!gpStore->Exists())
		gpStore->Create(path);
}

//-------------------------------------------------------------------------
// RemoveControlBarSettings - Remove all of the control bar information
//                            from the registry so that the new
//                            SECControlBar stuff works.
//-------------------------------------------------------------------------
void RemoveControlBarSettings()
{
	HKEY           gravKey;
	LONG           rc;
	FILETIME       ft;
	TCHAR          rcControlBarName [512];
	DWORD          len;
	int            i = 0;
	TCHAR          rcControlKey[512];
	CStringArray   controlKeys;

	// open the servers key
	if (ERROR_SUCCESS != UtilRegOpenKey(GetGravityRegKey(),
		&gravKey,
		KEY_ENUMERATE_SUB_KEYS))
		return;

	// loop over all of the control bar keys and remove them...
	while (TRUE)
	{
		len = sizeof(rcControlBarName);
		rc = RegEnumKeyEx(gravKey,            // open key
			DWORD(i++),        // index of entry
			rcControlBarName,  // buffer for subkey
			&len,              // length of subkey
			NULL,              // reserved, must be null
			NULL,              // buffer for class name
			NULL,              // size of class name buffer
			&ft);              // returned file time

		if (rc != ERROR_SUCCESS &&
			rc != ERROR_MORE_DATA)
			break;

		if (_tcsstr(rcControlBarName, "ControlBars") == rcControlBarName)
		{
			_tcscpy(rcControlKey, GetGravityRegKey());
			_tcscat(rcControlKey, rcControlBarName);
			controlKeys.Add(rcControlKey);
		}
	}

	RegCloseKey(gravKey);

	int numKeys = controlKeys.GetSize();

	for (i = 0; i < numKeys; i++)
		UtilRegDelKeyTree(HKEY_CURRENT_USER, controlKeys.GetAt(i));
}

//-------------------------------------------------------------------------
// UpdateRegistrySettings --
//
// RLW - iCurBuild now contains the version in high word
//       and version in low word.
//       We can now reset the build to zero when we release
//       a new version and this upgrade code will still work.
//       Please use HEX numbers below for any new upgrade parts.
//
//-------------------------------------------------------------------------
void UpdateRegistrySettings(int iCurBuild)
{
	BOOL fSave = FALSE;

	// Version : 2.7.1 is 0x010f????
	// Current build : 5 is 0x????0005
	// So full iCurBuild is 0x010f0005 (valid for 2.7.1 build 5)

	// Versions from 2.8 do not have third version number, so 2.8 is 0x0118 (convert 280 to hex) with build number in lower word.

	if (iCurBuild <= 0x012c0000) // 3.0 (300)
	{
	}
	else if (iCurBuild <= 0x01220000) // 2.9 (290)
	{
	}
	else if (iCurBuild <= 0x01180000) // 2.8 (280)
	{
		// 2.8.2 uses a different registry subkey, "Software/Microplanet/Gravity2.8/"

		// We do not automatically import older settings, but if the user wishes to
		// the code that does this is at the end of this class.
	}
	else if (iCurBuild <= 0x010f000) // 2.7.1
	{
		// Nothing changed in the registry in 2.7.1 upto but not including 2.8
	}
	else if (iCurBuild <= 1725)
	{
		// update the decode fraction RE
		CString  defRE;
		defRE = gpGlobalOptions->GetRegSystem()->GetFindFractionRE(true);
		gpGlobalOptions->GetRegSystem()->SetFindFractionRE(defRE);
		fSave = TRUE;
	}
	else if (iCurBuild <= 1576)
		UpdateURLPatterns( iCurBuild, &fSave );
	else if (iCurBuild <= 525)
		UpdateBuild525RegistrySettings( iCurBuild, &fSave );

	// Two reasons we call this.
	//  464->500   Convert from date format to string defined date format
	//  500->550   Widen the column we use for the Indicators.

	else if (iCurBuild <= 500)
	{
		gpGlobalOptions->GetRegUI()->Upgrade( iCurBuild );
		TTreeHeaderCtrl::Upgrade( iCurBuild, gpGlobalOptions->GetRegUI() );
		fSave = TRUE;
	}

	// all of the control bar stuff changed w/ first 2.0 beta 1, and again in
	// 2.0 beta 3, to accomodate the new customizable toolbars
	if (iCurBuild < 767)
		RemoveControlBarSettings();

	if (fSave)
		gpGlobalOptions->SaveToRegistry();
}

//-------------------------------------------------------------------------
// UpdateBuild496RegistrySettings - Updates builds older than 496
//                         with the appropriate registry settings.
//
void UpdateURLPatterns(int iCurBuild, BOOL * pfSave)
{
	// Get a pointer to the current URL settings...
	TURLSettings *pSettings = gpGlobalOptions->GetURLSettings();

	CString temp;

	// load the web, ftp, and mailto settings and set the new patterns...
	temp.LoadString(IDS_DEFAULT_URL_HTTP);
	pSettings->SetWebPattern(temp);
	temp.LoadString(IDS_DEFAULT_URL_FTP);
	pSettings->SetFtpPattern(temp);
	temp.LoadString(IDS_DEFAULT_URL_MAILTO);
	pSettings->SetMailToPattern(temp);

	*pfSave = TRUE;
}

//-------------------------------------------------------------------------
// UpdateBuild525RegistrySettings - Updates builds older than 525
//                         with the appropriate registry settings.
//
void UpdateBuild525RegistrySettings(int iCurBuild, BOOL * pfSave)
{
	*pfSave = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// OemMenuAdjust - Adjust the menus for the OEM version...
/////////////////////////////////////////////////////////////////////////////
void OemMenuAdjust()
{
#if defined(NO_EXPIRE)

	CMenu *pHelpMenu;
	CMenu* pTopMenu = AfxGetMainWnd()->GetMenu();

	int iPos;
	for (iPos = pTopMenu->GetMenuItemCount()-1; iPos >= 0; iPos--)
	{
		CMenu* pMenu = pTopMenu->GetSubMenu(iPos);
		if (pMenu && pMenu->GetMenuItemID(0) == ID_HELP)
		{
			pHelpMenu = pMenu;
			break;
		}
	}

	iPos = pHelpMenu->GetMenuItemCount();

	for (int i = 0; i < iPos; i++)
	{
		if (pHelpMenu->GetMenuItemID(i) == ID_HELP_HOWTOBUYNEWS32)
		{
			// delete the menu item and the divider below
			pHelpMenu->DeleteMenu(i, MF_BYPOSITION);
			pHelpMenu->DeleteMenu(i, MF_BYPOSITION);
			break;
		}
	}

	AfxGetMainWnd()->DrawMenuBar();
#endif
}

/////////////////////////////////////////////////////////////////////////////
// GetOSInfo - Find out what operating system we're using...
/////////////////////////////////////////////////////////////////////////////
void GetOSInfo()
{
	// Figure out if this is NT or Win95
	OSVERSIONINFO verinfo;
	verinfo.dwOSVersionInfoSize = sizeof(verinfo);
	GetVersionEx( &verinfo );
	gdwPlatform  = verinfo.dwPlatformId;
	gdwOSMajor = verinfo.dwMajorVersion;
}

/////////////////////////////////////////////////////////////////////////////
// InitServerByName - Open the indicated server, initialize it, and set the
//              current server to that one.
/////////////////////////////////////////////////////////////////////////////
BOOL CNewsApp::InitServerByName(
								 LPCTSTR        pszServer,
								 TNewsServer*&  rpServer            // pass this out
								 )
{
	ASSERT(gpStore);

	// find the server in the server list
	// and init the global news server pointer to it.
	// it's really tragic that the code has this problem
	// ?????
	{
		TNewsServer * pTempServer = GetServerPointer(pszServer);
		if (!pTempServer)
			return FALSE;
	}

	// open the server and set up the

	TNewsServer* pNewsServer = gpStore->OpenNewsServer(pszServer);
	if (!pNewsServer)
		return FALSE;

	// set the global string !
	SetCountedActiveServerName(pszServer);

	// set the current server in the registry
	SetCurrentServer(pszServer);

	// perform expiration on watch, ignore.  Must be done after server is open
	gpStore->ExpireThreadActions();

	// import newsrc if necessary
	if (pNewsServer->GetImport())
		ImportNewsrcUseDefaults();

	// Do some version tweaking
	ASSERT(pNewsServer);
	pNewsServer->GetSubscribedArray().UpgradeNeedsGlobalOptions(gpGlobalOptions);

	// start the tasker...
	gpTasker = 0;

	LogString("Init instance - start tasker");
	gpTasker = new TNewsTasker;
	gpTasker->CreateThread( CREATE_SUSPENDED );
	gpTasker->SetThreadPriority(gpGlobalOptions->GetRegSystem()->GetPrio(TRegSystem::kTasker));
	gpTasker->ResumeTasker();

	BuildNewsgroupList(FALSE, pNewsServer, gpTasker);

	rpServer = pNewsServer;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CNewsApp initialization
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
BOOL CNewsApp::InitInstance()
{
	CoInitialize(NULL);

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES|ICC_DATE_CLASSES|ICC_USEREX_CLASSES|ICC_COOL_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	if (!InitATL())
		return FALSE;

#if defined(_DEBUG) && defined(MR_MEMCHECK)
	int iAlbertFlag =  _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	if (iAlbertFlag & _CRTDBG_ALLOC_MEM_DF)
		OutputDebugString("_CRTDBG_ALLOC_MEM_DF is on \n");

	if (iAlbertFlag & _CRTDBG_CHECK_ALWAYS_DF)
		OutputDebugString("_CRTDBG_CHECK_ALWAYS_DF is on \n");

	if (iAlbertFlag & _CRTDBG_CHECK_CRT_DF)
		OutputDebugString("_CRTDBG_CHECK_CRT_DF is on \n");

	if (iAlbertFlag & _CRTDBG_LEAK_CHECK_DF)
		OutputDebugString("_CRTDBG_LEAK_CHECK_DF is on \n");

	iAlbertFlag |= _CRTDBG_ALLOC_MEM_DF    |
		// _CRTDBG_CHECK_ALWAYS_DF |
		_CRTDBG_CHECK_CRT_DF    |
		// _CRTDBG_DELAY_FREE_MEM_DF |
		_CRTDBG_LEAK_CHECK_DF;

	_CrtSetDbgFlag( iAlbertFlag );

#endif

	// if this isn't the first instance, return FALSE immediately to exit
	//   from InitInstance
	if (AbortSecondInstance(sCmdLineInfo))
		return FALSE;


	
	VERINFO verInfo;
	TLicenseSystem::GetVersionInt(verInfo);
	CString strVersion = TLicenseSystem::FormatVersionString();
	gGravityVersion.strVersion    = strVersion;
	gGravityVersion.wVersion      = WORD(verInfo.m_nMajor);
	gGravityVersion.wMinorVersion = WORD(verInfo.m_nMinor);

	// create the global article store object
	gpStore = new TNewsDB;

	// do we need to Try to fix the database?
	gfSafeStart = sCmdLineInfo.m_bSafeStart;
	gfLogToFile = BYTE(sCmdLineInfo.m_bLogStartup);

	// The -logo flag turns on the splash screen
	if (sCmdLineInfo.m_bShowSplash)
	{
		CString strMRUServer;

		// if we have no servers setup yet, skip the splash screen.
		if ( GetMRUServerNameFromRegistry(strMRUServer) )
		{
			gpGravitySplash = new CSplashScreenEx();
			if (gpGravitySplash)
			{
				if (gpGravitySplash->Create(NULL, gGravityVersion.strVersion, 2000, CSS_CENTERSCREEN|CSS_FADE|CSS_HIDEONCLICK))
				{
					gpGravitySplash->SetPNGImage(IDR_SPLASH_SMALL);
					gpGravitySplash->SetText("Microplanet Gravity " + gGravityVersion.strVersion);
					gpGravitySplash->SetTextRect(CRect(220,232,700,300));
					gpGravitySplash->SetTextColor(RGB(32,32,32));
					gpGravitySplash->SetTextFont("Tahoma", 140, 0);
					gpGravitySplash->Show();
				}
			}
		}
	}

	// find out what OS we're running under
	GetOSInfo();

	// set our base registry key. Toolbar positions will now be
	//  saved to the Registry instead of an .INI file
	CWinApp::SetRegistryKey("MicroPlanet");

	// Change the profile name to append the major.minor version numbers to it.
	CString strProfileName(GetGravityRegKey());
	strProfileName.TrimRight('\\');
	strProfileName = strProfileName.Mid(strProfileName.ReverseFind('\\')+1);
	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
	free((void*)m_pszProfileName);
	free((void*)m_pszAppName);
	m_pszProfileName = _tcsdup(strProfileName);
	m_pszAppName = _tcsdup(strProfileName);

	// Antti Nivala writes,
	// "AfxGetAppName() returns AfxGetModuleState()->m_lpszCurrentAppName, and that
	// pointer is not updated if you change the application name as instructed in
	// the documentation. To be safe, you must manually update the pointer after
	// you have changed the application name."
	AfxGetModuleState()->m_lpszCurrentAppName = m_pszAppName;
	AfxEnableMemoryTracking(bEnable);

	if (RenameBranchAnawaveToMicroplanet() == false)
	{
		delete gpStore;
		gpStore = 0;
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		delete gpStore;
		gpStore = 0;
		return FALSE;
	}

	// check to see a database already exists...
	BOOL  fDBExists = gpStore->Exists();

	try
	{
		bool fSuccess = false;
		// since this can throw exceptions, the rest of the init stuff
		//    is in a sub-function

		init_more(fDBExists, gfFirstUse, fSuccess);
		return fSuccess;
	}
	catch(TException *e)
	{
		e->PushError(IDS_ERR_CANT_CONTINUE, kFatal);
		e->Display();
		TException *ex = new TException(*e);
		e->Delete();
		throw(ex);
		return FALSE;
	}
	catch(CException *pCE)
	{
		pCE->ReportError();
		throw(NULL);
		return FALSE;
	}
} // InitInstance

// ------------------------------------------------------------------------
// App command to run the dialog
//
void CNewsApp::OnAppAbout()
{
#if defined(_DEBUG)
	TThreadListView * pThread = GetThreadView();

	int nItems = pThread->GetCount();
	CString msg; msg.Format("Listbox items = %d", nItems);
	AfxMessageBox(msg);
#endif
	if (gpGravitySplash)
	{
		gpGravitySplash->Hide();
		gpGravitySplash->SetStyle(CSS_CENTERSCREEN|CSS_HIDEONCLICK);
		gpGravitySplash->SetPNGImage(IDR_SPLASH_LARGE);
		gpGravitySplash->SetText(gGravityVersion.strVersion);
		gpGravitySplash->SetTextColor(RGB(16,16,16));
		gpGravitySplash->SetTextRect(CRect(200,195,700,300));
		gpGravitySplash->SetTextFont("Tahoma", 130, 0);
		gpGravitySplash->Show();
	}
	else
	{
		gpGravitySplash = new CSplashScreenEx();
		if (gpGravitySplash)
		{
			if (gpGravitySplash->Create(NULL, gGravityVersion.strVersion, 0, CSS_CENTERSCREEN|CSS_HIDEONCLICK))
			{
				gpGravitySplash->SetText(gGravityVersion.strVersion);
				gpGravitySplash->SetPNGImage(IDR_SPLASH_LARGE);
				gpGravitySplash->SetTextColor(RGB(16,16,16));
				gpGravitySplash->SetTextRect(CRect(200,195,700,300));
				gpGravitySplash->SetTextFont("Tahoma", 130, 0);
				gpGravitySplash->Show();
			}
		}
	}
}

extern BOOL gfShutdownRename;

/////////////////////////////////////////////////////////////////////////////
// ExitServer - Essentially the ExitInstance code for a particular server.
/////////////////////////////////////////////////////////////////////////////

extern bool TestCountedActiveServer();         // servcp.cpp

BOOL CNewsApp::ExitServer()
{
	// If we don't have any servers yet, exiting them must therefore succeed.
	if (!gpStore->AnyServers())
		return TRUE;

	TServerCountedPtr cpNewsServer;

	// tell the news view to shut down the
	// old newsgroup...
	if (gpTasker)  // iiserver
	{
		gpTasker->Disconnect();
		dialMgr.Disconnect(FALSE);

		if (gpTasker->IsRunning())
		{
			DWORD dwWait;
			// Ask the NewsPump to shutdown
			SetEvent(gpTasker->m_KillRequest);
			dwWait = WaitForSingleObject(gpTasker->m_Killed, 120000);
//#if defined(_DEBUG)
//			switch (dwWait)
//			{
//			case WAIT_OBJECT_0:
//				TRACE0("App: tasker is dead\n");
//				break;
//			case WAIT_TIMEOUT:
//				TRACE0("App: timed out waiting for tasker to end\n");
//				break;
//			default:
//				ASSERT(0);
//				break;
//			}
//#endif
		}

		// just to be really safe, use a temp var
		TNewsTasker * pDeadTasker = gpTasker;
		gpTasker = 0;

		delete pDeadTasker;
		//TRACE0("news:: tasker is freed\n");
	}

	// export newsrc if needed   // iiserver
	if (cpNewsServer->GetExport())
		ExportNewsrcUseDefaults();

	// save out the latest version of the RangeSets
	if (cpNewsServer)  // iiserver
	{
		int k;
		TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

		{
			TNewsGroupArrayWriteLock writeLock(vNewsGroups);
			for (k = 0; k < vNewsGroups->GetSize(); k++)
			{
				// double check status vector vs ReadRange

				TNewsGroup* pNG = vNewsGroups[k];
				pNG->validate_status_vector();
			}
		}

		cpNewsServer->SaveSubscribedGroups();

		{
			TNewsGroupArrayWriteLock readLock(vNewsGroups);
			int ngCount = vNewsGroups->GetSize();
			for (k = 0; k < ngCount; ++k)
			{
				TNewsGroup* pNG = vNewsGroups[k];
				TNewsGroup::m_library.UnregisterNewsgroup(pNG);
			}
		}
	}

	cpNewsServer->Close();
	ASSERT(TestCountedActiveServer());
	SetCountedActiveServerName("");
	return TRUE;
}

//-------------------------------------------------------------------------
int CNewsApp::ExitInstance()
{
	static bool  fExitInstance = false;    // guard against calling twice

	if (fExitInstance)
		return 0;

	CoUninitialize();

	IPC_Gallery_Shutdown();

	// de-initialize the DDEML
	InitDDEServer( FALSE );

	if (m_InitFlags & NEWS_DB_OPEN)
	{
		// write window positions to database (obviously depends on the database)
		gpUIMemory->Save();
	}

	// Write out a new version # to the registry
	if (gpGlobalOptions)
		gpGlobalOptions->GetRegSystem()->Save();

	if (IsActiveServer())
		ExitServer();

	gpGlobalOptions = 0;

	if (gpStore)
	{
		gpStore->Close();
		//TRACE0("\nThe ODB newsstore is closed\n\n");

		delete gpStore;
		gpStore = 0;
	}

	if ( ghRichEditLib )
	{
		FreeLibrary(ghRichEditLib);
		ghRichEditLib = NULL;
	}

	// Note: AfxSocketInit has a bug when MFC is statically linked. It should ensure
	//       that WSACleanup is called, but it never does get called.
	//
	if (m_InitFlags & NEWS_WINSOCK_INIT)
	{
		DWORD lastError;
		m_InitFlags &= ~NEWS_WINSOCK_INIT;
		if (0 != WSACleanup())
		{
			lastError = WSAGetLastError();
			ASSERT(0);
		}
	}

	fExitInstance = true;

	return FreeNamedSemaphoreAndExit();

	if (m_bATLInited)
	{
		_Module.RevokeClassObjects();
		_Module.Term();
	}
} // ExitInstance

////////////////////////////////////////////////////////////////////////////
// Build list of newsgroups for the tasker
void CNewsApp::BuildNewsgroupList(BOOL fIncremental, TNewsServer* pServer, TNewsTasker* pTasker)
{
	TNewsGroupArray& vGroupArray = pServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vGroupArray);

	// locking the array is ok. This happens during startup & subscribe
	ASSERT(pTasker);
	int tot = vGroupArray->GetSize();
	for (int i = 0; i < tot; ++i)
	{
		TNewsGroup * pNG = vGroupArray[i];
		pTasker->RegisterGroup( pNG->GetName() );
	}
}

// global function
void CNewsApp::AddToNewsgroupList( TNewsGroup * pGroup )
{
	// register this newsgroup with the task manager
	if (gpTasker)
		gpTasker->RegisterGroup( pGroup->GetName() );
}

// global function
BOOL CNewsApp::InNewsgroupList( const CString & groupName )
{
	TServerCountedPtr cpNewsServer;

	TNewsGroup* pFoundGrp = 0;
	// this is special = dont use the SmartPointer syntax
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

	// this is like checking for existence
	if (vNewsGroups.Exist( groupName ))
		return TRUE;
	else
		return FALSE;
}

// ------------------------------------------------------------------------
//
int CNewsApp::Run()
{
	CString strTExcept;  strTExcept.LoadString(IDS_TEXCEPTION_BYE);
	try
	{
		return CWinApp::Run();
	}
	catch (TException *except)
	{
		except->Display();

		AfxMessageBox(strTExcept);
		except->Delete();
		return 1;
	}
	catch (CException *ce)
	{
		// pass in default string, if the exception object does not
		//  have an error message.
		ce->ReportError(MB_OK, IDS_CEXCEPTION_BYE);
		ce->Delete();
		return 1;
	}

	// used to have catch (...) here, but it was screwing up
	// CPropertySheet::DoModal, believe it or not

	// page faults, access violations, stack faults should
	//  just go out.  The OS should report the error & address
}

/////////////////////////////////////////////////////////////////////////
// Return TRUE   if you need more processing
// Return FALSE  if you are idle
BOOL CNewsApp::OnIdle(LONG lCount)
{
	BOOL fMoreChange = FALSE;
	BOOL ret = CWinApp::OnIdle(lCount);

	if (ret)
		return ret;
	else
	{
		// (0==ret) the base class is satisfied
		if (0 == m_pNewsDoc)
			return 0;				

		if (m_pNewsDoc->GetStatusChangeCount() > 0)
			fMoreChange = m_pNewsDoc->DequeueArticleStatusChange();

		if (fMoreChange)
			return 1;

		return 0;
	}
}

void CNewsApp::OnHelpRegistration()
{
}

// -------------------------------------------------------------------------
// OnOptionsEditBozo -- called when the Options->Bozo Bin menu item is invoked
void CNewsApp::OnOptionsEditBozo()
{
	TBozoBin dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------------------
// OnOptionsWords -- called when the Options->Important Words menu item is invoked
void CNewsApp::OnOptionsWords()
{
	TImpWord dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------------------
void CNewsApp::OnWatch()
{
	EditThreadActionList(gpStore->GetWatchList(), IDS_WATCH_TITLE,
		IDS_WATCH_LIST_TITLE, IDS_STOP_PROMPT_WATCH);
}

// -------------------------------------------------------------------------
void CNewsApp::OnIgnore()
{
	EditThreadActionList(gpStore->GetIgnoreList(), IDS_IGNORE_TITLE,
		IDS_IGNORE_LIST_TITLE, IDS_STOP_PROMPT_IGNORE);
}

///////////////////////////////////////////////////////////////////////////
//
//  4-12-96 amc  Removed OFN_ASKOVERWRITE.  Calling function can
//               handle cases with file append
BOOL CNewsApp::GetSpec_SaveArticle(CWnd* pAnchorWnd, CString* pFileName,
								   CFile * pFile)
{
	CString & fileName = *pFileName;
	// Load initial dir
	BOOL fOverwrite = FALSE;
	TPath initDir;
	gpUIMemory->GetPath(TUIMemory::DIR_ARTICLE_DEST, initDir);
	CString str; str.LoadString(IDS_ARTICLE_TXT);
	BOOL fRet = TFileUtil::GetOutputFilename(
		&fileName,
		pAnchorWnd,
		IDS_SAVEARTICLE_TITLE,
		&initDir,
		OFN_NOREADONLYRETURN,                // more flags
		IDS_FILTER_SAVEART,
		str);                                // give a default ext

	if (fRet)
	{
		TYP_GETOUTPUTFILE sMode;

		sMode.flags = GETFILE_FLG_TEXT;

		int ret = TFileUtil::CreateOutputFile(
			pAnchorWnd,
			fileName,                    // filename
			initDir,                     // initial directory
			IDS_SAVEARTICLE_TITLE,       // caption id
			*pFile,
			&sMode,                      // controls actions
			false,                       // fFromCache_NoSKIP
			IDS_FILTER_SAVEART);         // filter str-id

		if (0 == ret)
		{
			// save this path
			TPath spec(fileName);
			spec.GetDirectory(initDir);
			gpUIMemory->SetPath(TUIMemory::DIR_ARTICLE_DEST, initDir);
		}
		else
			fRet = FALSE;
	}
	return fRet;
}

BOOL CNewsApp::InitApplication()
{
	CWinApp::InitApplication();

	// Register classes for modeless dialog boxes.  Thus
	//   they can have distinct icons
	RegisterSimilarDialogClass(IDS_CLASS_EVENTLOG,     IDI_EVENTLOG);
	RegisterSimilarDialogClass(IDS_CLASS_SEARCH,       IDI_SEARCH);
	RegisterSimilarDialogClass(IDS_CLASS_IMAGEGALLERY, IDI_GALLERY);
	RegisterSimilarDialogClass(IDS_CLASS_OUTBOX,       IDR_OUTBOX);
	RegisterSimilarDialogClass(IDS_CLASS_MAIL,         IDR_REPLYTYPE);
	RegisterSimilarDialogClass(IDS_CLASS_POST,         IDR_POSTTYPE);

	// note I split the ImageGallery & PrintGallery into 2 dlg templates
	// so Anawave can have separate icons for each

	RegisterSimilarDialogClass(IDS_CLASS_PRINT,        IDI_PRNTJOB);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// 12-26-95 amc  Created
//  8-08-96 amc  Use named semaphore
BOOL CNewsApp::FirstInstance(CWnd*& rpMainWnd)
{
	// look for our NAMED semaphore - is another instance of our App runnning?
	HANDLE hSem = CreateSemaphore(NULL, 0, 1, "MicroPlanet-Gravity-Exe");

	if (NULL == hSem)
	{
		// hard error, bug out
		AfxMessageBox(IDS_ERR_SINGLE_SEM_CREATE);
		return FALSE;
	}
	else
	{
		// we open this, Either we or the OS close it when the process dies.
		DWORD dwLastErr = GetLastError();
		if (0 == dwLastErr)
		{
			// I am the only instance. Proceed as normal.
			ghSemSingleInstance = hSem;
			return TRUE;
		}
		else
		{
			// probably ERROR_ALREADY_EXISTS
			ASSERT(ERROR_ALREADY_EXISTS == dwLastErr);
			ghSemSingleInstance = 0;
			CloseHandle(hSem);
		}
	}

	CString s; s.LoadString(IDS_CLASS_MAIN);
	CWnd *pPrevWnd, *pChildWnd;

	// Determine if another window with our class name exists...
	if ((pPrevWnd = CWnd::FindWindow(s,NULL)) != 0)
	{
		// if so, does it have any popups?
		pChildWnd=pPrevWnd->GetLastActivePopup();

		// Bring the main window to the top
		pPrevWnd->BringWindowToTop();

		// If iconic, restore the main window
		if (pPrevWnd->IsIconic())
			pPrevWnd->ShowWindow(SW_RESTORE);
		else
		{
			CWnd * pFore = CWnd::GetForegroundWindow();
			if (pFore && pFore->m_hWnd != pPrevWnd->m_hWnd)
				pPrevWnd->SetForegroundWindow();
		}

		// If there was an active popup, bring it along too!
		if (pChildWnd && (pPrevWnd != pChildWnd))
			pChildWnd->BringWindowToTop();

		// return ptr to frame window;
		rpMainWnd = pPrevWnd;

		// Return FALSE.  This isn't the first instance
		// and we are done activating the previous one.
		return FALSE;
	}
	else
	{
		// Return false.  No similar window, the true first instance
		// is probably dying now.
		return FALSE;
	}
}

// ------------------------------------------------------------------------
// Called from the newspump thread. Takes ownership of pFetch
void  CNewsApp::ReceiveArticle(TFetchArticle* pFetch)
{
	PostMessage(ghwndMainFrame, WMU_PUMP_ARTICLE, 0,(LPARAM) pFetch);
}

//-------------------------------------------------------------------------
int CNewsApp::FreeNamedSemaphoreAndExit(void)
{
	if (ghSemSingleInstance)
	{
		CloseHandle(ghSemSingleInstance);
		ghSemSingleInstance = 0;
	}
	return CWinApp::ExitInstance();
}

//-------------------------------------------------------------------------
int  CNewsApp::InitDDEServer(BOOL fConnect)
{
	if (fConnect)
	{
		if (DMLERR_NO_ERROR !=
			DdeInitialize(&gidDdeServerInst,          // receives instance identifier
			(PFNCALLBACK) DdeCallback,  // address of callback function
			APPCLASS_STANDARD           // filter notifications
			| APPCMD_FILTERINITS      /*
									  | CBF_FAIL_ADVISES
									  | CBF_FAIL_POKES
									  | CBF_FAIL_REQUESTS
									  | CBF_FAIL_SELFCONNECTIONS */

									  ,
									  0))
		{
			// I deal with 'Connects' and 'Executes', so ignore
			//  advise, poke & request

			// note: DdeCallback() is in urlsppt.cpp
			AfxMessageBox(IDS_ERR_DDE_INIT);
			return 1;
		}
		ghszServerName = DdeCreateStringHandle(gidDdeServerInst,
			"MicroPlanet-Gravity",
			CP_WINANSI);
		ghszTopic = DdeCreateStringHandle(gidDdeServerInst, "System", CP_WINANSI);

		// register the server Name
		VERIFY( DdeNameService(gidDdeServerInst, ghszServerName, 0L, DNS_REGISTER) );
		return 0;
	}
	else
	{
		if (0 == gidDdeServerInst)
			return 0;

		if (ghszServerName)
		{
			DdeFreeStringHandle(gidDdeServerInst, ghszServerName);
			ghszServerName = 0;
		}
		if (ghszTopic)
		{
			DdeFreeStringHandle(gidDdeServerInst, ghszTopic);
			ghszTopic = 0;
		}

		// uninitialize the instance of the server.
		DdeUninitialize(gidDdeServerInst);
		gidDdeServerInst = 0;
		return 0;
	}
}

HWND fnAccessSafeFrameHwnd(BOOL fLock)
{
	return theApp.AccessSafeFrameHwnd(fLock);
}

// --------------------------------------------------------------------------
HWND CNewsApp::AccessSafeFrameHwnd(BOOL fLock)
{
	if (fLock)
	{
		EnterCriticalSection(&m_csIndirectFrame);
		return m_hwndIndirectFrame;
	}
	else
	{
		LeaveCriticalSection(&m_csIndirectFrame);
		return 0;
	}
}

// --------------------------------------------------------------------------
void CNewsApp::ZeroSafeFrameWnd()
{
	EnterCriticalSection(&m_csIndirectFrame);
	m_hwndIndirectFrame = 0;
	LeaveCriticalSection(&m_csIndirectFrame);
}

// --------------------------------------------------------------------------
// AbortSecondInstance -- abort ourselves if a previous instance is running,
//    but not before we check out the command line parameters
bool CNewsApp::AbortSecondInstance(TNewsCommandLineInfo& sCmdLineInfo)
{
	// this is gross. we are checking for the /newsurl: flag on the command line
	ParseCommandLine( sCmdLineInfo );

	CWnd * pMainWnd = 0;

	if (FirstInstance(pMainWnd))
		return false;

	// we are the 2nd instance.  For newsurl, send the data
	//   before we go away!

	if (pMainWnd && sCmdLineInfo.m_bNewsURL)
	{
		// gonna use WM_COPYDATA to accomplish InterProcess Communication
		COPYDATASTRUCT sCD;

		// make a data packet to send
		CString & url = sCmdLineInfo.m_strNewsURL;
		LPCTSTR pUrl = LPCTSTR(url);

		//  this is a joke = RFC1738 describes URL's
		sCD.dwData = 1738;
		sCD.cbData = url.GetLength() + 1;
		sCD.lpData = (void* )pUrl;

		pMainWnd->SendMessage(WM_COPYDATA, NULL, LPARAM(&sCD));
	}

	// we are the 2nd instance.  pass on VCR info
	//   before we go away!

	if (pMainWnd && sCmdLineInfo.m_bVCRFile)
	{
		CString & file    = sCmdLineInfo.m_strVCRFile;
		LPCTSTR   pszFile = LPCTSTR(file);

		// gonna use WM_COPYDATA to accomplish InterProcess Communication
		COPYDATASTRUCT sCD;

		sCD.dwData = 1200;  // this is a joke, since VCR's always blink 12:00AM
		sCD.cbData = file.GetLength() + 1;
		sCD.lpData = (void* ) pszFile;

		pMainWnd->SendMessage(WM_COPYDATA, NULL, LPARAM(&sCD));

	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CNewsApp::OnServerAddRemove()
{
	TPickServerDlg servPick;

	GetCountedActiveServerName(servPick.m_serverIn);
	int RC = servPick.DoModal();

	// if they have removed all servers, we have no choice but to quit
	if (servPick.m_fExitNoServer)
	{
		AfxGetMainWnd()-> PostMessage(WM_COMMAND, ID_APP_EXIT, LPARAM(0));
		return;
	}

	// if they switched servers in the dialog, then we switch
	if (RC == IDOK && servPick.m_serverIn.CompareNoCase(servPick.m_serverOut))
		SwitchServer(servPick.m_serverOut);
}

/////////////////////////////////////////////////////////////////////////////
// ServerNameFromMenuID - get the menu string from the command ID that
//                        was selected
/////////////////////////////////////////////////////////////////////////////
BOOL CNewsApp::ServerNameFromMenuID(UINT command, CString & serverName)
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
	if (!pServerMenu)
		return FALSE;

	pServerMenu->GetMenuString(command, serverName, MF_BYCOMMAND);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// This form is publicly accessible by mainframe object
int  CNewsApp::ForceSwitchServer(const CString & server)
{
	return SwitchServer(server);
}

/////////////////////////////////////////////////////////////////////////////
// SwitchServer -- Returns   0 for success
//                           1 for no-op
//                           2 for failure
//
int CNewsApp::SwitchServer(const CString & newServer)
{
	extern LONG gl_Smtp_Sending;

	// check to see if mail is being transferred right now
	if (gl_Smtp_Sending > 0)
	{
		// Gravity is sending mail. Please wait until the operation is complete
		NewsMessageBox(AfxGetMainWnd(), IDS_EXITSERVER_MAIL,
			MB_OK | MB_ICONINFORMATION);
		return 2;
	}

	// check for open Post windows, ask Human to close them
	if (((CMainFrame*) AfxGetMainWnd())->ExistServerAnchoredWindows())
		return 2;

	if (IsActiveServer())
		((CMainFrame*) AfxGetMainWnd())->UnsubscribeFromSampled();

	CWaitCursor wait;

	// do this so that chunks of code don't think the
	// newsgroup is still open
	CString  oldServer;
	BOOL     fCurrentServer;
	GetCountedActiveServerName(oldServer);

	fCurrentServer = !oldServer.IsEmpty();

	if (fCurrentServer)
		GetNewsView()->CloseCurrentNewsgroup();

	// this is our big fat global variable
	CString  strCurrentServerName;
	GetCountedActiveServerName(strCurrentServerName);

	TNewsServer *pServer = GetServerPointer(newServer);

	if (pServer)
	{
		if (newServer == strCurrentServerName)
			return 1;
		else
		{
			// TTitleView must be notified when switching servers
			//   flush any 'GoBack' information
			TTitleView *pView = (TTitleView *) GetThreadView();
			pView->NotifySwitchServers();

			TNewsServer * pServer = 0;
			if (fCurrentServer)
			{
				GetNewsView()->GetDocument()->UpdateAllViews(NULL, VIEWHINT_EMPTY);
				ExitServer();
			}

			InitServerByName(newServer, pServer);
			GetNewsView()->GetDocument()->UpdateAllViews(NULL, 0);
			InitServerUI(pServer, AfxGetMainWnd());
			// send down notification message
			AfxGetMainWnd()->PostMessage(WMU_SERVER_SWITCH);
			GetNewsView()->GetDocument()->SetTitle(newServer);
			EmptyServerMenu();
			BuildServerMenu();
			// Pump messages for a bit until the UpdateAllViews gets through...
			{
				MSG dispatch;
				while (::PeekMessage( &dispatch, NULL, 0, 0, PM_NOREMOVE))
				{
					AfxGetThread()->PumpMessage();
				}
			}

			return 0;
		}
	}

	return 2;
}

// ------------------------------------------------------------------------
// init_more --
//
void CNewsApp::init_more(BOOL fDBExists, BYTE& rfFirstUse, bool& rfSuccess)
{
	TopLevelExceptionInit();

	bool bCreateDefaultSettings = true;

	// if no database exists, create it.  Otherwise open it
	if (!fDBExists)
	{
		// Get the location where our database should be
		TPath path = TFileUtil::GetAppData()+"\\Gravity\\";

		// RLW - test for existance of required folders and create if needed

		// Gravity
		if (_access(path, 6) == -1)
		{
			if (_mkdir(path))
				throw(new TException(IDS_ERR_CREATE_DIR, kFatal));
		}

		// downloads
		if (_access(path + _T("Download"), 6) == -1)
		{
			if (_mkdir(path + _T("Download")))
				throw(new TException (IDS_ERR_CREATE_DIR, kFatal));
		}

		// spell
		if (_access(path + _T("Spell"), 6) == -1)
		{
			if (_mkdir(path + _T("Spell")))
				throw(new TException(IDS_ERR_CREATE_DIR, kFatal));
		}

		// If we have been launched with a GSX file specified, try to find it
		bool bFoundGSX = false;
		if (sCmdLineInfo.m_bGSXFile)
		{
			TPath path;
			if (TFileUtil::UseProgramPath(sCmdLineInfo.m_strGSXFile, path))
			{
				if (_access(path, 4) != -1)
					bFoundGSX = true;
			}
		}

		if (bFoundGSX)
		{
			CWaitCursor busy;

			if (!ImportDatabase(path, false)) // Import it
				bFoundGSX = false;
			else
			{
				fDBExists = gpStore->Exists();
				if (!fDBExists)
					bFoundGSX = false;
			}
		}

		if (!bFoundGSX)
		{
			CString strOldDatabase;
			CString strOldRegKey;
			if (CheckForOldDatabase(strOldDatabase, strOldRegKey))
			{
				// We do not have a database but an old one has been found,
				// ask user if they want to import this database?
				if (IDYES == AfxMessageBox("An existing database has been found at\n\"" + strOldDatabase
					+ "\"\nWould you like to import this database?", MB_ICONHAND|MB_YESNO))
				{
					if (IDYES == AfxMessageBox("Would you like to import all settings too?", MB_ICONHAND|MB_YESNO))
					{
						CString strTmp = strOldRegKey.Left(strOldRegKey.ReverseFind('\\'));
						CWaitCursor busy;
						if (!ImportSettingsFromOldInstallation(strTmp))
							AfxMessageBox("Error importing settings.\nNew default settings will be created.", MB_ICONHAND | MB_OK);
						else
							bCreateDefaultSettings = false;
					}

					CWaitCursor busy;
					if (false == ImportDatabaseFromOldInstallation(strOldDatabase, strOldRegKey))
					{
						busy.Restore();
						AfxMessageBox("Error importing database.\nA new database will now be created.", MB_ICONHAND | MB_OK);
					}
					else
					{
						gpStore->CreateDBRegistryKeys(path+GetGravityNewsDBDir(), 5);
						fDBExists = gpStore->Exists();
						ASSERT(fDBExists);
					}

					// If import succeeded and user didn't want to import settings
					// create default settings.
					if (fDBExists && bCreateDefaultSettings)
					{
						// where database creation stuff was...  (step1)
						m_InitFlags |= NEWS_DB_OPEN;
						gpGlobalOptions = gpStore->GetGlobalOptions();

						// things to do after loading
						LogString("ii PostProcessing");
						gpGlobalOptions->PostProcessing();

						// save out the global options...  (step 3)
						LogString("ii SaveGlobalOptions");
						gpStore->SaveGlobalOptions();
					}
				}
			}

			// If user did not want to import or the import failed
			if (!fDBExists)
			{
				rfFirstUse = TRUE;

				try
				{
					LogString("ii CreateDatabase");
					CreateDatabase(path);
				}
				catch (TException *pE)
				{
					throw pE;
				}

				// where database creation stuff was...  (step1)
				m_InitFlags |= NEWS_DB_OPEN;
				gpGlobalOptions = gpStore->GetGlobalOptions();

				// things to do after loading
				LogString("ii PostProcessing");
				gpGlobalOptions->PostProcessing();

				// save out the global options...  (step 3)
				LogString("ii SaveGlobalOptions");
				gpStore->SaveGlobalOptions();
			}
		}
	}

	if (fDBExists)
	{
		giRegistryBuildNumber = (int) GetInstalledBuildNumber();

		LogString("InitInstance - open database");
		gpStore->Open();
		m_InitFlags |= NEWS_DB_OPEN;

		// Load the global options from the news store. & registry
		LogString("InitInstance - GetGlobalOptions");

		gpGlobalOptions = gpStore->GetGlobalOptions();
		if (gpGlobalOptions)
			UpdateRegistrySettings(giRegistryBuildNumber);
	}

	// check to see if there is a current server
	// if there is not one, we must force one to be
	// in the created state...

	CString currentServer;

	if (!GetCurrentServerName(gpStore, currentServer))
	{
		int i = create_first_server();
		if (i < 0)
		{
			// User did not want to complete the first run dialog, we cannot continue.
			AfxMessageBox("Quitting initial setup. Please note that you must fill\n"
				"in the details requested here before Gravity can be used.", MB_ICONHAND | MB_OK);
			return;
		}
		else if (i > 0)
		{
			// User has imported a DB, we're restarting.
			rfSuccess = false; // Set this to false to quit InitInstance
			return;
		}
	}

	if (!initialize_sockets())
	{
		return;
	}

	// initialize iso8859-1 tables
	init_my_chartables();

	// compile the URL recognition expression
	LogString("Compile regular expression");
	CompileURLExpression(gpGlobalOptions);

	// store threadid of UI thread
	gdwThreadId = GetCurrentThreadId();

	// Initialize the DDE library... wanted to do this
	// in urlsppt.cpp, but someone said that DDE doesn't work
	// right if not initialized in the main thread.  Am testing
	// this out.

	LogString("ii DDE Init");
	InitDDEServer(TRUE);

	//  since we register a similar window class

	LogString("Load richedit DLL");

	ghRichEditLib = LoadLibrary("RICHED32.DLL");
	if (!ghRichEditLib)
	{
		AfxMessageBox(IDS_ERR_LOAD_RICHEDIT);
		return;
	}

	LogString("ii Init Dial Manager");
	dialMgr.Initialize();     // Load RAS DLL

	LogString("ii LoadProfileSettings");
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// there should definitely be a server now, get the
	// name of the current one

	if (!GetCurrentServerName(gpStore, currentServer))
	{
		// ????  $$$ !!
	}

	TNewsServer * pServer = 0;

	// init server, and tasker
	InitServerByName(currentServer, pServer);

	{
		bool fInitUI = false;
		LogString("Init instance - start loading frame");
		init_ui_phase(pServer, fInitUI);
		if (!fInitUI)
			return ;
	}

	// more server stuff, assume the main window is present now.
	InitServerUI(pServer, AfxGetMainWnd());

	GetNewsView()->GetDocument()->SetTitle(currentServer);

	// do this only for program start up
	AfxGetMainWnd()->PostMessage(WMU_SPLASH_GONE);

	// now that the newsdb is open, upgrade bozo bin if needed
	if (giRegistryBuildNumber < 1100)
		ConvertBozos();

	LogString("InitInstance - end");

	// OK, I guess things are good.

	rfSuccess = true;
}

// ------------------------------------------------------------------------
//
bool CNewsApp::initialize_sockets()
{
	LogString("Initialize Sockets");

	if (AfxSocketInit())
	{
		LogString("Sockets Initialized Successfully");
		m_InitFlags |= NEWS_WINSOCK_INIT;
	}
	else
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return false;
	}

	TDirectSocket::InitInstance();

	return true;
}

// ------------------------------------------------------------------------
//
int CNewsApp::create_first_server(void)
{
	bool bImport = false;

	try
	{
		while (TRUE)
		{
			TSetupPage     pgHuman(true);
			TServersPage   pgSetupServers;

			TNewsServer * psServer = 0;

			if (TRUE == GetServerProfile(pgHuman, pgSetupServers, bImport))
			{
				if (bImport)
				{
					OnImportdatabase();
					return 1;
				}
				else if (0 == (psServer = fnCreateServer(pgHuman, pgSetupServers)))
				{
					// ??????  big-time error
				}
				else
				{
					// set the MRU server key in the registry
					SetCurrentServer(psServer->GetNewsServerName());

					// save the global options...
					gpStore->SaveGlobalOptions();

					break;  // we're done here
				}
			}
			else
				return -1; // User closed the First Run config dialog
		}
	}
	catch (TException *except)
	{
		except->PushError(IDS_ERR_CREATEFIRSTSERVER, kFatal);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
		return 0;
	}
	return 0;
}

// ------------------------------------------------------------------------
// init_ui_phase -- here comes the CFrameWnd!  Yee haw!
//
void CNewsApp::init_ui_phase(TNewsServer* pServer, bool& rfSuccess)
{
	rfSuccess = false;

	// Startup inter-process-communication with Gallery
	// (this is a weird place for this)
	IPC_Gallery_Startup();

	// initialize all of the document templates we use...
	CMultiDocTemplate *pDocTemplate = InitializeDocumentTemplates();

	// create main MDI Frame window - may depend on previous window posns
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return ;
	m_pMainWnd = pMainFrame;
	m_hwndIndirectFrame = ghwndMainFrame = pMainFrame->GetSafeHwnd();

	LogString("Init instance - finished loading frame");

	InitCommonControls();

	gptrApp = this;
	CNewsDoc * pNewsDoc = (CNewsDoc *) pDocTemplate->OpenDocumentFile(NULL);

	// The main window has been initialized, so show and update it.
	{
		// some people setup the shortcut to explicitly start the app minimized
		if (SW_SHOWNORMAL == m_nCmdShow)
		{
			// restore previous state
			TWindowStatus mainWndStatus;
			if (gpUIMemory->GetWindowStatus(TUIMemory::WND_MAIN, mainWndStatus))
				m_nCmdShow = mainWndStatus.m_place.showCmd;
		}
		pMainFrame->ShowWindow(m_nCmdShow);
		pMainFrame->UpdateWindow();
	}

	OemMenuAdjust();

	// update the server menu

	EmptyServerMenu();
	BuildServerMenu();

	rfSuccess = true;
} // init_windows_phase

// ------------------------------------------------------------------------
// Do actions concerned with opening the server
//
BOOL CNewsApp::InitServerUI(TNewsServer* pServer, CWnd* pMainWnd)
{
	BOOL fGroupsHere = pServer->GetGroupListExist();

	if (FALSE == fGroupsHere)
	{
		CNewsDoc * pNewsDoc = GetNewsView()->GetDocument();
		// "Gravity will now retrieve newsgroups. This may take a
		//  few minutes"
		NewsMessageBox(AfxGetMainWnd(), IDS_WARN_GETNGLIST, MB_OK | MB_ICONINFORMATION);

		// tasker will ask Pump to load in the groups
		pNewsDoc->UtilRetrieveRecentNewsgroups(TRUE,  // show dialog
			TRUE); // show empty list
	}

	// connect at startup
	if (fGroupsHere && pServer->GetConnectAtStartup())
	{
		CMainFrame * pMF = (CMainFrame*) pMainWnd;

		if (!pMF->IsVCRWindowActive())
			pMainWnd->PostMessage(WM_COMMAND, ID_FILE_SERVER_CONNECT, LPARAM(0));
	}

	// assuming we connect ok, the program then proceeds to D/L
	//   fresh newsgroups, D/L headers, etc...

	// check for newsurl to load - data is on command line
	pMainWnd->PostMessage(WMU_NEWSURL);

	return TRUE;
}

// ------------------------------------------------------------------------
// OnServerMenuItem -- called when user picks a server from the dynamic list
//                     at the end of the menu. Handles 1-50.
void CNewsApp::OnServerMenuItem(UINT nID)
{
	CString server;
	if (ServerNameFromMenuID(nID, server))
	{
		if (2 == SwitchServer(server))
		{
			// ????? throw up message box...
		}
	}
}

// ------------------------------------------------------------------------
// OnUpdateServerMenuItem --
void CNewsApp::OnUpdateServerMenuItem(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

CNEWSModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_GravityNewsgroup, CGravityNewsgroup)
END_OBJECT_MAP()

LONG CNEWSModule::Unlock()
{
	AfxOleUnlockApp();
	return 0;
}

LONG CNEWSModule::Lock()
{
	AfxOleLockApp();
	return 1;
}

LPCTSTR CNEWSModule::FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
	while (*p1 != NULL)
	{
		LPCTSTR p = p2;
		while (*p != NULL)
		{
			if (*p1 == *p)
				return CharNext(p1);
			p = CharNext(p);
		}
		p1++;
	}
	return NULL;
}

BOOL CNewsApp::InitATL()
{
	m_bATLInited = TRUE;

	_Module.Init(ObjectMap, AfxGetInstanceHandle());
	_Module.dwThreadID = GetCurrentThreadId();

	LPTSTR lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT
	TCHAR szTokens[] = _T("-/");

	BOOL bRun = TRUE;
	LPCTSTR lpszToken = _Module.FindOneOf(lpCmdLine, szTokens);
	while (lpszToken != NULL)
	{
		if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
		{
			_Module.UpdateRegistryFromResource(IDR_GRAVITYNEWSGROUP, FALSE);
			_Module.UnregisterServer(TRUE); //TRUE means typelib is unreg'd
			bRun = FALSE;
			break;
		}
		if (lstrcmpi(lpszToken, _T("RegServer"))==0)
		{
			_Module.UpdateRegistryFromResource(IDR_GRAVITYNEWSGROUP, TRUE);
			_Module.RegisterServer(TRUE);
			bRun = FALSE;
			break;
		}
		lpszToken = _Module.FindOneOf(lpszToken, szTokens);
	}

	if (!bRun)
	{
		m_bATLInited = FALSE;
		_Module.Term();
		CoUninitialize();
		return FALSE;
	}

	_Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER,
		REGCLS_MULTIPLEUSE);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
//
// Import and Export functions
//
////////////////////////////////////////////////////////////////////////

//
// Helper macros
//
#define PORT_FAILED(x, y) \
{ \
	TrashDirectory(x); \
	if (bReportErrors) \
		AfxMessageBox(y, MB_ICONHAND|MB_OK); \
	ASSERT(FALSE); \
	return false; \
}

#define PORT_FAILED_RESTART(x, y) \
{ \
	TrashDirectory(x); \
	StartDBActivity(); \
	if (bReportErrors) \
		AfxMessageBox(y, MB_ICONHAND|MB_OK); \
	ASSERT(FALSE); \
	return false; \
}


////////////////////////////////////////////////////////////////////////
//
// Menu message handlers
//

//
// Import all settings, database inc. news servers and groups from a GDX file.
// OR
// Import the settings and DB files for one news server from a GSX file.
//
void CNewsApp::OnImportdatabase()
{
	// Get the location of the database file to import
	static char BASED_CODE szFilter[] =
		"Gravity Archives (*.gdx;*.gsx)|*.gdx; *.gsx|"
		"Gravity Database Archive (*.gdx)|*.gdx|"
		"Gravity News Server Archive (*.gsx)|*.gsx|"
		"All Files (*.*)|*.*||";
	CFileDialog dlg(TRUE, NULL, "", OFN_FILEMUSTEXIST|OFN_FORCESHOWHIDDEN|OFN_EXPLORER, szFilter);
	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor busy;

	ImportDatabase(dlg.GetPathName(), true);
}

//
// Export all settings, database inc. news
// servers and groups to a GDX file.
//
void CNewsApp::OnExportdatabase()
{
	// Get the filename of the exported database
	static char BASED_CODE szFilter[] = "Gravity Database Archive (*.gdx)|*.gdx|All Files (*.*)|*.*||";
	CFileDialog dlg(FALSE, "gdx", "GravityDatabase", OFN_OVERWRITEPROMPT|OFN_FORCESHOWHIDDEN|OFN_EXPLORER, szFilter);
	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor busy;

	ExportDatabase(dlg.GetPathName(), true, false);
}

//
// Export a user specified news server + its associated
// settings to a GSX compressed file.
//
void CNewsApp::OnExportnewsserver()
{
	// If we don't have any servers yet, we can't export any
	if (!gpStore->AnyServers())
	{
		AfxMessageBox("Cannot export a News Server as none have been setup!", MB_ICONINFORMATION|MB_OK);
		return;
	}

	CString strExportFile;
	TPickServerDlg pickDlg(true);
	static char BASED_CODE szFilter[] = "Gravity News Server Archive (*.gsx)|*.gsx|All Files (*.*)|*.*||";

	if (pickDlg.DoModal() == IDOK)
	{
		CFileDialog dlg(FALSE, "gsx", pickDlg.m_serverOut, OFN_OVERWRITEPROMPT|OFN_FORCESHOWHIDDEN|OFN_EXPLORER, szFilter);
		// Get the filename of the export file
		if (dlg.DoModal() != IDOK)
			return;
		strExportFile = dlg.GetPathName();
	}

	TNewsServer *pServer = GetServerPointer(pickDlg.m_serverOut);

	CWaitCursor busy;

	// Call func to do the export
	ExportNewsServer(pServer, strExportFile, true);
}


////////////////////////////////////////////////////////////////////////
//
// Top level funcs to do the import / export
//

//
// Import GDX or GSX function
//
bool CNewsApp::ImportDatabase(CString strSourceFile, bool bReportErrors)
{
	// Importing needs the global store object
	if (!gpStore)
	{
		ASSERT(FALSE);
		return false;
	}

	// Create a temp directory in the users temp folder
	CString strSourceDir, strCommand, strTempExport;
	make_temp_filename(strSourceDir);
	strSourceDir += "\\";
	if (_mkdir(strSourceDir))
		PORT_FAILED(strSourceDir, "Import from "+strSourceFile+" failed.\nCould not create temporary directory.");

	if (!WaitForFile(strSourceDir))
		PORT_FAILED(strSourceDir, "Import from "+strSourceFile+" failed.\nCould not create temporary directory.");

	// Unzip the database & reg file to the temp folder
	if (!TFileUtil::Unzip(strSourceFile, strSourceDir))
		PORT_FAILED(strSourceDir, "Import from "+strSourceFile+" failed.\nCould not extract GDX/GSX file.");

	// Check the registry file
	strCommand.Format("%sgravity.reg", strSourceDir);
	if (!CheckRegFile(strCommand))
		PORT_FAILED(strSourceDir, "Import from "+strSourceFile+" failed.\n\nThe file has been tampered with. Do not use.");

	bool bCloseDown = false;

	// Now do the appropriate action
	if (strSourceFile.Right(3).CompareNoCase("gdx") == 0)
	{
		// Importing a GDX file

		// We might have arrived here from the First Time setup dialog, in which case there is no existing DB to export
		bool bHaveCurrentDB = true;
		CString currentServer;
		if ((gpStore->IsOpen() == FALSE) || (!GetCurrentServerName(gpStore, currentServer)))
			bHaveCurrentDB = false;

		// If we have a current DB make a temp backup just incase the import doesn't work
		if (bHaveCurrentDB)
		{
			make_temp_filename(strTempExport);
			// Change the temp filname to a GDX
			strTempExport.TrimRight("tmp");
			strTempExport += "gdx";
			if (!ExportDatabase(strTempExport, false, true))
				PORT_FAILED(strSourceDir, "Import from "+strSourceDir+" failed.\nCould not create temporary backup.\n\nPlease retry.");
		}

		// Close down the DB object just in case its in use
		if (!StopDBActivity())
			PORT_FAILED(strSourceDir, "Import from "+strSourceFile+" failed.\nCould not close news engine. Please retry.");

		// Close/Stop other things
		if (m_InitFlags & NEWS_DB_OPEN)
			gpUIMemory->Save();

		if (gpGlobalOptions)
			gpGlobalOptions->GetRegSystem()->Save();

		gpStore->Close();

		if (!ImportGDX(strSourceDir))
		{
			ASSERT(FALSE);
			TrashDirectory(strSourceDir);
			if (bReportErrors)
			{
				if (bHaveCurrentDB)
					AfxMessageBox("Import from "+strSourceFile+" failed.\n"
						"Gravity will now restore your original settings and then close down.\n"
						"Please restart Gravity manually to use the restored settings.", MB_ICONHAND|MB_OK);
				else
					AfxMessageBox("Import from "+strSourceFile+" failed.\n"
						"Please import another database or set up a new one.\n"
						"Gravity will now exit.", MB_ICONHAND|MB_OK);
			}

			if (bHaveCurrentDB)
			{
				// Restore the temp backup
				if (!RestoreTempBackup(strTempExport))
				{
					ASSERT(FALSE);
					if (bReportErrors)
						AfxMessageBox(
						"Could not restore your old settings. Gravity cannot proceed without\n"
						"a valid database.\n"
						"Please make a note of the name and location of the file below and\n"
						"refer to the user manual for further help on how to repair Gravity.\n\n"
						"Your current database and settings have been stored in file:\n"+strTempExport, MB_ICONHAND|MB_OK);
				}
				else
					DeleteFile(strTempExport);
			}
		}
		else
		{
			TrashDirectory(strSourceDir);
			if (bReportErrors)
				AfxMessageBox("Import from "+strSourceFile+" succeeded.\n"
					"Gravity will now close down. Please restart Gravity\n"
					"manually to use the imported settings and database.");
		}
		bCloseDown = true;
	}
	else
	{
		// Importing a GSX file
		if (!ImportGSX(strSourceDir, true))
			PORT_FAILED(strSourceDir, "Import from "+strSourceFile+" failed.")
		else
		{
			TrashDirectory(strSourceDir);
			if (bReportErrors)
				AfxMessageBox("Import from "+strSourceFile+" succeeded.\n"
					"Please restart Gravity to use the imported server.");

			return true;
		}
	}

	if (!bCloseDown)
	{
		if (bReportErrors)
			AfxMessageBox("Import from\n"+strSourceFile+"\nsucceeded.", MB_ICONINFORMATION|MB_OK);
	}
	else
		PostMessage(NULL, WM_QUIT,0,0);

	return true;
}

//
// Very cut down version of ImportDatabase, restores a temporary backup.
//
bool CNewsApp::RestoreTempBackup(CString strSourceFile)
{
	TRACE("CNewsApp::RestoreTempBackup '%s' >\n", strSourceFile);
	// Importing needs the global store object
	if (!gpStore)
	{
		TRACE("CNewsApp::RestoreTempBackup : Failed : gpStore NULL <\n");
		ASSERT(FALSE);
		return false;
	}

	bool bRV = true;

	// Create a temp directory in the users temp folder
	CString strSourceDir, strCommand, strTempExport;
	make_temp_filename(strSourceDir);
	strSourceDir += "\\";
	if (_mkdir(strSourceDir))
	{
		bRV = false;
		TRACE("CNewsApp::RestoreTempBackup : Failed : Could not create temp directory <\n");
	}
	else
	{
		if (!WaitForFile(strSourceDir))
		{
			bRV = false;
			TRACE("CNewsApp::RestoreTempBackup : Failed : Could not create temp directory <\n");
		}
		else
		{
			if (!TFileUtil::Unzip(strSourceFile, strSourceDir))
			{
				bRV = false;
				TRACE("CNewsApp::RestoreTempBackup : Failed : Could not unzip GDX <\n");
			}
			else
			{
				strCommand.Format("%sgravity.reg", strSourceDir);
				if (!CheckRegFile(strCommand))
				{
					bRV = false;
					TRACE("CNewsApp::RestoreTempBackup : CheckRegFile Failed <\n");
				}
				else
				{
					if (!ImportGDX(strSourceDir))
					{
						bRV = false;
						TRACE("CNewsApp::RestoreTempBackup : ImportGDX Failed <\n");
					}
				}
			}
		}
	}

	TrashDirectory(strSourceDir);

	TRACE("CNewsApp::RestoreTempBackup : %s >\n", bRV ? "succeeded" : "failed");
	return bRV;
}

//
// Import a GDX file.
//
bool CNewsApp::ImportGDX(CString strSourceDir)
{
	TRACE("CNewsApp::ImportGDX >\n");
	CString strCommand;
	// Copy "mywords.lst" from backup dir to live location
	if (!CopyDirectory(strSourceDir+"mywords.lst", TFileUtil::GetAppData()+"\\Gravity\\Spell", false))
	{
		TRACE("CNewsApp::ImportGDX : CopyDirectory failed copying mywords.lst to %s\n", TFileUtil::GetAppData()+"\\Gravity\\Spell");
		ASSERT(FALSE);
		return false;
	}

	// Delete the mywords.lst file as we've finished with it
	_unlink(strSourceDir+"mywords.lst");

	// Remove all current server entries in the registry
	gpStore->RemoveDBRegistryKeys();

	// Import all the registry stuff
	strCommand.Format("%sgravity.reg", strSourceDir);
	if (!CheckFixupAndMergeRegFile(strCommand, false, "", ""))
	{
		TRACE("CNewsApp::ImportGDX : CheckFixupAndMergeRegFile failed '%s'\n", strCommand);
		ASSERT(FALSE);
		return false;
	}

	// Delete the reg file as we've finished with it
	_unlink(strCommand);

	// Import the DB files
	if (!ImportDatabaseFiles(strSourceDir, true, false))
	{
		TRACE("CNewsApp::ImportGDX : ImportDatabaseFiles failed '%s'\n", strSourceDir);
		ASSERT(FALSE);
		return false;
	}

	// Fixup any incorrect paths in the registry 
	if (!FixDatabaseValues())
	{
		TRACE("CNewsApp::ImportGDX : FixDatabaseValues failed\n");
		ASSERT(FALSE);
		return false;
	}

	TRACE("CNewsApp::ImportGDX succeeded <\n");
	return true;
}

//
// Importing a GSX file.
//
bool CNewsApp::ImportGSX(CString strSourceDir, bool bReportErrors)
{
	// Get the name of the news server we're importing and check if
	// it will conflict with any of the existing news servers
	bool bRenameServer = false;
	CString strImportServerName, strOrigImportServerName, strImportServerRealName, strUserName, strCommand;
	strCommand.Format("%sgravity.reg", strSourceDir);

	// Get the details from the server import reg file
	if (!ParseRegFileForServerDetails(strCommand, strImportServerName, strImportServerRealName, strUserName))
	{
		if (bReportErrors)
			AfxMessageBox("News server import failed. Invalid import file.", MB_ICONHAND|MB_OK);
		ASSERT(FALSE);
		return false;
	}

	strOrigImportServerName = strImportServerName;

	// Is the server name a duplicate?
	int n = 2;
	while (gpStore->ServerExist(strImportServerName))
	{
		// A duplicate - append "imported" to the end of the server name
		strImportServerName.Format("%s (%d)", strImportServerName, n++);
		bRenameServer = true;
	}

	// Ask the user if they really want to import this server
	if (AfxMessageBox("Import details:-\n"
		"Server name : \'"+strImportServerName+"\'\n"
		"NNTP server name : \'"+strImportServerRealName+"\'\n"
		"User name : \'"+strUserName+"\'\n\n"
		"Do you wish to import this server?",
		MB_ICONQUESTION|MB_YESNO) == IDNO)
	{
		if (bReportErrors)
			AfxMessageBox("News server import aborted by user.", MB_ICONHAND|MB_OK);
		return true;
	}

	// If we get here the user is happy to import the server.

	// The server directories are called "server1", "server2", etc.
	// Find the next available one
	CString strNewServerKey = gpStore->GenServerRegKey();

	// Import all the registry stuff
	if (!CheckFixupAndMergeRegFile(strCommand, true, strImportServerName, strNewServerKey))
	{
		ASSERT(FALSE);
		return false;
	}

	// If we renamed the server nickname, the db file directory must be the same
	if (bRenameServer)
	{
		if (rename(strSourceDir+strOrigImportServerName, strSourceDir+strImportServerName))
		{
			ASSERT(FALSE);
			return false;
		}
		else
		{
			if (!WaitForFile(strSourceDir+strImportServerName))
			{
				ASSERT(FALSE);
				return false;
			}
		}
	}

	// Delete the reg file as we've finished with it
	_unlink(strCommand);

	// Import the DB files
	if (!ImportDatabaseFiles(strSourceDir, false, true))
	{
		ASSERT(FALSE);
		return false;
	}

	// Fixup any incorrect paths in the registry
	if (!FixDatabaseValues())
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}

//
// Export all settings, database, all news servers and groups.
//
// strDestFile = path and name of file to create export in.
// bReportErrors = true to report errors to user.
// bNoShutdownRestart = true to inhibit shutting down / restarting server engine
//
bool CNewsApp::ExportDatabase(CString strDestFile, bool bReportErrors, bool bNoShutdownRestart)
{
	// Create a temp file name in the users temp folder
	CString strDestDir, strSourcePath, strCommand;
	make_temp_filename(strDestDir);
	if (_access(strDestDir, 6) == -1)
	{
		if (!CreateDirectory(strDestDir, NULL))
		{
			PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nCould not create temporary folder.");
		}
		else
		{
			if (!WaitForFile(strDestDir))
				PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nCould not create temporary folder.");
		}
	}

	if (!bNoShutdownRestart)
		// Pause DB object just in case its in use
		if (!StopDBActivity())
			PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nCould not stop news engine.\n\nPlease retry.");

	// Copy the DB folder to the temp location
	if (!CopyDirectory(TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir(), strDestDir, true))
		PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nDatabase files open.\n\nPlease retry.");

	// Copy "mywords.lst" to the temp location
	if (!CopyDirectory(TFileUtil::GetAppData()+"\\Gravity\\Spell\\mywords.lst", strDestDir, false))
		PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nUsers custom word dictionary open.\n\nPlease retry.");

	// Export our whole reg tree to the temp folder
	TRegExport exporter(GetGravityRegKey(), strDestDir+"\\gravity.reg");
	if (!exporter.Export())
		PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nCould not export registry data.\n\nPlease retry.");

	// Compress the contents of the temp directory
	if (!TFileUtil::Zip(strDestFile, strDestDir))
		PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nCompression failed.\n\nPlease retry.");

	if (!WaitForFile(strDestFile))
		PORT_FAILED(strDestDir, "Database export to "+strDestFile+" failed.\nCompression failed.\n\nPlease retry.");

	// Tidy up
	TrashDirectory(strDestDir);

	if (!bNoShutdownRestart)
	{
		// Restart DB object
		if (!StartDBActivity())
		{
			// failed
			if (bReportErrors)
			{
				AfxMessageBox("Database exported to "+strDestFile+" successfully,\nhowever the news engine could not be restarted.\n"
					"Gravity will now close.\n\nPlease manually start Gravity.", MB_ICONHAND|MB_OK);
				((CMainFrame*)m_pMainWnd)->SetDoNotCompactDBOnShutdown(true);
				PostThreadMessage(WM_QUIT,0,0);
			}
			ASSERT(FALSE);
			return false;
		}
	}

	if (bReportErrors)
		AfxMessageBox("Database exported to "+strDestFile+" successfully.", MB_ICONINFORMATION|MB_OK);

	return true;
}

//
// Export a news server, including its settings and DB files.
//
bool CNewsApp::ExportNewsServer(TNewsServer *pNewsServerToExport, CString strDestFile, bool bReportErrors)
{
	if (!pNewsServerToExport)
	{
		ASSERT(FALSE);
		return false;
	}

	m_strStoppedServerName = "";

	CString strServerName = pNewsServerToExport->GetNewsServerName(),
		    strServerRegKey = pNewsServerToExport->GetRegistryKey(),
			strDestDir, strSourcePath, strCommand;

	// Create a temp dir in the users temp folder
	make_temp_filename(strDestDir);
	if (_access(strDestDir, 6) == -1)
	{
		if (!CreateDirectory(strDestDir, NULL))
		{
			PORT_FAILED(strDestDir, "News server export to "+strDestFile+" failed.\nCould not create temporary folder.");
		}
		else if (!WaitForFile(strDestDir))
		{
			PORT_FAILED(strDestDir, "News server export to "+strDestFile+" failed.\nCould not create temporary folder.");
		}
		if (!CreateDirectory(strDestDir+"\\"+strServerName, NULL))
		{
			PORT_FAILED(strDestDir, "News server export to "+strDestFile+" failed.\nCould not create temporary folder.");
		}
		else if (!WaitForFile(strDestDir+"\\"+strServerName))
		{
			PORT_FAILED(strDestDir, "News server export to "+strDestFile+" failed.\nCould not create temporary folder.");
		}
	}

	// Close news server just in case its in use
	if (GetCountedActiveServerName().CompareNoCase(pNewsServerToExport->GetNewsServerName()) == 0)
		if (!StopDBActivity())
			PORT_FAILED(strDestDir, "News server export to "+strDestFile+" failed.\nCould not stop news engine.\n\nPlease retry.");

	// Copy the DB folder to the temp location
	// Location to copy from
	if (!CopyDirectory(TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir()+strServerName, strDestDir+"\\"+strServerName, true))
		PORT_FAILED_RESTART(strDestDir, "News server export to "+strDestFile+" failed.\nDatabase files open.\n\nPlease retry.");

	// Export our whole reg tree to the temp folder
	TRegExport exporter(GetGravityRegKey()+"servers\\"+strServerRegKey, strDestDir+"\\gravity.reg");
	if (!exporter.Export())
		PORT_FAILED_RESTART(strDestDir, "News server export to "+strDestFile+" failed.\nCould not export registry data.\n\nPlease retry.");

	// Compress all the contents of the temp folder
	if (!TFileUtil::Zip(strDestFile, strDestDir))
		PORT_FAILED_RESTART(strDestDir, "News server export to "+strDestFile+" failed.\nCompression failed.\n\nPlease retry.");

	if (!WaitForFile(strDestFile))
		PORT_FAILED_RESTART(strDestDir, "News server export to "+strDestFile+" failed.\nCompression failed.\n\nPlease retry.");


	TrashDirectory(strDestDir);

	// Restart DB object
	if (!StartDBActivity())
	{
		// failed
		if (bReportErrors)
		{
			AfxMessageBox("News server \'"+strServerName+"\' exported to "+strDestFile+" successfully,\nhowever the news engine could not be restarted.\n"
				"Gravity will now close.\n\nPlease manually start Gravity.", MB_ICONHAND|MB_OK);
			((CMainFrame*)m_pMainWnd)->SetDoNotCompactDBOnShutdown(true);
			PostThreadMessage(WM_QUIT,0,0);
		}
		ASSERT(FALSE);
		return false;
	}

	if (bReportErrors)
		AfxMessageBox("News server \'"+strServerName+"\' exported to "+strDestFile+" successfully.", MB_ICONINFORMATION|MB_OK);

	return true;
}



//
// Helper functions for the Import / Export processes
//




//
// Try to locate an old installation of Gravity that we can use to import
//
bool CNewsApp::CheckForOldDatabase(CString &strDatabase, CString &strRegKey)
{
	// Ok, get version information
	bool bRV = false;
	LONG lRet;
	DWORD type, dwLen;
	TCHAR temp[200];
	HKEY hKey;
	CString strTmp, strPathToTest;

	VERINFO verInfo;
	TLicenseSystem::GetVersionInt(verInfo);

	if (verInfo.m_nMinor == 0)
	{
		verInfo.m_nMajor--; verInfo.m_nMinor =99;
	}
	else
		verInfo.m_nMinor--;

	// Look in the registry for older versions of Gravity
	while(!bRV && (verInfo.m_nMajor >= 2))
	{
		while(!bRV && (verInfo.m_nMinor >= 0))
		{
			if ((verInfo.m_nMajor == 2) && (verInfo.m_nMinor <= 7))
			{
				strTmp = "Software\\Microplanet\\Gravity\\";
			}
			else
			{
				strTmp.Format("Software\\Microplanet\\Gravity %d.%d\\", verInfo.m_nMajor, verInfo.m_nMinor);
			}

			// Check if this key exists?
			lRet = RegOpenKeyEx (HKEY_CURRENT_USER, strTmp+"Storage\\", 0, KEY_READ, &hKey);
			if (ERROR_SUCCESS == lRet)
			{
				// Woo! Found something. Read the database storage location
				dwLen = sizeof (temp);
				if (ERROR_SUCCESS == RegQueryValueEx (hKey, "DatabasePath",
					LPDWORD (0), &type, (LPBYTE) temp, &dwLen))
				{
					strPathToTest = temp;

					//
					// IF we are running on Vista or Win 7 (or anything later, maybe)
					// we must check if this DB is in Program Files.
					//
					// If this path DOES point at Program Files AND UAC is enabled
					// then the DB is actually in this users virtual store, so check
					// that first.
					
					// If we can't find anything in the users virtual store, then
					// check the actual Program Files as they might have installed it
					// with UAC disabled and then enabled UAC.

					bool bVistaOrWindows7 = false;
					bool bUACEnabled = GetUACEnabled();
					bool bDBInProgramFiles = false;

					int nOS = GetOS();
					if (nOS == RUNNING_ON_VISTA || nOS == RUNNING_ON_WIN7)
						bVistaOrWindows7 = true;
					if (strPathToTest.Find("Program Files") != -1)
						bDBInProgramFiles = true;

					// Should we check the VirtualStore first?
					if (bVistaOrWindows7 && bUACEnabled && bDBInProgramFiles)
					{
						// Yep, check it...
						// Get path to users virtual store
						CString strPath = TFileUtil::GetLocalAppData();

						if (!strPath.IsEmpty())
						{
							strPath += "\\VirtualStore\\"
								+ strPathToTest.Mid(strPathToTest.Find("Program Files"));
							if (_access(strPath, 4) == 0)
							{
								// It exists.
								strDatabase = strPath;
								strRegKey = strTmp+"Servers";
								bRV = true;
							}
						}
					}

					// If we haven't found the DB yet, check the actual path
					if (!bRV && _access(strPathToTest, 4) == 0)
					{
						// It exists.
						strDatabase = strPathToTest;
						strRegKey = strTmp+"Servers";
						bRV = true;
					}
				}
				RegCloseKey (hKey);
			}

			// If we got back to V 2.7 then give up, they all use same location so there can't be any earlier versions
			if ((verInfo.m_nMajor == 2) && (verInfo.m_nMinor <= 7))
			{
				verInfo.m_nMajor = 0;
				verInfo.m_nMinor = 0;
				break;
			}

			verInfo.m_nMinor--;
		}
		verInfo.m_nMajor--;
		verInfo.m_nMinor = 99;
	}

	return bRV;
}


//
// Two funcs that copy settings and the DB from an old installation.
// Used if one is detected at installation time.
//

//
// Copies all the registry data from strRegKeyToImport to our reg key.
//
bool CNewsApp::ImportSettingsFromOldInstallation(CString strRegKeyToImport)
{
	// strRegKeyToImport is the reg key containing the server declarations to copy

	// Location to copy to
	CString strDestRegKey(GetGravityRegKey());

	// Create the Microplanet key
	LONG lRet;
	HKEY hKeyMP;
	DWORD dwDisposition = 0;
	const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;
	lRet = RegCreateKeyEx(HKEY_CURRENT_USER, GetGravityRegKey(),
		(DWORD) NULL,_T("prefdata"), REG_OPTION_NON_VOLATILE, uAccess,
		NULL, &hKeyMP, &dwDisposition);
	if (ERROR_SUCCESS != lRet)
	{
		CString msg; msg.Format(IDS_BRANCH_ERR1, lRet);
		NewsMessageBox(AfxGetMainWnd(), msg, MB_OK | MB_ICONSTOP);
		ASSERT(FALSE);
		return false;
	}
	RegCloseKey(hKeyMP);

	// Copy all settings in registry
	if (!CopyRegistryBranch(strRegKeyToImport, strDestRegKey))
	{
		//TRACE("ERROR: CopyRegistryBranch returned error.\n");
		AfxMessageBox("Database import failed.\nCould not copy server details from registry.\n\nPlease retry.", MB_ICONHAND|MB_OK);
		ASSERT(FALSE);
		return false;
	}

	return true;
}

bool CNewsApp::ImportDatabaseFromOldInstallation(CString strDBToImport, CString strRegKeyToImport)
{
	if (!ImportDatabaseFiles(strDBToImport, false, false))
		return false;

	// Fixup all absolute paths in the registry data just imported
	return FixDatabaseValues();
}


//
// CheckFixupAndMergeRegFile
//
// This func firstly checks the reg file for invalid reg keys.
// Then fixes up any old paths so they point to our db folder.
// And also fixes up the news server nickname if required.
// It then merges the reg file into the registry.
//
bool CNewsApp::CheckFixupAndMergeRegFile(
	CString strRegFileToImport, 
	bool bRenameServer, 
	CString strImportServerName,
	CString strNewServerKey)
{
	TRACE("CNewsApp::CheckFixupAndMergeRegFile >\n");
	if (!FixupRegFile(strRegFileToImport, bRenameServer, strImportServerName, strNewServerKey))
	{
		TRACE("CNewsApp::CheckFixupAndMergeRegFile : FixupRegFile failed '%s'\n", strRegFileToImport);
		ASSERT(FALSE);
		return false;
	}

	// Enter into registry
	if (!MergeRegFile(strRegFileToImport))
	{
		TRACE("CNewsApp::CheckFixupAndMergeRegFile : MergeRegFile failed '%s'\n", strRegFileToImport);
		ASSERT(FALSE);
		return false;
	}

	if (!FixDatabaseValues())
	{
		TRACE("CNewsApp::CheckFixupAndMergeRegFile : FixDatabaseValues failed\n");
		ASSERT(FALSE);
		return false;
	}

	TRACE("CNewsApp::CheckFixupAndMergeRegFile succeeded <\n");
	return true;
}

//
// Checks that all hive keys are HKCU\Software\Microplanet
//
bool CNewsApp::CheckRegFile(CString strRegFileToCheck)
{
	if (!WaitForFile(strRegFileToCheck))
	{
		ASSERT(FALSE);
		return false;
	}
	// Go through the registry file manually changing the Gravity key to our own key
	CStdioFileEx fInput;
	if (!fInput.Open(strRegFileToCheck, CFile::modeRead|CFile::shareExclusive|CFile::typeText))
	{
		ASSERT(FALSE);
		return false; // failed
	}

	bool bHacked = false;
	CString strLine;

	// Search for any root hive keys that are not HKCU
	while(!bHacked && fInput.ReadString(strLine))
	{
		if (strLine.Find("[HKEY_", 0) != -1)
			if (strLine.Find("[HKEY_CURRENT_USER\\Software\\Microplanet\\", 0) == -1)
				bHacked = true;
	}

	fInput.Close();

	return !bHacked;
}

//
// Fixes up a number of things in a registry file.
// It creates a new reg file with the fixes in, then deletes the
// original and renames the new to the old.
//
// Firstly it updates the "Gravity x.x" and "newsdbx.x" values
// to our current value.
//
// Next, if bRenameServer is true, we scan through and update
// "NewsServer" values to use the new server nickname.
//
// Additionally we fix the bug in the CustomHeaders where V2.9 saved
// multiple custom headers as seperate lines in the reg file. We can't
// import these directly so need to fix them up.
//
// Lastly we get rid of anything that is created by the Windows
// Installer because we do not want to corrupt this installation
// with the values from the install that exported the GDX.
//
bool CNewsApp::FixupRegFile(CString strRegFileToImport,
							bool bRenameServer,
							CString strImportServerName,
							CString strNewServerKey)
{
	TRACE("CNewsApp::FixupRegFile >\n");
	if (!WaitForFile(strRegFileToImport))
	{
		ASSERT(FALSE);
		return false;
	}
	// Go through the registry file manually changing the Gravity key to our own key
	CStdioFileEx fInput;
	CStdioFileEx fOutput;

	CString strDest;
	make_temp_filename(strDest);

	// If we are running on Win Vista or Win 7 we need to change "Documents and Settings..." to "Users..."
	bool bUpgradeDocsAndSettings = false;
	OSVERSIONINFO osVerInfo = {0};
	osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&osVerInfo);
	if (osVerInfo.dwMajorVersion >= 6)
		bUpgradeDocsAndSettings = true;

	if (!fInput.Open(strRegFileToImport, CFile::modeRead|CFile::shareExclusive|CFile::typeText))
	{
		TRACE("CNewsApp::FixupRegFile : Unable to open input registry file '%s'\n", strRegFileToImport);
		ASSERT(FALSE);
		return false; // failed
	}

	if (!fOutput.Open(strDest, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeText))
	{
		// Error
		fInput.Close();
		TRACE("CNewsApp::FixupRegFile : Unable to open/create new output registry file '%s'\n", strDest);
		ASSERT(FALSE);
		return false;
	}

	// Temp string
	CString strLine;

	// We do NOT want to import the "InstallDir" as that is created by the installer.
	// Ditty for anything that is in a section ending with "}]" we cannot import as
	// those sections are created by the installer.
	CString strSkip1("\"installdir\"=");
	CString strSectionSkip1("}]");

	bool bReplaceVersion = false;
	bool bReplaceStore = false;
	CString strNewKey(GetGravityRegKey()), strOldKey("");
	strNewKey.TrimRight("\\");
	CString strNewStore(GetGravityNewsDBDir()), strOldStore("newsdb");
	strNewStore.TrimRight("\\");
	CString strOldServerNickname; // New server nickname is strImportServerName
	CString strOldDandS, strNewDandS(TFileUtil::GetAppData());
	strNewDandS.Replace("\\", "\\\\");

	// We only want the "gravity x.x" bit of it, strip off the "software\\microplanet\\" part.
	strNewKey.MakeLower();
	strNewKey.Replace("software\\microplanet\\", "");
	// Get the reg key from the reg file
	while(fInput.ReadString(strLine))
	{
		strLine.MakeLower();
		if (strLine.Find("[hkey_current_user", 0) != -1)
		{
			// Again we only want the "gravity x.x" part
			strLine.Replace("[hkey_current_user\\software\\microplanet\\", "");
			// If there is anything after the "gravity x.x" part chop it off
			if (strLine.Find("\\"))
				strLine = strLine.Left(strLine.Find("\\"));
			strOldKey = strLine;
			strOldKey.MakeLower();
			break;
		}
	}
	if (strNewKey.CompareNoCase(strOldKey) != 0)
	{
		bReplaceVersion = true;
		TRACE("CNewsApp::FixupRegFile : Updating version from " + strOldKey + " to " + strNewKey + "\n");
	}
	// Rewind the input file
	fInput.SeekToBegin();

	// Ditto for the directory the news DB is stored in
	if (strOldKey.ReverseFind(' ') != -1)
	{
		strOldStore += strOldKey.Mid(strOldKey.ReverseFind(' ')+1);
		strOldStore.MakeLower();

		if (strNewStore.CompareNoCase(strOldStore) != 0)
		{
			bReplaceStore = true;
			TRACE("CNewsApp::FixupRegFile : Updating store from " + strOldStore + " to " + strNewStore + "\n");
		}
	}

	if (bRenameServer)
	{
		// Get the old server nickname
		while(fInput.ReadString(strLine))
		{
			strLine.MakeLower();
			if (strLine.Find("servers\\server") != -1)
			{
				fInput.ReadString(strLine);
				strLine.TrimRight("\"");
				if (strLine.ReverseFind('\\') != -1)
				{
					strOldServerNickname = strLine.Mid(strLine.ReverseFind('\\')+1);
					strOldServerNickname.MakeLower();
				}
				break;
			}
		}
		// Rewind the input file
		fInput.SeekToBegin();

		TRACE("CNewsApp::FixupRegFile : Updating server nickname from " + strOldServerNickname + " to " + strImportServerName + "\n");
	}

	if (bUpgradeDocsAndSettings)
	{
		while(fInput.ReadString(strLine))
		{
			strLine.MakeLower();
			if (strLine.Find("documents and settings") != -1)
			{
				if (strLine.Find("\\gravity\\") != -1)
				{
					strLine = strLine.Left(strLine.Find("\\gravity\\"));
					if (strLine.ReverseFind('\"') != -1)
					{
						strOldDandS = strLine.Mid(strLine.ReverseFind('\"')+1);
						strOldDandS.MakeLower();
						strOldDandS.TrimRight("\\");
					}
				}
				break;
			}
		}
		// Rewind the input file
		fInput.SeekToBegin();

		TRACE("CNewsApp::FixupRegFile : Updating DandS from " + strOldDandS + " to " + strNewDandS + "\n");
	}

	// Check if we actually have any work to do?
	if (!bRenameServer && !bUpgradeDocsAndSettings && !bReplaceVersion && !bReplaceStore)
	{
		TRACE("CNewsApp::FixupRegFile succeeded (nothing to fix) <\n");
		return true;
	}

	// bReplaceVersion == All the key lines need checked for the version

	// bReplaceStore == The following lines need checked for the store:-
	// Decode (in Dirs key)
	// DatabasePath (in Server section and Storage section)
	// ImportFile (")
	// ExportFile (")

	// bRenameServer == The following lines need the server "nickname" upgraded to strImportServerName:-
	// "MRU" = "%strImportServerName%"
	// DatabasePath (in server section - always points to the directory)
	// ImportFile (in server section - only needs upgrading if it points to the OLD server nickname)
	// ExportFile (")
	// NewsServer (in server section - always upgrade)
	//
	// Also the "Servers\server<ID>" must be upgraded to strNewServerKey.

	// bUpgradeDocsAndSettings == The following paths MAY need upgraded.
	// We only upgrade them IF they point to the olde stylee
	// "Documents and Settings":-
	// Decode (in Dirs key)
	// DatabasePath (in Server section and Storage section)
	// ImportFile (in server section)
	// ExportFile (")

	// All in all this func fixes quite a bit and is accordingly a bit convoluted :(

	// Some temp strings
	CString strLine2(""), strTmp(""), strSection(""), strCustomHeader("");

	// Define some strings, lower case and normal, to make things easier to read.
	CString strHKCUUpdate1("\\servers\\server");
	CString strUpdate1("\"databasepath\"="), strUpdate1U("\"DatabasePath\"=");
	CString strUpdate2("\"newsserver\"="), strUpdate2U("\"NewsServer\"=");
	CString strUpdate3("\"importfile\"="), strUpdate3U("\"ImportFile\"=");
	CString strUpdate4("\"exportfile\"="), strUpdate4U("\"ExportFile\"=");
	CString strUpdate5("\"decode\"="), strUpdate5U("\"Decode\"=");
	CString strUpdate6("\"mru\"="), strUpdate6U("\"MRU\"=");
	CString strCustomHeaders("customheaders");
	CString strDandS("documents and settings");

	// Flag that indicates we are in the middle of a section we are removing
	bool bSectionSkip = false;
	// Flag that indicates we are in the middle of reading in a broken CustomHeader
	// (they are broken because they're split over multiple lines)
	bool bReadingCustomHeader = false;
	bool bUseOriginalLine = true;

	while(fInput.ReadString(strLine))
	{
		strLine2 = strLine;
		strLine.MakeLower();
		if (!bReadingCustomHeader)
		{
			// A key?
			if (strLine.Find("[hkey_current_user", 0) != -1)
			{
				// A section we should skip?
				if (strLine.Right(2) == strSectionSkip1)
				{
					// Yep
					bSectionSkip = true;
					continue;
				}

				// Replace the version if required
				if (bReplaceVersion)
					strLine.Replace(strOldKey, strNewKey);

				// If renaming server, check for server key
				if (bRenameServer && (strLine.Find(strHKCUUpdate1) != -1))
					strLine = "["+GetGravityHKCURegKey()+"servers\\"+strNewServerKey+"]";
		
				fOutput.WriteString(strLine+"\r\n");
				strSection = strLine;
			}
			else // Nope, it's a value or a blank line then
			{
				// In the middle of skipping a section?
				if (bSectionSkip)
				{
					// Yes, test for end of section (blank line)
					if (strLine.IsEmpty())
						bSectionSkip = false;
					continue;
				}

				// A value we should skip?
				if (strLine.Find(strSkip1) != -1)
					continue; // Yes

				// Start of CustomHeaders?
				if (strLine.Find(strCustomHeaders) != -1)
				{
					// If the custom header line does NOT end in a double quote,
					// it must be split over multiple lines. Start reading it in.
					if (strLine.Right(1) != '"')
					{
						strCustomHeader = strLine2+"\r\n";
						bReadingCustomHeader = true;
						continue;
					}
				}
				else
				{
					// Ewww, this is ugly...
					if (   (bReplaceStore || bRenameServer || bUpgradeDocsAndSettings)
						&& (strLine.Find(strUpdate1) != -1)) // DatabasePath
					{
						if (bReplaceStore)
							strLine.Replace(strOldStore, strNewStore);
						if (bRenameServer)
							strLine.Replace(strOldServerNickname, strImportServerName);
						if (bUpgradeDocsAndSettings && (strLine.Find(strDandS) != -1))
							strLine.Replace(strOldDandS, strNewDandS);
						bUseOriginalLine = false;
					}
					if (bRenameServer && (strLine.Find(strUpdate2) != -1)) // NewsServer
					{
						strLine.Replace(strOldServerNickname, strImportServerName);
					}
					if (   (bReplaceStore || bRenameServer || bUpgradeDocsAndSettings)
						&& (strLine.Find(strUpdate3) != -1)) // ImportFile
					{
						if (bReplaceStore)
							strLine.Replace(strOldStore, strNewStore);
						if (bRenameServer)
							strLine.Replace(strOldServerNickname, strImportServerName);
						if (bUpgradeDocsAndSettings && (strLine.Find(strDandS) != -1))
							strLine.Replace(strOldDandS, strNewDandS);
						bUseOriginalLine = false;
					}
					if (   (bReplaceStore || bRenameServer || bUpgradeDocsAndSettings)
						&& (strLine.Find(strUpdate4) != -1)) // ExportFile
					{
						if (bReplaceStore)
							strLine.Replace(strOldStore, strNewStore);
						if (bRenameServer)
							strLine.Replace(strOldServerNickname, strImportServerName);
						if (bUpgradeDocsAndSettings && (strLine.Find(strDandS) != -1))
							strLine.Replace(strOldDandS, strNewDandS);
						bUseOriginalLine = false;
					}
					if (   (bReplaceStore || bUpgradeDocsAndSettings)
						&& (strLine.Find(strUpdate5) != -1)) // Decode
					{
						if (bReplaceStore)
							strLine.Replace(strOldStore, strNewStore);
						if (bUpgradeDocsAndSettings && (strLine.Find(strDandS) != -1))
							strLine.Replace(strOldDandS, strNewDandS);
						bUseOriginalLine = false;
					}
					if (bRenameServer && (strLine.Find(strUpdate6) != -1)) // MRU
					{
						strLine.Replace(strOldServerNickname, strImportServerName);
						bUseOriginalLine = false;
					}
				}
				if (bUseOriginalLine)
					fOutput.WriteString(strLine2+"\r\n"); // If we haven't changed anything, preserve the case
				else
					fOutput.WriteString(strLine+"\r\n"); // If we have changed anything, it's just a file path so screw the case :)
			}
		}
		else
		{
			strCustomHeader += strLine2;
			if (strLine.Right(1) == '"')
			{
				// Found end of CustomHeader line, fix it up.
				strCustomHeader.TrimRight("\r\n");

				strCustomHeader.Replace("\\", "\\\\");
				// Change "CR" to "C\\R"
				strCustomHeader.Replace("CR", "C\\R");
				// Change "LF" to "L\\F"
				strCustomHeader.Replace("LF", "L\\F");
				// Replace 0x0D with "CR"
				strCustomHeader.Replace("\r", "CR");
				// Replace 0x0A with "LF"
				strCustomHeader.Replace("\n", "LF");

				fOutput.WriteString(strCustomHeader+"\r\n");

				bReadingCustomHeader = false;
			}
			else
				strCustomHeader += "\r\n";
		}

		bUseOriginalLine = true;
	}

	fInput.Close();
	fOutput.Close();

	// Delete the original file and rename the fixed up file to the orig file
	_unlink(strRegFileToImport);
	rename(strDest, strRegFileToImport);

	WaitForFile(strRegFileToImport);

	TRACE("CNewsApp::FixupRegFile succeeded <\n");
	return true;
}


//
// Takes a filename of a registry file and merges it.
//
bool CNewsApp::MergeRegFile(CString strRegFileToImport)
{
	TRACE("CNewsApp::MergeRegFile >\n");
	if (!WaitForFile(strRegFileToImport))
	{
		ASSERT(FALSE);
		return false;
	}
	TRegImport importer(strRegFileToImport);

	bool bRV = importer.Import();
	if (bRV)
		TRACE("CNewsApp::MergeRegFile : Succeeded <\n");
	else
		TRACE("CNewsApp::MergeRegFile : Failed <\n");
	return bRV;
}



//
// ImportDatabaseFiles - basically copies all the files in folder
// strDBToImport to the current newsdb folder after clearing current
// newsdb folder out or backing it up.
//
// This func can handle GDX and GSX files.
// strDBToImport = path to newsdb dir in temp directory - everything IN this dir is copied into newsdb
// bBackupExisting = set to true to rename the current newsdb folder rather than delete it.
//
bool CNewsApp::ImportDatabaseFiles(CString strDBToImport, bool bBackupExisting, bool bServer)
{
	if (!WaitForFile(strDBToImport))
	{
		ASSERT(FALSE);
		return false;
	}

	strDBToImport.TrimRight('\\');

	// Location to copy to
	CString strDestPath = TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir();

	// If we want to, rename the current DB directory
	if (bBackupExisting)
	{
		CString strBackupDirName(strDestPath);
		strBackupDirName.TrimRight('\\');

		// We want to restrict backups to only one, so search for any existing backups and delete them first
		WIN32_FIND_DATA findData;
		HANDLE hBackupToDelete = FindFirstFile(strBackupDirName+"*-backup", &findData);
		if (hBackupToDelete != INVALID_HANDLE_VALUE)
		{
			FindClose(hBackupToDelete);
			TrashDirectory(TFileUtil::GetAppData()+"\\Gravity\\"+findData.cFileName);
		}

		COleDateTime dtNow;
		dtNow = COleDateTime::GetCurrentTime();
		strBackupDirName += dtNow.Format("-%H_%M_%S_%d_%m_%y-backup");

		if (!MoveFile(strDestPath, strBackupDirName))
		{
			// Error
			ASSERT(FALSE);
			return false;
		}
		else
		{
			WaitForFile(strBackupDirName);

			if (_mkdir(strDestPath) == -1)
			{
				// Error
				ASSERT(FALSE);
				return false;
			}
		}
	}
	else
	{
		if (!bServer)
		{
			// Not backing up, so wipe out everything in the newsdb folder
			if (TrashDirectory(strDestPath))
			{
				if (_mkdir(strDestPath) == -1)
				{
					// Error
					ASSERT(FALSE);
					return false;
				}
				else
				{
					if (!WaitForFile(strDestPath))
					{
						ASSERT(FALSE);
						return false;
					}
				}
			}
			else
			{
				// Error
				ASSERT(FALSE);
				return false;
			}
		}
	}

	// Copy the db files from source dir to current dir
	if (!CopyDirectory(strDBToImport, strDestPath, true))
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}


//
// This func looks though a reg file for certain details.
//
// Used by the News Server Import functions to retreive the server
// nickname, NNTP server name and user name used for authentication.
//
bool CNewsApp::ParseRegFileForServerDetails(CString strRegFile, CString &strImportServerName, CString &strImportServerRealName, CString &strUserName)
{
	if (!WaitForFile(strRegFile))
	{
		ASSERT(FALSE);
		return false;
	}

	CStdioFileEx fInput;

	if (!fInput.Open(strRegFile, CFile::modeRead|CFile::shareExclusive|CFile::typeText))
	{
		ASSERT(FALSE);
		return false; // failed
	}

	CString strLine, strTmp;
	strImportServerName = "";
	strImportServerRealName = "";
	strUserName = "";

	while((strImportServerName.IsEmpty() || strImportServerRealName.IsEmpty() || strUserName.IsEmpty()) && fInput.ReadString(strLine))
	{
		if (strLine.Find("\"NewsServer\"=", 0) != -1)
		{
			strImportServerName = strLine.Mid(strLine.Find("=")+1);
			strImportServerName.Trim("\"");
		}
		else if (strLine.Find("\"NewsServerAddr\"=", 0) != -1)
		{
			strImportServerRealName = strLine.Mid(strLine.Find("=")+1);
			strImportServerRealName.Trim("\"");
		}
		else if (strLine.Find("\"NewsAccount\"=", 0) != -1)
		{
			strUserName = strLine.Mid(strLine.Find("=")+1);
			strUserName.Trim("\"");
		}
	}

	fInput.Close();

	return (!(strImportServerName.IsEmpty() || strImportServerRealName.IsEmpty() || strUserName.IsEmpty()));
}

//
// If a news server is currently open this func closes it down
// and remembers the server name.
//
bool CNewsApp::StopDBActivity()
{
	TRACE("CNewsApp::StopDBActivity >\n");
	GetCountedActiveServerName(m_strStoppedServerName);
	CNewsView *pView = GetNewsView();
	if (pView)
	{
		if (GetNewsView()->GetDocument())
		{
			GetNewsView()->GetDocument()->UpdateAllViews(NULL, VIEWHINT_EMPTY);
			TMyWords *pMyWords = GetNewsView()->GetDocument()->m_pMyWords;
			GetNewsView()->GetDocument()->m_pMyWords = NULL;
			if (pMyWords)
			{
				delete GetNewsView()->GetDocument()->m_pMyWords;
			}
		}
		GetNewsView()->CloseCurrentNewsgroup();
		GetNewsView()->EmptyListbox();
	}
	BOOL bRV = ExitServer();

	// I can't simply close down the DBStore without having to either restert Gravity or doing a shed load of work,
	// all I want it to do is save the Bozo bin and Watched / Ignore files, so call those funcs directly.
	if (gpStore)
		gpStore->SaveData();

	if (!((CMainFrame*)AfxGetMainWnd())->QuietOnlineStop())
		bRV = FALSE;

	// Whilst we were stopping things, the worker threads may have sent windows messages.
	// If they did, those messages will be sitting there waiting to fire, which they will
	// do as soon as we display a dialog
	//
	// Unfortunately, at that point the DB server has been closed down, as has the tasker
	// so lots of things can fall over.
	//
	// Simple solution is to eat all windows USER messages here.
	MSG msg;
	while (PeekMessage(&msg, NULL, WM_USER, WM_APP, PM_REMOVE))
	{
		if (WM_QUIT == msg.message)
			break;
	}

	TRACE("CNewsApp::StopDBActivity <\n");
	return bRV == TRUE;
}

//
// If StopDBActivity closed down a news server,
// we open it back up here.
//
bool CNewsApp::StartDBActivity()
{
	if (GetNewsView() && GetNewsView()->GetDocument())
	{
		GetNewsView()->GetDocument()->m_pMyWords = new TMyWords();
	}
	if (!m_strStoppedServerName.IsEmpty())
		return (SwitchServer(m_strStoppedServerName) == 0);
	else
		return true;
}


	// Create the Microplanet key
	//LONG lRet;
	//HKEY hKeyMP;
	//DWORD dwDisposition = 0;
	//const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;
	//lRet = RegCreateKeyEx (HKEY_CURRENT_USER, GetGravityRegKey()+"Servers",
	//	(DWORD) NULL,_T("prefdata"), REG_OPTION_NON_VOLATILE, uAccess,
	//	NULL, &hKeyMP, &dwDisposition);
	//if (ERROR_SUCCESS != lRet)
	//{
	//	ASSERT(FALSE);
	//	CString msg; msg.Format(IDS_BRANCH_ERR1, lRet);
	//	NewsMessageBox (AfxGetMainWnd(), msg, MB_OK | MB_ICONSTOP);
	//	return false;
	//}
	//RegCloseKey(hKeyMP);
