/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thread.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

#pragma once

#include "declare.h"
#include "thredfnx.h"

class TBaseArticleAction;
class TTreeCtrl;
class TIntVector;
class TThreadPile;

/////////////////////////////////////////////////////////////////////////////
// TThread - Thread of articles
/////////////////////////////////////////////////////////////////////////////

class TThread : public CObject {
	friend TThreadList;
	friend TThreadPile;
public:
	CTime m_oldestTime;
	long  m_lScore;
	long  m_lLowScore;
	bool  m_bZeroPresent;

public:
	TThread (TArtNode* pArtNode, TThreadPile* pPile = 0);
	TThread &operator= (const TThread &);
	~TThread (void);

	// reset oldestTime, lScore, lLowScore
	void ResetMetrics ();

	void FillTree ( THierDrawLbx* pTree, BOOL fCollapsed );

	// return TRUE if this is in the tree
	BOOL Have (TArticleHeader* pArtHdr, TArtNode*& rpNode);
	BOOL Have (int iArtInt, TArtNode*& rpNode);

	BOOL RemoveArticle (TArtNode* pNode, TThreadPile* pPile);

	// go through a thread's articles, performing some action on each
	// article's header
	// void OperateOnHeaders (void (* lpfn) (TArticleHeader * pHeader));

	// walk headers, do something, allow a passed in void ptr
	void OperateOnHeaders (void (* lpfn) (TArticleHeader* pArtHeader, void* pVD),
		void* pVoid);

	void OperateOnNodes (int (*lpfn) (TArtNode * pNode, void * pData),
		void * pData);

	// 3rd Try. return non-zero to stop
	int WalkHeaders (TBaseArticleAction* pAction);
	static int WalkHeaders_ProcessNode ( TArtNode * m_pNode, TBaseArticleAction* pAction);

	// for sorting
	int CalcThreadDate(void);
	// used when walking tree. pass in to OperateOnHeaders
	static void fnStepDate (TArticleHeader* pArtHdr, void* pV);
	static void fnStepScore (TArticleHeader* pArtHdr, void* pV);
	static int  fnCompareDate (const CObject* pOb1, const CObject* pOb2);
	static int  fnCompareSubj (const CObject* pOb1, const CObject* pOb2);
	static int  fnCompareScore (const CObject* pOb1, const CObject* pOb2);
	static int  fnCompareByDateBySubj (const CObject* pOb1, const CObject* pOb2);

	int      GetSubject (CString& subj);

	// used for Pile_Compare_Subj_Ascend
	LPCTSTR  GetSubject ();

	BOOL IsFirst(TArtNode* pNode);
	BOOL IsRoot(int iArtInt);

	BOOL HaveN(TIntVector * pIdsToFind);
	void CollectIds(TIntVector * pIdsFound);

	int FindTreeNext (TArtNode * pNode, ThreadFindNext * psFindNext,
		TArtNode*& rpArtNodeOut);

	void myAssertValid ();

	int Merge (TThread *& rpThread);

	// reset depth of each artnode
	void SetDepth ();

	TThreadPile *GetPile () { return m_pPile; }

protected:
	TThread ();                // unused
	TThread (const TThread &); // unused

	TThreadPile * m_pPile;     // parent pile
private:
	void set_root_node (TArtNode * pRootNode);

	TArtNode * m_pRootNode;
};
