/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tcompdlg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:59  richard_wood
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

// tcompdlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "tcompdlg.h"
#include "custmsg.h"
#include "globals.h"
#include "ngutil.h"
#include "utilpump.h"
#include "server.h"
#include "servcp.h"           // TServerCountedPtr
#include "nglist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TCompactDlg dialog

TCompactDlg::TCompactDlg(CWnd* pParent /*=NULL*/)
: CDialog(TCompactDlg::IDD, pParent)
{


	m_fStopCompacting = FALSE;
	m_fPurgeGroups    = FALSE;
}

TCompactDlg::~TCompactDlg(void)
{
}

void TCompactDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMPACTING_PROGRESS, m_progress);
}

BEGIN_MESSAGE_MAP(TCompactDlg, CDialog)
		ON_WM_DESTROY()

	ON_MESSAGE (WMU_START_COMPACT, StartCompact)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TCompactDlg message handlers

BOOL TCompactDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// get it and lock it.
	m_pNewsServer = GetCountedActiveServer();
	m_pNewsServer->AddRef ();

	// add the outbox to the list before we start...

	TNewsGroupDB * pOutbox = m_pNewsServer->GetOutbox ();

	if (pOutbox)
		m_groupsToPurge.Add (pOutbox);

	m_progress.SetRange (0, m_groupsToPurge.GetSize ());
	m_progress.SetStep (1);
	m_progress.SetPos (0);

	PostMessage (WMU_START_COMPACT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

afx_msg LRESULT TCompactDlg::StartCompact(WPARAM wP, LPARAM lP)
{
	Compact ();

	// let's quit now as if the user clicked cancel

	HWND  wnd;
	GetDlgItem (IDD_COMPACTING_STOP, &wnd);
	PostMessage (WM_COMMAND, IDCANCEL, BN_CLICKED);
	return 0;
}

UINT TCompactDlg::Compact ()

{
	try
	{
		TNewsGroupDB * pNG;
		TNewsGroupArray& vNewsGroups = m_pNewsServer->GetSubscribedArray();
		BOOL        fStopped = FALSE;
		CWinApp    *pApp = AfxGetApp();

		ShowWindow(SW_SHOWNORMAL);
		UpdateWindow();

		// lock the group array with a write lock....
		vNewsGroups.WriteLock ();

		int tot = m_groupsToPurge.GetSize();
		for (int i = 0; (i < tot) && !m_fStopCompacting; ++i)
		{
			// pump any messages in our queue
			MSG msg;
			while (::PeekMessage(&msg, m_hWnd, NULL, NULL, PM_REMOVE))
				// this should route to all modeless dialogs too
				if (!pApp->PreTranslateMessage(&msg))
					::DispatchMessage(&msg);

			pNG = m_groupsToPurge.GetAt(i);

			if (pNG->IsOpen ())
				continue;
			pNG->Open ();

			GetDlgItem (IDC_COMPACTING_FILE)->SetWindowText(pNG->GetName());
			if (m_fPurgeGroups)
			{
				const CString & name = pNG->GetName();

				if (0 == name.CompareNoCase (_T("outbox")))
				{
					// do no purging on outbox
				}
				else if (0 == name.CompareNoCase (_T("drafts")))
				{
					// do no drafts purging
				}
				else
				{
					pNG->PurgeByDate (true);
					CTime n = CTime::GetCurrentTime();
					UtilSetLastPurgeTime (static_cast<TNewsGroup *>(pNG), n);
				}
			}

			// virtual function
			pNG->MonthlyMaintenance ();

			pNG->Compact(TRUE);

			if (0 == pNG->GetName ().CompareNoCase (_T("outbox")))
			{
			}
			else if (0 == pNG->GetName ().CompareNoCase (_T("drafts")))
			{
			}
			else
				UtilSetLastCompactTime(static_cast<TNewsGroup *> (pNG),
				CTime::GetCurrentTime());

			m_progress.StepIt();
		}

		/// removed   07/27/99  amc, since purge by date writes out the pNG
		//m_pNewsServer->SaveSubscribedGroups ();

		vNewsGroups.UnlockWrite ();
		return 0;
	}
	catch (TException *rExcept)
	{
		rExcept->PushError (IDS_ERR_COMPACTING, kError);
		TException *ex = new TException(*rExcept);
		rExcept->Delete();
		throw(ex);
		return 0;
	}
}

///////////////////////////////////////////////////////////////////
// the close box comes thru here
// the ESC key comes thru here
void TCompactDlg::OnCancel ()

{
	m_fStopCompacting = TRUE;
	CDialog::OnCancel ();   // calls EndDialog
}

///////////////////////////////////////////////////////////////////
// - free resources
void TCompactDlg::OnDestroy()
{
	// release reference
	m_pNewsServer->Release ();
	m_pNewsServer = 0;

	// TODO: Add your message handler code here and/or call default
	CDialog::OnDestroy();
}
