/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: toperpg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:11  richard_wood
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

// toperpg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"            // IDD_OPTIONS_OPERATION
#include "toperpg.h"
#include "news.h"                // CNewsApp::fnAutoCycleWarning
#include "tmsgbx.h"              // NewsMessageBox
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOperationPage property page

IMPLEMENT_DYNCREATE(TOperationPage, CPropertyPage)

TOperationPage::TOperationPage() : CPropertyPage(TOperationPage::IDD)
{

	m_fAutoCycle = FALSE;
	m_iCycleMinutes = 0;
	m_fFullCrossPost = FALSE;
	m_fAutoGetTags = FALSE;
	m_fVerifyLocal = FALSE;
	m_fSubscribeDownload = FALSE;
	m_fLimitHeaders = FALSE;
	m_iHeaderLimit = 0;
	m_fNewsFarm = FALSE;
}

TOperationPage::~TOperationPage()
{
}

void TOperationPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_LIMITTXT2, m_sTextLimit2);
	DDX_Control(pDX, IDC_STATIC_LIMITTXT, m_sTextLimit1);
	DDX_Control(pDX, IDC_EDIT_LIMITNUM, m_sEbxLimit);
	DDX_Control(pDX, IDC_CONNECT_SPIN, m_sUpdateMinutesSpin);
	DDX_Control(pDX, IDC_FULL_CROSSPOSTINFO, m_sFullCrossPost);
	DDX_Check(pDX, IDC_CONNECT_AUTOCYCLE_CBX, m_fAutoCycle);
	DDX_Text(pDX, IDC_CONNECT_EVERY_EDIT, m_iCycleMinutes);
	DDV_MinMaxInt(pDX, m_iCycleMinutes, 10, 360);
	DDX_Check(pDX, IDC_FULL_CROSSPOSTINFO, m_fFullCrossPost);
	DDX_Check(pDX, IDC_CONNECT_AUTOGETTAGS_CBX, m_fAutoGetTags);
	DDX_Check(pDX, IDC_VERIFY_LOCAL, m_fVerifyLocal);
	DDX_Check(pDX, IDC_CBX_SUBSCRIBE_DL, m_fSubscribeDownload);
	DDX_Check(pDX, IDC_CHECK_LIMITHDRS, m_fLimitHeaders);
	DDX_Text(pDX, IDC_EDIT_LIMITNUM, m_iHeaderLimit);
	DDV_MinMaxInt(pDX, m_iHeaderLimit, 1, 40000000);
	DDX_Check(pDX, IDC_MSGS_NEWSFARM, m_fNewsFarm);


   if (!pDX->m_bSaveAndValidate)
      {
      OnAutocycleCbx();
      }

   if (pDX->m_bSaveAndValidate)
      {
      HWND hCycle = pDX->PrepareCtrl (IDC_CONNECT_AUTOCYCLE_CBX);
      if (CNewsApp::fnAutoCycleWarning (this, NULL))
         {
         CString part1, part2;
         part1.LoadString (IDS_WARN_AUTOCYCLECATCHUP);
         part2.LoadString (IDS_WARN_AUTOCYCLECATCHUP2);

         part1 += part2;
         NewsMessageBox (this, part1, MB_OK | MB_ICONWARNING);
         pDX->Fail ();
         }
      }
}

BEGIN_MESSAGE_MAP(TOperationPage, CPropertyPage)
		ON_BN_CLICKED(IDC_CONNECT_AUTOCYCLE_CBX, OnAutocycleCbx)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_CHECK_LIMITHDRS, OnCheckLimitHdrs)

   ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOperationPage message handlers

BOOL TOperationPage::OnInitDialog()
{
   CPropertyPage::OnInitDialog();

   m_sUpdateMinutesSpin.SetRange (10, 360);

   #ifdef LITE
   m_sFullCrossPost.EnableWindow (FALSE);
   #endif

   GreyControls ();

   return TRUE;  // return TRUE unless you set the focus to a control
}

//-------------------------------------------------------------------------
//
void TOperationPage::OnAutocycleCbx()
{
	BOOL fChecked = IsDlgButtonChecked (IDC_CONNECT_AUTOCYCLE_CBX);
   CWnd* pEvery = GetDlgItem (IDC_CONNECT_CYCLE_EVERY_DESC);
   CWnd* pEdit = GetDlgItem (IDC_CONNECT_EVERY_EDIT);
   CWnd* pSpin = GetDlgItem (IDC_CONNECT_SPIN);
   CWnd* pMins = GetDlgItem (IDC_STATIC_MINUTES);

   pEvery->EnableWindow (fChecked);
   pEdit ->EnableWindow (fChecked);
   pSpin ->EnableWindow (fChecked);
   pMins ->EnableWindow (fChecked);
}

BOOL TOperationPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
   AfxGetApp () -> HtmlHelp((DWORD)"servers-operation-tab.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_OPERATION);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOperationPage::OnPSNHelp (NMHDR *, LRESULT *)
{
   OnHelpInfo (NULL);
}

// -------------------------------------------------------------------------
void TOperationPage::OnCheckLimitHdrs ()
{
   UpdateData ();
   GreyControls ();
}

// -------------------------------------------------------------------------
void TOperationPage::GreyControls ()
{
	m_sTextLimit1.EnableWindow(m_fLimitHeaders);
	m_sEbxLimit.EnableWindow(m_fLimitHeaders);
	m_sTextLimit2.EnableWindow(m_fLimitHeaders);
}

