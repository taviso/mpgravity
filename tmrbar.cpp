/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tmrbar.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:10  richard_wood
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

// tmrbar.cpp -- manual-rule dialog bar

#include "stdafx.h"              // windows stuff
#include "tmrbar.h"              // this file's prototypes
#include "pobject.h"             // for rules.h
#include "mplib.h"               // for rules.h
#include "article.h"             // for rules.h
#include "newsgrp.h"             // for rules.h
#include "rules.h"               // Rule, ...
#include "newsview.h"            // CNewsView
#include "genutil.h"             // GetNewsView()
#include "tmanrule.h"            // TManualRule
#include "newsdb.h"              // TRuleIterator
#include "server.h"              // TNewsServer
#include "thrdlvw.h"             // TThreadListView

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// global pointer to the one manual-rule-bar
TManualRuleBar *gpsManualRuleBar;

TManualRuleBar::TManualRuleBar()
	: CDialogBar ()
{
}

void TManualRuleBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DIALOGBAR_RUN_RULE, m_sDialogBarRunRule);
	DDX_Control(pDX, IDC_DIALOGBAR_RULES, m_sDialogBarRules);
}

BEGIN_MESSAGE_MAP(TManualRuleBar, CDialogBar)
	ON_BN_CLICKED(IDC_DIALOGBAR_RUN_RULE, OnDialogbarRunRule)
	ON_UPDATE_COMMAND_UI(IDC_DIALOGBAR_RUN_RULE, OnUpdateRunRule)
END_MESSAGE_MAP()

void TManualRuleBar::UpdateRuleList ()
{
	m_sDialogBarRules.ResetContent ();

	TRuleIterator it (ReadLock);
	Rule *psRule;
	int iFoundSome = FALSE;
	while ((psRule = it.Next ()) != 0) {
		iFoundSome = TRUE;
		m_sDialogBarRules.AddString (psRule -> strRuleName);
	}

	if (iFoundSome)
		m_sDialogBarRules.SetCurSel (0);
}

void TManualRuleBar::Initialize ()
{
	UpdateData (FALSE);
}

void TManualRuleBar::OnUpdateRunRule(CCmdUI* pCmdUI)
{
	// if there is no rule selected, or if there is no newsgroup loaded, disable
	// the button
	CString str; m_sDialogBarRules.GetWindowText (str);
	CNewsView *psView = GetNewsView ();
	pCmdUI -> Enable (str != "" && psView && psView -> IsNewsgroupDisplayed ());
}

void TManualRuleBar::OnDialogbarRunRule()
{
	ASSERT (GetNewsView ());
	LONG lGroupID = GetNewsView () -> GetCurNewsGroupID ();
	ASSERT (lGroupID);

	int iIndex = m_sDialogBarRules.GetCurSel ();
	ASSERT (iIndex >= 0);
	CString strRule;
	m_sDialogBarRules.GetLBText (iIndex, strRule);

	Rules *psRules = GetGlobalRules ();
	psRules -> ReadLock ();
	Rule *psRule;
	BOOL RC = psRules -> Lookup (strRule, psRule);
	ASSERT (RC);
	int iRuleID = psRule -> GetID ();
	psRules -> UnlockRead ();

	// if two or more articles are selected, apply the rule to selected articles only
	TThreadListView *pThreadView = GetThreadView ();
	ASSERT (pThreadView);

	TManualRule sManualRuleDlg (this, lGroupID, FALSE /* bSelGroups */, iRuleID,
		TRUE /* bRunStraightThrough */, TRUE /* bShowDialog */,
		pThreadView -> GetSelCount () < 2 /* bAllArticles */);
	sManualRuleDlg.DoModal();
}
