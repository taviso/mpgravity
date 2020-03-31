/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ruleutil.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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

// ruleutil.cpp -- rule processing utilities

#include <io.h>					   // _access(), ...
#include <tchar.h>               // _tcslen

#include "stdafx.h"              // standard windows stuff
#include "article.h"             // TArticleHeader, ...
#include "newsgrp.h"             // TNewsGroup
#include "ruleutil.h"            // this file's prototypes
#include "arttext.h"             // fnFromLine()
#include "msgid.h"               // GenerateMessageID ()
#include "tglobopt.h"            // TGlobalOptions
#include "resource.h"            // IDS_*
#include "rules.h"               // Rule, ...
#include "genutil.h"             // EscapeString ()
#include "custmsg.h"             // WMU_REFRESH_OUTBOX, ...
#include "fileutil.h"            // NewsMessageBox
#include "server.h"              // TNewsServer, ...
#include "servcp.h"              // TServerCountedPtr
#include "newsdb.h"              // TRuleIterator
#include "ngutil.h"              // UtilGetStorageOption()
#include "rgui.h"                // GetShowFullHeader()

#include <sys/stat.h>
#include <fcntl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// ForwardArticleToIndividual -- forwards to a single address
void ForwardArticle (CString &strAddresses, TArticleHeader* pArtHdr,
					 TArticleText* pArtText, TNewsGroup *pNG)
{
	TServerCountedPtr cpNewsServer(pNG->GetParentServer());

	TEmailHeader * pMailHdr = new TEmailHeader;
	TEmailText   sMailText;
	TEmailText * pMailText = &sMailText;

	// create a new TEmailHeader and TEmailText, and insert the appropriate
	// fields, then ship them off.  The following snippets were taken from
	// rplydoc.cpp and rplyhdr.cpp

	int id = cpNewsServer->NextMailID ();
	CString strPostID;
	strPostID.Format ("%d", id);
	pMailText -> SetNumber (id);

	CString strMsgID;
	GenerateMessageID (cpNewsServer -> GetEmailAddress (),
		cpNewsServer -> GetSmtpServer (), id, strMsgID);
	pMailHdr -> SetMessageID (strMsgID);

	// save to "outbox" storage
	pMailHdr -> SetNumber (id);
	pMailHdr -> StampCurrentTime ();

	// subject, `from', and `to' fields
	CString strSubject; strSubject.LoadString (IDS_FWD_FWD);
	strSubject += ": (";
	strSubject += pNG -> GetName() +  ") ";
	strSubject += pArtHdr -> GetSubject();
	pMailHdr -> SetSubject (strSubject);

	CString strFrom;
	fnFromLine (strFrom, pNG -> m_GroupID, FALSE /* fAddressFirst */,
		TRUE /* fMailing */);
	pMailHdr -> SetFrom (strFrom);

	pMailHdr -> ParseTo (strAddresses); // expects a comma-separated recipient list

	CString str;
	CString strBody;
	strBody = "\r\n\r\n";
	str.LoadString (IDS_FWD_THIS_MESSAGE);
	strBody += str + " \"" + pNG -> GetName() + "\".\r\n\r\n----------- ";
	str.LoadString (IDS_FWD_BEGIN);
	strBody += str + " -----------\r\n\r\n";

	CString strHeader;
	if (gpGlobalOptions -> GetRegUI () -> GetShowFullHeader ())
		strHeader = pArtText -> GetHeader ();
	else
		SelectedHeaders (pArtHdr, pArtText, strHeader);

	strBody += strHeader;
	strBody += pArtText -> GetBody();
	pMailText -> SetBody (strBody);

	// outbox takes hdr
	cpNewsServer -> GetOutbox () -> SaveEmail (pMailHdr, pMailText);

	extern HWND ghwndMainFrame;
	PostMessage (ghwndMainFrame, WMU_REFRESH_OUTBOX, 0, 0);
	PostMessage (ghwndMainFrame, WMU_REFRESH_OUTBOX_BAR, 0, 0);
}

