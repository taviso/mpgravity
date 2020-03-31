/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thrdlvwx.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:05  richard_wood
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

// thrdlvwx.cpp  -- moved some navigation functions into its own file
//
//
//
#include "stdafx.h"
#include "thrdlvw.h"
#include "server.h"
#include "thredlst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
// Returns 0 on success, -1 to cancel, 1 to open another group
// provide services Next-Unread & Next-Local
//
int  TThreadListView::jump_engine (
   int   (*pfnTest)(TArticleHeader * pHdr, void * pData),
   bool  fViewArticle,
   bool  fLimitToCurrentPile,
   bool  fTestCurrent,
   int * pOutArtNum /* =NULL */ )
{
   int idx = -1;
   if (m_hlbx.GetCount() == 0)
      return 1;  // open another group

   int nSelCount = m_hlbx.GetSelCount();
   if (1 != nSelCount)
      {
      if ((idx = m_hlbx.GetCaretIndex()) == -1)
         return -1;                      // cancel
      }

   TServerCountedPtr cpNewsServer;

   if (-1 == idx)
      m_hlbx.GetSelItems(1, &idx);

   TArtNode * pNode = m_hlbx.GetpNode ( idx );

   int gid = GetNewsView()->GetCurNewsGroupID();
   BOOL fUseLock;
   TNewsGroup *pNG = 0;
   TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
   if (!fUseLock)
      return -1;

   TThreadPile * pPile = 0;
   TThread     * pThread = 0;
   TArtNode    * pNodeOut = 0;

   ThreadFindNext sTFN;
   sTFN.pfnTest   = pfnTest;
   sTFN.pData     = pNG;

   int iDataRet;

   // if we have just opened a new group, the selection is probably sitting
   //   on index-0, and he might match...
   //
   // this is different from the normal mode of figure-out-cursel and then
   //   go to the next artnode.
   //
   if (fTestCurrent && 0 == pfnTest (pNode->GetpRandomHeader(), pNG))
      {
      iDataRet = 0;
	  pNodeOut = pNode;
      }

   else if (m_header.IsThreaded())
      {
      iDataRet = pNG->GetpThreadList()->FindTreeNext ( pNode, &sTFN,
                                                       fLimitToCurrentPile,
                                                       pPile, pThread, pNodeOut );
      }
   else
      iDataRet = pNG->GetpThreadList()->FindFlatNext ( BYTE(m_header.IsSortAscend()),
         pNode, &sTFN, pNodeOut);

   if (0 == iDataRet)
      {
#if defined(_DEBUG)
      idx = pNodeOut->GetpRandomHeader()->GetNumber();
#endif
      if (pOutArtNum)
         {
         // we are only peeking !
         *pOutArtNum = pNodeOut->GetpRandomHeader()->GetNumber();
         }
      else
         {

         // BUG: should expand any collapsed branch?
         SelectAndShowArticle ( pNodeOut->GetpRandomHeader()->GetNumber(),
                                fViewArticle );
         }

      return 0;
      }
   return 1;
} // jump_engine

// ------------------------------------------------------------------------
int fnTextIsNext (TArticleHeader * pHdr, void * pData)
{
   // mere existence is good enough
   return 0;
}

// ------------------------------------------------------------------------
int fnTestIsNewArticle (TArticleHeader * pHdr, void * pData)
{
   TNewsGroup * pNG = (TNewsGroup*) pData;
   return pNG->IsStatusBitOn(pHdr->GetNumber(), TStatusUnit::kNew) ? 0 : 1;
}

// ------------------------------------------------------------------------
int fnTestIsLocalArticle (TArticleHeader * pHdr, void * pData)
{
   TNewsGroup * pNG = (TNewsGroup*) pData;

   int n = pHdr->GetNumber ();
   return pNG->TextRangeHave (n) ? 0 : 1;
}

// ------------------------------------------------------------------------
int fnTestIsUnreadLocalArticle (TArticleHeader * pHdr, void * pData)
{
   return  (0 == fnTestIsNewArticle ( pHdr, pData ) &&
            0 == fnTestIsLocalArticle ( pHdr, pData ) ) ? 0 : 1;
}

