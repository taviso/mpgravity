/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsopt.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.11  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.10  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.9  2009/01/28 13:16:37  richard_wood
/*  Fixed typo in new code.
/*
/*  Revision 1.8  2009/01/27 10:39:47  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.7  2009/01/26 11:25:38  richard_wood
/*  Updated "Compress" icon.
/*  Updated "DejaNews" icon and URL to Google Groups.
/*  Added error message for invalid date time format string.
/*  Fixed function ID for Run Rule Manually button.
/*  Updated greyscale buttons to look a lot more ghost like (less contrast, easier to tell they're inactive)
/*
/*  Revision 1.6  2009/01/26 08:34:08  richard_wood
/*  Added Date / Time format validation.
/*
/*  Revision 1.5  2008/09/19 14:51:34  richard_wood
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

// NewsOPT.cpp
//
#include "stdafx.h"
#include "News.h"

#include "globals.h"

// Dialog boxes
#include "tdlgtmpv.h"            // a property page
#include "toperpg.h"             // TOperationsPage
#include "tmailpg.h"
#include "tsigpg.h"
#include "twastepg.h"
#include "tglobopt.h"
#include "tpostpg.h"
#include "replypg.h"
#include "decodpg.h"
#include "encodpg.h"
#include "twarnpg.h"
#include "fontpg.h"
#include "layoutpg.h"
#include "newsgrp.h"
#include "tdisppg.h"
#include "tsetpg.h"
#include "tserverspage.h"        // server setup PropertyPage
#include "timeutil.h"

#include "tprefpg.h"             // Preferences page
#include "tspellpg.h"            // TSpellPage
#include "toptionsmark.h"

#include "optshet.h"
#include "custmsg.h"
#include "gstorpg.h"
#include "kidsecur.h"

#include "tidarray.h"            // SetMessageIDsSaved()

#include "thotlink.h"
#include "urlsppt.h"
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
#include "newsurl.h"             // TNewsUrl object
#include "newsdoc.h"             // pDoc->ConfigurePanicKey
#include "navigpg.h"             // TOptionsNavigatePage
#include "safelnch.h"
#include "update.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;
extern HWND ghwndMainFrame;
BOOL  gfShutdownRename = FALSE;

/////////////////////////////////////////////////////////////////////////////
// HandleServerNameChange - If the server name changed, we need to present
//                          the user with the issues and possibly ask
//                          the user to quit news and restart.
//
//                   Note : I would prefer to make this a member of CNewsApp
//                          but I was faced with recompiling everything
//                          after changing news.h, so I left it out.  If
//                          you care, I'll will change it.
/////////////////////////////////////////////////////////////////////////////

TServerChangeDlg::RenameChoice HandleServerNameChange (LPCTSTR oldName, LPCTSTR newName)

{
	TServerChangeDlg  changeDlg;

	changeDlg.m_renameChoice = 0;
	changeDlg.m_oldName = oldName;
	changeDlg.m_newName = newName;

	changeDlg.DoModal ();

	return static_cast<TServerChangeDlg::RenameChoice> (changeDlg.m_renameChoice);
}

/////////////////////////////////////////////////////////////////////////////
// CNewsApp commands                                             _OPTIONS
//
// PropertyPage - small
// PropertySheet - the binder
//
//
//
void CNewsApp::OnOptionsConfigure()
{
	TServerCountedPtr cpNewsServer;

	TOptionsSheet        dlgOptions(IDS_OPTIONS);
	TDlgTemplateView     pgViewTemplate;
	TOptionsSignatureDlg pgSignature;
	TOptionsNavigatePage pgNavigate;
	TWastebasketPage     pgPurge;
	TOptionsPostingDlg   pgPosting;
	TOptionsReplyDlg     pgReply;
	TDecodePage          pgDecode;
	TOptionsEncodeDlg    pgEncode;
	TWarningPage         pgWarnings;
	TOptionsFontDlg      pgFonts;
	TOptionsLayoutPage   pgLayout;
	TStorageGlobalPage   pgStorage;
	TOptionsKidSecurePage pgKidSecure;
	THotlinkPage         pgHotlink;
	TOptionsDisplayDlg   pgDisplay;
	TPreferencesPage     pgPrefs;
	TSpellPage           pgSpell;
	CUpdateDlg           pgUpdate;
	TOptionsMark         pgMark;

	TRegPurge*           pRegPurge = gpGlobalOptions->GetRegPurge();

	BOOL fPromptForShutdown = FALSE;

	// utility object
	TNewsUrl sUrlObject;
	BOOL     fRegisteredURL;

	// keep original versions of the Header Control settings.
	// we don't want to change the widths unless we have a distinct change
	CString oldTreeColumns;

	// check before and after
	TGlobalDef::EThreadSort eOldThreadSort;

	pgSignature.m_TsigList.Set ( gpGlobalOptions->GetpSignatureList() );

	pgPurge.m_fPurgeRead = pRegPurge->GetPurgeRead();
	pgPurge.m_iReadLimit = pRegPurge->GetPurgeReadLimit();
	pgPurge.m_fPurgeUnread = pRegPurge->GetPurgeUnread();
	pgPurge.m_iUnreadLimit = pRegPurge->GetPurgeUnreadLimit();
	pgPurge.m_fOnHdr        = pRegPurge->GetPurgeOnHdrs();
	pgPurge.m_iDaysHdrPurge = pRegPurge->GetPurgeOnHdrsEvery();
	pgPurge.m_fShutdown     = pRegPurge->GetCompactOnExit();
	pgPurge.m_iShutCompact  = pRegPurge->GetCompactOnExitEvery();
	pgPurge.m_skipBytes     = pRegPurge->GetSkipAmount ();
	pgPurge.m_skipPercent   = pRegPurge->GetSkipPercent();

	pgViewTemplate.InitializeCustomView (gpGlobalOptions->GetCustomView());

	pgViewTemplate.m_fGetFullHeader = gpGlobalOptions->DownloadFullHeader ();
	pgViewTemplate.m_fUsePopup = gpGlobalOptions->UsingPopup ();
	pgViewTemplate.m_defBackground = gpGlobalOptions->GetRegSwitch()->IsDefaultArticleBG();
	pgViewTemplate.m_iQuotedTextMax = gpGlobalOptions->GetRegUI()->GetShowQuotedMaxLines();

	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors ();
	pgViewTemplate.m_background = pBackgrounds->GetArtviewBackground();
	pgViewTemplate.m_fSkipHTMLPart = gpGlobalOptions->GetRegSwitch()->GetSkipHTMLPart ();

	pgPosting.m_Indent    = gpGlobalOptions->GetIndentString();
	pgPosting.m_LineWrap  = gpGlobalOptions->GetWordWrap();
	gpGlobalOptions->GetPostingFont( &pgPosting.m_logFont,
		&pgPosting.m_PointSize );
	pgPosting.m_IDsToRemember     = gpGlobalOptions->GetIDsToRemember();
	pgPosting.m_strDistribution = gpGlobalOptions->GetDistribution();
	CopyCStringList (pgPosting.m_headers, gpGlobalOptions->GetCustomHeaders ());

	// encoding options
	pgEncode.m_SplitLen.Format ("%d", gpGlobalOptions->GetArticleSplitLen());
	pgEncode.m_subjectTemplate = gpGlobalOptions->GetSubjectTemplate();
	pgEncode.m_kEncoding = gpGlobalOptions->GetEncodingType ();
	pgEncode.m_ContentEncoding  = gpGlobalOptions->Get_MIME_TransferEncoding();
	pgEncode.m_ContentType      = gpGlobalOptions->Get_MIME_ContentType();
	pgEncode.m_subjectTemplate   = gpGlobalOptions->GetSubjectTemplate();
	pgEncode.m_fSeparateArts = gpGlobalOptions->IsAttachmentsInSeparateArticle();
	pgEncode.m_iSendCharset  = gpGlobalOptions->GetRegCompose()->GetSendCharset();
	pgEncode.m_fSend8Bit = gpGlobalOptions->GetRegCompose()->GetAllow8Bit();
	pgEncode.m_fSend8BitHeaders = gpGlobalOptions->GetSend8BitHdrs();

	pgReply.m_fLimitQuoted  = gpGlobalOptions->IsLimitingQuoteLines ();
	pgReply.m_maxQuoteLines.Format ("%d", gpGlobalOptions->GetMaxQuotedLines());
	pgReply.m_fPasteOnReply = gpGlobalOptions->IncludingOriginalMail ();
	pgReply.m_fPasteOnFollowup = gpGlobalOptions->IncludingOriginalFollowup ();
	pgReply.m_folIntro         = gpGlobalOptions->GetFollowupIntro ();
	pgReply.m_rplyIntro        = gpGlobalOptions->GetReplyIntro ();
	pgReply.m_CCIntro          = gpGlobalOptions->GetCCIntro ();

	pgDecode.m_decodeDirectory = gpGlobalOptions->GetDecodeDirectory();
	pgDecode.m_fAutomaticManualDecode = gpGlobalOptions->IsLaunchingViewerOnManual ();
	pgDecode.m_fAutomaticRuleDecode = gpGlobalOptions->IsLaunchViewerByRules ();
	pgDecode.m_strOtherViewer = gpGlobalOptions->GetOtherViewer ();
	pgDecode.m_bRestartPausedJobs = gpGlobalOptions->GetRegSwitch()->GetResumePausedDecJob ();
	pgDecode.m_iAfterDecode = (int) gpGlobalOptions->GetRegSwitch()->GetDecodeMarksRead ();
	pgDecode.m_bDeleteOnExit = gpGlobalOptions->GetRegSwitch ()->GetDeleteOnExit ();
	pgDecode.m_bWriteDescription = gpGlobalOptions->GetWriteDescription ();
	if (gpGlobalOptions->IsUseImageGallery ())
		pgDecode.m_iViewMethod = 0;
	else if (gpGlobalOptions->IsUseOtherViewer ())
		pgDecode.m_iViewMethod = 2;
	else
		pgDecode.m_iViewMethod = 1;
	pgDecode.m_iFilenameConflict = gpGlobalOptions->FilenameConflict();
	pgDecode.m_uiPauseSpace = (UINT) gpGlobalOptions->GetRegSwitch ()->GetPauseSpace ();
	pgDecode.m_fPausingSpace = gpGlobalOptions->GetRegSwitch ()->GetPausingLowSpace ();

	pgWarnings.m_fCatchup = gpGlobalOptions->WarnOnCatchup();
	pgWarnings.m_fExitCompose = gpGlobalOptions->WarnOnExitCompose();
	pgWarnings.m_fExitNews32 = gpGlobalOptions->WarnOnExitNews32();
	pgWarnings.m_fMarkRead = gpGlobalOptions->WarnOnMarkRead();
	pgWarnings.m_fUnsubscribe = gpGlobalOptions->WarnOnUnsubscribe();
	pgWarnings.m_fWarnOnSending = gpGlobalOptions->WarnOnSending();
	pgWarnings.m_fWarnOnDeleteBinary = gpGlobalOptions->WarnOnDeleteBinary();
	pgWarnings.m_fErrorDuringDecode = gpGlobalOptions->WarnOnErrorDuringDecode();
	pgWarnings.m_fWarnOnRunExe = gpGlobalOptions->WarnOnRunExe();
	pgWarnings.m_fManualRuleOffline = gpGlobalOptions->WarnOnManualRuleOffline();
	pgWarnings.m_extensions = gSafeLaunch.GetExtensionList();
	pgWarnings.m_fWarnOnDeleteArticle = gpGlobalOptions->WarnOnDeleteArticle();

	TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();
	pgHotlink.m_fUnderline     = pURLSettings->UnderliningLinks();

	// the following line is weird radio button crap, only satan
	// would do something like this...
	pgHotlink.m_fMailUseReg    = !pURLSettings->MailUsingRegistry();

	pgHotlink.m_fWebUseReg     = pURLSettings->WebUsingRegistry();
	pgHotlink.m_fFtpUseReg     = pURLSettings->FtpUsingRegistry();
//	pgHotlink.m_fGopherUseReg  = pURLSettings->GopherUsingRegistry();
//	pgHotlink.m_fTelnetUseReg  = pURLSettings->TelnetUsingRegistry();
	pgHotlink.m_colorRef       = pURLSettings->GetHotlinkColor ();
	pgHotlink.m_webPattern     = pURLSettings->GetWebPattern();
	pgHotlink.m_ftpPattern     = pURLSettings->GetFtpPattern();
//	pgHotlink.m_gopherPattern  = pURLSettings->GetGopherPattern();
//	pgHotlink.m_telnetPattern  = pURLSettings->GetTelnetPattern();
	pgHotlink.m_mailToPattern  = pURLSettings->GetMailToPattern();

	pgHotlink.m_fHighlightMail   = pURLSettings->HighlightMail();
	pgHotlink.m_fHighlightWeb    = pURLSettings->HighlightWeb();
	pgHotlink.m_fHighlightFtp    = pURLSettings->HighlightFtp();
//	pgHotlink.m_fHighlightGopher = pURLSettings->HighlightGopher();
//	pgHotlink.m_fHighlightTelnet = pURLSettings->HighlightTelnet();

	TUrlDde *pURL = gpGlobalOptions->GetWebHelperPointer();
	pgHotlink.m_webHelper = *pURL;
	pURL = gpGlobalOptions->GetFtpHelperPointer();
	pgHotlink.m_ftpHelper = *pURL;
	pURL = gpGlobalOptions->GetGopherHelperPointer();
	pgHotlink.m_gopherHelper = *pURL;
	pURL = gpGlobalOptions->GetTelnetHelperPointer();
	pgHotlink.m_telnetHelper = *pURL;

	pgFonts.m_fCustomTreeFont = FALSE;
	pgFonts.m_fCustomNGFont   = FALSE;
	if (gpGlobalOptions->IsCustomTreeFont())
	{
		CopyMemory (&pgFonts.m_treeFont, gpGlobalOptions->GetTreeFont(), sizeof(LOGFONT));
		pgFonts.m_treeColor = gpGlobalOptions->GetRegFonts()->GetTreeFontColor();
		pgFonts.m_fCustomTreeFont = TRUE;
	}
	else
		CopyMemory (&pgFonts.m_treeFont, &gpGlobalOptions->m_defTreeFont, sizeof(LOGFONT));

	if (gpGlobalOptions->IsCustomNGFont())
	{
		CopyMemory (&pgFonts.m_ngFont, gpGlobalOptions->GetNewsgroupFont(), sizeof(LOGFONT));
		pgFonts.m_newsgroupColor = gpGlobalOptions->GetRegFonts()->GetNewsgroupFontColor();
		pgFonts.m_fCustomNGFont   = TRUE;
	}
	else
		CopyMemory (&pgFonts.m_ngFont, &gpGlobalOptions->m_defNGFont, sizeof(LOGFONT));

	pgFonts.m_fDefaultThreadBG    = gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG();
	pgFonts.m_fDefaultNewsgroupBG = gpGlobalOptions->GetRegSwitch()->IsDefaultNGBG();

	pgFonts.m_newsgroupBackground = pBackgrounds->GetNewsgroupBackground();
	pgFonts.m_threadBackground    = pBackgrounds->GetTreeBackground();

	pgMark.m_fMR_display  = gpGlobalOptions->GetRegSwitch()->GetMarkReadDisplay();
	pgMark.m_fMR_filesave = gpGlobalOptions->GetRegSwitch()->GetMarkReadFileSave();
	pgMark.m_fMR_followup = gpGlobalOptions->GetRegSwitch()->GetMarkReadFollowup();
	pgMark.m_fMR_forward  = gpGlobalOptions->GetRegSwitch()->GetMarkReadForward();
	pgMark.m_fMR_reply    = gpGlobalOptions->GetRegSwitch()->GetMarkReadReply();

	pgLayout.DataIn (gpGlobalOptions->GetRegLayoutMdi());

	pgStorage.m_storageMode = gpGlobalOptions->GetStorageMode ();
	pgStorage.m_fCacheInDatabase = gpGlobalOptions->GetRegSwitch()->GetDBSaveDLMessages ();

	pgKidSecure.m_cipherText = gpGlobalOptions->GetSubscribePassword();

	// $ not politically correct
	//pgKidSecure.m_fPanicEnabled = gpGlobalOptions->GetRegSystem()->EnablePanicKey();
	//pgKidSecure.m_uPanicVKey = gpGlobalOptions->GetRegSystem()->PanicKey();

	pgUpdate.m_fCheckForUpdates = gpGlobalOptions->GetRegSystem()->GetCheckForUpdates();
	//pgUpdate.m_daysBetweenChecks.Format ("%d",gpGlobalOptions->GetRegSystem()->GetUpdateCheckDays());

	pgDisplay.m_oldArticleColor = gpGlobalOptions->GetRegFonts()->GetNewArticleColor();

	eOldThreadSort = gpGlobalOptions->GetThreadSort ();
	pgDisplay.m_kSortThread = eOldThreadSort;

	gpGlobalOptions->GetRegUI()->GetTreeColumnsRoles (oldTreeColumns);
	pgDisplay.m_threadCols = oldTreeColumns;
	pgDisplay.m_strDateFormat = gpGlobalOptions->GetRegUI()->GetDateFormatStr();
	pgDisplay.m_iShowThreadCollapsed = gpGlobalOptions->GetRegUI()->GetShowThreadsCollapsed();
	pgDisplay.m_fPureThreading = gpGlobalOptions->GetRegSwitch()->GetThreadPureThreading();
	pgDisplay.m_fXFaces = gpGlobalOptions->GetRegSwitch()->GetXFaces();

	pgPrefs.m_f1ClickOpenGroup = gpGlobalOptions->GetRegUI()->GetOneClickGroup();
	pgPrefs.m_f1ClickOpenArt =  gpGlobalOptions->GetRegUI()->GetOneClickArt();
	pgPrefs.m_iCatchupLocal = gpGlobalOptions->GetRegSwitch()->GetCatchUpAll() ? 1 : 0;
	pgPrefs.m_iCatchupNoCPM = gpGlobalOptions->GetRegSwitch()->GetCatchUpCPM() ? 1 : 0;
	pgPrefs.m_iCatchupKeepTags = gpGlobalOptions->GetRegSwitch()->GetCatchUpKeepTags() ? 1 : 0;
	pgPrefs.m_iCatchupLoadNext = gpGlobalOptions->GetRegSwitch()->GetCatchUpLoadNext() ? 1 : 0;
	pgPrefs.m_iCatchupOnRtrv = gpGlobalOptions->GetRegSwitch()->GetCatchUpOnRetrieve() ? 1 : 0;
	pgPrefs.m_fURLDefaultReader = fRegisteredURL = sUrlObject.IsDefaultReader();
	pgPrefs.m_fFocusInArtpane = gpGlobalOptions->GetRegSwitch()->GetReadPutFocusToArtpane () ;
	pgPrefs.m_maxLines        = gpGlobalOptions->GetRegUI()->GetMaxLines();
	pgPrefs.m_maxLinesCmd     = gpGlobalOptions->GetRegUI()->GetMaxLinesCmd();

	//pgNavigate.m_fIgnoreThenMoveNU = gpGlobalOptions->GetRegSwitch()->GetIgnoreThenMoveNU();
	//pgNavigate.m_fTagThenMoveNU    = gpGlobalOptions->GetRegSwitch()->GetTagThenMoveNU();
	//pgNavigate.m_fWatchThenMoveNU  = gpGlobalOptions->GetRegSwitch()->GetWatchThenMoveNU();
	//pgNavigate.m_fSingleKeyRead    = gpGlobalOptions->GetRegSwitch()->GetSingleKeyDoesRead();
	gpGlobalOptions->GetRegUI()->GetNavig1KeyRead (
		pgNavigate.m_vPref[ 0][0],pgNavigate.m_vPref[ 0][1],
		pgNavigate.m_vPref[ 1][0],pgNavigate.m_vPref[ 1][1]);
	gpGlobalOptions->GetRegUI()->GetNavigIgnore   (
		pgNavigate.m_vPref[ 2][0],pgNavigate.m_vPref[ 2][1],
		pgNavigate.m_vPref[ 3][0],pgNavigate.m_vPref[ 3][1]);
	gpGlobalOptions->GetRegUI()->GetNavigKillArt  (
		pgNavigate.m_vPref[ 4][0],pgNavigate.m_vPref[ 4][1],
		pgNavigate.m_vPref[ 5][0],pgNavigate.m_vPref[ 5][1]);
	gpGlobalOptions->GetRegUI()->GetNavigKillThrd (
		pgNavigate.m_vPref[ 6][0],pgNavigate.m_vPref[ 6][1],
		pgNavigate.m_vPref[ 7][0],pgNavigate.m_vPref[ 7][1]);
	gpGlobalOptions->GetRegUI()->GetNavigTag      (
		pgNavigate.m_vPref[ 8][0],pgNavigate.m_vPref[ 8][1],
		pgNavigate.m_vPref[ 9][0],pgNavigate.m_vPref[ 9][1]);
	gpGlobalOptions->GetRegUI()->GetNavigWatch    (
		pgNavigate.m_vPref[10][0],pgNavigate.m_vPref[10][1],
		pgNavigate.m_vPref[11][0],pgNavigate.m_vPref[11][1]);
	pgNavigate.m_fMButSKR          = gpGlobalOptions->GetRegSwitch()->GetMiddleButtonSKR();

	pgSpell.m_fSpellcheck = gpGlobalOptions->GetRegSwitch()->m_fSpellcheck;
	pgSpell.m_fSkipNumbers = gpGlobalOptions->GetRegSystem()->GetSpelling_IgnoreNumbers();
	pgSpell.m_strAffinityFile = gpGlobalOptions->GetRegSystem()->GetSpellingAffinity();
	pgSpell.m_strDictionaryFile = gpGlobalOptions->GetRegSystem()->GetSpellingDictionary();

	//////////////////////////////////////
	// Add all pages to the property sheet

	dlgOptions.AddPage ( &pgViewTemplate );   // Article Layout
	dlgOptions.AddPage ( &pgDecode );         // Decoding

	dlgOptions.AddPage ( &pgDisplay );        // Display
	dlgOptions.AddPage ( &pgEncode );         // Encoding
	dlgOptions.AddPage ( &pgFonts );          // Fonts

#if defined(SYMANTEC)
	dlgOptions.AddPage ( &pgMark );           // Mark
#endif

	dlgOptions.AddPage ( &pgNavigate );       // Navigation
	dlgOptions.AddPage ( &pgPosting );        // Posting
	dlgOptions.AddPage ( &pgPrefs );          // Preferences
	dlgOptions.AddPage ( &pgPurge );          // "Purging"
	dlgOptions.AddPage ( &pgReply );          // Replying
	dlgOptions.AddPage ( &pgKidSecure );      // Security
	dlgOptions.AddPage ( &pgSignature );      // Signature
	dlgOptions.AddPage ( &pgSpell );          // Spelling
	dlgOptions.AddPage ( &pgStorage);         // Storage
	dlgOptions.AddPage ( &pgUpdate);          // Update
	dlgOptions.AddPage ( &pgHotlink);         // URL Hotlinks
	dlgOptions.AddPage ( &pgWarnings);        // Warnings
	dlgOptions.AddPage ( &pgLayout );         // Window Layout

	gpGlobalOptions->GetRegUI()->GetActiveOptionsPage(&dlgOptions.m_iMRUPage);
	dlgOptions.SetActivePage ( dlgOptions.m_iMRUPage );

	// warning situation across 2 property pages

	if (IDOK == dlgOptions.DoModal())
	{
		gpGlobalOptions->GetRegUI()->SetActiveOptionsPage(dlgOptions.m_iMRUPage);

		gpGlobalOptions->SetCustomView ( pgViewTemplate.GetCustomView() );
		gpGlobalOptions->GetRegSwitch()->SetSkipHTMLPart (pgViewTemplate.m_fSkipHTMLPart);

		// --
		gpGlobalOptions->SetDlFullHeader(pgViewTemplate.m_fGetFullHeader);
		gpGlobalOptions->SetUsePopup (pgViewTemplate.m_fUsePopup);
		gpGlobalOptions->SetTreeFont (&pgViewTemplate.m_treeFont);

		pBackgrounds->SetArtviewBackground(pgViewTemplate.m_background);
		gpGlobalOptions->GetRegSwitch()->SetDefaultArticleBG(pgViewTemplate.m_defBackground);
		gpGlobalOptions->GetRegUI()->SetShowQuotedMaxLines(pgViewTemplate.m_iQuotedTextMax);

		gpGlobalOptions->SetSignatureList ( &pgSignature.m_TsigList );

		pRegPurge->SetPurgeRead ( pgPurge.m_fPurgeRead );
		pRegPurge->SetPurgeReadLimit ( pgPurge.m_iReadLimit );
		pRegPurge->SetPurgeUnread ( pgPurge.m_fPurgeUnread );
		pRegPurge->SetPurgeUnreadLimit ( pgPurge.m_iUnreadLimit );
		pRegPurge->SetPurgeOnHdrs ( pgPurge.m_fOnHdr );
		pRegPurge->SetPurgeOnHdrsEvery ( pgPurge.m_iDaysHdrPurge );
		pRegPurge->SetCompactOnExit ( pgPurge.m_fShutdown );
		pRegPurge->SetCompactOnExitEvery ( pgPurge.m_iShutCompact );
		pRegPurge->SetSkipAmount (pgPurge.m_skipBytes);
		pRegPurge->SetSkipPercent (pgPurge.m_skipPercent);

		gpGlobalOptions->SetIndentString (pgPosting.m_Indent);
		gpGlobalOptions->SetWordWrap (pgPosting.m_LineWrap);
		gpGlobalOptions->SetPostingFont( &pgPosting.m_logFont,
			pgPosting.m_PointSize );

		gpGlobalOptions->SetIDsToRemember (pgPosting.m_IDsToRemember);
		gpGlobalOptions->SetDistribution (pgPosting.m_strDistribution);
		gpGlobalOptions->SetCustomHeaders (pgPosting.m_headers);

		// notify the global TIDArray object of its new setting
		(gpStore->GetIDArray ()).SetMessageIDsSaved (WORD(pgPosting.m_IDsToRemember));

		gpGlobalOptions->SetLimitQuoteLines (pgReply.m_fLimitQuoted);
		gpGlobalOptions->SetMaxQuotedLines (atol (pgReply.m_maxQuoteLines));
		gpGlobalOptions->SetIncludeOriginalMail (pgReply.m_fPasteOnReply);
		gpGlobalOptions->SetIncludeOriginalFollowup(pgReply.m_fPasteOnFollowup);
		gpGlobalOptions->SetFollowupIntro (pgReply.m_folIntro);
		gpGlobalOptions->SetReplyIntro (pgReply.m_rplyIntro);
		gpGlobalOptions->SetCCIntro (pgReply.m_CCIntro);

		gpGlobalOptions->SetArticleSplitLen( atoi(pgEncode.m_SplitLen) );
		gpGlobalOptions->SetSubjectTemplate (pgEncode.m_subjectTemplate);
		gpGlobalOptions->SetEncodingType (TGlobalDef::EEncoding(pgEncode.m_kEncoding));
		gpGlobalOptions->Set_MIME_TransferEncoding (pgEncode.m_ContentEncoding);
		gpGlobalOptions->Set_MIME_ContentType (pgEncode.m_ContentType);
		gpGlobalOptions->SetSubjectTemplate (pgEncode.m_subjectTemplate);
		gpGlobalOptions->SetAttachmentsInSeparateArticle(pgEncode.m_fSeparateArts);
		gpGlobalOptions->GetRegCompose()->SetSendCharset(pgEncode.m_iSendCharset);
		gpGlobalOptions->GetRegCompose()->SetAllow8Bit(pgEncode.m_fSend8Bit);
		gpGlobalOptions->GetRegCompose()->Set8BitHdrs(pgEncode.m_fSend8BitHeaders);

		gpGlobalOptions->SetDecodeDirectory(pgDecode.m_decodeDirectory);
		gpGlobalOptions->SetLaunchViewerOnManual (pgDecode.m_fAutomaticManualDecode);
		gpGlobalOptions->SetLaunchViewerByRules (pgDecode.m_fAutomaticRuleDecode);
		gpGlobalOptions->SetOtherViewer (pgDecode.m_strOtherViewer);
		gpGlobalOptions->GetRegSwitch()->SetResumePausedDecJob (pgDecode.m_bRestartPausedJobs);
		gpGlobalOptions->GetRegSwitch()->SetDecodeMarksRead ((TRegSwitch::EPostDecode)pgDecode.m_iAfterDecode);
		gpGlobalOptions->GetRegSwitch()->SetDeleteOnExit (pgDecode.m_bDeleteOnExit);
		gpGlobalOptions->SetWriteDescription (pgDecode.m_bWriteDescription);
		gpGlobalOptions->SetUseOtherViewer (pgDecode.m_iViewMethod == 2);
		gpGlobalOptions->SetUseImageGallery (pgDecode.m_iViewMethod == 0);
		gpGlobalOptions->SetFilenameConflict(pgDecode.m_iFilenameConflict);
		gpGlobalOptions->GetRegSwitch ()->SetPauseSpace ((int) pgDecode.m_uiPauseSpace);
		gpGlobalOptions->GetRegSwitch ()->SetPausingLowSpace (pgDecode.m_fPausingSpace);

		gpGlobalOptions->SetWarnOnCatchup(pgWarnings.m_fCatchup);
		gpGlobalOptions->SetWarnOnExitCompose(pgWarnings.m_fExitCompose);
		gpGlobalOptions->SetWarnOnExitNews32(pgWarnings.m_fExitNews32);
		gpGlobalOptions->SetWarnOnMarkRead(pgWarnings.m_fMarkRead);
		gpGlobalOptions->SetWarnOnUnsubscribe(pgWarnings.m_fUnsubscribe);
		gpGlobalOptions->SetWarnOnSending(pgWarnings.m_fWarnOnSending);
		gpGlobalOptions->SetWarnOnDeleteBinary(pgWarnings.m_fWarnOnDeleteBinary);
		gpGlobalOptions->SetWarnOnErrorDuringDecode(pgWarnings.m_fErrorDuringDecode);
		gpGlobalOptions->SetWarnOnRunExe(pgWarnings.m_fWarnOnRunExe);
		gpGlobalOptions->SetWarnOnManualRuleOffline(pgWarnings.m_fManualRuleOffline);
		gpGlobalOptions->SetWarnOnDeleteArticle (pgWarnings.m_fWarnOnDeleteArticle);

		gSafeLaunch.ReplaceAll (pgWarnings.m_extensions);

		TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();
		pURLSettings->SetUnderliningLinks(pgHotlink.m_fUnderline);
		pURLSettings->SetMailUsingRegistry(!pgHotlink.m_fMailUseReg);
		pURLSettings->SetWebUsingRegistry(pgHotlink.m_fWebUseReg);
		pURLSettings->SetFtpUsingRegistry(pgHotlink.m_fFtpUseReg);
//		pURLSettings->SetGopherUsingRegistry(pgHotlink.m_fGopherUseReg);
//		pURLSettings->SetTelnetUsingRegistry(pgHotlink.m_fTelnetUseReg);
		pURLSettings->SetHotlinkColor (pgHotlink.m_colorRef);
		pURLSettings->SetWebPattern(pgHotlink.m_webPattern   );
		pURLSettings->SetFtpPattern(pgHotlink.m_ftpPattern   );
//		pURLSettings->SetGopherPattern(pgHotlink.m_gopherPattern);
//		pURLSettings->SetTelnetPattern(pgHotlink.m_telnetPattern);
		pURLSettings->SetMailToPattern(pgHotlink.m_mailToPattern);
		pURLSettings->SetHighlightMail(pgHotlink.m_fHighlightMail);
		pURLSettings->SetHighlightWeb(pgHotlink.m_fHighlightWeb);
		pURLSettings->SetHighlightFtp(pgHotlink.m_fHighlightFtp);
//		pURLSettings->SetHighlightGopher(pgHotlink.m_fHighlightGopher);
//		pURLSettings->SetHighlightTelnet(pgHotlink.m_fHighlightTelnet);

		pURL = gpGlobalOptions->GetWebHelperPointer();
		*pURL = pgHotlink.m_webHelper;

		pURL = gpGlobalOptions->GetFtpHelperPointer();
		*pURL = pgHotlink.m_ftpHelper;

		pURL = gpGlobalOptions->GetGopherHelperPointer();
		*pURL = pgHotlink.m_gopherHelper;

		pURL = gpGlobalOptions->GetTelnetHelperPointer();
		*pURL = pgHotlink.m_telnetHelper;

		gpGlobalOptions->SetStorageMode ((TNewsGroup::EStorageOption) pgStorage.m_storageMode);
		gpGlobalOptions->GetRegSwitch()->SetDBSaveDLMessages (pgStorage.m_fCacheInDatabase);

		gpGlobalOptions->GetRegUI()->SetTreeColumnsRoles (pgDisplay.m_threadCols);
		CString strOldDateFormat = gpGlobalOptions->GetRegUI()->GetDateFormatStr();
		// Verify the time/date string format
		CString strTDFormat = pgDisplay.m_strDateFormat;
		CString strValidFormatSpecifiers("aAbBcdHIjmMpSUwWxXyYZz%");
		// Simply try to use it to generate a TD string
		bool bBad = false;
		int nLength = strTDFormat.GetLength();
		for (int i = 0; i < nLength && !bBad; i++)
		{
			if (strTDFormat[i] == '%')
			{
				if (i == nLength-1)
				{
					// Format starting char but no specifier - error
					strTDFormat.LoadString(IDS_ALTERNATE_DATE_2);
					bBad = true;
				}
				else if (strValidFormatSpecifiers.Find(strTDFormat[i+1]) == -1)
				{
					// Not a valid format specifier. It could be the '#' modifier?
					if (strTDFormat[i+1] == '#')
					{
						// '#' modifier - carry on, next char should be a valid format specifier
						if (i == nLength-2)
						{
							// Format starting char followed by a modifier but no specifier - error
							strTDFormat.LoadString(IDS_ALTERNATE_DATE_2);
							bBad = true;
						}
						else if (strValidFormatSpecifiers.Find(strTDFormat[i+2]) == -1)
						{
							// Not found, invalid format character.
							strTDFormat.LoadString(IDS_ALTERNATE_DATE_2);
							bBad = true;
						}
						else
							i+=2; // Valid modifier and specifier, increment loop to skip over both
					}
					else
					{
						// Not found, invalid format character.
						strTDFormat.LoadString(IDS_ALTERNATE_DATE_2);
						bBad = true;
					}
				}
				else // Else it is a valid specifier, increment the loop to skip over it
					i++;
			}
		}
		if (bBad)
			::AfxGetMainWnd()->MessageBox(
				"The DateTime format string you entered is invalid.\n"
				"Please refer to the Help for the list of valid formats.\n"
				"The format string has been reset to a default setting.",
				"DateTime Format string Error", MB_ICONHAND|MB_OK);
		gpGlobalOptions->GetRegUI()->SetDateFormatStr(strTDFormat);
		gpGlobalOptions->GetRegUI()->SetShowThreadsCollapsed(pgDisplay.m_iShowThreadCollapsed);
		gpGlobalOptions->GetRegSwitch()->SetThreadPureThreading(pgDisplay.m_fPureThreading);
		gpGlobalOptions->GetRegSwitch()->SetXFaces(pgDisplay.m_fXFaces);

		CMDIChildWnd * pMDIChild;
		CMDIFrameWnd * pMDIFrame = (CMDIFrameWnd*) AfxGetMainWnd();
		// note this is only 1 of the mdi children. could be more.
		//  we aren't handling them.
		pMDIChild = pMDIFrame->MDIGetActive();

		if (strOldDateFormat != pgDisplay.m_strDateFormat)
			pMDIChild->Invalidate();

		gpGlobalOptions->SetThreadSort ((TGlobalDef::EThreadSort) pgDisplay.m_kSortThread);

		if (!pgDisplay.ColumnsEquivalent(oldTreeColumns, pgDisplay.m_threadCols))
			pMDIChild->SendMessage (WMU_TREEVIEW_RESETHDR);

		if (eOldThreadSort != gpGlobalOptions->GetThreadSort())
			pMDIChild->SendMessage (WMU_TREEVIEW_REFILL);

		gpGlobalOptions->CustomTreeFont (pgFonts.m_fCustomTreeFont);
		if (pgFonts.m_fCustomTreeFont)
		{
			gpGlobalOptions->SetTreeFont ( &pgFonts.m_treeFont );
			gpGlobalOptions->GetRegFonts()->SetTreeFontColor( pgFonts.m_treeColor );
		}
		// apply the tree font
		pMDIChild->PostMessage ( WMU_TREEVIEW_NEWFONT );

		gpGlobalOptions->CustomNGFont (pgFonts.m_fCustomNGFont);
		if (pgFonts.m_fCustomNGFont)
		{
			gpGlobalOptions->SetNewsgroupFont ( &pgFonts.m_ngFont );
			gpGlobalOptions->GetRegFonts()->SetNewsgroupFontColor( pgFonts.m_newsgroupColor );
		}
		// apply the new ng font
		pMDIChild->PostMessage ( WMU_NEWSVIEW_NEWFONT );

		gpGlobalOptions->GetRegSwitch()->SetDefaultTreeBG(pgFonts.m_fDefaultThreadBG);
		gpGlobalOptions->GetRegSwitch()->SetDefaultNGBG(pgFonts.m_fDefaultNewsgroupBG);
		pBackgrounds->SetTreeBackground(pgFonts.m_threadBackground);
		pBackgrounds->SetNewsgroupBackground(pgFonts.m_newsgroupBackground);

		gpGlobalOptions->GetRegFonts()->SetNewArticleColor (pgDisplay.m_oldArticleColor);

		// apply new formatting to the RichEdit control
		if (pgViewTemplate.m_fChanged)
			pMDIChild->PostMessage ( WMU_ARTVIEW_NEWTEMPLATE );

		if (pgLayout.IsDifferent())
		{
			TRegLayoutMdi * pRLM = gpGlobalOptions->GetRegLayoutMdi();
			pgLayout.DataOut (pRLM);

			// apply new layout
			pMDIChild->PostMessage ( WMU_NEWSMDI_NEWLAYOUT );
		}

		gpGlobalOptions->SetSubscribePassword(pgKidSecure.m_cipherText);
		//gpGlobalOptions->GetRegSystem()->EnablePanicKey(pgKidSecure.m_fPanicEnabled);
		//gpGlobalOptions->GetRegSystem()->PanicKey(pgKidSecure.m_uPanicVKey);

		gpGlobalOptions->GetRegSystem()->SetCheckForUpdates(pgUpdate.m_fCheckForUpdates);
		//gpGlobalOptions->GetRegSystem()->SetUpdateCheckDays(atol(pgUpdate.m_daysBetweenChecks));

		// Immediately tell the Keyboard hook dll about the new key,
		//  or disable it altogether
		CNewsDoc* pDoc = ((CNewsApp*)AfxGetApp())->GetGlobalNewsDoc();
		pDoc->ConfigurePanicKey (pgKidSecure.m_fPanicEnabled);

		gpGlobalOptions->GetRegUI()->SetOneClickGroup(pgPrefs.m_f1ClickOpenGroup);
		gpGlobalOptions->GetRegUI()->SetOneClickArt(pgPrefs.m_f1ClickOpenArt);
		gpGlobalOptions->GetRegUI()->SetMaxLines(pgPrefs.m_maxLines);
		gpGlobalOptions->GetRegUI()->SetMaxLinesCmd (pgPrefs.m_maxLinesCmd);

		// these are radio button index
		gpGlobalOptions->GetRegSwitch()->SetCatchUpAll(pgPrefs.m_iCatchupLocal);
		gpGlobalOptions->GetRegSwitch()->SetCatchUpCPM(pgPrefs.m_iCatchupNoCPM);
		gpGlobalOptions->GetRegSwitch()->SetCatchUpKeepTags(pgPrefs.m_iCatchupKeepTags);
		gpGlobalOptions->GetRegSwitch()->SetCatchUpLoadNext(pgPrefs.m_iCatchupLoadNext);
		gpGlobalOptions->GetRegSwitch()->SetCatchUpOnRetrieve(pgPrefs.m_iCatchupOnRtrv);
		gpGlobalOptions->GetRegSwitch()->SetReadPutFocusToArtpane (pgPrefs.m_fFocusInArtpane) ;
		// if different do something
		if (pgPrefs.m_fURLDefaultReader != fRegisteredURL)
		{
			// turn on or off
			sUrlObject.SetDefaultReader (pgPrefs.m_fURLDefaultReader);
		}

		//gpGlobalOptions->GetRegSwitch()->SetIgnoreThenMoveNU(pgNavigate.m_fIgnoreThenMoveNU);
		//gpGlobalOptions->GetRegSwitch()->SetTagThenMoveNU   (pgNavigate.m_fTagThenMoveNU);
		//gpGlobalOptions->GetRegSwitch()->SetWatchThenMoveNU (pgNavigate.m_fWatchThenMoveNU);
		//gpGlobalOptions->GetRegSwitch()->SetSingleKeyDoesRead(pgNavigate.m_fSingleKeyRead);

		gpGlobalOptions->GetRegUI()->SetNavig1KeyRead (
			pgNavigate.m_vPref[ 0][0],pgNavigate.m_vPref[ 0][1],
			pgNavigate.m_vPref[ 1][0],pgNavigate.m_vPref[ 1][1]);
		gpGlobalOptions->GetRegUI()->SetNavigIgnore   (
			pgNavigate.m_vPref[ 2][0],pgNavigate.m_vPref[ 2][1],
			pgNavigate.m_vPref[ 3][0],pgNavigate.m_vPref[ 3][1]);
		gpGlobalOptions->GetRegUI()->SetNavigKillArt  (
			pgNavigate.m_vPref[ 4][0],pgNavigate.m_vPref[ 4][1],
			pgNavigate.m_vPref[ 5][0],pgNavigate.m_vPref[ 5][1]);
		gpGlobalOptions->GetRegUI()->SetNavigKillThrd (
			pgNavigate.m_vPref[ 6][0],pgNavigate.m_vPref[ 6][1],
			pgNavigate.m_vPref[ 7][0],pgNavigate.m_vPref[ 7][1]);
		gpGlobalOptions->GetRegUI()->SetNavigTag      (
			pgNavigate.m_vPref[ 8][0],pgNavigate.m_vPref[ 8][1],
			pgNavigate.m_vPref[ 9][0],pgNavigate.m_vPref[ 9][1]);
		gpGlobalOptions->GetRegUI()->SetNavigWatch    (
			pgNavigate.m_vPref[10][0],pgNavigate.m_vPref[10][1],
			pgNavigate.m_vPref[11][0],pgNavigate.m_vPref[11][1]);
		gpGlobalOptions->GetRegSwitch()->SetMiddleButtonSKR  (pgNavigate.m_fMButSKR);

#if defined(SYMANTEC)
		gpGlobalOptions->GetRegSwitch()->SetMarkReadDisplay ( pgMark.m_fMR_display);
		gpGlobalOptions->GetRegSwitch()->SetMarkReadFileSave( pgMark.m_fMR_filesave);
		gpGlobalOptions->GetRegSwitch()->SetMarkReadFollowup( pgMark.m_fMR_followup);
		gpGlobalOptions->GetRegSwitch()->SetMarkReadForward ( pgMark.m_fMR_forward);
		gpGlobalOptions->GetRegSwitch()->SetMarkReadReply   ( pgMark.m_fMR_reply);
#endif

		if (true)
		{
			gpGlobalOptions->GetRegSwitch()->m_fSpellcheck = pgSpell.m_fSpellcheck;
			gpGlobalOptions->GetRegSystem()->SetSpelling_IgnoreNumbers(pgSpell.m_fSkipNumbers);
			gpGlobalOptions->GetRegSystem()->SetSpellingAffinity(pgSpell.m_strAffinityFile);
			gpGlobalOptions->GetRegSystem()->SetSpellingDictionary(pgSpell.m_strDictionaryFile);
		}

		gpStore->SaveGlobalOptions();
		cpNewsServer->SaveSettings();
		CompileURLExpression (gpGlobalOptions);

		if (fPromptForShutdown)
		{
			gptrApp->ExitServer ();
			gpStore->CheckServerRename();
			gptrApp->SwitchServer (cpNewsServer->GetNewsServerName());
		}
	}
}

// ------------------------------------------------------------------------
// Function checks for two things.  "Catchup before getting headers"
//  and "Automatically Cycle getting headers"
BOOL CNewsApp::fnAutoCycleWarning ( TOperationPage    * ppgOperate,
								   TPreferencesPage  * ppgPrefs)
{
	if (ppgPrefs)
	{
		TServerCountedPtr cpNewsServer;
		return (ppgPrefs->m_iCatchupOnRtrv &&
			cpNewsServer->GetAutomaticCycle());
	}
	else
		return (gpGlobalOptions->GetRegSwitch()->GetCatchUpOnRetrieve() &&
		ppgOperate->m_fAutoCycle);
}
