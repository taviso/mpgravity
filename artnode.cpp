/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artnode.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
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

#include "stdafx.h"
#include "afxmt.h"
#include "artnode.h"
#include "article.h"
#include "resource.h"
#include "hierlbx.h"
#include "thread.h"
#include "thredlst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

// live and die at global scope
TMemShack TArtNode::m_gsMemShack(sizeof(TArtNode), "ArtNode");

CCriticalSection sANCritical;

///////////////////////////////////////////////////////////////////////////
ULONG TArtNode::ShrinkMemPool()
{
	if (true)
	{
		CSingleLock sLock(&sANCritical, TRUE);

		m_gsMemShack.Shrink ();
	}

	TArtUIData::ShrinkMemPool ();
	return 0;
}

/* ------------------------------------------------------------ */

#if defined(_DEBUG)
#define new DEBUG_NEW
#endif

TArtNode::TArtNode(TArticleHeader* pHdr, WORD statusBits)
: m_wStatusBits(statusBits)
{
	m_sArts.push_back (pHdr);

	m_pContainer = 0;
	m_pParent  = 0;

	m_depth = 0;
	m_pUIData = 0;

	// hmm - do we want copy_on_write or a Write-Thru
}

TArtNode::~TArtNode(void)
{
	STLMapAHdr::iterator it = m_sArts.begin();

	for (;  it != m_sArts.end(); it++)
	{
		TArticleHeader * pHdr = (*it);
		delete pHdr;
	}

	delete m_pUIData;
}

///////////////////////////////////////////////////////////////////////////
// cooperates with ::GetThread
void TArtNode::ConnectChild(TArtNode* pChild)
{
	pChild->m_pParent = this;

	// clear the ptr to container Thread.  Find this by asking our parent.
	pChild->m_pContainer = NULL;

	m_setKids.insert ( m_setKids.end(), pChild );
}

///////////////////////////////////////////////////////////////////////////
void TArtNode::ConnectChildren(TArtNode* pDeadChild)
{
	STLMapANode::iterator b = m_setKids.begin();
	STLMapANode::iterator e = m_setKids.end();

	STLMapANode::iterator it = find( b, e, pDeadChild );

	if (it == m_setKids.end())
	{
		ASSERT(0);
		return;
	}

	STLMapANode::iterator itDead = it;

	STLMapANode::iterator itGK = pDeadChild->GetKidsItBegin();
	STLMapANode::iterator itGKend = pDeadChild->GetKidsItEnd();

	// take all grandchildren as my own
	for ( ; itGK != itGKend; itGK++ )
	{
		TArtNode * pGrandKid = *itGK;

		pGrandKid->m_pParent = this;
		pGrandKid->RecursiveSetDepth (m_depth + 1);

		it = m_setKids.insert ( ++it, pGrandKid);
	}

	m_setKids.erase (itDead);
}

void TArtNode::RemoveChild ( TArtNode * pChild )
{

	STLMapANode::iterator it = find (m_setKids.begin(), m_setKids.end(), pChild );

	if (it == m_setKids.end())
	{
		ASSERT(0);
		return;
	}

	m_setKids.erase (it);
}

///////////////////////////////////////////////////////////////////////////
// 12-15-95  The Premia tree control has weak sorting capabilities. So
//   sort the kids before we add them to the UI.
int TArtNode::SortKidsByDate ()
{
	int tot = m_setKids.size();

	if (tot)
	{
		m_setKids.sort (   );
		//sort  (  m_setKids.begin(),  m_setKids.end(), ob_less_SortByDate() );
	}
	return tot;
}

// ------------------------------------------------------------------------
// Is this guy in our family?
BOOL TArtNode::Have (TArticleHeader* pArtHdr, TArtNode*& rpNode)
{
	return Have ( pArtHdr->GetNumber(), rpNode );
}

