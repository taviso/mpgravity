/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: friched.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/11 22:28:40  richard_wood
/*  Fixed the XFace corruption on scroll bug for keyboard Down, Up, Page Down, Page Up, mouse wheel scroll, space bar scroll.
/* */
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood                          */
/*  Build 6 : BETA release                                                   */
/*                                                                           */
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.    */
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text */
/*        and typing at or near top the text would jump around.              */
/*                                                                           */
/*  Revision 1.2  2008/09/19 14:51:25  richard_wood                          */
/*  Updated for VS 2005                                                      */
/*                                                                           */
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

// FRICHED.CPP : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "friched.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TFormRichEdit

TFormRichEdit::TFormRichEdit()
{
	m_lastWidth = 0;
	m_fWordWrap = FALSE;
	m_lastHeight = 0;
}

TFormRichEdit::~TFormRichEdit()
{
}

BEGIN_MESSAGE_MAP(TFormRichEdit, CRichEditCtrl)
	ON_WM_MOUSEACTIVATE()
	ON_WM_GETDLGCODE()
	ON_NOTIFY_REFLECT(EN_REQUESTRESIZE, OnRequestResize)
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_MESSAGE(WM_PASTE, OnPaste)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TFormRichEdit message handlers

///////////////////////////////////////////////////////////////////////////
// Why do I do this?  It wasn't linking to the parent (in my case the
// TArticleFormView) wasn't activated
//
int TFormRichEdit::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	//return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	const MSG* pMsg = GetCurrentMessage();
	return GetParent()->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
}

UINT TFormRichEdit::OnGetDlgCode()
{
	WNDPROC* basicProc = GetSuperWndProcAddr();

	UINT baseFlags = (*basicProc)(m_hWnd, WM_GETDLGCODE, 0, 0);

	// In a dialog box, by default, as you switch to an edit box,
	// the entire text is selected.
	// I want to squish this behavior

	return baseFlags & ~DLGC_HASSETSEL;
}

///////////////////////////////////////////////////////////////////////////
//  We must resend the EM_SETRECT message, since as the richedit fills up
//  the vertical scrollbar may appear and cut down on the width
void TFormRichEdit::OnRequestResize(NMHDR* pNMHDR, LRESULT* pLResult)
{
}

void TFormRichEdit::OnSize(UINT nType, int cx, int cy)
{
	CRichEditCtrl::OnSize(nType, cx, cy);
}

void TFormRichEdit::calculateFormatRect (int cx, int cy, BOOL fSizeMsg)
{
}

///////////////////////////////////////////////////////////////////////////
// If this is one of our compose windows, then when we paste we must
// downshift the RichText to use the current font
//
// created:  2-19-96  amc
afx_msg LRESULT TFormRichEdit::OnPaste(WPARAM wParam, LPARAM lParam)
{
	WNDPROC* basicProc = GetSuperWndProcAddr();
	int i = 0;
	CHARRANGE rangeStart, rangeEnd;
	LRESULT   ret;
	if (m_fWordWrap)
	{
		// from CRichEdit
		HideSelection(TRUE, FALSE);

		CHARFORMAT fmt;
		fmt.cbSize = sizeof(fmt);

		// remember current caret position
		SendMessage( EM_EXGETSEL, 0, LPARAM(&rangeStart) );

		// get the default font appearance
		GetDefaultCharFormat( fmt );

		ret = (*basicProc)(m_hWnd, WM_PASTE, wParam, lParam);

		SendMessage( EM_EXGETSEL, 0, LPARAM(&rangeEnd) );

		rangeStart.cpMax = rangeEnd.cpMax;

		// set the range, and change the font
		SendMessage( EM_EXSETSEL, 0, LPARAM(&rangeStart));
		SendMessage( EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&fmt));

		SendMessage( EM_EXSETSEL, 0, LPARAM(&rangeEnd));

		// re-enable selection
		HideSelection( FALSE, FALSE );

		m_chrgAfterPaste = rangeStart;
		return ret;
	}
	else
	{
		return (*basicProc)(m_hWnd, WM_PASTE, wParam, lParam);
	}
}

///////////////////////////////////////////////////////////////////////////
//
void TFormRichEdit::SetWrapWidth(BOOL fWrap)
{
	m_fWordWrap = fWrap;
}

//////////////////////////////////////////////////////////////////////////
//  Find the limits of the paragraph that contains 'iCharPos'
//
int TFormRichEdit::GetParagraphBounds(int iCharPos, CHARRANGE* pchrg)
{
	FINDTEXT  ft;
	TCHAR rcBuf[] = "\r\n";
	int  iBegin = 0;
	int  iEnd;
	int  ret;

	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = iCharPos;
	ft.lpstrText = rcBuf;

	// Starting from the top, keep searching for CRLF
	while (TRUE)
	{
		ret = SendMessage(EM_FINDTEXT, 0, (LPARAM)&ft);
		if (ret == -1)
			break;
		else
			iBegin = ret + 2;

		if (ret + 2 >= iCharPos)
			break;
		// go again
		ft.chrg.cpMin = ret + 2;
	}

	// ok we know the paragraph begins at iBegin

	ft.chrg.cpMin = iCharPos;
	ft.chrg.cpMax = iEnd = SendMessage(WM_GETTEXTLENGTH);

	// see where paragraph ends (EOF or CRLF)
	ret = SendMessage(EM_FINDTEXT, 0, (LPARAM) &ft);

	if (ret != -1)
		iEnd = ret;

	pchrg->cpMin = iBegin;
	pchrg->cpMax = iEnd;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//  Return TRUE if color is consistent throughout selection
//
BOOL TFormRichEdit::IsSelectionColorConsistent(COLORREF color)
{
	CHARFORMAT cfm;
	cfm.dwMask = CFM_COLOR;

	// get character attributes of selection
	GetSelectionCharFormat ( cfm );

	// see is color is uniform throughout ...      and matches
	if ((cfm.dwMask & CFM_COLOR) && (color == cfm.crTextColor))
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////
//  Set the color of the current selection. Selection must already be set
//
BOOL TFormRichEdit::SetSelectionColor(COLORREF color)
{
	CHARFORMAT cfm;
	// strictly speaking we shouldn't need to Zero it out. But it works!!
	ZeroMemory( &cfm, sizeof cfm );
	cfm.dwMask = CFM_COLOR;
	cfm.crTextColor = color;
	return SetSelectionCharFormat ( cfm );
}
