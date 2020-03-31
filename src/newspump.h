/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newspump.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

// newspump.h : header file
//
#pragma once

#include <afxmt.h>
#include "appversn.h"
#include "newsfeed.h"
#include "declare.h"
#include "pumpjob.h"
#include "article.h"
#include "timek.h"

#define PERFMON_STATS

#include "enumjob.h"
#include "humble.h"     // CHumbleThread
#include "tsubjct.h"    // TSubject

class TNewsTasker;      // Forward ref
class TNewsScribe;      // Forward ref
class TScribePrep;
class TPrepHeader;
class TPumpControl;
class TPumpView;
class TExpiryData;
class TPumpJobs;
class TPump;

#define PUMP_IS_DEAD          0
#define PUMP_IS_CONNECTING    1
#define PUMP_IS_CONNECTED     2
#define PUMP_IS_DISCONNECTING 3

// retrieve an article for the Printing Thread
typedef void (*P_OUTBOUNDFUNC)(int status, TArticleHeader* pArtHdr, TArticleText* pArtText);
typedef CMap<int, int, TArticleHeader*, TArticleHeader*&> Pump_HeaderMap;

/////////////////////////////////////////////////////////////////////////////
// observed by the decode thread
class TPumpExternalLink : public TSubject
{
public:
	// constructor
	TPumpExternalLink (TNewsTasker *  pTasker,
		const CString& strServer,
		TPumpJobs *    pJobs,
		HANDLE         hScribeUnderCapacity,
		bool           fEmergencyPump);

	// destructor
	~TPumpExternalLink();

	// implement TSubject interface
	DWORD GetSubjectState ();
	void  Update_Observers() { UpdateAllObservers(); }

	int            m_iSocketError;
	int            m_iNNTPError;
	bool           m_fUserStop;
	TNewsTasker *  m_pTasker;       // send scribe jobs to Tasker
	TPumpJobs *    m_pJobs;
	bool           m_fPumpCtorFinished;

	HANDLE         m_hScribeUnderCapacity;
	bool           m_fEmergencyPump;

	HANDLE         m_hKillRequest;   // owned by pump

	CString        m_strErrorMessage;
	CString        m_strServer;

	bool           m_fConnected;     // goes from false->true. OneWay only

	HANDLE         m_hPumpThread;    // used by Tasker
};

/////////////////////////////////////////////////////////////////////////////
// responsible for holding the jobs
//
class TPumpJobs : public CObject
{
public:
	TPumpJobs();
	~TPumpJobs();

	// Operations
	void AddJob (TPumpJob * pJob);            // put Job at end of queue
	void AddJobToFront (TPumpJob * pJob);     // put Job at front of queue
	void GetJobCount (int& pumpJobs);
	void OnDisconnect ();                     // cancel certain job types

	int  EraseAllTagJobs ();

	// take job from front of queue
	TPumpJob * RemoveJob ();

	void RequestBody (
		const CString &   groupName,
		LONG              groupID,
		int               artInt,
		int               iLines,
		BOOL              fFrontOfQueue,
		DWORD             dwJobFlags);

	int  CancelNewsgroup(LPCTSTR groupName);

	BOOL CancelArticleBody ( const CString& groupName, int artInt );
	BOOL CancelRetrieveJob ( const CString& groupName, int artInt );

	// for synchronous calls
	BOOL FetchBody (
		const CString&    groupName,
		int               groupID,
		const CString &   subject,
		CPoint &           ptPartID,
		int               artInt,
		int               iLines,
		TFetchArticle*    pFetchArticle,
		BOOL              bMarkAsRead /* = TRUE */);

	// return True if we moved something
	BOOL QueueReOrder (LPCTSTR groupName);

	void cleanup_queue ();

protected:
	BOOL cancel_job_in_queue (const CString& groupName, int artInt,
		TPumpJob::EPumpJobType eType);



public:
	HANDLE            m_hEventJobReady;
	BYTE              m_bySacrifice;          // test byte

protected:
	CRITICAL_SECTION  m_csProtect;            // cheaper than a mutex

	CTypedPtrList<CObList, TPumpJob*> m_Jobs;
};

struct TRETRY
{
	TRETRY()
	{
		dwJobAddress = 0;
		iFailures = 0;
	}

	bool CanRetryJob (DWORD dwAddress, int iMax)
	{
		if (dwJobAddress == dwAddress)
		{
			return (iFailures ++ < iMax);
		}
		else
		{
			iFailures = 0;
			dwJobAddress = dwAddress;
			return (iFailures ++ < iMax);
		}
	}

	DWORD  dwJobAddress;
	int    iFailures;
};

/////////////////////////////////////////////////////////////////////////////
// TPump object

