/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thredlst.cpp,v $
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
/*  Revision 1.5  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:52:05  richard_wood
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

// Latest Profiling + Tuning changes
//
//Changed Link() algorithm
//
//use m_Stage as a MAP from MsgID -> ArtNode ptr,  but
//then after linking, get rid of it.
//
//ArtUIData - use zoo to enforce 'From' refcounting
//ArtUIData - point to LPCTSTR subject in TArtNode
//ArtNode - get rid of POSITION  m_sTraversalPos
//Article, Thread  change  BOOL bZeroPresent to  'bool'
//ThreadPile - remove m_subject string
//ThreadList - get rid of m_rootArray.

#include "thredlst.h"
#include "artnode.h"
#include "globals.h"
#include "tmutex.h"
#include "killprfx.h"      // fnKillPrefix
#include "newsgrp.h"
#include "tglobopt.h"
#include "artact.h"

#include "primes.h"
#include "idxlst.h"        // TArticleIndexList
#include "rgsys.h"         // GetSubjectMatchCutoff
#include "ngutil.h"        // UtilGetStorageOption
#include "rgswit.h"        // GetRegSwitch();
#include "vfilter.h"       // TViewFilter
#include "server.h"
#include "utilpump.h"
#include "dlgquickfilter.h"

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

unsigned int CRC32(unsigned char *data, int len);

extern TGlobalOptions* gpGlobalOptions;

static int From_Compare_H (TArticleHeader *pHdr1, TArticleHeader *pHdr2);
static int Line_Compare_H (TArticleHeader *pHdr1, TArticleHeader *pHdr2);
static int Time_Compare_H (TArticleHeader *pHdr1, TArticleHeader *pHdr2)
{
	const CTime & tm1 = pHdr1->GetTime();
	const CTime & tm2 = pHdr2->GetTime();
	if (tm1 < tm2)
		return -1;
	if (tm1 == tm2)
		return 0;
	return 1;
}
static int Score_Compare_H (TArticleHeader *pHdr1, TArticleHeader *pHdr2);

void fnCleanPiles (std::vector<TThreadPile*> & rPiles);
static int fnCollectNodes(TArtNode * pArtNode, void* pV);

//---------------------------------------------------------------------------
// I want to compare subjects, but only out to a certain point (20 chars?)
// for instance
//   "Babylon 5 Episode 27"  should match
//   "Babylon 5 Episode 27 (Possible Spoilers)"          7-17-96 amc
UINT fnShortCRC (LPCTSTR lszSubj, int strLen)
{
	int iLen = min(strLen, gpGlobalOptions->GetRegSystem()->GetSubjectMatchCutoff());
	return CRC32((BYTE*)lszSubj, iLen);
}

TThreadList::TThreadList(void)
{
	m_pBank = 0;
}

TThreadList::~TThreadList(void)
{
	EmptyAll();
}

///////////////////////////////////////////////////////////////////////////
int TThreadList::EmptyAll(void)
{
	// big cleanup of the articles stored in our staging area
	if (m_pBank)
		m_pBank->Empty ();

	return CleanUp();
}

int TThreadList::EmptyList(void)
{
	return CleanUp();
}

///////////////////////////////////////////////////////////////////////////
// CleanUp -- we are cleaning up our assorted collections. The mass of
//    Artnodes are destroyed via 'm_pBank->Empty ()'
int TThreadList::CleanUp(void)
{
	// pile destructor is basic
	fnCleanPiles ( m_rPiles );

	// flatList holds ArtNodes, so we can sort articles by Lines, etc
	m_vecFlatList.clear();

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Link --
//
// note: WinVN's algorithm is documented in headarry.cpp
//
///////////////////////////////////////////////////////////////////////////
void TThreadList::Link (TNewsGroup* pNG, TViewFilter* pFilter)
{
	//TRACE0("Start threading\n");

	int         ret;
	NodeList    sNodeList;

	// also add to the sortable one-dimensional list.
	ret = LinkByReference ( pNG, pFilter, &sNodeList );
	if (ret)
		return;

	if (pFilter->getNoThread())
	{
		POSITION pos = sNodeList.GetHeadPosition();
		while (pos)
		{
			m_vecFlatList.push_back ( sNodeList.GetNext(pos) );
		}
	}
	else
	{
		// We started with  A B C.   (C is a child of B)
		// after LinkByReference,  C is connected to B
		// sNodeList contains A and B. (aka the rejects)
		// Note that sNodeList essentially still contains our Universe

		ret = AggregateCommonRoot_or_Subject ( &sNodeList );
		if (ret)
			return;

		HeapifyPiles ();

		PropagateScoresDownThePiles ();

		// get rid of m_Stage and fill up m_setNodes
		m_pBank->PuntLinkInformation ();

		// expects to remove items from m_setNodes
		WalkPilesDelete ( pNG, pFilter );

		//  setup  flatlist  after we do the subtractive operation in WalkPilesDelete
		AddPilesToFlat ();

		ASSERT(m_pBank->SanityCheck_CountNodes() == m_vecFlatList.size());
	}

	VEC_NODES(m_vecFlatList).swap(m_vecFlatList);  // shrink to fit

	// Done:
	//TRACE0("Done threading\n");
}

///////////////////////////////////////////////////////////////////////////
//  Basically marks everything in the ThreadList as Read, but does
//  not empty the m_Stage.  (the filter set might be showing new & read)
//
//  Note: Returns a weirdo value 'fKeepsTags'
BOOL TThreadList::CatchUpArticles(TNewsGroup* pNG, KThreadList::EXPost eXPost)
{
	BOOL fKeepTags = gpGlobalOptions->GetRegSwitch()->GetCatchUpKeepTags();

	// artbank.cpp has a few areas that Lock-Bank, the use pNG->ReadLock
	//   so to maintain order, Lock-Bank 1st.
	m_pBank->Lock ();
	{
		TSyncWriteLock writeLock(pNG);

		CString   strKey;
		TArtNode* pArtNode;

		// this filter is all inclusive
		TViewFilter sFilter(0);
		if (fKeepTags)
		{
			// refine filter. All articles that are Untagged
			sFilter.SetTag (TStatusUnit::kNo);
		}

		STLMapArtNode::iterator it;
		STLMapArtNode::iterator itEnd;

		TServerCountedPtr cpNewsServer(pNG->GetParentServer());
		TPersistentTags & rTags = cpNewsServer->GetPersistentTags();

		m_pBank->GetIteratorEx ( it, itEnd );

		while (it != itEnd)
		{
			pArtNode = *it++;

			if (sFilter.FilterMatch (pArtNode->GetpRandomHeader(), pNG, &rTags ))
			{
				int artInt = pArtNode->GetpRandomHeader()->GetNumber();

				// change from New to Read.
				try
				{
					pNG->StatusBitSet(artInt, TStatusUnit::kNew, FALSE);
				}
				catch(TException *pE) {pE->Delete(); }

				// add to READ pile. don't accumulate dirty bits.
				pNG->ReadRangeAdd ( pArtNode->GetpRandomHeader(), eXPost );
			}
		}
		pNG->SetDirty();

	}  // give up Write-Lock
	m_pBank->Unlock ();

	// save the newsgroup object (by saving out all newsgroups)
	//  NOTE this needs a ReadLock on the NG
	pNG->TextRangeSave();

	return fKeepTags;
}

///////////////////////////////////////////////////////////////////////////
//  For this Header, return the Pile, the Thread and the Node that
//  contain it
BOOL TThreadList::FindThreadPile (
								  TArticleHeader* pArtHdr,
								  TThreadPile*& pPile,
								  TThread*& rpThread,
								  TArtNode*& rpNode )
{
	TThread* pThread  = 0;
	BOOL fRet = FALSE;

	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	for (; it != m_rPiles.end(); it++)
	{
		if ((*it)->Have ( pArtHdr, rpThread, rpNode ))
		{
			fRet = TRUE;
			pPile = (*it);
			break;
		}
	}
	return fRet;
}

static int siftdown(int l, unsigned u, CObArray* pArray, P_COMPAREFUNC pfnSort);
int HeapSort (int n, CObArray* pArray, P_COMPAREFUNC pfnCompare)
{
	int i;
	if (0 == n)
		return 0;

	for (i = (n-1) / 2; i>=0; i--)
		siftdown(i, n-1, pArray, pfnCompare);
	for (i = n-1; i>=1; i--)
	{
		//swap(0, i);
		{
			CObject * pTemp = pArray->GetAt(i);
			pArray->SetAt(i, pArray->GetAt(0));
			pArray->SetAt(0, pTemp);
		}
		siftdown(0, i-1, pArray, pfnCompare);
	}

#if defined(_DEBUG)
	for (i = 1; i < n; ++i)
	{
		int ret;
		ret = pfnCompare(pArray->GetAt(i-1), pArray->GetAt(i));

		ASSERT( ret == 0 || ret == -1 );
	}
#endif
	return 0;
}

// ARRAY[u] is still within Bounds
int siftdown(int l, unsigned int u, CObArray* pArray,  P_COMPAREFUNC pfnCompare)
{
	unsigned int i;
	unsigned int c;
	int      cmpStat;

	i = l;
	while (1)
	{
		c = i + i + 1;

		if (c > u)
			break;
		if (c+1 <= u)
		{
			//if (x[c+1] > x[c])
			cmpStat = pfnCompare(pArray->GetAt(c+1), pArray->GetAt(c));
			if (cmpStat > 0)
				c++;
		}
		///if (x[i] >= x[c])
		cmpStat = pfnCompare(pArray->GetAt(i), pArray->GetAt(c));
		if (cmpStat >= 0)
			break;
		{
			//pSwpArray->Swap(c, i);
			CObject* pTmp;
			pTmp = pArray->GetAt(i);
			pArray->SetAt(i, pArray->GetAt(c));
			pArray->SetAt(c, pTmp);
		}
		i = c;
	} // while loop

	return 0;
}

// #########

// --------------------------------------------------------------------------
static int fnStrCompareSubjectSansRE(LPCTSTR p1, LPCTSTR p2)
{
	if (('r' == p1[0] || 'R' == p1[0]) &&
		('e' == p1[1] || 'E' == p1[1]) &&
		(':' == p1[2]) &&
		(' ' == p1[3]))  p1 += 4;
	if (('r' == p2[0] || 'R' == p2[0]) &&
		('e' == p2[1] || 'E' == p2[1]) &&
		(':' == p2[2]) &&
		(' ' == p2[3]))  p2 += 4;
	int ret = lstrcmpi (p1, p2);

	return ret;
}

static int Score_Compare_H(TArticleHeader *pHdr1, TArticleHeader *pHdr2)
{
	TArticleHeader* pArtHdr;
	long cnt1 = 0;
	long cnt2 = 0;
	if (pHdr1->IsArticle())
	{
		pArtHdr = pHdr1->CastToArticleHeader();
		cnt1 = pArtHdr->GetScore();
	}
	if (pHdr2->IsArticle())
	{
		pArtHdr = pHdr2->CastToArticleHeader();
		cnt2 = pArtHdr->GetScore();
	}

	if (cnt1 == cnt2)
		return Time_Compare_H (pHdr1, pHdr2);

	if (cnt1 < cnt2)
		return -1;
	return 1;
}

// used by Indicator_Compare
static TNewsGroup* sgpSortNG;

static int Indicator_Compare(TArticleHeader *pH1, TArticleHeader *pH2)
{
	int ArtInt1 = pH1->GetNumber();
	int ArtInt2 = pH2->GetNumber();

	WORD wArtStat1 = 0;
	WORD wArtStat2 = 0;

	int ret1 = sgpSortNG->iStatusDirect(ArtInt1, wArtStat1);
	int ret2 = sgpSortNG->iStatusDirect(ArtInt2, wArtStat2);
	if (ret1 && ret2)
		return Time_Compare_H ( pH1, pH2 );

	if (ret1)
		return -1;      // failure to get status of First
	if (ret2)
		return 1;       // failure to get status of 2nd

	BYTE byI1 = (wArtStat1 & TStatusUnit::kImportant) ? BYTE(1) : BYTE(0);
	BYTE byI2 = (wArtStat2 & TStatusUnit::kImportant) ? BYTE(1) : BYTE(0);

	if (byI1 == byI2)
		return Time_Compare_H ( pH1, pH2 );
	if (byI1 > byI2)
		return 1;
	return -1;

	// important bit is the same - use Time as secondary index
}

///////////////////////////////////////////////////////////////////////////
// STL sort helper objects
class TSortBySubject
{
public:
	bool operator () (TArtNode * pNode1, TArtNode * pNode2)
	{
		TArticleHeader* pHdr1 = pNode1->GetpRandomHeader();
		TArticleHeader* pHdr2 = pNode2->GetpRandomHeader();

		LPCTSTR p1 = pHdr1->GetSubject();
		LPCTSTR p2 = pHdr2->GetSubject();

		int ret = fnStrCompareSubjectSansRE (p1, p2);

		if (-1 == ret)
			return true;
		if (1 == ret)
			return false;

		// subjects are equal, check time
		if (pHdr1->GetTime() < pHdr2->GetTime())
			return true;

		return false;
	}
};

class TSortByFrom
{
public:
	bool operator () (TArtNode * pNode1, TArtNode * pNode2)  // less-function
	{
		TArticleHeader* pHdr1 = pNode1->GetpRandomHeader();
		TArticleHeader* pHdr2 = pNode2->GetpRandomHeader();

		CString phr1, addr1, phr2, addr2;
		pHdr1->ParseFrom ( phr1, addr1 );
		if (phr1.IsEmpty())
			phr1 = addr1;
		pHdr2->ParseFrom ( phr2, addr2 );
		if (phr2.IsEmpty())
			phr2 = addr2;

		int stat = phr1.CompareNoCase ( phr2 );

		if (0 < stat)
			return true;
		if (0 > stat)
			return false;

		// froms are equal, check time
		if (pHdr1->GetTime() < pHdr2->GetTime())
			return true;

		return false;
	}
};

class TSortByTime
{
public:
	bool operator () (TArtNode * pNode1, TArtNode * pNode2)
	{
		TArticleHeader* pHdr1 = pNode1->GetpRandomHeader();
		TArticleHeader* pHdr2 = pNode2->GetpRandomHeader();

		const CTime& t1 = pHdr1->GetTime();
		const CTime& t2 = pHdr2->GetTime();

		if (t1 < t2)
			return true;

		if (t1 > t2)
			return false;

		LPCTSTR p1 = pHdr1->GetSubject();
		LPCTSTR p2 = pHdr2->GetSubject();

		int ret = fnStrCompareSubjectSansRE (p1, p2);

		if (ret == -1)
			return true;
		else
			return false;
	}
};

class TSortByLines
{
public:
	bool operator () (TArtNode * pNode1, TArtNode * pNode2)
	{
		int iL1 = pNode1->GetLines();
		int iL2 = pNode2->GetLines();
		if (iL1 < iL2)
			return true;
		else if (iL1 > iL2)
			return false;

		// lines are equal, check time
		TArticleHeader* pHdr1 = pNode1->GetpRandomHeader();
		TArticleHeader* pHdr2 = pNode2->GetpRandomHeader();

		if (pHdr1->GetTime() < pHdr2->GetTime())
			return true;

		return false;
	}
};

class TSortByScores
{
public:
	bool operator () (TArtNode * pNode1, TArtNode * pNode2)
	{
		TArticleHeader* pHdr1 = pNode1->GetpRandomHeader();
		TArticleHeader* pHdr2 = pNode2->GetpRandomHeader();

		return -1 ==  Score_Compare_H (pHdr1, pHdr2);
	}
};

class TSortByIndicator
{
public:
	bool operator () (TArtNode * pNode1, TArtNode * pNode2)
	{
		TArticleHeader* pHdr1 = pNode1->GetpRandomHeader();
		TArticleHeader* pHdr2 = pNode2->GetpRandomHeader();

		return -1 == Indicator_Compare(pHdr1, pHdr2);
	}
};

// #####################################

BOOL TThreadList::SubjSortFlatlist()
{
	// sort by subject
	stable_sort (m_vecFlatList.begin(),
		m_vecFlatList.end(),
		TSortBySubject());

	m_iSortField = SORT_BY_SUBJECT;
	return TRUE;
}

BOOL TThreadList::FromSortFlatlist()
{
	// sort by sender.
	stable_sort (m_vecFlatList.begin(),
		m_vecFlatList.end(),
		TSortByFrom() );

	m_iSortField = SORT_BY_AUTHOR;
	return TRUE;
}

BOOL TThreadList::DateSortFlatlist()
{
	// sort by time.
	stable_sort (m_vecFlatList.begin(),
		m_vecFlatList.end(),
		TSortByTime() );

	m_iSortField = SORT_BY_DATE;
	return TRUE;
}

BOOL TThreadList::LineSortFlatlist()
{
	stable_sort (m_vecFlatList.begin(),
		m_vecFlatList.end(),
		TSortByLines() );

	m_iSortField = SORT_BY_LINES;
	return TRUE;
}

BOOL TThreadList::ScoreSortFlatlist()
{
	stable_sort (m_vecFlatList.begin(),
		m_vecFlatList.end(),
		TSortByScores() );

	m_iSortField = SORT_BY_SCORE;
	return TRUE;
}

// --------------------------------------------------------------------------
BOOL TThreadList::IndicatorSortFlatlist(TNewsGroup* pNG)
{
	sgpSortNG = pNG;

	stable_sort (m_vecFlatList.begin(),
		m_vecFlatList.end(),
		TSortByIndicator() );

	m_iSortField = SORT_BY_INDICATOR;
	sgpSortNG = 0;
	return TRUE;
}

BOOL fnBitOn(WORD w, WORD f)
{
	return (w & f) ? TRUE : FALSE;
}

///////////////////////////////////////////////////////////////////////////
void fnCleanPiles (std::vector<TThreadPile*> & rPiles)
{
	// ThreadPile destructor does nothing recursive
	//  but RecursiveFree() does

	std::vector<TThreadPile*>::iterator it = rPiles.begin();

	for (; it != rPiles.end(); it++)
	{
		TThreadPile * pPile = *it;

		// this frees the pThreads in the pPile
		pPile->RecursiveFree ();

	}
	rPiles.clear();
}

template <class T>
class TPile_Less_Subj : public binary_function<T, T, bool>
{
public:
	bool operator()( T  pPile1, 
		T  pPile2) const
	{
		TThread * pT1 = (TThread*) pPile1->GetElem(0);
		TThread * pT2 = (TThread*) pPile2->GetElem(0);

		// note:  "Re: Albatross" should sort before "Baseball"

		// this returns -1  if pPile1 is less than pPile2
		CString s1, s2;
		pT1->GetSubject (s1);
		pT2->GetSubject (s2);
		if (-1 == fnStrCompareSubjectSansRE (s1, s2))
			return true;
		else
			return false;
	}
};

template <class T>
class TPile_Less_Date : public binary_function<T, T, bool>
{
public:
	bool operator()(T pPile1, T pPile2) const
	{
		if (pPile1->m_oldTime < pPile2->m_oldTime)
			return true;

		if (pPile1->m_oldTime > pPile2->m_oldTime)
			return false;

		// they are equal,  use subject as tie-breaker

		return  TPile_Less_Subj<T>()(pPile1, pPile2);
	}
};

// Use date as secondary sort, subj as tertiary
template <class T>
class TPile_Less_Score : public binary_function<T, T, bool>
{
public:
	bool operator()(T pPile1, T pPile2) const
	{
		long lScore1 = pPile1 -> getScore();
		long lScore2 = pPile2 -> getScore();
		long lLowScore1 = pPile1 -> getLowScore();
		long lLowScore2 = pPile2 -> getLowScore();
		BOOL bZero1 = pPile1 -> ZeroPresent();
		BOOL bZero2 = pPile2 -> ZeroPresent();

		// positive scores are first, then threads with just zero, then threads with
		// negative scores

		TPile_Less_Date<T> sLessDate;

		// one or both threads have just zeros
		BOOL bJustZero1 = lScore1 < lLowScore1;
		BOOL bJustZero2 = lScore2 < lLowScore2;
		if (bJustZero1 || bJustZero2)
		{
			if (bJustZero1 && bJustZero2)
				return sLessDate(pPile1, pPile2);

			if (bJustZero1)
			{
				if (lScore2 > 0)
					return true;
				if (lScore2 < 0)
					return false;
				return sLessDate(pPile1, pPile2);
			}
			if (lScore1 > 0)
				return false;
			if (lScore1 < 0)
				return true;
			return sLessDate(pPile1, pPile2);
		}

		// neither thread has just zeros, so compare highest scores
		if (lScore1 == lScore2)
			return sLessDate(pPile1, pPile2);

		if (lScore1 < lScore2)
			return true;
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////
// Public function
//
void TThreadList::UpdateSort (BOOL fAscend       /* =TRUE */,
							  BOOL fResetMetrics /* =TRUE */)
{
	if (fResetMetrics)
	{
		// if we deleted a thread, we have to recalculate the 'm_oldestTime'
		//   and 'm_lScore' for pile
		std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
		for (; it != m_rPiles.end(); it++)
		{
			if ((*it)->DirtyDuringDelete())
				(*it)->ReCalcPileMetrics ();
		}
	}

	// Sort PILES, after piles are complete.

	switch (gpGlobalOptions->GetThreadSort ())
	{
	case (TGlobalDef::kSortSubject):
		{
			TPile_Less_Subj<TThreadPile*> predSubj;
			if (fAscend)
				stable_sort (m_rPiles.begin(), m_rPiles.end(), predSubj);
			else
				stable_sort (m_rPiles.begin(), m_rPiles.end(), not2(predSubj));
			break;
		}

	case (TGlobalDef::kSortScore):
		{
			TPile_Less_Score<TThreadPile*> predScore;
			if (fAscend)
				stable_sort (m_rPiles.begin(), m_rPiles.end(), predScore);
			else
				stable_sort (m_rPiles.begin(), m_rPiles.end(), not2(predScore));
			break;
		}

	case (TGlobalDef::kSortDate):
	default:
		{
			TPile_Less_Date<TThreadPile*> predDate;
			if (fAscend)
				stable_sort (m_rPiles.begin(), m_rPiles.end(), predDate);
			else
				stable_sort (m_rPiles.begin(), m_rPiles.end(), not2(predDate));
			break;
		}
	}
}

