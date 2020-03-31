/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: trulecon.cpp,v $
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

// trulecon.cpp -- rule conditions

#include <stdlib.h>              // max, atoi()
#include <string.h>              // strlen()
#include "stdafx.h"              // Windows API, MFC, ...
#include "resource.h"            // ID*
#include "pobject.h"             // for rules.h
#include "mplib.h"               // for rules.h
#include "article.h"             // for rules.h
#include "newsgrp.h"             // for rules.h
#include "rules.h"               // Rule, ...
#include "trulecon.h"            // this file's prototypes
#include "genutil.h"             // MsgResource(), ...
#include "rgswit.h"              // TRegSwitch
#include "tglobopt.h"            // TGlobalOptions
#include "server.h"              // needed by newsdb.h
#include "newsdb.h"              // TRuleIterator

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

static int ConstructCondition (const CStringArray &rstrArray,
							   Condition *&psCondition, int &iLineIndex, int &iIndex);
static int ConstructTerm (const CStringArray &rstrArray,
						  Condition *&psCondition, int &iLineIndex, int &iIndex);
static int ConstructFactor (const CStringArray &rstrArray,
							Condition *&psCondition, int &iLineIndex, int &iIndex);
static int GetNextToken (const CStringArray &rstrArray, int &iLineIndex,
						 int &iIndex, CString &strPhrase, BOOL *pbRE = NULL);
static int RecognizeSearchString (const CStringArray &rstrArray,
								  int &iLineIndex, int &iIndex, int iPositive, int iNegative, BOOL *pbRE,
								  CString &strPhrase);
static int VerifyWord (const CStringArray &rstrArray, int &iLineIndex,
					   int &iIndex, int iID);
static int VerifyWord (const CStringArray &rstrArray, int &iLineIndex,
					   int &iIndex, LPCTSTR pchWord);
static int GetWord (const CStringArray &rstrArray, int &iLineIndex,
					int &iIndex, CString &strWord);

// -------------------------------------------------------------------------
// FreeCondition -- frees a condition tree
void FreeCondition (Condition*& psCondition)
{
	if (!psCondition)
		return;
	FreeCondition (psCondition -> psLeft);
	FreeCondition (psCondition -> psRight);
	delete psCondition;
	psCondition = NULL;
}

// -------------------------------------------------------------------------
// ConstructACondition -- top-level function, takes a CString array and returns
// a condition
int ConstructACondition (const CStringArray &rstrArray, Condition *&psCondition,
						 int &iLineIndex)
{
	iLineIndex = 0;
	int iIndex = 0;
	int iBad = ConstructCondition (rstrArray, psCondition, iLineIndex, iIndex);
	if (iBad) {
		FreeCondition (psCondition);
		return iBad;
	}
	CString strWord;
	if (!GetWord (rstrArray, iLineIndex, iIndex, strWord)) {
		// did not parse the whole condition
		FreeCondition (psCondition);
		return UNEXPECTED_TOKEN;
	}
	return 0;
}

// -------------------------------------------------------------------------
// recursive descent parsing of conditions uses the following grammar:
//  condition ::= term 'and' condition
//  condition ::= term
//  term ::= factor 'or' term
//  term ::= factor
//  factor ::= '(' condition ')'
//  factor ::= 'not' factor
//  factor ::= RESPONSE_TO_ME | BODY_CONTAINS | HEADER_CONTAINS | ...

// ConstructCondition -- construct a condition tree from a CString array
static int ConstructCondition (const CStringArray &rstrArray,
							   Condition *&psCondition, int &iLineIndex, int &iIndex)
{
	// special case -- if array is empty, return an empty condition and succeed
	if (!rstrArray.GetSize ()) {
		psCondition = NULL;
		return 0;
	}

	psCondition = NULL;
	Condition * psMyNode;
	int iBad = ConstructTerm (rstrArray, psMyNode, iLineIndex, iIndex);
	if (iBad) {
		FreeCondition (psMyNode);
		return iBad;
	}

	// if 'and' follows, read it
	CString strPhrase;
	int iTempLineIndex = iLineIndex;
	int iTempIndex = iIndex;
	int iToken = GetNextToken (rstrArray, iTempLineIndex, iTempIndex, strPhrase);
	if (iToken != AND_NODE) {
		// condition ::= term
		psCondition = psMyNode;
		return 0;
	}
	iIndex = iTempIndex;
	iLineIndex = iTempLineIndex;

	Condition * psMyNode2;
	iBad = ConstructCondition (rstrArray, psMyNode2, iLineIndex, iIndex);
	if (iBad) {
		FreeCondition (psMyNode);
		return iBad;
	}

	// condition ::= term 'and' condition
	psCondition = new Condition;
	psCondition -> iNode = AND_NODE;
	psCondition -> psLeft = psMyNode;
	psCondition -> psRight = psMyNode2;

	return 0;
}

// -------------------------------------------------------------------------
// ConstructTerm -- construct a term tree from a CString array
static int ConstructTerm (const CStringArray& rstrArray,
						  Condition*& psCondition, int &iLineIndex, int& iIndex)
{
	psCondition = NULL;
	Condition * psMyNode;
	int iBad = ConstructFactor (rstrArray, psMyNode, iLineIndex, iIndex);
	if (iBad) {
		FreeCondition (psMyNode);
		return iBad;
	}

	// if 'or' follows, read it
	CString strPhrase;
	int iTempLineIndex = iLineIndex;
	int iTempIndex = iIndex;
	int iToken = GetNextToken (rstrArray, iTempLineIndex, iTempIndex, strPhrase);
	if (iToken != OR_NODE) {
		// term ::= factor
		psCondition = psMyNode;
		return 0;
	}
	iIndex = iTempIndex;
	iLineIndex = iTempLineIndex;

	Condition * psMyNode2;
	iBad = ConstructTerm (rstrArray, psMyNode2, iLineIndex, iIndex);
	if (iBad) {
		FreeCondition (psMyNode);
		return iBad;
	}

	// term ::= factor 'or' term
	psCondition = new Condition;
	psCondition -> iNode = OR_NODE;
	psCondition -> psLeft = psMyNode;
	psCondition -> psRight = psMyNode2;

	return 0;
}

