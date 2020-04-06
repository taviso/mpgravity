/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rules.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

// rules.cpp -- code for the rules class

#include "stdafx.h"

#include <memory.h>              // memset()
#include <tchar.h>               // _tcslen
#include <stdlib.h>              // atoi()

#include <mmsystem.h>            // PlaySound()
#include "news.h"                // CNewsApp
#include "resource.h"            // for rules.h, IDS_*
#include "rules.h"               // Rule, Rules, ...
#include "globals.h"             // gpStore
#include "names.h"               // NAME_RULES
#include "rulesdlg.h"            // TRulesDlg
#include "trulegen.h"            // TRuleGeneral
#include "trulecon.h"            // TRuleConditions, ...
#include "truleact.h"            // TRuleActions
#include "thredlst.h"            // TThreadList
#include "statvec.h"             // TStatusUnit::k*
#include "tasker.h"              // CancelArticleBody()
#include "tidarray.h"            // IsSomeMessageIDInSet()
#include "ruleutil.h"            // ForwardArticle(), ...
#include "decoding.h"            // QueueArticleForDecoding()
#include "genutil.h"             // MsgResource(), ...
#include "ngutil.h"              // UtilGetStorageOption()
#include "tmanrule.h"            // TManualRule
#include "server.h"              // TPersistentTags
#include "servcp.h"              // TServerCountedPtr
#include "idxlst.h"              // TArticleIndexList
#include "iterhn.h"              // TArticleNewIterator
#include "uipipe.h"              // gpUIPipe
#include "newsview.h"            // CNewsView
#include "newsdb.h"              // TRuleIterator
#include "thrdact.h"             // TThreadActionList::Add()
#include "tbozobin.h"            // AddBozo()
#include "timpword.h"            // AddImportantWord()
#include "mainfrm.h"             // CMainFrame
#include "tnews3md.h"            // TNews3MDIChildWnd

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsTasker* gpTasker;

static LPCTSTR gpchArticleSubject;  // for display-message rule action
static LPCTSTR gpchArticleAuthor;   // for display-message rule action
static LPCTSTR gpchArticleGroup;    // for display-message rule action
static LPCTSTR gpchRuleMessage;     // for display-message rule action
static int giHourglassInEffect;         // is hourglass displayed?

class QueuedAlert : public CObject {
public:
	Rule *psRule;
	CString strSubject;
	CString strFrom;
	CString strGroup;
};
CPtrList QueuedDisplayList;

static const ULONG glSecondsPerDay = (ULONG) 60 * 60 * 24;
static const int giDefaultRuleExpirationDays = 7;

// -------------------------------------------------------------------------
Rule::Rule (Rules *psRuleCollection /* = NULL */)
	: PObject (RULE_VERSION_NUMBER)
{
	// if psRuleCollection is NULL, use the global rules
	if (!psRuleCollection)
		psRuleCollection = GetGlobalRules ();

	psRuleCollection->WriteLock ();

	// give the new rule a unique ID. Reserve the ID 0 for invalid-ID
	do
	{
		m_iID = psRuleCollection->m_iNextID ++;
	} while (!m_iID);

	// find a unique name for our rule
	CString strBase; strBase.LoadString (IDS_RULE_NUM);
	strBase += " ";
	unsigned long lNum = 1;
	CString strName;
	char rchTemp [10];
	void * pDummyPtr;

	while (lNum < 0xffffffff)
	{
		strName = strBase + (CString) _ltoa (lNum, rchTemp, 10);
		if (!psRuleCollection->Lookup (strName, (Rule *&) pDummyPtr))
			break;
		lNum ++;
	}

	psRuleCollection->UnlockWrite ();

	m_bCheckedForSubstitution = FALSE;
	m_dwTicks = 0;
	m_iEvaluated = m_iFired = 0;

	strRuleName = strName;
	bEnabled = 0;
	bAllGroups = 1;
	bWildGroups = 0;

	// condition field
	psCondition = NULL;

	// no need to initialize rstrGroups

	// initialize sound-to-play with the default sound directory name
	CString strTemp;
	GetDefaultSoundDirectory (strTemp);
	if (strTemp[strTemp.GetLength() - 1] != '\\')
		strTemp += "\\";
	pathSoundToPlay = strTemp;

	// expiration
	iExpirationDays = giDefaultRuleExpirationDays;
	iExpirationType = NO_EXPIRATION;
	iExpirationAction = DISABLE;

	// action fields
	bPlaySound = 0;
	bShowAlert = 0;
	strAlertText = "";
	bSaveToFile = 0;
	strSaveFilename = "";
	bForwardTo = 0;
	strForwardToName = "";
	bImportantEnable = 0;
	bImportant = 1;
	bReadEnable = 0;
	bRead = 1;
	bProtectedEnable = 0;
	bProtected = 1;
	bDiscard = 0;
	bDecode = 0;
	strUUDecodeDirectory = "";
	bAddToScore = 0;
	lScore = 0;
	bTag = 0;
	bGetBody = 0;
	bAddToIgnore = 0;
	bAddToWatch = 0;
	bAddToBozo = 0;

	bReserved2 = 0;
	bReserved3 = 0;
	bReserved4 = 0;
	bReserved5 = 0;
	bReserved6 = 0;
	bReserved7 = 0;
	bReserved8 = 0;
	bReserved9 = 0;
	bReserved10 = 0;
	ulReserved1 = 0;
	ulReserved2 = 0;
	ulReserved3 = 0;
	ulReserved4 = 0;
	ulReserved5 = 0;
	ulReserved6 = 0;
	ulReserved7 = 0;
	ulReserved8 = 0;
	ulReserved9 = 0;
	ulReserved10 = 0;
}

// -------------------------------------------------------------------------
Rule::~Rule()
{
	if (psCondition)
		FreeCondition (psCondition);
}

// -------------------------------------------------------------------------
void Rule::operator= (const Rule &source)
{
	TExpirable::operator= (source);

	strRuleName = source.strRuleName;
	bEnabled = source.bEnabled;
	bAllGroups = source.bAllGroups;
	bWildGroups = source.bWildGroups;
	strWildGroups = source.strWildGroups;

	m_bCheckedForSubstitution = source.m_bCheckedForSubstitution;
	m_dwTicks = source.m_dwTicks;
	m_iEvaluated = source.m_iEvaluated;
	m_iFired = source.m_iFired;

	rstrCondition.Copy (source.rstrCondition);
	rstrGroups.Copy (source.rstrGroups);

	// don't bother copying the condition tree.  It will be constructed when
	// needed
	psCondition = FALSE;

	// expiration
	iExpirationDays = source.iExpirationDays;
	iExpirationType = source.iExpirationType;
	iExpirationAction = source.iExpirationAction;

	// action fields
	bPlaySound = source.bPlaySound;
	pathSoundToPlay = source.pathSoundToPlay;
	bShowAlert = source.bShowAlert;
	strAlertText = source.strAlertText;
	bSaveToFile = source.bSaveToFile;
	strSaveFilename = source.strSaveFilename;
	bForwardTo = source.bForwardTo;
	strForwardToName = source.strForwardToName;
	bReadEnable = source.bReadEnable;
	bRead = source.bRead;
	bImportantEnable = source.bImportantEnable;
	bImportant = source.bImportant;
	bProtectedEnable = source.bProtectedEnable;
	bProtected = source.bProtected;
	bDiscard = source.bDiscard;
	bDecode = source.bDecode;
	strUUDecodeDirectory = source.strUUDecodeDirectory;
	bTag = source.bTag;
	bAddToScore = source.bAddToScore;
	lScore = source.lScore;
	bGetBody = source.bGetBody;
	bAddToIgnore = source.bAddToIgnore;
	bAddToWatch = source.bAddToWatch;
	bAddToBozo = source.bAddToBozo;
}

