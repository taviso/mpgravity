/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutldlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:21  richard_wood
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

// tutldlg.cpp -- dialog to view the jobs in a utility-thread

#include "stdafx.h"              // precompiled header
#include "resource.h"            // ID*
#include "tutldlg.h"             // this file's prototypes
#include "tutlq.h"               // TUtilityQueue
#include "tutljob.h"             // TUtilityJob, ...
#include "tglobopt.h"            // TGlobalOptions
#include "rgui.h"                // SaveUtilDlg(), ...
#include "utilsize.h"            // Utility_
#include "server.h"              // TNewsServer
#include "nglist.h"              // TNewsGroupUseLock
#include "genutil.h"             // MsgResource()
#include "tutlthrd.h"            // TUtilityThread
#include "tutlmsg.h"             // ID_WAITING_INSERT, ...

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// IMPORTANT: keep this message map synchronized with the one in tdecdlg.cpp
BEGIN_MESSAGE_MAP(TUtilityDialog, CDialog)
		ON_BN_CLICKED (IDC_PRINT_UP, OnUtilQUp)
	ON_BN_CLICKED (IDC_PRINT_DOWN, OnUtilQDn)
	ON_BN_CLICKED (IDC_PRINT_DELETE, OnUtilQDel)
	ON_WM_DESTROY ()
	ON_MESSAGE (ID_WAITING_INSERT, OnWaitingInsert)
	ON_MESSAGE (ID_WAITING_INSERT_AT_TOP, OnWaitingInsertAtTop)
	ON_MESSAGE (ID_WAITING_DELETE, OnWaitingDelete)
	ON_MESSAGE (ID_FINISHED_INSERT, OnFinishedInsert)
	ON_MESSAGE (ID_FINISHED_DELETE, OnFinishedDelete)
	ON_MESSAGE (ID_FINISHED_CHANGED, OnFinishedChanged)
	ON_MESSAGE (ID_FINISHED_CHANGED_NO_HILIGHT, OnFinishedChangedNoHighlight)
	ON_MESSAGE (ID_CURRENT_CHANGE, OnCurrentChange)
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

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TUtilityDialog::TUtilityDialog (int iResourceID, CWnd *pParent /* = NULL */)
: CDialog(iResourceID, pParent)
{


	m_But.cx = m_But.cy = 0;
	m_iButGapY = HIWORD(::GetDialogBaseUnits()) / 4;

	m_pNewsServer = 0;
}

// -------------------------------------------------------------------------
void TUtilityDialog::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
}

// -------------------------------------------------------------------------
void TUtilityDialog::ResizeStatusBar ()
{
	int parts [3];
	CStatusBarCtrl *pBar = (CStatusBarCtrl *) GetDlgItem (IDC_STATUSBAR);
	if (pBar) {
		RECT rect;
		GetWindowRect (&rect);
		pBar -> SetWindowPos (NULL, 0, 0,
			rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOMOVE | SWP_NOZORDER);

		CRect statusRect;
		pBar -> GetWindowRect (&statusRect);
		ScreenToClient (&statusRect);
		parts [0] = statusRect.Width () - 200;
		parts [1] = statusRect.Width () - 100;
		parts [2] = -1;
		pBar -> SetParts (3, parts);
	}
}

