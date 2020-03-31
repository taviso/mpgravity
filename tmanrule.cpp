/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tmanrule.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:09  richard_wood
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

// tmanrule.cpp -- dialog to apply a rule manually

#include "stdafx.h"              // windows stuff
#include "pobject.h"             // database API
#include "names.h"               // FORMAT_BODIES, ...
#include "tmanrule.h"            // this file's prototypes
#include "globals.h"             // gpUIMemory
#include "nglist.h"              // TNewsGroupArray
#include "rules.h"               // TestAndApplyRule()
#include "ngutil.h"              // UtilGetStorageOption()
#include "newsview.h"            // CNewsView
#include "genutil.h"             // fnFetchBody(), ...
#include "tasker.h"              // TNewsTasker, IsConnected(), ...
#include "mainfrm.h"             // CMainFrame
#include "thrdlvw.h"             // TThreadListView
#include "thredlst.h"            // TThreadList
#include "mlayout.h"             // TMdiLayout
#include "tnews3md.h"            // TNews3MDIChildWnd
#include "sharesem.h"            // TSyncWriteLock
#include "iterhn.h"              // TArticleIterator
#include "tglobopt.h"            // WarnOnDeleteBinary(), ...
#include "rgwarn.h"              // TRegWarn
#include "warndlg.h"             // WarnWithCBX()
#include "ruleutil.h"            // RuleMayRequireBody()
#include "newsdb.h"              // TRuleIterator
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "nglist.h"              // TNewsGroupUseLock
#include "vfilter.h"             // TViewFilter
#include "autodrw.h"             // TLockDraw

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CString TManualRule::s_strLastRule; // last rule that was invoked

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TGroupCheckListBox, CCheckListBox)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
int TGroupCheckListBox::CompareItem (LPCOMPAREITEMSTRUCT psCompare)
{
	CString str1, str2;
	GetText (psCompare -> itemID1, str1);
	GetText (psCompare -> itemID2, str2);

	if (str1 > str2)
		return 1;
	if (str1 < str2)
		return -1;

	return 0;
}

// -------------------------------------------------------------------------
TManualRule::TManualRule(CWnd* pParent /* = NULL */, LONG lGroupID /* = 0 */,
						 BOOL bSelGroups /* = FALSE */, int iInitialRuleID /* = -1 */,
						 int bRunStraightThrough /* = FALSE */, BOOL bShowDialog /* = TRUE */,
						 BOOL bAllArticles /* = TRUE */, BOOL bRefreshWhenDone /* = TRUE */,
						 CPtrList *pHeaders /* = NULL */)
						 : CDialog(TManualRule::IDD, pParent)
{

	m_strCount = _T("");
	m_strActions = _T("");
	m_strArticle = _T("");
	m_strFirings = _T("");
	m_strGroup = _T("");
	m_iAllArticles = -1;


	m_lInitialGroupID = lGroupID;
	m_bRunStraightThrough = bRunStraightThrough;
	m_iInitialRuleID = iInitialRuleID;
	m_bAllArticles = bAllArticles;
	m_iStartTimer = 0;
	m_iDrawTimer = 0;
	m_bShowDialog = bShowDialog;
	m_bRefreshWhenDone = bRefreshWhenDone;
	m_pHeaders = pHeaders;
	m_iPos = NULL;
	m_bSelGroups = bSelGroups;
	m_pNewsServer = 0;
}

// -------------------------------------------------------------------------
void TManualRule::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_GROUPS, m_sGroups);
	DDX_Control(pDX, IDC_ALL_ARTICLES, m_sAllArticles);
	DDX_Control(pDX, IDCANCEL, m_sCancel);
	DDX_Control(pDX, IDC_STOP, m_sStop);
	DDX_Control(pDX, IDC_START, m_sStart);
	DDX_Control(pDX, IDC_RESET, m_sReset);
	DDX_Control(pDX, IDC_RULE, m_sRule);
	DDX_Control(pDX, IDC_PROGRESS, m_sProgress);
	DDX_Radio(pDX, IDC_ALL_ARTICLES, m_iAllArticles);

	DDX_InfoFields (pDX);
	DDX_Control(pDX, IDC_SELECTED_ARTICLES, m_sSelectedArticles);
}

// -------------------------------------------------------------------------
// DDX_InfoFields -- More efficient than DoDataExchange, calls DDX_Text
//    without the overhead of DDX_Control.
// Called from DoDataExchange, and UpdateInfoFields.
void TManualRule::DDX_InfoFields (CDataExchange *pDX)
{
	DDX_Text(pDX, IDC_INFO_COUNT, m_strCount);
	DDX_Text(pDX, IDC_INFO_ACTIONS, m_strActions);
	DDX_Text(pDX, IDC_INFO_ARTICLE, m_strArticle);
	DDX_Text(pDX, IDC_INFO_FIRINGS, m_strFirings);
	DDX_Text(pDX, IDC_INFO_GROUP, m_strGroup);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TManualRule, CDialog)
		ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_CBN_SELCHANGE(IDC_RULE, OnSelchangeRule)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_ALL_ARTICLES, OnAllArticles)
	ON_BN_CLICKED(IDC_SELECTED_ARTICLES, OnSelectedArticles)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_GROUPS, OnSelchangeGroups)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