// -------------------------------------------------------------------------
// ConstructFactor -- construct a factor tree from a CString array
static int ConstructFactor (const CStringArray& rstrArray,
							Condition*& psCondition, int &iLineIndex, int& iIndex)
{
	psCondition = NULL;

	CString strPhrase;
	BOOL bRE = FALSE;
	int iToken = GetNextToken (rstrArray, iLineIndex, iIndex, strPhrase, &bRE);
	if (iToken == LPAREN_TOKEN) {
		// factor ::= '(' condition ')'
		int iBad = ConstructCondition (rstrArray, psCondition, iLineIndex,
			iIndex);
		if (iBad) {
			FreeCondition (psCondition);
			return iBad;
		}
		iToken = GetNextToken (rstrArray, iLineIndex, iIndex, strPhrase);
		if (iToken != RPAREN_TOKEN) {
			FreeCondition (psCondition);
			return UNBALANCED_PARENS;
		}
		return 0;
	}

	if (iToken == NOT_NODE) {
		// factor ::= 'not' factor
		Condition * psMyNode;
		int iBad = ConstructFactor (rstrArray, psMyNode, iLineIndex, iIndex);
		if (iBad) {
			FreeCondition (psMyNode);
			return iBad;
		}

		psCondition = new Condition;
		psCondition -> iNode = NOT_NODE;
		psCondition -> psLeft = psMyNode;
		psCondition -> psRight = NULL;
		return 0;
	}

	switch (iToken) {
	  case LINES_MORE_THAN:
	  case IN_REPLY_NODE:
	  case SUBJECT_CONTAINS_NODE:
	  case HEADER_CONTAINS_NODE:
	  case FROM_CONTAINS_NODE:
	  case BODY_CONTAINS_NODE:
	  case SUBJECT_CONTAINS_NOT_NODE:
	  case HEADER_CONTAINS_NOT_NODE:
	  case FROM_CONTAINS_NOT_NODE:
	  case BODY_CONTAINS_NOT_NODE:
	  case CROSSPOSTED_NODE:
	  case IN_WATCH_LIST_NODE:
	  case IN_IGNORE_LIST_NODE:
	  case MARKED_AS_UNREAD:
	  case MARKED_AS_READ:
	  case MARKED_AS_IMPORTANT:
	  case MARKED_AS_NORMAL:
	  case MARKED_AS_PROTECTED:
	  case MARKED_AS_DELETABLE:
	  case MARKED_AS_WATCHED:
	  case MARKED_AS_IGNORED:
	  case MARKED_AS_DECODE_Q:
	  case MARKED_AS_DECODED:
	  case MARKED_AS_DECODE_ERR:
	  case MARKED_AS_LOCAL:
	  case MARKED_AS_TAGGED:
	  case POSTED_DAYS_NODE:
	  case SCORE_MORE_THAN:
		  psCondition = new Condition;
		  psCondition -> iNode = iToken;
		  psCondition -> strPhrase = strPhrase;
		  psCondition -> psLeft = NULL;
		  psCondition -> psRight = NULL;
		  psCondition -> bRE = bRE;
		  if (iToken == LINES_MORE_THAN ||
			  iToken == CROSSPOSTED_NODE ||
			  iToken == POSTED_DAYS_NODE ||
			  iToken == SCORE_MORE_THAN)
			  psCondition -> lVal = atol (strPhrase);
		  break;
	  default:
		  return UNEXPECTED_TOKEN;
		  break;
	}

	return 0;
}

