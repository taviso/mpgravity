/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vcrdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.2  2008/09/19 14:52:25  richard_wood
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

// vcrdlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "vcrdlg.h"
#include "picksvr.h"
#include "fileutil.h"
#include "servcp.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void get_path_to_newsdb (CString & path)
{
	TServerCountedPtr cpServer;
	CString serverDir = cpServer->GetServerDatabasePath();

	int idx = serverDir.ReverseFind('\\');

	if (idx > 2)
		path = serverDir.Left(idx);
}

/////////////////////////////////////////////////////////////////////////////
// TVCRDialog dialog

TVCRDialog::TVCRDialog(CWnd* pParent, void * pVoidInfo,
					   const CString & cmdline_vcrfile)
					   : CDialog(TVCRDialog::IDD, pParent)
{
	m_pDLInfo = (ListDownloadInfo*) pVoidInfo;

	m_pOutList = 0;

	m_strCmdLineVCRFile = cmdline_vcrfile;


	m_fExitGravity = FALSE;
}

void TVCRDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DATETIME_TIME, m_ctlTime);
	DDX_Control(pDX, IDC_DATETIME_DATE, m_ctlDate);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Check(pDX, IDC_CBX_EXITGRAV, m_fExitGravity);
}

BEGIN_MESSAGE_MAP(TVCRDialog, CDialog)
		ON_BN_CLICKED(IDC_RBT_STARTAT, OnRbtStartat)
	ON_BN_CLICKED(IDC_RBT_STARTNOW, OnRbtStartnow)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, OnTreeClick)
	ON_BN_CLICKED(IDC_VCR_SAVE, OnVcrSave)
	ON_BN_CLICKED(IDC_VCR_LOAD, OnVcrLoad)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TVCRDialog message handlers

void TVCRDialog::OnOK()
{
	// TODO: Add extra validation here

	CDialog::OnOK();

	// if we started up from the commandline , then don't save out 'default.vcr'
	if (m_strCmdLineVCRFile.IsEmpty())
		AutoSave ();

	if (IsDlgButtonChecked (IDC_RBT_STARTAT))
	{
		m_fSpecificTime = true;
		// get the date from one and the time from the other
		SYSTEMTIME t1;    m_ctlDate.GetTime (&t1);
		SYSTEMTIME t2;    m_ctlDate.GetTime (&t2);
		t2.wYear = t1.wYear;
		t2.wMonth = t1.wMonth;
		t2.wDay   = t1.wDay;
		t2.wDayOfWeek = t1.wDayOfWeek;
		COleDateTime tmp(t1);

		m_oleTime = tmp;
	}
	else
		m_fSpecificTime = false;

	create_action_list ();
}

// --------------------------------------------------------------------------
void TVCRDialog::AutoSave ()
{
	// auto save to DEFAULT.VCR
	CString defaultDir;
	get_path_to_newsdb ( defaultDir );
	TPath fullpath (defaultDir, "default.vcr");

	CFileStatus status;

	// clear out 'default.vcr'
	try
	{
		if (CFile::GetStatus (fullpath, status))
			CFile::Remove (fullpath);

		CStdioFile fl;
		CFileException fe;

		if (fl.Open(fullpath,
			CFile::modeCreate | CFile::typeText | CFile::modeWrite,
			&fe))
		{
			// save out to 'default.vcr'
			SaveToFile (fl);
		}
	}
	catch (CFileException *fee)
	{
		fee->ReportError ();
		fee->Delete();
	}
}

