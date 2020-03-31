/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: servrang.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:51  richard_wood
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
#include "crc32.h"
#include "servrang.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL (TServerRangeSet, PObject, 1)
IMPLEMENT_SERIAL (TCrcKey, PObject, 1)

/////////////////////////////////////////////////////////////////////////////
// TCrcKey::operator== - This is used by the CMap to compare keys...
/////////////////////////////////////////////////////////////////////////////

BOOL TCrcKey::operator==(const TCrcKey & key) const
{
	if (&key == this)
		return TRUE;

	if ((this->m_whole == key.m_whole) &&
		(this->m_half  == key.m_half))
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// TCrcKey::Serialize - Read or write a TCrcKey element from or to an archive.
/////////////////////////////////////////////////////////////////////////////

void TCrcKey::Serialize (CArchive & ar)
{
	if (ar.IsStoring())
	{
		ar << m_whole;
		ar << m_half;
	}
	else
	{
		ar >> m_whole;
		ar >> m_half;
	}
}

/////////////////////////////////////////////////////////////////////////////
// HashKey - Helper function for CMap...
/////////////////////////////////////////////////////////////////////////////

template <> UINT AFXAPI HashKey (TCrcKey & crcKey)
{
	DWORD key = crcKey.m_whole + crcKey.m_half;
	return ((UINT)(void*)(DWORD)key) >> 4;
}

/////////////////////////////////////////////////////////////////////////////
// GetRangeSet - takes the name of a group and returns a pointer
//               to its range set.  If the group is not found, it is
//               added automatically.  This is called from server.cpp and
//               THAT object has a mutex
/////////////////////////////////////////////////////////////////////////////

TRangeSet *TServerRangeSet::GetRangeSet(LPCTSTR group)
{
	TRangeSet *pSet;
	TCrcKey    key;

	// form the key...
	make_key ( group, &key );

	if (IsSubscribedName (key))
	{
		if (!m_rangeMap.Lookup (key, pSet))
		{
			pSet = new TRangeSet;
			m_rangeMap.SetAt (key, pSet);
		}

		return pSet;
	}
	else
	{
		return NULL;
	}
}

void TServerRangeSet::RemoveRangeSet (LPCTSTR group)
{
	TRangeSet *pSet;
	TCrcKey    key;

	WriteLock ();

	// form the key...
	make_key ( group, &key );

	if (m_rangeMap.Lookup (key, pSet))
	{
		m_rangeMap.RemoveKey (key);
		delete pSet;
	}

	UnlockWrite ();
}

/////////////////////////////////////////////////////////////////////////////
// SerializeRangeMap -- used to serialize a TServerRangeMap
static void SerializeRangeMap (CArchive &ar, TServerRangeMap &map)
{
	ASSERT_VALID(&map);

	if (ar.IsStoring()) {
		ar.WriteCount (map.GetCount ());
		if (map.GetCount () == 0)
			return;  // nothing more to do

		POSITION pos = map.GetStartPosition ();
		while (pos) {
			TCrcKey sKey;
			TRangeSet *pRangeSet;
			map.GetNextAssoc (pos, sKey, pRangeSet);
			sKey.Serialize (ar);
			pRangeSet -> Serialize (ar);
		}
	}
	else {
		DWORD nNewCount = ar.ReadCount();
		TCrcKey sKey;
		TRangeSet *pRangeSet;
		while (nNewCount--) {
			pRangeSet = new TRangeSet;
			sKey.Serialize (ar);
			pRangeSet -> Serialize (ar);
			map.SetAt (sKey, pRangeSet);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Serialize - Write a TServerRangeSet object out to disk or read it in.
/////////////////////////////////////////////////////////////////////////////

void TServerRangeSet::Serialize (CArchive & ar)
{
	if (ar.IsStoring())
	{
		ReadLock ();
		PObject::Serialize (ar);
		SerializeRangeMap (ar, m_rangeMap);
		UnlockRead ();
	}
	else
	{
		WriteLock ();
		PObject::Serialize (ar);
		SerializeRangeMap (ar, m_rangeMap);
		UnlockWrite ();
	}
}

/////////////////////////////////////////////////////////////////////////////
TServerRangeSet::TServerRangeSet ()
{
}

/////////////////////////////////////////////////////////////////////////////
// free the range-sets that we're holding in the map
TServerRangeSet::~TServerRangeSet ()
{
	POSITION pos = m_rangeMap.GetStartPosition ();
	TCrcKey sKey;
	TRangeSet *pRangeSet;
	while (pos) {
		m_rangeMap.GetNextAssoc (pos, sKey, pRangeSet);
		delete pRangeSet;
	}
	m_rangeMap.RemoveAll ();
}

// ------------------------------------------------------------------------
void TServerRangeSet::make_key (LPCTSTR group, TCrcKey* pKey)
{
	int iLen = _tcslen (group);
	pKey->m_whole = CRC32 ((unsigned char *) group, iLen);
	pKey->m_half  = CRC32 ((unsigned char *) group, iLen / 2);
}

// ------------------------------------------------------------------------
// We keep X-post info only for groups that are subscribed
void TServerRangeSet::AddSubscribedName (LPCTSTR group)
{
	TCrcKey key;
	TRangeSet * pSet = (TRangeSet*)(void*)0xCAFE;

	make_key ( group, &key );

	if (!m_subscribed.Lookup (key, pSet))
		m_subscribed.SetAt (key, (TRangeSet*) pSet);
}

// ------------------------------------------------------------------------
void TServerRangeSet::RemoveSubscribedName (LPCTSTR group)
{
	TCrcKey key;

	make_key ( group, &key );

	m_subscribed.RemoveKey ( key );
}

// ------------------------------------------------------------------------
BOOL TServerRangeSet::IsSubscribedName (TCrcKey & key)
{
	TRangeSet * pSet = 0;  // dummy
	return m_subscribed.Lookup (key, pSet);
}

// ------------------------------------------------------------------------
// EraseGroupInfo -- Removes info for non-subscribed groups.  Caller
//   must have write lock.
// History:  builds <= 556 wrote out ReadRange info for x-posted groups,
//       even if they were not subscribed. We are doing a cleanup after
//       the fact.
int  TServerRangeSet::EraseGroupInfo (void)
{
	TCrcKey key;
	TRangeSet* pSet = 0;
	TRangeSet* pTest = 0;

	POSITION posRng = m_rangeMap.GetStartPosition ();
	while (posRng)
	{
		m_rangeMap.GetNextAssoc (posRng, key, pSet);

		// does this dataobject correspond to a subscribed newsgroup ?
		if (!m_subscribed.Lookup (key, pTest))
		{
			m_rangeMap.RemoveKey (key);
			delete pSet;
		}
	}
	return 0;
}
