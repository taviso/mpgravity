/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: decoding.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2008/09/19 14:51:20  richard_wood
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

// decoding.cpp -- handles decode messages from the menus

#include "stdafx.h"              // standard stuff
#include "decoding.h"            // this file's prototypes
#include "tdecthrd.h"            // gpDecodeThread, ...
#include "tdecjob.h"             // TDecodeJob
#include "tmandec.h"             // gpManualDecodeDlg
#include "tglobopt.h"            // gpGlobalOptions
#include "rgswit.h"              // TRegSwitch
#include "news.h"                // CNewsApp
#include "newsdoc.h"             // stores PanicMode boolean
#include "tdecutil.h"            // GetDecodeDirectory(), ...
#include "nglist.h"              // TNewsGroupUseLock, ...
#include "uipipe.h"              // gpUIPipe
#include "gallery.h"             // ViewBinary()
#include "artnode.h"             // TArtNode iterators

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static char grchFilename [MAX_PATH + 1];  // filename for UI decoding

// -------------------------------------------------------------------------
// Public
//
int QueueArticleForManualDecoding (TArticleHeader *pHdr, int iSelected,
   TNewsGroup *pNG, DWORD)
{
   if (!iSelected)
      return 0;
   gpsManualDecodeDlg->AddPart (pHdr, pNG);
   return 0;
}

// -------------------------------------------------------------------------
//
// CALLED FROM:  QueueRuleArticleForDecoding,  UtilDecodeCallback
//
//
static void fnAddDecodeJob (TNewsGroup *     pNG,
                            TArtNode *       pNode,
                            TArticleHeader * pHeader,
                            const CString &  strDirectory,
                            int              iRuleBased = FALSE,
                            TArticleText *   pText = NULL,
                            BOOL             bLaunchViewer = FALSE,
                            BOOL             bPriority = FALSE)
{
   BOOL bEliminateDuplicate;

   ASSERT(pNode || pHeader);

   // queue the article

   // get the name or the nickname
   CString strGroupNickname = pNG->GetBestname ();

   VEC_HDRS * pVecHdrs = new VEC_HDRS;

   if (pNode)  // check node first
      {
      STLMapAHdr::iterator   it = pNode->GetArtsItBegin();

      for (; it != pNode->GetArtsItEnd(); it++)
         {
         if (it == pNode->GetArtsItBegin())
            pHeader =  *it;                  // take the first guy as Leader
         else
            pVecHdrs->push_back ( *it );
         }
      }
   else if (pHeader)
      {
      // pVecHdrs stays empty
      }

   TDecodeJob *pJob = new TDecodeJob (pNG->m_GroupID,
                                      pNG->GetName (),
                                      strGroupNickname,
                                      pHeader,
                                      strDirectory,
                                      iRuleBased,
                                      pText,
                                      NULL /* pchSubject */,
                                      bLaunchViewer);

   if (iRuleBased)
      bEliminateDuplicate = FALSE;
   else
      bEliminateDuplicate = TRUE;

   gpDecodeThread->AddJob (pJob,
                             pVecHdrs,
                             bEliminateDuplicate,
                             bPriority);

   delete pVecHdrs;
}

// -------------------------------------------------------------------------
// Public
// QueueRuleArticleForDecoding -- a version for the rule system to call,
//    which may have the text handy at the time
//
void QueueRuleArticleForDecoding (TArticleHeader *pHeader, TArticleText *pText,
   TNewsGroup *pNG, CString &strDirectory)
{
   fnAddDecodeJob (pNG, NULL /*pNode*/, pHeader, strDirectory, TRUE /* iRuleBased */, pText);
}

// -------------------------------------------------------------------------
//  Use pNode if provided, else use pHeader
static int UtilDecodeCallback (bool bPriority,
                               TArtNode * pNode,
                               TArticleHeader * pHeader,
                               TNewsGroup * pNG,
                               DWORD dwData)
{
   DecodeArticleCallbackData *pData = (DecodeArticleCallbackData *) dwData;

   BOOL bLaunch         =  FALSE;
   TArticleText * pText =  NULL;
   CString strDirectory;

   if (pData)
      {
      bLaunch = pData->bLaunch;
      pText   = pData->pText;
      strDirectory = pData->decodeDir;
      }

   // if priority mode is on, move the job to the top if it's in
   // the waiting queue
   if (bPriority)
      {
      int iID = gpDecodeThread->GetWaitingJobID (pNG->m_GroupID,
                                                   pHeader->GetNumber ());

      if (iID != -1)
         {
         gpDecodeThread->SetJobLaunch (iID);
         gpDecodeThread->MoveWaitingJobToTop (iID);
         return 0;
         }
      }

   if (bLaunch)
      {
      CString strPath;

      if (!gpDecodeThread->ArticleFullPath (pNG->m_GroupID,
                                              pHeader->GetNumber (),
                                              strPath))
         {
         // if path is empty, then the decode failed.  If boss is looking,
         // don't view image
         bool bBossMode =
            ((CNewsApp *) AfxGetApp ())->GetGlobalNewsDoc ()->PanicMode ();

         if (!strPath.IsEmpty () && !bBossMode)
            {
            ViewBinary (bBossMode, strPath);

            if (gpGlobalOptions->GetRegSwitch()->GetDecodeMarksRead () ==
                  TRegSwitch::kReadAfterView)
                pNG->ReadRangeAdd (pHeader->GetNumber ());
            }
         return 0;
         }
      }

   fnAddDecodeJob (pNG,
                   pNode,
                  pHeader,
                  strDirectory,
                  FALSE /* iRuleBased */,
                  pText,
                  bLaunch,
                  bPriority);
   return 0;
}

// -------------------------------------------------------------------------
// NormalDecodeCallback -- takes an article and performs some operation on it
// if it's selected
//   pHeader -- article's header
//   pNG -- newsgroup object
//   dwData -- pointer to a structure of type DecodeArticleCallbackData
// Returns nonzero if an error occurs, otherwise returns zero
//
int NormalDecodeCallback (TArtNode * pNode,
                           TArticleHeader *pHeader,
                           TNewsGroup *pNG,
                           DWORD dwData)
{
   return UtilDecodeCallback (false, // bPriority,
                             pNode,
                             pHeader,
                             pNG,
                             dwData);
}

// -------------------------------------------------------------------------
int PriorityDecodeCallback (TArtNode * pNode,
                            TArticleHeader * pHeader,
                            TNewsGroup *     pNG,
                            DWORD)
{
   CString dir;
   DecodeArticleCallbackData sData(TRUE, NULL, dir);

   return UtilDecodeCallback (true,  // bPriority
                             pNode,
                             pHeader,
                             pNG,
                             (DWORD) &sData);
}