static int GetNumSelectedArticlesCallback (TArticleHeader *pHdr,
										   TNewsGroup *pNG, DWORD dwCountPtr)
{
	(* ((int *) dwCountPtr)) ++;
	return 0;
}

// -------------------------------------------------------------------------
static int GetNumSelectedArticles ()
{
	int iCount = 0;

	BOOL fMax;
	TNews3MDIChildWnd *pActiveKid = (TNews3MDIChildWnd *)
		((CMainFrame *) AfxGetMainWnd ()) -> MDIGetActive (&fMax);
	if (!pActiveKid)
		return 0;
	TMdiLayout *pLayout = pActiveKid -> GetLayoutWnd ();
	if (!pLayout)
		return 0;
	TThreadListView * pThreadView = pLayout -> GetThreadView();
	if (!pThreadView)
		return 0;
	pThreadView -> ApplyToSelectedItems (GetNumSelectedArticlesCallback,
		FALSE /* fRepaint */, (DWORD) &iCount);

	return iCount;
}

// -------------------------------------------------------------------------
static int GetSelectedArticlesCallback (TArticleHeader *pHdr, TNewsGroup *pNG,
										DWORD dwArrayPtr)
{
	CPtrArray *rpHeaders = (CPtrArray *) dwArrayPtr;
	rpHeaders -> Add ((void *) pHdr);
	return 0;
}

// -------------------------------------------------------------------------
// GetSelectedArticles -- gets pointers to the headers for the selected
// articles
static void GetSelectedArticles (CPtrArray *pList)
{
	pList -> RemoveAll ();

	BOOL fMax;
	TNews3MDIChildWnd *pActiveKid = (TNews3MDIChildWnd *)
		((CMainFrame *) AfxGetMainWnd ()) -> MDIGetActive (&fMax);
	if (!pActiveKid)
		return;
	TMdiLayout *pLayout = pActiveKid -> GetLayoutWnd ();
	if (!pLayout)
		return;
	TThreadListView * pThreadView = pLayout -> GetThreadView();
	if (!pThreadView)
		return;
	pThreadView -> ApplyToSelectedItems (GetSelectedArticlesCallback,
		FALSE /* fRepaint */, (DWORD) pList);
}

// -------------------------------------------------------------------------
void TManualRule::FillRuleList ()
{
	m_sRule.ResetContent ();

	TRuleIterator it (ReadLock);
	Rule *psRule;
	int iSelectedRule = -1;
	while ((psRule = it.Next ()) != NULL) {
		// add rule to menu
		int iIndex = m_sRule.AddString (psRule -> strRuleName);
		if (iIndex < 0)
			break;

		// if caller said to select this rule initially, do it
		if (m_iInitialRuleID) {
			if (m_iInitialRuleID == psRule -> GetID ()) {
				iSelectedRule = iIndex;
				m_sRule.SetCurSel (iIndex);
			}
		}
		else if (psRule -> strRuleName == s_strLastRule) {
			// if this rule is the last one that was invoked, select it
			iSelectedRule = iIndex;
			m_sRule.SetCurSel (iIndex);
		}
	}

	// if no rule selected yet, select first rule
	if (iSelectedRule < 0 && m_sRule.GetCount ()) {
		iSelectedRule = 0;
		m_sRule.SetCurSel (0);
	}
}

// -------------------------------------------------------------------------
void TManualRule::FillNewsgroupList ()
{
	// first, get the IDs of the selected groups
	CNewsView *psView = GetNewsView ();
	int iNumSelected = psView -> NumSelected ();
	int *piSelectedIDs = new int [iNumSelected];
	psView -> GetSelectedIDs (piSelectedIDs, iNumSelected);

	BOOL bFoundOne = FALSE;
	TNewsGroupArray &rsNewsGroups = m_pNewsServer -> GetSubscribedArray ();
	TNewsGroupArrayReadLock ngMgr (rsNewsGroups);
	int iLen = rsNewsGroups -> GetSize ();
	int iMaxPixels = 1;
	int i = 0;
	for (i = 0; i < iLen; i++) {

		TNewsGroup *psNG = rsNewsGroups [i];

		int iIndex = ListboxAddAdjustHScroll (m_sGroups, psNG -> GetBestname (),
			TRUE /* bCheckListbox */);
		if (iIndex < 0)
			break;
		m_sGroups.SetItemData (iIndex, psNG -> m_GroupID);

		// is this group selected in the group view?
		BOOL bSelected = FALSE;
		for (int j = 0; j < iNumSelected; j++)
			if (piSelectedIDs [j] == psNG -> m_GroupID)
				bSelected = TRUE;

		// if this group's ID was passed to the constructor, select this one
		if (m_lInitialGroupID == psNG -> m_GroupID ||
			(m_bSelGroups && bSelected)) {
				bFoundOne = TRUE;
				m_sGroups.SetCheck (iIndex, TRUE);
		}
	}

	// if no group selected yet, select the first one
	if (!bFoundOne && m_sGroups.GetCount ())
		m_sGroups.SetCheck (0, TRUE);

	// make the first checked item selected, and make sure it's visible
	iLen = m_sGroups.GetCount ();
	for (i = 0; i < iLen; i++)
		if (m_sGroups.GetCheck (i)) {
			m_sGroups.SetCurSel (i);
			m_sGroups.SetTopIndex (i);
			break;
		}

		delete [] piSelectedIDs;
}