// dirty prototype
extern   TQuickFilterData * fnGetQuickFilterData ();

// -------------------------------------------------------------------------
int TThreadList::WalkPilesDelete (TNewsGroup * pNG, TViewFilter * pFilter)
{
	// we might have a funky filter setting. 'show whole thread if any member
	//   matches filter'  So we:
	//       build threads (based on everything)
	//       toss out threads that do not match

	if (pFilter->getShowThread() == 0)
		return 0;

	TNewsServer * pServer = pNG->GetParentServer();

	TServerCountedPtr cpServer(pServer);   // do ref counting

	TPersistentTags & rTags = pServer->GetPersistentTags();

	TQuickFilterData * pData = fnGetQuickFilterData ();

	// we might be deleting stuff, so work backwards
	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	int ded = 0;
	for (; it != m_rPiles.end(); )
	{
		PumpAUsefulMessageLITE ();

		TArticleActionElementMatch sAction(pFilter, pNG, &rTags, pData);

		TThreadPile * pPile = *it;

		pPile->WalkHeaders (&sAction);

		if (!sAction.IsMatch ())
		{
			it = m_rPiles.erase (it);

			// remove nodes from the m_setNodes
			pPile->Excoriate ();

			// nodes are freed, the pPile and pThread are still here

			// free this pile and the one thread it holds
			pPile->RecursiveFree ();

			ded++;
		}
		else
			++it;
	}

	return 0;
}

