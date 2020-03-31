/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsgrp.cpp,v $
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
/*  Revision 1.4  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
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

#include "stdafx.h"
#include <tchar.h>               // _tcslen(), _ttoi()
#include "newsgrp.h"
#include "globals.h"

#include "thredlst.h"
#include "tmutex.h"
#include "critsect.h"
#include "custmsg.h"
#include "tglobopt.h"
#include "rules.h"               // EvalRulesHaveHeader(), ...
#include "tasker.h"
#include "ngutil.h"
#include "utilpump.h"
#include "utilrout.h"            // PostMainWndMsg
#include "rgconn.h"
#include "bits.h"
#include "grpdb.h"
#include "mplib.h"               // TException
#include "artclean.h"
#include "rgswit.h"
#include "rgui.h"                // TRegUI
#include "expire.h"              // TExpiryData
#include "iterhn.h"              // TArticleNewIterator
#include "newsdb.h"              // THeaderIterator
#include "server.h"              // TNewsServer
#include "servcp.h"              // GetCountedActiveServer
#include "uimem.h"               // TUIMemory
#include "vfilter.h"             // TAllViewFilter
#include "ruleutil.h"            // TagArticle()
#include "tscoring.h"            // ScoreHeader()
#include "genutil.h"             // CopyCStringList ()
#include "superstl.h"            // vector,
#include "tbozobin.h"            // CheckForBozo()
#include "usrdisp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ===========================================================================
// History:
//    Version 1 - Your basic TNewsGroup object
//    version 2 - Add fields m_iServerLow, m_iServerHi
//    version 3 - no real data change, but version change indicates
//                that conversion from 'Storage State (Default)' to
//                a definite storage state
//    version 4 - added m_strDecodeDir, m_strEmail, m_strFullName,
//                m_sCustomHeaders, m_bOverrideDecodeDir, m_bOverrideEmail,
//                m_bOverrideFullName, m_bOverrideCustomHeaders
//    version 5 - take one byte from m_save[3] and use it as m_byProxicomFlags
//    version 6 - change m_unused6 to store m_iDefaultFilter
//    version 7 - add m_fOverrideLimitHeaders, m_iGetHeadersLimit
//    version 8 - separate files are marked with signature bytes.
//    version 9 - add m_fOverrideSig and m_strSigFile (per group signatures)
//
// ********************************************************************
static const int NEWSGROUP_VERSION = 9;

extern TNewsTasker* gpTasker;

TNewsGroupLibrary TNewsGroup::m_library;

IMPLEMENT_SERIAL(TNewsGroup, PObject, VERSIONABLE_SCHEMA|1);

// Constructor for serialization
TNewsGroup::TNewsGroup(void)
:  PObject(NEWSGROUP_VERSION)
{
	Common_Construct (0 /* iGoBack */ );
}

///////////////////////////////////////////////////////////////////////////
// this Constructor is called first.  The name is set and the NG object
// is written to disk.  When instantiated via serialization, the default
// ctor is used, and the name is read in.
//
TNewsGroup::TNewsGroup(TNewsServer *pServer,
					   LPCTSTR  lpstrName,
					   TGlobalDef::ENewsGroupType eType,
					   EStorageOption* peStorage,
					   int iGoBack
					   )
					   : PObject(NEWSGROUP_VERSION),
					   m_name(lpstrName),
					   m_ArtBank(pServer)
{
	// from base class
	SetServerName (pServer->GetNewsServerName());

	// This special Byte identifies the type
	switch (eType)
	{
	case (TGlobalDef::kPostAllowed):
		m_PostingState = kPostAllowed;
		break;
	case (TGlobalDef::kPostNotAllowed):
		m_PostingState = kPostNotAllowed;
		break;
	case (TGlobalDef::kPostModerated):
		m_PostingState = kPostModerated;
		break;
	default:
		throw(new TException(IDS_ERR_GROUP_TYPE, kError));
	}

	Common_Construct (iGoBack);

	// storage mode set from Subscribe Dlg
	if (0 == peStorage)
		m_fUseGlobalStorageOptions = TRUE;
	else
	{
		m_fUseGlobalStorageOptions = FALSE;
		m_kStorageOption = *peStorage;
	}
}

// ------------------------------------------------------------------------
// Common_Construct --
//
void TNewsGroup::Common_Construct (int iGoBack)
{
	m_GroupID = 0;
	m_byRangeDirty = FALSE;

	m_bOverrideDecodeDir = m_bOverrideEmail = m_bOverrideFullName =
		m_bOverrideCustomHeaders = FALSE;
	m_fOverrideLimitHeaders = FALSE;
	m_iGetHeadersLimit = 500;

	m_byPurgeFlags = 0;

	eType = kSubscribed;
	m_kStorageOption = kHeadersOnly;

	// use global settings by default
	m_kPurgeType     = TGlobalDef::kNever;
	m_lPurgeDays     = 7;
	m_fUseGlobalPurge = TRUE;

	m_fUseGlobalStorageOptions = TRUE;

	m_pThreadList = new TThreadList;

	// If we came from the default ctor, this will be set to a
	//   Good value during Serialize()
	m_iRetrieveRange = iGoBack;

	m_iSubscribeLowMark   = -1;

	m_bLoadArticlesMutex = FALSE;

	m_iServerLow = m_iServerHi = 0;

	m_lastPurgeTime = m_lastCompactTime = CTime::GetCurrentTime();

	m_byPingReadArticles = FALSE;

	// the threadlist and the articlebank work together
	m_pThreadList->InstallBank ( &m_ArtBank );

	m_bSample = FALSE;

	m_byProxicomFlags = TNewsGroup::kProxPostingAllowed |
		TNewsGroup::kProxNewTopicAllowed;

	// default filter for this newsgroup is not set
	m_iDefaultFilter = 0;

	// 2 reserved,unused bytes ARE serialized.  So at least write out Zeroes.
	ZeroMemory (m_save, sizeof m_save);

	m_fOverrideSig = FALSE;
}

TNewsGroup::~TNewsGroup(void)
{
	delete m_pThreadList;
	m_library.UnregisterNewsgroup ( this );
}

// -------------------------------------------------------------------------
void TNewsGroup::InstallGroupID (LONG iGroupID)
{
	m_GroupID = iGroupID;
	m_statusMgr.InstallGroupID ( m_GroupID );
}

// -------------------------------------------------------------------------
TViewFilter * TNewsGroup::GetpCurViewFilter ()
{
	bool fDone = false;

	TViewFilter * pVF;
	TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();

	int iFilterID = gpGlobalOptions->GetRegUI()->GetViewFilterID();

	pVF = pAllFilters->GetByID ( iFilterID );

	if (pVF)
		return pVF;

	CString filterName = gpGlobalOptions->GetRegUI()->Deprecated_GetViewFilterName();
	pVF = pAllFilters->GetByName ( filterName );

	if (pVF)
		gpUIMemory->SetViewFilterID ( pVF->getID() );
	return pVF;
}

// -------------------------------------------------------------------------
// fFilterFromCache - if TRUE then use the cache, otherwise hit the server
int TNewsGroup::LoadArticles(BOOL fFilterFromCache)
{
	EStorageOption eStor = UtilGetStorageOption(this);

	// Mode1 is allowed to keep the cache
	if (eStor == kHeadersOnly || eStor == kStoreBodies || !fFilterFromCache)
		Empty();

	BOOL fHitServer = !fFilterFromCache;

	int ret = 1;
	TViewFilter * pVF = TNewsGroup::GetpCurViewFilter ();

	if (0 == pVF)
		AfxMessageBox (IDS_ERR_NOVFLT);
	else
		ret = ForegroundLoadArticles (eStor, fHitServer, pVF);

	return ret;
}

// -------------------------------------------------------------------------
int TNewsGroup::ForegroundLoadArticles (EStorageOption eStor,
										BOOL fHitServer, TViewFilter * pFilter)
{
	TAutoClose auto_close(this);

	BOOL fLinkNow = FALSE;
	BOOL fAllowSaveHeaders = (kNothing == eStor) ? FALSE : TRUE;

	switch (eStor)
	{
	case kHeadersOnly:
	case kStoreBodies:
		{
			TUserDisplay_UIPane sAutoDraw("Threading...");

			// ask the bank to change contents
			m_ArtBank.ChangeContents (fHitServer,
				pFilter,
				this, fLinkNow);

			if (true)
			{
				// thread articles together
				TSyncWriteLock sync(this);
				m_pThreadList->Link(this, pFilter);
			}

			// for statusbar UI pane
			//this->UpdateFilterCount ( true );
			break;
		}

	case kNothing:
		break;
	}
	return 0;
}

// -------------------------------------------------------------------------
int TNewsGroup::Mode1_Thread (BOOL fOld, BOOL fNew)
{
	m_ArtBank.TagContents (fOld, fNew);

	return deferred_thread (TRUE);
}

// ------------------------------------------------------------------------
// deferred_thread -- called from Mode1_Thread and ForegroundLoadArticles
//
int TNewsGroup::deferred_thread (BOOL fEmpty)
{
	{
		// thread articles together
		TSyncWriteLock sync(this);

		// if the user sent 2 async mode1 jobs to the pump, then we should
		// empty out before the threadList links

		if (fEmpty)
			m_pThreadList->EmptyList ();

		TViewFilter * pVF =  TNewsGroup::GetpCurViewFilter();
		if (0 == pVF)
			AfxMessageBox (IDS_ERR_NOVFLT);
		else
			m_pThreadList->Link(this, pVF);
	}

	// the newsview should update the [New] count
	AfxGetMainWnd()->PostMessage( WMC_DISPLAY_ARTCOUNT, m_GroupID );
	return 0;
}

//-------------------------------------------------------------------------
//  Use the cache as much as we can
int TNewsGroup::ReloadArticles (BOOL fEmptyFirst)
{
	{
		TSyncWriteLock sync (this);

		if (fEmptyFirst)
			m_pThreadList->EmptyAll();
		else
			m_pThreadList->EmptyList();
	}

	// act as if we are filtering - use the Cache
	return LoadArticles (TRUE);
}