// --------------------------------------------------------------------------
BOOL TVCRDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_imgList.Create (IDB_VCRCHECK,
		12,             // width of 1 frame
		1,              // grow factor
		RGB(255,0,255)  // transparent color (hot purple)
		);

	m_tree.SetImageList (&m_imgList, TVSIL_STATE);

	// This list should already be sorted by server name
	POSITION pos = m_pDLInfo->GetHeadPosition();

	HTREEITEM hti;
	while (pos)
	{
		TServerDownloadInfo * pInfo = m_pDLInfo->GetNext (pos);

		TVINSERTSTRUCT  tvi;

		ZeroMemory (&tvi, sizeof(tvi));
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask =  TVIF_TEXT | TVIF_STATE;
		tvi.item.state = INDEXTOSTATEIMAGEMASK(1);
		tvi.item.stateMask = TVIS_STATEIMAGEMASK;
		tvi.item.pszText = (LPTSTR)(LPCTSTR) pInfo->m_strServerName;

		hti = m_tree.InsertItem ( &tvi );

		// Groups are listed by the True name, no nicknames here
		POSITION p2 = pInfo->m_lstGroups.GetHeadPosition ();
		while (p2)
		{

			TVINSERTSTRUCT  tv2;
			ZeroMemory (&tv2, sizeof(tv2));

			tv2.hParent = hti;

			tv2.item.mask =  TVIF_TEXT | TVIF_STATE;
			tv2.item.state = INDEXTOSTATEIMAGEMASK(1);
			tv2.item.stateMask = TVIS_STATEIMAGEMASK;
			tv2.item.pszText = (LPTSTR)(LPCTSTR) pInfo->m_lstGroups.GetNext (p2);

			m_tree.InsertItem ( &tv2 );
		}

		m_tree.SortChildren (hti);
	}

	// user specifies date & time
	CheckRadioButton (IDC_RBT_STARTNOW, IDC_RBT_STARTAT, IDC_RBT_STARTNOW);

	// either run the vcr settings specified on the command line
	//   or load the default vcr settings.

	CString defaultDir;
	get_path_to_newsdb ( defaultDir );

	if (m_strCmdLineVCRFile.IsEmpty())
	{
		TPath fullpath (defaultDir, "default.vcr");

		CFileStatus status;

		// check if file exists
		if (CFile::GetStatus( fullpath, status ))
			VcrLoadFile (fullpath);
	}
	else
	{
		bool fPostMsg  = false;
		// assume this is the full filespec
		if (VcrLoadFile (m_strCmdLineVCRFile) == 0)
		{
			fPostMsg = true;
		}
		else
		{
			// maybe this is just a filename and we have to provide ..\newsdb

			TPath fullpath(defaultDir, m_strCmdLineVCRFile);

			if (VcrLoadFile (fullpath) == 0)
				fPostMsg = true;
		}

		if (fPostMsg)
			PostMessage (WM_COMMAND, IDOK);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------------
// Returns 0 for success.
int TVCRDialog::create_action_list ()
{
	m_tree.SetRedraw (FALSE);

	m_pOutList = new ListVCRAction;

	HTREEITEM hOneRoot;

	hOneRoot = m_tree.GetRootItem ();

	while (NULL != hOneRoot)
	{

		ListVCRAction lstTemp;

		if (m_tree.ItemHasChildren (hOneRoot))
		{
			HTREEITEM hChild = m_tree.GetChildItem (hOneRoot);
			CString grp;

			// process first child
			if (m_tree.GetCheck (hChild))
			{
				grp = m_tree.GetItemText (hChild);
				lstTemp.AddTail (  new TVCRActionGroup ( grp ) );
			}
			HTREEITEM hSib;
			while ( (hSib = m_tree.GetNextSiblingItem (hChild)) != NULL)
			{
				if (m_tree.GetCheck (hSib))
				{
					grp = m_tree.GetItemText (hSib);
					lstTemp.AddTail (  new TVCRActionGroup ( grp ) );
				}
				hChild = hSib;
			}
		}

		if (lstTemp.GetCount() > 0)
		{
			// add server first
			CString svr =  m_tree.GetItemText (hOneRoot);
			m_pOutList->AddTail ( new TVCRActionServer (svr ) );

			m_pOutList->AddTail ( &lstTemp );

			lstTemp.RemoveAll ();
		}

		hOneRoot = m_tree.GetNextSiblingItem ( hOneRoot );

	} // while loop

	int count = m_pOutList->GetCount();
	if (count > 0 && m_fSpecificTime)
	{
		m_pOutList->AddHead ( new TVCRActionStart ( m_oleTime ) );
	}
	if (count > 0 && m_fExitGravity)
	{
		m_pOutList->AddTail ( new TVCRActionExit () );
	}
	return 0;
}

void TVCRDialog::OnRbtStartat()
{
	m_ctlTime.EnableWindow (TRUE);
	m_ctlDate.EnableWindow (TRUE);
}

void TVCRDialog::OnRbtStartnow()
{
	m_ctlTime.EnableWindow (FALSE);
	m_ctlDate.EnableWindow (FALSE);
}

void TVCRDialog::OnTreeClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	POINT pt;
	DWORD dwPos;

	// we must find out what item was clicked
	// Find out where the cursor was for this message
	dwPos = GetMessagePos();
	pt.x = LOWORD(dwPos);
	pt.y = HIWORD(dwPos);

	::MapWindowPoints(HWND_DESKTOP, m_tree.GetSafeHwnd(), &pt, 1);

	TVHITTESTINFO  hti;
	hti.pt     = pt;
	hti.hItem  = 0;

	HTREEITEM hTi = m_tree.HitTest ( &hti );
	if (0 == hTi)
		return;

	if (hti.flags & TVHT_ONITEMSTATEICON)
	{
		DWORD dw = m_tree.GetItemState (hti.hItem , TVIS_STATEIMAGEMASK);

		dw = dw >> 12;

		BOOL fServerItem = (NULL == m_tree.GetParentItem (hti.hItem));

		bool fIsNowChecked = false;

		if (1 == dw)
		{
			ChangeToChecked ( hti.hItem, true );
			fIsNowChecked = true;
		}
		else if (2 == dw)
			ChangeToChecked ( hti.hItem, false );
		else if (3 == dw)
			ChangeToChecked ( hti.hItem, false );

		if (fServerItem)
		{
			ApplyParentToChildren (hti.hItem, fIsNowChecked);
		}
		else
		{
			ApplyChildrenToParent (hti.hItem);
		}
	}

	*pResult = 0;
}

