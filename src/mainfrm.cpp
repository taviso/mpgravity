/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mainfrm.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.9  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.8  2010/01/07 17:35:51  richard_wood
/*  Updates for 2.9.14.
/*
/*  Revision 1.7  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.6  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.5  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.4  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.3  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.17  2009/05/08 15:16:02  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.16  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.15  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.14  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.13  2009/01/28 22:45:49  richard_wood
/*  Added border round article pane in main window.
/*  Cleaned up some memory leaks.
/*
/*  Revision 1.12  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.11  2009/01/27 10:39:46  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.10  2009/01/26 11:25:38  richard_wood
/*  Updated "Compress" icon.
/*  Updated "DejaNews" icon and URL to Google Groups.
/*  Added error message for invalid date time format string.
/*  Fixed function ID for Run Rule Manually button.
/*  Updated greyscale buttons to look a lot more ghost like (less contrast, easier to tell they're inactive)
/*
/*  Revision 1.9  2009/01/26 08:34:46  richard_wood
/*  The buttons are back!!!
/*
/*  Revision 1.8  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.7  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.6  2008/10/03 08:21:06  richard_wood
/*  Tidying up code and comments.
/*
/*  Revision 1.5  2008/09/19 14:51:30  richard_wood
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

// mainfrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <afxinet.h>

#include "News.h"

#include "mainfrm.h"
#include "usrdisp.h"
#include "custmsg.h"          // custom windows messages (WM_USER + ???)
#include "newsgrp.h"
#include "globals.h"
#include "tasker.h"
#include "mlayout.h"
#include "tnews3md.h"
#include "newsview.h"
#include "tglobopt.h"
#include "warndlg.h"
#include "tprndlg.h"          // TPrintDialog
#include "tdecdlg.h"          // TDecodeDialog
#include "tdecthrd.h"         // gpsDecodeDialog
#include "fileutil.h"
#include "tnews3md.h"
#include "rgwarn.h"
#include "log.h"
#include "outbxdlg.h"
#include "rtfspt.h"
#include "posttmpl.h"         // TPostTemplate
#include "ngutil.h"
#include "rgpurg.h"
#include "rgsys.h"
#include "getlist.h"
#include "bits.h"             // class THeapBitmap
#include "genutil.h"          // GetNewsView(), ...
#include "taskcomp.h"
#include "tcompdlg.h"
#include "rglaymdi.h"
#include "intvec.h"
#include "expire.h"
#include "server.h"           // TNewsServer
#include "servcp.h"           // TServerCountedPtr
#include "nglist.h"
#include "uipipe.h"           // TUIPipe
#include "newsdb.h"           // TNewsDBStats
#include "rulestat.h"         // TRuleStats
#include "newsrc.h"           // TExportNewsrc, ...
#include "ncmdline.h"         // TNewsCmdLineInfo
#include "gotoart.h"          // TGotoArticle
#include "newsurl.h"          // TNewsUrl
#include "regutil.h"
#include "rgswit.h"           // GetFilterBarPosInit(), ...
#include "tscoring.h"         // TScoringDlg, ...
#include "hints.h"            // VIEWHINT_SERVER_SWITCH
#include "gallery.h"          // ViewBinary()
#include "fltbar.h"           // TViewFilterBar
#include "utilerr.h"          // TNNTPErrorDialog
#include "dialman.h"          // RAS dialup manager
#include "dialmane.h"         // RAS dialup manager
#include "picksvr.h"          // GetServersDownloadInfo
#include "vcrdlg.h"
#include "vcrrun.h"
#include "trcdlg.h"              // TTraceDlg  socket trace dlg
#include "dlgQuickFilter.h"
#include "evtlog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

HWND  ghwndStatusBar;

extern BOOL gfShutdownRename;
extern TNewsCommandLineInfo sCmdLineInfo;

#define TOOLBAR_INFO_SECTION _T("Control-Bars")

extern TNewsTasker* gpTasker;
extern TGlobalOptions *gpGlobalOptions;
extern CNewsApp *gptrApp;
extern BYTE gfFirstUse;

// from afxpriv.h
#define WM_SETMESSAGESTRING 0x0362

#define TIMERID_RECONNECT  333
#define TIMERID_SPEEDOMETER  334
#define TIMERID_UPDATECHECK  335

//-------------------------------------------------------------------------
// show messages on the status line as RAS goes thru connection stages
//   pass in -1 to clear display
void fnRasStatusDisplay(int iStringID)
{
	CString s;
	if (-1 == iStringID)
		gpUserDisplay->SetText ( s, FALSE );
	else if (s.LoadString(iStringID))
		gpUserDisplay->SetText ( s, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

#define new DEBUG_NEW

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(IDD_STATBAR, OnViewStatusBar)
	ON_UPDATE_COMMAND_UI(IDD_STATBAR, OnUpdateViewStatusBar)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_SEARCH_SEARCH, OnSearchSearch)
	ON_COMMAND(ID_VIEW_EVENTLOG, OnViewEventlog)
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_CURRENT_PRINT_JOBS, OnCurrentPrintJobs)
	ON_COMMAND(ID_FILE_SERVER_CONNECT, OnFileServerConnect)
	ON_COMMAND(ID_FILE_SERVER_RECONNECT, OnFileServerReConnect)
	ON_COMMAND(ID_FILE_SERVER_RECONNECTDELAY, OnFileServerReConnectDelay)
	ON_UPDATE_COMMAND_UI(ID_FILE_SERVER_CONNECT, OnUpdateFileServerConnect)
	ON_COMMAND(ID_CURRENT_DECODE_JOBS, OnCurrentDecodeJobs)
//	ON_COMMAND(ID_HELP_HOWTOBUYNEWS32, OnHelpHowtobuynews32)
//	ON_COMMAND(ID_HELP_GETTINGTHEMOSTOUTOFNEWS32, OnHelpGettingthemostoutofnews32)
	ON_COMMAND(ID_OUTBOX, OnOutbox)
	ON_COMMAND(ID_DRAFTMSGS, OnDraftMsgs)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_SEARCH, OnUpdateSearchSearch)
	ON_COMMAND(ID_APP_FETCH_TAGGED, OnAppFetchTagged)
	ON_UPDATE_COMMAND_UI(ID_APP_FETCH_TAGGED, OnUpdateAppFetchTagged)
	ON_COMMAND(ID_ONLINE_STOP, OnOnlineStop)
	ON_COMMAND(ID_VIEW_RULE_BAR, OnToggleRuleBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RULE_BAR, OnUpdateViewRuleBar)
	ON_COMMAND(ID_VIEW_OUTBOX_BAR, OnToggleOutboxBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTBOX_BAR, OnUpdateViewOutboxBar)
	ON_COMMAND(ID_DATEPURGE_ALL, OnDatePurgeAll)
	ON_UPDATE_COMMAND_UI(ID_DATEPURGE_ALL, OnUpdateDatePurgeAll)
	ON_COMMAND(ID_NEWSGROUP_FILTERDISPLAY, OnChooseFilterDisplay)
	ON_COMMAND(ID_NEWSGROUP_DEFINEDISPLAYFILTER, OnDefineFilterDisplay)
	ON_COMMAND(ID_VIEW_FILTER_BAR, OnToggleFilterBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILTER_BAR, OnUpdateToggleFilterBar)
	ON_COMMAND(ID_UPDATE_NG_COUNTS, OnUpdateNgCounts)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_NG_COUNTS, OnUpdateUpdateNgCounts)
	ON_COMMAND(ID_VIEW_RULESTATS, OnViewRulestats)
//	ON_COMMAND(ID_HELP_TUTORIAL, OnHelpTutorial)
	ON_COMMAND(ID_NEWSRC_EXPORT, OnNewsrcExport)
	ON_COMMAND(ID_NEWSRC_IMPORT, OnNewsrcImport)
	ON_COMMAND(ID_IMAGE_GALLERY, OnImageGallery)
	//ON_COMMAND(IDM_TOOLBAR_CUSTOMIZE, OnToolbarCustomize)
	ON_COMMAND(ID_POPUP_CUSTOMIZE, OnToolbarCustomize)
	ON_COMMAND(ID_DEJANEWS, OnDejanews)
	ON_COMMAND(ID_SCORING, OnScoring)
	ON_UPDATE_COMMAND_UI(ID_RESCORE, OnUpdateRescore)
	ON_COMMAND(ID_RESCORE, OnRescore)
	ON_UPDATE_COMMAND_UI(ID_QUICK_SCORE, OnUpdateQuickScore)
	ON_COMMAND(ID_QUICK_SCORE, OnQuickScore)
	ON_COMMAND(ID_SCORE_COLORS, OnScoreColors)
	ON_UPDATE_COMMAND_UI(ID_APPLY_BOZO, OnUpdateApplyBozo)
	ON_COMMAND(ID_APPLY_BOZO, OnApplyBozo)
	ON_COMMAND(IDM_VCR, OnVcr)
	ON_COMMAND(IDM_SOCKET_TRACE, OnSocketTrace)
	ON_WM_TIMER()
	ON_WM_QUERYENDSESSION()
	ON_COMMAND(ID_TOGFILTER_SHOWALL, OnTogfilterShowall)
	ON_COMMAND(IDM_QUICKFILTER, OnQuickFilter)
	ON_COMMAND_RANGE(ID_BETA_TEST1, ID_BETA_TEST5, OnBetaTestAll)
	ON_WM_DESTROY()

	ON_MESSAGE(WMU_MODE1_HDRS_DONE,  OnMode1HeadersDone)
	ON_MESSAGE(WMU_NGROUP_HDRS_DONE, OnNgroupHeadersDone)
	ON_MESSAGE(WM_SETMESSAGESTRING,   OnSetMessageString)
	ON_MESSAGE(WMU_CHECK_UIPIPE, OnCheckUIPipe)
	ON_MESSAGE(WMC_DISPLAY_ARTCOUNT,   OnDisplayArtcount)
	ON_MESSAGE(WMU_REFRESH_OUTBOX, OnRefreshOutbox)
	ON_MESSAGE(WMU_REFRESH_DRAFT, OnRefreshDraft)
	ON_MESSAGE(WMU_PUMP_ARTICLE,   OnPumpArticle)
	ON_MESSAGE(WMU_PROCESS_PUMP_ARTICLE, OnProcessPumpArticle)
	ON_MESSAGE(WMU_NONBLOCKING_CURSOR, OnNonBlockingCursor)
	ON_MESSAGE(WMU_CONNECT_OK, OnConnectOK)
	ON_MESSAGE(WMU_GETLIST_DONE, OnGetlistDone)
	ON_MESSAGE(WMU_REFRESH_OUTBOX_BAR, OnRefreshOutboxBar)
	ON_MESSAGE(WMU_EXPIRE_ARTICLES, OnExpireArticles)
	ON_MESSAGE(WMU_ERROR_FROM_SERVER, OnErrorFromServer)
	ON_MESSAGE(WMU_FORCE_FILTER_CHANGE, OnForceFilterChange)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_MESSAGE(WMU_NEWSURL, OnNewsURL)
	ON_MESSAGE(WMU_SPLASH_GONE, OnSplashScreenGone)
	//   ON_MESSAGE(WMU_SAVEBARSTATE, OnSaveBarState)
	// Global help commands
	//ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP_INDEX, CMDIFrameWnd::OnHelpIndex)
	ON_COMMAND(ID_HELP_USING, CMDIFrameWnd::OnHelpUsing)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpIndex)
	ON_MESSAGE(WMU_DOCK_DIR_BAR_ON_RIGHT_OF_TOOLBAR, OnDockDirBarOnRightOfToolbar)
	ON_MESSAGE(WMU_PANIC_KEY, OnPanicKeyMsg)
	ON_MESSAGE(WMU_RESET_FACTORY_COLUMNS, OnResetFactoryColumns)
	ON_MESSAGE(WMU_SERVER_SWITCH, OnServerSwitch)
	ON_MESSAGE(WMU_INTERNAL_DISCONNECT, OnInternalDisconnect)
	ON_MESSAGE(WMU_LOW_SPACE, OnLowSpace)
	ON_MESSAGE(WMU_GROUPRENUMBERED_WARNING, OnGroupRenumbered)
	ON_MESSAGE(WMU_BOZO_CONVERTED, OnBozoConverted)
	///////ON_MESSAGE(WMU_UPDATE_CHECKCOMPLETE, OnUpdateCheckComplete)
	ON_MESSAGE(WMU_RASDIAL_EVENT, OnRasDialSuccess)
	ON_MESSAGE(WMU_VCR_GROUPDONE, OnVCRGroupDone)
	ON_MESSAGE(WMU_SHOW_CONNECT_RESULT,  OnDisplayConnectError)
	ON_MESSAGE(WMU_READYTO_RUN,  OnReadyToRun)
	ON_MESSAGE(WMU_QUICK_FILTER, OnQuickFilterApply)
	ON_COMMAND(ID_HELP_VISITTHEGRAVITYWEBSITE, OnHelpVisitTheGravityWebsite)
	ON_COMMAND(ID_HELP_CHECKFORUPDATE, CheckForUpdate)
	ON_MESSAGE(WMU_CHECKFORUPDATE, OnCheckForUpdate)
END_MESSAGE_MAP()

// RLW - This is the definitive list of all the menu options currently available in Gravity 2.7.1
//
// They are organised exactly as they appear in the menus. *
//
// This list includes the menu items from the "New Post"/"Followup" windows, and the popup menus
// from the newsgroup window, thread window, attachments, etc. Everything.
//
// * Note that if a menu command appears in the main menu and then appeared in another menu further
// down, the second (and third, fourth, etc) instances have been deleted. (the button will do the
// right action whatever window is open / has the focus.
//
// Items here that have a button in the current toolbar buttons have a entry in the table.
// Items that do NOT have a button (yet) are the commented out lines.
//
// So as you can see there's potentially ~250% more buttons we could have if we wanted to represent
// *every single* menu action as a button.

// This is created from G_AllToolButtons at startup
// (easy to do and means there's only one list to maintain)
static TBBUTTON BASED_CODE *G_AllRawButtons = NULL;

// The global list of all *available* buttons
static CToolBarInfo BASED_CODE G_AllToolButtons[] =
{
	// Each line has 7 entries:-
	// 1 : Index into toolbar bitmap for this button
	// 2 : Menu command ID of this buttons operation
	// 3 : button initial state - set to TBSTATE_ENABLED
	// 4 : button or seperator
	// 5 : 0
	// 6 : 0
	// 7 : Text description of button

	// File - these have been moved into other groups cause there were only three
    // "&Keep All Sampled Newsgroups", ID_KEEP_ALL_SAMPLED
    // "&Import NEWSRC File",         ID_NEWSRC_IMPORT
    // "&Export NEWSRC File",         ID_NEWSRC_EXPORT
    // "&Trace File...",              IDM_SOCKET_TRACE
    // "Pa&ge Setup...",              ID_FILE_PRINT_SETUP
    // "E&xit",                       ID_APP_EXIT

	// Server
	{{0,  ID_FILE_SERVER_CONNECT,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Connect/Disconnect"},
	{{43, ID_UPDATE_NG_COUNTS,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Update Server Counts"},
	{{49, ID_GETHEADERS_ALLGROUPS,              TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Retrieve Headers"},
	{{53, ID_APP_FETCH_TAGGED,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Retrieve Tagged Articles"},
    // "&Retrieve Tagged for Selected Groups", IDM_GETTAGGED_FORGROUPS
	{{2,  ID_ONLINE_STOP,                       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Stop Retrieving"},
    // "&Properties...\tCtrl+Shift+P", ID_SERVER_OPTIONS
	{{34, ID_SERVER_ADDREMOVE,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Edit Servers"},
	{{35, ID_DATEPURGE_ALL,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Purge and Compact Database"},

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Navigate
	{{52, ID_ARTICLE_SKIPNEXTUNREAD,            TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Skip to Next Unread"},
	{{59, ID_ARTICLE_SKIPNEXTUNREADINTHREAD,    TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Skip to Next Unread in Thread"},
	{{54, ID_ARTICLE_SKIPNEXTUNREADLOCAL,       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Skip to Next Unread Local"},
	{{51, ID_ARTICLE_VIEWNEXTUNREAD,            TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "View Next Unread"},
	{{58, ID_ARTICLE_VIEWNEXTUNREADINTHREAD,    TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "View Next Unread in Thread"},
	{{55, ID_ARTICLE_VIEWNEXTUNREADLOCAL,       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "View Next Unread Local"},
	{{60, ID_ARTICLE_PREVIOUS,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Previous"},
	{{24, ID_ARTICLE_NEXT,                      TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Next"},
    // "&More\tSpace",                ID_ARTICLE_MORE
	{{57, ID_BACKWARD,                          TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Backward"},
	{{56, ID_FORWARD,                           TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Forward"},

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Edit
	//{{15, ID_EDIT_CUT,                          TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Cut"}, // not on main menu
	{{16, ID_EDIT_COPY,                         TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Copy"},
	//{{17, ID_EDIT_PASTE,                        TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Paste"}, // not on main menu
	{{18, ID_FILE_PRINT,                        TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Print"},
    // "Select &All\tCtrl+A",         ID_EDIT_SELECT_ALL

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Search
	{{7,  ID_SEARCH_FIND,                       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Find"},
	{{30, ID_SEARCH_SEARCH,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Search"},
	{{40, ID_DEJANEWS,                          TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Search Google Groups"},

	// Newsgroup
	{{38, ID_NEWSGROUP_SUBSCRIBE,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Subscribe..."},
    // "&Unsubscribe",                IDR_NGPOPUP_UNSUBSCRIBE
	{{3, ID_GETHEADERS_MGROUPS,                 TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Retrieve Newsgroup Headers"},
    // "&Retrieve Limited Number of Headers...", ID_GETHEADER_LIMITED
    // "&Keep Sampled Newsgroup",     ID_KEEP_SAMPLED
	{{44, IDR_NGPOPUP_CATCHUPALLARTICLES,       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Group Read"},
	{{48, ID_NEWSGROUP_RECENT,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Get New Groups"},
	{{42, ID_NEWSGROUP_GETALL,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Re-read All Groups"},
    // "S&ort By...",                 ID_NEWSGROUP_SORT
    // "&Filter Display...",          ID_NEWSGROUP_FILTERDISPLAY, GRAYED
    // "Define Display Filter...",    ID_NEWSGROUP_DEFINEDISPLAYFILTER, GRAYED
    // "&Lock Filter",                ID_NEWSGROUP_PINFILTER, CHECKED
    // "QuickFilter...\tCtrl+Q",      IDM_QUICKFILTER
	{{37, IDR_NGPOPUP_PROPERTIES,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Newsgroup Properties"},

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Thread
    // "&Previous\tCtrl+P",           ID_THREAD_PREVIOUS
    // "&Next\tCtrl+N",               ID_THREAD_NEXT
    //POPUP "Mark Thread as"
	{{8,  ID_THREAD_CHANGETHREADSTATUSTO_READ,  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Thread Read"},
    //     "Unread",                      ID_THREAD_CHANGETHREADSTATUSTO_NEW
    //     "Important",                   ID_THREAD_CHANGETHREADSTATUSTO_IMPORTANT
    //     "Normal",                      ID_THREAD_CHANGETHREADSTATUSTO_NORMALIMP
    //     "Protected",                   ID_THREAD_CHANGETHREADSTATUSTO_PERMANENT
    //     "Deletable",                   ID_THREAD_CHANGETHREADSTATUSTO_DELETABLE
	{{22, ID_ADD_TO_WATCH,                      TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Watch Thread"},
	{{23, ID_ADD_TO_IGNORE,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Ignore Thread"},
    // "Expand\t+",                   ID_EXPAND_THREAD
    // "Collapse\t-",                 ID_COLLAPSE_THREAD
    // "&Expand All/Collapse All\tCtrl+T", ID_THREAD_EXPANDCOLLAPSE

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Article
	{{4,  ID_NEWSGROUP_POSTARTICLE,             TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Post Article"},
	{{5,  ID_ARTICLE_FOLLOWUP,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Followup Article"},
	{{6,  ID_ARTICLE_REPLYBYMAIL,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Reply to Article"},
	{{28, ID_ARTICLE_BOZO,                      TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Bozo Author"},
	{{26, ID_ARTICLE_TOGGLE_FULL_HDR,           TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Show Full Headers"},
	//{{33, IDM_DRAFT,                            TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Save Draft Article"},
    // "Mute &Quoted Text\tQ",        ID_ARTICLE_TOGGLE_QUOTEDTEXT, CHECKED
    // "Show Source",                 ID_ARTICLE_SHOW_SOURCE, CHECKED
    //POPUP "Mark as"
	{{9,  ID_ARTICLE_CHANGESTATUSTO_READ,       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Article Read"},
	{{31, ID_ARTICLE_CHANGESTATUSTO_NEW,        TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Article Unread"},
    //     "Important\tCtrl+R",           ID_ARTICLE_CHANGESTATUSTO_IMPORTANT
    //     "Normal\tCtrl+O",              ID_ARTICLE_CHANGESTATUSTO_NORMALIMP
	{{32, ID_ARTICLE_CHANGESTATUSTO_PERMANENT,  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Article Protected"},
    //     "Deletable\tCtrl+Shift+L",     ID_ARTICLE_CHANGESTATUSTO_DELETABLE
	{{39, ID_ARTICLE_DELETE_SELECTED,           TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Delete Article"},
	{{10, IDC_DECODE,                           TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Decode"},
    // "Decode To...",                ID_DECODE_TO
	{{11, IDC_VIEW_BINARY,                      TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "View Binary"},
    // "Pr&iority View\tCtrl+Shift+V", ID_PRIORITY_DECODE
    // "Manua&l Decode...\tCtrl+M",   IDC_MANUAL_DECODE
    // "Uns&cramble (ROT-13)",        ID_ARTICLE_ROT13
	{{25, ID_ARTICLE_VIEWNEXTLOCAL,             TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Next Local Body"},
	{{36, ID_ARTICLE_SAVE_AS,                   TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Save Article As"},
	{{29, ID_ARTICLE_TAGFOR_FETCH,              TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Un/Tag Article"},
	{{21, ID_ARTICLE_CANCELMSG,                 TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Cancel Article"},
    // "C&haracter Coding...",        ID_ARTICLE_CHARCODING

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// View
    // "&Toolbar",                    ID_VIEW_TOOLBAR
    // "&Status Bar",                 ID_VIEW_STATUS_BAR
    // "&Rule Bar",                   ID_VIEW_RULE_BAR
    // "Out&box Bar",                 ID_VIEW_OUTBOX_BAR
    // "&Filter Bar",                 ID_VIEW_FILTER_BAR
    // "&Customize toolbar...",       ID_POPUP_CUSTOMIZE
	{{12, ID_IMAGE_GALLERY,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Gallery"},
	{{41, ID_CURRENT_DECODE_JOBS,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Image Factory"},
    // "Outbo&x...",                  ID_OUTBOX
    // "&Draft Messages...",          ID_DRAFTMSGS
    // "&Print Jobs...",              ID_CURRENT_PRINT_JOBS
    // "&Event Log...",               ID_VIEW_EVENTLOG
    // "R&ule Statistics...",         ID_VIEW_RULESTATS
    // "&Watch List...",              ID_WATCH
    // "Ig&nore List...",             ID_IGNORE
	{{27, ID_ZOOM_PANE,                         TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Zoom In"},
    // "Newsgroup Pane\t1",           ID_VIEW_NEWSGROUPPANE
    // "Thread Pane\t2",              ID_VIEW_THREADPANE
    // "Article Pane\t3",             ID_VIEW_ARTICLEPANE

	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Tools
	{{14, ID_OPTIONS_CONFIGURE,                 TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Options"},
	{{13, ID_OPTIONS_RULES,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Rules"},
	{{45, ID_OUTBOX,                            TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Outbox"},
    // "Apply a Rule &Manually...\tCtrl+Shift+M", ID_OPTIONS_MANUAL_RULE
	{{50, ID_OPTIONS_MANUAL_RULE,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Apply Rule"},
    // "&Bozo Bin...",                ID_OPTIONS_EDIT_BOZO
    // "A&pply bozo bin to current group", ID_APPLY_BOZO
    // "&Important Words...",         ID_OPTIONS_WORDS
    // "&Scoring...\tCtrl+Shift+G",   ID_SCORING
    // "Score &Colors...",            ID_SCORE_COLORS
    // "R&e-score Current Group\tCtrl+Shift+I", ID_RESCORE
    // "&Add Scoring Entry...\tS",    ID_QUICK_SCORE
    // "&VCR...\tShift+V",            IDM_VCR

	//{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""}, // Separator

	// Help
    // "Gravity Help &Topics",        ID_HELP
    // "Tut&orial",                   ID_HELP_TUTORIAL
    // "&Using Help",                 ID_HELP_USING
    // "&Getting the Most Out of Gravity", ID_HELP_GETTINGTHEMOSTOUTOFNEWS32
    // "&About Gravity...",           ID_APP_ABOUT


	// Article pane popup menu
    // "&Post",                       ID_HELLO_POSTNEWARTICLE
    // "&Follow Up",                  ID_HELLO_POSTFOLLOWUP
    // "&Reply",                      ID_HELLO_MAILREPLY
    //POPUP "&Edit"
    //     "Select &All\tCtrl+A",         ID_EDIT_SELECT_ALL
    // "For&ward",                    IDR_ARTPOP_FORWARDBYMAIL
    // "Add &Important Word",         ID_ARTICLE_IMPWORD
    // "Repair URL",                  ID_ARTICLE_REPAIRURL
    // "Pr&operties...",              IDR_ARTPOP_CHOOSEFONT


	// New post window menu
    //POPUP "&File"
    //     "&Insert File...",             IDM_INSERT_TEXT_FILE
    //     "&Send\tCtrl+Enter",           IDM_POST_SEND
    //     "Save &Draft\tCtrl+D",         IDM_DRAFT
    //     "&Cancel\tEsc",                IDM_CANCEL
    //POPUP "&Edit"
    //     "&Undo\tCtrl+Z",               ID_EDIT_UNDO
    //POPUP "&Options"
	//{{19, IDM_COMPOSE_ATTACH,                   TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Attach File"},
    //     "Check &Spelling\tF7",         ID_CHECK_SPELLING
    //     "&Choose Newsgroups...",       IDM_CHOOSE_GROUP
    //     "&Wrap selected lines\tCtrl+W", ID_SELECTION_WRAP


	// Newsgroup popup menu
    // "&Open",                       IDR_NGPOPUP_OPEN
    // "&Verify Local Headers",       ID_VERIFY_HDRS
    // "&Post...",                    ID_POST_SELECTED
    // "Manually Apply a &Rule...",   IDC_NGPOPUP_MANUAL_RULE


	// Threadlist popup menu
	// RLW - hmmm, use these or the "Article pane popup menu" ones? The IDs should be the same anyway!
	//{{20, ID_THREADLIST_POSTNEWARTICLE,         TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Post New Article"},
    // "&Follow Up...",               ID_THREADLIST_POSTFOLLOWUP
    // "&Reply...",                   ID_THREADLIST_REPLYBYMAIL
    // "Forward...",                  ID_THREAD_FORWARD_SELECTED
    // "Save &As...",                 ID_THREAD_SAVE_SELECTED
    // "&Search...",                  ID_THREADLIST_SEARCH
    // "Pr&operties...",              ID_THREADLIST_PROPERTIES


	// Attachment popup menu
    // "&Add Attachment...",          IDM_POP_COMPOSE_ATTACH
    // "&Remove Attachment",          IDM_POP_COMPOSE_DETACH


	// Buttons that don't seem to be used on the current toolbar?

	//{{46,  ,                                    TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Outbox"}, // red exclamation - error sending?
	//{{47,  ,                                    TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Outbox"}, // blue arrow with paper - articles waiting to be sent?
};


//static TBBUTTON BASED_CODE *G_RawButtons = NULL;
//static CToolBarInfo BASED_CODE G_ToolButtons[] =
//{
//	{{0,  ID_FILE_SERVER_CONNECT,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Connect/Disconnect"},
//	{{2,  ID_ONLINE_STOP,                       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Disconnect"},
//	{{49, ID_GETHEADERS_ALLGROUPS,              TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Get Headers"},
//	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""},
//	{{34, ID_SERVER_ADDREMOVE,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Edit Servers"},
//	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""},
//	{{57, ID_BACKWARD,                          TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Backward"},
//	{{56, ID_FORWARD,                           TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Forward"},
//	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""},
//	{{4,  ID_NEWSGROUP_POSTARTICLE,             TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Post"},
//	{{5,  ID_ARTICLE_FOLLOWUP,                  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Followup"},
//	{{6,  ID_ARTICLE_REPLYBYMAIL,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Reply"},
//	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""},
//	{{30, ID_SEARCH_SEARCH,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Search"},
//	{{8,  ID_THREAD_CHANGETHREADSTATUSTO_READ,  TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Thread Read"},
//	{{44, IDR_NGPOPUP_CATCHUPALLARTICLES,       TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Mark Group Read"},
//	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""},
//	{{10, IDC_DECODE,                           TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Decode"},
//	{{11, IDC_VIEW_BINARY,                      TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "View"},
//	{{41, ID_CURRENT_DECODE_JOBS,               TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Image Factory"},
//	{{0,  0,                                    TBSTATE_ENABLED,  TBSTYLE_SEP,     0,  0},  ""},
//	{{13, ID_OPTIONS_RULES,                     TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Rules"},
//	{{14, ID_OPTIONS_CONFIGURE,                 TBSTATE_ENABLED,  TBSTYLE_BUTTON,  0,  0},  "Options"}
//};


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_MessagesSaved = 0;
	m_pOutboxDialog = new COutboxDlg;
	m_pDraftDialog = new TDraftsDlg;
	m_pGotoInfo = 0;
	m_bDoNotCompactDBOnShutdown = false;

	//m_pCheckingForUpdate = 0;
	m_pUpdateNotify = 0;

	// dynamic creation to cut down H file dependecies
	m_pViewFilterBar = new TViewFilterBar;

	InitializeCriticalSection (&m_csVCRDlg);
	m_hwVCRDlg = 0;
	m_iReconnectCount = 0;
	m_hTimerReconnect  = 0;
	m_iCountDownReconnect = 0;

	m_pQuickFilter = new DlgQuickFilter(this);
	m_pQuickFilterData = 0;

	// Create G_AllRawButtons from G_AllToolButtons
	G_AllRawButtons = new TBBUTTON[ELEM(G_AllToolButtons)];
	for (int i = 0; i < ELEM(G_AllToolButtons); i++)
	{
		G_AllRawButtons[i] = G_AllToolButtons[i].tbButton;
	}
}

CMainFrame::~CMainFrame()
{
	DeleteCriticalSection (&m_csVCRDlg);

	delete m_pViewFilterBar;
	delete m_pOutboxDialog;
	delete m_pDraftDialog;
	delete m_pGotoInfo;

	delete m_pQuickFilter;
	
	// Delete the array of raw button objects
	delete [] G_AllRawButtons;
}

//------------------------------------------------------------------
// Do we have to seave the state the first time through?
//------------------------------------------------------------------

BOOL ControlBarsExist ()
{
	HKEY cbars;
	if (ERROR_SUCCESS != UtilRegOpenKey (GetGravityRegKey()+_T("ControlBars-Bar0"),
		&cbars,
		KEY_ENUMERATE_SUB_KEYS))
		return FALSE;

	RegCloseKey (cbars);
	return TRUE;
}

// ------------------------------------------------------------------------
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	ghwndStatusBar = NULL;
	LogString ("mainframe start create");

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (-1 == setup_toolbar ())
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// start Status Bar stuff
	if (!m_wndStatusBar.Create(this) /*||
									 !m_wndStatusBar.SetIndicators(indicators,
									 sizeof(indicators)/sizeof(UINT)) */
									 )
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.InitializePanes();

	ghwndStatusBar = m_wndStatusBar.m_hWnd;

	EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM);//CBRS_ALIGN_ANY);

	// All toolbars initially dock to the top or bottom of the mainframe.
	// Once they're created they load their settings which may
	// override this to dock them where the user dragged them to.

	// outbox dialog bar
	if (!m_sOutboxBar.Create (this,
		IDD_OUTBOX_DIALOGBAR,
		CBRS_BOTTOM | WS_VISIBLE | CBRS_SIZE_DYNAMIC,
		4246))
	{
		TRACE0("Failed to create outbox dialog bar\n");
		return -1;
	}

	gpsOutboxBar = &m_sOutboxBar;
	gpsOutboxBar->UpdateDisplay ();

	// filter dialog bar == take over id of m_smallToolBar. LoadBarState already
	//  saved something out.
	if (!m_pViewFilterBar->Create (this,
		IDD_FILTER_DIALOGBAR,
		CBRS_TOP | WS_VISIBLE | CBRS_SIZE_DYNAMIC,
		4244))
	{
		TRACE0("Could not create Filter dialog bar\n");
		return -1;
	}

	// manual-rule dialog bar
	if (!m_sManualRuleBar.Create (this,
		IDD_MANUAL_RULE_DIALOGBAR,
		CBRS_BOTTOM | WS_VISIBLE | CBRS_SIZE_DYNAMIC,
		4245))
	{
		TRACE0("Failed to create manual-rule dialog bar\n");
		return -1;
	}

	m_sManualRuleBar.Initialize ();
	m_sManualRuleBar.UpdateRuleList ();
	gpsManualRuleBar = &m_sManualRuleBar;

	m_pViewFilterBar->Initialize ();

	// Enable docking to the top or bottom of the mainframe
	m_wndToolBar.EnableDocking (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	m_sManualRuleBar.EnableDocking (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	m_sOutboxBar.EnableDocking (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	m_pViewFilterBar->EnableDocking (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);

	// Do not force bars to goto top or bottom,
	// let them go wherever the user has put them.
	DockControlBar(&m_wndToolBar);
	DockControlBar(m_pViewFilterBar);
	DockControlBar(&m_sManualRuleBar);
	DockControlBar(&m_sOutboxBar);

	try
	{
		LogString("mainframe start load toolbar info");

		LoadBarState(TOOLBAR_INFO_SECTION);
		m_wndToolBar.RestoreState();

		LogString("mainframe end load toolbar info");
	}
	catch(...)
	{
		// catch weirdo GPF's
		NewsMessageBox(this, IDS_ERR_LOADTOOLBAR);
		return -1;
	}

	// If first time run, put filter bar to the right of toolbar
	if (gfFirstUse)
		OnDockDirBarOnRightOfToolbar(0,0);

	// global link to statusbar
	gpUserDisplay->SetWindow( m_hWnd, m_wndStatusBar.m_hWnd );

	LogString("mainframe end create");

	// get similar class for RICHEDIT
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	VERIFY(::GetClassInfo(AfxGetInstanceHandle(), _T("RICHEDIT"), &wc));
	wc.lpszClassName = _T("MICROPLANET_RICHEDIT");

	VERIFY(::RegisterClass (&wc) );

	SetTimer(TIMERID_SPEEDOMETER, 6000, NULL);
	SetTimer(TIMERID_UPDATECHECK, 5000, NULL);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// OnDockDirBarOnRightOfToolbar -- called once the first time the program
// runs, to set the initial position of the dir bar to the right of the toolbar
LRESULT CMainFrame::OnDockDirBarOnRightOfToolbar(WPARAM, LPARAM)
{
	//TRACE("OnDockDir\n");

	// get toolbar window
	CControlBar *pToolbar = &m_wndToolBar;

	CRect sToolbarRect;
	pToolbar->GetWindowRect (&sToolbarRect);
	int iToolbarHeight = sToolbarRect.Height();
	int iToolbarWidth = sToolbarRect.Width();

	// dock filter bar to right of toolbar
	CRect sRect;
	m_pViewFilterBar->GetWindowRect (&sRect);
	int iFilterHeight = sRect.Height();
	int iFilterWidth = sRect.Width();

	sRect.top = sToolbarRect.top;
	sRect.bottom = sRect.top + iFilterHeight;
	sRect.left = sToolbarRect.right + 8;
	sRect.right = sRect.left + iFilterWidth;
	DockControlBar (m_pViewFilterBar, AFX_IDW_DOCKBAR_TOP, &sRect);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
CWnd* CMainFrame::GetMessageBar()
{
	if (m_wndStatusBar.GetSimple())
		return &m_wndStatusBar;

	// we are in the Complex, multipane mode and
	// we are avoiding the "Press F1 for help" message

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

afx_msg LRESULT CMainFrame::OnMode1HeadersDone(WPARAM wP, LPARAM lP)
{
	return Send_NewsView (WMU_MODE1_HDRS_DONE, wP, lP);
}

// ------------------------------------------------------------------------
//  8-01-97  amc  call send_goto_message again.
afx_msg LRESULT CMainFrame::OnNgroupHeadersDone(WPARAM wP, LPARAM lP)
{
	LRESULT lRet =  Send_NewsView (WMU_NGROUP_HDRS_DONE, wP, lP);

	// now that group is finished downloading, go to the specified newsurl msg
	send_goto_message ();

	return lRet;
}

void CMainFrame::OnViewStatusBar()
{
	// This is a CFrameWnd function
	OnBarCheck (IDD_STATBAR);
}

void CMainFrame::OnUpdateViewStatusBar(CCmdUI* pCmdUI)
{
	// This is a CFrameWnd function.   pCmdUI should have the right ID.
	ASSERT(pCmdUI->m_nID == IDD_STATBAR);

	OnUpdateControlBarMenu ( pCmdUI );
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWnd::OnSize(nType, cx, cy);

	BOOL fMaximized;
	CMDIChildWnd* pChild = MDIGetActive (&fMaximized);
	if (pChild && !fMaximized)
		MDITile(MDITILE_HORIZONTAL);
}

///////////////////////////////////////////////////////////////////////////
//
//  4-11-96  amc  On first use, set a minimum size of 640 and center the wnd
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL ret;

	TWindowStatus ws;
	BOOL fPrevSettings = gpUIMemory->GetWindowStatus (TUIMemory::WND_MAIN, ws);

	if (fPrevSettings)
	{
		cs.x  =  ws.m_place.rcNormalPosition.left;
		cs.y  =  ws.m_place.rcNormalPosition.top;
		cs.cx =  ws.m_place.rcNormalPosition.right - ws.m_place.rcNormalPosition.left;
		cs.cy =  ws.m_place.rcNormalPosition.bottom - ws.m_place.rcNormalPosition.top;
		// can't call SetWindowPlacement (no hWnd yet)
	}
	else if (gfFirstUse)
	{
		// if this is the first use, then set minimum size by hand
		if (cs.cx < 640)
		{
			// this is the size of a full size window.  (Screen - Taskbar = FSW)
			int iScrCX = GetSystemMetrics(SM_CXFULLSCREEN);
			int iScrCY = GetSystemMetrics(SM_CYFULLSCREEN);
			if (iScrCX <= 640)
			{
				cs.cx = iScrCX;
				cs.cy = iScrCY;
			}
			else
			{
				// higher resolutions hog the screen
				cs.cx = iScrCX * 9 / 10;
				cs.cy = iScrCY * 9 / 10;
			}
			// center it
			cs.x = (iScrCX - cs.cx) / 2;
			cs.y = (iScrCY - cs.cy) / 2;
		}
	}

	ret = CMDIFrameWnd::PreCreateWindow(cs);

	// Register a custom class
	if (m_className.IsEmpty())
	{
		WNDCLASS wndcls;

		ZeroMemory (&wndcls, sizeof(wndcls));

		// retrieve WNDCLASS structure for the base window class
		VERIFY ( ::GetClassInfo(AfxGetInstanceHandle(),
			cs.lpszClass,
			&wndcls) );

		m_className.LoadString (IDS_CLASS_MAIN);

		// Give new class a unique name
		wndcls.lpszClassName = (LPTSTR)(LPCTSTR) m_className;

		wndcls.hIcon = AfxGetApp()->LoadIcon ( IDR_MAINFRAME );
		VERIFY ( ::RegisterClass (&wndcls) );

	}
	cs.lpszClass = (LPTSTR)(LPCTSTR) m_className;
	return ret;
}

// --------------------------------------------------------------------------
void CMainFrame::UnsubscribeFromSampled ()
{
	TServerCountedPtr cpNewsServer;
	TNewsGroupArray &rsSubscribed = cpNewsServer->GetSubscribedArray ();
	TNewsGroupArrayWriteLock ngMgr (rsSubscribed);

	BOOL bYesToAll = FALSE, bNoToAll = FALSE;
	CString strQuestion; strQuestion.LoadString (IDS_SAMPLE_QUESTION);

	// count down just in case unsubscribing changes the array
	int iNum = rsSubscribed->GetSize ();
	for (int i = iNum - 1; i >= 0; i--) {
		TNewsGroup *pNG = rsSubscribed [i];
		if (pNG->IsSampled ())
			EnsureUnsubscribed (pNG->GetName (), strQuestion, &rsSubscribed,
			bYesToAll, bNoToAll);
	}
}

//-------------------------------------------------------------------------
void CMainFrame::OnClose()
{
	handleClose ( false );
}

//-------------------------------------------------------------------------
// return 0 for success
int CMainFrame::handleClose (bool fExitInstance)
{
	ASSERT(::IsWindow(m_hWnd));

	// check with any open Post, Followup or Email windows
	if (CanCloseComposeWindows() == FALSE)
		return 1;

	extern LONG gl_Smtp_Sending;

	if (gl_Smtp_Sending > 0)
	{
		// News32 is sending email. Are you sure you want to exit?
		if (IDYES != NewsMessageBox(this, IDS_WARN_SENDEMAIL, MB_YESNO | MB_ICONQUESTION))
			return 1;
	}

	// see if they really want to leave...
	// if we're shutting down for a server rename, don't
	// ask at all...

	if (FALSE == gfShutdownRename)
	{
		if (gpGlobalOptions->WarnOnExitNews32())
		{
			BOOL  fDisableWarning = FALSE;
			if (WarnWithCBX (IDS_WARNING_EXITNEWS32, &fDisableWarning))
			{
				if (fDisableWarning)
				{
					gpGlobalOptions->SetWarnOnExitNews32(FALSE);
					TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
					pRegWarning->Save();
				}
			}
			else
				return 1;
		}
	}

	if (m_hTimerReconnect)
	{
		KillTimer (m_hTimerReconnect);
		m_hTimerReconnect = 0;
	}

	// Set flag so any MessageBoxes can no longer use it as an anchor.
	//   This is the point of no return
	((CNewsApp*)AfxGetApp())->ZeroSafeFrameWnd();

	// shut down pumps,  hangup any RAS connection
	disconnect_hangup(FALSE);

	// Logically we should be able to shut down the UI before the thread.
	CloseModelessDialogs ();
	gpTasker->KillPrintAndDecodeThreads ();

	if (IsActiveServer())
	{
		UnsubscribeFromSampled ();

		if (!fExitInstance && !m_bDoNotCompactDBOnShutdown)
		{
			// compact database unless this is an emergency shutdown
			CompactDatabase ();
		}
	}

	SaveBarState (TOOLBAR_INFO_SECTION);

	// Save the window position
	TWindowStatus ws;
	gpUIMemory->GetWindowStatus (TUIMemory::WND_MAIN, ws);

	ws.m_place.length = sizeof(ws.m_place);
	GetWindowPlacement ( &ws.m_place );

	// GetWindowPlacement is faulty if the TaskBar is at Top (or Left)
	//  Y-position should be ~28, but GWP returns zero.
	if (SW_SHOWNORMAL == ws.m_place.showCmd)
	{
		// if we are not zoomed or iconized, get a good reading on the
		//   window pos.
		GetWindowRect ( &ws.m_place.rcNormalPosition );
	}

	gpUIMemory->SetWindowStatus (TUIMemory::WND_MAIN, ws);

	// CMDIFrameWnd::OnClose() destroys the main news document, so we need
	// to deinitialize our global newsdoc pointer first
	gptrApp->SetGlobalNewsDoc (NULL);

	extern CNewsApp * gptrApp;

	if (fExitInstance)
	{
		// go through here if we are processing WM_QUERYENDSESSION

		try
		{
			CMDIFrameWnd::OnClose();
		}
		catch(...)
		{
			fExitInstance = fExitInstance;
		}

		// do major cleanup here
		gptrApp->ExitInstance ();
	}
	else
	{
		try
		{
			CMDIFrameWnd::OnClose();
		}
		catch(...)
		{
			fExitInstance = fExitInstance;

			PostMessage(WM_DESTROY);
			AfxPostQuitMessage(1);
		}
	}

	return 0;
}

// ============================================================================
void CMainFrame::OnDestroy()
{
	static bool fDestroyed = false;
	try
	{
		if (false == fDestroyed)
		{
			fDestroyed = true;
			CMDIFrameWnd::OnDestroy();
		}

	}
	catch(...)
	{
		TRACE("Caught Exception in mainfrm.cpp OnDestroy()\n");
		AfxPostQuitMessage(1);
	}
}

// ============================================================================
// periodically we compact newsgroups when we close
void CMainFrame::CompactDatabase (void)
{
	if (!PrepareForCompact(FALSE))
		return;

	if (!gpTasker->GetOutboxCompactLock (3000))
	{
		AfxMessageBox (IDS_ERR_COMPACT_OUTBOX);
		return;
	}

	TCompactDlg compactor;

	compactor.m_fPurgeGroups = TRUE;

	GetCompactList (FALSE, compactor.m_groupsToPurge);

	if (compactor.m_groupsToPurge.GetSize () >= 1)
	{

		try
		{
			compactor.DoModal ();
		}
		catch(...)
		{
			gpTasker->ReleaseOutboxCompactLock ();
			AfxMessageBox (IDS_ERR_COMPACTING_DB);
		}
	}

	gpTasker->ReleaseOutboxCompactLock ();
}

void CMainFrame::CloseModelessDialogs (void)
{
	// close any open modeless dialogs
	if (IsWindow (m_searchDialog.m_hWnd))
		m_searchDialog.SendMessage (WM_CLOSE);
	if (IsWindow (m_eventViewDlg.m_hWnd))
		m_eventViewDlg.SendMessage (WM_CLOSE);
	if (IsWindow (m_decodeDialog.m_hWnd))
		m_decodeDialog.SendMessage (WM_CLOSE);
	if (IsWindow (m_printDialog.m_hWnd))
		m_printDialog.SendMessage (WM_CLOSE);
	if (IsWindow (m_pOutboxDialog->m_hWnd))
		m_pOutboxDialog->SendMessage (WM_CLOSE);
	if (IsWindow (m_pQuickFilter->m_hWnd))
		m_pQuickFilter->SendMessage (WM_CLOSE);

	TPostTemplate* pPostTemplate = gptrApp->GetPostTemplate();
	/* bEndSession is false below so that the modeless post dialogs are
	* closed */
	pPostTemplate->CloseAllDocuments (FALSE /* bEndSession */);
}

void CMainFrame::OnEditCopy()
{
	// TODO: Add your command handler code here
	//TRACE0("\n\nMainfrm:: Copy\n");
}

void CMainFrame::OnSearchSearch()
{
	// TODO: Add your command handler code here
	if (IsWindow(m_searchDialog.m_hWnd))
	{
		m_searchDialog.SetActiveWindow();
		if (m_searchDialog.IsIconic ())
			m_searchDialog.ShowWindow (SW_RESTORE);
	}

	if (!::IsWindow(m_searchDialog.m_hWnd))
		m_searchDialog.Create (IDD_SEARCH, GetDesktopWindow());
}

void CMainFrame::PostNcDestroy()
{
	CMDIFrameWnd::PostNcDestroy();
}

void CMainFrame::OnViewEventlog()
{
	if (IsWindow(m_eventViewDlg.m_hWnd))
	{
		m_eventViewDlg.SetActiveWindow();
		if (m_eventViewDlg.IsIconic ())
			m_eventViewDlg.ShowWindow (SW_RESTORE);
	}

	if (!::IsWindow(m_eventViewDlg.m_hWnd))
	{
		m_eventViewDlg.Create (IDD_EVENT_LOG, GetDesktopWindow());
	}
}

void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	if (0xFFFF == nFlags)
	{
		// exit menu
		CMDIFrameWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);
		m_wndStatusBar.SetSimple(FALSE);
		// SB_SETSIMPLE:FALSE
	}
	else
	{
		// enter menu
		// SB_SETSIMPLE:TRUE
		m_wndStatusBar.SetSimple(TRUE);
		CMDIFrameWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);
	}
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wP, LPARAM lP)
{
	if (AFX_IDS_IDLEMESSAGE == wP)
	{
		if (::IsWindow(m_wndStatusBar.m_hWnd))
			m_wndStatusBar.SetSimple(FALSE);   // back to our complex display

		// normally "CFrameWnd::OnSetMessageString" would
		//   show "Press F1 for Help" in Pane 0
		return CFrameWnd::OnSetMessageString(wP, lP);
	}
	else
	{
		if (::IsWindow(m_wndStatusBar.m_hWnd))
			m_wndStatusBar.SetSimple(TRUE);

		// show help for menu item
		return CFrameWnd::OnSetMessageString(wP, lP);
	}
}

// -------------------------------------------------------------------------
void CMainFrame::OnFileServerReConnect()
{
	try
	{
		TServerCountedPtr cpNewsServer;
		BOOL fRetry       = cpNewsServer->GetRetrying();
		int  iRetryCount  = cpNewsServer->GetRetryCount();

		++m_iReconnectCount;

		if (fRetry && (m_iReconnectCount <= iRetryCount))
			ServerConnectHelper ();
	}
	catch(...)
	{
	}
}

// -------------------------------------------------------------------------
// I guess this is the toolbar button
void CMainFrame::OnFileServerConnect()
{
	CancelReconnect ();   // abort any countdown to reconnect
	m_iReconnectCount = 0;

	ServerConnectHelper ();
}

// -------------------------------------------------------------------------
//
void CMainFrame::ServerConnectHelper ()
{
	if (NULL == gpTasker)
		return;

	// connected, so disconnect
	// or user wants to interrupt dialing, so disconnect RAS
	TDialupManager::ECheckConnect eConnect = dialMgr.GetDialupState ();

	if (TDialupManager::kNotInstalled == eConnect)
	{
		PostMessage (WMU_RASDIAL_EVENT);
		return;
	}

	if (TDialupManager::kConnected == eConnect  ||
		TDialupManager::kConnecting == eConnect)
	{
		disconnect_hangup (FALSE);
		return ;
	}

	// start up the band
	//  call RasDial and return
	BOOL fRasConnect = dialMgr.Connect (fnRasStatusDisplay,
		m_hWnd,
		WMU_RASDIAL_EVENT);
}

// ------------------------------------------------------------------------
// OnRasDialSuccess -- do the LAN level connect

LRESULT CMainFrame::OnRasDialSuccess(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
	{
		return OnRasDialBusy (wParam, lParam);
	}

	bool fConnecting;
	BOOL fConnected = gpTasker->IsConnected (&fConnecting);

	if (!fConnected && false == fConnecting)
	{
		TServerCountedPtr cpNewsServer;
		gpTasker->Connect( cpNewsServer->GetNewsServerAddress() );
	}
	else
	{
		// come here if we are connectED, or connectING
		disconnect_hangup (FALSE);
	}
	return 0;
}

LRESULT CMainFrame::OnRasDialBusy(WPARAM wParam, LPARAM lParam)
{
	bool fOKtoRedial = dialMgr.incrementRedialCount (false);

	if (!fOKtoRedial)
	{
		dialMgr.ShowRasError (true, wParam);
		return 0;
	}

	bool fRedial = dialMgr.ShowRedialDlg (this);

	if (fRedial)
		SendMessage (WM_COMMAND, ID_FILE_SERVER_CONNECT);
	else
		dialMgr.incrementRedialCount (true); // reset to zero
	return 0;
}

void CMainFrame::SetConnectImage(BOOL fConnected)
{
#pragma message ("CMainFrame::SetConnectImage ; needs work")
#if 0
	i = 0;
	while (i < count)
	{
		if((pCustom->GetItemID(i)==ID_FILE_SERVER_CONNECT) ||
			(pCustom->GetItemID(i)==ID_FILE_SERVER_DISCONNECT))
		{
			pCustom->RemoveButton(i);
			if (fConnected)
				pCustom->AddButton (i, ID_FILE_SERVER_CONNECT);
			else
				pCustom->AddButton (i, ID_FILE_SERVER_DISCONNECT);
		}
		i++;
	}
#endif
}

void CMainFrame::OnUpdateFileServerConnect(CCmdUI* pCmdUI)
{
	static BYTE oldConnect = FALSE;  // track previous state
	if (0 == gpTasker)
		return;

	if (gpTasker->IsConnected())
	{
		if (!oldConnect)
		{
			m_wndToolBar.SetButtonInfo(0, ID_FILE_SERVER_CONNECT, TBBS_CHECKBOX, 0);
			SetConnectImage (TRUE);
			oldConnect = TRUE;
		}
		pCmdUI->SetCheck (1);
	}
	else
	{
		if (oldConnect)
		{
			m_wndToolBar.SetButtonInfo(0, ID_FILE_SERVER_CONNECT, TBBS_CHECKBOX, 1);
			SetConnectImage (FALSE);
			oldConnect = FALSE;
		}
		pCmdUI->SetCheck (0);
	}
}

// -------------------------------------------------------------------------
void CMainFrame::OnCurrentPrintJobs()
{
	// if the dialog box is already up, ignore this command
	if (IsWindow(m_printDialog.m_hWnd)) {
		m_printDialog.SetActiveWindow();
		if (m_printDialog.IsIconic ())
			m_printDialog.ShowWindow (SW_RESTORE);
	}
	else
		m_printDialog.Create (IDD_FACTORY_PRINT, GetDesktopWindow ());
}

// -------------------------------------------------------------------------
void CMainFrame::OnCurrentDecodeJobs()
{
	// if the dialog box is already up, ignore this command
	if (IsWindow(m_decodeDialog.m_hWnd)) {
		m_decodeDialog.SetActiveWindow();
		if (m_decodeDialog.IsIconic ())
			m_decodeDialog.ShowWindow (SW_RESTORE);
	}
	else
		m_decodeDialog.Create (IDD_FACTORY_IMAGE, GetDesktopWindow ());

	// move it to the top (useful when it's brought up by the gallery)
	if (IsWindow (m_decodeDialog.m_hWnd))
		m_decodeDialog.SetForegroundWindow ();
}

// -------------------------------------------------------------------------
void CMainFrame::OnImageGallery()
{
	StartGallery ();
}

///////////////////////////////////////////////////////////////////////////
// We have 2 toolbars to hide/show
void CMainFrame::OnToggleToolbar()
{
	/* pre-stingray
	ShowControlBar(&m_wndToolBar,
	(m_wndToolBar.GetStyle() & WS_VISIBLE) == 0, FALSE);
	*/
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	return CMDIFrameWnd::WindowProc(message, wParam, lParam);
}

LRESULT  CMainFrame::OnCheckUIPipe (WPARAM wP, LPARAM lP)
{
	if (gpUIPipe->IsEmpty())
		return 0;

	TPipeItem * pItem = gpUIPipe->RemovePipeHead();

	BOOL fMax;
	CMDIChildWnd* pActiveKid = ((CMainFrame*)AfxGetMainWnd())->MDIGetActive(&fMax);
	if (pActiveKid)
	{
		switch (pItem->m_eMessage)
		{
		case TPipeItem::kRedrawArtint:
			// groupID, artInt
			((TNews3MDIChildWnd*) pActiveKid)->RedrawTitles(
				pItem->sRedrawArtint.iGroupID, pItem->sRedrawArtint.iArtInt,
				pItem->sRedrawArtint.fStatusOnly);
			break;

		case TPipeItem::kRedrawGroup:
			Send_NewsView (WMC_DISPLAY_ARTCOUNT, pItem->sRedrawGroup.iGroupID, 0);
			break;

		case TPipeItem::kViewBinary:
			{
				CString * pFileName = pItem->sViewBinary.pstrFullName;
				CNewsDoc * pDoc = ((CNewsApp*) AfxGetApp())->GetGlobalNewsDoc();
				ViewBinary ( pDoc->PanicMode(), *pFileName );
			}
			break;
		}
	}

	gpUIPipe->DeleteItem ( pItem );
	return 0;
}

void CMainFrame::OnHelp()
{
//	::AfxGetApp()->HtmlHelp(0, HH_HELP_FINDER);
	//HH_AKLINK link;
	//link.cbStruct =     sizeof(HH_AKLINK) ;
	//link.fReserved =    FALSE ;
	//link.pszKeywords =  "Decode" ;
	//link.pszUrl =       NULL ;
	//link.pszMsgText =   NULL ;
	//link.pszMsgTitle =  NULL ;
	//link.pszWindow =    NULL ;
	//link.fIndexOnFail = TRUE ;
	::AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
}

LRESULT  CMainFrame::OnDisplayArtcount (WPARAM wP, LPARAM lP)
{
	//TRACE0("OD artcount 1\n");
	Sleep(0);

	//TRACE0("OD artcount 2\n");
	// this was a custom message.
	LRESULT ret = Send_NewsView (WMC_DISPLAY_ARTCOUNT, wP, lP);

	//TRACE0("OD artcount 3\n");
	return ret;
}

// 10-19-95  make a pipeline to the newsview
LRESULT CMainFrame::Send_NewsView(UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL fMax;
	CMDIChildWnd* pActiveKid = ((CMainFrame*)AfxGetMainWnd())->MDIGetActive(&fMax);
	if (pActiveKid)
	{
		// groupID, artInt
		return ((TNews3MDIChildWnd*) pActiveKid)->Send_NewsView( message, wParam, lParam );
	}
	return 0;
}

//void CMainFrame::OnHelpHowtobuynews32()
//{
//	// TODO: Add your command handler code here
//	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
//}
//
//void CMainFrame::OnHelpGettingthemostoutofnews32()
//{
//	// TODO: Add your command handler code here
//	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
//}

// ----------------------------------------------------------
void CMainFrame::OnOutbox()
{
	if (IsWindow(m_pOutboxDialog->m_hWnd))
	{
		m_pOutboxDialog->SetActiveWindow();
		if (m_pOutboxDialog->IsIconic ())
			m_pOutboxDialog->ShowWindow (SW_RESTORE);
	}

	if (!::IsWindow(m_pOutboxDialog->m_hWnd))
	{
		m_pOutboxDialog->Create (IDD_OUTBOX, GetDesktopWindow());
	}
}

// ----------------------------------------------------------
void CMainFrame::OnDraftMsgs()
{
	BOOL fExists = IsWindow(m_pDraftDialog->m_hWnd);

	if (fExists)
	{
		m_pDraftDialog->SetActiveWindow();
		if (m_pDraftDialog->IsIconic ())
			m_pDraftDialog->ShowWindow (SW_RESTORE);
	}
	else
	{
		m_pDraftDialog->Create (IDD_DRAFTS, GetDesktopWindow());
	}
}

LRESULT CMainFrame::OnRefreshOutbox(WPARAM wP, LPARAM lP)

{
	if (IsWindow(m_pOutboxDialog->m_hWnd))
		m_pOutboxDialog->SendMessage (WMU_REFRESH_OUTBOX, (WPARAM) 0, (LPARAM) 0);

	if (IsWindow(m_pDraftDialog->m_hWnd))
		m_pDraftDialog->SendMessage (WMU_REFRESH_OUTBOX, (WPARAM) 0, (LPARAM) 0);

	return 0;
}

LRESULT CMainFrame::OnRefreshDraft(WPARAM wP, LPARAM lP)

{
	if (IsWindow(m_pDraftDialog->m_hWnd))
		m_pDraftDialog->SendMessage (WMU_REFRESH_OUTBOX, (WPARAM) 0, (LPARAM) 0);

	return 0;
}

void CMainFrame::OnUpdateSearchSearch(CCmdUI* pCmdUI)
{
	if (IsActiveServer())
	{
		TServerCountedPtr cpNewsServer;
		TNewsGroupArray &vNewsGroups = cpNewsServer->GetSubscribedArray ();
		pCmdUI->Enable (vNewsGroups->GetSize() != 0);
	}
}

///////////////////////////////////////////////////////////////
// Collect tagged articles from newsgroups, and fetch them
//
void CMainFrame::OnAppFetchTagged()
{
	retrieve_tagged ( false );
}

///////////////////////////////////////////////////////////////
void CMainFrame::retrieve_tagged (bool fUponConnect)
{
	if (IsActiveServer())
	{
		TServerCountedPtr cpNewsServer;

		if (fUponConnect)
		{
			// clear out any existing Tag jobs from Pump Queue
			if (gpTasker)
				gpTasker->EraseAllTagJobs ();
		}

		// resubmit all tag jobs
		CDWordArray junk;
		cpNewsServer->GetPersistentTags().RetrieveTagged(true, junk);
	}
}

void CMainFrame::OnUpdateAppFetchTagged(CCmdUI* pCmdUI)
{
	if (gpTasker && gpTasker->IsConnected())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);

	// calculate if anything is tagged
}

