/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

// tdecdlg.cpp -- dialog to view decode jobs

#include "stdafx.h"              // precompiled header
#include "resource.h"            // IDS_*, IDD_FACTORY_IMAGE
#include "tdecdlg.h"             // this file's prototypes
#include "tdecthrd.h"            // gpDecodeThread
#include "tdecq.h"               // TDecodeQueue
#include "warndlg.h"             // WarnWithCBX()
#include "tglobopt.h"            // WarnOnDeleteBinary(), ...
#include "trendlg.h"             // TRenameDialog
#include "genutil.h"             // MsgResource(), ...
#include "rgui.h"                // SaveUtilDlg(), ...
#include "rgwarn.h"              // TRegWarn
#include "rgswit.h"              // TRegSwitch
#include "fileutil.h"            // NewsMessageBox()
#include "critsect.h"            // TEnterCSection
#include "news.h"                // CNewsApp
#include "newsdoc.h"             // CNewsDoc
#include "tdecthrd.h"            // gpsDecodeDialog
#include "tutlmsg.h"             // ID_WAITING_INSERT, ...
#include "gallery.h"             // ViewBinary()

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// IMPORTANT: keep this message map synchronized with the one in tutldlg.cpp
BEGIN_MESSAGE_MAP(TDecodeDialog, CDialog)
	ON_BN_CLICKED (IDC_DELETE_BINARY_FROM_DISK, OnDeleteBinaryFromDisk)
	ON_BN_CLICKED (IDC_DELETE_BINARY_FROM_LIST, OnDeleteBinaryFromList)
	ON_BN_CLICKED (IDC_SELECT_ALL_COMPLETED, OnSelectAllCompleted)
	ON_BN_CLICKED (IDC_RENAME, OnRename)
	ON_BN_CLICKED (IDC_VIEW_BINARY, OnViewBinary)
	ON_NOTIFY (NM_DBLCLK, IDC_PRINT_COMPLETED, OnFinishedDoubleClick)
	ON_BN_CLICKED(IDC_RETRYDEC, OnRetryDecodeJob)
	ON_BN_CLICKED (IDC_PRINT_UP, OnUtilQUp)
	ON_BN_CLICKED (IDC_PRINT_DOWN, OnUtilQDn)
	ON_BN_CLICKED (IDC_PRINT_DELETE, OnUtilQDel)
	ON_WM_DESTROY ()
	ON_MESSAGE (ID_WAITING_INSERT, OnWaitingInsert)
	ON_MESSAGE (ID_WAITING_INSERT_AT_TOP, OnWaitingInsertAtTop)
	ON_MESSAGE (ID_WAITING_DELETE, OnWaitingDelete)
	ON_MESSAGE (ID_CURRENT_CHANGE, OnCurrentChange)
	ON_MESSAGE (ID_FINISHED_INSERT, OnFinishedInsert)
	ON_MESSAGE (ID_FINISHED_DELETE, OnFinishedDelete)
	ON_MESSAGE (ID_FINISHED_CHANGED, OnFinishedChanged)
	ON_MESSAGE (ID_FINISHED_CHANGED_NO_HILIGHT, OnFinishedChangedNoHighlight)
	ON_MESSAGE (ID_WAITING_MOVE_UP, OnWaitingMoveUp)
	ON_MESSAGE (ID_WAITING_MOVE_DOWN, OnWaitingMoveDown)
	ON_MESSAGE (ID_PAUSE, OnPause)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PRINT_COMPLETED, OnCompletedSelChanged)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SELECT_ALL_WAITING, OnSelectAllWaiting)
	ON_BN_CLICKED(IDC_PRINT_MOVE_TO_TOP, OnUtilQMoveToTop)
	ON_BN_CLICKED(IDC_PAUSE, OnUtilQPause)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PRINT_QUEUE, OnWaitingSelChanged)
	ON_BN_CLICKED(IDC_FACTORY_AUTOSAVE, OnFactoryAutosave)
	ON_EN_KILLFOCUS(IDC_FACTORY_ASAVEMIN, OnAutoSaveMinutes)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TDecodeDialog::TDecodeDialog (CWnd *pParent /* = NULL */)
