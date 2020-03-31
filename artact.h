/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artact.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:09  richard_wood
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

#include "statvec.h"
#include "tsearch.h"
#include "dataquickfilter.h"

class TNewsGroup;
class TThreadList;
class TPersist822Header;
class TArticleHeader;
class TIntVector;

class TBaseArticleAction {
public:
   TBaseArticleAction() {}
   ~TBaseArticleAction() {}
   virtual int Run (TPersist822Header* pHdr, LPVOID pVoid) = 0;
};

class TArticleAction : public TBaseArticleAction {
public:
   TArticleAction(TNewsGroup* pNG, TStatusUnit::EStatus kStatus, BOOL fOn);
   ~TArticleAction();

   // Override a virtual
   int Run(TPersist822Header* pHdr, LPVOID pVoid);

protected:
   TNewsGroup*          m_pNG;
   TStatusUnit::EStatus m_kStatus;
   BOOL                 m_fOn;
};

class TArticleActionFetch : public TBaseArticleAction {
public:
   TArticleActionFetch(int artInt);

   virtual int Run(TPersist822Header* pHdr, LPVOID pVoid);

   int    m_artInt;   // search by artInt

   LPVOID          m_pVoid;    // return tree-node
   TArticleHeader* m_pHeader;  // return header
};

class TArticleActionHaveN : public TBaseArticleAction {
public:
   TArticleActionHaveN (TIntVector * pIdsToFind)
      : m_pIdsToFind(pIdsToFind), m_fFound(FALSE) {}

   // Override a virtual
   int Run(TPersist822Header* pHdr, LPVOID pVoid);

   BOOL m_fFound;
private:
   TIntVector * m_pIdsToFind;
};

class TArticleActionCollectIds : public TBaseArticleAction {
public:
   TArticleActionCollectIds (TIntVector * pIdsFound)
      : m_pIdsFound(pIdsFound) {}

   // Override a virtual
   int Run(TPersist822Header* pHdr, LPVOID pVoid);

private:
   TIntVector * m_pIdsFound;
};

class TArticleActionSetPileScore : public TBaseArticleAction {
public:
   void SetScore (long lScore) { m_lScore = lScore; }
   void SetZeroPresent (BOOL bZeroPresent) { m_bZeroPresent = bZeroPresent; }
   int Run (TPersist822Header* pHdr, LPVOID pVoid);

private:
   long m_lScore;
   BOOL m_bZeroPresent;
};

class TViewFilter;      // forward declaration
class TPersistentTags;

// used to see if any element of threadpile matches the filter

class TArticleActionElementMatch : public TBaseArticleAction {
public:
   TArticleActionElementMatch (TViewFilter *     pFilter,
                               TNewsGroup  *     pNG,
                               TPersistentTags * pTags,
                               TQuickFilterData * pQuickFilterData);

   // override a virtual
   int Run (TPersist822Header* pHdr, LPVOID pVoid);

   BOOL IsMatch () { return m_fElementMatch; }

private:
   TViewFilter * m_pFilter;

   TNewsGroup  *     m_pNG;
   TPersistentTags * m_pTags;

   BOOL          m_fElementMatch;
   BOOL          m_fQuickFilter;
   BOOL          m_fQuickFrom;
   BOOL          m_fQuickSubj;

   TSearch       m_search;
};
