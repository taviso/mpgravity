/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngoverpg.cpp,v $
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

// ngoverpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "ngoverpg.h"
#include "genutil.h"             // DDX_CEditStringList

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(TNewsGroupOverrideOptions, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
TNewsGroupOverrideOptions::TNewsGroupOverrideOptions() : CPropertyPage(TNewsGroupOverrideOptions::IDD)
{

	m_strDecodeDir = _T("");
	m_strEmail = _T("");
	m_strFullName = _T("");
	m_bOverrideCustomHeaders = FALSE;
	m_bOverrideDecodeDir = FALSE;
	m_bOverrideEmail = FALSE;
	m_bOverrideFullName = FALSE;
	m_iHeaderLimit = 0;
	m_fOverrideLimitHeaders = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
TNewsGroupOverrideOptions::~TNewsGroupOverrideOptions()
{
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_HEADERS_LIMIT, m_sLimitHeaders);
	DDX_Control(pDX, IDC_FULL_NAME, m_sFullName);
	DDX_Control(pDX, IDC_EMAIL, m_sEmail);
	DDX_Control(pDX, IDC_CUSTOM_HEADERS, m_sCustomHeadersCtrl);
	DDX_Control(pDX, IDC_DECODE_DIR, m_sDecodeDir);
	DDX_Text(pDX, IDC_DECODE_DIR, m_strDecodeDir);
	DDX_Text(pDX, IDC_EMAIL, m_strEmail);
	DDX_Text(pDX, IDC_FULL_NAME, m_strFullName);
	DDX_Check(pDX, IDC_OVERRIDE_CUSTOM_HEADERS, m_bOverrideCustomHeaders);
	DDX_Check(pDX, IDC_OVERRIDE_DECODE_DIR, m_bOverrideDecodeDir);
	DDX_Check(pDX, IDC_OVERRIDE_EMAIL, m_bOverrideEmail);
	DDX_Check(pDX, IDC_OVERRIDE_FULL_NAME, m_bOverrideFullName);
	DDX_Text(pDX, IDC_EDIT_HEADERS_LIMIT, m_iHeaderLimit);
	DDV_MinMaxUInt(pDX, m_iHeaderLimit, 1, 40000000);
	DDX_Check(pDX, IDC_OVERRIDE_HEADER_LIMIT, m_fOverrideLimitHeaders);


	if (pDX->m_bSaveAndValidate)
		m_iHeaderLimit = abs((long)m_iHeaderLimit);     

	// custom headers
	DDX_CEditStringList (pDX, IDC_CUSTOM_HEADERS, m_sCustomHeaders);
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(TNewsGroupOverrideOptions, CPropertyPage)
	ON_BN_CLICKED(IDC_OVERRIDE_CUSTOM_HEADERS, OnOverrideCustomHeaders)
	ON_BN_CLICKED(IDC_OVERRIDE_DECODE_DIR, OnOverrideDecodeDir)
	ON_BN_CLICKED(IDC_OVERRIDE_EMAIL, OnOverrideEmail)
	ON_BN_CLICKED(IDC_OVERRIDE_FULL_NAME, OnOverrideFullName)
	ON_BN_CLICKED(IDC_OVERRIDE_HEADER_LIMIT, OnOverrideLimitHeaders)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::OnOverrideCustomHeaders() 
{
	GreyControls ();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::OnOverrideDecodeDir() 
{
	GreyControls ();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::OnOverrideEmail() 
{
	GreyControls ();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::OnOverrideFullName() 
{
	GreyControls ();
}

/////////////////////////////////////////////////////////////////////////////
BOOL TNewsGroupOverrideOptions::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	GreyControls ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::OnOverrideLimitHeaders() 
{
	GreyControls ();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupOverrideOptions::GreyControls ()
{
	UpdateData ();
	m_sFullName.EnableWindow (m_bOverrideFullName);
	m_sEmail.EnableWindow (m_bOverrideEmail);
	m_sDecodeDir.EnableWindow (m_bOverrideDecodeDir);
	m_sCustomHeadersCtrl.EnableWindow (m_bOverrideCustomHeaders);
	m_sLimitHeaders.EnableWindow (m_fOverrideLimitHeaders);
}

BOOL TNewsGroupOverrideOptions::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"group-properties-overrides.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TNewsGroupOverrideOptions::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
