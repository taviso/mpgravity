/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thrdact.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:04  richard_wood
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

// thrdact.cpp -- actions on threads, e.g., kill and watch

#include "stdafx.h"              // precompiled header
#include "resource.h"            // ID*
#include "thrdact.h"             // TThreadAction, ...
#include "newsdb.h"              // TNewsDB
#include "server.h"              // needed by someone
#include "servcp.h"              // TServerCountedPtr
#include "globals.h"             // gpStore
#include "rules.h"               // GetGlobalRules(), ...
#include "ruleutil.h"            // GetRule()
#include "genutil.h"             // MsgResource()
#include "tmrbar.h"              // gpsManualRuleBar

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const ULONG glSecondsPerDay = (ULONG) 60 * 60 * 24;

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
IMPLEMENT_SERIAL (TIntegerList, PObject, TINTEGER_LIST_VERSION_NUMBER)

// -------------------------------------------------------------------------
TIntegerList::TIntegerList ()
: PObject (TINTEGER_LIST_VERSION_NUMBER)
{
}

// -------------------------------------------------------------------------
TIntegerList &TIntegerList::operator= (const TIntegerList &src)
{
	m_sList.RemoveAll ();

	POSITION pos = src.GetHeadPosition ();
	while (pos) {
		LONG lInteger = src.GetNext (pos);
		AddTail (lInteger);
	}

	return *this;
}

// -------------------------------------------------------------------------
void TIntegerList::Serialize (CArchive &archive)
{
	PObject::Serialize (archive);

	if (archive.IsStoring ()) {
		// write number of integers followed by the integers
		archive << (int) m_sList.GetCount ();
		POSITION pos = GetHeadPosition ();
		while (pos)
			archive << (LONG) GetNext (pos);
	}
	else {
		if (GetObjectVersion () < TINTEGER_LIST_VERSION_NUMBER) {
			// it's an older version, here we need to read the object in its
			// original format then convert it...
			ASSERT (0);
		}
		else {
			int iCount;
			archive >> iCount;
			for (int i = 0; i < iCount; i++) {
				LONG lInteger;
				archive >> lInteger;
				AddTail (lInteger);
			}
		}
	}
}

// -------------------------------------------------------------------------
TThreadActionItem::TThreadActionItem ()
: PObject (TTHREAD_ACTION_ITEM_VERSION_NUMBER)
{
}

// -------------------------------------------------------------------------
TThreadActionItem::TThreadActionItem (const CString &subject,
									  const CStringList &references, const CTime &sLastSeen, LONG lGroupID,
									  LONG lArticleNum)
									  : PObject (TTHREAD_ACTION_ITEM_VERSION_NUMBER), TExpirable (sLastSeen)
{
	m_strSubject = subject;
	CopyCStringList (m_sReferences, references);
	AddArticleNum (lGroupID, lArticleNum);
}

// -------------------------------------------------------------------------
TThreadActionItem::TThreadActionItem (const TThreadActionItem &src)
: PObject (TTHREAD_ACTION_ITEM_VERSION_NUMBER)
{
	*this = src;
}

