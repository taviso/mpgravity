/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artbank.cpp,v $
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
/*  Revision 1.5  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.4  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:09  richard_wood
/*  Updated for VS 2005
/*  */
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
#include "artbank.h"

#include "tasker.h"
#include "resource.h"
#include "newsdb.h"                    // THeaderIterator
#include "newsgrp.h"
#include "ngutil.h"                    // UtilGetStorageOption
#include "artnode.h"
#include "idxlst.h"                    // TArticleIndexList
#include "server.h"                    // gpNewsStore
#include "vfilter.h"                   // TViewFilter
#include "stldeclare.h"                // VEC_HDRS
#include "dlgquickfilter.h"
#include "tsearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsTasker *gpTasker;
extern TNewsDB *gpStore;

// serialization ctor
TArticleBank::TArticleBank()
{
	m_fHaveOld = m_fHaveNew = FALSE;
	m_pMyViewFilter = NULL;
	m_pServer = 0;
}

TArticleBank::TArticleBank(TNewsServer* pServer)
: m_pServer(pServer)
{
	m_fHaveOld = m_fHaveNew = FALSE;

	m_pMyViewFilter = NULL;

	m_pServer->AddRef ();
}

TArticleBank::~TArticleBank()
{
	// no exceptions can leave a destructor
	try
	{
		if (m_pServer)
		{
			m_pServer->Release ();
			m_pServer = 0;
		}
		Empty ();
	}
	catch(...)
	{}
}

//-------------------------------------------------------------------------
//
//
int  TArticleBank::ChangeContents(BOOL fHitServer, TViewFilter * pFilter,
								  TNewsGroup* pNG, BOOL& fLink)
{
	CSingleLock sLock(&m_crit, TRUE);

	TNewsGroup::EStorageOption eStor = UtilGetStorageOption (pNG);
	int iRet = -1;

	if (TNewsGroup::kHeadersOnly == eStor  ||
		TNewsGroup::kStoreBodies == eStor)
	{
		// read from the database
		iRet = change_contents_mode2 (pFilter, pNG);
		fLink = TRUE;
	}
	else if (TNewsGroup::kNothing == eStor)
	{
	}
	else
	{
		// unknown storage mode
		ASSERT(0);
	}

	TArtUIData::FlushBuffers ();

	return iRet;
}

//-------------------------------------------------------------------------
// global function
void TArticleBank::translate_filter(TViewFilter * pFilter, BOOL& fNeedOld,
									BOOL& fNeedNew)
{
	fNeedOld = FALSE;
	fNeedNew = FALSE;
	TStatusUnit::ETriad eTri = pFilter->GetNew ();
	if (TStatusUnit::kMaybe == eTri)
	{
		fNeedOld = TRUE;
		fNeedNew = TRUE;
	}
	else if (TStatusUnit::kYes == eTri)
		fNeedNew = TRUE;
	else
		fNeedOld = TRUE;
}

//-------------------------------------------------------------------------
BOOL  TArticleBank::need_server(TViewFilter * pFilter)
{
	if (m_fHaveOld && m_fHaveNew)
		return FALSE;

	BOOL fNeedOld, fNeedNew;
	TArticleBank::translate_filter (pFilter, fNeedOld, fNeedNew);

	if (fNeedOld && !m_fHaveOld)
		return TRUE;
	if (fNeedNew && !m_fHaveNew)
		return TRUE;

	return FALSE;
}

/// ------------- prototypes from tdecjob
extern int AlphaCharChecksum (const CString & str);
extern int  ScanBinarySubject (const CString &    strSubject,
							   CString &          strPrefix,
							   CString *          pstrFileName,
							   int &              iNumerator,
							   int &              iTotal,
							   BOOL &             fImpliedPart1,
							   bool &             fValidFraction);

static bool is_all_parts_here (int iTotal, MAP_HDRS * pMap)
{
	return (iTotal > 0) && (pMap->size() == iTotal);
}

#define FLAG_COMPLETE  0x1
#define FLAG_INVALID   0x2

