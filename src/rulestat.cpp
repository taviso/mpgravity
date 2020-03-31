/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rulestat.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.2  2008/09/19 14:51:48  richard_wood
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

// rulestat.cpp -- rule-statistics dialog

#include "stdafx.h"              // precompiled header
#include "resource.h"            // wanted by rulestat.h
#include "rulestat.h"            // this file's prototypes
#include "article.h"             // for rules.h
#include "newsgrp.h"             // for rules.h
#include "rules.h"               // GetGlobalRules()
#include "server.h"              // for newsdb.h
#include "newsdb.h"              // TRuleIterator

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TRuleStats::TRuleStats(CWnd* pParent /*=NULL*/)
	: CDialog(TRuleStats::IDD, pParent)
{
}

// -------------------------------------------------------------------------
void TRuleStats::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);



   // do this one manually because it's a COwnerDrawListView
	DDX_Control(pDX, IDC_STATS, m_sStats);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleStats, CDialog)
		ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_STATS, OnColumnclickStats)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TRuleStats::OnInitDialog() 
{
	CDialog::OnInitDialog();
   InitializeList ();
   m_iSortBy = 3;    // sort by seconds
   FillList ();
	return TRUE;
}

// -------------------------------------------------------------------------
void TRuleStats::InitializeList ()
{
   RECT rct;
   m_sStats.GetClientRect (&rct);
   int iWidth = rct.right - 240 - 16;  // - 16 to leave room for scroll bar

   CString str; str.LoadString (IDS_RULESTATS_NAME_COL);
   m_sStats.InsertColumn (0, str, LVCFMT_RIGHT, iWidth, 0);

   str.LoadString (IDS_RULESTATS_EVAL_COL);
   m_sStats.InsertColumn (1, str, LVCFMT_LEFT, 60, 1);

   str.LoadString (IDS_RULESTATS_FIRED_COL);
   m_sStats.InsertColumn (2, str, LVCFMT_LEFT, 60, 2);

   str.LoadString (IDS_RULESTATS_TIME_COL);
   m_sStats.InsertColumn (3, str, LVCFMT_LEFT, 60, 3);

   str.LoadString (IDS_RULESTATS_PERCENT_COL);
   m_sStats.InsertColumn (4, str, LVCFMT_LEFT, 60, 4);
}

// -------------------------------------------------------------------------
static int CALLBACK CompareItems (LPARAM lParam1, LPARAM lParam2,
   LPARAM lParamSort)
{
   int iColumn = (int) lParamSort;
   Rule *pRule1 = GetGlobalRules () -> RuleFromID (BYTE(lParam1));
   Rule *pRule2 = GetGlobalRules () -> RuleFromID (BYTE(lParam2));
   int iResult = 0;

   ASSERT (iColumn >= 0 && iColumn <= 4);
   switch (iColumn) {
      case 0:  // compare names
         iResult = pRule1 -> strRuleName.CompareNoCase (pRule2 -> strRuleName);
		 if (iResult > 1) iResult = 1;
		 if (iResult < -1) iResult = -1;
         break;
      case 1:  // compare times evaluated
         if (pRule1 -> m_iEvaluated > pRule2 -> m_iEvaluated)
            iResult = 1;
         else if (pRule1 -> m_iEvaluated < pRule2 -> m_iEvaluated)
            iResult = -1;
         break;
      case 2:  // compare times fired
         if (pRule1 -> m_iFired < pRule2 -> m_iFired)
            iResult = 1;
         else if (pRule1 -> m_iFired > pRule2 -> m_iFired)
            iResult = -1;
         break;
      case 3:  // compare ticks
      case 4:
         if (pRule1 -> m_dwTicks < pRule2 -> m_dwTicks)
            iResult = 1;
         else if (pRule1 -> m_dwTicks > pRule2 -> m_dwTicks)
            iResult = -1;
         break;
      }

   return iResult;
}

// -------------------------------------------------------------------------
void TRuleStats::FillList ()
{
   m_sStats.DeleteAllItems ();

   DWORD dwTotalTicks = 0;
   TRuleIterator it1 (ReadLock);
	Rule *pRule;
	while ((pRule = it1.Next ()) != 0)
      dwTotalTicks += pRule -> m_dwTicks;

   TRuleIterator it (ReadLock);
	while ((pRule = it.Next ()) != 0) {
      m_sStats.InsertItem (LVIF_TEXT | LVIF_PARAM, 0 /* index */,
         pRule -> strRuleName, 0, 0, 0, pRule -> GetID ());

      CString str;
      str.Format (_T("%d"), pRule -> m_iEvaluated);
      m_sStats.SetItemText (0 /* index */, 1, str);

      str.Format (_T("%d"), pRule -> m_iFired);
      m_sStats.SetItemText (0 /* index */, 2, str);

      str.Format (_T("%-13.3f"), (double) pRule -> m_dwTicks / 1000);
      m_sStats.SetItemText (0 /* index */, 3, str);

      double percent =
         (dwTotalTicks ? ((double) pRule -> m_dwTicks / dwTotalTicks) * 100 : 0);
      str.Format (_T("%-5.1f%%"), percent);
      m_sStats.SetItemText (0 /* index */, 4, str);
		}

   m_sStats.SortItems (CompareItems, m_iSortBy);
}

// -------------------------------------------------------------------------
void TRuleStats::OnClear() 
{
   Rules &rules = *GetGlobalRules ();
   rules.m_dwTicks = 0;

   TRuleIterator it (ReadLock);
	Rule *pRule;
	while ((pRule = it.Next ()) != 0) {
      pRule -> m_dwTicks = 0;
      pRule -> m_iEvaluated = pRule -> m_iFired = 0;
      }

   FillList ();
}

// -------------------------------------------------------------------------
void TRuleStats::OnRefresh() 
{
   FillList ();
}

// -------------------------------------------------------------------------
void TRuleStats::OnColumnclickStats(NMHDR* pNMHDR, LRESULT* pResult) 
{
   m_iSortBy = ((NM_LISTVIEW*) pNMHDR) -> iSubItem;
   FillList ();
	*pResult = 0;
}
