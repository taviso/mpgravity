/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: pumpjob.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/18 22:05:02  richard_wood
/*  Refactored XOVER and XHDR commands so they fetch item data in batches of 300 (or less) if we want > 300 articles.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:41  richard_wood
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
#include "pumpjob.h"
#include "newsgrp.h"
#include "nglist.h"
#include "custmsg.h"
#include "fetchart.h"      // TFetchArticle
#include "servcp.h"        // TServerCountedPtr

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#if defined(_DEBUG)
// override a virtual
void TPumpJob::Dump(CDumpContext& dc) const
{
	// base class version
	CObject::Dump(dc);

	dc <<  "TPumpJob";
}

#endif

TPumpJob::TPumpJob()
: m_eJobDone(TPumpJob::kDoneNothing)
{
}

// copy ctor
TPumpJob::TPumpJob(const TPumpJob& src)
{
	m_kJobType = src.m_kJobType;
	m_eJobDone = src.m_eJobDone;
}

// assignment
TPumpJob& TPumpJob::operator=(const TPumpJob& rhs)
{
	if (&rhs == this)
		return *this;

	m_kJobType = rhs.m_kJobType;
	m_eJobDone = rhs.m_eJobDone;

	return *this;
}

TPumpJobPingGroup::TPumpJobPingGroup(const CString & grp)
: m_NewsGroup(grp)
{
	m_kJobType = kJobPingGroup;
	m_fOK = FALSE;
	m_OldNew = 0x1;
}

// copy ctor
TPumpJobPingGroup::TPumpJobPingGroup(const TPumpJobPingGroup& src)
: m_NewsGroup(src.m_NewsGroup), m_Ack(src.m_Ack)
{
	fRefreshUI = src.fRefreshUI;
	m_iRet     = src.m_iRet;
	m_OldNew   = src.m_OldNew;
	m_fOK      = src.m_fOK;
	m_GroupFirst = src.m_GroupFirst;
	m_GroupLast = src.m_GroupLast;
}

// assignment
TPumpJobPingGroup& TPumpJobPingGroup::operator=(const TPumpJobPingGroup& rhs)
{
	if (&rhs == this) return *this;

	TPumpJob::operator=(rhs);

	fRefreshUI = rhs.fRefreshUI;
	m_iRet     = rhs.m_iRet;
	m_OldNew   = rhs.m_OldNew;
	m_fOK      = rhs.m_fOK;
	m_GroupFirst = rhs.m_GroupFirst;
	m_GroupLast = rhs.m_GroupLast;
	m_Ack       = rhs.m_Ack;
	m_NewsGroup = rhs.m_NewsGroup;
	return *this;
}

void TPumpJobPingGroup::SetGroupResults(BOOL fOK, int ret, int first, int last)
{
	m_fOK        = fOK;
	m_iRet       = ret;
	m_GroupFirst = first;
	m_GroupLast  = last;
}

// for entering group
TPumpJobEnterGroup::TPumpJobEnterGroup(const CString& grp,
									   TPumpJob::EPumpJobDone eDone,
									   BOOL      fGetAll,
									   int       iHdrLimit)
									   : TPumpJobPingGroup(grp)
{
	m_kJobType = kJobGroup;
	m_eJobDone = eDone;
	//m_hEventDone = hEventDone;
	m_hEventDone = 0;

	m_fGetAll = fGetAll;
	m_iHdrLimit = iHdrLimit;
}

///////////////////////////////////////////////////////////////////////////
// If program is waiting to enter a Mode1 NG that is no longer on the server
//
BOOL TPumpJobEnterGroup::Release()
{
	if (m_hEventDone)
	{
		SetEvent(m_hEventDone);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
// to retreive article
TPumpJobArticle::TPumpJobArticle(
								 const CString &  grp,
								 LONG             groupID,
								 const CString &  subject,
								 CPoint &          ptPartID,
								 int              artnum,
								 int              bodyLines,
								 DWORD            jobFlags,
								 TArticleHeader * pHeader,     // we make a copy for ourselves
								 TFetchArticle  * pFetchArt)
								 :  m_NewsGroup(grp),
								 m_GroupID(groupID),
								 m_subject(subject),
								 m_ptPartID(ptPartID),
								 m_artNumber(artnum),
								 m_Lines(bodyLines),
								 m_dwJobFlags(jobFlags),
								 m_pFetchArticle(pFetchArt)
{
	// make our own copy

	if (pHeader)
		m_pArtHeader = new TArticleHeader(*pHeader);
	else
		m_pArtHeader = NULL;
}

///////////////////////////////////////////////////////////////////////////
TPumpJobArticle::~TPumpJobArticle()
{
	delete m_pArtHeader;
	m_pArtHeader = 0;

	if (m_pFetchArticle)
	{
		m_pFetchArticle -> DestroySelf ();
		m_pFetchArticle = NULL;
	}
}

TFetchArticle * TPumpJobArticle::ReleaseFetchObject()
{
	TFetchArticle * pTmp = m_pFetchArticle;

	m_pFetchArticle = 0;

	return pTmp;
}

///////////////////////////////////////////////////////////////////////////
// override a virtual
TPumpJob *  TPumpJobArticle::Resubmit ()
{
	TArticleHeader * pHdr = m_pArtHeader;

	m_pArtHeader = 0;

	TPumpJobArticle * pCopy = new TPumpJobArticle (m_NewsGroup,
		m_GroupID,
		m_subject,
		m_ptPartID,
		m_artNumber,
		m_Lines,
		m_dwJobFlags,
		pHdr,
		ReleaseFetchObject());
	return pCopy;
}

///////////////////////////////////////////////////////////////////////////
CString TPumpJobArticle::FormatStatusString ()
{
	CString strStatusBar;

	if (m_ptPartID.y > 0)
	{

		if (1 == m_ptPartID.y && 1 == m_ptPartID.x)
		{
			strStatusBar.Format ("Fetching %s (%d lines)",
				LPCTSTR(m_subject), GetBodyLines());
		}
		else
		{
			// for jobs that have the info (ex: decode jobs) display
			//  part info

			strStatusBar.Format ("Fetching part %d of %d  %s (%d lines)",
				m_ptPartID.x,
				m_ptPartID.y,
				LPCTSTR(m_subject), GetBodyLines());
		}

	}
	else
	{
		strStatusBar.Format ( IDS_FETCH_STATUS, LPCTSTR(this->GetGroup()),
			this->GetArtInt(), GetBodyLines() );
	}
	return strStatusBar;
}

///////////////////////////////////////////////////////////////////////////
// to post an article
TPumpJobPost::TPumpJobPost(LONG artInt, CMemFile* pmemFile,
						   const CString& groupName, bool fNewTopic)
						   : m_pmemFile(pmemFile), m_groupName(groupName), m_fNewTopic(fNewTopic)
{
	m_ArtInt  = artInt;

	m_fOK = FALSE;
	m_kJobType = kJobPost;
}

TPumpJobPost::~TPumpJobPost(void)
{
	delete m_pmemFile;
}

///////////////////////////////////////////////////////////////////////////
//
// overview job - ctor 1/2 with RangeSet
//
//
TPumpJobOverview::TPumpJobOverview(const CString & group, LONG GroupID,
								   int iGroupLow,
								   TRangeSet * pRange,
								   bool fExpire,
								   TRangeSet* pPingRange,
								   TPumpJob::EPumpJobDone eDone)
								   : m_NewsGroup(group), m_pRange(pRange), m_pPingRange(pPingRange)
{
	m_iExpireLow = iGroupLow;
	m_eJobDone = eDone;
	m_low = m_high = 0;
	m_kJobType = kJobOverview;
	m_GroupID  = GroupID;
	m_fExpirePhase = fExpire;
}

//////////////////////////////////////////////////////////////////////////
// overview job - with low and high
//
//
TPumpJobOverview::TPumpJobOverview(const CString & group, LONG GroupID, int low, int high)
: m_NewsGroup(group)
{
	m_low  = low;
	m_high = high;
	m_pRange = 0;
	m_pPingRange = 0;
	m_kJobType = kJobOverview;
	m_GroupID  = GroupID;

	m_iExpireLow = 0; // unused
	m_fExpirePhase = true;  // inactive
}

TPumpJobOverview::~TPumpJobOverview()
{
	delete m_pRange;
	delete m_pPingRange;
}

TPumpJob * TPumpJobOverview::Resubmit ()
{
	TPumpJobOverview * pCopy =
		new TPumpJobOverview (m_NewsGroup, m_GroupID,
		m_iExpireLow,
		m_pRange,
		m_fExpirePhase,
		m_pPingRange,
		m_eJobDone);

	// we have transferred ownership
	m_pPingRange = 0;
	m_pRange = 0;

	return pCopy;
}

TPumpJobBigList::TPumpJobBigList(BOOL fRecent, WPARAM wParam,
								 CTime& prevCheck)
								 :  m_prevCheck(prevCheck)
{
	m_fRecent = fRecent;
	m_wParam = wParam;
}

TPumpJobBigList::~TPumpJobBigList(void)
{
}

// EOF
