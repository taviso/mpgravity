/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ruleutil.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:49  richard_wood
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

// ruleutil.h -- utilities for rule processing

#pragma once

#include "article.h"          // TArticleHeader, TArticleText
#include "newsgrp.h"          // TNewsGroup
#include "trulecon.h"         // Condition

enum AddNodeType {ADD_COND_FROM, ADD_COND_SUBJECT, ADD_COND_BODY,
ADD_COND_HEADER};

void ForwardArticle (CString &strAddresses, TArticleHeader* pArtHdr,
					 TArticleText* pArtText, TNewsGroup *pNG);
void SaveToFileAction (TArticleHeader* pArtHdr, TArticleText* pArtText,
					   CString &strFilename, BOOL bSeparator = TRUE, int iFullHeader = -1);
Rule *GetRule (LPCTSTR pchName);
int GetRuleIndex (LPCTSTR pchName);
void AddCondToStringList (Rule *pRule, AddNodeType iNodeType,
						  CStringList &rstr);
void AddCondToStringList (Condition *psCondition, AddNodeType iNodeType,
						  CStringList &rstr);
void AddStringListToCond (CStringList &rstr, CStringArray &rstrCondition,
						  AddNodeType iNodeType);
BOOL InCStringList (CStringList &rstr, CString &str);
BOOL EnabledRulesExist ();
int DoRuleSubstitution (Rule &sRule, TArticleHeader *psHeader,
						TNewsGroup *psGroup, const char *pchExistingString);
BOOL RuleMayRequireBody (Rule &sRule);
void TagArticle (TNewsGroup *pNG, TArticleHeader *pHeader);

/////////////////////////////////////////////////////////////////////////////
// TRuleTextSubstitution dialog
class TRuleTextSubstitution : public CDialog
{
public:
	TRuleTextSubstitution(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_RULE_TEXT_SUBSTITUTION };
	CEdit	m_sSubstituteWith;
	CButton	m_sOK;
	CString	m_strSubstituteWith;
	CString	m_strRuleName;
	CString	m_strGroupName;
	CString	m_strExistingString;
	CString	m_strArticleSubject;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSubstituteWith();
	DECLARE_MESSAGE_MAP()
};
