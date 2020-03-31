/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: treehctl.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.9  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.8  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.7  2009/02/17 14:28:23  richard_wood
/*  Deleted old commented out code.
/*
/*  Revision 1.6  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.5  2009/01/30 00:09:18  richard_wood
/*  Removed redundant files.
/*  Fixed header on thread view.
/*
/*  Revision 1.4  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:16  richard_wood
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

// treehctl.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "treehctl.h"
#include "tglobopt.h"
#include "rgui.h"
#include "thrdlvw.h"
#include "sortart.h"             // TSortArticlesDlg
#include "hierlbxw.h"
#include "superstl.h"            // istrstream, ...
#include "autodrw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;
extern CFont*        gpHeaderCtrlFont;

#define THREAD_ICON_COL_WIDTH 25

static const int kiIndicatorColCX = (TREE_WIDTH_TOTAL - TREE_WIDTH_OPEN + 4);

typedef struct
{
	TCHAR c;
	TTreeHeaderCtrl::EColumn eCol;
} THC_MU;

THC_MU vTHCMap[] = { {'T', TTreeHeaderCtrl::kThread       },
{'F', TTreeHeaderCtrl::kFrom         },
{'I', TTreeHeaderCtrl::kIndicator    },
{'S', TTreeHeaderCtrl::kSubject      },
{'L', TTreeHeaderCtrl::kLines        },
{'R', TTreeHeaderCtrl::kScore        },
{'D', TTreeHeaderCtrl::kDate         } };

// 2 prototypes
static TCHAR THCMap_Lookup(TTreeHeaderCtrl::EColumn eCol);
static TTreeHeaderCtrl::EColumn THCMap_Lookup(TCHAR c);
static int TreeHdrCtrl_SaveWidthSettings (TRegUI* pUI, int * riWidth);
static void TreeHdrCtrl_SetWidths (CString& strWidths, int * riWidth);

/////////////////////////////////////////////////////////////////////////////
// TTreeHeaderCtrl

TTreeHeaderCtrl::TTreeHeaderCtrl()
{
	m_eSortColumn = TTreeHeaderCtrl::kThread;
	m_fSortAscend = TRUE;
	m_imgList.Create(IDB_THREAD_GIZMO, 10, 1, RGB(255,255,255));

	ZeroMemory (m_vWidth, sizeof m_vWidth);
}

TTreeHeaderCtrl::~TTreeHeaderCtrl()
{
}

BEGIN_MESSAGE_MAP(TTreeHeaderCtrl, CHeaderCtrl)
	ON_NOTIFY_REFLECT(HDN_ITEMCHANGED, OnItemChange)
	ON_NOTIFY_REFLECT(HDN_ITEMCLICK,   OnItemClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TTreeHeaderCtrl message handlers

///////////////////////////////////////////////////////////////////////////
//  This has two parts:  setup the widths, and then create the Header Fields
//
int TTreeHeaderCtrl::LoadSettings(int ParentWidth)
{
	SetFont (gpHeaderCtrlFont, FALSE);
	SetImageList(&m_imgList);

	CString data;
	CString columns;

	TRegUI* pRegUI = gpGlobalOptions->GetRegUI();

	pRegUI->LoadTreehdrCtrlWidths(data);
	pRegUI->GetTreeColumnsRoles( columns );

	if (data.IsEmpty())
	{
		// Thread icon, From, Indicators, Subject, Lines, Date, Score
		m_template = "TFISLDR";
		set_widths_profile( ParentWidth );
	}
	else
	{
		build_template( columns, m_template );
		set_widths_registry ( data );
		// RLW - force the thread column to be a fixed width
		m_vWidth[0] = THREAD_ICON_COL_WIDTH;
	}

	add_virtual_column(data.IsEmpty(), columns);

	template_insert();

	CString sortData;
	pRegUI->GetTreePaneSort(sortData, "TDA");

	m_eSortColumn = THCMap_Lookup(sortData[0]);

	m_ePrevSortColumn = THCMap_Lookup(sortData[1]);
	m_fSortAscend = ('A' == sortData[2]);

	UpdateThreadedIcon();

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// User may have added/deleted columns or changed their order.  I just
//   average the widths.
int TTreeHeaderCtrl::ResetSettings(int ParentWidth)
{
	CString columns;

	TLockDraw  sLock (this, TRUE);

	int nColCount = GetItemCount();

	CWnd *pDad = GetParent();
	CListCtrl *pLC = (CListCtrl*) pDad;

	for (int i = nColCount -1 ; i >= 0; i--)
		pLC->DeleteColumn (i);

	while (GetItemCount() > 0)
		DeleteItem (0);

	TRegUI *pRegUI = gpGlobalOptions->GetRegUI();

	pRegUI->GetTreeColumnsRoles(columns);

	build_template(columns, m_template);
	set_widths_average(ParentWidth);

	add_virtual_column(FALSE, columns);

	template_insert();

	// RLW : LART the zoomed widths cause they'll be wrong now
	pRegUI->SaveTreehdrCtrlZoomWidths("");

	return 0;
}

// ------------------------------------------------------------------------
//
void TTreeHeaderCtrl::add_virtual_column(BOOL fUseDefault, CString& columns)
{
	if (m_template[0] != 'T')
	{
		TRegUI* pRegUI = gpGlobalOptions->GetRegUI();

		// version 1.01 has a virtual Thread Column
		CString tmp = m_template;
		m_template = 'T' + tmp;

		// shrink - everyone contributes space
		int iDelta = 0;
		int iRequired;
		BOOL fChangeInLoop;
		do
		{
			fChangeInLoop = FALSE;
			for (int i = 0; i < 6 && iDelta < THREAD_ICON_COL_WIDTH; i++)
			{
				iRequired = THREAD_ICON_COL_WIDTH - iDelta;
				if (iRequired > 5)
					iRequired = 5;
				if (m_vWidth[i] > 100)
				{
					// steal upto 5 pixels
					m_vWidth[i] -= iRequired;
					iDelta += iRequired;
					fChangeInLoop = TRUE;
				}
				else if (m_vWidth[i] > 10)
				{
					--m_vWidth[i];
					++iDelta;
					fChangeInLoop = TRUE;
				}
			}
			// fChange indicates the for loop accomplished something
		} while (fChangeInLoop && (iDelta < THREAD_ICON_COL_WIDTH));

		// shift down
		MoveMemory(&m_vWidth[1], &m_vWidth[0], 6*sizeof(int));
		m_vWidth[0] = THREAD_ICON_COL_WIDTH;

		// write out new widths, after conversion
		SaveWidthSettings( false );

		if (fUseDefault)
			columns = "yTyFyIySyLyDyR";
		else
			columns = "yT" + columns;

		pRegUI->SetTreeColumnsRoles( columns );
	}
}

//-------------------------------------------------------------------------
// called during WM_DESTROY
int TTreeHeaderCtrl::SaveSettings( bool fZoomed)
{
	SaveWidthSettings(fZoomed);
	return SaveSortSettings();
}

//-------------------------------------------------------------------------
// Saves widths. Unzoomed only if fZoomed is false, both sets if fZoomed is true
int TTreeHeaderCtrl::SaveWidthSettings(bool fZoomed)
{
	if (fZoomed)
		return SaveZoomedWidthSettings ();
	return TreeHdrCtrl_SaveWidthSettings (gpGlobalOptions->GetRegUI(), m_vWidth);
}

//-------------------------------------------------------------------------
// Zoomed widths. Calculate and save
int TTreeHeaderCtrl::SaveZoomedWidthSettings ()
{
	int i, tot = GetItemCount ();
	if (tot > 0)
	{
		CString strAllWidths, strNum;
		HD_ITEM hd;
		for (hd.mask = HDI_WIDTH, i = 0; i < tot; i++)
		{
			hd.cxy  = 0;
			GetItem (i, &hd);
			strNum.Format ("%d ", hd.cxy);
			strAllWidths += strNum;
		}

		TRegUI * pUI = gpGlobalOptions->GetRegUI();

		// save off zoomed widths
		pUI->SaveTreehdrCtrlZoomWidths(strAllWidths);
	}
	return 0;
}

// Unzoomed widths.
static int TreeHdrCtrl_SaveWidthSettings (TRegUI* pUI, int * riWidth)
{
	CString final;

	final.Format("%d %d %d %d %d %d %d", riWidth[0],  riWidth[1], riWidth[2],
		riWidth[3],  riWidth[4], riWidth[5],
		riWidth[6]);

	pUI->SaveTreehdrCtrlWidths(final);

	return 0;
}

//-------------------------------------------------------------------------
// class-wide function.  Called from InitInstance().  Tweak data in registry
int TTreeHeaderCtrl::Upgrade (int iCurBuild, TRegUI * pRegUI)
{
	if (iCurBuild <= 500)
	{
		CString strWidths;

		pRegUI->LoadTreehdrCtrlWidths(strWidths);
		if (strWidths.IsEmpty())
			return 0;

		CString columns;
		pRegUI->GetTreeColumnsRoles( columns );

		// columns is a string -   yTyFyIySyLyD   the 'y' indicates Enabled.

		// utility c-function eliminates the columns that are Disabled
		CString strTemplate;
		build_template(columns, strTemplate);

		// template contains the Active columns

		int vWidth[7]; ZeroMemory(vWidth, sizeof vWidth);

		// c-function
		TreeHdrCtrl_SetWidths (strWidths, vWidth);

		// NOTE: in 1.10 we added the Watch Indicator. So the indicator
		//   column should be a little wider...

		BOOL fDirty = FALSE;
		for (int i = 0; i < 7 && i < strTemplate.GetLength(); ++i)
		{
			// find the correct column

			if ('I' == strTemplate[i])
			{
				if (vWidth[i] < kiIndicatorColCX)
				{
					vWidth[i] = kiIndicatorColCX;
					fDirty = TRUE;
				}
				break;
			}
		}

		if (fDirty)
		{
			TreeHdrCtrl_SaveWidthSettings ( pRegUI, vWidth );
		}
	}

	return 0;
}

//-------------------------------------------------------------------------
int TTreeHeaderCtrl::SaveSortSettings()
{
	CString data;
	TCHAR c = THCMap_Lookup(m_eSortColumn);
	data += c;
	c = THCMap_Lookup(m_ePrevSortColumn);
	data += c;
	data += m_fSortAscend ? 'A' : 'D';

	gpGlobalOptions->GetRegUI()->SetTreePaneSort(data);
	return 0;
}

// ------------------------------------------------------------------------
//
int TTreeHeaderCtrl::insert_item(int StrID, int width, DWORD moreflags, LPARAM* plAppData)
{
	HD_ITEM hdi;
	ZeroMemory(&hdi, sizeof(hdi));

	UINT style = 0;
	CString s;
	if (StrID)
	{
		style = HDI_TEXT;
		s.LoadString (StrID);
		hdi.pszText = (LPTSTR) (LPCTSTR) s;
		hdi.iImage = 3;
	}
	else
	{
		if ((moreflags & HDF_IMAGE) != 0)
		{
			style = HDI_IMAGE;
			hdi.iImage = 3;
		}
		else
		{
			style = HDI_TEXT;
			hdi.pszText = (LPTSTR) "";
		}
	}

	hdi.mask    = style | HDI_FORMAT | HDI_WIDTH | HDI_LPARAM;
	hdi.cxy     = width;
	if (style & HDI_TEXT)
		hdi.fmt = HDF_CENTER | HDF_STRING ;
	else
		hdi.fmt = HDF_CENTER | HDF_BITMAP;
	hdi.fmt |= moreflags;

	if (0==plAppData)
		hdi.lParam  = 0;
	else
	{
		hdi.mask |= HDI_LPARAM;
		hdi.lParam = *plAppData;
	}

	int idxNew = GetItemCount();

	if (true)
	{
		CWnd * pDad = GetParent();
		CListCtrl * pLC = (CListCtrl*) pDad;

		pLC->InsertColumn (idxNew, "", LVCFMT_LEFT, width);
	}

	BOOL fRet  = SetItem (idxNew, &hdi);

	return fRet;
}

// ------------------------------------------------------------------------
int TTreeHeaderCtrl::GetWidthArray(int n, int* pWidths)
{
	ZeroMemory(pWidths, n*sizeof(int));
	int tot = sizeof(m_vWidth) / sizeof(m_vWidth[0]);
	ASSERT(n == tot);
	for (int i = 0; i < tot; i++)
		*pWidths++ = m_vWidth[i];
	return 0;
}

// ------------------------------------------------------------------------
// handles WM_NOTIFY sent to our parent.  It is reflected down to us
// HDN_ITEMCHANGED
afx_msg void TTreeHeaderCtrl::OnItemChange(NMHDR* pNMHdr, LRESULT* pResult)
{
	HD_NOTIFY* pHDN = (HD_NOTIFY*) pNMHdr;
	if (pHDN->pitem->mask & HDI_WIDTH)
	{
		ASSERT(pHDN->iItem >= 0);
		ASSERT(pHDN->iItem < (sizeof(m_vWidth)/sizeof(m_vWidth[0])));
		// keep array of widths up to score
		m_vWidth[pHDN->iItem] = pHDN->pitem->cxy;

		// Hard reset to minimum width
		if (pHDN->pitem->cxy < giMinimumHdrCtrlSectionWidth)
		{
			HD_ITEM sItem; ZeroMemory(&sItem, sizeof sItem);
			sItem.mask = HDI_WIDTH;
			sItem.cxy  = giMinimumHdrCtrlSectionWidth;
			VERIFY(SetItem (pHDN->iItem, &sItem));
			m_vWidth[pHDN->iItem] = giMinimumHdrCtrlSectionWidth;
		}
		// Do not allow column 0 to change width
		if ((pHDN->iItem == 0) && (pHDN->pitem->cxy != THREAD_ICON_COL_WIDTH))
		{
			HD_ITEM sItem; ZeroMemory(&sItem, sizeof sItem);
			sItem.mask = HDI_WIDTH;
			sItem.cxy  = THREAD_ICON_COL_WIDTH;
			VERIFY(SetItem (pHDN->iItem, &sItem));
			m_vWidth[pHDN->iItem] = THREAD_ICON_COL_WIDTH;
		}

		CWnd * pDad = GetParent();
		if (pDad)
			pDad->Invalidate ();     // listctrl must redraw all
	}
}

// -------------------------------------------------------------------------
// handles WM_NOTIFY sent to our parent.  HDN_ITEMCLICK is reflected back
// down to us
afx_msg void TTreeHeaderCtrl::OnItemClick(NMHDR* pNMHdr, LRESULT* pResult)
{
	HD_NOTIFY* pHDN = reinterpret_cast<HD_NOTIFY*>(pNMHdr);

	// get it again to fetch the AppData
	HD_ITEM hdi;
	ZeroMemory(&hdi, sizeof hdi);
	hdi.mask = HDI_LPARAM;
	GetItem(pHDN->iItem, &hdi);
	TTreeHeaderCtrl::EColumn eCol = (TTreeHeaderCtrl::EColumn) hdi.lParam;

	column_change ( eCol, NULL );

	UpdateThreadedIcon();
}

//-------------------------------------------------------------------------
//
// pfAscend is NULL if change comes from a CHeaderCtrl click. If change
// comes from a dialog box, pfAscend is a BOOL*
void TTreeHeaderCtrl::column_change (TTreeHeaderCtrl::EColumn eCol,
									 BOOL* pfAscend)
{
	BOOL fForceRedrawNew = TRUE;

	if (kThread == eCol)
	{
		TTreeHeaderCtrl::EColumn eLameDuck =
			set_column (eCol, true, true /* allow thread toggle */);

		// go from thread to a column
		DispatchToParent(eLameDuck,
			m_eSortColumn, IsThreaded(), m_fSortAscend);

		fForceRedrawNew = FALSE;
	}
	else
	{
		BOOL fOldSort = m_fSortAscend;

		// change originates from dialog box
		if (pfAscend)
			m_fSortAscend = BYTE(*pfAscend);

		if (m_eSortColumn != eCol)
		{
			TTreeHeaderCtrl::EColumn eLameDuck = set_column (eCol, true, true);

			// give parent a before/after look
			DispatchToParent(eLameDuck, m_eSortColumn, fOldSort, m_fSortAscend);
		}
		else
		{
			if (NULL == pfAscend)
			{
				// switch sort ascending
				m_fSortAscend = !m_fSortAscend;
			}

			// give parent a before/after look
			DispatchToParent(m_eSortColumn, m_eSortColumn,
				fOldSort, m_fSortAscend);
		}
	}

	if (pfAscend && fForceRedrawNew)
	{
		// there is no click on the "new" column to invalidate it
		// redraw this dlg-based change by hand
		invalidate_column ( m_eSortColumn );
	}
}

