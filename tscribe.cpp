/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tscribe.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:52:18  richard_wood
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

// tscribe.cpp : implementation file
//
//    As the newspump reads articles, TNewsScribe will write them
//    to the TNewsStore

#include "stdafx.h"
#include "resource.h"

#include "tscribe.h"
#include "article.h"
#include "globals.h"
#include "tmutex.h"

#include <memory>                  // auto_ptr
using namespace std;

#include "custmsg.h"               // WMU_INTERNAL_DISCONNECT
#include "newsgrp.h"
#include "pobject.h"
#include "names.h"
#include "rules.h"
#include "evtlog.h"
#include "tglobopt.h"
#include "ruleutil.h"               // EnabledRulesExist(), ...
#include "rgsys.h"
#include "enumjob.h"
#include "taglist.h"
#include "ngutil.h"
#include "tasker.h"                 // TNewsTasker
#include "rgswit.h"                 // m_fUsePGP
#include "topexcp.h"                // TopLevelException()
#include "nglist.h"                 // TNewsGroupUseLock
#include "server.h"                 // TNewsServer
#include "uipipe.h"
#include "utilrout.h"               // PostMainWndMsg
#include "tscoring.h"               // ScoreHeader(), ...
#include "tbozobin.h"               // CheckForBozo()

extern TGlobalOptions* gpGlobalOptions;
extern LONG gl_Scribe_Busy;

extern HWND ghwndMainFrame;
static int PumpAhead_Freeze_Bytes;
static int PumpAhead_Go_Bytes;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TNewsScribe

IMPLEMENT_DYNCREATE(TNewsScribe, CHumbleThread)

// ------------------------------------------------------------------------
TNewsScribe::TNewsScribe(void)
{
}

// ------------------------------------------------------------------------
TNewsScribe::TNewsScribe(HANDLE evt, TNewsServer* pCurrentServer)
: m_lastFlushTime(1975, 1, 1, 0, 0, 1), m_iFlushCount(0),
m_cpNewsServer(pCurrentServer)
{
	m_fScribeRunning = FALSE;
	// values in Kbytes
	gpGlobalOptions->GetRegSystem()->GetPumpHalt(&PumpAhead_Go_Bytes,
		&PumpAhead_Freeze_Bytes);
	PumpAhead_Go_Bytes *= 1024;

	MEMORYSTATUS  ms;

	GlobalMemoryStatus ( &ms );

	//PumpAhead_Freeze_Bytes *= 1024;
	PumpAhead_Freeze_Bytes = (int) (ms.dwTotalPhys * 0.9f);

	m_evtScribe = evt;
	m_instanceRet = 666;
	m_KillRequest = CreateEvent(NULL, TRUE, FALSE, NULL);   // Off by default
	m_Killed      = CreateEvent(NULL, TRUE, FALSE, NULL);   // Off by default

	m_hMutexScribeBucket = CreateMutex ( NULL,                // no security
		NULL,                // not owned
		"MutexForScribe");   // name

	m_hEventArticleReady = CreateEvent ( NULL,       // no security
		TRUE,       // we want manual reset
		FALSE,      // start out unsignaled
		NULL );     // no name
}

// -------------------------------------------------------------------------
TNewsScribe::~TNewsScribe()
{
	try
	{
		CloseHandle (m_hEventArticleReady);
		CloseHandle (m_hMutexScribeBucket);
		CloseHandle (m_KillRequest);
		CloseHandle (m_Killed);
		//TRACE0("Scribe: handles are gone\n");

		// clean up remaining articles in the Bucket
		int total = m_Bucket.GetSize();
		for (int i = 0; i < total; i++)
		{
			TScribePrep * pPrep = m_Bucket.GetAt(i);
			delete pPrep;
		}
		m_Bucket.RemoveAll();
	}
	catch(...)
	{
		// catch everything. uncaught exceptions in Destructors
		// will terminate the program!
	}
}

