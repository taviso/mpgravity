/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: expire.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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

// Used to transfer information between threads
//

#pragma once

#include "intvec.h"

class  THeapBitmap;

struct TExpiryDataInt
{
public:
   TIntVector m_ids;

   // User chose to skipped the Expire work for speedup.
   bool       m_fAssumeAllIdsArePresent;

   TExpiryDataInt()
      {
      m_fAssumeAllIdsArePresent = false;
      }

   // assume all are present
   void  SetAllPresent (bool fOn)
      {
      m_fAssumeAllIdsArePresent = fOn;
      }

   bool GetAllPresent (void)
      {
      return m_fAssumeAllIdsArePresent;
      }

   bool Have (int n)
      {
      if (GetAllPresent())
         return true;
      return m_ids.Have (n) ? true : false;
      }
   void Add (int n)
      {
      m_ids.Add (n);
      }
};

class TExpiryData : public CObject
{
public:
   enum EMissing { kMissing, kExpired };

public:
   TExpiryData(bool fExpirePhase, int iGroupID, int iExpireLow);

   // if bit is not set, article is missing
   // BOOL fMissing -  TRUE  means article is missing from server
   //                  FALSE means article is expired from server
   TExpiryData(EMissing eMissing, bool fExpirePhase,
               int iGroupID, int iExpireLow);

   // destructor
   ~TExpiryData ();

   // assume all are present
   void  SetAllPresent (bool fOn)
      {
      m_sHelper.SetAllPresent (fOn);
      }

   bool GetAllPresent (void)
      {
      return m_sHelper.GetAllPresent();
      }

   BOOL        m_fUseVec;
   BOOL        m_fMissing;
   int         m_iGroupID;
   int         m_iExpireLow;

   CTypedPtrArray<CPtrArray, THeapBitmap*> m_ArraypBits;

   bool        Have (int n);
   void        Add  (int n) { m_sHelper.Add (n); }

   bool        m_fRedrawUI;

protected:
   TExpiryDataInt  m_sHelper;
};
