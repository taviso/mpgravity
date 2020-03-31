/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vfilter.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
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

#include "stdafx.h"
#include "vfilter.h"
#include "regutil.h"
#include "tglobopt.h"
#include "taglist.h"
#include "newsgrp.h"
#include "article.h"
#include "trulecon.h"            // Condition
#include "rules.h"               // EvaluateCondition()
#include "resource.h"            // IDS_ERR_CREATE_KEY
#include "atlregkey.h"           // TRegKey

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////
// Version 2: added the m_fCompleteBinary field
//         3  added m_fSkipThreading field
static const BYTE VFILTER_VERSION = 3;

// -------------------------------------------------------------------------
// prototypes
static void ConditionToArray (const CString &strCondition,
							  CStringArray &rstrCondition);
static void ConditionFromArray (const CStringArray &rstrCondition,
								CString &strCondition);
static LONG OpenAdvancedKey (HKEY &hKey);
static void DeleteAdvancedCondition (TCHAR *pName);
static void ReadAdvancedCondition (TCHAR *pName, CStringArray &rstrCondition);

#define STR_SHOWTHREAD			"ShowThread"
#define STR_COMPLETEBINARIES	"CompleteBinary"

static TAllViewFilter * sgpAllFilters;

// -------------------------------------------------------------------------
TAllViewFilter::TAllViewFilter()
{
	// when we create a new database, we create this and then save it out
	m_iGlobalDefaultFilterID = 0;
	m_iNextFilterID = 0;
	sgpAllFilters = this;
}

// -------------------------------------------------------------------------
TAllViewFilter::~TAllViewFilter()
{
	empty ();
}

//-------------------------------------------------------------------------
void TAllViewFilter::empty ()
{
	for (int tot = m_vec.GetSize() - 1; tot >= 0; --tot)
		delete m_vec[tot];
	m_vec.RemoveAll ();
}

// -------------------------------------------------------------------------
void TAllViewFilter::DefaultSet()
{
	empty ();

	int iUnreadID;

	// need some defaults
	TViewFilter * pFilter = new TViewFilter(IDS_FLTNAME_UNREAD);
	pFilter->SetNew (TStatusUnit::kYes);

	iUnreadID = pFilter->getID();
	m_vec.Add (pFilter);

	pFilter = new TViewFilter(IDS_FLTNAME_READ);
	pFilter->SetNew (TStatusUnit::kNo);
	m_vec.Add (pFilter);

	pFilter = new TViewFilter(IDS_FLTNAME_ALL);
	m_vec.Add (pFilter);

	pFilter = new TViewFilter(IDS_FLTNAME_IMPUNREAD);
	pFilter->SetImp (TStatusUnit::kYes);
	pFilter->SetNew (TStatusUnit::kYes);
	m_vec.Add (pFilter);

	pFilter = new TViewFilter(IDS_FLTNAME_IMP_ALL);
	pFilter->SetImp (TStatusUnit::kYes);
	m_vec.Add (pFilter);

	pFilter = new TViewFilter(IDS_FLTNAME_LOC_ALL);
	pFilter->SetLocal (TStatusUnit::kYes);
	m_vec.Add (pFilter);

	pFilter = new TViewFilter(IDS_FLTNAME_TAG_ALL);
	pFilter->SetTag (TStatusUnit::kYes);
	m_vec.Add (pFilter);

	m_iGlobalDefaultFilterID = iUnreadID;
}

// ------------------------------------------------------------------------
// fill a CListCtrl or a CComboBox
void TAllViewFilter::FillList (CWnd * pW, BOOL fListCtrl, BOOL fSetItemData)
{
	ASSERT(pW->IsKindOf(RUNTIME_CLASS(CComboBox)) ||
		pW->IsKindOf(RUNTIME_CLASS(CListCtrl)));

	int tot = m_vec.GetSize();
	int iAt;
	for (int i = 0; i < tot; ++i)
	{
		if (fListCtrl)
		{
			CListCtrl * pList = (CListCtrl*) pW;

			// note:  using image callback for all items.
			iAt = pList->InsertItem (i, m_vec[i]->GetName(), I_IMAGECALLBACK);
			if (fSetItemData) {
				TViewFilter *pFilter = new TViewFilter ( *(m_vec[i]) );
				pList->SetItemData (iAt, (DWORD) pFilter);
			}
		}
		else
		{
			CComboBox * pCombo = (CComboBox*) pW;
			iAt = pCombo->AddString (m_vec[i]->GetName());
			if (fSetItemData) {
				TViewFilter *pFilter = new TViewFilter ( *(m_vec[i]) );
				pCombo->SetItemData (iAt, (DWORD) pFilter);
			}
		}
	}
}