// -------------------------------------------------------------------------
TThreadActionItem &TThreadActionItem::operator= (const TThreadActionItem &src)
{
	TExpirable::operator= (src);
	m_strSubject = src.m_strSubject;
	CopyCStringList (m_sReferences, src.m_sReferences);
	m_sArticleNums = src.m_sArticleNums;
	return *this;
}

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
// the VERSIONABLE_SCHEMA thing is needed because it's being serialized through
// a CObList
IMPLEMENT_SERIAL (TThreadActionItem, PObject,
				  VERSIONABLE_SCHEMA | TTHREAD_ACTION_ITEM_VERSION_NUMBER)

				  // -------------------------------------------------------------------------
				  void TThreadActionItem::Serialize (CArchive &archive)
{
	PObject::Serialize (archive);

	if (archive.IsStoring ()) {
		TExpirable::Serialize (archive);
		archive << m_strSubject;
		m_sReferences.Serialize (archive);
		m_sArticleNums.Serialize (archive);
	}
	else {
		if (GetObjectVersion () < TTHREAD_ACTION_ITEM_VERSION_NUMBER) {
			// it's an older version, here we need to read the object in its
			// original format then convert it...
			if (GetObjectVersion () == 1) {
				ULONG lTime;
				archive >> lTime;
				SetLastSeen (lTime);
				archive >> m_strSubject;
				m_sReferences.Serialize (archive);
			}
			else if (GetObjectVersion () == 2) {
				ULONG lTime;
				archive >> lTime;
				SetLastSeen (lTime);
				archive >> m_strSubject;
				m_sReferences.Serialize (archive);
				m_sArticleNums.Serialize (archive);
			}
			else
				ASSERT (0);
		}
		else {
			TExpirable::Serialize (archive);
			archive >> m_strSubject;
			m_sReferences.Serialize (archive);
			m_sArticleNums.Serialize (archive);
		}
	}
}

// -------------------------------------------------------------------------
BOOL TThreadActionItem::ReferenceInMyThread (LPCTSTR pchReference)
{
	POSITION pos = m_sReferences.GetHeadPosition ();
	while (pos) {
		const CString &strReference = m_sReferences.GetNext (pos);
		if (strReference == pchReference)
			return TRUE;
	}

	return FALSE;
}

// -------------------------------------------------------------------------
BOOL TThreadActionItem::SubjectInMyThread (LPCTSTR pchSubject)
{
	return m_strSubject == pchSubject;
}

// -------------------------------------------------------------------------
void TThreadActionItem::RefsIDontHave (LPCTSTR pchArticleID,
									   const TFlatStringArray &references, CStringList &sNewRefs)
{
	sNewRefs.RemoveAll ();

	int iLen = references.GetSize ();
	for (int i = -1; i < iLen; i++) {
		LPCTSTR pchReference =
			(i == -1 ? pchArticleID : references.get_string (i));
		if (!m_sReferences.Find (pchReference))
			sNewRefs.AddHead (pchReference);
	}
}

// -------------------------------------------------------------------------
void TThreadActionItem::AugmentReferences (CStringList &sNewRefs)
{
	m_sReferences.AddTail (&sNewRefs);
}

// -------------------------------------------------------------------------
BOOL TThreadActionItem::HaveArticleNum (LONG lGroupID, LONG lArticleNum)
{
	POSITION pos = m_sArticleNums.GetHeadPosition ();
	while (pos) {
		LONG lListGroupID = m_sArticleNums.GetNext (pos);
		LONG lListArticleNum = m_sArticleNums.GetNext (pos);
		if (lListGroupID == lGroupID && lListArticleNum == lArticleNum)
			return TRUE;
	}

	return FALSE;
}

// -------------------------------------------------------------------------
void TThreadActionItem::AddArticleNum (LONG lGroupID, LONG lArticleNum)
{
	if (!HaveArticleNum (lGroupID, lArticleNum)) {
		m_sArticleNums.AddHead (lArticleNum);
		m_sArticleNums.AddHead (lGroupID);
	}
}

// -------------------------------------------------------------------------
void TThreadActionItem::RemoveArticleIcons ()
{
	TServerCountedPtr cpNewsServer;        // smart pointer

	// go through the item's articles and remove the watch/ignore icon from each
	POSITION pos = m_sArticleNums.GetHeadPosition ();
	while (pos) {
		LONG lGroupID = m_sArticleNums.GetNext (pos);
		LONG lArticleNum = m_sArticleNums.GetNext (pos);

		BOOL fUseLock;
		TNewsGroup *pNG = 0;
		TNewsGroupUseLock useLock (cpNewsServer, lGroupID, &fUseLock, pNG);
		if (!fUseLock) {
			ASSERT (0);
			return;
		}

		pNG -> StatusBitSet (lArticleNum, TStatusUnit::kWatch, 0);
		pNG -> StatusBitSet (lArticleNum, TStatusUnit::kIgnore, 0);
	}
}

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
IMPLEMENT_SERIAL (TThreadActionItemList, CObList, TTHREAD_ACTION_LIST_VERSION_NUMBER)

