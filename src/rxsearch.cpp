/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rxsearch.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:49  richard_wood
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
#include "pcre\pcre.h"
#include "regex.h"
#include "rxsearch.h"
#include "mplib.h"
#include "tmutex.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Regular Expression Searching
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// RxSearch::RxSearch - Constructor for regular expression searching.
/////////////////////////////////////////////////////////////////////////////
RxSearch::RxSearch()
{
	m_pRegEx      = NULL;
	m_mutex       = CreateMutex (NULL, FALSE, NULL);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
RxSearch::~RxSearch()
{
	delete m_pRegEx;

	CloseHandle (m_mutex);
}

/////////////////////////////////////////////////////////////////////////////
// The assumption is that the result you return
//   is buffer + some offset
/////////////////////////////////////////////////////////////////////////////
LPCTSTR RxSearch::Search(LPCTSTR  buffer, int *resultLen)
{
	try
	{
		TMutex   mutex(m_mutex);
		LPCTSTR  result;
		if (NULL == m_pRegEx)
			throw(new TException(IDS_ERR_NO_EXPRESSION, kError));

		bool found = m_pRegEx->Search(buffer);
		if (found)
		{
			LPCTSTR match = m_pRegEx->Match(0);
			result = _tcsstr(buffer, match);
			*resultLen = lstrlen(match);
			return result;
		}

		return NULL;
	}
	catch (TException *except)
	{
		except->Display();
		except->Delete();
		return NULL;
	}
	catch(...)
	{
		ASSERT(0);
		TRACE0("RxSearch::Search encountered an exception\n");
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Compile -- returns 0 for success, non-0 for failure
int RxSearch::Compile(LPCTSTR pattern, BOOL fIgnoreCase)
{
	try
	{
		TMutex   mutex(m_mutex);
		if (m_pRegEx)
		{
			delete m_pRegEx;
			m_pRegEx = NULL;
		}
		try
		{
			// RLW : Added PCRE_MULTILINE so we can do a proper regexp search in the header/body text.
			RegEx* pTemp = new RegEx(pattern, PCRE_MULTILINE | (fIgnoreCase ? PCRE_CASELESS : 0));
			m_pRegEx = pTemp;
		}
		catch(...)
		{
			CString errMsg;
			errMsg.Format(IDS_ERR_INVALID_RE, pattern);

			throw(new TException(errMsg, kError));
		}
	}
	catch (TException *except)
	{
		except->Display();
		except->Delete();
		return 1;
	}
	return 0;
}

LPCTSTR RxSearch::GetAssignment(int n)
{
	return m_pRegEx->Match(n);
}
