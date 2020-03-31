/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: statbar.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:54  richard_wood
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

// statbar.cpp : implementation file
//
// 2-16-96  Added Mutex to protect everything.  Makes sense

#include "stdafx.h"
#include "news.h"
#include "statbar.h"
#include "tasker.h"
#include "globals.h"
#include "custmsg.h"
#include "tmutex.h"
#include "usrdisp.h"             // TUserDisplay
#include "sysclr.h"              // TSystem
#include "gallery.h"             // GetGalleryWindow()
#include "ipcgal.h"

extern TNewsTasker* gpTasker;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int giUIPane = 0;
static const int iJobsPane   = 1;
static const int iStatusPane = 2;        /* has thermometer and status text */
static const int iTimerNum = 666;
static const int iTimerMsec = 300;

/////////////////////////////////////////////////////////////////////////////
// TStatusBar

TStatusBar::TStatusBar()
{
	m_fSimple = FALSE;
	m_hTimerHandle = NULL;
	m_fShowEmpty = TRUE;
	m_fPriority = FALSE;
	m_hMutexStatus = CreateMutex (NULL, // no security
		NULL, // not owned
		NULL); // no name
}

TStatusBar::~TStatusBar()
{
	CloseHandle ( m_hMutexStatus );
}

void TStatusBar::InitializePanes()
{
	m_hTimerHandle = SetTimer(iTimerNum,   // event
		iTimerMsec, // elapse
		NULL);
}

BOOL TStatusBar::SetSimple(BOOL fSimple)
{
	TMutex mtx_mgr(m_hMutexStatus);

	m_fSimple = (BYTE)fSimple;
	return GetStatusBarCtrl().SetSimple(fSimple);
}

LRESULT TStatusBar::OnSetText(WPARAM wParam, LPARAM lParam)
{
	TMutex mtx_mgr(m_hMutexStatus);

	int iPaneNum = 0;
	if (m_fSimple) iPaneNum = 255;
	return !GetStatusBarCtrl().SetText((LPCTSTR)lParam, iPaneNum, 0);
}

BEGIN_MESSAGE_MAP(TStatusBar, CStatusBar)
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_WM_TIMER()
	ON_MESSAGE(WMU_PROGRESS_STEP,     OnStep)
	ON_MESSAGE(WMU_PROGRESS_POS,      OnSetPos)
	ON_MESSAGE(WMU_PROGRESS_END,      OnDone)
	ON_WM_DESTROY()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TStatusBar message handlers

void TStatusBar::OnSize(UINT nType, int cx, int cy)
{
	TMutex mtx_mgr(m_hMutexStatus);

	CStatusBar::OnSize(nType, cx, cy);

	RECT rct;
	GetClientRect ( &rct );
	int width = rct.right;

	CStatusBarCtrl& ctrl = GetStatusBarCtrl();

	// 4-12-96 change to 2 panes [Jobs: 5] [statustext && thermometer]

	// 2-26-00 change to 3 panes [UI ] [Jobs : 5] [[statustext && thermometer]

	int segment = width / 10;
	int iPanesPositions[3];
	iPanesPositions[0] =  segment;
	iPanesPositions[1] =  2 * segment + 10;
	iPanesPositions[2] =  width - 18 ;

	if (iPanesPositions[iStatusPane] < 0)
	{
		iPanesPositions[0] = 0 ;
		iPanesPositions[1] = 0 ;
		iPanesPositions[iStatusPane] = 0 ;
	}
	ctrl.SetParts ( 3, iPanesPositions );
}

#if defined(NOT_USED_NOW)

CWnd* TStatusBar::GetTextPane()
{
	return &GetStatusBarCtrl();
}

#endif

void TStatusBar::OnTimer(UINT nIDEvent)
{
	TMutex mtx_mgr(m_hMutexStatus);

	// TODO: Add your message handler code here and/or call default
	timerAction();
	refresh_jobs_pane();
	CStatusBar::OnTimer(nIDEvent);
}

///////////////////////////////////////////////////////////////////////////
// 4/3/98 -- turn off all optimizations... there seems to be a code generation
// bug causing a status bar refresh problem on win98, on the laptop, in
// release mode, while Gravity is maximized
#pragma optimize( "", off )

