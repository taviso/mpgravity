/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: taglist.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:57  richard_wood
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
#include "taglist.h"
#include "critsect.h"
#include "tasker.h"                 // RetrieveArticle
#include "fileutil.h"               // NewsMessageBox
#include "newsgrp.h"                // TNewsGroup
#include "nglist.h"                 // TNewsGroupUseLock
#include "server.h"                 // TNewsServer
#include "servcp.h"                 // TServerCountedPtr
#include "tglobopt.h"
#include "rgswit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsTasker* gpTasker;

IMPLEMENT_SERIAL(TTagArt, CObject, 1);
IMPLEMENT_SERIAL(TTagElement, CObject, 1);
IMPLEMENT_SERIAL(TPersistentTags, CObject, 1);

TTagElement::TTagElement(const CString & groupName, int groupID)
   : m_groupName(groupName), m_GroupID(groupID)
{
}

//-------------------------------------------------------------------------
TPersistentTags::TPersistentTags()
   : PObject(2)
{
   InitializeCriticalSection ( &m_CritSect );
}

TPersistentTags::~TPersistentTags()
{
   int tot = m_ptrArray.GetSize();
   for (--tot; tot >= 0; --tot)
      {
      TTagElement* pElem = (TTagElement*) m_ptrArray[tot];
      delete pElem;
      }
   DeleteCriticalSection ( &m_CritSect );
}

///////////////////////////////////////////////////////////////////////////
// Serialize -- Mistake! Version 1.00 did not call PObject::Serialize, so
//              we have no version number.

#define PERSISTTAG_KEY 4194304

void TPersistentTags::Serialize(CArchive& ar)
{
   int i;
   LONG items;
   const LONG lKeyBeginVersion = PERSISTTAG_KEY;

   TEnterCSection mgr (&m_CritSect);

   if (ar.IsStoring())
      {
      ar << lKeyBeginVersion;

      PObject::Serialize ( ar );

      items = m_ptrArray.GetSize();
      ar << items;

      for (i = 0; i < items; ++i)
         {
         TTagElement* pElem = m_ptrArray.GetAt(i);

         pElem->Serialize( ar );
         }
      }
   else
      {
      LONG lTemp;
      ar >> lTemp;

      if (lTemp < lKeyBeginVersion)
         {
         // this is version one. The hack assumes that the size of the
         //  array would be less than 4194304
         items = lTemp;
         SerializeVer464 (ar, items);
         return;
         }

      PObject::Serialize ( ar );

      ar >> items;

      m_ptrArray.SetSize ( items );
      for (int i=0; i < items; ++i)
         {
         TTagElement* pElem = new TTagElement;
         pElem->Serialize ( ar );
         m_ptrArray.SetAt (i, pElem);
         }
      }
}

//-------------------------------------------------------------------------
void TPersistentTags::SerializeVer464(CArchive& ar, LONG items)
{
   // we are reading data in
   ASSERT(ar.IsStoring() == FALSE);

   m_ptrArray.SetSize( items );
   for (int i=0; i < items; ++i)
      {
      TTagElement* pElem = new TTagElement;
      pElem->SerializeVer464 ( ar );
      m_ptrArray.SetAt (i, pElem);
      }
}

/////////////////////////////////////////////////////////////////
//
//  handle duplicates
int TPersistentTags::AddTag (TNewsGroup *pNG,
   int artInt, int iLines, TArticleHeader* pHdr /* =NULL */)
{
   ASSERT(pNG);

   // If the body is local already then don't allow Tag operation.
   // Note - I don't have to Open the newsgroup to do this.
   if (pNG->TextRangeHave (artInt))
      {
      delete pHdr;
      return -1;
      }

   const int & groupID = pNG->m_GroupID;

   if (FindTag (groupID, artInt))
      {
      delete pHdr;
      return -1;
      }

   TEnterCSection mgr (&m_CritSect);

   int tot = m_ptrArray.GetSize();
   BOOL fFoundGroup = FALSE;
   int ret;
   for (--tot; tot >= 0; --tot)
      {
      TTagElement* pElem = (TTagElement*) m_ptrArray[tot];
      if (pElem->m_GroupID == groupID)
         {
         ret = pElem->Add (artInt, iLines, pHdr);
         ASSERT(0 == ret);
         fFoundGroup = TRUE;
         break;
         }
      }

   if (!fFoundGroup)
      {
      TTagElement* pNew = new TTagElement (pNG->GetName(), groupID);
      pNew->m_rArt.Add ( new TTagArt(artInt, iLines, pHdr) );
      m_ptrArray.Add ( pNew );
      ret = 0;
      }

   // new option for Build 525
   if (gpGlobalOptions->GetRegSwitch()->GetTagDoesUnread ())
      {
      // mark it as 'New' but do not 'undo' cross-post management
      pNG->ReadRangeSubtract_NoCPM ( artInt );
      }
   return ret;
}

