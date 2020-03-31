/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: trxdlg.cpp,v $
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
/*  Revision 1.3  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.2  2008/09/19 14:52:18  richard_wood
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

// trxdlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "trxdlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TRxDlg dialog
TRxDlg::TRxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TRxDlg::IDD, pParent)
{

	m_ftp = _T("");
	m_gopher = _T("");
	m_mail = _T("");
	m_telnet = _T("");
	m_web = _T("");
	m_highlightWeb = FALSE;
	m_highlightTelnet = FALSE;
	m_highlightMail = FALSE;
	m_highlightGopher = FALSE;
	m_highlightFtp = FALSE;

}

void TRxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_URL_RECOGNIZE_FTP,      m_ftp);
	DDX_Text(pDX, IDC_URL_RECOGNIZE_GOPHER,   m_gopher);
	DDX_Text(pDX, IDC_URL_RECOGNIZE_MAIL,     m_mail);
	DDX_Text(pDX, IDC_URL_RECOGNIZE_TELNET,   m_telnet);
	DDX_Text(pDX, IDC_URL_RECOGNIZE_WEB,      m_web);
	DDX_Check(pDX, IDC_URL_RECOGNIZE_WEB_HILITE, m_highlightWeb);
	DDX_Check(pDX, IDC_URL_RECOGNIZE_TELNET_HILITE, m_highlightTelnet);
	DDX_Check(pDX, IDC_URL_RECOGNIZE_MAIL_HILITE, m_highlightMail);
	DDX_Check(pDX, IDC_URL_RECOGNIZE_GOPHER_HILITE, m_highlightGopher);
	DDX_Check(pDX, IDC_URL_RECOGNIZE_FTP_HILITE, m_highlightFtp);

}

BEGIN_MESSAGE_MAP(TRxDlg, CDialog)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_GOPHERUSEDEF,   OnUrlRecognizeGopherusedef)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_MAILUSEDEF,     OnUrlRecognizeMailusedef)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_TELNETUSEDEF,   OnUrlRecognizeTelnetusedef)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_WEBUSEDEF,      OnUrlRecognizeWebusedef)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_FTPUSEDEF,      OnUrlRecognizeFtpusedef)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_WEB_HILITE,     EnableDisable)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_TELNET_HILITE,  EnableDisable)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_MAIL_HILITE,    EnableDisable)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_GOPHER_HILITE,  EnableDisable)
	ON_BN_CLICKED(IDC_URL_RECOGNIZE_FTP_HILITE,     EnableDisable)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TRxDlg message handlers

void TRxDlg::OnUrlRecognizeGopherusedef() 
{
	CString temp;
	temp.LoadString (IDS_DEFAULT_URL_GOPHER);
	CEdit *pEdit = (CEdit *) GetDlgItem(IDC_URL_RECOGNIZE_GOPHER);
	pEdit->SetWindowText (temp);

}

void TRxDlg::OnUrlRecognizeMailusedef() 
{
	CString temp;
	temp.LoadString (IDS_DEFAULT_URL_MAILTO);
	CEdit *pEdit = (CEdit *) GetDlgItem(IDC_URL_RECOGNIZE_MAIL);
	pEdit->SetWindowText (temp);

}

void TRxDlg::OnUrlRecognizeTelnetusedef() 
{
	CString temp;
	temp.LoadString (IDS_DEFAULT_URL_TELNET);
	CEdit *pEdit = (CEdit *) GetDlgItem(IDC_URL_RECOGNIZE_TELNET);
	pEdit->SetWindowText (temp);
}

void TRxDlg::OnUrlRecognizeWebusedef() 
{
	CString temp;
	temp.LoadString (IDS_DEFAULT_URL_HTTP);
	CEdit *pEdit = (CEdit *) GetDlgItem(IDC_URL_RECOGNIZE_WEB);
	pEdit->SetWindowText (temp);

}

void TRxDlg::OnUrlRecognizeFtpusedef() 
{
	CString temp;
	temp.LoadString (IDS_DEFAULT_URL_FTP);
	CEdit *pEdit = (CEdit *) GetDlgItem(IDC_URL_RECOGNIZE_FTP);
	pEdit->SetWindowText (temp);
}

void TRxDlg::EnableDisable ()
{
	BOOL fEnable;

	fEnable = ((CButton*)GetDlgItem (IDC_URL_RECOGNIZE_MAIL_HILITE))->GetCheck();
	GetDlgItem(IDC_URL_RECOGNIZE_MAIL)->EnableWindow (fEnable);
	GetDlgItem(IDC_URL_RECOGNIZE_MAILUSEDEF)->EnableWindow (fEnable);

	fEnable = ((CButton*)GetDlgItem (IDC_URL_RECOGNIZE_WEB_HILITE))->GetCheck();
	GetDlgItem(IDC_URL_RECOGNIZE_WEB)->EnableWindow (fEnable);
	GetDlgItem(IDC_URL_RECOGNIZE_WEBUSEDEF)->EnableWindow (fEnable);

	fEnable = ((CButton*)GetDlgItem (IDC_URL_RECOGNIZE_FTP_HILITE))->GetCheck();
	GetDlgItem(IDC_URL_RECOGNIZE_FTP)->EnableWindow (fEnable);
	GetDlgItem(IDC_URL_RECOGNIZE_FTPUSEDEF)->EnableWindow (fEnable);

	fEnable = ((CButton*)GetDlgItem (IDC_URL_RECOGNIZE_GOPHER_HILITE))->GetCheck();
	GetDlgItem(IDC_URL_RECOGNIZE_GOPHER)->EnableWindow (fEnable);
	GetDlgItem(IDC_URL_RECOGNIZE_GOPHERUSEDEF)->EnableWindow (fEnable);

	fEnable = ((CButton*)GetDlgItem (IDC_URL_RECOGNIZE_TELNET_HILITE))->GetCheck();
	GetDlgItem(IDC_URL_RECOGNIZE_TELNET)->EnableWindow (fEnable);
	GetDlgItem(IDC_URL_RECOGNIZE_TELNETUSEDEF)->EnableWindow (fEnable);
}

BOOL TRxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	EnableDisable ();
	return TRUE;
}