BOOL TArtNode::Have (int iArtInt, TArtNode*& rpNode)
{
	// 5-31-96 Note: this is rather interesting.  With manual rules,
	//   the dialog box loads a threadList, creates an TArticleIndexList
	//   and rules really uses that. (it leverages the miracle of ref-counting)
	//
	//   So when someone calls "TThreadList::RemoveArticle()", testing
	//   the pointer addresses is failing.
	//
	//   So I'm going to compare the Article Numbers

	STLMapAHdr::iterator itH = m_sArts.begin();

	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		if (pHdr->GetNumber() == iArtInt)
		{
			// pass back the Node that has it.
			rpNode = this;
			return TRUE;
		}
	}
	// if (m_pArtHeader->GetNumber() == iArtInt)
	//    {
	//    // pass back the Node that has it.
	//    rpNode = this;
	//    return TRUE;
	//    }
	//

	STLMapANode::iterator it  = m_setKids.begin();
	STLMapANode::iterator itE = m_setKids.end();

	TArtNode * pChild;
	for (; it != itE; it++)
	{
		pChild = *it;
		if (pChild->Have(iArtInt, rpNode))
			return TRUE;
	}
	return FALSE;
}

int TArtNode::GetSubject ( CString& subj )
{
	CString strMin;
	STLMapAHdr::iterator it = m_sArts.begin();

	for (;  it != m_sArts.end(); it++)
	{
		TArticleHeader * pHdr = (*it);

		if (strMin.IsEmpty())
			strMin = pHdr->GetSubject();
		else
			strMin = (strMin < pHdr->GetSubject()) ?  strMin : pHdr->GetSubject();
	}

	subj = strMin;
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Add self, and add the kids.  (Append to the end of the listbox)
//
int TArtNode::AddKids (THierDrawLbx* pHierLbx, int depth, BOOL fCollapsed)
{
	int     iKids = 0;

	m_depth = depth;

	// experiment with passing data into the HierLbx and HE defines
	// the column order

	int uniqNumber = GetArticleNumber();

	iKids = SortKidsByDate();

	BOOL fOpenNode = TRUE;
	if (fCollapsed && iKids > 0)
		fOpenNode = FALSE;

	pHierLbx->AddInfo (this, uniqNumber, fOpenNode);

	// show thread collapsed - don't insert the kids
	if (fCollapsed && (depth+1 > 0))
	{
		if (iKids <= 0)
			return 0;
		// we aren't adding the kids, but recursively set the depth
		RecursiveSetDepth(m_depth);
	}
	else
	{
		if (iKids <= 0)
			return 0;

		STLMapANode::iterator itB = m_setKids.begin();

		TArtNode * pChild;
		for (; itB != m_setKids.end(); itB++)
		{
			pChild =  *itB;

			pChild->AddKids (pHierLbx, depth + 1, fCollapsed );
		}
	}
	return 0;
} // AddKids

///////////////////////////////////////////////////////////////////////////
//
//
int TArtNode::InsertKids(
						 THierDrawLbx * pHierLbx,
						 int   idx,
						 BOOL  fAddSelf,
						 int   iDepth,
						 int&  added
						 )
{
	// have to set depth?
	int iAddIdx;
	//CString control = pHierLbx->GetTemplate();
	CString line;

	if (fAddSelf)
	{
		STLMapAHdr::iterator itH = m_sArts.begin();
		for (; itH != m_sArts.end(); itH++)
		{
			TArticleHeader * pHdr = (*itH);

			pHdr->FormatExt ("fslk", line);
			break;
		}

		m_depth = iDepth;

		iAddIdx = pHierLbx->InsertInfo (idx, line, this,
			this->GetMinUnique());
		++added;
	}
	else
		iAddIdx = idx - 1;

	if (SortKidsByDate() > 0)
	{

		STLMapANode::iterator itB = m_setKids.begin();

		TArtNode * pChild;
		for (; itB != m_setKids.end(); itB++)
		{
			pChild =  *itB;

			iAddIdx = pChild->InsertKids ( pHierLbx, iAddIdx + 1, TRUE,
				iDepth + 1, added );
		}

	}
	return iAddIdx;
}

int TArtNode::GetArticleNumber ()
{
	return this->GetMinUnique();//m_pArtHeader->GetNumber();
}

BOOL TArtNode::IsLastChild()
{
	if (0 == m_pParent)
		return TRUE;

	if (m_pParent->m_setKids.size() <= 0)
		return TRUE;

	STLMapANode::reverse_iterator itR = m_pParent->m_setKids.rbegin();

	TArtNode * pFound = *itR;
	if (this == pFound)
		return TRUE;
	return FALSE;
}

int TArtNode::CountChildren ()
{
	return m_setKids.size();
}

///////////////////////////////////////////////////////////////////////////
//  Goal: reset depth (and lines) for the hierLbx. reset parent ptrs too.
//
//
//  NB: what about the big cleanup when the App shuts down?  we don't
//      walk this tree.
//
void TArtNode::Delink (TThreadPile* pPile, TThread* pThread)
{
	// give children to my dad
	if (m_pParent)
		m_pParent->ConnectChildren (this);
	else
	{
		CPtrArray vec;
		int iMax = -1, n;
		CTime oldest;

		// tell all my children they have no DAD

		STLMapANode::iterator it  = m_setKids.begin();
		STLMapANode::iterator itE = m_setKids.end();

		for (; it != itE; it++)
		{
			TArtNode * pKid = *it;

			pKid->m_pParent = NULL;

			TThread * pAThread = new TThread (pKid, pPile);
			pAThread->CalcThreadDate();
			if ((n=vec.Add (pAThread)) == 0)
			{
				oldest = pAThread->m_oldestTime;
				iMax = n;
			}
			else if (pAThread->m_oldestTime < oldest)
			{
				oldest = pAThread->m_oldestTime;
				iMax = n;
			}
		}

		if (iMax > -1)
		{
			TThread * pOldest = (TThread*) vec.GetAt(iMax);
			vec.RemoveAt(iMax);

			// Pile contains pThread already
			*pThread = *pOldest;
			delete pOldest;

			for (n = 0; n < vec.GetSize(); n++)
			{
				TThread * pT = (TThread*) vec.GetAt(n);
				pThread->Merge ( pT );
			}
			pThread->SetDepth ();
		}
	}

	// I have no parent now
	m_pParent = 0;
	m_setKids.clear ();
}

//-------------------------------------------------------------------------
// Set the correct depth of myself and the kids
//
int TArtNode::RecursiveSetDepth (int depth)
{
	m_depth = depth;

	STLMapANode::iterator it  = m_setKids.begin();
	STLMapANode::iterator itE = m_setKids.end();

	for (;  it != itE; it++)
	{
		TArtNode* pKid = *it;
		pKid->RecursiveSetDepth (depth + 1);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
TThread* TArtNode::GetThread(void)
{
	if (m_pContainer)
		return m_pContainer;
	TArtNode * tmp = m_pParent;
	while (tmp)
	{
		if (tmp->m_pContainer)
			return tmp->m_pContainer;
		tmp = tmp->m_pParent;
	}
	throw(new TException(IDS_ERR_NODE_NOT_SET, kFatal));
	return 0;
}

BOOL TArtNode::FirstInThread()
{
	TThread* pThread = GetThread();
	return pThread->IsFirst(this);
}

// --------------------------------------------------------------------------
// GetDisplayData -- create data struct once, use it many times
int  TArtNode::GetDisplayData (TArtUIData * & rpUIData)
{
	int iDirty = 0;

	if (0 == m_pUIData)
	{
		TArticleHeader * pMinHdr =  0;
		int iLineCount = 0;
		STLMapAHdr::iterator itH = m_sArts.begin();
		for (; itH != m_sArts.end(); itH++)
		{
			TArticleHeader * pHdr = (*itH);

			iLineCount += pHdr->GetLines();

			if (0==pMinHdr)
				pMinHdr = pHdr;
			else
			{
				if ( lstrcmp (pMinHdr->GetSubject(), pHdr->GetSubject()) <= 0)
					pMinHdr = pMinHdr;
				else
					pMinHdr = pHdr;
			}
		}

		ASSERT(pMinHdr);

		iDirty = pMinHdr->MakeUIData (m_pUIData);

		m_pUIData->m_iLines = iLineCount;
	}

	rpUIData = m_pUIData;

	return iDirty;
}

void TArtNode::Isolate ()
{
	m_pParent = 0;
	m_setKids.clear ();
	m_depth = 0;
}

struct THAVEUNREAD
{
	bool m_fHaveUnread;

	THAVEUNREAD::THAVEUNREAD()
		: m_fHaveUnread(false) { }
};

// -------------------------------------------------------------------------
// return non-zero to stop
int xxTestHaveUnread(TArtNode * pNode, void * pData)
{
	if (pNode->GetStatusBits() & TStatusUnit::kNew)
	{
		THAVEUNREAD * pHU = (THAVEUNREAD*) pData;

		pHU->m_fHaveUnread = true;

		return 1;
	}

	return 0;
}

// -------------------------------------------------------------------------
// return true if I am unread or any of my kids are unread
bool TArtNode::HaveUnreadElements ()
{
	TThread * pThread = GetThread ();

	if (NULL == pThread)
		return false;

	THAVEUNREAD  sHU;

	pThread->OperateOnNodes ( xxTestHaveUnread, &sHU);

	return sHU.m_fHaveUnread;
}

// -------------------------------------------------------------------------
// Exception handling in the function is crucial!
//   I think stoopid code generation can make the "catch ..." fail.
//   Just in case I'm turning off optimizations from here on

// Turn off all optimizations
///#pragma optimize( "", off )

// this returns ONE article header, out of the set.
// at least it is consistent
//
TArticleHeader*  TArtNode::GetpRandomHeader (void)
{
	try
	{
		STLMapAHdr::iterator itH = m_sArts.begin();
		for (; itH != m_sArts.end(); itH++)
		{
			TArticleHeader * pHdr = (*itH);
			return pHdr;
		}
		return NULL;

	}
	catch(...)
	{
		return NULL;
	}
}

// Turn optimizations back on
///#pragma optimize( "", on )

bool ob_anode_ptr::operator< (const ob_anode_ptr & y) const
{
	const CTime & tmHdr1 = pNode->GetTime();
	const CTime & tmHdr2 = y.pNode->GetTime();

	if (tmHdr1 < tmHdr2)
		return true;

	if (tmHdr1 > tmHdr2)
		return false;

	// if the times are equal down to the second, check the subject lines
	//   as a tiebreaker
	CString sj1; pNode->GetSubject(sj1);
	CString sj2; y.pNode->GetSubject(sj2);

	if (sj1 >= sj2)
		return false;

	return true;
}

bool ob_ahdr_ptr::operator < (const ob_ahdr_ptr & y) const
{
	int ret = lstrcmp(pHdr->GetSubject (), y.pHdr->GetSubject());

	if (ret < 0)
		return true;
	if (ret > 0)
		return false;

	const CTime & tmHdr1 = pHdr->GetTime();
	const CTime & tmHdr2 = y.pHdr->GetTime();

	if (tmHdr1 < tmHdr2)
		return true;

	if (tmHdr1 > tmHdr2)
		return false;

	return false;
}

int TArtNode::GetMinUnique()
{
	int iMin = 0;
	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);
		if (0==iMin)
		{
			iMin = pHdr->GetNumber();
			break;
		}
		else
		{
			iMin = min(iMin, pHdr->GetNumber());
		}
	}
	return iMin;
}

void TArtNode::ClearpHeader (void)
{
	m_sArts.clear();
}

int  TArtNode::ProcessHeaders ( TBaseArticleAction* pAction )
{
	int ret = 0;
	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		ret = pAction->Run ( pHdr, NULL );
		if (ret)
			break;
	}
	return ret;
}

