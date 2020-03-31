/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: bucket.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.2  2008/09/19 14:51:14  richard_wood
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

////////////////////////////////////////////////////////////////
//
//  a thread-safe ptrlist  (Perhaps a Template would be better)
//
#pragma once

#include "critsect.h"

// this class is so basic, it's like embarassing
class TSyncBucket
{
public:
	TSyncBucket();
	virtual ~TSyncBucket () = 0;

	// interface for external locking
	void Lock ()    { EnterCriticalSection ( &m_CriticalSection ); }
	void Unlock ()  { LeaveCriticalSection ( &m_CriticalSection ); }

protected:
	CRITICAL_SECTION  m_CriticalSection;
};

/// Wrapper for CPtrList

class TPtrBucket : public TSyncBucket
{
public:
	~TPtrBucket(void);

	// mimic the CPtrList
	POSITION AddHead(LPVOID pVoid);
	POSITION AddTail(LPVOID pVoid);
	LPVOID   RemoveHead(void);
	LPVOID   RemoveTail(void);
	void     RemoveAll(void);
	int      GetCount() ;
	BOOL     IsEmpty()  ;
	POSITION Find (void * pData, POSITION posStart = NULL);
	void     RemoveAt (POSITION pos);
	POSITION GetHeadPosition() ;
	LPVOID   GetNext(POSITION& rPosition) ;

protected:
	CPtrList m_list;
};

/// Wrapper for CObList
class TObBucket : public TSyncBucket
{
public:
	~TObBucket(void);

	// mimic the CPtrList
	POSITION AddHead(CObject* pObj);
	POSITION AddTail(CObject* pObj);
	CObject* RemoveHead(void);
	CObject* RemoveTail(void);
	int      GetCount() ;
	BOOL     IsEmpty()  ;
	POSITION GetHeadPosition() ;
	POSITION GetTailPosition() ;
	CObject*  GetNext(POSITION& rPosition) ;
	CObject*  GetPrev(POSITION& rPosition) ;

	// support for deleting items and moving items around
	CObject *RemoveAt (POSITION &rPosition);
	void MoveUp (POSITION &rPosition);
	void MoveDown (POSITION &rPosition);
	void SetAt (POSITION iPos, CObject *pObj);

protected:
	CObList m_list;
};

////////////////////////////////////////////////////////////////////////
//
//  TSyncTypedPtrMap
//
// Inherit form CTypedPtrMap - which is a template class.
//    Added value - it protects itself with a critical section
//    theoretically the CMap takes an nBlockSize as a Ctor parameter, but
//    I can't figure out the syntax
////////////////////////////////////////////////////////////////////////

template<class BASE_CLASS, class KEY, class VALUE>
class TSyncTypedPtrMap : public CTypedPtrMap<BASE_CLASS, KEY, VALUE>
{
public:
	// Constructor
	TSyncTypedPtrMap();

	// Destructor
	~TSyncTypedPtrMap();

	// Lock and Unlock
	void Lock()
	{
		EnterCriticalSection ( &m_critSect );
	}

	void Unlock()
	{
		LeaveCriticalSection ( &m_critSect );
	}

	// Attributes
	int GetCount() const
	{
		TEnterCSection mgr(&m_critSect);
		return CTypedPtrMap::GetCount();
	}

	BOOL IsEmpty() const
	{
		TEnterCSection mgr(&m_critSect);
		return BASE_CLASS::IsEmpty();
	}
	BOOL Lookup(KEY key, VALUE& rValue);

	// Operations

	// Lookup and add if not there
	VALUE& operator[](typename BASE_CLASS::BASE_ARG_KEY key)
	{
		TEnterCSection mgr(&m_critSect);
		return (VALUE&)BASE_CLASS::operator[](key);
	}

	// add a new (key, value) pair
	void SetAt(KEY key, VALUE newValue)
	{
		TEnterCSection mgr(&m_critSect);
		BASE_CLASS::SetAt(key, newValue);
	}

	// removing existing (key, ?) pair
	BOOL RemoveKey(KEY key)
	{
		TEnterCSection mgr(&m_critSect);
		return BASE_CLASS::RemoveKey (key);
	}

	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition();
	void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue);

protected:
	CRITICAL_SECTION m_critSect;
};

template<class BASE_CLASS, class KEY, class VALUE>
TSyncTypedPtrMap<BASE_CLASS, KEY, VALUE>::TSyncTypedPtrMap(void)
{
	InitializeCriticalSection ( &m_critSect );
}

template<class BASE_CLASS, class KEY, class VALUE>
TSyncTypedPtrMap<BASE_CLASS, KEY, VALUE>::~TSyncTypedPtrMap()
{
	DeleteCriticalSection ( &m_critSect );
}

///// --- here we go ---

// Lookup
template<class BASE_CLASS, class KEY, class VALUE>
BOOL TSyncTypedPtrMap<BASE_CLASS, KEY, VALUE>::Lookup(
	KEY key,
	VALUE& rValue)
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrMap<BASE_CLASS, KEY, VALUE>::Lookup (key, rValue);
}

// RemoveAll
template<class BASE_CLASS, class KEY, class VALUE>
void TSyncTypedPtrMap<BASE_CLASS, KEY, VALUE>::RemoveAll(void)
{
	TEnterCSection mgr(&m_critSect);
	CTypedPtrMap<BASE_CLASS, KEY, VALUE>::RemoveAll ();
}