// -------------------------------------------------------------------------
void TManualRule::OnSelchangeRule()
{
	OnReset ();
}

// -------------------------------------------------------------------------
static void ConditionalAddRuleFlag (TArticleHeader *psHeader,
									USHORT iOldFlags, USHORT iNewFlags, USHORT &iRemaining, USHORT iFlag,
									TNewsGroup *pNG, TArtRuleInfo *pRuleInfo)
{
	if ((iOldFlags & iFlag) && !(iNewFlags & iFlag))
	{
		TSyncWriteLock sAutoLock(pNG);

		pRuleInfo -> SetRuleFlag (iFlag);
		pNG -> RuleInfoDirty ();
		iRemaining = (USHORT) (iRemaining + iFlag);
	}
}

// -------------------------------------------------------------------------
// DiscardArticle -- discards an article's header (removes it from the
// newsgroup)
static void DiscardArticle (TArticleHeader *psHeader, TNewsGroup *psNG,
							TThreadList *psThreadList)
{
	ASSERT (psHeader);
	int iArtNum = psHeader -> GetArticleNumber ();

	// check the protected-bit
	if (psNG -> IsStatusBitOn (iArtNum, TStatusUnit::kPermanent))
		return;

	TNewsGroup::EStorageOption iMode = UtilGetStorageOption (psNG);
	psNG -> ReadRangeAdd (psHeader);
	psNG -> StatusDestroy (iArtNum);
	psNG -> HTRangeSubtract (iArtNum);
	psNG -> SetDirty ();

	if (psThreadList)
		psThreadList -> RemoveArticle (NULL, psHeader);

	// since even a Mode 1 newsgroup can Tag & Retrieve a body, always
	//   purge both

	psNG -> PurgeHeader (iArtNum);
	psNG -> PurgeBody (iArtNum);
}

// -------------------------------------------------------------------------
// WarnConnection -- returns 0 if OK to proceed, non-0 otherwise
static int WarnConnection (Rule *psRule, CWnd *pWnd, BOOL &bConnected)
{
	extern TNewsTasker *gpTasker;
	bConnected = gpTasker -> IsConnected ();
	if (!bConnected && RuleMayRequireBody (*psRule) &&
		gpGlobalOptions -> WarnOnManualRuleOffline ()) {
			BOOL bTurnOff;
			BOOL RC = WarnWithCBX (IDS_WARN_MANUAL_RULE_OFFLINE, &bTurnOff, pWnd);
			if (bTurnOff) {
				gpGlobalOptions -> SetWarnOnManualRuleOffline (FALSE);
				TRegWarn *psRegWarning = gpGlobalOptions -> GetRegWarn ();
				psRegWarning -> Save ();
			}
			if (!RC)
				return 1;
	}

	return 0;
}

// -------------------------------------------------------------------------
static void PumpAppMessages ()
{
	MSG msg;
	CWinApp* pApp = AfxGetApp();
	while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		// this should route to all modeless dialogs too
		if (!pApp->PreTranslateMessage(&msg))
			::DispatchMessage(&msg);
}

// -------------------------------------------------------------------------
// GetNextArticle -- returns 0 for success, non-0 for failure
static int GetNextArticle (CPtrList *pHeaders, POSITION &iPos,
						   BOOL bAllArticles, POSITION &posIndex, TArticleIndexList &sIndexList,
						   int &iSelectedHeaderPos, CPtrArray &sSelectedHeaders,
						   TArticleHeader *&psHeader)
{
	if (pHeaders) {
		if (!iPos)
			return 1;
		psHeader = (TArticleHeader *) pHeaders -> GetNext (iPos);
		ASSERT (psHeader);
	}
	else if (bAllArticles) {
		if (!posIndex)
			return 1;
		psHeader = sIndexList -> GetNext (posIndex);
	}
	else {
		if (iSelectedHeaderPos > sSelectedHeaders.GetUpperBound ())
			return 1;
		psHeader = (TArticleHeader *) sSelectedHeaders [iSelectedHeaderPos ++];
		if (!psHeader)
			return 1;
	}

	return 0;
}

