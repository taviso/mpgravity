/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: servrang.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

#pragma once

#include "mplib.h"
#include "sharesem.h"
#include "pobject.h"

class TCrcKey : public PObject
{
   public:
      DECLARE_SERIAL (TCrcKey)
      void Serialize (CArchive & ar);
      BOOL operator==(const TCrcKey  & key) const;

   public:
      DWORD m_whole;
      DWORD m_half;
};

template <> UINT AFXAPI HashKey(TCrcKey & crcKey);

typedef CMap <TCrcKey,
              TCrcKey &,
              TRangeSet *,
              TRangeSet * > TServerRangeMap;

class TServerRangeSet : public PObject, public TSynchronizable
{
public:
   DECLARE_SERIAL (TServerRangeSet)
   TServerRangeSet (void);
   ~TServerRangeSet (void);
   TRangeSet *GetRangeSet(LPCTSTR group);
   void Serialize (CArchive & ar);
   void RemoveRangeSet (LPCTSTR group);

   void AddSubscribedName (LPCTSTR group);
   void RemoveSubscribedName (LPCTSTR group);

   // removes info for non-subscribed groups
   int  EraseGroupInfo (void);

private:
   void make_key (LPCTSTR group, TCrcKey* pKey);
   BOOL IsSubscribedName (TCrcKey& key);

   TServerRangeMap   m_rangeMap;

   TServerRangeMap   m_subscribed;  // not serialized
};