// iteration start
template<class BASE_CLASS, class KEY, class VALUE>
POSITION TSyncTypedPtrMap<BASE_CLASS, KEY, VALUE>::GetStartPosition()
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrMap<BASE_CLASS, KEY, VALUE>::GetStartPosition();
}

// iteration
template<class BASE_CLASS, class KEY, class VALUE>
void TSyncTypedPtrMap<BASE_CLASS, KEY, VALUE>::GetNextAssoc(
	POSITION& rPosition, KEY& rKey, VALUE& rValue)
{
	TEnterCSection mgr(&m_critSect);
	CTypedPtrMap<BASE_CLASS, KEY, VALUE>::GetNextAssoc (rPosition, rKey, rValue);
}

////////////////////////////////////////////////////////////////////////
//
//  TSyncTypedPtrArray  (unfinished)
//
////////////////////////////////////////////////////////////////////////

template<class BASE_CLASS, class TYPE>
class TSyncTypedPtrArray : public CTypedPtrArray<BASE_CLASS, TYPE>
{
public:
	TSyncTypedPtrArray();
	~TSyncTypedPtrArray();

	// attributes
	int   GetSize() ;
	BOOL  IsEmpty() ;

	// operations
	void  Lock();
	void  Unlock();

	TYPE operator[](int idx) ;

	int  Add(TYPE ptr);
	void InsertAt(int idx, TYPE ptr);
	TYPE GetAt(int idx) ;
	void RemoveAll(void);
	void RemoveAt(int i);
	void SetAt(int i, TYPE ptr);

protected:
	CRITICAL_SECTION  m_critSect;
};

// ctor
template<class BASE_CLASS, class TYPE>
TSyncTypedPtrArray<BASE_CLASS, TYPE>::TSyncTypedPtrArray(void)
{
	InitializeCriticalSection ( &m_critSect );
}

// dtor
template<class BASE_CLASS, class TYPE>
TSyncTypedPtrArray<BASE_CLASS, TYPE>::~TSyncTypedPtrArray(void)
{
	DeleteCriticalSection ( &m_critSect );
}

// GetCount
template<class BASE_CLASS, class TYPE>
int TSyncTypedPtrArray<BASE_CLASS, TYPE>::GetSize()
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrArray<BASE_CLASS, TYPE>::GetSize();
}

// IsEmpty
template<class BASE_CLASS, class TYPE>
BOOL TSyncTypedPtrArray<BASE_CLASS, TYPE>::IsEmpty()
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrArray<BASE_CLASS, TYPE>::IsEmpty();
}

// Lock
template<class BASE_CLASS, class TYPE>
void TSyncTypedPtrArray<BASE_CLASS, TYPE>::Lock()
{
	EnterCriticalSection ( &m_critSect );
}

// Unlock
template<class BASE_CLASS, class TYPE>
void TSyncTypedPtrArray<BASE_CLASS, TYPE>::Unlock()
{
	LeaveCriticalSection ( &m_critSect );
}

// operator[]
template<class BASE_CLASS, class TYPE>
TYPE TSyncTypedPtrArray<BASE_CLASS, TYPE>::operator[](int idx)
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrArray<BASE_CLASS, TYPE>::operator[](idx);
}

// Add
template<class BASE_CLASS, class TYPE>
int TSyncTypedPtrArray<BASE_CLASS, TYPE>::Add(TYPE ptr)
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrArray<BASE_CLASS, TYPE>::Add(ptr);
}

// InsertAt
template<class BASE_CLASS, class TYPE>
void TSyncTypedPtrArray<BASE_CLASS, TYPE>::InsertAt(int idx, TYPE ptr)
{
	TEnterCSection mgr(&m_critSect);
	CTypedPtrArray<BASE_CLASS, TYPE>::InsertAt(idx, ptr);
}

// GetAt
template<class BASE_CLASS, class TYPE>
TYPE TSyncTypedPtrArray<BASE_CLASS, TYPE>::GetAt(int idx)
{
	TEnterCSection mgr(&m_critSect);
	return CTypedPtrArray<BASE_CLASS, TYPE>::GetAt(idx);
}

// RemoveAll
template<class BASE_CLASS, class TYPE>
void TSyncTypedPtrArray<BASE_CLASS, TYPE>::RemoveAll(void)
{
	TEnterCSection mgr(&m_critSect);
	CTypedPtrArray<BASE_CLASS, TYPE>::RemoveAll();
}

// RemoveAt
template<class BASE_CLASS, class TYPE>
void TSyncTypedPtrArray<BASE_CLASS, TYPE>::RemoveAt(int i)
{
	TEnterCSection mgr(&m_critSect);
	CTypedPtrArray<BASE_CLASS, TYPE>::RemoveAt(i);
}

// SetAt
template<class BASE_CLASS, class TYPE>
void TSyncTypedPtrArray<BASE_CLASS, TYPE>::SetAt(int  i, TYPE ptr)
{
	TEnterCSection mgr(&m_critSect);
	CTypedPtrArray<BASE_CLASS, TYPE>::SetAtAt(i, ptr);
}