/////////////////////////////////////////////////////////////////
// Delete the tag.  If the job has been submitted to the Pump,
//   kill the job.
int TPersistentTags::DeleteTagCancelJob (const CString & groupName,
   int groupID, int artInt, BOOL fCancelDownloadJob)
{
   int iSubmitted = 0;
   int iStat = 0;
   {
   // get the critical section and call internal function
   TEnterCSection mgr (&m_CritSect);

   iStat = delete_tag (groupID, artInt, &iSubmitted);
   }

   if (iSubmitted && fCancelDownloadJob && gpTasker)
      {
      gpTasker->CancelRetrieveJob ( groupName, artInt );
      }
   return iStat;
}

// ------------------------------------------------------------------------
// piSubmitted is TRUE if request has already been passed to Pump
int TPersistentTags::delete_tag (int groupID, int artInt,
   int* piSubmitted /* =0 */)
{
   int tot = m_ptrArray.GetSize();
   for (--tot; tot >= 0; --tot)
      {
      TTagElement* pElem = (TTagElement*) m_ptrArray[tot];
      if (pElem->m_GroupID == groupID)
         {
         int n = pElem->m_rArt.GetSize();
         for (int i = 0; i < n; i++)
            {
            TTagArt* pTagArt = pElem->m_rArt[i];

            if (pTagArt->m_artInt == artInt)
               {
               pElem->m_rArt.RemoveAt(i);

               // return TRUE if request has already been passed to Pump
               if (piSubmitted)
                  *piSubmitted = pTagArt->m_fSubmitted;

               delete pTagArt;
               if (1 == n)
                  {
                  // last guy - delete this set for newsgroup
                  m_ptrArray.RemoveAt(tot);
                  delete pElem;
                  }
               return 0;
               }
            }
         }
      }

   // probably OK here... the rules-based fetch-body action also goes through
   // here
   return 0;
}

//////////////////////////////////////////////////////////////
// Public function
//
BOOL TPersistentTags::FindTag (int groupID, int artInt)
{
   TEnterCSection mgr (&m_CritSect);

   int tot = m_ptrArray.GetSize();
   for (--tot; tot >= 0; --tot)
      {
      TTagElement* pElem = (TTagElement*) m_ptrArray[tot];
      if (pElem->m_GroupID == groupID)
         {
         if (pElem->FindTag( artInt ))
            return TRUE;
         }
      }
   return FALSE;
}

//////////////////////////////////////////////////////////////
static int  RetrieveTagged1Element (TServerCountedPtr & cpNewsServer,
                                    TTagElement * pElem )
{
   BOOL fRet;
   BOOL fNGFound;

   // double check to make sure the newsgroup is still subscribed

   {
    BOOL fUseLock;
    TNewsGroup * pNG;
    TNewsGroupUseLock useLock(cpNewsServer, pElem->m_GroupID, &fUseLock, pNG);
    fNGFound = fUseLock;
   }

   if (!fNGFound)
      return 1;

   CTypedPtrArray<CObArray, TTagArt*>& v = pElem->m_rArt;

   for (int i = 0; i < v.GetSize(); i++)
      {
      TTagArt* pTagArt = v.GetAt(i);
      if (FALSE == pTagArt->m_fSubmitted)
         {
         fRet = gpTasker->RetrieveArticle( pElem->m_groupName,
                                           pElem->m_GroupID,
                                           pTagArt->m_artInt,  // art #
                                           pTagArt->m_iLines   // line count
                                         );
         if (FALSE==fRet)
            {
            CString str; str.LoadString (IDS_ERR_RETRIEVE_ARTICLES);
            NewsMessageBox(TGlobalDef::kMainWnd, str);
            return 0;
            }
         else
            {
            // mark this guy as Queued
            pTagArt->m_fSubmitted = TRUE;
            }
         }
      }
   return 0;
}

