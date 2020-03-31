/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: gstorpg.cpp,v $
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
/*  Revision 1.3  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:26  richard_wood
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

// gstorpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "gstorpg.h"
#include "helpcode.h"            // HID_OPTIONS_*
#include "newsgrp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TStorageGlobalPage property page

IMPLEMENT_DYNCREATE(TStorageGlobalPage, CPropertyPage)

TStorageGlobalPage::TStorageGlobalPage() : CPropertyPage(TStorageGlobalPage::IDD)
{

	m_storageMode = -1;
	m_fStoreBodies = FALSE;
	m_fCacheInDatabase = FALSE;
}

TStorageGlobalPage::~TStorageGlobalPage()
{
}

void TStorageGlobalPage::DoDataExchange(CDataExchange* pDX)
{
	StoreModeToBoolean ();

	CPropertyPage::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_OPTIONS_GLOBAL_STORAGE_STOREBODIES, m_fStoreBodies);
	DDX_Check(pDX, IDC_OPTIONS_STORAGE_GLOBAL_CACHE, m_fCacheInDatabase);
}

BEGIN_MESSAGE_MAP(TStorageGlobalPage, CPropertyPage)
		ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL TStorageGlobalPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void TStorageGlobalPage::OnOK()
{

	BooleanToStoreMode ();

	CPropertyPage::OnOK();
}

void TStorageGlobalPage::StoreModeToBoolean()
{
	TNewsGroup::EStorageOption kStorageMode =
		(TNewsGroup::EStorageOption) m_storageMode;

	switch (kStorageMode)
	{
	case TNewsGroup::kStoreBodies:
		m_fStoreBodies = TRUE;
		break;
	case TNewsGroup::kHeadersOnly:
		m_fStoreBodies  = FALSE;
		break;
	}
}

void TStorageGlobalPage::BooleanToStoreMode()
{
	if (m_fStoreBodies)
		m_storageMode = (int) TNewsGroup::kStoreBodies;
	else
		m_storageMode = (int) TNewsGroup::kHeadersOnly;
}

BOOL TStorageGlobalPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp ()->HtmlHelp((DWORD)"preferences-storage.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_STORAGE);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TStorageGlobalPage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
