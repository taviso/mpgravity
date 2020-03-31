/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tscoring.h,v $
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

// tscoring.h -- scoring dialog

#pragma once

#ifndef IDD_SCORING
#include "resource.h"
#endif      // #ifndef IDD_SCORING

#include "sharesem.h"            // TSynchronizable
#include "pobject.h"             // PObject
#include "tsearch.h"             // TSearch
#include "declare.h"             // TArtHdrArray
#include "expirable.h"           // TExpirable

class TArticleHeader;
class TArticleText;
class TNewsGroup;
class TThread;

// -------------------------------------------------------------------------
#define SCORE_TYPE_TEXT       0
#define SCORE_TYPE_WILDMAT    1
#define SCORE_TYPE_RE         2

#define SCORE_WHERE_SUBJECT   0
#define SCORE_WHERE_FROM      1
#define SCORE_WHERE_BODY      2
#define SCORE_WHERE_ALL       3  /* subject, from and body */

#define SCORE_VERSION 4

class Score : public PObject, public TExpirable {
public:
	void Serialize (CArchive& sArchive);
	Score ();
	~Score () {}
	Score (const Score &src);
	Score &operator= (const Score &src);

	bool MatchText (const CString& txt);

	TSearch sSearch;
	CString strWord;
	long lScore;
	BOOL bWholeWord;
	int iType;                    // see SCORE_TYPE_*
	int iWhere;                   // see SCORE_WHERE_*
	BOOL bExpires;
	int iExpirationDays;
};

typedef CList <Score, Score&> ScoreSetType;
class ScoreSet : public ScoreSetType {
public:
	ScoreSet (const ScoreSet &src);
	ScoreSet () {}
	~ScoreSet () {}
	ScoreSet &operator= (const ScoreSet &src);
	void Serialize (CArchive& sArchive);   // read/write the object

	CString strGroup;
	TSearch sSearch;
};

typedef CList <ScoreSet, ScoreSet&> ScoreSetListType;
class ScoreSets : public ScoreSetListType, public TSynchronizable {
public:
	ScoreSets &operator= (const ScoreSets &src);
	void Serialize (CArchive& sArchive);   // read/write the object
};

// -------------------------------------------------------------------------
class TScoringDlg : public CDialog
{
public:
	TScoringDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SCORING };
	CButton	m_sOK;
	CButton	m_sCancel;
	CButton	m_sAddWord;
	CButton	m_sAddGroup;
	CListCtrl	m_sWords;
	CButton	m_sEdit;
	CButton	m_sDelete;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnItemchangedWords(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkWords(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditWords(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddGroup();
	afx_msg void OnAddWord();
	afx_msg void OnKeydownWords(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	DECLARE_MESSAGE_MAP()

	void GrayControls ();
	void SetDirty () { m_bDirty = TRUE; }
	BOOL IsDirty () { return m_bDirty; }
	void GetScoreSetAndIndex (int iStartIndex, int &iScoreSetIndex,
		ScoreSet *&psScoreSet);
	BOOL EditOK ();
	void SizeControls ();

	BOOL m_bDirty;                // has the data been changed?
	ScoreSets m_sScoreSets;       // local copy of score sets
};

// -------------------------------------------------------------------------
class TScoreEdit : public CDialog
{
public:
	TScoreEdit(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SCORING_EDIT };
	CButton	m_sOK;
	CEdit	m_sExpirationDays;
	CEdit	m_sPhrase;
	CString	m_strPhrase;
	long	m_lScore;
	int		m_iWhere;
	int		m_iType;
	BOOL	m_bWholeWord;
	int		m_iExpiration;
	UINT	m_iExpirationDays;

	BOOL m_bMultiple;
	static long s_lLastScore;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnExpirationNone();
	afx_msg void OnExpires2();
	afx_msg void OnChangePhrase();
	DECLARE_MESSAGE_MAP()

	void GrayControls ();

	bool m_fTestCompilePhrase;
};

// -------------------------------------------------------------------------
class TScoreEditGroup : public CDialog
{
public:
	TScoreEditGroup(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SCORING_EDIT_GROUP };
	CString	m_strGroup;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};

// -------------------------------------------------------------------------
class TQuickScore : public CDialog
{
public:
	TQuickScore(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SCORING_QUICK };
	CEdit	m_sExpirationDays;
	CComboBox	m_sScore;
	CButton	m_sOK;
	CString	m_strPhrase;
	BOOL	m_bRescore;
	CString	m_strScore;
	BOOL	m_bWholeWord;
	int		m_iWhere;
	int		m_iType;
	UINT	m_iExpirationDays;
	int		m_iExpiration;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePhrase();
	afx_msg void OnMainScoring();
	afx_msg void OnExpirationNone();
	afx_msg void OnExpires2();

	DECLARE_MESSAGE_MAP()

	void GrayControls ();

	BOOL m_bOriginalRescore;
	CString m_rstrMRU [5];        // MRU scores for combobox
};

// -------------------------------------------------------------------------

#define SCORE_COLOR_VERSION 1

class ScoreColor : public PObject {
public:
	ScoreColor ();
	void Serialize (CArchive& sArchive);

	BOOL bEnabled;
	long lFrom;
	long lTo;
	COLORREF rgbColor;
};

extern ScoreColor grScoreColors [5];

// -------------------------------------------------------------------------
class TScoreColors : public CDialog
{
public:
	TScoreColors(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SCORE_COLORS };
	CEdit	m_sTo5;
	CEdit	m_sTo4;
	CEdit	m_sTo3;
	CEdit	m_sTo2;
	CEdit	m_sTo1;
	CEdit	m_sFrom5;
	CEdit	m_sFrom4;
	CEdit	m_sFrom3;
	CEdit	m_sFrom2;
	CEdit	m_sFrom1;
	CButton	m_sColor5;
	CButton	m_sColor4;
	CButton	m_sColor3;
	CButton	m_sColor2;
	CButton	m_sColor1;
	long	m_lTo1;
	long	m_lFrom1;
	long	m_lFrom2;
	long	m_lFrom3;
	long	m_lFrom4;
	long	m_lFrom5;
	long	m_lTo2;
	long	m_lTo3;
	long	m_lTo4;
	long	m_lTo5;
	BOOL	m_bEnabled1;
	BOOL	m_bEnabled2;
	BOOL	m_bEnabled3;
	BOOL	m_bEnabled4;
	BOOL	m_bEnabled5;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnColor1();
	afx_msg void OnColor2();
	afx_msg void OnColor3();
	afx_msg void OnColor4();
	afx_msg void OnColor5();
	afx_msg void OnEnabled1();
	afx_msg void OnEnabled2();
	afx_msg void OnEnabled3();
	afx_msg void OnEnabled4();
	afx_msg void OnEnabled5();
	DECLARE_MESSAGE_MAP()

	void OnColor (int iIndex);
	void GrayControls ();

	ScoreColor m_rScoreColors [5];
};

// -------------------------------------------------------------------------
int ScoreHeader(TNewsGroup     * pNG, TArticleHeader * pHdr);
int ScoreBody (TArticleHeader *pHdr, TArticleText *pBody, TNewsGroup *pNG);
void RescoreCurrentGroup ();
void ScoreSetBackground (DWORD &dwColor, long lScore);
