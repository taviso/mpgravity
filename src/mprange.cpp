/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mprange.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/18 22:05:02  richard_wood
/*  Refactored XOVER and XHDR commands so they fetch item data in batches of 300 (or less) if we want > 300 articles.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:31  richard_wood
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

// mprange.cpp -- ranges and range sets

#include "stdafx.h"
#include <afxtempl.h>
#include "mplib.h"
#include "pobject.h"
#include <stdlib.h>              // qsort()
#include "resource.h"            // IDS*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// warning about CException has no copy-ctor
#pragma warning (disable : 4671 4673)

/////////////////////////////////////////////////////////////////////////////
// RangeSetOK -- debugging function
BOOL RangeSetOK (TRangeSet &sRangeSet)
{
	// check each range's upper is >= its lower
	int iSize = sRangeSet.RangeCount ();
	int i = 0;
	for (i = 0; i < iSize; i++) {
		int iUpper, iLower;
		sRangeSet.GetRange (i, iLower, iUpper);
		if (iLower > iUpper) {
			ASSERT (0);
			return FALSE;
		}
	}

	// check order and merging anomalies
	for (i = 1; i < iSize; i++) {
		int iUpper, iLower;
		sRangeSet.GetRange (i, iLower, iUpper);
		int iBelowUpper, iBelowLower;
		sRangeSet.GetRange (i - 1, iBelowLower, iBelowUpper);
		if (iLower < iBelowLower || iLower - 1 <= iBelowUpper) {
			ASSERT (0);
			return FALSE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL  (TRange, CObject, 1)

// new STL template syntax
template <> void AFXAPI
SerializeElements <TRange> (CArchive & ar, TRange *pItem, int nCount)
{
	for ( int i = 0; i < nCount; i++, pItem++ )
		pItem->Serialize (ar);
}

/////////////////////////////////////////////////////////////////////////////
TRange::TRange (const TRange& src)
{
	m_upper = src.m_upper;
	m_lower = src.m_lower;
}

/////////////////////////////////////////////////////////////////////////////
TRange & TRange::operator=(const TRange & rhs)
{
	m_upper = rhs.m_upper;
	m_lower = rhs.m_lower;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void TRange::Serialize (CArchive & archive)
{
	// call base class function first
	CObject::Serialize( archive );

	if( archive.IsStoring() )
		archive << m_lower << m_upper;
	else
		archive >> m_lower >> m_upper;
}

#if defined(_DEBUG)

/////////////////////////////////////////////////////////////////////////////
// Dump --
void TRange::Dump(CDumpContext& dc) const
{
	CObject::Dump( dc );
	dc << "TRange(" << m_lower << "," << m_upper << ")\n" ;
}

#endif

/////////////////////////////////////////////////////////////////////////////
// Have -- Check if an integer is in the rangeset.
BOOL TRangeSet::Have (RANGEINT iNum)
{
	return FallsIn (iNum) != -1;
}

/////////////////////////////////////////////////////////////////////////////
// Add -- add an integer to the rangeset.
void TRangeSet::Add (RANGEINT iNum)
{
	if (Have (iNum))
		return;

	// find index of higher range
	int iHigherIndex = FallsBelow (iNum);
	int iCount = RangeCount ();

	// if higher range exists, Try merging with it
	if (iHigherIndex < iCount) {
		TRange &sHigher = m_rangeList [iHigherIndex];
		if (iNum + 1 == sHigher.Lower ()) {
			sHigher.SetLower (iNum);
			Merge (iHigherIndex);
			return;
		}
	}

	// if lower range exists, Try merging with it
	if (iHigherIndex - 1 >= 0) {
		TRange &sLower = m_rangeList [iHigherIndex - 1];
		if (iNum - 1 == sLower.Upper ()) {
			sLower.SetUpper (iNum);
			Merge (iHigherIndex - 1);
			return;
		}
	}

	// no merging possible, so make its own range
	TRange sNew (iNum);
	m_rangeList.InsertAt (iHigherIndex, sNew);
}

/////////////////////////////////////////////////////////////////////////////
void TRangeSet::Add (RANGEINT iLow, RANGEINT iHigh)
{
	// add new range at proper position (for lower number) then merge
	Add (iLow);
	int iIndex = FallsIn (iLow);
	ASSERT (iIndex != -1);
	TRange &sNew = m_rangeList.ElementAt (iIndex);
	sNew.SetUpper (iHigh);
	Merge (iIndex);
}

/////////////////////////////////////////////////////////////////////////////
// INEFFICIENT, still implemented the old way (doesn't take advantage of the
// range sorting)
void TRangeSet::Subtract(TRangeSet& sub, TRangeSet& result)
{
	int size = sub.m_rangeList.GetSize();

	for (int i = 0; i < size; ++i)
	{
		TRange& rng = sub.m_rangeList [i];

		for (int j = rng.Lower(); j <= rng.Upper(); ++j)
		{
			if (FALSE == Have( j ))
				result.Add ( j );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void TRangeSet::Subtract(RANGEINT iNum)
{
	if (!Have (iNum))
		return;

	int iIndex = FallsIn (iNum);
	TRange &sRange = m_rangeList [iIndex];

	if (sRange.Lower() == sRange.Upper())
		m_rangeList.RemoveAt (iIndex);
	else if (sRange.Lower () == iNum)
		sRange.SetLower (iNum + 1);
	else if (sRange.Upper () == iNum)
		sRange.SetUpper (iNum - 1);
	else {
		RANGEINT iOldUpper = sRange.Upper();
		sRange.SetUpper (iNum - 1);
		TRange sNew (iNum + 1, iOldUpper);
		m_rangeList.InsertAt (iIndex + 1, sNew);
	}
}

/////////////////////////////////////////////////////////////////////////////
void TRangeSet::Serialize (CArchive & ar)
{
	m_rangeList.Serialize(ar);

	// if reading, check the order and sort if needed.  Previous versions of
	// Gravity didn't keep the range set in order.
	// Also check whether any adjacent ranges can be merged, and if so, merge.
	// Previous versions of Gravity didn't keep the range set totally merged
	if (!ar.IsStoring()) {

		// check order and merging anomalies
		BOOL bFix = FALSE;
		int iSize = RangeCount ();
		for (int i = 1; i < iSize; i++)
			if (m_rangeList [i].Lower () < m_rangeList [i - 1].Lower () ||
				m_rangeList [i].Lower () - 1 <= m_rangeList [i - 1].Upper ()) {
					bFix = TRUE;
					break;
			}

			// if in need of fixing, fix
			if (bFix) {
				Sort ();
				MergeAll ();
			}
	}

	ASSERT (RangeSetOK (*this));
}

/////////////////////////////////////////////////////////////////////////////
void TRangeSet::Format (CString & str)
{
	int size = m_rangeList.GetSize ();
	int   i;

	str.Empty();
	CString elem;
	for (i = 0; i < size; i++)
	{
		elem.Format("[%d-%d]", m_rangeList[i].Lower(), m_rangeList[i].Upper() );
		str += elem;
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL TRangeSet::RemoveHead (RANGEINT *piReturn)
{
	if (IsEmpty ())
		return FALSE;

	TRange &sFirst = m_rangeList [0];
	*piReturn = sFirst.Lower ();
	if (sFirst.Lower () == sFirst.Upper ())
		m_rangeList.RemoveAt (0);
	else
		sFirst.SetLower (1 + sFirst.Lower ());

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL TRangeSet::IsEmpty()
{
	return (0 == RangeCount());
}

/////////////////////////////////////////////////////////////////////////////
int TRangeSet::RangeCount(void) const
{
	return m_rangeList.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
void TRangeSet::GetRange(int idx, int& low, int& hi) const
{
	low = m_rangeList[idx].Lower();
	hi  = m_rangeList[idx].Upper();
}

/////////////////////////////////////////////////////////////////////////////
void TRangeSet::Empty(void)
{
	m_rangeList.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
TRangeSet& TRangeSet::operator=(const TRangeSet& rhs)
{
	if (&rhs == this)
		return *this;

	int i, n = rhs.m_rangeList.GetSize();
	m_rangeList.SetSize(n);
	for (i = 0; i < n; ++i)
		m_rangeList[i] = rhs.m_rangeList.GetAt(i);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// FallsIn -- gives the index of the range where iNum falls, or -1 if the
// number doesn't fall in a range
int TRangeSet::FallsIn (RANGEINT iNum)
{
	int iCount = RangeCount ();

	// first test for out of bounds
	if (!iCount ||
		iNum < m_rangeList [0].Lower () ||
		iNum > m_rangeList [iCount - 1].Upper ())
		return -1;

	int iLowIndex = 0;
	int iHighIndex = iCount - 1;
	int iMiddleIndex;
	int iPrevMiddleIndex = -1;

	while (1) {
		// find middle range
		iMiddleIndex = (iLowIndex + iHighIndex) / 2;

		// test for iNum between two adjacent ranges
		if (iMiddleIndex == iPrevMiddleIndex)
			return -1;
		iPrevMiddleIndex = iMiddleIndex;

		// test for iNum in the current range
		RANGEINT iLow = m_rangeList [iMiddleIndex].Lower ();
		RANGEINT iHigh = m_rangeList [iMiddleIndex].Upper ();
		if (iLow <= iNum && iHigh >= iNum)
			return iMiddleIndex;

		// test for iNum in the range above the current range (to fix problem
		// with rounding off when dividing by 2 above)
		if (iMiddleIndex + 1 < iCount) {
			RANGEINT iLow = m_rangeList [iMiddleIndex + 1].Lower ();
			RANGEINT iHigh = m_rangeList [iMiddleIndex + 1].Upper ();
			if (iLow <= iNum && iHigh >= iNum)
				return iMiddleIndex + 1;
		}

		// adjust iLowIndex or iHighIndex
		if (iLow > iNum)
			iHighIndex = iMiddleIndex;
		else
			iLowIndex = iMiddleIndex;
	}
}

/////////////////////////////////////////////////////////////////////////////
// FallsBelow -- gives the index of the range immediately above iNum, or i+1
// where i is the last range if the number is larger than the last range's
// upper bound (assumes iNum doesn't fall in a range)
int TRangeSet::FallsBelow (RANGEINT iNum)
{
	ASSERT (FallsIn (iNum) == -1);

	int iCount = RangeCount ();

	// first test for out of bounds
	if (!iCount || iNum < m_rangeList [0].Lower ())
		return 0;

	if (iNum > m_rangeList [iCount - 1].Upper ())
		return iCount;

	int iLowIndex = 0;
	int iHighIndex = iCount - 1;
	int iMiddleIndex;
	int iPrevMiddleIndex = -1;

	while (1) {
		// find middle range
		iMiddleIndex = (iLowIndex + iHighIndex) / 2;

		// test for iNum below this range
		if (iMiddleIndex > 0 &&
			iNum < m_rangeList [iMiddleIndex].Lower () &&
			iNum > m_rangeList [iMiddleIndex - 1].Upper ())
			return iMiddleIndex;

		// test for iNum above this range
		if (iMiddleIndex + 1 < iCount &&
			iNum > m_rangeList [iMiddleIndex].Upper () &&
			iNum < m_rangeList [iMiddleIndex + 1].Lower ())
			return iMiddleIndex + 1;

		// check for infinite loop
		if (iMiddleIndex == iPrevMiddleIndex) {   // range list not sorted?
			ASSERT (0);
			CString str = "Internal error in FallsBelow()";
			throw(new TException (str, kFatal));
		}
		iPrevMiddleIndex = iMiddleIndex;

		// adjust iLowIndex or iHighIndex
		if (m_rangeList [iMiddleIndex].Lower () > iNum)
			iHighIndex = iMiddleIndex;
		else
			iLowIndex = iMiddleIndex;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Merge -- merge a range with its neighbors
void TRangeSet::Merge (int iIndex)
{
	TRange &sMiddle = m_rangeList [iIndex];

	// merge above until we can't anymore
	while (iIndex < RangeCount () - 1) {
		TRange &sUpper = m_rangeList [iIndex + 1];

		// if not possible to merge, exit the loop
		if (sMiddle.Upper () + 1 < sUpper.Lower ())
			break;

		// merge
		sMiddle.SetUpper (sUpper.Upper ());
		m_rangeList.RemoveAt (iIndex + 1);
	}

	// merge below until we can't anymore
	while (iIndex > 0) {
		TRange &sLower = m_rangeList [iIndex - 1];

		// if not possible to merge, exit the loop
		if (sMiddle.Lower () - 1 > sLower.Upper ())
			break;

		// merge
		sMiddle.SetLower (sLower.Lower ());
		m_rangeList.RemoveAt (iIndex - 1);
		iIndex --;
	}
}

/////////////////////////////////////////////////////////////////////////////
typedef struct {
	int iLow, iHigh;
} Range;

/////////////////////////////////////////////////////////////////////////////
int __cdecl CompareRanges (const void *pArg1, const void *pArg2)
{
	Range *pRange1 = (Range *) pArg1;
	Range *pRange2 = (Range *) pArg2;

	if (pRange1->iLow < pRange2->iLow)
		return -1;
	if (pRange1->iLow > pRange2->iLow)
		return 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Sort -- sorts the ranges in a range set... only called on deserializing
// to repair order written by older version of Gravity
void TRangeSet::Sort()
{
	int iCount = RangeCount ();
	Range *pRanges = new Range [iCount];
	if (!pRanges) {
		CString str; str.LoadString (IDS_ERR_OUT_OF_MEMORY);
		throw(new TException (str, kFatal));
	}

	// add the ranges to the Range array
	int i = 0;
	for (i = 0; i < iCount; i++) {
		pRanges [i].iLow = m_rangeList [i].Lower ();
		pRanges [i].iHigh = m_rangeList [i].Upper ();
	}

	qsort (pRanges, iCount, sizeof (Range), CompareRanges);

	// move the sorted ranges back to the range-set
	m_rangeList.RemoveAll ();
	for (i = 0; i < iCount; i++) {
		TRange sRange (pRanges [i].iLow, pRanges [i].iHigh);
		m_rangeList.Add (sRange);
	}

	delete pRanges;
}

/////////////////////////////////////////////////////////////////////////////
// MergeAll -- makes sure a range set is properly merged... only called on
// deserializing to repair non-merged rangesets written by older version of
// Gravity
void TRangeSet::MergeAll ()
{
	BOOL bFix = TRUE;

	// keep looping until no more fixes are applied
	while (bFix) {
		bFix = FALSE;

		// look for one anomaly and fix it
		int iSize = RangeCount ();
		int i;
		for (i = 1; i < iSize; i++)
			if (m_rangeList [i].Lower () - 1 <= m_rangeList [i - 1].Upper ()) {
				Merge (i);

				// loop again looking for another anomaly
				bFix = TRUE;
				break;
			}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CountItems -- count number of integers in this rangeset
int  TRangeSet::CountItems ()
{
	int low, high;
	int i, iTotal = 0;
	const int rangeTot = RangeCount();
	for (i = 0; i < rangeTot; i++)
	{
		GetRange ( i, low, high );
		iTotal += high - low + 1;
	}
	return iTotal;
}

/////////////////////////////////////////////////////////////////////////////
// DeleteRangesBelow -- erase any range sets that are below iLimit.
//                      Note: Does not split a range
void TRangeSet::DeleteRangesBelow (int iLimit)
{
	int low, high;

	// work backwards cuz we are deleting
	for (int i = RangeCount() - 1; i >= 0; i--)
	{
		GetRange ( i, low, high );

		if (low < iLimit && high < iLimit)
			m_rangeList.RemoveAt (i);
	}
}

#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////
// Dump --
void TRangeSet::Dump(CDumpContext& dc) const
{
	CObject::Dump( dc );
	dc << "TRangeSet\n";
}
#endif

//void TRangeSet::DebugPrint()
//{
//	int nCountOfRanges = m_rangeList.GetCount();
//	TRACE("TRangeSet (0x%08x) has %d ranges", this, nCountOfRanges);
//	if (!nCountOfRanges)
//	{
//		TRACE("\n");
//		return;
//	}
//
//	for (int i = 0; i < nCountOfRanges; i++)
//	{
//		TRACE("Range %d : Lower %d\t\tUpper %d\n", i, 
//			m_rangeList.GetAt(i).Lower(),
//			m_rangeList.GetAt(i).Upper());
//	}
//}

/////////////////////////////////////////////////////////////////////////////
// toString -- get printable human readable form
//
int TRangeSet::toString (CString & txt)
{
	int low, high;
	int i, iTotal = 0;
	const int rangeTot = RangeCount();
	CString  strAdd;
	bool fFirst = true;

	for (i = 0; i < rangeTot; i++)
	{
		GetRange ( i, low, high );

		if (low == high)
			strAdd.Format ("%d", low);
		else
			strAdd.Format ("%d-%d", low, high);

		if (!fFirst)
			txt += ",";
		fFirst = false;

		txt += strAdd;
	}
	return 0;
}
