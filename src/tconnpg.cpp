/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tconnpg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:59  richard_wood
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

// tconnpg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"            // IDD_OPTIONS_CONNECTION
#include "tconnpg.h"
#include "tglobopt.h"
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

/////////////////////////////////////////////////////////////////////////////
// TConnectionPage property page

IMPLEMENT_DYNCREATE(TConnectionPage, CPropertyPage)

TConnectionPage::TConnectionPage() : CPropertyPage(TConnectionPage::IDD)
{
   
   m_fConnectAtStartup = FALSE;
   m_fKeepAlive = FALSE;
   m_iKeepAliveMinutes = 0;
   m_fAutoQuit = FALSE;
   m_iAutoQuitMinutes = 0;
	m_fRetryPause = FALSE;
	m_fRetry = FALSE;
	m_iPauseCount = 0;
	m_iRetryCount = 0;
	m_fModeReader = FALSE;
}

TConnectionPage::~TConnectionPage()
{
}

void TConnectionPage::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
   
   DDX_Check(pDX, IDC_CONNECT_CONNECTATSTARTUP, m_fConnectAtStartup);
   DDX_Check(pDX, IDC_CONNECT_KEEPALIVE_CBX, m_fKeepAlive);
   DDX_Text(pDX, IDC_CONNECT_ALIVE_EDIT, m_iKeepAliveMinutes);
   DDV_MinMaxInt(pDX, m_iKeepAliveMinutes, 1, 120);
   DDX_Check(pDX, IDC_CONNECT_AUTOQUIT_CBX, m_fAutoQuit);
   DDX_Text(pDX, IDC_CONNECT_AUTOQUIT_EDIT, m_iAutoQuitMinutes);
   DDV_MinMaxInt(pDX, m_iAutoQuitMinutes, 1, 60);
	DDX_Check(pDX, IDC_CONNECT_PAUSE, m_fRetryPause);
	DDX_Check(pDX, IDC_CONNECT_RETRY, m_fRetry);
	DDX_Text(pDX, IDC_CONNECT_PEDIT, m_iPauseCount);
	DDV_MinMaxUInt(pDX, m_iPauseCount, 1, 120);
	DDX_Text(pDX, IDC_CONNECT_REDT, m_iRetryCount);
	DDV_MinMaxUInt(pDX, m_iRetryCount, 1, 100);
	DDX_Check(pDX, IDC_CONNECT_MODEREADER, m_fModeReader);


   if (!pDX->m_bSaveAndValidate)
      {
      // checkboxes may disable associated editboxes
      doKeepAliveCheckbox();
      OnAutoDisconnectCbx();
      }
}

BEGIN_MESSAGE_MAP(TConnectionPage, CPropertyPage)
      ON_BN_CLICKED(IDC_CONNECT_KEEPALIVE_CBX, OnConnectKeepaliveCbx)
   ON_BN_CLICKED(IDC_CONNECT_AUTOQUIT_CBX, OnAutoDisconnectCbx)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_CONNECT_PAUSE, OnConnectPause)
	ON_BN_CLICKED(IDC_CONNECT_RETRY, OnConnectRetry)

   ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TConnectionPage message handlers

BOOL TConnectionPage::OnInitDialog()
{
   CPropertyPage::OnInitDialog();

   CSpinButtonCtrl * pSpin = (CSpinButtonCtrl*) GetDlgItem(IDC_ALIVE_SPIN);
   pSpin->SetRange (1, 120);

   pSpin = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_AUTOQUIT_SPIN));
   pSpin->SetRange (1, 60);

   // Retry Count
   pSpin = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN_RTRYCOUNT));
   pSpin->SetRange (1, 100);

   pSpin = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PAUSE_SPIN));
   pSpin->SetRange (1, 120);

   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}

void TConnectionPage::OnConnectKeepaliveCbx()
{
   doKeepAliveCheckbox();
}

void TConnectionPage::doKeepAliveCheckbox()
{
   update_spinners (IDC_CONNECT_KEEPALIVE_CBX,
                     IDC_CONNECT_ALIVE_EDIT,
                     IDC_ALIVE_SPIN,
                     IDC_STATIC_MINUTES2);
}

// ------------------------------------------------------------------------
void TConnectionPage::OnAutoDisconnectCbx()
{
   BOOL fChecked = IsDlgButtonChecked (IDC_CONNECT_AUTOQUIT_CBX);
   GetDlgItem (IDC_CONNECT_AUTOQUIT_EDIT)->EnableWindow (fChecked);
   GetDlgItem (IDC_AUTOQUIT_SPIN)->EnableWindow (fChecked);
}

BOOL TConnectionPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
   AfxGetApp () -> HtmlHelp((DWORD)"servers-connect-tab.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_CONNECT);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TConnectionPage::OnPSNHelp (NMHDR *, LRESULT *)
{
   OnHelpInfo (NULL);
}

// -------------------------------------------------------------------------
void TConnectionPage::update_spinners (int iCbx, int iEbx, int iSpinner, int iStatic)
{
   BOOL fEnabled = IsDlgButtonChecked (iCbx);

   GetDlgItem (iEbx)     -> EnableWindow (fEnabled);
   GetDlgItem (iSpinner) -> EnableWindow (fEnabled);
   GetDlgItem (iStatic)  -> EnableWindow (fEnabled);
}

// -------------------------------------------------------------------------
void TConnectionPage::OnConnectRetry()
{
   update_spinners (IDC_CONNECT_RETRY,
                    IDC_CONNECT_REDT,
                    IDC_SPIN_RTRYCOUNT,
                    IDC_CONNECT_PZTEXT);
}

// -------------------------------------------------------------------------
void TConnectionPage::OnConnectPause()
{
   update_spinners (IDC_CONNECT_PAUSE,
                    IDC_CONNECT_PEDIT,
                    IDC_PAUSE_SPIN,
                    IDC_CONNECT_PZTEXT);
}

