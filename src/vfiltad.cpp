/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vfiltad.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:52:25  richard_wood
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

// vfiltad.cpp : implementation file

#include "stdafx.h"
#include "news.h"
#include "vfiltad.h"
#include "tmsgbx.h"
#include "genutil.h"             // MsgResource()
#include "rules.h"               // Rule
#include "trulecon.h"            // TRuleConditions()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
TViewFilterAddDlg::TViewFilterAddDlg(CListCtrl * pList, CWnd* pParent /*=NULL*/)
: CDialog(TViewFilterAddDlg::IDD, pParent)
{
	m_fEdit = FALSE;
	m_pList = pList;

	m_wFilter = m_wRequired = 0;

	// The HIWORD()==0x8000 is used to indicate descending sort
	m_dwSortCode = 0;

	m_strName = _T("");
	m_fShowEntireThread = FALSE;
	m_fCompleteBinaries = FALSE;
	m_fSkipThreads = FALSE;
}

// ------------------------------------------------------------------------
BOOL TViewFilterAddDlg::OnInitDialog()
{
	// should initialize m_sDescription first because data-exchange accesses it
	HWND hWnd = GetDlgItem (IDC_FILT_DESC) -> m_hWnd;
	m_sDescription.Attach (hWnd);

	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// ------------------------------------------------------------------------
TViewFilterAddDlg::~TViewFilterAddDlg ()
{
	m_sDescription.Detach ();
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::config_all_bits (BOOL fSave)
{
	config_bits(fSave, TStatusUnit::kFilterNew,        IDC_VFILT_NEW);
	config_bits(fSave, TStatusUnit::kFilterImportant,  IDC_VFILT_IMP);
	config_bits(fSave, TStatusUnit::kFilterLocal,      IDC_VFILT_LOCAL);
	config_bits(fSave, TStatusUnit::kFilterTag,        IDC_VFILT_TAG);
	config_bits(fSave, TStatusUnit::kFilterDecoded,    IDC_VFILT_DECODED);
	config_bits(fSave, TStatusUnit::kFilterPermanent,  IDC_VFILT_PROTECTED);
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_strName);
	DDV_MaxChars(pDX, m_strName, 60);
	DDX_Check(pDX, IDC_VFILT_SHOWTHREAD, m_fShowEntireThread);
	DDX_Check(pDX, IDC_VFILT_COMPLETE, m_fCompleteBinaries);
	DDX_Control(pDX, IDC_CMB_FLTSORT, m_cmbSort);
	DDX_Check(pDX, IDC_VFILT_SKIPTHREAD, m_fSkipThreads);


	if (pDX->m_bSaveAndValidate && m_strName.IsEmpty())
	{
		NewsMessageBox(this, IDS_VFILTNAME_REQUIRED, MB_OK | MB_ICONWARNING);
		pDX->Fail();
	}
	if (pDX->m_bSaveAndValidate)
	{
		int nMatch = CountNameMatches(m_strName);
		// if we are editing, then we can conceivably match our old name
		// if we are adding a filter, the name must be unique

		if ( (m_fEdit && (nMatch > 1))  || (!m_fEdit && (nMatch > 0)))
		{
			NewsMessageBox(this, IDS_VFILTNAME_UNIQUE, MB_OK | MB_ICONWARNING);
			pDX->Fail();
		}
	}

	config_all_bits (pDX -> m_bSaveAndValidate);
	config_sort_combo (pDX->m_bSaveAndValidate);

	if (!pDX->m_bSaveAndValidate)
		update_banner();

	if (!pDX->m_bSaveAndValidate && m_fEdit)
	{
		// set new dialog caption
		CString s; s.LoadString (IDS_VFILT_EDITFILT);
		SetWindowText (s);
	}
}

// ------------------------------------------------------------------------
int TViewFilterAddDlg::CountNameMatches (const CString & name)
{
	int iMatch = 0;
	int tot = m_pList->GetItemCount();
	for (int i = 0; i < tot; ++i)
	{
		if (name == m_pList->GetItemText (i, 0))
			++iMatch;
	}
	return iMatch;
}

// ------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TViewFilterAddDlg, CDialog)
	ON_BN_CLICKED(IDC_VFILT_IMP, OnVfiltImp)
	ON_BN_CLICKED(IDC_VFILT_LOCAL, OnVfiltLocal)
	ON_BN_CLICKED(IDC_VFILT_NEW, OnVfiltNew)
	ON_BN_CLICKED(IDC_VFILT_TAG, OnVfiltTag)
	ON_BN_CLICKED(IDC_VFILT_DECODED, OnVfiltDecoded)
	ON_BN_CLICKED(IDC_VFILT_PROTECTED, OnVfiltProtected)
	ON_BN_CLICKED(IDC_ADVANCED, OnAdvanced)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_CBN_SELCHANGE(IDC_CMB_FLTSORT, OnSelchangeCmbFltsort)
	ON_BN_CLICKED(IDC_RADIO_FLTSORT_ASEND, OnRadioFltsortAsend)
	ON_BN_CLICKED(IDC_RADIO_FLTSORT_DSEND, OnRadioFltsortDsend)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TViewFilterAddDlg message handlers

void TViewFilterAddDlg::OnVfiltNew()
{
	config_bits(TRUE, TStatusUnit::kFilterNew, IDC_VFILT_NEW);
	update_banner();
}

void TViewFilterAddDlg::OnVfiltImp()
{
	config_bits(TRUE, TStatusUnit::kFilterImportant, IDC_VFILT_IMP);
	update_banner();
}

void TViewFilterAddDlg::OnVfiltLocal()
{
	config_bits(TRUE, TStatusUnit::kFilterLocal, IDC_VFILT_LOCAL);
	update_banner();
}

void TViewFilterAddDlg::OnVfiltTag()
{
	config_bits(TRUE, TStatusUnit::kFilterTag, IDC_VFILT_TAG);
	update_banner();
}

void TViewFilterAddDlg::OnVfiltDecoded()
{
	config_bits(TRUE, TStatusUnit::kFilterDecoded, IDC_VFILT_DECODED);
	update_banner();
}

void TViewFilterAddDlg::OnVfiltProtected()
{
	config_bits(TRUE, TStatusUnit::kFilterPermanent, IDC_VFILT_PROTECTED);
	update_banner();
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::config_bits(BOOL fSave, TStatusUnit::EStatusFilter eStatus,
									int iCtrlID)
{
	CButton * pBut = static_cast<CButton*>(GetDlgItem(iCtrlID));
	if (!pBut)
		return;
	if (!fSave)
	{
		pBut->SetCheck (GetState (eStatus));
	}
	else
	{
		switch (pBut->GetCheck())
		{
		case 0:
			// must be Off
			m_wRequired |= eStatus;
			m_wFilter   &= ~eStatus;
			break;

		case 1:
			// must be ON
			m_wRequired |= eStatus;
			m_wFilter   |= eStatus;
			break;

		case 2:
			// don't care
			m_wRequired &= ~eStatus;
			m_wFilter   &= ~eStatus;
			break;
		}
	}
}

// ------------------------------------------------------------------------
// return 1, 0, or 2 for dont-care
int TViewFilterAddDlg::GetState(TStatusUnit::EStatusFilter eStatus)
{
	if (m_wRequired & eStatus)
		return (m_wFilter & eStatus) ? 1 : 0 ;
	else
		return 2;
}

// ------------------------------------------------------------------------
// GetDescription -- gets the basic description (i.e., not including the
// advanced conditions)
void TViewFilterAddDlg::GetDescription (CString &str)
{
	int vPhrases[6][3] =
	{ {IDS_UTIL_READ_ARTICLES, IDS_UTIL_NEW_ARTICLES, IDS_UTIL_ALL_ARTICLES},
	{IDS_UTIL_NOTIMP_ARTICLES, IDS_UTIL_IMP_ARTICLES, 0},
	{IDS_UTIL_NOTLOCAL_ARTICLES, IDS_UTIL_LOCAL_ARTICLES, 0},
	{IDS_UTIL_NOTTAG_ARTICLES, IDS_UTIL_TAG_ARTICLES, 0},
	{IDS_UTIL_NODECODE_ARTICLES, IDS_UTIL_DECODE_ARTICLES, 0},
	{IDS_UTIL_NOPROT_ARTICLES, IDS_UTIL_PROT_ARTICLES, 0}
	};

	int iNewStringID;
	int riAttrib[5];

	iNewStringID = vPhrases[0][GetState(TStatusUnit::kFilterNew)];

	// Attribute 0 - Important
	riAttrib[0] = vPhrases[1][GetState(TStatusUnit::kFilterImportant)];

	// Attribute 1 - Local
	riAttrib[1] = vPhrases[2][GetState(TStatusUnit::kFilterLocal)];

	// Attribute 2 - Tagged
	riAttrib[2] = vPhrases[3][GetState(TStatusUnit::kFilterTag)];

	// Attribute 3 - Decoded
	riAttrib[3] = vPhrases[4][GetState(TStatusUnit::kFilterDecoded)];

	// Attribute 4 - Protected
	riAttrib[4] = vPhrases[5][GetState(TStatusUnit::kFilterPermanent)];

	CString stub;
	int iPhraseCount = 0;   // controls if we need a Comma

	// start off describing (Unread/Read/All)
	str.LoadString(iNewStringID);

	for (int k = 0; k < (sizeof(riAttrib)/sizeof(riAttrib[0])); ++k)
	{
		if (riAttrib[k])
		{
			stub.LoadString (riAttrib[k]);
			str += (iPhraseCount == 0) ? " " : ", ";
			str += stub;
			++ iPhraseCount;
		}
	}
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::update_banner()
{
	CString strTotal;
	GetDescription (strTotal);

	// add the "advanced" conditions
	CString strTemp; strTemp.LoadString (IDS_UTIL_ALL_ARTICLES);
	BOOL bAll = (strTemp == strTotal);  // all articles?
	int iLines = m_rstrCondition.GetSize ();
	if (iLines) {

		if (bAll)
			strTotal = "";

		if (!bAll) {
			CString strAnd; strAnd.LoadString (IDS_AND);
			strTotal += CString ("\r\n") + strAnd + "\r\n(\r\n";
		}

		for (int i = 0; i < m_rstrCondition.GetSize (); i++) {
			strTotal += m_rstrCondition.GetAt (i);
			strTotal += "\r\n";
		}

		if (!bAll)
			strTotal += ")\r\n";
	}

	m_sDescription.SetWindowText ( strTotal );
	CheckIndentation (m_sDescription);
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::OnOK()
{
#ifdef LITE
	MsgResource (IDS_LITE_NO_FILTERS);
	return;
#endif

	CDialog::OnOK();
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::OnAdvanced()
{
	Rule sRule;
	sRule.rstrCondition.Copy (m_rstrCondition);

	// add the "advanced" conditions
	CString strCondition;
	GetDescription (strCondition);
	CString strTemp; strTemp.LoadString (IDS_UTIL_ALL_ARTICLES);
	if (strTemp == strCondition)
		strCondition = "";
	else {
		CString strAnd; strAnd.LoadString (IDS_AND);
		strCondition += CString ("\r\n") + strAnd;
	}
	TRuleConditions sCondPage (strCondition);
	sCondPage.SetRule (&sRule);

	CPropertySheet sDlg (IDS_ADVANCED_CONDITIONS);
	sDlg.AddPage (&sCondPage);

	if (sDlg.DoModal () != IDOK)
		return;

	m_rstrCondition.Copy (sRule.rstrCondition);
	update_banner();
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::OnClear()
{
	// clear filter
	m_rstrCondition.RemoveAll ();
	m_wFilter = m_wRequired = 0;

	// update text description
	update_banner ();

	// update buttons
	config_all_bits (FALSE /* fSave */);
}

// NOTE: matches definitions in vfilter.h
static int gsFilterSortMap[] = { IDS_SORT_THREAD,  0,
IDS_SORT_STATUS,  1,
IDS_SORT_FROM,    2,
IDS_SORT_SUBJECT, 3,
IDS_SORT_LINES,   4,
IDS_SORT_DATE,    5,
IDS_SORT_SCORE,   6  };

// ------------------------------------------------------------------------
void TViewFilterAddDlg::config_sort_combo (BOOL fSave)
{
	if (!fSave)
	{
		CString tmp;
		m_cmbSort.ResetContent ();
		for (int i = 0; i < (sizeof(gsFilterSortMap) / sizeof(gsFilterSortMap[0]));
			i+=2)
		{
			tmp.LoadString (gsFilterSortMap[i]);
			int idx = m_cmbSort.AddString (tmp);

			DWORD dwSortcode = gsFilterSortMap[i+1];
			m_cmbSort.SetItemData (idx, dwSortcode);
			if (LOWORD(dwSortcode) == LOWORD(m_dwSortCode))
				m_cmbSort.SetCurSel (idx);
		}

		if (HIWORD(m_dwSortCode) == 0)
			OnRadioFltsortAsend();
		else
			OnRadioFltsortDsend();
	}
	else
	{
		int idx = m_cmbSort.GetCurSel ();
		if (CB_ERR == idx)
		{
			m_dwSortCode = 0;
			return;
		}
		WORD wSort = LOWORD(m_cmbSort.GetItemData (idx));
		if (IsDlgButtonChecked (IDC_RADIO_FLTSORT_DSEND))
			m_dwSortCode = MAKELONG(wSort, 0x8000);
		else
			m_dwSortCode = MAKELONG(wSort, 0x0000);
	}
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::OnSelchangeCmbFltsort()
{
	return;
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::OnRadioFltsortAsend()
{
	CheckRadioButton (IDC_RADIO_FLTSORT_ASEND, IDC_RADIO_FLTSORT_DSEND,
		IDC_RADIO_FLTSORT_ASEND);
}

// ------------------------------------------------------------------------
void TViewFilterAddDlg::OnRadioFltsortDsend()
{
	CheckRadioButton (IDC_RADIO_FLTSORT_ASEND, IDC_RADIO_FLTSORT_DSEND,
		IDC_RADIO_FLTSORT_DSEND);
}
