/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: statvec.cpp,v $
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:56  richard_wood
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
#include "mplib.h"

#include "statvec.h"
#include "statchg.h"
#include "critsect.h"

// this is gross
#include "news.h"
#include "newsdoc.h"

IMPLEMENT_SERIAL(TStatusMgr,  PObject, 1);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TStatusUnit::TStatusUnit(void)
{
	m_articleInt = 0;
	m_status = kNew;
}

TStatusUnit::TStatusUnit(int artInt)
{
	m_articleInt = artInt;
}

TStatusUnit::TStatusUnit(const TStatusUnit& src)
: m_timeRead(src.m_timeRead)
{
	m_articleInt      =  src.m_articleInt;
	m_status          =  src.m_status;
}

TStatusUnit&
TStatusUnit::operator= (const TStatusUnit& rhs)
{
	if (this == &rhs) return *this;
	m_timeRead     =  rhs.m_timeRead;
	m_articleInt   =  rhs.m_articleInt;
	m_status       =  rhs.m_status;
	return *this;
}

///////////////////////////////////////////////////////////////////////////
//
//
TStatusMgr::TStatusMgr(void)
{
	m_lOuterGroupID = 0;  // used for the status deltas
}

TStatusMgr::~TStatusMgr(void)
{
}

void  TStatusMgr::Serialize(CArchive& ar)
{
	PObject::Serialize ( ar );
	m_StatusSearch.Serialize ( ar );
}

// ------------------------------------------------------------------------
// Returns 0 if we created a new statusunit, 1 if unit existed already
int TStatusMgr::Add_New(int articleNumber, WORD status)
{
	return m_StatusSearch.AddItem(articleNumber, status);
}

void TStatusMgr::Collect(
						 WORD status,
						 TRangeSet* pRangeSet,
						 int & found,
						 int & total)
{

	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	int sz = m_StatusSearch.GetSize();
	found = 0;
	total = 0;
	const StatusUnitI* pU = m_StatusSearch.GetData();
	for (int i = 0; i < sz; ++i, ++pU)
	{
		total++;
		if (status & TStatusUnit::kOut)
		{
			if (pU->m_status & TStatusUnit::kOut)
			{
				found ++;
				pRangeSet->Add ( pU->m_articleInt );
			}
		}
		else if (status & TStatusUnit::kSent)
		{
			if (pU->m_status & TStatusUnit::kSent)
			{
				found++;
				pRangeSet->Add ( pU->m_articleInt );
			}
		}
		else if ((pU->m_status & status) == status)
		{
			// don't show Outbox or Sent messages
			if ((pU->m_status & TStatusUnit::kSent) ||
				(pU->m_status & TStatusUnit::kOut))
				;
			else
			{
				found++;
				pRangeSet->Add ( pU->m_articleInt );
			}
		}
	}
}

// gather Articles that have been READ and are older than TIME
void TStatusMgr::CollectOld(CTime      threshold,
							TRangeSet* pRangeSet,
							BOOL       fIgnoreUndeletable)
{
	time_t tmThreshold = threshold.GetTime();

	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	int sz = m_StatusSearch.GetSize();
	for (int i = 0; i < sz; ++i, ++pU)
	{
		if (!(pU->m_status & TStatusUnit::kNew))
		{
			if (fIgnoreUndeletable)
			{
				if ((pU->m_status & TStatusUnit::kPermanent) == TStatusUnit::kPermanent)
					continue;
			}
			if (pU->m_timeRead < tmThreshold)
				pRangeSet->Add (pU->m_articleInt);
		}
	}
}

// keep limbo as backup
void TStatusMgr::Destroy(int artInt)
{
	TStatusUnit unit;

	m_StatusSearch.RemoveItem(artInt, &unit);

	// m_limbo.Add (unit);
}

BOOL TStatusMgr::Lookup (int& key, TStatusUnit& unit)
{
	int idx = -1;

	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	BOOL fFound = m_StatusSearch.Find (key, idx);
	if (fFound)
	{
		const StatusUnitI* pUnit = m_StatusSearch.GetData( idx );
		unit.m_status     = pUnit->m_status;
		unit.m_articleInt = pUnit->m_articleInt;
		unit.m_timeRead   = CTime(pUnit->m_timeRead);
	}
	return fFound;
}

void  TStatusMgr::SetAt (int key, TStatusUnit& unit)
{
	int idx = -1;

	int artInt = 0;
	WORD wOldStat, wNewStat;

	{
		TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

		BOOL fFound = m_StatusSearch.Find (key, idx);
		if (fFound)
		{
			const StatusUnitI* pOne = m_StatusSearch.GetData(idx);

			if (pOne->m_status != unit.m_status)
			{
				artInt   = pOne->m_articleInt;
				wOldStat = pOne->m_status;
				wNewStat = unit.m_status;
			}
			m_StatusSearch.SetItemByIndex(idx, unit.m_status, unit.m_timeRead.GetTime());
		}
		else
		{
			m_StatusSearch.AddItem(key, unit.m_status, unit.m_timeRead.GetTime(), TRUE);
		}
	}
	if (artInt)
		status_delta (artInt, wOldStat, wNewStat);
}