// ------------------------------------------------------------------------
// Return 0 for match
int fnTestIsNew_WatchFree (TArticleHeader * pHdr, void * pData)
{
   TNewsGroup * pNG = (TNewsGroup*) pData;

   WORD wStatus;
   BOOL fHaveBody;

   int n = pHdr->GetNumber ();
   if (pNG->ArticleStatus (n, wStatus, fHaveBody))
      return 1;

   //  look for new and !watched
   //
   //    New(1)             Watch(0)        ---  wStatus
   //    New(0)             Watch(1)        ---  XorValue
   //    -------- xor      ---------
   //        1                    1         =    wAndMask
   //

   WORD wAndMask = TStatusUnit::kNew | TStatusUnit::kWatch;

   if (wAndMask == (wAndMask & (wStatus ^ (TStatusUnit::kWatch))))
      return 0;
   return 1;
}

// ------------------------------------------------------------------------
// Return 0 for match
int fnTestIsNew_IgnoreFree (TArticleHeader * pHdr, void * pData)
{
   TNewsGroup * pNG = (TNewsGroup*) pData;

   WORD wStatus;
   BOOL fHaveBody;

   int n = pHdr->GetNumber ();
   if (pNG->ArticleStatus (n, wStatus, fHaveBody))
      return 1;

   //  look for new and !ignored
   //
   //    New(1)             Ignor(0)        ---  wStatus
   //    New(0)             Ignor(1)        ---  XorValue
   //    -------- xor      ---------
   //        1                    1         =    wAndMask
   //

   WORD wAndMask = TStatusUnit::kNew | TStatusUnit::kIgnore;

   if (wAndMask == (wAndMask & (wStatus ^ (TStatusUnit::kIgnore))))
      return 0;
   return 1;
}

// ------------------------------------------------------------------------
// We are looking for an article that is  UnTagged, No-Body and Unread
//
// (after all, if the body is here, why do we need to tag it?)
//
int fnTestIsNew_TagFree (TArticleHeader * pHdr, void * pData)
{
   TNewsGroup * pNG = (TNewsGroup*) pData;

   WORD wStatus;
   BOOL fHaveBody;

   int n = pHdr->GetNumber ();
   if (FALSE == pNG->ArticleStatus (n, wStatus, fHaveBody))
      return 1;

   if (0 == (wStatus & TStatusUnit::kNew) || fHaveBody)
      return 1;

   TServerCountedPtr cpServer( pNG->GetParentServer() );
   if (cpServer->GetPersistentTags().FindTag (pNG->m_GroupID, n))
      return 1;
   return 0;
}

