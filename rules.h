/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rules.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:39  richard_wood
/*  Tidying up formatting
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

// rules.h -- class definitions of the Rule and Rules classes

#pragma once

// disable synonym warning
#pragma warning (disable : 4097)

#include "pobject.h"             // PObject
#include "sharesem.h"            // TSynchronizable
#include "mplib.h"               // TPath
#include "tsearch.h"             // TSearch
#include "expirable.h"           // TExpirable

// version of this Rule data structure
#define RULE_VERSION_NUMBER   4
#define RULES_VERSION_NUMBER  1

// possible return values (can be combined) for EvalRulesHaveHeader() and
// EvalRulesHaveBody()
#define RULES_SAYS_DISCARD    0x0001
#define RULES_SAYS_SAVE       0x0002
#define RULES_SAYS_TAG        0x0004

struct Condition;
class Rule;
class TNewsGroup;
class TArticleHeader;
class TArticleText;

typedef CList <Rule*,Rule*&> TRulesType;

// -------------------------------------------------------------------------
class Rules : public TRulesType, public TSynchronizable {

   friend class Rule;

public:
   Rules();
   Rules& operator= (Rules& source);
   ~Rules ();
   BOOL Lookup (LPCTSTR pchName, Rule *&pRule);
   void SetAt (LPCTSTR pchName, Rule *pRule);
   void RemoveKey (LPCTSTR pchName);
   void Serialize (CArchive& sArchive);   // read/write the object
   void CreateInitialRules ();
   Rule *RuleFromID (UCHAR iID);
   int Export (CString &strFile);
   int Import (CString &strFile, int &iLine, BOOL bDeleteOriginal);

   DWORD m_dwTicks;              // milliseconds spent processing rules

protected:
   void FreeAll ();
   UCHAR m_iNextID;              // next ID assigned to a new rule
};

// -------------------------------------------------------------------------
// Rule -- represents a single rule
class Rule : public PObject, public TExpirable {
public:
   Rule::Rule (Rules *psRuleCollection = NULL);
   Rule::~Rule ();
   void operator= (const Rule &source);
   int CompileCondition ();
   int PerformSubstitution (TArticleHeader *pArtHdr, TNewsGroup *psNG);
   int Test (TArticleHeader* pArtHdr, TArticleText* pArtText,
      TNewsGroup *psNG);
   int Invoke (TArticleHeader* pArtHdr, TArticleText* pArtText,
      TNewsGroup * pNG, int *piActionCount = NULL);
   void Serialize (CArchive& sArchive);
   void ResetSubstitution ();
   UCHAR GetID () { return m_iID; }
   void Export (CFile &file);
   int Import (CStdioFile &file, int &iLine);
   int ImportGroups (CStdioFile &file, int &iLine);
   int ImportCondition (CStdioFile &file, int &iLine);
   int ImportActions (CStdioFile &file, int &iLine);
   BOOL AppliesToGroup (TNewsGroup *pNG);
   void TestExpiration ();

   DECLARE_SERIAL (Rule)

public:
   BOOL m_bCheckedForSubstitution;
   DWORD m_dwTicks;        // milliseconds spent processing this rule
   int m_iEvaluated;       // number of times evaluated
   int m_iFired;           // number of times fired

// private?
public:

   UCHAR m_iID;            // unique ID for this rule
   CString strRuleName;    // rule's name
   BOOL bEnabled;          // is the rule enabled?
   BOOL bAllGroups;        // rule applies to all groups?
   CStringArray rstrGroups;// array of names of the groups which this
                           // rule applies to
   BOOL bWildGroups;       // use group wildcard specification
   CString strWildGroups;  // group wildcard specification

   // condition fields
   CStringArray rstrCondition;   // array of CStrings containing the condition
   Condition * psCondition;   // condition tree for this rule

   // expiration
   USHORT iExpirationDays;
   enum ExpirationType { NO_EXPIRATION, DAYS_AFTER_USED, DAYS_AFTER_CREATE };
   ExpirationType iExpirationType;
   enum ExpirationAction { DISABLE, REMOVE };
   ExpirationAction iExpirationAction;

   // action fields
   BOOL bPlaySound;        // play sound?
   TPath pathSoundToPlay;  // sound file to play
   BOOL bShowAlert;        // show an alert dialog?
   CString strAlertText;   // alert text
   BOOL bSaveToFile;       // save article?
   CString strSaveFilename;// file to save to
   BOOL bForwardTo;        // forward to someone?
   CString strForwardToName;  // username
   BOOL bDecode;           // decode article?
   CString strUUDecodeDirectory; // dirctory to decode into
   BOOL bAddToScore;       // add to article's score?
   long lScore;            // amount to add to article's score
   BOOL bTag;              // tag article for retrieval?
   BOOL bGetBody;          // get article body?
   BOOL bDiscard;          // discard?
   BOOL bReadEnable;       // set read or set unread?
   BOOL bRead;             // set read?
   BOOL bImportantEnable;  // set important or set normal?
   BOOL bImportant;        // set important?
   BOOL bProtectedEnable;  // set protected or set deletable?
   BOOL bProtected;        // set protected?
   BOOL bAddToIgnore;      // add to ignore list
   BOOL bAddToWatch;       // add to watch list
   BOOL bAddToBozo;        // add to bozo bin

   BOOL bReserved2;
   BOOL bReserved3;
   BOOL bReserved4;
   BOOL bReserved5;
   BOOL bReserved6;
   BOOL bReserved7;
   BOOL bReserved8;
   BOOL bReserved9;
   BOOL bReserved10;
   ULONG ulReserved1;
   ULONG ulReserved2;
   ULONG ulReserved3;
   ULONG ulReserved4;
   ULONG ulReserved5;
   ULONG ulReserved6;
   ULONG ulReserved7;
   ULONG ulReserved8;
   ULONG ulReserved9;
   ULONG ulReserved10;
   CString strReserved2;
   CString strReserved3;
   CString strReserved4;
   CString strReserved5;
   CString strReserved6;
   CString strReserved7;
   CString strReserved8;
   CString strReserved9;
   CString strReserved10;

private:
   TSearch sSearch;              // compiled strWildGroups
};

// -------------------------------------------------------------------------
int TestAndApplyRule (TArticleHeader* pArtHdr, TArticleText* pArtText,
   Rule * pRule, TNewsGroup * pNG, BOOL bIgnoreDisabled = FALSE,
   BOOL *pbFiredFlag = NULL, int *piActionCount = NULL);
void PlaySoundFile (TPath& pathSoundFile);
void GetDefaultSoundDirectory (CString& strDir);
void SoundFileFromLongname (CString& strSoundDir, CString& strLongName,
   TPath& pathSound);
void SoundLongnameFromFile (TPath& pathSoundToPlay, CString& strSoundName,
   CString& strSoundDir);
int EvalRulesHaveHeader (TArticleHeader * pHeader, TNewsGroup * pNG);
int EvalRulesHaveBody (TArticleHeader * pHeader, TArticleText * pBody,
   TNewsGroup * pNG, Rule *pOnlyThisRule = NULL,
   BOOL bIgnoreDisabled = FALSE, BOOL *pbFiredFlag = NULL,
   int *piActionCount = NULL);
int EvalRulesHaveBody_Committed(TArticleHeader * pHeader, TArticleText * pBody, TNewsGroup * pNG);
int CStringArrayFind (const CStringArray& rstr, const CString& str);
void ResetAndPerformRuleSubstitution (TArticleHeader *psHeader,
   TNewsGroup *psNG, Rule *psRule = NULL);
void ResetAllRuleSubstitution ();
void RulesHourglassStart ();
void RulesHourglassEnd ();
int EvaluateCondition (Condition * psCondition, TArticleHeader* pArtHdr,
   TArticleText* pArtText, TNewsGroup *psNG);

#include "globals.h"             // gpStore... can't go above though because
                                 // of circularity
#define GetGlobalRules() (gpStore->GetRuleList ())
