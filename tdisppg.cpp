/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdisppg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.4  2009/01/26 08:34:46  richard_wood
/*  The buttons are back!!!
/*
/*  Revision 1.3  2008/09/19 14:52:00  richard_wood
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

// tdisppg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "tdisppg.h"
#include "pktxtclr.h"
#include "globals.h"
#include "tglobopt.h"
#include "helpcode.h"            // HID_OPTIONS_*

extern TGlobalOptions* gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsDisplayDlg property page

IMPLEMENT_DYNCREATE(TOptionsDisplayDlg, CPropertyPage)

TOptionsDisplayDlg::TOptionsDisplayDlg() : CPropertyPage(TOptionsDisplayDlg::IDD)
{
	m_fXFaces = FALSE;
	m_fChanged = FALSE;
}

TOptionsDisplayDlg::~TOptionsDisplayDlg()
{
}

/////////////////////////////////////////////////////////////////////
// Custom Validation routine
//
void AFXAPI DDV_DisplayColumns(CDataExchange* pDX, int nIDC, CWnd* pDlg)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);

	if (pDX->m_bSaveAndValidate)
	{
		BOOL fFound = FALSE;
		CCheckListBox* pCheck = (CCheckListBox*) pDlg->GetDlgItem(nIDC);
		int tot = pCheck->GetCount();
		for (--tot; tot >= 0; --tot)
		{
			if (1 == pCheck->GetCheck(tot))
			{
				fFound = TRUE;
				break;
			}
		}

		if (!fFound)
		{
			AfxMessageBox(IDS_ERR_DISPLAYCOLUMN);
			pDX->Fail();
		}
	}
	else
	{
		// do nothing
	}
}

void TOptionsDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	//  DDX_Radio takes an IDC and a zero-based index
	DDX_Radio(pDX, IDC_OPTDISP_THRD_BYDATE, m_kSortThread);
	DDX_Text(pDX, IDC_OPTIONS_DISPLAY_TIME, m_strDateFormat);
	DDX_Check(pDX, IDC_OPTDISP_COLLAPSE, m_iShowThreadCollapsed);
	DDX_Radio(pDX, IDC_OPTDISP_THREDSUBJ, m_fPureThreading);
	DDX_Check(pDX, IDC_XFACES, m_fXFaces);

	DDV_DisplayColumns(pDX, IDC_OPTIONS_DISPLAY_THREADCOLS, this);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		if (m_lbxThreadCols.GetCount() > 0)
			return;

		init_check_lbx (&m_lbxThreadCols, m_threadCols);
	}
	else
	{
		m_fChanged = TRUE;
		save_check_lbx (&m_lbxThreadCols, m_threadCols);
	}
}

BEGIN_MESSAGE_MAP(TOptionsDisplayDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_BUT_NEWARTCOLOR, OnButOldArticleColor)
	ON_BN_CLICKED(IDC_DISPLAY_OPTIONS_TUP, OnDisplayOptionsTup)
	ON_BN_CLICKED(IDC_DISPLAY_OPTIONS_TDN, OnDisplayOptionsTdn)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_OPTDISP_THREDMSGID, OnOptdispThredmsgid)
	ON_BN_CLICKED(IDC_OPTDISP_THREDSUBJ, OnOptdispThredsubj)
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsDisplayDlg message handlers

void TOptionsDisplayDlg::OnButOldArticleColor()
{
	LOGFONT        lf;

	// need some valid font
	gpVariableFont->GetLogFont( &lf );

	TPickTextColorDlg dlgTextColor(&lf,
		CF_INITTOLOGFONTSTRUCT
		| CF_ANSIONLY
		| CF_SCREENFONTS
		| CF_EFFECTS
		| CF_FORCEFONTEXIST,
		NULL,
		this);

	dlgTextColor.m_cf.rgbColors = m_oldArticleColor;

	if (IDOK == dlgTextColor.DoModal())
		m_oldArticleColor = dlgTextColor.GetColor();
}

BOOL TOptionsDisplayDlg::OnInitDialog()
{
	m_lbxThreadCols.SubclassDlgItem (IDC_OPTIONS_DISPLAY_THREADCOLS, this);
	m_lbxThreadCols.SetCheckStyle (BS_AUTOCHECKBOX);

	CPropertyPage::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void TOptionsDisplayDlg::init_check_lbx(CCheckListBox* pLbx, CString data)
{
	int iLen = data.GetLength();
	BOOL fEnabled;
	int  iStrID;
	CString str;

	for (int i = 0; i < iLen; i+= 2)
	{
		TCHAR  dat = 0;
		fEnabled = 'y' == data[i];
		switch (data[i+1])
		{
		case 'T':
		case 't':
			// virtual Thread Column
			dat = 0;
			break;

		case 'F':
		case 'f':
			dat = 'F';
			iStrID = IDS_HDRCTRL_FROM;
			break;

		case 'I':      // status indicators
		case 'i':
			dat = 'I';
			iStrID = IDS_HDRCTRL_STATUS;
			break;

		case 'S':
		case 's':
			dat = 'S';
			iStrID = IDS_HDRCTRL_SUBJECT;
			break;

		case 'L':
		case 'l':
			dat = 'L';
			iStrID = IDS_HDRCTRL_LINES;
			break;

		case 'D':
		case 'd':
			dat = 'D';
			iStrID = IDS_HDRCTRL_DATE;
			break;

		case 'R':
		case 'r':
			dat = 'R';
			iStrID = IDS_HDRCTRL_SCORE;
			break;
		}
		if (dat)
		{
			str.LoadString(iStrID);
			int at = pLbx->AddString( str );
			pLbx->SetItemData(at, dat);

			if (fEnabled)
				pLbx->SetCheck(at, 1);
		}
	}
}

void TOptionsDisplayDlg::save_check_lbx (CCheckListBox* pLbx, CString& data)
{
	data.Empty();
	int tot = pLbx->GetCount();
	for (int i = 0; i < tot; ++i)
	{
		TCHAR dat = (TCHAR) pLbx->GetItemData(i);
		BOOL  fEnabled = (pLbx->GetCheck(i) == 1);
		data += fEnabled ? 'y' : 'n';
		data += dat;
	}

	// version 1.01 has a virtual column here
	if (IDC_OPTIONS_DISPLAY_THREADCOLS == pLbx->GetDlgCtrlID())
		data = "yT" + data;
}

// -------------------------------------------------------------------------
static enum Direction {UP, DOWN};

// -------------------------------------------------------------------------
static void MoveItem (CCheckListBox* psLbx, Direction iDirection)
{
	// get the currently-selected item's index
	int iOldIndex = psLbx -> GetCurSel ();
	if (iOldIndex < 0)
		return;

	// if index is already at bottom/top, stop
	if ((iDirection == DOWN && iOldIndex == psLbx -> GetCount () - 1) ||
		(iDirection == UP && !iOldIndex))
		return;

	// get its string
	CString str;
	psLbx -> GetText (iOldIndex, str);

	// get its state
	int iCheckState = psLbx->GetCheck (iOldIndex);

	// get its data
	DWORD dwData = psLbx -> GetItemData (iOldIndex);

	// insert into new position
	int iNewIndex = psLbx -> InsertString (iDirection == DOWN
		? iOldIndex + 2 : iOldIndex - 1, str);
	if (UP == iDirection) psLbx -> SetCurSel (iNewIndex);
	psLbx -> SetItemData (iNewIndex, dwData);
	psLbx -> SetCheck (iNewIndex, iCheckState);

	// delete from old position
	psLbx -> DeleteString (iDirection == DOWN ? iOldIndex : iOldIndex + 1);
	if (DOWN == iDirection)
	{
		// we are deleting the old version above us, so we have to set
		// set selection to N-1
		psLbx -> SetCurSel (--iNewIndex);
	}
}

void TOptionsDisplayDlg::OnDisplayOptionsTup()
{
	MoveItem(&m_lbxThreadCols, UP);
}

void TOptionsDisplayDlg::OnDisplayOptionsTdn()
{
	MoveItem(&m_lbxThreadCols, DOWN);
}

BOOL TOptionsDisplayDlg::ColumnsEquivalent(CString& strOld, CString& strNew)
{
	CString sumOld;
	CString sumNew;
	int i;
	int tot = strOld.GetLength();
	for (i = 0; i < tot; i += 2)
	{
		if ('y' == strOld[i])
			sumOld += strOld[i+1];
	}
	tot = strNew.GetLength();
	for (i = 0; i < tot; i+=2)
		if ('y' == strNew[i])
			sumNew += strNew[i+1];
	return sumOld == sumNew;
}

BOOL TOptionsDisplayDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp () -> HtmlHelp((DWORD)"customize-thread-pane.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_DISPLAY);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsDisplayDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}

void TOptionsDisplayDlg::OnOptdispThredmsgid()
{
}

void TOptionsDisplayDlg::OnOptdispThredsubj()
{
}
