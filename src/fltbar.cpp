/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fltbar.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
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

// fltbar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "fltbar.h"
//#include "tglobopt.h"         // gpGlobalOptions
#include "vfltdlg.h"          // TViewFiltersDlg
#include "vfilter.h"          // TAllViewFilter
#include "newsview.h"         // ->RefreshCurrentNewsgroup
#include "thrdlvw.h"          // ->SortBy()
#include "uimem.h"            // gpUIMemory
#include "nglist.h"           // TNewsGroupUseLock
#include "genutil.h"          // GetThreadView()
#include "servcp.h"           // TServerCountedPtr
#include "utilsize.h"
#include "globals.h"
#include "newsdb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TOOLTIP_TIMER  13

// prototypes
static void selectSort (TViewFilter * pVF);

/////////////////////////////////////////////////////////////////////////////
// TViewFilterBar dialog
TViewFilterBar::TViewFilterBar()
	: CDialogBar()
{
	m_pToolTip = NULL;
	m_iTimerId = 0;
	m_fIsShowingAll = false;
}

TViewFilterBar::~TViewFilterBar()
{
	delete m_pToolTip;
}

/////////////////////////////////////////////////////////////////////////////
void TViewFilterBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VIEWFILTER_CFG, m_btnEdit);
	DDX_Control(pDX, IDC_VIEWFILTER_COMBO, m_combo);

	CString name;

	if (!pDX->m_bSaveAndValidate)
	{
		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();

		// store filter ID into itemdata
		pAllFilters->FillCombo (&m_combo, TRUE /* fSetIDinData */);

		bool fSelectionSet = false;
		int iRecentFilterID = gpUIMemory->GetViewFilterID();

		if (0 == selectString(iRecentFilterID, false))
			fSelectionSet = true;

		if (false == fSelectionSet)
		{
			name = gpUIMemory->Deprecated_GetViewFilterName();
			if (CB_ERR == m_combo.SelectString ( -1, name ))
				m_combo.SetCurSel (0);
		}

		int iSel = m_combo.GetCurSel ();
		if (CB_ERR != iSel)
		{
			TViewFilter * pVF = pAllFilters->GetByID (m_combo.GetItemData(iSel));
			if (pVF)
				m_fIsShowingAll = (pVF->IsViewAll()) ? true : false;
		}
		else
		{
			CString strAll; strAll.LoadString (IDS_FLTNAME_ALL);
			if (strAll == name)
				m_fIsShowingAll = true;
		}
	}
}

