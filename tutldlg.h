/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutldlg.h,v $
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

// tutldlg.h -- dialog to view the jobs in a utility-thread

#pragma once

class TNewsServer;
class TUtilityQueue;
class TUtilityThread;

// -------------------------------------------------------------------------
class TUtilityDialog : public CDialog
{
public:
	TUtilityDialog (int iResourceID, CWnd *pParent = NULL);
	void ResetColumns ();

protected:
	virtual void DoDataExchange (CDataExchange* pDX);   // DDX/DDV support
	afx_msg void OnUtilQUp();
	afx_msg void OnUtilQDn();
	afx_msg void OnUtilQDel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LONG OnWaitingInsert (UINT wParam, LONG lParam);
	afx_msg LONG OnWaitingInsertAtTop (UINT wParam, LONG lParam);
	afx_msg LONG OnWaitingDelete (UINT wParam, LONG lParam);
	afx_msg LONG OnFinishedInsert (UINT wParam, LONG lParam);
	afx_msg LONG OnFinishedDelete (UINT wParam, LONG lParam);
	afx_msg LONG OnFinishedChanged (UINT wParam, LONG lParam);
	afx_msg LONG OnFinishedChangedNoHighlight (UINT wParam, LONG lParam);
	afx_msg LONG OnCurrentChange (UINT wParam, LONG lParam);
	afx_msg LONG OnWaitingMoveUp (UINT wParam, LONG lParam);
	afx_msg LONG OnWaitingMoveDown (UINT wParam, LONG lParam);
	afx_msg LONG OnPause (UINT wParam, LONG lParam);
	afx_msg void OnOK ();
	afx_msg void OnCancel ();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnCompletedSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg void OnSelectAllWaiting();
	afx_msg void OnUtilQMoveToTop();
	afx_msg void OnUtilQPause();
	afx_msg void OnWaitingSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	void UpdateLists ();
	virtual void SetButtonStates ();
	virtual void SetGlobalPointer () = 0;
	virtual void ClearGlobalPointer () = 0;
	virtual void GetCompletedJobStatus (CString &str) = 0;
	virtual void SaveHeaderSizes () = 0;
	virtual void SetHeaderSizes () = 0;
	virtual void RememberPos (CRect & rct) = 0;
	virtual int  GetOptionsHeight() = 0;
	virtual void PlaceOptions(int y) = 0;
	void ResizeStatusBar ();
	void RefreshStatusBar ();
	LONG HandleOnFinishedChanged (UINT wParam, LONG lParam, BOOL bHighlight);

	TUtilityQueue *m_psQueue;
	TUtilityQueue *m_psCompleted;
	TUtilityQueue *m_psCurrent;
	TUtilityThread *m_pThread;       // thread whose jobs we're viewing
	CString m_strWindowTitle;        // "Print Jobs", "Decode Jobs", ...
	CStatusBar m_sStatusBar;         // shows status of currently selected job

private:
	void ResizeMyControls (int cx, int cy);
	LONG OnWaitingInsertAtPos (UINT wParam, LONG lParam, BOOL bTop);
	void PlaceButton(int id, int x, int y);

	CSize m_But;                     // measure dlg button
	int   m_iButGapY;                // quarter of dlgbaseunit

	TNewsServer  *m_pNewsServer;     // our own ptr
};