// -------------------------------------------------------------------------
void TThreadActionItemList::RemoveAll ()
{
	POSITION pos = GetHeadPosition ();
	while (pos) {
		TThreadActionItem *pItem = (TThreadActionItem *) CObList::GetNext (pos);
		delete pItem;
	}
	CObList::RemoveAll ();
}

// -------------------------------------------------------------------------
TThreadActionList::TThreadActionList ()
: PObject (TTHREAD_ACTION_LIST_VERSION_NUMBER)
{
	m_bDirty = FALSE;
	m_iDays = 7;            // default to 7 days
	m_bTestSubjects = TRUE; // default to treating identical subjects as
	// belonging to same thread
}

// -------------------------------------------------------------------------
TThreadActionList::~TThreadActionList ()
{
	m_sList.RemoveAll ();
}

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
IMPLEMENT_SERIAL (TThreadActionList, PObject, TTHREAD_ACTION_LIST_VERSION_NUMBER)

// -------------------------------------------------------------------------
void TThreadActionList::Serialize (CArchive &archive)
{
	PObject::Serialize (archive);

	if (archive.IsStoring ()) {
		archive << m_iDays;
		archive << m_bTestSubjects;
		archive << m_bDirty;
		m_sList.Serialize (archive);
		m_sIDs.Serialize (archive);
		m_sSubjects.Serialize (archive);
	}
	else {
		if (GetObjectVersion () < TTHREAD_ACTION_LIST_VERSION_NUMBER) {
			// it's an older version, here we need to read the object in its
			// original format then convert it...
			ASSERT (0);
		}
		else {
			archive >> m_iDays;
			archive >> m_bTestSubjects;

			// NOTE: shouldn't be serializing m_bDirty.  Remove it from the
			// serialize function when we make the next version
			archive >> m_bDirty;
			m_bDirty = FALSE;

			m_sList.Serialize (archive);
			m_sIDs.Serialize (archive);
			m_sSubjects.Serialize (archive);
		}
	}
}

// -------------------------------------------------------------------------
void TThreadActionList::AddRefsToArray (const CStringList &sList)
{
	POSITION pos = sList.GetHeadPosition ();
	while (pos) {
		const CString &str = sList.GetNext (pos);
		int iIndex;
		IDInMyThreads (str, iIndex);
		m_sIDs.InsertAt (iIndex, str);
	}
}

// -------------------------------------------------------------------------
void TThreadActionList::RemoveRefsFromArray (const CStringList &sList)
{
	POSITION pos = sList.GetHeadPosition ();
	while (pos) {
		const CString &str = sList.GetNext (pos);
		int iIndex;
		BOOL bResult = IDInMyThreads (str, iIndex);
		ASSERT (bResult);
		if (bResult)
			m_sIDs.RemoveAt (iIndex);
	}
}

// -------------------------------------------------------------------------
void TThreadActionList::AddSubjectToArray (const CString &strSubject)
{
	int iIndex;
	SubjectInMyThreads (strSubject, iIndex);
	m_sSubjects.InsertAt (iIndex, strSubject);
}

// -------------------------------------------------------------------------
void TThreadActionList::RemoveSubjectFromArray (const CString &strSubject)
{
	int iIndex;
	BOOL bResult = SubjectInMyThreads (strSubject, iIndex);
	ASSERT (bResult);
	if (bResult)
		m_sSubjects.RemoveAt (iIndex);
}