//////////////////////////////////////////////////////////////
//
//
int TPersistentTags::RetrieveTagged (bool fAllGroups, CDWordArray & vecGroupIDs)
{
   TEnterCSection mgr(&m_CritSect);
   TServerCountedPtr cpNewsServer;     // smart pointer
   BOOL              fUnsubcribeVictim = FALSE;

   VERIFY (gpTasker->IsConnected());

   if (fAllGroups)
      {
      int tot = m_ptrArray.GetSize();
      for (--tot; tot >= 0; --tot)
         {
         TTagElement * pElem = m_ptrArray[tot];

         if (RetrieveTagged1Element ( cpNewsServer, pElem ))
            {
            // newsgroup has been unsubscribed! Remove Element for this NG.
            pElem->Clean();
            m_ptrArray.RemoveAt(tot);
            fUnsubcribeVictim = TRUE;
            }
         }
      }
   else
      {
      for (int  i = 0; i < vecGroupIDs.GetSize(); i++)
         {
         int idx = (int) vecGroupIDs[i];

         for (int j = 0; j < m_ptrArray.GetSize(); j++)
            {
            if (m_ptrArray[j]->m_GroupID == idx)
               {
               TTagElement * pElem =  m_ptrArray[j];
               if (RetrieveTagged1Element ( cpNewsServer, pElem ))
                  {
                  pElem->Clean ();
                  m_ptrArray.RemoveAt (j);
                  fUnsubcribeVictim = TRUE;
                  }
               break;
               }
            }
         }
      }

   // update disk image
   if (fUnsubcribeVictim)
      cpNewsServer->SavePersistentTags ();
   return 0;
}

//-------------------------------------------------------------------------
// Mark every article as unsubmitted
int TPersistentTags::InitializeSubmit ()
{
   TEnterCSection mgr (&m_CritSect);
   int tot = m_ptrArray.GetSize();
   for (--tot; tot >= 0; --tot)
      {
      (m_ptrArray[tot]) -> InitializeSubmit ();
      }
   return 0;
}

//-------------------------------------------------------------------------
// Mark every article as unsubmitted
int TTagElement::InitializeSubmit ()
{
   int i = m_rArt.GetSize();
   for (--i; i >= 0; --i)
      m_rArt[i]->m_fSubmitted = FALSE;
   return 0;
}

//-------------------------------------------------------------------------
// ctor
TTagElement::TTagElement()
   : PObject(2)
{
   m_fSorted = FALSE;
}

TTagElement::~TTagElement()
{
   int tot = m_rArt.GetSize();
   for (int i = 0; i < tot; ++i)
      {
      TTagArt* pTagArt = m_rArt[i];
      delete pTagArt;
      }
}

//-------------------------------------------------------------------------
void TTagElement::Serialize(CArchive & ar)
{
   LONG tot;
   int i;

   // write out version number
   PObject::Serialize ( ar );

   if (ar.IsStoring())
      {
      ar << m_groupName << m_GroupID;
      tot = m_rArt.GetSize();
      ar << tot;
      for (i = 0; i < tot; ++i)
         {
         m_rArt[i]->Serialize( ar );
         }
      }
   else
      {
      ar >> m_groupName >> m_GroupID >> tot;
      m_rArt.SetSize( tot );
      for (i = 0; i < tot; ++i)
         {
         TTagArt * pTagArt = new TTagArt;
         pTagArt->Serialize ( ar );
         m_rArt.SetAt(i, pTagArt);
         }
      }
}