struct  T_COMPLETEBINARY
{
public:
	BYTE     m_byMemberCompleteBinary;
	int      iThisPart;
	int      iTotal;

protected:
	CString  strPrefix;
	int      iChecksum;
public:
	bool     fValidFraction;

public:
	T_COMPLETEBINARY(const CString & strSubject)
		:  m_byMemberCompleteBinary(0)
	{
		BOOL fImpliedPart1;

		iChecksum = AlphaCharChecksum (strSubject);

		ScanBinarySubject (strSubject,      // INPUT
			this->strPrefix,
			NULL, // pstrFileName,
			this->iThisPart,
			this->iTotal,
			fImpliedPart1,
			fValidFraction);
	}

	bool Match(T_COMPLETEBINARY * pCB)
	{
		return  (iChecksum == pCB->iChecksum)  && (strPrefix == pCB->strPrefix);
	}
};

//-------------------------------
// each HdrList is single-author
int EarmarkCompletedBinaries (TArtHdrList * pHL, CPtrList & sList)
{
	POSITION pos = pHL->GetHeadPosition ();

	int   iNumerator = 0;
	int   iTotal     = 0;
	BOOL  fImpliedPart1 = FALSE;

	T_COMPLETEBINARY * pCBData;

	while (pos)
	{
		TArticleHeader * pHdr = pHL->GetNext (pos);

		pCBData = (T_COMPLETEBINARY *) pHdr->m_pvBinary;

		// flags are zero, so we have done ZERO processing on this guy
		if (0 == pCBData->m_byMemberCompleteBinary)
		{
			iNumerator  = pCBData->iThisPart;
			iTotal      = pCBData->iTotal;

			// create checkoff list
			MAP_HDRS * pMap = new MAP_HDRS;

			bool fCompletePost = false;

			if (false == pCBData->fValidFraction)
			{
				// tag it so we don't process it again
				pCBData->m_byMemberCompleteBinary |= FLAG_INVALID;
				goto zoiks;
			}

			if (0 == iTotal) 
			{
				// tag it so we don't process it again
				pCBData->m_byMemberCompleteBinary |= FLAG_INVALID;
				goto zoiks;
			}

			if (iNumerator >= 1 && iNumerator <= iTotal)
			{
				if (pMap->end() == pMap->find (iNumerator))
					pMap->insert ( MAP_HDRS::value_type(iNumerator, pHdr) );
			}

			if (is_all_parts_here (iTotal, pMap))
			{
				pCBData->m_byMemberCompleteBinary |= FLAG_COMPLETE;
				fCompletePost = true;
			}
			else
			{

				// match versus the rest of the (unused) universe
				POSITION p2 = pos;
				while (p2)
				{
					TArticleHeader * pHdr2 = pHL->GetNext(p2);

					T_COMPLETEBINARY * pCBData2 = (T_COMPLETEBINARY *) pHdr2->m_pvBinary;

					if ((0 == pCBData2->m_byMemberCompleteBinary) &&
						(pHdr2->HashKey() !=  pHdr->HashKey())   )
					{
						int iPart = 0;

						if ( pCBData->Match (pCBData2) )
						{
							iPart = pCBData2->iThisPart;
							iTotal= pCBData2->iTotal;

							ASSERT(iPart >= 0);
							///ASSERT(iPart <= iTotal);
							if (iPart >= 1 && iPart <= iTotal)
							{
								if (pMap->end() == pMap->find (iPart))
									pMap->insert ( MAP_HDRS::value_type(iPart, pHdr2) );
							}

							if (is_all_parts_here(iTotal, pMap))
							{
								MAP_HDRS::iterator it = pMap->begin();
								for (; it != pMap->end(); it++)
								{
									TArticleHeader* pHdrZ =  (*it).second;

									T_COMPLETEBINARY * pCBDataZ = (T_COMPLETEBINARY *) pHdrZ->m_pvBinary;

									pCBDataZ->m_byMemberCompleteBinary |= FLAG_COMPLETE;
									fCompletePost  = true;
								}
								break;
							}
						}
					}
				} // while

			} // else clause for 1-of-1

zoiks:
			if (false==fCompletePost )
			{
				MAP_HDRS::iterator it = pMap->begin();
				for (; it != pMap->end(); it++)
				{
					TArticleHeader * pHdr = (*it).second;
					if (pHdr)
					{
						T_COMPLETEBINARY * pCBData2 = (T_COMPLETEBINARY *) pHdr->m_pvBinary;
						pCBData2->m_byMemberCompleteBinary |= FLAG_INVALID;
					}
				}

				pMap->clear();
				delete pMap;
			}
			else
			{
				sList.AddTail (pMap);
			}

		} // still pure
	} // big loop

	return 0;
}