// ------------------------------------------------------------------------
TTreeHeaderCtrl::EColumn TTreeHeaderCtrl::set_column (
	EColumn eNewCol, bool draw, bool fAllowThreadToggle)
{
	TTreeHeaderCtrl::EColumn eLameDuck = m_eSortColumn;

	if (kThread == eNewCol)
	{
		if (false == fAllowThreadToggle)
		{
			m_eSortColumn = kThread;
			if (eLameDuck != kThread)
				m_ePrevSortColumn = eLameDuck;
		}
		else
		{
			if (eLameDuck == kThread)
			{
				// leaving Threaded mode
				m_eSortColumn = GetPrevCol ();
			}
			else
			{
				// entering Threaded mode
				m_ePrevSortColumn = eLameDuck;  // save a history of prev flat sort
				m_eSortColumn = kThread;
			}
		}
	}
	else
	{
		m_ePrevSortColumn = eLameDuck;
		m_eSortColumn = eNewCol;
	}

	if (draw)
	{
		invalidate_column (m_eSortColumn);  // new guy
		invalidate_column (eLameDuck);
	}
	return eLameDuck;
}

// ------------------------------------------------------------------------
void TTreeHeaderCtrl::SetSortColumn (TTreeHeaderCtrl::EColumn eCol)
{
	// If newsgroup-A has associated Filter (ShowAll,Threaded)
	//  and user switches to newsgroup-B with
	//  associated Filter (ShowUnread, Threaded) we want to stay in
	//  threaded mode.

	// The tree header control, on the other hand, starts in threaded
	//  mode. If you click on the thread column it Reverts back to some
	//  previous flat-mode.
	set_column (eCol, false, false /* fAllowThreadToggle */);

	UpdateThreadedIcon();
}