// ------------------------------------------------------------------------
// optionally store a ID of filter
void TAllViewFilter::FillCombo (CComboBox* pCmb, BOOL fSetItemData)
{
	int iAt;
	for (int i = 0; i < m_vec.GetSize(); i++)
	{
		iAt = pCmb->AddString (m_vec[i]->GetName());
		if (fSetItemData)
		{
			TViewFilter * pFilter = m_vec[i];
			pCmb->SetItemData ( iAt, DWORD(pFilter->getID()) );
		}
	}
}

// ------------------------------------------------------------------------
// optionally store a ID of filter
void TAllViewFilter::FillLbx (CListBox * pLbx, bool fSetIDinData /*=false*/)
{
	ASSERT(pLbx);
	int tot = m_vec.GetSize();
	for (int i=0; i < tot; ++i)
	{
		int iAt = pLbx->AddString ( m_vec[i]->GetName() );
		if (fSetIDinData)
		{
			TViewFilter * pFilter = m_vec[i];
			pLbx->SetItemData ( iAt, DWORD(pFilter->getID()) );
		}
	}
}

// ------------------------------------------------------------------------
// Read the filter definitions out of the ListCtrl
void TAllViewFilter::ReadFromList (CListCtrl * pListCtrl)
{
	// clear out current contents
	empty();

	int tot = pListCtrl->GetItemCount();

	for (int i = 0; i < tot; ++i)
	{
		// get the name and the bits
		CString aName = pListCtrl->GetItemText (i, 0);
		TViewFilter *pFilter = (TViewFilter *) pListCtrl -> GetItemData (i);
		ASSERT (pFilter);

		// use copy ctor, but reset the name
		TViewFilter * pVF = new TViewFilter( *pFilter );
		pVF->Name (aName);

		m_vec.Add (pVF);
	}
}

// ------------------------------------------------------------------------
// Returns 0 for success
int TAllViewFilter::GetGenericFilterViewAll (TViewFilter*& rpFilter)
{
	// first Try, lookup by name
	CString strAll; strAll.LoadString (IDS_FLTNAME_ALL);

	TViewFilter * pTest = GetByName ( strAll  );
	if (pTest)
	{
		rpFilter = pTest;
		return 0;
	}

	// if that failed, look for something that shows all articles, the sort
	//   can be anything
	for (int i = m_vec.GetSize() - 1; i >= 0;i--)
	{
		if (m_vec[i]->IsViewAll ())  // let filter member function work
		{
			rpFilter = m_vec[i];
			return 0;
		}
	}

	// set output to NULL
	rpFilter = NULL;
	return 1;
}

// ------------------------------------------------------------------------
TViewFilter* TAllViewFilter::GetByName (const CString & name)
{
	return get_by_util (0, &name, 0, 0);
}

// ------------------------------------------------------------------------
TViewFilter* TAllViewFilter::GetByValue (DWORD dw)
{
	return get_by_util (1, NULL, dw, 0);
}

// ------------------------------------------------------------------------
TViewFilter* TAllViewFilter::GetByID (int iFilterID)
{
	return get_by_util (2, NULL, 0, iFilterID);
}

// ------------------------------------------------------------------------
TViewFilter* TAllViewFilter::get_by_util(int key, const CString * pName,
										 DWORD dw, int iID)
{
	int tot = m_vec.GetSize();
	for (int i = 0; i < tot; ++i)
	{
		if (0 == key)
		{
			if ((*pName) == m_vec[i]->GetName())
				return m_vec[i];
		}
		else if (1 == key)
		{
			if (dw == m_vec[i]->GetDWordData())
				return m_vec[i];
		}
		else if (2 == key)
		{
			if (iID == m_vec[i]->getID())
				return m_vec[i];
		}
	}
	ASSERT(0);
	return NULL;
}