///////////////////////////////////////////////////////////////////////////
// Filters out unnecessary redraws (if position or string didn't change)
//
void TStatusBar::timerAction()
{
	TMutex mtx_mgr(m_hMutexStatus);

	static CRect rctOld;
	CRect        rctClient;

	USRDISP_INFO sDI;

	// get data in one fell swoopt
	gpUserDisplay->GetAllCurrentInfo ( &sDI );

	updateUIPane (sDI.m_uiStatus,  sDI.m_ptFilter);

	if (!sDI.m_fPrio)
		notifyGallery (sDI.m_iCurrent, sDI.m_iHigh);

	GetClientRect (&rctClient);

	BOOL& fPrio = sDI.m_fPrio;
	CString& realStatus = sDI.m_text;
	int usrPos = sDI.m_iCurrent;

	BOOL fDraw = FALSE;
	if ( (m_fPriority != fPrio) ||
		(curDisplayString != realStatus) ||
		(curPos != usrPos) ||
		!rctClient.EqualRect (&rctOld) )
	{
		fDraw = TRUE;
		rctOld = rctClient;
	}

	// Values are updated at end of DrawItem()
	//m_fPriority = fPrio;
	//curDisplayString = realStatus;
	//curPos = usrPos;

	if (fDraw)
	{
		InvalidateProgressPane();        // thermometer & text rectangle
	}
}

// Turn optimizations back on
#pragma optimize( "", on )

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void TStatusBar::refresh_jobs_pane()
{
	TMutex mtx_mgr(m_hMutexStatus);

	CStatusBarCtrl& ctrl = GetStatusBarCtrl();

	extern UINT gintBytesPerSecond;

	if (0 == gpTasker)
	{
		ctrl.SetText(_T(""), iJobsPane, 0);
	}
	else
	{
		int nFetch = 0;
		gpTasker->GetStatusData ( nFetch );

		UINT KBperSec = gintBytesPerSecond / 1024;

		if ((0 == nFetch) && (0 == KBperSec) )
		{
			if (!m_fShowEmpty)
			{
				// don't keep resetting to ""
				ctrl.SetText(_T(""), iJobsPane, 0);
				m_fShowEmpty = TRUE;
			}
		}
		else
		{
			CString msg;

			if (KBperSec >= 1024)
				msg.Format (IDS_STATBAR_JOBSMB,
				nFetch,
				KBperSec / 1024,
				KBperSec / 102 % 10);
			else
				msg.Format (IDS_STATBAR_JOBSKB, nFetch, KBperSec);

			ctrl.SetText(msg, iJobsPane, 0);

			// don't keep resetting to ""
			m_fShowEmpty = FALSE;
		}
	}
}

void TStatusBar::InvalidateProgressPane(void)
{
	BOOL fRet;
	TMutex mtx_mgr(m_hMutexStatus);

	CStatusBarCtrl& sbc = GetStatusBarCtrl();

	fRet = sbc.SetText(LPCTSTR(0x123),  // junk LPCTSTR
		iStatusPane,     // pane number
		SBT_OWNERDRAW);

	// 4-17-97 added call to updatewindow; the new comctl32.dll is weird
	if (fRet)
		sbc.UpdateWindow ();
}