///////////////////////////////////////////////////////////////////////////
// do some self correction,in case the status unit doesn't match
//   ReadRangeHave
// Returns 1 if correction happened
int TNewsGroup::validate_status_vector ()
{
	TRangeSet rngOut;
	int       iFound = 0;
	int       iTotal = 0;
	int       i, x, y;
	bool      fCorrection = false;

	m_statusMgr.Collect (TStatusUnit::kNew, &rngOut, iFound, iTotal);

	TServerCountedPtr cpNewsServer(GetParentServer());
	TRangeSetReadLock sLock(cpNewsServer, GetName());

	for (i = 0; i < rngOut.RangeCount(); i++)
		for (rngOut.GetRange (i, x, y); x <= y; x++)
			if (read_range_have (&sLock, x))
			{
				m_statusMgr.StatusMarkRead (x);
				fCorrection = true;
			}
			ASSERT(!fCorrection);
			return fCorrection ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////
// ValidateStati -- accounting function
int TNewsGroup::ValidateStati ()
{

#if defined(_DEBUG)
	int nHeadMap, nBodyMap;
	int nHdrRange;
	int nTxtRange;
	int nStatUnitNew, nStatUnitTotal;
	CString ngTR;
	CString dbTR;
	CString msTR;

	TNewsGroupDB::GetHeaderCount (nHeadMap);
	TNewsGroupDB::GetBodyCount   (nBodyMap);
	TNewsGroupDB::GetBodyPrintable (dbTR);
	if (true)
	{
		TSyncReadLock sync (this);

		nHdrRange = m_HdrRange.CountItems ();
		nTxtRange = m_TextRange.CountItems ();
		m_TextRange.toString (ngTR);

		m_MissingRange.toString (msTR);
		//CFile sF3("d:\\msTR.txt", CFile::modeCreate | CFile::modeWrite);
		//sF3.Write ( msTR, msTR.GetLength() );
		m_statusMgr.CountNew (nStatUnitNew, nStatUnitTotal);

		if (true)
		{
			CString rdTR;
			//CFile sF4("d:\\rdTR.txt", CFile::modeCreate | CFile::modeWrite);

			TRangeSetReadLock sLock(GetParentServer(), GetName());

			sLock.m_pRangeSet->toString (rdTR);
			//sF4.Write( rdTR, rdTR.GetLength() );
		}
	}
	//CFile sF1("D:\\ngTR.txt", CFile::modeCreate | CFile::modeWrite);
	//sF1.Write ( ngTR, ngTR.GetLength() );

	//CFile sF2("D:\\dbTR.txt", CFile::modeCreate | CFile::modeWrite);
	//sF2.Write ( dbTR, dbTR.GetLength() );
	ASSERT(!(nBodyMap < nTxtRange));
	ASSERT(!(nBodyMap > nTxtRange));
	ASSERT(nBodyMap == nTxtRange);
	ASSERT(nHeadMap == nHdrRange);
	ASSERT(nHdrRange == nStatUnitTotal);

	if ((nBodyMap != nTxtRange) ||
		(nHeadMap != nHdrRange) ||
		(nHdrRange != nStatUnitTotal))
		return 0;
#endif

	vector<LONG> my_ints;
	vector<LONG> vFlush;
	bool         fSetDirty = false;

	m_statusMgr.AllArticleNumbers (my_ints);

	{
		TSyncReadLock sync (this);

		for (vector<LONG>::iterator it = my_ints.begin();
			it != my_ints.end(); it++)
		{
			LONG artInt = *it;

			if (NULL == TNewsGroupDB::GetHeader (artInt))
			{
				// we found a StatusUnit that refers to an article header.
				//   but the article header is not in our headerMap
				vFlush.insert (vFlush.end(), artInt);
			}
		}
	}

	for (vector<LONG>::iterator i = vFlush.begin();
		i != vFlush.end(); i++)
	{
		m_statusMgr.Destroy ( *i );
		fSetDirty = true;
	}

	int ret = validate_status_vector();
	if (ret || fSetDirty)
		SetDirty();

	return ret;
}

///////////////////////////////////////////////////////////////////////////
//  This is more complex than i thought (especially with Cross-Post-Mgmt)
//
int TNewsGroup::CatchUpArticles(void)
{
	BOOL fBothLocalAndServer = gpGlobalOptions->GetRegSwitch()->GetCatchUpAll();
	KThreadList::EXPost eXPost = gpGlobalOptions->GetRegSwitch()->GetCatchUpCPM()
		? KThreadList::kHandleCrossPost
		: KThreadList::kIgnoreCrossPost;

	// internal functions should handle the write-lock
	// Bad scenario in ver.100
	// SaveSubscribedGroups
	//    needs Read lock on each Group
	// CatchUp
	//    Gets WriteLock on NG, calls ThredLst.CatchUp
	//    ThredLst.CatchUp changes the RangeSets and calls SaveSubscribedGroups
	//
	// Fix - ThredLst.CatchUp does a UnlockWrite, the calls SaveSubscribedGroups

	if (fBothLocalAndServer)
	{
		// do local
		catchup_local ( eXPost );

		// do server
		catchup_server ( FALSE );
	}
	else
	{
		// just local articles

		catchup_local ( eXPost );
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//
void TNewsGroup::catchup_local (KThreadList::EXPost eXPost)
{
	if (kNothing == UtilGetStorageOption(this))
	{
		// examine thread list and wipe out
		m_pThreadList->CatchUpArticles ( this, eXPost );
	}
	else
	{
		BOOL fKeepTags;  // Tags are impervious to catchup

		if (KThreadList::kHandleCrossPost == eXPost)
		{
			// wipe out thread list. kill loaded contents

			m_pThreadList->CatchUpArticles ( this, eXPost );
			Empty();

			//  LoadArticlesIntoThreadList calls Open/Close to load headers
			// pNG->Close will call PreDeleteHeaders and wipe out the thread list
			// so...Hold Newsgroup Open

			{
				TAutoClose sCloser(this);

				BOOL fLinkNow;  // unused
				// 2nd pass load latest database stuff into article bank

				// make and ad-hoc filter
				TViewFilter sVF(0);
				sVF.SetNew(TStatusUnit::kYes);

				m_ArtBank.ChangeContents (FALSE, // fHitServer
					&sVF,
					this, fLinkNow);

				// wipe out this thread list
				fKeepTags = m_pThreadList->CatchUpArticles ( this, eXPost );

				// just in case there's a status unit that is not in the
				//  thread list
				catchup_status ( fKeepTags );

				Empty();
			}
		} // yes CPM
		else
		{
			// wipe out contents of thread list
			fKeepTags = m_pThreadList->CatchUpArticles ( this, eXPost );
			m_pThreadList->EmptyAll ();

			// mark anything else in the database as read (stuff not loaded)
			catchup_status ( fKeepTags );
		} // no CPM
	}
}

///////////////////////////////////////////////////////////////////////////
//  In truth, we never do Cross-Post-Management for the non-local arts
//
void TNewsGroup::catchup_server (BOOL fKeepTags)
{
	fKeepTags = gpGlobalOptions->GetRegSwitch()->GetCatchUpKeepTags();

	TPersistentTags* pTags = 0;
	TServerCountedPtr cpNewsServer(GetParentServer());

	if (fKeepTags)
		pTags = &(cpNewsServer->GetPersistentTags ());

	int itmpServerLow = m_iServerLow;
	if (itmpServerLow < m_iSubscribeLowMark)
		itmpServerLow = m_iSubscribeLowMark;

	for (int iArtInt = itmpServerLow; iArtInt <= m_iServerHi; iArtInt++)
	{
		// note: article may have been downloaded and still exist at the server
		if (fKeepTags && pTags->FindTag (m_GroupID, iArtInt))
			;
		else
		{
			// kill it
			read_range_util ( iArtInt,
				NULL,   // we never do CrossPost mgmt on Server arts
				KThreadList::kIgnoreCrossPost,
				TRUE ); // fMarkSeen
		}
	}
}

// --------------------------------------------------------------------------
// helper function to 'zero-out' the status mgr.  Just in case there is
// something in here that is not loaded via the thread list.
void TNewsGroup::catchup_status (BOOL fKeepTags)
{
	TPersistentTags* pTags = 0;
	TRangeSet range;
	int found = 0;
	int total = 0;

	TServerCountedPtr cpNewsServer(GetParentServer());

	if (fKeepTags)
		pTags = &(cpNewsServer->GetPersistentTags ());

	// collect any that are new
	m_statusMgr.Collect (TStatusUnit::kNew, &range, found, total);
	total = range.RangeCount();
	for (int i = 0; i < total; ++i)
	{
		int lo, hi;
		range.GetRange(i, lo, hi);

		// mark read in status unit and range-set
		for (int j = lo; j <= hi; ++j)
		{
			if (fKeepTags && pTags->FindTag (m_GroupID, j))
			{
				// keep alive
			}
			else
			{
				read_range_util ( j, NULL,
					KThreadList::kIgnoreCrossPost,
					TRUE );  // mark seen
			}
		}
	}
}

BOOL TNewsGroup::HdrRangeHave(int artInt)
{
	BOOL ret;
	TSyncReadLock sync (this);
	ret = m_HdrRange.Have(artInt);
	return ret;
}

void TNewsGroup::HdrRangeAdd(int artInt)
{
	TSyncWriteLock sync (this);
	m_HdrRange.Add(artInt);

	SetDirty ();
}

void TNewsGroup::HdrRangeSave(void)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	TSyncReadLock sync(this);
	cpNewsServer->SaveIndividualGroup (this);
	//TRACE1("%s Header rangeSet saved\n", GetName() );
	ClearDirty();
}

void TNewsGroup::HdrRangeSubtract(int artInt)
{
	TSyncWriteLock sync(this);
	m_HdrRange.Subtract ( artInt );
	SetDirty ();
}

void TNewsGroup::HdrRangeEmpty(void)
{
	TSyncWriteLock sync(this);
	m_HdrRange.Empty();
	SetDirty ();
}

// created for the Cleanup headers of expired articles function  1-2-96
void TNewsGroup::HdrRangeConvert(CList<int, int&>* pList)
{
	TSyncReadLock sync(this);
	int cnt = m_HdrRange.RangeCount();
	for (int i = 0; i < cnt; i++)
	{
		int lo, hi;
		m_HdrRange.GetRange(i, lo, hi);
		for (int k = lo; k <= hi; ++k)
		{
			if (!m_TextRange.Have ( k ))
				pList->AddTail ( k );
		}
	}
}

int  TNewsGroup::HdrRangeCount()
{
	TSyncReadLock sync(this);
	return  m_HdrRange.RangeCount();
}

void TNewsGroup::TextRangeSave(void)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	//TRACE1("%s Text rangeSet saved (start)\n", GetName() );

	// to save 1 newsgroup we have to save them all
	cpNewsServer->SaveIndividualGroup ( this );

	//TRACE1("%s Text rangeSet saved\n", GetName() );
	ClearDirty();
}

void TNewsGroup::TextRangeDump(CString & str)
{
}

BOOL TNewsGroup::TextRangeHave(int artInt)
{
	BOOL fRet;
	TSyncReadLock sync(this);

	fRet = m_TextRange.Have(artInt);
	return fRet;
}

void TNewsGroup::TextRangeAdd(int artInt)
{
	TSyncWriteLock sync(this);
	m_TextRange.Add(artInt);
	SetDirty();
}

void TNewsGroup::TextRangeSubtract(TRangeSet& sub, TRangeSet& result)
{
	TSyncWriteLock sync(this);
	m_TextRange.Subtract ( sub, result );
	SetDirty();
}

void TNewsGroup::TextRangeSubtract (int artInt)
{
	TSyncWriteLock sync(this);
	m_TextRange.Subtract(artInt);
	SetDirty();
}

void TNewsGroup::TextRangeEmpty (void)
{
	TSyncWriteLock sync(this);
	m_TextRange.Empty();
	SetDirty();
}

// --------------------------------------------------------------------------
// Serialize --
void TNewsGroup::Serialize(CArchive & ar)
{
	TSyncReadLock sync(this);

	BYTE objectVersion;

	PObject::Serialize ( ar );

	objectVersion = GetObjectVersion ();

	if (ar.IsStoring())
		storeSerialize ( ar );
	else
		loadSerialize ( ar, objectVersion );
}

#define NEWSGROUP_FILE_SIGNATURE_BYTES   0xFBCAFEDB

// -------------------------------------------------------------------------
void TNewsGroup::storeSerialize  (CArchive & ar)
{
	// store signature DWORD

	DWORD dwSignature = NEWSGROUP_FILE_SIGNATURE_BYTES;
	ar <<    dwSignature;

	ar <<    m_GroupID;
	ar <<    m_name;
	m_HdrRange.Serialize (ar);
	m_TextRange.Serialize (ar);
	ar <<    m_byPurgeFlags;
	ar <<    m_wPurgeReadLimit;
	ar <<    m_wPurgeUnreadLimit;
	ar <<    m_wPurgeOnHdrsEvery;
	ar <<    m_wCompactOnShutdownEvery;
	ar <<    LONG(eType);
	ar << (LONG) m_kStorageOption;
	ar << m_nickname;
	ar << m_iRetrieveRange;
	ar << m_iSubscribeLowMark;
	ar << LONG(m_kPurgeType);
	ar << m_lPurgeDays;
	ar << m_fUseGlobalPurge;
	ar << m_fUseGlobalStorageOptions;
	ar << BYTE(m_PostingState);
	ar << BYTE(m_byProxicomFlags & ~TNewsGroup::kProxValidated);
	ar << m_save[0] << m_save[1];
	ar << m_iDefaultFilter;
	m_statusMgr.Serialize (ar);
	m_MissingRange.Serialize (ar);

	ar << m_lastPurgeTime;
	ar << m_lastCompactTime;

	ar << m_iServerLow;
	ar << m_iServerHi;
	ar << m_bOverrideDecodeDir;
	ar << m_bOverrideEmail;
	ar << m_bOverrideFullName;
	ar << m_bOverrideCustomHeaders;
	ar << m_strDecodeDir;
	ar << m_strEmail;
	ar << m_strFullName;
	int iCount = m_sCustomHeaders.GetCount ();
	ar << iCount;
	POSITION pos = m_sCustomHeaders.GetHeadPosition ();
	while (pos) {
		CString str = m_sCustomHeaders.GetNext (pos);
		ar << str;
	}

	// limit headers to (500)
	ar << m_fOverrideLimitHeaders;
	ar << m_iGetHeadersLimit;

	// ver 9 stuff
	ar << m_fOverrideSig;
	ar << m_strSigShortName;
}

// -------------------------------------------------------------------------
void TNewsGroup::loadSerialize  (CArchive & ar, BYTE objectVersion)
{
	DWORD dwSignatureBytes;

	if (objectVersion >= 8)
	{
		ar >> dwSignatureBytes;

		if (NEWSGROUP_FILE_SIGNATURE_BYTES != dwSignatureBytes)
		{
			throw(new TException ("bad signature", kError));
		}
	}

	ar >>    m_GroupID;
	ar >>    m_name;
	m_HdrRange.Serialize (ar);
	m_TextRange.Serialize (ar);
	ar >>    m_byPurgeFlags;
	ar >>    m_wPurgeReadLimit;
	ar >>    m_wPurgeUnreadLimit;
	ar >>    m_wPurgeOnHdrsEvery;
	ar >>    m_wCompactOnShutdownEvery;
	LONG     temp;
	ar >>    temp;
	eType = ENewsGroupType(temp);
	ar >> temp;

	// 3-8-99  - we are dumping mode-1, convert it to mode2
	if (TNewsGroup::kNothing == EStorageOption(temp))
		m_kStorageOption = TNewsGroup::kHeadersOnly;
	else
		m_kStorageOption = EStorageOption(temp);

	ar >> m_nickname;
	ar >> m_iRetrieveRange;
	ar >> m_iSubscribeLowMark;
	ar >> temp;
	m_kPurgeType = (TGlobalDef::EPurgeType) temp;
	ar >> m_lPurgeDays;
	ar >> m_fUseGlobalPurge;
	ar >> m_fUseGlobalStorageOptions;
	BYTE byTmp;
	ar >> byTmp;
	m_PostingState = (EPosting) byTmp;

	ar >> m_byProxicomFlags;
	if (objectVersion < 5)
	{
		// only valid in objectVersion 5 and up
		m_byProxicomFlags = (TNewsGroup::kProxPostingAllowed |
			TNewsGroup::kProxNewTopicAllowed);
	}

	ar >> m_save[0] >> m_save[1];

	ar >> m_iDefaultFilter;
	if ((objectVersion < 6) || (m_iDefaultFilter < 0))
	{
		// only valid in objectVersion 6 and up
		//  -OR- clear the default filter if it is a garbage number
		m_iDefaultFilter = 0;
	}
	m_statusMgr.InstallGroupID (m_GroupID);
	m_statusMgr.Serialize (ar);
	m_MissingRange.Serialize (ar);
	ar >> m_lastPurgeTime;
	ar >> m_lastCompactTime;

	// ----   versioning  add-ons

	if (objectVersion > 1)
	{
		ar >> m_iServerLow;
		ar >> m_iServerHi;
	}
	if (objectVersion > 3)
	{
		ar >> m_bOverrideDecodeDir;
		ar >> m_bOverrideEmail;
		ar >> m_bOverrideFullName;
		ar >> m_bOverrideCustomHeaders;
		ar >> m_strDecodeDir;
		ar >> m_strEmail;
		ar >> m_strFullName;
		int iCount;
		ar >> iCount;
		for (int i = 0; i < iCount; i++) {
			CString str;
			ar >> str;
			m_sCustomHeaders.AddTail (str);
		}
	}
	if (objectVersion > 6)
	{
		ar >> m_fOverrideLimitHeaders;
		ar >> m_iGetHeadersLimit;
	}

	if (objectVersion >= 9)
	{
		ar >> m_fOverrideSig;
		ar >> m_strSigShortName;
	}
}

// -------------------------------------------------------------------------
// This sort of upgrade conversion usually is done via Versioning/Serialization
//   but here I need the gpGlobalOptions pointer to be setup correctly.
//
void TNewsGroup::UpgradeNeedsGlobalOptions (TGlobalOptions * pOptions)
{
	BYTE objectVersion = GetObjectVersion ();

	if (objectVersion < 3)
	{
		// if this Group's storage is setup to 'Use Global Default'
		//  convert it.
		if (m_fUseGlobalStorageOptions)
		{
			m_kStorageOption = pOptions->GetStorageMode ();
			m_fUseGlobalStorageOptions = FALSE;
		}
	}
}

// -------------------------------------------------------------------------
BOOL TNewsGroup::ReadRangeHave(int artInt)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	TRangeSetReadLock sLock(cpNewsServer, GetName());

	return read_range_have (&sLock, artInt);
}

