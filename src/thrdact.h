/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thrdact.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

// thrdact.h -- actions on threads, e.g., kill and watch

#pragma once

#include "pobject.h"          // PObject
#include "sharesem.h"         // TSynchronizable
#include "article.h"          // TArticleHeader
#include "odlist.h"           // COwnerDrawListView
#include "newsgrp.h"          // TNewsGroup
#include "expirable.h"        // TExpirable

// -------------------------------------------------------------------------
// current version of the TIntegerList class
#define TINTEGER_LIST_VERSION_NUMBER 1

class TIntegerList : public PObject {
public:
	DECLARE_SERIAL (TIntegerList)
	TIntegerList ();
	TIntegerList &operator= (const TIntegerList &src);
	void Serialize (CArchive &archive);
	void AddHead (LONG lInteger) { m_sList.AddHead ((void *) lInteger); }
	void AddTail (LONG lInteger) { m_sList.AddTail ((void *) lInteger); }
	LONG GetNext (POSITION &pos) const { return (LONG) m_sList.GetNext (pos); }
	POSITION GetHeadPosition () const { return m_sList.GetHeadPosition (); }

private:
	CPtrList m_sList;    // m_List holds the integers in the form of pointers
};

// -------------------------------------------------------------------------
// current version of the TThreadActionItem class
#define TTHREAD_ACTION_ITEM_VERSION_NUMBER 3

class TThreadActionItem : public PObject, public TExpirable {
public:
	DECLARE_SERIAL (TThreadActionItem)
	TThreadActionItem ();   // constructor for serialization
	TThreadActionItem (const TThreadActionItem &src); // constructor for CObList
	TThreadActionItem (const CString &subject, const CStringList &references,
		const CTime &sLastSeen, LONG lGroupID, LONG lArticleNum);
	void Serialize (CArchive &archive);
	const CString &Subject () const { return m_strSubject; }
	const CStringList &References () const { return m_sReferences; }
	BOOL ReferenceInMyThread (LPCTSTR pchReference);
	BOOL SubjectInMyThread (LPCTSTR pchSubject);
	TThreadActionItem &operator= (const TThreadActionItem &src);
	void RefsIDontHave (LPCTSTR pchArticleID,
		const TFlatStringArray &references, CStringList &sNewRefs);
	void AugmentReferences (CStringList &sNewRefs);
	BOOL HaveArticleNum (LONG lGroupID, LONG lArticleNum);
	void AddArticleNum (LONG lGroupID, LONG lArticleNum);
	void RemoveArticleIcons ();

protected:
	CString m_strSubject;
	CStringList m_sReferences;
	TIntegerList m_sArticleNums;
};

// -------------------------------------------------------------------------
// current version of the TThreadActionItemList class
#define TTHREAD_ACTION_ITEM_LIST_VERSION_NUMBER 1

class TThreadActionItemList : public CObList {
public:
	DECLARE_SERIAL (TThreadActionItemList)
	TThreadActionItem &GetNext (POSITION &pos)
	{ return *(TThreadActionItem *) CObList::GetNext (pos); }
	POSITION AddTail (TThreadActionItem &item)
	{
		TThreadActionItem *pNew = new TThreadActionItem (item);
		return CObList::AddTail (pNew);
	}
	void RemoveAll ();
};

// -------------------------------------------------------------------------
// current version of the TThreadActionList class
// NOTE: shouldn't be serializing m_bDirty.  Remove it from the serialize
// function when we make the next version
#define TTHREAD_ACTION_LIST_VERSION_NUMBER 1

class TThreadActionList : public PObject, public TSynchronizable {
public:
	DECLARE_SERIAL (TThreadActionList)
	TThreadActionList ();
	~TThreadActionList ();
	void Serialize (CArchive &archive);
	void Add (TNewsGroup *pNG, TArticleHeader *pArticle);
	void Remove (POSITION iPos);
	void Remove (TNewsGroup *pNG, TArticleHeader *pHeader);
	POSITION GetHeadPosition () const;
	const TThreadActionItem &GetNext (POSITION &iPos);
	BOOL MessageInMyThreads (TNewsGroup *pNG, TArticleHeader *pHeader,
		POSITION *pPos = NULL);
	BOOL Dirty () const { return m_bDirty; }
	void MakeDirty () {m_bDirty = TRUE;
	}
	void GetSettings (unsigned &iDays, BOOL &bTestSubjects) const;
	void SetSettings (unsigned iDays, BOOL bTestSubjects);
	TThreadActionList &operator= (TThreadActionList &src);
	void Expire ();

protected:
	void AddRefsToArray (const CStringList &list);
	void RemoveRefsFromArray (const CStringList &list);
	void AddSubjectToArray (const CString &strSubject);
	void RemoveSubjectFromArray (const CString &strSubject);
	BOOL IDInMyThreads (LPCTSTR pchID, int &iIndex);
	BOOL SubjectInMyThreads (LPCTSTR pchSubject, int &iIndex);

	TThreadActionItemList m_sList;// list of threads being watched, ignored, etc.
	CStringArray m_sIDs;          // array of all IDs being watched, ignored, etc.
	CStringArray m_sSubjects;     // array of all subjects being watched, ignored, etc.
	BOOL m_bDirty;                // has this changed since loading from disk?
	unsigned m_iDays;             // expiration days
	BOOL m_bTestSubjects;         // treat identical subjects as belonging to
	// same thread
};

// -------------------------------------------------------------------------
class TThreadAction : public CDialog
{
public:
	TThreadAction (TThreadActionList &sList, int iTitleID, int iListTitleID,
		int iSettingsStopPrompt, CWnd* pParent = NULL);
	TThreadActionList m_sList;

	enum { IDD = IDD_THREAD_ACTION };
	CStatic  m_sTitle;
	CButton  m_sRemove;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnRemove();
	afx_msg void OnThreadsSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickThreads(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSettings();
	DECLARE_MESSAGE_MAP()

	void InitializeList ();
	void InsertItem (const TThreadActionItem &item, POSITION pos);
	void GreyButtons ();

	int m_iTitleID;
	int m_iListTitleID;
	int m_iSettingsStopPrompt;
	COwnerDrawListView m_sThreads;
};

// -------------------------------------------------------------------------
void EditThreadActionList (TThreadActionList &sList, int iTitleID,
						   int iListTitleID, int iSettingsStopPrompt);
