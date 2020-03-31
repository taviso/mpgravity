/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thredlst.h,v $
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
/*  Revision 1.2  2008/09/19 14:52:05  richard_wood
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

#pragma once

#include "thread.h"
#include "declare.h"
#include "sortlst.h"
#include "statvec.h"
#include "artnode.h"
#include "intvec.h"
#include "thredtyp.h"               // holds enums
#include "thredfnx.h"
#include "thredpl.h"

typedef vector<TArtNode *> VEC_NODES;

/////////////////////////////////////////////////////////////////////////////
// TThreadList - List of threads and corresponding articles.  Base
//               document for the TThreadView.
//               handles "Threading"
/////////////////////////////////////////////////////////////////////////////

class TThreadList : public CObject {
	friend class TThreadListView;
	friend TThreadPile;
public:
	// this typedef is based in this class to keep global namespace clean
	typedef CTypedPtrArray<CObArray, TArtNode*> NodeArray;

	typedef CTypedPtrList<CObList, TArtNode*> NodeList;

public:
	TThreadList ();

	void InstallBank (TArticleBank* pBank);

	~TThreadList();
	int EmptyList(void);
	int EmptyAll(void);

	int RemoveArticle (TArtNode * pNode, TArticleHeader * pArtHdr, BOOL fDeleteHdrPtr = TRUE);
	int RemoveThread  (TThread * pThread, BOOL fRemoveFromFlat);

	void myAssertValid ();

	void Link(TNewsGroup* pNG, TViewFilter* pFilter);

	// Sort by date. Date of thread == date of oldest article in thread
	void UpdateSort(BOOL fAscend=TRUE, BOOL fResetMetrics=TRUE);

	// sort the flat list by different criteria
	BOOL SubjSortFlatlist();
	BOOL FromSortFlatlist();
	BOOL DateSortFlatlist();
	BOOL LineSortFlatlist();
	BOOL ScoreSortFlatlist();
	BOOL IndicatorSortFlatlist(TNewsGroup* pNG);

	// mark all articles as 'Read'
	BOOL CatchUpArticles(TNewsGroup* pNG,
		KThreadList::EXPost eXPost = KThreadList::kHandleCrossPost);

	void CreateArticleIndex (TArticleIndexList * pArtIndex);

	void FlagMetricsClean ();

	VEC_NODES::iterator GetFlatBegin() { return m_vecFlatList.begin(); }
	VEC_NODES::iterator GetFlatEnd  () { return m_vecFlatList.end();   }

	VEC_NODES::reverse_iterator GetFlatRBegin() { return m_vecFlatList.rbegin(); }
	VEC_NODES::reverse_iterator GetFlatREnd  () { return m_vecFlatList.rend();   }

protected:

	BOOL RemoveFromBankAndFree (TArtNode*& pArtNode);

public:
	BOOL FindThreadPile (TArticleHeader* pArtHdr, TThreadPile*& pPile,
		TThread*& pThread, TArtNode*& pArtNode) ;
	BOOL FindNPileContents (TPileIDS* psIDS);

	// allow access to arbitrary flat list headers
	int FlatListLength ();
	TArticleHeader *FlatListHeader (int iIndex);
	TArtNode *  FlatListNode (int iIndex);

	// mark a thread with a status.  Header is one member of thread.
	int MarkThreadStatus (TNewsGroup* pNG, TArticleHeader* pArtHeader,
		TStatusUnit::EStatus kStatus, BOOL fOn=TRUE);

	// mark a thread with a status.  TArtNode is root of thread
	int MarkThreadStatus (TNewsGroup* pNG, TArtNode* pArtNode,
		TStatusUnit::EStatus kStatus, BOOL fOn=TRUE);

	// mark a pile with a status.  Header is one member of thread.
	int MarkPileStatus (TNewsGroup* pNG, TArticleHeader* pArtHeader,
		TStatusUnit::EStatus kStatus, BOOL fOn=TRUE);

	BOOL FindUIData ( LPVOID& rpVoid, TArticleHeader*& pHdr, int artInt );

	int FillTree (THierDrawLbx* pTree, BOOL fCollapsed);

	void PrepareHashTable (int count);

	// Used for find-next-unread
	int  FindTreeNext (TArtNode *       pNode,
		ThreadFindNext * psFindNext,
		bool             fLimitToCurPile,
		TThreadPile * &  rpPile,
		TThread * &      rpThread,
		TArtNode* &      rpReturnArtNode);

	int  FindFlatNext (BYTE fAscend, TArtNode* pNode,
		ThreadFindNext * psFindNext,
		TArtNode*& rpReturnArtNode);

public:     // ACCESSORS

protected:
	BOOL FindUIData_Helper (TBaseArticleAction* pAction);

private: // functions
	int  CleanUp(void);

	int  WalkPilesDelete (TNewsGroup * pNG, TViewFilter * pFilter);

	BOOL find_dad(TNewsGroup* pNG, TArticleBank* pBank, TArtNode* pNode,
		TArtNode*& rpDadNode);

	BOOL remove_pile(const TThreadPile * pPile);
	BOOL remove_node_from_flatlist (TArtNode * pNode);

private:
	int  LinkByReference (TNewsGroup* pNG, TViewFilter* pFilter,
		NodeList * pOutList);

	int  ThreadNode2( TNewsGroup* pNG, TArtNode* pNode );
	int  AggregateCommonRoot_or_Subject (NodeList * pOutList);
	int  HeapifyPiles ( void );
	int  PropagateScoresDownThePiles ( void );
	int  AddPilesToFlat ();

private: // data

	// holds thread ptrs.  when threads are unified into one pile,
	//    they are removed from rootArray

	// each pile should own 1 thread
	std::vector<TThreadPile*> m_rPiles;

	// owns the universe of artnodes from which we form threads
	TArticleBank* m_pBank;

	// for sort by subject, sort by lines
	VEC_NODES   m_vecFlatList;

	enum {SORT_BY_DATE, SORT_BY_SUBJECT, SORT_BY_AUTHOR,
		SORT_BY_LINES, SORT_BY_INDICATOR, SORT_BY_SCORE}
	m_iSortField;
};