// -------------------------------------------------------------------------
static void RememberRuleInfo (TArtRuleInfo *&pRuleInfo, USHORT &iOldRuleFlags,
							  UCHAR riOldBodyRules [], UCHAR *&riHeaderBodyRules, TNewsGroup *pNG,
							  LONG lArtNum)
{
	pRuleInfo = pNG -> GetHeaderRuleInfo (lArtNum);
	if (pRuleInfo) {
		// copy and clear the header's rule flags
		iOldRuleFlags = pRuleInfo -> GetRuleFlags ();
		pRuleInfo -> ClearRuleFlags ();

		// copy and clear the header's body-rules
		riHeaderBodyRules = pRuleInfo -> GetBodyRules ();
		for (int i = 0; i < MAX_BODY_RULES; i++)
			riOldBodyRules [i] = riHeaderBodyRules [i];
		pRuleInfo -> ClearBodyRules ();
	}
	else {
		iOldRuleFlags = 0;
		for (int i = 0; i < MAX_BODY_RULES; i++)
			riOldBodyRules [i] = 0;
	}
}

// -------------------------------------------------------------------------
static void RestoreBodyActions (TArticleHeader *pHeader,
								USHORT &iOldRuleFlags, USHORT &iNewRuleFlags, TNewsGroup *pNG,
								TArtRuleInfo *&pRuleInfo, BOOL &bSaveRuleInfo)
{
	// restore the header's body-actions, minus any actions we have done
	USHORT iRemaining = 0;
	ConditionalAddRuleFlag (pHeader, iOldRuleFlags, iNewRuleFlags,
		iRemaining, BODY_ACTION_SAVE_TO_FILE, pNG, pRuleInfo);
	ConditionalAddRuleFlag (pHeader, iOldRuleFlags, iNewRuleFlags,
		iRemaining, BODY_ACTION_FORWARD_TO, pNG, pRuleInfo);
	if (iRemaining != iOldRuleFlags)
		bSaveRuleInfo = TRUE;
}

// -------------------------------------------------------------------------
static void RestoreBodyRules (Rule *pRule,
							  UCHAR riOldBodyRules [],
							  TNewsGroup * pNG,
							  TArtRuleInfo *&pRuleInfo,
							  BOOL &bSaveRuleInfo)
{
	// restore the header's body-rules, minus this rule
	for (int i = 0; i < MAX_BODY_RULES; i++)
	{
		if (!riOldBodyRules [i])
			continue;

		if (riOldBodyRules [i] == pRule -> GetID ())
			bSaveRuleInfo = TRUE;
		else
		{
			TSyncWriteLock sAutoLock(pNG);
			pRuleInfo -> AddBodyRule (riOldBodyRules [i]);
		}
	}
}

// -------------------------------------------------------------------------
static int ApplyRule (Rules *pRules, Rule *pRule, TArticleHeader *pHeader,
					  TNewsGroup *pNG, BOOL &bFiredFlag, int &iActionCount)
{
	pRules -> WriteLock ();
	DWORD dwStart = GetTickCount ();

	// apply rule to header
	int iResult = TestAndApplyRule (pHeader, NULL, pRule, pNG,
		TRUE /* bIgnoreDisabled */, &bFiredFlag, &iActionCount);

	pRules -> m_dwTicks += GetTickCount () - dwStart;
	pRules -> UnlockWrite ();

	return iResult;
}

// -------------------------------------------------------------------------
static void SaveArticle (TArticleHeader *pHeader, TNewsGroup *pNG,
						 LONG lArtNum)
{
	// save the header
	if (!pNG -> GetHeader (lArtNum)) {
		TArticleHeader *pHeaderCopy = new TArticleHeader (*pHeader);
		pNG -> WriteLock ();
		pNG -> AddHeader (lArtNum, pHeaderCopy);
		pNG -> UnlockWrite ();
	}

	// queue the body for fetching & saving
	extern TNewsTasker *gpTasker;
	gpTasker -> RetrieveArticle (pNG -> GetName (), pNG -> m_GroupID, lArtNum,
		pHeader -> GetLines ());
}

// -------------------------------------------------------------------------
static BOOL BodyRuleExists (TArtRuleInfo *pRuleInfo)
{
	if (pRuleInfo)
		for (int i = 0; i < MAX_BODY_RULES; i++)
			if ((pRuleInfo -> GetBodyRules ()) [i])
				return TRUE;
	return FALSE;
}

// -------------------------------------------------------------------------
static void GetNewRuleFlags (USHORT &iNewRuleFlags, BOOL &bSaveRuleInfo,
							 TArtRuleInfo *pRuleInfo, USHORT iOldRuleFlags)
{
	iNewRuleFlags = 0;
	bSaveRuleInfo = FALSE;

	if (pRuleInfo && pRuleInfo -> GetRuleFlags () != iOldRuleFlags) {
		// remember the body-actions that we are about to perform
		iNewRuleFlags = pRuleInfo -> GetRuleFlags ();

		bSaveRuleInfo = TRUE;
	}
}

