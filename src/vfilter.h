/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vfilter.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:52:25  richard_wood
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

#pragma once

#include "rgbase.h"
#include "pobject.h"
#include "declare.h"
#include "statunit.h"

struct Condition;

class TViewFilter : public PObject
{
public:
	static TViewFilter* ReadFromSubkey (LPTSTR pKeyName);
	static int   SortColumn (DWORD dwSortCode);
	static bool  SortAscending (DWORD dwSortCode);

public:
	// constructor 1
	TViewFilter(LPCTSTR name, WORD wFilter, WORD wRequired);

	// constructor 2
	TViewFilter(const CString & name,
		DWORD dwData,
		CStringArray &rstrCondition,
		DWORD dwSort,
		BOOL fShowEntireThread,
		bool fCompleteBinary,
		BOOL fSkipThreads);

	// constructor 3
	TViewFilter(int iStringID, WORD wFilter = 0, WORD wRequired = 0);

	// ctor  5 : for serialization
	TViewFilter (LOGFONT * pLF);

	virtual void Serialize (CArchive & archive);

protected:
	// protected ctors  ctor 4
	TViewFilter(const CString & name,
		DWORD           dwData,
		CStringArray &  rstrCondition,
		DWORD           dwSort,
		int             iShowThread,
		int             iID);
public:
	~TViewFilter ();

	TViewFilter::TViewFilter (const TViewFilter & src);
	TViewFilter& operator= (const TViewFilter & rhs);

	// call BeginRun() before applying this filter to a group of articles
	void BeginRun () { m_bRunError = FALSE; }

	BOOL FilterMatch (TArticleHeader * pHdr, TNewsGroup * pNG,
		TPersistentTags * pTags);

	BOOL FilterMatch (int iArtInt, TNewsGroup * pNG,
		TPersistentTags * pTags, TArticleHeader *psHeader = NULL);

	const CString & GetName () { return m_Name; }

	void SetDWordData (DWORD dw)
	{
		m_wRequired = HIWORD(dw);
		m_wFilter   = LOWORD(dw);
	}
	DWORD GetDWordData () { return (DWORD)MAKELONG(m_wFilter, m_wRequired); }

	void SetAttrib (TStatusUnit::EStatusFilter eStatus, TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetAttrib (TStatusUnit::EStatusFilter eStatus);

	void SetNew (TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetNew ();

	void SetImp (TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetImp ();

	void SetLocal (TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetLocal ();

	void SetTag (TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetTag ();

	void SetDecoded (TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetDecoded ();

	void SetProtected (TStatusUnit::ETriad eTri);
	TStatusUnit::ETriad GetProtected ();

	// True if this is the generic ViewAll filter
	BOOL IsViewAll ();
	BOOL IsViewUnread ();

	int WriteToSubkey (const CString & dadPath);

	DWORD SortCode () { return m_dwSort; }
	void SortCode (DWORD dw) { m_dwSort = dw; }

	int  getID () { return m_iID; }
	void setID (int n) { m_iID = n; }

	const CString& Name () { return m_Name; }
	void Name (const CString& n) { m_Name = n; }

	int  getShowThread ()        { return m_iShowThread; }
	void setShowThread (int n)   { m_iShowThread = n; }

	BOOL getCompleteBinaries() { return m_fCompleteBinaries; }
	void setCompleteBinaries(BOOL b) { m_fCompleteBinaries=b; }

	BOOL getNoThread()         { return m_fSkipThreading; }
	void setNoThread(BOOL b)   { m_fSkipThreading=b; }

	bool isCompleteBinaries();

	WORD         m_wFilter;
	WORD         m_wRequired;
	CStringArray m_rstrCondition;
	Condition *  m_psCondition;
	BOOL         m_bCompileError;
	BOOL         m_bRunError;

protected:

	// values for m_iSort
	//     0 = thread             HIBIT(off) is Ascending, HIBIT(on) is Descending
	//     1 = status
	//     2 = author
	//     3 = subject
	//     4 = line count
	//     5 = date
	//     6 = score
	DWORD  m_dwSort;

	int      m_iShowThread;      // show whole thread if any members match filter

	int      m_iID;
	CString  m_Name;

	BOOL	  m_fCompleteBinaries;
	BOOL    m_fSkipThreading;
};

class TAllViewFilter :  public PObject
{
public:
	int    GetNextFilterID ()
	{
		return ++ m_iNextFilterID;
	}

public:

	TAllViewFilter();
	~TAllViewFilter();
	void FillList (CWnd * pW, BOOL fListCtrl, BOOL fSetItemData);
	void FillLbx (CListBox * pW, bool fSetIDinData = false);
	void FillCombo (CComboBox* pCmb, BOOL fSetIDinData);
	void ReadFromList (CListCtrl * pListCtrl);

	void Upgrade (int iCurBuild, WORD wFilter, CString * pName);
	// data
	CTypedPtrArray<CPtrArray, TViewFilter*> m_vec;

	void empty ();

	int GetGenericFilterViewAll (TViewFilter*& rpFilter);

	TViewFilter* GetByName (const CString & name);
	TViewFilter* GetByValue (DWORD dw);
	TViewFilter* GetByID (int iFilterID);

	// get the global default viewfilter id
	int GetGlobalDefFilterID() { return m_iGlobalDefaultFilterID; }
	void SetGlobalDefFilterID(int iFID)

	{
		m_iGlobalDefaultFilterID = iFID;
	}

	void DefaultSet();

protected:
	virtual void Serialize (CArchive & archive);

protected:
	TViewFilter* get_by_util(int key, const CString * pName, DWORD dw, int iD);
	void minimum_requirements (int * piGlobalDefaultFilter);

	int m_iGlobalDefaultFilterID;

	int m_iNextFilterID;
};