class TPump : public CObject
{
public:
	enum EHeaderParts{ kXFrom, kXDate, kXRefs, kXMsgID,
		kXSubject, kXLines, kXCrossPost, kXNewsgroups };
	typedef enum {kBatchHeaders = 1000000};  // no partial batching
	typedef enum { kMustUseXHDR = 0x1, kMustUseSTAT = 0x2,
		kMustUseXLines = 0x4, kXRefinXOver = 0x8};

#if defined(PERFMON_STATS)
	// perfmon stuff
	static BOOL PreparePerformanceMonitoring (LPHANDLE pHandle);
	static BOOL ShutdownPerformanceMonitoring (HANDLE hMap);
#endif

	static UINT  ThreadFunction (LPVOID pParam);

	TPump(PVOID  pVoid);

	UINT  Run ();

protected:
	TPumpExternalLink * m_pLink;
	TErrorList          m_sErrorList;
	TErrorList *        m_pErrList;

	// Attributes
public:

	virtual ~TPump();
	LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);
	void CancelBlockingCall();

	CString m_errorMessage;
	int     m_iNNTPError;

	static int GetSocketErrorID (int iLocalError);

	// Operations
public:
	void Delete();         // thread cleanup
	BOOL IsRunning();
	//EPumpStage CheckConnected();
	DWORD ResumePump(void);

	void ScribeJob (TScribePrep * pPrep);

	BOOL StartConnect ();

	void NoSendQuit () { m_fQuitNNTP = FALSE; }

	// Implementation
protected:
	TPumpJob * GetJob(void);

	void CommonSetup(void);

	int  CustomizedWait();
	void SingleStep (void);
	void SingleStepDispatch (TPumpJob * pJob);

	int  CheckNewArticles(TPumpJob * pJob);
	//void DownloadHeader(TPumpJob * pJob);
	//void DownloadFullHeader(TPumpJob * pJob);
	int  DownloadArticle(TPumpJob * pJob);
	int  download_body_util(TPumpJobArticle* pJob, TArticleText* pText);

	void DownloadFetchBody(TPumpJob * pJob);
	void DownloadOverview(TPumpJob * pJob);
	int  straight_overview(TPumpJob * pJob, int low, int high, MPBAR_INFO * psBarInfo,
		TArtHdrArray* pHdrArray);

	int  DoPingArticles (TPumpJob*  pJob);
	int  PostOneMessage(TPumpJob * pJob);

	void EndGroup(TPumpJob* pJob);
	void XDate(TPumpJob* pJob);

	int  Use_XHDR_Instead(TPumpJobOverview*& pJob, MPBAR_INFO * psBarInfo,
		BOOL fFullCrossPost, TArtHdrArray* pHdrArray);

	int  iSyncGroup( const CString& rGroup, FEED_SETGROUP * pSG);
	void StoreHeaderPart ( TPump::EHeaderParts ePart, CString& line, MPBAR_INFO * psBarInfo);

	int  connect (CString& rstrWelcome);

	void CheckHeadersStatus ( TPumpJob * pJob );
	void CheckHeadersStatus_by_stat ( TPumpJob * pJob );
	BOOL CheckHeadersStatus_by_listgroup ( TPumpJob * pJob );
	int  group_have(const CString& group, int iArticle, BOOL* pHave);
	int  DoHelpCommand ( TPumpJob* pOneJob );
	void DoConnect ( TPumpJob* pOneJob );
	int  AfterConnect (TPumpJobConnect* pJob);

	int  logon_style (int iWelcomeCode, CString & errorMsg,
		int iLogonStyle, LPCTSTR usrname, LPCTSTR password, void * pSPA);

	BOOL pump_lastrites();

	void parse_header_save_header(TPumpJobArticle * pJob, LPTSTR fullHeader);
	int  process_overview_data(TPumpJobOverview* pBaseJob, BOOL fCrossPost,
		void * pExtra, TArtHdrArray* pHdrArray);
	void overview_chopline(LPTSTR fatLine);
	BOOL overview_to_article (T2StringList & rList, CString & strFat,
		LPTSTR & pSubj, int & bufSz, TArticleHeader*& rpArtHdr, int & iArtInt);

	BOOL overview_make_art (LPTSTR line, LPTSTR pSubj, int bufSz,
		TArticleHeader*& rpArtHdr);

	void size_alloc (LPTSTR & pSubj, int & bufSz, int iNewSize);
	void realloc_error (int iOld, int iNew);
	void add_all_xpost (CStringArray& crossPost, int iLow, int iArtInt,
		CString & fat, CStringArray& vNG, LPTSTR & pSubj,
		int & bufSz, TArticleHeader * pHdr);
	void add_xpost_info (CString & fat, LPTSTR & pSubj, int & bufSz,
		TArticleHeader * pHdr);

	void catch_header (TNewsGroup*      pNG,
		TArtHdrArray*    pHdrArray,
		LPCTSTR          lszGroupName,
		int              iArtInt,
		TArticleHeader*& rpHdr);

	void DoGetNewsgroupList(TPumpJob* pOneJob);

	void PingMultiGroup (TPumpJob* pBaseJob);

	void freeHeaderMap(void);
	BOOL dl_body_start(TNewsGroup * pNG, TPumpJobArticle*& pJob);
	void dl_body_success (TNewsGroup * pNG, TPumpJobArticle*& pJob,
		TArticleText* pText);
	int  dl_body_fail_winsock (TPumpJobArticle * pJob, DWORD dwError,
		CString & strError, bool & fResubmit);
	int  dl_body_fail_user (TPumpJobArticle * pJob, DWORD dwError, CString & strError);
	int  dl_body_fail_nntp (TNewsGroup *      pNG,TPumpJobArticle * pJob,
		DWORD    dwError, CString & strError);

	int ping_articles (TRangeSet * prsPing, BOOL fExpirePhase, int iGroupID,
		int iExpireLow, int grandTotal,
		LPPOINT pptLogical, int iEstimate, BOOL fRedraw);
	int ping_articles_lines (TRangeSet * prsPing, BOOL fExpirePhase,
		int iGroupID, int iExpireLow, int grandTotal,
		LPPOINT pptLogical, BOOL fRedraw);

	int deliver_headers_to_scribe(TPumpJobOverview* pJob,
		TArtHdrArray*& pHdrArray,
		BOOL fGetBodies);

	// fBody - set to False if this is a newsgroup where the
	//         user chooses articles to download
	void SaveHeader_and_Request(const CString& groupName,
		LONG    groupID,
		TArticleHeader* pArtHdr,
		int artInt,
		BOOL fBody);

	void BatchSaveHeader_and_Request(const CString &        groupName,
		LONG                   groupID,
		TArtHdrArray *         pHdrArray,
		BOOL                   fBody,
		TPumpJob::EPumpJobDone eJobDone);

	void ExpireHeadersNotFound (TExpiryData*& pExpiryData);

	int         FreeConnection ();
	EErrorClass GetErrorClass (DWORD & dwError, CString & desc);
	void        PostDisconnectMsg (bool fIdleDisconnect, bool fReconnect,
		int  iSleepSecs = 0);

	int         resubmit_job ( TPumpJob * pJob );
	int         log_nntp_error (CString & ack);
	int         error_xref (LPCTSTR cmd);
	int         error_xhdr (const CString & cmd, TPumpJobOverview * pJob);

	int         get_xpost_info (
		TPumpJobOverview * pJob,
		int low,
		int high,
		void * psExtra,
		MPBAR_INFO * psBarInfo);

	int      xhdr_command (TPumpJobOverview * pJob,
		int low, int high, LPCTSTR token,
		EHeaderParts ePart, int & total,
		MPBAR_INFO * psBarInfo);