// ------------------------------------------------------------------------
int TTreeHeaderCtrl::GetRightMost ()
{
	int n = GetRightMargin (m_template.GetLength() - 1);
	return n;
}

// ------------------------------------------------------------------------
int TTreeHeaderCtrl::GetRightMargin(int iItem)
{
	if (iItem < 0)
		return 0;
	int total;
	for (total = 0;iItem >= 0; iItem--)
		total += m_vWidth[iItem];
	return total;
}

void TTreeHeaderCtrl::set_widths_profile(int ParentWidth)
{
	int iFrom, iIndic, iSubject, iLines, iDate, iScore;

	iFrom    = ParentWidth * 25 / 100;
	iIndic   = ParentWidth * 10 / 100;
	iSubject = ParentWidth * 35 / 100;
	iLines   = ParentWidth *  7 / 100;
	iDate    = ParentWidth * 10 / 100;
	iScore   = ParentWidth - (iFrom + iIndic + iSubject + iLines + iDate);
	int avgChar = LOWORD(GetDialogBaseUnits());

	m_vWidth[0] = max(iFrom,    10*avgChar);
	// note each column seems to clip the image somewhat
	m_vWidth[1] = max(iIndic,  kiIndicatorColCX);
	m_vWidth[2] = max(iSubject, 15*avgChar);
	m_vWidth[3] = max(iLines,   3*avgChar);
	m_vWidth[4] = max(iDate,    10*avgChar);
	m_vWidth[5] = max(iScore,   4*avgChar);

	// subtract off a scrollbar's worth if we can
	avgChar = GetSystemMetrics(SM_CXVSCROLL);
	if (iScore > avgChar)
		m_vWidth[5] = iScore - avgChar;
}

