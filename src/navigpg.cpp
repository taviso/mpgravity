/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: navigpg.cpp,v $
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
/*  Revision 1.4  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:33  richard_wood
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

// navigpg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "navigpg.h"
#include "navigdef.h"
#include "helpcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsNavigatePage property page

IMPLEMENT_DYNCREATE(TOptionsNavigatePage, CPropertyPage)

TOptionsNavigatePage::TOptionsNavigatePage() : CPropertyPage(TOptionsNavigatePage::IDD)
{

	m_fMButSKR = FALSE;

}

TOptionsNavigatePage::~TOptionsNavigatePage()
{
}

void TOptionsNavigatePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_1KEYREAD_THREADED, m_cmb1KeyRead_Threaded);
	DDX_Control(pDX, IDC_1KEYREAD_SORTED, m_cmb1KeyRead_Sorted);
	DDX_Control(pDX, IDC_IGNORE_THREADED, m_cmbIgnore_Threaded);
	DDX_Control(pDX, IDC_IGNORE_SORTED, m_cmbIgnore_Sorted);
	DDX_Control(pDX, IDC_K_THREADED, m_cmbK_Threaded);
	DDX_Control(pDX, IDC_K_SORTED, m_cmbK_Sorted);
	DDX_Control(pDX, IDC_KTHREAD_THREADED, m_cmbKThread_Threaded);
	DDX_Control(pDX, IDC_KTHREAD_SORTED, m_cmbKThread_Sorted);
	DDX_Control(pDX, IDC_TAG_THREADED, m_cmbTag_Threaded);
	DDX_Control(pDX, IDC_TAG_SORTED, m_cmbTag_Sorted);
	DDX_Control(pDX, IDC_WATCH_THREADED, m_cmbWatch_Threaded);
	DDX_Control(pDX, IDC_WATCH_SORTED, m_cmbWatch_Sorted);
	DDX_Check(pDX, IDC_OPTNAV_MBUTTON, m_fMButSKR);


	if (FALSE == pDX->m_bSaveAndValidate)  // OnInitDialog
	{
		initialize ();
	}
	else
	{
		int idx = m_tab.GetCurSel();

		if (-1 == idx) return;

		int iMode = idx;   // idx zero is Offline, idx 1 is Online

		SaveData (iMode ? true : false);
	}
}

BEGIN_MESSAGE_MAP(TOptionsNavigatePage, CPropertyPage)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void TOptionsNavigatePage::initialize ()
{
	// fill up the Online/offline  toggle control

	CString tmp;

	// we get called twice for some reason ...
	if (m_tab.GetItemCount() == 0)
	{
		tmp.LoadString (IDS_NAVUTIL_OFFLINE);
		m_tab.InsertItem (0, tmp);

		tmp.LoadString (IDS_NAVUTIL_ONLINE);
		m_tab.InsertItem (1, tmp);

		m_tab.SetCurSel (1);
	}
	ShowOnlineData (true);
}

/////////////////////////////////////////////////////////////////////////////
void TOptionsNavigatePage::ShowOnlineData (bool fOnline)
{
	int iSecondaryIdx = (fOnline) ? 1 : 0;

	fill_and_select (m_cmb1KeyRead_Threaded,  m_vPref[0][iSecondaryIdx]);
	fill_and_select (m_cmb1KeyRead_Sorted,    m_vPref[1][iSecondaryIdx]);
	fill_and_select (m_cmbIgnore_Threaded,    m_vPref[2][iSecondaryIdx]);
	fill_and_select (m_cmbIgnore_Sorted,      m_vPref[3][iSecondaryIdx]);
	fill_and_select (m_cmbK_Threaded,         m_vPref[4][iSecondaryIdx]);
	fill_and_select (m_cmbK_Sorted,           m_vPref[5][iSecondaryIdx]);
	fill_and_select (m_cmbKThread_Threaded,   m_vPref[6][iSecondaryIdx]);
	fill_and_select (m_cmbKThread_Sorted,     m_vPref[7][iSecondaryIdx]);
	fill_and_select (m_cmbTag_Threaded,       m_vPref[8][iSecondaryIdx]);
	fill_and_select (m_cmbTag_Sorted,         m_vPref[9][iSecondaryIdx]);
	fill_and_select (m_cmbWatch_Threaded,     m_vPref[10][iSecondaryIdx]);
	fill_and_select (m_cmbWatch_Sorted,       m_vPref[11][iSecondaryIdx]);
}

/////////////////////////////////////////////////////////////////////////////
void TOptionsNavigatePage::SaveData (bool fOnline)
{
	int iSecondaryIdx = (fOnline) ? 1 : 0;
	save_cmb_action (m_cmb1KeyRead_Threaded,  m_vPref[0][iSecondaryIdx]);
	save_cmb_action (m_cmb1KeyRead_Sorted,    m_vPref[1][iSecondaryIdx]);
	save_cmb_action (m_cmbIgnore_Threaded,    m_vPref[2][iSecondaryIdx]);
	save_cmb_action (m_cmbIgnore_Sorted,      m_vPref[3][iSecondaryIdx]);
	save_cmb_action (m_cmbK_Threaded,         m_vPref[4][iSecondaryIdx]);
	save_cmb_action (m_cmbK_Sorted,           m_vPref[5][iSecondaryIdx]);
	save_cmb_action (m_cmbKThread_Threaded,   m_vPref[6][iSecondaryIdx]);
	save_cmb_action (m_cmbKThread_Sorted,     m_vPref[7][iSecondaryIdx]);
	save_cmb_action (m_cmbTag_Threaded,       m_vPref[8][iSecondaryIdx]);
	save_cmb_action (m_cmbTag_Sorted,         m_vPref[9][iSecondaryIdx]);
	save_cmb_action (m_cmbWatch_Threaded,     m_vPref[10][iSecondaryIdx]);
	save_cmb_action (m_cmbWatch_Sorted,       m_vPref[11][iSecondaryIdx]);
}

/////////////////////////////////////////////////////////////////////////////
void TOptionsNavigatePage::fill_and_select (CComboBox & cmb, int iAction)
{

	CString strLoader;
	int     i, iAt;

	// we don't need to empty out & refill, since the choices are static
	if (cmb.GetCount() == 0)
	{
		// fill with everything
		for ( i = 0; i < (sizeof(vNavMap)/sizeof(vNavMap[0])); i++)
		{
			strLoader.LoadString (vNavMap[i].idCmd);       // Y has the stringID

			// This is probably a string with Status-line-text + \n + tooltip-text
			iAt = cmb.AddString (strLoader.SpanExcluding ("\n") );

			cmb.SetItemData (iAt, vNavMap[i].byAction);      // X is the ActionCode
		}
	}

	bool fFound = false;
	// select the proper one
	for ( i = 0; i < cmb.GetCount(); i++)
	{
		if ((int) cmb.GetItemData (i) == iAction)
		{
			cmb.SetCurSel (i);
			fFound = true;
			break;
		}
	}

	if (!fFound)
		cmb.SetCurSel (0);
}

/////////////////////////////////////////////////////////////////////////////
void TOptionsNavigatePage::save_cmb_action (CComboBox & cmb, int & iAction)
{
	int idx = cmb.GetCurSel ();
	if (CB_ERR == idx)
		iAction = 0;
	else
		iAction = (int)cmb.GetItemData(idx);
}

// ---------------------------------------------------------------------------
void TOptionsNavigatePage::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iIndex = m_tab.GetCurSel ();

	if (0 == iIndex)
	{
		// the new mode is Offline, so save the ONLINE stuff
		SaveData (true);
		ShowOnlineData (false);
	}
	else if (1 == iIndex)
	{
		// the new mode is Online, so save the OFFLINE stuff
		SaveData (false);
		ShowOnlineData (true);
	}

	*pResult = 0;
}

BOOL TOptionsNavigatePage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp ()->HtmlHelp((DWORD)"preferences-spacebar.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_NAVIGATE);

	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsNavigatePage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}

