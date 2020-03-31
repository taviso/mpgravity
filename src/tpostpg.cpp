/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tpostpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.3  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:14  richard_wood
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

// tpostpg.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "tpostpg.h"
#include "fileutil.h"
#include "mimedlg.h"
#include "helpcode.h"            // HID_OPTIONS_*
#include "nameutil.h"            // DDV_EmailName()
#include "genutil.h"             // DDX_CEditStringList()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsPostingDlg property page

IMPLEMENT_DYNCREATE(TOptionsPostingDlg, CPropertyPage)

TOptionsPostingDlg::TOptionsPostingDlg() : CPropertyPage(TOptionsPostingDlg::IDD)
{
	m_Indent = _T("");
	m_LineWrap = 0;
	m_IDsToRemember = 0;
	m_strDistribution = _T("");
}

TOptionsPostingDlg::~TOptionsPostingDlg()
{
}

void TOptionsPostingDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_POSTING_INDENT, m_Indent);
	DDV_MaxChars(pDX, m_Indent, 6);
	DDX_Text(pDX, IDC_POSTING_LINEWRAP, m_LineWrap);
	DDV_MinMaxInt(pDX, m_LineWrap, 1, 800);
	DDX_Text(pDX, IDC_POSTING_IDS_TO_REMEMBER, m_IDsToRemember);
	DDV_MinMaxInt(pDX, m_IDsToRemember, 0, 250);
	DDX_Text(pDX, IDC_POSTING_DISTRIBUTION, m_strDistribution);
	DDV_MaxChars(pDX, m_strDistribution, 128);

	DDX_CEditStringList(pDX, IDC_POSTING_HEADERS, m_headers);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		Util_UpdateMiniStatus( LPCTSTR(m_logFont.lfFaceName), m_PointSize );
	}
}

BEGIN_MESSAGE_MAP(TOptionsPostingDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_POSTING_CHOOSEFONT, OnPostingChoosefont)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsPostingDlg message handlers

void TOptionsPostingDlg::OnPostingChoosefont()
{
	CFontDialog dlgFont;

	LOGFONT lf;

	CopyMemory(&lf, &m_logFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;

	dlgFont.m_cf.Flags |=
		CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY | CF_FORCEFONTEXIST
		| CF_FIXEDPITCHONLY;

	dlgFont.m_cf.Flags &= ~CF_EFFECTS;

	if (IDOK == dlgFont.DoModal())
	{
		CopyMemory(&m_logFont, &lf, sizeof(LOGFONT));
		CString faceName;
		faceName = dlgFont.GetFaceName();

		// this is in tenths of a point!
		m_PointSize = dlgFont.GetSize();
		Util_UpdateMiniStatus(faceName, m_PointSize);
	}
}

void TOptionsPostingDlg::Util_UpdateMiniStatus(const CString& faceName, int PointSize)
{
	CString ptSize;
	ptSize.Format(_T("%d"), m_PointSize / 10);
	CString result;
	AfxFormatString2(result, IDS_FONT_FORMAT, faceName, ptSize);
	SetDlgItemText(IDC_POSTING_FONT, result);
}

BOOL TOptionsPostingDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CSpinButtonCtrl * pArtNumSpin = (CSpinButtonCtrl *) GetDlgItem(IDC_POSTING_IDS_TO_REMEMBER_SCRL);
	pArtNumSpin->SetRange(0, 250);

	CSpinButtonCtrl * pLineWrapSpin = (CSpinButtonCtrl *) GetDlgItem(IDC_POSTING_LINEWRAPSCRL);
	pLineWrapSpin->SetRange(1, 800);

	return TRUE;
}

BOOL TOptionsPostingDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"posting_global-options-posting.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_POSTING);
	return 1;
}

afx_msg void TOptionsPostingDlg::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
