/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: trulegen.cpp,v $
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

// trulegen.cpp -- a rule's general properties page

#include "stdafx.h"              // Windows API, MFC, ...
#include "resource.h"            // ID*
#include "pobject.h"             // for rules.h
#include "mplib.h"               // for rules.h
#include "article.h"             // for rules.h
#include "newsgrp.h"             // for rules.h
#include "rules.h"               // Rule, ...
#include "trulegen.h"            // this file's prototypes
#include "nglist.h"              // TNewsGroupArray
#include "server.h"              // TNewsServer
#include "genutil.h"             // ListboxAddAdjustHScroll()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TRuleGeneral::TRuleGeneral(void)
: CPropertyPage (TRuleGeneral::IDD, 0)
{

	m_strWildGroups = _T("");
	m_iAllGroups = -1;
	m_iExpirationDays = 1;
	m_iExpiration = -1;


	m_pNewsServer = 0;
}

// -------------------------------------------------------------------------
void TRuleGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EXPIRATION_DAYS, m_sExpirationDays);
	DDX_Control(pDX, IDC_WILD_GROUPS, m_sWildGroups);
	DDX_Control(pDX, IDC_RULE_GROUPS_NOT, m_sRuleGroupsNot);
	DDX_Control(pDX, IDC_RULE_GROUPS, m_sRuleGroups);
	DDX_Control(pDX, IDC_RULE_DEL_GROUP, m_sDelGroup);
	DDX_Control(pDX, IDC_RULE_ADD_GROUP, m_sAddGroup);
	DDX_Text(pDX, IDC_WILD_GROUPS, m_strWildGroups);
	DDX_Radio(pDX, IDC_ALL_GROUPS, m_iAllGroups);
	DDX_Text(pDX, IDC_EXPIRATION_DAYS, m_iExpirationDays);
	DDV_MinMaxUInt(pDX, m_iExpirationDays, 1, 999);
	DDX_Radio(pDX, IDC_EXPIRATION_NONE, m_iExpiration);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleGeneral, CPropertyPage)
		ON_BN_CLICKED(IDC_RULE_ADD_GROUP, OnRuleAddGroup)
	ON_BN_CLICKED(IDC_RULE_DEL_GROUP, OnRuleDelGroup)
	ON_LBN_DBLCLK(IDC_RULE_GROUPS, OnDblclkRuleGroups)
	ON_LBN_DBLCLK(IDC_RULE_GROUPS_NOT, OnDblclkRuleGroupsNot)
	ON_BN_CLICKED(IDC_ALL_GROUPS, OnAllGroups)
	ON_BN_CLICKED(IDC_SPECIFIC_NEWSGROUPS, OnSpecificNewsgroups)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_WILD, OnWild)
	ON_EN_CHANGE(IDC_WILD_GROUPS, OnChangeWildGroups)
	ON_BN_CLICKED(IDC_EXPIRATION_NONE, OnExpirationNone)
	ON_BN_CLICKED(IDC_EXPIRES, OnExpires)
	ON_EN_CHANGE(IDC_EXPIRATION_DAYS, OnChangeExpirationDays)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TRuleGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// lock the server
	m_pNewsServer = GetCountedActiveServer ();
	m_pNewsServer->AddRef ();

	DisplayCurrentRule ();
	m_bDirty = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
// DisplayCurrentRule -- takes the current rule name from the rule listbox, and
// displays that rule in the dialog box's data fields
void TRuleGeneral::DisplayCurrentRule ()
{
	// all-groups / specific-groups radio buttons
	if (m_psRule -> bAllGroups)
		m_iAllGroups = 0;
	else if (m_psRule -> bWildGroups)
		m_iAllGroups = 1;
	else
		m_iAllGroups = 2;

	m_strWildGroups = m_psRule -> strWildGroups;

	// display the rule's subscribed/unsubscribed newsgroups.  Clear the listbox
	// first
	m_sRuleGroups.ResetContent ();
	m_sRuleGroupsNot.ResetContent ();

	// first, add all groups in m_psRule -> rstrGroups, then add the rest of
	// the subscribed groups into the m_sRuleGroupsNot listbox

	int iSize = m_psRule -> rstrGroups.GetSize ();
	int i = 0;
	for (i = 0; i < iSize; i++)
		ListboxAddAdjustHScroll (m_sRuleGroups, m_psRule -> rstrGroups [i]);

	TNewsGroupArray& vNewsGroups = m_pNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vNewsGroups);

	int iTotal = vNewsGroups -> GetSize();
	for (i = 0; i < iTotal; ++i) {
		TNewsGroup* pGrp = vNewsGroups[i];
		const CString &strName = pGrp -> GetName ();
		// add this group either to the subscribed-groups or the
		// unsubscribed-groups listbox
		// TODO: This is inefficient since it uses the inefficient Array search.
		// A better way is to add the subscribed groups to the subscribed-groups
		// listbox then use that listbox's search function (which should be fast
		// since the listbox is sorted) while adding elements to the
		// unsubscribed-groups listbox
		if (CStringArrayFind (m_psRule -> rstrGroups, strName) == -1)
			ListboxAddAdjustHScroll (m_sRuleGroupsNot, strName);
	}

	// expiration
	m_iExpiration = m_psRule -> iExpirationType == Rule::NO_EXPIRATION ? 0 : 1;
	m_iExpirationDays = (int) m_psRule -> iExpirationDays;

	UpdateData (FALSE /* bSaveAndValidate */);
	UpdateGreyState ();
}