void TTreeHeaderCtrl::set_widths_average(int ParentWidth)
{
	int tot = m_template.GetLength();
	// fill out missing columns-widths at the end of the array
	int i = 0;
	for (i = 0; i < 6; ++i)
		m_vWidth[i] = 100;

	int avgChar = LOWORD(GetDialogBaseUnits());

	for (i = 0; i < tot; ++i)
		m_vWidth[i] = ParentWidth/tot;

	if (tot > 1)
	{
		int iIndicatorIdx = m_template.Find('I');

		if ((iIndicatorIdx >= 0) && (m_vWidth[0] > kiIndicatorColCX))
		{
			for (i = 0; i < tot-1; ++i)
			{
				if (i == iIndicatorIdx)
					m_vWidth[i] = kiIndicatorColCX;
				else
					m_vWidth[i] = (ParentWidth-kiIndicatorColCX)/(tot-1);
			}
		}
	}
}

// member function
void TTreeHeaderCtrl::set_widths_registry(CString& strWidths)
{
	TreeHdrCtrl_SetWidths ( strWidths, m_vWidth );
}

// c-function
static void TreeHdrCtrl_SetWidths (CString& strWidths, int * riWidth)
{
	istrstream iss ((LPTSTR) (LPCTSTR) strWidths);
	iss >> riWidth[0] >> riWidth[1] >> riWidth[2]
	>> riWidth[3] >> riWidth[4] >> riWidth[5] >> riWidth[6];
}

