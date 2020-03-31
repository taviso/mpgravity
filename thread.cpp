/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thread.cpp,v $
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

#include "stdafx.h"

#include "thread.h"
#include "article.h"
#include "artnode.h"
#include "artact.h"

#include "thredlst.h"
#include "hierlbx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static void OperateOnHeadersHelper (
									TArtNode * pNode,
									void (* lpfn) (TArticleHeader* pHeader, void* pV),
									void* pVoid);

TThread::TThread(TArtNode* pArtNode, TThreadPile* pPile /* = 0 */)
: m_oldestTime(2020,1,1,0,0,0),
m_lScore((long) -0x70000000),   /* very low */
m_lLowScore (0x70000000),       /* very high */
m_bZeroPresent (FALSE)
{
	set_root_node (pArtNode);

	// constructor
	//m_pCurrentNode = 0;
	m_pPile = pPile;
}

///////////////////////////////////////////////////////////////////////////
// the node points at us (the container)
void TThread::set_root_node (TArtNode * pRootNode)
{
	ASSERT(pRootNode);
	m_pRootNode = pRootNode;

	// do some ptr bookkeeping
	m_pRootNode->SetRootnodeThread (this);
	m_pRootNode->m_depth = 0;
}

// constructor
TThread::TThread()
{
	m_pPile = 0;
	ResetMetrics ();
}

// ------------------------------------------------
void TThread::ResetMetrics ()
{
	m_oldestTime   = CTime(2020, 1, 1, 0,0,0);
	m_lScore       = (long) -0x70000000;  // very low
	m_lLowScore    = (long)  0x70000000;  // very high
	m_bZeroPresent = false;
}

TThread::~TThread()
{
	// destructor
}

