/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: bucket.cpp,v $
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

#include "stdafx.h"
#include "bucket.h"
#include "critsect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
// TSyncBucket constructor
TSyncBucket::TSyncBucket ()
{
   InitializeCriticalSection ( &m_CriticalSection );
}

// ------------------------------------------------------------------------
// TSyncBucket Destructor
TSyncBucket::~TSyncBucket ()
{
   DeleteCriticalSection  ( &m_CriticalSection );
}

TPtrBucket::~TPtrBucket(void)
{
   // the elements should all be gone
   ASSERT(m_list.IsEmpty());
}

POSITION TPtrBucket::AddHead(LPVOID pVoid)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.AddHead ( pVoid );
}

POSITION TPtrBucket::AddTail(LPVOID pVoid)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.AddTail ( pVoid );
}

LPVOID   TPtrBucket::RemoveHead(void)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.RemoveHead ();
}

LPVOID   TPtrBucket::RemoveTail(void)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.RemoveTail();
}

void TPtrBucket::RemoveAll ()
{
   TEnterCSection mgr(&m_CriticalSection);
   m_list.RemoveAll ();
}

int TPtrBucket::GetCount()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetCount();
}

BOOL TPtrBucket::IsEmpty()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.IsEmpty();
}

POSITION TPtrBucket::GetHeadPosition()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetHeadPosition();
}

LPVOID   TPtrBucket::GetNext(POSITION& rPosition)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetNext( rPosition );
}

// ------------------------------------------------------------------------
POSITION TPtrBucket::Find (void * pData, POSITION posStart)
{
   TEnterCSection mgr(&m_CriticalSection);

   return m_list.Find (pData, posStart);
}

// ------------------------------------------------------------------------
void TPtrBucket::RemoveAt (POSITION pos)
{
   TEnterCSection mgr(&m_CriticalSection);
   m_list.RemoveAt (pos);
}

////////////////////////////////////////////////////////////////

TObBucket::~TObBucket(void)
{
   // the elements should all be gone
   ASSERT(m_list.IsEmpty());
}

POSITION TObBucket::AddHead(CObject* pObject)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.AddHead ( pObject );
}

POSITION TObBucket::AddTail(CObject* pObject)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.AddTail ( pObject );
}

CObject*   TObBucket::RemoveHead(void)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.RemoveHead ();
}

CObject*   TObBucket::RemoveTail(void)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.RemoveTail();
}

int TObBucket::GetCount()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetCount();
}

BOOL TObBucket::IsEmpty()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.IsEmpty();
}

POSITION TObBucket::GetHeadPosition()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetHeadPosition();
}

POSITION TObBucket::GetTailPosition()
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetTailPosition();
}

CObject*  TObBucket::GetNext(POSITION& rPosition)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetNext( rPosition );
}

CObject*  TObBucket::GetPrev(POSITION& rPosition)
{
   TEnterCSection mgr(&m_CriticalSection);
   return m_list.GetPrev( rPosition );
}

CObject *TObBucket::RemoveAt (POSITION &rPosition)
{
   TEnterCSection mgr(&m_CriticalSection);
   CObject *pResult = m_list.GetAt (rPosition);
   m_list.RemoveAt (rPosition);
   return pResult;
}

void TObBucket::MoveUp (POSITION &rPosition)
{
   TEnterCSection mgr(&m_CriticalSection);

   POSITION rItemPosition = rPosition;    // remember rPosition
   CObject *pItem = m_list.GetAt (rItemPosition);
   ASSERT (pItem);

   // GetNext() changes rPosition
   CObject *pNext = m_list.GetPrev (rPosition);
   if (!pNext)
      // the object at `rPosition' is already at the head of the list
      return;

   // move the object at `rItemPosition' ahead by one slot
   m_list.RemoveAt (rItemPosition);
   m_list.InsertBefore (rPosition, pItem);
}

void TObBucket::MoveDown (POSITION &rPosition)
{
   TEnterCSection mgr(&m_CriticalSection);

   POSITION rItemPosition = rPosition;    // remember rPosition
   CObject *pItem = m_list.GetAt (rItemPosition);
   ASSERT (pItem);

   // GetNext() changes rPosition
   CObject *pPrev = m_list.GetNext (rPosition);
   if (!pPrev)
      // the object at `rPosition' is already at the tail of the list
      return;

   // move the object at `rItemPosition' behind by one slot
   m_list.RemoveAt (rItemPosition);
   m_list.InsertAfter (rPosition, pItem);
}

void TObBucket::SetAt (POSITION iPos, CObject *pObj)
{
   TEnterCSection mgr(&m_CriticalSection);
   m_list.SetAt (iPos, pObj);
}
