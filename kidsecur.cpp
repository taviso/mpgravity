/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: kidsecur.cpp,v $
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
/*  Revision 1.4  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:28  richard_wood
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

// kidsecur.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "kidsecur.h"
#include "kidsecst.h"            // set pwd
#include "kidsecch.h"            // change pwd
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsKidSecurePage property page

IMPLEMENT_DYNCREATE(TOptionsKidSecurePage, CPropertyPage)

TOptionsKidSecurePage::TOptionsKidSecurePage() : CPropertyPage(TOptionsKidSecurePage::IDD)
{
	m_uPanicVKey = VK_F9;


	m_fPanicEnabled = FALSE;
}

TOptionsKidSecurePage::~TOptionsKidSecurePage()
{
}

//
//DDX_Control(pDX, IDC_CMB_PANICKEY, m_cmbPanicKey);
//DDX_Check(pDX, IDC_PANIC_ENABLED, m_fPanicEnabled);
void TOptionsKidSecurePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);


	if (FALSE == pDX->m_bSaveAndValidate)
	{
		update_button_state();
		//OnPanicEnabled() ;
	}
	//ddx_panickey_combo (pDX->m_bSaveAndValidate);
}

//
//ON_BN_CLICKED(IDC_PANIC_ENABLED, OnPanicEnabled)
BEGIN_MESSAGE_MAP(TOptionsKidSecurePage, CPropertyPage)
	ON_BN_CLICKED(IDC_KSEC_SETPWD, OnKsecSetpwd)
	ON_BN_CLICKED(IDC_KSEC_CHGPWD, OnKsecChgpwd)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsKidSecurePage message handlers

void TOptionsKidSecurePage::OnKsecSetpwd()
{
	TKidSecureSetPwd dlg(this, m_cipherText);
	dlg.DoModal();
	update_button_state();
}

void TOptionsKidSecurePage::update_button_state()
{
	if (m_cipherText.IsEmpty())
	{
		GetDlgItem(IDC_KSEC_SETPWD)->EnableWindow(TRUE);
		GetDlgItem(IDC_KSEC_CHGPWD)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_KSEC_SETPWD)->EnableWindow(FALSE);
		GetDlgItem(IDC_KSEC_CHGPWD)->EnableWindow(TRUE);
	}
}

void TOptionsKidSecurePage::OnKsecChgpwd()
{
	TKidSecureChgPwd dlg(this, m_cipherText);
	dlg.DoModal();
	update_button_state();
}

BOOL TOptionsKidSecurePage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp ()->HtmlHelp((DWORD)"preferences-security.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_SECURITY);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsKidSecurePage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