TThread& TThread::operator= (const TThread& rhs)
{
	if (&rhs == this)
		return *this;

	set_root_node ( rhs.m_pRootNode );
	m_pPile = rhs.m_pPile;
	//m_pCurrentNode = 0;
	m_lScore = rhs.m_lScore;
	m_lLowScore = rhs.m_lLowScore;
	m_bZeroPresent = rhs.m_bZeroPresent;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
// Reset depth of artnodes
void TThread::SetDepth ()
{
	if (m_pRootNode)
		m_pRootNode->RecursiveSetDepth (0);
}

//-------------------------------------------------------------------------
// fCollapsed is TRUE if thread should be collapsed initially
void TThread::FillTree(THierDrawLbx* pHierLbx, BOOL fCollapsed)
{
	ASSERT(m_pRootNode);
	m_pRootNode->AddKids ( pHierLbx, 0, fCollapsed );
}

BOOL TThread::Have(TArticleHeader* pArtHdr, TArtNode*& rpNode)
{
	// call helper function
	return Have ( pArtHdr->GetNumber(), rpNode );
}

BOOL TThread::Have (int iArtInt, TArtNode*& rpNode)
{
	// let node do it.
	return m_pRootNode->Have (iArtInt, rpNode);
}

// return non-zero to stop
int TThread::WalkHeaders (TBaseArticleAction* pAction)
{
	return WalkHeaders_ProcessNode (m_pRootNode, pAction);
}

// Class Wide handles an ArtNode recursively
int TThread::WalkHeaders_ProcessNode (
									  TArtNode * pNode,
									  TBaseArticleAction* pAction
									  )
{
	// process this node
	int ret  =  pNode -> ProcessHeaders( pAction );

	if (ret)
		return 1;  // stop

	STLMapANode::iterator  it  = pNode->GetKidsItBegin ();
	STLMapANode::iterator  itE = pNode->GetKidsItEnd ();

	for (; it != itE; it++)
	{
		TArtNode * pChild = *it;

		if (WalkHeaders_ProcessNode (pChild, pAction))
			return 1;
	}
	return 0;
}

// go through a thread's articles, calling a function on each
// article's header
void TThread::OperateOnHeaders (
								void (* lpfn) (TArticleHeader * pArtHdr, void* pV),
								void* pVoid
								)
{
	if (0 == m_pRootNode)
		return;
	OperateOnHeadersHelper (m_pRootNode, lpfn, pVoid);
}

///////////////////////////////////
// pass in the pVoid
static void OperateOnHeadersHelper (
									TArtNode * pNode,
									void (* lpfn) (TArticleHeader* pArtHdr, void* pV),
									void* pVoid
									)
{
	pNode->OperateOnHeaders ( lpfn, pVoid );

	STLMapANode::iterator  it  = pNode->GetKidsItBegin ();
	STLMapANode::iterator  itE = pNode->GetKidsItEnd ();

	// go through this node's children recursively
	for (; it != itE; it++)
	{
		TArtNode * pChild = *it;

		OperateOnHeadersHelper (pChild, lpfn, pVoid);
	}
}

// ------------------------------------------------------------------------

static int sfnOperateOnNodesHelper (
									TArtNode * pNode,
									int (* lpfn)(TArtNode * pNode, void * pData),
									void * pData)
{
	if (0 != lpfn (pNode, pData))
		return 1;         // stop

	STLMapANode::iterator  it  = pNode->GetKidsItBegin ();
	STLMapANode::iterator  itE = pNode->GetKidsItEnd ();

	// go through this node's children recursively
	for (; it != itE; it++)
	{
		TArtNode* pChild = *it;

		if (sfnOperateOnNodesHelper (pChild, lpfn, pData))
			return 1;      // stop
	}

	return 0;      // continue
}

void TThread::OperateOnNodes (int (* lpfn)(TArtNode * pNode, void * pData),
							  void * pData)
{
	if (0 == m_pRootNode)
		return;
	sfnOperateOnNodesHelper (m_pRootNode, lpfn, pData);
}

// ------------------------------------------------------------------------
//
// global func
void TThread::fnStepDate(TArticleHeader* pArtHdr, void* pV)
{
	TThread * pT = (TThread*) pV;
	if (pArtHdr->GetTime() < pT->m_oldestTime)
		pT->m_oldestTime = pArtHdr->GetTime();

	if (!pArtHdr -> GetScore ())
		pT -> m_bZeroPresent = TRUE;
	else {
		pT -> m_lScore = max (pT -> m_lScore, pArtHdr -> GetScore ());
		pT -> m_lLowScore = min (pT -> m_lLowScore, pArtHdr -> GetScore ());
	}
}

// find oldest time in tree of child articles
int TThread::CalcThreadDate(void)
{
	OperateOnHeaders ( fnStepDate, this );
	return 0;
}

// global func
int TThread::fnCompareDate(const CObject* pOb1, const CObject* pOb2)
{
	TThread* pT1 = (TThread*) pOb1;
	TThread* pT2 = (TThread*) pOb2;
	if (pT1->m_oldestTime == pT2->m_oldestTime)
		return 0;
	if (pT1->m_oldestTime < pT2->m_oldestTime)
		return -1;
	return 1;
}

// global func
int TThread::fnCompareSubj(const CObject* pOb1, const CObject* pOb2)
{
	TThread* pT1 = (TThread*) pOb1;
	TThread* pT2 = (TThread*) pOb2;
	CString sj1, sj2;

	pT1->m_pRootNode->GetSubject (sj1);
	pT2->m_pRootNode->GetSubject (sj2);

	LPCTSTR p1 = sj1;
	LPCTSTR p2 = sj2;

	if (('r' == p1[0] || 'R' == p1[0]) &&
		('e' == p1[1] || 'E' == p1[1]) &&
		(':' == p1[2]) &&
		(' ' == p1[3]))  p1 += 4;
	if (('r' == p2[0] || 'R' == p2[0]) &&
		('e' == p2[1] || 'E' == p2[1]) &&
		(':' == p2[2]) &&
		(' ' == p2[3]))  p2 += 4;
	int ret = lstrcmpi(p1, p2);
	if (ret)
		return ret;
	const CTime& t1 = pT1->m_pRootNode->GetTime();
	const CTime& t2 = pT2->m_pRootNode->GetTime();
	if (t1 == t2)
		return 0;
	else if (t1 < t2)
		return -1;
	else
		return 1;
}

// global func
int TThread::fnCompareScore(const CObject* pOb1, const CObject* pOb2)
{
	ASSERT (0);
	return 0;
#ifdef OLD_STUFF_TAKE_THIS_OUT /* !!! */
	TThread *pThread1 = (TThread*) pOb1;
	TThread *pThread2 = (TThread*) pOb2;
	long lScore1 = pThread1 -> m_lScore;
	long lScore2 = pThread2 -> m_lScore;
	long lLowScore1 = pThread1 -> m_lLowScore;
	long lLowScore2 = pThread2 -> m_lLowScore;
	BOOL bZero1 = pThread1 -> m_bZeroPresent;
	BOOL bZero2 = pThread2 -> m_bZeroPresent;

	// positive scores are first, then threads with just zero, then threads with
	// negative scores

	// one or both threads have just zeros
	BOOL bJustZero1 = lScore1 < lLowScore1;
	BOOL bJustZero2 = lScore2 < lLowScore2;
	if (bJustZero1 || bJustZero2) {
		if (bJustZero1 && bJustZero2)
			return 0;
		if (bJustZero1) {
			if (lScore2 > 0)
				return -1;
			if (lScore2 < 0)
				return 1;
			return 0;
		}
		if (lScore1 > 0)
			return 1;
		if (lScore1 < 0)
			return -1;
		return 0;
	}

	// neither thread has just zeros, so compare highest scores
	if (lScore1 == lScore2)
		return 0;
	if (lScore1 < lScore2)
		return -1;
	return 1;
#endif
}

int TThread::fnCompareByDateBySubj (const CObject* pOb1, const CObject* pOb2)
{
	int iCmp = TThread::fnCompareDate (pOb1, pOb2);

	if (iCmp != 0)
		return iCmp;

	// iCmp == zero,  2ndary sort via subject
	return TThread::fnCompareSubj ( pOb1, pOb2 );
}

int TThread::GetSubject(CString& subj)
{
	return m_pRootNode->GetSubject ( subj );
}

/////////////////////////////////////////////////////////////////
// Return true if the thread is should be removed
//
BOOL TThread::RemoveArticle (TArtNode* pNode, TThreadPile* pPile)
{
	// Unhook this node from parent & children, pass in the current Pile.
	pNode->Delink( pPile, this );
	BOOL fLastOne = (m_pRootNode == pNode);

	if (fLastOne)
		m_pRootNode = NULL;

	return fLastOne;
}

BOOL TThread::IsFirst(TArtNode* pNode)
{
	if (pNode != m_pRootNode)
		return FALSE;
	// see if thread is 1st in Pile
	return m_pPile->IsFirst(this);
}

BOOL TThread::IsRoot(int iArtInt)
{
	if (0 == m_pRootNode)
		return FALSE;
	return m_pRootNode->GetMinUnique() == iArtInt;
}

BOOL TThread::HaveN(TIntVector * pIdsToFind)
{
	TArticleActionHaveN sArtActHaveN(pIdsToFind);
	WalkHeaders ( &sArtActHaveN );
	return sArtActHaveN.m_fFound;
}

void TThread::CollectIds(TIntVector * pIdsFound)
{
	TArticleActionCollectIds sActionCollectIds(pIdsFound);

	WalkHeaders ( &sActionCollectIds );
}

// ------------------------------------------------------------------------
//
typedef struct
{
	int (*pfnTest)(TArticleHeader* pHdr, void * pData);
	void * pData;
	int  iState;
	// 0 = means Start
	// 1 = accept next node that matches
	TArtNode * pStartNode;
	TArtNode * pRetNode;
} IntThread_FindNext;

int sfnFindTreeNextHelper (TArtNode * pNode, void * pData)
{
	IntThread_FindNext * pTFN = (IntThread_FindNext *) pData;

	switch (pTFN->iState)
	{
	case 0:
		if (pNode == pTFN->pStartNode)
			pTFN->iState = 1;
		break;

	case 1:
		// run Match function
		if (0 == pTFN->pfnTest (pNode->GetpRandomHeader(), pTFN->pData))
		{
			pTFN->pRetNode = pNode;
			pTFN->iState = 2;
			return 1;  // stop
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	// continue
	return 0;
}

// ------------------------------------------------------------------------
int TThread::FindTreeNext (TArtNode * pNode, ThreadFindNext * psFindNext,
						   TArtNode*& rpArtNodeOut)
{
	IntThread_FindNext sFN;
	sFN.pfnTest = psFindNext->pfnTest;
	sFN.pData   = psFindNext->pData;
	sFN.pRetNode = NULL;
	sFN.pStartNode = pNode;
	sFN.iState = pNode ? 0 : 1;

	OperateOnNodes ( sfnFindTreeNextHelper, &sFN );
	if (sFN.pRetNode)
	{
		rpArtNodeOut = sFN.pRetNode;
		return 0;
	}
	return 1;
}

#if defined(_DEBUG)
// ------------------------------------------------------------------------
// util function for 'myAssertValid'
static int sfnValidateNode (TArtNode* pNode, void * pData)
{
	ASSERT(pNode);
	TThread * pThread = reinterpret_cast<TThread *>(pData);

	int iMyDepth = pNode->m_depth ;

	// test the article header object, by asking for the subject
	CString junk;

	pNode->GetSubject ( junk );

	// test that artnode eventually points back to the correct thread
	ASSERT(pThread == pNode->GetThread());

	STLMapANode::iterator  it  = pNode->GetKidsItBegin ();
	STLMapANode::iterator  itE = pNode->GetKidsItEnd ();

	for (; it != itE; it++)
	{
		TArtNode * pChild = *it;

		ASSERT(pChild->m_depth == (1 + iMyDepth));

		ASSERT(pChild->GetParent() == pNode);
	}

	return 0;
}
#endif // _DEBUG

// ------------------------------------------------------------------------
void TThread::myAssertValid ()
{
#if defined(_DEBUG)
	ASSERT(m_pRootNode);
	ASSERT(m_pRootNode->GetParent() == NULL);

	ASSERT(0 == m_pRootNode->m_depth);

	OperateOnNodes ( sfnValidateNode, this );
#endif
}

// ------------------------------------------------------------------------
// steal contents of thread passed in
int TThread::Merge (TThread *& rpThread)
{
	m_pRootNode->ConnectChild ( rpThread->m_pRootNode );

	rpThread->m_pRootNode->RecursiveSetDepth (m_pRootNode->m_depth + 1);

	rpThread->m_pRootNode  = 0;
	delete rpThread;
	rpThread = 0;
	return 0;
}

