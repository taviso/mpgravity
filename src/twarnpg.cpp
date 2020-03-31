/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: twarnpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
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
/*  Revision 1.2  2008/09/19 14:52:22  richard_wood
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

// twarnpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "twarnpg.h"
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TWarningPage property page

IMPLEMENT_DYNCREATE(TWarningPage, CPropertyPage)

TWarningPage::TWarningPage() : CPropertyPage(TWarningPage::IDD)
{
   
   m_fCatchup = FALSE;
   m_fExitCompose = FALSE;
   m_fExitNews32 = FALSE;
   m_fMarkRead = FALSE;
   m_fUnsubscribe = FALSE;
   m_fWarnOnSending = FALSE;
   m_fWarnOnDeleteBinary = FALSE;
   m_fErrorDuringDecode = FALSE;
   m_fWarnOnRunExe = FALSE;
   m_fManualRuleOffline = FALSE;
   m_extensions = _T("");
	m_fWarnOnDeleteArticle = FALSE;
}

TWarningPage::~TWarningPage()
{
}

void TWarningPage::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
   
   DDX_Check(pDX, IDC_WARNINGS_CATCHUP, m_fCatchup);
   DDX_Check(pDX, IDC_WARNINGS_COMPOSE, m_fExitCompose);
   DDX_Check(pDX, IDC_WARNINGS_EXITNEWS32, m_fExitNews32);
   DDX_Check(pDX, IDC_WARNINGS_MARKREAD, m_fMarkRead);
   DDX_Check(pDX, IDC_WARNINGS_UNSUBSCRIBE, m_fUnsubscribe);
   DDX_Check(pDX, IDC_WARNINGS_SEND, m_fWarnOnSending);
   DDX_Check(pDX, IDC_WARNINGS_DELETE_IMAGE, m_fWarnOnDeleteBinary);
   DDX_Check(pDX, IDC_WARNINGS_DECODE_ERROR, m_fErrorDuringDecode);
   DDX_Check(pDX, IDC_WARNINGS_MANRULE_OFFLINE, m_fManualRuleOffline);
   DDX_Text(pDX, IDC_WARN_EXT, m_extensions);
	DDX_Check(pDX, IDC_WARNONDELETE, m_fWarnOnDeleteArticle);
}

BEGIN_MESSAGE_MAP(TWarningPage, CPropertyPage)
      ON_WM_HELPINFO()
   
   ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL TWarningPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
   AfxGetApp () -> HtmlHelp((DWORD)"preferences-warnings.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_WARNINGS);
   return 1;
}

// -------------------------------------------------------------------------
afx_msg void TWarningPage::OnPSNHelp (NMHDR *, LRESULT *)
{
   OnHelpInfo (NULL);
}
