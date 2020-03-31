/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vfltdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:52:26  richard_wood
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

// vfltdlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "vfltdlg.h"

#include "tglobopt.h"
#include "vfilter.h"			      // TAllViewFilter
#include "vfiltad.h"			      // TViewFilterAddDlg
#include "tmsgbx.h"              // NewsMessageBox
#include "genutil.h"             // MsgResource ()
#include "globals.h"             // gpStore
#include "newsdb.h"              // use of TNewsDB

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
TViewFiltersDlg::TViewFiltersDlg(CWnd* pParent /*=NULL*/)
: CDialog(TViewFiltersDlg::IDD, pParent)
{
	VERIFY (m_imgList.Create (IDB_EYEGLASS, 15 /* frame width */,
		1, RGB(255, 0, 255)));
}

// ------------------------------------------------------------------------

void TViewFiltersDlg::OnDestroy()
{
	// for each item in the listbox, delete its associated filter object
	int iNum = m_list.GetItemCount ();
	for (int i = 0; i < iNum; i++) {
		TViewFilter *pFilter = GetpFilter (i);
		ASSERT (pFilter);
		delete pFilter;
	}

	CDialog::OnDestroy();
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_VIEWFILTER_LCX, m_list);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		// just 1 column. Just 1 fixed image of 'eyeglasses'
		CRect crctList;
		m_list.GetClientRect( &crctList );
		m_list.InsertColumn (0, "", LVCFMT_LEFT, crctList.Width() );
		m_list.SetImageList ( &m_imgList, LVSIL_SMALL );

		// the image is I_IMAGECALLBACK, see OnGetDisplayInfo()
		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();
		pAllFilters->FillList (&m_list, TRUE, TRUE);

		// setup the static text that shows what the global default filter is
		fill_default ();
	}
}

BEGIN_MESSAGE_MAP(TViewFiltersDlg, CDialog)
	ON_BN_CLICKED(IDC_VIEWFILTER_ADD, OnViewfilterAdd)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_VIEWFILTER_LCX, OnBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_VIEWFILTER_LCX, OnEndLabelEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWFILTER_LCX, OnItemChange)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_VIEWFILTER_LCX, OnGetDisplayInfo)
	ON_BN_CLICKED(IDC_VIEWFILTER_EDIT, OnViewfilterEdit)
	ON_BN_CLICKED(IDC_VIEWFILTER_DEL, OnViewfilterDel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_VIEWFILTER_MKDEF, OnMakeDefault)
	ON_BN_CLICKED(IDC_VIEWFILTER_COPY, OnViewfilterCopy)
END_MESSAGE_MAP()

// ------------------------------------------------------------------------
// OnBeginLabelEdit
void TViewFiltersDlg::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Zero means OK to edit
	*pResult = 0;

	CEdit * pEdit = m_list.GetEditControl ();

	// limit to 60 bytes
	if (pEdit)
		pEdit->SetLimitText ( 60 );
}

// ------------------------------------------------------------------------
// OnEndLabelEdit
void TViewFiltersDlg::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO * pDisp = reinterpret_cast<LV_DISPINFO*>(pNMHDR);

	// if user cancelled out the string ptr is null
	if (pDisp->item.pszText)
	{
		TRACE1("EDIT STRING IS %s\n", pDisp->item.pszText);
		m_list.SetItemText (pDisp->item.iItem, 0, pDisp->item.pszText);
	}
	*pResult = 0;
}

