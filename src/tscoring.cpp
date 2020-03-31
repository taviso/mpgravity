/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tscoring.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:18  richard_wood
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

// tscoring.cpp -- scoring dialog

#include "stdafx.h"              // precompiled header
#include "tscoring.h"            // this file's prototypes
#include "genutil.h"             // MsgResource(), ...
#include "newsdb.h"              // TNewsDB
#include "globals.h"             // gpStore
#include "server.h"              // TNewsServer
#include "genutil.h"             // GetNewsView(), ...
#include "newsview.h"            // CNewsView
#include "thredlst.h"            // TThreadList
#include "idxlst.h"              // TArticleIndexList
#include "rgswit.h"              // TRegSwitch
#include "rgui.h"                // TRegUI
#include "tglobopt.h"            // gpGlobalOptions
#include "thread.h"              // TThread
#include "thredpl.h"             // TThreadPile

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long TScoreEdit::s_lLastScore;
const CString gstrSpaces = "      ";
ScoreColor grScoreColors [5];
#define DEFAULT_EXPIRATION_DAYS 7
#define NO_EXPIRATION_STRING "-"

// -------------------------------------------------------------------------
// forward declarations
static int ApplyScoreSet (
						  ScoreSet &        sScoreSet,
						  TNewsGroup *      pNG,
						  TArticleHeader *  pHdr,
						  TArticleText *    pBody
						  );

static int TestAndApplyScore (
							  Score &           sScore,
							  TNewsGroup *      pNG,
							  TArticleHeader *  pHdr,
							  TArticleText *    pBody
							  );

// -------------------------------------------------------------------------
Score::Score () : PObject (SCORE_VERSION)
{
	lScore = 0;
	bWholeWord = FALSE;
	iType = SCORE_TYPE_TEXT;
	iWhere = SCORE_WHERE_ALL;
	bExpires = FALSE;
	iExpirationDays = DEFAULT_EXPIRATION_DAYS;
}

// -------------------------------------------------------------------------
void Score::Serialize (CArchive& sArchive)
{
	PObject::Serialize (sArchive);

	if (sArchive.IsStoring ()) {
		TExpirable::Serialize (sArchive);
		sArchive << strWord;
		sArchive << lScore;
		sArchive << bWholeWord;
		sArchive << iType;
		sArchive << iWhere;
		sArchive << bExpires;
		sArchive << iExpirationDays;
	}
	else {

		if (GetObjectVersion () > 2)
			TExpirable::Serialize (sArchive);

		sArchive >> strWord;
		sArchive >> lScore;

		if (GetObjectVersion () > 1) {
			sArchive >> bWholeWord;
			sArchive >> iType;
			sArchive >> iWhere;

			// in version 4, iWhere value of 2 becomes iWhere value of 3
			if (GetObjectVersion () <= 3 && iWhere == 2)
				iWhere = 3;
		}

		if (GetObjectVersion () > 2) {
			sArchive >> bExpires;
			sArchive >> iExpirationDays;
		}
		else
			// scores converted from old format do not expires
			bExpires = FALSE;
	}
}

// -------------------------------------------------------------------------
Score::Score (const Score &src)
{
	*this = src;
}

// -------------------------------------------------------------------------
Score &Score::operator= (const Score &src)
{
	TExpirable::operator= (src);
	strWord = src.strWord;
	lScore = src.lScore;
	bWholeWord = src.bWholeWord;
	iType = src.iType;
	iWhere = src.iWhere;
	bExpires = src.bExpires;
	iExpirationDays = src.iExpirationDays;
	return *this;
}

// -------------------------------------------------------------------------
ScoreSet::ScoreSet (const ScoreSet &src)
{
	*this = src;
}

// -------------------------------------------------------------------------
ScoreSet &ScoreSet::operator= (const ScoreSet &src)
{
	strGroup = src.strGroup;

	// copy scores
	RemoveAll ();
	POSITION pos = src.GetHeadPosition ();
	while (pos) {
		Score sScore = src.GetNext (pos);
		AddTail (sScore);
	}

	return *this;
}

// -------------------------------------------------------------------------
void ScoreSet::Serialize (CArchive& sArchive)
{
	if (sArchive.IsStoring ()) {
		sArchive << (int) 1; // version number
		sArchive << strGroup;

		sArchive << (int) GetCount ();
		POSITION pos = GetHeadPosition ();
		while (pos) {
			Score &sScore = GetNext (pos);
			sScore.Serialize (sArchive);
		}
	}
	else {
		int iVersion;
		sArchive >> iVersion;
		ASSERT (iVersion == 1);
		sArchive >> strGroup;

		int iCount;
		sArchive >> iCount;
		for (int i = 0; i < iCount; i++) {
			Score sScore;
			sScore.Serialize (sArchive);
			AddTail (sScore);
		}
	}
}

// -------------------------------------------------------------------------
ScoreSets &ScoreSets::operator= (const ScoreSets &src)
{
	// copy scores sets
	RemoveAll ();
	POSITION pos = src.GetHeadPosition ();
	while (pos) {
		ScoreSet sScoreSet;
		sScoreSet = src.GetNext (pos);
		AddTail (sScoreSet);
	}

	return *this;
}

// -------------------------------------------------------------------------
void ScoreSets::Serialize (CArchive& sArchive)
{
	if (sArchive.IsStoring ()) {
		sArchive << (int) 1; // version number

		sArchive << (int) GetCount ();
		POSITION pos = GetHeadPosition ();
		while (pos) {
			ScoreSet &sScoreSet = GetNext (pos);
			sScoreSet.Serialize (sArchive);
		}
	}
	else {
		int iVersion;
		sArchive >> iVersion;
		ASSERT (iVersion == 1);

		int iCount;
		sArchive >> iCount;
		for (int i = 0; i < iCount; i++) {
			ScoreSet sScoreSet;
			sScoreSet.Serialize (sArchive);
			AddTail (sScoreSet);
		}
	}
}

// -------------------------------------------------------------------------
TScoringDlg::TScoringDlg(CWnd* pParent /*=NULL*/)
: CDialog(TScoringDlg::IDD, pParent)
{



	m_bDirty = FALSE;
}

// -------------------------------------------------------------------------
void TScoringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Control(pDX, IDCANCEL, m_sCancel);
	DDX_Control(pDX, IDC_ADD_WORD, m_sAddWord);
	DDX_Control(pDX, IDC_ADD_GROUP, m_sAddGroup);
	DDX_Control(pDX, IDC_WORDS, m_sWords);
	DDX_Control(pDX, IDC_EDIT, m_sEdit);
	DDX_Control(pDX, IDC_DELETE, m_sDelete);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TScoringDlg, CDialog)
		ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_WORDS, OnItemchangedWords)
	ON_NOTIFY(NM_DBLCLK, IDC_WORDS, OnDblclkWords)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_WORDS, OnEndlabeleditWords)
	ON_BN_CLICKED(IDC_ADD_GROUP, OnAddGroup)
	ON_BN_CLICKED(IDC_ADD_WORD, OnAddWord)
	ON_NOTIFY(LVN_KEYDOWN, IDC_WORDS, OnKeydownWords)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TScoringDlg::OnOK()
{
	if (IsDirty ()) {
		ScoreSets *pScoreSets = gpStore -> GetScoreSets ();
		pScoreSets -> WriteLock ();
		*pScoreSets = m_sScoreSets;
		pScoreSets -> UnlockWrite ();

		// save new rules to theDatabase
		gpStore -> SaveScoreSets ();
	}

	CDialog::OnOK();
}

// -------------------------------------------------------------------------
static const CString &WholeWordText (BOOL bWholeWord)
{
	static CString strYes;
	static CString strNo;
	if (strYes.IsEmpty ()) {
		strYes.LoadString (IDS_YES);
		strNo.LoadString (IDS_NO);
	}
	return bWholeWord ? strYes : strNo;
}

// -------------------------------------------------------------------------
static const CString &TypeText (int iType)
{
	static CString strText;
	static CString strWildcard;
	static CString strRE;
	if (strText.IsEmpty ()) {
		strText.LoadString (IDS_TEXT);
		strWildcard.LoadString (IDS_WILDCARD);
		strRE.LoadString (IDS_REG_EXPR);
	}
	return iType == SCORE_TYPE_TEXT ? strText :
		(iType == SCORE_TYPE_WILDMAT ? strWildcard : strRE);
}

// -------------------------------------------------------------------------
static const CString &WhereText (int iWhere)
{
	static CString strSubject;
	static CString strFrom;
	static CString strAll;
	static CString strBody;
	if (strSubject.IsEmpty ()) {
		strSubject.LoadString (IDS_SUBJECT);
		strFrom.LoadString (IDS_FROM);
		strAll.LoadString (IDS_ALL);
		strBody.LoadString (IDS_BODY);
	}
	switch (iWhere) {
	  case SCORE_WHERE_SUBJECT:
		  return strSubject;
	  case SCORE_WHERE_FROM:
		  return strFrom;
	  case SCORE_WHERE_BODY:
		  return strBody;
	}
	return strAll;
}

// -------------------------------------------------------------------------
static const CString &ExpiresText (BOOL bExpires, int iExpirationDays,
								   const CTime &sLastSeen)
{
	static CString strText;
	if (!bExpires)
		strText = NO_EXPIRATION_STRING;
	else {
		CString strTemp = DaysTillExpiration (sLastSeen, iExpirationDays);
		strText.Format (  _T("%d (%s left)"), iExpirationDays, strTemp);
	}
	return strText;
}

