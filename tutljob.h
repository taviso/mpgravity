/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutljob.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:39  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:52:21  richard_wood
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

// tutljob.h -- utility-thread job

#pragma once

#include "article.h"          // TArticleHeader, TArticleText
#include "statunit.h"         // TStatusUnit
#include "pobject.h"          // PObject
#include "mplib.h"            // TError

class TUtilityJobs;

enum EArticleSource { SRC_FROM_DB, SRC_FROM_CACHE, SRC_FROM_NET } ;

// -------------------------------------------------------------------------
class TUtilityJobJanitor : public CObject
{
public:
   TUtilityJobJanitor ();
   ~TUtilityJobJanitor ();
};

// -------------------------------------------------------------------------
class TUtilityJob : public PObject
{
   // for callbacks from the tasker thread
   friend void TaskerCallback (int iStatus, TArticleHeader *pHdr,
      TArticleText *pText);
   friend class TUtilityJobs;
   friend class TUtilityJobJanitor;

public:
   TUtilityJob (BYTE bVersion);     // default constructor
   TUtilityJob (BYTE bVersion, const TUtilityJob *pCopy);
   TUtilityJob (BYTE bVersion, LONG lGroupID, const CString & rstrGroupName,
      const CString & rstrGroupNickname, TArticleHeader *psHdr, int iRuleBased);
   virtual ~TUtilityJob () {};
   TUtilityJob &operator= (const TUtilityJob &src);
   virtual bool DoYourJob (HANDLE hKillEvent) = 0;
   int WriteArticleToFile (char * pchTempFile, HANDLE hKillEvent);

   LPCTSTR Subject ();
   const CString &Newsgroup () { return m_strGroupName; }
   const CString &NewsgroupNickname () { return m_strGroupNickname; }
   const CString &Status () { return m_strStatus; }
   int RuleBased () { return m_iRuleBased; }
   LONG GetGroupID () { return m_lGroupID; }
   enum StatusCode { NORMAL_STATUS, ERROR_STATUS };
   void SetStatus (int iResourceID, StatusCode iCode = NORMAL_STATUS);
   void SetStatus (LPCTSTR pchStatus, int iResourceID,
      StatusCode iCode = NORMAL_STATUS);
   void SetStatusBits (TStatusUnit::EStatus iBit);
   long GetArticleNumber () { return m_sHdr.GetNumber (); }
   virtual void Serialize (CArchive& sArchive);
   virtual BOOL MergeJobsInResultPane (TUtilityJob *pOtherJob)
      { return m_strPrefix == pOtherJob->m_strPrefix; }
   virtual void JobKickedBackToWaitQueue () {};

   virtual bool CanClone() = 0;
   virtual TUtilityJob*  Clone() = 0;
     
public:
   int m_iID;                 // unique ID among a job queue's elements
   CString m_strPrefix;       // holds a decode job's prefix, and is used to
                              // update the visual display

protected:

   int PlsFetchArticle (TError & sErrorRet,
                        EArticleSource & eArticleSource,
                        TArticleHeader *pHdr,
                        TArticleText *&pText,
                        bool fUseCache,
                        HANDLE hKillEvent,
                        int iThis = 0,
                        int iTotal = 0);

   int GetTextFromNewsfeed (TArticleHeader *pHdr, TArticleText *&pText,
      HANDLE hKillEvent);
   virtual CDialog *MonitorDialog () = 0; // dialog that may be monitoring us
   virtual void LockDialogPtr (BOOL fLock) = 0;
   virtual void SetSubclassStatus (int iResourceID) { /* empty */ };
   void PostToMonitorDialog (int iMessage, WPARAM wParam = 0,
      LPARAM lParam = 0);

   LONG m_lGroupID;           // newsgroup's ID (to get DB collection's name)
   CString m_strGroupName;    // newsgroup's name (to fetch article)
   CString m_strGroupNickname;// newsgroup's name (to display in dialog)
   TArticleHeader m_sHdr;     // to retrieve body, and to decode header fields
   CString m_strStatus;       // status of this job
   int m_iRuleBased;          // started by a rule?
   CString m_strJobType;      // "Print", "Decode", ...  Used by SetStatus()
   CString m_strSubject;      // subject from article

private:
   static int s_iFetchStatus; // result of tasker text-fetch
   static TArticleText *s_pFetchText; // text fetched by tasker
   static HANDLE s_hFetchDone;
   static HANDLE s_hMutex;    // mutex for GetTextFromNewsfeed()
   static int s_iIDCounter;   // produces unique IDs
   static TUtilityJobJanitor s_Janitor;  // calls Close on s_hMutex
};

// -------------------------------------------------------------------------
// TUtilityJobs -- collection of utility jobs
class TUtilityJobs : public CPtrList
{
public:
   TUtilityJobs () { }        // default ctor

   TUtilityJobs (const CString& serverName)
      : m_strServerName(serverName) {  /* empty */ }

   ~TUtilityJobs();           // destructor

   void delete_all();

   void Serialize (CArchive& sArchive);
   virtual TUtilityJob *CreateJob () = 0;
   CString m_strServerName;
};

// -------------------------------------------------------------------------
class TDecodeCache
{
public:
   TDecodeCache();
   ~TDecodeCache();

   int Load (TArticleHeader * pHdr, TArticleText * & pText);
   int Save (TArticleHeader * pHdr, TArticleText * pText);

   int EraseAll ();

private:
   TPath  GenerateFilename ( TArticleHeader * pHdr );
   TPath  m_path;
   bool   m_fReady;
};

extern TDecodeCache * gpDecodeCache;
