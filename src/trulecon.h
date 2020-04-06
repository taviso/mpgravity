/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: trulecon.h,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
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

// trulecon.h -- rule conditions

#pragma once

#include "tsearch.h"             // TSearch

#ifndef IDD_RULE_CONDITIONS
#include "resource.h"
#endif

// -------------------------------------------------------------------------
class TRuleConditions : public CPropertyPage
{
public:
	TRuleConditions (LPCTSTR pchStaticCondition = NULL);
	~TRuleConditions ();
	void SetRule (Rule *psRule) { m_psRule = psRule; }
	BOOL m_bDirty;

	enum { IDD = IDD_RULE_CONDITIONS };
	CEdit	m_sScore;
	CEdit	m_sDaysAgo;
	CComboBox	m_sAttribute;
	CComboBox	m_sListName;
	CEdit	m_sGroups;
	CButton	m_sRE;
	CEdit	m_sLines;
	CEdit	m_sPhrase;
	CEdit	m_sCrossGroup;
	CString m_strCrossGroup;
	CComboBox	m_sYesNo;
	CComboBox	m_sArticleField;
	long	m_lLines;
	int		m_iYesNo;
	CString	m_strPhrase;
	int		m_iArticleField;
	int		m_iRadio;
	BOOL	m_bRE;
	int		m_iGroups;
	int		m_iListName;
	int		m_iAttribute;
	UINT	m_iDaysAgo;
	long	m_lScore;
	CEdit	m_sStaticCondition;
	CString	m_strStaticCondition;

public:
	virtual BOOL OnKillActive();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnAddRuleCondition();
	afx_msg void OnFieldContains();
	afx_msg void OnInReplyCondition();
	afx_msg void OnRuleAnd();
	afx_msg void OnRuleConditionDelete();
	afx_msg void OnRuleLparen();
	afx_msg void OnRuleMoveDown();
	afx_msg void OnRuleMoveUp();
	afx_msg void OnRuleNot();
	afx_msg void OnRuleOr();
	afx_msg void OnRuleRparen();
	afx_msg void OnLinesMoreThan();
	afx_msg void OnCrossposted();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnInList();
	afx_msg void OnMarkedAs();
	afx_msg void OnCrosspostedGroup();
	afx_msg void OnPostTime();
	afx_msg void OnRuleImportCond();
	afx_msg void OnScoreMoreThan();
	afx_msg void OnDestroy();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	afx_msg void OnCondKillFocus ();
	afx_msg void OnCondSetFocus ();
	afx_msg void OnCondChange ();

	void doCondChange ();
	void InsertToken (const char * pchToken);
	void UpdateGreyStates ();
	int CheckRadioButton (CString& strItem, char * pchDescription);
	void CheckIndentation ();
	void DisplayCurrentRule ();
	int UpdateRule ();
	void OnOK ();

	Rule *          m_psRule;
	int             m_bEmpty;
	CRichEditCtrl   m_sCondition;
	bool            m_fInCondChange;
};

#define NOT_NODE                    1
#define AND_NODE                    2
#define OR_NODE                     3
#define IN_REPLY_NODE               4
#define SUBJECT_CONTAINS_NODE       5
#define FROM_CONTAINS_NODE          6
#define HEADER_CONTAINS_NODE        7
#define BODY_CONTAINS_NODE          8
#define SUBJECT_CONTAINS_NOT_NODE   9
#define FROM_CONTAINS_NOT_NODE      10
#define HEADER_CONTAINS_NOT_NODE    11
#define BODY_CONTAINS_NOT_NODE      12
#define LPAREN_TOKEN                13
#define RPAREN_TOKEN                14
#define LINES_MORE_THAN             15
#define CROSSPOSTED_NODE            16
#define IN_WATCH_LIST_NODE          17
#define IN_IGNORE_LIST_NODE         18
#define POSTED_DAYS_NODE            19
#define MARKED_AS_UNREAD            20
#define MARKED_AS_READ              21
#define MARKED_AS_IMPORTANT         22
#define MARKED_AS_NORMAL            23
#define MARKED_AS_PROTECTED         24
#define MARKED_AS_DELETABLE         25
#define MARKED_AS_WATCHED           26
#define MARKED_AS_IGNORED           27
#define MARKED_AS_DECODE_Q          28
#define MARKED_AS_DECODED           29
#define MARKED_AS_DECODE_ERR        30
#define MARKED_AS_LOCAL             31
#define MARKED_AS_TAGGED            32
#define SCORE_MORE_THAN             33
#define CROSSPOSTED_GROUP_NODE      34

#define UNBALANCED_PARENS           1
#define UNEXPECTED_TOKEN            2

typedef struct Condition {
	int iNode;						// type of this node
	CString strPhrase;			// phrase to look for
	BOOL bRE;                  // use regular-expression search?
	TSearch sSearch;				// search object
	long lVal;                 // for conditions that involve an integer
	struct Condition * psLeft, * psRight;	// left and right children
} Condition;

void FreeCondition (Condition*& psCondition);
int ConstructACondition (const CStringArray &rstrArray, Condition *&psCondition,
						 int &iLineIndex);

// -------------------------------------------------------------------------
class TPickRule : public CDialog
{
public:
	TPickRule(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_RULE_PICK };
	CButton	m_sOK;
	CListBox	m_sRules;
	int m_iRule;                  // name of rule chosen

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeRules();
	afx_msg void OnDblclkRules();
	DECLARE_MESSAGE_MAP()

	void GreyButtons ();
};