void BinaryProcessHeader (TArticleHeader * pHdr)
{
	CString strSubj = pHdr->GetSubject();

	// tuck it away.
	pHdr->m_pvBinary = new T_COMPLETEBINARY(strSubj);
}

// ------------------------------------------------------------------------
int OnlyShowCompleteBinaries (TArtHdrList * psHdrListIn,
							  CPtrList &  sList)
{
	TArticleHeader *     pHdr;

	CMapStringToPtr  sMasterMap;

	PVOID  pVoid;
	POSITION pos = psHdrListIn->GetHeadPosition();

	while (pos)
	{
		pHdr = psHdrListIn->GetNext(pos);

		if (pHdr)
		{
			BinaryProcessHeader (pHdr);

			// preliminary breakdown by Author
			CString strFrom = pHdr->GetFrom();

			BOOL fFound = sMasterMap.Lookup (strFrom, pVoid);

			if (fFound)
			{
				TArtHdrList *  pNLfrom = (TArtHdrList*) pVoid;

				pNLfrom->AddTail (pHdr);
			}
			else
			{
				TArtHdrList * pNLfrom = new TArtHdrList;
				pNLfrom->AddTail (pHdr);
				sMasterMap.SetAt (LPCTSTR(strFrom), pNLfrom);
			}
		}

	}

	// sMasterMap has a bunch of HdrLists. Each HdrList is single-author.

	CString strKey;
	pos = sMasterMap.GetStartPosition ();
	while (pos)
	{
		sMasterMap.GetNextAssoc (pos, strKey, pVoid);

		TArtHdrList * pHL = (TArtHdrList*) pVoid;

		EarmarkCompletedBinaries (pHL, sList);

		POSITION p3 = pHL->GetHeadPosition();
		while (p3)
		{
			TArticleHeader * pHdr = pHL->GetNext(p3);
			T_COMPLETEBINARY * pCBData = (T_COMPLETEBINARY *) pHdr->m_pvBinary;

			// delete bookkeeping struct
			delete pCBData;
		}

		delete pHL;  // clean up sMasterMap
	}

	// good shit in sList
	return 0;
}

// dirty prototype
extern   TQuickFilterData * fnGetQuickFilterData ();

//-------------------------------------------------------------------------
void fnApplyQuickFilter (TArtHdrList & sHdrList1,
						 TQuickFilterData * pData,
						 TArtHdrList & sHdrList2)
{
	CString      subj, from;
	TSearch      search;
	int          iResultLen;
	DWORD        dwPos;

	if (pData->m_fRE)
		search.SetPattern (pData->m_strText, FALSE, TSearch::RE);
	else
		search.SetPattern (pData->m_strText, FALSE, TSearch::NON_RE);

	POSITION pos = sHdrList1.GetHeadPosition();
	while (pos)
	{
		TArticleHeader * pHdr = sHdrList1.GetNext (pos);

		if (pData->m_fSubj)
		{
			if (search.Search (pHdr->GetSubject(), iResultLen, dwPos))
			{
				sHdrList2.AddTail (pHdr);
				continue;
			}
		}

		if (pData->m_fFrom)
		{
			if (search.Search (pHdr->GetFrom(), iResultLen, dwPos))
			{
				sHdrList2.AddTail (pHdr);
			}
		}
	}
}

