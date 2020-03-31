/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thotlink.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:04  richard_wood
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

// thotlink.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "thotlink.h"
#include "turldef.h"
#include "globals.h"
#include "trxdlg.h"
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// THotlinkPage property page

IMPLEMENT_DYNCREATE(THotlinkPage, CPropertyPage)

THotlinkPage::THotlinkPage() : CPropertyPage(THotlinkPage::IDD), 
                               m_webHelper("Web"),
                               m_ftpHelper("Ftp"),
                               m_gopherHelper("Gopher"),
                               m_telnetHelper("Telnet")
{
   
   m_fUnderline = FALSE;
   m_fMailUseReg = 2;
   m_fWebUseReg = FALSE;
   m_fTelnetUseReg = FALSE;
   m_fGopherUseReg = FALSE;
   m_fFtpUseReg = FALSE;
   m_colorRef = 0;      // black for now
   
}

THotlinkPage::~THotlinkPage()
{
}

void THotlinkPage::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
   
   DDX_Check(pDX, IDC_OPTIONS_HOTLINK_UNDERLINE, m_fUnderline);
   DDX_Radio(pDX, IDC_OPTIONS_HOTLINK_MAIL_USEREG1, m_fMailUseReg);
   DDX_Check(pDX, IDC_OPTIONS_HOTLINK_WEB_USEREG, m_fWebUseReg);
   DDX_Check(pDX, IDC_OPTIONS_HOTLINK_TELNET_USEREG, m_fTelnetUseReg);
   DDX_Check(pDX, IDC_OPTIONS_HOTLINK_GOPHER_USEREG, m_fGopherUseReg);
   DDX_Check(pDX, IDC_OPTIONS_HOTLINK_FTP_USEREG, m_fFtpUseReg);
   
}

BEGIN_MESSAGE_MAP(THotlinkPage, CPropertyPage)
      ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_COLOR, OnOptionsHotlinkColor)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_FTP_CUSTOM, OnOptionsHotlinkFtpCustom)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_FTP_USEREG, OnOptionsHotlinkFtpUsereg)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_GOPHER_CUSTOM, OnOptionsHotlinkGopherCustom)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_GOPHER_USEREG, OnOptionsHotlinkGopherUsereg)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_MAIL_CUSTOM, OnOptionsHotlinkMailCustom)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_MAIL_USEREG1, OnOptionsHotlinkMailUsereg1)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_TELNET_CUSTOM, OnOptionsHotlinkTelnetCustom)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_TELNET_USEREG, OnOptionsHotlinkTelnetUsereg)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_WEB_CUSTOM, OnOptionsHotlinkWebCustom)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_WEB_USEREG, OnOptionsHotlinkWebUsereg)
   ON_WM_CTLCOLOR()
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_UNDERLINE, OnOptionsHotlinkUnderline)
   ON_BN_CLICKED(IDC_OPTIONS_HOTLINK_RECOGNIZE, OnOptionsHotlinkRecognize)
	ON_WM_HELPINFO()

   ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// THotlinkPage message handlers

void THotlinkPage::OnOptionsHotlinkColor() 
{
CColorDialog colorDlg (m_colorRef);
colorDlg.m_cc.Flags = colorDlg.m_cc.Flags|CC_PREVENTFULLOPEN;

if (IDOK == colorDlg.DoModal ())
   {
   m_colorRef = colorDlg.GetColor();
   CWnd *pWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_SAMPLE);
   pWnd->Invalidate();
   }
}

void THotlinkPage::DoCustom(const CString &  caption,
                            TUrlDde *        pDDE)

{
   TURLDef  custDlg;

   custDlg.m_service       = pDDE->GetService();
   custDlg.m_topic         = pDDE->GetTopic ();
   custDlg.m_fUseDDE       = pDDE->UseDDE ();
   custDlg.m_application   = pDDE->GetApplication();
   custDlg.m_data          = pDDE->GetData ();
   custDlg.m_item          = pDDE->GetItem ();
   custDlg.m_transType     = pDDE->GetTransactionType();
   custDlg.m_caption       = caption;

   if (IDOK == custDlg.DoModal())
      {
      pDDE->SetService(custDlg.m_service);
      pDDE->SetTopic (custDlg.m_topic);
      pDDE->SetUseDDE (custDlg.m_fUseDDE);
      pDDE->SetApplication(custDlg.m_application);
      pDDE->SetData (custDlg.m_data);
      pDDE->SetItem (custDlg.m_item);
      pDDE->SetTransactionType(custDlg.m_transType);
      }
}

void THotlinkPage::OnOptionsHotlinkFtpCustom() 
{
   DoCustom ("File Transfer Protocol", &m_ftpHelper);
}

void THotlinkPage::OnOptionsHotlinkFtpUsereg() 
{
   CButton *pWnd = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_FTP_USEREG);
   CButton *pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_FTP_CUSTOM);
   pButton->EnableWindow (!pWnd->GetCheck());
   
}

void THotlinkPage::OnOptionsHotlinkGopherCustom() 
{
   DoCustom ("Gopher", &m_gopherHelper);
   
}

void THotlinkPage::OnOptionsHotlinkGopherUsereg() 
{
   CButton *pWnd = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_GOPHER_USEREG);
   CButton *pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_GOPHER_CUSTOM);
   pButton->EnableWindow (!pWnd->GetCheck());
   
}

void THotlinkPage::OnOptionsHotlinkMailCustom() 
{
   
}

