/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thredpl.cpp,v $
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
#include "thredpl.h"
#include "thread.h"

#include "thredlst.h"
#include "mplib.h"
#include "resource.h"
#include "utilpump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

//////////////////////////////////////////////////////////////////////////
// return the Thread that contains the Article header
//
BOOL TThreadPile::Have (TArticleHeader* pArtHdr,
						TThread*& rpThread, TArtNode*& rpNode)
{
	int tot = m_array.GetSize();
	for (--tot; tot >= 0; --tot)
	{
		TThread* pThread = (TThread*) m_array[tot];
		if (pThread->Have ( pArtHdr, rpNode ))
		{
			rpThread = pThread;
			return TRUE;
		}
	}
	return FALSE;
}

int TThreadPile::FillTree (THierDrawLbx* pTree, BOOL fCollapsed)
{
	int totThread = m_array.GetSize();
	for (int j = 0; j < totThread; j++)
	{
		PumpAUsefulMessageLITE ();

		TThread* pThread = (TThread*) m_array[j];
		pThread->FillTree ( pTree, fCollapsed );
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//
TThreadPile::TThreadPile(TThreadList* pList, TThread* pThread)
: m_pThreadList(pList)
{
	pThread->m_pPile = this;

	m_array.Add( pThread );
	ASSERT(m_array.GetSize() == 1);
	pThread->CalcThreadDate();

	m_oldTime      = pThread->m_oldestTime;
	m_lScore       = pThread->m_lScore;
	m_lLowScore    = pThread->m_lLowScore;
	m_bZeroPresent = pThread->m_bZeroPresent;

	m_fDirtyDuringDelete = false;
}

// --------------------------------------------------------
void TThreadPile::CopyMinMaxMetrics ( TThread *pThread )
{
	m_oldTime    =  min (m_oldTime, pThread->m_oldestTime);
	m_lScore     =  max (m_lScore,     pThread->m_lScore);
	m_lLowScore  =  min (m_lLowScore,  pThread->m_lLowScore);

	if (pThread->m_bZeroPresent)
		m_bZeroPresent = true;
}

///////////////////////////////////////////////////////////////////////////
//
//
int TThreadPile::RemoveThread(const TThread* pThread)
{
	int iIndex = FindPosition (pThread);
	if (iIndex < 0)
		throw(new TException(IDS_ERR_THREAD_NOT_FOUND, kError));
	m_array.RemoveAt (iIndex);
	return m_array.GetSize();
}

///////////////////////////////////////////////////////////////////////////
//
//
int TThreadPile::FindPosition(const TThread* pThread)
{
	BOOL fFound = FALSE;
	int tot = m_array.GetSize();

	// find out where this thread is
	int i = 0;
	for (i = 0; i < tot; ++i)
	{
		if (pThread == (TThread*) m_array[i])
		{
			fFound = TRUE;
			break;
		}
	}
	if (i >= tot)
		return -1;
	return i;
}

TThreadPile::~TThreadPile()
{}

// Insert thread into our array of threads
int TThreadPile::InsertThreadByTime (TThread* pThread)
{
	// maintain the oldest date
	pThread->CalcThreadDate ();
	m_oldTime = min(m_oldTime, pThread->m_oldestTime);

	// maintain scoring info
	if (pThread -> m_bZeroPresent)
		m_bZeroPresent = true;

	m_lScore = max (m_lScore, pThread -> m_lScore);
	m_lLowScore = min (m_lLowScore, pThread -> m_lLowScore);

	CPtrArray& v = m_array;

#if defined(_DEBUG) && defined(CRAP)
	for (int k = 0; k < m_array.GetSize(); k++)
	{
		if (pThread == (TThread*) m_array[k])
		{
			return 0;
		}
	}
#endif

	// each thread points to the parent pile
	pThread->m_pPile = this;

	int tot = v.GetSize();
	for (--tot; tot >= 0; --tot)
	{
		TThread* pOne = (TThread*) v[tot];

		if (-1 == TThread::fnCompareByDateBySubj (pOne, pThread))
		{
			v.InsertAt(tot + 1, pThread);
			return 0;
		}
	}
	v.InsertAt(0, pThread);
	return 0;
}

BOOL TThreadPile::IsFirst(const TThread* pThread)
{
	if (m_array.GetSize() <= 0)
		return FALSE;

	TThread* pOne = (TThread*) m_array[0];
	return (pOne == pThread);
}

//-------------------------------------------------------------------------
BOOL  TThreadPile::HaveAny(TIntVector * pIdsToFind)
{
	BOOL fHave = FALSE;
	const int tot = m_array.GetSize();
	for (int i = 0; i < tot; ++i)
	{
		TThread * pThread = (TThread*) m_array[i];
		if (pThread->HaveN(pIdsToFind))
		{
			fHave = TRUE;
			break;
		}
	}
	return fHave;
}

//-------------------------------------------------------------------------
int  TThreadPile::CollectIds(TIntVector * pIdsFound)
{
	const int tot = m_array.GetSize();
	for (int i = 0; i < tot; ++i)
	{
		((TThread*) m_array[i]) -> CollectIds ( pIdsFound );
	}
	return 0;
}

//-------------------------------------------------------------------------
int  TThreadPile::WalkHeaders(TBaseArticleAction* pAction)
{
	int tot = m_array.GetSize();
	for (int i = 0; i < tot; ++i)
	{
		TThread* pOneThread = (TThread*) m_array[i];
		pOneThread->WalkHeaders ( pAction );
	}
	return 0;
}

// ------------------------------------------------------------------------
//
int  TThreadPile::FindTreeNext ( TArtNode * pNode,
								ThreadFindNext * psFindNext,
								TThread*& rpThreadOut,
								TArtNode*& rpArtNodeOut )
{
	if (0 == m_array.GetSize())
		return 1;
	TThread * pThread;
	pThread = pNode ? pNode->GetThread() : (TThread*) m_array[0];

	if (0 == pThread -> FindTreeNext ( pNode, psFindNext, rpArtNodeOut ))
	{
		rpThreadOut = pThread;
		return 0;
	}

	int i,  tot = m_array.GetSize();
	int iCur = -1;
	for (i = 0; i < tot; ++i)
	{
		if (pThread == (TThread*) m_array[i])
		{
			iCur = i;
			break;
		}
	}
	if (-1 == iCur || iCur >= (tot - 1))
		return 1;
	for (i = 1 + iCur; i < tot; ++i)
	{
		pThread = (TThread*) m_array[i];

		if (0 == pThread -> FindTreeNext ( NULL, psFindNext, rpArtNodeOut ))
		{
			rpThreadOut = pThread;
			return 0;
		}
	}
	return 1;
}

// ------------------------------------------------------------------------
//
void TThreadPile::myAssertValid ()
{
#if defined(_DEBUG)
	TThread * pThred;

	for (int i = 0; i < m_array.GetSize(); i++)
	{
		pThred = (TThread*) m_array[i];

		ASSERT(pThred->GetPile() == this);
		pThred->myAssertValid ();
	}
#endif
}

// ------------------------------------------------------------------------
// CompactThreadsToOneThread -- we want to mimic Agent's behavior of having
//    1 big tree.  added 2-23-98
// Before:  Separate threads were organized under a ThreadPile
// After:  ThreadPile holds 1 thread.
//
int TThreadPile::CompactThreadsToOneThread ()
{
	ASSERT(m_pThreadList);
	int sz = m_array.GetSize();
	if (sz <= 1)
		return 0;

	TThread * pFatThread = (TThread*) m_array[0];

	// mash sibling threads into FatThred
	for (int i = m_array.GetSize() - 1; i >= 1 ; i--)
	{
		TThread * pSibThread = (TThread*) m_array[i];

		if (pSibThread != pFatThread)
		{
			TArtNode * pSibRootNode = pSibThread->m_pRootNode;

			//         // now that we have the rootNode, we don't need the Thread object
			//         if (!m_pThreadList->remove_thread_from_rootarray (pSibThread))
			//            ASSERT(0);
			//         else
			{
				pSibThread->m_pRootNode = 0;  // not strictly necessary
				delete pSibThread;

				pFatThread->m_pRootNode->ConnectChild ( pSibRootNode );
			}
		}
	}

	// shrink m_array[]
	m_array.RemoveAll ();
	m_array.Add ( pFatThread );

	return 0;
}

int fnPile_CollectNodes (TArtNode* pNode, void * pData)
{
	VEC_NODES * pVec = (VEC_NODES *) pData;

	pVec->push_back (pNode);

	return 0;
}

// ------------------------------------------------------------------------
// excoriate_pile -- Remove this pile from all our collections. this is
//                   used in 'show entire thread' operations
int TThreadPile::Excoriate ()
{
	for (int i = m_array.GetSize() - 1; i >= 0; i--)
	{
		TThread * pOneThread = (TThread*) m_array[i];

		VEC_NODES vecNodes;
		pOneThread->OperateOnNodes (fnPile_CollectNodes, &vecNodes);

		VEC_NODES::reverse_iterator it = vecNodes.rbegin();

		for (; it != vecNodes.rend(); it++)
		{
			TArtNode * pNode = (*it);

			//TArticleHeader * pHdr = pNode->Getp$$Header();

			m_pThreadList->RemoveFromBankAndFree ( pNode );

		}
	}

	return 0;
}

// ------------------------------------------------------------------------
int TThreadPile::CollectNodes (  PVOID pvVec )
{
	for (int i = m_array.GetSize() - 1; i >= 0; i--)
	{
		TThread * pOneThread = (TThread*) m_array[i];

		pOneThread->OperateOnNodes (fnPile_CollectNodes, pvVec);
	}
	return 0;
}

// ------------------------------------------------------------------------
void TThreadPile::RecursiveFree ()
{
	for (int i = m_array.GetSize() - 1; i >= 0; i--)
	{
		TThread * pOneThread = (TThread*) m_array[i];

		// dtor is basic
		delete pOneThread;
	}

	delete this;
}

// ------------------------------------------------------------------------
// walk each child thread, and reset the
//    oldest time        min
//    m_lScore           max
//    m_lLowScore        min
//    m_bZeroPresent     initially false
//
void TThreadPile::ReCalcPileMetrics ()
{
	// reset to baseline values
	m_oldTime      = CTime(2020,1,1,0,0,0);
	m_lScore       =  (long) -0x70000000 ; // very low
	m_lLowScore    =  (long)  0x70000000 ; // very high
	m_bZeroPresent = false;

	for (int i = 0; i < m_array.GetSize(); i++)
	{
		TThread * pThread = static_cast<TThread*>(m_array[i]);
		pThread->ResetMetrics ();

		pThread->CalcThreadDate ();

		this->CopyMinMaxMetrics ( pThread );
	}
}

