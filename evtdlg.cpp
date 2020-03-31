/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: evtdlg.cpp,v $
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:23  richard_wood
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

// evtdlg.cpp : implementation file
//
// Goal:  View the Event Log
//
// + Set the MINMAX size
// + smaller font
// + limit array size
// + autorefresh
// + save, load to registry
//   1 Size of window, position of window (2 ints, 2 ints)
//   2 hdr widths(4ints)
//   3 fAutoRefresh (1 int)
// + needs an icon
//

#include "stdafx.h"
#include "news.h"
#include "evtdlg.h"
#include "utilsize.h"
#include "vlist.h"
#include "evtlog.h"
#include "sysclr.h"
#include "evtdtl.h"
#include "tglobopt.h"
#include "autofont.h"
#include "rgui.h"
#include "gdiutil.h"
#include "superstl.h"            // istrstream, ...

extern TGlobalOptions* gpGlobalOptions;
extern TEventLog* gpEventLog;
extern TSystem gSystem;

const int HDRCTRL_ID = 14145;
const int EVTLOG_TIMER_NUM = 27;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

enum ESource   { kGeneral, kPump, kScribe, kRules, kDecode, kTasker };
typedef struct tagLogEvent
{
	TEventEntry::ESource eSource;
	WORD                 wStringID;
} T_LogEvent;

T_LogEvent mymap[] = { {TEventEntry::kGeneral,     IDS_EVENT_GENERAL},
{TEventEntry::kPump,        IDS_EVENT_PUMP},
{TEventEntry::kScribe,      IDS_EVENT_SCRIBE},
{TEventEntry::kRules,       IDS_EVENT_RULES},
{TEventEntry::kDecode,      IDS_EVENT_DECODE},
{TEventEntry::kTasker,      IDS_EVENT_TASKER},
{TEventEntry::kCleaner,     IDS_EVENT_CLEANER} };

/////////////////////////////////////////////////////////////////////////////
// TEventViewerDlg dialog

TEventViewerDlg::TEventViewerDlg(CWnd* pParent /*=NULL*/)
: CDialog(TEventViewerDlg::IDD, pParent)
{

	m_iAutoRefresh = FALSE;

	m_fFontInited = FALSE;
	m_fWidthsInited = FALSE;
	m_iAutoRefresh = TRUE;
}

void TEventViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_EVENT_REFRESHCBX, m_iAutoRefresh);


	if (!pDX->m_bSaveAndValidate)
	{
		::SendMessage ( m_hwndVList, VLB_INITIALIZE, 0, 0L);

		gpEventLog->Lock();
		CWnd * pButton = GetDlgItem(IDC_EVTLOG_DETAILS);
		pButton->EnableWindow ( gpEventLog->m_vEvents.GetSize() > 0 );
		gpEventLog->Unlock();
	}
}

BEGIN_MESSAGE_MAP(TEventViewerDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_MESSAGE(VLB_RANGE,        On_vlbRange)
	ON_MESSAGE(VLB_PREV,         On_vlbPrev)
	ON_MESSAGE(VLB_FINDPOS,      On_vlbFindPos)
	ON_MESSAGE(VLB_FINDITEM,     On_vlbFindItem)
	ON_MESSAGE(VLB_FINDSTRING,   On_vlbFindString)
	ON_MESSAGE(VLB_SELECTSTRING, On_vlbSelectString)
	ON_MESSAGE(VLB_NEXT,         On_vlbNext)
	ON_MESSAGE(VLB_FIRST,        On_vlbFirst)
	ON_MESSAGE(VLB_LAST,         On_vlbLast)
	ON_MESSAGE(VLB_GETTEXT,      On_vlbGetText)
	ON_WM_DRAWITEM()
	ON_NOTIFY(HDN_ENDTRACK, HDRCTRL_ID, OnHdrctrlEndTrack)
	ON_LBN_DBLCLK( IDC_EVENT_LBX, OnVListDblClk)
	ON_WM_GETMINMAXINFO()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_EVTLOG_DETAILS, OnEvtlogDetails)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TEventViewerDlg message handlers

void TEventViewerDlg::OnOK()
{
	OnClose();
}