// -------------------------------------------------------------------------
// SerializeCStringArray -- saves/loads a CStringArray
static void SerializeCStringArray (CArchive& sArchive, CStringArray& rstr)
{
	if (sArchive.IsStoring()) {
		unsigned short iNum = (unsigned short) rstr.GetSize();
		sArchive << (unsigned short) iNum;
		for (int i = 0; i < iNum; i++)
			sArchive << (CString) rstr [i];
	}
	else {
		rstr.RemoveAll();
		unsigned short iNum;
		sArchive >> (unsigned short&) iNum;
		CString str;
		for (int i = 0; i < iNum; i++) {
			sArchive >> (CString&) str;
			rstr.SetAtGrow (i, str);
		}
	}
}

// -------------------------------------------------------------------------
static void ConvertOlderConditionString (CString &str)
{
	// in older versions, there could be only one string on a line, and the
	// closing doublequotes occur as the last character
	if (str.IsEmpty () || str [str.GetLength () - 1] != '"')
		return;

	int iStart = str.Find ('"') + 1;
	CString strEscaped = str.Mid (iStart, str.GetLength () - iStart - 1);
	strEscaped = EscapeQuotes (strEscaped);

	str = str.Left (iStart) + strEscaped + "\"";
}

// -------------------------------------------------------------------------
// ConvertOlderCondition -- escapes embedded double-quotes
static void ConvertOlderCondition (CStringArray &rstrCondition)
{
	int iNum = rstrCondition.GetSize ();
	for (int i = 0; i < iNum; i++)
		ConvertOlderConditionString (rstrCondition [i]);
}

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
IMPLEMENT_SERIAL (Rule, PObject, RULE_VERSION_NUMBER)

// -------------------------------------------------------------------------
// Serialize -- read/write the object from/to a file
void Rule::Serialize (CArchive& sArchive)
{
	PObject::Serialize (sArchive);
	BOOL bObsolete;
	int i;

	if (sArchive.IsStoring ()) {

		TExpirable::Serialize (sArchive);

		sArchive << (CString) strRuleName;
		sArchive << m_iID;
		sArchive << (unsigned short) bEnabled;
		sArchive << (unsigned short) bAllGroups;
		SerializeCStringArray (sArchive, rstrGroups);

		// condition
		SerializeCStringArray (sArchive, rstrCondition);

		// expiration
		sArchive << iExpirationDays;
		sArchive << (USHORT) iExpirationType;
		sArchive << (USHORT) iExpirationAction;

		// actions
		sArchive << (unsigned short) bPlaySound;
		sArchive << (TPath) pathSoundToPlay;
		sArchive << (unsigned short) bShowAlert;
		sArchive << (CString) strAlertText;
		sArchive << (unsigned short) bSaveToFile;
		sArchive << (CString) strSaveFilename;
		sArchive << (unsigned short) bForwardTo;
		sArchive << (CString) strForwardToName;
		sArchive << (unsigned short) bDiscard;
		sArchive << (unsigned short) bReadEnable;
		sArchive << (unsigned short) bRead;
		sArchive << (unsigned short) bImportantEnable;
		sArchive << (unsigned short) bImportant;
		sArchive << (unsigned short) bProtectedEnable;
		sArchive << (unsigned short) bProtected;
		sArchive << (unsigned short) bDecode;
		sArchive << (CString) strUUDecodeDirectory;
		sArchive << (unsigned short) bTag;
		sArchive << (unsigned short) bGetBody;
		sArchive << (unsigned short) bAddToIgnore;
		sArchive << (unsigned short) bAddToWatch;
		sArchive << (unsigned short) bAddToBozo;
		sArchive << (unsigned short) bAddToScore;
		sArchive << (unsigned long) lScore;

		sArchive << (unsigned short) bWildGroups;
		for (i = 2; i <= 10; i++)
			// bReserved2 through bReserved10
			sArchive << (unsigned short) bReserved2;

		for (i = 1; i <= 10; i++)
			// ulReserved1 through ulReserved10
			sArchive << ulReserved1;

		sArchive << strWildGroups;
		for (i = 2; i <= 10; i++)
			// strReserved2 through ulReserved10
			sArchive << strReserved2;
	}
	else {

		if (GetObjectVersion () < RULE_VERSION_NUMBER) {
			// it's an older version, here we need to read the object in its
			// original format then convert it...

			if (GetObjectVersion () == 1) {
				sArchive >> (CString&) strRuleName;
				sArchive >> m_iID;
				sArchive >> (unsigned short&) bEnabled;
				sArchive >> (unsigned short&) bAllGroups;
				SerializeCStringArray (sArchive, rstrGroups);

				// condition
				SerializeCStringArray (sArchive, rstrCondition);
				ConvertOlderCondition (rstrCondition);

				// actions
				sArchive >> (unsigned short&) bPlaySound;
				sArchive >> (TPath&) pathSoundToPlay;
				sArchive >> (unsigned short&) bShowAlert;
				sArchive >> (CString&) strAlertText;
				sArchive >> (unsigned short&) bSaveToFile;
				sArchive >> (CString&) strSaveFilename;
				sArchive >> (unsigned short&) bForwardTo;
				sArchive >> (CString&) strForwardToName;

				sArchive >> (unsigned short&) bImportant;
				if (bImportant)
					bImportantEnable = TRUE;
				else
					bImportant = TRUE;

				BOOL bThreadImportant = 0;
				sArchive >> (unsigned short&) bThreadImportant;
				if (bThreadImportant) {
					CString str; str.LoadString (IDS_OBSOLETE_THREAD_IMP);
					CString s2;   s2.LoadString (IDS_OBSOLETE_THREAD_IMP2);
					str += s2;
					CString strMessage; strMessage.Format (str, strRuleName);
					AfxGetMainWnd ()->MessageBox (strMessage);
				}

				BOOL bOldThrowAway = 0, bOldInTrash = 0, bOldDiscard = 0;
				sArchive >> (unsigned short&) bOldThrowAway;
				sArchive >> (unsigned short&) bOldInTrash;
				sArchive >> (unsigned short&) bOldDiscard;
				if (bOldThrowAway) {
					if (bOldInTrash) {
						bReadEnable = TRUE;
						bRead = TRUE;
					}
					else
						bDiscard = TRUE;
				}

				sArchive >> (unsigned short&) bDecode;
				sArchive >> (CString&) strUUDecodeDirectory;

				long lOldReserved;
				sArchive >> (unsigned long&) lOldReserved;
				sArchive >> (unsigned long&) lOldReserved;
				sArchive >> (unsigned long&) lOldReserved;
				sArchive >> (unsigned short&) bTag;
				sArchive >> (unsigned short&) bGetBody;
			}
			else if (GetObjectVersion () == 2) {
				sArchive >> (CString&) strRuleName;
				sArchive >> m_iID;
				sArchive >> (unsigned short&) bEnabled;
				sArchive >> (unsigned short&) bAllGroups;
				SerializeCStringArray (sArchive, rstrGroups);

				// condition
				SerializeCStringArray (sArchive, rstrCondition);
				ConvertOlderCondition (rstrCondition);

				// expiration
				unsigned long lExpiration; // old field, was never exposed in the UI
				sArchive >> (unsigned long&) lExpiration;

				// actions
				sArchive >> (unsigned short&) bPlaySound;
				sArchive >> (TPath&) pathSoundToPlay;
				sArchive >> (unsigned short&) bShowAlert;
				sArchive >> (CString&) strAlertText;
				sArchive >> (unsigned short&) bSaveToFile;
				sArchive >> (CString&) strSaveFilename;
				sArchive >> (unsigned short&) bForwardTo;
				sArchive >> (CString&) strForwardToName;

				BOOL bThreadImportant = 0;
				sArchive >> (unsigned short&) bThreadImportant;
				if (bThreadImportant) {
					CString str; str.LoadString (IDS_OBSOLETE_THREAD_IMP);
					CString s2;  s2.LoadString (IDS_OBSOLETE_THREAD_IMP2);
					str += s2;
					CString strMessage; strMessage.Format (str, strRuleName);
					AfxGetMainWnd ()->MessageBox (strMessage);
				}

				sArchive >> (unsigned short&) bDiscard;
				sArchive >> (unsigned short&) bReadEnable;
				sArchive >> (unsigned short&) bRead;
				sArchive >> (unsigned short&) bImportantEnable;
				sArchive >> (unsigned short&) bImportant;
				sArchive >> (unsigned short&) bProtectedEnable;
				sArchive >> (unsigned short&) bProtected;
				sArchive >> (unsigned short&) bDecode;
				sArchive >> (CString&) strUUDecodeDirectory;
				sArchive >> (unsigned short&) bTag;
				sArchive >> (unsigned short&) bGetBody;
				sArchive >> (unsigned short&) bAddToIgnore;
				sArchive >> (unsigned short&) bAddToWatch;
				sArchive >> (unsigned short&) bAddToBozo;
				sArchive >> (unsigned short&) bObsolete;
				sArchive >> (unsigned short&) bAddToScore;
				sArchive >> (unsigned long&) lScore;
			}
			else if (GetObjectVersion () == 3) {

				sArchive >> (CString&) strRuleName;
				sArchive >> m_iID;
				sArchive >> (unsigned short&) bEnabled;
				sArchive >> (unsigned short&) bAllGroups;
				SerializeCStringArray (sArchive, rstrGroups);

				// condition
				SerializeCStringArray (sArchive, rstrCondition);

				// expiration
				sArchive >> iExpirationDays;
				if (!iExpirationDays)
					iExpirationDays = giDefaultRuleExpirationDays;
				USHORT temp;
				sArchive >> temp;
				iExpirationType = (ExpirationType) temp;
				sArchive >> temp;
				iExpirationAction = (ExpirationAction) temp;

				// actions
				sArchive >> (unsigned short&) bPlaySound;
				sArchive >> (TPath&) pathSoundToPlay;
				sArchive >> (unsigned short&) bShowAlert;
				sArchive >> (CString&) strAlertText;
				sArchive >> (unsigned short&) bSaveToFile;
				sArchive >> (CString&) strSaveFilename;
				sArchive >> (unsigned short&) bForwardTo;
				sArchive >> (CString&) strForwardToName;
				sArchive >> (unsigned short&) bDiscard;
				sArchive >> (unsigned short&) bReadEnable;
				sArchive >> (unsigned short&) bRead;
				sArchive >> (unsigned short&) bImportantEnable;
				sArchive >> (unsigned short&) bImportant;
				sArchive >> (unsigned short&) bProtectedEnable;
				sArchive >> (unsigned short&) bProtected;
				sArchive >> (unsigned short&) bDecode;
				sArchive >> (CString&) strUUDecodeDirectory;
				sArchive >> (unsigned short&) bTag;
				sArchive >> (unsigned short&) bGetBody;
				sArchive >> (unsigned short&) bAddToIgnore;
				sArchive >> (unsigned short&) bAddToWatch;
				sArchive >> (unsigned short&) bAddToBozo;
				sArchive >> (unsigned short&) bObsolete;
				sArchive >> (unsigned short&) bAddToScore;
				sArchive >> (unsigned long&) lScore;

				for (i = 0; i < 4; i++)
					sArchive >> (unsigned short&) bObsolete;

				sArchive >> (unsigned short&) bWildGroups;
				for (i = 2; i <= 10; i++)
					sArchive >> (unsigned short&) bReserved2;

				for (i = 1; i <= 10; i++)
					sArchive >> ulReserved1;

				sArchive >> strWildGroups;
				for (i = 2; i <= 10; i++)
					sArchive >> strReserved2;
			}
			else {
				ASSERT (0);
			}
		}
		else {

			TExpirable::Serialize (sArchive);

			sArchive >> (CString&) strRuleName;
			sArchive >> m_iID;
			sArchive >> (unsigned short&) bEnabled;
			sArchive >> (unsigned short&) bAllGroups;
			SerializeCStringArray (sArchive, rstrGroups);

			// condition
			SerializeCStringArray (sArchive, rstrCondition);

			// expiration
			sArchive >> iExpirationDays;
			if (!iExpirationDays)
				iExpirationDays = giDefaultRuleExpirationDays;
			USHORT temp;
			sArchive >> temp;
			iExpirationType = (ExpirationType) temp;
			sArchive >> temp;
			iExpirationAction = (ExpirationAction) temp;

			// actions
			sArchive >> (unsigned short&) bPlaySound;
			sArchive >> (TPath&) pathSoundToPlay;
			sArchive >> (unsigned short&) bShowAlert;
			sArchive >> (CString&) strAlertText;
			sArchive >> (unsigned short&) bSaveToFile;
			sArchive >> (CString&) strSaveFilename;
			sArchive >> (unsigned short&) bForwardTo;
			sArchive >> (CString&) strForwardToName;
			sArchive >> (unsigned short&) bDiscard;
			sArchive >> (unsigned short&) bReadEnable;
			sArchive >> (unsigned short&) bRead;
			sArchive >> (unsigned short&) bImportantEnable;
			sArchive >> (unsigned short&) bImportant;
			sArchive >> (unsigned short&) bProtectedEnable;
			sArchive >> (unsigned short&) bProtected;
			sArchive >> (unsigned short&) bDecode;
			sArchive >> (CString&) strUUDecodeDirectory;
			sArchive >> (unsigned short&) bTag;
			sArchive >> (unsigned short&) bGetBody;
			sArchive >> (unsigned short&) bAddToIgnore;
			sArchive >> (unsigned short&) bAddToWatch;
			sArchive >> (unsigned short&) bAddToBozo;
			sArchive >> (unsigned short&) bAddToScore;
			sArchive >> (unsigned long&) lScore;

			sArchive >> (unsigned short&) bWildGroups;
			for (i = 2; i <= 10; i++)
				sArchive >> (unsigned short&) bReserved2;

			for (i = 1; i <= 10; i++)
				sArchive >> ulReserved1;

			sArchive >> strWildGroups;
			for (i = 2; i <= 10; i++)
				sArchive >> strReserved2;
		}
	}
}