// -------------------------------------------------------------------------
// GetNextToken -- gets a token from a CString array.  Returns 0 for failure
// or a token symbol (see trulecon.h for symbol definitions)
static int GetNextToken (const CStringArray &rstrArray, int &iLineIndex,
						 int &iIndex, CString &strPhrase, BOOL *pbRE /* = NULL */)
{
	// if we're past the end of the array, fail
	if (rstrArray.GetSize() <= iLineIndex)
		return 0;

	CString strWord;
	if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
		return 0;

	CString str;

	str.LoadString (IDS_AND);
	if (!strWord.CompareNoCase (str))
		return AND_NODE;

	str.LoadString (IDS_OR);
	if (!strWord.CompareNoCase (str))
		return OR_NODE;

	str.LoadString (IDS_NOT);
	if (!strWord.CompareNoCase (str))
		return NOT_NODE;

	if (!strWord.CompareNoCase ("("))
		return LPAREN_TOKEN;

	if (!strWord.CompareNoCase (")"))
		return RPAREN_TOKEN;

	str.LoadString (IDS_IN);
	if (!strWord.CompareNoCase (str)) {
		if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
			return 0;

		str.LoadString (IDS_REPLY);
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_TO) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_MY) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_MESSAGE))
				return 0;
			return IN_REPLY_NODE;
		}

		str.LoadString (IDS_A);
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_WATCHED) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_THREAD))
				return 0;
			return IN_WATCH_LIST_NODE;
		}

		str.LoadString (IDS_AN);
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_IGNORED) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_THREAD))
				return 0;
			return IN_IGNORE_LIST_NODE;
		}
	}

	str.LoadString (IDS_LINES);
	if (!strWord.CompareNoCase (str)) {

		if (VerifyWord (rstrArray, iLineIndex, iIndex, ">"))
			return 0;

		// strPhrase returns the number of lines (as a string)
		if (GetWord (rstrArray, iLineIndex, iIndex, strPhrase))
			return 0;

		return LINES_MORE_THAN;
	}

	str.LoadString (IDS_SCORE);
	if (!strWord.CompareNoCase (str)) {

		if (VerifyWord (rstrArray, iLineIndex, iIndex, ">"))
			return 0;

		// strPhrase returns the score (as a string)
		if (GetWord (rstrArray, iLineIndex, iIndex, strPhrase))
			return 0;

		return SCORE_MORE_THAN;
	}

	// must check both "crossposted" and "cross-posted" because version 1.1 b1
	// switched to the "cross-posted" form (unwittingly) thus we must support
	// both word forms now
	str.LoadString (IDS_CROSSPOSTED);
	CString str1; str1.LoadString (IDS_CROSSPOSTED_NO_HYPHEN);
	if (!strWord.CompareNoCase (str) ||
		!strWord.CompareNoCase (str1)) {

			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_TO) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, ">"))
				return 0;

			// strPhrase returns the number of groups (as a string)
			if (GetWord (rstrArray, iLineIndex, iIndex, strPhrase))
				return 0;

			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_GROUPS))
				return 0;

			return CROSSPOSTED_NODE;
	}

	str.LoadString (IDS_POSTED);
	if (!strWord.CompareNoCase (str)) {

		if (VerifyWord (rstrArray, iLineIndex, iIndex, ">"))
			return 0;

		// strPhrase returns the number of groups (as a string)
		if (GetWord (rstrArray, iLineIndex, iIndex, strPhrase))
			return 0;

		if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_DAYS) ||
			VerifyWord (rstrArray, iLineIndex, iIndex, IDS_AGO))
			return 0;

		return POSTED_DAYS_NODE;
	}

	str.LoadString (IDS_MARKED);
	if (!strWord.CompareNoCase (str)) {

		if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_AS))
			return 0;

		if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
			return 0;

		str.LoadString (IDS_UNREAD);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_UNREAD;

		str.LoadString (IDS_READ);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_READ;

		str.LoadString (IDS_IMPORTANT);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_IMPORTANT;

		str.LoadString (IDS_NORMAL);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_NORMAL;

		str.LoadString (IDS_PROTECTED);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_PROTECTED;

		str.LoadString (IDS_DELETABLE);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_DELETABLE;

		str.LoadString (IDS_WATCHED);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_WATCHED;

		str.LoadString (IDS_IGNORED);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_IGNORED;

		str.LoadString (IDS_DECODED);
		if (!strWord.CompareNoCase (str))
			return MARKED_AS_DECODED;

		str.LoadString (IDS_HAVING); // "having a ..."
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_A))
				return 0;

			if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
				return 0;

			str.LoadString (IDS_LOCAL);
			if (!strWord.CompareNoCase (str)) {
				if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_BODY))
					return 0;
				return MARKED_AS_LOCAL;    // "having a local body"
			}

			str.LoadString (IDS_DECODE);
			if (!strWord.CompareNoCase (str)) {
				if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_ERROR))
					return 0;
				return MARKED_AS_DECODE_ERR;    // "having a decode error"
			}

			return 0;
		}

		str.LoadString (IDS_IN);   // "in decode queue"
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_DECODE) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_QUEUE))
				return 0;
			return MARKED_AS_DECODE_Q;
		}

		str.LoadString (IDS_TAGGED); // "tagged for download"
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_FOR) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_DOWNLOAD))
				return 0;
			return MARKED_AS_TAGGED;
		}
	}

	str.LoadString (IDS_SUBJECT);
	if (!strWord.CompareNoCase (str))
		return RecognizeSearchString (rstrArray, iLineIndex, iIndex,
		SUBJECT_CONTAINS_NODE, SUBJECT_CONTAINS_NOT_NODE, pbRE, strPhrase);

	str.LoadString (IDS_HEADER);
	if (!strWord.CompareNoCase (str))
		return RecognizeSearchString (rstrArray, iLineIndex, iIndex,
		HEADER_CONTAINS_NODE, HEADER_CONTAINS_NOT_NODE, pbRE, strPhrase);

	str.LoadString (IDS_FROM);
	if (!strWord.CompareNoCase (str))
		return RecognizeSearchString (rstrArray, iLineIndex, iIndex,
		FROM_CONTAINS_NODE, FROM_CONTAINS_NOT_NODE, pbRE, strPhrase);

	str.LoadString (IDS_BODY);
	if (!strWord.CompareNoCase (str))
		return RecognizeSearchString (rstrArray, iLineIndex, iIndex,
		BODY_CONTAINS_NODE, BODY_CONTAINS_NOT_NODE, pbRE, strPhrase);

	return 0;
}

// -------------------------------------------------------------------------
// RecognizeSearchString -- takes a condition element string and tells whether
// it's a particular type of search (e.g., searching the subject)
static int RecognizeSearchString (const CStringArray &rstrArray,
								  int &iLineIndex, int &iIndex, int iPositive, int iNegative, BOOL *pbRE,
								  CString &strPhrase)
{
	// scan "contains" or "does not contain"
	CString strWord;
	if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
		return 0;

	int iResult;
	CString str;
	str.LoadString (IDS_CONTAINS);
	if (!strWord.CompareNoCase (str))
		iResult = iPositive;
	else {
		str.LoadString (IDS_DOES);
		if (!strWord.CompareNoCase (str)) {
			if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_NOT) ||
				VerifyWord (rstrArray, iLineIndex, iIndex, IDS_CONTAIN))
				return 0;
			iResult = iNegative;
		}
		else
			return 0;
	}

	// scan optional "reg. expr." and the constant string
	if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
		return 0;

	if (pbRE) *pbRE = FALSE;
	str.LoadString (IDS_REG);
	if (!strWord.CompareNoCase (str)) {
		if (pbRE) *pbRE = TRUE;
		if (VerifyWord (rstrArray, iLineIndex, iIndex, IDS_EXPR))
			return 0;
		if (GetWord (rstrArray, iLineIndex, iIndex, strWord))
			return 0;
	}

	// verify and remove double-quotes
	int iLen = strWord.GetLength ();
	if (iLen < 2 || strWord [0] != '"' || strWord [iLen - 1] != '"')
		return 0;

	strWord = strWord.Right (iLen - 1);
	strWord = strWord.Left (iLen - 2);

	// done
	strPhrase = strWord;
	return iResult;
}

// -------------------------------------------------------------------------
// VerifyWord -- returns 0 for success, non-0 for failure
static int VerifyWord (const CStringArray &rstrArray, int &iLineIndex,
					   int &iIndex, int iID)
{
	CString str; str.LoadString (iID);
	return VerifyWord (rstrArray, iLineIndex, iIndex, str);
}

// -------------------------------------------------------------------------
// VerifyWord -- returns 0 for success, non-0 for failure
static int VerifyWord (const CStringArray &rstrArray, int &iLineIndex,
					   int &iIndex, LPCTSTR pchWord)
{
	CString strWord;
	return GetWord (rstrArray, iLineIndex, iIndex, strWord) ||
		strWord.CompareNoCase (pchWord);
}