void TEventViewerDlg::OnCancel()
{
	OnClose();
}

void TEventViewerDlg::OnClose()
{
	// save settings to registry

	// even if the window is minimized, we will get the correct position
	WINDOWPLACEMENT wp;
	GetWindowPlacement (&wp);

	RECT & rct = wp.rcNormalPosition;

	m_iAutoRefresh = IsDlgButtonChecked(IDC_EVENT_REFRESHCBX);
	CString data;
	data.Format ("%d %d %d %d %d %d %d %d %d",
		rct.left, rct.top, rct.right-rct.left, rct.bottom-rct.top,
		m_vHdrWidths[0],m_vHdrWidths[1],m_vHdrWidths[2],m_vHdrWidths[3],
		m_iAutoRefresh);

	gpGlobalOptions->GetRegUI()->SaveEventLogData(data);

	KillTimer( m_timerID );
	// modeless dlgs need this
	DestroyWindow();
}

BOOL TEventViewerDlg::OnInitDialog()
{
	TRegUI* pRegUI = gpGlobalOptions->GetRegUI();
	CString data;
	pRegUI->LoadEventLogData(data);
	POINT dlgPos;
	SIZE  dlgSize;
	BOOL fSetPos = FALSE;
	if (!data.IsEmpty())
	{
		istrstream iss((LPTSTR)(LPCTSTR) data);
		iss >> dlgPos.x >> dlgPos.y >> dlgSize.cx >> dlgSize.cy
			>> m_vHdrWidths[0] >> m_vHdrWidths[1] >> m_vHdrWidths[2]
		>> m_vHdrWidths[3] >> m_iAutoRefresh ;
		fSetPos = TRUE;
		m_fWidthsInited = TRUE;
	}
	setup_font();
	setup_headerctrl();

	RegisterVListBox(AfxGetInstanceHandle());

	m_hwndVList = ::CreateWindow(VLIST_CLASSNAME, "myVL",
		VLBS_OWNERDRAWFIXED | VLBS_NOTIFY |
		WS_VISIBLE | WS_BORDER | WS_CHILD,
		0, 0,
		10, 10,
		this->m_hWnd,
		(HMENU) IDC_EVENT_LBX,
		AfxGetInstanceHandle(),
		NULL);

	ASSERT(m_hwndVList);

	// call DDX
	CDialog::OnInitDialog();

	m_timerID = SetTimer(EVTLOG_TIMER_NUM, 1000, NULL);

	if (fSetPos)
	{
		// now that we have created everything. Implement the size we
		// read from the Registry
		SetWindowPos(NULL, dlgPos.x, dlgPos.y, dlgSize.cx, dlgSize.cy,
			SWP_NOZORDER);
	}
	CRect rct;
	GetClientRect (&rct);
	SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rct.right, rct.bottom));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void TEventViewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// Move the button
	CWnd* but = GetDlgItem(IDOK);

	if (!but)
		return;

	LONG lBoth = ::GetDialogBaseUnits();
	int pixelX = (2*LOWORD(lBoth)) / 4;
	int pixelY = (4*HIWORD(lBoth)) / 8;

	CRect rct;
	but->GetWindowRect(&rct);

	int j = cx - (rct.right-rct.left) - pixelX;
	but->SetWindowPos(NULL,
		j,
		pixelY,
		NULL,
		NULL,
		SWP_NOZORDER | SWP_NOSIZE);

	// Checkbox is right under the button
	CWnd* pCheck = GetDlgItem(IDC_EVENT_REFRESHCBX);
	int iCheckY = pixelY + (rct.bottom-rct.top) + (HIWORD(lBoth));
	pCheck->SetWindowPos(NULL, j, iCheckY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	int VCY = cy - (2*pixelY);
	int VCX = j - (2*pixelX);

	// details button is under the checkbox (separated by an AvgChar 'pixelY')
	pCheck->GetWindowRect(&rct);
	GetDlgItem(IDC_EVTLOG_DETAILS)->SetWindowPos(NULL, j,
		iCheckY + pixelY + rct.Height(),
		0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// place the header
	int shuffle = 40;
	RECT sRect;
	sRect.top = pixelY;
	sRect.bottom = sRect.top + shuffle;
	sRect.left = pixelX;
	sRect.right = pixelX + VCX;
	WINDOWPOS wp;
	ZeroMemory(&wp, sizeof(wp));
	HD_LAYOUT sLayout;
	sLayout.prc = &sRect;
	sLayout.pwpos = &wp;
	m_hdr.Layout ( &sLayout );
	m_hdr.SetWindowPos(NULL, wp.x, wp.y, wp.cx, wp.cy,
		wp.flags | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
	// painting is not right
	m_hdr.Invalidate();

	// place virtual listbox
	::MoveWindow(m_hwndVList, pixelX,
		pixelY+wp.cy, VCX, VCY-wp.cy, TRUE);
}

///////Virtual Listbox Stuff
LRESULT TEventViewerDlg::On_vlbRange   (WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT  lpvlbInStruct = (LPVLBSTRUCT)lParam;
	gpEventLog->Lock();
	lpvlbInStruct->lIndex = gpEventLog->m_vEvents.GetSize();
	gpEventLog->Unlock();
	lpvlbInStruct->nStatus = VLB_OK;
	return TRUE;
}

LRESULT TEventViewerDlg::On_vlbPrev    (WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT  lpvlbInStruct = (LPVLBSTRUCT)lParam;
	if ( lpvlbInStruct->lIndex > 0 ) {
		lpvlbInStruct->nStatus = VLB_OK;
		lpvlbInStruct->lIndex--;
		lpvlbInStruct->lData = lpvlbInStruct->lIndex;

		//lstrcpy (m_szText, m_pCurGroups->GetItem ( lpvlbInStruct->lIndex ) );
		lpvlbInStruct->lpTextPointer = 0;
		return TRUE;
	}
	else {
		lpvlbInStruct->nStatus = VLB_ENDOFFILE;
		return TRUE;
	}
}

LRESULT TEventViewerDlg::On_vlbFindPos (WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;
	if (0 == lpvlbInStruct->lIndex)
		return On_vlbFirst (wP, lParam);
	else if (100 == lpvlbInStruct->lIndex)
		return On_vlbLast (wP, lParam);
	else
	{
		int total;
		gpEventLog->Lock();
		total = gpEventLog->m_vEvents.GetSize();
		gpEventLog->Unlock();

		lpvlbInStruct->lIndex = (100*total) / lpvlbInStruct->lIndex ;

		// sanity chk
		if (lpvlbInStruct->lIndex >= total)
			lpvlbInStruct->lIndex = total-1;

		lpvlbInStruct->nStatus = VLB_OK;
		lpvlbInStruct->lpTextPointer = 0;
		return TRUE;
	}
}

//  VLB gives us an index. We find out the string and pass it back.
//    since this is ownerdrawn, we just take the index and pass it
//    back as the LB_SETITEMDATA argument.
LRESULT TEventViewerDlg::On_vlbFindItem(WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

	lpvlbInStruct->lData = lpvlbInStruct->lIndex ;
	lpvlbInStruct->nStatus = VLB_OK;

	lpvlbInStruct->lpTextPointer = 0;
	return TRUE;
}

// Needs work
//  VLB gives us a string. We figure out the Index and pass it
//  back.
LRESULT TEventViewerDlg::On_vlbFindString(WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;
	return TRUE;
}

LRESULT TEventViewerDlg::On_vlbSelectString(WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;
	return TRUE;
}

LRESULT TEventViewerDlg::On_vlbNext    (WPARAM wP, LPARAM lParam)
{
	int total;
	gpEventLog->Lock();
	total = gpEventLog->m_vEvents.GetSize();
	gpEventLog->Unlock();

	LPVLBSTRUCT  lpvlbInStruct = (LPVLBSTRUCT)lParam;
	if ( lpvlbInStruct->lIndex < total - 1 ) {
		lpvlbInStruct->nStatus = VLB_OK;
		lpvlbInStruct->lIndex++;
		lpvlbInStruct->lData = lpvlbInStruct->lIndex;
		lpvlbInStruct->lpTextPointer = 0;
		return TRUE;
	}
	else {
		lpvlbInStruct->nStatus = VLB_ENDOFFILE;
		return TRUE;
	}
}

LRESULT TEventViewerDlg::On_vlbFirst   (WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

	lpvlbInStruct->nStatus = VLB_OK;
	lpvlbInStruct->lIndex = 0L;
	lpvlbInStruct->lpTextPointer = 0;
	lpvlbInStruct->lData = lpvlbInStruct->lIndex;
	return TRUE;
}

LRESULT TEventViewerDlg::On_vlbLast    (WPARAM wP, LPARAM lParam)
{
	int total;
	gpEventLog->Lock();
	total = gpEventLog->m_vEvents.GetSize();
	gpEventLog->Unlock();

	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

	lpvlbInStruct->nStatus = VLB_OK;
	lpvlbInStruct->lIndex = total - 1;
	lpvlbInStruct->lpTextPointer = 0;
	lpvlbInStruct->lData = lpvlbInStruct->lIndex;
	return TRUE;
}

LRESULT TEventViewerDlg::On_vlbGetText (WPARAM wP, LPARAM lParam)
{
	LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;
	return TRUE;
}

void TEventViewerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
	if (nIDCtl != IDC_EVENT_LBX)
		return;

	if (-1 == lpdis->itemID)
		return;

	int n;
	gpEventLog->Lock();
	n = gpEventLog->m_vEvents.GetSize();
	gpEventLog->Unlock();

	if ((0 == n) || ((int)lpdis->itemData >= n))
		return;

	if ((lpdis->itemAction & ODA_DRAWENTIRE) ||
		((lpdis->itemAction & ODA_SELECT) && (lpdis->itemState & ODS_SELECTED))
		)
	{
		// draw text normally or Selected Text
		draw_select( lpdis );
		if (lpdis->itemState & ODS_FOCUS)
			draw_focus (lpdis);
		return;
	}
	// Draw hilite color, draw text
	if (lpdis->itemAction & ODA_SELECT)
	{
		draw_select ( lpdis );
		return;
	}
	// Draw focus rect
	if (lpdis->itemAction & ODA_FOCUS)
	{
		draw_focus ( lpdis );
		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpdis);
}

void TEventViewerDlg::draw_select(LPDRAWITEMSTRUCT lpdis)
{
	gpEventLog->Lock();
	const TEventEntry& entry = gpEventLog->m_vEvents.GetAt(lpdis->itemData);
	blast_text ( lpdis, &entry );
	gpEventLog->Unlock();
}

void TEventViewerDlg::blast_text(LPDRAWITEMSTRUCT lpdis, const TEventEntry* pEntry)
{
	CDC* pDC = CDC::FromHandle(lpdis->hDC);
	COLORREF oldColor;
	int      oldMode;

	DWORD dwBack, dwText;
	if (lpdis->itemState & ODS_SELECTED)
	{
		dwBack = gSystem.Highlight();
		dwText = gSystem.HighlightText();
	}
	else
	{
		dwBack = gSystem.Window();
		// force the grey background
		//dwBack = gSystem.ColorScrollbar();
		dwText = gSystem.WindowText();
	}

	// fill the background
	CBrush br(dwBack);
	pDC->FillRect(&lpdis->rcItem, &br);
	oldMode = pDC->SetBkMode (TRANSPARENT);

	CString strPrio;
	// get bright color for priority token
	get_prio_string(pEntry, strPrio);
	switch (pEntry->m_ePriority)
	{
	case TEventEntry::kError:
		oldColor = pDC->SetTextColor(RGB(128,0,0));
		strPrio.LoadString(IDS_EVENT_ERROR);
		break;
	case TEventEntry::kWarning:
		oldColor = pDC->SetTextColor(RGB(0,0,128));
		strPrio.LoadString(IDS_EVENT_WARNING);
		break;
	case TEventEntry::kInfo:
		oldColor = pDC->SetTextColor(dwText);
		strPrio.LoadString(IDS_EVENT_INFO);
		break;
	}

	int vOffset[4];
	vOffset[0] = 4;  // don't be jammed up against the side
	vOffset[1] = vOffset[0] + m_vHdrWidths[0];
	vOffset[2] = vOffset[1] + m_vHdrWidths[1];
	vOffset[3] = vOffset[2] + m_vHdrWidths[2];

	TAutoFont fontmgr(pDC, &m_font);
	RECT rctClip = lpdis->rcItem;

	//[Priority] [Time] [Source] [Text]
	rctClip.right = vOffset[1];
	pDC->ExtTextOut(lpdis->rcItem.left+ vOffset[0],
		lpdis->rcItem.top, ETO_CLIPPED,
		&rctClip, LPCTSTR(strPrio), strPrio.GetLength(), NULL);

	pDC->SetTextColor(dwText);  // normal or highlighted color

	// the time
	rctClip.right = vOffset[2];
	strPrio = pEntry->m_time.Format("%I:%M:%S %p");
	pDC->ExtTextOut(lpdis->rcItem.left + vOffset[1],
		lpdis->rcItem.top, ETO_CLIPPED,
		&rctClip, LPCTSTR(strPrio), strPrio.GetLength(), NULL);
	// the source
	strPrio.Empty();
	for(int j = 0; j < sizeof(mymap)/sizeof(mymap[0]); ++j)
	{
		if (pEntry->m_eSource == mymap[j].eSource) {
			strPrio.LoadString(mymap[j].wStringID);
			break;
		}
	}
	rctClip.right = vOffset[3];
	pDC->ExtTextOut(lpdis->rcItem.left + vOffset[2],
		lpdis->rcItem.top, ETO_CLIPPED,
		&rctClip, LPCTSTR(strPrio), strPrio.GetLength(), NULL);

	pDC->ExtTextOut(lpdis->rcItem.left + vOffset[3],
		lpdis->rcItem.top, ETO_CLIPPED,
		&lpdis->rcItem,
		LPCTSTR(pEntry->m_desc), pEntry->m_desc.GetLength(), NULL);

	pDC->SetBkMode(oldMode);
	pDC->SetTextColor(oldColor);
}

void TEventViewerDlg::draw_focus(LPDRAWITEMSTRUCT lpdis)
{
	::DrawFocusRect ( lpdis->hDC, &lpdis->rcItem );
}

// get header widths from registry or use some defaults
void TEventViewerDlg::get_header_widths(int* pWidths)
{
	if (FALSE == m_fWidthsInited)
	{
		LONG lBoth = ::GetDialogBaseUnits();
		pWidths[0] = ( 9 * LOWORD(lBoth)) ;               // priority
		pWidths[1] = (11 * LOWORD(lBoth)) ;               // time
		pWidths[2] = (11 * LOWORD(lBoth)) ;               // source
		pWidths[3] = (50 * LOWORD(lBoth)) ;               // description
	}
}

void TEventViewerDlg::setup_headerctrl()
{
	RECT arct; arct.top = arct.left = 0;
	arct.bottom = 30;
	arct.right = 200;
	VERIFY (
		m_hdr.Create(HDS_HORZ | /*CCS_NORESIZE | */ WS_CHILD | WS_VISIBLE,
		arct, this, HDRCTRL_ID) );

	// Try to get widths from registry
	get_header_widths (m_vHdrWidths);

	HD_ITEM hdi;
	CString str;
	str.LoadString(IDS_EVENT_HDRPRIORITY);
	hdi.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hdi.cxy  = m_vHdrWidths[0];
	hdi.pszText = LPTSTR(LPCTSTR(str));
	hdi.cchTextMax = str.GetLength();
	hdi.fmt = HDF_STRING | HDF_LEFT;
	m_hdr.InsertItem(0, &hdi);

	str.LoadString(IDS_EVENT_HDRTIME);
	hdi.cxy  = m_vHdrWidths[1];
	hdi.pszText = LPTSTR(LPCTSTR(str));
	hdi.cchTextMax = str.GetLength();
	m_hdr.InsertItem(1, &hdi);

	str.LoadString(IDS_EVENT_HDRSOURCE);
	hdi.cxy  = m_vHdrWidths[2];
	hdi.pszText = LPTSTR(LPCTSTR(str));
	hdi.cchTextMax = str.GetLength();
	m_hdr.InsertItem(2, &hdi);

	str.LoadString(IDS_EVENT_HDRDESC);
	hdi.cxy  = m_vHdrWidths[3];
	hdi.pszText = LPTSTR(LPCTSTR(str));
	hdi.cchTextMax = str.GetLength();
	m_hdr.InsertItem(3, &hdi);
}

void TEventViewerDlg::OnHdrctrlEndTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	HD_NOTIFY* pHDN = (HD_NOTIFY*) pNMHDR;
	HD_ITEM hdi;
	hdi.mask = HDI_WIDTH;
	for (int i = 0; i < 4; ++i)
	{
		if (i == pHDN->iItem)
		{
			m_vHdrWidths[i] = pHDN->pitem->cxy;
		}
		else
		{
			m_hdr.GetItem(i, &hdi);
			m_vHdrWidths[i] = hdi.cxy;      // ownerdraw listbox will use these
		}
	}
	::InvalidateRect(m_hwndVList, NULL, TRUE);
}

