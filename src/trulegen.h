/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: trulegen.h,v $
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

// trulegen.h -- a rule's general properties page

#pragma once

class TNewsServer;
class Rule;

class TRuleGeneral : public CPropertyPage
{
public:
	TRuleGeneral ();
	void SetRule (Rule *psRule) { m_psRule = psRule; }
	BOOL m_bDirty;

	enum { IDD = IDD_RULE_GENERAL };
	CEdit	m_sExpirationDays;
	CEdit	m_sWildGroups;
	CListBox	m_sRuleGroupsNot;
	CListBox	m_sRuleGroups;
	CButton	m_sDelGroup;
	CButton	m_sAddGroup;
	CString	m_strWildGroups;
	int		m_iAllGroups;
	UINT	m_iExpirationDays;
	int		m_iExpiration;

public:
	virtual void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnRuleAddGroup();
	afx_msg void OnRuleDelGroup();
	afx_msg void OnDblclkRuleGroups();
	afx_msg void OnDblclkRuleGroupsNot();
	afx_msg void OnAllGroups();
	afx_msg void OnSpecificNewsgroups();
	afx_msg void OnDestroy();
	afx_msg void OnWild();
	afx_msg void OnChangeWildGroups();
	afx_msg void OnExpirationNone();
	afx_msg void OnExpires();
	afx_msg void OnChangeExpirationDays();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void DisplayCurrentRule ();
	int UpdateRule ();
	void UpdateGreyState ();

	Rule *m_psRule;

	TNewsServer *m_pNewsServer;
};