// -------------------------------------------------------------------------
static void CheckRuleExists (TThreadActionList *pList)
{
	BOOL bIgnore = (pList == &gpStore -> GetIgnoreList ());

	GetGlobalRules () -> WriteLock ();

	CString strRule;
	strRule.LoadString (bIgnore ? IDS_IGNORE_RULE_NAME : IDS_WATCH_RULE_NAME);
	Rule *pRule = GetRule (strRule);
	if (pRule) {
		GetGlobalRules () -> UnlockWrite ();
		return;
	}

	pRule = new Rule;
	if (!pRule) {
		MsgResource (IDS_ERR_OUT_OF_MEMORY);
		GetGlobalRules () -> UnlockWrite ();
		return;
	}
	pRule -> strRuleName = strRule;
	pRule -> bEnabled = TRUE;

	// condition
	CString strItem, str;
	strItem.LoadString (IDS_IN);
	str.LoadString (bIgnore ? IDS_AN : IDS_A);
	strItem += CString (" ") + str + " ";
	str.LoadString (bIgnore ? IDS_IGNORED : IDS_WATCHED);
	strItem += str + " ";
	str.LoadString (IDS_THREAD);
	strItem += str;
	pRule -> rstrCondition.Add (strItem);

	// actions
	if (bIgnore) {
		pRule -> bReadEnable = TRUE;
		pRule -> bRead = TRUE;
		pRule -> bAddToIgnore = TRUE;
	}
	else {
		pRule -> bGetBody = TRUE;
		pRule -> bAddToWatch = TRUE;
	}

	GetGlobalRules () -> SetAt (pRule -> strRuleName, pRule);
	GetGlobalRules () -> UnlockWrite ();

	gpsManualRuleBar -> UpdateRuleList (); // update the rule bar
	gpStore -> SaveRules ();   // save rule set
}

// -------------------------------------------------------------------------
void TThreadActionList::Add (TNewsGroup *pNG, TArticleHeader *pArticle)
{
	if (MessageInMyThreads (pNG, pArticle))
		return;     // has been assimilated by MessageInMyThreads()

	CStringList sReferences;
	pArticle -> GetReferencesStringList (&sReferences);

	// add the article's ID as well
	sReferences.AddHead (pArticle -> GetMessageID ());

	// for display purposes, erase any leading "re:"
	CString strSubject = pArticle -> GetSubject ();
	CString strRE; strRE.LoadString (IDS_RE);
	int iRELen = strRE.GetLength ();
	int iLen = strSubject.GetLength ();
	if (iLen > iRELen && !strSubject.Left (iRELen).CompareNoCase (strRE)) {
		strSubject = strSubject.Right (iLen - iRELen);
		strSubject.TrimLeft ();
	}

	CTime now = CTime::GetCurrentTime ();
	TThreadActionItem sNew (strSubject, sReferences, now, pNG -> m_GroupID,
		pArticle -> GetArticleNumber ());

	WriteLock ();

	m_sList.AddTail (sNew);
	m_bDirty = TRUE;

	// also add the new references to the ID array, and the subject to the
	// subject array
	AddRefsToArray (sReferences);
	AddSubjectToArray (strSubject);

	UnlockWrite ();

	CheckRuleExists (this);
}

// -------------------------------------------------------------------------
void TThreadActionList::Remove (POSITION iPos)
{
	WriteLock ();

	TThreadActionItem *pItem = (TThreadActionItem *) m_sList.GetAt (iPos);

	// remove IDs from ID array, and subject from subject array
	const CStringList &sReferences = pItem -> References ();
	RemoveRefsFromArray (sReferences);
	RemoveSubjectFromArray (pItem -> Subject ());
	pItem -> RemoveArticleIcons ();

	delete pItem;
	m_sList.RemoveAt (iPos);
	m_bDirty = TRUE;

	UnlockWrite ();
}

// -------------------------------------------------------------------------
void TThreadActionList::Remove (TNewsGroup *pNG, TArticleHeader *pHeader)
{
	POSITION pos;
	if (MessageInMyThreads (pNG, pHeader, &pos))
		Remove (pos);
}

