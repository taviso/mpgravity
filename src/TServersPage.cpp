/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TServersPage.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2010/04/20 21:04:55  richard_wood
/*  Updated splash screen component so it works properly.
/*  Fixed crash from new splash screen.
/*  Updated setver setup dialogs, changed into a Wizard with a lot more verification and "user helpfulness".
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:08  richard_wood
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

// TServersPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TServersPage.h"

#include "nameutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TServersPage property page

IMPLEMENT_DYNCREATE(TServersPage, CPropertyPage)

TServersPage::TServersPage() : CPropertyPage(TServersPage::IDD)
{
	m_newsServer = _T("");
	m_strNNTPAddress = _T("");
	m_iLogonStyle_NTP = 0;
	m_authName_NTP = _T("");
	m_authPass_NTP = _T("");

	m_smtpServer = _T("");
	m_iLogonStyle_STP = 0;
	m_authName_STP = _T("");
	m_authPass_STP = _T("");
	m_iPortNNTP = 119;
	m_iPortSMTP = 25;
}

TServersPage::~TServersPage()
{
}

void TServersPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_NNTPSERVER_ADDRESS, m_strNNTPAddress);

	DDX_Radio(pDX, IDC_OPTSERVER_RBT_NOPWD, m_iLogonStyle_NTP);

	DDX_Radio(pDX, IDC_RBT_SMTP_NOPWD,  m_iLogonStyle_STP);

	DDX_Text(pDX, IDC_CONNECT_AUTHNAME, m_authName_NTP);
	DDV_MaxChars(pDX, m_authName_NTP, 80);
	DDX_Text(pDX, IDC_CONNECT_AUTHPWD, m_authPass_NTP);
	DDV_MaxChars(pDX, m_authPass_NTP, 80);

	DDX_Text(pDX, IDC_CONNECT_SMTPSERVER, m_smtpServer);
	DDX_Text(pDX, IDC_EBX_SMTP_USER, m_authName_STP);
	DDV_MaxChars(pDX, m_authName_STP, 80);
	DDX_Text(pDX, IDC_EBX_SMTP_PWD, m_authPass_STP);
	DDV_MaxChars(pDX, m_authPass_STP, 80);

	DDX_Text(pDX, IDC_CONNECT_ADVANCED_NNTP2, m_iPortNNTP);
	DDV_MinMaxInt(pDX, m_iPortNNTP, 0, 65535);

	DDX_Text(pDX, IDC_CONNECT_ADVANCED_SMTP2, m_iPortSMTP);
	DDV_MinMaxInt(pDX, m_iPortSMTP, 0, 65535);

	DDX_HostName(pDX, IDC_CONNECTION_HOST, m_newsServer);
	DDV_HostName(pDX, IDC_CONNECTION_HOST, m_newsServer, TRUE);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		EnableAccountName(1 == m_iLogonStyle_NTP);
		EnableSmtpAccountName(1 == m_iLogonStyle_STP);
	}
}

BEGIN_MESSAGE_MAP(TServersPage, CPropertyPage)
	ON_BN_CLICKED(IDC_RBT_SMTP_NOPWD, OnRbtSmtpNopwd)
	ON_BN_CLICKED(IDC_RBT_SMTPPWD, OnRbtSmtppwd)
	ON_BN_CLICKED(IDC_OPTSERVER_RBT_NOPWD, OnOptserverRbtNopwd)
	ON_BN_CLICKED(IDC_OPTSERVER_RBT_LOGONW, OnOptserverRbtLogonw)
	ON_BN_CLICKED(IDC_OPTSERVER_RBT_SPA, OnOptserverRbtSpa)
	ON_EN_CHANGE(IDC_CONNECTION_HOST, &TServersPage::OnEnChangeUpdateButtons)
	ON_EN_CHANGE(IDC_NNTPSERVER_ADDRESS, &TServersPage::OnEnChangeUpdateButtons)
	ON_EN_CHANGE(IDC_CONNECT_AUTHNAME, &TServersPage::OnEnChangeUpdateNNTPAuth)
	ON_EN_CHANGE(IDC_CONNECT_AUTHPWD, &TServersPage::OnEnChangeUpdateNNTPAuth)
	ON_BN_CLICKED(IDC_RBT_SMTP_USE_NNTP_AUTH, &TServersPage::OnBnClickedRbtSmtpUseNntpAuth)
	ON_EN_CHANGE(IDC_EBX_SMTP_USER, &TServersPage::OnEnChangeUpdateButtons)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TServersPage message handlers

void TServersPage::OnRbtSmtpNopwd() 
{
	// clear everything out
	m_authName_STP = m_authPass_STP = "";
	GetDlgItem(IDC_EBX_SMTP_USER)->SetWindowText("");
	GetDlgItem(IDC_EBX_SMTP_PWD)->SetWindowText("");

	EnableSmtpAccountName(FALSE);
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::OnRbtSmtppwd() 
{
	EnableSmtpAccountName(TRUE);
	GetDlgItem(IDC_EBX_SMTP_USER)->SetFocus();
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::OnBnClickedRbtSmtpUseNntpAuth()
{
	m_authName_STP = m_authName_NTP;
	m_authPass_STP = m_authPass_NTP;
	GetDlgItem(IDC_EBX_SMTP_USER)->SetWindowText(m_authName_STP);
	GetDlgItem(IDC_EBX_SMTP_PWD)->SetWindowText(m_authPass_STP);
	EnableSmtpAccountName(FALSE);
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::OnOptserverRbtNopwd() 
{
	m_authName_NTP = m_authPass_NTP = "";
	GetDlgItem(IDC_CONNECT_AUTHNAME)->SetWindowText("");
	GetDlgItem(IDC_CONNECT_AUTHPWD)->SetWindowText("");
	EnableAccountName(FALSE);
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::OnOptserverRbtLogonw() 
{
	m_authName_NTP = m_authPass_NTP = "";

	GetDlgItem(IDC_CONNECT_AUTHNAME)->SetWindowText("");
	GetDlgItem(IDC_CONNECT_AUTHPWD)->SetWindowText("");

	int nSMTPUseNNTPCredentials = ((CButton*)GetDlgItem(IDC_RBT_SMTP_USE_NNTP_AUTH))->GetCheck();
	if (nSMTPUseNNTPCredentials)
		OnBnClickedRbtSmtpUseNntpAuth();
	
	EnableAccountName(TRUE);
	GetDlgItem(IDC_CONNECT_AUTHNAME)->SetFocus();
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::OnEnChangeUpdateNNTPAuth()
{
	UpdateData();
	int nSMTPUseNNTPCredentials = ((CButton*)GetDlgItem(IDC_RBT_SMTP_USE_NNTP_AUTH))->GetCheck();
	if (nSMTPUseNNTPCredentials)
	{
		m_authName_STP = m_authName_NTP;
		m_authPass_STP = m_authPass_NTP;
	}
	UpdateData(FALSE);
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::OnOptserverRbtSpa() 
{
	EnableAccountName(FALSE);
	OnEnChangeUpdateButtons();
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::EnableAccountName(BOOL fEnable)
{
	GetDlgItem(IDC_CONNECT_AUTHNAME_DESC)->EnableWindow(fEnable);
	GetDlgItem(IDC_CONNECT_AUTHNAME)->EnableWindow (fEnable);
	GetDlgItem(IDC_CONNECT_AUTHPWD_DESC)->EnableWindow(fEnable);
	GetDlgItem(IDC_CONNECT_AUTHPWD)->EnableWindow (fEnable);
}

/////////////////////////////////////////////////////////////////////////////
void TServersPage::EnableSmtpAccountName(BOOL fEnable)
{
	GetDlgItem(IDC_STATIC_SMTP1)->EnableWindow(fEnable);
	GetDlgItem(IDC_EBX_SMTP_USER)->EnableWindow (fEnable);
	GetDlgItem(IDC_STATIC_SMTP2)->EnableWindow(fEnable);
	GetDlgItem(IDC_EBX_SMTP_PWD)->EnableWindow (fEnable);
}

void TServersPage::OnEnChangeUpdateButtons()
{
	if (((CPropertySheet*)GetParent())->IsWizard())
	{
		CString strServerName, strServerAddress, strUsername, strSMTPServer, strSMTPUsername;

		GetDlgItem(IDC_CONNECTION_HOST)->GetWindowText(strServerName);
		GetDlgItem(IDC_NNTPSERVER_ADDRESS)->GetWindowText(strServerAddress);
		GetDlgItem(IDC_CONNECT_AUTHNAME)->GetWindowText(strUsername);
		GetDlgItem(IDC_CONNECT_SMTPSERVER)->GetWindowText(strSMTPServer);
		GetDlgItem(IDC_EBX_SMTP_USER)->GetWindowText(strSMTPUsername);

		int nUsernameChoice = ((CButton*)GetDlgItem(IDC_OPTSERVER_RBT_LOGONW))->GetCheck();
		int nSMTPUsernameChoice = ((CButton*)GetDlgItem(IDC_RBT_SMTPPWD))->GetCheck();

		bool bValid = false;
		// Must have a server name and address
		if (!strServerName.IsEmpty() && !strServerAddress.IsEmpty())
		{
			// If using log on credentials, must have at least a username
			if (
				(nUsernameChoice == 0)
				||
				((nUsernameChoice == 1) && !strUsername.IsEmpty())
			   )
			{
				// SMTP server can be empty
				if (strSMTPServer.IsEmpty())
					bValid = true;
				else
				{
					// If it is not, and we're using SMTP credentials,
					// we must have at least an SMTP username.
					if (
						(nSMTPUsernameChoice == 0)
						||
						((nSMTPUsernameChoice == 1) && !strSMTPUsername.IsEmpty())
					   )
					   bValid = true;
				}
			}
		}

		((CPropertySheet*)GetParent())->SetWizardButtons(bValid ? PSWIZB_BACK|PSWIZB_FINISH : PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
	}
}

BOOL TServersPage::OnSetActive()
{
	OnEnChangeUpdateButtons();

	return CPropertyPage::OnSetActive();
}

BOOL TServersPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"servers-servers-tab.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TServersPage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
