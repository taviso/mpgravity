/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artrefs.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:11  richard_wood
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
#include "artrefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(TFlatStringArray, CObject, 1)
IMPLEMENT_SERIAL(T822References, TFlatStringArray, 1)

TFlatStringArray::TFlatStringArray()
{
	m_dataSize = m_count = 0;
	m_pText = 0;
	m_pData = 0;
}

TFlatStringArray::~TFlatStringArray()
{
	cleanUp();
}

// Free memory
void TFlatStringArray::cleanUp()
{
	m_count = 0;
	m_dataSize = 0;
	if (m_pData) {
		free( m_pData );
		m_pData = 0;
	}
	m_pText = 0;
}

void TFlatStringArray::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_count;
		if (m_count)
		{
			ar << m_dataSize;
			ar.Write(m_pData, m_dataSize);
		}
	}
	else
	{
		cleanUp();
		ar >> m_count;
		if (m_count)
		{
			ar >> m_dataSize;

			m_pData = (LPTSTR) malloc(m_dataSize);
			if (0 == m_pData)
				AfxThrowMemoryException();
			ar.Read(m_pData, m_dataSize);
			m_pText = (LPTSTR) (((BYTE*) m_pData) + m_count*sizeof(LONG));
		}
	}
}

TFlatStringArray&
TFlatStringArray::operator= (const TFlatStringArray& rhs)
{
	if (this == &rhs)
		return *this;

	cleanUp();
	m_count = rhs.m_count;
	m_dataSize = rhs.m_dataSize;
	m_pData = (LPTSTR)malloc(m_dataSize);
	if (0 == m_pData)
		AfxThrowMemoryException();

	CopyMemory(m_pData, rhs.m_pData, m_dataSize);

	m_pText = (LPTSTR) ((PBYTE) m_pData + (m_count*sizeof(LONG)));
	return *this;
}
void TFlatStringArray::AddString(LPCTSTR str)
{
	int addLen = _tcslen(str) + 1;
	int addSize = addLen * sizeof(TCHAR);

	if (0 == m_pData)
	{
		// table (with 1 entry) + string
		m_pData = (LPTSTR) malloc(addSize + sizeof(LONG));
		if (0 == m_pData)
			AfxThrowMemoryException();
		*((LONG*)m_pData) = 0;
		m_pText = (LPTSTR) ((PBYTE) m_pData + sizeof(LONG));
		_tcscpy(m_pText, str);
		m_count = 1;
		m_dataSize = addSize + sizeof(LONG);
		return;
	}
	else
	{
		// dont' forget the new Table Entry
		LPTSTR pMore = (LPTSTR) realloc(m_pData,
			m_dataSize + addSize + sizeof(LONG));
		if (0==pMore)
			AfxThrowMemoryException();
		m_pData = pMore;
		m_pText = (LPTSTR) ( (PBYTE)m_pData + m_count*sizeof(LONG) );

		// shift all strings down by 1 Long
		MoveMemory((PBYTE) m_pText + sizeof(LONG), m_pText,
			m_dataSize - (sizeof(LONG) * m_count));

		// copy new string to the new end
		PBYTE pOldEnd = (PBYTE) (m_pData + (m_dataSize/sizeof(TCHAR)));
		LPTSTR pAddOn = (LPTSTR) (pOldEnd + sizeof(LONG));
		_tcscpy(pAddOn, str);

		m_dataSize += addSize + sizeof(LONG);
		m_count++;

		m_pText = (LPTSTR) ( (PBYTE)m_pData + m_count*sizeof(LONG) );

		// put offset Entry into the table
		((LONG*) m_pData)[m_count-1] = ((PBYTE)pAddOn - (PBYTE)m_pText) / sizeof(TCHAR);
	}
}

int TFlatStringArray::get_offset(int i) const
{
	return *( ((LONG*) m_pData) + i);
}

LPCTSTR TFlatStringArray::get_string(int i) const
{
	return m_pText + get_offset(i);
}

// Old format
void TFlatStringArray::FillStringList(CStringList* pList) const
{
	for (int i = 0; i < m_count; ++i)
	{
		LPCTSTR item = get_string(i);
		pList->AddTail( item );
	}
}

// tells how many strings are stored
int TFlatStringArray::GetSize () const
{
	return m_count;
}

///////////////////////////////////////////////////////////////////////////
// Derived Class
//

T822References::T822References()
{
}

T822References::~T822References()
{
}

void T822References::Serialize(CArchive & ar)
{
	// just do base class stuff
	TFlatStringArray::Serialize( ar );
}

void T822References::GetFirstRef(CString& str)
{
	if (0 == m_count)
		return;
	LPCTSTR pSource = get_string(0);
	str += pSource;
}

