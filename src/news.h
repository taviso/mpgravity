/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: news.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.7  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.6  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.5  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.4  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.3  2009/06/14 20:47:36  richard_wood
/*  Vista/Win7 compatible.
/*  Multi user.
/*  Multi version side-by-side.
/*  Import/Export DB & settings.
/*  Updated credits file for the UFT CStdioFileEx and compression code.
/*
/*  Revision 1.2  2009/06/14 13:17:22  richard_wood
/*  Added side by side installation of Gravity.
/*  Adding (WORK IN PORGRESS!!!) DB export/import.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

// News.h : main header file for the NEWS application
//

#pragma once

#include "appversn.h"            // Gravity versions  OEM, LITE, KISS

/////////////////////////////////////////////////////////////////////////////

#include "mplib.h"
#include "resource.h"       // main symbols
#include "declare.h"
#include "NEWS_i.h"

class CNewsDoc;
class TOperationPage;
class TPreferencesPage;
class TNewsCommandLineInfo;

/////////////////////////////////////////////////////////////////////////////
// CNewsApp:
// See News.cpp for the implementation of this class
//

class CNewsApp : public CWinApp
{
public:
	static  void AddToNewsgroupList( TNewsGroup * pGroup );
	static  BOOL InNewsgroupList ( const CString & groupName );
	static  BOOL fnAutoCycleWarning (TOperationPage * ppgOperate, TPreferencesPage * ppgPrefs);

	CNewsApp();
	~CNewsApp(void);

	HWND       AccessSafeFrameHwnd (BOOL fLock);
	void       ZeroSafeFrameWnd ();
	TPopFrame* GetViewFrame()                      { return m_pViewFrame; }
	void       SetViewFrame(TPopFrame* pViewFrame) { m_pViewFrame = pViewFrame; }
	void       ZeroViewFrame(void) { m_pViewFrame = 0; }

	CMultiDocTemplate * InitializeDocumentTemplates();
	TViewwndTemplate * GetViewwndTemplate(void) { return m_pViewwndTemplate; }
	TPostTemplate * GetPostTemplate() { return m_pPostTemplate; }
	TPostTemplate * GetFollowTemplate() { return m_pFollowTemplate; }
	TPostTemplate * GetReplyTemplate() { return m_pReplyTemplate; }
	TPostTemplate * GetForwardTemplate() { return m_pForwardTemplate; }
	TPostTemplate * GetBugTemplate () {return m_pBugTemplate;}
	TPostTemplate * GetSuggestionTemplate () {return m_pSuggestionTemplate;}
	TPostTemplate * GetSendToFriendTemplate () {return m_pSendToFriendTemplate;}
	TPostTemplate * GetMailToTemplate () {return m_pMailToTemplate;}

	BOOL       Registered();
	BOOL       CheckSerial ();
	int        Run ();
	void       ReceiveArticle (TFetchArticle* pFetch);
	int        ForceSwitchServer (const CString & server);
	void       BuildNewsgroupList(BOOL fIncremental, TNewsServer* pServer, TNewsTasker* pTasker);
	CNewsDoc*  GetGlobalNewsDoc(void);
	void       SetGlobalNewsDoc(CNewsDoc* pNewsDoc);
	BOOL       AcceptLicense();
	BOOL       GetSpec_SaveArticle(CWnd* pAnchorWnd, CString* pFileName, CFile * pFile);
	BOOL       CheckConnectionSettings();
	BOOL       ExitServer();

	virtual int ExitInstance();

	bool StopDBActivity();
	bool StartDBActivity();

protected:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual BOOL InitApplication();
	afx_msg void OnAppAbout();
	afx_msg void OnOptionsConfigure();
	afx_msg void OnOptionsRules();
	afx_msg void OnOptionsManualRule();
	afx_msg void OnHelpRegistration();
	afx_msg void OnOptionsEditBozo();
	afx_msg void OnOptionsWords();
	afx_msg void OnDialDialNow();
	afx_msg void OnDialHangup();
	afx_msg void OnWatch();
	afx_msg void OnIgnore();
	afx_msg void OnServerAddRemove();
	afx_msg void OnServerOptions();
	afx_msg void OnServerMenuItem(UINT nID);
	afx_msg void OnUpdateServerMenuItem(CCmdUI* pCmdUI);
	afx_msg void OnImportdatabase();
	afx_msg void OnExportdatabase();
	afx_msg void OnExportnewsserver();
	DECLARE_MESSAGE_MAP()

	bool AbortSecondInstance (TNewsCommandLineInfo& sCmdLineInfo);
	BOOL FirstInstance(CWnd*& rpMainWnd);
	int  FreeNamedSemaphoreAndExit();
	int  SpawnChildInstanceAndExit();
	int  InitDDEServer (BOOL fConnect);
	BOOL InitServerByName (LPCTSTR   pszServer, TNewsServer*& rpServer);
	BOOL InitServerUI (TNewsServer* pServer, CWnd* pMainWnd);
	int  SwitchServer (const CString & server);
	BOOL ServerNameFromMenuID (UINT nID, CString & serverName);
	bool initialize_sockets ();
	int  create_first_server ();
	void init_more(BOOL fDBExists, BYTE& fFirstUse, bool& rfSuccess);
	void init_ui_phase (TNewsServer* pServer, bool& rfSuccess);


	DWORD      m_InitFlags;
	CRITICAL_SECTION m_csIndirectFrame;
	HWND             m_hwndIndirectFrame;

	TPopFrame* m_pViewFrame;
	CNewsDoc*  m_pNewsDoc;
	COleTemplateServer m_server;

private:
	BOOL InitATL();

	// Funcs used to import DB and settings from an old installation
	bool CheckForOldDatabase(CString &strDatabase, CString &strRegKey);
	bool ImportSettingsFromOldInstallation(CString strRegKeyToImport); // Import all the settings from old version
	bool ImportDatabaseFromOldInstallation(CString strDBToImport, CString strRegKeyToImport);

	bool ImportDatabase(CString strSourceFile, bool bReportErrors);
	bool ExportDatabase(CString strDestFile, bool bReportErrors, bool bNoShutdownRestart);
	bool ExportNewsServer(TNewsServer *pNewsServerToExport, CString strDestFile, bool bReportErrors);
	bool RestoreTempBackup(CString strSourceFile);

	bool ImportDatabaseFiles(CString strDBToImport, bool bBackupExisting, bool bServer);
	
	bool ImportGDX(CString strSourceDir);
	bool ImportGSX(CString strSourceDir, bool bReportErrors);

//	bool ImportSettingsFromFile(CString strFileToImport, bool bJustCheck, CString strOrigImportServerName, CString strImportServerName);
	bool ParseRegFileForServerDetails(CString strRegFile, CString &strImportServerName, CString &strImportServerRealName, CString &strUserName);

	bool CheckFixupAndMergeRegFile(
		CString strRegFileToImport, 
		bool bRenameServer, 
		CString strImportServerName,
		CString strNewServerKey);
	bool CheckRegFile(CString strRegFileToCheck);
	bool FixupRegFile(CString strRegFileToImport, bool bRenameServer, CString strImportServerName, CString strNewServerKey);
	bool MergeRegFile(CString strRegFileToImport);

	BOOL m_bATLInited;
	BOOL m_fCreateStore;

	CString m_strStoppedServerName;

	CMultiDocTemplate*   m_pBrowserTemplate;
	TPostTemplate *      m_pPostTemplate;
	TPostTemplate *      m_pFollowTemplate;
	TPostTemplate *      m_pReplyTemplate;
	TPostTemplate *      m_pForwardTemplate;
	TPostTemplate *      m_pBugTemplate;
	TPostTemplate *      m_pSuggestionTemplate;
	TPostTemplate *      m_pSendToFriendTemplate;
	TPostTemplate *      m_pMailToTemplate;
	TPopwndTemplate *  m_pPopwndTemplate;
	TViewwndTemplate *   m_pViewwndTemplate;
};

inline CNewsDoc* CNewsApp::GetGlobalNewsDoc()
{ return m_pNewsDoc; }

inline void      CNewsApp::SetGlobalNewsDoc(CNewsDoc* pNewsDoc)
{ m_pNewsDoc = pNewsDoc; }

extern CNewsApp * gptrApp;