BEGIN_MESSAGE_MAP(TViewFilterBar, CDialogBar)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_VIEWFILTER_COMBO, OnSelchangeViewfilterCombo)
	ON_BN_CLICKED(IDC_VIEWFILTER_CFG, OnViewfilterCfg)
	ON_UPDATE_COMMAND_UI(IDC_VIEWFILTER_CFG, OnUpdateConfigButton)
	ON_BN_CLICKED(IDC_VIEWFILTER_TAKE, OnMakeDefault)
	ON_UPDATE_COMMAND_UI(IDC_VIEWFILTER_TAKE, OnUpdateDefaultButton)
	ON_BN_CLICKED(IDC_PUSHPIN,  OnBothPushpinAction)
	ON_UPDATE_COMMAND_UI(IDC_PUSHPIN,         OnUpdateDefaultButton)
	ON_BN_CLICKED(IDC_PUSHPIN2, OnBothPushpinAction)
	ON_UPDATE_COMMAND_UI(IDC_PUSHPIN2,        OnUpdateDefaultButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TViewFilterBar message handlers

// ------------------------------------------------------------------------
// Handles the Button
void TViewFilterBar::OnViewfilterCfg()
{
	// call public member function
	DefineFilters ();
}

void TViewFilterBar::OnSelchangeViewfilterCombo()
{
	int iSel = m_combo.GetCurSel();
	if (CB_ERR != iSel)
	{
		int iFilterID  =  m_combo.GetItemData (iSel);

		gpUIMemory->SetViewFilterID (iFilterID);

		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
		TViewFilter * pVF = pAllFilters->GetByID (iFilterID);

		if (pVF)
		{
			selectSort ( pVF );

			m_fIsShowingAll = (pVF->IsViewAll()) ? true : false;
		}
		GetNewsView()->RefreshCurrentNewsgroup ();
	}
}

void TViewFilterBar::OnUpdateConfigButton(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (TRUE);
}

void TViewFilterBar::OnUpdateDefaultButton(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (TRUE);
}

// ------------------------------------------------------------------------
// OnMakeDefault -- make current filter the default filter for the
//    current newsgroup.
void TViewFilterBar::OnMakeDefault()
{
	int iSel = m_combo.GetCurSel();
	if (CB_ERR != iSel)
	{
		CString name;
		m_combo.GetLBText (iSel, name);

		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
		TViewFilter * pVF = pAllFilters->GetByName (name);
		CNewsView * pNV = GetNewsView();

		// pass in integer ID of the view filter
		if (pVF && pNV)
		{
			TServerCountedPtr cpNewsServer;
			LONG lGroupID = pNV->GetCurNewsGroupID ();
			BOOL fUseLock = FALSE;
			TNewsGroup* pNG = 0;

			// lock down newsgroup object
			TNewsGroupUseLock useLock(cpNewsServer, lGroupID, &fUseLock, pNG);

			if (fUseLock)
				pNG->SetFilterID ( pVF->getID() );
		}
	}
}

// ------------------------------------------------------------------------
// This is Pin1 and Pin2
void TViewFilterBar::OnBothPushpinAction ()
{
	// just a regular command msg   for newsview
	PostMessage(WM_COMMAND, ID_NEWSGROUP_PINFILTER);
}

void TViewFilterBar::OnDestroy()
{
	CDialogBar::OnDestroy();

	if (m_iTimerId)
		KillTimer (m_iTimerId);
}

//-----------------------------------------------------------------
// note: I find that initial startup dialogs (such as 'Do you want
//       to make Gravity your default newsreader') interfere with
//       Tooltips behaving correctly.
//
void TViewFilterBar::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (TOOLTIP_TIMER == nIDEvent)
	{
		KillTimer (m_iTimerId);
		m_iTimerId = 0;

		enableTooltips (true);
	}
	CDialogBar::OnTimer(nIDEvent);
}


BOOL TViewFilterBar::PreTranslateMessage(MSG* pMsg)
{
	if (m_pToolTip &&
		WM_MOUSEFIRST <= pMsg->message &&
		pMsg->message <= WM_MOUSELAST)
		m_pToolTip->RelayEvent (pMsg);

	return CDialogBar::PreTranslateMessage(pMsg);
}






void TViewFilterBar::Initialize()
{
	UpdateData (FALSE);

	// Steal his size & position
	CWnd * pWnd = GetDlgItem (IDC_VIEWFILTER_TAKE);

	CRect rct;
	Utility_GetPosParent(this, pWnd, rct);

	pWnd->DestroyWindow ();

	// image list used for Toolbar buttons 15x15
	m_ImageList.Create (IDB_PINLINK, 15, 1, RGB(255,0,255));

	rct.right = rct.left + 21;
	rct.bottom = rct.top + 20;

	// create the CBitmapButton
	VERIFY (m_Link.Create ("Link",
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW |
		BS_PUSHBUTTON, rct, this, IDC_VIEWFILTER_TAKE));

	VERIFY(m_Link.LoadBitmaps (IDB_LINKU, IDB_LINKD));

	m_Link.SetImageList ( m_ImageList, 2, 3 );

	CRect rctPin;
	rctPin.left = rct.right + 1;
	rctPin.top  = rct.top;
	rctPin.bottom = rct.bottom;
	rctPin.right = rctPin.left + 21;

	DWORD base = WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_VISIBLE;
	DWORD style;

	style = base;
	if (m_fPinned)
		style &= ~WS_VISIBLE;
	m_pin1.Create ("Pin",  style , rctPin, this, IDC_PUSHPIN);
	VERIFY(m_pin1.LoadBitmaps (IDB_PUSHPINU, IDB_PUSHPIND));
	m_pin1.SetImageList ( m_ImageList, 0, 0 );

	CRect rctPin2 = rctPin;

	style = base;
	if (!m_fPinned)
		style &= ~WS_VISIBLE;

	m_pin2.Create ("Pin2", style, rctPin2, this, IDC_PUSHPIN2);

	VERIFY(m_pin2.LoadBitmaps (IDB_PUSHPIND, IDB_PUSHPINU));
	m_pin2.SetImageList ( m_ImageList, 1, 1 );

	// wait before setting up tooltips
	m_iTimerId = SetTimer (TOOLTIP_TIMER, 3000, NULL);  // 3 seconds
}

