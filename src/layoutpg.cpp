/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: layoutpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
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
/*  Revision 1.4  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.3  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:29  richard_wood
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

// layoutpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "layoutpg.h"
#include "globals.h"
#include "helpcode.h"            // HID_OPTIONS_*

#include "utilsize.h"
#include "rglaymdi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

typedef struct tagPANESYNC {
	TUIMemory::EPaneType ePaneType;
	int                  strID;
} PANESYNC;

static PANESYNC vPaneSync[] = {
	{TUIMemory::PANE_NEWSGROUPS, IDS_LAYOUT_NEWSGROUPS},
	{TUIMemory::PANE_THREADVIEW, IDS_LAYOUT_THREADS},
	{TUIMemory::PANE_ARTVIEW,    IDS_LAYOUT_ARTICLE}
};

/////////////////////////////////////////////////////////////////////////////
// TOptionsLayoutPage property page

IMPLEMENT_DYNCREATE(TOptionsLayoutPage, CPropertyPage)

TOptionsLayoutPage::TOptionsLayoutPage() : CPropertyPage(TOptionsLayoutPage::IDD)
{
	uDragListboxMessage = 0;
}

TOptionsLayoutPage::~TOptionsLayoutPage()
{
}

void TOptionsLayoutPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_3_UP, m_s3Up);
	DDX_Control(pDX, IDC_3_DOWN, m_s3Down);

	CListBox* pLBP;      // picture
	CListBox* pLBT;      // text

	pLBP = &m_lbx3;
	pLBT = &m_lbxText3;

	int k, tot;
	if (FALSE == pDX->m_bSaveAndValidate)
	{
		// set correct cursel
		tot = pLBP->GetCount();
		for (k = 0; k < tot; ++k)
		{
			if ( (TUIMemory::EMdiLayout) pLBP->GetItemData(k) == m_eLayout )
			{
				pLBP->SetCurSel(k);
				break;
			}
		}
	}
	else
	{
		// save to DataStruct
		int cursel = pLBP->GetCurSel();
		if (LB_ERR != cursel)
			m_eLayout = (TUIMemory::EMdiLayout) pLBP->GetItemData(cursel);
		// remember the order
		tot = pLBT->GetCount();
		for (k = 0; k < tot; ++k)
		{
			m_vPanes[k] = (TUIMemory::EPaneType) pLBT->GetItemData(k);
		}
	}
}

BEGIN_MESSAGE_MAP(TOptionsLayoutPage, CPropertyPage)
	ON_BN_CLICKED(IDC_3_DOWN, On3Down)
	ON_BN_CLICKED(IDC_3_UP, On3Up)
	ON_WM_HELPINFO()
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsLayoutPage override
LRESULT TOptionsLayoutPage::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	if (message == uDragListboxMessage)
	{
		DRAGLISTINFO* pInfo = (DRAGLISTINFO*) lParam;

		switch (pInfo->uNotification)
		{
		case DL_BEGINDRAG:
			return lbxBeginDrag (wParam, pInfo);

		case DL_CANCELDRAG:
			return lbxCancelDrag (wParam, pInfo);

		case DL_DRAGGING:
			return lbxDragging (wParam, pInfo);

		case DL_DROPPED:
			return lbxDropped (wParam, pInfo);

		default:
			ASSERT(0);
			return 0;
		}
	}
	else
		return CPropertyPage::WindowProc ( message, wParam, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// TOptionsLayoutPage message handlers

BOOL TOptionsLayoutPage::OnInitDialog()
{
	int i, k;

	// make backup copy of original values
	m_bakLayout = m_eLayout;
	for (i = 0; i < (sizeof(m_vPanes)/sizeof(m_vPanes[0])); ++i)
		m_bakPanes[i] = m_vPanes[i];

	// let the .RC file create the Listbox, but hook in the TLayoutLbx behavior
	m_lbx3.SubclassDlgItem (IDC_LAYOUT_3PICTURES, this);

	m_lbx3.FillPictures();
	CString str;
	m_lbxText3.SubclassDlgItem (IDC_LAYOUT_3PANES, this);

	// Fill the three panes option. Order matters
	for (i = 0; i < 3; ++i)
	{
		// locate correct stringid
		for (k = 0; k < sizeof(vPaneSync)/sizeof(vPaneSync[0]); ++k)
		{
			if (vPaneSync[k].ePaneType == m_vPanes[i])
				break;
		}

		str.LoadString (vPaneSync[k].strID);
		m_lbxText3.InsertString(-1, str);
		m_lbxText3.SetItemData(i, (DWORD) vPaneSync[k].ePaneType);
	}

	// these are ownerdrawn, but we missed out on WM_MEASUREITEM.
	int h;
	m_lbxText3.CalcItemHeight ( h );
	m_lbxText3.SetItemHeight ( 0, h );

	// make these draggable
	VERIFY( MakeDragList (m_lbxText3.m_hWnd) );

	uDragListboxMessage = RegisterWindowMessage(DRAGLISTMSGSTRING);

	// call DoDataExchange
	CPropertyPage::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT TOptionsLayoutPage::lbxBeginDrag  (WPARAM wParam, DRAGLISTINFO* pInfo)
{
	TRACE0("Start   dragging\n");
	m_iDragCtrlId = (int) wParam;
	int i = LBItemFromPt (pInfo->hWnd, pInfo->ptCursor, FALSE);
	if (-1 != i)
	{
		m_iDragIndex = i;
		TRACE1(" item is %d\n", i);
	}
	return TRUE;
}

LRESULT TOptionsLayoutPage::lbxCancelDrag (WPARAM wParam, DRAGLISTINFO* pInfo)
{
	m_iDragIndex = 666;
	return NULL;
}

LRESULT TOptionsLayoutPage::lbxDragging   (WPARAM wParam, DRAGLISTINFO* pInfo)
{
	// find out what item is under cursor
	int i = LBItemFromPt (pInfo->hWnd, pInfo->ptCursor, TRUE /* autoscroll */);
	if (-1 != i)
	{
		// give some feedback
		DrawInsert(this->m_hWnd, pInfo->hWnd, i/*IDI_DRAGLBX_ARROW*/);
	}

	if (-1 == i)
		return DL_STOPCURSOR;
	else
		return DL_MOVECURSOR;
}