// called from newspump thread
afx_msg LRESULT CMainFrame::OnPumpArticle (WPARAM wP, LPARAM lP)
{
	m_fetchArticleArray.Add ( (TFetchArticle*) lP );

	// post a message; keep asynch to release pump
	// thread from any further delay
	PostMessage (WMU_PROCESS_PUMP_ARTICLE);
	return 0;
}

afx_msg LRESULT CMainFrame::OnProcessPumpArticle(WPARAM wP, LPARAM lP)
{
	BOOL fMaximized;
	CMDIChildWnd* pChild = MDIGetActive (&fMaximized);
	if (pChild)
	{
		TFetchArticle* pFetch = m_fetchArticleArray.GetAt(0);
		m_fetchArticleArray.RemoveAt(0);

		((TNews3MDIChildWnd*) pChild)->ProcessPumpArticle ( pFetch );
	}
	return 0;
}

afx_msg LRESULT CMainFrame::OnNonBlockingCursor(WPARAM wP, LPARAM lP)
{
	BOOL fMaximized;

	CMDIChildWnd* pChild = MDIGetActive (&fMaximized);
	pChild->SendMessage(WMU_NONBLOCKING_CURSOR, wP, lP);
	return 0;
}

// ------------------------------------------------------------------------
// WMU_CONNECT_OK is posted from the CNewsDoc
afx_msg LRESULT CMainFrame::OnConnectOK(WPARAM wP, LPARAM lP)
{
	if (gfFirstUse)
		return 0;

	if (m_iReconnectCount > 0)
	{
		CString msg; msg.LoadString (IDS_UTIL_RECONNECT_OK);
		gpEventLog->AddInfo(TEventEntry::kGeneral, msg);
	}

	bool fVCRMode = IsVCRWindowActive ();

	CWaitCursor wait;
	TServerCountedPtr cpNewsServer;

	if (false == fVCRMode)
	{
		int iOption = cpNewsServer->GetUpdateGroupCount ();
		switch (iOption)
		{
		case 0:
			// manual updating of NewsView pane
			break;

		case 1:
			if (!cpNewsServer->GetGroupCountsUpdated())
				gpTasker->StartPingCycle ();
			cpNewsServer->SetGroupCountsUpdated(true);
			break;

		case 2:
			// update counts everytime we connect
			gpTasker->StartPingCycle ();
			cpNewsServer->SetGroupCountsUpdated(true);
			break;
		}
	}

	if (false == fVCRMode)
	{
		// get any newly created newsgroups
		if (TGlobalDef::kGroupsUponConnecting == cpNewsServer->GetGroupSchedule())
		{
			// proxicom server, you gotta use LIST.
			//   the NEWGROUPS command is not implemented.

			CNewsApp* pNews = (CNewsApp*) AfxGetApp();
			pNews->GetGlobalNewsDoc()->UtilRetrieveRecentNewsgroups (
				false,
				cpNewsServer->GetDisplayIncomingGroups());
		}
	}

	if (false == fVCRMode)
	{
		// option to get headers for subscribed groups
		if (cpNewsServer->GetAutomaticCycle())
		{
			CNewsDoc::DocGetHeadersAllGroups (true /* fForceCycle */, false);
		}
	}

	// $$ i suppose this is OK for VCR to perform this also..
	// option to Fetch tagged articles
	if (cpNewsServer->GetAutoRetrieveTagged())
	{
		retrieve_tagged ( true );
	}

	if (false == fVCRMode)
	{
		// newsurl - jump to this newsgroup
		if (m_pGotoInfo)
			send_goto_message ();

	}

	if (fVCRMode)
	{
		HWND hDlg = LockVCRWindow ();
		if (hDlg)
			::PostMessage (hDlg, WMU_VCR_NEXTSTEP, 0, 0);
		LockVCRWindow (false);
	}

	return 0;
}

