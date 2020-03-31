/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artclean.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:10  richard_wood
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

// artclean.cpp : implementation file
//
//  4-29-96 amc  This is a shadow of its former self
//

#include "stdafx.h"
#include "mplib.h"
#include "artclean.h"
#include "article.h"
#include "newsgrp.h"
#include "utilpump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TArticleCleaner

TArticleCleaner::TArticleCleaner()
{
}

TArticleCleaner::~TArticleCleaner()
{
}

//////////////////////////////////////////////////////////////
// a utility function that destroys articles.
//
//
// caller must call SaveSubscribedGroups
// call must clean up the UI - no more ODB callback
void destroy_one(BOOL fMarkRead, BOOL fKillCrossPosts, TNewsGroup* pNG,
                 int artInt, TArticleHeader* pHdr)
{
   // depend on ReadRangeAdd to do it's own locking/unlocking around
   //  ProcessXRefs

   if (fMarkRead)
      {
      if (fKillCrossPosts)
         {
         ASSERT(pHdr);
         // this does the magic cross-post management.
         pNG->ReadRangeAdd (pHdr, KThreadList::kHandleCrossPost);
         }
      else
         {
         pNG->ReadRangeAdd (artInt, KThreadList::kIgnoreCrossPost);
         }
      }

   // Now pin the newsgroup with a Write lock
   TSyncWriteLock writeLock(pNG);

   pNG->StatusDestroy( artInt );

   pNG->HTRangeSubtract ( artInt );

   pNG->PurgeHeader ( artInt );
   pNG->PurgeBody ( artInt );
   pNG->RemoveHeaderRuleInfo ( artInt );
}

void TArticleCleaner::DestroyOne(TNewsGroup* pNG, TArticleHeader* pHdr)
{
   destroy_one (TRUE, TRUE /*cpm*/ , pNG, pHdr->GetNumber(), pHdr);
}

///////////////////////////////////////////////////////////////////////////
// global function - called by Clean()
void  TArticleCleaner::ConvertToArray(TRangeSet* pSet, CDWordArray* pArray)
{
   int count = pSet->RangeCount();
   int i, k;
   for (i = 0; i < count; ++i)
      {
      int lo, hi;
      pSet->GetRange ( i, lo, hi );
      for (k = lo; k <= hi; ++k)
         pArray->Add ( k );
      }
}

// Global func
void TArticleCleaner::Clean(TNewsGroup* pNG, TRangeSet* pCleanRange,
   BOOL fMarkRead)
{
   TRACE0("start Clean()\n");
   CDWordArray nums;
   ConvertToArray ( pCleanRange, &nums );

   if (nums.GetSize() > 0)
      {
      TAutoClose sAutoCloser(pNG);

      // destroy without CrossPost mgmt.
      for (int i = 0; i < nums.GetSize(); i++)
         {
         destroy_one (fMarkRead, FALSE,
                      pNG, (int)nums[i], NULL);

         PumpAUsefulMessageLITE ();
         }
      }
   TRACE0("end Clean()\n");
}