/* ------------------------------------------------------------------------
   Returns 0 for success
   enum EJmpQuery  {  kQueryNextUnread,
                      kQueryNextLocal,
                      kQueryNextUnreadLocal,
                      kQueryNextUnreadInThread };
*/
int  TThreadListView::JumpToEngine (
   EJmpQuery   eQuery,
   bool        fViewArticle,
   bool        fLimitToPile )
{
   int  ret = 0;
   bool fChangeGroup = true;
   bool fTestCursel = false;

   if (m_fTestCurrentNavItem)
      {
      m_fTestCurrentNavItem = false;   // set via FilterOutReadThreads ()

      fTestCursel = true;
      }

   switch (eQuery)
      {
      case kQueryDoNotMove:
         break;

      case kQueryNext:
         ret = jump_engine (fnTextIsNext,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      case kQueryNextUnread:
         ret = jump_engine (fnTestIsNewArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      case kQueryNextUnreadLocal:
         ret = jump_engine (fnTestIsUnreadLocalArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      case kQueryNextLocal:
         ret = jump_engine (fnTestIsLocalArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      case kQueryNextUnreadInThread:
         ret = jump_engine (fnTestIsNewArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);

         // dont' need to roll over to next group in this case
         fChangeGroup = false;
         break;

      case kQueryNextUnreadWatchFree:
         ret = jump_engine (fnTestIsNew_WatchFree,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      case kQueryNextUnreadIgnoreFree:
         ret = jump_engine (fnTestIsNew_IgnoreFree,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      case kQueryNextUnreadTagFree:
         ret = jump_engine (fnTestIsNew_TagFree,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel);
         break;

      }

   // 0 means we found an article that satisfies
   if (-1 == ret || 0 == ret)
      return ret;

   if (!fChangeGroup)
      return -1;

   // article not found in this group, must move to next group

   int gid = 0;
   CNewsView * pNV = GetNewsView();

   // peek ahead at the status units
   ret = pNV->GetNextGroup (eQuery, &gid);
   if (ret)
      {
      return -1;
      }

   return JumpSecondPass (pNV, eQuery, gid, fViewArticle, fLimitToPile);
}

// --------------------------------------------------------------------------
int TThreadListView::JumpSecondPass (CNewsView * pNV,
                                     EJmpQuery  eQuery,
                                     int   gid,
                                     bool fViewArticle,
                                     bool fLimitToPile)
{
   int ret = 0;

   // boy this is hardcore!
   pNV->OpenNewsgroup (CNewsView::kOpenNormal,
                       CNewsView::kPreferredFilter,
                       gid);

   // keep selection in the ng pane synced-up
   pNV->AddOneClickGroupInterference ();
   pNV->SetSelectedGroupID (gid);

   // locate article within the listbox
   switch (eQuery)
      {
      case kQueryDoNotMove:
         break;

      case kQueryNext:
         ret = jump_engine (fnTextIsNext,
                            fViewArticle,
                            fLimitToPile,
                            true); // test cursel?
         break;

      case kQueryNextUnread:
         ret = jump_engine (fnTestIsNewArticle,
                            fViewArticle,
                            fLimitToPile,
                            true);  // test cursel?
         break;

      case kQueryNextUnreadLocal:
         ret = jump_engine (fnTestIsUnreadLocalArticle,
                            fViewArticle,
                            fLimitToPile,
                            true);
         break;

      case kQueryNextLocal:
         ret = jump_engine (fnTestIsLocalArticle,
                            fViewArticle,
                            fLimitToPile,
                            true);
         break;

      case kQueryNextUnreadWatchFree:
         ret = jump_engine (fnTestIsNew_WatchFree,
                            fViewArticle,
                            fLimitToPile,
                            true);
         break;

      case kQueryNextUnreadIgnoreFree:
         ret = jump_engine (fnTestIsNew_IgnoreFree,
                            fViewArticle,
                            fLimitToPile,
                            true);
         break;

      case kQueryNextUnreadTagFree:
         ret = jump_engine (fnTestIsNew_TagFree,
                            fViewArticle,
                            fLimitToPile,
                            true);
         break;
      }

   return ret;
}

// Info required -- 'Next' Article in current group or in next group
//
// returns  -1  for error
//           0  'next' article is in this group :  iOutArtNum valid
//           1  'next' article is in next group :  iOutGID valid

int  TThreadListView::QueryToEngine (
   EJmpQuery   eQuery,
   bool        fLimitToPile,
   int &       iOutGID,
   int &       iOutArtNum)

{
   int  ret = 0;
   bool fChangeGroup = true;
   bool fTestCursel = false;
   bool fViewArticle = false;

   if (m_fTestCurrentNavItem)
      {
      m_fTestCurrentNavItem = false;   // set via FilterOutReadThreads ()

      fTestCursel = true;
      }

   switch (eQuery)
      {
      case kQueryDoNotMove:
         break;

      case kQueryNext:
         ret = jump_engine (fnTextIsNext,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      case kQueryNextUnread:
         ret = jump_engine (fnTestIsNewArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      case kQueryNextUnreadLocal:
         ret = jump_engine (fnTestIsUnreadLocalArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      case kQueryNextLocal:
         ret = jump_engine (fnTestIsLocalArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      case kQueryNextUnreadInThread:
         ret = jump_engine (fnTestIsNewArticle,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);

         // dont' need to roll over to next group in this case
         fChangeGroup = false;
         break;

      case kQueryNextUnreadWatchFree:
         ret = jump_engine (fnTestIsNew_WatchFree,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      case kQueryNextUnreadIgnoreFree:
         ret = jump_engine (fnTestIsNew_IgnoreFree,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      case kQueryNextUnreadTagFree:
         ret = jump_engine (fnTestIsNew_TagFree,
                            fViewArticle,
                            fLimitToPile,
                            fTestCursel,
                            &iOutArtNum);
         break;

      }

   // 0 means we found an article that satisfies
   if (-1 == ret || 0 == ret)
      return ret;

   if (!fChangeGroup)
      return -1;

   // article not found in this group, must move to next group

   int gid = 0;
   CNewsView * pNV = GetNewsView();

   // peek ahead at the status units
   ret = pNV->GetNextGroup (eQuery, &gid);
   if (ret)
      {
      return -1;  // no more groups
      }

   iOutGID = gid;

   // 1 means the next group is the target
   return 1;
}
