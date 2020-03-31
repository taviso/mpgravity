/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: outbox.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:39  richard_wood
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
#include "resource.h"
#include "newsdb.h"
#include "outbox.h"
#include "server.h"              // TNewsServer
#include "servcp.h"
#include "ngstat.h"
#include "critsect.h"
#include "rgswit.h"              // TRegSwitch
#include "tglobopt.h"            // gpGlobalOptions
#include "custmsg.h"             // WMU_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsGroupStatus* gpNewsGroupStatus;

///////////////////////////////////////////////////////////////////////////
TMetaOutbox::TMetaOutbox(TNewsServer * pServer)
{
	m_pRep = new TOutboxInternal;

	// base class func
	SetServerName (pServer->GetNewsServerName());
}

///////////////////////////////////////////////////////////////////////////
TMetaOutbox::~TMetaOutbox()
{
	delete m_pRep;
}

///////////////////////////////////////////////////////////////////////////
void TMetaOutbox::Serialize(CArchive & ar)
{
	// Get an read lock (others cannot change contents while I write)
	TSyncReadLock readLock(this);

	if (ar.IsStoring())
	{
		if (IsOpen())
		{
			// it's open  - hold it open while we save headers
			Open ();
			SaveHeaders ();
			Close ();
		}

		m_pRep->Serialize ( ar );
	}
	else
	{
		// LoadHeaders();
		// Stati
		m_pRep->Serialize ( ar );
	}
}

///////////////////////////////////////////////////////////////////////////
//  Takes possession of pHdr!!
//
void TMetaOutbox::SaveMessage (TPersist822Header * pHdr, TPersist822Text *pBody)
{
	int artInt = pHdr->GetNumber();

	TAutoClose sCloser(this);   // get and release  'open' critical section ...

	if (true)
	{
		TSyncWriteLock writeLok(this);   // ... before we get this mutex

		// note : the tasker sometimes gets the critsection and then
		// gets the read lock, so follow suit.

		AddHeader ( artInt, pHdr );
		SaveBody  ( artInt, pBody );
	}

	// make a status for this outbox message
	m_pRep->AddItem ( artInt,   (WORD)  TStatusUnit::kOut );
}

/////////////////////////////////////////////////////////////////////////////
// Have we done our reset?  do this once for each app Startup.
BYTE  TOutbox::m_byResetForInstance;

/////////////////////////////////////////////////////////////////////////////
// constructor
TOutbox::TOutbox(TNewsServer * pServer)
: TMetaOutbox(pServer)
{
	m_name = "outbox";   // this is here so virtual GetName works
}

/////////////////////////////////////////////////////////////////////////////
TOutbox::~TOutbox()
{
	// empty
}

/////////////////////////////////////////////////////////////////////////////
// Open - Overrides TNewsGroupDB Open call so that we can call
//        ResetInterruptedMessages...
/////////////////////////////////////////////////////////////////////////////

void TOutbox::Open()
{
	TNewsGroupDB::Open ();

	// calls Serialize eventually
	// TNewsDB::LoadFromDatabase(this, m_pServer->GetServerDatabasePath(), OUTBOX_FILE);

	InitOnce ();
}

void TOutbox::InitOnce ()
{
	// reset once each time the app starts up. No more than that!
	if (0 == m_byResetForInstance)
	{
		m_byResetForInstance = 1;
		TNewsGroupDB::Open ();
		try
		{
			ResetInterruptedMessages ();
			Verify ();  // needs headers loaded
		}
		catch(...)
		{
			TNewsGroupDB::Close ();
			throw;
		}
		TNewsGroupDB::Close ();
	}
}

void TOutbox::ResetInterruptedMessages ()
{
	m_pRep->ResetInterruptedMessages();
}

///////////////////////////////////////////////////////////////////////////
//  Verify the headers versus the status units.  Needs the headers loaded
//  4-26-96  amc  Created
void TOutbox::Verify ()
{
	int iRemoved = 0;
	TSyncWriteLock writeLock(this);

	// Verify self as we startup
	CDWordArray vArtInts;
	m_pRep->GetAll ( &vArtInts );

	// There should be a body and a header for each of these status units
	int tot = vArtInts.GetSize();
	for (int i = 0; i < tot; ++i)
	{
		//TRACE("TOutbox::Verify : verifying article %d\n", vArtInts[i]);
		LONG dummy;
		int num = vArtInts[i];
		TPersist822Header * pHdr = GetHeader ( num );
		//TRACE("\t\t\theader %s\n", pHdr ? "exists" : "does not exist");
		BOOL fBodyExist = CheckBodyExist ( num, dummy );
		//TRACE("\t\t\tbody %s\n", fBodyExist ? "exists" : "does not exist");

		// Sortof draconian
		if (!pHdr || !fBodyExist)
		{
			//TRACE("\tremoving article %d\n", vArtInts[i]);
			m_pRep->RemoveItem ( num );
			if (pHdr)
				PurgeHeader ( num );
			if (fBodyExist)
				PurgeBody ( num );
			++iRemoved;
		}
	}
}