// -------------------------------------------------------------------------
BOOL TScoringDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// initialize list control
	CString strHeader;
	strHeader.LoadString (IDS_PHRASE);
	m_sWords.InsertColumn (0 /* column */, strHeader, LVCFMT_LEFT,
		0 /* width, temporary */, 0 /* subitem */);
	strHeader.LoadString (IDS_SCORE);
	m_sWords.InsertColumn (1 /* column */, strHeader, LVCFMT_RIGHT,
		0 /* width, temporary */, 1 /* subitem */);
	strHeader.LoadString (IDS_WHOLE_WORD);
	m_sWords.InsertColumn (2 /* column */, strHeader, LVCFMT_LEFT,
		0 /* width, temporary */, 2 /* subitem */);
	strHeader.LoadString (IDS_TYPE);
	m_sWords.InsertColumn (3 /* column */, strHeader, LVCFMT_LEFT,
		0 /* width, temporary */, 3 /* subitem */);
	strHeader.LoadString (IDS_LOCATION);
	m_sWords.InsertColumn (4 /* column */, strHeader, LVCFMT_LEFT,
		0 /* width, temporary */, 4 /* subitem */);
	strHeader.LoadString (IDS_EXPIRATION_DAYS);
	m_sWords.InsertColumn (5 /* column */, strHeader, LVCFMT_LEFT,
		0 /* width, temporary */, 5 /* subitem */);

	// set window size & pos
	CString strSizePos = gpGlobalOptions -> GetRegUI () -> GetScoringSizePos ();
	int dx, dy, x, y;
	if (!DecomposeSizePos (strSizePos, dx, dy, x, y))
		SetWindowPos (NULL, x, y, dx, dy, SWP_NOZORDER);
	SizeControls ();

	// get name of currently-open group
	CString strCurrentGroup;
	CNewsView *pView = GetNewsView ();
	LONG lGroup = pView -> GetCurNewsGroupID ();
	TNewsGroup *pNG = 0;
	BOOL fUseLock;
	TServerCountedPtr cpNewsServer;
	TNewsGroupUseLock (cpNewsServer, lGroup, &fUseLock, pNG);
	if (fUseLock)
		strCurrentGroup = pNG -> GetName ();

	// write the score data into the dialog
	ScoreSets *pScoreSets = gpStore -> GetScoreSets ();
	pScoreSets -> ReadLock ();
	m_sScoreSets = *pScoreSets;
	pScoreSets -> UnlockRead ();
	POSITION pos = m_sScoreSets.GetHeadPosition ();
	int iSets = 0;
	while (pos) {
		POSITION prevPos = pos;
		ScoreSet &sScoreSet = m_sScoreSets.GetNext (pos);

		int iIndex = m_sWords.GetItemCount ();
		m_sWords.InsertItem (iIndex, sScoreSet.strGroup);
		// list-control item's data is the score-set index within m_sScoreSets
		m_sWords.SetItemData (iIndex, (DWORD) prevPos);

		// in the list control, put focus on the currently-open group
		if (sScoreSet.strGroup == strCurrentGroup)
			m_sWords.SetItemState (iIndex, LVIS_SELECTED | LVIS_FOCUSED,
			LVIS_SELECTED | LVIS_FOCUSED);

		// write this set's scores into the dialog
		POSITION pos2 = sScoreSet.GetHeadPosition ();
		while (pos2) {
			Score &sScore = sScoreSet.GetNext (pos2);
			iIndex ++;
			CString strWord = gstrSpaces + sScore.strWord;
			m_sWords.InsertItem (iIndex, strWord);
			// list-control item's data is -1 to mark the item as a word/score pair
			m_sWords.SetItemData (iIndex, (DWORD) -1);
			CString strScore; strScore.Format (_T("%ld"), sScore.lScore);
			m_sWords.SetItemText (iIndex, 1 /* column */, strScore);
			m_sWords.SetItemText (iIndex, 2 /* column */,
				WholeWordText (sScore.bWholeWord));
			m_sWords.SetItemText (iIndex, 3 /* column */,
				TypeText (sScore.iType));
			m_sWords.SetItemText (iIndex, 4 /* column */,
				WhereText (sScore.iWhere));
			m_sWords.SetItemText (iIndex, 5 /* column */,
				ExpiresText (sScore.bExpires, sScore.iExpirationDays,
				sScore.LastSeen ()));
		}
	}

	// find the selected item, if any, and scroll it into view
	int iSize = m_sWords.GetItemCount ();
	for (int i = 0; i < iSize; i++)
		if (m_sWords.GetItemState (i, LVIS_SELECTED)) {
			m_sWords.EnsureVisible (i, FALSE /* bPartialOK */);
			break;
		}

		GrayControls ();
		return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TScoringDlg::OnDestroy()
{
	// save window size & pos
	CRect sRect;
	GetWindowRect (&sRect);
	CString strSizePos = ComposeSizePos ((int) (sRect.right - sRect.left),
		(int) (sRect.bottom - sRect.top), sRect.left, sRect.top);
	gpGlobalOptions -> GetRegUI () -> SetScoringSizePos (strSizePos);

	CDialog::OnDestroy();
}