// -------------------------------------------------------------------------
void TUtilityDialog::RefreshStatusBar ()
{
	CStatusBarCtrl *pBar = (CStatusBarCtrl *) GetDlgItem (IDC_STATUSBAR);
	CString strWaiting, strDone;
	CString str; str.LoadString (IDS_UTL_WAITING);
	strWaiting.Format (str, m_psQueue -> GetItemCount ());
	str.LoadString (IDS_UTL_DONE);
	strDone.Format (str, m_psCompleted -> GetItemCount ());
	pBar -> SetText (strWaiting, 1, 0);
	pBar -> SetText (strDone, 2, 0);
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnUtilQUp ()
{
	if (m_psQueue -> GetSelectedCount () != 1) {
		MsgResource (IDS_SELECT_JUST_ONE, this);
		return;
	}

	// Try to move the currently-selected list item up, then refresh the display
	int iIndex = m_psQueue -> GetSelectedIndex ();
	if (iIndex == -1 || !iIndex)
		return; // no item is selected, or the selected item is already on top
	int iID = (int) m_psQueue -> GetItemData (iIndex);
	m_pThread -> MoveUpWaitingJob (iID);
	SetButtonStates ();
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnUtilQDn ()
{
	if (m_psQueue -> GetSelectedCount () != 1) {
		MsgResource (IDS_SELECT_JUST_ONE, this);
		return;
	}

	// Try to move the currently-selected list item down, then refresh the display
	int iIndex = m_psQueue -> GetSelectedIndex ();
	if (iIndex == -1 || iIndex >= m_psQueue -> GetItemCount () - 1)
		return; // no item is selected, or the selected item is already at bottom
	int iID = (int) m_psQueue -> GetItemData (iIndex);
	m_pThread -> MoveDownWaitingJob (iID);
	SetButtonStates ();
}

// -------------------------------------------------------------------------
// OnUtilQDel -- Deleting a waiting job. This handles a button click
void TUtilityDialog::OnUtilQDel ()
{
	// confirm if more than one selected
	if (m_psQueue -> GetSelectedCount () > 1) {
		CString strPrompt; strPrompt.LoadString (IDS_CONFIRM_DELETE_ALL_SEL_JOBS);
		CString str; str.LoadString (IDS_PROG_NAME);
		if (MessageBox (strPrompt, str, MB_YESNO | MB_ICONQUESTION) != IDYES)
			return;
	}

	CWaitCursor wait;

	// as we go through the list, keep the current newsgroup open so that if
	// many consecutive jobs belong to the same group and it's not currently
	// open, each job won't open and close it
	long lGroup = 0;
	TNewsGroup *pNG;
	TNewsGroupUseLock *pUseLock = 0;
	BOOL fLocked = FALSE;

	// work backwards so we don't need to worry about adjusting indices
	int iCount = m_psQueue -> GetItemCount ();
	CDWordArray sSelected;

	// first gather the indices
	for (int i = iCount - 1; i >= 0; i--)
		if (m_psQueue -> IsSelected (i))
			sSelected.Add (i);

	m_psQueue -> SetRedraw (FALSE);

	for (int n = 0; n < sSelected.GetSize(); n++) {
		int i = (int) sSelected[n];

		int iID = (int) m_psQueue -> GetItemData (i);

		// NOTE: There is a possible bug here.  pJob really belongs to the
		// decode-thread's waiting-job-queue, but for the next few lines,
		// this function treats it as its own (it could possibly be deleted
		// while we're working with it)
		TUtilityJob *pJob = m_pThread -> GetWaitingJob (iID);
		if (!pJob)
			continue;
		if (lGroup != pJob -> GetGroupID ()) {
			if (lGroup)
				pNG -> Close ();
			lGroup = pJob -> GetGroupID ();
			pUseLock = new TNewsGroupUseLock (m_pNewsServer, lGroup, &fLocked, pNG);
			if (!fLocked) {
				delete pUseLock;
				pUseLock = 0;
				lGroup = 0;
			}
			else
				pNG -> Open ();
		}

		m_pThread -> RemoveWaitingJob (iID);

		// remove from list control
		m_psQueue -> DeleteLine (i);
	}

	m_psQueue -> SetRedraw ();

	if (lGroup)
		pNG -> Close ();
	if (pUseLock)
		delete pUseLock;

	GotoDlgCtrl (m_psQueue);
	void DeleteLine (int iIndex);
	SetButtonStates ();
	RefreshStatusBar ();
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnSelectAllWaiting()
{
	// set focus to the waiting-jobs listbox
	GotoDlgCtrl (m_psQueue);

	int iCount = m_psQueue -> GetItemCount ();
	for (int i = 0; i < iCount; i++)
		m_psQueue -> Select (i);

	SetButtonStates ();
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnUtilQMoveToTop()
{
	// work backwards so the selected items retain their relative positions
	int iCount = m_psQueue -> GetItemCount ();
	for (int i = iCount - 1; i >= 0; i--)
		if (m_psQueue -> IsSelected (i)) {
			int iID = (int) m_psQueue -> GetItemData (i);
			m_pThread -> MoveWaitingJobToTop (iID);

			// this lets MFC clean up temporary handles, otherwise resource
			// utilization in win95 goes way up if we're processing many entries
			AfxGetApp () -> OnIdle (1);
		}
		SetButtonStates ();
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnUtilQPause()
{
	// tell the thread to pause/resume
	m_pThread -> Pause ();

	CString strWait;
	strWait.LoadString (IDS_PLEASE_WAIT);
	CButton *pButton = (CButton *) GetDlgItem (IDC_PAUSE);
	pButton -> SetWindowText (strWait);
}

// -------------------------------------------------------------------------
BOOL TUtilityDialog::OnInitDialog ()
{
	// create a status bar...
	RECT rect;
	ZeroMemory (&rect, sizeof (rect));

	// set dialog's title
	SetWindowText (m_strWindowTitle);

	// connect our derived class to the CListCtrl fields in the .RC file
	m_psQueue -> SubclassDlgItem (IDC_PRINT_QUEUE, this);
	m_psQueue -> SetPosition (TUtilityQueue::WAIT_Q);
	m_psQueue -> MakeColumns ();
	m_psQueue -> SetThread (m_pThread);

	m_psCompleted -> SubclassDlgItem (IDC_PRINT_COMPLETED, this);
	m_psCompleted -> SetPosition (TUtilityQueue::COMPLETED);
	m_psCompleted -> MakeColumns ();
	m_psCompleted -> SetThread (m_pThread);

	m_psCurrent -> SubclassDlgItem (IDC_PRINT_CURRENT, this);
	m_psCurrent -> SetPosition (TUtilityQueue::CURRENT);
	m_psCurrent -> MakeColumns ();
	m_psCurrent -> SetThread (m_pThread);

	CDialog::OnInitDialog ();

	UpdateLists ();
	m_psQueue -> SetFocus ();

	// resize the dialog so that the buttons are in the right places to start
	// with
	int iX, iY;
	BOOL bMax, bMin;
	TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
	if (pRegUI -> LoadUtilDlg (m_strWindowTitle, iX, iY, bMax, bMin)) {
		// use default values
		iX = 600 + 2;
		iY = 444 + 2;
		bMax = bMin = FALSE;
	}
	SetWindowPos (NULL, 0, 0, iX, iY, SWP_NOZORDER | SWP_NOMOVE);
	if (bMax)
		ShowWindow (SW_SHOWMAXIMIZED);
	if (bMin)
		ShowWindow (SW_SHOWMINIMIZED);
	SetHeaderSizes ();

	// set initial state for the pause button
	OnPause (m_pThread -> IsPaused (), 0);

	ResizeStatusBar ();
	RefreshStatusBar ();

	// OK, now it's safe to set a global pointer to this dialog object
	SetGlobalPointer ();

	// lock the server
	m_pNewsServer = GetCountedActiveServer ();
	m_pNewsServer -> AddRef ();

	return FALSE; // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
// SetButtonStates -- set the `greyed' state of the buttons
void TUtilityDialog::SetButtonStates ()
{
	int iItems = m_psQueue -> GetItemCount ();
	int iSelected = m_psQueue -> GetSelectedCount ();
	int iSelectedIndex = iSelected ? m_psQueue -> GetSelectedIndex () : 0;

	GetDlgItem (IDC_PRINT_UP) -> EnableWindow (iSelected == 1 && iSelectedIndex);
	GetDlgItem (IDC_PRINT_DOWN) -> EnableWindow (iSelected == 1 &&
		iSelectedIndex != iItems - 1);
	GetDlgItem (IDC_PRINT_DELETE) -> EnableWindow (iSelected);
	GetDlgItem (IDC_SELECT_ALL_WAITING) -> EnableWindow (iItems);
	GetDlgItem (IDC_PRINT_MOVE_TO_TOP) -> EnableWindow (iSelected &&
		(iSelected > 1 || iSelectedIndex != 0 ));
	GetDlgItem (IDC_PAUSE) -> EnableWindow (iItems);
}

// -------------------------------------------------------------------------
// UpdateLists -- updates the three screen lists
void TUtilityDialog::UpdateLists ()
{
	m_psQueue -> Fill ();    // displays the current print thread waiting-job list
	m_psCurrent -> Fill ();  // displays the current job
	m_psCompleted -> Fill ();// displays the completed jobs

	SetButtonStates ();
}

// -------------------------------------------------------------------------
// OnDestroy -- called when dialog is being destroyed
// TODO: shouldn't we get rid of this function now that it does nothing??
void TUtilityDialog::OnDestroy()
{
	CDialog::OnDestroy ();
}

// -------------------------------------------------------------------------
// OnWaitingInsert -- called when a job is inserted at the bottom of the
// waiting-jobs view
LONG TUtilityDialog::OnWaitingInsert (UINT wParam, LONG lParam)
{
	return OnWaitingInsertAtPos (wParam, lParam, FALSE);
}

// -------------------------------------------------------------------------
// OnWaitingInsertAtTop -- called when a job is to be inserted at the top
// of the waiting-job view
LONG TUtilityDialog::OnWaitingInsertAtTop (UINT wParam, LONG lParam)
{
	return OnWaitingInsertAtPos (wParam, lParam, TRUE);
}

// -------------------------------------------------------------------------
// OnWaitingInsertAtPos -- called when a job is inserted into the waiting-jobs
// view
LONG TUtilityDialog::OnWaitingInsertAtPos (UINT wParam, LONG lParam, BOOL bTop)
{
	void *pJob = NULL;
	POSITION iPos = (POSITION) lParam;
	m_pThread -> GetViewMemberInfo (TUtilityThread::QUEUED_VIEW, (int) wParam,
		pJob, iPos);
	if (!pJob)
		return 0;

	// got a job description from the utility-thread. Now display it in the
	// appropriate index
	m_psQueue -> InsertJob (pJob, (int) wParam, bTop ? 0 : -1);

	// our job to destroy the `job' object
	delete (TUtilityJob *) pJob;

	SetButtonStates ();
	RefreshStatusBar ();
	return 0;
}

// -------------------------------------------------------------------------
// OnFinishedInsert -- called when a job is inserted into the finished-jobs
// view
LONG TUtilityDialog::OnFinishedInsert (UINT wParam, LONG lParam)
{
	void *pJob = NULL;
	POSITION iPos = (POSITION) lParam;
	m_pThread -> GetViewMemberInfo (TUtilityThread::FINISHED_VIEW, (int) wParam,
		pJob, iPos);
	if (!pJob)
		return 0;

	// got a job description from the utility-thread. Now display it in the
	// appropriate index
	m_psCompleted -> InsertJob (pJob, (int) wParam);

	// our job to destroy the `job' object
	delete (TUtilityJob *) pJob;

	SetButtonStates ();
	RefreshStatusBar ();
	return 0;
}

// -------------------------------------------------------------------------
// OnFinishedDelete -- called when a job in the finished-jobs view is removed
LONG TUtilityDialog::OnFinishedDelete (UINT wParam, LONG lParam)
{
	TYP_UTIL_DELINFO sDeleteInfo;

	// the HintIndex is a best guess.
	sDeleteInfo.iID = (int) lParam;
	sDeleteInfo.iLbxHintIndex = (int) wParam;

	int iIndex = m_psCompleted -> DeleteJob (&sDeleteInfo);
	GotoDlgCtrl (m_psCompleted);
	if (iIndex == -1)
		return 0;

	SetButtonStates ();
	RefreshStatusBar ();
	return 0;
}

// -------------------------------------------------------------------------
// OnCurrentChange -- called when the item in the current-job view changes
LONG TUtilityDialog::OnCurrentChange (UINT wParam, LONG lParam)
{
	m_psCurrent -> Fill ();
	SetButtonStates ();
	return 0;
}

// -------------------------------------------------------------------------
// OnWaitingMoveUp -- called when an item in the current-job view moves up
LONG TUtilityDialog::OnWaitingMoveUp (UINT wParam, LONG lParam)
{
	m_psQueue -> MoveItemUp ((int) lParam);
	GotoDlgCtrl (m_psQueue);
	SetButtonStates ();
	return 0;
}

// -------------------------------------------------------------------------
// OnWaitingMoveDown -- called when an item in the current-job view moves down
LONG TUtilityDialog::OnWaitingMoveDown (UINT wParam, LONG lParam)
{
	m_psQueue -> MoveItemDown ((int) lParam);
	GotoDlgCtrl (m_psQueue);
	SetButtonStates ();
	return 0;
}

// -------------------------------------------------------------------------
// OnWaitingDelete -- called when a job is deleted from the waiting-jobs view
LONG TUtilityDialog::OnWaitingDelete (UINT wParam, LONG lParam)
{
	TYP_UTIL_DELINFO sDeleteInfo;

	// if wParam is non zero, it is the (iLbxIndex + 1)
	sDeleteInfo.iID = (int) lParam;
	sDeleteInfo.iLbxHintIndex = wParam ? int(wParam) - 1 : -1;

	m_psQueue -> DeleteJob (&sDeleteInfo);
	SetButtonStates ();
	RefreshStatusBar ();
	return 0;
}

// -------------------------------------------------------------------------
// OnPause -- called when the thread wants to set the pause/resume state
LONG TUtilityDialog::OnPause (UINT wParam, LONG lParam)
{
	// set the button's text
	CString strPause;
	strPause.LoadString (wParam ? IDS_RESUME : IDS_PAUSE);
	CButton *pButton = (CButton *) GetDlgItem (IDC_PAUSE);
	pButton -> SetWindowText (strPause);

	// also set the status bar message
	CString strStatus;
	strStatus.LoadString (wParam ? IDS_PAUSED : IDS_RESUMED);
	CStatusBarCtrl *pBar = (CStatusBarCtrl *) GetDlgItem (IDC_STATUSBAR);
	pBar -> SetText (strStatus, 0, 0);

	return 0;
}

// -------------------------------------------------------------------------
// OnFinishedChangedNoHighlight -- called when a job in the finished-jobs view
// is changed
LONG TUtilityDialog::OnFinishedChangedNoHighlight (UINT wParam, LONG lParam)
{
	return HandleOnFinishedChanged (wParam, lParam, FALSE /* bHighlight */);
}

// -------------------------------------------------------------------------
// OnFinishedChanged -- called when a job in the finished-jobs view is changed
LONG TUtilityDialog::OnFinishedChanged (UINT wParam, LONG lParam)
{
	return HandleOnFinishedChanged (wParam, lParam, TRUE /* bHighlight */);
}

// -------------------------------------------------------------------------
// HandleOnFinishedChanged -- common code for OnFinishedChanged() and
// OnFinishedChangedNoHighlight()
LONG TUtilityDialog::HandleOnFinishedChanged (UINT wParam, LONG lParam,
											  BOOL bHighlight)
{
	void *pJob = NULL;
	POSITION iPos = (POSITION) lParam;
	m_pThread -> GetViewMemberInfo (TUtilityThread::FINISHED_VIEW, (int) wParam,
		pJob, iPos);
	if (!pJob)
		return 0;

	int iIndex = m_psCompleted -> DeleteJob (wParam);
	if (iIndex == -1) {
		delete (TUtilityJob *) pJob;
		return 0;
	}

	// got a job description from the utility-thread. Now display it in the
	// appropriate index
	m_psCompleted -> InsertJob (pJob, (int) wParam, iIndex);

	if (bHighlight) {
		// highlight the newly modified item, and de-highlight the items
		// directly before and after it
		m_psCompleted -> SetItemState (iIndex, LVIS_SELECTED, LVIS_SELECTED);
		if (m_psCompleted -> GetItemCount () > iIndex + 1)
			m_psCompleted -> SetItemState (iIndex + 1, 0, LVIS_SELECTED);
		if (iIndex)
			m_psCompleted -> SetItemState (iIndex - 1, 0, LVIS_SELECTED);
	}

	// our job to destroy the `job' object
	delete (TUtilityJob *) pJob;

	SetButtonStates ();
	return 0;
}

// -------------------------------------------------------------------------
afx_msg void TUtilityDialog::OnCancel ()
{
	ClearGlobalPointer ();
	CDialog::OnCancel ();
	OnClose ();
}

// -------------------------------------------------------------------------
afx_msg void TUtilityDialog::OnOK ()
{
	ClearGlobalPointer ();
	CDialog::OnOK ();
	OnClose ();
}

// -------------------------------------------------------------------------
afx_msg void TUtilityDialog::OnClose ()
{
	// if the user clicks on the "close-box" we may not have gone through
	//   OnOK() or OnCancel()
	ClearGlobalPointer ();

	// remember the dialog's dimensions
	SaveHeaderSizes ();

	// unzoom before looking at dlg's position  -amc  11/25/98
	BOOL bZoomed = IsZoomed ();
	BOOL bIconic = IsIconic ();

	// remember the dialog's (Normal) position
	CRect sRect;
	RememberPos (sRect);

	TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
	pRegUI -> SaveUtilDlg (m_strWindowTitle,
		(int) sRect.Width(),
		(int) sRect.Height(),
		bZoomed,
		bIconic);

	// free the server
	m_pNewsServer -> Release ();
	m_pNewsServer = 0;

	DestroyWindow ();
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	// don't let window get smaller than a certain size
	int  iTotalCY = 0;
	CWnd * pBut = 0;
	RECT clntRect;
	clntRect.top = clntRect.left = 0;
	clntRect.right = 600;               // 600 from the IDD_PRINT_QUEUE dialog
	clntRect.bottom = 444;              // 444 from the IDD_PRINT_QUEUE dialog

	// the height of this dialog box is determined by height of buttons
	if ((pBut = GetDlgItem(IDOK)) != 0)
	{
		CRect rct;  pBut->GetWindowRect(&rct);
		// actually we have 12 buttons, but we need room for statusbar
		iTotalCY = m_iButGapY + (13 * (rct.Height()+m_iButGapY));
		clntRect.bottom = max(clntRect.bottom, iTotalCY);
	}

	AdjustWindowRect (&clntRect,
		WS_MINIMIZEBOX | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU |
		WS_THICKFRAME, FALSE);
	lpMMI->ptMinTrackSize.x = clntRect.right - clntRect.left;
	lpMMI->ptMinTrackSize.y = clntRect.bottom - clntRect.top;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// -------------------------------------------------------------------------
void TUtilityDialog::PlaceButton(int id, int x, int y)
{
	// move a button without changing the size
	CWnd* pWnd = GetDlgItem( id );
	pWnd->SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnSize (UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (nType != SIZE_RESTORED && nType != SIZE_MAXIMIZED)
		return;

	ResizeMyControls (cx, cy);

	// the 'current job' control resizes its columns if a horizontal scrollbar
	// has appeared
	// could be that none of the buttons are constructed yet, hence the "if"
	if (GetDlgItem (IDC_PRINT_CURRENT))
		m_psCurrent -> CheckForHorizontalScrollbar ();
}

// -------------------------------------------------------------------------
void TUtilityDialog::ResizeMyControls (int cx, int cy)
{
#define OK_MARGIN 12
#define COMPLETED_TOP 22
#define CURRENT_TOP_MINUS_COMPLETED_BOTTOM 20
#define QUEUED_TOP_MINUS_COMPLETED_BOTTOM 100
#define BUTTON_TO_BUTTON_MARGIN 30

#define FIXED_TOT (OK_MARGIN + COMPLETED_TOP + CURRENT_TOP_MINUS_COMPLETED_BOTTOM + QUEUED_TOP_MINUS_COMPLETED_BOTTOM)
	// so that the entire client area is erased
	Invalidate ();

	// get height of middle list
#define DEFAULT_CURRENT_JOB_HEIGHT 37
	int iCurrentJobHeight;
	LOGFONT sLogFont;
	BOOL RC = SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof sLogFont,
		&sLogFont, 0 /* fWinIni */);
	if (!RC || !sLogFont.lfHeight)
		iCurrentJobHeight = DEFAULT_CURRENT_JOB_HEIGHT;
	else {
		CFont sFont;
		CWnd *pWnd = GetDlgItem (IDC_PRINT_CURRENT);
		if (!sFont.CreateFontIndirect (&sLogFont) || !pWnd)
			iCurrentJobHeight = DEFAULT_CURRENT_JOB_HEIGHT;
		else {
			CDC *pDC = m_psCurrent -> GetDC ();
			pDC -> SelectObject (&sFont);
			CSize sSize = pDC -> GetTextExtent ("A", 1);
			iCurrentJobHeight = (2*sSize.cy) + 26;  // +26 for the control's header
			m_psCurrent -> ReleaseDC (pDC);
		}
	}

	// measure button width
	if (0 == m_But.cx && ::GetDlgItem(m_hWnd,IDOK)) {
		CRect rctBut;
		GetDlgItem (IDOK) -> GetWindowRect( &rctBut );
		m_But.cx = rctBut.Width();
		m_But.cy = rctBut.Height();
	}

	ResizeStatusBar ();

	CRect sRect;
	CWnd *pWnd;
	int iQueueRight = cx - OK_MARGIN * 2 - m_But.cx;
	int iOKLeft = iQueueRight + OK_MARGIN;
	// FIXED_TOT is total height of all the constant-height stuff
	int iListHeight = (cy - FIXED_TOT) / 2;
	int iCurrentY = COMPLETED_TOP + iListHeight + CURRENT_TOP_MINUS_COMPLETED_BOTTOM;
	int iQueuedY = COMPLETED_TOP + iListHeight + QUEUED_TOP_MINUS_COMPLETED_BOTTOM;
	int iQueuedCY = iListHeight - GetOptionsHeight();
	int iLeftSide;
	int iY;

	// could be that none of the buttons are constructed yet, hence the "if"
	if ((pWnd = GetDlgItem (IDC_PRINT_COMPLETED)) == 0)
		return;
	pWnd -> GetWindowRect (&sRect);
	this -> ScreenToClient (&sRect);
	iLeftSide = sRect.left;
	pWnd -> SetWindowPos (NULL, 0, 0, iQueueRight - sRect.left,
		iListHeight, SWP_NOZORDER | SWP_NOMOVE);

	// current job
	pWnd = GetDlgItem (IDC_LABEL_2);
	pWnd -> SetWindowPos (NULL, iLeftSide, iCurrentY - 16,
		0, 0, SWP_NOZORDER | SWP_NOSIZE);

	pWnd = GetDlgItem (IDC_PRINT_CURRENT);
	pWnd -> GetWindowRect (&sRect);
	this -> ScreenToClient (&sRect);
	pWnd -> SetWindowPos (NULL, iLeftSide, iCurrentY, iQueueRight - iLeftSide,
		iCurrentJobHeight, SWP_NOZORDER);

	// waiting jobs
	pWnd = GetDlgItem (IDC_LABEL_3);
	pWnd -> SetWindowPos (NULL, iLeftSide, iQueuedY - 16,
		0, 0, SWP_NOZORDER | SWP_NOSIZE);

	pWnd = GetDlgItem (IDC_PRINT_QUEUE);
	pWnd -> SetWindowPos (NULL, iLeftSide, iQueuedY, iQueueRight - iLeftSide,
		iQueuedCY, SWP_NOZORDER);

	// virt fun
	PlaceOptions (iQueuedY + iQueuedCY);

	iY = m_iButGapY;
	PlaceButton ( IDOK, iOKLeft, iY );

	// view-binary, delete-from-disk, and delete-from-list may or may not exist
	if (0 != (pWnd = GetDlgItem (IDC_VIEW_BINARY))) {
		iY += m_But.cy + m_iButGapY;
		PlaceButton (IDC_VIEW_BINARY, iOKLeft, iY);

		iY += m_But.cy + m_iButGapY;
		PlaceButton (IDC_DELETE_BINARY_FROM_LIST, iOKLeft, iY);

		iY += m_But.cy + m_iButGapY;
		PlaceButton (IDC_DELETE_BINARY_FROM_DISK, iOKLeft, iY);

		iY += m_But.cy + m_iButGapY;
		PlaceButton (IDC_SELECT_ALL_COMPLETED, iOKLeft, iY);

		iY += m_But.cy + m_iButGapY;
		PlaceButton (IDC_RENAME, iOKLeft, iY);

		iY += m_But.cy + m_iButGapY;
		PlaceButton (IDC_RETRYDEC, iOKLeft, iY);
	}

	// Pause button is centered
	iY = (cy - m_But.cy) / 2;
	PlaceButton (IDC_PAUSE, iOKLeft, iY);

	iY = iQueuedY;
	PlaceButton (IDC_PRINT_UP, iOKLeft, iY);

	iY += m_But.cy + m_iButGapY;
	PlaceButton (IDC_PRINT_DOWN, iOKLeft, iY);

	iY += m_But.cy + m_iButGapY;
	PlaceButton (IDC_PRINT_DELETE, iOKLeft, iY);

	iY += m_But.cy + m_iButGapY;
	PlaceButton (IDC_SELECT_ALL_WAITING, iOKLeft, iY);

	iY += m_But.cy + m_iButGapY;
	PlaceButton (IDC_PRINT_MOVE_TO_TOP, iOKLeft, iY);
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnCompletedSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// display the newly selected item's status
	CString strStatus;
	GetCompletedJobStatus (strStatus);
	CStatusBarCtrl *pBar = (CStatusBarCtrl *) GetDlgItem (IDC_STATUSBAR);
	pBar -> SetText (strStatus, 0, 0);

	SetButtonStates ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TUtilityDialog::OnWaitingSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	SetButtonStates ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TUtilityDialog::ResetColumns ()
{
	m_psCurrent -> SetDefaultWidths ();
}
