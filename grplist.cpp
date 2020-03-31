/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: grplist.cpp,v $
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
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/10/03 08:21:06  richard_wood
/*  Tidying up code and comments.
/*
/*  Revision 1.2  2008/09/19 14:51:26  richard_wood
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

#include <tchar.h>
#include "stdafx.h"
#include "mplib.h"
#include "grplist.h"
#include "boyerc.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/****************************************************************************
Programmer Notes:       9-16-97

The group list has 2 disparate components, yet they are logically linked.

ElementArray - this array implicitly defines the alphabetical sort of
all the group names.  Each element contains the NameOffset.

m_pText -
has format:  <groupName1><SPACE><groupName2><SPACE><Name3><SPACE><NULL>
This is essentially one big string, so Boyer-Moore can search through
it, quickly.  The NameOffset points into this mondo string.

OK. Now, to complicate matters, there is a "type-search" search-stack;
this is implemented via m_pSearchStack. Each frame on the stack
logically holds a set of groupNames, but it is implemented by keeping
a set of NameOffsets.

TGroupList::NumGroups() and TGroupList::GetItem() return integers
*with respect to* the current search stack.  There is no similar stack
for the ElementArray; logically there should be.

****************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// TSearchStackItem Constructor:    Pre-allocate a table waiting for
//                                  some results.
/////////////////////////////////////////////////////////////////////////////
TSearchStackItem::TSearchStackItem ()
{
	// using malloc in this module because we need to realloc
	m_pOffsets = (LONG *) malloc (kSearchListExpand * sizeof (LONG));
	if (NULL == m_pOffsets)
		throw(new TException (IDS_ERR_ALLOC_RESULT, kError));

	m_arrayItems = kSearchListExpand;
	m_numItems = 0;
	m_pNext = 0;
}

/////////////////////////////////////////////////////////////////////////////
// TSearchStackItem Destructor - Get rid of any associated memory.  Assumes
//                               someone else it unhooking the pointer to
//                               the next item on the stack.
/////////////////////////////////////////////////////////////////////////////
TSearchStackItem::~TSearchStackItem ()
{
	if (m_pOffsets)
		free (m_pOffsets);
}

/////////////////////////////////////////////////////////////////////////////
// TSearchStackItem::Grow - make the table bigger to accomodate more items.
/////////////////////////////////////////////////////////////////////////////
void TSearchStackItem::Grow ()
{
	// lengthen the array by the desired amount...
	m_pOffsets = (LONG *) realloc (m_pOffsets,
		(m_arrayItems * sizeof (LONG)) +
		sizeof (LONG) * kSearchListExpand);
	if (NULL == m_pOffsets)
		throw(new TException (IDS_ERR_EXPAND_RESULT, kError));

	m_arrayItems += kSearchListExpand;
}

/////////////////////////////////////////////////////////////////////////////
// TSearchStackItem::AddResult - add a new result to the search stack.
/////////////////////////////////////////////////////////////////////////////
void TSearchStackItem::AddResult (LONG offset)
{
	if (m_numItems == m_arrayItems)
		Grow ();
	m_pOffsets[m_numItems++] = offset;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList Constructor - Preallocate the first chunks of memory and
//                          get ready to stuff in some groups.
/////////////////////////////////////////////////////////////////////////////
TGroupList::TGroupList()
{
	// next insert starts off as 0
	m_nextInsert   = 0;
	m_pSearchStack = 0;
	m_numGroups    = 0;

#ifdef _DEBUG
	m_checkSum     = 0;
#endif

	m_pText = (TCHAR *) malloc (kGroupTextExpand);
	if (m_pText == NULL)
	{
		CString str; str.LoadString (IDS_ERR_GROW_TEXT);
		throw(new TMemoryException (str, kError));
	}

	m_textSize = kGroupTextExpand;
	m_pText[0] = '\0';

	m_iElemCount = kGroupElemExpand;
	m_pElem = (GroupListElem *) malloc (m_iElemCount * sizeof(GroupListElem));
	if (m_pElem == NULL)
	{
		free (m_pText);
		CString str; str.LoadString (IDS_ERR_GROW_ELEMENTS_MEMORY);
		throw(new TMemoryException (str, kError));
	}
	m_iGapIndex = 0;
	m_iGapSize  = m_iElemCount;

	// special boolean for when searching for our separator character
	m_fEmptySet = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList Destructor - Free up the big blocks of memory.
/////////////////////////////////////////////////////////////////////////////
TGroupList::~TGroupList ()
{
	Empty ();
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::GrowText - Grow the text region by the incremental amount.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::GrowText()
{
	if (NULL == (m_pText = (TCHAR *) realloc (m_pText, m_textSize + kGroupTextExpand)))
	{
		CString str; str.LoadString (IDS_ERR_GROW_TEXT);
		throw(new TMemoryException (str, kError));
	}
	m_textSize += kGroupTextExpand;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::GrowElements - Grow the element memory by the desired amount.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::GrowElements()
{
	DWORD sz = (m_iElemCount + kGroupElemExpand) * sizeof(GroupListElem);

	m_pElem = (GroupListElem*) realloc (m_pElem, sz);
	if (NULL == m_pElem)
	{
		CString str; str.LoadString (IDS_ERR_GROW_ELEMENTS);
		throw(new TMemoryException (str, kError));
	}

	m_iGapIndex = m_iElemCount;         // gap is at end
	m_iGapSize  = kGroupElemExpand;

	m_iElemCount += kGroupElemExpand;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::Compare - Compare the group to the item pointed to in
//                       the offset table.
/////////////////////////////////////////////////////////////////////////////
int TGroupList::Compare (int iElemIndex, LPCTSTR group) const
{
	int      i = 0;
	TCHAR *  ptr;
	TCHAR *  pStart;

	ptr = pStart = m_pText + GetElemAt(iElemIndex)->lOffset;

	ptr = _tcschr ( pStart, kGroupSeparator );

	ASSERT(ptr > pStart);
	*ptr = 0;              // temporarily insert a null over separator

	i = _tcscmp (group, pStart);

	*ptr = kGroupSeparator;

	return i;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::FindGroup - Find a group in the table doing a binary search
//                         and return either the index of the item greater
//                         or where the item occurs.
//
// 10-14-97  amc  Removed 'TCHAR grp[215]'
/////////////////////////////////////////////////////////////////////////////
BOOL TGroupList::FindGroup (LPCTSTR group, int & spot) const
{
	int   left = 0;
	int   right = m_numGroups - 1;
	int   index;
	int   result;

	if (m_numGroups == 0)
	{
		spot = 0;
		return FALSE;
	}

	do
	{
		index = (left + right) / 2;
		if ((result = Compare (index, group)) == 0)
		{
			spot = index;
			return TRUE;
		}
		if (result < 0)
			right = index - 1;
		else
			left  = index + 1;
	}
	while (left <= right);

	if (result < 0)
		spot = index;
	else
		spot = index + 1;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::AddGroup - Add a group to the container.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::AddGroup(LPCTSTR   group,
						  WORD      numArticles,
						  TGlobalDef::ENewsGroupType eType)
{
	int   index;
	LONG  newOffset;

	if (FindGroup(group, index))
		return;

	newOffset = m_nextInsert;

	// check if we have space for the string, a separator, and a null terminator
	if ((m_textSize - (m_nextInsert + 2)) < (LONG) _tcslen (group))
		GrowText();
	if (m_numGroups == m_iElemCount)
		GrowElements();

	// write the text at the end...
	_tcscpy ((m_pText + m_nextInsert), group);
	m_nextInsert += _tcslen (group);

	// output a separator
	*(m_pText + m_nextInsert) = kGroupSeparator;
	m_nextInsert++;

	// whole list is null terminated for use w/ strstr
	*(m_pText + m_nextInsert) = 0;

	// now shift the Element array

	GroupListElem* pEntry = InsertElem( index );

	pEntry->lOffset         = newOffset;
	pEntry->wNumArticles    = numArticles;
	pEntry->byNewsgroupType = (BYTE) eType;

	m_numGroups++;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::Empty - Empty out the container
/////////////////////////////////////////////////////////////////////////////
void TGroupList::Empty()
{
	// just free up the memory and reset the variables
	if (NULL != m_pText)
	{
		free (m_pText);
		m_pText = 0;
	}

	if (NULL != m_pElem)
	{
		free (m_pElem);
		m_pElem = 0;
	}

	if (m_pSearchStack)
		ClearSearch ();

	m_iElemCount = 0;
	m_numGroups = 0;
	m_textSize = 0;
	m_nextInsert = 0;
	m_iGapIndex = 0;
	m_iGapSize = 0;
}

/////////////////////////////////////////////////////////////////////////////
// TGroupList::Serialize - read or write the container from/to an archive.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::Serialize (CArchive & arc)
{
	CString  str;
	LONG     currOffset = 0;
	int      i;
	LONG     sizeElem;
	BYTE     byVersion;

	PObject::Serialize (arc);

	byVersion = GetObjectVersion();

	if (arc.IsStoring())
	{
		CompactData();

		LONG  sizeTextRegion = m_nextInsert + 1;
		sizeElem = m_numGroups * sizeof(GroupListElem);

		arc << sizeTextRegion;
		arc << m_numGroups;
		arc << sizeElem;

		// here we're going to write the groups out sorted so
		// they come back that way...
		//TGlobalDef::ENewsGroupType eType;

		for (i = 0; i < m_numGroups; i++)
		{
			get_item (i, str);
			arc.Write ((LPCTSTR) str, str.GetLength ());
			arc << (BYTE) kGroupSeparator;
			m_pElem[i].lOffset = currOffset;
			currOffset += str.GetLength() + 1;
		}

		// write out the zero
		arc << (BYTE) '\0';
		arc.Write ((void*)m_pElem, sizeElem);
	}
	else
	{
		// make sure things are clear
		Empty ();
		arc >> m_textSize;
		arc >> m_numGroups;
		arc >> sizeElem;

		m_pText = (TCHAR *) malloc (m_textSize);
		if (NULL == m_pText)
		{
			CString str; str.LoadString (IDS_ERR_ALLOC_TEXT_REGION);
			throw(new TMemoryException (str, kError));
		}

		m_pElem = (GroupListElem *) malloc (sizeElem);
		if (NULL == m_pElem)
		{
			CString str; str.LoadString (IDS_ERR_ALLOC_TABLE);
			throw(new TMemoryException (str, kError));
		}
		m_iElemCount = m_numGroups;

		arc.Read ((void *) m_pText, m_textSize);
		arc.Read ((void *) m_pElem, sizeElem);

		// offset of trailing 0 is at the end
		m_nextInsert = m_textSize - 1;

		m_iGapIndex = m_iElemCount;
		m_iGapSize  = 0;

#ifdef _DEBUG
		m_checkSum = 0;
		for (int i = 0; i < m_nextInsert; i++)
			m_checkSum += m_pText[i];

#endif
	}
}

#if defined(_DEBUG)
/////////////////////////////////////////////////////////////////////////////
// GroupList::Print - Dump the contents to a text file.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::Print (CStdioFile & outfile)
{
}
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int TGroupList::NumGroups ()
{
	if (m_pSearchStack)
		return m_pSearchStack->m_numItems;
	else
		return m_numGroups;
}

/////////////////////////////////////////////////////////////////////////////
// Get the group at the index desired.
/////////////////////////////////////////////////////////////////////////////
int TGroupList::GetItem ( int      iMutableIndex,
						 CString& group,
						 WORD &   numArticles,
						 TGlobalDef::ENewsGroupType& eType) const
{
	ASSERT(iMutableIndex >= 0);
	ASSERT(iMutableIndex < m_numGroups);

	if (iMutableIndex < 0 || iMutableIndex >= m_numGroups)
	{
		CString  temp;
		temp.Format (IDS_INVALID_INDEX_GETITEM, iMutableIndex);
		throw(new TException (temp, kError));
	}

	GroupListElem * pElem = 0;

	int  off = get_offset (iMutableIndex, &pElem);

	if (off < 0)
		throw(new TException(IDS_ERR_INVALID_GROUPLIST_OFFSET, kError));

	// Extract the name

	ExtractGroupName ( off, group );

	// if no ptr set yet...
	if (0 == pElem)
	{
		int iElemIdx = 0;

		// from the Name, find the stable elem index.

		VERIFY(GroupExist (group, &iElemIdx));
		pElem = GetElemAt (iElemIdx);
	}

	numArticles = pElem->wNumArticles;
	eType       = (TGlobalDef::ENewsGroupType) pElem->byNewsgroupType;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//
//  4-12-96 amc  We have to handle the Space char special.
void TGroupList::Search (LPCTSTR pSubString)
{
	int               offset = 0;
	int               pos;
	LPCTSTR           pGroup;
	TSearchStackItem *pResults;
	LONG              resultOffset;
	TBoyerMoore       bm(pSubString, TRUE);

#ifdef _DEBUG
	WORD checksum = 0;
	for (int i = 0; i < m_nextInsert; i++)
		checksum += m_pText[i];
	if (checksum != m_checkSum)
		ASSERT (FALSE);
#endif

	// bug out if we see a Space character - it's our separator!
	if (_tcschr(pSubString, kGroupSeparator))
	{
		if (m_pSearchStack)
			ClearSearch ();

		// return an empty set - No newsgroup is supposed to have a Space
		//   in the name.
		m_pSearchStack = new TSearchStackItem;
		m_fEmptySet = TRUE;
		return;
	}
	else if (m_fEmptySet)
	{
		if (m_pSearchStack)
			ClearSearch ();
		m_fEmptySet = FALSE;
	}

	if (m_pSearchStack)
	{
		// if the substring is the same as the last, but one character shorter,
		// pop the stack and return...
		int subLen = _tcslen (pSubString);

		if ( subLen == (m_pSearchStack->m_pattern.GetLength() - 1) &&
			_tcscmp (pSubString, m_pSearchStack->m_pattern.Left (subLen)) == 0)
		{
			PopSearch ();
			if (m_pSearchStack)
				return;
		}
		// if the pattern is not the same as the last, but one char longer
		// we have to make sure that the old result is a subset of the pattern
		else if (CString(pSubString).Find (m_pSearchStack->m_pattern) != 0)
			ClearSearch ();
	}

	// allocate a new set of results
	pResults = new TSearchStackItem;

	if (NumGroups () < kSimpleSearchThreshhold)
	{
		CString strGroup;
		for (int i = 0; i < NumGroups(); i++)
		{
			if (0 == get_item (i, strGroup) && (-1 != strGroup.Find (pSubString)))
			{
				pResults->AddResult (get_offset (i));
			}
		}
	}
	else
	{
		// do boyer-moore string search repeatedly to get the results
		while (TRUE == bm.Search (m_pText + offset, m_nextInsert - offset, pos))
		{
			// move backward to the beginning of the buffer or a space
			pGroup = m_pText + offset + pos;
			while ((pGroup != m_pText) && (*pGroup != kGroupSeparator))
				pGroup--;

			// now copy the group into the buffer...
			if (pGroup != m_pText)
				pGroup++;

			resultOffset = pGroup - m_pText;

			pResults->AddResult (resultOffset);

			// skip ahead to the next one
			while (*pGroup++ != kGroupSeparator)
				;

			--pGroup;

			// the new offset is at the space behind our found item
			offset = (pGroup - m_pText);
		}
	}

	pResults->m_pNext = m_pSearchStack;
	pResults->m_pattern = pSubString;
	m_pSearchStack = pResults;
}

/////////////////////////////////////////////////////////////////////////////
// TSearchStackItem::PopSearch - Pop the topmost set of results off the
//                               search stack.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::PopSearch ()
{
	TSearchStackItem * pItem;

	if (!m_pSearchStack)
		throw(new TException (IDS_ERR_STACK_EMPTY, kError));

	pItem = m_pSearchStack->m_pNext;
	delete m_pSearchStack;
	m_pSearchStack = pItem;
}

/////////////////////////////////////////////////////////////////////////////
// ClearSearch - Removes all search results from the search stack.
/////////////////////////////////////////////////////////////////////////////
void TGroupList::ClearSearch ()
{
	TSearchStackItem *pItem = m_pSearchStack;

	while (pItem)
	{
		pItem = m_pSearchStack->m_pNext;
		delete m_pSearchStack;
		m_pSearchStack = pItem;
	}
}

/////////////////////////////////////////////////////////////////////////////
// operator += add elements from another group list into the current one.
/////////////////////////////////////////////////////////////////////////////
TGroupList & TGroupList::operator += (const TGroupList & rhs)
{
	CString s;
	WORD    numArticles;
	TGlobalDef::ENewsGroupType eType;

	for (int i = 0; i < rhs.m_numGroups; i++)
	{
		rhs.GetItem(i, s, numArticles, eType);
		AddGroup (s, numArticles, eType);
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// Retrieve special byte at the end of the group name.
// if not found, return 0;
//
BYTE TGroupList::GetGroupType(LPCTSTR group)
{
	int spot = -1;

	BOOL fFound = FindGroup (group, spot);
	if (!fFound)
		return 0;

	return GetElemAt(spot)->byNewsgroupType;
}

///////////////////////////////////////////////////////////////////
// Move data around and return a ptr to a valid parking space
//
// Implementation:
//    Normally would be a simple array of integers that you manipulate
//    with memmove.  I find this is rather expensive. In fact, most of the
//    time we are inserting consecutive indices, so I will maintain
//    a gap in the array:
//
//    0 1 2 3 ? ? ? 4 5 6
//
//    if the next element to be inserted goes to index #4, then we
//    eliminate a call to memmove
//
TGroupList::GroupListElem* TGroupList::InsertElem(int index)
{
	ASSERT(m_iGapSize > 0);

	int shift;

	// does it fall into the gap?
	//                                     the == case works
	//                                       add to front of hi data <-> add
	//                                       to back of lo data
	if ((index >= m_iGapIndex) && (index <= m_iGapIndex + m_iGapSize))
	{
		if (index > m_iGapIndex)
		{
			// pull some down from upper chunk
			shift = index - m_iGapIndex;
			memmove ( m_pElem + m_iGapIndex,
				m_pElem + m_iGapIndex + m_iGapSize,
				shift * sizeof(GroupListElem) );
			m_iGapIndex += shift;
		}

		m_iGapSize--;
		return m_pElem + m_iGapIndex++;
	}

	if (index < m_iGapIndex)
	{
		shift = m_iGapIndex - index;
		memmove ( m_pElem + m_iGapIndex + m_iGapSize - shift,
			m_pElem + index,
			shift * sizeof(GroupListElem) );
		m_iGapSize--;
		m_iGapIndex = index + 1;
		return m_pElem + index;
	}

	// index falls into the high data chunk  (put gap after it)
	// backfill
	shift = index - m_iGapIndex;

	memmove ( m_pElem + m_iGapIndex,
		m_pElem + m_iGapIndex + m_iGapSize,
		shift * sizeof(GroupListElem) );

	m_iGapSize--;
	m_iGapIndex += shift;
	return m_pElem + m_iGapIndex++;
}

///////////////////////////////////////////////////////////////////
// Move the gap to the end
//
void TGroupList::CompactData(void)
{
	if (m_iGapSize == 0)
		return;

	if (m_iGapIndex == m_numGroups)
		return;

	int shift = m_iElemCount - (m_iGapIndex + m_iGapSize);
	memmove ( m_pElem + m_iGapIndex,
		m_pElem + m_iGapIndex + m_iGapSize,
		shift * sizeof(GroupListElem) );
	m_iGapIndex += shift;
}

// ------------------------------------------------------------------------
// Returns TRUE if the group is in our list
BOOL TGroupList::GroupExist (LPCTSTR group, int * pIdx) const
{
	return FindGroup ( group, *pIdx );
}

// ------------------------------------------------------------------------
// Slightly less work intensive version, for internal use
int TGroupList::get_item (int iMutableIndex, CString& group) const
{
	ASSERT(iMutableIndex >= 0);
	ASSERT(iMutableIndex < m_numGroups);

	if (iMutableIndex < 0 || iMutableIndex >= m_numGroups)
	{
		CString  temp;
		temp.Format (IDS_INVALID_INDEX_GETITEM, iMutableIndex);
		throw(new TException (temp, kError));
	}

	int off = get_offset (iMutableIndex);

	return ExtractGroupName (off, group);
}

// ------------------------------------------------------------------------
int TGroupList::ExtractGroupName (int offset, CString& group) const
{
	LPTSTR p = m_pText + offset;

	if (p >= (m_pText + m_textSize))
		throw(new TException(IDS_ERR_OUTSIDE_TEXT, kError));

	TCHAR * end = _tcschr (p, kGroupSeparator);
	int total = (end - p) / sizeof(TCHAR);

	LPTSTR pInner = group.GetBuffer (total+1);
	_tcsncpy (pInner, p, total);
	pInner[total] = '\0';
	group.ReleaseBuffer (total);
	return 0;
}

// ------------------------------------------------------------------------
//
int TGroupList::get_offset (int iMutableIndex,
							GroupListElem ** ppElem /* =0 */) const
{
	if (m_pSearchStack)
		return m_pSearchStack->m_pOffsets[iMutableIndex];
	else
	{
		// iElemIndex == iMutableIndex

		GroupListElem  * pFoundElem = GetElemAt(iMutableIndex);

		if (ppElem)
			*ppElem = pFoundElem;

		return pFoundElem->lOffset;
	}
}