// ------------------------------------------------------------------------
void TAllViewFilter::Serialize (CArchive & ar)
{
	int      i;

	PObject::Serialize (ar);

	if (ar.IsStoring())
	{
		ar << (DWORD) m_iGlobalDefaultFilterID;

		int iCount = m_vec.GetSize() ;

		// write out m_iNextFilterID to use
		int iMaxID = 0;

		for (i = 0; i < iCount; i++)
			iMaxID = max (iMaxID, m_vec[i]->getID());

		m_iNextFilterID = iMaxID + 1;

		ar << (DWORD) m_iNextFilterID;

		// write out pointer array

		ar << (DWORD) iCount;

		for (int i = 0; i < iCount; i++)
		{
			m_vec[i] -> Serialize (ar);
		}
	}
	else
	{
		DWORD  dwTemp;
		int    iCount;

		ar >> dwTemp;    m_iGlobalDefaultFilterID = (int) dwTemp;

		ar >> dwTemp;    m_iNextFilterID = (int) dwTemp;

		ar >> dwTemp;    iCount = (int) dwTemp;

		m_vec.RemoveAll();
		for (i = 0; i < iCount; i++)
		{
			LOGFONT sLF;
			bool    fOK = false;
			TViewFilter * pFilter;

			try
			{
				pFilter =  new TViewFilter (&sLF);

				pFilter -> Serialize ( ar );

				fOK = true;
			}
			catch(...)
			{
				break;
			}
			if (fOK)
			{
				m_vec.Add ( pFilter );
			}
		}

		// enforce minimum requirements
		minimum_requirements (&m_iGlobalDefaultFilterID);
	}
}

// ------------------------------------------------------------------------
// Enforce minimum requirements
void TAllViewFilter::minimum_requirements (int * piGlobalDefaultFilter)
{
	int iFilterThatIsUnread = 0;

	int tot = m_vec.GetSize();
	if (0 == tot)
		DefaultSet ();
	else
	{
		int  i ;
		bool fFoundAll = false;
		bool fFoundUnread = false;

		// GUARANTEE that at least 1 filter is 'All Articles'
		// GUARANTEE that at least 1 filter is 'Unread Articles'
		for (i = 0; i < tot; ++i)
		{
			if (m_vec[i]->IsViewAll())
				fFoundAll = true;
			if (m_vec[i]->IsViewUnread())
			{
				fFoundUnread = true;
				iFilterThatIsUnread = m_vec[i]->getID();
			}
		}
		if (!fFoundAll)
		{
			TViewFilter * pAll = new TViewFilter(IDS_FLTNAME_ALL, 0, 0);

			// check that filter matches our own test.
			ASSERT(pAll->IsViewAll());

			m_vec.Add ( pAll );
		}
		if (!fFoundUnread)
		{
			TViewFilter * pFilter =
				new TViewFilter(IDS_FLTNAME_UNREAD, TStatusUnit::kNew, TStatusUnit::kNew);

			// check that filter matches our own test.
			ASSERT( pFilter->IsViewUnread() );

			m_vec.Add ( pFilter );
			iFilterThatIsUnread = pFilter->getID();
		}

		bool fFoundDefFilter = false;

		// make sure that the DefaultFilter points to something valid
		for (i = 0; i < m_vec.GetSize(); i++)
		{
			if (*piGlobalDefaultFilter == m_vec[i]->getID())
			{
				fFoundDefFilter = true;
				break;
			}
		}

		if (!fFoundDefFilter)
		{
			*piGlobalDefaultFilter = iFilterThatIsUnread;
		}
	}
}

// ------------------------------------------------------------------------
// ctor 1
TViewFilter::TViewFilter(LPCTSTR name, WORD wFilter, WORD wRequired)
: PObject(VFILTER_VERSION), m_wFilter(wFilter), m_wRequired(wRequired), m_Name(name)
{
	m_psCondition = 0;
	m_bCompileError = m_bRunError = FALSE;
	m_dwSort = 0;
	m_iShowThread = 0;

	m_iID = sgpAllFilters -> GetNextFilterID ();
	m_fCompleteBinaries = false;
	m_fSkipThreading = FALSE;
}

// ------------------------------------------------------------------------
// ctor 3
TViewFilter::TViewFilter(int iStringID, WORD wFilter, WORD wRequired)
: PObject(VFILTER_VERSION), m_wFilter(wFilter), m_wRequired(wRequired)
{
	if (iStringID)
		m_Name.LoadString(iStringID);

	m_psCondition = 0;
	m_bCompileError = m_bRunError = FALSE;
	m_dwSort = 0;
	m_iShowThread = 0;
	m_iID = sgpAllFilters -> GetNextFilterID ();
	m_fCompleteBinaries = false;
	m_fSkipThreading = FALSE;
}