// -------------------------------------------------------------------------
// DoBodyActions -- returns 1 to break out of loop, 0 otherwise
static int DoBodyActions (USHORT iNewRuleFlags, BOOL bSaveRuleInfo,
						  LONG lArtNum, TArticleHeader *pHeader, TNewsGroup *pNG,
						  TArtRuleInfo *pRuleInfo, Rule *pRule, BOOL &bFiredFlag, BOOL bConnected)
{
	if (!(iNewRuleFlags & BODY_ACTION_SAVE_TO_FILE) &&
		!(iNewRuleFlags & BODY_ACTION_FORWARD_TO) &&
		!BodyRuleExists (pRuleInfo))
		return 0;

	TArticleText * pBody;
	CPoint         ptPartID(0,0);
	TError         sErrorRet;

	if (!fnFetchBody (sErrorRet,
		pNG,
		pHeader,
		pBody,
		ptPartID,
		FALSE /* bMarkAsRead */,
		bConnected /* bTryFromNewsfeed */)) {

			// got the body
			int iResult = EvalRulesHaveBody (pHeader, pBody, pNG, pRule,
				TRUE /* bIgnoreDisabled */, &bFiredFlag, NULL);

			// OK to call EvalRulesHaveBody_Committed() here... it contains time-
			// consuming deferred actions that shouldn't be done when downloading
			// headers
			EvalRulesHaveBody_Committed (pHeader, pBody, pNG);

			// our responsibility to delete the fetched body
			delete pBody;

			if (iResult & RULES_SAYS_DISCARD)
				return 1;

			if (iResult & RULES_SAYS_SAVE) {
				// save the header
				if (!pNG -> GetHeader (lArtNum)) {
					TArticleHeader *pHeaderCopy = new TArticleHeader (*pHeader);
					pNG -> WriteLock ();
					pNG -> AddHeader (lArtNum, pHeaderCopy);
					pNG -> UnlockWrite ();
				}

				// queue the body for fetching & saving
				extern TNewsTasker *gpTasker;
				gpTasker -> RetrieveArticle (pNG -> GetName (), pNG -> m_GroupID,
					lArtNum, pHeader -> GetLines ());
			}

			if (iResult & RULES_SAYS_TAG)
				TagArticle (pNG, pHeader);
	}
	else
		;  // no action... already warned about being disconnected

	return 0;
}

// -------------------------------------------------------------------------
// GetRule -- returns 0 for success, non-0 for failure
static int GetRule (Rules *psRules, CComboBox &sRule, Rule *&psRule,
					CString &strLastRule)
{
	int iIndex = sRule.GetCurSel ();
	if (iIndex == CB_ERR)
		return 1;

	sRule.GetLBText (iIndex, strLastRule);   // remember the last rule used

	if (!psRules -> Lookup (strLastRule, psRule))
		return 1;

	return 0;
}

// -------------------------------------------------------------------------
// GetGroupInfo -- returns 0 for success, non-0 for failure
static void GetGroupInfo (TNewsGroup *pNG, TThreadList *&psThreadList,
						  TArticleIndexList &sIndexList, POSITION &posIndex)
{
	TViewFilter sFilter(0);   // matches all articles
	CWaitCursor wait;
	pNG -> LoadForArticleIndex (&sFilter, FALSE /* fCreateStati */,
		TRUE /* fHitServer */, &sIndexList);
	posIndex = sIndexList -> GetHeadPosition ();
}