/////////////////////////////////////////////////////////////////////////////
// This is a self-draw item - this is a virtual function
void TStatusBar::DrawItem(LPDRAWITEMSTRUCT lpDraw)
{
	TMutex mtx_mgr(m_hMutexStatus);

	CDC theDC;
	theDC.Attach (lpDraw->hDC);

	USRDISP_INFO sDI;

	gpUserDisplay -> GetAllCurrentInfo ( &sDI );

	RECT cliprct, orgRct;
	CStatusBarCtrl& ctrl = GetStatusBarCtrl();
	ctrl.GetRect ( iStatusPane, &orgRct );

	cliprct = orgRct;

	if (sDI.m_iCurrent > sDI.m_iHigh)
		sDI.m_iCurrent = sDI.m_iHigh;

	if (sDI.m_iHigh - sDI.m_iLow > 0)
	{
		// setup clip rect
		cliprct.right = ( (sDI.m_iCurrent) * (cliprct.right - cliprct.left) )
			/ (sDI.m_iHigh - sDI.m_iLow)  ;
		cliprct.right += cliprct.left;
	}
	else
		cliprct.right = 0;

	// Draw first half. use red if this is priority
	DWORD dwTextColor = gSystem.HighlightText ();
	DWORD dwBackColor = sDI.m_fPrio ? RGB(255,0,0) : gSystem.Highlight ();

	// Set the textcolor & background mode for Highlighted Half
	COLORREF oldTextColor = theDC.SetTextColor ( dwTextColor );
	COLORREF oldBackColor = theDC.SetBkColor ( dwBackColor );

	if (cliprct.right > 0)
		theDC.ExtTextOut (lpDraw->rcItem.left + 2,
		lpDraw->rcItem.top,
		ETO_CLIPPED | ETO_OPAQUE,
		&cliprct,
		sDI.m_text,
		sDI.m_text.GetLength(),
		NULL);

	theDC.SetBkColor (oldBackColor);
	theDC.SetTextColor ( oldTextColor );

	// Draw normal half
	int oldMode = theDC.SetBkMode (TRANSPARENT);
	cliprct.left = cliprct.right;
	cliprct.right = orgRct.right;
	theDC.ExtTextOut (lpDraw->rcItem.left + 2,
		lpDraw->rcItem.top,
		ETO_CLIPPED,
		&cliprct,
		sDI.m_text,
		sDI.m_text.GetLength(),
		NULL);
	theDC.SetBkMode (oldMode);
	theDC.Detach ();

	// update our state to match reality
	m_fPriority = sDI.m_fPrio;
	curDisplayString = sDI.m_text;
	curPos = sDI.m_iCurrent;
}

afx_msg LRESULT TStatusBar::OnStep(WPARAM wP, LPARAM lParam)
{
	TMutex mtx_mgr(m_hMutexStatus);

	if (wP)
		InvalidateProgressPane();
	return 0;
}

// if WPARAM then invalidate, else depend on AutoRefresh Timer
afx_msg LRESULT TStatusBar::OnSetPos(WPARAM wP, LPARAM lParam)
{
	TMutex mtx_mgr(m_hMutexStatus);

	if (wP)
		InvalidateProgressPane();
	return 0;
}

void TStatusBar::OnDestroy()
{
	TMutex mtx_mgr(m_hMutexStatus);

	CStatusBar::OnDestroy();

	if (m_hTimerHandle)
		KillTimer (iTimerNum);
}

///////////////////////////////////////////////////////////////////////////
// Reset text and zero out
//
LRESULT TStatusBar::OnDone(WPARAM wP, LPARAM lParam)
{
	// note:
	//  normal job (long)
	//                      priority jobs starts
	//                      priority job  ends
	//  redraw status of normal job
	//

	/* due to cable modems being really fast
	* let us Try letting the timer handle this
	*  amc  6/15/98
	*/
	////timerAction();

	return 0;
}

///////////////////////////////////////////////////////////////////////////
void  TStatusBar::updateUIPane (const CString & strUIStatus,
								const CPoint & ptFilter)
{

	// see if Pane0 has changed
	CStatusBarCtrl& ctrl = GetStatusBarCtrl();

	int iType    = 0;
	int iStrLen  = ctrl.GetTextLength (giUIPane, &iType);

	if ( strUIStatus.IsEmpty() )
	{
		// we would be showing nothing here,  so show the  XXX / YYY Filter #'s
		CString frac;  frac.Format (_T("%d/%d"), ptFilter.x, ptFilter.y);

		ctrl.SetText ( frac, giUIPane, 0 );

		return;
	}

	LPTSTR pText = new TCHAR[iStrLen + 1];

	if (pText)
	{
		ctrl.GetText ( pText, giUIPane, &iType );

		if (strUIStatus.Compare (pText) != 0)
		{
			// install new string
			ctrl.SetText ( strUIStatus, giUIPane, 0 );
		}
		else
		{
		}
		delete [] pText;
	}
}

///////////////////////////////////////////////////////////////////////////
void  TStatusBar::notifyGallery (int iCurrent, int iHigh)
{
	// notify gallery
	HWND hWnd = GetGalleryWindow ();

	if (hWnd)
	{
		if (!iHigh)
			iHigh = 1;

		int iPercent = min (iCurrent * 100 / iHigh ,  100);

		IPC_Gallery_Progress (hWnd, iPercent);
	}
}

