/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tmanrule.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

// tmanrule.h -- dialog to apply a rule manually

#pragma once

#include <afxtempl.h>         // template collection classes, for article.h
#include <afxcmn.h>           // CProgressCtrl
#include "resource.h"         // IDD_MANUAL_RULE, ...
#include "article.h"          // TArticleHeader
#include "newsgrp.h"          // TNewsGroup
#include "rules.h"            // Rule
#include "idxlst.h"           // TArticleIndexList

class TNewsServer;

// parameters for GetNewGroup ()
#define ALL_ARTICLES 1
#define SELECTED_ARTICLES 2

typedef CMap <CString,LPCTSTR,TThreadList*,TThreadList*&>
CMapNamesToThreadLists;

// -------------------------------------------------------------------------
class TGroupCheckListBox : public CCheckListBox
{
public:
	TGroupCheckListBox() {};
	virtual ~TGroupCheckListBox() {};
	int CompareItem (LPCOMPAREITEMSTRUCT psCompare);

protected:
	DECLARE_MESSAGE_MAP()
};

// -------------------------------------------------------------------------
class TManualRule : public CDialog
{
public:
	TManualRule(CWnd* pParent = NULL, LONG lGroupID = 0, BOOL bSelGroups = FALSE,
		int iInitialRuleID = -1,  int bRunStraightThrough = FALSE,
		BOOL bShowDialog = TRUE, BOOL bAllArticles = TRUE,
		BOOL bRefreshWhenDone = TRUE, CPtrList *pHeaders = NULL);

	void DoRefreshNewsgroup ();

	enum { IDD = IDD_MANUAL_RULE };
	TGroupCheckListBox	m_sGroups;
	CButton	m_sAllArticles;
	CButton	m_sCancel;
	CButton	m_sStop;
	CButton	m_sStart;
	CButton	m_sReset;
	CComboBox	m_sRule;
	CProgressCtrl	m_sProgress;
	CString	m_strCount;
	CString	m_strActions;
	CString	m_strArticle;
	CString	m_strFirings;
	CString	m_strGroup;
	int		m_iAllArticles;
	CButton	m_sSelectedArticles;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnReset();
	afx_msg void OnSelchangeRule();
	afx_msg void OnStart();
	afx_msg void OnStop();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnAllArticles();
	afx_msg void OnSelectedArticles();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeGroups();
	DECLARE_MESSAGE_MAP()

private:
	void UpdateInfoFields (BOOL fImmediate = FALSE);
	void DDX_InfoFields (CDataExchange * pDX);
	void UpdateGreyState (BOOL bStopGrey);
	void FillRuleList ();
	void FillNewsgroupList ();
	void GrayGroupControls ();

	int m_iTotalArticles;
	int m_iArticle;
	int m_iActions;
	int m_iFirings;
	TArticleHeader *m_psHeader;
	LONG m_lInitialGroupID;          // ID of newsgroup, passed to constructor
	BOOL m_bRunning;                 // TRUE if running
	BOOL m_bStop;                    // set to TRUE to stop running
	BOOL m_bDisplayedGroupDirty;     // have we dirtied the displayed group?
	LONG m_lDisplayedGroup;          // group ID of currently-displayed group
	static CString s_strLastRule;    // last rule that was invoked
	int m_bRunStraightThrough;       // run immediately?
	int m_iInitialRuleID;            // ID of initial rule to set to
	BOOL m_bAllArticles;             // process all articles
	UINT m_iStartTimer;              // used for delayed starting from rule bar
	UINT m_iDrawTimer;               // used to update text fields when running
	BOOL m_bShowDialog;              // used when running straight through
	BOOL m_bRefreshWhenDone;         // used if current group is changed
	CPtrList *m_pHeaders;            // headers to iterate through (if non-null)
	POSITION m_iPos;                 // current position within m_pHeaders
	BOOL m_bSelGroups;               // read selected groups from newsgroup pane?
	TNewsServer *m_pNewsServer;      // our ptr to news server object
};