// -------------------------------------------------------------------------
// GetHeadPosition -- caller is responsible for locking this object
POSITION TThreadActionList::GetHeadPosition () const
{
	return m_sList.GetHeadPosition ();
}

// -------------------------------------------------------------------------
// GetNext -- caller is responsible for locking this object
const TThreadActionItem &TThreadActionList::GetNext (POSITION &iPos)
{
	return m_sList.GetNext (iPos);
}

// -------------------------------------------------------------------------
// IDInMyThreads -- tells whether an ID is in threads I'm watching... also
// tells where it should be inserted
static BOOL CStringBinarySearch (CStringArray &array, LPCTSTR pchString,
								 int &iIndex)
{
	// binary search
	int &iMiddle = iIndex;
	int iLen = array.GetSize ();
	int iUpper = iLen - 1;
	int iLower = 0;

	iMiddle = (iUpper + iLower) / 2;
	int iPrevMiddle = iMiddle;
	while (iUpper > iLower) {
		int iResult = array [iMiddle].Compare (pchString);
		if (!iResult)
			return TRUE;
		if (iResult > 0)
			iUpper = iMiddle;
		else
			iLower = iMiddle;
		iMiddle = (iUpper + iLower) / 2;

		if (iPrevMiddle == iMiddle)
			break;   // we're getting nowhere

		iPrevMiddle = iMiddle;
	}

	// at this point, iUpper and iLower are either the same or iUpper ==
	// iLower + 1, so check the 3 positions: iLower, iLower+1, and iLower+2
	iMiddle = iLower;
	if (iLen > iMiddle) {
		int iResult = array [iMiddle].Compare (pchString);
		if (!iResult)
			return TRUE;
		if (iResult < 0) {
			iMiddle ++;
			if (iLen > iMiddle) {
				iResult = array [iMiddle].Compare (pchString);
				if (!iResult)
					return TRUE;
				if (iResult < 0) {
					iMiddle ++;
					if (iLen > iMiddle) {
						iResult = array [iMiddle].Compare (pchString);
						if (!iResult)
							return TRUE;
						if (iResult < 0)
							iMiddle ++;
					}
				}
			}
		}
	}

	return FALSE;
}

// -------------------------------------------------------------------------
// IDInMyThreads -- tells whether an ID is in threads I'm watching... also
// tells where it should be inserted
BOOL TThreadActionList::IDInMyThreads (LPCTSTR pchID, int &iIndex)
{
	return CStringBinarySearch (m_sIDs, pchID, iIndex);
}

// -------------------------------------------------------------------------
// SubjectInMyThreads -- tells whether a subject is in threads I'm watching...
// also tells where it should be inserted
BOOL TThreadActionList::SubjectInMyThreads (LPCTSTR pchSubject, int &iIndex)
{
	return CStringBinarySearch (m_sSubjects, pchSubject, iIndex);
}

