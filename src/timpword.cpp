/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: timpword.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:08  richard_wood
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

// timpword.cpp : implementation file

#include "stdafx.h"              // windows API
#include "timpword.h"            // this file's prototypes
#include "genutil.h"             // DeleteSelected()
#include "ruleutil.h"            // GetRule(), ...
#include "rules.h"               // Rule
#include "resource.h"            // IDS_OUT_OF_MEMORY
#include "tmrbar.h"              // gpsManualRuleBar
#include "newsdb.h"              // TNewsDB
#include "server.h"              // TNewsServer

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TImpWord::TImpWord(CWnd* pParent /*=NULL*/) : CDialog(TImpWord::IDD, pParent)
{

	m_strWords = _T("");
}

// -------------------------------------------------------------------------
void TImpWord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ADD, m_sAdd);
	DDX_Control(pDX, IDC_DELETE, m_sDelete);
	DDX_Control(pDX, IDC_WORDS, m_sWords);
	DDX_Control(pDX, IDC_WORD, m_sWord);

	DDX_LBStringList(pDX, IDC_WORDS, m_rstrWords);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TImpWord, CDialog)
		ON_LBN_SELCHANGE(IDC_WORDS, OnSelchangeWords)
	ON_EN_CHANGE(IDC_WORD, OnChangeWord)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TImpWord::OnAdd()
{
   // add the contents of the new-phrase edit field to the From-phrases listbox
   char rchLine [100];
   rchLine [0] = 0;
   m_sWord.GetLine (0, rchLine, sizeof (rchLine) - 1);
   rchLine [m_sWord.LineLength ()] = 0;
   m_sWords.AddString (rchLine);

   // empty the edit box
   rchLine [0] = 0;
   m_sWord.SetWindowText(rchLine);
}

// -------------------------------------------------------------------------
void TImpWord::OnDelete()
{
   // delete whatever items are selected in the word list
   DeleteSelected (&m_sWords);

   // update the grey state
   OnSelchangeWords ();
}

// -------------------------------------------------------------------------
void TImpWord::OnChangeWord()
{
   // if the new-word edit field contains something, enable the "add" buttons
   // and make it the default... otherwise, disable it and make the OK button
   // the default
   BOOL bEnable = m_sWord.LineLength ();

   m_sAdd.EnableWindow (bEnable);
   SetDefID (bEnable ? IDC_ADD : IDOK);
}

// -------------------------------------------------------------------------
BOOL TImpWord::OnInitDialog()
{
   // load the contents of the rule into m_rstrWords before DDX occurs
   GetGlobalRules () -> ReadLock ();
   CString strRule; strRule.LoadString (IDS_IMP_WORD_RULE_NAME);
   Rule *pRule = GetRule (strRule);
   if (pRule) {
      AddCondToStringList (pRule, ADD_COND_HEADER, m_rstrWords);
      AddCondToStringList (pRule, ADD_COND_BODY, m_rstrWords);
      }
   GetGlobalRules () -> UnlockRead ();

	CDialog::OnInitDialog();

   // set initial enabled state
   OnChangeWord ();
   OnSelchangeWords ();

   // put focus in the edit box
   m_sWord.SetFocus ();

	return FALSE; // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
// GetWordRule -- gets the imp-word rule or a new rule if no imp-word rule
// exists.  Assumes the global rules are already read-locked
static Rule *GetWordRule ()
{
   CString strRule; strRule.LoadString (IDS_IMP_WORD_RULE_NAME);
   Rule *pRule = GetRule (strRule);
   if (!pRule) {
      pRule = new Rule;
      if (!pRule) {
         MsgResource (IDS_ERR_OUT_OF_MEMORY);
         return NULL;
         }
      pRule -> strRuleName = strRule;
      pRule -> bImportantEnable = TRUE;
      pRule -> bImportant = TRUE;

      // default to enabled
      pRule -> bEnabled = TRUE;

      GetGlobalRules () -> SetAt (pRule -> strRuleName, pRule);

      // also update the rule list in the manual-rule dialog bar
      gpsManualRuleBar -> UpdateRuleList ();
      }

   return pRule;
}

// -------------------------------------------------------------------------
void TImpWord::OnOK()
{
	CDialog::OnOK();  // performs DDX

   GetGlobalRules () -> WriteLock ();

   Rule *pRule = GetWordRule ();
   if (!pRule) {
      GetGlobalRules () -> UnlockWrite ();
      return;
      }
   pRule -> rstrCondition.RemoveAll ();
   FreeCondition (pRule -> psCondition);
   AddStringListToCond (m_rstrWords, pRule -> rstrCondition, ADD_COND_HEADER);
   AddStringListToCond (m_rstrWords, pRule -> rstrCondition, ADD_COND_BODY);

   // if the condition is empty (last word was removed), remove the rule
   if (m_rstrWords.IsEmpty ()) {
      GetGlobalRules () -> RemoveKey (pRule -> strRuleName);
      delete pRule;
      }

   GetGlobalRules () -> UnlockWrite ();

   // save the rule-set
   gpStore -> SaveRules ();

   // also update the rule list in the manual-rule dialog bar
   gpsManualRuleBar -> UpdateRuleList ();
}

// -------------------------------------------------------------------------
void TImpWord::OnSelchangeWords ()
{
   // if there are any selected items, enable the "delete" button
   m_sDelete.EnableWindow (m_sWords.GetSelCount ());
}

// -------------------------------------------------------------------------
void TImpWord::AddImportantWord (const char *pchWord)
{
   GetGlobalRules () -> WriteLock ();

   Rule *pRule = GetWordRule ();
   if (!pRule) {
      GetGlobalRules () -> UnlockWrite ();
      return;
      }

   CStringList rstr;
   AddCondToStringList (pRule, ADD_COND_HEADER, rstr);
   AddCondToStringList (pRule, ADD_COND_BODY, rstr);
   rstr.AddTail (pchWord);

   pRule -> rstrCondition.RemoveAll ();
   FreeCondition (pRule -> psCondition);
   AddStringListToCond (rstr, pRule -> rstrCondition, ADD_COND_HEADER);
   AddStringListToCond (rstr, pRule -> rstrCondition, ADD_COND_BODY);

   GetGlobalRules () -> UnlockWrite ();

   // save the rule-set
   gpStore->SaveRules ();
}
