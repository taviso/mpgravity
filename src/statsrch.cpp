/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: statsrch.cpp,v $
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:55  richard_wood
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
#include "statsrch.h"
#include "log.h"

#include "critsect.h"
#include "resource.h"

#include "ngstat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(TStatusSearch, CObject, 1);

TStatusSearch::TStatusSearch()
{
	InitializeCriticalSection(&m_critSect);

	m_iCurElem = 0;
	m_iMaxElem = 1;

	m_hUnits = 0;
	m_pUnits = 0;
	alloc_array(1);
}

TStatusSearch::~TStatusSearch()
{
	if (m_hUnits)
	{
		GlobalUnlock (m_hUnits);
		GlobalFree (m_hUnits);
	}
	DeleteCriticalSection(&m_critSect);
}

// ------------------------------------------------------------------------
// 12-10-96  amc  OR in any existing bits
// Returns 0 if we added a new unit. 1 if the unit existed already
int  TStatusSearch::AddItem(
							int  artInt,
							WORD status,
							time_t readTime,
							BOOL fForce /*FALSE*/)
{
	TEnterCSection mgr(&m_critSect);
	BOOL fFound;
	int idx = 0;

	if (m_iCurElem + 1 >= m_iMaxElem)
		grow_array();
	if (fForce || (0==m_iCurElem))
		fFound = FALSE;
	else
		fFound = Find(artInt, idx);
	if (fFound)
	{
		// keep current settings
	}
	else
	{
		// shift items downstream
		MoveMemory(m_pUnits + idx + 1, m_pUnits + idx,
			(m_iCurElem - idx) * sizeof(StatusUnitI));

		m_pUnits[idx].m_articleInt = artInt;
		m_pUnits[idx].m_status = status;
		m_pUnits[idx].m_timeRead = readTime;
		++m_iCurElem;
	}
	return fFound ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////
//
//
void
TStatusSearch::RemoveItem(
						  int artInt,
						  TStatusUnit* pUnit /*= 0*/)
{
	TEnterCSection mgr(&m_critSect);
	int idx = -1;
	BOOL fFound = Find (artInt, idx);
	if (fFound)
	{
		if (pUnit)
		{
			pUnit->m_articleInt = artInt;
			pUnit->m_status = m_pUnits[idx].m_status;
			pUnit->m_timeRead = CTime(m_pUnits[idx].m_timeRead);
		}
		MoveMemory(m_pUnits+idx, m_pUnits+idx+1,
			(m_iCurElem - (idx + 1)) * sizeof(StatusUnitI));

		ZeroMemory(m_pUnits+m_iCurElem - 1, sizeof(StatusUnitI));
		--m_iCurElem;
	}
	else
	{
		// OK -- probably just removed this article but user tried to remove
		// it again before it got yanked from the display
	}
}

///////////////////////////////////////////////////////////////////////////
// I copied this from Sedgewick, since I have no brain of my own. -amc
//
//
BOOL TStatusSearch::Find(int artInt, int& idx)
{
	idx = 0;
	if (!m_pUnits || 0==m_iCurElem)
		return FALSE;

	int lo = 0;
	int hi = m_iCurElem - 1;
	int mid;
	register int candidate;
	do
	{
		mid = (lo + hi) / 2;
		candidate = m_pUnits[mid].m_articleInt;
		if (artInt < candidate)
			hi = mid - 1;
		else
			lo = mid + 1;
	} while ( (artInt != candidate) && (lo <= hi) );
	if (artInt == candidate)
	{
		idx = mid;
		return TRUE;
	}

	if (artInt < candidate)
		idx = mid;
	else
		idx = mid+1;
	return FALSE;
}

void TStatusSearch::Dump (CString & str)
{
	TEnterCSection mgr(&m_critSect);
	int i, tot = GetSize();

	CString one;
	str = "(";
	for (i = 0; i < tot; ++i)
	{
		one.Format("%d,", m_pUnits[i].m_articleInt);
		if (0 == (i % 8) && i > 0)
			str += one + "\r\n";
		else
			str += one;
	}
	str += ")";
}
///////////////////////////////////////////////////////////////////////////
//
//
void TStatusSearch::alloc_array(int n)
{
	// don't call GlobalAlloc on zero
	if (0 == n)
		n = 1;

	if (m_hUnits)
	{
		GlobalUnlock( m_hUnits );     // just discard current contents
		GlobalFree ( m_hUnits );
		m_hUnits = 0;
	}
	m_hUnits = GlobalAlloc (GHND, n*sizeof(StatusUnitI));  // zero inited
	if (NULL == m_hUnits)
		AfxThrowMemoryException();
	m_pUnits = (StatusUnitI*) GlobalLock (m_hUnits);
	if (0 == m_pUnits)
		AfxThrowMemoryException();
}

///////////////////////////////////////////////////////////////////////////
//
//
void TStatusSearch::grow_array()
{
	int newCount = m_iCurElem + 100;
	GlobalUnlock (m_hUnits);
	HANDLE hNew = GlobalReAlloc(m_hUnits,
		newCount * sizeof(StatusUnitI),
		GMEM_MOVEABLE | GMEM_ZEROINIT);
	if (0 == hNew)
	{
		m_pUnits = (StatusUnitI*) GlobalLock (m_hUnits);
		ASSERT(0);
	}
	else
	{
		m_hUnits = hNew;
		m_pUnits = (StatusUnitI*) GlobalLock (m_hUnits);
		m_iMaxElem = newCount;
	}
}

int QSortStatusUnits(const void *pLeft, const void *pRight)
{
	StatusUnitI *pSILeft = (StatusUnitI*)pLeft;
	StatusUnitI *pSIRight = (StatusUnitI*)pRight;

	if (!pSILeft || !pSIRight)
		return 0;

	if ((int(pSILeft->m_articleInt)) == (int(pSIRight->m_articleInt)))
		return 0;
	else if ((int(pSILeft->m_articleInt)) > (int(pSIRight->m_articleInt)))
		return 1;
	else
		return -1;
}

void TStatusSearch::Serialize(CArchive& ar)
{
	CObject::Serialize ( ar );
	if ( ar.IsStoring() )
	{
		ar << m_iCurElem;
		ar << m_iMaxElem;
		ar.Write(m_pUnits, m_iCurElem * sizeof(StatusUnitI));
	}
	else
	{
		ar >> m_iCurElem;
		ar >> m_iMaxElem;

		m_iMaxElem = m_iCurElem;

		alloc_array (m_iCurElem);

		ar.Read(m_pUnits, m_iCurElem * sizeof(StatusUnitI));
		//for (int i = 0; i < m_iCurElem; i++)
		//{
		//	TRACE("TStatusSearch::Serialize : Read in header info %d, %s %s,\tart ID %lu\n",
		//		i,
		//		m_pUnits[i].m_status ? "valid" : "invalid",
		//		TNewsGroupStatus::IsEmail(m_pUnits[i].m_articleInt) ? "email" : "news ",
		//		m_pUnits[i].m_articleInt);
		//}

		// RLW - stupid thing, this lot is an array, should be sorted but some old files are not
		// sorted, so do a qsort here...
		qsort(m_pUnits, m_iCurElem, sizeof(StatusUnitI), QSortStatusUnits);

		//for (int i = 0; i < m_iCurElem; i++)
		//{
		//	TRACE("TStatusSearch::Serialize : Read in header info %d, %s %s,\tart ID %d\n",
		//		i,
		//		m_pUnits[i].m_status ? "valid" : "invalid",
		//		TNewsGroupStatus::IsEmail(m_pUnits[i].m_articleInt) ? "email" : "news ",
		//		m_pUnits[i].m_articleInt);
		//}

		// Get rid of any articles with negative IDs as they're illegal
		// (code uses ints to pass artIDs, and they're stored in LONG, which is unsigned, oops)
		for (int i = 0; i < m_iCurElem; i++)
		{
			if (int(m_pUnits[i].m_articleInt) < 0)
			{
				//TRACE("TStatusSearch::Serialize : Removing invalid artID %d\n", m_pUnits[i].m_articleInt);
				// Get rid of this one
				m_iCurElem--;
				for (int j = i; j < m_iCurElem; j++)
					memcpy(&m_pUnits[j], &m_pUnits[j+1], sizeof(StatusUnitI));
				i--;
			}
		}
	}
}

void TStatusSearch::SetItemByIndex(int idx, WORD status, time_t readTime)
{
	ASSERT(idx >= 0);
	ASSERT(idx < m_iCurElem);

	TEnterCSection mgr(&m_critSect);

	m_pUnits[idx].m_status = status;
	m_pUnits[idx].m_timeRead = readTime;
}

void TStatusSearch::SetItemByKey(int artInt, WORD status, time_t readTime)
{
	TEnterCSection mgr(&m_critSect);
	int idx = -1;
	if (Find(artInt, idx))
	{
		SetItemByIndex(idx,status, readTime);
		return;
	}
	ASSERT(0);
}

void TStatusSearch::DestroyAll()
{
	TEnterCSection mgr(&m_critSect);
	GlobalUnlock(m_hUnits);
	GlobalFree(m_hUnits);
	m_iCurElem = 0;
	m_iMaxElem = 0;
	m_pUnits = 0;
	alloc_array (1);
}

//-------------------------------------------------------------------------
// All article ints greater than 'iNewHi' are now 'New' again
void TStatusSearch::ResetHighestArticleRead (int iNewHi)
{
	TEnterCSection mgr(&m_critSect);

	for (int i = 0; i < m_iCurElem; ++i)
	{
		StatusUnitI* pU = (m_pUnits + i);
		if (pU->m_articleInt > iNewHi)
		{
			pU->m_status |= TStatusUnit::kNew;
		}
	}
}