///////////////////////////////////////////////////////////////////////////
//
// mark read and... note the current time (for purging)
// return -1 for not found
//         0 for ok
//         1 for null operation
//  5-15-96  amc  no longer throws exception
//
int TStatusMgr::StatusMarkRead(int artInt)
{
	return status_markread (artInt, TRUE);
}

int TStatusMgr::StatusMarkUnread(int artInt)
{
	return status_markread (artInt, FALSE);
}

//-------------------------------------------------------------------------
//
int TStatusMgr::status_markread(int artInt, BOOL fMarkRead)
{
	extern CTime gTimeAncient;
	BOOL fCreateDelta = FALSE;
	WORD oldStat, newStat;
	int  iRet;

	int idx = -1;

	{
		TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

		BOOL fFound = m_StatusSearch.Find(artInt, idx);
		if (!fFound)
		{
			return -1;
		}

		const StatusUnitI* pU = m_StatusSearch.GetData( idx );

		if (fMarkRead)
		{
			if (pU->m_status & TStatusUnit::kNew)
			{
				oldStat = pU->m_status;
				newStat = oldStat & ~TStatusUnit::kNew;

				fCreateDelta = TRUE;

				m_StatusSearch.SetItemByIndex(idx, newStat,
					CTime::GetCurrentTime().GetTime());
				iRet = 0;
			}
			else
				iRet = 1;
		}
		else
		{
			// make it new
			if (!(pU->m_status & TStatusUnit::kNew))
			{
				oldStat = pU->m_status;
				newStat = oldStat | TStatusUnit::kNew;

				fCreateDelta = TRUE;

				m_StatusSearch.SetItemByIndex(idx, newStat, gTimeAncient.GetTime());
				iRet = 0;
			}
			else
				iRet = 1;
		}
	} // release crit section

	if (fCreateDelta)
		status_delta (artInt, oldStat, newStat);
	return iRet;
}

void TStatusMgr::status_delta (int artInt, WORD oldStatus, WORD newStatus)
{
	if (0 == m_lOuterGroupID)
		return;

	TStatusChg change(m_lOuterGroupID, artInt, oldStatus, newStatus);
	CNewsApp* pApp = (CNewsApp*) AfxGetApp();
	CNewsDoc* pNewsDoc = pApp->GetGlobalNewsDoc();

	// if pNewsDoc is NULL, we're in the process of shutting down (we could
	// be called by a worker thread after the newsdoc is destroyed)
	if (!pNewsDoc)
		return;

	pNewsDoc->AddArticleStatusChange ( &change );
}

void TStatusMgr::DestroyAll(void)
{
	m_StatusSearch.DestroyAll();
}

void TStatusMgr::CountNew(int& totalNew, int& total)
{
	int junk1, junk2;
	CountNew_andAbove(0, totalNew, total, junk1, junk2);
}

///////////////////////////////////////////////////////////////////////////
//
// pass in an integer threshold.  Count new articles that are >= threshold
// and count the new articles/total articles

void TStatusMgr::CountNew_andAbove(int iLowMark, int& iCountNew, int& total,
								   int& threshNew, int& threshTotal)

{
	iCountNew = threshNew = threshTotal = 0;

	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	total = m_StatusSearch.GetSize();
	for (int i = 0; i < total; ++i, ++pU)
	{
		if (pU->m_status & TStatusUnit::kNew)
		{
			if (0 == (pU->m_status & (TStatusUnit::kOut |
				TStatusUnit::kSent |
				TStatusUnit::kSending)))
			{
				++ iCountNew;
				if (pU->m_articleInt >= iLowMark)
					++ threshNew;

			}
		}
		if (pU->m_articleInt >= iLowMark)
			++threshTotal;
	}
}

//-------------------------------------------------------------------------
// All articles above "iNewHi" become New again
void TStatusMgr::ResetHighestArticleRead (int iNewHi)
{
	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	m_StatusSearch.ResetHighestArticleRead ( iNewHi );
}

//-------------------------------------------------------------------
void TStatusMgr::AllArticleNumbers (LONGV & vnum)
{
	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	int total = m_StatusSearch.GetSize();
	for (int i = 0; i < total; ++i, ++pU)
		vnum.insert(vnum.end(), pU->m_articleInt);
}

//-------------------------------------------------------------------------
bool TStatusMgr::Exist_scanbitmask (WORD wAndMask, WORD wXorValue, WORD wMatch)
{
	TEnterCSection critSect(m_StatusSearch.GetpCriticalSection());

	const StatusUnitI* pU = m_StatusSearch.GetData();
	int total = m_StatusSearch.GetSize();
	for (int i = 0; i < total; ++i, ++pU)
	{
		if (wMatch == ((pU->m_status  &  wAndMask) ^ wXorValue))
			return true;
	}

	return false;
}

