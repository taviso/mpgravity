/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: dialuppg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

// DialupPg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "DialupPg.H"
#include "dialman.h"
#include "dialmane.h"
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TDialupManager dialMgr;

/////////////////////////////////////////////////////////////////////////////
// TDialupPage property page

IMPLEMENT_DYNCREATE(TDialupPage, CPropertyPage)

TDialupPage::TDialupPage() : CPropertyPage(TDialupPage::IDD)
{

	m_connectionName = _T("");
	m_fForceConnection = FALSE;
	m_fUseExistingConnection = FALSE;
	m_fPromptBeforeConnecting = FALSE;
	m_fPromptBeforeDisconnecting = FALSE;
	m_fDisconnectIfWeOpened = FALSE;
	m_fForceDisconnect = FALSE;


	m_fConnectionSet = FALSE;
}

TDialupPage::~TDialupPage()
{
}

void TDialupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_CBString(pDX, IDC_DIALUP_CONNECTION_COMBO, m_connectionName);
	DDX_Check(pDX, IDC_DIALUP_FORCE_CONNECTION, m_fForceConnection);
	DDX_Check(pDX, IDC_DIALUP_USE_EXISTING, m_fUseExistingConnection);
	DDX_Check(pDX, IDC_DIALUP_PROMPT_FIRST, m_fPromptBeforeConnecting);
	DDX_Check(pDX, IDC_DIALUP_PROMPT_DISCONNECT, m_fPromptBeforeDisconnecting);
	DDX_Check(pDX, IDC_DIALUP_IF_WE_OPENED, m_fDisconnectIfWeOpened);
	DDX_Check(pDX, IDC_DIALUP_DISCONNECT, m_fForceDisconnect);

}

BEGIN_MESSAGE_MAP(TDialupPage, CPropertyPage)
	ON_BN_CLICKED(IDC_DIALUP_NEW_CONNECTION, OnDialupNewConnection)
	ON_BN_CLICKED(IDC_DIALUP_CONNECTION_PROPS, OnDialupConnectionProps)
	ON_BN_CLICKED(IDC_DIALUP_FORCE_CONNECTION, OnDialupForceConnection)
	ON_BN_CLICKED(IDC_DIALUP_DISCONNECT, OnDialupDisconnect)
	ON_CBN_SELCHANGE(IDC_DIALUP_CONNECTION_COMBO, OnSelchangeDialupConnectionCombo)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TDialupPage message handlers

void TDialupPage::OnDialupNewConnection()
{
	// if the presses "New Connection" to create a new dialup
	// networking connection, then this function is called
	// RasCreatePhonebookEntry is a Win32 call that sprouts
	// a common dialog

	if (!dialMgr.RasCreatePhonebookEntry(this->m_hWnd, NULL))
	{
		// update the connection list and set focus to the
		// new item.  Update the name and password fields
		// (new stuff doesn't have a name and password yet)
		FillConnectionList(TRUE);
		UpdateNamePassword ();
	}
}

/////////////////////////////////////////////////////////////////////////////

void TDialupPage::OnDialupConnectionProps()
{
	int curr;
	CComboBox *pCombo = (CComboBox *) GetDlgItem (IDC_DIALUP_CONNECTION_COMBO);

	// If the user clicks the Properties... button, then
	// call the Win32 Ras stuff to pop up a dialog to
	// edit the dialup network settings.

	if ((curr = pCombo->GetCurSel()) >= 0)
	{
		DWORD    rc;
		TCHAR connection[256];

		pCombo->GetLBText(curr, connection);
		rc = dialMgr.RasEditPhonebookEntry (this->m_hWnd,
			NULL,
			connection);

		if (0 == rc)
			UpdateNamePassword ();
	}
}

/////////////////////////////////////////////////////////////////////////////

void TDialupPage::OnDialupForceConnection()
{
	EnableDisable ();
}