// -------------------------------------------------------------------------
BOOL TNewsScribe::InitInstance()
{
	try
	{
		m_bAutoDelete = 0;
		HANDLE rHandles[2];

		rHandles[0] = m_KillRequest;
		rHandles[1] = m_hEventArticleReady;
		DWORD         dwWaitRet;

		for (;;)
		{
			dwWaitRet = WaitForMultipleObjects(2, rHandles,
				FALSE,   // fWaitAll
				INFINITE);
			if (0 == dwWaitRet)
			{
				m_instanceRet = 3;
				break;
			}

			SingleStep();
		}

		// system code will call delete::
		return FALSE;
	}
	catch (TException *Except)
	{
		// I think this should notify the Pump and it should die also.
		Except->PushError(IDS_ERR_SCRIBE, kFatal);
		//pExcept->PushError("Gravity is unable to continue - press more for specifics.", kFatal);
		Except->Display ();
		TopLevelException (kThreadDatabase);
		TException *ex = new TException(*Except);
		Except->Delete();
		throw(ex);
		return FALSE;
	}
	catch (CException *pCExc)
	{
		pCExc->ReportError();
		pCExc->Delete();
		TopLevelException (kThreadDatabase);
		throw;
		return FALSE;
	}
	catch(...)
	{
		TopLevelException (kThreadDatabase);
		throw;
		return FALSE;
	}
}

void TNewsScribe::Delete()
{
	CHumbleThread::Delete();
	//TRACE0("scribe: ok, i'm dead\n");
	SetEvent ( m_Killed );     // Admit that I'm dead.
}

int TNewsScribe::ExitInstance()
{
	CHumbleThread::ExitInstance();
	return m_instanceRet;
}

