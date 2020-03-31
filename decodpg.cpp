/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: decodpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:21  richard_wood
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

// tdecodp.cpp : implementation file

#include "stdafx.h"
#include <io.h>                  // _access
#include "news.h"
#include "decodpg.h"
#include "dirpick.h"
#include "fileutil.h"            // GetInputFilename()
#include "helpcode.h"            // HID_OPTIONS_*
#include "rgswit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
IMPLEMENT_DYNCREATE(TDecodePage, CPropertyPage)

// -------------------------------------------------------------------------
TDecodePage::TDecodePage() : CPropertyPage(TDecodePage::IDD)
{
	m_decodeDirectory = _T("");
	m_fAutomaticManualDecode = FALSE;
	m_fAutomaticRuleDecode = FALSE;
	m_strOtherViewer = _T("");
	m_bRestartPausedJobs = FALSE;
	m_bDeleteOnExit = FALSE;
	m_bWriteDescription = FALSE;
	m_iViewMethod = -1;
	m_uiPauseSpace = 0;
	m_iFilenameConflict = 0;
	m_fPausingSpace = FALSE;
}

// -------------------------------------------------------------------------
void AFXAPI DDV_Directory(CDataExchange* pDX, int nIDC, CString& strDir)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate) {
		// empty path is ok
		if (strDir.IsEmpty())
			return;

		// Note: does not handle UNC paths
		if (-1 == _access(strDir, 0)) {
			CString msg;
			AfxFormatString1(msg, IDS_ERR_DIRNOTFOUND, (LPCTSTR) strDir);
			AfxMessageBox(msg, MB_ICONSTOP | MB_OK);
			pDX->Fail();
		}
	}
}

// -------------------------------------------------------------------------
void TDecodePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CMB_NAMECONFLICT, m_cmbFilename);
	DDX_Control(pDX, IDC_OTHER_VIEWER_BROWSE, m_sViewerBrowse);
	DDX_Control(pDX, IDC_OTHER_VIEWER, m_sOtherViewer);
	DDX_Check(pDX, IDC_ATTACH_DECODE_LAUNCHVIEWMANUAL, m_fAutomaticManualDecode);
	DDX_Check(pDX, IDC_ATTACH_DECODE_LAUNCHBYRULES, m_fAutomaticRuleDecode);
	DDX_Text(pDX, IDC_OTHER_VIEWER, m_strOtherViewer);
	DDX_Check(pDX, IDC_DECODEPG_RESTARTPAUSE, m_bRestartPausedJobs);
	DDX_Check(pDX, IDC_DECODEPG_DELETEONEXIT, m_bDeleteOnExit);
	DDX_Check(pDX, IDC_DECODEPG_WRITEDESCR, m_bWriteDescription);
	DDX_Radio(pDX, IDC_USE_IMAGE_GALLERY, m_iViewMethod);
	DDX_Text(pDX, IDC_PAUSE_SPACE, m_uiPauseSpace);
	DDV_MinMaxUInt(pDX, m_uiPauseSpace, 1, 999);
	DDX_Check(pDX, IDC_CBX_PAUSELOWDISK, m_fPausingSpace);


	// vc4.2 class wizard chokes on this
	DDX_Text(pDX, IDC_ATTACH_DECODEDIR, m_decodeDirectory);
	DDV_Directory(pDX, IDC_ATTACH_DECODEDIR, m_decodeDirectory);

	transfer_postprocessing ( pDX );
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TDecodePage, CPropertyPage)
	ON_BN_CLICKED(IDC_ATTACH_DECODE_BROWSE, OnAttachDecodeBrowse)
	ON_BN_CLICKED(IDC_OTHER_VIEWER_BROWSE, OnOtherViewerBrowse)
	ON_BN_CLICKED(IDC_DECODEPG_AFTERDECODE, OnDecodepgAfterdecode)
	ON_BN_CLICKED(IDC_USE_ALTERNATE, OnUseAlternate)
	ON_BN_CLICKED(IDC_USE_IMAGE_GALLERY, OnUseImageGallery)
	ON_BN_CLICKED(IDC_USE_REGISTRY, OnUseRegistry)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_CBX_PAUSELOWDISK, OnCbxPauselowdisk)
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TDecodePage::OnAttachDecodeBrowse()
{
	CString  temp;
	if (DirectoryPicker (this , &temp))
	{
		CEdit *pEdit = (CEdit *) GetDlgItem (IDC_ATTACH_DECODEDIR);
		pEdit->SetWindowText (temp);
		m_decodeDirectory = temp;
	}
}

// -------------------------------------------------------------------------
void TDecodePage::OnOtherViewerBrowse()
{
	CString fname;
	if (TFileUtil::GetInputFilename (&fname, this, IDS_GETVIEWEXE_TITLE,
		NULL /* initDir */, 0, IDS_FILTER_GETVIEWEXE))
		((CEdit *) GetDlgItem (IDC_OTHER_VIEWER))->SetWindowText (fname);
}

