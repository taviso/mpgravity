/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tpendec.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:13  richard_wood
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

// tpendec.cpp -- pending decode

#include "stdafx.h"           // something below wants this
#include "tpendec.h"          // this file's prototypes
#include "ruleutil.h"         // DB()
#include "globals.h"          // gpStore, ...
#include "names.h"            // FORMAT_HEADERS, NAME_PENDING_DECODE_JOBS
#include "resource.h"         // IDS_*
#include "genutil.h"          // SetStatusBit()
#include "nglist.h"           // TNewsGroupUseLock
#include "server.h"           // TNewsServer
#include "tdecjob.h"          // TDecodeJob

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// how long till a job expires... 7 days
const time_t s_lExpirationPeriod = 60 * 60 * 24 * 7;

TDecodeChart gsChart;

// -------------------------------------------------------------------------
// ConsultPendingJobs -- looks for this message's prefix among the pending
// jobs.  If it doesn't exist, start a new pending job and fail.  If it does
// exist and is still incomplete, add this header and fail.  If it does exist
// and is now complete, succeed and remove the pending job
int TDecodeJob::ConsultPendingJobs (LPCTSTR pchPrefix, int iThis)
{
	TPendingDecode *pJob = NULL;
	POSITION iPos;
	int iResult = 1;
	int i;
	BOOL fUseLock = FALSE;
	TNewsGroupUseLock *pUseLock = 0;
	BOOL bGroupsLocked = FALSE;
	BOOL bGroupOpen = FALSE;
	TServerCountedPtr cpNewsServer;        // smart pointer

	if (gsChart.FindJob (pchPrefix, pJob, iPos)) {
		// job not in the list, so create a new one and insert it

		pJob = new TPendingDecode (m_lGroupID);
		if (!pJob)
			goto end;      // bail out
		pJob -> m_riNums = new LONG [m_iNumParts];
		if (!pJob -> m_riNums)
			goto end;
		memset (pJob -> m_riNums, 0, sizeof (LONG) * m_iNumParts);
		pJob -> m_strPrefix = pchPrefix;
		pJob -> m_iNumParts = m_iNumParts;
		pJob -> m_riNums [iThis-1] = m_sHdr.GetNumber ();
		pJob -> m_iGotParts = 1;
		time (& pJob -> m_lCreateTime);
		gsChart.AddJob (pJob);

		CString strError; strError.Format (_T("(1/%d) "), m_iNumParts);
		SetStatus (strError, IDS_DECODE_WAITING_FOR_PARTS);
		return 1;
	}

	if (m_iNumParts != pJob -> m_iNumParts) {
		SetStatus (IDS_DECODE_CONFLICT_TOTAL);
		return 1;
	}

	// add this to sanitize the next array access
	if ( (iThis <= 0)  ||  (iThis > pJob -> m_iNumParts) )
	{
		SetStatus (IDS_DECODE_CONFLICT_TOTAL);
		return 1;
	}

	// $$ be wary of this array access
	if (pJob -> m_riNums [iThis-1]) {             // yeah, it could happen
		SetStatus (IDS_DECODE_DUPLICATE_PART);
		return 1;
	}

	// add this decode-job to the pending-decode-job that we found
	pJob -> m_riNums [iThis-1] = m_sHdr.GetNumber ();
	pJob -> m_iGotParts ++;

	// if we haven't gotten all parts, stop here
	if (pJob -> m_iGotParts != pJob -> m_iNumParts) {
		CString strError;
		strError.Format (_T("(%d/%d) "), pJob -> m_iGotParts, m_iNumParts);
		SetStatus (strError, IDS_DECODE_WAITING_FOR_PARTS);
		return 1;
	}

	// got all parts... now transfer all information to this decode-job and
	// delete the pending-decode-job
	gsChart.RemoveJob (iPos);

	m_iGotParts = m_iNumParts;
	for (i = 0; i < m_iNumParts; i++)
		m_riFoundParts [i] = TRUE;

	// retrieve each part's header from the database
	BOOL RC;
	TNewsGroup *psNG;
	pUseLock = new TNewsGroupUseLock(cpNewsServer, m_lGroupID, &fUseLock, psNG);

	if (!fUseLock) {
		SetStatus (IDS_DECODE_COULD_NOT_LOCATE);
		goto end;
	}

	psNG -> Open ();
	bGroupOpen = TRUE;
	for (i = 0; i < m_iNumParts; i++) {
		if (i == iThis-1) {
			// already have the header
			m_rsPartHeaders [i] = m_sHdr;
			continue;
		}

		// use pJob -> m_riNums[i] to fetch a header into m_rsPartHeaders[i]
		RC = 0;
		try {
			TArticleHeader *psHeader =
				(TArticleHeader *) psNG -> GetHeader (pJob -> m_riNums [i]);
			if (!psHeader)
				throw(new TException ());

			m_rsPartHeaders [i] = *psHeader;
		}
		catch (TException *pE)
		{
			pE->Delete();
			// pretend that FindObject() returned failure.  What probably happened
			// here is that the newsgroup was unsubscribed, and the collection
			// no longer exists
			RC = 1;
		}

		if (RC) {
			// bail out
			SetStatus (IDS_DECODE_COULD_NOT_LOCATE);
			goto end;
		}
	}

	iResult = 0;

end:

	if (bGroupOpen)
		psNG -> Close ();

	delete pUseLock;

	if (pJob)
		delete pJob;

	return iResult;
}

// -------------------------------------------------------------------------
// PurgePendingJob -- if a pending-job matching the prefix is found, it is
// purged
void TDecodeJob::PurgePendingJob (LPCTSTR pchPrefix)
{
	gsChart.PurgeJob (pchPrefix);
}

// -------------------------------------------------------------------------
// FindJob -- finds a specific job using a subject-line prefix
//            returns 0 for success
//
int TDecodeChart::FindJob (LPCTSTR pchPrefix, TPendingDecode *&pJob,
						   POSITION &iPos)
{
	POSITION iPrevPos;

	iPos = GetHeadPosition();

	while (iPos)
	{
		iPrevPos = iPos;
		pJob = (TPendingDecode *) GetNext (iPos);
		if (0 == _tcscmp (pJob -> m_strPrefix, pchPrefix))
		{
			iPos = iPrevPos;
			return 0;
		}
	}
	return 1;
}

// -------------------------------------------------------------------------
// PurgeJob -- if a job matching the prefix is found, it is purged
void TDecodeChart::PurgeJob (LPCTSTR pchPrefix)
{
	TPendingDecode *pJob;
	POSITION iPos;
	if (!FindJob (pchPrefix, pJob, iPos))
		RemoveJob (iPos);
}

// -------------------------------------------------------------------------
// ~TDecodeChart -- destructor which deallocates all remaining pending-decode
// jobs
TDecodeChart::~TDecodeChart ()
{
	TPendingDecode *pJob;
	while (GetCount ()) {
		pJob = (TPendingDecode *) RemoveHead ();
		delete pJob;
	}
}

// -------------------------------------------------------------------------
// SetStatusBits -- sets a status bit for each article in this job
void TPendingDecode::SetStatusBits (TStatusUnit::EStatus iBit)
{
	CDWordArray vArtNums;

	for (int i = 0; i < m_iNumParts; i++)
		if (m_riNums [i])
			vArtNums.Add (m_riNums[i]);

	// I would like to use GenSetManyObjStatusBit
	//   but I don't have access to the pArtHeader
	//
	GenSetManyStatusBit (m_lGroupID, vArtNums, iBit);
}