void TVCRDialog::ApplyChildrenToParent (HTREEITEM hti)
{
	HTREEITEM hDad = m_tree.GetParentItem (hti);

	HTREEITEM hKid = m_tree.GetChildItem (hDad);

	// survey kids

	int iChecked = 0;
	int iTotal   = 0;

	if (!hKid)
		return;

	iTotal ++;

	if (IsChecked (hKid))
		iChecked ++;

	while ((hKid = m_tree.GetNextSiblingItem(hKid)) != 0)
	{
		iTotal ++;

		if (IsChecked (hKid))
			iChecked ++;
	}

	// ----- tally results

	if (iChecked == 0)
	{
		ChangeToChecked (hDad, false);
	}
	else if (iChecked > 0 && iTotal == iChecked)
		ChangeToChecked (hDad, true);
	else
		ChangeToGrey (hDad);
}

// --------------------------------------------------------------------------
void TVCRDialog::ApplyParentToChildren (HTREEITEM hti, bool IsNowChecked)
{
	HTREEITEM hKid = m_tree.GetChildItem (hti);

	if (!hKid)
		return;

	ChangeToChecked (hKid, IsNowChecked);

	while ((hKid = m_tree.GetNextSiblingItem (hKid)) != 0)
		ChangeToChecked (hKid, IsNowChecked);
}