//-------------------------------------------------------------------------
//  load these headers from the database. When we are done we have a
//  foundation to begin threading the articles together
int TArticleBank::change_contents_mode2 (TViewFilter * pFilter, TNewsGroup* pNG)
{
	TPersist822Header* pHdr = 0;
	TPersistentTags & rTags = getp_server()->GetPersistentTags();

	TAutoClose sAutoClose ( pNG );

	// Use a Readlock here, the article bank is the
	//  object we are "writing" to and it has it's own crit-sect

	pFilter->BeginRun ();

	// special case - 'show entire thread if any element is (unread)'
	BOOL fShowEntireThread = pFilter->getShowThread();

	THeaderIterator it(pNG, THeaderIterator::kReadLock);

	// ############# $$$$ NEW STUFF  $$$$$

	TArtHdrList sHdrList1;

	TArtHdrList  sHdrList2;

	TArtHdrList * psList = 0;

	while (it.Next(pHdr))
	{
		TArticleHeader * pArtHdr = pHdr->CastToArticleHeader();
		if (NULL == pArtHdr)
			continue;

		if (fShowEntireThread || pFilter->FilterMatch (pArtHdr, pNG, &rTags))
		{
			sHdrList1.AddTail (pArtHdr);
		}
	}

	TQuickFilterData * pData = fnGetQuickFilterData ();

	if (pData && FALSE==fShowEntireThread)
	{
		fnApplyQuickFilter (sHdrList1, pData, sHdrList2);
		psList = &sHdrList2;
	}
	else
		psList = &sHdrList1;

	if (pFilter->isCompleteBinaries())
	{
		CPtrList    sVecList;

		OnlyShowCompleteBinaries (psList, sVecList);

		POSITION pos = sVecList.GetHeadPosition();
		while  (pos)
		{
			MAP_HDRS * pMap = (MAP_HDRS *)  sVecList.GetNext(pos);

			insert_headers ( pMap, NULL, pNG );

			pMap->clear();
			delete pMap;
		}
	}
	else
	{
		POSITION pos = psList->GetHeadPosition();
		while (pos)
		{
			TArticleHeader * pArtHdr = psList->GetNext (pos);
			insert_headers ( NULL, pArtHdr, pNG );
		}
	}
	return 0;
}

//-------------------------------------------------------------------------
int  TArticleBank::LoadFromDB(TViewFilter* pFilter, TNewsGroup* pNG)
{
	load_from_db (pFilter, pNG);
	return 0;
}

//-------------------------------------------------------------------------
// returns count of added articles
int  TArticleBank::load_from_db (TViewFilter * pFilter, TNewsGroup* pNG)
{
	TPersist822Header* pHdr = 0;
	int  artInt;
	int  iTotal = 0;

	TPersistentTags & rTags = getp_server()->GetPersistentTags();
	TAutoClose sAutoClose ( pNG );

	// Use a Readlock here, the article bank is the
	//  object we are "writing" to and it has it's own crit-sect

	pFilter->BeginRun ();
	THeaderIterator it(pNG, THeaderIterator::kReadLock);

	// take each header and add it to the m_Stage

	while (it.Next (pHdr))
	{
		// note - perhaps we should just add everything from the db.
		TArticleHeader * pArtHdr = pHdr->CastToArticleHeader ();

		if (!pArtHdr)
			continue;
		artInt = pArtHdr->GetNumber ();

		if (pFilter->FilterMatch (pArtHdr, pNG, &rTags) &&
			!m_ids.Have(artInt))
		{

			insert_headers (NULL, pHdr->CastToArticleHeader(), pNG);

			++iTotal;
		}
	}

	return iTotal;
}

//-------------------------------------------------------------------------
//
int  TArticleBank::change_contents_mode1_internal(TViewFilter * pFilter, TNewsGroup* pNG)
{

	// just some subtraction
	return 0;
}