// -------------------------------------------------------------------------
//  07-12-99  amc  Changed code such that it did not hold a WriteLock on
//                 the pNG
//
void TManualRule::OnStart()
{
	Rule *psRule = 0;
	BOOL bRulesLocked = FALSE;

	// set state to "running"
	UpdateGreyState (FALSE);
	m_bRunning = TRUE;
	m_bStop = FALSE;

	// give focus to the 'stop' button, just so focus won't get stuck
	m_sStop.SetFocus ();

	// initialize psRules
	Rules *psRules; psRules = GetGlobalRules ();
	ASSERT (psRules);
	psRules -> WriteLock ();
	bRulesLocked = TRUE;

	// these need to be before the first goto
	int iSelectedHeaderPos = 0;
	CPtrArray sSelectedHeaders;
	sSelectedHeaders.SetSize (32, 32);  // is this needed?
	TArticleIndexList sIndexList; // index of articles to go through

	// initialize psRule
	if (GetRule (psRules, m_sRule, psRule, s_strLastRule)) {
		MsgResource (IDS_ERR_RULE_LOOKUP);
		goto end;
	}
	BOOL bRuleWasEnabled; bRuleWasEnabled = psRule -> bEnabled;
	psRule -> bEnabled = TRUE;

	// warn if the rule might need bodies and we are offline
	BOOL bConnected;
	if (WarnConnection (psRule, this, bConnected))
		goto end;

	// initialize pNG
	int iNumGroups; iNumGroups = m_sGroups.GetCount ();
	int iGroup;
	for (iGroup = 0; iGroup < iNumGroups; iGroup++)
	{
		if (!m_sGroups.GetCheck (iGroup))
			continue;

		TNewsGroup *pNG; pNG = 0;
		BOOL fUseLock;
		TNewsGroupUseLock (m_pNewsServer, m_sGroups.GetItemData (iGroup),
			&fUseLock, pNG);
		if (!fUseLock)
		{
			MsgResource (IDS_ERR_GROUP_LOOKUP);
			continue;
		}

		m_strGroup = pNG -> GetBestname ();

		// open the newsgroup
		TAutoClose sAutoCloser (pNG);

		// initialize group's info
		POSITION posIndex;            // current position in sIndexList
		TThreadList *psThreadList; psThreadList = 0;// thread-list to go through
		if (m_bAllArticles)
			GetGroupInfo (pNG, psThreadList, sIndexList, posIndex);
		TNewsGroup::EStorageOption iMode; iMode = UtilGetStorageOption (pNG);
		if (pNG -> m_GroupID == m_lDisplayedGroup)
			m_bDisplayedGroupDirty = TRUE;

		// if only processing selected articles, get the list of selected
		// articles
		if (!m_bAllArticles)
			GetSelectedArticles (&sSelectedHeaders);

		try {
			while (1)
			{
				if (GetNextArticle (m_pHeaders, m_iPos, m_bAllArticles,
					posIndex, sIndexList, iSelectedHeaderPos,
					sSelectedHeaders, m_psHeader))
					break;

				// save old rule info for this article
				TArtRuleInfo *pRuleInfo;
				USHORT iOldRuleFlags;
				UCHAR riOldBodyRules [MAX_BODY_RULES];
				UCHAR *riHeaderBodyRules = 0;
				LONG lArtNum = m_psHeader -> GetArticleNumber ();
				RememberRuleInfo (pRuleInfo, iOldRuleFlags, riOldBodyRules,
					riHeaderBodyRules, pNG, lArtNum);

				int iActionCount = 0;
				BOOL bFiredFlag;
				int iResult = ApplyRule (psRules, psRule, m_psHeader, pNG,
					bFiredFlag, iActionCount);

				if (iResult & RULES_SAYS_DISCARD)
				{
					DiscardArticle (m_psHeader, pNG, psThreadList);
					goto end_of_loop;
				}

				if (iResult & RULES_SAYS_SAVE)
					SaveArticle (m_psHeader, pNG, lArtNum);

				if (iResult & RULES_SAYS_TAG)
					TagArticle (pNG, m_psHeader);

				// may be that there is now a rule-info structure for the header
				if (!pRuleInfo)
					pRuleInfo = pNG -> GetHeaderRuleInfo (lArtNum);

				USHORT iNewRuleFlags;
				BOOL bSaveRuleInfo;     // save header back to DB?
				GetNewRuleFlags (iNewRuleFlags, bSaveRuleInfo, pRuleInfo,
					iOldRuleFlags);

				if (DoBodyActions (iNewRuleFlags, bSaveRuleInfo, lArtNum,
					m_psHeader, pNG, pRuleInfo, psRule, bFiredFlag, bConnected))
					goto end_of_loop;

				RestoreBodyActions (m_psHeader, iOldRuleFlags, iNewRuleFlags,
					pNG, pRuleInfo, bSaveRuleInfo);
				RestoreBodyRules (psRule, riOldBodyRules, pNG, pRuleInfo, bSaveRuleInfo);

				// if we're to save the header back to the database, do it
				if (bSaveRuleInfo &&
					(iMode == TNewsGroup::kHeadersOnly ||
					iMode == TNewsGroup::kStoreBodies))
					pNG -> RuleInfoDirty ();

end_of_loop:

				// update the various counts
				m_iArticle ++;
				m_iActions += iActionCount;
				if (bFiredFlag)
					m_iFirings ++;
				m_sProgress.StepIt ();
				// if stopping update screen info now
				UpdateInfoFields (m_bStop);

				PumpAppMessages ();

				// if the rule has been disabled (e.g., by a user-action during the
				// rule's invocation), stop now
				if (!psRule -> bEnabled)
					m_bStop = TRUE;

				if (m_bStop)
					break;
			}
		}
		catch (TException *pE)
		{
			pE->Delete();
		}
		catch (CException *pE)
		{
			pE->Delete ();
		}

		// if any actions were taken, set the newsgroup dirty just in case
		if (m_iActions)
			pNG -> SetDirty ();

		m_psHeader = NULL;   // can't safely use this past this point

		// must be done before closing the group, because closing the group will
		// free the headers
		sIndexList.Empty ();

	} //for loop

end:

	if (psRule)
		psRule -> bEnabled = bRuleWasEnabled;

	if (bRulesLocked)
		psRules -> UnlockWrite ();

	// reset running state
	m_bRunning = FALSE;
	UpdateGreyState (TRUE);

	// give focus to the 'reset' button, just so focus won't get stuck
	m_sReset.SetFocus ();

	// if stop button wasn't hit, take progress indicator to the end. This
	// is done because sometimes the actual number of headers is less than
	// the number of headers reported by pNG->GetNumArticles()
	if (!m_bStop) {
		m_iArticle = m_iTotalArticles;
		m_sProgress.SetPos (m_iTotalArticles);
	}

	// update info on screen
	m_strGroup.LoadString (IDS_NONE);
	UpdateInfoFields (TRUE);

	// if we were not stopped manually, and we are running straight through,
	// exit the dialog
	if (m_bRunStraightThrough && !m_bStop)
		PostMessage (WM_COMMAND, IDCANCEL);
}

