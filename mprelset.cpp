/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mprelset.cpp,v $
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

/////////////////////////////////////////////////////////////////////////////
// MicroPlanet TRangeSet Implementation
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxtempl.h>
#include "mplib.h"
#include "pobject.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL  (TRelevance, CObject, 1)

/////////////////////////////////////////////////////////////////////////////
// SerializeElements - Serialize an array of TRelevance items.
/////////////////////////////////////////////////////////////////////////////

template <> void AFXAPI SerializeElements <TRelevance> (CArchive & ar, TRelevance *pItem, int nCount)
{
	for ( int i = 0; i < nCount; i++, pItem++ )
		pItem->Serialize (ar);
}

/////////////////////////////////////////////////////////////////////////////
// Assignment operator.
/////////////////////////////////////////////////////////////////////////////

TRelevance & TRelevance::operator=(const TRelevance & rhs)

{
	m_keyword = rhs.m_keyword;
	m_value   = rhs.m_value;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Serialize - Function to serialize a TRelevance object.
/////////////////////////////////////////////////////////////////////////////

void TRelevance::Serialize (CArchive & archive)
{
	// call base class function first
	CObject::Serialize( archive );

	if( archive.IsStoring() )
		archive << m_keyword << m_value;
	else
		archive >> m_keyword >> m_value;
}

//*************************************************************************//
// TRelevanceSet - Holds an array of keywords.
//*************************************************************************//

/////////////////////////////////////////////////////////////////////////////
// Add - Add a keyword/value pair to the relevance set.
/////////////////////////////////////////////////////////////////////////////

void TRelevanceSet::Add (const CString & keyword,
						 REL_VALUE       value)

{
	try
	{
		int   size = m_set.GetSize();

		for (int i = 0; i < size; i++)
			if ((m_set[i].GetKeyword()).CompareNoCase (keyword) == 0)
			{
				m_set[i].Set(keyword, value);
				return;
			}

			TRelevance  rel;
			rel.Set (keyword, value);
			m_set.Add (rel);
	}
	catch(TException *e)
	{
		throw e;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Del - Remove a keyword from the relevance set.
/////////////////////////////////////////////////////////////////////////////

void TRelevanceSet::Del (const CString & keyword)

{
	try
	{
		int   size = m_set.GetSize ();

		for (int i = 0; i < size; i++)
			if ((m_set[i].GetKeyword()).CompareNoCase (keyword) == 0)
			{
				m_set.RemoveAt (i);
				return;
			}
	}
	catch(TException *e)
	{
		throw e;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Get - Get the relevance for a keyword.  If not found it returns 0.
/////////////////////////////////////////////////////////////////////////////

REL_VALUE TRelevanceSet::Get (const CString & keyword)

{
	try
	{
		int   size = m_set.GetSize ();

		for (int i = 0; i < size; i++)
			if ((m_set[i].GetKeyword()).CompareNoCase (keyword) == 0)
				return m_set[i].GetValue();
		return 0;
	}
	catch(TException *e)
	{
		throw e;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Set - Set the relevance for an item, throws if it is not there.  Use
//       Add or ask me to change this if that is a problem.
/////////////////////////////////////////////////////////////////////////////

void TRelevanceSet::Set (const CString & keyword, REL_VALUE value)

{
	try
	{
		int   size = m_set.GetSize ();

		for (int i = 0; i < size; i++)
			if ((m_set[i].GetKeyword()).CompareNoCase (keyword) == 0)
			{
				m_set[i].Set (keyword, value);
				return;
			}

			throw(new TException (IDS_ERR_ITEM_NOT_IN_SET, kError));
	}
	catch(TException *e)
	{
		throw e;
	}
}

TRelevance & TRelevanceSet::operator[] (int pos)

{
	try
	{
		int   size;

		size = m_set.GetSize ();

		if (pos >= 0 && pos < size)
			return m_set[pos];

		throw(new TException (IDS_ERR_RELEVANCE_SET, kError));
		return m_set[pos];
	}
	catch(TException *e)
	{
		throw e;
	}
}

BOOL TRelevanceIterator::operator() (TRelevance * relevance)
{
	try
	{
		int   size;

		size = m_relSet.GetSize();

		if (m_index == size)
		{
			m_index = 0;
			return FALSE;
		}

		*relevance = m_relSet[m_index];
		m_index++;
		return TRUE;
	}
	catch(TException *e)
	{
		throw e;
	}
}