//-------------------------------------------------------------------------
// Return 0 for success
int  TArticleBank::artid_we_need(BOOL fOld, BOOL fNew, TNewsGroup* pNG,
								 int lo, int hi, int iHdrLimit, TRangeSet* pSet)
{
	// these loops operate backwards.  If user is limiting headers,
	//     get the most recent articles (high numbers) first

	int iTotal = 0;
	if (fOld && fNew)
	{
		// request everything
		for (int i = hi - 1; i >= lo; i--)
			if (!m_ids.Have(i))
			{
				pSet->Add(i);
				if (++iTotal >= iHdrLimit)
					break;
			}
	}
	else if (fOld)
	{
		for (int i = hi - 1; i >= lo; i--)
			if (!m_ids.Have(i) && pNG->ReadRangeHave(i))
			{
				pSet->Add(i);
				if (++iTotal >= iHdrLimit)
					break;
			}
	}
	else if (fNew)
	{
		for (int i = hi - 1; i >= lo; i--)
			if (!m_ids.Have(i) && !pNG->ReadRangeHave(i))
			{
				pSet->Add(i);
				if (++iTotal >= iHdrLimit)
					break;
			}
	}

	TRACE1("We need %d arts for mode1\n", iTotal);
	return 0;
}

// ------------------------------------------------------------------------
// insert_header -- add header to our collections
BOOL TArticleBank::insert_headers (
								   PVOID           pVoid,
								   TArticleHeader* pArtHdr,
								   TNewsGroup *    pNG)
{
	if (0 == pArtHdr && 0 == pVoid)
		return FALSE;

	CSingleLock sLock(&m_crit, TRUE);

	if (pArtHdr)
	{

		WORD wArtStatus = 0;

		if (pNG->iStatusDirect (pArtHdr->GetNumber(), wArtStatus) != 0)
			return FALSE;

		if (StageLookup( pArtHdr ))
		{
			// caller retains ownership
			return FALSE;
		}
		else
		{

			TArticleHeader * pCopy = new TArticleHeader (*pArtHdr);

			m_ids.Add ( pCopy->GetNumber() );

			// some Readers don't preserve case on msg-ids.
			CString strCaps = pCopy->GetMessageID();
			strCaps.MakeUpper ();
			TArtNode * pNode = new TArtNode(pCopy, wArtStatus);
			m_Stage.insert( NodeMap::value_type(strCaps, pNode) );

			TArtUIData * pJunk;

			// force formation of display data.  Note : a large part of
			//   the effort here is parsing the smtp address into the
			//   component parts
			int iDirty = pNode->GetDisplayData (pJunk);

			// hopefully the newsgroup will save this stuff
			if (iDirty)
				pNG->HeadersDirty ();

			return TRUE;
		}
	}
	else
	{
		bool fAllGood = true;
		CDWordArray  rStati;

		MAP_HDRS * pMap = (MAP_HDRS*) pVoid;

		// phase 1 :  check all the stati, to see if they are all there

		MAP_HDRS::iterator it = pMap->begin();
		for (; it != pMap->end(); it++)
		{
			WORD wArtStatus = 0;
			TArticleHeader * pArtHdr = (*it).second;
			if (pNG->iStatusDirect (pArtHdr->GetNumber(), wArtStatus) != 0)
			{
				fAllGood = false;  // don't add any??
				return FALSE;
			}
			else
				rStati.Add (wArtStatus);
		}

		bool fFirst = true;

		// phase 2 : find least subject

		it = pMap->begin();
		MAP_HDRS::iterator itSpecial;
		LPCTSTR pszSubj = 0;
		for (; it != pMap->end(); it++)
		{
			if (0==pszSubj)
			{
				pszSubj = ((*it).second)->GetSubject();
				itSpecial = it;
			}
			else
			{
				TArticleHeader * pArtHdr  = (*it).second;

				LPCTSTR pszS2 = pArtHdr->GetSubject();

				if (-1 == lstrcmp (pszS2, pszSubj))
				{
					pszSubj = pszS2;
					itSpecial = it;
				}
			}
		}

		// phase 3 : hdr with least subject is seed for artnode

		TArticleHeader * pArtHdr  = (*itSpecial).second;

		TArticleHeader * pCopy = new TArticleHeader (*pArtHdr);
		m_ids.Add ( pCopy->GetNumber() );

		// some Readers don't preserve case on msg-ids.
		CString strCaps = pCopy->GetMessageID();
		strCaps.MakeUpper ();

		TArtNode * pNode = new TArtNode(pCopy, WORD(rStati[0]) );

		m_Stage.insert (NodeMap::value_type(strCaps, pNode));

		// phase 4 : do the rest

		it =  pMap->begin();
		for (; it != pMap->end(); it++)
		{
			if (it == itSpecial)
				continue; // we did u already

			TArticleHeader * pArtHdr  = (*it).second;
			TArticleHeader * pCopy = new TArticleHeader (*pArtHdr);
			m_ids.Add ( pCopy->GetNumber() );

			pNode->addSiblingParts ( pCopy );
		} // loop through vec

		// phase 5 : generate the display data

		TArtUIData * pJunk;

		// force formation of display data.  Note : a large part of
		//   the effort here is parsing the smtp address into the
		//   component parts
		int iDirty = pNode->GetDisplayData (pJunk);

		// hopefully the newsgroup will save this stuff
		if (iDirty)
			pNG->HeadersDirty ();

	}  // else 'multiple

	return TRUE;
}