// -------------------------------------------------------------------------
// SaveToFileAction -- performs a save-to-file action
void SaveToFileAction (TArticleHeader* pArtHdr, TArticleText* pArtText,
					   CString &strFilename, BOOL bSeparator /* = TRUE */, int iFullHeader /* = -1 */)
{
	const char * gpchMessageSeparator =
		"\r\n------------------------------------------------------------\r\n\r\n";

	try {
		// if file doesn't exist, create it first
		if (_access ((LPCTSTR) strFilename, 0) == -1) {
			int iFile = -1;
			errno_t err = _sopen_s(&iFile, (LPCTSTR) strFilename, _O_CREAT|_O_TRUNC, _SH_DENYRW, _S_IWRITE);
			if (iFile != -1)
				_close (iFile);
		}

		CFile sFile ((LPCTSTR) strFilename, CFile::modeWrite |
			CFile::shareDenyNone);
		sFile.SeekToEnd ();

		CString strHeader;
		BOOL bFullHeader;
		if (iFullHeader == -1)
			bFullHeader = gpGlobalOptions -> GetRegUI () -> GetShowFullHeader ();
		else
			bFullHeader = iFullHeader;
		if (bFullHeader)
			strHeader = pArtText -> GetHeader ();
		else
			SelectedHeaders (pArtHdr, pArtText, strHeader);

		sFile.Write (strHeader, strHeader.GetLength ());
		sFile.Write ("\r\n", 2);
		const CString& strBody = pArtText -> GetBody();
		sFile.Write (strBody, strBody.GetLength());
		if (bSeparator)
			sFile.Write (gpchMessageSeparator, _tcslen (gpchMessageSeparator));
		else
			sFile.Write ("\r\n", 2);

		sFile.Close ();
	}
	catch (CFileException *pfe) {
		CString str; str.LoadString (IDS_ERR_APPEND);
		TCHAR rcBuf[256];
		if (pfe -> GetErrorMessage (rcBuf, sizeof rcBuf)) {
			str += "\r\n";
			str += rcBuf;
		}
		else {
			str += " ";
			str += strFilename + ".";
		}

		// function resets priority
		NewsMessageBox (TGlobalDef::kMainWnd, str, MB_OK | MB_ICONSTOP);
		pfe->Delete ();
	}
}

// -------------------------------------------------------------------------
// GetRule -- find a rule with the given name
Rule *GetRule (LPCTSTR pchName)
{
	TRuleIterator it (ReadLock);
	Rule *pRule;
	while ((pRule = it.Next ()) != 0) {
		if (pRule -> strRuleName == pchName)
			return pRule;
	}
	return NULL;
}

// -------------------------------------------------------------------------
// GetRuleIndex -- returns -1 for failure
int GetRuleIndex (LPCTSTR pchName)
{
	int iResult = -1;
	TRuleIterator it (ReadLock);
	Rule *pRule;
	int iIndex = 0;
	while ((pRule = it.Next ()) != 0) {
		if (pRule -> strRuleName == pchName) {
			iResult = iIndex;
			break;
		}
		iIndex ++;
	}
	return iResult;
}

// -------------------------------------------------------------------------
void AddCondToStringList (Rule *pRule, AddNodeType iNodeType,
						  CStringList &rstr)
{
	// if the rule's condition expression isn't there, construct it
	if (!pRule -> psCondition) {
		int iIndex;
		int RC = ConstructACondition (pRule -> rstrCondition,
			pRule -> psCondition, iIndex);
		if (RC)
			return;
	}

	// go through the rule's condition and add any node of the given type
	AddCondToStringList (pRule -> psCondition, iNodeType, rstr);
}

// -------------------------------------------------------------------------
// InCStringList -- is a string in a string-list?
BOOL InCStringList (CStringList &rstr, CString &str)
{
	POSITION iPos = rstr.GetHeadPosition ();
	while (iPos)
		if (str == rstr.GetNext (iPos))
			return TRUE;
	return FALSE;
}