// --------------------------------------------------------------------------
void TVCRDialog::ChangeToChecked (HTREEITEM hti, bool fCheck)
{
	if (fCheck)
		m_tree.SetItemState (hti, INDEXTOSTATEIMAGEMASK(2), TVIS_STATEIMAGEMASK);
	else
		m_tree.SetItemState (hti, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
}

// --------------------------------------------------------------------------
void TVCRDialog::ChangeToGrey (HTREEITEM hti)
{
	m_tree.SetItemState (hti, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
}

// --------------------------------------------------------------------------
bool TVCRDialog::IsChecked (HTREEITEM hti)
{
	DWORD dw = m_tree.GetItemState (hti, TVIS_STATEIMAGEMASK);

	dw = dw >> 12;

	return (2 == dw);
}

// --------------------------------------------------------------------------
// VCR Version: 1
// Server:
// Group:
// Exit:
//

void TVCRDialog::OnVcrSave()
{
	CString fileSpec;

	CString defaultDir ;
	CString defaultFileName;

	defaultFileName.LoadString (IDS_VCRSETTINGS);

	get_path_to_newsdb ( defaultDir );

	BOOL fRet = TFileUtil::GetOutputFilename(
		&fileSpec,
		this,
		IDS_CAPT_SAVEVCR,  //defaultTitleStrID
		&defaultDir,
		0,
		IDS_FILTER_VCR,
		defaultFileName);

	if (fRet)
	{
		CStdioFile fl;
		CFileException fe;

		if (fl.Open(fileSpec,
			CFile::modeCreate | CFile::typeText | CFile::modeWrite,
			&fe))
			SaveToFile (fl);
	}
}

// --------------------------------------------------------------------------
void TVCRDialog::SaveToFile (CStdioFile & fl)
{
	// Write preamble

	fl.WriteString (_T("Gravity VCR File Version: 1\n"));

	bool      fEmptyFile = true;
	HTREEITEM hOneRoot;

	hOneRoot = m_tree.GetRootItem ();

	while (NULL != hOneRoot)
	{

		CStringList lstTempGroups;

		if (m_tree.ItemHasChildren (hOneRoot))
		{
			HTREEITEM hChild = m_tree.GetChildItem (hOneRoot);

			while ( NULL != hChild)
			{
				if (m_tree.GetCheck (hChild))
					lstTempGroups.AddTail ( m_tree.GetItemText (hChild) );

				hChild = m_tree.GetNextSiblingItem (hChild);
			}
		}

		// see if this server had any Checked groups at all
		if (lstTempGroups.GetCount() > 0)
		{

			// add server first
			CString svr =  m_tree.GetItemText (hOneRoot);

			CString out = "Server:" + svr + "\n";
			fl.WriteString (out);

			POSITION pos =  lstTempGroups.GetHeadPosition ();
			while (pos)
			{
				CString grp = "Group:" + lstTempGroups.GetNext (pos) + "\n";
				fl.WriteString (grp);
			}

			fEmptyFile = false;
		}

		hOneRoot = m_tree.GetNextSiblingItem ( hOneRoot );

	} // while loop

	if ( !fEmptyFile && IsDlgButtonChecked (IDC_CBX_EXITGRAV) )
		fl.WriteString (_T("Exit: when done\n"));
}

// --------------------------------------------------------------------------
void TVCRDialog::OnVcrLoad()
{
	CString fileSpec;

	CString defaultDir ;

	get_path_to_newsdb ( defaultDir );

	if (TFileUtil::GetInputFilename ( &fileSpec,
		this,
		IDS_CAPT_LOADVCR,      // stringid of Caption
		&defaultDir,           // initial dir
		0,                     // more flags
		IDS_FILTER_VCR ))
	{
		VcrLoadFile ( fileSpec );
	}
}

// --------------------------------------------------------------------------
// Returns 0 for success
int TVCRDialog::VcrLoadFile (const CString & fileSpec)
{
	CStdioFile     fl;
	CFileException fe;

	if (fl.Open (fileSpec, CFile::modeRead, &fe))
	{
		ReadFromFile (fl);

		return 0;
	}

	return 1;
}

// --------------------------------------------------------------------------
void TVCRDialog::ReadFromFile (CStdioFile & fl)
{
	bool fValidFile = false;
	CString line;

	HTREEITEM htiServer = 0;

	while (fl.ReadString ( line ))
	{

		if (0 == line.Find (_T("Gravity VCR File Version:")))
		{
			fValidFile = true;
		}
		else if (fValidFile && 0 == line.Find (_T("Server:")))
		{
			htiServer = 0;
			CString serverName = line.Mid (7);

			// find hti
			HTREEITEM hOneRoot = m_tree.GetRootItem ();
			while (NULL != hOneRoot)
			{
				if (serverName == m_tree.GetItemText (hOneRoot))
				{
					htiServer = hOneRoot;
					break;
				}
				hOneRoot = m_tree.GetNextSiblingItem (hOneRoot);
			}

		}
		else if (fValidFile && 0 == line.Find (_T("Group:")))
		{
			CString groupName = line.Mid (6);

			FindChildAndCheck (htiServer, groupName);
		}
		else if (fValidFile && 0 == line.Find (_T("Exit:")))
		{
			CheckDlgButton (IDC_CBX_EXITGRAV, 1);
		}
	}
}

// ------------------------------------------------------------------------------
void TVCRDialog::FindChildAndCheck (HTREEITEM htiServer, const CString & group)
{
	if (0 == htiServer)
		return;
	if (m_tree.ItemHasChildren (htiServer))
	{
		HTREEITEM hChild = m_tree.GetChildItem (htiServer);

		while ( NULL != hChild)
		{
			if (group ==  m_tree.GetItemText (hChild) )
			{
				ChangeToChecked (hChild, true);

				ApplyChildrenToParent (hChild);
				break;
			}

			hChild = m_tree.GetNextSiblingItem (hChild);
		}
	}
}