// -------------------------------------------------------------------------
void TManualRule::OnStop()
{
	m_bStop = TRUE;
	m_bRunStraightThrough = 0; // if stop button is hit, don't exit automatically
}

// -------------------------------------------------------------------------
void TManualRule::OnCancel()
{
	if (m_bRunning)
		return;

	// if the displayed group has been dirtied, reload it
	if (m_bDisplayedGroupDirty && m_bRefreshWhenDone) {

		// remember the first visible index in the listbox and restore it after
		// refreshing.  Same with the current selection.
		TThreadListView *pThreadView = GetThreadView ();
		int iIndex;
		int *piSelected = 0;
		int iNumSelected;
		TLockDraw * psDraw = 0;

		if (pThreadView) {
			iIndex = pThreadView -> GetTopIndex ();

			iNumSelected = pThreadView -> GetSelCount ();
			piSelected = new int [iNumSelected];
			if (piSelected)
				pThreadView -> GetSelItems (iNumSelected, piSelected);

			psDraw = new TLockDraw (pThreadView, TRUE);
		}

		// public member function
		DoRefreshNewsgroup ();

		if (pThreadView) {

			if (piSelected) {
				int iLen = pThreadView -> GetCount ();
				for (int i = 0; i < iNumSelected; i++)
					if (i < iLen)
						pThreadView -> SetSel (piSelected [i]);
				delete [] piSelected;
			}

			pThreadView -> SetTopIndex (iIndex);

			delete psDraw;
		}
	}

	ResetAllRuleSubstitution ();

	// Essentially we want to prevent the thread view from painting.
	//   Now that it is back to a good state, we can call the base
	//   class function and dismiss the dialog box. The underlying
	//   area beneath us will naturally redraw.
	CDialog::OnCancel();
}

// -------------------------------------------------------------------------
// public function
void TManualRule::DoRefreshNewsgroup ()
{
	CNewsView *psView = GetNewsView ();
	if (psView)
	{
		psView -> RefreshCurrentNewsgroup ( false );

		// redraw counts in newsgroup pane
		psView -> Invalidate ();
	}
}

// -------------------------------------------------------------------------
void TManualRule::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == m_iStartTimer) {
		// kill the timer and start the run
		KillTimer (m_iStartTimer);
		m_iStartTimer = 0;
		PostMessage (WM_COMMAND, IDC_START);
	}
	else if (nIDEvent == m_iDrawTimer) {
		// update fields onscreen
		CDataExchange sDX (this, FALSE);
		DDX_InfoFields (&sDX);
	}

	CDialog::OnTimer (nIDEvent);
}

// -------------------------------------------------------------------------
void TManualRule::OnDestroy()
{
	CDialog::OnDestroy();

	// delete the screen update timer
	if (m_iDrawTimer) {
		KillTimer (m_iDrawTimer);
		m_iDrawTimer = 0;
	}

	// unlock the news server object. We locked in OnInitDialog
	m_pNewsServer->Release ();
	m_pNewsServer = 0;
}

// -------------------------------------------------------------------------
void TManualRule::OnAllArticles()
{
	m_bAllArticles = TRUE;
	OnReset ();
}

// -------------------------------------------------------------------------
void TManualRule::OnSelectedArticles()
{
	m_bAllArticles = FALSE;
	OnReset ();
}

