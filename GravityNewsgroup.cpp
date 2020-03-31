/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: GravityNewsgroup.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:06  richard_wood
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

// GravityNewsgroup.cpp : Implementation of CGravityNewsgroup
#include "stdafx.h"
#include "NEWS.h"
#include "GravityNewsgroup.h"

#include "comdef.h"

#include "servcp.h"
#include "newsgrp.h"
#include "nglist.h"
#include "server.h"
#include "genutil.h"
#include "newsview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGravityNewsgroup

// ============================================================================
// ============================================================================
STDMETHODIMP CGravityNewsgroup::Open(BSTR bsNewsgroup)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

		_bstr_t  bstNewsgroup(bsNewsgroup, true);  // make copy

	try
	{
		TServerCountedPtr cpNewsServer;
		TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
		TNewsGroupArrayReadLock ngMgr(vNewsGroups);

		int count = vNewsGroups->GetSize();

		bool fFoundIt = false;
		int  id = 0;

		for (int j = 0; j < count; ++j)
		{
			TNewsGroup* pNG = vNewsGroups[j];

			if (pNG->GetName () == (LPCTSTR) bstNewsgroup)
			{
				fFoundIt = true;
				id = pNG->m_GroupID;
				break;
			}
		}

		if (!fFoundIt)
			return E_INVALIDARG;

		CNewsView * pNewsView = GetNewsView();

		pNewsView->OpenNewsgroup (CNewsView::kOpenNormal, 
			CNewsView::kPreferredFilter,
			id);

		m_lGroupID = id;

		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

// ============================================================================
_variant_t convert_array_to_variant (CDWordArray & vData)
{
	_variant_t vOut;
	int len = vData.GetSize();
	if (0 == len)
		return vOut;

	COleSafeArray    mfcSafeArray;

	mfcSafeArray.CreateOneDim (VT_I4,  len);

	for (int i = 0; i < len; i++)
	{
		LONG  lData = vData[i];
		LONG  n = i;

		mfcSafeArray.PutElement (&n, &lData);
	}
	vOut = mfcSafeArray.Detach ();
	return vOut;
}

// ============================================================================
STDMETHODIMP CGravityNewsgroup::GetArticleNumbers(VARIANT * pVar)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

		// TODO: Add your implementation code here
		TServerCountedPtr cpNewsServer;

	try
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, m_lGroupID, &fUseLock, pNG);
		if (fUseLock)
		{
			CDWordArray  rNums;

			pNG->GetLocalBodies (rNums);

			_variant_t  vRes1 = convert_array_to_variant (rNums);

			*pVar = vRes1.Detach ();
			return S_OK;
		}
		else
			return E_FAIL;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

// ============================================================================
// ============================================================================
STDMETHODIMP CGravityNewsgroup::IsArticleProtected(long lArticleNum)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

		// TODO: Add your implementation code here

		TServerCountedPtr cpNewsServer;

	// find it
	try
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, m_lGroupID, &fUseLock, pNG);
		if (fUseLock)
		{
			BOOL fProtected = pNG->IsStatusBitOn(lArticleNum, TStatusUnit::kPermanent);

			if (fProtected)
				return S_OK;
			else
				return S_FALSE;
		}
		else
			return E_FAIL;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

// ============================================================================
// ============================================================================
STDMETHODIMP CGravityNewsgroup::ArticleSetProtected(long lArtNum, long fProtected)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

		// TODO: Add your implementation code here
		TServerCountedPtr cpNewsServer;

	if (fProtected != 1  && fProtected != 0)
		return E_INVALIDARG;

	// find it
	try
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, m_lGroupID, &fUseLock, pNG);
		if (fUseLock)
		{
			pNG->StatusBitSet (lArtNum, 
				TStatusUnit::kPermanent, 
				fProtected);
			return S_OK;         
		}
		else
			return E_FAIL;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

// ============================================================================
// ============================================================================
STDMETHODIMP CGravityNewsgroup::GetArticleByNumber(long lArtNum, BSTR *pbstrArticle)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

		// TODO: Add your implementation code here
		TServerCountedPtr cpNewsServer;

	try
	{
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, m_lGroupID, &fUseLock, pNG);
		if (fUseLock)
		{
			TArticleText sText;

			TAutoClose  sCloser(pNG);

			try
			{   
				pNG->LoadBody (lArtNum, &sText);   
			}
			catch(...)
			{
				return E_INVALIDARG;
			}

			_bstr_t bstFullArt = sText.GetHeader();
			bstFullArt += LPCTSTR("\r\n");
			bstFullArt += LPCTSTR(sText.GetBody());

			*pbstrArticle = bstFullArt.copy();
		}
		else
			return E_FAIL;
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}

