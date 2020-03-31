/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mainfrm.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2010/01/07 17:35:51  richard_wood
/*  Updates for 2.9.14.
/*
/*  Revision 1.2  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.4  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.3  2008/09/19 14:51:30  richard_wood
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

// mainfrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "richedit.h"
#include "tsrchdlg.h"
#include "evtdlg.h"
#include "tdecdlg.h"             // TDecodeDialog
#include "tprndlg.h"             // TPrintDialog
#include "statbar.h"
#include "bucket.h"
#include "tmrbar.h"              // TManualRuleBar
#include "outbxdlg.h"            // TOutboxBar
#include "tdraftsdlg.h"           // TDraftDlg
#include "tcompdlg.h"            // TCompactDlg
#include "update.h"
#include "DlgQuickFilter.h"
#include "dktoolbar.h"

class TStatusWrap;
class COutboxDlg;
class TExpiryData;
class TGotoArticle ;
class TViewFilterBar ;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	enum { ID_HIDDEN_RTF = 104242 };

	CMainFrame();
	virtual ~CMainFrame();

	CWnd* GetMessageBar();

	bool ExistServerAnchoredWindows ();
	void UnsubscribeFromSampled ();

	// action for filter bar
	void UpdatePinFilter ( BOOL fPinFilter );
	int SelectFilter (int iFilterID);

	bool IsVCRWindowActive ();
	void SetVCRWindow (HWND h);
	HWND LockVCRWindow (bool fLock = true);    // handles lock, unlock

	TQuickFilterData * GetQuickFilterData() { return m_pQuickFilterData; }

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void CalcFixedFontWordwrap (CHARFORMAT* pcf, int WordWrap, int& pixelWidth);
	
	bool CheckForProgramUpdate(bool &bFailed, bool bCheckBetaAgainstStable = false);

	void SetDoNotCompactDBOnShutdown(bool bNoCompact)
		{ m_bDoNotCompactDBOnShutdown = bNoCompact; };
	
	bool QuietOnlineStop();

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void CheckForUpdate();
	afx_msg void OnHelpVisitTheGravityWebsite();
	afx_msg void OnViewStatusBar();
	afx_msg void OnUpdateViewStatusBar(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnEditCopy();
	afx_msg void OnSearchSearch();
	afx_msg void OnViewEventlog();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnHelp();
	afx_msg void OnCurrentPrintJobs();
	afx_msg void OnFileServerConnect();
	afx_msg void OnFileServerReConnect();
	afx_msg void OnFileServerReConnectDelay();
	afx_msg void OnUpdateFileServerConnect(CCmdUI* pCmdUI);
	afx_msg void OnCurrentDecodeJobs();
	afx_msg void OnToggleToolbar();
	afx_msg void OnHelpHowtobuynews32();
	afx_msg void OnHelpGettingthemostoutofnews32();
	afx_msg void OnOutbox();
	afx_msg void OnDraftMsgs();
	afx_msg void OnUpdateSearchSearch(CCmdUI* pCmdUI);
	afx_msg void OnAppFetchTagged();
	afx_msg void OnUpdateAppFetchTagged(CCmdUI* pCmdUI);
	afx_msg void OnOnlineStop();
	afx_msg void OnToggleRuleBar();
	afx_msg void OnUpdateViewRuleBar (CCmdUI* pCmdUI);
	afx_msg void OnToggleOutboxBar();
	afx_msg void OnUpdateViewOutboxBar (CCmdUI* pCmdUI);
	afx_msg void OnDatePurgeAll();
	afx_msg void OnUpdateDatePurgeAll (CCmdUI* pCmdUI);
	afx_msg void OnChooseFilterDisplay();
	afx_msg void OnDefineFilterDisplay();
	afx_msg void OnToggleFilterBar();
	afx_msg void OnUpdateToggleFilterBar (CCmdUI* pCmdUI);
	afx_msg void OnUpdateNgCounts();
	afx_msg void OnUpdateUpdateNgCounts(CCmdUI* pCmdUI);
	afx_msg void OnViewRulestats();
	afx_msg void OnHelpTutorial();
	afx_msg void OnNewsrcExport();
	afx_msg void OnNewsrcImport();
	afx_msg void OnImageGallery();
	afx_msg void OnToolbarCustomize();
	afx_msg void OnSaveBarState();
	afx_msg void OnDejanews();
	afx_msg void OnScoring();
	afx_msg void OnUpdateRescore(CCmdUI* pCmdUI);
	afx_msg void OnRescore();
	afx_msg void OnUpdateQuickScore(CCmdUI* pCmdUI);
	afx_msg void OnQuickScore();
	afx_msg void OnScoreColors();
	afx_msg void OnUpdateApplyBozo(CCmdUI* pCmdUI);
	afx_msg void OnApplyBozo();
	afx_msg void OnVcr();
	afx_msg void OnSocketTrace();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnTogfilterShowall();
	afx_msg void OnQuickFilter();
	afx_msg void OnDestroy();
	afx_msg void OnBetaTestAll(UINT id);
	afx_msg LRESULT OnDockDirBarOnRightOfToolbar(WPARAM, LPARAM);
	afx_msg LRESULT OnMode1HeadersDone(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnNgroupHeadersDone(WPARAM wP, LPARAM lP);
	//afx_msg LRESULT OnBodiesDone(WPARAM wP, LPARAM lP);
	//afx_msg LRESULT OnOneBodyDone(WPARAM wP, LPARAM lP);
	afx_msg LRESULT  OnSetMessageString(WPARAM wP, LPARAM lP);
	afx_msg LRESULT  OnCheckUIPipe (WPARAM wP, LPARAM lP);
	afx_msg LRESULT  OnDisplayArtcount (WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnRefreshOutbox(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnRefreshDraft(WPARAM wP, LPARAM lP);

	afx_msg LRESULT OnPumpArticle (WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnProcessPumpArticle(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnNonBlockingCursor(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnConnectOK(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnGetlistDone(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnRefreshOutboxBar(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnExpireArticles(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnErrorFromServer(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnForceFilterChange(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnNewsURL(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnCopyData(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnSplashScreenGone(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnPanicKeyMsg(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnResetFactoryColumns(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnServerSwitch(WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnInternalDisconnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLowSpace(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGroupRenumbered(WPARAM, LPARAM);
	afx_msg LRESULT OnBozoConverted(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateCheckComplete(WPARAM, LPARAM);
	afx_msg LRESULT OnRasDialSuccess(WPARAM,LPARAM);
	afx_msg LRESULT OnRasDialBusy(WPARAM,LPARAM);
	afx_msg LRESULT OnVCRGroupDone(WPARAM,LPARAM);
	afx_msg LRESULT OnVCRIdle(WPARAM,LPARAM);
	afx_msg LRESULT OnDisplayConnectError(WPARAM,LPARAM);
	afx_msg LRESULT OnReadyToRun(WPARAM,LPARAM);
	afx_msg LRESULT OnQuickFilterApply(WPARAM,LPARAM);
	afx_msg LRESULT OnCheckForUpdate(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	void WriteAllRangeSet(BOOL fAllowTimeOut = FALSE);
	LRESULT Send_NewsView(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL PrepareForCompact (BOOL fAskUser = TRUE);
	void CompactDatabase();
	void CloseModelessDialogs();
	BOOL CanCloseComposeWindows ();
	BOOL CanCloseComposeWindowType (TPostTemplate *pPostTemplate);
	bool ExistComposeWindows ();
	bool ExistComposeWindowType (TPostTemplate *pPostTemplate);
	void GetCompactList (BOOL fPurgeAll, TPurgeArray & compactList);

	int HandleNewsURL (const CString& url, bool fLoading);
	int ShowGroupWithArticle (const CString& group, int iArtNum);
	int send_goto_message ();
	void SetConnectImage(BOOL fConnected);
	void DoUpdateCheck ();

	void retrieve_tagged (bool fUponConnect);

	void ServerConnectHelper ();
	void CancelReconnect ();

	// disconnect main socket, hangup RAS
	int  disconnect_hangup (BOOL fIdleDisconnect);

	void OnVCRWorker (bool fVCRFile, const CString & filename);

	int  handleClose (bool fExitInstance);

	void calc_download_speed ();

	int  setup_toolbar ();
	void AttachToolbarImages (UINT inNormalImageID, UINT inDisabledImageID);


	CRITICAL_SECTION        m_csVCRDlg;
	HWND                    m_hwVCRDlg;
	TStatusBar              m_wndStatusBar;
	TManualRuleBar          m_sManualRuleBar;
	TOutboxBar              m_sOutboxBar;

	// Toolbar stuff
	CDkToolBar              m_wndToolBar;
	CImageList              m_ToolbarImages;
	CImageList              m_ToolbarImagesDisabled;
	CImageList              m_ToolbarImagesHot;
	CBitmap                 m_bmpTool;
	CBitmap                 m_bmpToolDisabled;

	int                     m_MessagesSaved;

	TViewFilterBar *        m_pViewFilterBar;

	TSearchDialog           m_searchDialog;
	TEventViewerDlg         m_eventViewDlg;
	TDecodeDialog           m_decodeDialog;
	TPrintDialog            m_printDialog;
	//CCheckingForUpdateDlg * m_pCheckingForUpdate;
	CUpdateNotify         * m_pUpdateNotify;
	COutboxDlg *   m_pOutboxDialog;  // ptr, to ease compile dependencies
	TDraftsDlg *   m_pDraftDialog;

	DlgQuickFilter *        m_pQuickFilter;

	TQuickFilterData *      m_pQuickFilterData;

	// class of window
	CString        m_className;

	TSyncTypedPtrArray<CPtrArray, TFetchArticle*> m_fetchArticleArray;

	TGotoArticle*  m_pGotoInfo;

	int            m_iReconnectCount;
	UINT           m_hTimerReconnect;
	int            m_iCountDownReconnect;

	bool           m_bDoNotCompactDBOnShutdown;
};

/////////////////////////////////////////////////////////////////////////////
// TRenumberDlg dialog

class TRenumberDlg : public CDialog
{
public:
	TRenumberDlg(CWnd* pParent,
		const CString& newsgroup,
		int   iServerLow,
		int   iPreviousLow);

	enum { IDD = IDD_RENUMBER_WARNING };

	CString m_strNewsgroup;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	int m_iServerLow;
	int m_iPreviousLow;
};
