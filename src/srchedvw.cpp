/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: srchedvw.cpp,v $
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
/*  Revision 1.3  2008/09/19 14:51:53  richard_wood
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

// srchedvw.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "srchedvw.h"
#include "artdisp.h"          // SetInitialFont
#include "tglobopt.h"
#include "rgbkgrnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TSearchEditView

IMPLEMENT_DYNCREATE(TSearchEditView, CView)

TSearchEditView::TSearchEditView()
{
}

TSearchEditView::~TSearchEditView()
{
}

BEGIN_MESSAGE_MAP(TSearchEditView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSearchEditView drawing
void TSearchEditView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// TSearchEditView diagnostics
#ifdef _DEBUG
void TSearchEditView::AssertValid() const
{
	CView::AssertValid();
}

void TSearchEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TSearchEditView message handlers
int TSearchEditView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialize the rich edit control
	RECT rct; rct.left = 0; rct.top = 0;
	rct.right = lpCreateStruct->cx; rct.bottom = lpCreateStruct->cy;

	if (m_rich.Create(WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE |
		WS_VSCROLL,
		rct, this, IDC_SEARCH_RICHEDIT))
	{
		// sets some event mask flags
		m_rich.PostSubclass();

		LONG lStyle = GetWindowLong(m_rich.m_hWnd, GWL_EXSTYLE);
		SetWindowLong(m_rich.m_hWnd, GWL_EXSTYLE,
			lStyle & ~WS_EX_CLIENTEDGE);

		// please send WM_NOTIFY:EN_SELCHANGE to the parent
		DWORD dwEvntMask = m_rich.SendMessage(EM_GETEVENTMASK);
		dwEvntMask |= ENM_SELCHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_SCROLLEVENTS;
		m_rich.SendMessage(EM_SETEVENTMASK, 0, dwEvntMask);
		//ENM_SELCHANGE | m_rich.SendMessage(EM_GETEVENTMASK));

		SetInitialFont(&m_rich);

		// auto vscroll (duh)
		m_rich.SendMessage(EM_SETOPTIONS, ECOOP_OR, ECO_AUTOVSCROLL | ECO_READONLY);

		// ECO_AUTOVSCROLL somehow creates a phantom scrollbar
		//   do a little detour to crush the vertical scrollbar
		SCROLLINFO sScrollInfo;
		sScrollInfo.cbSize = sizeof(sScrollInfo);
		sScrollInfo.fMask = SIF_RANGE;
		sScrollInfo.nMin = sScrollInfo.nMax = 0;
		m_rich.SetScrollInfo(SB_VERT, &sScrollInfo, TRUE);

		TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
		COLORREF crf = pBackgrounds->GetEffectiveArtviewBackground();

		m_rich.SendMessage(EM_SETBKGNDCOLOR, FALSE, LPARAM(crf));

		// Clear SES_XLTCRCRLFTOCR
		m_rich.SendMessage(EM_SETEDITSTYLE, 0, SES_XLTCRCRLFTOCR);

		m_rich.SetupXFaceWindow();
	}
	else
		ASSERT (0);

	return 0;
}

void TSearchEditView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_rich.MoveWindow(0, 0, cx, cy);
}