// ---------------------------------------------------------------------
// wrapper function to reinforce strong type check
void TOutbox::SaveEmail (TEmailHeader *& pHdr, TEmailText * pText)

{
	SaveMessage ( pHdr, pText );
	pHdr = 0;
}

// ---------------------------------------------------------------------
// wrapper function to reinforce strong type checking
void TOutbox::SaveArticle (TArticleHeader *& pHdr, TArticleText * pText)

{
	SaveMessage ( pHdr, pText );
	pHdr = 0;
}

BOOL TOutbox::PeekArticle(WORD status, int* pArtInt)
{
	InitOnce ();
	return m_pRep->PeekArticle(status, pArtInt);
}

BOOL TOutbox::PeekEmails(WORD status, CDWordArray* pIntArray)
{
	InitOnce ();
	return m_pRep->PeekEmails (status, pIntArray);
}

/////////////////////////////////////////////////////////////////////
// return a count of waiting messages, and send errors
//
// Note -  We can count the status units, without loading the headers
void TOutbox::CountWaitingAndSending(int& wait, int& sending, int& errors)
{
	InitOnce ();
	m_pRep->CountWaitingAndSending ( wait, sending, errors );
}

////////////////////////////////////////////////////////////////////////////
void TOutbox::DoPurging ()
{
	BOOL fPurgeOutbox;
	int  iPurgeOutboxDays;

	{
		TServerCountedPtr cpServer( GetParentServer () );
		fPurgeOutbox     = cpServer->GetPurgeOutbox ();
		iPurgeOutboxDays = cpServer->GetPurgeOutboxDays ();
	}

	if (FALSE == fPurgeOutbox)
		return;

	CTimeSpan sTimeSpan (iPurgeOutboxDays, 0, 0, 0 /* hours, minutes, seconds */);
	CTime sPurgeCutoff = CTime::GetCurrentTime () - sTimeSpan;

	TAutoClose sAutoCloser(this);
	WriteLock ();

	THeaderIterator it (this);
	TPersist822Header *pHeader;
	while (it.Next (pHeader))
	{

		const CTime sArticleTime = pHeader -> GetTime ();
		if (sArticleTime > sPurgeCutoff)
			continue;

		StatusUnitI stat;
		int iArticle = pHeader -> GetNumber ();
		(*this) -> GetStatus (iArticle, &stat);
		if (! (stat.m_status & TStatusUnit::kSent))
			continue;

		PurgeHeader (iArticle);
		PurgeBody (iArticle);
		(*this) -> RemoveItem (iArticle);
	}

	UnlockWrite ();

	SaveHeaders ();
	SaveBodyMetaData ();
}

////////////////////////////////////////////////////////////////////////////
TOutboxInternal::TOutboxInternal()
{
}

TOutboxInternal::~TOutboxInternal()
{
}

void TOutboxInternal::Serialize(CArchive& ar)
{
	// always do base class first
	PObject::Serialize( ar );

	m_StatusSearch.Serialize (ar);
}

void TOutboxInternal::AddItem(int artInt, WORD status, time_t readTime)
{
	m_StatusSearch.AddItem(artInt, status, readTime);
}

void TOutboxInternal::RemoveItem(int artInt)
{
	m_StatusSearch.RemoveItem(artInt);
}

BOOL TOutboxInternal::PeekArticle(WORD status, int* pArtInt)
{
	return peek_util(status, TRUE, pArtInt);
}