void TTreeHeaderCtrl::template_insert()
{
	LPARAM lAppData;

	for (int i = 0; i < m_template.GetLength(); ++i)
	{
		TCHAR datum = m_template[i];
		switch (datum)
		{
		case 'T':
			lAppData = (LPARAM) kThread;

			// 1-StrID, 2-width, 3-styles, 4-lParam
			insert_item(0, m_vWidth[i], HDF_IMAGE, &lAppData);
			break;

		case 'F':
			lAppData = (LPARAM) kFrom;
			insert_item(IDS_HDRCTRL_FROM, m_vWidth[i], HDF_STRING, &lAppData);
			break;

		case 'I':
			lAppData = (LPARAM) kIndicator;
			insert_item(IDS_HDRCTRL_STATUS, m_vWidth[i], HDF_STRING, &lAppData);
			break;

		case 'S':
			lAppData = (LPARAM) kSubject;
			insert_item(IDS_HDRCTRL_SUBJECT, m_vWidth[i], HDF_STRING, &lAppData);
			break;

		case 'L':
			lAppData = (LPARAM) kLines;
			insert_item(IDS_HDRCTRL_LINES, m_vWidth[i], HDF_RIGHT | HDF_STRING, &lAppData);
			break;

		case 'D':
			lAppData = (LPARAM) kDate;
			insert_item(IDS_HDRCTRL_DATE, m_vWidth[i], HDF_STRING, &lAppData);
			break;

		case 'R':
			lAppData = (LPARAM) kScore;
			insert_item(IDS_HDRCTRL_SCORE, m_vWidth[i], HDF_STRING, &lAppData);
			break;

		default:
			ASSERT(0);
			break;
		}
	}
}