// FlatListLength -- allow access to arbitrary flat list headers
int TThreadList::FlatListLength ()
{
	return m_vecFlatList.size ();
}

//-------------------------------------------------------------------------
// mark a thread with a status.  Header is one member of thread.
int TThreadList::MarkThreadStatus (
								   TNewsGroup*        pNG,
								   TArticleHeader* pArtHeader,
								   TStatusUnit::EStatus kStatus,
								   BOOL fOn)
{
	TThread* pThread;    // article's thread
	ASSERT(pNG->GetpThreadList() == this);

	TThreadPile * pPile = 0;
	TArtNode * pNode = 0;

	// ?? lock this down?
	if (!FindThreadPile ( pArtHeader, pPile, pThread, pNode ))
	{
		ASSERT(0);
		return -1;
	}
	TArticleAction actionObject(pNG, kStatus, fOn);
	pThread->WalkHeaders ( &actionObject );
	return 0;
}

// mark a thread with a status.  TArtNode is root of thread
int TThreadList::MarkThreadStatus (
								   TNewsGroup*          pNG,
								   TArtNode*            pArtNode,
								   TStatusUnit::EStatus kStatus,
								   BOOL                 fOn)
{
	// send this soldier in
	TArticleAction actionObject (pNG, kStatus, fOn);

	// call class wide function
	return TThread::WalkHeaders_ProcessNode ( pArtNode, &actionObject );
}

