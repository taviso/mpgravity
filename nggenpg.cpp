/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: nggenpg.cpp,v $
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:36  richard_wood
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

// nggenpg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "nggenpg.h"
#include <dlgs.h>
#include "fontdlg.h"
#include "tglobopt.h"
#include "sysclr.h"
#include "gdiutil.h"
#include "nglist.h"              // TNewsGroupArray
#include "fileutil.h"            // NewsMessageBox
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "gdiutil.h"             // gdiCustomColorDlg

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TSystem gSystem;
extern TGlobalOptions * gpGlobalOptions;

/////////////////////////////////////////////////////////////////////////////
// TNewGroupGeneralOptions property page

IMPLEMENT_DYNCREATE(TNewGroupGeneralOptions, CPropertyPage)

TNewGroupGeneralOptions::TNewGroupGeneralOptions()
: CPropertyPage(TNewGroupGeneralOptions::IDD)
{

	m_nickname = _T("");
	m_kStorageOption = -1;
	m_iHighestArticleRead = -1;
	m_fStorageBodies = FALSE;
	m_fDiv100 = FALSE;
	m_bSample = FALSE;
	m_fInActive = FALSE;


	// default text color
	m_newsgroupColor = gSystem.WindowText();
}

TNewGroupGeneralOptions::~TNewGroupGeneralOptions()
{
}

static void DDV_NewsGroupName (CDataExchange* pDX, CString &strNickname,
							   CString &strGroupName)
{
	if (!pDX -> m_bSaveAndValidate || !strNickname.GetLength ())
		return;

	int bDuplicate = FALSE;

	TServerCountedPtr cpNewsServer;
	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vNewsGroups);

	int iTotal = vNewsGroups -> GetSize ();
	for (int i = 0; i < iTotal; i++) {
		TNewsGroup *psGroup = vNewsGroups[i];
		if (psGroup -> GetNickname () == strNickname &&
			psGroup -> GetName () != strGroupName) {
				bDuplicate = TRUE;
				break;
		}
	}

	if (bDuplicate) {
		CString strError;
		strError; strError.LoadString (IDS_ERR_NICKNAME_UNIQUE);
		AfxMessageBox (strError, MB_ICONEXCLAMATION);
		strError.Empty ();     // exception prep
		pDX -> Fail ();
	}
}

//-------------------------------------------------------------------------
//
BOOL TNewGroupGeneralOptions::IsGobackValid ()
{
	if ((-1 == m_iHighestArticleRead0) &&
		((m_lowWater < m_iServerLow) || (m_lowWater > m_iServerHi)))
		return FALSE;
	if ((m_iServerLow > m_iHighestArticleRead0) &&
		(m_iServerLow >= m_lowWater))
		// the server articles have moved past us
		return FALSE;

	return TRUE;
}

// transfer enum to 2 Booleans
void TNewGroupGeneralOptions::ExchangeStorageMode (CDataExchange * pDX)
{
	if (FALSE == pDX->m_bSaveAndValidate)
	{
		if (TNewsGroup::kStoreBodies == m_kStorageOption)
			m_fStorageBodies  = TRUE;
		else
			m_fStorageBodies  = FALSE;

	}
	else
	{
		if (m_fStorageBodies)
			m_kStorageOption = TNewsGroup::kStoreBodies;
		else
			m_kStorageOption = TNewsGroup::kHeadersOnly;

		// Warn about transitions
		warn_about_transition (pDX);
	}
}