void T822References::GetLastRef(CString& str)
{
	if (m_count > 0)
	{
		int offset = get_offset(m_count - 1);

		str = LPCTSTR(m_pText + offset);
	}
}

int T822References::GetCount()
{
	return m_count;
}

BOOL T822References::Find(const CString & msgID)
{
	LPCTSTR pHaystack;

	for (int i = 0; i < m_count; ++i)
	{
		pHaystack = get_string(i);
		if (0 == msgID.Compare(pHaystack))
			return TRUE;
	}
	return FALSE;
}

void T822References::CopyReferences(const T822References & src)
{
	cleanUp();
	m_count    = src.m_count;
	m_dataSize = src.m_dataSize;

	if (m_dataSize)
	{
		m_pData = (LPTSTR) malloc(m_dataSize);
		if (0 == m_pData)
			AfxThrowMemoryException();

		CopyMemory(m_pData, src.m_pData, m_dataSize);
		m_pText = (LPTSTR) (((BYTE*) m_pData) + (m_count*sizeof(LONG)));
	}
}

void
T822References::ConstructReferences(const T822References & src,
									const CString& msgid)
{
	cleanUp();
	m_count = src.m_count + 1;
	m_dataSize = src.m_dataSize + sizeof(LONG) + (msgid.GetLength() + 1);
	m_pData = (LPTSTR) malloc(m_dataSize);
	if (0 == m_pData)
		AfxThrowMemoryException();

	m_pText = (LPTSTR) ( ((BYTE*)m_pData) + (m_count * sizeof(LONG)) );

	// copy strings from source
	int sourceTextSize = src.m_dataSize - (src.m_count * sizeof(LONG));
	CopyMemory(m_pText, src.m_pText, sourceTextSize);

	// copy the new element
	_tcscpy(m_pText + (sourceTextSize/sizeof(TCHAR)), msgid);

	// copy table from source
	CopyMemory(m_pData, src.m_pData, src.m_count * sizeof(LONG));

	// index for new element
	((LONG*) m_pData)[m_count-1] = sourceTextSize/sizeof(TCHAR);
}

// The line has: <GreatGranddad><Grandad><Dad>
void T822References::SetReferences(const CString& inRefs)
{
	cleanUp();

	int iCurOffset = 0;
	int iStartChar = 0;
	LPTSTR startCopy, endCopy;
	LPCTSTR p = inRefs;
	LPTSTR head;
	LPCTSTR start = p;
	// Assume every reference begins with '<'
	while (*p)
		if (*p++ == '<') iStartChar++;
	if (iStartChar <= 0)
		return;

	int iLen = inRefs.GetLength();

	// this is probably oversized - that's ok
	m_dataSize = (iStartChar+iLen) * sizeof(TCHAR) + (iStartChar*sizeof(LONG));
	m_pData = (LPTSTR) malloc(m_dataSize);
	if (0 == m_pData)
		AfxThrowMemoryException();
	m_count = iStartChar;
	m_pText = (LPTSTR) (((BYTE*)m_pData) + m_count*sizeof(LONG));

	LONG* pTable = (LONG*) m_pData;
	head = m_pText;
	// reset p
	p = start;
	for (int i = 0; i < m_count; ++i)
	{
		pTable[i] = (head - m_pText) / sizeof(TCHAR);
		while (*p && ('<' != *p)) p++;              // find start of ref

		startCopy = head;
		*head++ = *p++;                             // copy <

		while (*p && ('<' != *p))
		{
			*head++ = *p++;                          // copy 1 ref
		}
		endCopy = head-1;
		*head++ = '\0';                             // cap off copy

		// TrimRight
		while (endCopy > startCopy)
		{
			if (isspace(*endCopy))
			{
				*endCopy = '\0';
				--head;
			}
			else
				break;
			--endCopy;
		}
	}
	m_dataSize = (head - m_pData);

}

// Returns a space separated list
void T822References::MakeWhiteSpaceList(CString& out)
{
	out.Empty();
	int itemLen;
	int i;
	if (0 == m_count)
		return;

	LPCTSTR pSrc = m_pText;
	LPTSTR pOut = out.GetBuffer(m_dataSize);
	LPTSTR pStart = pOut;
	for (i = 0; i < m_count; ++i)
	{
		itemLen = _tcslen(pSrc);
		_tcscpy(pOut, pSrc);
		if (i < m_count-1)
			*(pOut + itemLen) = ' ';
		pOut += itemLen + 1;
		pSrc += itemLen + 1;
	}
	i = (pOut - pStart - 1)/sizeof(TCHAR);
	out.ReleaseBuffer(i);
}