//-------------------------------------------------------------------------
// mark a Pile with a status.  Header is one member of thread.
int TThreadList::MarkPileStatus (TNewsGroup * pNG,
								 TArticleHeader* pArtHeader,
								 TStatusUnit::EStatus kStatus,
								 BOOL fOn)
{
	TThreadPile * pPile = 0;
	TThread* pThread = 0;    // article's thread
	TArtNode* pNode = 0;

	ASSERT(pNG->GetpThreadList() == this);

	if (!FindThreadPile ( pArtHeader,   pPile, pThread, pNode ))
	{
		ASSERT(0);
		return -1;
	}

	if (pPile)
	{
		TArticleAction actionObject(pNG, kStatus, fOn);
		pPile -> WalkHeaders ( &actionObject );
	}
	else
		ASSERT(0);
	return 0;
}

///////////////////////////////////////////////////////////
// Find by artInt
//   Return a pointer to the LP_TREE_NODE
BOOL TThreadList::FindUIData (
							  LPVOID&           rpVoid,
							  TArticleHeader*&  rpHdr,
							  int artInt )
{
	TArticleActionFetch objFetch(artInt);
	BOOL fRet = FindUIData_Helper ( &objFetch );
	if (fRet)
	{
		rpVoid = objFetch.m_pVoid;
		rpHdr   = objFetch.m_pHeader;
	}
	return fRet;
}

