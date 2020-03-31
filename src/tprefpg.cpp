/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tprefpg.cpp,v $
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

// tprefpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "tprefpg.h"
#include "helpcode.h"            // HID_OPTIONS_*
#include "tmsgbx.h"              // NewsMessageBox

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TPreferencesPage property page

IMPLEMENT_DYNCREATE(TPreferencesPage, CPropertyPage)

TPreferencesPage::TPreferencesPage() : CPropertyPage(TPreferencesPage::IDD)
{

	m_f1ClickOpenGroup = FALSE;
	m_iCatchupLocal = -1;
	m_iCatchupNoCPM = -1;
	m_iCatchupKeepTags = FALSE;
	m_iCatchupLoadNext = FALSE;
	m_f1ClickOpenArt = FALSE;
	m_iCatchupOnRtrv = FALSE;
	m_fURLDefaultReader = FALSE;
	m_fFocusInArtpane = FALSE;
	m_maxLines = 0;
	m_maxLinesCmd = -1;
}

TPreferencesPage::~TPreferencesPage()
{
}

void TPreferencesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_OPTPREFS_ONECLIKGROUP, m_f1ClickOpenGroup);
	DDX_Radio(pDX, IDC_CATCHUP_LOCAL, m_iCatchupLocal);
	DDX_Radio(pDX, IDC_CATCHUP_NOCPM, m_iCatchupNoCPM);
	DDX_Check(pDX, IDC_OPTPREFS_CATCHUP_KEEPTAGS, m_iCatchupKeepTags);
	DDX_Check(pDX, IDC_OPTPREFS_CATCHUP_LOADNEXT, m_iCatchupLoadNext);
	DDX_Check(pDX, IDC_OPTPREFS_ONECLIKART, m_f1ClickOpenArt);
	DDX_Check(pDX, IDC_OPTPREFS_CATCHUP_RTRV, m_iCatchupOnRtrv);
	DDX_Check(pDX, IDC_PREFS_URLHANDLER, m_fURLDefaultReader);
	DDX_Check(pDX, IDC_FOCUS_IN_ARTPANE, m_fFocusInArtpane);
	DDX_Text(pDX, IDC_MAXLINES, m_maxLines);
	DDX_CBIndex(pDX, IDC_MAXLINES_CMD, m_maxLinesCmd);


	if (pDX->m_bSaveAndValidate)
	{
		HWND hWndCtrl = pDX->PrepareCtrl (IDC_OPTPREFS_CATCHUP_RTRV);
		if (CNewsApp::fnAutoCycleWarning (NULL, this))
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

BEGIN_MESSAGE_MAP(TPreferencesPage, CPropertyPage)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL TPreferencesPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

#ifdef LITE
	CWnd *pWnd;
	pWnd = GetDlgItem (IDC_CATCHUP_NOCPM);
	pWnd -> EnableWindow (FALSE);
	pWnd = GetDlgItem (IDC_CATCHUP_CPM);
	pWnd -> EnableWindow (FALSE);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
BOOL TPreferencesPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp () -> HtmlHelp((DWORD)"preferences-intro.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_PREFERENCES);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TPreferencesPage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
