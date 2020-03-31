/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tasker.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:58  richard_wood
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

// tasker.h : header file
//

#pragma once

#include <afxmt.h>
#include "appversn.h"
#include "declare.h"
#include "article.h"
#include "callbks.h"
#include "smtp.h"
#include "taskcon.h"
#include "pumpjob.h"
#include "humble.h"

class TRangeSet;
class TFetchArticle;
class THeapBitmap;
class TScribePrep;
class TNewsScribe;
class TOutbox;
class TPump;
class TPumpJobs;
class TPumpExternalLink;

/////////////////////////////////////////////////////////////////////////////
// TNewsTasker thread

class TNewsTasker : public CHumbleThread
{
   DECLARE_DYNCREATE(TNewsTasker)

friend TPump;

// Types
public:
   enum EHeaderType { kHeadersNew, kHeadersRead, kHeadersNewAndRead };

   // when getting headers - separate between UserAction and a program
   //    action.  The UserAction will re-open the current NG after getting
   //    stuff.
   enum EGetAction  { kActionUser, kActionProgram };
   enum EGetType    { kGetHeader,  kGetCounts };
   enum EArtMissing { kArtMissing, kArtExpired };

// Operations
public:
   TNewsTasker (void);
   virtual ~TNewsTasker();
   int RegisterGroup (const CString& groupName);
   int UnregisterGroup (TNewsGroup* pGrp);

   DWORD ResumeTasker(void);
   BOOL IsRunning(void) { return m_fTaskerRunning; }

   void Delete ();      // thread cleanup

   void AddResult (TPumpJob * pResult);

   int  PriorityArticle (const CString & ngroup,
                         LONG groupID,
                         BOOL fSaveBody,
                         TArticleHeader* pHeader,
                         TFetchArticle* pObjFetch);
   int  NonPriorityArticle (const CString & ngroup,
                         LONG groupID,
                         BOOL fSaveBody,
                         TArticleHeader* pHeader,
                         TFetchArticle* pObjFetch);

   //  start connections to the newsserver
   int Connect (LPCTSTR server);
   int SecondConnect (LPCTSTR server);

   //  drop connections to the newsserver
   void Disconnect ();
   void SecondDisconnect (void);

   BOOL IsConnected(bool * pfConnecting = NULL);
   BOOL SecondIsConnected(BOOL* pfContinue, BOOL* pfNull = NULL);

   // get these articles specifically
   BOOL RetrieveArticle ( const CString& groupName,
                          LONG  groupID,
                          int artInt, int iLineCount );

   // pass thru to pump
   //BOOL QueueAnalyze ( TPumpControl& control );
   //int  QueueReOrder ( TPumpControl& control );

   // destroy msg (article|email) in db - pass info to ArtCleaner
   BOOL DestroyMessage ( const CString& groupName, int msgInt );

   // rules has analyzed the header and wants to throw this article away
   //  (cancel download of the article text)
   BOOL CancelArticleBody ( const CString& groupName, int artInt );

   // DEL key applied to tagged article.  chase down & destroy pump job
   BOOL CancelRetrieveJob ( const CString& groupName, int artInt );

   // to prevent 2 copies of tag jobs (dropped line , reconnect, resubmit taglist)
   int EraseAllTagJobs ();

   // For 'store-nothing' newsgroups
   // BOOL GetRangeFor ( const CString& groupName, BOOL fOld, BOOL fNew);
   //BOOL Quick_LoadHeaders ( const CString& groupName, EHeaderType eHdrType,
   //                         BOOL fImportant, HANDLE hEventDone);

   void GetStatusData(int& iFetch);

   int  TaskerFetchBody (const CString & groupName, int groupID,
                         const CString & subject, CPoint & ptPartID,
                         int artInt, int iLines,
                         TFetchArticle* pobjFetch, BOOL fPrioPump,
                         BOOL fMarkAsRead, BOOL bEngage = TRUE);

   BOOL PrioritizeNewsgroup ( LPCTSTR groupName,
                              BOOL    fGetAll,
                              int     iHdrsLimit,
                              EGetAction eAction = TNewsTasker::kActionUser );

   int  VerifyLocalHeaders (TNewsGroup* pNG);

   int  StartRetrieveGroup (TNewsTasker::EGetAction eAction, const CString & ngName);
   int  ForceRetrieveGroup (TNewsTasker::EGetAction eAction, const CString & ngName);
   int  StartRetrieveCycle (TNewsTasker::EGetAction eAction);

   BOOL CanRetrieveCycle(void);