BOOL TThreadList::FindUIData_Helper (TBaseArticleAction* pFetchAction)
{
	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	for (; it != m_rPiles.end(); it++)
	{
		// returns non-zero to stop
		if ((*it)->WalkHeaders ( pFetchAction ))
			return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
//
//  fCollapsed is TRUE if the threads should be collapsed initially
int TThreadList::FillTree (THierDrawLbx* pTree, BOOL fCollapsed)
{
	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	for (; it != m_rPiles.end(); it++)
	{
		TThreadPile* pPile = *it;
		pPile->FillTree ( pTree, fCollapsed );
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// This is the tip of the 'IceBerg'
//
//  4-12-96 amc Remove art from m_Stage too
int TThreadList::RemoveArticle (TArtNode * pNodeIn,
								TArticleHeader* pArtHdr, BOOL fDeleteArtPtrs /* = TRUE */)
{
	bool fCrushNode ;
	if (pNodeIn)
	{
		fCrushNode = true;
		pArtHdr = pNodeIn->GetpRandomHeader();
	}
	else
	{
		fCrushNode = false;
	}

	if (0 == pArtHdr)
		throw(new TException (IDS_ERR_REMOVEARTICLE, kFatal));
	TThreadPile* pPile = 0;
	TThread*     pThread = 0;
	TArtNode*    pNode = 0;

	//TRACE1("Remove Article on %d\n", pArtHdr->GetNumber());

	// figure out the Pile, Thread, and Node
	if (FindThreadPile (pArtHdr, pPile, pThread, pNode))
	{
		pPile->DirtyDuringDelete (true);  // flag as dirty

		if (0 == pNode)
		{
			ASSERT(0);
			return 1;
		}

		if (fCrushNode || (1 == pNode->GetArtsCount()) )
		{
			// this unhooks the Node from the thread

			BOOL fThreadEmpty = pThread->RemoveArticle(pNode, pPile);

			if (fThreadEmpty)
				RemoveThread ( pThread, TRUE );
		}
	}

	if (fCrushNode || (1 == pNode->GetArtsCount()))
	{
		// this whole node is going away

		// remove header from article bank
		m_pBank->UnhookNode ( pNode );

		// remove node from flat list
		remove_node_from_flatlist ( pNode );

		if (fDeleteArtPtrs)
		{
			// delete the node + delete the articles
			delete pNode;
		}
		else
		{
			// caller frees the header
			pNode->ClearpHeader ();
			delete pNode;
		}
	}
	else
	{
		// removing 1 arthdr from node, but the rest is ok.

		pNode->RemoveOneOfArts ( pArtHdr->GetArticleNumber () );

		if (fDeleteArtPtrs)
		{
			delete pArtHdr;
		}
		else
		{
			// nothing
		}
	}

#ifdef _DEBUG
	myAssertValid ();
#endif

	return 0;
}

//-------------------------------------------------------------------------
// Remove thread from the threadpile, remove threadpile from thread list
//  but do not destroy articles
int TThreadList::RemoveThread (TThread* pThread, BOOL fRemoveFromFlat)
{
	TThreadPile * pPile = pThread->m_pPile;
	if (0 == pPile)
		return 1;

	int iPileSize = pPile->RemoveThread ( pThread );
	BOOL fFound = TRUE;

	//BOOL fFound = remove_thread_from_rootarray ( pThread );

	ASSERT(fFound);

	if (fRemoveFromFlat)
	{
		CPtrArray vec;
		// extract all the nodes
		pThread->OperateOnNodes ( fnCollectNodes, &vec );
		for (int i = 0; i < vec.GetSize(); ++i)
		{
			TArtNode * pNode = (TArtNode*) vec[i];
			remove_node_from_flatlist ( pNode );
		}
	}

	delete pThread;

	if (0 == iPileSize)
	{
		fFound = remove_pile ( pPile );
		ASSERT(fFound);
		delete pPile;
	}
	return 0;
}

//-------------------------------------------------------------------------
BOOL TThreadList::remove_pile(const TThreadPile * pPile)
{
	BOOL fFound = FALSE;

	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	for (; it != m_rPiles.end(); it++)
	{
		if (pPile == (*it))
		{
			fFound = TRUE;
			m_rPiles.erase(it);
			break;
		}
	}
	return fFound;
}

//-------------------------------------------------------------------------
BOOL TThreadList::remove_node_from_flatlist (TArtNode * pNode1)
{
	VEC_NODES::iterator it = m_vecFlatList.begin();

	for (; it != m_vecFlatList.end(); it++)
	{
		TArtNode * pNode = (*it);

		if (pNode->GetArticleNumber() == pNode1->GetArticleNumber() )
		{
			m_vecFlatList.erase (it);
			return TRUE;
		}
	}
	ASSERT(0);
	return FALSE;
}

//-------------------------------------------------------------------------
static int fnCollectNodes(TArtNode * pArtNode, void* pV)
{
	CPtrArray * pArray = static_cast<CPtrArray*>(pV);
	pArray->Add (pArtNode);
	return 0;
}

void TThreadList::PrepareHashTable(int count)
{
	ASSERT(m_pBank->GetCount() == 0);
	int set = prime_larger_than(count);

	m_pBank->InitHashTable (set);
}

///////////////////////////////////////////////////////////////////////////
// Makes a flat list of article header pointers. Copy the pointers out of
// the thread list; the IndexList manages the reference count on articles
//
// Note: the list is not ordered
void TThreadList::CreateArticleIndex(TArticleIndexList * pArtIndex)
{
	ASSERT( pArtIndex );
	m_pBank->CreateArticleIndex ( pArtIndex );
}

//-------------------------------------------------------------------------
// called from ThreadNode2
//
// RLW - Changed function so it parses ALL the Ref-IDs until it either
// finds a parent or runs out of Ref-IDs.
//
// This results in much better threading if articles are missing on the
// server.
//
BOOL TThreadList::find_dad(TNewsGroup* pNG, TArticleBank* pBank,
						   TArtNode* pNode, TArtNode*& rpDadNode)
{
	CString dadRef;
	CStringList parentsList;

	int artInt = pNode->GetpRandomHeader()->GetNumber ();
	rpDadNode = 0;

	pNode->GetpRandomHeader()->GetReferencesStringList( &parentsList );
	while (!parentsList.IsEmpty())
	{
		dadRef = parentsList.RemoveTail();

		if (TNewsGroup::kHeadersOnly == UtilGetStorageOption(pNG) ||
			TNewsGroup::kStoreBodies == UtilGetStorageOption(pNG))
		{
			if (!pBank->Lookup ( dadRef, rpDadNode ))
				continue; // Parent not found, try next ref-ID
			else
				return TRUE; // Found parent
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE; // No parent not found
}

//-------------------------------------------------------------------------
// Helper Object - add the integer once and only once
void TPileIDS::UniqueAdd (int n)
{
	int tot = m_ArtIdsFound.GetCount();
	for (int i = 0; i < tot; ++i)
	{
		if ((int) m_ArtIdsFound.GetAt(i) == n)
			return;
	}

	m_ArtIdsFound.Add ( n );
}

//-------------------------------------------------------------------------
//  given a bunch of ids, return the set of ids for the entire pile
//  10                                 Returns 10,11,12, 20,22,23
//   11     [input 11]
//    12
//  20
//   22
//   23     [input 23]
BOOL TThreadList::FindNPileContents (TPileIDS* psIDS)
{
	TThread* pThread = 0;
	BOOL fRet = FALSE;

	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	for (; it != m_rPiles.end(); ++it)
	{
		if ((*it)->HaveAny ( &psIDS->m_ArtIdsToFind ))
		{
			fRet = TRUE;
			(*it)->CollectIds ( &psIDS->m_ArtIdsFound );
		}
	}
	return fRet;
}

// ------------------------------------------------------------------------
//
// Used for find-next-unread
// Returns rpArtNodeOut - the next artNode that matches psFindNext
//         rpPileOut and rpThreadOut are just context for rpArtNodeOut
int  TThreadList::FindTreeNext (
								TArtNode *       pNode,
								ThreadFindNext * psFindNext,
								bool             fLimitToCurPile,
								TThreadPile * &  rpPileOut,
								TThread * &      rpThreadOut,
								TArtNode * &     rpArtNodeOut)
{
	TThread * pThreadSrc = pNode->GetThread ();
	ASSERT( pThreadSrc );
	TThreadPile * pPileSrc = pThreadSrc -> m_pPile;

	// is it in the current pile?
	if (0 == pPileSrc -> FindTreeNext (pNode,
		psFindNext,
		rpThreadOut, rpArtNodeOut))
	{
		// yes, we found it.
		rpPileOut = pPileSrc;
		return 0;
	}
	else if (fLimitToCurPile)
	{
		// if we are limiting tho current pile, then don't go looking at
		//   the next pile.

		return 1;
	}
	else
	{
		// get index of current pile
		std::vector<TThreadPile*>::iterator start = m_rPiles.end();
		std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
		for (; it != m_rPiles.end(); ++it)
		{
			if ((*it) == pPileSrc)
			{
				start = it;
				break;
			}
		}

		// is this the last pile?
		if (start == m_rPiles.end())
			return 1;

		for (it = start + 1; it != m_rPiles.end(); ++ it)
		{
			// start traversing a brand new pile
			if (0 == (*it) -> FindTreeNext (NULL,
				psFindNext,
				rpThreadOut, rpArtNodeOut))
			{
				rpPileOut = (*it);
				return 0;
			}
		}
		// fell off loop.  Not found in any pile
		return 1;
	}
}

// ------------------------------------------------------------------------
//
int  TThreadList::FindFlatNext (BYTE fAscend, TArtNode* pNode_,
								ThreadFindNext * psFindNext, TArtNode*& rpReturnArtNode)
{
	if (fAscend)
	{
		VEC_NODES::iterator it = m_vecFlatList.begin();
		VEC_NODES::iterator itX;

		bool fFound = false;

		// find current position
		for (; it != m_vecFlatList.end(); it++)
		{
			TArtNode * pNode = (*it);

			if (pNode == pNode_)
			{
				fFound = true;
				itX = it;
				break;
			}
		}

		if (fFound)
		{
			itX ++;
			while (itX != m_vecFlatList.end())
			{
				TArtNode * pNode = (*itX);
				if (0 == psFindNext->pfnTest( pNode->GetpRandomHeader(),
					psFindNext->pData))
				{
					rpReturnArtNode = pNode;
					return 0;
				}
				itX++;
			}
		}
		return 1;
	}
	else
	{
		VEC_NODES::reverse_iterator it = m_vecFlatList.rbegin();
		VEC_NODES::reverse_iterator itX;

		bool fFound = false;

		// find current position
		for (; it != m_vecFlatList.rend(); it++)
		{
			TArtNode * pNode = (*it);

			if (pNode == pNode_)
			{
				fFound = true;
				itX = it;
				break;
			}
		}

		if (fFound)
		{
			itX ++;
			while (itX != m_vecFlatList.rend())
			{
				TArtNode * pNode = (*itX);
				if (0 == psFindNext->pfnTest( pNode->GetpRandomHeader(),
					psFindNext->pData))
				{
					rpReturnArtNode = pNode;
					return 0;
				}
				itX++;
			}
		}
		return 1;

	}
}

// ------------------------------------------------------------------------
// myAssertValid -- commented out during Release builds
void TThreadList::myAssertValid ()
{
#if defined(_DEBUG)
	// check validity of each pile
	std::vector<TThreadPile*>::iterator it = m_rPiles.begin();
	for (; it != m_rPiles.end(); ++it)
	{
		TThreadPile * pPile = *it;

		pPile->myAssertValid ();
	}

	// check validity of ArtBank

	// check validity of FlatList
#endif
}

// ------------------------------------------------------------------------
// RemoveFromBank - called from threadpile object
BOOL TThreadList::RemoveFromBankAndFree (TArtNode *& pArtNode)
{
	BOOL fRet = m_pBank->UnhookNode ( pArtNode );

	m_pBank->DeleteNode ( pArtNode );

	return fRet;
}

// ------------------------------------------------------------------------
void TThreadList::InstallBank (TArticleBank* pBank)
{
	m_pBank = pBank;

	PrepareHashTable (30000);
}

////////////////////////////////////////////////////////////////////////////
//  N E W   T H R E A D I N G    S T U F F
////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
int  TThreadList::LinkByReference (TNewsGroup* pNG, TViewFilter* pFilter,
								   NodeList * pOutList)
{
	TArtNode *     pNode;

	BOOL  fShowEntireThread = pFilter->getShowThread();
	BOOL  fSkipThreads = pFilter->getNoThread();  // skip threads for big binary groups

	if (fSkipThreads)
		fShowEntireThread = FALSE;

	m_pBank->Lock ();

	TServerCountedPtr cpNewsServer(pNG->GetParentServer());

	TPersistentTags & rTags = cpNewsServer->GetPersistentTags();

	TArticleBank::NodeMap::iterator it = m_pBank->m_Stage.begin();

	for (; it != m_pBank->m_Stage.end(); it++)
	{
		pNode = (*it).second;
		ASSERT(pNode);

		if (fShowEntireThread ||
			pFilter->FilterMatch ( pNode->GetpRandomHeader(),
			pNG,
			&rTags ))
		{

			pOutList->AddTail ( pNode );
		}

		PumpAUsefulMessageLITE ();
	}
	m_pBank->Unlock ();

	if (fSkipThreads)
	{
		return 0;
	}

	int total = pOutList->GetCount ();

	for (int i = 0; i < total; i++)
	{
		PumpAUsefulMessageLITE ();

		pNode =  pOutList->RemoveHead ();

		if (ThreadNode2 ( pNG, pNode ))
		{
			// ancestor not found, so re-add to back of the line

			pOutList->AddTail ( pNode );
		}
	}
	return 0;
}

// returns 0 for success
int  TThreadList::ThreadNode2( TNewsGroup* pNG, TArtNode* pNode )
{
	// definitely a root
	if (pNode->GetpRandomHeader()->AtRoot())
	{
		return 1;
	}

	TArtNode* pNodeDad = 0;

	// find any ancestor

	if (find_dad (pNG, m_pBank, pNode, pNodeDad))
	{
		ASSERT(pNodeDad);

		// attach to parent node
		pNodeDad->ConnectChild ( pNode );
		return 0;
	}
	return 1;
}

// --------------------------------------------------------------------------
//  Step 40
//  Input -- Nodes for which we failed to find the Parent article
//
int TThreadList::AggregateCommonRoot_or_Subject (NodeList * pOutList)
{
	CMap <CString, LPCTSTR, DWORD, DWORD> sMapSubj;
	CMap <CString, LPCTSTR, DWORD, DWORD> sMapRoot;

	sMapSubj.InitHashTable (prime_larger_than(1000));
	sMapRoot.InitHashTable (prime_larger_than(1000));

	// sMapSubj - for all pPile in m_rPiles, map subject to idx in m_rPiles[]
	//
	// sMapRoot - for all pPile in m_rPiles, map ancestor msg-id to idx in m_rPiles[]
	//
	BOOL    fFoundSubj, fFoundRoot;

	TCHAR   rcStub[5];

	CString     keyAncestor;

	int iDummy = 0;

	DWORD       dwIdxSubj, dwIdxRoot;

	// dwIndex is the index into m_rPiles
	DWORD       dwIndex = 0;

	// sMapSubj  =  for every pile in m_rPiles, map the subject to the
	//              correct PilePtr (via dwIdxSubj)
	//
	// sMapRoot  =  for every pile in m_rPiles, map the ancestor to the
	//              correct PilePtr (via DWORD index)

	BOOL fThreadBySubject = ! gpGlobalOptions->GetRegSwitch()->GetThreadPureThreading();

	//
	// I want to compare subjects, but only out to a certain point (20 chars?)
	// for instance
	//
	//   "Babylon 5 Episode 27"  should match
	//   "Babylon 5 Episode 27 (Possible Spoilers)"          7-17-96 amc

	int iCutoffLen = gpGlobalOptions->GetRegSystem()->GetSubjectMatchCutoff();

	TCHAR * pShortSubj = new TCHAR[iCutoffLen + 1];
	auto_ptr<TCHAR> sShortSubjDeleter(pShortSubj);

	POSITION pos = pOutList->GetHeadPosition ();

	while (pos)
	{
		LPCTSTR pKeySubj = 0;
		TArtNode * pNode = pOutList->GetNext (pos);

		TArticleHeader * pHdr = pNode->GetpRandomHeader();

		if (NULL == pHdr)
			continue;

		if (fThreadBySubject)  // optional
		{
			// setup subject key
			pKeySubj = pHdr->GetSubject ();
			lstrcpyn (rcStub, pKeySubj, 5);

			if (0 == lstrcmpi (rcStub, "Re: "))
				pKeySubj += 4;

			lstrcpyn (pShortSubj, pKeySubj, iCutoffLen);
		}

		// setup ancestor key
		keyAncestor.Empty();
		pHdr->GetFirstRef (keyAncestor);

		if (keyAncestor.GetLength() == 0)
			fFoundRoot = FALSE;
		else
			fFoundRoot = sMapRoot.Lookup (keyAncestor, dwIdxRoot);

		if (fThreadBySubject)
			fFoundSubj = sMapSubj.Lookup (pShortSubj, dwIdxSubj);
		else
			fFoundSubj = FALSE;

		// !find both
		if (!fFoundRoot && !fFoundSubj)
		{
			TThread * pThread = new TThread(pNode);  // $$ Need to Free This

			// make brand new thread
			TThreadPile * pPile = new TThreadPile (this, pThread);

			// add to both
			m_rPiles.push_back ( pPile );

			if (keyAncestor.GetLength() > 0)
				sMapRoot.SetAt ( keyAncestor, dwIndex );

			if (fThreadBySubject)
				sMapSubj.SetAt ( pShortSubj,    dwIndex );
			dwIndex++;
		}

		// find subj, !find root
		else if (fFoundSubj && !fFoundRoot)
		{
			//sMapSubj. digup & append
			TThreadPile * pOnePile = m_rPiles[dwIdxSubj];

			TThread * pThread = new TThread(pNode);

			// add thread to pile
			pOnePile->InsertThreadByTime ( pThread );

			// subservient to sMapSubj
			if (keyAncestor.GetLength() > 0)
				sMapRoot.SetAt (keyAncestor, dwIdxSubj);
		}

		// !find subj, find root  -OR-  find both
		else if ((!fFoundSubj && fFoundRoot) || (fFoundSubj && fFoundRoot))
		{
			//sMapRoot -- digup and append
			TThreadPile * pOnePile = m_rPiles[dwIdxRoot];

			TThread * pThread = new TThread(pNode);

			pOnePile->InsertThreadByTime ( pThread );

			if (fThreadBySubject)
				// subservient to root
				sMapSubj.SetAt (pShortSubj, dwIdxRoot);
		}
	}
	return 0;
}

// --------------------------------------------------------------------------
//
int TThreadList::HeapifyPiles ( void )
{
	std::vector<TThreadPile*>::reverse_iterator it = m_rPiles.rbegin();

	for (; it != m_rPiles.rend(); ++it)
	{
		PumpAUsefulMessageLITE ();

		TThreadPile * pPile = *it;

		pPile->CompactThreadsToOneThread ();
	}
	return 0;
}

// --------------------------------------------------------------------------
//
int TThreadList::PropagateScoresDownThePiles ( void )
{

	// scoring -- propagate pile score to the articles in the pile
	TArticleActionSetPileScore sAction;

	std::vector<TThreadPile*>::reverse_iterator it = m_rPiles.rbegin();

	for (; it != m_rPiles.rend(); ++it)
	{
		PumpAUsefulMessageLITE ();

		TThreadPile * pPile = *it;

		sAction.SetScore ( pPile->getScore() );
		sAction.SetZeroPresent ( pPile->ZeroPresent() );

		pPile->WalkHeaders ( &sAction );
	}
	return 0;
}

// --------------------------------------------------------------------------
// Now that WalkPilesDelete has deleted stuff, we are free to add the
//   remainder to the flatlist
int TThreadList::AddPilesToFlat ()
{
	std::vector<TThreadPile*>::reverse_iterator it = m_rPiles.rbegin();
	for (; it != m_rPiles.rend(); ++it)
	{
		TThreadPile * pPile = *it;

		pPile->CollectNodes ( &m_vecFlatList );
	}

	return 0;
}

static void  ddd (TThreadPile * pPile)
{
	pPile->DirtyDuringDelete(false);
}

// ------------------------------------------------------------------------
void TThreadList::FlagMetricsClean ()
{
	// just wanted to use for_each just for the hell of it
	for_each (m_rPiles.begin(), m_rPiles.end(), ddd);
}