// -------------------------------------------------------------------------
void TScoringDlg::SizeControls ()
{
	CRect sRect;
	GetClientRect (&sRect);
	int iRight = sRect.Width ();
	int iBottom = sRect.Height ();
	m_sOK.GetClientRect (&sRect);
	int iButtonWidth = sRect.Width ();
	int iButtonHeight = sRect.Height ();
#define BORDER 10
#define BUTTON_OFFSET 28

	// buttons are aligned with right side
	int iX = iRight - BORDER - iButtonWidth;
	int iY = BORDER;
	m_sAddGroup.SetWindowPos (0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	iY += BUTTON_OFFSET;
	m_sAddWord.SetWindowPos (0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	iY += BUTTON_OFFSET;
	m_sDelete.SetWindowPos (0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	iY += BUTTON_OFFSET;
	m_sEdit.SetWindowPos (0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// OK and cancel are at bottom
	iY = iBottom - BORDER - iButtonHeight;
	m_sCancel.SetWindowPos (0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	iY -= BUTTON_OFFSET;
	m_sOK.SetWindowPos (0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// score list-control is expanded
	int iWordsWidth = iRight - BORDER * 3 - iButtonWidth;
	m_sWords.SetWindowPos (0, BORDER, BORDER, iWordsWidth, iBottom - BORDER * 2,
		SWP_NOZORDER);

	// score list-control's columns
#define SCORE_COLUMN_WIDTH    50
#define WHOLE_COLUMN_WIDTH    80
#define TYPE_COLUMN_WIDTH     65
#define LOCATION_COLUMN_WIDTH 60
#define EXPIRES_COLUMN_WIDTH  85
#define OTHER_COLUMN_WIDTHS (SCORE_COLUMN_WIDTH + WHOLE_COLUMN_WIDTH + \
	TYPE_COLUMN_WIDTH + LOCATION_COLUMN_WIDTH + EXPIRES_COLUMN_WIDTH)
	m_sWords.SetColumnWidth (0,
		iWordsWidth - OTHER_COLUMN_WIDTHS - 4 - GetSystemMetrics (SM_CXVSCROLL));
	m_sWords.SetColumnWidth (1, SCORE_COLUMN_WIDTH);
	m_sWords.SetColumnWidth (2, WHOLE_COLUMN_WIDTH);
	m_sWords.SetColumnWidth (3, TYPE_COLUMN_WIDTH);
	m_sWords.SetColumnWidth (4, LOCATION_COLUMN_WIDTH);
	m_sWords.SetColumnWidth (5, EXPIRES_COLUMN_WIDTH);
}

// -------------------------------------------------------------------------
void TScoringDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// could be that we're not initialized yet
	if (m_sOK.m_hWnd)
		SizeControls ();
}

// -------------------------------------------------------------------------
void TScoringDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	// don't let window get smaller than a certain size
	lpMMI->ptMinTrackSize.x = 620;
	lpMMI->ptMinTrackSize.y = 300;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// -------------------------------------------------------------------------
BOOL TScoringDlg::EditOK ()
{
	// edit button is enabled if one item is selected, or if more than one are
	// selected but they're all phrase/score pairs
	int iSelected = m_sWords.GetSelectedCount ();
	if (iSelected < 1)
		return FALSE;
	if (iSelected == 1)
		return TRUE;
	if (iSelected > 1) {
		int iCount = m_sWords.GetItemCount ();
		for (int i = 0; i < iCount; i++)
			if (m_sWords.GetItemState (i, LVIS_SELECTED)) {
				DWORD dwData = m_sWords.GetItemData (i);
				if (dwData != (DWORD) -1)
					return FALSE;
			}
	}
	return TRUE;
}

// -------------------------------------------------------------------------
void TScoringDlg::GrayControls ()
{
	m_sDelete.EnableWindow (m_sWords.GetSelectedCount ());
	m_sEdit.EnableWindow (EditOK ());
}

// -------------------------------------------------------------------------
void TScoringDlg::OnDelete()
{
	// work backwards so we don't need to worry about adjusting indices
	int iCount = m_sWords.GetItemCount ();
	int i;
	for (i = iCount - 1; i >= 0; i--)
		if (m_sWords.GetItemState (i, LVIS_SELECTED)) {

			// find the score set that this item corresponds to
			int iScoreSetIndex;
			ScoreSet *psScoreSet;
			GetScoreSetAndIndex (i, iScoreSetIndex, psScoreSet);
			iScoreSetIndex --;

			DWORD dwData = m_sWords.GetItemData (i);
			if (dwData == -1) {
				// phrase/score pair
				POSITION pos = psScoreSet -> GetHeadPosition ();
				for (int j = 0; j < iScoreSetIndex; j++)
					psScoreSet -> GetNext (pos);
				psScoreSet -> RemoveAt (pos);
				m_sWords.DeleteItem (i);
			}
			else {

				// free this group
				psScoreSet -> RemoveAll ();

				// remove this score-set from the score-sets
				POSITION pos = m_sScoreSets.GetHeadPosition ();
				while (pos) {
					POSITION prevPos = pos;
					ScoreSet &sScoreSet = m_sScoreSets.GetNext (pos);
					ScoreSet *psCurrentScoreSet = &sScoreSet;
					if (psScoreSet == psCurrentScoreSet) {
						m_sScoreSets.RemoveAt (prevPos);
						break;
					}
				}

				do {
					// keep removing items from the list control until the next
					// group is found
					m_sWords.DeleteItem (i);
					if (i >= m_sWords.GetItemCount ())
						break;
				} while (m_sWords.GetItemData (i) == (DWORD) -1);
			}
		}

		// select the focused item
		iCount = m_sWords.GetItemCount ();
		for (i = 0; i < iCount; i++)
			if (m_sWords.GetItemState (i, LVIS_FOCUSED))
				m_sWords.SetItemState (i, LVIS_SELECTED, LVIS_SELECTED);

		GotoDlgCtrl (&m_sWords);
		SetDirty ();
		GrayControls ();
}

// -------------------------------------------------------------------------
// PhraseExists -- tells whether a phrase already exists in a score set
static BOOL PhraseExists (ScoreSet *psScoreSet, const CString &strPhrase,
						  int iIndexToIgnore = -1)
{
	POSITION pos = psScoreSet -> GetHeadPosition ();
	int iCount = 0;
	while (pos) {
		const Score &sScore = psScoreSet -> GetNext (pos);
		if (iIndexToIgnore != iCount &&
			!strPhrase.CompareNoCase (sScore.strWord))
			return TRUE;
		iCount ++;
	}
	return FALSE;
}

// -------------------------------------------------------------------------
void TScoringDlg::OnEdit()
{
	if (!EditOK ())
		return;

	// either one group item is selected, or one or more phrase/score items are
	// selected

	BOOL bGroup = TRUE;
	int iCount = m_sWords.GetItemCount ();
	int i;
	for (i = 0; i < iCount; i++)
		if (m_sWords.GetItemState (i, LVIS_SELECTED)) {
			DWORD dwData = m_sWords.GetItemData (i);
			if (dwData == (DWORD) -1)
				bGroup = FALSE;
			break;
		}

		if (bGroup) {
			// a group is selected
			TScoreEditGroup sDlg;
			sDlg.m_strGroup = m_sWords.GetItemText (i, 0 /* column */);
			if (sDlg.DoModal () != IDOK)
				return;
			ScoreSet &sScoreSet =
				m_sScoreSets.GetAt ((POSITION) m_sWords.GetItemData (i));
			sScoreSet.strGroup = sDlg.m_strGroup;
			m_sWords.SetItemText (i, 0 /* column */, sDlg.m_strGroup);
		}
		else {
			// one or more phrase/score pairs are selected

			// initialize edit-dialog's phrase and score
			long lScore = 0;
			CString strPhrase;
			BOOL bScoreInitialized = FALSE;
			BOOL bMultiple = FALSE;
			BOOL bWholeWord = FALSE;
			int iType = SCORE_TYPE_TEXT;
			int iWhere = SCORE_WHERE_ALL;
			BOOL bExpires = FALSE;
			int iExpirationDays = DEFAULT_EXPIRATION_DAYS;
			for (i = 0; i < iCount; i++)
				if (m_sWords.GetItemState (i, LVIS_SELECTED)) {

					if (bScoreInitialized)
						bMultiple = TRUE;
					bScoreInitialized = TRUE;

					// get value for this row
					CString str, strTemp;
					long lThisScore;
					BOOL bThisWholeWord;
					int iThisType, iThisWhere;
					BOOL bThisExpires;
					int iThisExpirationDays;

					// score
					str = m_sWords.GetItemText (i, 1);
					lThisScore = _ttol (str);

					// whole word
					str = m_sWords.GetItemText (i, 2);
					strTemp.LoadString (IDS_YES);
					bThisWholeWord = str == strTemp;

					// type
					str = m_sWords.GetItemText (i, 3);
					strTemp.LoadString (IDS_TEXT);
					if (str == strTemp)
						iThisType = SCORE_TYPE_TEXT;
					else {
						strTemp.LoadString (IDS_WILDCARD);
						iThisType = (str == strTemp) ?
SCORE_TYPE_WILDMAT : SCORE_TYPE_RE;
					}

					// where
					str = m_sWords.GetItemText (i, 4);
					strTemp.LoadString (IDS_SUBJECT);
					if (str == strTemp)
						iThisWhere = SCORE_WHERE_SUBJECT;
					else {
						strTemp.LoadString (IDS_FROM);
						if (str == strTemp)
							iThisWhere = SCORE_WHERE_FROM;
						else {
							strTemp.LoadString (IDS_BODY);
							if (str == strTemp)
								iThisWhere = SCORE_WHERE_BODY;
							else
								iThisWhere = SCORE_WHERE_ALL;
						}
					}

					// expiration
					str = m_sWords.GetItemText (i, 5);
					if (str == NO_EXPIRATION_STRING) {
						bThisExpires = FALSE;
						iThisExpirationDays = DEFAULT_EXPIRATION_DAYS;
					}
					else {
						bThisExpires = TRUE;
						iThisExpirationDays = _ttoi (str);
					}

					if (bMultiple) {
						strPhrase = "";
						if (lThisScore != lScore)
							lScore = 0;
						if (bThisWholeWord != bWholeWord)
							bWholeWord = FALSE;
						if (iThisType != iType)
							iType = SCORE_TYPE_TEXT;
						if (iThisWhere != iWhere)
							iWhere = SCORE_WHERE_ALL;
						if (bThisExpires != bExpires)
							bExpires = FALSE;
						if (iThisExpirationDays != iExpirationDays)
							iExpirationDays = DEFAULT_EXPIRATION_DAYS;
					}
					else {
						// not multiple
						strPhrase = m_sWords.GetItemText (i, 0);
						strPhrase.TrimLeft ();
						lScore = lThisScore;
						bWholeWord = bThisWholeWord;
						iType = iThisType;
						iWhere = iThisWhere;
						bExpires = bThisExpires;
						iExpirationDays = iThisExpirationDays;
					}
				}

				TScoreEdit sDlg;
				sDlg.m_strPhrase = strPhrase;
				sDlg.m_lScore = lScore;
				sDlg.m_bMultiple = bMultiple;
				sDlg.m_bWholeWord = bWholeWord;
				sDlg.m_iType = iType;
				sDlg.m_iWhere = iWhere;
				sDlg.m_iExpiration = bExpires ? 1 : 0;
				sDlg.m_iExpirationDays = iExpirationDays;
				if (sDlg.DoModal () != IDOK)
					return;

				// make sure phrase is non-null
				if (sDlg.m_strPhrase.IsEmpty () && !bMultiple) {
					MsgResource (IDS_ERR_EMPTY_PHRASE);
					return;
				}

				CString strScore;
				strScore.Format (_T("%d"), sDlg.m_lScore);
				for (i = 0; i < iCount; i++)
					if (m_sWords.GetItemState (i, LVIS_SELECTED)) {

						// find the score set that this item corresponds to
						int iScoreSetIndex;
						ScoreSet *psScoreSet;
						GetScoreSetAndIndex (i, iScoreSetIndex, psScoreSet);
						iScoreSetIndex --;

						if (bMultiple) {

							// set list control data
							m_sWords.SetItemText (i, 1 /* column */, strScore);
							m_sWords.SetItemText (i, 2 /* column */, WholeWordText (sDlg.m_bWholeWord));
							m_sWords.SetItemText (i, 3 /* column */, TypeText (sDlg.m_iType));
							m_sWords.SetItemText (i, 4 /* column */, WhereText (sDlg.m_iWhere));
							m_sWords.SetItemText (i, 5 /* column */,
								ExpiresText (sDlg.m_iExpiration != 0,
								sDlg.m_iExpirationDays, CTime::GetCurrentTime ()));

							// set data structure data
							POSITION pos = psScoreSet -> GetHeadPosition ();
							for (int i = 0; i <= iScoreSetIndex; i++) {
								Score &sScore = psScoreSet -> GetNext (pos);
								if (i == iScoreSetIndex) {
									sScore.lScore = sDlg.m_lScore;
									sScore.bWholeWord = sDlg.m_bWholeWord;
									sScore.iType = sDlg.m_iType;
									sScore.iWhere = sDlg.m_iWhere;
									sScore.bExpires = sDlg.m_iExpiration != 0;
									sScore.iExpirationDays = sDlg.m_iExpirationDays;

									// score is 'seen' as of now
									sScore.Seen ();
								}
							}
						}
						else {

							// make sure this isn't a duplicate phrase
							if (PhraseExists (psScoreSet, sDlg.m_strPhrase, iScoreSetIndex)) {
								MsgResource (IDS_ERR_DUPLICATE_PHRASE);
								GotoDlgCtrl (&m_sWords);
								GrayControls ();
								return;
							}

							// set list control data
							m_sWords.SetItemText (i, 0 /* column */,
								gstrSpaces + sDlg.m_strPhrase);
							m_sWords.SetItemText (i, 1 /* column */, strScore);
							m_sWords.SetItemText (i, 2 /* column */,
								WholeWordText (sDlg.m_bWholeWord));
							m_sWords.SetItemText (i, 3 /* column */,
								TypeText (sDlg.m_iType));
							m_sWords.SetItemText (i, 4 /* column */,
								WhereText (sDlg.m_iWhere));
							m_sWords.SetItemText (i, 5 /* column */,
								ExpiresText (sDlg.m_iExpiration != 0, sDlg.m_iExpirationDays,
								CTime::GetCurrentTime ()));

							// set data structure data
							POSITION pos = psScoreSet -> GetHeadPosition ();
							for (int i = 0; i <= iScoreSetIndex; i++) {
								Score &sScore = psScoreSet -> GetNext (pos);
								if (i == iScoreSetIndex) {
									sScore.strWord = sDlg.m_strPhrase;
									sScore.lScore = sDlg.m_lScore;
									sScore.bWholeWord = sDlg.m_bWholeWord;
									sScore.iType = sDlg.m_iType;
									sScore.iWhere = sDlg.m_iWhere;
									sScore.bExpires = sDlg.m_iExpiration != 0;
									sScore.iExpirationDays = sDlg.m_iExpirationDays;

									// score is 'seen' as of now
									sScore.Seen ();
								}
							}
						}
					}
		}

		GotoDlgCtrl (&m_sWords);
		SetDirty ();
		GrayControls ();
}

// -------------------------------------------------------------------------
void TScoringDlg::OnEndlabeleditWords(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	// make sure phrase is non-null
	if (!pDispInfo -> item.pszText || !pDispInfo -> item.pszText [0]) {
		*pResult = 0;  // do not accept the change
		return;
	}

	// change the phrase in this item's data structure
	int iIndex = pDispInfo -> item.iItem;
	CString strLabel = pDispInfo -> item.pszText;
	strLabel.TrimLeft ();

	// find the score set that this item corresponds to
	int iScoreSetIndex;
	ScoreSet *psScoreSet;
	GetScoreSetAndIndex (iIndex, iScoreSetIndex, psScoreSet);
	iScoreSetIndex --;

	DWORD dwData = m_sWords.GetItemData (iIndex);
	if (dwData == (DWORD) -1) {

		// make sure this isn't a duplicate phrase
		if (PhraseExists (psScoreSet, strLabel, iScoreSetIndex)) {
			MsgResource (IDS_ERR_DUPLICATE_PHRASE);
			*pResult = 0;  // don't accept the change
			return;
		}

		// phrase/score pair
		POSITION pos = psScoreSet -> GetHeadPosition ();
		for (int i = 0; i <= iScoreSetIndex; i++) {
			Score &sScore = psScoreSet -> GetNext (pos);
			if (i == iScoreSetIndex)
				sScore.strWord = strLabel;
		}
	}
	else
		// group
		psScoreSet -> strGroup = strLabel;

	SetDirty ();
	*pResult = 1;  // accept the change
}

// -------------------------------------------------------------------------
void TScoringDlg::OnDblclkWords(NMHDR*, LRESULT* pResult)
{
	OnEdit ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TScoringDlg::OnItemchangedWords(NMHDR*, LRESULT* pResult)
{
	GrayControls ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TScoringDlg::OnKeydownWords(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

	if (pLVKeyDow -> wVKey == VK_INSERT)
		OnAddWord ();
	if (pLVKeyDow -> wVKey == VK_DELETE)
		OnDelete ();

	*pResult = 0;
}

// -------------------------------------------------------------------------
static void RemoveSelections (CListCtrl &sCtrl)
{
	int iCount = sCtrl.GetItemCount ();
	for (int i = 0; i < iCount; i ++)
		if (sCtrl.GetItemState (i, LVIS_SELECTED))
			sCtrl.SetItemState (i, 0, LVIS_SELECTED);
}

// -------------------------------------------------------------------------
// TrimGroupName -- removes junk from left side of a group name
static CString TrimGroupName (const CString &strName)
{
	CString strResult = strName;
	while (!strResult.IsEmpty () &&
		(strResult [0] == '*' || strResult [0] == '?' || strResult [0] == ' '))
		strResult = strResult.Right (strResult.GetLength () - 1);
	return strResult;
}

// -------------------------------------------------------------------------
// CompareGroups -- compares two group names. Returns 0 if equal, -1 if first
// is smaller, 1 if first is bigger
static int CompareGroups (const CString &strGroup1, const CString &strGroup2)
{
	CString strTrimmedGroup1 = TrimGroupName (strGroup1);
	CString strTrimmedGroup2 = TrimGroupName (strGroup2);
	return strTrimmedGroup1.CompareNoCase (strTrimmedGroup2);
}

// -------------------------------------------------------------------------
void TScoringDlg::OnAddGroup()
{
	TScoreEditGroup sDlg;
	if (sDlg.DoModal () != IDOK)
		return;

	ScoreSet sNew;
	sNew.strGroup = sDlg.m_strGroup;

	// find the position that the new group goes into, both on-screen and in
	// the data structure
	int iGroupNum = 0;
	int iItemCount = m_sWords.GetItemCount ();
	int iIndex = -1;  // list control's index where the new group goes
	int i = 0;
	for (i = 0; i < iItemCount; i++) {
		// if this item is a group, and it doesn't precede the new group, we
		// know where to insert the new group
		if (m_sWords.GetItemData (i) != (DWORD) -1) {
			CString strExistingGroup = m_sWords.GetItemText (i, 0 /* column */);
			if (CompareGroups (sNew.strGroup, strExistingGroup) < 0) {
				iIndex = i;
				break;
			}
			iGroupNum ++;
		}
	}
	if (iIndex == -1)
		iIndex = m_sWords.GetItemCount ();

	// insert new group into data structure
	POSITION pos = m_sScoreSets.GetHeadPosition ();
	for (i = 0; i < iGroupNum; i++)
		m_sScoreSets.GetNext (pos);
	if (pos)
		pos = m_sScoreSets.InsertBefore (pos, sNew);
	else
		pos = m_sScoreSets.AddTail (sNew);

	m_sWords.InsertItem (iIndex, sNew.strGroup);
	m_sWords.EnsureVisible (iIndex, FALSE /* bPartialOK */);

	// list-control item's data is the score-set index within m_sScoreSets
	m_sWords.SetItemData (iIndex, (DWORD) pos);

	// set focus to list-control and put focus on the new group
	GotoDlgCtrl (&m_sWords);
	RemoveSelections (m_sWords);
	m_sWords.SetItemState (iIndex, LVIS_FOCUSED | LVIS_SELECTED,
		LVIS_FOCUSED | LVIS_SELECTED);

	SetDirty ();
}

// -------------------------------------------------------------------------
// GetScoreSetAndIndex -- takes an index in the list-control, and returns
// the ScoreSet corresponding to the item at that index, and the
// score-set-internal index of that list-control item
void TScoringDlg::GetScoreSetAndIndex (int iStartIndex, int &iScoreSetIndex,
									   ScoreSet *&psScoreSet)
{
	iScoreSetIndex = 0;
	psScoreSet = 0;
	int iGroupIndex = iStartIndex;
	while (iGroupIndex >= 0) {
		DWORD dwData = m_sWords.GetItemData (iGroupIndex);

		// if data is -1, we're looking at a phrase/score pair
		if (dwData == -1) {
			iScoreSetIndex ++;
			iGroupIndex --;
			continue;
		}

		// we're looking at a group
		POSITION pos = (POSITION) dwData;
		psScoreSet = &m_sScoreSets.GetAt (pos);
		break;
	}
	ASSERT (psScoreSet);
}

// -------------------------------------------------------------------------
void TScoringDlg::OnAddWord()
{
	TScoreEdit sDlg;

	// set MRU values for whole-word, type, and where, etc...
	sDlg.m_bWholeWord = gpGlobalOptions -> GetRegSwitch () -> GetScoreWholeWord ();
	sDlg.m_iType = gpGlobalOptions -> GetRegSwitch () -> GetScoreType ();
	sDlg.m_iWhere = gpGlobalOptions -> GetRegSwitch () -> GetScoreWhere ();
	sDlg.m_iExpiration =
		gpGlobalOptions -> GetRegSwitch () -> GetScoreExpires () ? 1 : 0;
	sDlg.m_iExpirationDays =
		gpGlobalOptions -> GetRegSwitch () -> GetScoreExpirationDays ();

	if (sDlg.DoModal () != IDOK)
		return;

	// remember values for whole-word, type, and where, etc...
	gpGlobalOptions -> GetRegSwitch () -> SetScoreWholeWord (sDlg.m_bWholeWord);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreType (sDlg.m_iType);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreWhere (sDlg.m_iWhere);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreExpires (sDlg.m_iExpiration != 0);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreExpirationDays (sDlg.m_iExpirationDays);
	gpStore -> SaveGlobalOptions ();

	// make new score
	Score sNew;
	sNew.strWord = sDlg.m_strPhrase;
	sNew.lScore = sDlg.m_lScore;
	sNew.bWholeWord = sDlg.m_bWholeWord;
	sNew.iType = sDlg.m_iType;
	sNew.iWhere = sDlg.m_iWhere;
	sNew.bExpires = sDlg.m_iExpiration != 0;
	sNew.iExpirationDays = sDlg.m_iExpirationDays;

	// score is 'seen' as of now
	sNew.Seen ();

	// if there are no groups, add a default one
	int iCount = m_sWords.GetItemCount ();
	if (!iCount) {
		ScoreSet sNewSet;
		sNewSet.strGroup = "*";
		POSITION pos = m_sScoreSets.AddTail (sNewSet);
		m_sWords.InsertItem (0, sNewSet.strGroup);
		m_sWords.SetItemData (0, (DWORD) pos);
	}

	// to add the new item in its proper sorted order, find the list-control
	// item that precedes it in alphabetic order
	int iFocused;
	for (iFocused = 0; iFocused < iCount; iFocused ++)
		if (m_sWords.GetItemState (iFocused, LVIS_FOCUSED))
			break;
	if (iFocused >= iCount)
		iFocused = 0;

	// work from the focused item up until we find the group, then down until
	// we find the proper index
	int iIndex = iFocused;
	while (iIndex >= 0) {
		if (m_sWords.GetItemData (iIndex) != (DWORD) -1)
			break;
		iIndex --;
	}
	while (iIndex < iCount - 1) {
		int iNextIndex = iIndex + 1;
		CString strExistingWord =
			m_sWords.GetItemText (iNextIndex, 0 /* column */);
		strExistingWord.TrimLeft ();
		if (m_sWords.GetItemData (iNextIndex) != (DWORD) -1)
			// next word is a group, so we're done
			break;
		if (sNew.strWord.CompareNoCase (strExistingWord) < 0)
			// next word is alphabetically after the new one, so we're done
			break;
		iIndex ++;
	}
	iIndex ++;

	// find the score set that this item will be added to
	int iScoreSetIndex;
	ScoreSet *psScoreSet;
	GetScoreSetAndIndex (iIndex - 1, iScoreSetIndex, psScoreSet);

	// make sure this isn't a duplicate phrase
	if (PhraseExists (psScoreSet, sNew.strWord)) {
		MsgResource (IDS_ERR_DUPLICATE_PHRASE);
		return;
	}

	// insert new score into score set
	POSITION pos = psScoreSet -> GetHeadPosition ();
	for (int i = 0; i < iScoreSetIndex; i++)
		psScoreSet -> GetNext (pos);
	if (pos)
		psScoreSet -> InsertBefore (pos, sNew);
	else
		psScoreSet -> AddTail (sNew);

	// insert new score into list control
	CString strWord = gstrSpaces + sNew.strWord;
	m_sWords.InsertItem (iIndex, strWord);
	CString strScore;
	strScore.Format (_T("%ld"), sNew.lScore);
	m_sWords.SetItemText (iIndex, 1 /* column */, strScore);
	m_sWords.SetItemText (iIndex, 2 /* column */, WholeWordText (sNew.bWholeWord));
	m_sWords.SetItemText (iIndex, 3 /* column */, TypeText (sNew.iType));
	m_sWords.SetItemText (iIndex, 4 /* column */, WhereText (sNew.iWhere));
	m_sWords.SetItemText (iIndex, 5 /* column */, ExpiresText (sNew.bExpires,
		sNew.iExpirationDays, sNew.LastSeen ()));
	m_sWords.EnsureVisible (iIndex, FALSE /* bPartialOK */);
	// list-control item's data is -1 to mark it as a phrase/score pair
	m_sWords.SetItemData (iIndex, (DWORD) -1);

	// set focus to list-control and put focus on the new word
	GotoDlgCtrl (&m_sWords);
	RemoveSelections (m_sWords);
	m_sWords.SetItemState (iIndex, LVIS_FOCUSED | LVIS_SELECTED,
		LVIS_FOCUSED | LVIS_SELECTED);
	SetDirty ();
}

// -------------------------------------------------------------------------
TScoreEdit::TScoreEdit(CWnd* pParent /*=NULL*/)
: CDialog(TScoreEdit::IDD, pParent)
{

	m_strPhrase = _T("");
	m_lScore = 0;
	m_iWhere = -1;
	m_iType = -1;
	m_bWholeWord = FALSE;
	m_iExpiration = -1;
	m_iExpirationDays = 0;


	m_lScore = s_lLastScore;
	m_bMultiple = FALSE;
	m_bWholeWord = FALSE;
	m_iType = SCORE_TYPE_TEXT;
	m_iWhere = SCORE_WHERE_ALL;

	m_fTestCompilePhrase = true;  // during DDV
}

// -------------------------------------------------------------------------
void TScoreEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Control(pDX, IDC_EXPIRATION_DAYS, m_sExpirationDays);
	DDX_Control(pDX, IDC_PHRASE, m_sPhrase);
	DDX_Text(pDX, IDC_PHRASE, m_strPhrase);
	DDX_Text(pDX, IDC_SCORE, m_lScore);
	DDX_Radio(pDX, IDC_SUBJECT, m_iWhere);
	DDX_Radio(pDX, IDC_TEXT, m_iType);
	DDX_Check(pDX, IDC_WHOLE_WORD, m_bWholeWord);
	DDX_Radio(pDX, IDC_EXPIRATION_NONE, m_iExpiration);
	DDX_Text(pDX, IDC_EXPIRATION_DAYS, m_iExpirationDays);
	DDV_MinMaxUInt(pDX, m_iExpirationDays, 1, 999);


	if (pDX->m_bSaveAndValidate && !m_bMultiple)
	{

		// don't perform check if we are called from GrayControls()
		if (m_fTestCompilePhrase)
		{
			// make sure we can compile the search phrase

			BOOL bRE           =   m_iType == SCORE_TYPE_RE;
			BOOL bWildmat      =   m_iType == SCORE_TYPE_WILDMAT;
			CString testPhrase =   m_strPhrase;

			if (bWildmat)
				WildmatToRE (m_strPhrase, testPhrase, FALSE /* bMatchAll */);

			try
			{
				TSearch sSearch;

				sSearch.SetPattern ( testPhrase,
					FALSE,     // fCaseSensitive
					(bRE || bWildmat)
					? TSearch::RE
					: TSearch::NON_RE);
			}
			catch(...)
			{
				pDX->Fail ();
			}
		}
	}
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TScoreEdit, CDialog)
		ON_BN_CLICKED(IDC_EXPIRATION_NONE, OnExpirationNone)
	ON_BN_CLICKED(IDC_EXPIRES2, OnExpires2)
	ON_EN_CHANGE(IDC_PHRASE, OnChangePhrase)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TScoreEdit::OnInitDialog()
{
	CDialog::OnInitDialog();
	GrayControls ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TScoreEdit::OnOK()
{
	CDialog::OnOK();
	s_lLastScore = m_lScore;
}

// -------------------------------------------------------------------------
void TScoreEdit::OnExpirationNone()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreEdit::OnExpires2()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreEdit::GrayControls ()
{
	m_fTestCompilePhrase = false;
	UpdateData ();
	m_fTestCompilePhrase = true;

	m_sPhrase.EnableWindow (!m_bMultiple);
	m_sOK.EnableWindow (m_bMultiple || !m_strPhrase.IsEmpty ());
	m_sExpirationDays.EnableWindow (m_iExpiration != 0);
}

// -------------------------------------------------------------------------
void TScoreEdit::OnChangePhrase()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
TScoreEditGroup::TScoreEditGroup(CWnd* pParent /*=NULL*/)
: CDialog(TScoreEditGroup::IDD, pParent)
{

	m_strGroup = _T("");
}

// -------------------------------------------------------------------------
void TScoreEditGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_GROUP, m_strGroup);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TScoreEditGroup, CDialog)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TQuickScore::TQuickScore(CWnd* pParent /*=NULL*/)
: CDialog(TQuickScore::IDD, pParent)
{

	m_strPhrase = _T("");
	m_bRescore = FALSE;
	m_strScore = _T("");
	m_bWholeWord = FALSE;
	m_iWhere = -1;
	m_iType = -1;
	m_iExpirationDays = 0;
	m_iExpiration = -1;
}

// -------------------------------------------------------------------------
void TQuickScore::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EXPIRATION_DAYS, m_sExpirationDays);
	DDX_Control(pDX, IDC_SCORE, m_sScore);
	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Text(pDX, IDC_PHRASE, m_strPhrase);
	DDX_Check(pDX, IDC_RESCORE, m_bRescore);
	DDX_CBString(pDX, IDC_SCORE, m_strScore);
	DDX_Check(pDX, IDC_WHOLE_WORD, m_bWholeWord);
	DDX_Radio(pDX, IDC_SUBJECT, m_iWhere);
	DDX_Radio(pDX, IDC_TEXT, m_iType);
	DDX_Text(pDX, IDC_EXPIRATION_DAYS, m_iExpirationDays);
	DDV_MinMaxUInt(pDX, m_iExpirationDays, 1, 999);
	DDX_Radio(pDX, IDC_EXPIRATION_NONE, m_iExpiration);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TQuickScore, CDialog)
		ON_EN_CHANGE(IDC_PHRASE, OnChangePhrase)
	ON_BN_CLICKED(IDC_MAIN_SCORING, OnMainScoring)
	ON_BN_CLICKED(IDC_EXPIRATION_NONE, OnExpirationNone)
	ON_BN_CLICKED(IDC_EXPIRES2, OnExpires2)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TQuickScore::OnChangePhrase()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TQuickScore::GrayControls ()
{
	UpdateData ();
	m_sOK.EnableWindow (!m_strPhrase.IsEmpty ());
	m_sExpirationDays.EnableWindow (m_iExpiration != 0);
}

// -------------------------------------------------------------------------
BOOL TQuickScore::OnInitDialog()
{
	// set MRU values for whole-word, type, where, etc...
	m_bWholeWord = gpGlobalOptions -> GetRegSwitch () -> GetScoreWholeWord ();
	m_iType = gpGlobalOptions -> GetRegSwitch () -> GetScoreType ();
	m_iWhere = gpGlobalOptions -> GetRegSwitch () -> GetScoreWhere ();
	m_iExpiration =
		gpGlobalOptions -> GetRegSwitch () -> GetScoreExpires () ? 1 : 0;
	m_iExpirationDays =
		gpGlobalOptions -> GetRegSwitch () -> GetScoreExpirationDays ();

	// score is initialized to last score entered. If no score was entered
	// this session, use the persistent value from the last session
	m_strScore.Format (_T("%ld"),
		TScoreEdit::s_lLastScore ?
		TScoreEdit::s_lLastScore :
	gpGlobalOptions -> GetRegUI () -> GetLastQuickScore ());

	m_bOriginalRescore = m_bRescore =
		gpGlobalOptions -> GetRegSwitch () -> GetAutoRescore ();
	CDialog::OnInitDialog();
	GrayControls ();

	// get score MRU values from registry and put them in m_rstrMRU
	CString strMRU = gpGlobalOptions -> GetRegUI () -> GetScoreMRU ();
	int i;
	for (i = 0; i < 5; i++) {
		strMRU.TrimLeft ();
		while (!strMRU.IsEmpty () && strMRU [0] != ' ') {
			char ch = strMRU [0];
			strMRU = strMRU.Right (strMRU.GetLength () - 1);
			m_rstrMRU [i] += ch;
		}
	}

	// add MRU values to combobox
	BOOL bAdded = FALSE;
	for (i = 0; i < 5; i++)
		if (!m_rstrMRU [i].IsEmpty ()) {
			m_sScore.AddString (m_rstrMRU [i]);
			bAdded = TRUE;
		}
		if (!bAdded)
			m_sScore.AddString (_T("100"));

		return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
static void AddPair (CString &strPhrase, long lScore, BOOL bWholeWord,
					 int iType, int iWhere, BOOL bExpires, int iExpirationDays)
{
	// get score sets
	ScoreSets *pScoreSets = gpStore -> GetScoreSets ();
	pScoreSets -> WriteLock ();

	// make new score
	Score sNew;
	sNew.strWord = strPhrase;
	sNew.lScore = lScore;
	sNew.bWholeWord = bWholeWord;
	sNew.iType = iType;
	sNew.iWhere = iWhere;
	sNew.bExpires = bExpires;
	sNew.iExpirationDays = iExpirationDays;

	// score is 'seen' as of now
	sNew.Seen ();

	// get current group's name
	CNewsView *pView = GetNewsView ();
	LONG lGroup = pView -> GetCurNewsGroupID ();
	TNewsGroup *pNG;
	BOOL fUseLock;
	TServerCountedPtr cpNewsServer;
	TNewsGroupUseLock (cpNewsServer, lGroup, &fUseLock, pNG);
	if (!fUseLock) {
		ASSERT (0);
		return;
	}
	const CString &strGroup = pNG -> GetName ();

	// get current group's score-set
	ScoreSet *pScoreSet = 0;
	POSITION pos = pScoreSets -> GetHeadPosition ();
	while (pos) {
		POSITION prevPos = pos;
		ScoreSet &sScoreSet = pScoreSets -> GetNext (pos);

		int iComp = CompareGroups (sScoreSet.strGroup, strGroup);
		if (!iComp) {
			// found it
			pScoreSet = &sScoreSet;
			pos = prevPos;
			break;
		}

		if (iComp > 0) {
			// passed this group's position, so the group doesn't exist
			pos = prevPos;
			break;
		}
	}

	// if group didn't exist, add it
	if (!pScoreSet) {

		ScoreSet sNewSet;
		sNewSet.strGroup = strGroup;
		if (pos)
			pos = pScoreSets -> InsertBefore (pos, sNewSet);
		else
			pos = pScoreSets -> AddTail (sNewSet);

		ScoreSet &sAdded = pScoreSets -> GetAt (pos);
		pScoreSet = &sAdded;
	}

	// make sure this isn't a duplicate phrase
	if (PhraseExists (pScoreSet, strPhrase)) {
		MsgResource (IDS_ERR_DUPLICATE_PHRASE);
		return;
	}

	// find the position to insert the new pair and insert it
	pos = pScoreSet -> GetHeadPosition ();
	while (pos) {
		POSITION prevPos = pos;
		Score &sScore = pScoreSet -> GetNext (pos);
		if (sScore.strWord.CompareNoCase (strPhrase) > 0) {
			pos = prevPos;
			break;
		}
	}
	if (pos)
		pScoreSet -> InsertBefore (pos, sNew);
	else
		pScoreSet -> AddTail (sNew);

	// save score sets back
	pScoreSets -> UnlockWrite ();
	gpStore -> SaveScoreSets ();
}

// -------------------------------------------------------------------------
void TQuickScore::OnOK()
{
	UpdateData ();

	// remember values for whole-word, type, and where
	gpGlobalOptions -> GetRegSwitch () -> SetScoreWholeWord (m_bWholeWord);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreType (m_iType);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreWhere (m_iWhere);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreExpires (m_iExpiration != 0);
	gpGlobalOptions -> GetRegSwitch () -> SetScoreExpirationDays (m_iExpirationDays);
	gpStore -> SaveGlobalOptions ();

	long lScore = _ttol (m_strScore);
	TScoreEdit::s_lLastScore = lScore;
	gpGlobalOptions -> GetRegUI () -> SetLastQuickScore (lScore);
	AddPair (m_strPhrase, lScore, m_bWholeWord, m_iType, m_iWhere,
		m_iExpiration != 0, m_iExpirationDays);

	if (m_bOriginalRescore != m_bRescore) {
		gpGlobalOptions -> GetRegSwitch () -> SetAutoRescore (m_bRescore);
		gpStore -> SaveGlobalOptions ();
	}

	if (m_bRescore)
		RescoreCurrentGroup ();

	// update MRU values
	BOOL bExists = FALSE;
	int i;
	for (i = 0; i < 5; i++)
		if (m_rstrMRU [i] == m_strScore)
			bExists = TRUE;
	if (!bExists) {
		for (i = 4; i >= 1; i--)
			m_rstrMRU [i] = m_rstrMRU [i-1];
		m_rstrMRU [0] = m_strScore;
		CString strMRU;
		for (i = 0; i < 5; i++)
			if (!m_rstrMRU [i].IsEmpty ()) {
				if (!strMRU.IsEmpty ())
					strMRU += " ";
				strMRU += m_rstrMRU [i];
			}
			gpGlobalOptions -> GetRegUI () -> SetScoreMRU (strMRU);
			gpGlobalOptions -> SaveUISettings ();
	}

	CDialog::OnOK();
}

// -------------------------------------------------------------------------
void TQuickScore::OnMainScoring()
{
	AfxGetMainWnd () -> PostMessage (WM_COMMAND, ID_SCORING);
	CDialog::OnOK ();
}

// -------------------------------------------------------------------------
void TQuickScore::OnExpirationNone()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TQuickScore::OnExpires2()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void RescoreCurrentGroup ()
{
	CNewsView *pView = GetNewsView ();
	LONG lGroup = pView -> GetCurNewsGroupID ();

	// initialize pNG
	TNewsGroup *pNG = 0;
	BOOL fUseLock;
	TServerCountedPtr cpNewsServer;
	TNewsGroupUseLock (cpNewsServer, lGroup, &fUseLock, pNG);
	if (!fUseLock) {
		ASSERT (0);
		return;
	}

	CWaitCursor wait;

	pNG -> Open ();
	pNG -> WriteLock ();

	// initialize group's info
	TThreadList *psThreadList = 0;// thread-list to go through

	// if this newsgroup's thread-list is already constructed, use it.
	// Otherwise, make our own threadlist
	TArticleIndexList sIndexList; // index of articles to go through
	if (pNG -> FlatListLength ()) {
		psThreadList = pNG -> GetpThreadList ();
		psThreadList -> CreateArticleIndex (&sIndexList);
	}
	else {
		pNG -> LoadForArticleIndex (TNewsGroup::GetpCurViewFilter (),
			FALSE /* fCreateStati */, TRUE /* fHitServer */, &sIndexList);
	}

	POSITION pos = sIndexList -> GetHeadPosition ();
	while (pos) {
		TArticleHeader *pHdr = sIndexList -> GetNext (pos);

		// reset header's score
		pHdr -> SetScore (0);
		pHdr -> SetBodyScore (0);

		if (ScoreHeader (pNG, pHdr))
			break;

		// Try to get body from local database... if it's there, score it too
		TArticleText * pBody;
		CPoint         ptPartID(0,0);
		TError         sErrorRet;

		if (!fnFetchBody (sErrorRet,
			pNG, 
			pHdr, 
			pBody, 
			ptPartID, 
			FALSE /* bMarkAsRead */,
			FALSE /* bTryFromNewsfeed */)) {

				int RC = ScoreBody (pHdr, pBody, pNG);

				// our responsibility to delete the fetched body
				delete pBody;

				if (RC)
					break;   // error scoring body
		}
	}

	// must be done before closing the group, because closing the group will
	// free the headers
	sIndexList.Empty ();

	pNG -> SetDirty ();
	pNG -> HeadersDirty ();
	pNG -> UnlockWrite ();
	pNG -> Close ();

	pView -> RefreshCurrentNewsgroup ();
}

// -------------------------------------------------------------------------
bool Score::MatchText (const CString & strTxt)
{
	int   iResultLen = 0;    // length of sequence
	DWORD iPos;              // position of sequence

	LPCTSTR pText = strTxt;

	if (!sSearch.Search (pText, iResultLen, iPos))
		return false;

	// the text matches, but the user might be doing a whole-word search
	if (!bWholeWord)
		return true;

	ASSERT(iPos + iResultLen <= strTxt.GetLength());
	// manually check boundaries

	TCHAR chLeft  = (0 == iPos) ? '\0' : pText[iPos-1];
	TCHAR chRight = pText[iPos + iResultLen];

	if (isalpha(chLeft) || isalpha(chRight))
	{
		// this is part of a bigger word
		return false;
	}
	return true;
}

static void cpu_loader ()
{
	CString s;
	for (int i = 0; i < 50; i++) s += "k";
}

// -------------------------------------------------------------------------
// ScoreHeaderOrBody
//
static int ScoreHeaderOrBody (
							  TArticleHeader * pHdr,
							  TArticleText   * pBody,
							  TNewsGroup     * pNG
							  )
{
	// if scoring a body, reset the header's "body score". The call to
	// GetScore() below is meant to decrease the chances that headers will be
	// dirtied
	if (pBody && pHdr -> GetScore ())
	{
		pNG -> HeadersDirty ();
		pNG -> SetDirty ();
		pHdr -> SetBodyScore (0);
	}

	ScoreSets *pScoreSets     = gpStore -> GetScoreSets ();
	const CString &strGroup   = pNG -> GetName ();
	// find the sets that this group name matches
	POSITION pos = pScoreSets -> GetHeadPosition ();
	while (pos)
	{
		ScoreSet &sScoreSet = pScoreSets -> GetNext (pos);
		TSearch &sSearch = sScoreSet.sSearch;

		// compile the score-set's pattern if needed
		if (!sSearch.HasPattern ())
		{
			CString strRE;
			WildmatToRE (sScoreSet.strGroup, strRE);
			try
			{
				sSearch.SetPattern (strRE,
					FALSE,         // fCaseSensitive
					TSearch::RE);  // iSearchType
			}
			catch(...)
			{
				return 1;
			}
		}

		DWORD iPos;
		int   iResultLen;

		// does it match name of the newsgroup ?
		if (sSearch.Search (strGroup, iResultLen, iPos))
		{
			// go through this score set and look for matches in
			// subject & from lines

			ApplyScoreSet (sScoreSet, pNG, pHdr, pBody);

		}
	}
	return 0;
}

// -------------------------------------------------------------------------
static int ApplyScoreSet (
						  ScoreSet &        sScoreSet,
						  TNewsGroup *      pNG,
						  TArticleHeader *  pHdr,
						  TArticleText *    pBody
						  )
{
	CPtrList sRemoveList;
	sRemoveList.RemoveAll ();

	POSITION pos2 = sScoreSet.GetHeadPosition ();
	while (pos2)
	{
		POSITION sOriginalPos = pos2;
		Score &sScore = sScoreSet.GetNext (pos2);

		// check for expiration
		if (sScore.bExpires)
		{
			CTimeSpan sDays (sScore.iExpirationDays /* days */,
				0                      /* hours */,
				0                      /* minutes */,
				0                      /* seconds */);

			CTime sSeenCutoff = CTime::GetCurrentTime () - sDays;
			time_t lCutoff = sSeenCutoff.GetTime ();
			if (sScore.NotSeenSince (lCutoff))
			{
				sRemoveList.AddTail ((void *) sOriginalPos);
				continue;
			}
		}

		TestAndApplyScore (sScore, pNG, pHdr, pBody);

	} // while loop

	// remove expired entries
	pos2 = sRemoveList.GetHeadPosition ();
	while (pos2)
	{
		POSITION sTempPos = (POSITION) sRemoveList.GetNext (pos2);
		sScoreSet.RemoveAt (sTempPos);
	}
	return 0;
}

// -------------------------------------------------------------------------
static int TestAndApplyScore (
							  Score &           sScore,
							  TNewsGroup *      pNG,
							  TArticleHeader *  pHdr,
							  TArticleText *    pBody
							  )
{
	const CString &strSubject = pHdr -> GetSubject ();
	const CString &strFrom    = pHdr -> GetFrom ();

	TSearch &sSearch = sScore.sSearch;

	// compile the score's pattern if needed
	if (!sSearch.HasPattern ())
	{
		BOOL bRE = sScore.iType == SCORE_TYPE_RE;
		BOOL bWildmat = sScore.iType == SCORE_TYPE_WILDMAT;
		BOOL bWholeWord = sScore.bWholeWord;
		CString strWord = sScore.strWord;

		if (bWildmat)
			WildmatToRE (sScore.strWord, strWord, FALSE /* bMatchAll */);

		try
		{
			sSearch.SetPattern (strWord,
				FALSE /* fCaseSensitive */,
				(bRE || bWildmat)
				? TSearch::RE
				: TSearch::NON_RE);
		}
		catch(...)
		{
			return 1;
		}
	}

	// search subject, from and body as needed
	BOOL bSubject = FALSE, bFrom = FALSE, bBody = FALSE;
	switch (sScore.iWhere)
	{
	case SCORE_WHERE_SUBJECT:
		bSubject = TRUE;
		break;
	case SCORE_WHERE_FROM:
		bFrom    = TRUE;
		break;
	case SCORE_WHERE_BODY:
		bBody    = TRUE;
		break;
	default:
		bSubject = TRUE;
		bFrom    = TRUE;
		bBody    = TRUE;
	}

	// if body is available, score only the body... the subject and
	// from fields will be scored separately
	if (pBody)
	{
		if (bBody)
		{
			BOOL bFoundElsewhere =
				(bSubject && sScore.MatchText (strSubject)) ||
				(bFrom    && sScore.MatchText (strFrom));

			if (!bFoundElsewhere)
			{
				if (sScore.MatchText (pBody -> GetBody ()))
				{
					pHdr -> AddToBodyScore (sScore.lScore);
					pNG -> HeadersDirty ();
					pNG -> SetDirty ();
					sScore.Seen ();
				}
			}
		}
	}
	else
		if ((bSubject && sScore.MatchText (strSubject)) ||
			(bFrom    && sScore.MatchText (strFrom)))
		{
			pHdr -> AddToScore (sScore.lScore);
			sScore.Seen ();
		}

		return 0;
}

// -------------------------------------------------------------------------
// ScoreHeader -- takes a header and applies the scoring system, i.e., adds
// to its score based on whether it matches any scoring criteria. Returns 0
// for success, and non-0 for error
//
// Process one header or multiple
//
int ScoreHeader (TNewsGroup * pNG, TArticleHeader * pHdr)
{
	return ScoreHeaderOrBody (pHdr, NULL /* pBody */, pNG);
}

// -------------------------------------------------------------------------
int ScoreBody (TArticleHeader *pHdr, TArticleText *pBody, TNewsGroup *pNG)
{
	return ScoreHeaderOrBody (pHdr, pBody, pNG);
}

// -------------------------------------------------------------------------
TScoreColors::TScoreColors(CWnd* pParent /*=NULL*/)
: CDialog(TScoreColors::IDD, pParent)
{

	m_lTo1 = 0;
	m_lFrom1 = 0;
	m_lFrom2 = 0;
	m_lFrom3 = 0;
	m_lFrom4 = 0;
	m_lFrom5 = 0;
	m_lTo2 = 0;
	m_lTo3 = 0;
	m_lTo4 = 0;
	m_lTo5 = 0;
	m_bEnabled1 = FALSE;
	m_bEnabled2 = FALSE;
	m_bEnabled3 = FALSE;
	m_bEnabled4 = FALSE;
	m_bEnabled5 = FALSE;
}

// -------------------------------------------------------------------------
void TScoreColors::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TO5, m_sTo5);
	DDX_Control(pDX, IDC_TO4, m_sTo4);
	DDX_Control(pDX, IDC_TO3, m_sTo3);
	DDX_Control(pDX, IDC_TO2, m_sTo2);
	DDX_Control(pDX, IDC_TO1, m_sTo1);
	DDX_Control(pDX, IDC_FROM5, m_sFrom5);
	DDX_Control(pDX, IDC_FROM4, m_sFrom4);
	DDX_Control(pDX, IDC_FROM3, m_sFrom3);
	DDX_Control(pDX, IDC_FROM2, m_sFrom2);
	DDX_Control(pDX, IDC_FROM1, m_sFrom1);
	DDX_Control(pDX, IDC_COLOR5, m_sColor5);
	DDX_Control(pDX, IDC_COLOR4, m_sColor4);
	DDX_Control(pDX, IDC_COLOR3, m_sColor3);
	DDX_Control(pDX, IDC_COLOR2, m_sColor2);
	DDX_Control(pDX, IDC_COLOR1, m_sColor1);
	DDX_Text(pDX, IDC_TO1, m_lTo1);
	DDX_Text(pDX, IDC_FROM1, m_lFrom1);
	DDX_Text(pDX, IDC_FROM2, m_lFrom2);
	DDX_Text(pDX, IDC_FROM3, m_lFrom3);
	DDX_Text(pDX, IDC_FROM4, m_lFrom4);
	DDX_Text(pDX, IDC_FROM5, m_lFrom5);
	DDX_Text(pDX, IDC_TO2, m_lTo2);
	DDX_Text(pDX, IDC_TO3, m_lTo3);
	DDX_Text(pDX, IDC_TO4, m_lTo4);
	DDX_Text(pDX, IDC_TO5, m_lTo5);
	DDX_Check(pDX, IDC_ENABLED1, m_bEnabled1);
	DDX_Check(pDX, IDC_ENABLED2, m_bEnabled2);
	DDX_Check(pDX, IDC_ENABLED3, m_bEnabled3);
	DDX_Check(pDX, IDC_ENABLED4, m_bEnabled4);
	DDX_Check(pDX, IDC_ENABLED5, m_bEnabled5);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TScoreColors, CDialog)
		ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_COLOR1, OnColor1)
	ON_BN_CLICKED(IDC_COLOR2, OnColor2)
	ON_BN_CLICKED(IDC_COLOR3, OnColor3)
	ON_BN_CLICKED(IDC_COLOR4, OnColor4)
	ON_BN_CLICKED(IDC_COLOR5, OnColor5)
	ON_BN_CLICKED(IDC_ENABLED1, OnEnabled1)
	ON_BN_CLICKED(IDC_ENABLED2, OnEnabled2)
	ON_BN_CLICKED(IDC_ENABLED3, OnEnabled3)
	ON_BN_CLICKED(IDC_ENABLED4, OnEnabled4)
	ON_BN_CLICKED(IDC_ENABLED5, OnEnabled5)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TScoreColors::OnInitDialog()
{
	for (int i = 0; i < 5; i++)
		m_rScoreColors [i] = grScoreColors [i];

	m_lFrom1 = m_rScoreColors [0].lFrom;
	m_lFrom2 = m_rScoreColors [1].lFrom;
	m_lFrom3 = m_rScoreColors [2].lFrom;
	m_lFrom4 = m_rScoreColors [3].lFrom;
	m_lFrom5 = m_rScoreColors [4].lFrom;

	m_lTo1 = m_rScoreColors [0].lTo;
	m_lTo2 = m_rScoreColors [1].lTo;
	m_lTo3 = m_rScoreColors [2].lTo;
	m_lTo4 = m_rScoreColors [3].lTo;
	m_lTo5 = m_rScoreColors [4].lTo;

	m_bEnabled1 = m_rScoreColors [0].bEnabled;
	m_bEnabled2 = m_rScoreColors [1].bEnabled;
	m_bEnabled3 = m_rScoreColors [2].bEnabled;
	m_bEnabled4 = m_rScoreColors [3].bEnabled;
	m_bEnabled5 = m_rScoreColors [4].bEnabled;

	CDialog::OnInitDialog();
	GrayControls ();

	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TScoreColors::OnOK()
{

	// check to see if there are any backward score ranges...
	UpdateData (TRUE);

	if ((m_bEnabled1 && (m_lFrom1 > m_lTo1)) ||
		(m_bEnabled2 && (m_lFrom2 > m_lTo2)) ||
		(m_bEnabled3 && (m_lFrom3 > m_lTo3)) ||
		(m_bEnabled4 && (m_lFrom4 > m_lTo4)) ||
		(m_bEnabled5 && (m_lFrom5 > m_lTo5)))
	{
		CString  errString;
		errString.LoadString (IDS_ERR_SCORECOLOR_RANGE);
		AfxMessageBox (errString);
		return;
	}

	CDialog::OnOK();

	m_rScoreColors [0].lFrom = m_lFrom1;
	m_rScoreColors [1].lFrom = m_lFrom2;
	m_rScoreColors [2].lFrom = m_lFrom3;
	m_rScoreColors [3].lFrom = m_lFrom4;
	m_rScoreColors [4].lFrom = m_lFrom5;

	m_rScoreColors [0].lTo = m_lTo1;
	m_rScoreColors [1].lTo = m_lTo2;
	m_rScoreColors [2].lTo = m_lTo3;
	m_rScoreColors [3].lTo = m_lTo4;
	m_rScoreColors [4].lTo = m_lTo5;

	m_rScoreColors [0].bEnabled = m_bEnabled1;
	m_rScoreColors [1].bEnabled = m_bEnabled2;
	m_rScoreColors [2].bEnabled = m_bEnabled3;
	m_rScoreColors [3].bEnabled = m_bEnabled4;
	m_rScoreColors [4].bEnabled = m_bEnabled5;

	for (int i = 0; i < 5; i++)
		grScoreColors [i] = m_rScoreColors [i];

	gpStore -> SaveScoreColors ();

	CNewsView *psView = GetNewsView ();
	if (psView)
		psView -> RefreshCurrentNewsgroup ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl >= IDC_COLOR1 && nIDCtl <= IDC_COLOR5) {
		int iIndex = nIDCtl - IDC_COLOR1;

		CButton *pButton;
		switch (iIndex) {
		 case 0:
			 pButton = &m_sColor1;
			 break;
		 case 1:
			 pButton = &m_sColor2;
			 break;
		 case 2:
			 pButton = &m_sColor3;
			 break;
		 case 3:
			 pButton = &m_sColor4;
			 break;
		 case 4:
			 pButton = &m_sColor5;
			 break;
		 default:
			 ASSERT (0);
		}
		UINT iState = pButton -> GetState ();
		BOOL bDepress = iState & 4;
		BOOL bFocus = iState & 8;

		CDC sDC;
		sDC.Attach (lpDrawItemStruct -> hDC);
		CRect sRect = lpDrawItemStruct -> rcItem;

		// draw color
		sDC.FillSolidRect (&sRect, m_rScoreColors [iIndex].rgbColor);

		// draw border
		CPen sPen1, sPen2;
		if (bDepress) {
			sPen1.CreatePen (PS_SOLID, 3 /* width */, RGB (0,0,0));
			sPen2.CreatePen (PS_SOLID, 1 /* width */, RGB (0,0,0));
		}
		else if (bFocus) {
			sPen1.CreatePen (PS_SOLID, 1 /* width */, RGB (0,0,0));
			sPen2.CreatePen (PS_SOLID, 2 /* width */, RGB (0,0,0));
		}
		else {
			sPen1.CreatePen (PS_SOLID, 1 /* width */, RGB (210,210,210));
			sPen2.CreatePen (PS_SOLID, 1 /* width */, RGB (0,0,0));
		}
		sDC.SelectObject (&sPen1);
		sDC.MoveTo (sRect.left, sRect.top);
		sDC.LineTo (sRect.right-1, sRect.top);
		sDC.SelectObject (&sPen2);
		sDC.LineTo (sRect.right-1, sRect.bottom-1);
		sDC.LineTo (sRect.left, sRect.bottom-1);
		sDC.SelectObject (&sPen1);
		sDC.LineTo (sRect.left, sRect.top);

		// draw focus rect
		if (bFocus) {
			CPen sPen3;
			sPen3.CreatePen (PS_DOT, 1 /* width */, RGB (0,0,0));
			sDC.SelectObject (&sPen3);
			sDC.MoveTo (sRect.left + 4, sRect.top + 4);
			sDC.LineTo (sRect.right - 4, sRect.top + 4);
			sDC.LineTo (sRect.right - 4, sRect.bottom - 4);
			sDC.LineTo (sRect.left + 4, sRect.bottom - 4);
			sDC.LineTo (sRect.left + 4, sRect.top + 4);
		}

		sDC.Detach ();
		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

// -------------------------------------------------------------------------
void TScoreColors::OnColor (int iIndex)
{
	CColorDialog sDlg (m_rScoreColors [iIndex].rgbColor, CC_ANYCOLOR, this);
	if (sDlg.DoModal () != IDOK)
		return;
	m_rScoreColors [iIndex].rgbColor = sDlg.GetColor ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnColor1()
{
	OnColor (0);
	m_sColor1.Invalidate ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnColor2()
{
	OnColor (1);
	m_sColor2.Invalidate ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnColor3()
{
	OnColor (2);
	m_sColor3.Invalidate ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnColor4()
{
	OnColor (3);
	m_sColor4.Invalidate ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnColor5()
{
	OnColor (4);
	m_sColor5.Invalidate ();
}

// -------------------------------------------------------------------------
ScoreColor::ScoreColor () : PObject (SCORE_COLOR_VERSION)
{
	rgbColor = RGB (255,0,0);
	bEnabled = FALSE;
	lFrom = lTo = 0;
}

// -------------------------------------------------------------------------
void ScoreColor::Serialize (CArchive& sArchive)
{
	PObject::Serialize (sArchive);

	if (sArchive.IsStoring ()) {
		sArchive << rgbColor;
		sArchive << bEnabled;
		sArchive << lFrom;
		sArchive << lTo;
	}
	else {
		sArchive >> rgbColor;
		sArchive >> bEnabled;
		sArchive >> lFrom;
		sArchive >> lTo;
	}
}

// -------------------------------------------------------------------------
void TScoreColors::OnEnabled1()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnEnabled2()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnEnabled3()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnEnabled4()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreColors::OnEnabled5()
{
	GrayControls ();
}

// -------------------------------------------------------------------------
void TScoreColors::GrayControls ()
{
	UpdateData ();
	m_sFrom1.EnableWindow (m_bEnabled1);
	m_sTo1.EnableWindow (m_bEnabled1);
	m_sColor1.EnableWindow (m_bEnabled1);
	m_sFrom2.EnableWindow (m_bEnabled2);
	m_sTo2.EnableWindow (m_bEnabled2);
	m_sColor2.EnableWindow (m_bEnabled2);
	m_sFrom3.EnableWindow (m_bEnabled3);
	m_sTo3.EnableWindow (m_bEnabled3);
	m_sColor3.EnableWindow (m_bEnabled3);
	m_sFrom4.EnableWindow (m_bEnabled4);
	m_sTo4.EnableWindow (m_bEnabled4);
	m_sColor4.EnableWindow (m_bEnabled4);
	m_sFrom5.EnableWindow (m_bEnabled5);
	m_sTo5.EnableWindow (m_bEnabled5);
	m_sColor5.EnableWindow (m_bEnabled5);
}

// -------------------------------------------------------------------------
void ScoreSetBackground (DWORD &dwColor, long lScore)
{
	for (int i = 0; i < 5; i++) {
		ScoreColor &sColor = grScoreColors [i];
		if (!sColor.bEnabled)
			continue;
		if (lScore >= sColor.lFrom && lScore <= sColor.lTo) {
			dwColor = sColor.rgbColor;
			break;
		}
	}
}
