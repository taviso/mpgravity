/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: statsrch.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

#pragma once

#include "statunit.h"

class TStatusUnit;

class TStatusSearch : public CObject
{
public:
   DECLARE_SERIAL(TStatusSearch);
   TStatusSearch();
   ~TStatusSearch();
   virtual void Serialize(CArchive& ar);

   const StatusUnitI* GetData(int idx=0) { return m_pUnits+idx; }

   int  AddItem(int artInt, WORD status, time_t readTime = 0, BOOL fForce = FALSE);
   void SetItemByIndex(int idx, WORD status, time_t readTime);
   void SetItemByKey(int artInt,WORD status, time_t readTime);

   void DestroyAll();
   int  GetSize () { return m_iCurElem; }
   void RemoveItem(int artInt, TStatusUnit* pUnit = 0);
   BOOL Find(int artInt, int& idx);

   LPCRITICAL_SECTION GetpCriticalSection() { return &m_critSect; }

   void Dump (CString & str);

   void ResetHighestArticleRead (int iNewHi);

protected:
   void alloc_array(int n);
   void grow_array();

protected:
   StatusUnitI* m_pUnits;
   HANDLE       m_hUnits;
   LONG         m_iCurElem;
   LONG         m_iMaxElem;
   CRITICAL_SECTION m_critSect;
};