// -------------------------------------------------------------------------
BOOL Rule::AppliesToGroup (TNewsGroup *pNG)
{
	if (bAllGroups)
		return TRUE;

	const CString &strGroup = pNG->GetName ();

	if (bWildGroups)
	{
		if (!sSearch.HasPattern ())
		{
			CString strRE;
			WildmatToRE (strWildGroups, strRE);
			sSearch.SetPattern (strRE, FALSE /* fCaseSensitive */,
				TSearch::RE /* iSearchType */);
		}

		int iResultLen;
		DWORD iPos;
		if (sSearch.Search (strGroup, iResultLen, iPos))
			return TRUE;
	}

	if (CStringArrayFind (rstrGroups, strGroup) != -1)
		return TRUE;

	return FALSE;
}

// -------------------------------------------------------------------------
void Rule::TestExpiration ()
{
	if (!bEnabled || iExpirationType == NO_EXPIRATION)
		return;

	CTimeSpan sDays (iExpirationDays /* days */, 0 /* hours */, 0 /* minutes */,
		0 /* seconds */);
	CTime sSeenCutoff = CTime::GetCurrentTime () - sDays;
	time_t lCutoff = sSeenCutoff.GetTime ();

	if (NotSeenSince (lCutoff))
		bEnabled = FALSE;
}

// -------------------------------------------------------------------------
// ApplyRules -- given a newsgroup, applies all rules which apply to a
// newsgroup to an article.  Tests each rule, and executes it if the test
// succeeds. Return value bits include (can be combined):
//   0 -- normal
//   RULES_SAYS_DISCARD -- caller is requested to discard this article, i.e.,
//        not to save it or do any further processing for it (but do cleanup
//        if needed)
//   RULES_SAYS_SAVE -- caller is requested to retrieve this article's body
//        and save it in the database, along with the header
//   RULES_SAYS_TAG -- caller is requested to tag this article for retrieval
// NOTE: we assume the group is already open
static int ApplyRules (TArticleHeader* pArtHdr, TArticleText* pArtText,
					   TNewsGroup * pNG)
{
	// I think WriteLock is needed, but does it restrict parallelism too much?
	TRuleIterator it (WriteLock);
	Rule *psRule;
	int iResult = FALSE;
	while ((psRule = it.Next ()) != 0)
		if (psRule->AppliesToGroup (pNG))
			iResult |= TestAndApplyRule (pArtHdr, pArtText, psRule, pNG);
	return iResult;
}