// ------------------------------------------------------------------------
// constructor 2
TViewFilter::TViewFilter (
						  const CString & name,
						  DWORD           dwData,
						  CStringArray &  rstrCondition,
						  DWORD           dwSort,
						  BOOL            fShowEntireThread,
						  bool            fCompleteBinary,
						  BOOL            fSkipThreading)
						  : PObject(VFILTER_VERSION), m_Name (name)
{
	m_wRequired = HIWORD(dwData);
	m_wFilter = LOWORD(dwData);
	m_rstrCondition.Copy (rstrCondition);
	m_psCondition = 0;
	m_bCompileError = m_bRunError = FALSE;
	m_dwSort = dwSort;
	m_iShowThread = fShowEntireThread;
	m_iID = sgpAllFilters -> GetNextFilterID ();
	m_fCompleteBinaries = fCompleteBinary ? TRUE : FALSE;
	m_fSkipThreading = fSkipThreading;
}

// ------------------------------------------------------------------------
// protected constructor 4
//
TViewFilter::TViewFilter(const CString & name, DWORD dwData,
						 CStringArray &rstrCondition, DWORD dwSort, int iShowThread, int iID)
						 : PObject(VFILTER_VERSION), m_Name(name)
{
	m_wRequired = HIWORD(dwData);
	m_wFilter = LOWORD(dwData);
	m_rstrCondition.Copy (rstrCondition);
	m_psCondition = 0;
	m_bCompileError = m_bRunError = FALSE;
	m_dwSort = dwSort;
	m_iID = iID;
	m_iShowThread = iShowThread;
	m_fCompleteBinaries = false;
	m_fSkipThreading = FALSE;
}

// ------------------------------------------------------------------------
// ctor  5 for serialization
TViewFilter::TViewFilter (LOGFONT * pLF)
: PObject(VFILTER_VERSION)
{
	m_wRequired = m_wFilter = 0;
	m_psCondition = 0;
	m_bCompileError = m_bRunError = FALSE;
	m_dwSort = 0;
	m_iShowThread = 0;
	m_iID = 0;
	m_fCompleteBinaries = false;
	m_fSkipThreading = FALSE;
}

// ------------------------------------------------------------------------
TViewFilter::~TViewFilter ()
{
	FreeCondition (m_psCondition);
}

// ------------------------------------------------------------------------
// Copy constructor
TViewFilter::TViewFilter (const TViewFilter & src)
: PObject(VFILTER_VERSION), m_Name(src.m_Name)
{
	m_wFilter       = src.m_wFilter;
	m_wRequired     = src.m_wRequired;
	m_rstrCondition.Copy (src.m_rstrCondition);
	m_psCondition   = 0;
	m_bCompileError = m_bRunError = FALSE;
	m_dwSort        = src.m_dwSort;
	m_iID           = src.m_iID;
	m_iShowThread   = src.m_iShowThread;
	m_fCompleteBinaries=src.m_fCompleteBinaries;
	m_fSkipThreading = src.m_fSkipThreading;
}

// ------------------------------------------------------------------------
TViewFilter& TViewFilter::operator= (const TViewFilter & rhs)
{
	if (this == &rhs)
		return *this;
	m_wFilter       = rhs.m_wFilter;
	m_wRequired     = rhs.m_wRequired;
	m_Name          = rhs.m_Name;
	m_rstrCondition.Copy (rhs.m_rstrCondition);
	m_psCondition   = 0;
	m_bCompileError = m_bRunError = FALSE;

	m_dwSort        = rhs.m_dwSort;
	m_iID           = rhs.m_iID;
	m_iShowThread   = rhs.m_iShowThread;
	m_fCompleteBinaries=rhs.m_fCompleteBinaries;
	m_fSkipThreading = rhs.m_fSkipThreading;
	return *this;
}

// ------------------------------------------------------------------------
void TViewFilter::SetNew (TStatusUnit::ETriad eTri)
{ SetAttrib (TStatusUnit::kFilterNew, eTri); }

TStatusUnit::ETriad TViewFilter::GetNew ()
{ return GetAttrib (TStatusUnit::kFilterNew); }

// ------------------------------------------------------------------------
void TViewFilter::SetImp (TStatusUnit::ETriad eTri)
{ SetAttrib (TStatusUnit::kFilterImportant, eTri); }

TStatusUnit::ETriad TViewFilter::GetImp ()
{ return GetAttrib (TStatusUnit::kFilterImportant); }