// -------------------------------------------------------------------------
// private
// called from  ReadRangeHave,  validate_status_vector
//
BOOL TNewsGroup::read_range_have (TRangeSetReadLock * psLock, int artInt)
{
	if (psLock->m_pRangeSet)
		return psLock->m_pRangeSet->Have (artInt);
	else
		return FALSE;
}

// static class function
void TNewsGroup::NextXRefsToken (const char *pchSrc, int &iPos, CString &strDest)
{
	strDest = "";

	// ignore whitespace
	while (pchSrc[iPos] && pchSrc[iPos] == ' ')
		iPos ++;

	// read characters until end-of-string, or whitespace, or a colon
	while (pchSrc[iPos] && pchSrc[iPos] != ' ' && pchSrc[iPos] != ':')
		strDest += pchSrc[iPos++];

	// ignore whitespace, and step over colons
	while (pchSrc[iPos] && (pchSrc[iPos] == ' ' || pchSrc[iPos] == ':'))
		iPos ++;
}

// ------------------------------------------------------------------------
// static class function
void TNewsGroup::ProcessXRefs (LPCTSTR pchRefs, BOOL fMarkSeen)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	// the X-Ref string contains items of the form <newsgroup>:<number>
	// separated by whitespace
	int iLen = _tcslen (pchRefs);

	if (0 == iLen)                // optimization 4-25-96 -amc
		return;

	CString strGroup('#', 256);   // big initial size to minimize reallocs
	CString strNum('#', 16);
	int artNum;
	TNewsGroup *pNG;
	int iPos = 0;                 // position while scanning pchRefs
	BOOL fUseLock;

	// read the initial token, which should be the server's name
	NextXRefsToken (pchRefs, iPos, strGroup);

	while (iPos < iLen) {
		NextXRefsToken (pchRefs, iPos, strGroup);
		NextXRefsToken (pchRefs, iPos, strNum);
		ASSERT (!strGroup.IsEmpty () && !strNum.IsEmpty ());
		artNum = _ttoi (strNum);

		{
			// add the article to this group's CPM read-range, whether we're
			// subscribed to it or not
			TRangeSetWriteLock sWriteLock(cpNewsServer, strGroup);

			// note - we don't keep CP info for non-subscribed groups. check for NULL
			if (sWriteLock.m_pRangeSet)
			{
				if (fMarkSeen)
					sWriteLock.m_pRangeSet->Add (artNum);
				else
					sWriteLock.m_pRangeSet->Subtract (artNum);
			}
		}

		fUseLock = FALSE;
		pNG = 0;
		TNewsGroupUseLock useLock (cpNewsServer, strGroup, &fUseLock, pNG);

		if (fUseLock && pNG)
		{
			// do more work if we are actually subscribed
			pNG->read_range_chglocal ( artNum, fMarkSeen );
		}
	}
}

