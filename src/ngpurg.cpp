/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngpurg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:37  richard_wood
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

// ngpurg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "ngpurg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TPurgePage property page

IMPLEMENT_DYNCREATE(TPurgePage, CPropertyPage)

TPurgePage::TPurgePage() : CPropertyPage(TPurgePage::IDD)
{

	m_fPurgeRead = FALSE;
	m_fPurgeUnread = FALSE;
	m_fOnHdr = FALSE;
	m_fShutdown = FALSE;
	m_iReadLimit = 0;
	m_iUnreadLimit = 0;
	m_iDaysHdrPurge = 0;
	m_iShutCompact = 0;
	m_fOverride = FALSE;
}

TPurgePage::~TPurgePage()
{
}

void TPurgePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_OPTIONS_PURGE_RHSPIN, m_spinGetHeaders);
	DDX_Control(pDX, IDC_OPTIONS_PURGE_ONSHUTSPIN, m_spinShutdown);
	DDX_Control(pDX, IDC_OPTIONS_PURGE_ULSPIN, m_spinUnreadLimit);
	DDX_Control(pDX, IDC_OPTIONS_PURGE_RLSPIN, m_spinReadLimit);
	DDX_Check(pDX, IDC_NGPURGE_OVERRIDE, m_fOverride);
	DDX_Check(pDX, IDC_OPTIONS_PURGE_READCBX, m_fPurgeRead);
	DDX_Check(pDX, IDC_OPTIONS_PURGE_UNREADCBX, m_fPurgeUnread);
	DDX_Check(pDX, IDC_OPTIONS_PURGE_ONHDRCBX, m_fOnHdr);
	DDX_Check(pDX, IDC_OPTIONS_PURGE_ONSHUTCBX, m_fShutdown);
	DDX_Text(pDX, IDC_OPTIONS_PURGE_READ_LIMIT, m_iReadLimit);
	DDV_MinMaxInt(pDX, m_iReadLimit, 1, 99);
	DDX_Text(pDX, IDC_OPTIONS_PURGE_UNREAD_LIMIT, m_iUnreadLimit);
	DDV_MinMaxInt(pDX, m_iUnreadLimit, 1, 99);
	DDX_Text(pDX, IDC_OPTIONS_PURGE_ONHDR_EVERY, m_iDaysHdrPurge);
	DDV_MinMaxInt(pDX, m_iDaysHdrPurge, 0, 99);
	DDX_Text(pDX, IDC_OPTIONS_PURGE_ONSHUT_EVERY, m_iShutCompact);
	DDV_MinMaxInt(pDX, m_iShutCompact, 0, 99);


	if (!pDX->m_bSaveAndValidate)
		OnNgpurgeOverride();
}

BEGIN_MESSAGE_MAP(TPurgePage, CPropertyPage)
	ON_BN_CLICKED(IDC_OPTIONS_PURGE_READCBX, OnOptionsPurgeReadcbx)
	ON_BN_CLICKED(IDC_OPTIONS_PURGE_UNREADCBX, OnOptionsPurgeUnreadcbx)
	ON_BN_CLICKED(IDC_OPTIONS_PURGE_ONHDRCBX, OnOptionsPurgeOnhdrcbx)
	ON_BN_CLICKED(IDC_OPTIONS_PURGE_ONSHUTCBX, OnOptionsPurgeOnshutcbx)
	ON_BN_CLICKED(IDC_NGPURGE_OVERRIDE, OnNgpurgeOverride)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TPurgePage message handlers

void TPurgePage::OnOptionsPurgeReadcbx()
{
	disable_by_cbx(IDC_OPTIONS_PURGE_READCBX,
		IDC_OPTIONS_PURGE_READ_LIMIT,
		IDC_OPTIONS_PURGE_RLSPIN,
		IDC_OPTIONS_PURGE_TEXT1);
}

void TPurgePage::OnOptionsPurgeUnreadcbx()
{
	disable_by_cbx(IDC_OPTIONS_PURGE_UNREADCBX,
		IDC_OPTIONS_PURGE_UNREAD_LIMIT,
		IDC_OPTIONS_PURGE_ULSPIN,
		IDC_OPTIONS_PURGE_TEXT4);
}

void TPurgePage::OnOptionsPurgeOnhdrcbx()
{
	disable_by_cbx(IDC_OPTIONS_PURGE_ONHDRCBX,
		IDC_OPTIONS_PURGE_ONHDR_EVERY,
		IDC_OPTIONS_PURGE_RHSPIN,
		IDC_OPTIONS_PURGE_TEXT6);
}

void TPurgePage::OnOptionsPurgeOnshutcbx()
{
	disable_by_cbx(IDC_OPTIONS_PURGE_ONSHUTCBX,
		IDC_OPTIONS_PURGE_ONSHUT_EVERY,
		IDC_OPTIONS_PURGE_ONSHUTSPIN,
		IDC_OPTIONS_PURGE_TEXT7);
}

void
TPurgePage::disable_by_cbx(int iCbxID, int iTarg1, int iTarg2, int iTarg3)
{
	CWnd* pWnd1 = GetDlgItem(iTarg1);
	CWnd* pWnd2 = GetDlgItem(iTarg2);
	CWnd* pWnd3 = GetDlgItem(iTarg3);

	BOOL fOn = IsDlgButtonChecked(iCbxID);

	pWnd1->EnableWindow(fOn);
	pWnd2->EnableWindow(fOn);
	pWnd3->EnableWindow(fOn);
}

static int vControlIDs[] = { IDC_OPTIONS_PURGE_READCBX,
IDC_OPTIONS_PURGE_READ_LIMIT,
IDC_OPTIONS_PURGE_RLSPIN,
IDC_OPTIONS_PURGE_TEXT1,
IDC_OPTIONS_PURGE_UNREADCBX,
IDC_OPTIONS_PURGE_UNREAD_LIMIT,
IDC_OPTIONS_PURGE_ULSPIN,
IDC_OPTIONS_PURGE_TEXT4,
IDC_OPTIONS_PURGE_ONHDRCBX,
IDC_OPTIONS_PURGE_ONHDR_EVERY,
IDC_OPTIONS_PURGE_RHSPIN,
IDC_OPTIONS_PURGE_TEXT6,
IDC_OPTIONS_PURGE_ONSHUTCBX,
IDC_OPTIONS_PURGE_ONSHUT_EVERY,
IDC_OPTIONS_PURGE_ONSHUTSPIN,
IDC_OPTIONS_PURGE_TEXT7 };

void TPurgePage::OnNgpurgeOverride() 
{
	BOOL fEnable = IsDlgButtonChecked(IDC_NGPURGE_OVERRIDE);
	int tot = sizeof(vControlIDs) / sizeof(vControlIDs[0]);
	for (--tot; tot >= 0; --tot)
		GetDlgItem( vControlIDs[tot] )->EnableWindow( fEnable );
}

BOOL TPurgePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// mainly to set the direction of the Spin Button. Up should be N+1
	m_spinReadLimit.SetRange  (1, 99);
	m_spinUnreadLimit.SetRange(1, 99);
	m_spinGetHeaders.SetRange (0, 99);
	m_spinShutdown.SetRange   (0, 99);

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL TPurgePage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"preferences-purging.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TPurgePage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