: TUtilityDialog (TDecodeDialog::IDD, pParent)
{
	// NOTE: the ClassWizard will add member initialization here
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnDeleteBinaryFromDisk()
{
	CWaitCursor wait;
	m_psCompleted -> SetRedraw (FALSE);

	// work backwards so we don't need to worry about adjusting indices
	BOOL        bWarned = FALSE;
	int         iCount = m_psCompleted -> GetItemCount ();
	CDWordArray sSelected;
	int         i, n;

	// first gather the indices.
	// Things to avoid: check if last item is selected
	//                  delete last item
	//                  adjust selection
	//                  is this selected?, then delete this?
	//                  end up deleting everything!
	//
	for (i = iCount - 1; i >= 0; i--)
		if (m_psCompleted -> IsSelected (i))
			sSelected.Add (i);

	for (n = 0; n < sSelected.GetSize(); n++) {
		i = (int) sSelected[n];
		bool fLastItem = n == sSelected.GetSize() - 1;

		CString strFullName = m_psCompleted -> GetItemText (i, 2);
		if (!strFullName.IsEmpty ()) {

			// issue warning
			if (!bWarned && gpGlobalOptions -> WarnOnDeleteBinary ()) {
				bWarned = TRUE;
				CString strWarning; strWarning.LoadString (IDS_CONFIRM_DELETE_ALL_SEL);
				BOOL bDoNotWarn;
				BOOL RC = WarnWithCBX (strWarning, &bDoNotWarn, (CWnd *) this);
				if (bDoNotWarn) {
					gpGlobalOptions -> SetWarnOnDeleteBinary (FALSE);
					TRegWarn *psRegWarning = gpGlobalOptions -> GetRegWarn ();
					psRegWarning -> Save ();
				}
				if (!RC)
					break;
			}

			if (!DeleteFile (strFullName)) {
				CString strError; strError.LoadString (IDS_ERR_DELETE);
				strError += " ";
				strError += strFullName;
				CString str; str.LoadString (IDS_PROG_NAME);
				MessageBox (strError, str);
			}

			// delete the thumbnail too, if it exists
			DeleteFile (strFullName + ".thu");
		}

		// delete the item from the listbox too. Adjust selection
		//   when performing our last remove.
		DeleteBinaryFromListByIndex (i, fLastItem);

		// this lets MFC clean up temporary handles, otherwise resource
		// utilization in win95 goes way up if we're processing many entries
		AfxGetApp () -> OnIdle (1);
	}

	m_psCompleted -> SetRedraw ();

	GotoDlgCtrl (m_psCompleted);
	SetButtonStates ();
	RefreshStatusBar ();
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnDeleteBinaryFromList()
{
	CWaitCursor wait;
	m_psCompleted -> SetRedraw (FALSE);

	// work backwards so we don't need to worry about adjusting indices
	int iCount = m_psCompleted -> GetItemCount ();
	CDWordArray sSelected;
	int         i, n;

	// first gather the indices
	for (i = iCount - 1; i >= 0; i--)
		if (m_psCompleted -> IsSelected (i))
			sSelected.Add (i);

	for (n = 0; n < sSelected.GetSize(); n++) {
		i = (int) sSelected[n];
		bool fLastItem = n == sSelected.GetSize() - 1;

		DeleteBinaryFromListByIndex (i, fLastItem);

		// this lets MFC clean up temporary handles, otherwise resource
		// utilization in win95 goes way up if we're processing many entries
		AfxGetApp () -> OnIdle (1);
	}

	m_psCompleted -> SetRedraw ();

	GotoDlgCtrl (m_psCompleted);
	SetButtonStates ();
	RefreshStatusBar ();
}

// -------------------------------------------------------------------------
void TDecodeDialog::DeleteBinaryFromListByIndex (int  iIndex,
												 bool fAdjustSelection /*=true*/)
{
	// get the uniqe id of the job
	int iID = (int) m_psCompleted -> GetItemData (iIndex);

	// remove from thread's data structure
	m_pThread -> RemoveCompletedJob (iID);

	// remove from list control
	m_psCompleted -> DeleteLine (iIndex, fAdjustSelection);
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnViewBinary()
{
	// return focus back to the completed-jobs listbox
	GotoDlgCtrl (m_psCompleted);

	int iCount = m_psCompleted -> GetItemCount ();
	for (int i = 0; i < iCount; i++)
		if (m_psCompleted -> IsSelected (i)) {
			CString strFullName = m_psCompleted -> GetItemText (i, 2);
			BOOL bBoss =
				((CNewsDoc *) ((CNewsApp *) AfxGetApp ()) -> GetGlobalNewsDoc ())
				-> PanicMode ();
			if (!strFullName.IsEmpty () && !bBoss) {
				ViewBinary (FALSE /* fPanic */, strFullName);

				// wait between launches... not waiting was causing confusion with
				// acdsee starting up okay but other instances which were in the
				// process of starting were getting DDE messages and as a result
				// were showing the wrong image (or something like that)
				Sleep (200);   // 1/5 second

				// some users like to mark article as read after viewing
				if (gpGlobalOptions -> GetRegSwitch () -> GetDecodeMarksRead () ==
					TRegSwitch::kReadAfterView)
					((TDecodeThread *) m_pThread) ->
					MarkArticlesRead (m_psCompleted -> GetJobID (i));
			}

			// this lets MFC clean up temporary handles, otherwise resource
			// utilization in win95 goes way up if we're processing many entries
			AfxGetApp () -> OnIdle (1);
		}
}

// -------------------------------------------------------------------------
afx_msg void TDecodeDialog::OnFinishedDoubleClick (NMHDR* pNHdr, LRESULT* pLResult)
{
	OnViewBinary();
}

// -------------------------------------------------------------------------
afx_msg BOOL TDecodeDialog::OnInitDialog ()
{
	m_pThread = gpDecodeThread;

	m_psQueue = (TUtilityQueue *) &m_sQueue;
	m_psCompleted = (TUtilityQueue *) &m_sCompleted;
	m_psCurrent = (TUtilityQueue *) &m_sCurrent;

	m_strWindowTitle.LoadString (IDS_DECODE_TITLE);

	// set the window's position
	CString strSizePos = gpGlobalOptions -> GetRegUI () -> GetFactorySizePos ();
	int dx, dy, x, y;

	// we only want the X,Y coordinates
	if (0 == DecomposeSizePos (strSizePos, dx, dy, x, y))
		SetWindowPos (NULL, x, y, NULL, NULL, SWP_NOZORDER | SWP_NOSIZE);

	TRegSwitch* pRegSwitch = gpGlobalOptions -> GetRegSwitch();
	BOOL fEnable = pRegSwitch->GetAutoSaveDecodeJobs ();
	CButton* pCbx = (CButton*) GetDlgItem (IDC_FACTORY_AUTOSAVE);
	if (pCbx)
		pCbx->SetCheck (fEnable);
	enable_disable_options(fEnable);

	SetDlgItemInt (IDC_FACTORY_ASAVEMIN, pRegSwitch->GetAutoSaveDecodeJobsInterval());

	// this sets the window's size & column sizes, etc...
	BOOL RC = TUtilityDialog::OnInitDialog ();

	return RC;
}

// -------------------------------------------------------------------------
afx_msg void TDecodeDialog::OnSelectAllCompleted()
{
	// set focus to the completed-jobs listbox
	GotoDlgCtrl (m_psCompleted);

	int iCount = m_psCompleted -> GetItemCount ();
	for (int i = 0; i < iCount; i++)
		m_psCompleted -> Select (i);

	SetButtonStates ();
}

// -------------------------------------------------------------------------
afx_msg void TDecodeDialog::OnRename ()
{
	if (m_psCompleted -> GetSelectedCount () != 1) {
		MsgResource (IDS_SELECT_JUST_ONE, this);
		return;
	}

	CString strPath;
	CString strBarePath;
	CString strNewPath;
	int i, iID;
	TRenameDialog sDlg (this);

	int iIndex = m_psCompleted -> GetSelectedIndex ();
	if (iIndex == -1)
		goto end;
	strPath = m_psCompleted -> GetItemText (iIndex, 2);

	// take out the path, leave the filename+extension
	for (i = strPath.GetLength () - 1; i >= 0; i--)
		if (strPath [i] == '\\' || strPath [i] == ':')
			break;

	// put up a dialog, and let the user modify the name
	sDlg.m_strFile =
		(i >= 0 ? strPath.Right (strPath.GetLength () - i - 1) : strPath);
	if (sDlg.DoModal () != IDOK)
		goto end;

	strBarePath = strPath.Left (i+1);
	strNewPath = strBarePath + sDlg.m_strFile;

	// Try renaming the disk file
	try {
		CFile::Rename (strPath, strNewPath);
	}
	catch (CFileException *pFE) {
		TCHAR rcErrMsg[257];
		if (pFE->GetErrorMessage(rcErrMsg, sizeof rcErrMsg))
			NewsMessageBox(this, rcErrMsg, MB_OK | MB_ICONEXCLAMATION);
		pFE->Delete ();
		MsgResource (IDS_ERR_COULD_NOT_RENAME, this);
		goto end;
	}

	iID = (int) m_psCompleted -> GetItemData (iIndex);
	m_pThread -> RenameCompletedJob (iID, strNewPath);

end:
	GotoDlgCtrl (m_psCompleted);
	SetButtonStates ();
}

// -------------------------------------------------------------------------
void TDecodeDialog::GetCompletedJobStatus (CString &str)
{
	m_psCompleted -> GetCurrentLineStatus (str);
}

// -------------------------------------------------------------------------
void TDecodeDialog::SaveHeaderSizes ()
{
	int riSizes [9];

	// get header widths
	int iWidth = 0;
	int i;
	for (i = 0; i < 2; i++)
		riSizes [iWidth++] = m_sQueue.GetColumnWidth (i);
	for (i = 0; i < 3; i++)
		riSizes [iWidth++] = m_sCurrent.GetColumnWidth (i);
	for (i = 0; i < 4; i++)
		riSizes [iWidth++] = m_sCompleted.GetColumnWidth (i);

	// save widths
	TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
	pRegUI -> SaveUtilHeaders ("Decode", riSizes, sizeof riSizes / sizeof (int));
}

// -------------------------------------------------------------------------
void TDecodeDialog::SetHeaderSizes ()
{
	// get widths
	int riSizes [9];
	TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
	if (pRegUI -> LoadUtilHeaders ("Decode", riSizes,
		sizeof riSizes / sizeof (int)))
		return;

	// set header widths
	int iWidth = 0;
	int i;
	for (i = 0; i < 2; i++)
		m_sQueue.SetColumnWidth (i, riSizes [iWidth++]);
	for (i = 0; i < 3; i++)
		m_sCurrent.SetColumnWidth (i, riSizes [iWidth++]);
	for (i = 0; i < 4; i++)
		m_sCompleted.SetColumnWidth (i, riSizes [iWidth++]);
}

// -------------------------------------------------------------------------
void TDecodeDialog::SetGlobalPointer (void)
{
	TEnterCSection sLock(&gcritDecodeDialog);
	gpsDecodeDialog = this;
}

// -------------------------------------------------------------------------
void TDecodeDialog::ClearGlobalPointer (void)
{
	TEnterCSection sLock(&gcritDecodeDialog);
	gpsDecodeDialog = 0;
}

// -------------------------------------------------------------------------
void TDecodeDialog::SetButtonStates ()
{
	TUtilityDialog::SetButtonStates ();

	int iItems = m_sCompleted.GetItemCount ();
	int iSelected = m_sCompleted.GetSelectedCount ();

	GetDlgItem (IDC_VIEW_BINARY) -> EnableWindow (iSelected);
	GetDlgItem (IDC_DELETE_BINARY_FROM_LIST) -> EnableWindow (iSelected);
	GetDlgItem (IDC_DELETE_BINARY_FROM_DISK) -> EnableWindow (iSelected);
	GetDlgItem (IDC_SELECT_ALL_COMPLETED) -> EnableWindow (iItems);
	GetDlgItem (IDC_RENAME) -> EnableWindow (1 == iSelected);
	GetDlgItem (IDC_RETRYDEC) -> EnableWindow (1 == iSelected);
}

// -------------------------------------------------------------------------
void TDecodeDialog::RememberPos (CRect & rct)
{
	WINDOWPLACEMENT wp;

	GetWindowPlacement (&wp);

	rct = wp.rcNormalPosition ;

	CString strSizePos = ComposeSizePos (0, 0, rct.left, rct.top);
	gpGlobalOptions -> GetRegUI () -> SetFactorySizePos (strSizePos);
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnRetryJob ()
{
	int iIndex;

	if (m_psCompleted->GetSelectedCount() != 1)
		return;

	iIndex = m_psCompleted->GetNextItem (-1, LVNI_SELECTED);
	if (-1 == iIndex)
		return;

	// get unique id of the job
	int iID = (int) m_psCompleted -> GetItemData (iIndex);

	// tell thread to move the job back to waiting queue
	m_pThread -> retryJob (iID);

	// remove from list control
	m_psCompleted -> DeleteLine (iIndex);

	SetButtonStates ();
	RefreshStatusBar ();
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnRetryDecodeJob()
{
	OnRetryJob ();
}

// -------------------------------------------------------------------------
// override a virtual
int TDecodeDialog::GetOptionsHeight()
{
	return HIWORD(  ::GetDialogBaseUnits() );
}

// -------------------------------------------------------------------------
// override a virtual
void TDecodeDialog::PlaceOptions(int y)
{
	int cy = GetOptionsHeight();
	int vcenter;
	CWnd * pWnd;
	CRect rct;

	// checkbox
	pWnd = GetDlgItem(IDC_FACTORY_AUTOSAVE);
	pWnd->GetWindowRect (&rct);
	ScreenToClient (&rct);
	vcenter = cy - rct.Height() / 2;
	pWnd->SetWindowPos (NULL, rct.left, y + vcenter, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// static
	pWnd = GetDlgItem(IDC_FACTORY_TXT_ASAVE1);
	pWnd->GetWindowRect (&rct);
	ScreenToClient (&rct);
	vcenter = cy - rct.Height() / 2;
	pWnd->SetWindowPos (NULL, rct.left, y + vcenter, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// ebx
	pWnd = GetDlgItem(IDC_FACTORY_ASAVEMIN);
	pWnd->GetWindowRect (&rct);
	ScreenToClient (&rct);
	vcenter = cy - rct.Height() / 2;
	pWnd->SetWindowPos (NULL, rct.left, y + vcenter, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	//static
	pWnd = GetDlgItem(IDC_FACTORY_TXT_ASAVE2);
	pWnd->GetWindowRect (&rct);
	ScreenToClient (&rct);
	vcenter = cy - rct.Height() / 2;
	pWnd->SetWindowPos (NULL, rct.left, y + vcenter, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnFactoryAutosave() 
{
	BOOL fEnabled = IsDlgButtonChecked (IDC_FACTORY_AUTOSAVE);

	enable_disable_options(fEnabled);

	TRegSwitch* pRegSwitch = gpGlobalOptions -> GetRegSwitch();
	pRegSwitch->SetAutoSaveDecodeJobs (fEnabled);      
}

// -------------------------------------------------------------------------
void TDecodeDialog::enable_disable_options(BOOL fEnable)
{
	CWnd * pWnd;
	pWnd = GetDlgItem(IDC_FACTORY_TXT_ASAVE1);
	if (pWnd)   pWnd->EnableWindow (fEnable);

	pWnd = GetDlgItem(IDC_FACTORY_ASAVEMIN);
	if (pWnd)   pWnd->EnableWindow (fEnable);

	pWnd = GetDlgItem(IDC_FACTORY_TXT_ASAVE2);
	if (pWnd)   pWnd->EnableWindow (fEnable);
}

// -------------------------------------------------------------------------
void TDecodeDialog::OnAutoSaveMinutes()
{
	BOOL bTrans;
	TRegSwitch* pRegSwitch = gpGlobalOptions -> GetRegSwitch();
	int iMinutes = GetDlgItemInt (IDC_FACTORY_ASAVEMIN, &bTrans, FALSE);

	if (bTrans)
		pRegSwitch->SetAutoSaveDecodeJobsInterval ( iMinutes );
}