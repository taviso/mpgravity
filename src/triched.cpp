/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: triched.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.7  2009/01/12 10:04:47  richard_wood
/*  Fixed another bug with XFace scrolling - mose wheel scrolling when focus was
/*  not in article window caused XFace tearing.
/*
/*  Revision 1.6  2009/01/11 22:28:40  richard_wood
/*  Fixed the XFace corruption on scroll bug for keyboard Down, Up, Page Down, Page Up, mouse wheel scroll, space bar scroll.
/*
/*  Revision 1.5  2009/01/11 02:01:34  richard_wood
/*  Fixed bug with XFaces tearing when the article window is scrolled.
/*
/*  Revision 1.4  2008/09/19 14:52:17  richard_wood
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

// triched.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "richedit.h"
#include "triched.h"
#include "custmsg.h"
#include <initguid.h>
#include "tglobopt.h"
#include "urlsppt.h"
#include <commdlg.h>
#include "artview.h"
#include "turldde.h"
#include "rgswit.h"
#include "autodrw.h"
#include "tnews3md.h"
#include "genutil.h"

// warning about CException has no copy-ctor
//#pragma warning( disable : 4671 4673 )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define WM_XFACEON WM_USER+0x1234

static UINT WM_FINDREPLACE = ::RegisterWindowMessage (FINDMSGSTRING);

/////////////////////////////////////////////////////////////////////////////
// TRichEd

TRichEd::TRichEd()
{
	m_popUpMenu.LoadMenu (IDR_ARTPOP_MENU);
	m_pFindDialog = 0;
	m_hArrowCursor = LoadCursor (AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDC_MYHAND));
	m_bTurnXFaceOn = false;
}

TRichEd::~TRichEd()
{
}

/////////////////////////////////////////////////////////////////////////////
void TRichEd::PostSubclass ()
{
	DWORD dwEvntMask = SendMessage ( EM_GETEVENTMASK );
	dwEvntMask |=  ENM_MOUSEEVENTS | ENM_KEYEVENTS;
	SendMessage ( EM_SETEVENTMASK, 0, dwEvntMask);
}

BEGIN_MESSAGE_MAP(TRichEd, TFormRichEdit)
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_SETCURSOR()
	ON_WM_NCHITTEST()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(EN_MSGFILTER, OnMsgFilterReflect)
	ON_COMMAND(ID_SEARCH_FIND, OnSearchFind)
	ON_MESSAGE(WMU_RTF_YIELD, OnRtfYield)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnSearchFindCB)
	ON_MESSAGE(WM_XFACEON, OnXFaceOn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TRichEd message handlers

void TRichEd::OnRButtonDown(UINT nFlags, CPoint point)
{

	// get the selection... if nothing is selected, let's
	// move the caret to where the user clicked
	DWORD wParam;
	DWORD lParam;
	SendMessage (EM_GETSEL, (WPARAM) &wParam, (LPARAM) &lParam);

	if ((wParam - lParam) == 0)
	{
		LRESULT  pos = SendMessage (EM_CHARFROMPOS, 0, LPARAM(&point));
		SendMessage (EM_SETSEL, WPARAM(LOWORD(pos)), WPARAM(LOWORD(pos)));
	}

	// let default stuff happen
	//   activate the ARTVIEW and let the MDI child know we are the
	//   active CView
	TFormRichEdit::OnRButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////
void TRichEd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (VK_TAB == nChar)
	{
		if (GetStyle() & ES_READONLY)
		{
			// pass this up so we can Tab and Shift-Tab around
			GetParent()->PostMessage (WMU_CHILD_TAB, GetKeyState(VK_SHIFT) < 0, 0);
			return;
		}
	}

	// Note we don't handle VK_SPACE.  The SpaceBar is mapped to a true
	//  accelerator

	TFormRichEdit::OnChar(nChar, nRepCnt, nFlags);
}

///////////////////////////////////////////////////////////////////////////
// GetWordAtCharPos -- returns the word at a specific character position.
// Returns 0 for success, non-0 for failure
int TRichEd::GetWordAtCharPos (int iCharPos, char *pchLine, int iSize,
							   int &iLeft, int &iLine, BOOL & fAngleBrackets)
{
	ASSERT (IsWindow (m_hWnd));
	ASSERT (iCharPos >= 0);

	fAngleBrackets = FALSE;

	// figure out what line the character position is in
	iLine = SendMessage (EM_LINEFROMCHAR, (WPARAM) iCharPos);

	// figure out which character is the start character for that line
	LRESULT iLinePos = SendMessage (EM_LINEINDEX, (WPARAM) iLine);
	if (iLinePos == -1)
		return 1;

	// get the index of the character within the returned line
	int iIndex = iCharPos - iLinePos;
	if (iIndex < 0) {
		ASSERT(0);
		return 1;
	}

	// if the buffer is too small, just fail
	if (iIndex >= iSize)
		return 1;

	*((WORD *) pchLine) = WORD(iSize - 1);

	// copy the line into a buffer
	LRESULT iLen = SendMessage (EM_GETLINE, (WPARAM) iLine,
		(LPARAM) (LPCSTR) pchLine);
	if ((iLen <= 0) || (iLen >= iSize))
		return 1;

	// invariant: iLen > 0

	// NULL terminate the string, EM_GETLINE doesn't do that for us
	pchLine [iLen] = 0;

	// search for whitespace backward from the current character
	int i = iIndex;
	BOOL bBackedUp = FALSE;
	while (i > 0 && !URL_UNSAFE (pchLine [i])) {
		i--;
		bBackedUp = TRUE;
	}

	// if we've backed up past the start of the word, go forward once space
	if (bBackedUp && URL_UNSAFE (pchLine [i]))
	{
		if (pchLine[i] == '<')
			fAngleBrackets = TRUE;
		i++;
	}

	iLeft = i;

	// forward until whitespace
	while (i < iLen && !URL_UNSAFE (pchLine [i]))
		i++;
	if (fAngleBrackets && URL_UNSAFE(pchLine[i]))
		fAngleBrackets = fAngleBrackets && (pchLine[i] == '>');
	pchLine [i] = 0;
	return 0;
}

///////////////////////////////////////////////////////////////////////////
void TRichEd::GetWordUnderCaret (CString &strWord)
{
	ASSERT (IsWindow (m_hWnd));

	strWord = "";

	// get the character position of the caret
	DWORD dwPos;
	SendMessage (EM_GETSEL, (WPARAM) &dwPos);

	char rchLine [200];
	int iLeft, iLine;
	BOOL  fAngleBrackets;
	if (GetWordAtCharPos (dwPos,
		rchLine,
		sizeof (rchLine),
		iLeft,
		iLine,
		fAngleBrackets))
		return;
	strWord = rchLine + iLeft;
}

///////////////////////////////////////////////////////////////////////////
// GetWordAtPoint -- returns the word at a specific point.  Returns 0
// for success, non-0 for failure
int TRichEd::GetWordAtPoint (CPoint sPoint, char *pchLine, int iSize,
							 int &iLeft, int &iLine, BOOL & fAngleBrackets)
{
	ASSERT (IsWindow (m_hWnd));

	// get the character position within the control
	LRESULT pos = SendMessage (EM_CHARFROMPOS, 0, LPARAM (&sPoint));
	if (pos == -1)
		return 1;
	return GetWordAtCharPos (LOWORD (pos),
		pchLine,
		iSize,
		iLeft,
		iLine,
		fAngleBrackets);
}

///////////////////////////////////////////////////////////////////////////
int TRichEd::ClickHandler (CPoint * pptButtonDown, CPoint * pptButtonUp)
{
	POINTL ptl;
	ptl.x = pptButtonUp->x;
	ptl.y = pptButtonUp->y;

	// compare the upclick and make sure it is near the downclick
	// this desensitizes launching the URL in case folks want to copy the URL
	// via mouse-selection
	int abx = abs(pptButtonDown->x - pptButtonUp->x);
	int aby = abs(pptButtonDown->y - pptButtonUp->y);

	if (abx > 6 || aby > 6)
	{
		TRACE("triched rejected URL ==> abx:%d  aby:%d\n", abx, aby);
		return 0;
	}

	// get the character position within the control
	LRESULT pos = SendMessage (EM_CHARFROMPOS, 0, LPARAM ((POINTL*) &ptl));
	if (pos == -1)
		return 0;

	int line = SendMessage (EM_EXLINEFROMCHAR, 0, pos);

	int len = SendMessage (EM_LINELENGTH, pos, 0);

	// snag the whole line
	LPTSTR pLine = new TCHAR[len + 1 + sizeof(WORD)];

	//auto_ptr<TCHAR> sDeleter(pLine);

	*((WORD*)pLine) = len + 1;
	SendMessage (EM_GETLINE, line, LPARAM(pLine));
	pLine[len] = 0;

	int li = SendMessage (EM_LINEINDEX, WPARAM(line), 0);

	int ich = pos - li;

	// Post a message to ourselves to give us a clean slate
	// in case we have to call 'CDlgQueryMsgID'
	PostMessage (WMU_RTF_YIELD, ich, LPARAM(pLine));
	return 0;
}

afx_msg LRESULT TRichEd::OnRtfYield (WPARAM wParam, LPARAM lParam)
{
	int ich = (int) wParam;
	LPTSTR pLine = (LPTSTR) lParam;
	try
	{
		Process2URL (ich, pLine, m_hWnd);     // see URLSppt.cpp for this
	}
	catch (TUrlException *except)
	{
		except->Display ();
		except->Delete();
	}

	delete [] pLine;
	return 0;
}

///////////////////////////////////////////////////////////////////////////
void TRichEd::OnSearchFind()
{
	// TODO: Add your command handler code here
	if (m_pFindDialog)
	{
		if (IsWindow (m_pFindDialog->m_hWnd))
		{
			m_pFindDialog->SetActiveWindow ();
			if (m_pFindDialog->IsIconic ())
				m_pFindDialog->ShowWindow (SW_RESTORE);
		}
		return;
	}

	m_pFindDialog = new CFindReplaceDialog;

	FINDREPLACE & sFind = m_pFindDialog->m_fr;

	sFind.lStructSize = sizeof (sFind);
	sFind.hwndOwner = m_hWnd;
	sFind.lCustData = 0;
	m_searchPos = 0;
	m_selected.cpMin = -1;
	m_selected.cpMax = 0;

	if (0 == m_pFindDialog->Create (TRUE, "", "", FR_DOWN|FR_HIDEUPDOWN, this))
		throw(new TException (IDS_ERR_CREATE_FIND, kError));
}

// ----------------------------------------------------------------------
//  4-07-98  amc  I changed func to use PostMessage(WM_DESTROY).  Was getting
//    a crash in Debug mode.
LRESULT TRichEd::OnSearchFindCB(WPARAM wParam, LPARAM lParam)
{
	CHARRANGE   select;

	if (m_pFindDialog->IsTerminating())
	{
		CString  searchString = m_pFindDialog->GetFindString();
		CWnd * pWndDead = m_pFindDialog;
		m_pFindDialog = 0;
		pWndDead -> PostMessage (WM_DESTROY);

		Invalidate();
		UpdateWindow();
		select.cpMin = m_searchPos - searchString.GetLength();
		select.cpMax = m_searchPos;
		SendMessage (EM_EXSETSEL,
			(WPARAM) (INT) 0,
			(LPARAM) (INT) &select);
		return 0;
	}

	if (m_pFindDialog->FindNext ())
	{
		LRESULT  result;
		CString  findString = m_pFindDialog->GetFindString();
		WPARAM   flags = 0;

		if (m_pFindDialog->MatchWholeWord())
			flags |= FR_WHOLEWORD;
		if (m_pFindDialog->MatchCase ())
			flags |= FR_MATCHCASE;

		FINDTEXT findText;
		findText.chrg.cpMin = m_searchPos;
		findText.chrg.cpMax = SendMessage (WM_GETTEXTLENGTH, 0, 0) - 1;
		findText.lpstrText = (LPTSTR) (LPCTSTR) findString;

		result = SendMessage (EM_FINDTEXT,
			(WPARAM) (UINT) flags,
			(LPARAM) (FINDTEXT FAR *) &findText);

		if (-1 == result)
		{
			AfxMessageBox (IDS_ERR_TEXT_NOT_FOUND); // resource file
			// reset the search to the top
			m_searchPos = 0;
		}
		else
		{
			// select the text

			select.cpMin = result;
			select.cpMax = result + findString.GetLength ();
			SendMessage (EM_EXSETSEL,
				(WPARAM) (INT) 0,
				(LPARAM) (INT) &select);
			SendMessage (EM_HIDESELECTION, (WPARAM) FALSE, (LPARAM) 0);
			SendMessage (EM_SCROLLCARET, (WPARAM) 0, (LPARAM) 0);
			m_searchPos = result + findString.GetLength();
		}

	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Part of UpdateCommandUI
//
//
BOOL TRichEd::CanRequestPageDown()
{
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	// don't know why this fails sometimes.  amc 10-1-96
	if (FALSE == ::GetScrollInfo(m_hWnd, SB_VERT, &si) ||
		0 == si.nPage)
		return FALSE;

	int logMax = si.nMax - si.nPage;

	if (si.nPos >= logMax - 5) // we already see end of document
		return FALSE;

	return TRUE;  // true means there is more in this article to see
}

///////////////////////////////////////////////////////////////////////////
void TRichEd::DoPageDown ()
{
	DWORD dwRet;
	WORD  wfSuccess, wLinesMoved;

	SetRedraw (FALSE);
	dwRet = SendMessage(EM_SCROLL, SB_PAGEDOWN);
	wfSuccess = HIWORD(dwRet);
	wLinesMoved = LOWORD(dwRet);
	ASSERT(wfSuccess);

	if (wLinesMoved > 3)
	{
		// move backwards a bit to give reader some visual overlapp
//		SendMessage(EM_SCROLL, SB_LINEUP);
		SendMessage(EM_SCROLL, SB_LINEUP);
	}
	SetRedraw ();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////
// a request to page down.  return 0 if successful.  return -1 if at EOF
//
int TRichEd::RequestPageDown(void)
{
	LPARAM lFirstVisibleLine1;
	LPARAM lFirstVisibleLine0 = SendMessage (EM_GETFIRSTVISIBLELINE);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	::GetScrollInfo(m_hWnd, SB_VERT, &si);
	int logMax = si.nMax - si.nPage;

	if (0 == si.nPage)  // i've seen this odd value
	{
		// we already see end of document
		return -1;
	}
	else if (si.nPos + si.nPage < UINT(logMax))
	{
		// full page down
		DoPageDown ();
	}
	else if (si.nPos >= logMax - 5)
	{
		// we already see end of document
		return -1;
	}
	else
	{
		BOOL fFull = gpGlobalOptions->GetRegSwitch()->GetSpaceBarFullPgDn();

		if (fFull)
		{
			DoPageDown ();
		}
		else
		{
			TAutoDraw autoDraw(this, TRUE);
			int n = 0;                 // make sure we don't loop forever
			do
			{
				SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN,0));
				::GetScrollInfo(m_hWnd, SB_VERT, &si);
			} while ((++n < 500) && (si.nPos < logMax));
		}
	}

	lFirstVisibleLine1 = SendMessage(EM_GETFIRSTVISIBLELINE);
	if (lFirstVisibleLine1 == lFirstVisibleLine0)
	{
		// hey we didn't move at all! we must be at the end of the article
		return -1;
	}

	// get character index of Start of that line
	LPARAM lCharIdx = SendMessage(EM_LINEINDEX, lFirstVisibleLine1);

	CHARRANGE cr;
	cr.cpMin = cr.cpMax = lCharIdx;
	SendMessage(EM_EXSETSEL, 0, LPARAM(&cr));
	return 0;
}

BOOL TRichEd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Try the parent (ArticleFormView) as last resort
	if (GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// we get last crack
	return TFormRichEdit::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

static CPoint gsPoint;
LRESULT TRichEd::OnNcHitTest(CPoint point)
{
	gsPoint = point;
	return TFormRichEdit::OnNcHitTest(point);
}

BOOL TRichEd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	static int iCachedLine;       // last tested line
	static int iCachedLeft;       // position in rchCachedURL
	static int iCachedResult;     // iURL for rchCachedURL
	static int iCacheValid;       // has cache been written to?
	int iURL = FALSE;             // is mouse over a URL?

	TCHAR rchLine [100];
	int   iLeft;
	int   iLine;
	BOOL  fAngleBrackets;
	ScreenToClient (&gsPoint);
	if (!GetWordAtPoint (gsPoint,
		rchLine,
		sizeof (rchLine),
		iLeft,
		iLine,
		fAngleBrackets)) {

			if (iCacheValid && iCachedLine == iLine && iCachedLeft == iLeft)
				iURL = iCachedResult;
			else {
				iURL = IsURL (rchLine + iLeft);
				iCachedResult = iURL;
				iCachedLeft = iLeft;
				iCachedLine = iLine;
				iCacheValid = TRUE;
			}

			if (iURL) {
				ASSERT (m_hArrowCursor);
				::SetCursor (m_hArrowCursor);
				// return nonzero to halt further processing
				return 1;
			}
	}

	return TFormRichEdit::OnSetCursor(pWnd, nHitTest, message);
}

/////////////////////////////////////////////////////////////////////////
// The standard accelerator (Shift-F10) may be interpreted
//  by the Rich Edit control.  This is here for good form.
// function is called directly by the RButtonUp handler
//
// Note: VC4.0 handles Shift-F10 correctly. How??
void TRichEd::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	if (-1 == ptScreen.x && -1 == ptScreen.y)
	{
		// summoned with Shift-F10 or the Microsoft Keyboard
		ptScreen.x = ptScreen.y = 0;
		ClientToScreen ( &ptScreen );
	}

	CMenu * pTrackMenu = m_popUpMenu.GetSubMenu ( 0 );
	CWnd* pParent = GetParent();

	// show popup menu only if parent responds to this message
	//   and enables/disables stuff appriopriately
	if (pParent->SendMessage(WMU_ARTVIEW_UPDATEPOPMENU, 0, (LPARAM) pTrackMenu))
		pTrackMenu -> TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		ptScreen.x, ptScreen.y, this);
}

void TRichEd::OnRButtonUp(UINT nFlags, CPoint point)
{
	TFormRichEdit::OnRButtonUp(nFlags, point);

	// evil genutil function
	if (RUNNING_ON_WIN95 == GetOS())
	{
		// for Win95  & Win98 we have to call OnContextMenu manually.
		// for the NT codebase, it happens automatically
		ClientToScreen ( &point );
		OnContextMenu(this, point ); // pass in screen coords
	}
}

// -------------------------------------------------------------------------
// returns a copy of selected text
void TRichEd::SelectedText (CString &str)
{
	str = "";

	DWORD dwStart, dwEnd;
	SendMessage (EM_GETSEL, (WPARAM) (LPDWORD) &dwStart,
		(LPARAM) (LPDWORD) &dwEnd);

	ASSERT (dwEnd >= dwStart);
	if (dwEnd == dwStart)
		return;

	DWORD len = dwEnd - dwStart;

	char *pch = new char [len  + 1];
	if (!pch)
		return;

	SendMessage (EM_GETSELTEXT, 0, (LPARAM) (LPSTR) pch);

	str = pch;
	delete pch;
}

void TRichEd::OnMButtonDown(UINT nFlags, CPoint point)
{
	// if we are contained in the ArtView, he passes it to tnews3md.cpp
	//  if we are in the outbox dlg, do nothing

	GetParent()->SendMessage ( WMU_MBUTTON_DOWN );
}

// ------------------------------------------------------------------------
BOOL TRichEd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// if we are search window, etc
	if (GetParent()->IsKindOf(RUNTIME_CLASS(TArticleFormView)))
	{
		// this stuff applies only to main Article pane

		// let the mdi-child decide which pane will handle this
		CWnd* pMdiChild = ((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame();
		if (!pMdiChild)
			return FALSE;

		if ( ((TNews3MDIChildWnd*) pMdiChild)->handleMouseWheel(nFlags, zDelta, pt))
			return TRUE;
	}
	return /*TFormRichEdit::OnMouseWheel*/handleMouseWheel(nFlags, zDelta, pt);
}