// -------------------------------------------------------------------------
// GetWord -- returns 0 for success, non-0 for failure
static int GetWord (const CStringArray &rstrArray, int &iLineIndex,
					int &iIndex, CString &strWord)
{
	int iLines = rstrArray.GetSize ();
	int iLineLen;
	CString strLine;

	do {
		if (iLineIndex >= iLines)
			return 1;

		strLine = rstrArray [iLineIndex];

		// skip whitespace
		iLineLen = strLine.GetLength ();
		while (iIndex < iLineLen) {
			if (strLine [iIndex] != ' ')
				break;
			iIndex ++;
		}

		if (iIndex < iLineLen)
			break;

		iLineIndex ++;
		iIndex = 0;

	} while (1);

	// iIndex now points to the start of the word.  Extract the word and
	// advance iIndex
	BOOL bString = (strLine [iIndex] == '"');
	BOOL bIgnoreNextQuote = FALSE;
	int iRight = iIndex + 1;
	while (iRight < iLineLen) {
		if (!bString && strLine [iRight] == ' ')
			break;

		if (bString && strLine [iRight] == '"' && !bIgnoreNextQuote) {
			iRight ++;
			break;
		}

		bIgnoreNextQuote = (strLine [iRight] == '\\');
		iRight ++;
	}

	strWord = strLine.Mid (iIndex, iRight - iIndex);
	if (bString)
		strWord = UnescapeQuotes (strWord);

	iIndex = iRight;

	return 0;
}

// -------------------------------------------------------------------------
TRuleConditions::TRuleConditions (LPCTSTR pchStaticCondition /* = NULL */)
: CPropertyPage(TRuleConditions::IDD, 0)
{

	m_lLines = 0;
	m_iYesNo = -1;
	m_strPhrase = _T("");
	m_iArticleField = -1;
	m_iRadio = -1;
	m_bRE = FALSE;
	m_iGroups = 0;
	m_iListName = -1;
	m_iAttribute = -1;
	m_iDaysAgo = 0;
	m_lScore = 0;


	if (pchStaticCondition)
		m_strStaticCondition = pchStaticCondition;

	m_fInCondChange = false;
}

// -------------------------------------------------------------------------
TRuleConditions::~TRuleConditions ()
{
}

// -------------------------------------------------------------------------
void TRuleConditions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SCORE, m_sScore);
	DDX_Control(pDX, IDC_DAYS_AGO, m_sDaysAgo);
	DDX_Control(pDX, IDC_ATTRIBUTE, m_sAttribute);
	DDX_Control(pDX, IDC_LIST_NAME, m_sListName);
	DDX_Control(pDX, IDC_GROUPS, m_sGroups);
	DDX_Control(pDX, IDC_RE, m_sRE);
	DDX_Control(pDX, IDC_LINES, m_sLines);
	DDX_Control(pDX, IDC_FIELD_PHRASE, m_sPhrase);
	DDX_Control(pDX, IDC_FIELD_YES_NO, m_sYesNo);
	DDX_Control(pDX, IDC_ARTICLE_FIELD, m_sArticleField);
	DDX_Text(pDX, IDC_LINES, m_lLines);
	DDV_MinMaxLong(pDX, m_lLines, 0, 999999);
	DDX_CBIndex(pDX, IDC_FIELD_YES_NO, m_iYesNo);
	DDX_Text(pDX, IDC_FIELD_PHRASE, m_strPhrase);
	DDX_CBIndex(pDX, IDC_ARTICLE_FIELD, m_iArticleField);
	DDX_Radio(pDX, IDC_ARTICLE_FIELD_CONTAINS, m_iRadio);
	DDX_Check(pDX, IDC_RE, m_bRE);
	DDX_Text(pDX, IDC_GROUPS, m_iGroups);
	DDV_MinMaxInt(pDX, m_iGroups, 0, 999999);
	DDX_CBIndex(pDX, IDC_LIST_NAME, m_iListName);
	DDX_CBIndex(pDX, IDC_ATTRIBUTE, m_iAttribute);
	DDX_Text(pDX, IDC_DAYS_AGO, m_iDaysAgo);
	DDX_Text(pDX, IDC_SCORE, m_lScore);


	if (!m_strStaticCondition.IsEmpty ()) {
		DDX_Control(pDX, IDC_STATIC_CONDITION, m_sStaticCondition);
		DDX_Text(pDX, IDC_STATIC_CONDITION, m_strStaticCondition);
	}
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleConditions, CPropertyPage)
	ON_BN_CLICKED(IDC_ADD_RULE_CONDITION, OnAddRuleCondition)
	ON_BN_CLICKED(IDC_ARTICLE_FIELD_CONTAINS, OnFieldContains)
	ON_BN_CLICKED(IDC_IN_REPLY_CONDITION, OnInReplyCondition)
	ON_BN_CLICKED(IDC_RULE_AND, OnRuleAnd)
	ON_BN_CLICKED(IDC_RULE_CONDITION_DELETE, OnRuleConditionDelete)
	ON_BN_CLICKED(IDC_RULE_LPAREN, OnRuleLparen)
	ON_BN_CLICKED(IDC_RULE_MOVE_DOWN, OnRuleMoveDown)
	ON_BN_CLICKED(IDC_RULE_MOVE_UP, OnRuleMoveUp)
	ON_BN_CLICKED(IDC_RULE_NOT, OnRuleNot)
	ON_BN_CLICKED(IDC_RULE_OR, OnRuleOr)
	ON_BN_CLICKED(IDC_RULE_RPAREN, OnRuleRparen)
	ON_BN_CLICKED(IDC_LINES_MORE_THAN, OnLinesMoreThan)
	ON_BN_CLICKED(IDC_CROSSPOSTED, OnCrossposted)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_IN_LIST, OnInList)
	ON_BN_CLICKED(IDC_MARKED_AS, OnMarkedAs)
	ON_BN_CLICKED(IDC_POST_TIME, OnPostTime)
	ON_BN_CLICKED(IDC_RULE_IMPORT_COND, OnRuleImportCond)
	ON_BN_CLICKED(IDC_SCORE_MORE_THAN, OnScoreMoreThan)
	ON_WM_DESTROY()
	ON_EN_KILLFOCUS (IDC_CONDITION, OnCondKillFocus)
	ON_EN_SETFOCUS (IDC_CONDITION, OnCondSetFocus)
	ON_EN_CHANGE(IDC_CONDITION, OnCondChange)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TRuleConditions::UpdateGreyStates ()
{
	UpdateData ();
	m_sArticleField.EnableWindow (m_iRadio == 0);
	m_sYesNo.EnableWindow (m_iRadio == 0);
	m_sPhrase.EnableWindow (m_iRadio == 0);
	m_sRE.EnableWindow (m_iRadio == 0);
	m_sLines.EnableWindow (m_iRadio == 2);
	m_sGroups.EnableWindow (m_iRadio == 3);
	m_sDaysAgo.EnableWindow (m_iRadio == 4);
	m_sScore.EnableWindow (m_iRadio == 5);
	m_sListName.EnableWindow (m_iRadio == 6);
	m_sAttribute.EnableWindow (m_iRadio == 7);
}