//-------------------------------------------------------------------------
void TTagElement::SerializeVer464(CArchive& ar)
{
   ASSERT(ar.IsStoring() == FALSE);
   LONG tot;
   int i;
   CObject::Serialize(ar);

   ar >> m_groupName >> m_GroupID >> tot;
   m_rArt.SetSize( tot );
   for (i = 0; i < tot; ++i)
      {
      TTagArt * pTagArt = new TTagArt;
      pTagArt->SerializeVer464 ( ar );
      m_rArt.SetAt(i, pTagArt);
      }
}

//-------------------------------------------------------------------------
void TTagElement::Clean()
{
   int tot = m_rArt.GetSize();
   for (int i = 0; i < tot; ++i)
      delete m_rArt[i];
   m_rArt.RemoveAll();
}

//-------------------------------------------------------------------------
BOOL TTagElement::FindTag(int artInt)
{
   if (FALSE == m_fSorted)
      {
      // SORT LIST via base class
      m_rArt.HeapSortItems ( TTagElement::fnSortCompare );

      m_fSorted = TRUE;
      }

   // use binary search here
   int idx = 0;
   BOOL fRet = m_rArt.BinSearch ( TTagElement::fnSearchCompare,
                                  &artInt, &idx );

   return fRet;
}

//-------------------------------------------------------------------------
// static class function
int TTagElement::fnSortCompare(const CObject* pObj1, const CObject* pObj2)
{
   const TTagArt* pTArt1 = static_cast<const TTagArt*>(pObj1);
   const TTagArt* pTArt2 = static_cast<const TTagArt*>(pObj2);
   if (pTArt1->m_artInt > pTArt2->m_artInt)
      return 1;
   else if (pTArt1->m_artInt < pTArt2->m_artInt)
      return -1;
   else
      return 0;
}

//-------------------------------------------------------------------------
// static class function
int TTagElement::fnSearchCompare(void * pVoid, const CObject* pObj1)
{
   const TTagArt* pTArt1 = static_cast<const TTagArt*>(pObj1);
   int key = *((int*) pVoid);
   if (key > pTArt1->m_artInt)
      return 1;
   else if (key < pTArt1->m_artInt)
      return -1;
   else
      return 0;
}

int TTagElement::Add (int artInt, int iLines, TArticleHeader* pHdr)
{
   if (FALSE == m_fSorted)
      {
      // SORT LIST via base class
      m_rArt.HeapSortItems ( TTagElement::fnSortCompare );
      m_fSorted = TRUE;
      }

   // use binary search here
   int idx = 0;
   BOOL fRet = m_rArt.BinSearch ( TTagElement::fnSearchCompare,
                                  &artInt, &idx );

   if (fRet)
      {
      // already in there
      ASSERT(0);
      return -1;
      }
   else
      {
      TTagArt* pArt = new TTagArt(artInt, iLines, pHdr);
      m_rArt.InsertAt ( idx, pArt );
      }
   return 0;
}

//-------------------------------------------------------------------------
// constructor for serialization
TTagArt::TTagArt()
   : PObject(2)
{
   m_pArtHdr = 0;
   m_fSubmitted = 0;
}

///////////////////////////////////////////////////////////////////////////
// accepts ownership of the ptr
//
TTagArt::TTagArt(int iArtNum, int iLines, TArticleHeader* pHdr)
   : PObject(2), m_artInt(iArtNum), m_iLines(iLines)
{
   m_fSubmitted = 0;
   m_pArtHdr = pHdr;  // this member is never serialized
}

///////////////////////////////////////////////////////////////////////////
// a personal copy of the Article Header may or may not be here
TTagArt::~TTagArt()
{
   delete m_pArtHdr;
}

//-------------------------------------------------------------------------
void TTagArt::Serialize(CArchive & ar)
{
   PObject::Serialize( ar );

   if (ar.IsStoring())
      {
      ar << m_artInt << m_iLines;
      }
   else
      {
      ar >> m_artInt >> m_iLines;
      }
}

//-------------------------------------------------------------------------
void TTagArt::SerializeVer464(CArchive & ar)
{
   ASSERT(ar.IsStoring() == FALSE);
   ar >> m_artInt >> m_iLines;
}