//-------------------------------------------------------------------------
void CMainFrame::OnOnlineStop()
{
	if (0 == gpTasker)
		return;

	if (gpTasker->EmergencyPumpBusy())
	{
		CString str; str.LoadString (IDS_STOP_DOWNLOAD);
		if ((NewsMessageBox(this, str, MB_ICONQUESTION | MB_YESNO) == IDYES)
			&& gpTasker->EmergencyPumpBusy())
		{
			// still busy
			//TRACE0("STOP button: Request stop emergency pump\n");
			gpTasker->StopEmergencyPump ();
		}
	}
	else
	{
		CString str; str.LoadString (IDS_STOP_ACTIVITY);
		if (NewsMessageBox(this, str, MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			// even if pump is not busy now, disconnect

			CancelReconnect ();  // abort reconnect countdown

			disconnect_hangup (FALSE);
		}
	}
}

static BOOL is_toolbar_visible (HWND hWnd)
{
	return (::IsWindow(hWnd) && ::IsWindowVisible(hWnd));
}

void CMainFrame::OnToggleRuleBar()
{
	ShowControlBar (&m_sManualRuleBar,
		!(m_sManualRuleBar.GetStyle() & WS_VISIBLE), FALSE);
}

void CMainFrame::OnUpdateViewRuleBar (CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (is_toolbar_visible (m_sManualRuleBar.m_hWnd));
}

void CMainFrame::OnToggleOutboxBar()
{
	ShowControlBar (&m_sOutboxBar,
		!(m_sOutboxBar.GetStyle() & WS_VISIBLE), FALSE);
}

void CMainFrame::OnUpdateViewOutboxBar (CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (is_toolbar_visible (m_sOutboxBar.m_hWnd));
}

void CMainFrame::OnToggleFilterBar()
{
	ShowControlBar (m_pViewFilterBar,
		!(m_pViewFilterBar->GetStyle() & WS_VISIBLE), FALSE);
}

void CMainFrame::OnUpdateToggleFilterBar (CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (is_toolbar_visible (m_pViewFilterBar->m_hWnd));
}

///////////////////////////////////////////////////////////////////////////
// Display the list of newsgroups - the pump has fetched it Asynchronously
//
LRESULT CMainFrame::OnGetlistDone(WPARAM wParam, LPARAM lP)
{
	LPT_GROUPLISTDONE pDone = (LPT_GROUPLISTDONE) lP;

	TGroupList * pFreshGroups = pDone->pGroupList;
	int iNTPRet = pDone->iNTPRet;
	CString strAck = pDone->strAck;
	delete pDone;

	BOOL fPopupEmptyList = (wParam & kGetListPopupEmptyList);
	if (iNTPRet >= 300)
	{
		// newsview shows error
		PTYP_ERROR_FROM_SERVER psErr = new TYP_ERROR_FROM_SERVER;
		psErr->iRet = iNTPRet;
		psErr->actionDesc.LoadString (IDS_UTIL_READGNAMES);
		psErr->serverString = strAck;

		Send_NewsView (WMU_ERROR_FROM_SERVER, WPARAM(psErr), 0);
	}
	else
	{
		if (wParam & kGetListShowDialog)
		{
			// user picks from the fresh newsgroups if list is !empty
			// or they chose it manually from the menu

			if ((NULL==pFreshGroups) || pFreshGroups->NumGroups())
			{
				CNewsDoc* pDoc = ((CNewsApp*) AfxGetApp())->GetGlobalNewsDoc();
				pDoc->CallSubscribeDlg( pFreshGroups );
			}
			else if (fPopupEmptyList && 0 == pFreshGroups->NumGroups())
			{
				// 4-23-96 just show a message
				NewsMessageBox(this, IDS_WARN_NONEWGROUPS, MB_OK | MB_ICONINFORMATION);
			}

			if (pFreshGroups)
				pFreshGroups->Empty();
		}

		delete pFreshGroups;
	}
	return 0;
} // OnGetlistDone

///////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnRefreshOutboxBar (WPARAM wParam, LPARAM lP)
{
	gpsOutboxBar->UpdateDisplay ();
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Called when doing the overview stage
//
LRESULT CMainFrame::OnExpireArticles (WPARAM wParam, LPARAM lParam)
{
	ASSERT(0 == wParam);   // unused parameter

	TExpiryData* pExpiryData = (TExpiryData*) lParam;

	if (0 == pExpiryData)
	{
		ASSERT(0);
		return 0;
	}

	TServerCountedPtr cpNewsServer;
	BOOL fUseLock = FALSE;
	TNewsGroup* pNG = 0;

	// get the newsgroup object
	TNewsGroupUseLock useLock(cpNewsServer, pExpiryData->m_iGroupID, &fUseLock, pNG);

	if (!fUseLock)
	{
		// might be unsubscribed - clean it all up
		delete pExpiryData;
		return 0;
	}

	CWaitCursor wait;

	// ng does the real work.

	// handles Results from LISTGROUP. or Missing articles
	pNG->ExpireArticles ( pExpiryData );

	// manually verifying local headers, we do a redraw ourselves
	if (pExpiryData->m_fRedrawUI)
	{
		if (pExpiryData->m_iGroupID == GetNewsView()->GetCurNewsGroupID())
			GetNewsView()->RefreshCurrentNewsgroup ();

		GetNewsView()->PostMessage (WMC_DISPLAY_ARTCOUNT, pExpiryData->m_iGroupID);
	}

	// clean up datastruct
	delete pExpiryData;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// PrepareForCompact - does the following:
// 1. checks for existing pumps.  If there is one, the user
//    is told they can't compact now and we return false
// 2. If we're going to ask the user, we get the database
//    stats and pop up a dialog to see if they want to go through
//    with it.
// 3. If they do, we close the current group and return true (the
//    server can't be compacted with open newsgroups.
/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::PrepareForCompact (BOOL fAskUser)
{
	if (gpTasker)
		if (gpTasker->EmergencyPumpBusy() ||
			gpTasker->NormalPumpBusy ())
		{
			AfxMessageBox (IDS_ERR_COMPACT_JOBS);
			return FALSE;
		}

		if (fAskUser)
		{
			// get the stats for this server...
			TNewsDBStats   dbStats;
			TServerCountedPtr cpNewsServer;

			if (IsActiveServer ())
				cpNewsServer->GetDBStats (&dbStats);

			TAskCompact askCompDlg;
			askCompDlg.m_bytesSaved.Format (_T("%d"), dbStats.GetCurrentSize () -
				dbStats.GetCompressibleSize ());
			LONG  percentSaved = 0;
			if (dbStats.GetCurrentSize ())
				percentSaved =
				(100 * (dbStats.GetCurrentSize() - dbStats.GetCompressibleSize()))
				/ dbStats.GetCurrentSize ();

			askCompDlg.m_percentSaved.Format (_T("%d%%"), percentSaved);
			askCompDlg.m_spaceRequired.Format (_T("%d"), dbStats.GetExtraSpaceNeeded());

			TPath    serverDBPath = cpNewsServer->GetServerDatabasePath ();
			TCHAR    buff[1024];
			LPTSTR   pFileName;

			GetFullPathName (serverDBPath, sizeof (buff), buff, &pFileName);

			// if the full path is drive letter based, hack it  off
			// after the first backslash.  Otherwise, it should be of
			// the form \\computer\resource\ in which case we put the
			// zero just after the fourth slash

			if (buff[1] == ':')
				buff[3] = 0;
			else
			{
				int     found = 0;
				size_t  pos   = 0;

				while (!found && (pos < _tcslen (buff)))
					if (buff[pos++] == '\\')
					{
						found++;
						if (found == 4)
							break;
					}
					buff[pos] = 0;
			}

			DWORD dwSectorsPerCluster;
			DWORD dwBytesPerSector;
			DWORD dwNumFreeClusters;
			DWORD dwTotalNumOfClusters;

			GetDiskFreeSpace (buff,
				&dwSectorsPerCluster,
				&dwBytesPerSector,
				&dwNumFreeClusters,
				&dwTotalNumOfClusters);

			DWORDLONG freeSpace =
				UInt32x32To64 (dwSectorsPerCluster * dwBytesPerSector,
				dwNumFreeClusters);

			askCompDlg.m_spaceAvailable.Format (_T("%I64d"), freeSpace);

			if (IDOK != askCompDlg.DoModal())
				return FALSE;
		}

		// close the current newsgroup...

		CNewsView *pNewsView = GetNewsView ();

		if (pNewsView)
			pNewsView->CloseCurrentNewsgroup ();

		return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CMainFrame::GetCompactList (BOOL  fCompactAll, TPurgeArray & compactList)
{
	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vNewsGroups);

	int count = vNewsGroups->GetSize();

	// Ick! each newsgroup might have custom settings that override the default
	for (int i = 0; i < count; ++i)
	{
		TNewsGroup* pNG = vNewsGroups[i];
		TNewsGroupDB * pDB = pNG;

		if (fCompactAll || pNG->NeedsCompact())
			compactList.Add (pDB);
	}
}

/////////////////////////////////////////////////////////////////////////////
// OnDatePurgeAll - Called when user selects Purge All Newsgroups
//                  from the File menu.
//
// Manually purge all newsgroups using Date criteria
//   and server low bound
/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnDatePurgeAll()
{
	if (!PrepareForCompact())
		return;

	if (!gpTasker->GetOutboxCompactLock (3000))
	{
		AfxMessageBox (IDS_ERR_COMPACT_OUTBOX);
		return;
	}

	TCompactDlg compactor;

	compactor.m_fPurgeGroups = TRUE;

	GetCompactList (TRUE, compactor.m_groupsToPurge);

	try
	{
		compactor.DoModal ();
	}
	catch(TException * rTE)
	{
		gpTasker->ReleaseOutboxCompactLock ();
		rTE->PushError (IDS_ERR_COMPACTING_DB, kError);
		rTE->Display ();
		rTE->Delete();
	}
	catch(...)
	{
		gpTasker->ReleaseOutboxCompactLock ();
		AfxMessageBox (IDS_ERR_COMPACTING_DB);
	}

	gpTasker->ReleaseOutboxCompactLock ();
}

void CMainFrame::OnUpdateDatePurgeAll (CCmdUI* pCmdUI)
{
	if (!IsActiveServer())
	{
		pCmdUI->Enable (FALSE);
		return;
	}

	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

	pCmdUI->Enable ((vNewsGroups->GetSize() > 0) ? TRUE : FALSE);
}

BOOL CMainFrame::CanCloseComposeWindows ()
{
	return
		CanCloseComposeWindowType (gptrApp->GetPostTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetFollowTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetReplyTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetForwardTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetBugTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetSuggestionTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetSendToFriendTemplate ()) &&
		CanCloseComposeWindowType (gptrApp->GetMailToTemplate ());
}

BOOL CMainFrame::CanCloseComposeWindowType (TPostTemplate *pPostTemplate)
{
	POSITION posDoc = pPostTemplate->GetFirstDocPosition ();

	while (posDoc)
	{
		CDocument * pDoc = pPostTemplate->GetNextDoc ( posDoc );
		if (pDoc)
		{
			POSITION posV = pDoc->GetFirstViewPosition ();
			if (posV)
			{
				CView * pView = pDoc->GetNextView (posV);
				if (!pDoc->CanCloseFrame ( pView->GetTopLevelFrame() ))
					return FALSE;
			}
		}
	}

	return TRUE;
}

// ------------------------------------------------------------------------
// Someone (pump?) is forcing the UI to show an error message
LRESULT CMainFrame::OnErrorFromServer(WPARAM wP, LPARAM lP)
{
	return Send_NewsView (WMU_ERROR_FROM_SERVER, wP, lP);
}

// ------------------------------------------------------------------------
void CMainFrame::OnChooseFilterDisplay()
{
	m_pViewFilterBar->ChooseFilter (this);
}

// ------------------------------------------------------------------------
//
void CMainFrame::OnDefineFilterDisplay()
{
	m_pViewFilterBar->DefineFilters ();
}

// ------------------------------------------------------------------------
// Forcibly change the filter to the all inclusive one 'All Articles'
//  Used from :  search dialog
// Returns: 0 for success, non-zero for error
LRESULT CMainFrame::OnForceFilterChange(WPARAM wP, LPARAM lP)
{
	return m_pViewFilterBar->ForceShowAll ();
}

// -------------------------------------------------------------------
// update numbers in the newsview pane
void CMainFrame::OnUpdateNgCounts()
{
	if (gpTasker && gpTasker->IsConnected())
		gpTasker->StartPingCycle ();
}

// -------------------------------------------------------------------
void CMainFrame::OnUpdateUpdateNgCounts(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (gpTasker && gpTasker->IsConnected());
}

// -------------------------------------------------------------------
void CMainFrame::OnViewRulestats()
{
	TRuleStats dlg;
	dlg.DoModal ();
}

// -------------------------------------------------------------------
//void CMainFrame::OnHelpTutorial()
//{
//	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
//}

// -------------------------------------------------------------------
void CMainFrame::OnNewsrcExport()
{
	TExportNewsrc sDlg;
	sDlg.DoModal ();
}

// -------------------------------------------------------------------
void CMainFrame::OnNewsrcImport()
{
	TImportNewsrc sDlg;
	sDlg.DoModal ();
}

// -------------------------------------------------------------------
LRESULT CMainFrame::OnCopyData(WPARAM wParam, LPARAM lP)
{
	int i = 0;
	PCOPYDATASTRUCT pCD = (COPYDATASTRUCT*) lP;

	switch ( pCD->dwData )
	{
		// vcr data
	case 1200:
		{
			CString vcrFile = LPCTSTR(pCD->lpData);

#if defined(_DEBUG)
			MessageBox ( vcrFile );
#endif

			OnVCRWorker (false, vcrFile);
			break;
		}

		// news url message.
	case 1738:
		HandleNewsURL (LPCTSTR(pCD->lpData), false);
		break;
	}
	return 1;
}

// -------------------------------------------------------------------
LRESULT CMainFrame::OnNewsURL(WPARAM wParam, LPARAM lP)
{
	// this came through the command line and InitInstance(),
	//   so Gravity has just been started
	ASSERT(0 == wParam);

	if (sCmdLineInfo.m_bNewsURL)
	{
		HandleNewsURL (sCmdLineInfo.m_strNewsURL, true);

		// flush data, so we don't do it again - like during SwitchServer
		sCmdLineInfo.m_bNewsURL = FALSE;
		sCmdLineInfo.m_strNewsURL.Empty ();
	}

	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 for success
int  url_server_exist (CWnd* pWnd, LPTSTR serverName)
{
	if (!gpStore->ServerExist (serverName))
	{
		// this news server has not been added to Gravity.

		CString display; display.Format (IDS_INVL_URL_SERVER, serverName);
		NewsMessageBox (pWnd, display, MB_OK | MB_ICONWARNING);
		return 1;
	}
	return 0;
}

// ------------------------------------------------------------------------
// HandleNewsURL --
int CMainFrame::HandleNewsURL (const CString& urlIn, bool fLoading)
{
	int iBufLen = urlIn.GetLength();
	LPTSTR pHost = new TCHAR[iBufLen];
	LPTSTR pGroup = new TCHAR[iBufLen];
	int iArtNum = -1;
	int iPort = -1;

	if (0 == (urlIn.Left(7).CompareNoCase(_T("nntp://"))))
	{
		// this form can specify        nntp://host:port/group/article#
		//  the port is optional
		CString data = urlIn.Mid(7);

		*pHost = 0;

		std::istrstream in(data);

		in.get (pHost, iBufLen, '/');
		in.get ();
		in.get (pGroup, iBufLen, '/');
		in.get ();
		in >> iArtNum;

		if (-1 == iArtNum)
		{
			// "Invalid NNTP link - article number is missing"

			CString fmat; fmat.LoadString (IDS_INVL_NNTP_URL);
			CString display; display.Format(fmat, LPCTSTR(urlIn));
			NewsMessageBox (this, display, MB_OK | MB_ICONWARNING);
			goto site_clean;
		}

		TCHAR* pColon = _tcschr(pHost, ':');
		if (pColon)
		{
			*pColon = _T('\0');   // cap off pHost
			iPort = _ttoi(pColon + 1);
		}

		// worry about case sensitive?
		if (url_server_exist (this, pHost))
			goto site_clean;

		ShowGroupWithArticle (pGroup, iArtNum);
	}
	else if (0 == urlIn.Left(5).CompareNoCase(_T("news:")))
	{
		// this form can specify    news:<message-id>                [url1738]
		//   or                     news:<newsgroup-name>            [url1738]
		//  (microsoft-extension)   news://<server>/<newsgroup-name>
		CString afterColon = urlIn.Mid(5);

		if (-1 != afterColon.Find('@'))
		{
			// if we find a AT-SIGN assume it's a message-id
			// "Gravity does not handle this form of URL"
			CString fmat; fmat.LoadString (IDS_INVL_URL_MSGID);
			CString display; display.Format (fmat, LPCTSTR(afterColon));

			NewsMessageBox (this, display, MB_OK | MB_ICONWARNING);
		}
		else
		{
			if (0 == afterColon.Find(_T("//")))  // FIND "://"
			{
				// could be a MS-style newsurl.  news://<server>/<newsgroup-name>
				CString rev = afterColon;

				// gather the server
				TCHAR* pFillHost = pHost;
				for (int i = 2; i < afterColon.GetLength() && ('/' != afterColon[i]); i++)
					*pFillHost++ = afterColon[i];
				*pFillHost = '\0';

				if (url_server_exist (this, pHost))
					goto site_clean;

				rev.MakeReverse();

				// pick off the groupName.
				CString onlyGroup = rev.SpanExcluding(_T("/"));

				// back to normal
				onlyGroup.MakeReverse();

				// since we are just starting up, we can switch servers without
				//   interrupting anything important
				//
				//  -- OR --
				// Gravity was already running, but maybe we are idle enough
				//   to allow switching
				if (fLoading || (gpTasker && !gpTasker->NormalPumpBusy()))
				{
					if (2 == gptrApp->ForceSwitchServer ( LPCTSTR(pHost) ))
					{
						CString errSRV;
						errSRV.Format(IDS_ERR_CHANGE_SERVER, LPCTSTR(pHost));
						NewsMessageBox (this, errSRV, MB_OK | MB_ICONWARNING);
						goto site_clean;
					}
				}
				ShowGroupWithArticle (onlyGroup, iArtNum);
			}
			else
			{
				// I think this looks like  "news:<newsgroup-name>"
				//   pass in stuff after the colon
				ShowGroupWithArticle (afterColon, iArtNum);
			}
		}
	}
site_clean:
	delete [] pGroup;
	delete [] pHost;
	return 0;
}

// ------------------------------------------------------------------------
// more support for news url stuff.
int CMainFrame::ShowGroupWithArticle (const CString& group, int iArtNum)
{
	TServerCountedPtr cpNewsServer;

	// see if group is subscribed already
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

	LONG lGroupID = 0;

	if (vNewsGroups.Exist (group))
	{
		TNewsGroup * pNG = 0;
		BOOL fLock;
		TNewsGroupUseLock useLock(cpNewsServer, group, &fLock, pNG);
		if (fLock)
			lGroupID = pNG->m_GroupID;
	}
	else
	{
		TGlobalDef::ENewsGroupType eType;

		// find it in our list??
		TGroupList * pBigList = cpNewsServer->LoadServerGroupList ();

		int idx = 0;
		WORD wNumArts = 0;
		if (!pBigList->GroupExist (group, &idx))
		{
			delete pBigList;
			MessageBox (_T("Group not found in Gravity's group list"));
			return 0;
		}
		else
		{
			CString strGrpName;  // unused
			// see if group is Yes/No/Moderated
			pBigList->GetItem (idx, strGrpName, wNumArts, eType);

			TNewsGroup *pOneGroup = cpNewsServer->SubscribeGroup (
				group,
				eType,   // Yes-No-Moderated
				BYTE(STORAGE_MODE_DEFAULT),
				cpNewsServer->GetGoBackArtcount (),
				true );  // sample

			lGroupID = pOneGroup->m_GroupID;

			// add it to our working list
			gptrApp->AddToNewsgroupList (pOneGroup);

			CNewsDoc * pDoc = gptrApp->GetGlobalNewsDoc ();
			if (pDoc)
				pDoc->SubscribeGroupUpdate ();
		}
		delete pBigList;
	}

	if (lGroupID)
	{
		m_pGotoInfo = new TGotoArticle;

		m_pGotoInfo->m_articleNumber = iArtNum > 0 ? iArtNum : 0;
		m_pGotoInfo->m_groupNumber = lGroupID;
		m_pGotoInfo->m_byLoad      = TRUE;
		m_pGotoInfo->m_byOpenNG    = TRUE;
		m_pGotoInfo->m_byDownloadNG = TRUE;     // specifies fetch-on-zero

		if (gpTasker->IsConnected())
			send_goto_message ();
		else
			gpTasker->Connect ( cpNewsServer->GetNewsServerAddress() );
	}
	return 0;
}

// ------------------------------------------------------------------------
// pass info to newsview and delete the data struct.  Hooks into the
//   Search-Window 'JumpTo' code.
int  CMainFrame::send_goto_message ()
{
	if (0 == m_pGotoInfo)
		return 0;

	// m_pGotoInfo was setup previously.
	LRESULT lRet = MDIGetActive()->SendMessage (WMU_NEWSVIEW_GOTOARTICLE, 0,
		LPARAM(m_pGotoInfo));

	if (0 == lRet)
	{
		// we opened the newsgroup. If 1==lRet, then we just started the
		//   download and wait for 'OnNgroupHeadersDone' to call us.
		delete m_pGotoInfo;
		m_pGotoInfo = 0;
	}
	return 0;
}

// ------------------------------------------------------------------------
// gotta know when the splash screen is gone, so we can show our dlg.
LRESULT CMainFrame::OnSplashScreenGone(WPARAM wP, LPARAM lP)
{
	// check that we are still registered in the Registry to handle news url's

	TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
	TNewsUrl sTempUrl;

	if (pRegWarning->m_fWarnAboutNewsURL && !sTempUrl.IsDefaultReader())
	{
		BOOL  fDisableWarning = FALSE;
		if (WarnWithCBX_Ex (IDS_WARNING_INSISTNEWSURL,
			&fDisableWarning, this))
			sTempUrl.SetDefaultReader ();

		if (fDisableWarning)
		{
			pRegWarning->m_fWarnAboutNewsURL = FALSE;
			pRegWarning->Save();
		}
	}

#if defined(_DEBUG)

	HMENU hPop = ::CreatePopupMenu();

	AppendMenu (hPop, MF_STRING | MF_ENABLED, ID_BETA_TEST1, "Test &1");
	AppendMenu (hPop, MF_STRING | MF_ENABLED, ID_BETA_TEST2, "Test &2");
	AppendMenu (hPop, MF_STRING | MF_ENABLED, ID_BETA_TEST3, "Test &3");
	AppendMenu (hPop, MF_STRING | MF_ENABLED, ID_BETA_TEST4, "Test &4");
	AppendMenu (hPop, MF_STRING | MF_ENABLED, ID_BETA_TEST5, "Test &5");

	CMenu * pMenuMain = GetMenu();

	pMenuMain->AppendMenu (MF_POPUP | MF_STRING, (UINT) hPop, "&Beta");
#endif

	return 0;
}

#if defined(_DEBUG)
#include "dllver.h"
#include "mailadr.h"
#endif

afx_msg void CMainFrame::OnBetaTestAll(UINT id)
{
#if defined(_DEBUG)
	switch (id)
	{
	case ID_BETA_TEST1:
		{
			TCHAR rcHost[30];    lstrcpy(rcHost, "mplanet");
			TCHAR rcAddress[300]; lstrcpy(rcAddress, "\"Albert M. Choy\" < alchoy @ yahoo . com >");
			LPTSTR pBuf = rcAddress;
			CStringList errList;

			MAIL_ADDRESS * psAddress = rfc822_parse_address (&pBuf, rcHost, &errList);

			if (psAddress)
				delete psAddress;

			break;
		}
	case ID_BETA_TEST2:
		{
			DWORD dwVer = GetDllVersion(_T("comctl32.dll"));
			break;
		}

	case ID_BETA_TEST3:
		PostMessage (WMU_DOCK_DIR_BAR_ON_RIGHT_OF_TOOLBAR);
		break;
	}
#endif
}

afx_msg void CMainFrame::OnSaveBarState()
{
	if (!ControlBarsExist())
	{
		SaveBarState (TOOLBAR_INFO_SECTION);
	}
}

// ------------------------------------------------------------------------
bool CMainFrame::ExistComposeWindows ()
{
	return
		ExistComposeWindowType (gptrApp->GetPostTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetFollowTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetReplyTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetForwardTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetBugTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetSuggestionTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetSendToFriendTemplate ()) ||
		ExistComposeWindowType (gptrApp->GetMailToTemplate ());
}

// ------------------------------------------------------------------------
// ExistComposeWindowType -- check if this window type exists
//
bool CMainFrame::ExistComposeWindowType (TPostTemplate *pPostTemplate)
{
	POSITION posDoc = pPostTemplate->GetFirstDocPosition ();

	while (posDoc)
	{
		CDocument * pDoc = pPostTemplate->GetNextDoc ( posDoc );
		if (pDoc)
		{
			POSITION posV = pDoc->GetFirstViewPosition ();
			if (posV)
				return true;
		}
	}

	return false;
}

// ------------------------------------------------------------------------
// Call this just before switching servers
bool CMainFrame::ExistServerAnchoredWindows ()
{
	if (ExistComposeWindows ())
	{
		NewsMessageBox (this, IDS_PLS_CLOSE_COMPWINDS);
		return true;
	}

	if (IsWindow(m_searchDialog.m_hWnd))
	{
		if (IDYES != NewsMessageBox (this, IDS_PLS_CLOSE_SRCHWIND, MB_YESNO))
			return true;
		else
		{
			::SendMessage (m_searchDialog.m_hWnd, WM_CLOSE, 0, 0);
		}
	}

	HWND vHandles[4];

	vHandles[0] = m_pOutboxDialog->m_hWnd;
	vHandles[1] = m_decodeDialog.m_hWnd;
	vHandles[2] = m_printDialog.m_hWnd;
	vHandles[3] = m_pDraftDialog->m_hWnd;

	// play hard ball with these other guys
	for (int i = 0; i < ELEM(vHandles); i++)
	{
		HWND hw = vHandles[i];
		if (IsWindow (hw))
			::SendMessage (hw, WM_CLOSE, 0, 0L);
	}

	return false;
}

// ------------------------------------------------------------------------
void CMainFrame::OnToolbarCustomize()
{
	m_wndToolBar.PostMessage (WM_COMMAND, ID_POPUP_CUSTOMIZE);
}

// ------------------------------------------------------------------------
void CMainFrame::OnDejanews()
{
	CString strDejaNewsURL = gpGlobalOptions->GetRegSystem()->GetDejanewsURL();

	ShellExecute (GetDesktopWindow ()->m_hWnd /* parent window */,
		_T("open")          /* command */,
		strDejaNewsURL  /* file */,
		NULL            /* command line */,
		NULL            /* directory */,
		SW_SHOW         /* nCmdShow */);
}

// ------------------------------------------------------------------------
// This message originates from the GravMon.DLL, which contains a
//   keyboard hook
LRESULT CMainFrame::OnPanicKeyMsg (WPARAM wP, LPARAM lP)
{
	CNewsDoc* pDoc = ((CNewsApp*) AfxGetApp())->GetGlobalNewsDoc();
	if (pDoc)
	{
		if (pDoc->PanicMode())
		{
			ShowWindow (SW_SHOW);
			pDoc->PanicMode (false);
		}
		else
		{
			ShowWindow (SW_HIDE);
			pDoc->PanicMode (true);

			// hide the image gallery by shutting it down.
			HWND hwndGallery = GetGalleryWindow ();
			if (hwndGallery)
				::PostMessage (hwndGallery, WM_CLOSE, 0, 0);
		}
	}
	return 0L;
}

// ------------------------------------------------------------------------
void CMainFrame::OnScoring()
{
	TScoringDlg sDlg;
	sDlg.DoModal ();
}

// ------------------------------------------------------------------------
// Returns 0 for success
int CMainFrame::SelectFilter (int iFilterID)
{
	return m_pViewFilterBar->SelectFilter (iFilterID);
}

// ------------------------------------------------------------------------
// This message originates from the image factory's middle list control when
// its columns have been resized so that they're too big
LRESULT CMainFrame::OnResetFactoryColumns (WPARAM wParam, LPARAM)
{
	if (wParam)
		m_decodeDialog.ResetColumns ();
	else
		m_printDialog.ResetColumns ();
	return 0;
}

// ------------------------------------------------------------------------
void CMainFrame::OnUpdateRescore(CCmdUI* pCmdUI)
{
	CNewsView *pView = GetNewsView ();
	pCmdUI->Enable (pView->IsNewsgroupDisplayed ());
}

// ------------------------------------------------------------------------
void CMainFrame::OnRescore()
{
	RescoreCurrentGroup ();
}

// ------------------------------------------------------------------------
void CMainFrame::OnUpdateApplyBozo(CCmdUI* pCmdUI)
{
	CNewsView *pView = GetNewsView ();
	pCmdUI->Enable (pView->IsNewsgroupDisplayed ());
}

// ------------------------------------------------------------------------
void CMainFrame::OnApplyBozo()
{
	ApplyBozoToCurrentGroup ();
}

// ------------------------------------------------------------------------
void CMainFrame::OnUpdateQuickScore(CCmdUI* pCmdUI)
{
	CNewsView *pView = GetNewsView ();
	pCmdUI->Enable (pView->IsNewsgroupDisplayed ());
}

// ------------------------------------------------------------------------
void CMainFrame::OnQuickScore()
{
	TQuickScore sDlg;
	sDlg.DoModal ();
}

// ------------------------------------------------------------------------
void CMainFrame::OnScoreColors()
{
	TScoreColors sDlg;
	sDlg.DoModal ();
}

// ------------------------------------------------------------------------
// OnServerSwitch -- do some UI chores associated with changing servers.
//    This is purely a notification, no info is passed in.
LRESULT CMainFrame::OnServerSwitch (WPARAM wP, LPARAM lP)
{
	try
	{
		CMDIChildWnd * pMDIChild;
		CDocument * pDoc;

		if ((pMDIChild = MDIGetActive ()) != 0)
		{
			// the mdi child is derived from CFrameWindow
			if ((pDoc = pMDIChild->GetActiveDocument ()) != 0)
				pDoc->UpdateAllViews (NULL, VIEWHINT_SERVER_SWITCH);
		}
	}
	catch(...)
	{ }
	return 0L;
}

// ------------------------------------------------------------------------
// message usually posted by scribe.
// message can be sent by the pump itself, as a notification to the Tasker
//       to clean up.
LRESULT CMainFrame::OnInternalDisconnect (WPARAM wParam, LPARAM lParam)
{
	if (0 == gpTasker)
		return 0;

	if (1 == wParam)
	{
		// cleanup E-Pump, but don't hangup Ras.
		gpTasker->SecondDisconnect ();
	}
	else
	{
		// cleanup everything : both pumps and hangup Ras.
		BOOL fIdleDisconnect = (BOOL) lParam;
		disconnect_hangup (fIdleDisconnect);
	}
	return 0;
}

// ------------------------------------------------------------------------
void CMainFrame::UpdatePinFilter ( BOOL fPinFilter )
{
	m_pViewFilterBar->UpdatePin ( fPinFilter );
}

// ------------------------------------------------------------------------
// posted by decode thread
LRESULT CMainFrame::OnLowSpace (WPARAM, LPARAM)
{
	CString str;
	str.LoadString (IDS_ERR_LOW_SPACE);
	MessageBox (str);
	return 0;
}

// ------------------------------------------------------------------------
// present explanation dlg to user
LRESULT CMainFrame::OnGroupRenumbered (WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
	{
		ASSERT(0);
	}
	else
	{
		PTYP_GROUPRENUM psRenum = (PTYP_GROUPRENUM) (void*) wParam;

		TRenumberDlg sDlg(this, psRenum->m_strNewsGroup,
			psRenum->m_iServerLow, psRenum->m_iPreviousLow);

		sDlg.DoModal ();

		delete psRenum;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// TRenumberDlg dialog

TRenumberDlg::TRenumberDlg(CWnd* pParent,
						   const CString& newsgroup,
						   int   iServerLow,
						   int   iPreviousLow)
						   : CDialog(TRenumberDlg::IDD, pParent),
						   m_strNewsgroup(newsgroup),
						   m_iServerLow(iServerLow),
						   m_iPreviousLow(iPreviousLow)
{
}

void TRenumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TRenumberDlg, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TRenumberDlg message handlers

BOOL TRenumberDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	CString str;
	str.Format (IDS_UTIL_RENUM,
		LPCTSTR(m_strNewsgroup),
		m_iServerLow,
		m_iPreviousLow);

	CWnd * pWnd = GetDlgItem(IDC_STATIC_ONE);
	if (pWnd)
		pWnd->SetWindowText (str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnBozoConverted(WPARAM, LPARAM)
{
	CString str;
	str.LoadString (IDS_BOZOS_CONVERTED);
	MessageBox (str);
	return 0;
}

// ------------------------------------------------------------------------
// Returns 0 for success.  These two functions should be paired together,
//   called alot from within this module.
int CMainFrame::disconnect_hangup(BOOL fIdleDisconnect)
{
	TRACE("CMainFrame::disconnect_hangup >\n");
	if (0 == gpTasker)
	{
		TRACE("CMainFrame::disconnect_hangup : gpTasker is NULL <\n");
		return 1;
	}

	TRACE("CMainFrame::disconnect_hangup : Calling gpTasker->Disconnect\n");
	gpTasker->Disconnect();
	TRACE("CMainFrame::disconnect_hangup : Calling dialMgr.Disconnect\n");
	dialMgr.Disconnect(fIdleDisconnect);

	TRACE("CMainFrame::disconnect_hangup <\n");
	return 0;
}

// ------------------------------------------------------------------------
void CMainFrame::OnVcr()
{
	CString dummy;

	OnVCRWorker ( false, dummy );
}

// ------------------------------------------------------------------------
void CMainFrame::OnVCRWorker (bool fVCRFile, const CString & filename)
{
	if (IsVCRWindowActive ())
		return;  // no recursion

	int stat;

	// TODO: Add your command handler code here
	ListDownloadInfo sDLInfo;

	{
		CWaitCursor  sWait;

		stat = GetServersDownloadInfo ( sDLInfo );
	}

	ASSERT(0 == stat);

	TVCRDialog sDlg(this, &sDLInfo, filename);

	if (IDOK == sDlg.DoModal ())
	{

		TVCRRunDlg * pRun = new TVCRRunDlg(this,
			sDlg.m_pOutList,
			sDlg.m_fSpecificTime,
			sDlg.m_oleTime,
			sDlg.m_fExitGravity);

		pRun->Create (IDD_VCR_RUNNING,    this);

		SetVCRWindow (pRun->m_hWnd);

		pRun->ShowWindow (SW_SHOW);
	}

	while (!sDLInfo.IsEmpty())
		delete sDLInfo.RemoveHead ();
}

// ------------------------------------------------------------------------
// Tasker needs this, so I made it a C-Function
//

bool CMainFrame::IsVCRWindowActive ()
{
	TEnterCSection loc(&m_csVCRDlg);
	return (m_hwVCRDlg != 0);
}

void CMainFrame::SetVCRWindow (HWND h)
{
	TEnterCSection loc(&m_csVCRDlg);
	m_hwVCRDlg = h;
}

// ------------------------------------------------------------------------
HWND CMainFrame::LockVCRWindow (bool fLock /* =true */)
{
	if (fLock)
	{
		EnterCriticalSection (&m_csVCRDlg);
		return m_hwVCRDlg;
	}
	else
	{
		LeaveCriticalSection (&m_csVCRDlg);
		return m_hwVCRDlg;
	}
}

// ------------------------------------------------------------------------
LRESULT CMainFrame::OnVCRGroupDone (WPARAM, LPARAM)
{
	// Q:  what if a bunch of headers are queued for decoding?

	HWND hDlg = LockVCRWindow ();

	if (hDlg)
		CWnd::FromHandle(hDlg)->PostMessage (WMU_VCR_NEXTSTEP);

	LockVCRWindow (false);
	return 0;
}

// ------------------------------------------------------------------------
void CMainFrame::OnSocketTrace()
{
	TTraceDlg socketTraceDialog(this);

	socketTraceDialog.DoModal();
}

// ------------------------------------------------------------------------
LRESULT CMainFrame::OnDisplayConnectError (WPARAM wParam, LPARAM lParam)
{
	CNewsDoc* pDoc = ((CNewsApp*) AfxGetApp())->GetGlobalNewsDoc();
	if (pDoc)
	{
		HWND hwndVCR = LockVCRWindow (true);

		if (hwndVCR)
		{
			CWnd::FromHandle(hwndVCR)->SendMessage (WMU_VCR_BADSERVER);

			LockVCRWindow (false);

			// free the memory
			pDoc->DisplayConnectError ( m_hWnd, true, lParam );

		}
		else
		{
			LockVCRWindow (false);
			pDoc->DisplayConnectError ( m_hWnd, false, lParam );
		}
	}

	return 0;
}

// ------------------------------------------------------------------------
LRESULT CMainFrame::OnReadyToRun(WPARAM,LPARAM)
{
	if (sCmdLineInfo.m_bVCRFile)
	{
		// hand off to utility function
		OnVCRWorker (sCmdLineInfo.m_bVCRFile, sCmdLineInfo.m_strVCRFile);
	}

	return 0;
}

// ------------------------------------------------------------------------
void CMainFrame::OnFileServerReConnectDelay ()
{
	TServerCountedPtr cpNewsServer;

	if (m_hTimerReconnect)
		KillTimer (m_hTimerReconnect);

	if (cpNewsServer->GetPausing())
	{
		m_hTimerReconnect = SetTimer (TIMERID_RECONNECT, 1000, NULL);

		m_iCountDownReconnect = 0;
		m_iCountDownReconnect = cpNewsServer->GetPauseCount();
	}
	else
	{
		PostMessage (WM_COMMAND, ID_FILE_SERVER_RECONNECT);
	}
}

// ------------------------------------------------------------------------
void CMainFrame::OnTimer(UINT nIDEvent)
{
	if (TIMERID_RECONNECT == nIDEvent)
	{
		if (-- m_iCountDownReconnect <= 0)
		{
			CancelReconnect ();
			PostMessage (WM_COMMAND, ID_FILE_SERVER_RECONNECT);
		}
		else
		{
			CString msg;
			gpUserDisplay->SetPos   (0, FALSE, FALSE);
			gpUserDisplay->SetRange (0, 10, FALSE);
			msg.Format (IDS_RECONNECT_SECS, m_iReconnectCount + 1, m_iCountDownReconnect);

			gpUserDisplay->SetText (msg, FALSE);
		}
	}
	else if (TIMERID_SPEEDOMETER == nIDEvent)
	{
		calc_download_speed ();
	}
	else if (TIMERID_UPDATECHECK == nIDEvent)
	{
		// The initial check for update after startup.
		// Kill the timer cause we only do this once.
		KillTimer(TIMERID_UPDATECHECK);
		if (gpGlobalOptions->GetRegSystem()->GetCheckForUpdates())
		{
			bool bFailed = false;
			if (CheckForProgramUpdate(bFailed))
			{
				// A program update is available, ask the user if they want to quit Gravity
				// and goto the Gravity website?
				if (MessageBox("A newer version of Gravity is available.\nWould you like to exit Gravity and visit the\n"
					"main Gravity website to download the latest version?", "Update Gravity?", MB_ICONQUESTION|MB_YESNO) == IDYES)
				{
					OnHelpVisitTheGravityWebsite();
					PostMessage(WM_CLOSE);
				}
			}
		}
	}

	CMDIFrameWnd::OnTimer(nIDEvent);
}

// ------------------------------------------------------------------------
void CMainFrame::CancelReconnect()
{
	TRACE("CMainFrame::CancelReconnect >\n");
	if (m_hTimerReconnect)
	{
		CString msg;

		KillTimer(TIMERID_RECONNECT);
		m_hTimerReconnect = 0;
		TRACE("CMainFrame::CancelReconnect : Killed reconnect timer\n");

		gpUserDisplay->SetText(msg, FALSE);
	}

	m_iCountDownReconnect = 0;
	TRACE("CMainFrame::CancelReconnect <\n");
}

// ------------------------------------------------------------------------
BOOL CMainFrame::OnQueryEndSession()
{
	if (!CMDIFrameWnd::OnQueryEndSession())
		return FALSE;

	if (handleClose (true) != 0)
		return FALSE;

	return TRUE;
}

// ------------------------------------------------------------------------
// Toggle the view filter between default and ShowAll
void CMainFrame::OnTogfilterShowall()
{
	CNewsView *pView = GetNewsView ();

	if (pView)
	{
		LONG lGroupID = pView->GetCurNewsGroupID();

		TServerCountedPtr cpNewsServer;
		BOOL fUseLock = FALSE;
		TNewsGroup* pNG = 0;

		// get the newsgroup object
		TNewsGroupUseLock useLock(cpNewsServer, lGroupID, &fUseLock, pNG);

		if (!fUseLock)
			return;

		int iDefaultFilter = pView->GetPreferredFilter ( pNG );

		m_pViewFilterBar->ToggleFilterShowall (iDefaultFilter);
	}
}

// ------------------------------------------------------------------------
int fnGetByteCount (DWORD & dwRet)
{
	HANDLE hMappedObject;
	TCHAR szMappedObjectName[] = TEXT("GRAVITY_COUNTER_MEMCHUNK");

	int stat;
	PVOID pVoid;

	hMappedObject = CreateFileMapping ((HANDLE) 0xFFFFFFFF,
		NULL,
		PAGE_READONLY,
		0,
		4096,
		szMappedObjectName);
	if (NULL == hMappedObject)
	{
		return 1;
	}

	// it _should_ exist
	if (ERROR_ALREADY_EXISTS !=  GetLastError())
	{
		CloseHandle (hMappedObject);
		return 1;
	}

	dwRet = 0;

	// mapped object created okay
	//
	pVoid =          MapViewOfFile (hMappedObject,
		FILE_MAP_READ,
		0,
		0,
		1 * sizeof(DWORD));

	if (NULL == pVoid)
	{
		TRACE1 ("Failed to Map View of File %x\n", GetLastError());
		stat = 1;
	}
	else
	{
		dwRet  =  ((DWORD*) pVoid)[0];

		VERIFY(UnmapViewOfFile (pVoid));
		stat = 0;
	}

	CloseHandle (hMappedObject);

	return stat;
}

// ------------------------------------------------------------------------
struct T_TIMESLICE
{
	DWORD     dwBytes;
	CTime     m_time;

	T_TIMESLICE()
		: dwBytes(0), m_time(CTime::GetCurrentTime())
	{

	}
};

typedef deque<T_TIMESLICE> QUE_TSLICE;

UINT  gintBytesPerSecond  = 0;

// ------------------------------------------------------------------------
void CMainFrame::calc_download_speed ()
{
	static QUE_TSLICE  queSlice;
	T_TIMESLICE sSlice;

	if (0 == fnGetByteCount(sSlice.dwBytes))
	{

	}

	if (queSlice.size() >= 6)
		queSlice.pop_front ();
	queSlice.push_back ( sSlice );

	int  tot = queSlice.size();

	if (tot >= 2)
	{
		T_TIMESLICE & rSliceA = (* (queSlice.begin()) );

		T_TIMESLICE & rSliceY = queSlice.at (tot - 2);

		T_TIMESLICE & rSliceZ = queSlice.at (tot - 1);

		DWORD  dwBytesTot  = rSliceZ.dwBytes - rSliceA.dwBytes;
		CTimeSpan span     = rSliceZ.m_time - rSliceA.m_time;
		DWORD  dwSecsTot   = span.GetTotalSeconds();

		dwBytesTot += 2 * (rSliceZ.dwBytes - rSliceY.dwBytes);
		span       =  rSliceZ.m_time - rSliceY.m_time;
		dwSecsTot  += 2 * span.GetTotalSeconds();

		if (dwSecsTot > 0)
		{
			gintBytesPerSecond = dwBytesTot / dwSecsTot;
		}
	}

	return;
}

// ------------------------------------------------------------------------
void CMainFrame::OnQuickFilter()
{
	if (IsWindow(m_pQuickFilter->m_hWnd))
	{
		if (m_pQuickFilter->IsIconic ())
			m_pQuickFilter->ShowWindow (SW_RESTORE);

		m_pQuickFilter->PostMessage (WMU_QUICK_FILTER);
	}
	else
	{
		m_pQuickFilter->Create (IDD_QUICKFILTER, this);
	}
	m_pQuickFilter->ShowWindow (SW_SHOW);
}

// ------------------------------------------------------------------------
LRESULT CMainFrame::OnQuickFilterApply (WPARAM, LPARAM lParam)
{
	m_pQuickFilterData =  reinterpret_cast<TQuickFilterData*>(lParam);

	GetNewsView()->RefreshCurrentNewsgroup ();

	delete m_pQuickFilterData;
	m_pQuickFilterData = 0;
	return 0;
}

// ------------------------------------------------------------------------
TQuickFilterData * fnGetQuickFilterData ()
{
	return ((CMainFrame*) AfxGetMainWnd())->GetQuickFilterData ();
}

// ------------------------------------------------------------------------
int CMainFrame::setup_toolbar ()
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP |
		CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;

	if (!m_wndToolBar.Create(this,
		dwStyle,
		AFX_IDW_TOOLBAR,
		ELEM(G_AllToolButtons),	// Number of buttons in complete raw button array
		G_AllToolButtons,		// Array of ALL buttons
		GetGravityRegKey()+"DkToolbar",
		"ToolBar Settings"))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// attach the hicolor bitmaps to the toolbar
	AttachToolbarImages (IDB_TOOLBAR, IDB_TOOLBAR_DISABLED);

	CSize szButton(31,30);
	CSize szBmp(24,24);
	m_wndToolBar.SetSizes(szButton, szBmp);

	m_wndToolBar.GetToolBarCtrl().AddButtons (ELEM(G_AllToolButtons), G_AllRawButtons);

	return 0;
}

// create an image list for the specified BMP resource
static void	MakeToolbarImageList (CBitmap & bmp,
								  COLORREF transparentColor,
								  UINT depth,
								  CImageList&	outImageList)
{
	// create a 24 bit image list with the same dimensions and number
	// of buttons as the toolbar
	// VERIFY (outImageList.Create (kImageWidth,
	//                              kImageHeight,
	//                              kToolBarBitDepth | ILC_MASK, kNumImages, 1));

	VERIFY (
		outImageList.Create (24, 24, depth | ILC_MASK, 1, 1)
		);

	outImageList.Add ( &bmp,transparentColor  );
	outImageList.SetBkColor ((COLORREF) ::GetSysColor (COLOR_BTNFACE));
}

// load the high color toolbar images and attach them to m_wndToolBar
void CMainFrame::AttachToolbarImages (UINT inNormalImageID,
									  UINT inDisabledImageID)
{
	m_bmpTool.LoadBitmap(inNormalImageID);
	m_bmpToolDisabled.LoadBitmap(inDisabledImageID );

	// make high-color image lists for each of the bitmaps
	::MakeToolbarImageList ( m_bmpTool,         RGB(192,192,192), ILC_COLOR8, m_ToolbarImages );
	::MakeToolbarImageList ( m_bmpToolDisabled, RGB(192,192,192), ILC_COLOR8, m_ToolbarImagesDisabled );
	::MakeToolbarImageList ( m_bmpTool,         RGB(192,192,192), ILC_COLOR8, m_ToolbarImagesHot );

	// get the toolbar control associated with the CToolbar object
	CToolBarCtrl&	barCtrl = m_wndToolBar.GetToolBarCtrl();

	// attach the image lists to the toolbar control
	barCtrl.SetImageList (&m_ToolbarImages);
	barCtrl.SetDisabledImageList (&m_ToolbarImagesDisabled);
	barCtrl.SetHotImageList (&m_ToolbarImages);
}


void CMainFrame::OnHelpVisitTheGravityWebsite()
{
	CString strGravityWebsite("https://github.com/taviso/mpgravity");

	ShellExecute (GetDesktopWindow ()->m_hWnd /* parent window */,
		_T("open")          /* command */,
		strGravityWebsite  /* file */,
		NULL            /* command line */,
		NULL            /* directory */,
		SW_SHOW         /* nCmdShow */);
}

//
// User has pressed "Check For Updates" on main menu.
//
void CMainFrame::CheckForUpdate()
{
	OnCheckForUpdate(0,0);
}

//
// User has pressed "Check For Updates" on options tab.
//
LRESULT CMainFrame::OnCheckForUpdate(WPARAM, LPARAM lParam)
{
	bool bFailed = false;
	if (CheckForProgramUpdate(bFailed) == true)
	{
		// A program update is available, ask the user if they want to quit Gravity
		// and goto the Gravity website?
		if (MessageBox("A newer version of Gravity is available.\nWould you like to exit Gravity and visit the\n"
			"main Gravity website to download the latest version?", "Update Gravity?", MB_ICONQUESTION|MB_YESNO) == IDYES)
		{
			OnHelpVisitTheGravityWebsite();
			PostMessage(WM_CLOSE);
		}
	}
	else
	{
		if (!bFailed)
		{
			// Running latest version
			AfxMessageBox (IDS_RUNNING_CURRENT_VERSION);
		}
		else
		{
			// Error checking for update
			AfxMessageBox (IDS_ERR_GETTING_UPDATE_INFO);
		}
	}

	return 0;
}

//
// Retreives the version number of the latest version of Gravity
// from the main website and checks it against our version.
//
// https://raw.githubusercontent.com/taviso/mpgravity/master/stable_ver.txt
//
bool CMainFrame::CheckForProgramUpdate(bool &bFailed, bool bCheckBetaAgainstStable /* = false */)
{
	bool bRV = false;

	VERINFO verInfo;
	TLicenseSystem::GetVersionInt(verInfo);

	CString strURL;
	if (verInfo.m_bBeta && !bCheckBetaAgainstStable)
		strURL = _T("https://raw.githubusercontent.com/taviso/mpgravity/master/beta_ver.txt");
	else
		strURL = _T("https://raw.githubusercontent.com/taviso/mpgravity/master/stable_ver.txt");

	CInternetSession ses;
	ses.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, (DWORD)5000);
	CHttpFile *pFile = NULL;
	try
	{
		pFile = (CHttpFile*)ses.OpenURL(strURL, 1, INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_SECURE);
		if (pFile)
		{
			CString strLine, strTemp;

			// Read the first line (is all we need)
			if (pFile->ReadString(strLine))
			{
			    ::MessageBox(NULL, strLine, "read", 0);
				// Check for a 404 error
				if (strLine.Find(_T("404")) == -1)
				{
					int nNewMajor = -1, nNewMinor = -1, nNewBuild = -1, nStart = 0;
					strTemp = strLine.Tokenize(_T("."), nStart);
					if (nStart != -1)
					{
						nNewMajor = atoi(strTemp);
						strTemp = strLine.Tokenize(_T("."), nStart);
						if (nStart != -1)
						{
							nNewMinor = atoi(strTemp);
							strTemp = strLine.Tokenize(_T("."), nStart);
							if (nStart != -1)
							{
								nNewBuild = atoi(strTemp);

								// Compare the latest build against our build
								if (nNewMajor > verInfo.m_nMajor)
									bRV = true;
								else
								{
									// Only compare the minor number if major is the same
									if (nNewMajor == verInfo.m_nMajor)
									{
										if (nNewMinor > verInfo.m_nMinor)
											bRV = true;
										else
										{
											// Only compare the build number is major and minor are the same
											if (nNewMinor == verInfo.m_nMinor)
											{
												if (nNewBuild > verInfo.m_nBuild)
													bRV = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			// If we haven't found an upgrade yet, and we are running a beta version
			// we want to give the user the chance to upgrade to a newer stable version
			// if found, so call us again with bCheckBetaAgainstStable set to true.
			// (this will happen when a beta branch ends and becomes the start of
			//  a stable branch, i.e. when 2.7.11 became 2.8.0)
			if (!bRV && verInfo.m_bBeta && !bCheckBetaAgainstStable)
				bRV = CheckForProgramUpdate(bFailed, true);
		}
	}
	catch(CInternetException *e)
	{
		e->Delete();
	}
	catch(...)
	{
	}

	if (pFile)
		pFile->Close();
	delete pFile;
	pFile = NULL;
	ses.Close();

	return bRV;
}

bool CMainFrame::QuietOnlineStop()
{
	TRACE("CMainFrame::QuietOnlineStop >\n");
	if (0 == gpTasker)
	{
		TRACE("CMainFrame::QuietOnlineStop : gpTasker is NULL <\n");
		return true;
	}

	if (gpTasker->EmergencyPumpBusy())
	{
		TRACE("CMainFrame::QuietOnlineStop : Calling gpTasker->StopEmergencyPump\n");
		gpTasker->StopEmergencyPump();
	}
	else
	{
		TRACE("CMainFrame::QuietOnlineStop : Calling CancelReconnect\n");
		CancelReconnect();  // abort reconnect countdown
		TRACE("CMainFrame::QuietOnlineStop : Calling disconnect_hangup\n");
		disconnect_hangup(FALSE);
		// RLW : This is the one that seems to stop all the background downloading
		//gpTasker->KillPrintAndDecodeThreads();

		TRACE("CMainFrame::QuietOnlineStop : Waiting for gpTasker to disconnect\n");
		int nRetries = 0;
		while (gpTasker->IsConnected() && (nRetries < 10))
		{
			Sleep(100);
			nRetries++;
			TRACE("CMainFrame::QuietOnlineStop : waiting for gpTasker to disconnect, %DmS\n", nRetries*100);
		}
	}
	TRACE("CMainFrame::QuietOnlineStop <\n");
	return (gpTasker->IsConnected() == FALSE);
}
