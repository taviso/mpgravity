/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: srvoptg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:54  richard_wood
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

// srvoptg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "srvoptg.h"
#include "helpcode.h"         // HID_OPTIONS_SERVGROUP

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TServerGroups property page

IMPLEMENT_DYNCREATE(TServerGroups, CPropertyPage)

TServerGroups::TServerGroups() : CPropertyPage(TServerGroups::IDD)
{
	m_iUpdateServerCount = 0;

	m_fNewgroupsOnConnect = FALSE;
	m_fDisplayNewGroups = FALSE;
}

TServerGroups::~TServerGroups()
{
}

void TServerGroups::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_UPDATECOUNT_COMBO, m_cmbUpdateServerCount);
	DDX_Check(pDX, IDC_CONNECT_NEWGROUP_ONCONNECT, m_fNewgroupsOnConnect);
	DDX_Check(pDX, IDC_CONNECT_GROUPS_DISPLAY, m_fDisplayNewGroups);

	if (pDX->m_bSaveAndValidate)
	{
		int idx = m_cmbUpdateServerCount.GetCurSel();
		if (CB_ERR != idx)
		{
			m_iUpdateServerCount = (int) m_cmbUpdateServerCount.GetItemData(idx);
		}
	}
	else
	{
		// these strings have "<NUM> <DESCRIPTION>"
		int rID[3] = { IDS_UTIL_SRVCNT_MANUAL,
			IDS_UTIL_SRVCNT_1PERSESSION,
			IDS_UTIL_SRVCNT_ONCONNECT };

		m_cmbUpdateServerCount.ResetContent();
		CString s;
		for (int i = 0; i < (sizeof(rID)/sizeof(rID[0])); ++i)
		{
			s.LoadString(rID[i]);
			int iAt = m_cmbUpdateServerCount.InsertString(-1, s.Mid(2));
			int data = _ttoi(s);
			m_cmbUpdateServerCount.SetItemData (iAt, data);
			if (data == m_iUpdateServerCount)
				m_cmbUpdateServerCount.SetCurSel (iAt);
		}
	}
}

BEGIN_MESSAGE_MAP(TServerGroups, CPropertyPage)
	ON_WM_HELPINFO()
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TServerGroups::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp () -> HtmlHelp((DWORD)"servers-group-tab.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_SERVGROUP);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TServerGroups::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