// this is the PREFERRED form
void TNewsGroup::ReadRangeAdd(TArticleHeader * pArtHdr,
							  KThreadList::EXPost eXPost /* = kHandleCrossPost */)
{
	ASSERT(pArtHdr);
	int artInt = pArtHdr->GetNumber();

	read_range_util( artInt, pArtHdr, eXPost, TRUE );
}

// use this simple form if you really don't have an Article object handy
void TNewsGroup::ReadRangeAdd(int artInt,
							  KThreadList::EXPost eXPost /* = kHandleCrossPost */)
{

	if (KThreadList::kIgnoreCrossPost == eXPost)
	{
		// if we are not doing CPM, we don't need the article object

		read_range_chglocal ( artInt, TRUE /* fMarkSeen */ );

	}
	else
	{
		TPersist822Header * p822Hdr = GetHeader( artInt );

		if (0 == p822Hdr)
		{
			ASSERT(0);
			return;
		}

		read_range_util ( artInt, p822Hdr->CastToArticleHeader(), eXPost, TRUE );
	}
}

void TNewsGroup::read_range_util(int artInt, TArticleHeader * pArtHdr,
								 KThreadList::EXPost eXPost, BOOL fMarkSeen)
{
	/* TSyncWriteLock sync(this); */   // not needed, every locks on their own

	// does it's own locking
	read_range_chglocal (artInt, fMarkSeen);

	// does it's own locking
	SetDirty();

	// ProcessXRefs does its own locking of each newsgroup

	if (KThreadList::kHandleCrossPost == eXPost)
	{
		// also, mark the article as read in groups that it's cross-posted to
		ProcessXRefs (pArtHdr->GetXRef (), fMarkSeen);
	}
}

// --------------------------------------------------------------------
// Handles one newsgroup - StatusUnit & RangeSet
void TNewsGroup::read_range_chglocal (int artInt, BOOL fMarkSeen)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	{
		TRangeSetWriteLock sWriteLock(cpNewsServer, GetName());

		// add the article to this group's read-range
		ASSERT (sWriteLock.m_pRangeSet);

		if (sWriteLock.m_pRangeSet)
		{
			if (fMarkSeen)
				sWriteLock.m_pRangeSet->Add (artInt);
			else
				sWriteLock.m_pRangeSet->Subtract (artInt);
		}
	}

	// lock newsgroup object
	TSyncWriteLock sync(this);

	if (fMarkSeen)
	{
		// mark the article's read-bit in its status-unit
		if (0 == m_statusMgr.StatusMarkRead (artInt))
			SetDirty ();
	}
	else
	{
		// change article's status-unit to Unread
		if (0 == m_statusMgr.StatusMarkUnread (artInt))
			SetDirty ();
	}
}

// Sub - 'this' = result
void TNewsGroup::ReadRangeSubtract(TRangeSet& sub, TRangeSet& result)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	TRangeSetWriteLock sLock(cpNewsServer, GetName());
	TRangeSet *pReadRange = sLock.m_pRangeSet;
	if (pReadRange)
		pReadRange->Subtract (sub, result);
}

//-------------------------------------------------------------------------
// Preferred form - can undo CrossPostMgmt
void TNewsGroup::ReadRangeSubtract(TArticleHeader* pArtHdr,
								   KThreadList::EXPost eXPost)
{
	ASSERT(pArtHdr);

	int artInt = pArtHdr->GetNumber ();

	read_range_util ( artInt, pArtHdr, eXPost, FALSE );
}

//------------------------------------------------------------------------
void TNewsGroup::ReadRangeSubtract_NoCPM (int iArtInt)
{
	// no cross-post-management
	read_range_util ( iArtInt, NULL, KThreadList::kIgnoreCrossPost, FALSE );
}

void TNewsGroup::ReadRangeEmpty(void)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	TRangeSetWriteLock sLock(cpNewsServer, GetName());
	TRangeSet *pReadRange = sLock.m_pRangeSet;

	if (pReadRange)
		pReadRange->Empty();
}

void TNewsGroup::HTRangeSubtract(int artInt)
{
	TSyncWriteLock sync(this);

	m_HdrRange.Subtract(artInt);
	m_TextRange.Subtract(artInt);
}

void TNewsGroup::HTRangeAdd(int artInt)
{
	TSyncWriteLock sync(this);
	m_HdrRange.Add(artInt);
	m_TextRange.Add(artInt);
}

void TNewsGroup::MissingRangeAdd(int artInt)
{
	TSyncWriteLock sync(this);
	m_MissingRange.Add(artInt);
}

BOOL TNewsGroup::MissingRangeHave(int artInt)
{
	TSyncReadLock sync(this);
	return m_MissingRange.Have(artInt);
}

void TNewsGroup::MissingRangeEmpty()
{
	TSyncWriteLock sync(this);
	m_MissingRange.Empty();
}

void TNewsGroup::repair_statusunit (int artInt)
{
	// $$ create a status unit on demand
	StatusAdd_New ( artInt );
}

BOOL TNewsGroup::IsStatusBitOn(int artInt, TStatusUnit::EStatus status)
{
	BOOL fRet;
	TStatusUnit unit;

	{
		TSyncReadLock sync(this);
		fRet = m_statusMgr.Lookup ( artInt, unit );
	}

	if (FALSE == fRet)
	{
		repair_statusunit (artInt);
		return IsStatusBitOn (artInt, status);
	}
	fRet = (unit.m_status & status) ? TRUE : FALSE;
	return fRet;
}

// ------------------------------------------------------------------------
// This version will not add a status unit on demand.
// Returns 0 for success, non-zero otherwise
int  TNewsGroup::iStatusDirect(int artInt, WORD& wResult)
{
	TStatusUnit unit;

	{
		TSyncReadLock sync(this);
		wResult = 0;
		if (!m_statusMgr.Lookup (artInt, unit))
			return -1;
	}

	// transfer the status bits
	wResult = unit.m_status;

	return 0;
}

// ------------------------------------------------------------------------
// Will add a status unit on demand if not found
int  TNewsGroup::iStatusRepDirect(int artInt, WORD& wResult)
{
	if (0 == iStatusDirect (artInt, wResult))
		return 0;

	repair_statusunit ( artInt );
	return iStatusDirect (artInt, wResult);
}

void TNewsGroup::StatusSet(int artInt,
						   WORD status
						   )
{
	BOOL fRet;
	TStatusUnit unit;
	TSyncWriteLock sync(this);

	fRet = m_statusMgr.Lookup ( artInt, unit );
	if (FALSE == fRet)
	{
		repair_statusunit (artInt);
		StatusSet (artInt, status);
		return;
	}
	unit.m_status = status;
	m_statusMgr.SetAt(artInt, unit);
}

void TNewsGroup::StatusBitSet(int artInt,
							  TStatusUnit::EStatus kStatus,
							  BOOL                 fOn
							  )
{
	TSyncWriteLock sync(this);
	TStatusUnit unit;

	BOOL fRet = m_statusMgr.Lookup ( artInt, unit );
	if (FALSE == fRet)
	{
		unit.m_status = TStatusUnit::kNew;
	}

	if (fOn)
	{
		unit.m_status |= kStatus;
	}
	else
	{
		unit.m_status &= ~kStatus;
		if (kStatus == TStatusUnit::kNew)
		{
			unit.m_timeRead = CTime::GetCurrentTime();
		}
	}

	if (fRet)
		m_statusMgr.SetAt(artInt, unit);
	else
	{
		// lookup failed
		StatusAdd_New (artInt, unit.m_status);
	}
}

void TNewsGroup::StatusMarkRead(int artInt)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	TSyncWriteLock sync(this);

	int stat = m_statusMgr.StatusMarkRead ( artInt );
	if (0 == stat)
		SetDirty();
	else if (stat < 0)
		ASSERT(0);

	TRangeSetWriteLock sLock(cpNewsServer, GetName());
	TRangeSet *pReadRange = sLock.m_pRangeSet;

	if (pReadRange)
	{
		pReadRange->Add (artInt);
	}
}

