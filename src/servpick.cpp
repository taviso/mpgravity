/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: servpick.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.2  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:50  richard_wood
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

// servpick.cpp -- add/remove server dialog

#include "stdafx.h"
#include "news.h"
#include "Servpick.h"
#include "newsdb.h"
#include "globals.h"
#include "regutil.h"
#include <winreg.h>
#include "tsetpg.h"                 // server setup PropertyPage
#include "tserverspage.h"
#include "picksvr.h"
#include "servcp.h"
#include "hints.h"
#include "newsview.h"
#include "genutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL  GetCurrentServerName(TNewsDB* pDB, CString & server);
extern void SetCurrentServer (const CString & mruServer);

extern BOOL gfShutdownRename;

// To Do:
//    - handle checking for existing server anchored windows
//      when removing a server (code in CNewsApp::AddRemove
//      should check it before switching
//    - don't allow user to leave w/out an open server
//    - review each function carefully
//    + handle deleting the existing server (must close it first)
//    X should we prompt to ask the user if they want to open
//      the server that was just added?
//    + clean up CNewsApp::OnCmdMsg and SwitchServer so that
//      switch server can be used from something other than
//      the server menu items
//    + double-clicking opens


/////////////////////////////////////////////////////////////////////////////
TPickServerDlg::TPickServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TPickServerDlg::IDD, pParent)
{
	m_fExitNoServer     = FALSE;
	m_fRemovedServerIn  = FALSE;
	m_bExistingOnly		= false;
}

TPickServerDlg::TPickServerDlg(bool bExistingOnly, CWnd* pParent /*= NULL*/)
	: CDialog(TPickServerDlg::IDD, pParent)
{
	m_fExitNoServer     = FALSE;
	m_fRemovedServerIn  = FALSE;
	m_bExistingOnly		= bExistingOnly;
}

void TPickServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TPickServerDlg, CDialog)
	ON_BN_CLICKED(IDC_PICKSERVER_NEW, OnPickserverNew)
	ON_BN_CLICKED(IDC_PICKSERVER_REMOVE, OnPickserverRemove)
	ON_LBN_DBLCLK(IDC_PICKSERVER_LBX, OnDblclkPickserverLbx)
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
END_MESSAGE_MAP()

static BOOL ServerNameExists(LPCTSTR pchName)
{
	TServerIterator it(gpStore);
	TNewsServer *pServer;
	while (NULL != (pServer = it.Next()))
		if (!pServer -> GetNewsServerName().CompareNoCase(pchName))
			return TRUE;

	return FALSE;
}

void TPickServerDlg::OnPickserverNew()
{
	if (m_bExistingOnly) return;

	bool bImport = false;
	TSetupPage pgHuman(false);
	TServersPage pgServer;

	TNewsServer * pServer = 0;

	if (TRUE == GetServerProfile(pgHuman, pgServer, bImport))
	{
		if (ServerNameExists(pgServer.m_newsServer))
		{
			MsgResource(IDS_ERR_SERVER_NAME_EXISTS);
			return;
		}

		if ((pServer = fnCreateServer(pgHuman, pgServer)) != 0)
		{
			CListBox * pLbx = (CListBox *)GetDlgItem(IDC_PICKSERVER_LBX);
			pLbx->SetCurSel(pLbx->AddString(pServer->GetNewsServerName()));
			EmptyServerMenu();
			BuildServerMenu();
		}
		else
		{
			// ???? mondo error
		}
	}
}

void TPickServerDlg::OnOK()
{
	// get the server that has selection and
	// map it back to a server pointer

	CString           server;
	TServerIterator   it(gpStore);
	TNewsServer *     pServer = 0;
	int               index = -1;
	CListBox *        pLbx = (CListBox *)GetDlgItem(IDC_PICKSERVER_LBX);

	index = pLbx->GetCurSel();

	if (index >= 0)
		pLbx->GetText(index, m_serverOut);

	CDialog::OnOK();
}