// ------------------------------------------------------------------------
BOOL TRichEd::handleMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	if (XFaceShowing())
	{
		m_wndXFace.ShowWindow(SW_HIDE);
		m_bTurnXFaceOn = true;
	}

	if (zDelta > 0)
	{
		SendMessage(EM_SCROLL, SB_LINEUP);
		SendMessage(EM_SCROLL, SB_LINEUP);
	}
	else
	{
		SendMessage(EM_SCROLL, SB_LINEDOWN);
		SendMessage(EM_SCROLL, SB_LINEDOWN);
	}

	if (m_bTurnXFaceOn)
		PostMessage(WM_XFACEON,0,0); // Fire off a message that turns XFace back on

	return TRUE;

	// RLW - I've changed the mouse wheel handling so it just scrolls up/down
	// rather than use the mouse smooth scrolling.
	//
	// I cannot figure out how to tie into the smooth scrolling system.
	// If I could I could trigger a refresh of the XFace and message window
	// for each pixel scroll.

	//BOOL b = TFormRichEdit::OnMouseWheel(nFlags, zDelta, pt);
	//return b;
}

void TRichEd::SetupXFaceWindow()
{   	
	RECT rctClient;
	GetClientRect (&rctClient);

	CRect rct;
	rct.top =  0;
	rct.bottom = 50;
	rct.left = rctClient.right - 51;
	rct.right = rctClient.right - 1;

	BOOL fRet =
		m_wndXFace.Create (NULL,
		"",
		WS_BORDER | WS_CHILD,
		rct, this, 1);
	return ;
}