private:
	void kill_connection();
	void process_ng_line(CString& line, int offset, TArticleHeader* pHdr);
	int  run_exception(BOOL fSocket, TException* pe);
	BOOL cancel_job_in_queue (const CString& groupName, int artInt,
		TPumpJob::EPumpJobType eType);

	void set_alias_ptr ();
	int  connect_fail (TPumpJobConnect* pJob, BOOL fUserCancel,
		CString & errorMsg);

	int  CryptoLogin (void * pVoidSPA);
	int logon_style_error (TPumpJobConnect* pJob, CString & errorMsg);

private:
	HANDLE m_KillRequest;
	bool   m_fProcessJobs;     // set this to stop doing jobs

	static  TPumpTimeKeeper     m_timeKeeper;  // when to disconnect

	CTypedPtrList<CObList, TPumpJob*> m_Jobs;
	Pump_HeaderMap                    m_HeaderMap;
	HANDLE   m_hMutexJobBucket;   // protect m_Jobs (from self?)
	HANDLE   m_hEventJobReady;

	// BOOL   m_fPumpThreadRunning;

	// communication stuff
	CString          m_server;
	TNewsFeed*       m_pFeeed;
	CString          m_CurrentGroup;

	// the boss
	TNewsTasker *    m_pMasterTasker;

	int      m_instanceRet;

	BOOL m_fEmergencyPump;
	WORD m_wServerCap;            // server capabilities (kMustUseXHDR)
	BOOL m_fQuitNNTP;             // send QUIT line?

	CString    m_logFile;  // log GROUP commands
};