// -------------------------------------------------------------------------
static int GetSelectedLine (CRichEditCtrl &sEdit)
{
	// whatever line the cursor is on
	long lStart, lEnd;
	sEdit.GetSel (lStart, lEnd);
	int iLine = (int) sEdit.LineFromChar (lStart);

	// if cursor is past the last line, put it on the last line
	int iCount = sEdit.GetLineCount ();
	if (iLine == iCount - 1)
		iLine --;

	return iLine;
}

// -------------------------------------------------------------------------
static void SelectLine (CRichEditCtrl &sEdit, int iLine, BOOL bNoColor = FALSE)
{
	sEdit.SetSel (0, -1);
	CHARFORMAT cf;
	cf.cbSize = sizeof (cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = RGB (0, 0, 0);     // black
	sEdit.SetSelectionCharFormat (cf);

	// aparently, this is needed to remind the rich-edit control of how many
	// lines it has, otherwise (for lines above 30 or so) LineIndex() returns -1
	sEdit.GetLineCount ();

	int iIndex = sEdit.LineIndex (iLine);

	if (!bNoColor) {
		int iNextLineIndex = sEdit.LineIndex (iLine + 1);
		sEdit.SetSel (iIndex, iNextLineIndex);
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = RGB (0, 0, 255);   // blue
		sEdit.SetSelectionCharFormat (cf);
	}

	sEdit.SetSel (iIndex, iIndex);
}

// -------------------------------------------------------------------------
static void AddLine (CRichEditCtrl &sEdit, LPCTSTR pchLine, BOOL bAfter = TRUE)
{
	int iLine = GetSelectedLine (sEdit);
	if (bAfter)
		iLine = iLine + 1;
	int iIndex = sEdit.LineIndex (iLine);

	CString strLine = CString (pchLine) + "\r\n";
	sEdit.SetSel (iIndex, iIndex);
	sEdit.ReplaceSel (strLine);

	SelectLine (sEdit, iLine);

	// make sure the new line is visible. Assume 10 visible lines or so
	int iFirst = sEdit.GetFirstVisibleLine ();
	if (iLine > iFirst + 8)
		sEdit.LineScroll (iLine - iFirst + 8);
}

// -------------------------------------------------------------------------
BOOL TRuleConditions::OnInitDialog()
{
	HWND hWnd = GetDlgItem (IDC_CONDITION) -> m_hWnd;
	m_sCondition.Attach (hWnd);

	CPropertyPage::OnInitDialog();

	// if m_strStaticCondition is empty, remove the static-condition control
	if (m_strStaticCondition.IsEmpty ()) {
		RECT sRect, sStaticRect;
		m_sCondition.GetWindowRect (&sRect);
		ScreenToClient (&sRect);
		GetDlgItem (IDC_STATIC_CONDITION) -> GetWindowRect (&sStaticRect);
		ScreenToClient (&sStaticRect);
		int iDX = sStaticRect.right - sStaticRect.left;
		int iDY = sRect.bottom - sStaticRect.top;
		int iX = sStaticRect.left;
		int iY = sStaticRect.top;

		GetDlgItem (IDC_STATIC_CONDITION) -> DestroyWindow ();
		m_sCondition.SetWindowPos (NULL, iX, iY, iDX, iDY, SWP_NOZORDER);
	}

	// this "empty" message needs to be there to start with
	CString str; str.LoadString (IDS_NO_CONDITION);
	AddLine (m_sCondition, str);
	m_bEmpty = TRUE;

	m_sCondition.SetEventMask (ENM_CHANGE);
	DisplayCurrentRule ();

	m_bDirty = FALSE;
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TRuleConditions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPropertyPage::OnShowWindow(bShow, nStatus);

	// for some reason, when we draw colors in InitDialog(), it doesn't show
	// up, so we do it here...
	SelectLine (m_sCondition, m_psRule -> rstrCondition.GetSize() - 1);
}

// -------------------------------------------------------------------------
// DisplayCurrentRule -- takes the current rule name from the rule listbox, and
// displays that rule in the dialog box's data fields
void TRuleConditions::DisplayCurrentRule ()
{
	// before writing to the condition list, clear it
	m_sCondition.SetWindowText ("");
	int i;
	for (i = 0; i < m_psRule -> rstrCondition.GetSize(); i++)
		AddLine (m_sCondition, m_psRule -> rstrCondition [i]);
	CheckIndentation ();

	// give initial settings for the yes/no combo boxes and the radio buttons
	m_iRadio = 0;
	m_iArticleField = 0;
	m_iYesNo = 0;
	m_iListName = 0;
	m_iAttribute = 0;
	UpdateData (FALSE);

	// set the initial grey states
	UpdateGreyStates();

	// select the last item in the edit box. This is so that when
	// another item is added, an `and' is automatically placed between them
	if (!m_bEmpty) {
		int iLines = m_sCondition.GetLineCount () - 1;
		SelectLine (m_sCondition, iLines - 1);
	}
}

// -------------------------------------------------------------------------
// UpdateRule -- updates the rule with this dialog's displayed values
int TRuleConditions::UpdateRule ()
{
	CStringArray rstrTemp;

	// get the contents of the condition listbox
	CString strLine;
	if (!m_bEmpty)
		for (int i = 0; i < m_sCondition.GetLineCount () - 1; i++)
		{
			GetLine (m_sCondition, i, strLine);
			rstrTemp.Add (strLine);
		}

		// if the condition is invalid, fail
		Condition* psCondition;
		int iIndex;
		int iBad = ConstructACondition (rstrTemp, psCondition, iIndex);
		if (iBad) {
			SelectLine (m_sCondition, iIndex);
			switch (iBad) {
		 case UNBALANCED_PARENS:
			 MsgResource (IDS_ERR_UNBALANCED_PARENS);
			 break;
		 case UNEXPECTED_TOKEN:
			 MsgResource (IDS_ERR_BAD_EXPR);
			 break;
		 default:
			 MsgResource (IDS_ERR_BAD_EXPR);
			 break;
			}
			return 1;
		}

		// keep this condition with the rule
		m_psRule -> psCondition = psCondition;

		// whenever updating a rule's psCondition, update this flag too
		m_psRule -> m_bCheckedForSubstitution = FALSE;

		// read the values of controls into the current rule
		m_psRule -> rstrCondition.Copy (rstrTemp);

		// if the condition is empty, but the user has done something in the
		// upper portion of the dialog page, she probably got confused or forgot
		// to press "add condition", so display a warning
		UpdateData (TRUE);
		if (!rstrTemp.GetSize () &&
			(!m_strPhrase.IsEmpty () || m_iRadio != 0))
			MsgResource (IDS_ERR_POSSIBLE_COND_MISTAKE);

		return 0;
}

// -------------------------------------------------------------------------
afx_msg void TRuleConditions::OnCondKillFocus ()
{
	// if there is a selection, preserve it
	long lBegin, lEnd;
	m_sCondition.GetSel (lBegin, lEnd);
	if (lBegin != lEnd)
		return;

	m_sCondition.SetRedraw (FALSE);

	int iLine = GetSelectedLine (m_sCondition);
	SelectLine (m_sCondition, iLine);

	m_sCondition.SetRedraw (TRUE);
	m_sCondition.Invalidate ();
}

// -------------------------------------------------------------------------
afx_msg void TRuleConditions::OnCondSetFocus ()
{
	m_sCondition.SetRedraw (FALSE);

	// put the cursor on whatever line is highlighted
	int iLine = GetSelectedLine (m_sCondition);
	SelectLine (m_sCondition, iLine, TRUE /* bNoColor */);

	// if empty, set focus at end of text
	if (m_bEmpty) {
		int iIndex = m_sCondition.GetTextLength ();
		m_sCondition.SetSel (iIndex, iIndex);
	}

	m_sCondition.SetRedraw (TRUE);
	m_sCondition.Invalidate ();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnAddRuleCondition()
{
	UpdateData ();

	// add an appropriate new item to the condition listbox
	CString strItem;

	switch (m_iRadio) {
	  case 0:
		  {
			  CString strWhichField;

			  switch (m_iArticleField) {
	  case 0:
		  strWhichField.LoadString (IDS_SUBJECT);
		  break;
	  case 1:
		  strWhichField.LoadString (IDS_FROM);
		  break;
	  case 2:
		  strWhichField.LoadString (IDS_HEADER);
		  break;
	  case 3:
		  strWhichField.LoadString (IDS_BODY);
		  break;
			  }

			  if (CheckRadioButton (strItem, (LPSTR) (LPCTSTR) strWhichField))
				  return;
			  break;
		  }

	  case 1:
		  {
			  CString str;
			  str.LoadString (IDS_IN); strItem = str + " ";
			  str.LoadString (IDS_REPLY); strItem += str + " ";
			  str.LoadString (IDS_TO); strItem += str + " ";
			  str.LoadString (IDS_MY); strItem += str + " ";
			  str.LoadString (IDS_MESSAGE); strItem += str;
		  }
		  break;

	  case 2:
		  strItem.LoadString (IDS_LINES);
		  strItem += " > ";
		  char rchString [20];
		  _ltoa (m_lLines, rchString, 10);
		  strItem += rchString;
		  break;

	  case 3:
		  {
			  CString strCrossposted, strGroups, strTo;
			  strCrossposted.LoadString (IDS_CROSSPOSTED);
			  strTo.LoadString (IDS_TO);
			  strGroups.LoadString (IDS_GROUPS);
			  strItem.Format ("%s %s > %d %s", strCrossposted, strTo, m_iGroups,
				  strGroups);

			  TServerCountedPtr psServer;
			  if (!psServer -> GetFullCrossPost ())
				  MsgResource (IDS_USE_CROSSPOST_OPTION);
		  }
		  break;

	  case 4:
		  {
			  CString str1, str2, str3;
			  str1.LoadString (IDS_POSTED);
			  str2.LoadString (IDS_DAYS);
			  str3.LoadString (IDS_AGO);
			  strItem.Format ("%s > %d %s %s", str1, m_iDaysAgo, str2, str3);
		  }
		  break;

	  case 5:
		  {
			  CString str1;
			  str1.LoadString (IDS_SCORE);
			  strItem.Format ("%s > %d", str1, m_lScore);
		  }
		  break;

	  case 6:
		  {
			  strItem.LoadString (IDS_IN);
			  CString str;
			  str.LoadString (m_iListName == 0 ? IDS_A : IDS_AN);
			  strItem += CString (" ") + str + " ";
			  str.LoadString (m_iListName == 0 ? IDS_WATCHED : IDS_IGNORED);
			  strItem += str + " ";
			  str.LoadString (IDS_THREAD);
			  strItem += str;
		  }
		  break;

	  case 7:
		  {
			  strItem.LoadString (IDS_MARKED);
			  CString str;
			  str.LoadString (IDS_AS);
			  strItem += CString (" ") + str + " ";
			  int index = m_sAttribute.GetCurSel ();
			  m_sAttribute.GetLBText (index, str);
			  str.SetAt (0, (char) _tolower (str [0]));
			  strItem += str;
		  }
		  break;
	  default:
		  ASSERT (0);
		  break;
	}

	InsertToken (strItem);

	// if we just added a "header contains" condition, pop up a note about it
	if (m_iRadio == 0 && m_iArticleField == 2)
		MsgResource (IDS_HEADER_CONTAINS_MESSAGE);

	// warn about line counts in articles vs. line counts in complete binaries
	if (2 == m_iRadio)
		MsgResource (IDS_WARN_COMBINED_LINES, this);
}

// -------------------------------------------------------------------------
// CheckRadioButton -- if the button's corresponding phrase string is empty,
// display a warning and return -1.  If the button is hilighted and its
// corresponding phrase string is not empty, return 1 and construct an
// appropriate CString to insert into the condition viewbox
int TRuleConditions::CheckRadioButton (CString& strItem, char * pchDescription)
{
	// if r.e. doesn't compile, warn user
	if (m_bRE) {
		TSearch sSearch;
		try {
			sSearch.SetPattern (m_strPhrase, FALSE /* fCaseSensitive */,
				TSearch::RE);
		}
		catch (TException *pE)
		{
			pE->Delete();
			// error message already displayed by SetPattern ()
			return 1;
		}
	}

	// if no phrase, warn user then return failure
	if (!m_strPhrase.GetLength ()) {
		CString strError; strError.LoadString (IDS_ERR_NO_PHRASE);
		strError += " ";
		strError += pchDescription;
		CString strFrom; strFrom.LoadString (IDS_FROM);
		if (!strcmp (pchDescription, strFrom)) {
			strError += " ";
			CString strField; strField.LoadString (IDS_FIELD);
			strError += strField;
		}
		AfxMessageBox (strError, MB_OK, 0);
		return 1;
	}

	strItem = pchDescription;
	strItem += " ";
	CString strContains;
	if (!m_iYesNo)
		strContains.LoadString (IDS_CONTAINS);
	else {
		CString strWord;
		strWord.LoadString (IDS_DOES); strContains = strWord + " ";
		strWord.LoadString (IDS_LOWERCASE_NOT); strContains += strWord + " ";
		strWord.LoadString (IDS_CONTAIN); strContains += strWord;
	}
	strItem += strContains + " ";
	if (m_bRE) {
		CString str;
		str.LoadString (IDS_REG); strItem += str + " ";
		str.LoadString (IDS_EXPR); strItem += str + " ";
	}

	// escape double-quotes
	CString str = EscapeQuotes (m_strPhrase);
	strItem += "\"" + str + "\"";
	return 0;
}

// -------------------------------------------------------------------------
// triggered during EN_CHANGE
void TRuleConditions::OnCondChange()
{
	// protect vs. recursion
	if (!m_fInCondChange)
	{
		m_fInCondChange = true;
		doCondChange ();
		m_fInCondChange = false;
	}
}

// -------------------------------------------------------------------------
void TRuleConditions::doCondChange()
{
	m_bDirty = TRUE;

	if (m_bEmpty)
	{
		CString str; str.LoadString (IDS_NO_CONDITION);

		if (m_sCondition.GetTextLength () <= str.GetLength () + 2)
		{
			// if user didn't insert anything new, keep the empty message
			m_sCondition.SetWindowText ("");
			AddLine (m_sCondition, str);

			// set cursor on next line
			int iIndex = m_sCondition.LineIndex (1);
			m_sCondition.SetSel (iIndex, iIndex);
		}
		else
		{
			// to remove "empty" message, we first have to find it

			FINDTEXTEX sFind;

			sFind.chrg.cpMin = 0;
			sFind.chrg.cpMax = -1; // search everything

			str += "\r\n";
			sFind.lpstrText = (LPSTR)(LPCTSTR) str;

			// search for the ("empty" message + CRLF)
			long lret = m_sCondition.FindText (FR_MATCHCASE, &sFind);

			if (lret != -1)
			{
				m_sCondition.SetSel (lret, lret + str.GetLength());
				m_sCondition.ReplaceSel ("");
			}

			// put cursor at end of existing text
			int iIndex = m_sCondition.GetTextLength ();
			m_sCondition.SetSel (iIndex, iIndex);

			m_bEmpty = FALSE;
		}
	}
	else
	{
		if (!m_sCondition.GetTextLength ())
		{
			m_bEmpty = TRUE;
			CString str; str.LoadString (IDS_NO_CONDITION);
			AddLine (m_sCondition, str);

			// set cursor on next line
			int iIndex = m_sCondition.LineIndex (1);
			m_sCondition.SetSel (iIndex, iIndex);
		}
	}
}

// -------------------------------------------------------------------------
void TRuleConditions::OnFieldContains()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnInReplyCondition()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnLinesMoreThan()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnCrossposted()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnPostTime()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnInList()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnMarkedAs()
{
	UpdateGreyStates();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnScoreMoreThan()
{
	UpdateGreyStates ();
}

// -------------------------------------------------------------------------
static void RemoveLine (CRichEditCtrl &sEdit, int iLine)
{
	int iIndex = sEdit.LineIndex (iLine);
	int iNextLineIndex = sEdit.LineIndex (iLine + 1);
	sEdit.SetSel (iIndex, iNextLineIndex);
	sEdit.ReplaceSel ("");
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleConditionDelete()
{
	if (m_bEmpty)
		return;

	m_bDirty = TRUE;

	m_sCondition.SetRedraw (FALSE);

	// delete the current line
	int iLine = GetSelectedLine (m_sCondition);
	RemoveLine (m_sCondition, iLine);

	int iCount = m_sCondition.GetLineCount ();
	if (iLine <= iCount - 1)
		SelectLine (m_sCondition, iLine);
	else
		SelectLine (m_sCondition, iLine - 1);

	CheckIndentation ();

	m_sCondition.SetRedraw (TRUE);
	m_sCondition.Invalidate ();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleMoveDown()
{
	if (m_bEmpty)
		return;

	int iLine = GetSelectedLine (m_sCondition);
	int iCount = m_sCondition.GetLineCount ();

	// make sure this isn't the first item or the only one
	if ((iLine == iCount - 1) || (iCount <= 1))
		return;

	m_bDirty = TRUE;

	m_sCondition.SetRedraw (FALSE);

	// move the item down
	CString strItem;
	GetLine (m_sCondition, iLine, strItem);
	RemoveLine (m_sCondition, iLine);
	SelectLine (m_sCondition, iLine);
	AddLine (m_sCondition, strItem);

	CheckIndentation ();

	m_sCondition.SetRedraw (TRUE);
	m_sCondition.Invalidate ();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleMoveUp()
{
	if (m_bEmpty)
		return;

	int iLine = GetSelectedLine (m_sCondition);
	int iCount = m_sCondition.GetLineCount ();

	// make sure this isn't the first item or the only one
	if ((iLine == 0) || (iCount <= 1))
		return;

	m_bDirty = TRUE;

	m_sCondition.SetRedraw (FALSE);

	// move the item up
	CString strItem;
	GetLine (m_sCondition, iLine, strItem);
	RemoveLine (m_sCondition, iLine);
	SelectLine (m_sCondition, iLine - 1);
	AddLine (m_sCondition, strItem, FALSE /* bAfter */);
	CheckIndentation ();

	m_sCondition.SetRedraw (TRUE);
	m_sCondition.Invalidate ();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleAnd()
{
	CString str; str.LoadString (IDS_AND);
	InsertToken (str);
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleLparen()
{
	InsertToken ("(");
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleNot()
{
	CString str; str.LoadString (IDS_NOT);
	InsertToken (str);
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleOr()
{
	CString str; str.LoadString (IDS_OR);
	InsertToken (str);
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleRparen()
{
	InsertToken (")");
}

// -------------------------------------------------------------------------
// IsLeafString -- tells whether a listbox element represents a leaf token
static int IsLeafString (const char * pchString)
{
	int iExistingSpaces = CountLeadingSpaces (pchString);
	CString strWithoutSpaces = & pchString [iExistingSpaces];
	CString str;
	if (!_stricmp ((LPCTSTR) strWithoutSpaces, "("))
		return 0;
	if (!_stricmp ((LPCTSTR) strWithoutSpaces, ")"))
		return 0;
	str.LoadString (IDS_AND);
	if (!_stricmp ((LPCTSTR) strWithoutSpaces, str))
		return 0;
	str.LoadString (IDS_OR);
	if (!_stricmp ((LPCTSTR) strWithoutSpaces, str))
		return 0;
	str.LoadString (IDS_NOT);
	if (!_stricmp ((LPCTSTR) strWithoutSpaces, str))
		return 0;
	return 1;
}

// -------------------------------------------------------------------------
void TRuleConditions::InsertToken (const char * pchToken)
{
	m_bDirty = TRUE;

	m_sCondition.SetRedraw (FALSE);

	// if the currently-selected item is a leaf and the item being inserted is
	// a leaf, insert an 'and' first
	if (!m_bEmpty) {
		int iLine = GetSelectedLine (m_sCondition);
		CString strCurrentItem;
		GetLine (m_sCondition, iLine, strCurrentItem);
		if (IsLeafString (pchToken) && IsLeafString (strCurrentItem)) {
			CString str; str.LoadString (IDS_AND);
			AddLine (m_sCondition, str);
		}
	}

	AddLine (m_sCondition, pchToken);
	CheckIndentation ();

	m_sCondition.SetRedraw (TRUE);
	m_sCondition.Invalidate ();
}

// -------------------------------------------------------------------------
// CheckIndentation -- checks indentation in listbox and fixes if needed
void TRuleConditions::CheckIndentation ()
{
	// remember which element is selected, and make sure it's selected
	// after the add/delete operations
	int iSelected = GetSelectedLine (m_sCondition);

	::CheckIndentation (m_sCondition);

	SelectLine (m_sCondition, iSelected);
}

// -------------------------------------------------------------------------
BOOL TRuleConditions::OnKillActive()
{
	if (m_bDirty && UpdateRule ())
		// UpdateRule() failed, probably because of an invalid expression
		return 0;
	return CPropertyPage::OnKillActive();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnOK ()
{
	CPropertyPage::OnOK();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnDestroy()
{
	m_sCondition.Detach ();

	CPropertyPage::OnDestroy();
}

// -------------------------------------------------------------------------
void TRuleConditions::OnRuleImportCond()
{
	TPickRule sDlg;
	if (sDlg.DoModal () != IDOK)
		return;

	Rule *psRule = GetGlobalRules () -> RuleFromID ((UCHAR) sDlg.m_iRule);
	m_psRule -> rstrCondition.Copy (psRule -> rstrCondition);
	FreeCondition (m_psRule -> psCondition);

	DisplayCurrentRule ();
	// for some reason, when we draw colors in DisplayCurrentRule(), it
	// doesn't show up, so we do it here...
	SelectLine (m_sCondition, m_psRule -> rstrCondition.GetSize() - 1);
}

// -------------------------------------------------------------------------
TPickRule::TPickRule(CWnd* pParent /*=NULL*/)
: CDialog(TPickRule::IDD, pParent)
{
}

// -------------------------------------------------------------------------
void TPickRule::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Control(pDX, IDC_RULES, m_sRules);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPickRule, CDialog)
	ON_LBN_SELCHANGE(IDC_RULES, OnSelchangeRules)
	ON_LBN_DBLCLK(IDC_RULES, OnDblclkRules)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TPickRule::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRuleIterator it (ReadLock);
	Rule *psRule;
	while ((psRule = it.Next ()) != 0) {
		int iIndex = m_sRules.AddString (psRule -> strRuleName);
		m_sRules.SetItemData (iIndex, psRule -> GetID ());
	}

	GreyButtons ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TPickRule::OnOK()
{
	int iIndex = m_sRules.GetCurSel ();
	ASSERT (iIndex >= 0);
	m_iRule = (int) m_sRules.GetItemData (iIndex);
	CDialog::OnOK();
}

// -------------------------------------------------------------------------
void TPickRule::OnSelchangeRules()
{
	GreyButtons ();
}

// -------------------------------------------------------------------------
void TPickRule::GreyButtons ()
{
	int iIndex = m_sRules.GetCurSel ();
	m_sOK.EnableWindow (iIndex >= 0);
}

// -------------------------------------------------------------------------
void TPickRule::OnDblclkRules()
{
	OnOK ();
}

BOOL TRuleConditions::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TRuleConditions::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
