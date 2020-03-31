/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngutil.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/18 22:05:02  richard_wood
/*  Refactored XOVER and XHDR commands so they fetch item data in batches of 300 (or less) if we want > 300 articles.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:37  richard_wood
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
#include "tglobopt.h"
#include "newsgrp.h"
#include "ngutil.h"
#include "rgpurg.h"
#include "server.h"
#include "servcp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions * gpGlobalOptions;

/////////////////////////////////////////////////////////////////////////////
// UtilGetPurgeType - Utility function that gets the purge type for a
//                    newsgroup - it checks if the global settings are
//                    being used and if so, gets them from there.  Otherwise
//
/////////////////////////////////////////////////////////////////////////////

TGlobalDef::EPurgeType UtilGetPurgeType (TNewsGroup *pNG)
{

	return TGlobalDef::kNever;
}

TNewsGroup::EStorageOption UtilGetStorageOption (TNewsGroup *pNG)
{
	//if (pNG->UseGlobalStorageOptions())
	//   return gpGlobalOptions->GetStorageMode ();
	//else
	return pNG->GetStorageOption();
}

LONG  UtilGetPurgeDays (TNewsGroup *pNG)
{
	return 90;
}

BOOL  UtilGetPurgeRead (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetPurgeRead();
	return gpGlobalOptions->GetRegPurge()->GetPurgeRead();
}

int UtilGetPurgeReadLimit (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetPurgeReadLimit();
	return gpGlobalOptions->GetRegPurge()->GetPurgeReadLimit();
}

BOOL  UtilGetPurgeUnread (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetPurgeUnread();
	return gpGlobalOptions->GetRegPurge()->GetPurgeUnread();
}

int UtilGetPurgeUnreadLimit (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetPurgeUnreadLimit();
	return gpGlobalOptions->GetRegPurge()->GetPurgeUnreadLimit();
}

BOOL  UtilGetPurgeOnHdrs (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetPurgeOnHdrs();
	return gpGlobalOptions->GetRegPurge()->GetPurgeOnHdrs();
}

int UtilGetPurgeOnHdrsEvery (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetPurgeOnHdrsEvery();
	return gpGlobalOptions->GetRegPurge()->GetPurgeOnHdrsEvery();
}

BOOL  UtilGetCompactOnExit (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetCompactOnExit();
	return gpGlobalOptions->GetRegPurge()->GetCompactOnExit();
}

int UtilGetCompactOnExitEvery (TNewsGroup* pNG)
{
	if (pNG->IsPurgeOverride())
		return pNG->GetCompactOnExitEvery();
	return gpGlobalOptions->GetRegPurge()->GetCompactOnExitEvery();
}

void  UtilGetLastPurgeTime (TNewsGroup* pNG, CTime* pTime)
{
	// obviously every newsgroup has it's own purge time
	//
	// == there is no Global purge time
	pNG->GetLastPurgeTime (pTime);
}

void  UtilGetLastCompactTime (TNewsGroup* pNG, CTime* pTime)
{
	if (pNG->IsPurgeOverride())
		pNG->GetLastCompactTime (pTime);
	else
		*pTime = gpGlobalOptions->GetRegPurge()->m_lastCompactTime;
}

void UtilSetLastPurgeTime(TNewsGroup* pNG, const CTime& time)
{
	// a newsgroup always keeps a private time, whether the
	//  purge period is Global or private.
	pNG->SetPurgeTime (time);
}

void UtilSetLastCompactTime(TNewsGroup* pNG, const CTime& time)
{
	if (pNG->IsPurgeOverride())
		pNG->SetLastCompactTime (time);
	else
	{
		TRegPurge * pPurge = gpGlobalOptions->GetRegPurge();

		pPurge->m_lastCompactTime = time;
		pPurge->Save ();
	}
}

// return the number for maximum headers
BOOL UtilGetLimitHeaders (TNewsGroup * pNG, int & iLimit)
{
	if (pNG->GetOverrideLimitHeaders())
	{
		iLimit = pNG->GetHeadersLimit ();
		return TRUE;
	}

	TServerCountedPtr cpServer;

	if (cpServer->GetLimitHeaders ())
		iLimit = cpServer->HeadersLimit ();
	else
		iLimit = 40000000;  // a very big number
	return TRUE;
}
