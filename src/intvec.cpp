/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: intvec.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:27  richard_wood
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
#include <stdlib.h>
#include <search.h>
#include "intvec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////
//
//
TIntVector::TIntVector()
{
   m_iCur = 0;
   m_iMax = 256;
   m_pData = (int*) malloc(m_iMax * sizeof(int));
   if (0 == m_pData)
      AfxThrowMemoryException();
   m_fSorted = TRUE;
}

///////////////////////////////////////////////////////////////////////////
//
//
TIntVector::~TIntVector()
{
   if (m_pData)
      free(m_pData);
}

int TIntVector::GetCount()
{
   return m_iCur;
}

///////////////////////////////////////////////////////////////////////////
//
//
void TIntVector::Add(int n)
{
   if (m_iCur == m_iMax)
      {
      // alloc more space
      int iMore = m_iCur + 256;
      void * ptr = realloc(m_pData, iMore * sizeof(int));
      if (0==ptr)
         AfxThrowMemoryException();
      m_pData = (int*) ptr;
      m_iMax = iMore;
      }

   m_pData[m_iCur] = n;
   ++m_iCur;

   // once we've added something we must resort
   m_fSorted = FALSE;
}

int TIntVector::GetAt(int i)
{
   ASSERT(i >= 0);
   ASSERT(i < m_iCur);
   return m_pData[i];
}

int _cdecl TIntVector::qsort_cmp_int(const void * arg1, const void * arg2)
{
   if ((*(int*)arg1) == (*(int*)arg2))
      return 0;
   if ((*(int*)arg1) < (*(int*)arg2))
      return -1;
   return 1;
}

BOOL TIntVector::Have(int n)
{
   if (0==m_iCur)
      return FALSE;
   if (!m_fSorted)
      {
      qsort ( m_pData, m_iCur, sizeof(int), TIntVector::qsort_cmp_int );
      m_fSorted = TRUE;
      }
   void * pRet = bsearch(&n, m_pData, m_iCur, sizeof(int),
                         TIntVector::qsort_cmp_int);
   if (pRet)
      return TRUE;
   else
      return FALSE;
}
