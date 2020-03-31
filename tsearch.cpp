/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsearch.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:19  richard_wood
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

// tsearch.cpp -- text-search.  Uses regular-expression searching when the
// pattern contains metacharacters, and uses the faster non-r.e. searching
// when the pattern does not contain metacharacters

#include "stdafx.h"              // windows stuff
#include "tsearch.h"             // this file's prototypes
#include "mplib.h"               // TException, ...
#include "resource.h"            // IDS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TSearch::TSearch()
{
	m_psBoyerMoore = NULL;
	m_psRxSearch = NULL;
	m_iPatternLen = 0;
}

// -------------------------------------------------------------------------
TSearch::~TSearch()
{
	Deallocate();
}

// -------------------------------------------------------------------------
void TSearch::Deallocate()
{
	if (m_psBoyerMoore)
	{
		delete m_psBoyerMoore;
		m_psBoyerMoore = NULL;
	}
	if (m_psRxSearch)
	{
		delete m_psRxSearch;
		m_psRxSearch = NULL;
	}
}

// -------------------------------------------------------------------------
static BOOL PatternContainsMetachars(LPCTSTR pchPattern)
{
	int iLen = strlen(pchPattern);
	char ch;
	for (int i = 0; i < iLen; i++)
	{
		ch = pchPattern[i];

		// the following is a list of characters whose presence indicates
		// and R.E.
		//        < [ ( { . | $ * +
		if (ch == '<' || ch == '[' || ch == '(' || ch == '{' || ch == '.' ||
			ch == '|' || ch == '$' || ch == '*' || ch == '+')
			return TRUE;
	}

	return FALSE;
}

// -------------------------------------------------------------------------
void TSearch::SetPattern(LPCTSTR pchPattern,
						 BOOL fCaseSensitive /* = FALSE */,
						 ESearchType iSearchType /* = SCAN_PATTERN */)
{
	ASSERT(pchPattern);

	if (m_psRxSearch || m_psBoyerMoore)
		Deallocate();

	if (iSearchType == SCAN_PATTERN)
		iSearchType = PatternContainsMetachars(pchPattern) ? RE : NON_RE;

	if (iSearchType == RE)
	{
		m_psRxSearch = new RxSearch;
		if (!m_psRxSearch)
			throw(new TException(IDS_ERR_REGEXP_ALLOC, kError));
		if (m_psRxSearch->Compile(pchPattern, !fCaseSensitive))
			throw(new TException(IDS_ERR_REGEXP_COMPILE, kError));
	}
	else
	{
		m_psBoyerMoore = new TBoyerMoore(pchPattern, fCaseSensitive);
		if (!m_psBoyerMoore)
			throw(new TException(IDS_ERR_BM_ALLOC, kError));
	}

	m_iPatternLen = lstrlen(pchPattern);
}

// -------------------------------------------------------------------------
BOOL TSearch::Search(LPCTSTR pchText, int &iResultLen, DWORD &iPos,
					 DWORD lTextSize /* = -1 */)
{
	ASSERT (m_psRxSearch || m_psBoyerMoore);

	// are there times when 'm_psBoyerMoore' is NULL?    (11-20-96 amc)
	if (NULL == m_psRxSearch && NULL == m_psBoyerMoore)
		throw(new TException(IDS_ERR_SEARCH_COMPILE, kError));

	if (m_psRxSearch)
	{
		LPCTSTR pchResult = m_psRxSearch -> Search(pchText, &iResultLen);
		if (!pchResult)
			return FALSE;
		iPos = (DWORD) (unsigned long)
			((unsigned long) pchResult - (unsigned long) pchText);
		return TRUE;
	}

	if (lTextSize == -1)
		lTextSize = strlen(pchText);
	int iLocalPos;
	BOOL fResult = m_psBoyerMoore->Search(pchText, lTextSize, iLocalPos);

	iPos = (DWORD) iLocalPos;
	// original length of pattern
	iResultLen = m_iPatternLen;

	return fResult;
}

// -------------------------------------------------------------------------
BOOL TSearch::HasPattern ()
{
	if (m_psRxSearch)
		return m_psRxSearch->HasPattern ();

	if (m_psBoyerMoore)
		return m_psBoyerMoore->HasPattern ();

	return FALSE;
}