void TTreeHeaderCtrl::build_template(CString& columns, CString& strTemplate)
{
	strTemplate.Empty();

	for (int i = 0; i < columns.GetLength(); i+=2)
	{
		if ('y' == columns[i]) // If this column is enabled
			if (strTemplate.Find(columns[i+1]) == -1) // And its not a duplicate
				strTemplate += columns[i+1]; // Add it to the template
	}
}

//-------------------------------------------------------------------------
void TTreeHeaderCtrl::DispatchToParent(TTreeHeaderCtrl::EColumn eOldCol,
									   TTreeHeaderCtrl::EColumn eNewCol,
									   BOOL fOldSortA, BOOL fSortA)
{
	m_pParentThreadView->ResetContent(eOldCol, eNewCol, fOldSortA, fSortA);
}

// -------------------------------------------------------------------------
static TCHAR THCMap_Lookup(TTreeHeaderCtrl::EColumn eCol)
{
	int tot = sizeof(vTHCMap) / sizeof(vTHCMap[0]);
	for (int i = 0; i < tot; ++i)
		if (vTHCMap[i].eCol == eCol)
			return vTHCMap[i].c;

	ASSERT(0);
	return '\0';
}

// -------------------------------------------------------------------------
static TTreeHeaderCtrl::EColumn THCMap_Lookup(TCHAR c)
{
	int tot = sizeof(vTHCMap) / sizeof(vTHCMap[0]);
	for (int i = 0; i < tot; ++i)
		if (vTHCMap[i].c == c)
			return vTHCMap[i].eCol;

	ASSERT(0);
	return TTreeHeaderCtrl::kFrom;
}

