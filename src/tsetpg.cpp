/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsetpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/20 21:04:55  richard_wood
/*  Updated splash screen component so it works properly.
/*  Fixed crash from new splash screen.
/*  Updated setver setup dialogs, changed into a Wizard with a lot more verification and "user helpfulness".
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:19  richard_wood
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

// tsetpg.cpp : this was split off from the "Connections page"

#include "stdafx.h"
#include "News.h"
#include "tsetpg.h"
#include "tglobopt.h"
#include "helpcode.h"            // HID_OPTIONS_*
#include "nameutil.h"            // DDV_EmailName()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(TSetupPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
TSetupPage::TSetupPage(bool bShowImport /*  = false */) : CPropertyPage(TSetupPage::IDD)
{
	m_bShowImport = bShowImport;
	m_emailAddress = _T("");
	m_fullname = _T("");
	m_organization = _T("");
	m_strMailOverride = _T("");
}

/////////////////////////////////////////////////////////////////////////////
void TSetupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_CONNECT_FULLNAME, m_fullname);
	DDX_Text(pDX, IDC_CONNECT_ORG, m_organization);
	DDX_Text(pDX, IDC_MAIL_OVERRIDE, m_strMailOverride);

	DDX_EmailName(pDX, IDC_CONNECT_EMAILADDRESS, m_emailAddress);
	DDV_EmailName(pDX, IDC_CONNECT_EMAILADDRESS, m_emailAddress, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(TSetupPage, CPropertyPage)
	ON_WM_HELPINFO()
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &TSetupPage::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_CONNECT_FULLNAME, &TSetupPage::OnEnChangeUpdateButtons)
	ON_EN_CHANGE(IDC_CONNECT_EMAILADDRESS, &TSetupPage::OnEnChangeUpdateButtons)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL TSetupPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp()->HtmlHelp((DWORD)"servers-setup-tab.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_SETUP);
	return 1;
}

// -------------------------------------------------------------------------
void TSetupPage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}

// -------------------------------------------------------------------------
void TSetupPage::OnBnClickedButton1()
{
	EndDialog(IDC_BUTTON_IMPORT);
}

BOOL TSetupPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	if (!m_bShowImport)
	{
		// Hide import button and change text a bit...
		GetDlgItem(IDC_BUTTON_IMPORT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_LINE1)->SetWindowText("             Please enter your details below.");
		GetDlgItem(IDC_STATIC_LINE2)->SetWindowText("");
	}

	if (((CPropertySheet*)GetParent())->IsWizard())
		((CPropertySheet*)GetParent())->SetWizardButtons(0); // Disable prev, next and finish

	return TRUE;
}

void TSetupPage::OnEnChangeUpdateButtons()
{
	if (((CPropertySheet*)GetParent())->IsWizard())
	{
		CString strName, strEmail;
		GetDlgItem(IDC_CONNECT_FULLNAME)->GetWindowText(strName);
		GetDlgItem(IDC_CONNECT_EMAILADDRESS)->GetWindowText(strEmail);

		bool bCanDoNext = !(strName.IsEmpty() || strEmail.IsEmpty());
		((CPropertySheet*)GetParent())->SetWizardButtons(bCanDoNext ? PSWIZB_NEXT : 0);
	}
}

BOOL TSetupPage::OnSetActive()
{
	OnEnChangeUpdateButtons();

	return CPropertyPage::OnSetActive();
}