// -------------------------------------------------------------------------
// UpdateRule -- takes a rule, and updates it with this dialog's displayed
// values
int TRuleGeneral::UpdateRule ()
{
	UpdateData ();

	m_psRule -> bAllGroups = m_iAllGroups == 0;
	m_psRule -> bWildGroups = m_iAllGroups == 1;
	if (m_strWildGroups.IsEmpty ())
		m_strWildGroups = "*";
	m_psRule -> strWildGroups = m_strWildGroups;

	// replace the rule's group-list with the one displayed, which may be
	// different
	m_psRule -> rstrGroups.RemoveAll();
	CString strGroupName;      // group name from listbox
	for (int i = 0; i < m_sRuleGroups.GetCount(); i++) {
		m_sRuleGroups.GetText (i, strGroupName);
		m_psRule -> rstrGroups.Add (strGroupName);
	}

	// expiration
	m_psRule -> iExpirationType =
		m_iExpiration == 1 ? Rule::DAYS_AFTER_USED : Rule::NO_EXPIRATION;
	m_psRule -> iExpirationDays = (USHORT) m_iExpirationDays;

	return 0;
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnRuleAddGroup()
{
	// remove selected groups from the unsubscribed-groups listbox and add
	// them to the subscribed-groups listbox
	for (int i = 0; i < m_sRuleGroupsNot.GetCount (); i++) {
		if (m_sRuleGroupsNot.GetSel (i) > 0) {
			m_bDirty = TRUE;
			CString strGroupName;
			m_sRuleGroupsNot.GetText (i, strGroupName);
			m_sRuleGroupsNot.DeleteString (i);
			ListboxAddAdjustHScroll (m_sRuleGroups, strGroupName);
			i--;  // because an item was removed from the listbox
		}
	}
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnRuleDelGroup()
{
	// remove selected groups from the subscribed-groups listbox and add
	// them to the unsubscribed-groups listbox
	for (int i = 0; i < m_sRuleGroups.GetCount (); i++) {
		if (m_sRuleGroups.GetSel (i) > 0) {
			m_bDirty = TRUE;
			CString strGroupName;
			m_sRuleGroups.GetText (i, strGroupName);
			m_sRuleGroups.DeleteString (i);
			ListboxAddAdjustHScroll (m_sRuleGroupsNot, strGroupName);
			i--;  // because an item was removed from the listbox
		}
	}
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnDblclkRuleGroups()
{
	OnRuleDelGroup ();
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnDblclkRuleGroupsNot()
{
	OnRuleAddGroup ();
}

// -------------------------------------------------------------------------
// OnAllGroups -- called when "All Groups" radio button is selected
void TRuleGeneral::OnAllGroups()
{
	m_bDirty = TRUE;
	UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnChangeWildGroups()
{
	m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
// OnSpecificNewsgroups -- called when "Specific Groups" radio button is selected
void TRuleGeneral::OnSpecificNewsgroups()
{
	m_bDirty = TRUE;
	UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnWild()
{
	m_bDirty = TRUE;
	UpdateGreyState ();
}

// -------------------------------------------------------------------------
// UpdateGreyState -- updates grey state for all the page's fields
void TRuleGeneral::UpdateGreyState()
{
	UpdateData ();
	m_sWildGroups.EnableWindow (m_iAllGroups == 1);
	BOOL bSpecific = m_iAllGroups == 2;
	m_sRuleGroupsNot.EnableWindow (bSpecific);
	m_sRuleGroups.EnableWindow (bSpecific);
	m_sAddGroup.EnableWindow (bSpecific);
	m_sDelGroup.EnableWindow (bSpecific);
	m_sExpirationDays.EnableWindow (m_iExpiration == 1);
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnOK()
{
	if (m_bDirty)
		UpdateRule ();

	CPropertyPage::OnOK();
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnDestroy()
{
	CPropertyPage::OnDestroy();

	// free server
	m_pNewsServer->Release ();
	m_pNewsServer = 0;
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnExpirationNone()
{
	m_bDirty = TRUE;
	UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnExpires()
{
	m_bDirty = TRUE;
	UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRuleGeneral::OnChangeExpirationDays()
{
	m_bDirty = TRUE;
}

BOOL TRuleGeneral::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TRuleGeneral::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