// -------------------------------------------------------------------------
BOOL TThreadActionList::MessageInMyThreads (TNewsGroup *pNG,
											TArticleHeader *pHeader, POSITION *pPos /* = NULL */)
{
	if (m_sList.IsEmpty ())
		return FALSE;

	const TFlatStringArray &references = pHeader -> GetReferences ();
	LPCTSTR pchArticleID = pHeader -> GetMessageID ();

	WriteLock ();
	BOOL bFound = FALSE;
	BOOL bReference = FALSE;   // if found, reference or subject?
	LPCTSTR pchReference;      // reference that was found
	int iDummy;
	int result = FALSE;

	int iLen = references.GetSize ();
	for (int i = -1; i < iLen; i++) {

		// look at this article's ID on iteration -1
		pchReference = (i == -1 ? pchArticleID : references.get_string (i));

		if (IDInMyThreads (pchReference, iDummy)) {
			bFound = TRUE;
			bReference = TRUE;
			break;
		}
	}

	LPCTSTR pchSubject;
	if (!bFound && m_bTestSubjects) {
		pchSubject = pHeader -> GetSubject ();

		// take off leading "re:"
		if (strlen (pchSubject) > 3 &&
			(pchSubject [0] == 'r' || pchSubject [0] == 'R') &&
			(pchSubject [1] == 'e' || pchSubject [1] == 'E') &&
			(pchSubject [2] == ':'))
			pchSubject += 3;

		// take off leading whitespace
		while (pchSubject [0] == ' ')
			pchSubject ++;

		bFound = SubjectInMyThreads (pchSubject, iDummy);
	}

	if (bFound) {
		POSITION pos2 = m_sList.GetHeadPosition ();
		while (pos2) {
			if (pPos) *pPos = pos2;
			TThreadActionItem &item = m_sList.GetNext (pos2);
			if (bReference ? item.ReferenceInMyThread (pchReference) :
				item.SubjectInMyThread (pchSubject)) {

					m_bDirty = TRUE;  // item's last-seen time has changed, and we may
					// add some new refs below

					CStringList sNewRefs;
					item.RefsIDontHave (pchArticleID, references, sNewRefs);
					if (!sNewRefs.IsEmpty ()) {   // resistance is futile...
						item.AugmentReferences (sNewRefs);
						AddRefsToArray (sNewRefs);
					}

					item.AddArticleNum (pNG -> m_GroupID,
						pHeader -> GetArticleNumber ());

					item.Seen ();

					result = TRUE;
					break;
			}
		}
	}

	UnlockWrite ();
	return result;
}

// -------------------------------------------------------------------------
// GetSettings -- caller is responsible for locking this object
void TThreadActionList::GetSettings (unsigned &iDays, BOOL &bTestSubjects)
const
{
	iDays = m_iDays;
	bTestSubjects = m_bTestSubjects;
}

// -------------------------------------------------------------------------
// SetSettings -- caller is responsible for locking this object
void TThreadActionList::SetSettings (unsigned iDays, BOOL bTestSubjects)
{
	m_iDays = iDays;
	m_bTestSubjects = bTestSubjects;
	m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
// operator= -- caller is responsible for locking this object
TThreadActionList &TThreadActionList::operator= (TThreadActionList &src)
{
	m_sIDs.Copy (src.m_sIDs);
	m_sSubjects.Copy (src.m_sSubjects);
	m_bDirty = src.m_bDirty;
	m_iDays = src.m_iDays;
	m_bTestSubjects = src.m_bTestSubjects;

	m_sList.RemoveAll ();
	POSITION pos = src.m_sList.GetHeadPosition ();
	while (pos) {
		TThreadActionItem &item = src.m_sList.GetNext (pos);
		m_sList.AddTail (item);
	}

	return *this;
}

// -------------------------------------------------------------------------
void TThreadActionList::Expire ()
{
	unsigned iDays;
	BOOL bTestSubjects;
	GetSettings (iDays, bTestSubjects);

	CTime sNow = CTime::GetCurrentTime ();
	// threshold -- oldest time we'll accept
	time_t threshold = sNow.GetTime () - iDays * glSecondsPerDay;

	POSITION pos = GetHeadPosition ();
	while (pos) {
		POSITION iOldPos = pos;
		const TThreadActionItem &item = GetNext (pos);
		if (item.NotSeenSince (threshold))
			Remove (iOldPos);
	}

	// special check: if m_sList is empty, empty the ID array as well, just
	// in case somehow the two got out of sync... this way the user can always
	// fix things by emptying the watch/ignore list and restarting
	if (!GetHeadPosition ()) {
		ASSERT (!m_sIDs.GetSize () && !m_sSubjects.GetSize ());
		m_sIDs.RemoveAll ();
		m_sSubjects.RemoveAll ();
	}
}

// -------------------------------------------------------------------------
TThreadAction::TThreadAction (TThreadActionList &sList, int iTitleID,
							  int iListTitleID, int iSettingsStopPrompt, CWnd* pParent /*=NULL*/)
							  : m_iTitleID (iTitleID), m_iListTitleID (iListTitleID),
							  m_iSettingsStopPrompt (iSettingsStopPrompt),
							  CDialog(TThreadAction::IDD, pParent)
{
	m_sList = sList;
}

// -------------------------------------------------------------------------
void TThreadAction::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TITLE, m_sTitle);
	DDX_Control(pDX, IDC_REMOVE, m_sRemove);


	// do this one manually because it's a COwnerDrawListView
	DDX_Control(pDX, IDC_THREADS, m_sThreads);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TThreadAction, CDialog)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_THREADS, OnThreadsSelChange)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_THREADS, OnColumnclickThreads)
	ON_BN_CLICKED(IDC_SETTINGS, OnSettings)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TThreadActionList *gpSortList;