void TEventViewerDlg::setup_font()
{
	if (m_fFontInited) return;
	LOGFONT lf;
	CClientDC cdc(this);

	// make 10 points
	setupSansSerifFont ( NEWS32_BASE_FONT_PTSIZE, cdc.m_hDC, &lf );

	VERIFY ( m_font.CreateFontIndirect( &lf ) );
	m_fFontInited = TRUE;
}

afx_msg void TEventViewerDlg::OnVListDblClk(void)
{
	OnEvtlogDetails() ;
}

void TEventViewerDlg::get_prio_string(
									  const TEventEntry* pEntry,
									  CString& str)
{
	int str_id;
	switch (pEntry->m_ePriority)
	{
	case TEventEntry::kError:
		str_id = IDS_EVENT_ERROR;
		break;
	case TEventEntry::kWarning:
		str_id = IDS_EVENT_WARNING;
		break;
	case TEventEntry::kInfo:
		str_id = IDS_EVENT_INFO;
		break;
	default:
		ASSERT(0);
		return;
	}
	str.LoadString ( str_id );
}

void TEventViewerDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	lpMMI->ptMinTrackSize.x = 250;
	lpMMI->ptMinTrackSize.y = 190;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void TEventViewerDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if ((EVTLOG_TIMER_NUM == nIDEvent) &&
		IsDlgButtonChecked(IDC_EVENT_REFRESHCBX))
	{
		gpEventLog->Lock();
		if (gpEventLog->m_fViewIsOutofdate)
		{
			// reload from scratch
			::SendMessage( m_hwndVList, VLB_INITIALIZE, 0, 0L );
			gpEventLog->m_fViewIsOutofdate = FALSE;
		}

		CWnd * pButton = GetDlgItem(IDC_EVTLOG_DETAILS);
		pButton->EnableWindow ( gpEventLog->m_vEvents.GetSize() > 0 );
		gpEventLog->Unlock();
	}
	CDialog::OnTimer(nIDEvent);
}

void TEventViewerDlg::OnEvtlogDetails()
{
	int ret;
	ret = ::SendMessage( m_hwndVList, VLB_GETCURSEL, 0, 0);
	if (ret < 0) return;
	// this is a modeless dlgbox that is spawning a Modal Dialog box. hmm
	TEventViewDetailDlg dlg(this);
	gpEventLog->Lock();
	if (gpEventLog->m_vEvents.GetSize() <= 0)
	{
		gpEventLog->Unlock();
		return;
	}
	const TEventEntry& entry = gpEventLog->m_vEvents.GetAt(ret);

	get_prio_string(&entry, dlg.m_prio);
	dlg.m_strTime = entry.m_time.Format("%I:%M:%S %p");

	for(int j = 0; j < sizeof(mymap)/sizeof(mymap[0]); ++j)
		if (entry.m_eSource == mymap[j].eSource) {
			dlg.m_source.LoadString(mymap[j].wStringID);
			break;
		}

		dlg.m_desc = entry.m_desc;
		if (entry.m_pExtendedDesc)
			dlg.m_details = *(entry.m_pExtendedDesc);
		gpEventLog->Unlock();

		dlg.DoModal();  // does this prevent us from shutting down?
}