//-------------------------------------------------------------------------
// Warn about transitions
// Mode1 -> Mode2    (OK)
// Mode1 -> Mode3    (OK)
// Mode2 -> Mode1    Lose Headers
// Mode2 -> Mode3    (OK)
// Mode3 -> Mode1    Keep any Bodies we already have.
//                   Destroy any hdrs w/o matchin Body
//
// Mode3 -> Mode2    (OK) Keep any Bodies we already have
//
void TNewGroupGeneralOptions::warn_about_transition (CDataExchange *pDX)
{
	BOOL fWarn = FALSE;
	if ((int) TNewsGroup::kHeadersOnly == m_iOriginalMode &&
		TNewsGroup::kNothing == m_kStorageOption)
	{
		fWarn = TRUE;
	}
	if ((int) TNewsGroup::kStoreBodies == m_iOriginalMode &&
		TNewsGroup::kNothing == m_kStorageOption)
	{
		fWarn = TRUE;
	}

	if (fWarn)
	{
		// are you sure you want to lose Mode2 headers?
		if (IDYES == NewsMessageBox(this, IDS_WARN_CHANGEMODE1, MB_YESNO | MB_ICONWARNING))
		{
			; // continue
		}
		else
		{
			pDX->Fail ();   // throw exception and stop
		}
	}
}

//-------------------------------------------------------------------------
void TNewGroupGeneralOptions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	// enum to booleans
	if (FALSE == pDX->m_bSaveAndValidate)
		ExchangeStorageMode (pDX);


	DDX_Control(pDX, IDC_NGOPT_GENERAL_ART_SLIDER, m_highArtSlider);
	DDX_Text(pDX, IDC_NICKNAME_TXT, m_nickname);
	DDV_NewsGroupName(pDX, m_nickname, m_strGroupName);
	DDX_Check(pDX, IDC_GENERAL_STORE_BODIES,  m_fStorageBodies);
	DDX_Check(pDX, IDC_NGOPT_DEFBKGRND, m_fDefaultBackground);
	DDX_Check(pDX, IDC_NGOPT_SAMPLE, m_bSample);
	DDX_Check(pDX, IDC_NGOPT_INACTIVE, m_fInActive);


	// vc4.2 classWizard can't parse this
	if (IsGobackValid())
	{
		DDX_Text(pDX, IDC_NGOPT_RETRIEVE, m_iHighestArticleRead);

		// lower bound - the lowest number on the server
		// upper bound - min(highest read, highest number on the server)
		int nUpperBnd;

		nUpperBnd = min(m_iHighestArticleRead0, m_iServerHi);

		// if server has 1-10, then you can reset to Zero, thus the -1
		DDV_MinMaxInt(pDX, m_iHighestArticleRead, m_iServerLow-1, nUpperBnd);
	}

	if (FALSE == pDX->m_bSaveAndValidate)
		OnNgoptDefbkgrnd();

	// booleans to enum
	if (pDX->m_bSaveAndValidate)
		ExchangeStorageMode (pDX);
}

void TNewGroupGeneralOptions::OnOK(void)
{
}

BEGIN_MESSAGE_MAP(TNewGroupGeneralOptions, CPropertyPage)
	ON_BN_CLICKED(IDC_NGOPT_FONT, OnNgoptFont)
	ON_BN_CLICKED(IDC_NGOPT_DEFFONT, OnNgoptDeffont)
	ON_BN_CLICKED(IDC_NGOPT_BACKGROUND, OnNgoptBackground)
	ON_BN_CLICKED(IDC_NGOPT_DEFBKGRND, OnNgoptDefbkgrnd)
	ON_WM_HSCROLL()
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TNewGroupGeneralOptions message handlers