// -------------------------------------------------------------------------
// TestAndApplyRule -- applies a particular rule to an article
// Return value:
//   0 -- normal
//   1 -- caller is requested to discard this article, i.e., not to save it or
//        do any further processing for it (but do cleanup if needed)
//   2 -- caller is requested to retrieve this article's body and save it in
//        the database, along with the header
// NOTE: we assume the group is already open
int TestAndApplyRule (TArticleHeader* pArtHdr, TArticleText* pArtText,
					  Rule * pRule, TNewsGroup * pNG, BOOL bIgnoreDisabled /* = FALSE */,
					  BOOL *pbFiredFlag /* = NULL */, int *piActionCount /* = NULL */)
{
	try
	{
		int iResult = FALSE;

		if (!bIgnoreDisabled)
		{
			// give the rule a chance to expire
			pRule->TestExpiration ();
			if (!pRule->bEnabled)
				return 0;
		}

		DWORD dwStart = GetTickCount ();

		int iTestResult = pRule->Test (pArtHdr, pArtText, pNG);

		if (pbFiredFlag)
			*pbFiredFlag = (iTestResult == 1);

		// test this article.  If it passes, apply the rule
		if (iTestResult == 1)
			iResult = pRule->Invoke (pArtHdr, pArtText, pNG, piActionCount);

		if (iTestResult == -1)
		{
			// mark the header so that this rule is evaluated when the article body
			// becomes available
			TArtRuleInfo *pInfo =
				pNG->GetHeaderRuleInfo (pArtHdr->GetArticleNumber ());
			if (!pInfo)
				pInfo = pNG->CreateHeaderRuleInfo (pArtHdr->GetArticleNumber ());
			ASSERT (pInfo);

			pInfo->AddBodyRule (pRule->GetID ());
			pNG->RuleInfoDirty ();
		}

		pRule->m_dwTicks += GetTickCount () - dwStart;
		return iResult;
	}
	catch (TException *sException)
	{
		sException->Display ();
		CString str2; str2.LoadString (IDS_RULE_DISABLED);
		CString str; str.Format (str2, pRule->strRuleName);
		AfxMessageBox (str);
		pRule->bEnabled = FALSE;
		sException->Delete();
		return 0;
	}
}