// --------------------------------------------------------------------------
// Returns 0 if we created a new statusunit, 1 if unit existed already
int  TNewsGroup::StatusAdd_New(int artInt, WORD status)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	int ret = 0;
	TSyncWriteLock sync(this);
	ret = m_statusMgr.Add_New(artInt, status);

	// we're adding a new header's status-unit.  Check this group's read-range,
	// and if this article is in it, set its status to read
	TRangeSetReadLock sLock(cpNewsServer, GetName());
	TRangeSet *pReadRange = sLock.m_pRangeSet;
	ASSERT (pReadRange);
	if (pReadRange && pReadRange->Have (artInt))
	{
		int stat = m_statusMgr.StatusMarkRead (artInt);
		ASSERT(stat >= 0);
	}
	return ret;
}

void TNewsGroup::Collect_Old(CTime threshold, TRangeSet* pRangeSet)
{
	TSyncReadLock sync(this);
	m_statusMgr.CollectOld (threshold, pRangeSet);
}

void TNewsGroup::StatusDestroy (int artInt)
{
	TSyncWriteLock sync(this);
	m_statusMgr.Destroy(artInt);
}

void TNewsGroup::StatusDestroyAll(void)
{
	TSyncWriteLock sync(this);
	m_statusMgr.DestroyAll();
}

// ------------------------------------------------------------------------
// Returns just about everthing you need to know.  This used to be a Thrdlvw
//   function, but it's better as a newsgroup member func.
BOOL TNewsGroup::ArticleStatus (int artInt, WORD& wStatus, BOOL& fHaveBody)
{
	if (iStatusDirect ( artInt, wStatus ))
		return FALSE;

	if (wStatus & TStatusUnit::kNew)
	{
		if (ReadRangeHave ( artInt ))
			wStatus &= ~TStatusUnit::kNew;
	}

	fHaveBody = TextRangeHave ( artInt );
	return TRUE;
}

void TNewsGroup::SetDirty(void)
{
	TSyncWriteLock sync(this);
	++m_byRangeDirty;
}

BOOL TNewsGroup::GetDirty(int* pCount)
{
	BOOL fRet;
	TSyncReadLock sync(this);
	if (pCount)
		*pCount = int(m_byRangeDirty);
	fRet = (m_byRangeDirty > 0);
	return fRet;
}

void TNewsGroup::ClearDirty(void)
{
	// this should be a WriteLock, but it's so damn minor
	m_byRangeDirty = 0;
}

void TNewsGroup::Empty(void)
{
	TSyncWriteLock sync(this);
	{
		m_pThreadList->EmptyAll();
	}
}

BOOL TNewsGroup::FindThreadPile (TArticleHeader* pArtHdr, TThreadPile*& pPile)
{
	TThread*  pThread = 0;
	TArtNode* pNode = 0;
	TSyncReadLock sync(this);
	return m_pThreadList->FindThreadPile ( pArtHdr, pPile, pThread, pNode );
}

// a shell
BOOL TNewsGroup::FindThread (TArticleHeader* pArtHdr, TThread*& pFamily)
{
	TThreadPile * pPile = 0;   // not used
	TArtNode* pNode = 0;       // not used

	TSyncReadLock sync(this);

	return m_pThreadList->FindThreadPile ( pArtHdr, pPile, pFamily, pNode );
}

// --------------------------------------------------------------------------
// we warn user if she is unsubscribing from a group that has protected arts
void TNewsGroup::StatusCountProtected(int & iProtected)
{
	TSyncReadLock sync(this);

	int iTotal = 0;
	TRangeSet  sUnused;
	m_statusMgr.Collect (TStatusUnit::kPermanent, &sUnused, iProtected, iTotal);
}

BOOL TNewsGroup::GetpHeader(int artInt, TArticleHeader*& rpHeader)
{
	BOOL fRet = FALSE;

	LPVOID pJunk;

	fRet = m_pThreadList->FindUIData ( pJunk, rpHeader, artInt );

	return fRet;
}

//-------------------------------------------------------------------------
void TNewsGroup::MarkThreadRead( TArtNode* pArtNode )
{
	TSyncWriteLock sync(this);
	m_pThreadList->MarkThreadStatus ( this, pArtNode, TStatusUnit::kNew, FALSE );
}

//-------------------------------------------------------------------------
int  TNewsGroup::MarkThreadStatus (TArticleHeader* pArtHdr,
								   TStatusUnit::EStatus eStatus,
								   BOOL fBitOn /* == TRUE */)
{
	TSyncWriteLock sync(this);
	return m_pThreadList->MarkThreadStatus ( this, pArtHdr, eStatus, fBitOn );
}

//-------------------------------------------------------------------------
int  TNewsGroup::MarkPileStatus (TArticleHeader* pArtHdr,
								 TStatusUnit::EStatus eStatus,
								 BOOL fBitOn /* == TRUE */)
{
	TSyncWriteLock sync(this);
	return m_pThreadList->MarkPileStatus ( this, pArtHdr, eStatus, fBitOn );
}

////////////////////////////////////////////////////////////////////////
void TNewsGroup::CalcLowwaterMark(int iServerLo, int iServerHi)
{
	BOOL fValid = (iServerHi >= iServerLo) && (iServerHi > 0);
	// note that "rec.funny 0 0" would imply 1 article

	if (fValid && GetLowwaterMark() < 0)
	{
		int goback = GetRetrieveRange();
		int n = max(iServerHi - goback + 1, 0);
		SetLowwaterMark (n);
	}
}

////////////////////////////////////////////////////////////////////////
// adjust lower bound before calling overview. Ensure that:
//  1) the database doesn't have it.
//  2) the user hasn't read it.
//  3) limit headers according to user prefs. Get most recent 1st
//
void TNewsGroup::adjust_bound (
							   BOOL fGetAccordingToGroup,
							   int  iUserDefinedHdrsLimit,
							   const POINT& ptServer,
							   TRangeSet* pLumper,
							   TRangeSet* pArtPing   // see what hdrs (we got on disk) that the server expired
							   )
{
	BOOL fPing = TRUE;
	{
		TServerCountedPtr cpServer;               // counted pointer
		fPing = cpServer->GetExpirePhase ();
	}

	SetServerBounds(ptServer.x, ptServer.y);

	POINT ptS = ptServer;

	CalcLowwaterMark( ptS.x, ptS.y );

	// low bound can depend on how far you go back
	ptS.x = max(ptS.x, GetLowwaterMark());

	int first = ptS.x;
	int last  = ptS.y;

	int iHdrLimit = 0;
	if (fGetAccordingToGroup)
		UtilGetLimitHeaders (this, iHdrLimit);
	else
		iHdrLimit = iUserDefinedHdrsLimit;   // we actually prompted the user

	switch (UtilGetStorageOption(this))
	{
	case TNewsGroup::kHeadersOnly:
		adjust_bound_headers_only(first, last, iHdrLimit, pLumper, fPing, pArtPing);
		break;

	case TNewsGroup::kStoreBodies:
		adjust_bound_store_bodies(first, last, iHdrLimit, pLumper, fPing, pArtPing);
		break;

	default:
		ASSERT(0);
		break;
	}
}

// ------------------------------------------------------------------------
// Return pLumper - articles to request from the server, can be NULL
//        pArtPing - represents headers on disk, gotta find out if these
//                   articles are still alive on the server
//
// FormatNewArticles calls this function with pArtPing == NULL
int TNewsGroup::adjust_bound_headers_only (int        first,
										   int        last,
										   int        iHdrLimit,
										   TRangeSet* pLumper,
										   BOOL       fPingArticles,
										   TRangeSet* pArtPing)
{
	// everytime we download, we check Headers we have and make sure the
	//  bodies on the server are unexpired.
	int iAdded = 0;

	if (first <= last)
	{
		iAdded = build_request_set (&m_HdrRange, first, last, iHdrLimit, pLumper);
	}

#if defined(_DEBUGXXXX)
	afxDump << "Adjust bounds " << GetName() << "  Fetch " << iAdded << "\n";
#endif

	// gotta figure out if any hdrs that are on-disk correspond to
	// expired articles on the server
	if (fPingArticles && pArtPing)
	{
		if (!m_byPingReadArticles)
		{
			// only do this once a session
			m_byPingReadArticles = TRUE;

			// Investigate everything we have
			CalculatePingSet (false, pArtPing);
		}
		else
		{
			// Investigate only the unread articles. This is an optimization
			CalculatePingSet (true, pArtPing);
		}

		// if the server doesn't have these articles anymore,
		// we will know to purge them

		// !! need to use low-bound here
	} // pArtPing is non-null

	// use Low bound..

	return iAdded;
}