// -------------------------------------------------------------------------
void AddCondToStringList (Condition *psCondition, AddNodeType iNodeType,
						  CStringList &rstr)
{
	if (!psCondition)
		return;

	// process left child
	AddCondToStringList (psCondition -> psLeft, iNodeType, rstr);

	// if this node is of the appropriate type, add its string to the list
	if ((psCondition -> iNode == SUBJECT_CONTAINS_NODE && iNodeType == ADD_COND_SUBJECT) ||
		(psCondition -> iNode == FROM_CONTAINS_NODE && iNodeType == ADD_COND_FROM) ||
		(psCondition -> iNode == HEADER_CONTAINS_NODE && iNodeType == ADD_COND_HEADER) ||
		(psCondition -> iNode == BODY_CONTAINS_NODE && iNodeType == ADD_COND_BODY)) {

			// go through the string's or'd clauses
			CString strClause;
			int iLen = psCondition -> strPhrase.GetLength ();
			char ch;
			for (int i = 0; i < iLen; i++) {

				ch = psCondition -> strPhrase [i];

				// handle escaped-characters
				int iEscaped = (ch == '\\');
				if (iEscaped)
					if (i+1 < iLen) {
						i++;
						ch = psCondition -> strPhrase [i];
					}
					else
						iEscaped = FALSE;

				if (!iEscaped && (ch == '(' || ch == ')' || ch == '|')) {
					if (strClause.GetLength() && !InCStringList (rstr, strClause))
						rstr.AddTail (strClause);
					strClause = "";
				}
				else
					strClause += ch;
			}
	}

	// process right child
	AddCondToStringList (psCondition -> psRight, iNodeType, rstr);
}

// -------------------------------------------------------------------------
void AddStringListToCond (CStringList &rstr, CStringArray &rstrCondition,
						  AddNodeType iNodeType)
{
	// if no elements in string-list, do nothing
	if (rstr.IsEmpty())
		return;

	// insert "or" string if needed
	if (rstrCondition.GetSize ()) {
		CString strOr; strOr.LoadString (IDS_OR);
		rstrCondition.Add (strOr);
	}

	POSITION iPos = rstr.GetHeadPosition ();
	CString str;
	str.LoadString (iNodeType == ADD_COND_SUBJECT ? IDS_SUBJECT :
		(iNodeType == ADD_COND_FROM ? IDS_FROM :
		(iNodeType == ADD_COND_HEADER ? IDS_HEADER : IDS_BODY)));
	str += " ";
	CString strContains; strContains.LoadString (IDS_CONTAINS);

	CString strWord, strRE;
	strWord.LoadString (IDS_REG); strRE = strWord + " ";
	strWord.LoadString (IDS_EXPR); strRE += strWord;

	str += strContains + " " + strRE + " \"(";
	CString strCondition;
	int iFirst = TRUE;
	while (iPos) {
		if (!iFirst)
			str += "|";
		CString strNew = EscapeString (rstr.GetNext (iPos));
		strNew = EscapeQuotes (strNew);
		str += strNew;
		iFirst = FALSE;
	}
	str += ")\"";
	rstrCondition.Add (str);
}

// -------------------------------------------------------------------------
// EnabledRulesExist -- Try to find a rule that is enabled
BOOL EnabledRulesExist ()
{
	TRuleIterator it (ReadLock);
	Rule *pRule;
	while ((pRule = it.Next ()) != 0) {
		if (pRule -> bEnabled)
			return TRUE;
	}
	return FALSE;
}

// -------------------------------------------------------------------------
static void SubstituteWithString (Condition *psCondition,
								  CString &strSubstituteWith)
{
	// base case
	if (!psCondition)
		return;

	// process left child
	SubstituteWithString (psCondition -> psLeft, strSubstituteWith);

	// process right child
	SubstituteWithString (psCondition -> psRight, strSubstituteWith);

	// check this node's string and substitute if needed
	CString &strOld = psCondition -> strPhrase;
	int iPos;
	while ((iPos = strOld.Find ("%s")) != -1) {
		CString strNew = strOld.Left (iPos);
		strNew += strSubstituteWith;
		strNew += strOld.Right (strOld.GetLength () - iPos - 2);
		strOld = strNew;
	}
}