// ------------------------------------------------------------------------
void TViewFilter::SetLocal (TStatusUnit::ETriad eTri)
{ SetAttrib (TStatusUnit::kFilterLocal, eTri); }

TStatusUnit::ETriad TViewFilter::GetLocal ()
{ return GetAttrib (TStatusUnit::kFilterLocal); }

// ------------------------------------------------------------------------
void TViewFilter::SetTag (TStatusUnit::ETriad eTri)
{ SetAttrib (TStatusUnit::kFilterTag, eTri); }

TStatusUnit::ETriad TViewFilter::GetTag ()
{ return GetAttrib (TStatusUnit::kFilterTag); }

// ------------------------------------------------------------------------
void TViewFilter::SetDecoded (TStatusUnit::ETriad eTri)
{ SetAttrib (TStatusUnit::kFilterDecoded, eTri); }

TStatusUnit::ETriad TViewFilter::GetDecoded ()
{ return GetAttrib (TStatusUnit::kFilterDecoded); }

// ------------------------------------------------------------------------
void TViewFilter::SetProtected (TStatusUnit::ETriad eTri)
{ SetAttrib (TStatusUnit::kFilterPermanent, eTri); }

TStatusUnit::ETriad TViewFilter::GetProtected ()
{ return GetAttrib (TStatusUnit::kFilterPermanent); }

// ------------------------------------------------------------------------
void TViewFilter::SetAttrib (TStatusUnit::EStatusFilter eStatus,
							 TStatusUnit::ETriad eTri)
{
	if (TStatusUnit::kMaybe == eTri)
		m_wRequired &= ~eStatus;
	else
	{
		m_wRequired |= eStatus;

		if (TStatusUnit::kYes == eTri)
			m_wFilter |= eStatus;
		else
			m_wFilter &= ~eStatus;
	}
}

// ------------------------------------------------------------------------
TStatusUnit::ETriad TViewFilter::GetAttrib (TStatusUnit::EStatusFilter eStatus)
{
	return (eStatus & m_wRequired)
		? ((eStatus & m_wFilter) ? TStatusUnit::kYes : TStatusUnit::kNo)
		: TStatusUnit::kMaybe;
}

// ------------------------------------------------------------------------
// Does this article header meet criteria of the filter?
BOOL TViewFilter::FilterMatch (TArticleHeader * pHdr, TNewsGroup * pNG,
							   TPersistentTags * pTags)
{
	return FilterMatch (pHdr->GetNumber(), pNG, pTags, pHdr);
}

// ------------------------------------------------------------------------
// actually this is short hand for
//   (art & mask)  XOR  (filt & mask)
#define VFILT_XOR(art, filt, mask)  ((art ^ filt) & mask)

// ------------------------------------------------------------------------
BOOL TViewFilter::FilterMatch (int iArtInt, TNewsGroup * pNG,
							   TPersistentTags * pTags, TArticleHeader *psHeader /* = NULL */)
{
	// test for the simplest case - nothing is required
	int iConditionSize = m_rstrCondition.GetSize ();
	if ((0 == m_wRequired) && (0 == iConditionSize))
		return TRUE;

	WORD wStatus = 0;

	// these properties are encoded in status unit.
	WORD wBitEncoded = TStatusUnit::kNew |
		TStatusUnit::kImportant |
		TStatusUnit::kDecoded |
		TStatusUnit::kPermanent;

	if (m_wRequired & wBitEncoded)
	{
		// get status for this article
		if (pNG->iStatusDirect ( iArtInt, wStatus ))
			return FALSE;

		// be extra safe. matches pNG->ArticleStatus
		if ((wStatus & TStatusUnit::kNew) && pNG->ReadRangeHave (iArtInt))
			wStatus &= ~TStatusUnit::kNew;

		// crunch some bits
		//   3rd param - mask off kFilterLocal & kFilterTag
		if (VFILT_XOR(wStatus, m_wFilter, m_wRequired & wBitEncoded))
			return FALSE;
	}

	// "local" criteria
	TStatusUnit::ETriad eTri = GetLocal();
	if (TStatusUnit::kMaybe != eTri)
	{
		BOOL fLocal = pNG->TextRangeHave (iArtInt);
		if (fLocal && (TStatusUnit::kNo == eTri))
			return FALSE;
		if (!fLocal && (TStatusUnit::kYes == eTri))
			return FALSE;
	}

	// artbank may not have this
	// "tagged" criteria
	eTri = GetTag();
	if (TStatusUnit::kMaybe != eTri)
	{
		BOOL fTag = pTags->FindTag (pNG->m_GroupID, iArtInt);
		if (fTag && (TStatusUnit::kNo == eTri))
			return FALSE;
		if (!fTag && (TStatusUnit::kYes == eTri))
			return FALSE;
	}

	// also must meet advanced conditions
	if ((iConditionSize > 0) && !m_psCondition && !m_bCompileError) {
		int iLineIndex;
		if (ConstructACondition (m_rstrCondition, m_psCondition, iLineIndex)) {
			m_bCompileError = TRUE;
			CString strError;
			CString strTemp; strTemp.LoadString (IDS_ERR_FILTER_COMPILE);
			strError.Format ("%s %d", strTemp, iLineIndex);
			AfxGetMainWnd () -> MessageBox (strError);
		}
	}

	if (m_psCondition) {

		if (m_bRunError)
			// probably invalid regular expression... we'll consider it as
			// though the condition didn't match
			return FALSE;

		pNG -> Open ();

		if (!psHeader) {
			psHeader = (TArticleHeader *) pNG -> GetHeader (iArtInt);
			if (!psHeader) {
				ASSERT (0);
				pNG -> Close ();
				return FALSE;
			}
		}

		int RC;
		try {
			RC = EvaluateCondition (m_psCondition, psHeader, NULL, pNG);
		}
		catch (TException *pE)
		{
			pE->Delete();
			// probably invalid regular expression... we'll consider it as
			// though the condition didn't match
			m_bRunError = TRUE;
			RC = 0;
		}

		pNG -> Close ();

		// we'll accept 1 or -1 (meaning, if the body is required for the
		// condition, pretend the condition was satisfied
		if (!RC)
			return FALSE;
	}

	return TRUE;   // article meets filter's conditions
}