static int CALLBACK CompareItems (LPARAM lParam1, LPARAM lParam2,
								  LPARAM lParamSort)
{
	int iColumn = (int) lParamSort;
	POSITION iPos1 = (POSITION) lParam1;
	POSITION iPos2 = (POSITION) lParam2;
	const TThreadActionItem &item1 = gpSortList -> GetNext (iPos1);
	const TThreadActionItem &item2 = gpSortList -> GetNext (iPos2);
	int iResult = 0;

	ASSERT (iColumn <= 1 && iColumn >= 0);
	switch (iColumn) {
	  case 0:  // compare days-since-seen
		  {
			  time_t time1 = item1.LastSeen ().GetTime ();
			  time_t time2 = item2.LastSeen ().GetTime ();
			  if (time1 > time2)
				  iResult = -1;
			  if (time1 < time2)
				  iResult = 1;
			  break;
		  }
	  case 1:  // compare subjects
		  iResult = item1.Subject ().CompareNoCase (item2.Subject ());
		  break;
	}

	return iResult;
}

// -------------------------------------------------------------------------
BOOL TThreadAction::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str; str.LoadString (m_iTitleID);
	SetWindowText (str);
	str.LoadString (m_iListTitleID);
	m_sTitle.SetWindowText (str);

	InitializeList ();

	m_sList.ReadLock ();
	POSITION pos = m_sList.GetHeadPosition ();
	while (pos) {
		POSITION iItemPos = pos;
		const TThreadActionItem &item = m_sList.GetNext (pos);
		InsertItem (item, iItemPos);
	}
	m_sList.UnlockRead ();

	gpSortList = &m_sList;  // for CompareItems()
	m_sThreads.SortItems (CompareItems, 0 /* column 1 */);
	GreyButtons ();

	return TRUE;
}

// -------------------------------------------------------------------------
void TThreadAction::InitializeList ()
{
	CString strTitle; strTitle.LoadString (IDS_THREAD_ACTION_DAYS_COL);
	m_sThreads.InsertColumn (0, strTitle, LVCFMT_RIGHT, 95, 1);

	// second column's width is either the rest of the available width, or
	// the width of the longest string, whichever is more
	RECT rct;
	m_sThreads.GetClientRect (&rct);
	int iWidth = rct.right - 95 - 16; // - 16 to leave room for scroll bar
	m_sList.ReadLock ();
	POSITION pos = m_sList.GetHeadPosition ();
	while (pos) {
		POSITION iItemPos = pos;
		const TThreadActionItem &item = m_sList.GetNext (pos);
		iWidth = max (iWidth, m_sThreads.GetStringWidth (item.Subject ()) + 10);
	}
	m_sList.UnlockRead ();

	strTitle.LoadString (IDS_THREAD_ACTION_SUBJ_COL);
	m_sThreads.InsertColumn (1, strTitle, LVCFMT_LEFT, iWidth, 0);
}

