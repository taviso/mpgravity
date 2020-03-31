/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artnode.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:11  richard_wood
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
#include "artdata.h"          // TArtUIData
#include "memshak.h"
#include "superstl.h"
#include "artact.h"
#include "idxlst.h"

class THierDrawLbx;
class TArticleBank;

class TArtNode;  // fwd
typedef TArtNode * PARTNODE;

class ob_anode_ptr
{
public:
	ob_anode_ptr()  {  pNode = 0; }

	ob_anode_ptr(TArtNode *pN)  { pNode = pN; }

	ob_anode_ptr(const ob_anode_ptr & src)
	{
		pNode = src.pNode;
	}

	operator TArtNode*() { return pNode; }

	bool operator< (const ob_anode_ptr & rhs) const;

	//bool operator== (const TArtNode * pNodeRHS) const
	//{
	//	return (DWORD) pNode ==  (DWORD)(pNodeRHS);
	//}
	TArtNode * pNode;
};

typedef list<ob_anode_ptr > STLMapANode;

// ############################
class ob_ahdr_ptr
{
public:
	ob_ahdr_ptr()  {  pHdr = 0; }

	ob_ahdr_ptr(TArticleHeader *pH)  { pHdr = pH; }

	ob_ahdr_ptr(const ob_ahdr_ptr & src)
	{
		pHdr = src.pHdr;
	}

	operator TArticleHeader*() { return pHdr; }

	bool operator< (const ob_ahdr_ptr & rhs) const;

	bool operator== (const TArticleHeader * pHdrRHS) const
	{
		return (DWORD) pHdr ==  (DWORD)(pHdrRHS);
	}
	TArticleHeader * pHdr;
};

typedef list<ob_ahdr_ptr > STLMapAHdr;

class TArtNode : public CObject {
public:
	friend TThread;
	static ULONG  ShrinkMemPool();
	static TMemShack m_gsMemShack;

public:
	int m_depth;

	TArtNode(TArticleHeader* pArtHdr, WORD statusBits);
	~TArtNode(void);

	static BOOL fnDateLESSEQ(const CObject* pObj1, const CObject* pObj2);

	TArtNode*        GetParent (void)          { return m_pParent;  }
	void             SetParent (TArtNode* dad) { m_pParent = dad;   }
	TArticleHeader*  GetpRandomHeader (void);
	void             ClearpHeader (void);
	BOOL       IsLastChild ();

	BOOL       FindDad(TArticleBank* pBank, TArtNode*& pDad);

	void ConnectChild (TArtNode * pChild);
	void ConnectChildren(TArtNode * pChild);

	void RemoveChild (TArtNode* pChild);

	int  AddKids (THierDrawLbx* pHierLbx, int depth, BOOL fCollapsed);

	int  InsertKids(THierDrawLbx * pHierLbx, int idx, BOOL fAddSelf,
		int depth, int& added);

	BOOL       Have (TArticleHeader* pArtHdr, TArtNode*& rpNode);
	BOOL       Have (int iArtInt, TArtNode*& rpNode);

	bool       HaveUnreadElements ();

	STLMapANode::iterator  GetKidsItBegin () { return m_setKids.begin() ; }
	STLMapANode::iterator  GetKidsItEnd () { return m_setKids.end() ; }

	STLMapAHdr::iterator   GetArtsItBegin() { return m_sArts.begin(); }
	STLMapAHdr::iterator   GetArtsItEnd()   { return m_sArts.end();   }
	UINT                   GetArtsCount()   { return m_sArts.size();  }
	void                   RemoveOneOfArts(int iArtInt);

	void addSiblingParts ( TArticleHeader * pSibHdr );

	void TArtNode::CreateArticleIndex (TArticleIndexList * pIndexLst);

	void TArtNode::OperateOnHeaders (
		void (* lpfn) (TArticleHeader* pArtHdr, void* pV),
		PVOID pVoid
		);

	int  TArtNode::ProcessHeaders ( TBaseArticleAction* pAction );

	int         GetSubject ( CString& subj );
	LPCTSTR     GetpSubject ();

	CTime GetTime(void);

	int GetLines();

	int GetArticleNumber ();
	int CountChildren ();

	void Delink (TThreadPile* pPile, TThread* pThread);

	// only the root node has a ptr to the containing thread
	void SetRootnodeThread(TThread* pThread) { m_pContainer = pThread; }
	TThread* GetThread(void);

	BOOL FirstInThread();

	// return 1 if arthdr becomes dirty
	int  GetDisplayData (TArtUIData *& rpUIData);
	void Isolate ();

	WORD GetStatusBits (void)      { return m_wStatusBits; }
	void SetStatusBits (WORD bits) { m_wStatusBits = bits; }

protected:
	int  SortKidsByDate();
	int  RecursiveSetDepth (int depth);

	int  GetMinUnique();

private:
	TThread*  m_pContainer;
	TArtNode* m_pParent;

	STLMapANode  m_setKids;

	STLMapAHdr   m_sArts;

	TArtUIData * m_pUIData;

private:
	// data

	WORD  m_wStatusBits;       // status bits used for painting
};
