/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngfltpg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:35  richard_wood
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

// ngfltpg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "ngfltpg.h"
#include "vfilter.h"
#include "globals.h"
#include "newsdb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupFilterPage dialog

TNewsGroupFilterPage::TNewsGroupFilterPage()
: CPropertyPage(TNewsGroupFilterPage::IDD)
{

	m_fOverrideFilter = FALSE;

	m_iFilterID = 0;
}

void TNewsGroupFilterPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_NGFLT, m_lbx);
	DDX_Check(pDX, IDC_NGFLT_OVERRIDE, m_fOverrideFilter);


	if (!pDX->m_bSaveAndValidate)
	{
		m_lbx.ResetContent();  // in case DoDataExchange is called twice

		TAllViewFilter * pAllFilters = gpStore->GetAllViewFilters();

		// the item data holds the uniqueid of the filter
		pAllFilters->FillLbx (&m_lbx, true);
		int iSetSel = -1;
		for (int i = 0; i < m_lbx.GetCount(); i++)
		{
			int filterID = (int) m_lbx.GetItemData(i);
			if (filterID == m_iFilterID)
			{
				iSetSel = i;
				break;
			}
		}

		m_lbx.SetCurSel ((iSetSel < 0) ? 0 : iSetSel);

		OnOverrideFilter();  // enable/disable
	}
	else
	{
		int idx;
		if ((idx = m_lbx.GetCurSel()) == LB_ERR)
			idx = 0;

		m_iFilterID = (int) m_lbx.GetItemData(idx);
	}
}

BEGIN_MESSAGE_MAP(TNewsGroupFilterPage, CPropertyPage)
	ON_BN_CLICKED(IDC_NGFLT_OVERRIDE, OnOverrideFilter)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupFilterPage message handlers

void TNewsGroupFilterPage::OnOverrideFilter()
{
	BOOL fEnable = IsDlgButtonChecked (IDC_NGFLT_OVERRIDE) > 0;
	CWnd * pWnd = GetDlgItem(IDC_LINK_GROUP_DESC);
	if (pWnd)
		pWnd->EnableWindow (fEnable);  // description
	m_lbx.EnableWindow (fEnable);
}

BOOL TNewsGroupFilterPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"filters-default.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TNewsGroupFilterPage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
