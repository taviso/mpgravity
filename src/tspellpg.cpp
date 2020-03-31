/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tspellpg.cpp,v $
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
/*  Revision 1.4  2008/09/19 14:52:19  richard_wood
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

// tspellpg.cpp : implementation file
//
#include "stdafx.h"
#include "news.h"
#include "tspellpg.h"
#include "genutil.h"             // MsgResource()
#include "helpcode.h"
#include "newsdoc.h"
#include "mywords.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TSpellPage property page

IMPLEMENT_DYNCREATE(TSpellPage, CPropertyPage)

TSpellPage::TSpellPage() : CPropertyPage(TSpellPage::IDD)
{
	m_fSpellcheck = FALSE;
	m_strWord = _T("");
	m_strAffinityFile = _T("");
	m_strDictionaryFile = _T("");
	m_fSkipNumbers = FALSE;
}

TSpellPage::~TSpellPage()
{
}

void TSpellPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPELLCHECK, m_sSpellcheck);
	DDX_Control(pDX, IDC_ADD_WORD, m_sAdd);
	DDX_Control(pDX, IDC_REMOVE_WORD, m_sRemove);
	DDX_Control(pDX, IDC_WORDS, m_sWords);
	DDX_Check(pDX, IDC_SPELLCHECK, m_fSpellcheck);
	DDX_Text(pDX, IDC_WORD, m_strWord);
	DDX_Text(pDX, IDC_EBX_SPELAFF, m_strAffinityFile);
	DDX_Text(pDX, IDC_EBX_SPELDIC, m_strDictionaryFile);
	DDX_Check(pDX, IDC_IGNORE_NUMBERS, m_fSkipNumbers);
}

BEGIN_MESSAGE_MAP(TSpellPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ADD_WORD, OnAddWord)
	ON_EN_CHANGE(IDC_WORD, OnChangeWord)
	ON_LBN_SELCHANGE(IDC_WORDS, OnSelchangeWords)
	ON_BN_CLICKED(IDC_REMOVE_WORD, OnRemoveWord)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_SPELL_BROWSE1, OnBrowseDictionary)
	ON_BN_CLICKED(IDC_SPELL_BROWSE2, OnBrowseAffinity)
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

void TSpellPage::GrayButtons ()
{
	UpdateData ();
	m_sAdd.EnableWindow (!m_strWord.IsEmpty ());
	m_sRemove.EnableWindow (m_sWords.GetSelCount ());
}

void TSpellPage::OnChangeWord()
{
	GrayButtons ();
}

void TSpellPage::OnSelchangeWords()
{
	GrayButtons ();
}

BOOL TSpellPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	FillList ();
	GrayButtons ();
	return TRUE;
}

void TSpellPage::FillList ()
{
	m_sWords.ResetContent ();

	(CNewsDoc::m_pDoc)->GetMyWords()->FillListbox( m_sWords );
}

void TSpellPage::OnAddWord()
{
	UpdateData ();  // fill m_strWord

	(CNewsDoc::m_pDoc)->GetMyWords()->AddWord ( m_strWord );

	m_strWord = "";
	UpdateData (FALSE);

	FillList();

	GrayButtons ();
}

void TSpellPage::OnRemoveWord()
{
	// go through selected items backwards to avoid recalculating indices
	for (int i = m_sWords.GetCount () - 1; i >= 0; i--)
	{
		if (m_sWords.GetSel (i))
		{
			CString strWord;
			m_sWords.GetText (i, strWord);

			(CNewsDoc::m_pDoc)->GetMyWords()->RemoveWord ( strWord );

			m_sWords.DeleteString (i);
		}
	}

	GrayButtons ();
}

BOOL TSpellPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp () -> HtmlHelp((DWORD)"posting_spell-checker.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_SPELLING);
	return 1;
}

afx_msg void TSpellPage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}

void TSpellPage::OnBrowseDictionary()
{
	CString dir;
	CString caption; caption.LoadString(IDS_SPELL_TITLE1);
	CString filter = "Dic files (*.dic)|*.dic|All Files (*.*)|*.*||";

	CFileDialog dlg(TRUE,
		NULL,
		"en_US.dic",
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		filter,
		this);

	dlg.m_ofn.lpstrTitle = caption;

	if (0 == GetInstallDir (dir))
		dlg.m_ofn.lpstrInitialDir = dir;

	if (IDOK == dlg.DoModal())
	{
		m_strDictionaryFile = dlg.GetFileName();
		UpdateData(FALSE);
	}
}

void TSpellPage::OnBrowseAffinity()
{
	CString dir;
	CString caption; caption.LoadString(IDS_SPELL_TITLE2);

	CString filter = "aff files (*.aff)|*.aff|All Files (*.*)|*.*||";

	CFileDialog dlg(TRUE,
		NULL,
		"en_US.aff",
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		filter,
		this);

	dlg.m_ofn.lpstrTitle = caption;
	if (0 == GetInstallDir( dir ))
		dlg.m_ofn.lpstrInitialDir = dir;

	if (IDOK == dlg.DoModal())
	{
		m_strAffinityFile	= dlg.GetFileName();
		UpdateData(FALSE);
	}
}