///////////////////////////////////////////////////////////////////////////
// CalculatePingSet -- what local hdrs are we verifying
//
void TNewsGroup::CalculatePingSet (bool fOnlyCheckUnread, TRangeSet * pPing)
{
	ASSERT(pPing);
	int nRangeLow, nRangeHi;

	TSyncReadLock readLock (this);  // lock for hdr range

	for (int r = 0; r < m_HdrRange.RangeCount(); r++)
	{
		m_HdrRange.GetRange(r, nRangeLow, nRangeHi);
		for (; nRangeLow <= nRangeHi; nRangeLow++)
		{
			if ((nRangeLow >= m_iServerLow) && !TextRangeHave(nRangeLow))
			{
				// option to Investigate only the unread articles.

				if (!(fOnlyCheckUnread && ReadRangeHave(nRangeLow)))
					pPing->Add (nRangeLow);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// Return pLumper - articles to request from the server
//        pArtPing - represents headers on disk, gotta find out if these
//                   articles are still alive on the server
//
// FormatNewArticles calls this function with pArtPing == NULL
int TNewsGroup::adjust_bound_store_bodies(int first, int last,
										  int iHdrLimit,
										  TRangeSet* pLumper,
										  BOOL       fPingArticles,
										  TRangeSet* pArtPing)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	// fPingArticles controls whether we check Headers we have and make sure
	//  the bodies on the server are unexpired.

	int iAdded = 0;

	if (first <= last)
	{
		iAdded = build_request_set (&m_TextRange, first, last, iHdrLimit, pLumper);
	}

#if defined(_DEBUG)
	afxDump << "Adjust bounds " << GetName() << "  Fetch " << iAdded << "\n";
#endif

	// gotta figure out if any hdrs that are on-disk correspond to
	// expired articles on the server
	if (fPingArticles && pArtPing)
	{
		// Investigate everything we have
		CalculatePingSet (false, pArtPing);

		// if the server doesn't have these articles anymore,
		// we will know to purge them
	}
	return iAdded;
}

void TNewsGroup::excluded_add(int i, TRangeSet* pRangeSet)
{
	pRangeSet->Add (i);
}

///////////////////////////////////////////////////////////////////////////
// Show the user what's new on the server as quickly as possible
//  - display 'new articles that are resident'
//  - display 'new articles on the server'
//
//  - NOT display 'total new articles on the server'
void TNewsGroup::SetServerBounds(int srv_lo, int srv_hi)
{
	if ((m_iServerLow > 0) && srv_lo < m_iServerLow)
	{
		// this indicates a problem.
		ASSERT(0);

		// to be absolutely sure we have a problem.
		if (srv_hi < m_iServerLow)
		{
			// this probably indicates that the server has renumbered this
			//   newsgroup.
			PTYP_GROUPRENUM psRenum = new TYP_GROUPRENUM;
			psRenum->m_strNewsGroup = GetName();
			psRenum->m_iServerLow   = srv_lo;
			psRenum->m_iPreviousLow = m_iServerLow;

			// the UI thread will present the warning to the user
			PostMainWndMsg (WMU_GROUPRENUMBERED_WARNING, WPARAM(psRenum));
		}
	}

	m_iServerLow = srv_lo;
	m_iServerHi  = srv_hi;
}

///////////////////////////////////////////////////////////////////////////
//
// total number of new articles on the server
//   server_lo  -  server_hi
//
void  TNewsGroup::FormatNewArticles(int& iLocalNew, int& iServerNew, int & iLocalTotal)
{
	iLocalNew = iServerNew = 0;

	if (WAIT_OBJECT_0 != ReadLock(3000))
		return;

	CalcLowwaterMark ( m_iServerLow, m_iServerHi );

	int iTmpServerLow;
	try
	{
		//int rangeCount;
		//int rl, rh, i, j;

		// depends on how far we reach back
		iTmpServerLow = max(m_iServerLow, m_iSubscribeLowMark);

		if (kNothing != UtilGetStorageOption(this))
		{
			m_statusMgr.CountNew(iLocalNew, iLocalTotal);
		}

		// -- iLocalNew is calculated

		TRangeSet rsDummy;

		if (-1 == m_iSubscribeLowMark || (0==iTmpServerLow && 0==m_iServerHi))
			// We have never connected. We are not connected. We have no info
			iServerNew = 0;
		else
			// use same function we use to retrieve headers  7-12-96 amc
			iServerNew = adjust_bound_headers_only (iTmpServerLow,
			m_iServerHi,
			40000000,
			NULL,
			false, NULL);
	} // Try
	catch(...)
	{
		UnlockRead();
		throw;
	}
	UnlockRead();
}

// -------------------------------------------------------------------------
BOOL TNewsGroup::NeedsPurge()
{
	// if purging is totally disabled then return False.
	if (FALSE == UtilGetPurgeOnHdrs(this))
		return FALSE;

	TSyncReadLock  auto_loc(this);

	// check the time. fix so it measures between Wed 2pm and Thu 10am
	CTime lastPurge;
	UtilGetLastPurgeTime(this, &lastPurge);
	CTime normalizedPurge(lastPurge.GetYear(), lastPurge.GetMonth(),
		lastPurge.GetDay(), 0, 5, 0);
	CTimeSpan span = CTime::GetCurrentTime() - normalizedPurge;

	int iEvery = UtilGetPurgeOnHdrsEvery(this);
	if (span.GetDays() < iEvery)
	{
//#if defined(_DEBUG)
//		afxDump << GetName() << " PURGE-NO Last purge was "
//			<< normalizedPurge.Format("%m/%d/%Y")
//			<< " purge every " << iEvery << " days\n";
//#endif
		return FALSE;
	}
	else
	{
//#if defined(_DEBUG)
//		afxDump << GetName() << "PURGE-YES Last purge was "
//			<< normalizedPurge.Format("%m/%d/%Y")
//			<< " purge every " << iEvery << " days\n";
//#endif
		return TRUE;
	}
}

void TNewsGroup::SetPurgeTime(const CTime& curPurgeTime)
{
	TSyncWriteLock auto_lock(this);
	m_lastPurgeTime = curPurgeTime;
}

// override a virtual function. called from TNewsGroup::Close - before
// it deletes all article headers
void TNewsGroup::PreDeleteHeaders()
{
	Empty();  // analyze headers and delete Mode-1 headers

	// ok, delete the headers
	TNewsGroupDB::PreDeleteHeaders();
}

BOOL TNewsGroup::IsPurgeOverride()
{
	return (m_byPurgeFlags & TNewsGroup::kPurgeOverride) ? TRUE : FALSE;
}

void TNewsGroup::SetPurgeOverride(BOOL fOver)
{
	if (fOver)
		m_byPurgeFlags |= TNewsGroup::kPurgeOverride;
	else
		m_byPurgeFlags &= ~TNewsGroup::kPurgeOverride;
}

BOOL TNewsGroup::GetPurgeRead()
{
	// 0x2  -  return a strict Bool
	return (m_byPurgeFlags & TNewsGroup::kPurgeRead) ? TRUE : FALSE;
}

void TNewsGroup::SetPurgeRead(BOOL fPrgRead)
{
	if (fPrgRead)
		m_byPurgeFlags |= TNewsGroup::kPurgeRead;
	else
		m_byPurgeFlags &= ~TNewsGroup::kPurgeRead;
}

BOOL TNewsGroup::GetPurgeUnread()
{
	// 0x4  -  return a strict Bool
	return (m_byPurgeFlags & TNewsGroup::kPurgeUnread) ? TRUE : FALSE;
}

void TNewsGroup::SetPurgeUnread(BOOL fPrgUnread)
{
	if (fPrgUnread)
		m_byPurgeFlags |= TNewsGroup::kPurgeUnread;
	else
		m_byPurgeFlags &= ~TNewsGroup::kPurgeUnread;
}

// ------------------------------------------------------------------------
// Purging destroys old articles. It happens before header download, but
//   it may also be totally disabled.
BOOL TNewsGroup::GetPurgeOnHdrs()
{
	// 0x8  -  return a strict Bool
	return (m_byPurgeFlags & TNewsGroup::kPurgeOnHdrs) ? TRUE : FALSE;
}

void TNewsGroup::SetPurgeOnHdrs(BOOL fPrgOnHdrs)
{
	if (fPrgOnHdrs)
		m_byPurgeFlags |= TNewsGroup::kPurgeOnHdrs;
	else
		m_byPurgeFlags &= ~TNewsGroup::kPurgeOnHdrs;
}

BOOL TNewsGroup::GetCompactOnExit()
{
	// 0x10  -  return a strict Bool
	return (m_byPurgeFlags & TNewsGroup::kCompactOnExit) ? TRUE : FALSE;
}

void TNewsGroup::SetCompactOnExit(BOOL fCompact)
{
	if (fCompact)
		m_byPurgeFlags |= TNewsGroup::kCompactOnExit;
	else
		m_byPurgeFlags &= ~TNewsGroup::kCompactOnExit;
}

// ------------------------------------------------------------------------
// Purges articles according to the date stamped in article header
//
//  5-7-98  amc  added bool, so if we call function from loop, only
//                the last one needs to call 'SaveSubscribedGroups'
//
void TNewsGroup::PurgeByDate (bool fWriteOutGroup /*= true*/)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	VERIFY(_CrtCheckMemory());

	BOOL fPurgeRead   = UtilGetPurgeRead (this);
	BOOL fPurgeUnread = UtilGetPurgeUnread (this);

	// user has both criteria disabled
	if (!fPurgeUnread && !fPurgeRead)
		return;

	VERIFY(_CrtCheckMemory());

	int  iReadLimitDays   = UtilGetPurgeReadLimit (this);
	int  iUnreadLimitDays = UtilGetPurgeUnreadLimit (this);
	CTime now = CTime::GetCurrentTime();

	TAutoClose sCloser(this);

	TPersist822Header* p822Hdr;
	TRangeSet cleanRange;
	try
	{
		THeaderIterator it(this);  // gets read lock

		while (it.Next ( p822Hdr ))
		{
			WORD wBits = 0;

			// note - this may fail. We don't want to add a status unit
			//  on the fly, since we can't get a write-lock
			if (iStatusDirect (p822Hdr->GetNumber(), wBits))
			{
				cleanRange.Add ( p822Hdr->GetNumber() );
			}
			else if (wBits & TStatusUnit::kPermanent)
			{
				// this article is "Locked" and "Permanent"
				//  do not add it to the clean range
			}
			else
			{
				CTimeSpan span = now - p822Hdr->GetTime();
				int iArtInt = p822Hdr->GetNumber();
				BOOL fIsRead = ReadRangeHave(iArtInt) || ((wBits & TStatusUnit::kNew)==0);

				int TotHrs = span.GetTotalHours();

				if (fPurgeRead && fIsRead)
				{
					if (span.GetTotalHours() > (iReadLimitDays * 24))
					{
						cleanRange.Add ( iArtInt );
						continue;
					}
				}

				if (fPurgeUnread && !fIsRead)
				{
					if (span.GetTotalHours() > (iUnreadLimitDays * 24))
						cleanRange.Add ( iArtInt );
				}
			}
		} // while loop

		VERIFY(_CrtCheckMemory());

		TArticleCleaner::Clean (this, &cleanRange, TRUE);

		SetPurgeTime( now );   // amc 5-16-96

		if (fWriteOutGroup)
			cpNewsServer->SaveIndividualGroup (this);

		VERIFY(_CrtCheckMemory());

	}
	catch (TException *rTE)
	{
		rTE->PushError (IDS_ERR_PURGEBYDATE, kFatal);
		TException *ex = new TException(*rTE);
		rTE->Delete();
		throw(ex);
	}
}

// ------------------------------------------------------------------------
// called from CMainFrame::OnExpireArticles
void TNewsGroup::ExpireArticles (TExpiryData* pExpiryData)
{
	if (pExpiryData->m_fUseVec)
	{
		expire_articles_list (pExpiryData);
		return;
	}

	// fMissing - TRUE means article is missing from server
	//            FALSE means article is now expired from server & we must
	//                  clean up our side

//#if defined(_DEBUG)
//	{
//		CTime now = CTime::GetCurrentTime();
//		afxDump << now.Format("Start ng::ExpireArticlesMissing %H:%M:%S\n");
//	}
//#endif

	THeapBitmap* pBits;
	TNewsGroup* pNG = this;
	int i, low, high;

	int tot = pExpiryData->m_ArraypBits.GetSize();
	for (int k = 0; k < tot; ++k)
	{
		pBits = pExpiryData->m_ArraypBits.GetAt(k);

		pBits->GetBounds(&low, &high);

		if (pExpiryData->m_fMissing)
		{
			// if the server is a newsfarm we don't build-up the missing rangeset
			//   we will keep asking for article numbers in gaps.
			//

			TServerCountedPtr cpServer(GetParentServer());

			if (cpServer->GetNewsServerFarm() == FALSE)
			{

				// article is not on server
				for (i = low; i <= high; ++i)
				{
					if (FALSE == pBits->IsBitOn(i))
						pNG->MissingRangeAdd(i);      // does internal write lock
				}
			}

		}
		else
		{
			// Case: user has article header, but article body has expired on the server
			bool fFoundOne = false;
			BOOL fPurge = TRUE;

			TRangeSet cleanRange;

			// user can skip ExpirePhase to save download time
			for (i = low; i <= high; ++i)
			{
				// assume none are missing.
				if (pExpiryData->GetAllPresent())
					break;

				if (FALSE == pBits->IsBitOn(i))
				{
					if (pNG->HdrRangeHave(i) && !pNG->TextRangeHave(i))
					{
						fFoundOne = true;
						cleanRange.Add ( i );
					}
					pNG->MissingRangeAdd(i);       // does internal write lock
				}
			}

			// check versus the lowest number on server
			if (fPurge && pNG->CheckHeadersAgainstServerLowRange( &cleanRange ))
				fFoundOne = true;

			if (fFoundOne)
				TArticleCleaner::Clean( pNG, &cleanRange, TRUE );
		}
	}

//#if defined(_DEBUG)
//	{
//		CTime now = CTime::GetCurrentTime();
//		afxDump << now.Format("End ng::ExpireArticlesMissing %H:%M:%S\n");
//	}
//#endif
} // ExpireArticles

// Any articles that fall below the server low number are added to
// the range set
BOOL TNewsGroup::CheckHeadersAgainstServerLowRange(TRangeSet* pRange)
{
	TSyncWriteLock sync(this);
	int rangeCount = m_HdrRange.RangeCount();
	int j, rl, rh;
	BOOL fAddOne = FALSE;
	for (int i = 0; i < rangeCount; i++)
	{
		m_HdrRange.GetRange(i, rl, rh);
		for (j = rl; j <= rh; ++j)
		{
			if (j < m_iServerLow)
			{

				if (m_TextRange.Have (j) || IsStatusBitOn (j, TStatusUnit::kPermanent))
				{
					// keep it
				}
				else
				{
					fAddOne = TRUE;
					StatusMarkRead (j);

					//pRange->Add (j);
				}
			}
		}
	}
	return fAddOne;
}

///////////////////////////////////////////////////////////////////////////
// Results from LISTGROUP - any artInts in the IntVector represent
//   valid articles on the news server
//
void TNewsGroup::expire_articles_list(TExpiryData* pExpiryData)
{
	int nLow, nHigh;
	bool fFoundOne = false;
	TRangeSet  cleanRange;
	TRangeSet  copyHdrRange;

	{
		TSyncReadLock readLock(this);
		copyHdrRange = m_HdrRange;
	}

	int rangeTot = copyHdrRange.RangeCount();

	for (int j = 0; j < rangeTot; ++j)
	{
		copyHdrRange.GetRange(j, nLow, nHigh);
		for (int n = nLow; n <= nHigh; ++n)
		{
			if (TextRangeHave(n) || IsStatusBitOn (n, TStatusUnit::kPermanent))
			{
				// we have the Body... don't care about server

				// this should handle the PROTECTED case, too
			}
			else
			{
				// two cases here
				//   a) the article # is below the lowbound of the server
				//
				//   b) pump did expire phase && the article is not in
				//       the 'listgroup' set
				if ((n < pExpiryData->m_iExpireLow) || !pExpiryData->Have(n))
				{
					fFoundOne = true;
					cleanRange.Add ( n );
				}
			}
		}
	}

	if (fFoundOne)
		TArticleCleaner::Clean (this, &cleanRange, TRUE);
}

BOOL TNewsGroup::NeedsCompact()
{
	if (!UtilGetCompactOnExit(this))
		return FALSE;

	CTime lastCompact;
	int limit = UtilGetCompactOnExitEvery(this);
	UtilGetLastCompactTime(this, &lastCompact);

	CTime normalizedLastCompact(lastCompact.GetYear(),
		lastCompact.GetMonth(),
		lastCompact.GetDay(), 0, 0, 5);

	CTimeSpan span = CTime::GetCurrentTime() - normalizedLastCompact;
	return  (span.GetDays() >= limit);
}

///////////////////////////////////////////////////////////////////////////
//  If you want to start reading at a lower point.  For example:
//  Newsgroup has Articles 1-5000
//  News32 by default reaches back 1000 articles. (you start reading 4001)
//  If you really want to go back further you can reset the low bound
//
int   TNewsGroup::GetLowwaterMark()
{
	return m_iSubscribeLowMark;
}

///////////////////////////////////////////////////////////////////////////
//  If you want to start reading at a lower point.  For example:
//  Newsgroup has Articles 1-5000
//  News32 by default reaches back 1000 articles. (you start reading 4001)
//  If you really want to go back further you can reset the low bound
//
void  TNewsGroup::SetLowwaterMark(int n)
{
	m_iSubscribeLowMark = n;
}

/////////////////////////////////////////////////////////////////////////////
// destroy stuff if we transition from (Mode 2)->(Mode 1) or
// (Mode 3)->(Mode 1)
void TNewsGroup::StorageTransition (EStorageOption eOldStoreMode)
{
	TRangeSet cleanRange;
	BOOL fFoundOne = FALSE;
	int i, j, tot, lo, hi;

	// ACTUALLY the processing is the same.

	if (TNewsGroup::kHeadersOnly == eOldStoreMode ||
		TNewsGroup::kStoreBodies == eOldStoreMode)
	{
		// kill headers for which there are no bodies. If it has a body, it's
		// probably been tagged and retreived explicitly

		// we are going to remove items from m_HdrRange, get write lock
		TSyncWriteLock lock(this);

		tot = m_HdrRange.RangeCount ();
		for (i = 0; i < tot; ++i)
		{
			m_HdrRange.GetRange (i, lo, hi);
			for (j = lo; j <= hi; ++j)
			{
				if (!m_TextRange.Have(j))
				{
					cleanRange.Add ( j );
					fFoundOne = TRUE;
				}
			}
		}
	}

	// static function
	if (fFoundOne)
	{
		// destroy them. but do not mark them as read
		TArticleCleaner::Clean( this, &cleanRange, FALSE );
	}
}

//-------------------------------------------------------------------------
int  TNewsGroup::FlatListLength()
{
	return m_pThreadList->FlatListLength ();
}

//-------------------------------------------------------------------------
int  TNewsGroup::CreateArticleIndex (TArticleIndexList * pIndexList)
{
	m_pThreadList->CreateArticleIndex ( pIndexList );
	return 0;
}

//-------------------------------------------------------------------------
int  TNewsGroup::LoadForArticleIndex (
									  TViewFilter * pFilter,
									  BOOL  fCreateStati,
									  BOOL  fHitServer,
									  TArticleIndexList * pIndexList)        // holds articles on return
{
	if (0 == pFilter)
	{
		AfxMessageBox (IDS_ERR_NOVFLT);
		return 0;
	}

	TArticleBank bank(GetCountedActiveServer());

	int ret = 0;
	TNewsGroup::EStorageOption eStore = UtilGetStorageOption(this);
	if (kHeadersOnly == eStore || kStoreBodies == eStore)
	{
		bank.LoadFromDB (pFilter, this);
	}
	else
	{
		ASSERT(0);

		// 3-9-99  we are dumping mode-1
	}

	// bank is purely temporary
	ReadLock ();
	bank.CreateArticleIndex ( pIndexList );
	UnlockRead ();

	return 0;
}

// --------------------------------------------------------------------------
// override a virtual.  Rules calls this function, so I want to do extra
//   processing here.
void TNewsGroup::PurgeHeader (int artNum)
{
	// do base class functionality
	TNewsGroupDB::PurgeHeader ( artNum );

	TServerCountedPtr cpNewsServer(GetParentServer ());

	// remove the element from the tag list. Cancel queued job if possible.
	cpNewsServer->GetPersistentTags().DeleteTagCancelJob ( GetName(), m_GroupID,
		artNum, TRUE );
}

//-------------------------------------------------------------------------
//
BOOL TNewsGroup::FindNPileContents (TPileIDS* psIDS)
{
	TSyncReadLock readLok(this);
	return m_pThreadList->FindNPileContents (psIDS);
}

//-------------------------------------------------------------------------
BOOL TNewsGroup::GetHighestReadArtInt (int* piHigh)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	TRangeSetReadLock sLock(cpNewsServer,  GetName());
	if (NULL == sLock.m_pRangeSet)
		return FALSE;

	int count = sLock.m_pRangeSet->RangeCount();
	if (0 == count)
		return FALSE;

	int high = 0;
	int highest = 0;
	int low;

	for (int i = 0; i < count; i++)
	{
		sLock.m_pRangeSet->GetRange(i, low, high);
		if (high > highest)
			highest = high;
	}

	*piHigh = highest;

	return TRUE;
}

//-------------------------------------------------------------------------
//  all articles above iNewHi are now "New" again
BOOL TNewsGroup::ResetHighestArticleRead (int iNewHi, int iOldHi)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	ASSERT(iOldHi > iNewHi);

	if (iOldHi > iNewHi)
	{
		// all article numbers Greater than iNewHi will be marked "New"

		// do the Global 'ReadRange'
		{
			TRangeSetWriteLock sLock(cpNewsServer, GetName());
			if (sLock.m_pRangeSet)
			{
				for (int i = iNewHi + 1; i <= iOldHi; ++i)
				{
					sLock.m_pRangeSet->Subtract ( i );
				}
			}
		}

		// do the status bits
		m_statusMgr.ResetHighestArticleRead (iNewHi);
		return TRUE;
	}
	else
		return FALSE;
}