// ------------------------------------------------------------------------
// Is this the vanilla ViewAll Filter?
BOOL TViewFilter::IsViewAll ()
{
	// view all == nothing required
	//   don't care what sort is applied
	return (0 == m_wRequired) && (0 == m_rstrCondition.GetSize())
		/* && (0==m_dwSort) */ ;
}

// ------------------------------------------------------------------------
// Is this the vanilla View unread Filter?
BOOL TViewFilter::IsViewUnread ()
{
	// Unread ==    filter(New), required(yes)
	DWORD dwBits = (DWORD) MAKELONG(TStatusUnit::kNew, TStatusUnit::kNew);

	// what sort they have defined is not important to alchoy
	return  (dwBits == GetDWordData()) && (0 == m_rstrCondition.GetSize());
}

// ------------------------------------------------------------------------
// really returns TTreeHeaderCtrl::EColumn
int TViewFilter::SortColumn (DWORD dwSortCode)
{
	return int(LOWORD(dwSortCode));
}

// ------------------------------------------------------------------------
//
bool TViewFilter::SortAscending (DWORD dwSortCode)
{
	return HIWORD(dwSortCode) == 0;
}

// ------------------------------------------------------------------------
void TViewFilter::Serialize (CArchive & ar)
{
	// call base class function first
	PObject::Serialize ( ar );
	DWORD dwTemp;
	BYTE  objectVersion = GetObjectVersion();

	if ( ar.IsStoring() )
	{
		ar << m_Name;
		ar << (DWORD) m_iID;  // this is the FilterID

		dwTemp = MAKELONG(m_wFilter, m_wRequired);
		ar << dwTemp;

		ar << m_dwSort;
		ar << (DWORD) m_iShowThread;
		m_rstrCondition.Serialize (ar);

		ar << m_fCompleteBinaries;
		ar << m_fSkipThreading;
	}
	else
	{
		ar >> m_Name;
		ar >> dwTemp;   m_iID = (int) dwTemp;

		ar >> dwTemp;
		m_wFilter = LOWORD(dwTemp);
		m_wRequired = HIWORD(dwTemp);

		ar >> m_dwSort;
		ar >> dwTemp;   m_iShowThread = (int) dwTemp;

		m_rstrCondition.Serialize (ar);

		if ( objectVersion >= 2 )
			ar >> m_fCompleteBinaries;

		if ( objectVersion >= 3 )
			ar >> m_fSkipThreading;
	}
}

// ------------------------------------------------------------------------
bool TViewFilter::isCompleteBinaries()
{
	if (0 == GetName().Find(_T("Complete_Binaries")) || m_fCompleteBinaries)
		return true;
	return false;
}

