/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: outbxdlg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:39  richard_wood
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

// OutbxDlg.h : header file

#pragma once

#include "pobject.h"
#include "richedit.h"
#include "triched.h"
#include "outbox.h"

class TNewsServer;

#define  OUTBOX_FLAG_ARTICLE   0x1
#define  OUTBOX_FLAG_SENT      0x2
#define  OUTBOX_FLAG_CANCELMSG 0x4

// ------------------------------------------------------------------------
class TOutboxDlgStatus
{
public:
	TOutboxDlgStatus(void);
	~TOutboxDlgStatus(void);
	BOOL IsArticle()   { return m_byFlags & OUTBOX_FLAG_ARTICLE; }
	BOOL IsCancelMsg() { return m_byFlags & OUTBOX_FLAG_CANCELMSG; }
	BOOL IsSent()      { return m_byFlags & OUTBOX_FLAG_SENT; }

	int      m_articleInt;     // article number
	BYTE     m_byFlags;        // bit flags
	time_t   m_timeRead;
	CString* m_pCancelMsgID;    // required for Cancel
	CString* m_pCancelSubj;     // required for Cancel
	CString* m_pCancelNG;       // required for Cancel
};

// ------------------------------------------------------------------------
class COutboxDlg : public CDialog
{
public:
	COutboxDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_OUTBOX };
	CButton	m_sExport;
	CStatic  m_sPurgeDaysStatic;
	CButton  m_sPurge;
	CEdit m_sPurgeDays;
	CButton  m_sCancel;
	CButton  m_sDelete;
	CButton  m_sRetrySend;
	CListCtrl   m_listCtrl;
	BOOL  m_bPurge;
	UINT  m_iPurgeDays;

	HWND              m_hwndRTF;
	TRichEd           m_wndRich;

protected:
	virtual void PostNcDestroy();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void OnCancel();
	void LoadOutbox();
	void MakeColumns();
	void InsertOutboxItem (TPersist822Header *pItem);
	void RemoveOutboxItem (int artInt, int fArticle);

	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPurge();
	afx_msg void OnOutboxEdit();
	afx_msg void OnOutboxRetry();
	afx_msg void OnOutboxExport();
	afx_msg void OnOutboxDelete();
	afx_msg void OnOutboxCancelArt();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnItemChangeListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemGetDisplayInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkOutboxListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemOutboxListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnKickIdle (WPARAM, LPARAM);
	afx_msg LRESULT RefreshOutbox(WPARAM wP, LPARAM lP);
	DECLARE_MESSAGE_MAP()

	TRichEd *GetRichEdit () {
		return (TRichEd *) GetDlgItem (IDC_OUTBOX_RICHEDIT);
	}

	TOutboxDlgStatus* GetpStatus(int index);
	int  GetYPosition(int iButtonID);
	BOOL MoveOutboxButton(int iButtonID, LPRECT pClientRect);
	int GetFirstSelectedIndex ();
	int NumSelected ();
	void UpdateGrayState ();
	void UpdateRetryButton ();
	void DeleteOutboxMessage (int iIndex, BOOL bSelectAnother = TRUE,
		BOOL bSetFocus = TRUE);
	void FinishDeleting ();
	void set_senttime (int iIndex, time_t aTime);
	int  GetItemIndex (TPersist822Header *pItem);

	HANDLE      m_hOutboxIcon;    // outbox icon
	DWORD       m_hOldIcon;       // old icon
	CImageList  m_imageList;      // image list
	int         m_xMargin;        // 7 dialog units
	int         m_yMargin;        // 7 dialog units
	TOutbox   * m_pOutbox;        // outbox for this server...
	int         m_iDisplayedArticle; // article number of displayed article

	TNewsServer *m_pServer;       // news server;
	BOOL         m_fInitialLoad;  // Are we loading the outbox up?

	UINT         m_hTimer;
};

class TOutboxBar : public CDialogBar
{
public:
	TOutboxBar();
	void UpdateDisplay ();

	enum { IDD = IDD_OUTBOX_DIALOGBAR };
	CString  m_strNumErrors;
	CString  m_strNumWaiting;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

private:
	CRect m_createSize;
};

// global pointer to the one outbox-bar
extern TOutboxBar *gpsOutboxBar;