// -------------------------------------------------------------------------
// makes sure that the enum we return is in fact still in the m_template string
//
TTreeHeaderCtrl::EColumn TTreeHeaderCtrl::GetPrevCol()
{
	TCHAR c = THCMap_Lookup(m_ePrevSortColumn);
	if (-1 != m_template.Find(c))
		return m_ePrevSortColumn;

	// use the column next to the special virtual column
	c = m_template[1];
	return THCMap_Lookup(c);
}

// -------------------------------------------------------------------------
void TTreeHeaderCtrl::invalidate_column(TTreeHeaderCtrl::EColumn eCol)
{
	TCHAR c = THCMap_Lookup(eCol);
	int   idx = m_template.Find(c);
	if (-1 != idx)
	{
		CRect rct;  GetClientRect(&rct);
		for (int i = 0; i < idx; ++i)
			rct.left += m_vWidth[i];
		rct.right = rct.left + m_vWidth[idx];
		InvalidateRect ( &rct );
	}
}

// -------------------------------------------------------------------------
void TTreeHeaderCtrl::OnNewsgroupSort(CWnd* pParent)
{
	TSortArticlesDlg dlgSort(pParent);

	BOOL fPrevSort;
	int  iPrevIdx;

	dlgSort.m_strInputTemplate = m_template;

	// index into the string, indicates the current sort
	TCHAR cKey = THCMap_Lookup(m_eSortColumn);
	dlgSort.m_idxTemplate = dlgSort.m_strInputTemplate.Find(cKey);

	// if we are sorting by a field whose column is not displayed,
	//   iPrevIdx can be -1
	fPrevSort = dlgSort.m_fSortAscending = m_fSortAscend;
	iPrevIdx  = dlgSort.m_idxTemplate;

	if (IDOK == dlgSort.DoModal())
	{
		if ((fPrevSort == dlgSort.m_fSortAscending) &&
			(iPrevIdx  == dlgSort.m_idxTemplate))
		{
			// do nothing
			return;
		}
		else if (('T' == dlgSort.m_strInputTemplate[dlgSort.m_idxTemplate]) &&
			(iPrevIdx  == dlgSort.m_idxTemplate) &&
			(fPrevSort != dlgSort.m_fSortAscending))
		{
			// thread sort  has  changed direction
			m_fSortAscend = dlgSort.m_fSortAscending;

			// will update thread sort , and refill pane
			m_pParentThreadView->RefillTree (false);
		}
		else
		{
			cKey = dlgSort.m_strInputTemplate[dlgSort.m_idxTemplate];
			column_change ( THCMap_Lookup(cKey), &dlgSort.m_fSortAscending );
		}
	}
}

// ------------------------------------------------------------------------
//
void TTreeHeaderCtrl::AdjustForSmallerWidth ()
{
	fnUtilHdrProportionalWidth (m_template.GetLength(), m_vWidth, 8);
}

// ------------------------------------------------------------------------
void TTreeHeaderCtrl::ApplyWidthsFromString (CString & strWidths)
{
	if (strWidths.IsEmpty())
		return;

	int tot = GetItemCount ();
	istrstream iss ((LPTSTR) (LPCTSTR) strWidths);

	for (int i = 0; i < tot; i++)
	{
		HDITEM hd;

		hd.mask = HDI_WIDTH;
		hd.cxy = 20;
		iss >> hd.cxy;

		SetItem (i, &hd);
	}
}