BOOL TNewGroupGeneralOptions::OnInitDialog()
{
	// remember initial value
	if (-1 == m_iHighestArticleRead)
		m_iHighestArticleRead0 = m_iHighestArticleRead = m_lowWater;
	else
		m_iHighestArticleRead0 = m_iHighestArticleRead;

	CPropertyPage::OnInitDialog();

	//if (m_fUseGlobalStorageOptions)
	//   EnableDisable (FALSE);

	// if user has never read any articles, disable
	if (!IsGobackValid())
	{
		GetDlgItem(IDC_NGOPT_RETRIEVEDESC)->EnableWindow(FALSE);
		GetDlgItem(IDC_NGOPT_RETRIEVE)->SetWindowText("");
		GetDlgItem(IDC_NGOPT_RETRIEVE)->EnableWindow(FALSE);
		GetDlgItem (IDC_NGOPT_GENERAL_HIGHSERVERART)->SetWindowText ("");
		GetDlgItem (IDC_NGOPT_GENERAL_LOWSERVERART)->SetWindowText ("");
		GetDlgItem(IDC_NGOPT_GENERAL_LOWSERVERART)->EnableWindow(FALSE);
		GetDlgItem(IDC_NGOPT_GENERAL_HIGHSERVERART)->EnableWindow(FALSE);
		GetDlgItem(IDC_NGOPT_GENERAL_ART_SLIDER)->EnableWindow(FALSE);
	}
	else
	{
		CString  temp;

		// slider control only supports ranges as big as
		// a word, and the GetPos function returns an int
		// so to be on the safe side, we divide everything
		// above that by 100

		if (-1 != m_iHighestArticleRead0)
			m_iBarHi = m_iHighestArticleRead0 - m_iServerLow + 1;
		else
			m_iBarHi = m_lowWater - m_iServerLow + 1;

		if ( m_iBarHi > 32000)
		{
			m_fDiv100 = TRUE;
			m_iBarHi /= 100;
		}

		m_highArtSlider.SetRange (0 , m_iBarHi);

		m_highArtSlider.SetPos (m_iBarHi);

		temp.Format ("%d", m_iServerLow - 1);
		GetDlgItem (IDC_NGOPT_GENERAL_LOWSERVERART)->SetWindowText (temp);
		temp.Format ("%d", m_iHighestArticleRead0);
		GetDlgItem (IDC_NGOPT_GENERAL_HIGHSERVERART)->SetWindowText (temp);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void TNewGroupGeneralOptions::OnNgoptFont()
{
	CNewFontDialog dlgFont(m_newsgroupBackground);
	LOGFONT        lf;

	CopyMemory (&lf, &m_ngFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY |
		CF_EFFECTS | CF_FORCEFONTEXIST ;

	// might be using the default background
	dlgFont.m_cf.rgbColors = m_newsgroupColor;
	if (IsDlgButtonChecked(IDC_NGOPT_DEFBKGRND))
		dlgFont.SetBackground (gSystem.Window());

	if (IDOK == dlgFont.DoModal())
	{
		m_newsgroupColor = dlgFont.GetColor();
		CopyMemory (&m_ngFont, &lf, sizeof(m_ngFont) );
		//UpdateBanner (IDC_SHOW_NGFONT, &lf, dlgFont.GetSize() / 10);
		m_fCustomNGFont = TRUE;
	}
}

void TNewGroupGeneralOptions::OnNgoptDeffont()
{
	m_fCustomNGFont = FALSE;
	m_newsgroupColor = gSystem.WindowText();        // default text color

	CopyMemory (&m_ngFont, &gpGlobalOptions->m_defNGFont, sizeof(LOGFONT));
}

// 4-24-98  allow custom colors
void TNewGroupGeneralOptions::OnNgoptBackground()
{
	COLORREF crfOutput;

	if (gdiCustomColorDlg (this, m_newsgroupBackground, crfOutput))
		m_newsgroupBackground = crfOutput;
}

void TNewGroupGeneralOptions::OnNgoptDefbkgrnd()
{
	GetDlgItem (IDC_NGOPT_BACKGROUND)->EnableWindow (!IsDlgButtonChecked(IDC_NGOPT_DEFBKGRND));
}

void TNewGroupGeneralOptions::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	if (nSBCode == TB_THUMBTRACK)
	{
		CString pos;

		if (m_fDiv100)
			pos.Format ("%d", (m_iServerLow - 1) + (nPos * 100));
		else
			pos.Format ("%d", (m_iServerLow - 1) + nPos);

		GetDlgItem(IDC_NGOPT_RETRIEVE)->SetWindowText(pos);
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL TNewGroupGeneralOptions::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"group-properties-general-tab.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TNewGroupGeneralOptions::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