// -------------------------------------------------------------------------
int EvaluateCondition (Condition * psCondition, TArticleHeader* pArtHdr,
					   TArticleText* pArtText, TNewsGroup *psNG)
{
	// special case -- if the condition is NULL, return TRUE (empty condition
	// is always true)
	if (!psCondition)
		return 1;

	int iFound = 0;
	int iNot, iLeft;
	TSearch &sSearch = psCondition->sSearch;

	// action depends on the type of node we're currently evaluating
	switch (psCondition->iNode)
	{
	case NOT_NODE:
		iLeft = EvaluateCondition (psCondition->psLeft, pArtHdr, pArtText, psNG);
		if (iLeft == -1)
			return -1;
		return !iLeft;
	case AND_NODE:
		iLeft = EvaluateCondition (psCondition->psLeft, pArtHdr, pArtText, psNG);
		if (iLeft == -1)
			return -1;
		if (iLeft == 0)
			return 0;       // shortcut evaluation
		return EvaluateCondition (psCondition->psRight, pArtHdr, pArtText, psNG);
	case OR_NODE:
		iLeft = EvaluateCondition (psCondition->psLeft, pArtHdr, pArtText, psNG);
		if (iLeft == -1)
			return -1;
		if (iLeft == 1)
			return 1;       // shortcut evaluation
		return EvaluateCondition (psCondition->psRight, pArtHdr, pArtText, psNG);
	case IN_REPLY_NODE:
		{
			CStringList refList;
			pArtHdr->GetReferencesStringList (&refList);
			return (gpStore->GetIDArray ()).
				IsSomeMessageIDInSet (&refList) ? 1 : 0;
		}

	case LINES_MORE_THAN:
		return pArtHdr->GetLines () > psCondition->lVal ? 1 : 0;

	case SCORE_MORE_THAN:
		return pArtHdr->GetScore () > psCondition->lVal ? 1 : 0;

	case CROSSPOSTED_NODE:
		{
			int c1 = pArtHdr->GetNumOfNewsGroups ();  // 'Newsgroups:  field

			if (c1 > psCondition->lVal)
				return 1;

			LPCTSTR  pszXRef = pArtHdr->GetXRef ();

			// count semi-colons in xrefs line
			int c2 = 0;
			while (pszXRef && *pszXRef)
			{
				if (':' == *pszXRef++)
					c2++;
			}

			if (c2 > psCondition->lVal)
				return 1;
			else
				return 0;
		}

		case CROSSPOSTED_GROUP_NODE:
			{
				CStringList grpList;
				POSITION grpPos;
				CString grp;
				pArtHdr->GetNewsGroups(&grpList);
				for (grpPos = grpList.GetHeadPosition(); grpPos; ) {
					grp = grpList.GetNext(grpPos);
					if (grp.CompareNoCase(psCondition->strPhrase) == 0)
						return 1;
				}
				// No matches.
				return 0;
			}

	case IN_WATCH_LIST_NODE:
		return gpStore->GetWatchList ().MessageInMyThreads (psNG, pArtHdr);

	case IN_IGNORE_LIST_NODE:
		return gpStore->GetIgnoreList ().MessageInMyThreads (psNG, pArtHdr);

	case POSTED_DAYS_NODE:
		{
			time_t posted = pArtHdr->GetTime ().GetTime ();
			time_t threshold =
				CTime::GetCurrentTime ().GetTime () -
				psCondition->lVal * glSecondsPerDay;
			return posted < threshold ? 1 : 0;
		}

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
		{
			TStatusUnit::EStatus bit = TStatusUnit::kNew;
			BOOL bNegative = FALSE;
			switch (psCondition->iNode)
			{
			case MARKED_AS_UNREAD:
				bit = TStatusUnit::kNew;
				break;
			case MARKED_AS_READ:
				bit = TStatusUnit::kNew;
				bNegative = TRUE;
				break;
			case MARKED_AS_IMPORTANT:
				bit = TStatusUnit::kImportant;
				break;
			case MARKED_AS_NORMAL:
				bit = TStatusUnit::kImportant;
				bNegative = TRUE;
				break;
			case MARKED_AS_PROTECTED:
				bit = TStatusUnit::kPermanent;
				break;
			case MARKED_AS_DELETABLE:
				bit = TStatusUnit::kPermanent;
				bNegative = TRUE;
				break;
			case MARKED_AS_WATCHED:
				bit = TStatusUnit::kWatch;
				break;
			case MARKED_AS_IGNORED:
				bit = TStatusUnit::kIgnore;
				break;
			case MARKED_AS_DECODE_Q:
				bit = TStatusUnit::kQDecode;
				break;
			case MARKED_AS_DECODED:
				bit = TStatusUnit::kDecoded;
				break;
			case MARKED_AS_DECODE_ERR:
				bit = TStatusUnit::kDecodeErr;
				break;
			}
			BOOL bResult = psNG->IsStatusBitOn (pArtHdr->GetArticleNumber (), bit);
			if (bNegative)
				bResult = !bResult;
			return bResult ? 1 : 0;
		}

	case MARKED_AS_LOCAL:
		return psNG->TextRangeHave (pArtHdr->GetArticleNumber ()) ? 1 : 0;

	case MARKED_AS_TAGGED:
		{
			TServerCountedPtr cpNewsServer(psNG->GetParentServer());

			TPersistentTags &rTags = cpNewsServer->GetPersistentTags ();
			return rTags.FindTag (psNG->m_GroupID,
				pArtHdr->GetArticleNumber ()) ? 1 : 0;
		}

	case SUBJECT_CONTAINS_NODE:
	case FROM_CONTAINS_NODE:
	case HEADER_CONTAINS_NODE:
	case BODY_CONTAINS_NODE:
	case SUBJECT_CONTAINS_NOT_NODE:
	case FROM_CONTAINS_NOT_NODE:
	case HEADER_CONTAINS_NOT_NODE:
	case BODY_CONTAINS_NOT_NODE:
		iNot = (psCondition->iNode == SUBJECT_CONTAINS_NOT_NODE ||
			psCondition->iNode == FROM_CONTAINS_NOT_NODE ||
			psCondition->iNode == HEADER_CONTAINS_NOT_NODE ||
			psCondition->iNode == BODY_CONTAINS_NOT_NODE);  // inverse search?

		// if condition's TSearch hasn't been compiled, do it now
		if (!psCondition->sSearch.HasPattern()) {
			psCondition->sSearch.SetPattern (psCondition->strPhrase,
				FALSE /* fCaseSensitive */,
				psCondition->bRE ? TSearch::RE : TSearch::NON_RE);
		}

		int iDummy;
		DWORD dwDummy;
		switch (psCondition->iNode)
		{
		case SUBJECT_CONTAINS_NODE:
		case SUBJECT_CONTAINS_NOT_NODE:
			iFound = (int) sSearch.Search (pArtHdr->GetSubject(),
				iDummy, dwDummy);
			break;
		case FROM_CONTAINS_NODE:
		case FROM_CONTAINS_NOT_NODE:
			iFound = (int) sSearch.Search (pArtHdr->GetFrom(), iDummy,
				dwDummy);
			break;
		case HEADER_CONTAINS_NODE:
		case HEADER_CONTAINS_NOT_NODE:
			iFound = (int) sSearch.Search (pArtHdr->GetSubject(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetFrom(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetMessageID(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetDate(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetFollowup(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetReplyTo(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetSender(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetOrganization(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetKeywords(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetDistribution(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetExpires(), iDummy, dwDummy) ||
				(int) sSearch.Search (pArtHdr->GetSummary(), iDummy, dwDummy) ;
			break;
		case BODY_CONTAINS_NODE:
		case BODY_CONTAINS_NOT_NODE:
			{
				if (!pArtText)
					return -1;      // need the body's text to evaluate the rule

				// RLW - Add the header to the body so body searches can search
				// in header text too.
				CString strText = pArtText->GetHeader() + "\n" + pArtText->GetBody();
				iFound = (int) sSearch.Search (strText, iDummy, dwDummy);
				break;
			}
		}

		// convert a non-zero value to 1
		if (iFound)
			iFound = 1;
		return iNot? !iFound : iFound;
	default:
		MsgResource (IDS_ERR_RULES_INVALID_CONDITION_NODE);
		return 0;
	}
}

// -------------------------------------------------------------------------
static BOOL ConditionContainsPercentS (Condition *psCondition, const char *&pchExistingString)
{
	if (!psCondition)
		return FALSE;

	if (psCondition->strPhrase.Find ("%s") != -1)
	{
		pchExistingString = psCondition->strPhrase;
		return TRUE;
	}

	return ConditionContainsPercentS (psCondition->psLeft, pchExistingString)
		|| ConditionContainsPercentS (psCondition->psRight, pchExistingString);
}

// -------------------------------------------------------------------------
int Rule::CompileCondition ()
{
	// if the condition tree hasn't been constructed yet, do that now
	if (!psCondition) {

		// whenever updating a rule's psCondition, update this flag too
		m_bCheckedForSubstitution = FALSE;

		int iIndex;          // for ConstructACondition
		int RC = ConstructACondition (rstrCondition, psCondition, iIndex);
		if (RC) {
			// couldn't construct a condition ?!
			bEnabled = FALSE;

			CString str1; str1.LoadString (IDS_ERR_RULE_EVAL);
			CString str; str.Format (str1, strRuleName);
			AfxMessageBox (str, MB_OK, 0);
			return 1;
		}
	}

	return 0;
}

// -------------------------------------------------------------------------
// if the rule contains "%s" in one of its condition strings, performs
// substitution. Returns 0 for success, and 1 if user chose to abort the
// substitution and disable the rule
int Rule::PerformSubstitution (TArticleHeader *pArtHdr, TNewsGroup *psNG)
{
	if (CompileCondition ())
		return 1;

	// if the compiled condition contains "%s" somewhere in it, do the
	// user-replacement
	if (!m_bCheckedForSubstitution)
	{
		const char *pchExistingString;
		if (ConditionContainsPercentS (psCondition, pchExistingString) &&
			DoRuleSubstitution (*this, pArtHdr, psNG, pchExistingString))
			// rule has already been disabled by DoRuleSubstitution()
			return 1;
		m_bCheckedForSubstitution = TRUE;
	}

	return 0;
}

// -------------------------------------------------------------------------
// Test -- tests a rule's condition, given an article's header and text.
// Returns one of the following values:
//     0 - false
//     1 - true
//    -1 - article text needed
// The third possible result would indicate that pArtText was NULL but the
// article's text was needed in order to evaluate the rule
int Rule::Test (TArticleHeader* pArtHdr, TArticleText* pArtText,
				TNewsGroup *psNG)
{
	if (CompileCondition ())
		return 0;

	if (PerformSubstitution (pArtHdr, psNG))
		return 0;

	// evaluate the condition tree
	m_iEvaluated ++;
	int iResult = EvaluateCondition (psCondition, pArtHdr, pArtText, psNG);
	if (iResult == 1)
		m_iFired ++;
	return iResult;
}

// -------------------------------------------------------------------------
static BOOL CALLBACK ShowAlertProc (HWND hDlg, UINT iMessage, WPARAM wParam,
									LPARAM lParam)
{
	switch (iMessage) {
case WM_INITDIALOG:
	// set the dialog's fields
	SendDlgItemMessage (hDlg, IDC_RULE_MESSAGE, WM_SETTEXT, 0,
		(LPARAM) (LPCTSTR) gpchRuleMessage);
	SendDlgItemMessage (hDlg, IDC_ARTICLE_AUTHOR, WM_SETTEXT, 0,
		(LPARAM) (LPCTSTR) gpchArticleAuthor);
	SendDlgItemMessage (hDlg, IDC_ARTICLE_SUBJECT, WM_SETTEXT, 0,
		(LPARAM) (LPCTSTR) gpchArticleSubject);
	SendDlgItemMessage (hDlg, IDC_ARTICLE_GROUP, WM_SETTEXT, 0,
		(LPARAM) (LPCTSTR) gpchArticleGroup);
	return 0;
	break;
case WM_COMMAND:
	if (wParam == IDOK) {
		// if the checkbox is checked, return nonzero, otherwise return zero
		EndDialog (hDlg, SendDlgItemMessage (hDlg, IDC_DISABLE_RULE_FROM_MESSAGE, BM_GETCHECK, 0, 0L));
		return 1;
	}
	break;
	}
	return 0;   // did not process the message
}

// -------------------------------------------------------------------------
static void DisplayAnAlert (QueuedAlert *psAlert)
{
	if (!psAlert->psRule->bShowAlert)
		return;

	// set global strings for the dialog box to use
	gpchRuleMessage = psAlert->psRule->strAlertText;
	gpchArticleAuthor = psAlert->strFrom;
	gpchArticleSubject = psAlert->strSubject;
	gpchArticleGroup = psAlert->strGroup;
	CWnd * psWnd = AfxGetMainWnd();        // main app window
	HWND hWnd = psWnd->GetSafeHwnd();    // main window's handle
	int RC = DialogBox (AfxGetInstanceHandle(),
		MAKEINTRESOURCE (IDD_RULE_MESSAGE), hWnd, ShowAlertProc);
	if (RC)
		psAlert->psRule->bShowAlert = FALSE;
}

// -------------------------------------------------------------------------
// Invoke -- executes the actions contained in this rule
// Return value:
//   0 -- normal
//   1 -- caller is requested to discard this article, i.e., not to save it or
//        do any further processing for it (but do cleanup if needed)
//   2 -- caller is requested to retrieve this article's body and save it in
//        the database, along with the header
// NOTE: we assume the newsgroup is already open
int Rule::Invoke (TArticleHeader* pArtHdr, TArticleText* pArtText,
				  TNewsGroup * pNG, int *piActionCount /* = NULL */)
{
	Seen ();

	int iResult = 0;
	USHORT iNewRuleFlags = 0;
	UCHAR iSaveRule = 0;
	UCHAR iForwardRule = 0;

	if (bPlaySound) {
		if (piActionCount) (*piActionCount) ++;
		PlaySoundFile (pathSoundToPlay);
	}
	if (bShowAlert) {
		if (piActionCount) (*piActionCount) ++;

		QueuedAlert *psAlert = new QueuedAlert;
		psAlert->strSubject = pArtHdr->GetSubject ();
		psAlert->strFrom = pArtHdr->GetFrom ();
		psAlert->strGroup = pNG->GetBestname ();
		psAlert->psRule = this;

		// if an hourglass icon is displayed, queue this alert
		if (giHourglassInEffect)
			QueuedDisplayList.AddTail (psAlert);
		else {
			DisplayAnAlert (psAlert);
			delete psAlert;
		}
	}
	if (bSaveToFile) {
		if (piActionCount) (*piActionCount) ++;
		// this action to be invoked from EvalRulesHaveBody_Committed ()
		iNewRuleFlags |= BODY_ACTION_SAVE_TO_FILE;
		iSaveRule = GetID ();
	}
	if (bForwardTo) {
		if (piActionCount) (*piActionCount) ++;
		// this action to be invoked from EvalRulesHaveBody_Committed ()
		iNewRuleFlags |= BODY_ACTION_FORWARD_TO;
		iForwardRule = GetID ();
	}
	if (bAddToScore) {
		if (piActionCount) (*piActionCount) ++;
		pArtHdr->AddToScore (lScore);
		pNG->HeadersDirty ();
	}
	if (bReadEnable) {
		if (piActionCount) (*piActionCount) ++;

		// dirties group automatically
		if (bRead)
			pNG->ReadRangeAdd (pArtHdr);
		else
			pNG->ReadRangeSubtract (pArtHdr, KThreadList::kHandleCrossPost);

		// tell the UI to refresh this group's read count on-screen
		gpUIPipe->NewRedrawGroup (pNG->m_GroupID);
	}
	if (bImportantEnable) {
		if (piActionCount) (*piActionCount) ++;
		pNG->StatusBitSet (pArtHdr->GetNumber(), TStatusUnit::kImportant,
			bImportant ? 1 : 0);
	}
	if (bProtectedEnable) {
		if (piActionCount) (*piActionCount) ++;
		pNG->StatusBitSet (pArtHdr->GetNumber(), TStatusUnit::kPermanent,
			bProtected ? 1 : 0);
	}
	if (bDiscard) {
		if (piActionCount) (*piActionCount) ++;
		iResult |= RULES_SAYS_DISCARD;
	}
	if (bDecode) {
		if (piActionCount) (*piActionCount) ++;
		QueueRuleArticleForDecoding (pArtHdr, pArtText, pNG,
			strUUDecodeDirectory);
	}
	if (bTag) {
		if (piActionCount) (*piActionCount) ++;
		iResult |= RULES_SAYS_TAG;
	}
	if (bGetBody) {
		if (piActionCount) (*piActionCount) ++;
		if (gpTasker->IsConnected ())
			iResult |= RULES_SAYS_SAVE;
		else
			iResult |= RULES_SAYS_TAG;
	}
	if (bAddToWatch) {
		if (piActionCount) (*piActionCount) ++;
		gpStore->GetWatchList ().Add (pNG, pArtHdr);
		pNG->StatusBitSet (pArtHdr->GetNumber(), TStatusUnit::kWatch, 1);
	}
	if (bAddToIgnore) {
		if (piActionCount) (*piActionCount) ++;
		gpStore->GetIgnoreList ().Add (pNG, pArtHdr);
		pNG->StatusBitSet (pArtHdr->GetNumber(), TStatusUnit::kIgnore, 1);
	}
	if (bAddToBozo) {
		if (piActionCount) (*piActionCount) ++;
		TBozoBin::AddBozo (pArtHdr->GetFrom ());
	}

	if (iNewRuleFlags) {
		LONG lArtNum = pArtHdr->GetArticleNumber ();
		TArtRuleInfo *pRuleInfo = pNG->GetHeaderRuleInfo (lArtNum);
		if (!pRuleInfo)
			pRuleInfo = pNG->CreateHeaderRuleInfo (lArtNum);
		ASSERT (pRuleInfo);

		// set all iSaveRule bits that are on
		for (int i = 0; i < sizeof (iNewRuleFlags) * 8; i++) {
			USHORT j = (USHORT) (1 << i);
			if (iNewRuleFlags & j)
				pRuleInfo->SetRuleFlag ((USHORT) (iNewRuleFlags & j));
		}

		if (iNewRuleFlags & BODY_ACTION_SAVE_TO_FILE)
			pRuleInfo->SetSaveRule (iSaveRule);

		if (iNewRuleFlags & BODY_ACTION_FORWARD_TO)
			pRuleInfo->SetForwardRule (iForwardRule);

		pNG->RuleInfoDirty ();
	}

	return iResult;
}

// -------------------------------------------------------------------------
void Rule::ResetSubstitution ()
{
	m_bCheckedForSubstitution = FALSE;
	FreeCondition (psCondition);
}

// -------------------------------------------------------------------------
// PlaySoundFile -- plays a sound given the name of the file
void PlaySoundFile (TPath& pathSoundFile)
{
	CString strLongName;
	CString strDir;
	SoundLongnameFromFile (pathSoundFile, strLongName, strDir);
	// if no sound specified, don't play
	CString strNone; strNone.LoadString (IDS_NONE);
	if (strLongName != "" && strLongName != strNone)
		PlaySound ((LPCTSTR) pathSoundFile, NULL, SND_FILENAME | SND_SYNC);
}

// -------------------------------------------------------------------------
// SoundFileFromLongname -- takes a sound file's path and long-name, and returns
// a CString containing the full path+filename
void SoundFileFromLongname (CString& strSoundDir, CString& strLongName,
							TPath& pathSound)
{
	CString strFile;
	CString strNone; strNone.LoadString (IDS_NONE);
	if (strLongName == strNone)
		strFile = "";
	else
		strFile = strLongName + ".WAV";
	pathSound.FormPath (strSoundDir, strFile);
}

// -------------------------------------------------------------------------
// SoundLongnameFromFile -- takes a sound file's full path, and returns the sound's
// longname and its path individually
void SoundLongnameFromFile (TPath& pathSoundToPlay, CString& strSoundName,
							CString& strSoundDir)
{
	TPath pathFile, pathDir;
	pathSoundToPlay.GetFile(pathFile);
	strSoundName = pathFile;
	// if no filename, return "(None)".  If filename exists, strip the ".wav"
	if (strSoundName == "")
		strSoundName.LoadString (IDS_NONE);
	else if (strSoundName.Find (".WAV") > 0)
		strSoundName = strSoundName.Left (strSoundName.GetLength() - 4);
	pathSoundToPlay.GetDirectory (pathDir);
	strSoundDir = pathDir;
}

// -------------------------------------------------------------------------
// CNewsApp::OnOptionsRules -- called when the Options->Rules menu item is invoked
void CNewsApp::OnOptionsRules()
{
	TRulesDlg sDlg(AfxGetMainWnd ());
	sDlg.DoModal ();
}

// -------------------------------------------------------------------------
// CNewsApp::OnOptionsManualRule -- called when the Options->Apply Rule
// Manually menu item is invoked
void CNewsApp::OnOptionsManualRule ()
{
	CNewsView *pGroupView = GetNewsView ();
	LONG lGroup = 0;
	if (pGroupView)
		lGroup = pGroupView->GetSelectedGroupID ();

	// if newsgroup view has focus, apply the rule to the selected newsgroups
	BOOL fMax;
	TNews3MDIChildWnd *pMDIChild =
		(TNews3MDIChildWnd *)
		((CMainFrame *) AfxGetMainWnd ())->MDIGetActive (&fMax);
	BOOL bSelected = (pMDIChild->GetFocusCode () == 0);

	TManualRule sManualRuleDlg (AfxGetMainWnd (), lGroup, bSelected);
	sManualRuleDlg.DoModal();
}

// -------------------------------------------------------------------------
// GetDefaultSoundDirectory -- gets the name of the default sound directory
void GetDefaultSoundDirectory (CString& strDir)
{
	strDir = "\\"; // in case things don't work out
	char rchWinDir [MAX_PATH];
	rchWinDir [0] = '\0';
	int iLen = GetWindowsDirectory (rchWinDir, sizeof (rchWinDir));
	if (iLen && (iLen <= sizeof (rchWinDir))) {
		strDir = rchWinDir;
		// GetWindowsDirectory doesn't always return a path that ends in a backslash
		if (rchWinDir [strlen (rchWinDir) - 1] != '\\')
			strDir += "\\";
		strDir += "media";
	}
}

// -------------------------------------------------------------------------
// EvalRulesHaveHeader -- called when an article's header has been downloaded.
// Tries to evaluate all rules that apply to this newsgroup.  If the rule
// evaluates to true, the actions are taken, which may add some info to the
// header for deferred actions.  If the rule requires seeing the body, the
// header is marked so that the rule is re-evaluated when the body is
// downloaded
// Parameters:
//   TArticleHeader * pHeader -- the article's header
//   TNewsGroup * pNG -- the newsgroup which the article is in... is this needed??
// Return value bits include (can be combined):
//   0 -- normal
//   RULES_SAYS_DISCARD -- caller is requested to discard this article, i.e.,
//        not to save it or do any further processing for it (but do cleanup
//        if needed)
//   RULES_SAYS_SAVE -- caller is requested to retrieve this article's body
//        and save it in the database, along with the header
//   RULES_SAYS_TAG -- caller is requested to tag this article for retrieval
int EvalRulesHaveHeader (TArticleHeader * pHeader, TNewsGroup * pNG)
{
	DWORD dwStart = GetTickCount ();
	Rules &rules = *GetGlobalRules ();
	rules.WriteLock ();

	// make sure the newsgroup is open
	pNG->Open ();

	int iResult = ApplyRules (pHeader, NULL, pNG);

	// if we are to discard this article, do the following cleanup:
	//   1. cancel the download request for the article's body
	//   2. add the message ID to the range of messages retrieved for
	//      this group, so that they won't be retrieved again
	if (iResult & RULES_SAYS_DISCARD) {
		int iArticleNumber = pHeader->GetArticleNumber ();

		gpTasker->CancelArticleBody (pNG->GetName(), iArticleNumber);
		pNG->ReadRangeAdd (pHeader);  // dirties group automatically
	}

	pNG->Close ();

	rules.m_dwTicks += GetTickCount () - dwStart;
	rules.UnlockWrite ();
	return iResult;
}

// -------------------------------------------------------------------------
// EvalRulesHaveBody -- called when an article's body has been downloaded.
// Checks the article's header to see whether any rules needed to see the
// body. If there are any such rules, then these rules are evaluated and
// invoked if needed.  Parameters:
//   TArticleText * pBody -- the article's text
//   int iArticleNum -- the article's number (can retrieve the header with this)
//   TNewsGroup * pNG -- the newsgroup which the article is in... is this needed??
// Return value bits include (can be combined):
//   0 -- normal
//   RULES_SAYS_DISCARD -- caller is requested to discard this article, i.e.,
//        not to save it or do any further processing for it (but do cleanup
//        if needed)
//   RULES_SAYS_SAVE -- caller is requested to retrieve this article's body
//        and save it in the database, along with the header
//   RULES_SAYS_TAG -- caller is requested to tag this article for retrieval
int EvalRulesHaveBody (TArticleHeader * pHeader, TArticleText * pBody,
					   TNewsGroup * pNG, Rule *pOnlyThisRule /* = NULL */,
					   BOOL bIgnoreDisabled /* = FALSE */, BOOL *pbFiredFlag /* = NULL */,
					   int *piActionCount /* = NULL */)
{
	DWORD dwStart = GetTickCount ();
	Rules &rules = *GetGlobalRules ();
	rules.WriteLock ();

	int iResult = FALSE;
	LONG lArtNum = pHeader->GetArticleNumber ();
	TArtRuleInfo *pRuleInfo = pNG->GetHeaderRuleInfo (lArtNum);
	USHORT iOriginalRuleFlags =
		(USHORT) (pRuleInfo ? pRuleInfo->GetRuleFlags () : 0);

	// make sure the newsgroup is open
	pNG->Open ();

	if (!pOnlyThisRule) {
		if (pRuleInfo) {
			// all rules
			UCHAR *riRules = pRuleInfo->GetBodyRules ();
			Rule *pRule;

			for (int i = 0; i < MAX_BODY_RULES; i++)
				if (riRules [i]) {
					pRule = rules.RuleFromID (riRules [i]);
					if (pRule)
						iResult |= TestAndApplyRule (pHeader, pBody, pRule, pNG,
						bIgnoreDisabled, pbFiredFlag, piActionCount);
				}

				// clear the body-rules field after going through the rules
				pRuleInfo->ClearBodyRules ();
				pNG->RuleInfoDirty ();
		}
	}
	else
		// a specific rule only
		iResult = TestAndApplyRule (pHeader, pBody, pOnlyThisRule, pNG,
		bIgnoreDisabled, pbFiredFlag, piActionCount);

	TNewsGroup::EStorageOption iMode = UtilGetStorageOption (pNG);

	// could be that there is now rule info for the header
	if (!pRuleInfo)
		pRuleInfo = pNG->GetHeaderRuleInfo (lArtNum);

	if (pRuleInfo && iOriginalRuleFlags != pRuleInfo->GetRuleFlags ())
		pNG->RuleInfoDirty ();

	// if we are to discard this article, do the following cleanup:
	//   1. delete the header from the database (is this okay to do ??)
	//   2. add the message ID to the range of messages retrieved for
	//      this group, so that they won't be retrieved again

	if (iResult & RULES_SAYS_DISCARD)
	{
		int iArtNum = pHeader->GetArticleNumber ();
		if (TNewsGroup::kHeadersOnly == iMode)
			pNG->PurgeHeader (iArtNum);
		else if (TNewsGroup::kStoreBodies == iMode)
		{
			pNG->PurgeHeader (iArtNum);
			pNG->PurgeBody (iArtNum);
		}
		pNG->ReadRangeAdd (pHeader);

		// erase header rule info
		pNG->RemoveHeaderRuleInfo ( iArtNum );

		// don't forget to get rid of the status
		pNG->StatusDestroy (iArtNum);
		pNG->SetDirty ();
	}

	pNG->Close ();
	rules.m_dwTicks += GetTickCount () - dwStart;
	rules.UnlockWrite ();
	return iResult;
}

// -------------------------------------------------------------------------
// This is called after body is saved to the database
int EvalRulesHaveBody_Committed (TArticleHeader * pHeader,
								 TArticleText * pBody, TNewsGroup * pNG)
{
	LONG lArtNum = pHeader->GetArticleNumber ();
	TArtRuleInfo *pRuleInfo = pNG->GetHeaderRuleInfo (lArtNum);

	if (!pRuleInfo)
		return 0;

	DWORD dwStart = GetTickCount ();
	Rules &rules = *GetGlobalRules ();
	rules.WriteLock ();

	if (pRuleInfo->TestRuleFlag (BODY_ACTION_SAVE_TO_FILE)) {
		Rule *pRule = rules.RuleFromID (pRuleInfo->GetSaveRule ());
		if (pRule) {
			DWORD dwStart = GetTickCount ();
			SaveToFileAction (pHeader, pBody, pRule->strSaveFilename);
			pRule->m_dwTicks += GetTickCount () - dwStart;
		}
		pRuleInfo->ClearRuleFlag (BODY_ACTION_SAVE_TO_FILE);
		pNG->RuleInfoDirty ();
	}

	if (pRuleInfo->TestRuleFlag (BODY_ACTION_FORWARD_TO)) {
		Rule *pRule = rules.RuleFromID (pRuleInfo->GetForwardRule ());
		if (pRule) {
			DWORD dwStart = GetTickCount ();
			ForwardArticle (pRule->strForwardToName, pHeader, pBody, pNG);
			pRule->m_dwTicks += GetTickCount () - dwStart;
		}
		pRuleInfo->ClearRuleFlag (BODY_ACTION_FORWARD_TO);
		pNG->RuleInfoDirty ();
	}

	rules.m_dwTicks += GetTickCount () - dwStart;
	rules.UnlockWrite ();
	return 0;
}

// -------------------------------------------------------------------------
// CStringArrayFind -- gives the index of a CString in a CStringArray, or -1
// if it's not there
int CStringArrayFind (const CStringArray& rstr, const CString& str)
{
	int iMax = rstr.GetSize();
	for (int i = 0; i < iMax; i++)
		if (str == rstr [i])
			return i;
	return -1;
}

// -------------------------------------------------------------------------
// ResetAndPerformRuleSubstitution -- performs rule substitution for a
// specific rule, or for all enabled rules
void ResetAndPerformRuleSubstitution (TArticleHeader *psHeader,
									  TNewsGroup *psNG, Rule *psRule /* = NULL */)
{
	ResetAllRuleSubstitution ();

	if (psRule)
		psRule->ResetSubstitution ();
	else {
		TRuleIterator it (*GetGlobalRules(), WriteLock);
		while ((psRule = it.Next ()) != 0)
			if (psRule->bEnabled)
				// perform the substitution
				psRule->PerformSubstitution (psHeader, psNG);
	}
}

// -------------------------------------------------------------------------
// ResetAllRuleSubstitution -- reset each rule's substitution state
void ResetAllRuleSubstitution ()
{
	TRuleIterator it (WriteLock);
	Rule *psRule;
	while ((psRule = it.Next ()) != 0)
		psRule->ResetSubstitution ();
}

// -------------------------------------------------------------------------
void RulesHourglassStart ()
{
	giHourglassInEffect = TRUE;
}

// -------------------------------------------------------------------------
void RulesHourglassEnd ()
{
	giHourglassInEffect = FALSE;

	// display and free all queued display events
	QueuedAlert *psAlert;
	while (!QueuedDisplayList.IsEmpty ()) {
		psAlert = (QueuedAlert *) QueuedDisplayList.RemoveHead ();
		DisplayAnAlert (psAlert);
		delete psAlert;
	}
}

// -------------------------------------------------------------------------
Rules::Rules(void)
{
	m_dwTicks = 0;
	m_iNextID = 0;
}

// -------------------------------------------------------------------------
Rules::~Rules ()
{
	FreeAll ();
}

// -------------------------------------------------------------------------
// FreeAll -- free all rules in the rule collection
void Rules::FreeAll ()
{
	TRuleIterator it (*this, ::WriteLock);
	Rule *psRule;
	while ((psRule = it.Next ()) != 0)
		delete psRule;

	RemoveAll ();
}

// -------------------------------------------------------------------------
Rules& Rules::operator= (Rules& sSource)
{
	WriteLock ();

	// free the existing rules
	FreeAll ();

	// go through the source's rule list, copying each one
	TRuleIterator it (sSource, ::ReadLock);
	Rule *psRule;
	while ((psRule = it.Next ()) != 0) {
		Rule *psNewRule = new Rule (this);
		*psNewRule = *psRule;
		SetAt (psNewRule->strRuleName, psNewRule);
	}

	m_iNextID = sSource.m_iNextID;
	m_dwTicks = sSource.m_dwTicks;

	UnlockWrite ();
	return *this;
}

// -------------------------------------------------------------------------
// SubjectContainsString -- constructs a string that says "subject contains
// <phrase>"
static void SubjectContainsString (int iIndent, int bTrue, CString &str,
								   LPCTSTR pchPhrase, BOOL bRE = FALSE)
{
	str = "";
	for (int i = 0; i < iIndent; i++)
		str += " ";

	CString strTemp;
	strTemp.LoadString (IDS_SUBJECT);
	str += strTemp + " ";

	if (bTrue)
		strTemp.LoadString (IDS_CONTAINS);
	else {
		CString strWord;
		strWord.LoadString (IDS_DOES); strTemp = strWord + " ";
		strWord.LoadString (IDS_LOWERCASE_NOT); strTemp += strWord + " ";
		strWord.LoadString (IDS_CONTAIN); strTemp += strWord;
	}

	CString strRE;
	if (bRE) {
		CString str;
		str.LoadString (IDS_REG); strRE += str + " ";
		str.LoadString (IDS_EXPR); strRE += str + " ";
	}

	str += strTemp + " " + strRE + "\"" + pchPhrase + "\"";
}

// -------------------------------------------------------------------------
// CreateInitialRules -- returns a rule suitable as a default rule
void Rules::CreateInitialRules ()
{
	WriteLock ();

	CString str, str1, str2, str3;

	// default important-word-in-subject rule
	Rule *pRule = new Rule (this);

	pRule->bEnabled = FALSE;
	pRule->strRuleName.LoadString (IDS_IMPORTANT_WORD_IN_SUBJECT_RULE_NAME);
	pRule->bImportantEnable = TRUE;
	pRule->bImportant = TRUE;

	SubjectContainsString (0 /* iIndent */, TRUE /* bTrue */, str, "%s");
	pRule->rstrCondition.Add (str);

	SetAt (pRule->strRuleName, pRule);

	// default ignore-the-spam rule
	pRule = new Rule (this);

	pRule->bEnabled = FALSE;
	pRule->strRuleName.LoadString (IDS_IGNORE_SPAM_RULE_NAME);
	pRule->bReadEnable = pRule->bRead = TRUE;

	str1.LoadString (IDS_CROSSPOSTED);
	str2.LoadString (IDS_TO);
	str3.LoadString (IDS_GROUPS);
	str.Format ("%s %s > %d %s", str1, str2, 5, str3);
	pRule->rstrCondition.Add (str);

	SetAt (pRule->strRuleName, pRule);

	UnlockWrite ();
}

// -------------------------------------------------------------------------
void Rules::Serialize (CArchive& sArchive)
{
	if (sArchive.IsStoring ()) {
		sArchive << m_iNextID;

		// store the number of rules, then store each rule
		sArchive << (unsigned int) GetCount ();

		TRuleIterator it (*this, ::ReadLock);
		Rule *psRule;
		while ((psRule = it.Next ()) != 0)
			psRule->Serialize (sArchive);
	}
	else {
		sArchive >> m_iNextID;

		// empty out
		FreeAll ();

		// retrieve the number of rules, then retrieve each rule
		unsigned int uiCount;
		sArchive >> uiCount;

		Rule *psRule;
		for (unsigned int i = 0; i < uiCount; i++) {
			psRule = new Rule;
			psRule->Serialize (sArchive);
			SetAt (psRule->strRuleName, psRule);
		}
	}
}

// -------------------------------------------------------------------------
Rule *Rules::RuleFromID (UCHAR iID)
{
	Rule *pRule;
	TRuleIterator it (*this, ::ReadLock);
	while ((pRule = it.Next ()) != 0)
		if (pRule->GetID () == iID)
			return pRule;
	return NULL;
}

// -------------------------------------------------------------------------
// Lookup -- looks up a rule in the rule list. Returns TRUE for success, FALSE
// for failure
BOOL Rules::Lookup (LPCTSTR pchName, Rule *&pRule)
{
	POSITION pos = GetHeadPosition ();
	while (pos) {
		Rule *pCurrentRule = GetNext (pos);
		if (pCurrentRule->strRuleName == pchName) {
			pRule = pCurrentRule;
			return TRUE;
		}
	}
	return FALSE;
}

// -------------------------------------------------------------------------
// SetAt -- adds a rule to the rule collection
void Rules::SetAt (LPCTSTR pchName, Rule *pRule)
{
#ifdef _DEBUG
	Rule *pDummy;
	ASSERT (!Lookup (pchName, pDummy));
#endif   // DEBUG

	AddTail (pRule);
}

// -------------------------------------------------------------------------
// RemoveKey -- removes a rule from the collection
void Rules::RemoveKey (LPCTSTR pchName)
{
	POSITION pos = GetHeadPosition ();
	while (pos) {
		POSITION oldPos = pos;
		Rule *pRule = GetNext (pos);
		if (pRule->strRuleName == pchName) {
			RemoveAt (oldPos);
			return;
		}
	}
	ASSERT (0);
}