// -------------------------------------------------------------------------
BOOL TManualRule::OnInitDialog()
{
	CDialog::OnInitDialog();

	// get server ptr, and refcount it.
	m_pNewsServer = GetCountedActiveServer ();
	m_pNewsServer -> AddRef ();

	// NOTE: no need to lock the global newsgroup-array since this dialog is
	// modal, so a group can't be unsubscribed while we're working with it

	// give the window a new title
	CString str; str.LoadString (IDS_MANUAL_RULE_TITLE);
	SetWindowText (str);

	// keep track of whether we've dirtied the currently-displayed group
	CNewsView *psView = GetNewsView ();
	if (psView && psView -> IsNewsgroupDisplayed ())
		m_lDisplayedGroup = psView -> GetCurNewsGroupID ();
	else
		m_lDisplayedGroup = 0;
	m_bDisplayedGroupDirty = FALSE;

	// fill the rule-list and group-list
	FillRuleList ();
	FillNewsgroupList ();
	GrayGroupControls ();

	// give initial values to our counts
	OnReset ();

	UpdateGreyState (TRUE);

	// a timer is used to update the info fields
	m_iDrawTimer = 2;
	m_iDrawTimer = SetTimer (m_iDrawTimer, 300 /* msec */, NULL);

	if (m_bRunStraightThrough && m_bShowDialog) {
		m_iStartTimer = 1;
		m_iStartTimer = SetTimer (m_iStartTimer, 500 /* half second */,
			NULL /* lpTimerFunc */);
	}
	else if (m_bRunStraightThrough)
		PostMessage (WM_COMMAND, IDC_START);

	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
// GetTotalArticles -- gets the total # of articles for all checked groups
static int GetTotalArticles (TNewsServer* pNewsServer, CCheckListBox &sGroups)
{
	int iTotal = 0;

	int iNum = sGroups.GetCount ();
	for (int i = 0; i < iNum; i++)
		if (sGroups.GetCheck (i)) {
			BOOL fUseLock;
			TNewsGroup *pNG;
			TNewsGroupUseLock (pNewsServer, sGroups.GetItemData (i), &fUseLock,
				pNG);
			if (fUseLock)
				iTotal += pNG -> GetNumArticles ();
		}

		return iTotal;
}

// -------------------------------------------------------------------------
void TManualRule::OnReset()
{
	m_bRunning = FALSE;
	m_iActions = m_iFirings = m_iArticle = m_iTotalArticles = 0;
	m_psHeader = NULL;
	if (m_pHeaders)
		m_iPos = m_pHeaders -> GetHeadPosition ();

	// update m_iTotalArticles
	m_iTotalArticles = !m_bAllArticles ? GetNumSelectedArticles () :
		GetTotalArticles (m_pNewsServer, m_sGroups);

	// update the range for the progress-bar
	m_sProgress.SetRange (0, m_iTotalArticles);
	m_sProgress.SetStep (1);
	m_sProgress.SetPos (0);

	// start with 'none' as the group name
	m_strGroup.LoadString (IDS_NONE);

	// show initial values for the info fields
	UpdateInfoFields (TRUE);
	UpdateGreyState (TRUE);
	ResetAllRuleSubstitution ();
}

// -------------------------------------------------------------------------
void TManualRule::UpdateInfoFields (BOOL fImmediate /* = FALSE */)
{
	m_strCount.Format (_T("%d / %d"), m_iArticle, m_iTotalArticles);
	m_strActions.Format (_T("%d"), m_iActions);
	m_strFirings.Format (_T("%d"), m_iFirings);
	if (m_psHeader)
		m_strArticle = m_psHeader -> GetSubject ();
	else
		m_strArticle.LoadString (IDS_NONE);

	// if not immediate, screen data is updated by a timer
	if (fImmediate) {
		CDataExchange sDX (this, FALSE);
		DDX_InfoFields (&sDX);
	}
}

// -------------------------------------------------------------------------
void TManualRule::UpdateGreyState (BOOL bStopGrey)
{
	// if the current article count is zero, enable the start button, otherwise
	// it's gray.  This forces the user to hit reset after stopping midway
	BOOL bStartEnabled = bStopGrey && !m_iArticle;

	// if focus is on a greyed button, move it to an enabled button
	m_sStop.EnableWindow (1);
	m_sStart.EnableWindow (1);
	m_sReset.EnableWindow (1);
	HWND hFocus = ::GetFocus ();
	if ((hFocus == m_sStop.m_hWnd && bStopGrey) ||
		(hFocus == m_sStart.m_hWnd && !bStartEnabled))
		if (bStartEnabled)
			m_sStart.SetFocus ();
		else if (!bStopGrey)
			m_sStop.SetFocus ();
		else
			m_sReset.SetFocus ();

		m_sStop.EnableWindow (!bStopGrey);
		m_sCancel.EnableWindow (bStopGrey);
		m_sStart.EnableWindow (bStartEnabled);
		m_sReset.EnableWindow (bStopGrey);
		m_sRule.EnableWindow (bStopGrey);
		m_sGroups.EnableWindow (bStopGrey);

		// group controls... if stop is not gray, gray them
		GrayGroupControls ();
		if (!bStopGrey) {
			m_sAllArticles.EnableWindow (0);
			m_sSelectedArticles.EnableWindow (0);
		}
}

// -------------------------------------------------------------------------
void TManualRule::GrayGroupControls ()
{
	int iNumChecked = 0;
	BOOL bCurrentChecked = FALSE;

	int iCount = m_sGroups.GetCount ();
	for (int i = 0; i < iCount; i++) {
		if (m_sGroups.GetCheck (i)) {
			iNumChecked ++;
			if (((LONG) m_sGroups.GetItemData (i)) == m_lDisplayedGroup)
				bCurrentChecked = TRUE;
		}
	}

	BOOL bEnabled = (iNumChecked == 1 && bCurrentChecked);

	// update the radio buttons' selection state.  If buttons are gray, select
	// the "all articles" option (so the logic in OnStart() doesn't get
	// confused)
	if (!bEnabled)
		m_bAllArticles = TRUE;
	m_iAllArticles = (m_bAllArticles ? 0 : 1);
	UpdateData (FALSE);

	m_sAllArticles.EnableWindow (bEnabled);
	m_sSelectedArticles.EnableWindow (bEnabled);
}

// -------------------------------------------------------------------------
void TManualRule::OnSelchangeGroups()
{
	GrayGroupControls ();
	OnReset ();
}
