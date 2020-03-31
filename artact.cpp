/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artact.cpp,v $
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
/*  Revision 1.3  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:09  richard_wood
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
#include "artact.h"

#include "newsgrp.h"
#include "thredlst.h"
#include "article.h"
#include "vfilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
// TArticleAction -- constructor
TArticleAction::TArticleAction(TNewsGroup* pNG, TStatusUnit::EStatus kStatus,
							   BOOL fOn)
							   : m_pNG(pNG), m_kStatus(kStatus), m_fOn(fOn)
{
	ASSERT(m_kStatus != 0x1);
	// disallow turning TStatusUnit::k R e a d ON. You must turn
	//   TStatusUnit::kNew OFF.
}

// return 0 to continue
int TArticleAction::Run (TPersist822Header* pHdr, LPVOID pVoid)
{
	ASSERT(m_pNG);

	// ReadRangeAdd does this already.  But this is here if the
	//  status bit is Important or something
	if (m_kStatus != TStatusUnit::kNew)
	{
		m_pNG->StatusBitSet (pHdr->GetNumber(),
			m_kStatus,
			m_fOn);
	}

	// do cross-post management too.
	if (m_kStatus == TStatusUnit::kNew && !m_fOn)
		m_pNG->ReadRangeAdd ( pHdr->CastToArticleHeader() );
	else if (m_kStatus == TStatusUnit::kNew && m_fOn)
		m_pNG->ReadRangeSubtract ( pHdr->CastToArticleHeader(), KThreadList::kHandleCrossPost );

	return 0;
}

TArticleAction::~TArticleAction() {}

///////////////////////////////////////////////////////////////////////////
// Retrieve the LP_NODE
//

TArticleActionFetch::TArticleActionFetch(int artInt)
	: m_artInt(artInt)
{
	ASSERT(m_artInt != -1);
	m_pVoid = 0;
}

int TArticleActionFetch::Run(TPersist822Header* pHdr, LPVOID pVoid)
{
	// check article number
	ASSERT(m_artInt != -1);
	if (pHdr->GetNumber() == m_artInt)
	{
		m_pVoid = pVoid;    // this is a LP_NODE
		m_pHeader = (TArticleHeader*) pHdr;
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------------------
// Override
int TArticleActionHaveN::Run(TPersist822Header* pHdr, LPVOID pVoid)
{
	TArticleHeader* pArtHdr = pHdr->CastToArticleHeader();

	if (!pArtHdr)
		return 0;

	if (m_pIdsToFind->Have (pArtHdr->GetNumber()))
	{
		m_fFound = TRUE;
		return 1;
	}
	return 0;
}

// ------------------------------------------------------------------------
// Override
int TArticleActionCollectIds::Run(TPersist822Header* pHdr, LPVOID pVoid)
{
	TArticleHeader* pArtHdr = pHdr->CastToArticleHeader();

	if (pArtHdr)
		m_pIdsFound->Add (pArtHdr->GetNumber());

	return 0;
}

// ------------------------------------------------------------------------
int TArticleActionSetPileScore::Run (TPersist822Header *pHdr, LPVOID pVoid)
{
	((TArticleHeader *) pHdr)->SetPileScore (m_lScore);
	((TArticleHeader *) pHdr)->SetPileZeroPresent (m_bZeroPresent);
	return 0;
}

// ------------------------------------------------------------------------
// used to see if any element of threadpile matches the filter

TArticleActionElementMatch::TArticleActionElementMatch (
	TViewFilter *     pFilter,
	TNewsGroup  *     pNG,
	TPersistentTags * pTags,
	TQuickFilterData * pQuickFilterData)
{
	m_pFilter = pFilter;
	m_pNG     = pNG;
	m_pTags   = pTags;

	m_fElementMatch = FALSE;

	m_fQuickFilter  = FALSE;

	if (pQuickFilterData)
	{
		m_fQuickFilter = TRUE;
		m_fQuickFrom   = pQuickFilterData->m_fFrom;
		m_fQuickSubj   = pQuickFilterData->m_fSubj;
		m_search.SetPattern (pQuickFilterData->m_strText, FALSE, pQuickFilterData->m_fRE ? TSearch::RE : TSearch::NON_RE);
	}
}

// ------------------------------------------------------------------------
// return 0 to continue
int TArticleActionElementMatch::Run (TPersist822Header* pHdr, LPVOID pVoid)
{
	int          iResultLen;
	DWORD        dwPos;
	TArticleHeader * pArtHdr  = pHdr->CastToArticleHeader();

	if (m_pFilter->FilterMatch ( pArtHdr, m_pNG, m_pTags ))
	{
		if (m_fQuickFilter)
		{
			if (m_fQuickSubj)
			{
				if (m_search.Search (pArtHdr->GetSubject(), iResultLen, dwPos))
				{
					m_fElementMatch = TRUE;
					return 1;  // stop
				}
			}

			if (m_fQuickFrom)
			{
				if (m_search.Search (pArtHdr->GetFrom(), iResultLen, dwPos))
				{
					m_fElementMatch = TRUE;
					return 1;  // stop
				}
			}

		}
		else
		{
			m_fElementMatch = TRUE;
			return 1;  // stop
		}
	}

	return 0;
}