void THotlinkPage::OnOptionsHotlinkTelnetCustom() 
{
DoCustom ("Telnet", &m_telnetHelper);
   
}

void THotlinkPage::OnOptionsHotlinkTelnetUsereg() 
{
   CButton *pWnd = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_TELNET_USEREG);
   CButton *pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_TELNET_CUSTOM);
   pButton->EnableWindow (!pWnd->GetCheck());
}

void THotlinkPage::OnOptionsHotlinkWebCustom() 
{
   DoCustom ("World Wide Web", &m_webHelper);
}

void THotlinkPage::OnOptionsHotlinkMailUsereg1() 
{
   
}

void THotlinkPage::OnOptionsHotlinkWebUsereg() 
{
   CButton *pWnd = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_WEB_USEREG);
   CButton *pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_WEB_CUSTOM);
   pButton->EnableWindow (!pWnd->GetCheck());
}

BOOL THotlinkPage::OnInitDialog() 
{
   CPropertyPage::OnInitDialog();
   CWnd *pWnd;
   
   // TODO: Add extra initialization here

   if (gdwOSMajor >= 4)
      {
      if (m_fWebUseReg)
         {
         pWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_WEB_CUSTOM);
         pWnd->EnableWindow (FALSE);
         }

      if (m_fFtpUseReg)
         {
         pWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_FTP_CUSTOM);
         pWnd->EnableWindow (FALSE);
         }

      if (m_fGopherUseReg)
         {
         pWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_GOPHER_CUSTOM);
         pWnd->EnableWindow (FALSE);
         }

      if (m_fTelnetUseReg)
         {
         pWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_TELNET_CUSTOM);
         pWnd->EnableWindow (FALSE);
         }
      }
   else
      {
      // turn off and disable check boxes, disable mail registry
      // setting
      CButton *pButton;
      pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_MAIL_USEREG1);
      pButton->SetCheck (FALSE);
      pButton->EnableWindow (FALSE);
      pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_MAIL_CUSTOM);
      pButton->SetCheck (TRUE);
      pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_WEB_USEREG);
      pButton->SetCheck (FALSE);
      pButton->EnableWindow (FALSE);
      pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_FTP_USEREG);
      pButton->SetCheck (FALSE);
      pButton->EnableWindow (FALSE);
      pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_GOPHER_USEREG);
      pButton->SetCheck (FALSE);
      pButton->EnableWindow (FALSE);
      pButton = (CButton *) GetDlgItem (IDC_OPTIONS_HOTLINK_TELNET_USEREG);
      pButton->SetCheck (FALSE);
      pButton->EnableWindow (FALSE);
      }
   
   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH THotlinkPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
   HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
   
   // TODO: Change any attributes of the DC here
   CWnd *pSampWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_SAMPLE);
   BOOL  fUnderline = ((CButton *) GetDlgItem(IDC_OPTIONS_HOTLINK_UNDERLINE))->GetCheck();
   if (nCtlColor == CTLCOLOR_STATIC && pSampWnd->m_hWnd == pWnd->m_hWnd)
      {
      pDC->SetTextColor (m_colorRef);

      CFont *pFont;
      LOGFONT  lf;

      pFont = pSampWnd->GetFont();
      pFont->GetObject (sizeof (lf), &lf);
      lf.lfUnderline = fUnderline;
      CFont font;
      font.CreateFontIndirect (&lf);
      pDC->SelectObject (&font);
      }

   // TODO: Return a different brush if the default is not desired
   return hbr;
}

void THotlinkPage::OnOptionsHotlinkUnderline() 
{
   // so we can draw our sample...
   CWnd *pWnd = GetDlgItem (IDC_OPTIONS_HOTLINK_SAMPLE);
   pWnd->Invalidate();
   
}

void THotlinkPage::OnOptionsHotlinkRecognize() 
{

   TRxDlg   dialog;
   
   dialog.m_web               = m_webPattern;
   dialog.m_ftp               = m_ftpPattern;
   dialog.m_gopher            = m_gopherPattern;
   dialog.m_telnet            = m_telnetPattern;
   dialog.m_mail              = m_mailToPattern;

   dialog.m_highlightMail    = m_fHighlightMail;
   dialog.m_highlightWeb     = m_fHighlightWeb;
   dialog.m_highlightFtp     = m_fHighlightFtp;
   dialog.m_highlightGopher  = m_fHighlightGopher;
   dialog.m_highlightTelnet  = m_fHighlightTelnet;
   
   if (IDOK == dialog.DoModal())
      {
      m_webPattern       =    dialog.m_web;
      m_ftpPattern       =    dialog.m_ftp;
      m_gopherPattern    =    dialog.m_gopher;
      m_telnetPattern    =    dialog.m_telnet;
      m_mailToPattern    =    dialog.m_mail;
      m_fHighlightMail   =    dialog.m_highlightMail;
      m_fHighlightWeb    =    dialog.m_highlightWeb;
      m_fHighlightFtp    =    dialog.m_highlightFtp;
      m_fHighlightGopher =    dialog.m_highlightGopher;
      m_fHighlightTelnet =    dialog.m_highlightTelnet;
      }  
}

BOOL THotlinkPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   AfxGetApp () -> HtmlHelp((DWORD)"article-urls-and-hotlinks.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_URL_HOTLINKS);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void THotlinkPage::OnPSNHelp (NMHDR *, LRESULT *)
{
   OnHelpInfo (NULL);
}