// -------------------------------------------------------------------------
void TThreadAction::InsertItem (const TThreadActionItem &item, POSITION pos)
{
	int iIndex = m_sThreads.GetItemCount ();

	time_t now = CTime::GetCurrentTime ().GetTime ();
	int iDays = (now - item.LastSeen ().GetTime ()) / glSecondsPerDay;
	CString str; str.Format ("%d", iDays);
	m_sThreads.InsertItem (LVIF_TEXT | LVIF_PARAM, iIndex, str, 0, 0, 0,
		(LPARAM) pos);

	m_sThreads.SetItemText (iIndex, 1, item.Subject ());
}

// -------------------------------------------------------------------------
void TThreadAction::OnRemove()
{
	// work backwards so we don't need to worry about adjusting the indices
	for (int i = m_sThreads.GetItemCount (); i >= 0; i--)
		if (m_sThreads.GetItemState (i, LVIS_SELECTED)) {
			POSITION pos = (POSITION) m_sThreads.GetItemData (i);
			m_sList.Remove (pos);
			m_sThreads.DeleteItem (i);
		}
}

// -------------------------------------------------------------------------
void TThreadAction::OnColumnclickThreads(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_sThreads.SortItems (CompareItems, pNMListView -> iSubItem);
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TThreadAction::OnThreadsSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	GreyButtons ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TThreadAction::GreyButtons ()
{
	UINT iSelected = m_sThreads.GetSelectedCount ();
	m_sRemove.EnableWindow (iSelected);
}

// -------------------------------------------------------------------------
class TThreadActionExpiration : public CDialog
{
public:
	TThreadActionExpiration(CWnd* pParent = NULL);   // standard constructor
	int m_iSettingsStopPrompt;

	enum { IDD = IDD_THREAD_ACTION_EXPIRATION };
	UINT	m_iDaysAfterSeen;
	CString	m_strStopPrompt;
	BOOL	m_bTestSubjects;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

// -------------------------------------------------------------------------
TThreadActionExpiration::TThreadActionExpiration(CWnd* pParent /*=NULL*/)
: CDialog(TThreadActionExpiration::IDD, pParent)
{

	m_iDaysAfterSeen = 0;
	m_strStopPrompt = _T("");
	m_bTestSubjects = FALSE;


	m_iSettingsStopPrompt = 0;
}

// -------------------------------------------------------------------------
void TThreadActionExpiration::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_DAYS_AFTER_SEEN, m_iDaysAfterSeen);
	DDV_MinMaxUInt(pDX, m_iDaysAfterSeen, 0, 999);
	DDX_Text(pDX, IDC_STOP_PROMPT, m_strStopPrompt);
	DDX_Check(pDX, IDC_TEST_SUBJECTS, m_bTestSubjects);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TThreadActionExpiration, CDialog)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TThreadActionExpiration::OnInitDialog()
{
	m_strStopPrompt.LoadString (m_iSettingsStopPrompt);
	CDialog::OnInitDialog();   // data exchange
	return TRUE;
}

// -------------------------------------------------------------------------
void TThreadAction::OnSettings()
{
	TThreadActionExpiration dlg;
	m_sList.GetSettings (dlg.m_iDaysAfterSeen, dlg.m_bTestSubjects);
	dlg.m_iSettingsStopPrompt = m_iSettingsStopPrompt;
	if (dlg.DoModal () != IDOK)
		return;
	m_sList.SetSettings (dlg.m_iDaysAfterSeen, dlg.m_bTestSubjects);
}

// -------------------------------------------------------------------------
void EditThreadActionList (TThreadActionList &sList, int iTitleID,
						   int iListTitleID, int iSettingsStopPrompt)
{
	sList.WriteLock ();
	TThreadAction dlg (sList, iTitleID, iListTitleID, iSettingsStopPrompt,
		AfxGetMainWnd ());
	if (dlg.DoModal () == IDOK)
		sList = dlg.m_sList;
	sList.UnlockWrite ();
}