/////////////////////////////////////////////////////////////////////////////
// OnDialupDisconnect - Need to call EnableDisable...
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::OnDialupDisconnect()
{
	EnableDisable ();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::FillConnectionList (BOOL fUpdate)

{
	// fill the combo box with the system defined connections
	RASENTRYNAME      entries[20];      // ??? will anyone have more than 20?
	DWORD             rc;
	DWORD             buffSize = sizeof (entries);
	DWORD             numEntries;
	CComboBox *       pCombo;

	entries[0].dwSize = sizeof (RASENTRYNAME);

	pCombo = (CComboBox *) GetDlgItem (IDC_DIALUP_CONNECTION_COMBO);

	if (!fUpdate)
		pCombo->ResetContent();

	rc = dialMgr.RasEnumEntries (NULL,          // reserved
		NULL,          // phonebook file - NULL : use default
		entries,       // pointer to the entry buffer
		&buffSize,     // size of our buffer
		&numEntries);  // number of entries returned

	// if we're updating, then we just add the new item to the combo box and
	// set the current selection, otherwise each item gets added...

	if (0 == rc)
	{
		for (int i = 0; i < int(numEntries); i++)
		{
			if (fUpdate)
			{
				int pos;
				if (pCombo->FindString (-1, entries[i].szEntryName) < 0)
				{
					pos = pCombo->AddString (entries[i].szEntryName);
					pCombo->SetCurSel (pos);
					return;
				}
			}
			else
				pCombo->AddString (entries[i].szEntryName);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::UpdateNamePassword()

{
	CComboBox *       pCombo;
	int               curr;
	RASDIALPARAMS     rasDialParams;
	BOOL              fPassword = FALSE;
	DWORD             rc;
	TConnectionDelta  delta;

	pCombo = (CComboBox *) GetDlgItem (IDC_DIALUP_CONNECTION_COMBO);

	curr = pCombo->GetCurSel ();
	if (curr >= 0)
	{

		// first, if there was a connection already, then see if
		// it changed, and if so update it

		UpdateConnectionDelta();
		rasDialParams.dwSize = sizeof (rasDialParams);
		pCombo->GetLBText (curr, rasDialParams.szEntryName);

		// if we've already changed the entry during this session,
		// then use values from it here...
		if (m_deltaMap.Lookup (rasDialParams.szEntryName, delta))
		{
			GetDlgItem (IDC_DIALUP_USERNAME)->SetWindowText (delta.m_connectionUser);
			GetDlgItem (IDC_DIALUP_PASSWORD)->SetWindowText (delta.m_connectionPassword);
			m_userName = delta.m_connectionUser;
			m_password = delta.m_connectionPassword;
		}
		else
		{
			// otherwise we get it from the registry...
			rc = dialMgr.RasGetEntryDialParams(NULL,
				&rasDialParams,
				&fPassword);

			GetDlgItem (IDC_DIALUP_USERNAME)->SetWindowText (rasDialParams.szUserName);
			if (fPassword)
				GetDlgItem (IDC_DIALUP_PASSWORD)->SetWindowText (rasDialParams.szPassword);
			else
				GetDlgItem (IDC_DIALUP_PASSWORD)->SetWindowText ("");
			m_userName = rasDialParams.szUserName;
			m_password = rasDialParams.szPassword;
		}
		m_connection = rasDialParams.szEntryName;
		m_fConnectionSet = TRUE;
	}
	else
		m_fConnectionSet = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// EnableDisable - Enable and disable controls based on the values of
//                 certain ones.
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::EnableDisable()

{
	BOOL  fChecked;

	// if the force connection box is checked, check two other boxes
	fChecked = ((CButton *) GetDlgItem (IDC_DIALUP_FORCE_CONNECTION))->GetCheck ();
	GetDlgItem (IDC_DIALUP_PROMPT_FIRST)->EnableWindow (fChecked);
	GetDlgItem (IDC_DIALUP_USE_EXISTING)->EnableWindow (fChecked);

	// if the close dialup box is checked, check two other boxes
	fChecked = ((CButton *) GetDlgItem (IDC_DIALUP_DISCONNECT))->GetCheck ();
	GetDlgItem (IDC_DIALUP_PROMPT_DISCONNECT)->EnableWindow (fChecked);
	GetDlgItem (IDC_DIALUP_IF_WE_OPENED)->EnableWindow (fChecked);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOL TDialupPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// clear the delta map

	m_deltaMap.RemoveAll ();

	// TODO: Add extra initialization here

	FillConnectionList ();

	// set selection
	if (!m_connectionName.IsEmpty())
	{
		CComboBox * pCombo = (CComboBox *) GetDlgItem (IDC_DIALUP_CONNECTION_COMBO);
		int pos;
		if ((pos = pCombo->FindString (-1, m_connectionName)) >= 0)
		{
			pCombo->SetCurSel (pos);
			UpdateNamePassword ();
		}
	}

	// enable and disable the checkboxes
	EnableDisable();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::OnSelchangeDialupConnectionCombo()
{
	UpdateNamePassword ();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::ReflectRasChanges ()

{
	// If the user made changes to the name and password of any entries and
	// the OK button is hit, then this function is called to update the
	// registry based on the changes the user has made to the page.

	POSITION          pos;
	CString           key;
	TConnectionDelta  delta;
	RASDIALPARAMS     parms;
	BOOL              fPass;

	parms.dwSize = sizeof (parms);

	pos = m_deltaMap.GetStartPosition();

	while (pos)
	{
		m_deltaMap.GetNextAssoc (pos, key, delta);
		_tcscpy (parms.szEntryName, delta.m_connectionName);
		if (0 == dialMgr.RasGetEntryDialParams (NULL,
			&parms,
			&fPass))
		{
			_tcscpy (parms.szUserName, delta.m_connectionUser);
			_tcscpy (parms.szPassword, delta.m_connectionPassword);
			_tcscpy (parms.szEntryName, delta.m_connectionName);
			dialMgr.RasSetEntryDialParams (NULL,
				&parms,
				delta.m_connectionPassword.IsEmpty ());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TDialupPage::UpdateConnectionDelta ()

{
	// if there is a connection see if the username or password changed, and
	// if so, update our delta map so that if the user presses OK the changes
	// get saved to the registry...

	if (m_fConnectionSet)
	{
		TConnectionDelta  delta;
		CString           tempName;
		CString           tempPassword;

		GetDlgItem (IDC_DIALUP_USERNAME)->GetWindowText (tempName);
		GetDlgItem (IDC_DIALUP_PASSWORD)->GetWindowText (tempPassword);

		if (m_userName.Compare (tempName) || m_password.Compare (tempPassword))
		{

			delta.m_connectionName = m_connection;
			delta.m_connectionUser = tempName;
			delta.m_connectionPassword = tempPassword;

			m_deltaMap.SetAt (m_connection, delta);
		}
	}
}

void TDialupPage::OnOK()
{
	// if there is currently a connection set, then check
	// to see if anything has changed about it and if so update
	// our delta map

	if (m_fConnectionSet)
		UpdateConnectionDelta ();

	// copy the changes from the delta map into the registry...
	ReflectRasChanges();
	CPropertyPage::OnOK();
}

BOOL TDialupPage::OnHelpInfo(HELPINFO*)
{
	AfxGetApp ()->HtmlHelp((DWORD)"servers-dial-up-tab.html", HH_DISPLAY_TOPIC);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TDialupPage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
