/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: encodpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/16 16:47:41  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.4  2008/09/19 14:51:23  richard_wood
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

// encodpg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "encodpg.h"
#include "helpcode.h"            // HID_OPTIONS_*
#include "8859x.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsEncodeDlg property page

IMPLEMENT_DYNCREATE(TOptionsEncodeDlg, CPropertyPage)

TOptionsEncodeDlg::TOptionsEncodeDlg() : CPropertyPage(TOptionsEncodeDlg::IDD)
{

	m_subjectTemplate = _T("");
	m_kEncoding = -1;
	m_SplitLen = _T("");
	m_ContentType = _T("");
	m_ContentEncoding = _T("");
	m_fSeparateArts = FALSE;
	m_fSend8Bit = FALSE;
	m_fSend8BitHeaders = FALSE;
}

TOptionsEncodeDlg::~TOptionsEncodeDlg()
{
}

void TOptionsEncodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		if (m_SplitLen == "0")
		{
			m_SplitLen.LoadString (IDS_NOLIMIT);
		}
	}


	DDX_Control(pDX, IDC_CMB_SENDCHARSET, m_cmbSendCharset);
	DDX_Text(pDX, IDC_POST_SUBJ_TEMPLATE, m_subjectTemplate);
	DDX_Radio(pDX, IDC_POSTING_UUENCODE, m_kEncoding);
	DDX_CBString(pDX, IDC_CMB_SPLITLEN, m_SplitLen);
	DDX_CBString(pDX, IDC_MIME_CONTENTTYPE, m_ContentType);
	DDX_CBString(pDX, IDC_MIME_CONTENTENCODING, m_ContentEncoding);
	DDX_Check(pDX, IDC_CHECK_SEPARATE_ARTS, m_fSeparateArts);
	DDX_Check(pDX, IDC_CBX_8BIT, m_fSend8Bit);
	DDX_Check(pDX, IDC_CBX_8BITHDR, m_fSend8BitHeaders);


	if (pDX->m_bSaveAndValidate)
	{
		// 0 means "no limit"
		CString noLimit; noLimit.LoadString (IDS_NOLIMIT);
		if (m_SplitLen == noLimit)
			m_SplitLen = "0";

		int idx = m_cmbSendCharset.GetCurSel ();
		m_iSendCharset = (int) m_cmbSendCharset.GetItemData(idx);
	}
	else
	{
		list<GravCharset*>::iterator it = gsCharMaster.m_sCharsets.begin();
		for (; it != gsCharMaster.m_sCharsets.end(); it++)
		{
			GravCharset* pCharset = *it;
			if (pCharset->AllowPosting()) 
			{
				int idx = m_cmbSendCharset.AddString (pCharset->GetName());
				m_cmbSendCharset.SetItemData (idx, pCharset->GetId());
				if (m_iSendCharset == pCharset->GetId())
					m_cmbSendCharset.SetCurSel (idx);
			}
		}
	}
}

BEGIN_MESSAGE_MAP(TOptionsEncodeDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_POSTING_UUENCODE, OnPostingUuencode)
	ON_BN_CLICKED(IDC_POSTING_MIME, OnPostingMime)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsEncodeDlg message handlers

BOOL TOptionsEncodeDlg::OnInitDialog()
{
	CString str;

	// Fill with Content Type Encoding choices
	CComboBox* pCB = (CComboBox*) GetDlgItem (IDC_MIME_CONTENTENCODING);
	int i = 0;
	for (i = int(IDS_MIMEENCODE_START + 1);
		i < int(IDS_MIMEENCODE_END); ++i)
	{
		if (str.LoadString (i))
			pCB->AddString ( str );
	}

	pCB = (CComboBox*) GetDlgItem (IDC_MIME_CONTENTTYPE);
	// Fill with Content Type
	for (i = int(IDS_MIMECONTENT_START+1);
		i < int(IDS_MIMECONTENT_END);
		++i)
	{
		if (str.LoadString(i))
			pCB->AddString ( str );
	}

	pCB = (CComboBox*) GetDlgItem (IDC_CMB_SPLITLEN);

	// extra element, so we always have a [i+1] element to check
	int vSizes[] = {0, 32, 48, 64, 128, 256, 512, 1024, 0x7FFFFFFF};
	int nMax = sizeof(vSizes)/sizeof(int) - 1;
	int iSplitLen = atoi (LPCTSTR(m_SplitLen));

	for (i = 0; i < nMax; ++i)
	{
		if (vSizes[i])
		{
			str.Format("%d", vSizes[i]);
			pCB->InsertString ( -1, str );
		}

		if (vSizes[i] < iSplitLen  &&  iSplitLen < vSizes[i+1])
		{
			// insert custom length by hand
			str.Format("%d", iSplitLen);
			pCB->InsertString ( -1, str);
		}
	}
	str.LoadString (IDS_NOLIMIT);
	pCB->InsertString (-1, str);

	CPropertyPage::OnInitDialog();

	UpdateUI_MIMEControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void TOptionsEncodeDlg::OnPostingUuencode()
{
	UpdateUI_MIMEControls();
}

void TOptionsEncodeDlg::OnPostingMime()
{
	UpdateUI_MIMEControls();
}

void TOptionsEncodeDlg::UpdateUI_MIMEControls()
{
	BOOL fOn = IsDlgButtonChecked (IDC_POSTING_MIME);

	(GetDlgItem (IDC_ENCODE_DESC))->EnableWindow( fOn );
	(GetDlgItem (IDC_MIME_CONTENTENCODING))->EnableWindow( fOn );

	(GetDlgItem (IDC_TYPE_DESC))->EnableWindow( fOn );
	(GetDlgItem (IDC_MIME_CONTENTTYPE))->EnableWindow( fOn );
}

BOOL TOptionsEncodeDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp ()->HtmlHelp((DWORD)"binaries-posting.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_ENCODING);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsEncodeDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