void TArtNode::OperateOnHeaders (
								 void (* lpfn) (TArticleHeader* pArtHdr, void* pV),
								 PVOID pVoid
								 )
{
	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		lpfn ( pHdr, pVoid );
	}
}

CTime TArtNode::GetTime(void)
{
	bool fFirstTime = true;
	CTime sMinTime;

	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		if (fFirstTime)
		{
			sMinTime = pHdr->GetTime();
			fFirstTime = false;
		}
		else
		{
			sMinTime = min(sMinTime, pHdr->GetTime());
		}

	}
	return sMinTime;
}

void TArtNode::CreateArticleIndex (TArticleIndexList * pIndexLst)
{
	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		pIndexLst->AddItem ( pHdr );
	}
}

int TArtNode::GetLines()
{
	int iCount = 0;
	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);
		iCount += pHdr->GetLines();
	}
	return iCount;
}

void TArtNode::addSiblingParts ( TArticleHeader * pSibHdr )
{
	m_sArts.push_back ( pSibHdr );
}

void TArtNode::RemoveOneOfArts(int iArtInt)
{
	STLMapAHdr::iterator itH = m_sArts.begin();
	for (; itH != m_sArts.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		if (pHdr->GetArticleNumber() == iArtInt)
		{
			m_sArts.erase ( itH );
			return;
		}
	}
}