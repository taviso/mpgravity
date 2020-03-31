/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: sortlst.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:52  richard_wood
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

//////////////////////////////////////////////////////////////////////////
//
// Typed ptr array that can sort the elements
//   usage: you add items. then call SortItems.
//

#pragma once

typedef BOOL (*SortListLESSEQ) (const CObject* pObj1, const CObject* pObj2);
typedef int (*P_COMPAREFUNC)(const CObject* pObj1, const CObject* pObj2);

// returns -1, 0 or 1
typedef int (*P_BINSEARCHFUNC)(void * pKey, const CObject* pObj1);

// Inherit from CTypedPtrArray - which is a template class
template<class BASE_CLASS, class TYPE>
class TSortedPtrArray : public CTypedPtrArray<BASE_CLASS, TYPE>
{
public:
    BOOL SortItems(void);
    BOOL SortItems(SortListLESSEQ fnSort);
    BOOL HeapSortItems(P_COMPAREFUNC fpCompare);
    BOOL BinSearch(P_BINSEARCHFUNC fpCompare, void * pKey, int* pIndex);
};

///////////////////////////////////////////////////////////////////////////
// version 1
template <class BASE_CLASS, class TYPE>
BOOL TSortedPtrArray<BASE_CLASS, TYPE>::SortItems(void)
{
   int gap, i, j;

   int n = GetSize();

   for (gap = n/2; gap > 0; gap /= 2)
      for (i = gap; i < n; i++)
         for (j = i-gap; j>=0; j-= gap) {
            TYPE ptrOne = GetAt(j);
            TYPE ptrTwo = GetAt(j+gap);
            if (*ptrOne <= *ptrTwo)
               break;

            ElementAt(j)     = GetAt(j+gap);
            ElementAt(j+gap) = ptrOne;
         }

   return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// version Two
template <class BASE_CLASS, class TYPE>
BOOL TSortedPtrArray<BASE_CLASS, TYPE>::SortItems(SortListLESSEQ fnLESSEQ)
{
   int gap, i, j;

   int n = GetSize();

   for (gap = n/2; gap > 0; gap /= 2)
      for (i = gap; i < n; i++)
         for (j = i-gap; j>=0; j-= gap) {
            TYPE ptrOne = GetAt(j);
            TYPE ptrTwo = GetAt(j+gap);
            if (fnLESSEQ(ptrOne,ptrTwo))
               break;

            ElementAt(j)     = GetAt(j+gap);
            ElementAt(j+gap) = ptrOne;
         }

   return TRUE;
}

// prototype.  function is in thredlst.cpp
int HeapSort (int n, CObArray* pArray, P_COMPAREFUNC pfnCompare);

///////////////////////////////////////////////////////////////////////////
// version Three
template <class BASE_CLASS, class TYPE>
BOOL TSortedPtrArray<BASE_CLASS, TYPE>::HeapSortItems(
P_COMPAREFUNC pfnCompare)
{
   int total = GetSize();
   return HeapSort ( total, this, pfnCompare );
}

// Inherit from CTypedPtrList - which is a template class
template<class BASE_CLASS, class TYPE>
class TSortedPtrList : public CTypedPtrList<BASE_CLASS, TYPE>
{
public:
    BOOL SortItems(SortListLESSEQ fnSort);
};

//-------------------------------------------------------------------------
// BinSearch
template <class BASE_CLASS, class TYPE>
BOOL TSortedPtrArray<BASE_CLASS, TYPE>::BinSearch(
   P_BINSEARCHFUNC fpCompare,
   void * pKey,
   int* pIndex)
{
   *pIndex = 0;
   int lo = 0;
   int hi = GetSize() - 1;
   int val, mid;

   if (hi < 0)
      return FALSE;

   do
      {
      mid = (lo + hi) / 2;
      val = fpCompare ( pKey, ElementAt(mid) );
      if (0 == val)
         {
         *pIndex = mid;
         return TRUE;
         }

      else if (val < 0)
         hi = mid - 1;
      else
         lo = mid + 1;
      } while (lo <= hi);

   // return index of where it should go
   if (val < 0)
      *pIndex = mid;
   else
      *pIndex = mid + 1;
   return FALSE;
}

///////////////////////////////////////////////////////////////////////////
// Sort Function for TPtrList
//
template <class BASE_CLASS, class TYPE>
BOOL TSortedPtrList<BASE_CLASS, TYPE>::SortItems(SortListLESSEQ fnLESSEQ)
{
#if defined(_DEBUG)
   int n = GetCount();
   ASSERT(n > 0);
#endif

   // I looked at CObList::FindIndex - looks inefficient. Can't do ShellSort
   //   So I'm going to transfer everything to a temp list.
   //   and insert sort back into the original
   CTypedPtrList<BASE_CLASS, TYPE> tmpList;

   tmpList.AddHead ( this );
   RemoveAll ();

   TYPE ptr;
   while (!tmpList.IsEmpty())
      {
      ptr = tmpList.RemoveHead();

      // find correct spot
      POSITION pos = GetTailPosition();

      if (NULL == pos)
         {
         AddTail (ptr);
         continue;
         }
      else
         {
         POSITION oldPos;
         BOOL fAdded = FALSE;

         while (pos)
            {
            oldPos = pos;
            TYPE& origPtr = GetPrev(pos);
            if ( fnLESSEQ (origPtr, ptr) )  // orig is <= ptr
               {
               InsertAfter (oldPos, ptr);
               fAdded = TRUE;
               break;
               }
            // pos is already   --pos
            }
         if (!fAdded)
            AddHead ( ptr );
         }
      }
   return TRUE;
}