void TRichEd::OnSize(UINT nType, int cx, int cy)
{
	TFormRichEdit::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (::IsWindow(m_wndXFace.m_hWnd))
		m_wndXFace.SetWindowPos (NULL, cx-50, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void TRichEd::SetXFace (LPCTSTR line)
{
	if (::IsWindow(m_wndXFace.m_hWnd))
	{
		if (NULL != line)
		{
			m_wndXFace.SetXFace (line);
			m_wndXFace.ShowWindow (SW_SHOW);
		}
	}
}

void TRichEd::HideXFace ()
{
	if ( XFaceShowing () )
	{
		m_wndXFace.ShowWindow (SW_HIDE);
	}
	// RLW - If the XFace is hidden (mid scroll or something)
	// we do not want to show it again, so reset this flag
	m_bTurnXFaceOn = false;
}

bool TRichEd::XFaceShowing ()
{
	return (::IsWindow(m_wndXFace.m_hWnd)  && m_wndXFace.IsWindowVisible());
}

void TRichEd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// RLW - fix XFace scrolling bug
	if (XFaceShowing())
	{
		// If the XFace is being shown turn the xface off
		m_wndXFace.ShowWindow(SW_HIDE);
		// Perform the scroll
		TFormRichEdit::OnVScroll(nSBCode, nPos, pScrollBar);
		// Fire off a message that turns XFace back on
		PostMessage(WM_XFACEON,0,0);
	}
	else
		TFormRichEdit::OnVScroll(nSBCode, nPos, pScrollBar);
}

// user triggered function to clean up multi-line URLs
int TRichEd::ArticleRepairURL()
{
	CString text; SelectedText(text);

	if (text.IsEmpty())
		return 0;

	// Enhance the repair URL so it can cope with a valid URL that
	// is wrapped AND quoted.
	//
	// So it now consists of:-
	// optional bits are in [ ]
	//
	// optional start newlines
	// optional quoting chars
	// part of URL
	// quoting chars
	// part of URL
	// optional ending newlines
	// 
	// Quoting chars, the actual char is restricted to '>'
	// Multiple quoting can stack these up, with optional
	// spaces between them.

	// So:-
	//
	// Trim starting whitespace
	// Trim ending whitespace
	// Delete all space, tab and newline chars
	// Delete all '>' chars - we can do this as these are illegal in URLs

	// This lot assumes of course the user has selected the URL correctly.

	text.TrimLeft();
	text.TrimRight();
	text.Replace(" ", "");
	text.Replace("\t", "");
	text.Replace("\r", "");
	text.Replace("\n", "");
	text.Replace(">", "");
	text.Replace("<", "");

	CString final;
	TCHAR c;
	for (int i = 0; i < text.GetLength(); i++)
	{
		c = text[i];
		if (!_istspace(c))
			final += c;
	}
	if (final.IsEmpty())
		return 0;

	// If the URL actually starts with "URL:", strip it off
	if (final.Left(4).CompareNoCase("URL:")==0)
		final = final.Mid(4);

	try
	{
		Process2URL (0, final, m_hWnd);     // see URLSppt.cpp for this
	}
	catch (TUrlException *except)
	{
		except->Display ();
		except->Delete();
	}
	return 0;
}

afx_msg void TRichEd::OnMsgFilterReflect (NMHDR* lpNMHDR, LRESULT* plResult)
{
	try
	{
		MSGFILTER *  pMF = (MSGFILTER*) lpNMHDR;

		switch (pMF -> msg)
		{
		case WM_LBUTTONDOWN:
			{
				m_ptLastLeftClick.x = LOWORD(pMF->lParam);
				m_ptLastLeftClick.y = HIWORD(pMF->lParam);
			}
			break;
		case WM_LBUTTONUP:
			{
				CPoint pt(LOWORD(pMF->lParam), HIWORD(pMF->lParam));
				ClickHandler (&m_ptLastLeftClick, &pt);
			}
			break;
		case WM_CHAR:
			{
				if ((VK_RETURN == pMF->wParam) && m_pParentFormView)
				{
					m_pParentFormView->RichEditEnterKey();
				}
			}
			break;
		case WM_KEYDOWN:
			// RLW 11/1/09
			// Fixed XFace corruption bug when scrolling via
			// cursor up, down, page up and down.
			{
				if ((pMF->wParam == VK_UP) ||
					(pMF->wParam == VK_DOWN) ||
					(pMF->wParam == VK_LEFT) ||
					(pMF->wParam == VK_RIGHT) ||
					(pMF->wParam == VK_UP) ||
					(pMF->wParam == VK_END) ||
					(pMF->wParam == VK_HOME) ||
					(pMF->wParam == VK_PRIOR) ||
					(pMF->wParam == VK_NEXT))
				{
					// Turn off the XFace (if on) and then send outselves a message to turn it on
					// after the scrolling has finished
					if (XFaceShowing())
					{
						m_wndXFace.ShowWindow(SW_HIDE);
						PostMessage(WM_XFACEON,0,0);
					}
				}
			}
			break;
		case WM_VSCROLL:
			// RLW - If the m_bTurnXFaceOn flag is set, fire off a message
			// that will turn back on the XFace.
			{
				if (m_bTurnXFaceOn)
				{
					PostMessage(WM_XFACEON,0,0);
					m_bTurnXFaceOn = false;
				}
				// IF the user is trying to mouse scroll lock + scroll
				// I don't know how to latch into the smooth scrolling
				// mechanism so what I'll do it just turn the XFace off
				// for the duration of this post.
				else if (XFaceShowing())
					m_wndXFace.ShowWindow(SW_HIDE);
			}
			break;
		}
	}
	catch(...)
	{
	}
	*plResult = 0;
}

// RLW - Message handler that turns the XFace back on
// and invalidates & refreshes the XFace and message window.
afx_msg LRESULT TRichEd::OnXFaceOn(WPARAM wParam, LPARAM lParam)
{
	m_wndXFace.ShowWindow(SW_SHOW);
	m_wndXFace.Invalidate();
	Invalidate();
	return 0;
}

// RLW - If any sort of v scrolling, keyboard or moose,
// we hide the XFace to prevent the display corruption.
BOOL TRichEd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg)
	{
		switch (pMsg->message)
		{
		case WM_MOUSEWHEEL:
		case WM_VSCROLL:
			{
				if (XFaceShowing())
				{
					m_wndXFace.ShowWindow(SW_HIDE);
					m_bTurnXFaceOn = true;
				}
			}
			break;
		}
	}

	return TFormRichEdit::PreTranslateMessage(pMsg);
}