// ------------------------------------------------------------------------
// Called from 'button' code and mainfrm.cpp
void TViewFilterBar::DefineFilters ()
{
	int iOldDefault;
	int iSel = m_combo.GetCurSel();
	int filterID = 0;
	if (CB_ERR == iSel)
		return;

	TViewFiltersDlg sDlg;

	// store id of previous filter
	filterID = (int) m_combo.GetItemData (iSel);

	TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
	iOldDefault = sDlg.m_iDefaultFilterID = pAllFilters->GetGlobalDefFilterID();

	// Invoke the dialog box
	enableTooltips (false);

	int iDlgRet = sDlg.DoModal ();

	enableTooltips (true);

	if (IDOK != iDlgRet)
		return;

	// any changes will be saved out during newsdb::Close
	if (iOldDefault != sDlg.m_iDefaultFilterID)
	{
		pAllFilters->SetGlobalDefFilterID (sDlg.m_iDefaultFilterID);
	}

	// refill combo box
	m_combo.ResetContent ();
	pAllFilters->FillCombo (&m_combo, TRUE);

	BOOL fDone = FALSE;

	// is the prev filter still here?
	if (0 == selectString (filterID, true))
		fDone = TRUE;

	if (!fDone)
	{
		// previous filter is gone! what can I do? Use 1st item
		m_combo.SetCurSel ( 0 );

		gpUIMemory->SetViewFilterID( (int) m_combo.GetItemData(0) );

		OnSelchangeViewfilterCombo ();
	}
}

// ------------------------------------------------------------------------
void TViewFilterBar::ChooseFilter (CWnd * pAnchor)
{
	int iDlgRet;
	TSelectDisplayFilterDlg sDlg(pAnchor);
	enableTooltips (false);

	iDlgRet = sDlg.DoModal ();

	enableTooltips (true);

	if (IDOK == iDlgRet)
	{
		selectString (sDlg.m_iFilterID, false);
	}
}

// ------------------------------------------------------------------------
int TViewFilterBar::ForceShowAll ()
{
	// NOTE:  pass in a negative number to Choose All Articles
	return   SelectFilter ( -999 );
}

// ------------------------------------------------------------------------
// switch combobox to the specified filter. Return 0 for success.
int TViewFilterBar::SelectFilter (int iFilterID)
{
	TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
	TViewFilter * pFilter = 0;

	if (iFilterID <= 0)
		pAllFilters->GetGenericFilterViewAll (pFilter);
	else
		pFilter = pAllFilters->GetByID (iFilterID);

	if (!pFilter)
		return 1;

	selectSort (pFilter);
	return selectString (pFilter->getID (), true);
}

// ------------------------------------------------------------------------
// Private
int TViewFilterBar::selectString (int iFilterID, bool fSetUIMemory)
{
	int ret = 1;
	for (int i = 0; i < m_combo.GetCount(); i++)
	{
		if (iFilterID == (int) m_combo.GetItemData(i))
		{
			m_combo.SetCurSel (i);
			ret = 0;
			break;
		}
	}

	if (fSetUIMemory && 0==ret)
		gpUIMemory->SetViewFilterID( iFilterID );

	return ret;
}