// ------------------------------------------------------------------------
//  1-27-97  amc  Finally wrote this function
const CString& TNewsGroup::GetBestname ()
{
	const CString & rNick = GetNickname();
	if (rNick.IsEmpty())
		return m_name;
	return rNick;
}

// ------------------------------------------------------------------------
// Note pLumper can be null.
int TNewsGroup::build_request_set (
								   TRangeSet * pHaveRange,
								   int        first,
								   int        last,
								   int        iHdrLimit,
								   TRangeSet* pLumper)
{
	TServerCountedPtr cpNewsServer(GetParentServer());

	THeapBitmap seen(first, last);
	int iAdded = 0;

	POINT ptBound;
	ptBound.x = first;
	ptBound.y = last;

	// Turn bit on if # is in header RangeSet
	seen.TurnOn(pHaveRange, &ptBound);

	// Turn bit on if # is in missing RangeSet
	seen.TurnOn(&m_MissingRange, &ptBound);

	// Turn bit on if # is in read RangeSet
	{
		TRangeSetReadLock sLock(cpNewsServer, GetName());

		if (sLock.m_pRangeSet)
			// only need a read lock here
			seen.TurnOn( sLock.m_pRangeSet, &ptBound );
	}

	// fill a rangeset with articleInts to request. We work backwards when
	//   we limit headers, we want to get the most recent ones.
	//
	// do first thru last Inclusive
	for (int j = last ; j >= first; j--)
	{
		if (!seen.IsBitOn(j))
		{
			++iAdded;

			// don't exit loop. Number is useful for NG pane
			if (pLumper && iAdded <= iHdrLimit)
				pLumper->Add (j);
		}
	}
	return iAdded;
}