// -------------------------------------------------------------------------
// DoRuleSubstitution -- asks the user for a string to substitute in a rule,
// and does the substitution.  Returns 0 for success, non-0 if the user
// wants to disable the rule
int DoRuleSubstitution (Rule &sRule, TArticleHeader *psHeader,
						TNewsGroup *psGroup, const char *pchExistingString)
{
	TRuleTextSubstitution sDlg;

	sDlg.m_strRuleName = sRule.strRuleName;
	sDlg.m_strGroupName = psGroup -> GetName ();
	if (psHeader)
		sDlg.m_strArticleSubject = psHeader -> GetSubject ();
	else
		sDlg.m_strArticleSubject.LoadString (IDS_NO_ARTICLE_LOADED_YET);
	sDlg.m_strExistingString = pchExistingString;

	int RC = sDlg.DoModal ();
	if (RC != IDOK) {
		sRule.bEnabled = FALSE;
		return 1;
	}

	// substitute each occurrence of %s with sDlg.m_strSubstituteWith within
	// the rule's compiled condition
	SubstituteWithString (sRule.psCondition, sDlg.m_strSubstituteWith);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// TRuleTextSubstitution dialog

TRuleTextSubstitution::TRuleTextSubstitution(CWnd* pParent /*=NULL*/)
: CDialog(TRuleTextSubstitution::IDD, pParent)
{

	m_strSubstituteWith = _T("");
	m_strRuleName = _T("");
	m_strGroupName = _T("");
	m_strExistingString = _T("");
	m_strArticleSubject = _T("");
}

// -------------------------------------------------------------------------
void TRuleTextSubstitution::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SUBSTITUTE_WITH, m_sSubstituteWith);
	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Text(pDX, IDC_SUBSTITUTE_WITH, m_strSubstituteWith);
	DDX_Text(pDX, IDC_RULE_NAME, m_strRuleName);
	DDX_Text(pDX, IDC_GROUP_NAME, m_strGroupName);
	DDX_Text(pDX, IDC_EXISTING_STRING, m_strExistingString);
	DDX_Text(pDX, IDC_ARTICLE_SUBJECT, m_strArticleSubject);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleTextSubstitution, CDialog)
		ON_EN_CHANGE(IDC_SUBSTITUTE_WITH, OnChangeSubstituteWith)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TRuleTextSubstitution::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_sOK.EnableWindow (FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TRuleTextSubstitution::OnChangeSubstituteWith()
{
	m_sOK.EnableWindow (m_sSubstituteWith.LineLength ());
}

// -------------------------------------------------------------------------
// Export -- returns 0 for success, non-0 for failure
int Rules::Export (CString &strFile)
{
	CFile file;
	if (!file.Open (strFile, CFile::modeWrite | CFile::modeCreate))
		return 1;

	try {
		TRuleIterator it (*this, ::ReadLock);
		Rule *psRule;
		while ((psRule = it.Next ()) != 0)
			psRule -> Export (file);
	}
	catch (CException *pE)
	{
		pE->Delete();
		return 1;
	}

	// note : CFile destructor will call Close()
	return 0;
}

// -------------------------------------------------------------------------
// Import -- returns 0 for success, non-0 for failure
int Rules::Import (CString &strFile, int &iLine, BOOL bDeleteOriginal)
{
	iLine = 0;

	CStdioFile file;
	if (!file.Open (strFile, CFile::modeRead | CFile::typeText))
		return 1;

	WriteLock ();

	int iResult = 0;
	try {
		while (file.GetPosition () < file.GetLength () - 1) {
			Rule *psRule = new Rule;
			int RC = psRule -> Import (file, iLine);
			if (RC == 1) {
				// end of file
				delete psRule;
				break;
			}
			if (RC) {
				iResult = 1;
				delete psRule;
				break;
			}

			if (bDeleteOriginal) {
				Rule *psExists;
				if (Lookup (psRule -> strRuleName, psExists)) {
					RemoveKey (psRule -> strRuleName);
					delete psExists;
				}
			}
			else {
				// make sure the rule has a unique name
				CString orig = psRule -> strRuleName;
				Rule *psExists;
				int iNum = 1;
				char rchTemp [10];
				while (Lookup (psRule -> strRuleName, psExists) && iNum < 1000) {
					iNum ++;
					psRule -> strRuleName = orig + " " + _itoa (iNum, rchTemp, 10);
				}
			}

			SetAt (psRule -> strRuleName, psRule);
		}
	}
	catch (CException *pE)
	{
		pE->Delete();
		UnlockWrite ();
		return 1;
	}

	UnlockWrite ();

	// note : CFile destructor will call Close()
	return iResult;
}

// -------------------------------------------------------------------------
static void LineOut (CFile &file, const CString &line)
{
	file.Write (line, line.GetLength ());
	file.Write ("\r\n", 2);
}

// -------------------------------------------------------------------------
void Rule::Export (CFile &file)
{
	LineOut (file, CString ("rule = ") + strRuleName);
	LineOut (file, CString ("  enabled = ") + (bEnabled ? "Yes" : "No"));

	// expiration
	CString str;
	str.Format ("  expirationDays %d", iExpirationDays);
	LineOut (file, str);
	str.Format ("  expirationType %d", (int) iExpirationType);
	LineOut (file, str);
	str.Format ("  expirationAction %d", iExpirationAction);
	LineOut (file, str);

	LineOut (file, CString ("  allgroups = ") + (bAllGroups ? "Yes" : "No"));
	if (bWildGroups)
		LineOut (file, CString ("  groups = ") + strWildGroups);
	LineOut (file, CString ("begin groups"));
	int i = 0;
	for (int i = 0; i < rstrGroups.GetSize(); i++)
		LineOut (file, CString ("  ") + rstrGroups [i]);
	LineOut (file, CString ("end groups"));
	LineOut (file, CString ("begin condition"));
	for (i = 0; i < rstrCondition.GetSize(); i++)
		LineOut (file, CString ("  ") + rstrCondition [i]);
	LineOut (file, CString ("end condition"));
	LineOut (file, CString ("begin actions"));
	if (bPlaySound)
		LineOut (file, CString ("  playSound ") + pathSoundToPlay);
	if (bShowAlert)
		LineOut (file, CString ("  showAlert ") + strAlertText);
	if (bSaveToFile)
		LineOut (file, CString ("  saveTo ") + strSaveFilename);
	if (bForwardTo)
		LineOut (file, CString ("  forwardTo ") + strForwardToName);
	if (bDiscard)
		LineOut (file, CString ("  discard"));
	if (bReadEnable)
		LineOut (file, bRead ? CString ("  markAsRead") : CString ("  markAsUnread"));
	if (bImportantEnable)
		LineOut (file, bImportant ? CString ("  markAsImportant") : CString ("  markAsNormal"));
	if (bProtectedEnable)
		LineOut (file, bProtected ? CString ("  markAsProtected") : CString ("  markAsDeletable"));
	if (bDecode)
		LineOut (file, CString ("  decode ") + strUUDecodeDirectory);
	if (bTag)
		LineOut (file, CString ("  tag"));
	if (bGetBody)
		LineOut (file, CString ("  fetchBody"));
	if (bAddToWatch)
		LineOut (file, CString ("  watchThread"));
	if (bAddToIgnore)
		LineOut (file, CString ("  ignoreThread"));
	if (bAddToBozo)
		LineOut (file, CString ("  authorBozo"));
	if (bAddToScore) {
		CString strScore;
		strScore.Format ("%d", lScore);
		LineOut (file, CString ("  addToScore ") + strScore);
	}
	LineOut (file, CString ("end actions"));
	LineOut (file, CString ("end rule"));
	LineOut (file, CString (""));
}

// -------------------------------------------------------------------------
// GetWord -- returns the next word in the line (and removes it from the line).
// If the line doesn't have a word, it reads lines until it gets a word.
// Returns 0 for success, non-0 for failure
static int GetWord (CStdioFile &file, CString &line, CString &word, int &iLine)
{
	do {
		line.TrimLeft ();
		if (line.IsEmpty ()) {
			// read a new line
			iLine ++;
			if (!file.ReadString (line))
				return 1;
		}
	} while (line.IsEmpty ());

	line.TrimLeft ();
	word = "";
	int i = 0;
	while (i < line.GetLength ()) {
		if (line [i] == ' ')
			break;
		word += line [i];
		i++;
	}

	line = line.Right (line.GetLength () - i);
	return 0;
}

// -------------------------------------------------------------------------
static int VerifyWord (CStdioFile &file, CString &line, LPCTSTR pchVerify,
					   int &iLine)
{
	CString word;
	return (GetWord (file, line, word, iLine) || word.CompareNoCase (pchVerify));
}

// -------------------------------------------------------------------------
// Import -- returns 0 for success, 1 for end-of-file (no error), >1 for failure
int Rule::Import (CStdioFile &file, int &iLine)
{
	CString line, word;

	// read "rule"
	if (GetWord (file, line, word, iLine))
		return 1;   // end of file
	if (word.CompareNoCase ("rule"))
		return 2;

	// read '='
	if (GetWord (file, line, word, iLine) || word != "=")
		return 2;

	// rest of line is rule name
	line.TrimLeft ();
	line.TrimRight ();
	strRuleName = line;
	line = "";

	// read options & sections until done
	while (1) {
		if (GetWord (file, line, word, iLine))
			return 2;

		if (!word.CompareNoCase ("expirationDays")) {
			if (GetWord (file, line, word, iLine))
				return 2;
			iExpirationDays = (USHORT) atoi (word);
		}
		else if (!word.CompareNoCase ("expirationType")) {
			if (GetWord (file, line, word, iLine))
				return 2;
			iExpirationType = (ExpirationType) atoi (word);
		}
		else if (!word.CompareNoCase ("expirationAction")) {
			if (GetWord (file, line, word, iLine))
				return 2;
			iExpirationAction = (ExpirationAction) atoi (word);
		}
		else if (!word.CompareNoCase ("enabled")) {
			if (VerifyWord (file, line, "=", iLine))
				return 2;
			if (GetWord (file, line, word, iLine))
				return 2;
			bEnabled = !word.CompareNoCase ("yes");
		}
		else if (!word.CompareNoCase ("allgroups")) {
			if (VerifyWord (file, line, "=", iLine))
				return 2;
			if (GetWord (file, line, word, iLine))
				return 2;
			bAllGroups = !word.CompareNoCase ("yes");
		}
		else if (!word.CompareNoCase ("groups")) {
			if (VerifyWord (file, line, "=", iLine))
				return 2;
			bWildGroups = TRUE;
			line.TrimLeft ();
			strWildGroups = line;
			line = "";
		}
		else if (!word.CompareNoCase ("begin")) {
			if (GetWord (file, line, word, iLine))
				return 2;
			if (!word.CompareNoCase ("groups")) {
				if (ImportGroups (file, iLine))
					return 2;
			}
			else if (!word.CompareNoCase ("condition")) {
				if (ImportCondition (file, iLine))
					return 2;
			}
			else if (!word.CompareNoCase ("actions")) {
				if (ImportActions (file, iLine))
					return 2;
			}
			else
				return 2;
		}
		else if (!word.CompareNoCase ("end")) {
			if (VerifyWord (file, line, "rule", iLine))
				return 2;
			break;
		}
		else
			return 2;
	}

	return 0;
}

// -------------------------------------------------------------------------
// ImportCStringArray -- returns 0 for success, non-0 for failure
static int ImportCStringArray (CStdioFile &file, CStringArray &array,
							   int &iLine)
{
	CString line, word;

	while (1) {
		line = "";
		if (GetWord (file, line, word, iLine))
			return 1;

		// put the word back in the line
		line.TrimLeft ();
		line = word + " " + line;
		line.TrimLeft ();
		line.TrimRight ();

		if (!word.CompareNoCase ("end"))
			break;      // assume rest of line is "groups" or "condition"
		array.Add (line);
	}

	return 0;
}

// -------------------------------------------------------------------------
// ImportGroups -- returns 0 for success, non-0 for failure
int Rule::ImportGroups (CStdioFile &file, int &iLine)
{
	return ImportCStringArray (file, rstrGroups, iLine);
}

// -------------------------------------------------------------------------
// ImportCondition -- returns 0 for success, non-0 for failure
int Rule::ImportCondition (CStdioFile &file, int &iLine)
{
	return ImportCStringArray (file, rstrCondition, iLine);
}

// -------------------------------------------------------------------------
// ImportActions -- returns 0 for success, non-0 for failure
int Rule::ImportActions (CStdioFile &file, int &iLine)
{
	CString line, word;

	while (1) {
		line = "";
		if (GetWord (file, line, word, iLine))
			return 2;

		if (!word.CompareNoCase ("playSound")) {
			bPlaySound = TRUE;
			line.TrimLeft ();
			pathSoundToPlay = line;
		}
		else if (!word.CompareNoCase ("showAlert")) {
			bShowAlert = TRUE;
			line.TrimLeft ();
			strAlertText = line;
		}
		else if (!word.CompareNoCase ("saveTo")) {
			bSaveToFile = TRUE;
			line.TrimLeft ();
			strSaveFilename = line;
		}
		else if (!word.CompareNoCase ("forwardTo")) {
			bForwardTo = TRUE;
			line.TrimLeft ();
			strForwardToName = line;
		}
		else if (!word.CompareNoCase ("discard"))
			bDiscard = TRUE;
		else if (!word.CompareNoCase ("markAsRead"))
			bReadEnable = bRead = TRUE;
		else if (!word.CompareNoCase ("MarkAsUnread")) {
			bReadEnable = TRUE;
			bRead = FALSE;
		}
		else if (!word.CompareNoCase ("markAsImportant"))
			bImportantEnable = bImportant = TRUE;
		else if (!word.CompareNoCase ("markAsNormal")) {
			bImportantEnable = TRUE;
			bImportant = FALSE;
		}
		else if (!word.CompareNoCase ("markAsProtected"))
			bProtectedEnable = bProtected = TRUE;
		else if (!word.CompareNoCase ("markAsDeletable")) {
			bProtectedEnable = TRUE;
			bProtected = FALSE;
		}
		else if (!word.CompareNoCase ("decode")) {
			bDecode = TRUE;
			line.TrimLeft ();
			strUUDecodeDirectory = line;
		}
		else if (!word.CompareNoCase ("tag"))
			bTag = TRUE;
		else if (!word.CompareNoCase ("fetchBody"))
			bGetBody = TRUE;
		else if (!word.CompareNoCase ("watchThread"))
			bAddToWatch = TRUE;
		else if (!word.CompareNoCase ("ignoreThread"))
			bAddToIgnore = TRUE;
		else if (!word.CompareNoCase ("authorBozo"))
			bAddToBozo = TRUE;
		else if (!word.CompareNoCase ("addToScore")) {
			bAddToScore = TRUE;
			line.TrimLeft ();
			lScore = atol (line);
		}
		else if (!word.CompareNoCase ("end")) {
			break;   // assume rest of line is "actions"
		}
		else
			return 2;
	}

	return 0;
}

// -------------------------------------------------------------------------
static BOOL CondHasBodyContainsCond (Condition *psCond)
{
	if (!psCond)
		return FALSE;
	if (psCond -> iNode == BODY_CONTAINS_NODE)
		return TRUE;
	return CondHasBodyContainsCond (psCond -> psLeft) ||
		CondHasBodyContainsCond (psCond -> psRight);
}

// -------------------------------------------------------------------------
static BOOL RuleHasBodyContainsCond (Rule &sRule)
{
	if (!sRule.psCondition)
		sRule.CompileCondition ();
	return CondHasBodyContainsCond (sRule.psCondition);
}

// -------------------------------------------------------------------------
BOOL RuleMayRequireBody (Rule &sRule)
{
	return sRule.bSaveToFile || sRule.bForwardTo ||
		RuleHasBodyContainsCond (sRule);
}

// -------------------------------------------------------------------------
void TagArticle (TNewsGroup *pNG, TArticleHeader *pHeader)
{
	TServerCountedPtr cpNewsServer(pNG->GetParentServer());

	TArticleHeader *psTagHeaderPtr = 0;
	if (UtilGetStorageOption (pNG) == TNewsGroup::kNothing) {
		// take a copy of this header and queue it up to be saved later when we
		// save the body, we will have a header to go with it
		psTagHeaderPtr = new TArticleHeader;
		*psTagHeaderPtr = *pHeader;
	}

	TPersistentTags &rTags = cpNewsServer -> GetPersistentTags ();
	rTags.AddTag (pNG, pHeader -> GetNumber (), pHeader -> GetLines (),
		psTagHeaderPtr);
}
