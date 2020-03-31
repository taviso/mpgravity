/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tpendec.h,v $
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

// tpendec.h -- pending decode

#pragma once

#include <afxcoll.h>             // CObList, POSITION
#include "pobject.h"             // oid
#include "time.h"                // time_t
#include "statunit.h"            // TStatusUnit

class TDecodeJob;
class TDecodeChart;

// -------------------------------------------------------------------------
// TPendingDecode -- decode job waiting for some parts to come in
class TPendingDecode : public CObject
{
	friend class TDecodeJob;
	friend class TDecodeChart;

public:
	// first, default constructor for dynamic allocation
	TPendingDecode()
	{ m_riNums = NULL; }
	TPendingDecode(LONG lGroupID)
	{ m_lGroupID = lGroupID; m_riNums = NULL; }
	~TPendingDecode()
	{
		if (m_riNums)
			delete [] m_riNums;
	}

private:
	void SetStatusBits (TStatusUnit::EStatus iBit);

	CString m_strPrefix;       // prefix of the subject
	int m_iNumParts;           // how many parts to this post?
	int m_iGotParts;           // how many parts do we have?
	LONG *m_riNums;            // array of OIDs for the parts that we already
	// have
	time_t m_lCreateTime;      // time that this job was created
	LONG m_lGroupID;           // ID of newsgroup

	static const time_t s_lExpirationPeriod;  // how long till a job expires
};

// -------------------------------------------------------------------------
// TDecodeChart -- list of pending jobs
class TDecodeChart : public CObList
{
public:
	~TDecodeChart ();          // destructor

	int FindJob (LPCTSTR pchPrefix, TPendingDecode *&pJob, POSITION &iPos);
	void AddJob (TPendingDecode *pJob) { AddHead ((CObject *) pJob); }
	void RemoveJob (POSITION iPos) { RemoveAt (iPos); }
	void PurgeJob (LPCTSTR pchPrefix);
};