BOOL TOutboxInternal::PeekEmails(WORD status, CDWordArray* pIntArray)
{
	BOOL fFoundOne = FALSE;
	TEnterCSection mgr (m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	int sz = m_StatusSearch.GetSize();
	for (int i = 0; i < sz; ++i, ++pU)
	{
		if (TNewsGroupStatus::IsEmail(pU->m_articleInt))
		{
			if (status & pU->m_status)
			{
				//TRACE("TOutboxInternal::PeekEmails : Found an email, ID %d\n",
				//	pU->m_articleInt);
				fFoundOne = TRUE;
				pIntArray->Add (pU->m_articleInt);
			}
		}
	}
	return fFoundOne;
}

BOOL TOutboxInternal::peek_util(WORD status, BOOL fCountArticles, int* pArtInt)
{
	BOOL fFoundOne = FALSE;
	*pArtInt = -1;
	TEnterCSection mgr (m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	int sz = m_StatusSearch.GetSize();
	for (int i = 0; i < sz; ++i, ++pU)
	{
		if (fCountArticles && gpNewsGroupStatus->IsArticle(pU->m_articleInt))
		{
			if (status & pU->m_status)
			{
				fFoundOne = TRUE;
				*pArtInt = pU->m_articleInt;
				return TRUE;
			}
		}
	}
	return fFoundOne;
}

// created 5-11-96
void TOutboxInternal::CountWaitingAndSending(int& wait, int& sending, int& errors)
{
	wait = sending = errors = 0;
	TEnterCSection mgr (m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	int sz = m_StatusSearch.GetSize();
	for (int i = 0; i < sz; ++i, ++pU)
	{
		if (pU->m_status & TStatusUnit::kSendErr)
			++errors;

		// if it hasn't been sent successfully, and it hasn't generated an
		// error, then it's still waiting
		if (pU->m_status & TStatusUnit::kOut)
		{
			ASSERT(!(pU->m_status & TStatusUnit::kSent));
			ASSERT(!(pU->m_status & TStatusUnit::kSendErr));
			++wait;
		}
		else if (pU->m_status & TStatusUnit::kSending)
		{
			sending++;
		}

	}
}

////////////////////////////////////////////////////////////////////////////
// return 0 for success, -1 for not found, +1 for bitset already
//
int TOutboxInternal::Mark(int artInt, WORD wStatus)
{
	TEnterCSection mgr(m_StatusSearch.GetpCriticalSection());

	int  idx;
	WORD status;
	if (m_StatusSearch.Find(artInt, idx))
	{
		const StatusUnitI* pU = m_StatusSearch.GetData(idx);
		time_t timeSent = pU->m_timeRead;
		status = pU->m_status;

		// is it set already?
		if (status & wStatus)
			return 1;

		if (wStatus & TStatusUnit::kSending)
		{
			if ((status & TStatusUnit::kSent)    ||
				(status & TStatusUnit::kSendErr) ||
				(status & TStatusUnit::kSending))
				return 1;

			status &= ~(TStatusUnit::kOut | TStatusUnit::kSendErr | TStatusUnit::kSent);
			status |= TStatusUnit::kSending;
		}
		else if (wStatus & TStatusUnit::kSent)
		{
			status &= ~(TStatusUnit::kOut | TStatusUnit::kSending | TStatusUnit::kSendErr );
			status |= TStatusUnit::kSent;
			timeSent = (CTime::GetCurrentTime()).GetTime();
		}
		else if (wStatus & TStatusUnit::kOut)
		{
			if (status & TStatusUnit::kSent)
				return 1;
			status &= ~(TStatusUnit::kSending | TStatusUnit::kSent | TStatusUnit::kSendErr );
			status |= TStatusUnit::kOut;
		}
		else if (wStatus & TStatusUnit::kSendErr)
		{
			status &= ~( TStatusUnit::kOut | TStatusUnit::kSending | TStatusUnit::kSent);
			status |= TStatusUnit::kSendErr;
		}
		else if (wStatus & TStatusUnit::kDraft)
		{
			status &= ~(TStatusUnit::kOut | TStatusUnit::kSending |
				TStatusUnit::kSent | TStatusUnit::kSendErr);
			status |= TStatusUnit::kDraft;
		}
		else
		{
			// unknown
			ASSERT(0);
		}
		m_StatusSearch.SetItemByIndex(idx, status, timeSent);
		return 0;
	}
	ASSERT(0);
	return -1;
}

BOOL TOutboxInternal::ResetInterruptedMessages(void)
{
	TEnterCSection mgr(m_StatusSearch.GetpCriticalSection());

	int sz = m_StatusSearch.GetSize();
	const StatusUnitI* pU = m_StatusSearch.GetData();
	for (int i = 0; i < sz; ++i, ++pU)
	{
		if ((pU->m_status & TStatusUnit::kSending) &&
			!(pU->m_status & TStatusUnit::kSent))
			m_StatusSearch.SetItemByIndex(i, TStatusUnit::kOut, pU->m_timeRead);
	}
	return TRUE;
}

BOOL TOutboxInternal::GetStatus (int artInt, StatusUnitI *pStatus)
{
	TEnterCSection mgr(m_StatusSearch.GetpCriticalSection());
	int index;
	int found;
	found = m_StatusSearch.Find (artInt, index);
	if (found)
		*pStatus = *(m_StatusSearch.GetData(index));
	return found;
}

void TOutboxInternal::GetAll (CDWordArray * pIntArray)
{
	TEnterCSection mgr(m_StatusSearch.GetpCriticalSection());

	int sz = m_StatusSearch.GetSize();
	const StatusUnitI* pU = m_StatusSearch.GetData();

	for (int i = 0; i < sz; ++i, ++pU)
		pIntArray->Add (pU->m_articleInt);
}

// ------------------------------------------------------------------------
// Reset article from kSendError back to waiting
BOOL TOutboxInternal::RetrySend (int artInt)
{
	TEnterCSection mgr(m_StatusSearch.GetpCriticalSection());

	int index;
	int fFound = m_StatusSearch.Find (artInt, index);

	if (!fFound)
		return FALSE;

	const StatusUnitI * pStatus = m_StatusSearch.GetData (index);
	WORD wStatus = pStatus->m_status;

	// error bit must be set
	if (!(wStatus & TStatusUnit::kSendErr))
		return FALSE;

	wStatus &= ~(TStatusUnit::kSending | TStatusUnit::kSent | TStatusUnit::kSendErr);
	wStatus |= TStatusUnit::kOut;

	m_StatusSearch.SetItemByIndex (index, wStatus, pStatus->m_timeRead);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// SavingNewCopy -- called when the user saves a draft or sends an article/
// mail message.  This gives the outbox a chance to destroy the original
// copy of the article/mail message
void TDraftMsgs::SavingNewCopy (LONG lArticle)
{
	NotEditingArticle (lArticle);

	// look in the outbox and delete any articles with the same article number
	{
		TAutoClose  sCloser(this);

		TPersist822Header * pHeader = GetHeader (lArticle);

		if (pHeader)
		{
			WriteLock ();

			PurgeHeader (lArticle);
			PurgeBody (lArticle);

			(*this) -> RemoveItem (lArticle);
			UnlockWrite ();
		}

		SaveHeaders ();
		SaveBodyMetaData ();
	}

	// refresh draft box display
	AfxGetMainWnd () -> SendMessage (WMU_REFRESH_DRAFT);
}

void TDraftMsgs::SaveDraftMessage (TPersist822Header * pHdr, TPersist822Text *pBody)
{
	int artInt = pHdr->GetNumber();

	TAutoClose sCloser(this);

	if (true)
	{
		TSyncWriteLock writeLok(this);   // ... before we get this mutex

		// note : the tasker sometimes gets the critsection and then
		// gets the read lock, so follow suit.

		AddHeader ( artInt, pHdr );
		SaveBody  ( artInt, pBody );
	}

	// make a status for this draft message
	m_pRep->AddItem ( artInt, WORD(TStatusUnit::kDraft) );

	AfxGetMainWnd () -> PostMessage (WMU_REFRESH_DRAFT);
}

////////////////////////////////////////////////////////////////////////////
void TDraftMsgs::EditingArticle (LONG lArticle)
{
	// this info is not serialized

	m_stlsetEditing.insert ( lArticle );
}

////////////////////////////////////////////////////////////////////////////
BOOL TDraftMsgs::IsEditingArticle (LONG lArticle)
{
	return m_stlsetEditing.find ( lArticle ) != m_stlsetEditing.end() ;
}

////////////////////////////////////////////////////////////////////////////
void TDraftMsgs::NotEditingArticle (LONG lArticle)
{
	std::set<int>::size_type n =  m_stlsetEditing.erase ( lArticle );

	//ASSERT(n > 0);
}

////////////////////////////////////////////////////////////////////////////
// AbortingEdit -- called when a post/mail window is dismissed, so the outbox
// takes it out of its list of articles that are being edited
void TDraftMsgs::AbortingEdit (LONG lArticle)
{
	NotEditingArticle (lArticle);
}

