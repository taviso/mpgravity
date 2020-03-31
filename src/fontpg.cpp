/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fontpg.cpp,v $
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
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
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
/*  Revision 1.2  2008/09/19 14:51:25  richard_wood
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

// fontpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "fontpg.h"
#include "gdiutil.h"
#include <dlgs.h>
#include "fontdlg.h"
#include "helpcode.h"            // HID_OPTIONS_*

#include "tglobopt.h"
#include "sysclr.h"
//#include "pktxtclr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions* gpGlobalOptions;
extern TSystem gSystem;

/////////////////////////////////////////////////////////////////////////////
// TOptionsFontDlg property page

IMPLEMENT_DYNCREATE(TOptionsFontDlg, CPropertyPage)

TOptionsFontDlg::TOptionsFontDlg() : CPropertyPage(TOptionsFontDlg::IDD)
{


	m_newsgroupColor = 0;
	m_treeColor = 0;
}

TOptionsFontDlg::~TOptionsFontDlg()
{
}

void TOptionsFontDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);


	DDX_Check(pDX, IDC_OPTIONS_FONT_NGDEFBK,    m_fDefaultNewsgroupBG);
	DDX_Check(pDX, IDC_OPTIONS_FONT_THREADDEFBK,m_fDefaultThreadBG);


	if (FALSE == pDX->m_bSaveAndValidate)
	{
		int points;
		CDC* pdc = GetDC();
		fnCalcPointSize ( pdc, &m_ngFont, &points );
		UpdateBanner ( IDC_SHOW_NGFONT, &m_ngFont, points );

		fnCalcPointSize (pdc, &m_treeFont, &points );
		UpdateBanner (IDC_SHOW_TRFONT, &m_treeFont, points );

		ReleaseDC (pdc);
		OnOptionsFontNgdefbk();
		OnOptionsFontThreaddefbk();
	}
}

BEGIN_MESSAGE_MAP(TOptionsFontDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_BUT_NGFONT, OnButNgfont)
	ON_BN_CLICKED(IDC_BUT_NGDEFFONT, OnButNgdeffont)
	ON_BN_CLICKED(IDC_BUT_TRDEFFONT, OnButTrdeffont)
	ON_BN_CLICKED(IDC_BUT_TRFONT, OnButTrfont)
	ON_BN_CLICKED(IDC_OPTIONS_FONT_NGRP_BACKGROUND, OnOptionsFontNgrpBackground)
	ON_BN_CLICKED(IDC_OPTIONS_FONT_THREAD_BACKGROUND, OnOptionsFontThreadBackground)
	ON_BN_CLICKED(IDC_OPTIONS_FONT_NGDEFBK, OnOptionsFontNgdefbk)
	ON_BN_CLICKED(IDC_OPTIONS_FONT_THREADDEFBK, OnOptionsFontThreaddefbk)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsFontDlg message handlers

void TOptionsFontDlg::OnButNgdeffont()
{
	m_fCustomNGFont = FALSE;
	m_newsgroupColor = gSystem.WindowText();        // default text color

	CopyMemory (&m_ngFont, &gpGlobalOptions->m_defNGFont, sizeof(LOGFONT));
	int points;
	CDC* pdc = GetDC();
	if (pdc)
	{
		fnCalcPointSize ( pdc, &m_ngFont, &points );
		UpdateBanner ( IDC_SHOW_NGFONT, &m_ngFont, points );
		ReleaseDC (pdc);
	}
}

void TOptionsFontDlg::OnButNgfont()
{
	CNewFontDialog dlgFont(m_newsgroupBackground);
	LOGFONT        lf;

	CopyMemory (&lf, &m_ngFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY |
		CF_EFFECTS | CF_FORCEFONTEXIST ;
	dlgFont.m_cf.rgbColors = m_newsgroupColor;

	// are we using the system background?
	if (IsDlgButtonChecked(IDC_OPTIONS_FONT_NGDEFBK))
		dlgFont.SetBackground (gSystem.Window());

	if (IDOK == dlgFont.DoModal())
	{
		m_newsgroupColor = dlgFont.GetColor();
		CopyMemory (&m_ngFont, &lf, sizeof(m_ngFont) );
		UpdateBanner (IDC_SHOW_NGFONT, &lf, dlgFont.GetSize() / 10);
		m_fCustomNGFont = TRUE;
	}
}

void TOptionsFontDlg::OnButTrdeffont()
{
	m_fCustomTreeFont = FALSE;
	CopyMemory (&m_treeFont, &gpGlobalOptions->m_defTreeFont, sizeof(LOGFONT));
	m_treeColor = gSystem.WindowText();        // default text color

	int points;
	CDC* pdc = GetDC();
	if (pdc)
	{
		fnCalcPointSize (pdc, &m_treeFont, &points );
		UpdateBanner (IDC_SHOW_TRFONT, &m_treeFont, points );
		ReleaseDC (pdc);
	}
}

void TOptionsFontDlg::OnButTrfont()
{
	CNewFontDialog dlgFont(m_threadBackground);
	LOGFONT        lf;

	CopyMemory (&lf, &m_treeFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY | CF_FORCEFONTEXIST ;
	dlgFont.m_cf.rgbColors = m_treeColor;

	if (IsDlgButtonChecked(IDC_OPTIONS_FONT_THREADDEFBK))
		dlgFont.SetBackground (gSystem.Window());

	if (IDOK == dlgFont.DoModal())
	{
		m_treeColor = dlgFont.GetColor();
		CopyMemory (&m_treeFont, &lf, sizeof(m_treeFont) );
		UpdateBanner (IDC_SHOW_TRFONT, &lf, dlgFont.GetSize() / 10);
		m_fCustomTreeFont = TRUE;
	}
}

// ?? fix this to show color?
int TOptionsFontDlg::UpdateBanner (int id, LPLOGFONT pLF, int pointsize)
{
	CString num;
	num.Format("%d", pointsize);
	int stringID;
	if (pLF->lfWeight >= FW_BOLD)
	{
		if (pLF->lfItalic)
			stringID = IDS_FONTBANNER_BOLDITALIC;
		else
			stringID = IDS_FONTBANNER_BOLD;
	}
	else
	{
		if (pLF->lfItalic)
			stringID = IDS_FONTBANNER_ITALIC;
		else
			stringID = IDS_FONTBANNER_NORM;
	}
	CString banner;
	AfxFormatString2(banner, stringID, pLF->lfFaceName, num);

	GetDlgItem(id)->SetWindowText ( banner );
	return 0;
}

void TOptionsFontDlg::OnOptionsFontNgrpBackground()
{
	CColorDialog colorDlg(m_newsgroupBackground);
	colorDlg.m_cc.Flags = colorDlg.m_cc.Flags|CC_PREVENTFULLOPEN;

	if (IDOK == colorDlg.DoModal ())
	{
		m_newsgroupBackground = colorDlg.GetColor ();
	}
}

void TOptionsFontDlg::OnOptionsFontThreadBackground()
{
	CColorDialog colorDlg (m_threadBackground);
	colorDlg.m_cc.Flags = colorDlg.m_cc.Flags|CC_PREVENTFULLOPEN;

	if (IDOK == colorDlg.DoModal ())
	{
		m_threadBackground = colorDlg.GetColor ();
	}
}

void TOptionsFontDlg::OnOptionsFontNgdefbk()
{
	CbxCounterButton (IDC_OPTIONS_FONT_NGDEFBK,IDC_OPTIONS_FONT_NGRP_BACKGROUND);
}

void TOptionsFontDlg::CbxCounterButton(int cbx, int buttn)
{
	GetDlgItem (buttn)->EnableWindow (!IsDlgButtonChecked(cbx));
}

void TOptionsFontDlg::OnOptionsFontThreaddefbk()
{
	CbxCounterButton(IDC_OPTIONS_FONT_THREADDEFBK, IDC_OPTIONS_FONT_THREAD_BACKGROUND);
}

BOOL TOptionsFontDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp ()->HtmlHelp((DWORD)"customize-thread-pane.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_FONTS);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsFontDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
