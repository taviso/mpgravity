/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fetchart.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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
#include "fetchart.h"
#include "tasker.h"
#include "utilpump.h"     // pump some useful message

#include "news.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsTasker* gpTasker;

///////////////////////////////////////////////////////////////////////////
// Constructor 1 - wait on event
//
TFetchArticle::TFetchArticle()
: m_GroupID(0), m_artInt(0), m_hdrStatus(0), m_fOK(false)
{
	m_pArtText = 0;
	m_hFetchComplete = CreateEvent (NULL, TRUE, FALSE, NULL);
}

///////////////////////////////////////////////////////////////////////////
// Constructor 2 - asynchronous
//
TFetchArticle::TFetchArticle(LPCTSTR groupName,
							 LONG GroupID, int artInt, WORD hdrStatus)
							 : m_GroupID(GroupID), m_artInt(artInt),
							 m_hdrStatus(hdrStatus), m_groupName(groupName),  m_fOK(false)
{
	m_pArtText = 0;
	m_hFetchComplete = 0;
}

// ------------------------------------------------------------------------
int TFetchArticle::JobSuccess (TArticleText* pArtText)
{
	SetSuccess ( true );

	m_pArtText = pArtText;

	if (m_hFetchComplete)
		SetEvent (m_hFetchComplete);

	else
		((CNewsApp*)AfxGetApp())->ReceiveArticle ( this );

	return 0;
}

// ------------------------------------------------------------------------
int TFetchArticle::JobFailed (  EErrorClass  eClass,
							  DWORD dwError, CString & strError)
{
	SetSuccess (false);
	m_pArtText = 0;

	if (dwError)
	{
		TError  sError2(kError, eClass, dwError);

		m_mplibError = sError2;
	}
	else
	{
		TError sError3 (strError, kError, eClass);

		m_mplibError = sError3;
	}

	if (m_hFetchComplete)
		SetEvent (m_hFetchComplete);

	else
		((CNewsApp*)AfxGetApp())->ReceiveArticle ( this );

	return 0;
}

// ------------------------------------------------------------------------
//  This only access to the destructor.  Thus this object can only be New'ed
void TFetchArticle::DestroySelf ()
{
	delete this;
}

// ------------------------------------------------------------------------
TFetchArticle::~TFetchArticle(void)
{
	// status should already be set by SetStatus()

	if (m_hFetchComplete)
	{
		// we are complete, for good or bad. Release any waiting threads
		SetEvent (m_hFetchComplete);

		CloseHandle (m_hFetchComplete);
	}
}

///////////////////////////////////////////////////////////////////////////
// Return 0 for success, 1 otherwise ( detailed error  in sErrorRet )
//
// rpText - an unallocated ptr.  If success, will point to a ArticleText
//          the caller must free it
//
// hThrdKillRequest - handle to (decode) thread kill event
//
int BlockingFetchBody(
					  TError        & sErrorRet,
					  const CString & groupName,
					  int             groupID,
					  const CString & subject,
					  CPoint &        ptPartID,
					  int            articleNumber,
					  int            iLines,
					  TArticleText*& rpText,
					  BOOL           fPrioPump,
					  HANDLE         hThrdKillRequest,
					  BOOL           bMarkAsRead, /* = TRUE */
					  BOOL           bEngage /* = TRUE */        // Try engaging if not engaged
					  )
{
	CString strInternalError = "Internal error";  

	// we are going to surrender ownership of this ptr. The Pump job deletes it
	TFetchArticle* pObjFetch = new TFetchArticle();

	if (NULL == pObjFetch)
	{
		TError sTemp(strInternalError,  kError,  kClassResource);
		sErrorRet = sTemp;
		return 1;
	}

	HANDLE hFetchComplete = pObjFetch->m_hFetchComplete;

	int ret;

	try
	{
		ret = gpTasker->TaskerFetchBody( groupName, groupID,
			subject, ptPartID,
			articleNumber, iLines,
			pObjFetch, fPrioPump, bMarkAsRead,
			bEngage);
		TRACE1("returned (%d) from TaskerFetchBody\n", ret);
	}
	catch(...)
	{
		pObjFetch->DestroySelf ();
		pObjFetch = 0;

		TRACE("Caught exception in BlockingFetchBody\n");
		ASSERT(0);

		TError sTemp(strInternalError,  kError,  kClassInternal);
		sErrorRet = sTemp;

		return 1;
	}

	if (ret != 0)
	{
		if (pObjFetch)
			pObjFetch->DestroySelf ();

		// most likely PUMP_NOT_CONNECTED

		TError sTemp(IDS_NOT_CONNECTED, kError,  kClassUser);
		sErrorRet = sTemp;

		return 1;
	}

	DWORD   dwRet;
	HANDLE  vHndl[2];
	int     nTotHandles = 1;

	try
	{
		if (hThrdKillRequest)
			nTotHandles = 2;

		vHndl[0] = hFetchComplete;
		vHndl[1] = hThrdKillRequest;

		for (;;)
		{
			// wait for any one to signal
			dwRet = WaitForMultipleObjects ( nTotHandles, vHndl, FALSE, 30 );

			if (WAIT_TIMEOUT == dwRet)
			{
				// allow the decode thread to autosave dec-jobs
				AfxGetThread()->OnIdle (0);

				PumpAUsefulMessage();
			}
			else if (WAIT_OBJECT_0 == dwRet)
				break;
			else if (WAIT_OBJECT_0 + 1 == dwRet)
			{
				// causes a small memory leak.
				// PUMP_SHUTDOWN

				TError sTemp("Shutdown", kError, kClassInternal);
				sErrorRet = sTemp;

				return 1;
			}
			else
			{
				// ASSERT(0);
				break;
			}
		}

		if (pObjFetch->GetArtText ())
		{
			rpText = pObjFetch->GetArtText ();

			pObjFetch->DestroySelf ();
			pObjFetch = 0;

			return 0;   // Total unmitigated success !
		}
		else
		{
			// copy out the error info
			sErrorRet = pObjFetch->m_mplibError;

			pObjFetch->DestroySelf ();
			pObjFetch = 0;

			return 1;
		}
	}
	catch(...)
	{
		TRACE("Caught exception in BlockingFetchBody\n");
		ASSERT(0);
		if (pObjFetch)
		{
			pObjFetch->DestroySelf ();
			pObjFetch = 0;
		}
		TError sTemp(strInternalError,  kError,  kClassInternal);
		sErrorRet = sTemp;

		return 1;
	}
}

BOOL TFetchArticle::GetUIContext (
								  LONG* pGroupID,
								  int* partInt,
								  WORD* phdrStatus)
{
	if (pGroupID)
		*pGroupID   = m_GroupID;
	if (partInt)
		*partInt    = m_artInt;
	if (phdrStatus)
		*phdrStatus = m_hdrStatus;
	return  (m_GroupID) ? TRUE : FALSE;
}

// ------------------------------------------------------------------------
TArticleText* TFetchArticle::GetArtText (void)
{
	return m_pArtText;
}