LRESULT TOptionsLayoutPage::lbxDropped    (WPARAM wParam, DRAGLISTINFO* pInfo)
{
	TRACE0("Dropped\n");

	// find out what item is under cursor
	int i = LBItemFromPt (pInfo->hWnd, pInfo->ptCursor, FALSE);
	if (-1 != i)
	{
		// move Saved item before this
		if (i == m_iDragIndex)
			;
		else if (i > m_iDragIndex)
		{
			CListBox* pLbx = (CListBox*) CWnd::FromHandle(pInfo->hWnd);

			// re-insert at i-1
			int dx;
			CString str;   pLbx->GetText(m_iDragIndex, str);
			DWORD dw = pLbx->GetItemData(m_iDragIndex);
			dx = pLbx->InsertString(i, str);
			pLbx->SetItemData(dx, dw);
			pLbx->DeleteString (m_iDragIndex);
		}
		else if (i < m_iDragIndex)
		{
			CListBox* pLbx = (CListBox*) CWnd::FromHandle(pInfo->hWnd);
			// re-insert at i-1
			int dx;
			CString str;   pLbx->GetText(m_iDragIndex, str);
			DWORD dw = pLbx->GetItemData(m_iDragIndex);
			dx = pLbx->InsertString(i, str);
			pLbx->SetItemData(dx, dw);
			pLbx->DeleteString (m_iDragIndex+1);  // cuz we inserted one.
		}

	}
	m_iDragIndex = 666;

	CRect aRect;
	Utility_GetPosParent(CWnd::FromHandle(m_hWnd), CWnd::FromHandle(pInfo->hWnd), aRect);
	// erase the DrawInsert image
	// erase the area to the immediate left of the listbox
	aRect.right = aRect.left;
	aRect.left = aRect.right - 20;
	aRect.top -= 10;
	this->InvalidateRect(&aRect, TRUE);

	return DL_STOPCURSOR;
}

BOOL TOptionsLayoutPage::IsDifferent(void)
{
	if (m_eLayout != m_bakLayout)
		return TRUE;

	int count = TUIMemory::LayoutPaneCount(m_eLayout);
	for (--count; count >= 0; --count)
		if (m_vPanes[count] != m_bakPanes[count])
			return TRUE;
	return FALSE;
}

void TOptionsLayoutPage::DataIn(TRegLayoutMdi* pRegLayoutMdi)
{
	ASSERT(sizeof(pRegLayoutMdi->m_vPanes) == sizeof(m_vPanes));
	int max = sizeof m_vPanes / sizeof(m_vPanes[0]);
	m_bakLayout   = m_eLayout   =  pRegLayoutMdi->m_layout;
	for (int i = 0; i < max; i++)
		m_bakPanes[i] = m_vPanes[i] =  pRegLayoutMdi->m_vPanes[i];
}

void TOptionsLayoutPage::DataOut(TRegLayoutMdi* pRegLayoutMdi)
{
	ASSERT(sizeof(pRegLayoutMdi->m_vPanes) == sizeof(m_vPanes));

	int max = sizeof(pRegLayoutMdi->m_vPanes) /
		sizeof(pRegLayoutMdi->m_vPanes[0]);

	pRegLayoutMdi->m_layout    = m_eLayout;
	for (int i = 0; i < max; i++)
		pRegLayoutMdi->m_vPanes[i] = m_vPanes[i];
}

// -------------------------------------------------------------------------
static enum Direction {UP, DOWN};

// -------------------------------------------------------------------------
static void MoveItem (TLayoutTextLbx *psLbx, Direction iDirection)
{
	// get the currently-selected item's index
	int iOldIndex = psLbx->GetCurSel ();
	if (iOldIndex < 0)
		return;

	// if index is already at bottom/top, stop
	if ((iDirection == DOWN && iOldIndex == psLbx->GetCount () - 1) ||
		(iDirection == UP && !iOldIndex))
		return;

	// get its string
	CString str;
	psLbx->GetText (iOldIndex, str);

	// get its data
	DWORD dwData = psLbx->GetItemData (iOldIndex);

	// insert into new position
	int iNewIndex = psLbx->InsertString (
		iDirection == DOWN ? iOldIndex + 2 : iOldIndex - 1, str);
	psLbx->SetCurSel (iNewIndex);
	psLbx->SetItemData (iNewIndex, dwData);

	// delete from old position
	psLbx->DeleteString (iDirection == DOWN ? iOldIndex : iOldIndex + 1);
}

// -------------------------------------------------------------------------
void TOptionsLayoutPage::On3Down()
{
	MoveItem (&m_lbxText3, DOWN);
}

// -------------------------------------------------------------------------
void TOptionsLayoutPage::On3Up()
{
	MoveItem (&m_lbxText3, UP);
}

// -------------------------------------------------------------------------
BOOL TOptionsLayoutPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp ()->HtmlHelp((DWORD)"customize-pane-layout.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_WINDOW_LAYOUT);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsLayoutPage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