// ------------------------------------------------------------------------
void TViewFilterBar::UpdatePin (BOOL fShowPinned)
{
	// save and validate == FALSE

	HDWP hDF = BeginDeferWindowPos (2);
	if (!hDF)
		return ;

	if (fShowPinned)
	{
		// go to weird
		hDF = DeferWindowPos (hDF, m_pin2.m_hWnd, m_Link.m_hWnd, 0,0,0,0,
			SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		hDF = DeferWindowPos (hDF, m_pin1.m_hWnd, m_pin2.m_hWnd, 0,0,0,0,
			SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	}
	else
	{
		// go to normal
		hDF = DeferWindowPos (hDF, m_pin1.m_hWnd, m_Link.m_hWnd, 0,0,0,0,
			SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		hDF = DeferWindowPos (hDF, m_pin2.m_hWnd, m_pin1.m_hWnd, 0,0,0,0,
			SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	}

	EndDeferWindowPos (hDF);
}

// ------------------------------------------------------------------------
//
// I found that the dialog boxes interfered with the ToolTips.  So
//   I clean up,  show dlgbox, re-create tooltips.
//
void TViewFilterBar::enableTooltips (bool fOn)
{
	if (fOn)
	{
		// Setup the tooltip
		m_pToolTip = new CToolTipCtrl;
		if (m_pToolTip->Create(this))
		{
			if (!m_pToolTip->AddTool (&m_btnEdit, IDS_FLTBAR_TIP_MANAGE)   ||
				!m_pToolTip->AddTool (&m_combo,   IDS_FLTBAR_TIP_PICKFLT)  ||
				!m_pToolTip->AddTool (&m_Link,    IDS_FLTBAR_TIP_LINK)     ||
				!m_pToolTip->AddTool (&m_pin1,    IDS_FLTBAR_TIP_LOCK)     ||
				!m_pToolTip->AddTool (&m_pin2,    IDS_FLTBAR_TIP_UNLOCK) )
			{
				AfxMessageBox(IDS_FLTBAR_TERR1);
			}
		}
		else
		{
			AfxMessageBox(IDS_FLTBAR_TERR1);
		}
	}
	else
	{
		CToolTipCtrl * pDead = m_pToolTip;
		m_pToolTip = 0;
		delete pDead;
	}
}

//-----------------------------------------------------------------
void TViewFilterBar::ToggleFilterShowall (int iPreferredFilter)
{
	if (m_fIsShowingAll)
	{
		if (SelectFilter ( iPreferredFilter ))
			return;
		m_fIsShowingAll = false;
	}
	else
	{
		if (ForceShowAll ())
			return;
		m_fIsShowingAll = true;
	}

	GetNewsView()->RefreshCurrentNewsgroup ();
}







/////////////////////////////////////////////////////////////////////////////
// TSelectDisplayFilterDlg dialog

TSelectDisplayFilterDlg::TSelectDisplayFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TSelectDisplayFilterDlg::IDD, pParent)
{
}

void TSelectDisplayFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST1, m_lbx);

	if (!pDX->m_bSaveAndValidate)
	{
		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
		pAllFilters->FillLbx ( &m_lbx, true );

		int iID = gpUIMemory->GetViewFilterID();
		bool fSet = false;

		for (int i = 0; i < m_lbx.GetCount(); i++)
		{
			if (iID == (int) m_lbx.GetItemData(i))
			{
				m_lbx.SetCurSel (i);
				fSet = true;
				break;
			}
		}
		if (false == fSet)
			m_lbx.SetCurSel (0);
	}
}

BEGIN_MESSAGE_MAP(TSelectDisplayFilterDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSelectDisplayFilterDlg message handlers

void TSelectDisplayFilterDlg::OnOK()
{
	execute ();

	CDialog::OnOK();
}

void TSelectDisplayFilterDlg::OnDblclkList1()
{
	OnOK ();
}

void TSelectDisplayFilterDlg::execute ()
{
	int iSel = m_lbx.GetCurSel ();
	if (LB_ERR != iSel)
	{
		m_lbx.GetText ( iSel, m_newFilterName );

		int iFilterID = m_lbx.GetItemData (iSel);

		gpUIMemory->SetViewFilterID ( iFilterID );

		GetNewsView()->RefreshCurrentNewsgroup ();

		// pass id back to the FilterBar
		m_iFilterID = iFilterID;
	}
}

// ------------------------------------------------------------------------
// c-function.  Put settings into the header control.  Caller must
//   handle the redisplay
static void selectSort (TViewFilter * pVF)
{
	DWORD dwSortcode = pVF->SortCode();

	// Note the cast
	TTreeHeaderCtrl::EColumn eCol =
		(TTreeHeaderCtrl::EColumn) TViewFilter::SortColumn (dwSortcode);

	GetThreadView()->SortBy ( eCol, TViewFilter::SortAscending (dwSortcode));
}
