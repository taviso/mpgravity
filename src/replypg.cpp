/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: replypg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:43  richard_wood
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

// replypg.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "replypg.h"
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsReplyDlg property page

IMPLEMENT_DYNCREATE(TOptionsReplyDlg, CPropertyPage)

TOptionsReplyDlg::TOptionsReplyDlg() : CPropertyPage(TOptionsReplyDlg::IDD)
{

	m_folIntro = _T("");
	m_rplyIntro = _T("");
	m_fPasteOnFollowup = FALSE;
	m_fPasteOnReply = FALSE;
	m_fLimitQuoted = FALSE;
	m_maxQuoteLines = _T("");
	m_CCIntro = _T("");
}

TOptionsReplyDlg::~TOptionsReplyDlg()
{
}

void TOptionsReplyDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_FOLINTRO, m_folIntro);
	DDX_Text(pDX, IDC_EDIT_RPLYINTRO, m_rplyIntro);
	DDX_Check(pDX, IDC_POSTING_PASTE_FOLLOW, m_fPasteOnFollowup);
	DDX_Check(pDX, IDC_POSTING_PASTEORIG, m_fPasteOnReply);
	DDX_Check(pDX, IDC_POSTING_LIMITQUOTED, m_fLimitQuoted);
	DDX_Text(pDX, IDC_POSTING_QUOTELINES, m_maxQuoteLines);
	DDX_Text(pDX, IDC_EDIT_CCINTRO, m_CCIntro);
}

BEGIN_MESSAGE_MAP(TOptionsReplyDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_POSTING_LIMITQUOTED, OnPostingLimitQuoted)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsReplyDlg message handlers
void TOptionsReplyDlg::OnPostingLimitQuoted()
{
	CButton * pButton = (CButton *) GetDlgItem (IDC_POSTING_LIMITQUOTED);
	if (pButton->GetCheck())
	{
		pButton = (CButton *) GetDlgItem (IDC_POSTING_SPINDESC);
		pButton->EnableWindow (TRUE);
		pButton = (CButton *) GetDlgItem (IDC_POSTING_QUOTELINES);
		pButton->EnableWindow (TRUE);
		pButton = (CButton *) GetDlgItem (IDC_POSTING_QUOTESPIN);
		pButton->EnableWindow (TRUE);
	}
	else
	{
		pButton = (CButton *) GetDlgItem (IDC_POSTING_SPINDESC);
		pButton->EnableWindow (FALSE);
		pButton = (CButton *) GetDlgItem (IDC_POSTING_QUOTELINES);
		pButton->EnableWindow (FALSE);
		pButton = (CButton *) GetDlgItem (IDC_POSTING_QUOTESPIN);
		pButton->EnableWindow (FALSE);
	}
}

BOOL TOptionsReplyDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CSpinButtonCtrl * pSpin = (CSpinButtonCtrl *) GetDlgItem (IDC_POSTING_QUOTESPIN);
	pSpin->SetRange (0, 1000);

	CButton * pButton;
	// TODO: Add extra initialization here
	if (!m_fLimitQuoted)
	{
		pButton = (CButton *) GetDlgItem (IDC_POSTING_SPINDESC);
		pButton->EnableWindow (FALSE);
		pButton = (CButton *) GetDlgItem (IDC_POSTING_QUOTELINES);
		pButton->EnableWindow (FALSE);
		pButton = (CButton *) GetDlgItem (IDC_POSTING_QUOTESPIN);
		pButton->EnableWindow (FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL TOptionsReplyDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"posting_global-options-replying.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_REPLYING);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsReplyDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
