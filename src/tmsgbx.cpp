/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tmsgbx.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:10  richard_wood
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
#include "tmsgbx.h"
#include "news.h"
#include "resource.h"         // IDS_APPLICATION_TITLE
#include "autoprio.h"
#include "timemb.h"
#include "critsect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern HWND ghwndMainFrame;
// --------------------------------------------------------------------------
int NewsMessageBox (CWnd* pWnd, int stringID, UINT nType,
					LPCTSTR caption /* = NULL */)
{
	CString msg;
	msg.LoadString ( stringID );
	return NewsMessageBox (pWnd, msg, nType, caption );
}

// --------------------------------------------------------------------------
// 1-25-96  Boost priority, in case message box was invoked by a lower
//          priority thread.
int NewsMessageBox (CWnd* pWnd, LPCTSTR msg, UINT nType,
					LPCTSTR caption /* = NULL */)
{
	TRACE0("messagebox zulu\n");
	HANDLE  hThread = GetCurrentThread();
	int     oldPrio = GetThreadPriority ( hThread );

	CString strCaption;
	LPCTSTR szCaption = caption;
	if (NULL == szCaption)
	{
		strCaption.LoadString (IDS_APPLICATION_TITLE);
		szCaption = strCaption;
	}
	if (0 == pWnd)
		pWnd = CWnd::FromHandle (ghwndMainFrame);

	auto_prio sBooster(auto_prio::kAboveNormal);

	return pWnd->MessageBox (msg, szCaption, nType);
}

// --------------------------------------------------------------------------
int NewsMessageBoxTimeout (int     secs,
						   CWnd*   pWnd,
						   int     stringID,
						   UINT    nType /* = MB_OK */,
						   LPCTSTR caption /* = NULL */)
{
	CString msg;
	msg.LoadString ( stringID );

	return NewsMessageBoxTimeout (secs, pWnd, msg, nType, caption);
}

// --------------------------------------------------------------------------
// Shell around the MessageBox that times out.  If MessageBox times out, then
//   return IDTIMEOUT
int NewsMessageBoxTimeout (int secs,
						   CWnd* pWnd,
						   LPCTSTR msg,
						   UINT nType /* = MB_OK */,
						   LPCTSTR caption /* = NULL */)
{
	extern  HWND ghwndMainFrame;
	HWND    hWndAnchor;
	CString strCaption;
	LPCTSTR szCaption = caption;
	if (NULL == szCaption)
	{
		strCaption.LoadString (IDS_APPLICATION_TITLE);
		szCaption = strCaption;
	}

	if (0 == pWnd)
		hWndAnchor = ghwndMainFrame;
	else
		hWndAnchor = pWnd->m_hWnd;

	auto_prio sBooster(auto_prio::kAboveNormal);

	return TimeMsgBox_MessageBox(hWndAnchor, msg, szCaption, nType,
		1000 * secs);
}

// --------------------------------------------------------------------------
int NewsMessageBox (TGlobalDef::EMsgBoxAnchor eAnchor, int iStringID,
					UINT nType /* = MB_OK */, LPCTSTR caption /* = NULL */)
{
	CString strTemp;  strTemp.LoadString (iStringID);

	return NewsMessageBox ( eAnchor, strTemp, nType, caption );
}

// --------------------------------------------------------------------------
int NewsMessageBox (TGlobalDef::EMsgBoxAnchor eAnchor, LPCTSTR msg,
					UINT nType /* = MB_OK */, LPCTSTR caption /* = NULL */)
{
	TRACE0("messagebox lima\n");
	int ret = IDOK;
	ASSERT(eAnchor == TGlobalDef::kMainWnd);

	auto_prio sBooster(auto_prio::kNormal);

	CNewsApp * pNews = (CNewsApp*)AfxGetApp();

	// perform lock.  this guards against shutdown code in MainFrame::OnClose
	HWND  hwndFrame = pNews->AccessSafeFrameHwnd (TRUE);

	// if hwndframe is not valid then we may be shutting down

	ret = NewsMessageBox ((CWnd*) NULL, msg, nType, caption);

	pNews->AccessSafeFrameHwnd (FALSE);

	return ret;
}

// ------------------------------------------------------------------------
int NewsMessageBox (HWND hw, LPCTSTR msg,
					UINT nType /* = MB_OK */,
					LPCTSTR caption /* = NULL */)
{
	return NewsMessageBox (CWnd::FromHandle (hw), msg, nType, caption );
}