DWORD TNewsScribe::ResumeScribe(void)
{
	DWORD ret;
	m_fScribeRunning =   ( (ret = ResumeThread()) == 1 );
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// TNewsScribe message handlers

void TNewsScribe::SingleStep (void)
{
	TScribePrep * pPrep = 0;

	pPrep = GetPrep();
	if (0 == pPrep)
		return;
	InterlockedIncrement ( &gl_Scribe_Busy );
	try
	{
		switch (pPrep->GetType())
		{
		case TScribePrep::kPrepHdr:
			{
				TPrepHeader * pHdr = pPrep->CastToPrepHeader();
				SaveHeader ( pHdr );
			}
			break;

		case TScribePrep::kPrepBatchHdr:
			{
				TPrepBatchHeader* pBatchHdr = pPrep->CastToPrepBatchHeader();
				SaveBatchHeader ( pBatchHdr );
			}
			break;

		case TScribePrep::kPrepBody:
			{
				PutbackPrep ( pPrep );  // re-add to queue
				pPrep = NULL;

				SaveBody ( NULL );      // do a bunch
				break;
			}

		default:
			ASSERT(0);
			break;
		}

	} // Try
	catch (TException *pE)
	{
		InterlockedDecrement ( &gl_Scribe_Busy );
		delete pPrep;
		throw pE;
	}

	delete pPrep;
	PostCheckQ();

	// let MFC destroy things
	CHumbleThread::OnIdle (1);

	InterlockedDecrement ( &gl_Scribe_Busy );
} // TNewsScribe::SingleStep

// --------------------------------------------------------------------------
void TNewsScribe::SaveHeader(TPrepHeader* pHdrJob)
{
	try
	{
		TArticleHeader* pArtHdr;
		int artInt = pHdrJob->GetArticleInt();
		pHdrJob->GetpHeader()->SetArticleNumber ( artInt );
		//TRACE2("saving hdr %s %d ", (LPCTSTR) pHdrJob->GetGroup(), artInt);

		BOOL fUseLock;
		int  iStatusAddRet = 0;
		TNewsGroup* pNG;
		TNewsGroupUseLock uLock(m_cpNewsServer, pHdrJob->GetGroup(), &fUseLock, pNG);

		if (FALSE == fUseLock)
			return;

		TAutoClose sClose(pNG);

		// Save article into the store

		pArtHdr = pHdrJob->GetpHeader();
		artInt = pArtHdr->GetArticleNumber();

		if (FALSE == pNG->HdrRangeHave( artInt ))
		{
			// add a status unit, before we evaluate rules
			iStatusAddRet = pNG->StatusAdd_New ( artInt );

			// run scoring
			ScoreHeader (pNG, pArtHdr);

			// bozo too
			CheckForBozo (pArtHdr, pNG);

			// apply relevant rules to this article's header
			int iResult = EvalRulesHaveHeader (pArtHdr, pNG);

			if (iResult & RULES_SAYS_DISCARD)
			{
				// we don't want this header after all.
				if (0 == iStatusAddRet)
					pNG->StatusDestroy ( artInt );
			}
			else
			{

				if (true)
				{
					TSyncWriteLock writeLock(pNG);

					pNG->AddHeader ( artInt, pHdrJob->RemovepHeader() );
					pNG->HdrRangeAdd (artInt);
				}

				if (iResult & RULES_SAYS_SAVE)
				{
					// need to fetch & save the body too
					extern TNewsTasker *gpTasker;
					gpTasker->RetrieveArticle (pNG->GetName(), pNG->m_GroupID,
						pArtHdr->GetNumber(), pArtHdr->GetLines());
				}

				if (iResult & RULES_SAYS_TAG)
					TagArticle (pNG, pArtHdr);
			}
		}

		pNG->ClearDirty();
	}
	catch (TException *rte)
	{
		rte->PushError(IDS_ERR_DB_SAVE_HDR, kFatal);
		TException *ex = new TException(*rte);
		rte->Delete();
		throw(ex);
	}
	catch (CException * pCE)
	{
		CString str; str.LoadString (IDS_ERR_DB_SAVE_HDR);
		convert_c_excp (str, pCE);
	}
} // SaveHeader

//////////////////////////////////////////////////////////////////////////
//  This object has 2 roles.  It associates an Header with a Body
//   and it tracks if the Body has been discarded
class TScribeBinder : public CObject
{
public:
	TScribeBinder(TPrepBody* pPrep)
		: m_pPrep(pPrep)
	{
		m_pHdr = 0;
	}

	~TScribeBinder()
	{
		delete m_pHdr;
		delete m_pPrep;
	}

	TArticleHeader* m_pHdr ;
	TPrepBody*      m_pPrep;
protected:
};

//////////////////////////////////////////////////////////////////////////
//  This object has 1 role.  The destructor will ensure a cleanup of the
//  contained objects, and free any Header objects - even with an
//  exception thrown
class TScribeBinderArray : public CObject
{
public:
	TScribeBinderArray() {}
	~TScribeBinderArray();

	CTypedPtrArray<CObArray, TScribeBinder*> m_vec;
};

TScribeBinderArray::~TScribeBinderArray()
{
	int tot = m_vec.GetSize();
	for (int i = 0; i < tot; ++i)
		delete m_vec[i];
	m_vec.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
void TNewsScribe::SaveBody (TPrepBody* pNULLBody)
{
	BOOL fUseLock;
	TNewsGroup * pNG = 0;
	try
	{
		int i;
		int tot = 0;
		int t0 = 0;
		TPrepBody* pBody;

		QueueBodyCount (t0);
		int nLoop = 60;
		while (nLoop-- > 0)
		{
			// sleep for one second
			Sleep (1000);

			// check again
			QueueBodyCount (tot);
			if (t0 == tot)
			{
				//TRACE0("Scribe ::  no more\n");
				break;
			}

			t0 = tot;
		}
		//TRACE1("Scribe ::  found batch of  %d\n", tot);

		if (0 == tot)
			return;
		CTypedPtrArray<CObArray, TScribePrep*> set;

		MultiGetPrep(tot, set);

		tot = set.GetSize();
		int articleInt;
		if (0 == tot)
			return;

		// these guys are all from the same group.
		CString groupName;
		for (i = 0; i < tot; ++i)
		{
			TScribePrep* pPrep = set.GetAt(i);
			pBody = pPrep->CastToPrepBody();
			if (0 == i)
				groupName = pBody->GetGroup();
			else
				ASSERT(groupName == pBody->GetGroup());
		}

		TNewsGroupUseLock useLock(m_cpNewsServer, groupName, &fUseLock, pNG);

		if (FALSE == fUseLock)
			return;

		TAutoClose sClose(pNG);

		// see if any are already in the database, set element to NULL
		discard_local_bodies (set, pNG, groupName);

		TScribeBinderArray bindset;

		for (i = tot - 1; i >= 0; --i)
		{
			TScribePrep* pPrep = set.GetAt(i);
			if (NULL == pPrep)
				set.RemoveAt(i);
			else
			{
				pBody = pPrep->CastToPrepBody();
				articleInt = pBody->GetArticleInt();

				// this is used as the hash key.
				pBody->GetBody()->SetNumber (articleInt);

				// analyze the full header
				pBody->GetBody()->ParseFullHeader();

				TScribeBinder* pBind = new TScribeBinder(pBody);
				bindset.m_vec.Add(pBind);
			}
		}

		if (0 == bindset.m_vec.GetSize())
		{
			return;
		}

		// read a bunch of headers
		// eval rules

		BOOL iEnabledRulesExist = EnabledRulesExist ();
		bool fDisconnectMsgSent = false;
		bool fSaveTags = false;

		for (i = 0; i < bindset.m_vec.GetSize(); ++i)
		{
			TScribeBinder* pOneBind = bindset.m_vec[i];
			pBody = pOneBind->m_pPrep;
			articleInt = pBody->GetArticleInt();

			BOOL fHeaderFound = FALSE;
			bool fDiskLow = false;

			TArticleHeader* pHdr = 0;
			int iResult = 0;

			// returns a copy of an article header
			iResult = ScribeEvalRules ( pNG, articleInt, pBody, pHdr );

			// I own this copy and the TScribeBinderArray dtor will delete it
			pOneBind->m_pHdr = pHdr;

			if (! (iEnabledRulesExist && (iResult & RULES_SAYS_DISCARD)))
				save_body_util ( pBody, pNG, articleInt, fDiskLow );

			if (fDiskLow)
			{
				CString logMsg; logMsg.LoadString (IDS_DISK_SPACE_LOW);

				// event log message
				gpEventLog->AddError (TEventEntry::kScribe, logMsg);

				// disconnect the pump
				if (!fDisconnectMsgSent && PostMainWndMsg (WMU_INTERNAL_DISCONNECT))
					fDisconnectMsgSent = true;

				// present log window to user
				PostMainWndMsg (WM_COMMAND, ID_VIEW_EVENTLOG);
			}
			else
			{
				// if user tags article.  User foolishly starts a priority job
				//    on the same article which caches it to the DB.  Then
				//    article would be erroneously TAGGED & LOCAL.
				if ((pBody->m_eFlags & kJobPartsTag) ||
					(pBody->m_eFlags & kJobPartsCache))
				{
					// remove from tag list
					TPersistentTags & rTags = m_cpNewsServer->GetPersistentTags();
					rTags.DeleteTagCancelJob (pNG->GetName(), pNG->m_GroupID,
						articleInt, FALSE);

					// just set a flag, so we don't save the TagList inside this loop
					fSaveTags = true;
				}

				// Post some asynchronous data to the UI thread
				gpUIPipe->NewRedrawItem ( pNG->m_GroupID, articleInt, TRUE );
			}
		} // for loop

		if (fSaveTags)
			m_cpNewsServer->SavePersistentTags ();

		// flush the newsgroup occassionally
		CTime     timeNow = CTime::GetCurrentTime();
		CTimeSpan span    = timeNow - m_lastFlushTime;
		if (span.GetMinutes() >= 10 || ++m_iFlushCount >= 10)
		{
			//TRACE0("Scribe Flushing newsgroup\n");
			m_lastFlushTime = timeNow;
			m_iFlushCount = 0;
			pNG -> FlushBodies();
		}

		//TRACE1("Saved %d bodies in a batch\n", tot);

	} // Try-clause
	catch (TException *rte)
	{
		rte->PushError (IDS_ERR_DB_SAVE_BODY, kFatal);
		TException *ex = new TException(*rte);
		rte->Delete();
		throw(ex);
	}
	catch (CException * pCE)
	{
		CString str; str.LoadString (IDS_ERR_DB_SAVE_BODY);
		convert_c_excp (str, pCE);
	}
}

/// xxx

//////////////////////////////////////////////////////////////////////////
//
//  Returns copy of an article header
int TNewsScribe::ScribeEvalRules (TNewsGroup * pNG,
								  int  articleInt,
								  TPrepBody* pPrepBody,
								  TArticleHeader*& rpRetHdr)
{
	try
	{
		ASSERT(pPrepBody);
		ASSERT(pPrepBody->GetBody());
		ASSERT(articleInt);
		ASSERT(pNG);

		int  iResult = 0;
		BOOL fHeaderFound = FALSE;
		TArticleHeader *pGotHdr;
		rpRetHdr = 0;
		// we have to be careful about ownership. Both cases make a copy
		// which is returned to the caller
		switch (UtilGetStorageOption(pNG))
		{
		case TNewsGroup::kNothing:
			{
				ASSERT((pPrepBody->m_eFlags & kJobPartsTag) ||
					(pPrepBody->m_eFlags & kJobPartsCache));
				if ((fHeaderFound = pNG->GetpHeader (articleInt, pGotHdr)) != 0)
					rpRetHdr = new TArticleHeader(*pGotHdr);
				break;
			}
		case TNewsGroup::kHeadersOnly:
		case TNewsGroup::kStoreBodies:
			{
				pGotHdr = (TArticleHeader*) pNG->GetHeader( articleInt );
				fHeaderFound = (pGotHdr != 0);
				if (fHeaderFound)
					rpRetHdr = new TArticleHeader(*pGotHdr);
				break;
			}
		default:
			ASSERT(0);
			break;
		}

		if (fHeaderFound)
		{
			if (EnabledRulesExist ()) {
				// evaluate rules that require seeing the body
				iResult = EvalRulesHaveBody(rpRetHdr, pPrepBody->GetBody(), pNG);
				if (!(iResult & RULES_SAYS_DISCARD))
					EvalRulesHaveBody_Committed(rpRetHdr, pPrepBody->GetBody(), pNG);
			}

			// let the scoring system evaluate the body
			ScoreBody (rpRetHdr, pPrepBody -> GetBody (), pNG);
		}
		return iResult;
	}
	catch (TException *rte)
	{
		CString msg; msg.Format (IDS_ERR_DB_LINE, __LINE__);
		rte->PushError (msg, kFatal);
		TException *ex = new TException(*rte);
		rte->Delete();
		throw(ex);
		return 0;
	}
	catch (CException * pCE)
	{
		CString str; str.LoadString (IDS_ERR_DB_RULES);
		convert_c_excp (str, pCE);
		return 0;
	}
}

//-------------------------------------------------------------------------
void TNewsScribe::discard_local_bodies(
									   CTypedPtrArray<CObArray, TScribePrep*> & aSet,
									   TNewsGroup *                             pNG,
									   const CString &                          groupName
									   )
{
	try
	{
		int tot = aSet.GetSize();
		for (int i = 0; i < tot; ++i)
		{
			TScribePrep* pPrep = aSet.GetAt(i);
			TPrepBody* pBody = pPrep->CastToPrepBody();

			int articleInt = pBody->GetArticleInt();

			if (pNG->TextRangeHave(articleInt))
			{
				// this guy is done
				delete pPrep;
				aSet.SetAt(i, NULL);
			}
		}
	}
	catch (TException *rte)
	{
		CString str; str.LoadString (IDS_ERR_DB_DISCARDLOCAL);
		rte->PushError (str, kFatal);
		TException *ex = new TException(*rte);
		rte->Delete();
		throw(ex);
	}
	catch (CException * pCE)
	{
		CString str; str.LoadString (IDS_ERR_DB_DISCARDLOCAL);
		convert_c_excp (str, pCE);
	}
}

// 4/6/98 -- was having trouble with a code-generation problem (in the
// release version), so optimizations are off for this function
#pragma optimize("",off)

//-------------------------------------------------------------------------
void TNewsScribe::save_body_util(
								 TPrepBody *  pBody,
								 TNewsGroup * pNG,
								 int          artInt,
								 bool &       fDiskLow
								 )
{
	try
	{
		DWORDLONG quadSize;
		fDiskLow = false;

		if (gpGlobalOptions->GetRegSwitch()->GetPausingLowSpace() &&
			(0 == pNG->GetFreeDiskSpace (quadSize))  &&
			(quadSize < pBody->GetBody()->SizeHint() + 50000))
		{
			fDiskLow = true;
			return;
		}

		if (true)
		{
			TSyncWriteLock writeLock(pNG);

			pNG->SaveBody (artInt, pBody->GetBody());
			pNG->TextRangeAdd (artInt);
		}

		// call to SaveSubscribedGroups to save out the pNG?

		// $$ this is just irritating
		// CString msg;
		// CString str; str.LoadString (IDS_ERR_DB_SAVED);
		// msg.Format (str, artInt, LPCTSTR(pNG->GetName()));
		// gpEventLog->AddInfo (TEventEntry::kScribe, msg);
	}
	catch (TException *rte)
	{
		CString emsg; emsg.Format (IDS_ERR_DB_LINE, __LINE__);
		rte->PushError (emsg, kFatal);
		TException *ex = new TException(*rte);
		rte->Delete();
		throw(ex);
	}
	catch (CException * pCE)
	{
		CString str; str.LoadString (IDS_ERR_DB_SAVEBODYUTIL);
		convert_c_excp(str, pCE);
	}
}

#pragma optimize("",on)

void TNewsScribe::SaveFullHeader(TPrepFullHeader* pFullHdr)
{
}

// ------------------------------------------------------------------------
void TNewsScribe::AddPrep(TScribePrep * pPrep)
{
	TMutex mgr (m_hMutexScribeBucket);
	ResetEvent (m_hEventArticleReady);
	m_Bucket.Add ( pPrep );

	int sz = m_Bucket.GetSize();

	{
		TScribePrep* pqPrep;
		int tot = 0;
		for (int j = 0; j < sz; ++j)
		{
			pqPrep = m_Bucket.GetAt(j);
			tot += pqPrep->GetByteCount();
		}
		if (tot >= PumpAhead_Freeze_Bytes)
		{
			//TRACE0("           halt the pump\n");

			CString msg = "Pausing the download thread";
			gpEventLog->AddInfo (TEventEntry::kScribe, msg);

			ResetEvent ( m_evtScribe );
		}
	}

	SetEvent (m_hEventArticleReady);
}

// ------------------------------------------------------------------------
int TNewsScribe::JobCount(void)
{
	TMutex mgr (m_hMutexScribeBucket);
	return m_Bucket.GetSize();
}

// ------------------------------------------------------------------------
TScribePrep * TNewsScribe::GetPrep(void)
{
	TMutex mgr (m_hMutexScribeBucket);
	int sz = m_Bucket.GetSize();
	if (0 == sz)
	{
		ResetEvent(m_hEventArticleReady);
		return NULL;
	}
	if (1 == sz)
		ResetEvent(m_hEventArticleReady);
	TScribePrep* pPrep = m_Bucket.GetAt(0);
	m_Bucket.RemoveAt(0);
	return pPrep;
}

// ------------------------------------------------------------------------
void TNewsScribe::PutbackPrep(TScribePrep* pPrep)
{
	TMutex mgr (m_hMutexScribeBucket);
	m_Bucket.InsertAt(0, pPrep);
}

// ------------------------------------------------------------------------
void TNewsScribe::QueueBodyCount (int& n)
{
	TMutex mgr (m_hMutexScribeBucket);
	TScribePrep* pPrep;
	n = 0;
	CString groupName;
	int sz = m_Bucket.GetSize();
	for (int i = 0; i < sz; ++i)
	{
		pPrep = m_Bucket.GetAt(i);
		if (TScribePrep::kPrepBody != pPrep->GetType())
			return ;
		TPrepBody* pBody = pPrep->CastToPrepBody();

		// pass back only bodies in the same newsgroup
		if (groupName.IsEmpty())
			groupName = pBody->GetGroup();
		else if (groupName != pBody->GetGroup())
			return;
		++n;
	}
}

// ------------------------------------------------------------------------
// all jobs extracted will be from the same newsgroup
void TNewsScribe::MultiGetPrep (
								int n,
								CTypedPtrArray<CObArray, TScribePrep*>& aSet)
{
	TMutex mgr(m_hMutexScribeBucket);
	int sz = m_Bucket.GetSize();
	CString groupName;
	for (int i = 0; (i < n) && (i < sz); ++i)
	{
		TScribePrep* pPrep = m_Bucket.GetAt(0);
		TPrepBody* pBody = pPrep->CastToPrepBody();

		if (groupName.IsEmpty())
			groupName = pBody->GetGroup();
		else if (groupName != pBody->GetGroup())
			break;
		aSet.Add(pPrep);
		m_Bucket.RemoveAt(0);
	}
	sz = m_Bucket.GetSize();
	if (0 == sz)
		ResetEvent(m_hEventArticleReady);
}

// ------------------------------------------------------------------------
void TNewsScribe::PostCheckQ (void)
{
	TMutex mgr (m_hMutexScribeBucket);
	int sz = m_Bucket.GetSize();
	int totBytes = 0;
	for (int j = 0; j < sz-1; ++j)
	{
		TScribePrep* pqPrep = m_Bucket.GetAt(j);
		totBytes += pqPrep->GetByteCount();
	}
	if (totBytes < PumpAhead_Go_Bytes)
	{
		// change event - "I can handle more"
		//TRACE0("\n\n        release the pump\n");

		if (WAIT_TIMEOUT == WaitForSingleObject (m_evtScribe, 0))
		{
			// if we are transitioning
			CString msg = "Resuming the download thread";
			gpEventLog->AddInfo (TEventEntry::kScribe, msg);
		}

		SetEvent (m_evtScribe);
	}
}

// ------------------------------------------------------------------------
// The user is unsubscribing from a newsgroup, kill
// the pending jobs
int TNewsScribe::CancelNewsgroup (LPCTSTR groupName)
{
	TMutex mgr(m_hMutexScribeBucket);
	int sz = m_Bucket.GetSize();
	int killCount = 0;

	for (int i = sz - 1; i >= 0; --i)
	{
		TScribePrep* pPrep = m_Bucket.GetAt(i);
		const CString& rName = pPrep->GetGroup();
		if (0 == rName.CompareNoCase (groupName))
		{
			m_Bucket.RemoveAt(i);
			delete pPrep;
			++killCount;
		}
	}
	return killCount;
}

/////////////////////////////////////////////////////////////////
// SaveBatchHeader
//   - run rules
//   - physically save the header
//   - add to header RangeSet and create a Status for it
//
//
void TNewsScribe::SaveBatchHeader ( TPrepBatchHeader* pBatchHdr )
{
	const int iExceptionMsgSize = 200;
	TCHAR * pExceptionMsg = new TCHAR[iExceptionMsgSize];
	auto_ptr<TCHAR> sMsgDeleter(pExceptionMsg);

	CString strExceptionMsg('#', 256);

	int arraySize = pBatchHdr->m_pHdrArray->GetSize();
	TArticleHeader* pArtHdr = NULL;
	int  artInt = 0;

	BOOL fUseLock;
	TNewsGroup * pNG;
	int iGroupID = 0;
	TNewsGroupUseLock useLock(m_cpNewsServer, pBatchHdr->GetGroup(), &fUseLock, pNG);

	if (!fUseLock)
		return;

	try
	{
		pNG->Open();
	}
	catch (TException *rTExc)
	{
		CString str; str.LoadString (IDS_ERR_SAVEBATCHHEADER);
		rTExc->PushError (str, kFatal);
		TException *ex = new TException(*rTExc);
		rTExc->Delete();
		throw(ex);
		return;
	}
	catch (CException* pCExc)
	{
		strExceptionMsg.Format(IDS_ERR_SCRIBE_LINE, __LINE__);
		BOOL fMsg = pCExc->GetErrorMessage(pExceptionMsg, iExceptionMsgSize);

		pCExc->Delete();
		if (fMsg)
			strExceptionMsg += pExceptionMsg;
		throw(new TException(strExceptionMsg, kFatal));
		return;
	}
	catch(...)
	{
		strExceptionMsg.Format(IDS_ERR_SCRIBE_LINE, __LINE__);
		throw(new TException(strExceptionMsg, kFatal));
		return;
	}

	try
	{
		int iHeadersAdded = 0;
		{ // start good-stuff

			// Save group of headers into the store
			for (int k = 0; k < arraySize; ++k)
			{
				pArtHdr = pBatchHdr->m_pHdrArray->GetAt(k);
				artInt = pArtHdr->GetArticleNumber();

				if (FALSE == pNG->HdrRangeHave ( artInt ))
				{
					// add status unit before we evaluate rules
					int iStatusAddRet = pNG->StatusAdd_New ( artInt );

					// run scoring
					ScoreHeader (pNG, pArtHdr);

					// bozo too
					CheckForBozo (pArtHdr, pNG);

					// apply relevant rules to this article's header
					int iResult = EvalRulesHaveHeader (pArtHdr, pNG);

					if (iResult & RULES_SAYS_DISCARD)
					{
						// oops. we don't want this header after all
						if (0 == iStatusAddRet)
							pNG->StatusDestroy ( artInt );
					}
					else
					{
						// optimization : some thread has to bite the bullet
						//    and do this chore. results are cached
						if (true)
						{
							CString phrase, address;
							pArtHdr->ParseFrom (phrase, address);
						}

						// transfer ownership of pointer
						pNG->AddHeader( artInt, pArtHdr );
						pBatchHdr->m_pHdrArray->SetAt(k, NULL);

						// we already made a status unit for it
						pNG->HdrRangeAdd (artInt);
						iHeadersAdded++;

						if (iResult & RULES_SAYS_SAVE)
						{
							// need to fetch & save the body too
							extern TNewsTasker *gpTasker;
							gpTasker -> RetrieveArticle (pNG -> GetName (),
								pNG -> m_GroupID, pArtHdr -> GetNumber (),
								pArtHdr -> GetLines ());
						}

						if (iResult & RULES_SAYS_TAG)
							TagArticle (pNG, pArtHdr);

					} // not discarded by Rules
				}
			} // loop
		} // end good-stuff

		pNG->ClearDirty();

		pNG->SaveHeaders();

		// since we saved the header-objects, we should write out the HdrRangeHave
		//   for this one newsgroup
		//  -amc 5-6-96
		m_cpNewsServer->SaveIndividualGroup ( pNG );

		//TRACE2("Saved %d headers for %s\n", iHeadersAdded,
		//	LPCTSTR(pBatchHdr->GetGroup()));

		// the newsview should update the [New] count
		iGroupID = pNG->m_GroupID;
	}
	catch (TException  *rTExc)
	{
		pNG->Close();
		rTExc->PushError (IDS_ERR_SAVEBATCHHEADER_FATAL, kFatal);
		TException *ex = new TException(*rTExc);
		rTExc->Delete();
		throw(ex);
		return;
	}
	catch (CException * pCE)
	{
		pNG->Close();
		CString str; str.LoadString (IDS_ERR_SAVEBATCHHEADER_FATAL);
		convert_c_excp (str, pCE);
		return;
	}
	catch(...)
	{
		pNG->Close();
		throw;
		return;
	}

	try
	{
		// our cleanup

		pNG->Close();

		if (iGroupID > 0)
		{
			//TRACE0("end SaveBatchHeader - POST\n");

			PostMainWndMsg (WMC_DISPLAY_ARTCOUNT, iGroupID);

			// 3-24-96 If this is a user action, then
			//              I want to open and show the new articles in the current
			//              group now that the headers are done
			//         If this is a program action, pass the message
			//

			PostMainWndMsg (WMU_NGROUP_HDRS_DONE,
				iGroupID,
				TScribePrep::kDoneDisplay == pBatchHdr->GetDisposition());

			// send back an 'event' for the VCR window
			PostMainWndMsg (WMU_VCR_GROUPDONE);

			//TRACE0("end SaveBatchHeader\n");
		}
	}
	catch (TException *rTE)
	{
		CString str; str.Format(IDS_ERR_SCRIBE_LINE, __LINE__);
		rTE->PushError (str, kFatal);
		TException *ex = new TException(*rTE);
		rTE->Delete();
		throw(ex);
	}
	catch (CException * pCE)
	{
		CString str; str.Format (IDS_ERR_SCRIBE_LINE, __LINE__);
		convert_c_excp (str, pCE);
	}
}

// ------------------------------------------------------------------------
// Get description from the CException and throw a TException
void TNewsScribe::convert_c_excp(LPCTSTR descOuter, CException * pCExcp)
{
	// this c-function is in mpexcept.cpp

	ConvertCExceptionAndThrow ( pCExcp, descOuter );
}

// ------------------------------------------------------------------------
// Constructor
TPrepHeader::TPrepHeader(LPCTSTR newsGroup,
						 TArticleHeader * pArtHeader,
						 int artNum)
						 : TScribePrep(newsGroup, artNum)
{
	m_pArtHeader = pArtHeader;
	ASSERT(pArtHeader);
}

// ------------------------------------------------------------------------
// Destructor
TPrepHeader::~TPrepHeader(void)
{
	delete m_pArtHeader;
}

int TPrepHeader::GetByteCount()
{
	return _tcslen( m_pArtHeader->GetSubject() );
}

// ------------------------------------------------------------------------
// Constructor
TPrepBody::TPrepBody(const CString& newsGroup,
					 TArticleText * pText,
					 int artNum, EPumpJobFlags eFlags)
					 : TScribePrep(newsGroup, artNum)
{
	m_pText = pText;
	m_eFlags = eFlags;
}

// ------------------------------------------------------------------------
TPrepBody::~TPrepBody(void)
{
	delete m_pText;
}

// ------------------------------------------------------------------------
int TPrepBody::GetByteCount()
{
	return m_pText->GetBody().GetLength();
}

// ------------------------------------------------------------------------
//
TPrepBatchHeader::TPrepBatchHeader(LPCTSTR groupName,
								   LONG               groupID,
								   TArtHdrArray *     pHdrArray,
								   TScribePrep::EPrepDone eDone)

								   : TScribePrep(groupName, 0), m_eDone(eDone)
{
	m_groupID = groupID;
	m_pHdrArray = pHdrArray;
}

// ------------------------------------------------------------------------
//
TPrepBatchHeader::~TPrepBatchHeader(void)
{
	int total = m_pHdrArray->GetSize();
	for (--total; total >= 0; --total)
	{
		delete m_pHdrArray->GetAt(total);
	}
	delete m_pHdrArray;
}

// ------------------------------------------------------------------------
int TPrepBatchHeader::GetByteCount()
{
	return 100 * m_pHdrArray->GetSize();
}

