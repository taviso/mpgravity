/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tthredpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:20  richard_wood
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

// TThreadpg.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "TThredpg.h"
#include "sysclr.h"
#include <dlgs.h>
#include "fontdlg.h"
#include "tglobopt.h"
#include "sysclr.h"
#include "gdiutil.h"                // gdiCustomColorDlg

extern TGlobalOptions *gpGlobalOptions;
extern TSystem gSystem;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TThreadPage property page

IMPLEMENT_DYNCREATE(TThreadPage, CPropertyPage)

TThreadPage::TThreadPage() : CPropertyPage(TThreadPage::IDD)
{

	m_sortThread = -1;
	m_fDefaultBackground = FALSE;
}

TThreadPage::~TThreadPage()
{
}

void TThreadPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_CBIndex(pDX, IDC_TREEPROP_SORT_THREAD, m_sortThread);
	DDX_Check(pDX, IDC_TREEPROP_DEFBK, m_fDefaultBackground);


	if (FALSE == pDX->m_bSaveAndValidate)
	{
		OnTreepropDefbk();
	}
}

BEGIN_MESSAGE_MAP(TThreadPage, CPropertyPage)
	ON_BN_CLICKED(IDC_TREEPROP_FONT, OnTreepropFont)
	ON_BN_CLICKED(IDC_TREEPROP_USEDEF, OnTreepropUsedef)
	ON_BN_CLICKED(IDC_TREEPROP_BACKGROUND, OnTreepropBackground)
	ON_BN_CLICKED(IDC_TREEPROP_DEFBK, OnTreepropDefbk)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TThreadPage message handlers

void TThreadPage::OnTreepropFont()
{
	CNewFontDialog dlgFont(m_treeBackground);
	LOGFONT        lf;

	CopyMemory (&lf, &m_treeFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY | CF_FORCEFONTEXIST ;
	dlgFont.m_cf.rgbColors =  m_treeColor;
	if (IsDlgButtonChecked(IDC_TREEPROP_DEFBK))
		dlgFont.SetBackground (gSystem.Window());

	if (IDOK == dlgFont.DoModal())
	{
		m_treeColor = dlgFont.GetColor();
		CopyMemory (&m_treeFont, &lf, sizeof(m_treeFont) );
		m_fCustomTreeFont = TRUE;
	}
}

void TThreadPage::OnTreepropUsedef()
{
	m_fCustomTreeFont = FALSE;
	CopyMemory (&m_treeFont, &gpGlobalOptions->m_defTreeFont, sizeof(LOGFONT));
	m_treeColor = gSystem.WindowText();        // default text color
}

// 4-24-98  allow custom colors
void TThreadPage::OnTreepropBackground()
{
	COLORREF crfOutput;

	if (gdiCustomColorDlg (this, m_treeBackground, crfOutput))
		m_treeBackground = crfOutput;
}

void TThreadPage::OnTreepropDefbk()
{
	GetDlgItem (IDC_TREEPROP_BACKGROUND)->EnableWindow (!IsDlgButtonChecked(IDC_TREEPROP_DEFBK));
}

BOOL TThreadPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TThreadPage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