// ------------------------------------------------------------------------
//  We are only interested in the SelChange
void TViewFiltersDlg::OnItemChange (NMHDR* pNMHDR, LRESULT* pResult)
{
	// enable/disable the buttons
	UpdateGrayState ();
	*pResult = 0;
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::OnViewfilterEdit()
{
	int idx = m_list.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (idx < 0)
		return;

	// pass in the ListCtrl so we can check uniqueness
	TViewFilterAddDlg sDlg(&m_list, this);

	sDlg.m_fEdit = TRUE;

	sDlg.m_strName = m_list.GetItemText (idx, 0);
	TViewFilter *pFilter = GetpFilter (idx);
	ASSERT (pFilter);
	sDlg.SetDWordData ( pFilter->GetDWordData () );
	sDlg.m_rstrCondition.Copy (pFilter->m_rstrCondition);
	sDlg.m_dwSortCode = pFilter->SortCode ();
	sDlg.m_fShowEntireThread = pFilter->getShowThread();
	sDlg.m_fCompleteBinaries = pFilter->getCompleteBinaries();
	sDlg.m_fSkipThreads = pFilter->getNoThread();

	if (IDOK == sDlg.DoModal ())
	{
		m_list.SetItemText (idx, 0, sDlg.m_strName);
		pFilter->SetDWordData (sDlg.GetDWordData ());
		pFilter->m_rstrCondition.Copy (sDlg.m_rstrCondition);
		pFilter->SortCode (sDlg.m_dwSortCode);
		pFilter->setShowThread (sDlg.m_fShowEntireThread);
		pFilter->setCompleteBinaries (sDlg.m_fCompleteBinaries);
		pFilter->setNoThread (sDlg.m_fSkipThreads);
	}
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::OnViewfilterDel()
{
#ifdef LITE
	MsgResource (IDS_LITE_NO_FILTERS);
	return;
#endif

	int tot = m_list.GetItemCount();

	// you must not delete every filter
	if ((int) m_list.GetSelectedCount() == tot)
		return;

	int idx = m_list.GetItemCount() - 1;
	for (; idx >= 0; idx--)
	{
		// delete whatever is selected
		if (!m_list.GetItemState (idx, LVIS_SELECTED))
			continue;
		if (m_iDefaultFilterID == GetpFilter(idx)->getID())
		{
			// can't delete the default filter
			NewsMessageBox (this, IDS_NODEL_DEFVFILTER, MB_OK | MB_ICONSTOP);
		}
		else
			m_list.DeleteItem (idx);
	}
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::OnViewfilterAdd()
{
	// pass in the ListCtrl so we can check uniqueness
	TViewFilterAddDlg sDlg(&m_list, this);

	if (IDOK == sDlg.DoModal ())
	{
		int idx = m_list.GetItemCount();
		int iAt = m_list.InsertItem (idx, sDlg.m_strName, I_IMAGECALLBACK);

		// constructor will assign an id
		TViewFilter *pFilter =
			new TViewFilter (sDlg.m_strName,
			sDlg.GetDWordData (),
			sDlg.m_rstrCondition,
			sDlg.m_dwSortCode,
			sDlg.m_fShowEntireThread,
			sDlg.m_fCompleteBinaries ? true : false,
			sDlg.m_fSkipThreads);

		m_list.SetItemData (iAt, (DWORD) pFilter);
	}
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::OnOK()
{
	ASSERT(m_list.GetItemCount() > 0);

	TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();

	// get the data out of the CListCtrl
	pAllFilters->ReadFromList (&m_list);

	CDialog::OnOK();
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::UpdateGrayState ()
{
	UINT uSelCount = m_list.GetSelectedCount();
	GetDlgItem(IDC_VIEWFILTER_EDIT)->EnableWindow (1 == uSelCount);
	GetDlgItem(IDC_VIEWFILTER_COPY)->EnableWindow (1 == uSelCount);

	GetDlgItem(IDC_VIEWFILTER_DEL)->EnableWindow (
		(uSelCount > 0) && ((int)uSelCount < m_list.GetItemCount()) );
	GetDlgItem(IDC_VIEWFILTER_MKDEF)->EnableWindow (1 == uSelCount);
}

// ------------------------------------------------------------------------
BOOL TViewFiltersDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetFocus ();

	m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIF_STATE);
	UpdateGrayState ();

	return FALSE;  // return TRUE unless you set the focus to a control
}

// ------------------------------------------------------------------------
// fill_default -- fill the static text that shows what the global default
//    filter is.
void TViewFiltersDlg::fill_default ()
{
	// looking for m_iDefaultFilterID
	for (int i = m_list.GetItemCount() - 1; i >= 0; i--)
	{
		TViewFilter * pFilter = (TViewFilter*) m_list.GetItemData (i);
		if (m_iDefaultFilterID == pFilter->getID())
			SetDlgItemText (IDC_VIEWFILTER_DEFAULT_TXT, pFilter->GetName());
	}
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::OnMakeDefault()
{
	// set m_iDefaultFilterID
	int idx = m_list.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (idx < 0)
		return;

	int iPrev = m_iDefaultFilterID;
	TViewFilter * pFilter = GetpFilter (idx);
	if (pFilter)
	{
		m_iDefaultFilterID = pFilter->getID ();

		// reset static text
		fill_default ();

		// redraw new guy and redraw lame-duck
		if (iPrev != m_iDefaultFilterID)
			m_list.InvalidateRect (NULL);
	}
}

// ------------------------------------------------------------------------
// Handles WM_NOTIFY callback for display info
void TViewFiltersDlg::OnGetDisplayInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDisp = reinterpret_cast<LV_DISPINFO*>(pNMHDR);

	if (pDisp->item.mask & LVIF_IMAGE)
	{
		TViewFilter * pFilter = GetpFilter (pDisp->item.iItem);
		if (pFilter && (m_iDefaultFilterID == pFilter->getID()))
			pDisp->item.iImage = 1;
		else
			pDisp->item.iImage = 0;
	}
}

// ------------------------------------------------------------------------
TViewFilter * TViewFiltersDlg::GetpFilter (int idx)
{
	return (TViewFilter*) m_list.GetItemData (idx);
}

// ------------------------------------------------------------------------
void TViewFiltersDlg::OnViewfilterCopy()
{
	int idx = m_list.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (idx < 0)
		return;

	TViewFilter *pFilterOld = GetpFilter (idx);

	CString newName = "Copy of " + pFilterOld->GetName();

	idx = m_list.GetItemCount();
	int iAt = m_list.InsertItem (idx, newName, I_IMAGECALLBACK);

	// constructor will set the ID of the filter
	TViewFilter *pFilter = new TViewFilter ( newName,
		pFilterOld->GetDWordData(),
		pFilterOld->m_rstrCondition,
		pFilterOld->SortCode(),
		pFilterOld->getShowThread(),
		pFilterOld->getCompleteBinaries() ? true : false,
		pFilterOld->getNoThread() );

	m_list.SetItemData (iAt, (DWORD) pFilter);
}