BOOL TPickServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CListBox *pLbx = (CListBox *)GetDlgItem(IDC_PICKSERVER_LBX);

	TServerIterator it(gpStore);
	TNewsServer *pServer;

	while (NULL != (pServer = it.Next()))
	{
		pLbx->AddString(pServer->GetNewsServerName ());
	}

	if (!m_serverIn.IsEmpty())
		pLbx->SelectString(-1, m_serverIn);

	if (m_bExistingOnly)
	{
		GetDlgItem(IDC_PICKSERVER_NEW)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PICKSERVER_REMOVE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPEN)->SetWindowText("Select");
	}

	return TRUE;
}

void TPickServerDlg::OnCancel()
{
	if (m_bExistingOnly) 
	{
		CDialog::OnCancel();
		return;
	}

	CString  message;
	CListBox *pLbx = (CListBox*)GetDlgItem(IDC_PICKSERVER_LBX);
	if (0 == pLbx->GetCount())
	{
		message.LoadString(IDS_WARN_NOSERVER);
		if (IDNO == AfxMessageBox(message, MB_YESNO))
			return;
		else
		{
			m_fExitNoServer = TRUE;

			// arghhhhhhhhhhhhhh....  wish there was a better way...
			// but the hook is already there so I'll use it...
			gfShutdownRename = TRUE;
		}
		CDialog::OnCancel();
	}
	else if (m_fRemovedServerIn)
	{
		CString selectedServer;
		int  index = pLbx->GetCurSel();

		if (LB_ERR == index)
			index = 0;

		pLbx->GetText(index, selectedServer);

		message.Format(IDS_QUERY_OPEN_SERVER, (LPCTSTR)selectedServer);
		if (IDNO == AfxMessageBox(message, MB_YESNO | MB_ICONQUESTION))
			return;

		// call the OK logic
		OnOK();
	}
	else
		CDialog::OnCancel();
}

void TPickServerDlg::OnPickserverRemove()
{
	if (m_bExistingOnly) return;

	CString           server;
	TServerIterator   it(gpStore);
	TNewsServer *     pServer = 0;
	int               index = -1;
	CListBox *        pLbx = (CListBox *)GetDlgItem(IDC_PICKSERVER_LBX);

	index = pLbx->GetCurSel();

	if (index >= 0)
		pLbx->GetText(index, server);
	else
	{
		AfxMessageBox(IDS_ERR_NO_SERVER);
		return;
	}

	while (NULL != (pServer = it.Next()))
	{
		if (pServer->GetNewsServerName().CompareNoCase(server) == 0)
		{
			CString temp;
			CString str; str.LoadString(IDS_PROMPT_REMOVE);
			temp.Format(str, pServer->GetNewsServerName());
			if (IDYES == AfxMessageBox(temp, MB_YESNO))
			{
				// if the server they are trying to remove happens to be
				// the one that is open, then we have to exit it first...
				if (GetCountedActiveServerName().CompareNoCase(server) == 0)
				{
					GetNewsView()->GetDocument()->UpdateAllViews(NULL, VIEWHINT_EMPTY);
					GetNewsView()->CloseCurrentNewsgroup();
					gptrApp->ExitServer();

					// empty the newsgroup list
					GetNewsView()->EmptyListbox();
				}

				gpStore->RemoveServer(pServer->GetNewsServerName());
				pLbx->DeleteString(index);
				if (pLbx->GetCount())
				{
					if (index < pLbx->GetCount())
						pLbx->SetCurSel(index);
					else
						pLbx->SetCurSel(pLbx->GetCount() - 1);
				}
				EmptyServerMenu();
				BuildServerMenu();
				if (0 == server.CompareNoCase(m_serverIn))
					m_fRemovedServerIn = TRUE;
			}
			return;
		}
	}
}

void TPickServerDlg::OnDblclkPickserverLbx()
{
	CListBox *pLbx = (CListBox *)GetDlgItem(IDC_PICKSERVER_LBX);

	int index = pLbx->GetCurSel();
	if (index >= 0)
		PostMessage(WM_COMMAND, IDOK, 0);
}

void TPickServerDlg::OnOpen()
{
	CListBox *pLbx = (CListBox *)GetDlgItem(IDC_PICKSERVER_LBX);

	int index = pLbx->GetCurSel();
	if (index < 0)
	{
		AfxMessageBox(IDS_ERR_NO_SERVER);
		return;
	}

	PostMessage(WM_COMMAND, IDOK, 0);
}