   int  StartPingCycle(void);
   int  PingList (const CStringList& lstNames);
   void AddScribeJob(TScribePrep* pPrep);

   BOOL NormalPumpBusy ();
   BOOL EmergencyPumpBusy ();

   void StopEmergencyPump ();

   int RetrieveNewsgroups(BOOL fGetAll, WPARAM wParam,
                          CTime& since);

   BOOL GetOutboxCompactLock (DWORD wait);
   void ReleaseOutboxCompactLock ();

   void KillPrintAndDecodeThreads ();

   int  AddJob (TPumpJob * pJob);

   //void SignalDisconnect ();
   BOOL IsSendingEmail ();

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(TNewsTasker)
	public:
   virtual BOOL InitInstance();
   virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
   TPumpJob* GetResult (void);
   void SingleStep();
   void ProcessResult(void);
   void ResultExpandGroup(TPumpJob* pResult);
   void MarkArticleAsPosted(TPumpJob* pResult);
   BOOL CheckOutgoingPosts(void);
   void HandleOverview ( TPumpJob* pResult );
   void HandleXDate ( TPumpJob* pResult );
   void Process_SMTP_Result();
   BOOL DoAllOutgoing(void);
   BOOL CheckOutgoingEmail (void);
   int  SendOutboundArticle ( int artInt );
   void HandleHeadersStatus (TPumpJob* pResult);
   void log_smtp_result();

   BOOL RetrieveOneGroup (LPCTSTR groupName, BOOL fGetAll, int iHdrsLimit, EGetAction eGetAction);
   BOOL HandleConnectResult (TPumpJob*& pResult);

   void ReportConnectError (TPumpJob* & pResult);
   void calculate_new_articles(TPumpJob* pBaseJob);

   // Generated message map functions
   //{{AFX_MSG(TNewsTasker)
      // NOTE - the ClassWizard will add and remove member functions here.
   //}}AFX_MSG

   DECLARE_MESSAGE_MAP()

   void start_thread_scribe ();
   void end_thread_scribe ();
   void delete_thread_scribe ();

   int  priority_article_driver(TPumpJobs* pPumpJobs,
                                const CString &  newsgroup,
                                LONG groupID,
                                BOOL fSaveBody,
                                TArticleHeader* pArtHdr, TFetchArticle* pObjFetch);
   int connect_driver(BOOL fEmergency, LPCTSTR server);

   void disconnect_normal ();
   void disconnect_prio   ();
   void disconnect_helper (TPumpExternalLink * & rpLink, TPumpJobs * pPumpJobs);

   int  engage_normpump ();   // handles on demand-requests
   int  engage_priopump ();

   //void SetNormalPumpState(TTaskerConnector::ETaskerConnect eState);
   //void SetPrioPumpState(TTaskerConnector::ETaskerConnect eState);

   BOOL prepare_msgpool();
   BOOL thread_outgoing_email(TOutbox* pOutbox, CDWordArray& vArtInts);
   int  validate_pump(int iWhich, BOOL * pfNormalValid);
   int  periodic_retr_cycle ();
   int  periodic_saveng_cycle ();
   int  periodic_save_decode_jobs ();

public:  // data
   HANDLE m_KillRequest;
   HANDLE m_Killed;

private:
   // list of newsgroups (make a circular list type?)
   //CTypedPtrMap<CMapStringToPtr, CString, TNewsGroup*> m_GrpMap;

   BOOL        m_fTaskerRunning;

   CTime        m_LastGetHdrs4GroupsTime;
   CTime        m_LastSaveNGListTime;

   TNewsScribe * m_pScribe;

   HANDLE        m_hmtxTasker;

   // jobs are owned by the Tasker.  That way the Queue doesn't disappear
   //   when the pump is disconnected
   TPumpJobs   * m_pJobsPump1;
   TPumpJobs   * m_pJobsPump2;

   // this memory is allocated by Tasker, and freed by the pump destructor
   TPumpExternalLink * m_pPumpLink1;
   TPumpExternalLink * m_pPumpLink2;

private:   // result bucket
   CTypedPtrList<CObList, TPumpJob*> m_Results;
   HANDLE   m_hMutexResBucket;
   HANDLE   m_hEventResReady;
   HANDLE   m_hEventMail;     // smtp result done
   int      m_instanceRet;
   HANDLE   m_hScribeUndercapacity;
   CMutex   m_mtxProtectsScribe;
   HANDLE   m_outboxCompactMutex;

protected:
   TEmailMsgPool m_MsgPool;

private:

   HANDLE           m_hPumpPerfmonData;

   CTime            m_timeLastSaveDecodeJobs;
};