// ------------------------------------------------------------------------
void TNewsGroup::SetServerName (const CString& strServerName)
{
	TNewsGroupDB::SetServerName (strServerName);

	m_ArtBank.SetServerName (strServerName);
}

// ------------------------------------------------------------------------
void TNewsGroup::SetCustomHeaders (const CStringList &sCustomHeaders)
{
	CopyCStringList (m_sCustomHeaders, sCustomHeaders);
}

// ------------------------------------------------------------------------
// added 4-8-98.  Override a baseclass virtual
void TNewsGroup::MonthlyMaintenance ()
{
	// trim status unit vs TGroupDB::GetHeader
	// check status unit read vs CPM-ReadRange
	ValidateStati ();

	// this is called during the weekly compact.  I want to eliminate
	//   any data that grows w/o bounds
	//
	// - trim  m_MissingRange
	// - trim  Cross Post Mgmt info

	if (m_iServerLow > 0)
	{
		// rationale.  If these ranges are below what the server knows about
		//   why keep tracking them?

		m_MissingRange.DeleteRangesBelow (m_iServerLow);
	}

	if (m_iServerLow > 0)
	{
		// lock down the CrossPost Mgmt ReadRange for this group

		TServerCountedPtr cpNewsServer(GetParentServer());
		TRangeSetWriteLock sWriteLock(cpNewsServer, GetName());

		ASSERT (sWriteLock.m_pRangeSet);

		// this is just a normal RangeSet ptr now

		sWriteLock.m_pRangeSet->DeleteRangesBelow ( m_iServerLow );
	}

	/***
	Scenario:  User has tagged and retrieved article GroupA:Art-1
	(article still unread)
	GroupA moves to Art4-9
	We flush CPM for Art 1,2,3
	In GroupB, we read a crossposted copy.  ProcessXRefs
	will mark the StatusUnit "READ" in GroupB + GroupA
	and also add to the CPinfo again.

	see function ProcessXRefs
	****/
}

// ------------------------------------------------------------------------
// QueryExistArticle --
//               Returns TRUE if an article matching the criteria exists.
//               Used to inspect group before opening the group. Used during
//               'next-unread-body' function
//
//enum EJmpQuery { kQueryNextUnread, kQueryNextLocal, kQueryNextUnreadLocal,
//                      kQueryNextUnreadInThread };
//

BOOL TNewsGroup::QueryExistArticle (EJmpQuery eQuery)
{
	int  iRangeCount, rh, rl;
	int  i , j;                    // loop vars
	BOOL fRet = FALSE;

	switch (eQuery)
	{
	case kQueryDoNotMove:
		return FALSE;

	case kQueryNext:
		{
			return m_statusMgr.GetStatusVectorLength() ? TRUE : FALSE;
		}

	case kQueryNextUnread:
		{
			// check if there's any unread

			WORD   wAndMask = TStatusUnit::kNew ;
			WORD   wXorMask = 0;
			WORD   wMatch   = wAndMask;

			return m_statusMgr.Exist_scanbitmask (wAndMask, wXorMask, wMatch)
				?  TRUE : FALSE;
			break;
		}

	case kQueryNextLocal:
		{
			TSyncReadLock sync (this);

			// check if there's any local article
			iRangeCount = m_TextRange.RangeCount();
			return iRangeCount > 0;
			break;
		}

	case kQueryNextUnreadLocal:
		{
			TSyncReadLock sync (this);

			// check if there's any unread local article
			iRangeCount = m_TextRange.RangeCount();
			for (i = 0; i < iRangeCount; i++)
			{
				m_TextRange.GetRange (i, rl, rh);

				for (j = rl; j <= rh; j++)
				{
					if (!ReadRangeHave (j))
					{
						WORD wStatus = 0;
						if (0 == iStatusDirect ( j, wStatus ) &&
							(wStatus & TStatusUnit::kNew))
						{
							return TRUE;
						}
					}
				}
			}
			return FALSE;
			break;
		}

	case kQueryNextUnreadInThread:
		break;

	case kQueryNextUnreadWatchFree:
		{
			WORD   wAndMask = TStatusUnit::kNew | TStatusUnit::kWatch;
			WORD   wXorMask =                    TStatusUnit::kWatch;
			WORD   wMatch   = wAndMask;

			return m_statusMgr.Exist_scanbitmask (wAndMask, wXorMask, wMatch)
				?  TRUE : FALSE;
			break;
		}

	case kQueryNextUnreadIgnoreFree:
		{
			WORD   wAndMask = TStatusUnit::kNew | TStatusUnit::kIgnore;
			WORD   wXorMask =                     TStatusUnit::kIgnore;
			WORD   wMatch   = wAndMask;

			return m_statusMgr.Exist_scanbitmask (wAndMask, wXorMask, wMatch)
				?  TRUE : FALSE;
			break;
		}

	case kQueryNextUnreadTagFree:
		{
			TSyncReadLock sync (this);

			TServerCountedPtr cpServer( this->GetParentServer() );
			TPersistentTags & sTags = cpServer->GetPersistentTags();

			WORD wStatus;
			BOOL fHaveBody;

			iRangeCount = m_HdrRange.RangeCount();
			for (i = 0; i < iRangeCount; i++)
			{
				m_HdrRange.GetRange (i, rl, rh);
				for (j = rl; j <= rh; j++)
				{
					if (ArticleStatus (j, wStatus, fHaveBody))
					{
						if ((wStatus & TStatusUnit::kNew) &&
							FALSE == fHaveBody &&
							!sTags.FindTag (m_GroupID, j))
							return TRUE;
					}
				}
			}
			return FALSE;
			break;
		}

	} // switch

	return FALSE;
}  // QueryExistArticle

void TNewsGroup::UpdateFilterCount (bool fBoth /* =false */)
{
	try
	{
		gpUserDisplay->SetCountFilter ( m_pThreadList->FlatListLength() );

		if (fBoth)
		{
			int iTotalInGroup = 0 ;
			GetHeaderCount ( iTotalInGroup );
			gpUserDisplay->SetCountTotal ( iTotalInGroup );
		}
	}
	catch(...)
	{

	}
}

////////////////////////////////////////////////////
int  TNewsGroup::GetLocalBodies (CDWordArray & rNums)
{
	TSyncReadLock sync (this);

	int i, j, rl, rh;

	// check if there's any unread local article
	int iRangeCount = m_TextRange.RangeCount();
	for (i = 0; i < iRangeCount; i++)
	{
		m_TextRange.GetRange (i, rl, rh);

		for (j = rl; j <= rh; j++)
		{
			rNums.Add (j);
		}
	}

	return 0;
}
