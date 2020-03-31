/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thredpl.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:06  richard_wood
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

#include "intvec.h"
#include "thredfnx.h"

class TBaseArticleAction;
class TArticleHeader;
class TThreadList;
class TArticleBank;
class TThread;
class THierDrawLbx;
class TArtNode;

extern UINT fnShortCRC (LPCTSTR lszSubj, int strLen);

class TPileIDS : public CObject
{
public:
   TIntVector m_ArtIdsToFind;
   TIntVector m_ArtIdsFound;

   void UniqueAdd (int n);
};

///////////////////////////////////////////////////////////////////////////
// a loose collection of threads that share the same Subject
//
class TThreadPile : public CObject
{
public:
   TThreadPile (TThreadList* pList, TThread* pThread);
   ~TThreadPile();

   void RecursiveFree ();

   int FillTree (THierDrawLbx* pTree, BOOL fCollapsed);
   BOOL Have (TArticleHeader* pArtHdr, TThread*& rpThread,
              TArtNode*& rpNode);

   BOOL HaveAny(TIntVector * pIdsToFind);
   int  CollectIds(TIntVector * pIdsFound);

   int  RemoveThread(const TThread* pThread);
   int  FindPosition(const TThread* pThread);
   int  InsertThreadByTime(TThread* pThread);
   BOOL IsFirst(const TThread* pThread);

   int WalkHeaders (TBaseArticleAction* pAction);

   int FindTreeNext ( TArtNode * pNode,
                      ThreadFindNext * psFindNext,
                      TThread*& rpThreadOut,
                      TArtNode*& rpArtNodeOut );

   // added 2-23-98
   int CompactThreadsToOneThread ();

   void myAssertValid ();

   int  Excoriate ();
   int  CollectNodes ( PVOID pvVec );

public:
   int     GetSize()       { return m_array.GetSize(); }
   void *  GetElem(int k)  { return m_array[k];        }
public:

   bool    ZeroPresent () const
           { return m_bZeroPresent; }

   void    ZeroPresent (BOOL fZero)
            {
            m_bZeroPresent = fZero ? true : false;
            }

   void ReCalcPileMetrics ();

   bool DirtyDuringDelete ()       { return m_fDirtyDuringDelete; }
   void DirtyDuringDelete (bool d) { m_fDirtyDuringDelete = d; }

public:
   CTime     m_oldTime;

   long      getScore () const            { return m_lScore; }
   void      setScore (long lScore)
      {
      m_lScore = lScore;
      }
   long      getLowScore () const         { return m_lLowScore; }
   void      setLowScore (long low)
      {
      m_lLowScore = low;
      }

protected:
   long      m_lScore;
   long      m_lLowScore;

private:
   bool      m_bZeroPresent;
   bool      m_fDirtyDuringDelete;

protected:
   void      CopyMinMaxMetrics ( TThread *pThread );

   CPtrArray    m_array;
   TThreadList* m_pThreadList;
};
