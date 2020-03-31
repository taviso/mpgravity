/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsrcpg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

// newsrcpg.cpp -- newsrc options page

#include "stdafx.h"              // precompiled header
#include "resource.h"            // needed by newsrcpg.h
#include "newsrcpg.h"            // this file's prototypes
#include "genutil.h"             // Browse()
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
IMPLEMENT_DYNCREATE(TOptionsNewsrcDlg, CPropertyPage)

TOptionsNewsrcDlg::TOptionsNewsrcDlg() : CPropertyPage(TOptionsNewsrcDlg::IDD)
{

	m_bExport = FALSE;
	m_strExportFile = _T("");
	m_bImport = FALSE;
	m_strImportFile = _T("");
	m_bSubscribedOnly = FALSE;
}

// -------------------------------------------------------------------------
void TOptionsNewsrcDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SUBSCRIBED_ONLY, m_sSubscribedOnly);
	DDX_Control(pDX, IDC_IMPORT_FILE, m_sImportFile);
	DDX_Control(pDX, IDC_IMPORT_BROWSE, m_sImportBrowse);
	DDX_Control(pDX, IDC_EXPORT_FILE, m_sExportFile);
	DDX_Control(pDX, IDC_EXPORT_BROWSE, m_sExportBrowse);
	DDX_Check(pDX, IDC_EXPORT, m_bExport);
	DDX_Text(pDX, IDC_EXPORT_FILE, m_strExportFile);
	DDX_Check(pDX, IDC_IMPORT, m_bImport);
	DDX_Text(pDX, IDC_IMPORT_FILE, m_strImportFile);
	DDX_Check(pDX, IDC_SUBSCRIBED_ONLY, m_bSubscribedOnly);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TOptionsNewsrcDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE, OnExportBrowse)
	ON_BN_CLICKED(IDC_IMPORT_BROWSE, OnImportBrowse)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TOptionsNewsrcDlg::OnExportBrowse() 
{
	// read m_strExportFile from display
	UpdateData ();

	Browse (m_strExportFile, FALSE /* bOpen */);

	// write m_strExportFile to display
	UpdateData (FALSE /* bSaveAndValidate */);
}

// -------------------------------------------------------------------------
void TOptionsNewsrcDlg::OnImportBrowse() 
{
	// read m_strImportFile from display
	UpdateData ();

	Browse (m_strImportFile, TRUE /* bOpen */);

	// write m_strImportFile to display
	UpdateData (FALSE /* bSaveAndValidate */);
}

// -------------------------------------------------------------------------
BOOL TOptionsNewsrcDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	SetGreyState ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TOptionsNewsrcDlg::SetGreyState ()
{
	UpdateData ();

	m_sImportFile.EnableWindow (m_bImport);
	m_sImportBrowse.EnableWindow (m_bImport);

	m_sExportFile.EnableWindow (m_bExport);
	m_sExportBrowse.EnableWindow (m_bExport);
	m_sSubscribedOnly.EnableWindow (m_bExport);
}

// -------------------------------------------------------------------------
void TOptionsNewsrcDlg::OnExport() 
{
	SetGreyState ();
}

// -------------------------------------------------------------------------
void TOptionsNewsrcDlg::OnImport() 
{
	SetGreyState ();
}

// -------------------------------------------------------------------------
BOOL TOptionsNewsrcDlg::OnHelpInfo(HELPINFO*)
{
	AfxGetApp () -> HtmlHelp((DWORD)"servers-newsrc-tab.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_NEWSRC);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsNewsrcDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