// -------------------------------------------------------------------------
void TDecodePage::UpdateGreyState()
{
	UpdateData ();
	BOOL bEnable = (m_iViewMethod == 2);
	m_sOtherViewer.EnableWindow (bEnable);
	m_sViewerBrowse.EnableWindow (bEnable);
}

// -------------------------------------------------------------------------
BOOL TDecodePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CheckDlgButton (IDC_CBX_PAUSELOWDISK, m_fPausingSpace);
	OnCbxPauselowdisk() ;

	UpdateGreyState ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// ItemData [0-3].  Array position is significant
static int griNameConflict[] = {
	IDS_UTIL_ASK,  
	IDS_UTIL_AUTOSKIP,
	IDS_UTIL_AUTOWRITE,
	IDS_UTIL_AUTONAME
};

// -------------------------------------------------------------------------
// What to do after decoding. Nothing, mark as read, markread after Viewing
void TDecodePage::transfer_postprocessing(CDataExchange* pDX)
{

	if (!pDX->m_bSaveAndValidate)
	{
		if (m_iAfterDecode)
		{
			CheckDlgButton(IDC_DECODEPG_AFTERDECODE, TRUE);
			CheckRadioButton (IDC_DECODE_DECMARKREAD,
				IDC_DECODE_VIEWMARKREAD,
				(m_iAfterDecode == (int) TRegSwitch::kReadAfterDecode)
				? IDC_DECODE_DECMARKREAD
				: IDC_DECODE_VIEWMARKREAD);
		}
		else
		{
			enable_afterdecode(FALSE);
		}

		// fill combo box
		CString util;
		int i, iAt;
		m_cmbFilename.ResetContent();
		for (i = 0; i < (sizeof(griNameConflict)/sizeof(int)); i++)
		{
			util.LoadString(griNameConflict[i]);
			iAt = m_cmbFilename.InsertString(i, util);
			m_cmbFilename.SetItemData(iAt, i);              
		}
		for (i = 0; i < (sizeof(griNameConflict)/sizeof(int)); i++)
			if (m_iFilenameConflict == (int) m_cmbFilename.GetItemData(i))
			{
				m_cmbFilename.SetCurSel(i);
				break;
			}
	}
	else
	{
		if (IsDlgButtonChecked(IDC_DECODEPG_AFTERDECODE))
		{
			if (IsDlgButtonChecked(IDC_DECODE_DECMARKREAD))
				m_iAfterDecode = (int) TRegSwitch::kReadAfterDecode;
			else
				m_iAfterDecode = (int) TRegSwitch::kReadAfterView;
		}
		else
			m_iAfterDecode = (int) TRegSwitch::kNothing;

		// what to do for filename conflicts
		int iSel = m_cmbFilename.GetCurSel ();
		if (iSel >= 0)
			m_iFilenameConflict = int(m_cmbFilename.GetItemData (iSel));
	}
}

// -------------------------------------------------------------------------
// Enable / Disable check box set
void TDecodePage::enable_afterdecode(BOOL fEnable)
{
	if (!fEnable)
	{
		// before disabling put radio buttons into definite state
		CheckRadioButton(IDC_DECODE_DECMARKREAD, IDC_DECODE_VIEWMARKREAD,
			IDC_DECODE_VIEWMARKREAD);
	}
	GetDlgItem(IDC_DECODE_DECMARKREAD)->EnableWindow(fEnable);
	GetDlgItem(IDC_DECODE_VIEWMARKREAD)->EnableWindow(fEnable);
}

// -------------------------------------------------------------------------
// check box controls 2 radio buttons
void TDecodePage::OnDecodepgAfterdecode() 
{
	enable_afterdecode(IsDlgButtonChecked(IDC_DECODEPG_AFTERDECODE));
}

// -------------------------------------------------------------------------
void TDecodePage::OnUseAlternate() 
{
	UpdateGreyState();
}

// -------------------------------------------------------------------------
void TDecodePage::OnUseImageGallery() 
{
	UpdateGreyState();
}

// -------------------------------------------------------------------------
void TDecodePage::OnUseRegistry() 
{
	UpdateGreyState();
}

// -------------------------------------------------------------------------
BOOL TDecodePage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"binaries-attachment-options.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_DECODING);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TDecodePage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}

// -------------------------------------------------------------------------
void TDecodePage::OnCbxPauselowdisk() 
{
	BOOL fEnabled = FALSE;
	if (IsDlgButtonChecked(IDC_CBX_PAUSELOWDISK))
		fEnabled = TRUE;

	GetDlgItem(IDC_PAUSE_SPACE)->EnableWindow(fEnabled);
	GetDlgItem(IDC_STATIC_MEGABYTES)->EnableWindow(fEnabled);
}
