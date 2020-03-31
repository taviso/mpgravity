/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecjob.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:59  richard_wood
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

// tdecjob.h -- decoding job

#pragma once

#include "tutljob.h"          // TUtilityJob, TArticleHeader, TArticleText

#include "stldeclare.h"       // VEC_HDRS

// -------------------------------------------------------------------------
// version of this decode-job data structure
#define DECODE_JOB_VERSION_NUMBER   2

// -------------------------------------------------------------------------
// TDecodeJob -- type of CObject we store in our queue
class TDecodeJob : public TUtilityJob
{
public:
   TDecodeJob ();             // default constructor for empty job objects
   TDecodeJob (const TDecodeJob *pCopy);
   TDecodeJob (LONG lGroupID,
               const CString & rstrGroupName,
               const CString & rstrGroupNickname,
               TArticleHeader *psHdr,
               const CString & strDirectory,
               int iRuleBased = FALSE,    // if true, then consult pending-jobs list
               TArticleText *pText = NULL,
               char *pchSubject = NULL,
               BOOL bLaunchViewer = FALSE);

   TDecodeJob::~TDecodeJob ();
   TDecodeJob &operator= (const TDecodeJob &src);

   // return true to re-queue the job
   bool DoYourJob (HANDLE hKillEvent);
   void PreprocessJob (VEC_HDRS * pVecHdrs);
   const CString &Directory () { return m_strDirectory; }
   const CStringList &Filenames () const { return m_rstrFilenames; }
   const CString FirstFilename ();
   void SetFilename (const CString &strNewName);
   void CopyVisualFields (TDecodeJob *pSource);
   void SetSubclassStatus (int iResourceID);
   void JobBeingCancelled ();
   void Serialize (CArchive& sArchive);
   int ArticleFullPath (LONG lGroupID, LONG lArtNum, CString &strPath);
   void ComputePrefix ();
   void ZeroNumParts () { m_iNumParts = 0; } // used in TDecodeThread::AddJob()
   BOOL WasPartNumInSubject () { return !GetImpliedPart1 (); }
   BOOL MergeJobsInResultPane (TUtilityJob *pOtherJob);
   void SetLaunchViewer () { m_bLaunchViewer = TRUE; }
   void JobKickedBackToWaitQueue ();
   void MarkArticlesRead ();

   bool CanClone() { return true; }
   TUtilityJob * Clone();

   DECLARE_SERIAL (TDecodeJob)

private:
   enum EFlagDecodeJob { kImpliedPart1 = 0x1, kOK = 0x2 };

   BOOL LocateParts (const CString & strPrefix, int iThisPart, VEC_HDRS* pVecHdrs);
   void LocatePartsFromUI (const CString & strPrefix, int * piSetStatus, VEC_HDRS* pVecHdrs);
   void AddPart (TArticleHeader *pHdr, int iPart);
   int ConsultPendingJobs (LPCTSTR pchPrefix, int iThis);
   void PurgePendingJob (LPCTSTR pchPrefix);
   CDialog *MonitorDialog (); // dialog that may be monitoring us
   void  LockDialogPtr(BOOL fLock);
   void SetStatusBits (TStatusUnit::EStatus iBit, BOOL fValue = TRUE);
   void SetImpliedPart1 ();
   BOOL GetImpliedPart1 ();
   void SetSuccessful ();
   BOOL GetSuccessful ();
   BOOL LaunchViewer ();

   CStringList m_rstrFilenames;  // one or more binary files
   CString m_strDirectory;    // user can override default directory
   TArticleText *m_pText;     // article's text, if already available
   int m_iNumParts;           // number of parts in this post
   int m_iGotParts;           // number of parts that have been located
   int *m_riFoundParts;       // array of found-flags
   TArticleHeader *m_rsPartHeaders; // array of headers
   BOOL m_bLaunchViewer;      // launch viewer after decoding
   int m_iThisPart;           // this part's number
   int m_iPreProcessError;    // problem with LocateParts
   WORD m_wFlags;             // not serialized

   CString  m_strSubjFilename; // possible filename extracted from subject line
};

// -------------------------------------------------------------------------
// TDecodeJobs -- collection of decode jobs
class TDecodeJobs : public TUtilityJobs
{
public:
   // default ctor
   TDecodeJobs () { }

   // pass in name of news server
   TDecodeJobs (const CString& serverName)
      : TUtilityJobs (serverName) { }

   TUtilityJob *CreateJob () { return new TDecodeJob; };
};
