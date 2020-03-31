/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: turldef.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:21  richard_wood
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

// turldef.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "turldef.h"
#include "turldde.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TURLDef dialog

TURLDef::TURLDef(CWnd* pParent /*=NULL*/)
   : CDialog(TURLDef::IDD, pParent)
{
   
   m_service = _T("");
   m_topic = _T("");
   m_fUseDDE = FALSE;
   m_application = _T("");
   m_data = _T("");
   m_item = _T("");
   m_transType = TUrlDde::kRequest;
   
}

void TURLDef::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   
   DDX_Text(pDX, IDC_URL_DEFINITION_SERVICE, m_service);
   DDX_Text(pDX, IDC_URL_DEFINITION_TOPIC, m_topic);
   DDX_Check(pDX, IDC_URL_DEFINITION_USEDDE, m_fUseDDE);
   DDX_Radio(pDX, IDC_URL_DEFINITION_EXECUTE, m_transType);
   DDX_Text(pDX, IDC_URL_DEFINITION_APP, m_application);
   DDX_Text(pDX, IDC_URL_DEFINITION_DATA, m_data);
   DDX_Text(pDX, IDC_URL_DEFINITION_ITEM, m_item);
   
}

BEGIN_MESSAGE_MAP(TURLDef, CDialog)
      ON_BN_CLICKED(IDC_URL_DEFINITION_USEDDE, OnUrlDefinitionUsedde)
   ON_BN_CLICKED(IDC_URL_DEFINITION_APPBROWSE, OnUrlDefinitionAppbrowse)
   
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TURLDef message handlers

void TURLDef::OnUrlDefinitionUsedde() 
{
   CButton *pButton = (CButton *) GetDlgItem (IDC_URL_DEFINITION_USEDDE);
   EnableDisable (pButton->GetCheck ());
}

void TURLDef::OnUrlDefinitionAppbrowse() 
{
   TCHAR   filter[80];
   LoadString (AfxGetResourceHandle (), IDS_FILTER_URLPROGRAM, filter, sizeof (filter) - 1);
   CFileDialog fileDlg(TRUE,
                       "exe",
                       NULL,
                       OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST,
                       filter);
   if (IDOK == fileDlg.DoModal ())
      {
      CEdit *pEdit = (CEdit *) GetDlgItem (IDC_URL_DEFINITION_APP);
      pEdit->SetWindowText (fileDlg.GetPathName ());
      }
   
}

void TURLDef::EnableDisable (BOOL fEnable)

{
CWnd *pWnd;
pWnd = GetDlgItem (IDC_URL_DEFINITION_SERVICEDESC);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_SERVICE);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_TOPICDESC);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_TOPIC);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_TRANSDESC);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_EXECUTE);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_POKE);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_REQUEST);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_ITEMDESC);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_ITEM);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_DATADESC);
pWnd->EnableWindow (fEnable);
pWnd = GetDlgItem (IDC_URL_DEFINITION_DATA);
pWnd->EnableWindow (fEnable);
}

BOOL TURLDef::OnInitDialog() 
{
   CDialog::OnInitDialog();
   
   // TODO: Add extra initialization here
   if (!m_fUseDDE)
      EnableDisable(FALSE);

   SetWindowText (m_caption);
   
   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}