//-------------------------------------------------------------------------
// StageLookup --
//
// called from  insert_header()
//
BOOL TArticleBank::StageLookup(TArticleHeader* pArtHdr)
{
	// everything we insert is upper-case
	CString strCaps(pArtHdr->GetMessageID());
	strCaps.MakeUpper ();

	CSingleLock sLock(&m_crit, TRUE);

	NodeMap::iterator it = m_Stage.find ( strCaps );
	if (it != m_Stage.end())
	{
		TRACE1("ARTBANK: squatter found %s\n",
			(LPCTSTR) pArtHdr->GetMessageID());
		return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------
int TArticleBank::CreateArticleIndex (TArticleIndexList * pIndexLst)
{
	ASSERT( pIndexLst );

	CSingleLock sLock(&m_crit, TRUE);

	// RLW - only call PuntLinkInformation if there are articles to move to m_setNodes
	if (GetCount() > 0)
	{
		// move m_Stage to m_setNodes
		PuntLinkInformation ();
	}

	// Note: flat list is created during Link().  So go directly(!) to
	//  the m_setNodes

	STLMapArtNode::iterator it = m_setNodes.begin();

	while (it != m_setNodes.end() )
	{
		TArtNode * pNode = *it++;

		ASSERT(pNode);

		pNode->CreateArticleIndex ( pIndexLst );

	}
	return 0;
}

//-------------------------------------------------------------------------
int TArticleBank::Empty ()
{
	CSingleLock sLock(&m_crit, TRUE);

	ASSERT(m_Stage.size() == 0);
	m_Stage.clear();

	STLMapArtNode::iterator it = m_setNodes.begin();

	while (it != m_setNodes.end())
	{
		TArtNode * pNode = *it;

		// delete call  (1/3)
		delete pNode;
		it++;
	}
	m_setNodes.erase (m_setNodes.begin(), m_setNodes.end());

	m_ids.Empty();

	delete m_pMyViewFilter;
	m_pMyViewFilter = 0;

	return 0;
}

//-------------------------------------------------------------------------
BOOL TArticleBank::UnhookNode (TArtNode * pNode)
{
	CSingleLock sLock(&m_crit, TRUE);

	STLMapArtNode::iterator it = m_setNodes.find ( pNode );

	if (it != m_setNodes.end())
	{
		TRACE1("UnhookNode: %x\n", (DWORD) pNode);

		m_setNodes.erase (it);
		m_ids.Subtract ( pNode->GetArticleNumber() );

	}
	else
	{
		ASSERT(0);
	}

	return TRUE;
}

//-------------------------------------------------------------------------
void TArticleBank::DeleteNode ( TArtNode * & rpNode )
{
	// RLW - This call should not do anything anyway, but...
	UnhookNode(rpNode);

	// delete call 2/3
	delete rpNode;

	rpNode = 0;
}

//-------------------------------------------------------------------------
bool TArticleBank::GetIteratorEx( STLMapArtNode::iterator & itBegin,
								 STLMapArtNode::iterator & itEnd )
{

	m_fShowImportant = FALSE;
	// clean up old threading info (shear off any children)
	// since we are re-threading
	isolate_all_nodes ();

	// copy this filter
	//if (pFilter)
	//   replace_filter ( pFilter );

	itBegin = m_setNodes.begin();
	itEnd   = m_setNodes.end();
	return true;
}

//-------------------------------------------------------------------------
void TArticleBank::InitHashTable(UINT uSize)
{
	// empty
}

//-------------------------------------------------------------------------
int  TArticleBank::GetCount()
{
	CSingleLock sLock(&m_crit, TRUE);

	return m_Stage.size();
}

//  jan-18-99  looks fishy, find out WHEN this is called
// called during threading, so it's ok
//-------------------------------------------------------------------------
BOOL TArticleBank::Lookup(CString& key, TArtNode*& rpNode)
{
	CString strCaps(key);
	strCaps.MakeUpper ();

	CSingleLock sLock(&m_crit, TRUE);

	BOOL found = FALSE;
	NodeMap::iterator it = m_Stage.find ( strCaps );
	if (it != m_Stage.end())
	{
		found = TRUE;
		rpNode = (*it).second;
	}
	// everything we insert is upper-case
	return found;
}

//-------------------------------------------------------------------------
void TArticleBank::TagContents(BOOL fOld, BOOL fNew)
{
	m_fHaveOld = fOld;
	m_fHaveNew = fNew;
}

//-------------------------------------------------------------------------
void TArticleBank::isolate_all_nodes(void)
{
	CSingleLock sLock(&m_crit, TRUE);

	STLMapArtNode::iterator it = m_setNodes.begin();

	while (it != m_setNodes.end())
	{
		TArtNode * pNode = *it++;

		pNode->Isolate ();
	}
}

//-------------------------------------------------------------------------
//  if we are Filtering important articles, weed out non-important articles.
BOOL TArticleBank::filter_important(TNewsGroup* pNG, int iArtInt)
{
	if (!m_fShowImportant)
		return TRUE;

	if (pNG->IsStatusBitOn (iArtInt, TStatusUnit::kImportant))
		return TRUE;
	else
		return FALSE;
}

// -----------------------------------------------------------------------
// set the filter used for enumeration during GetStartPositionEx()
void TArticleBank::replace_filter (TViewFilter * pFilter)
{
	delete m_pMyViewFilter;

	m_pMyViewFilter = new TViewFilter (*pFilter);
}

// -----------------------------------------------------------------------
TNewsServer* TArticleBank::getp_server ()
{
	if (m_pServer)
		return m_pServer;

	ASSERT(m_ServerName.GetLength());

	// setup ptr on demand

	m_pServer = gpStore->GetServerByName (m_ServerName);
	m_pServer->AddRef ();
	return m_pServer;
}

// -----------------------------------------------------------------------
void TArticleBank::PuntLinkInformation ()
{
	CSingleLock sLock(&m_crit, TRUE);

	// move m_Stage  into   m_setNodes

	ASSERT(m_setNodes.size() == 0);

	NodeMap::iterator it = m_Stage.begin();

	for (; it != m_Stage.end(); it++)
	{
		TArtNode * pNode = (*it).second;

		if (pNode)
			m_setNodes.insert ( pNode );
	}

	m_Stage.clear ();

	// after Linking, we want to hold onto the ptrs, but we don't necessarily
	//    need to hang onto the MSGID-> ptr  Map.  Takes up too much memory
}

// -----------------------------------------------------------------------
int TArticleBank::SanityCheck_CountNodes ()
{
	CSingleLock sLock(&m_crit, TRUE);

	return m_setNodes.size();
}