// ------------------------------------------------------------------------
//
void TTreeHeaderCtrl::OnZoom(bool fZoom)
{
	CString strWidths;
	TRegUI* pRegUI = gpGlobalOptions->GetRegUI();

	if (fZoom)
	{
		// The roles are the same, but the widths can be different

		// save off Normal widths
		TreeHdrCtrl_SaveWidthSettings(pRegUI, m_vWidth);

		pRegUI->LoadTreehdrCtrlZoomWidths(strWidths);

		// auto redraw object
		TLockDraw sDraw(GetParent(), TRUE);

		ApplyWidthsFromString(strWidths);
	}
	else
	{
		if (GetItemCount() > 0)
		{
			//  we are coming back to Normal
			SaveZoomedWidthSettings();

			// load normal
			pRegUI->LoadTreehdrCtrlWidths(strWidths);

			// re-apply normal

			// auto redraw object
			TLockDraw sDraw(GetParent(), TRUE);

			ApplyWidthsFromString(strWidths);
		}
	}
}

void TTreeHeaderCtrl::fnUtilHdrProportionalWidth(int nCol, int * piCX, int iTooSmall)
{
	ASSERT(nCol > 0);

	CRect rct; GetClientRect (&rct);
	int CX = rct.Width();
	int nOldCX = 0;
	int i;
	for (i = 0; i < nCol; ++i)
		nOldCX += piCX[i];

	BOOL fTooSmall = FALSE;

	int * pNewCX = new int[nCol];
	// proportional to the old
	for (i = nCol - 1; i >= 0; --i)
	{
		pNewCX[i] = CX * piCX[i] / nOldCX;
		if (pNewCX[i] <= iTooSmall)
		{
			fTooSmall = TRUE;
			break;
		}
	}

	if (fTooSmall)
	{
		for (i = nCol - 1; i >= 0; --i)
		{
			pNewCX[i] = CX / nCol;  // evenly divided
		}
	}

	for (i = nCol - 1; i >= 0; --i)
	{
		HD_ITEM sHD; ZeroMemory(&sHD, sizeof sHD);

		sHD.mask = HDI_WIDTH;
		sHD.cxy  = pNewCX[i];
		SetItem (i, &sHD);
	}

	delete [] pNewCX;
}

void TTreeHeaderCtrl::UpdateThreadedIcon()
{
	// If threaded, change icon to point down
	// Otherwise, change icon to point to right
	HD_ITEM hdi;
	ZeroMemory(&hdi, sizeof(hdi));
	hdi.iImage = 1;

	if (m_eSortColumn != kThread)
		hdi.iImage = 0;

	// First column is the threaded column
	hdi.mask = HDI_IMAGE;// | HDI_FORMAT;
//	hdi.fmt = HDF_IMAGE | HDF_LEFT;
	SetItem (0, &hdi);

	// Also if not threaded, set the sort "up" or "down" icon on the relevant sort column
	int iColumn = 1;
	ZeroMemory(&hdi, sizeof(hdi));
	hdi.mask = HDI_FORMAT;

	if (m_eSortColumn == kThread)
	{
		// Threaded. Turn OFF sorting on the last sort column
		iColumn = m_template.Find(THCMap_Lookup(m_ePrevSortColumn));
		if (-1 != iColumn)
		{
			GetItem(iColumn, &hdi);
			hdi.fmt = hdi.fmt & (~(HDF_SORTDOWN|HDF_SORTUP));
			SetItem(iColumn, &hdi);
		}
	}
	else
	{
		// Not threaded.
		// First hide the icon in the previous sort column.
		iColumn = m_template.Find(THCMap_Lookup(m_ePrevSortColumn));
		if (-1 != iColumn)
		{
			GetItem(iColumn, &hdi);
			hdi.fmt = hdi.fmt & (~(HDF_SORTDOWN|HDF_SORTUP));
			SetItem(iColumn, &hdi);
		}

		// Then set the icon according to the sort ascending flag on the current sort column
		iColumn = m_template.Find(THCMap_Lookup(m_eSortColumn));
		if (-1 != iColumn)
		{
			GetItem(iColumn, &hdi);
			hdi.fmt = hdi.fmt & (~(HDF_SORTDOWN|HDF_SORTUP));
			if (m_fSortAscend)
				hdi.fmt = hdi.fmt | HDF_SORTUP;
			else
				hdi.fmt = hdi.fmt | HDF_SORTDOWN;
			SetItem(iColumn, &hdi);
		}
	}
}
