/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: evtdlg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/29 17:22:34  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.2  2008/09/19 14:51:23  richard_wood
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

// evtdlg.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TEventViewerDlg dialog

class TEventEntry;

class TEventViewerDlg : public CDialog
{
public:
	TEventViewerDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_EVENT_LOG };
	BOOL	m_iAutoRefresh;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT On_vlbRange   (WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbPrev    (WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbFindPos (WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbFindItem(WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbFindString(WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbSelectString(WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbNext    (WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbFirst   (WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbLast    (WPARAM wP, LPARAM lParam);
	afx_msg LRESULT On_vlbGetText (WPARAM wP, LPARAM lParam);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnHdrctrlEndTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVListDblClk(void);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEvtlogDetails();
	DECLARE_MESSAGE_MAP()

protected:
	void setup_font();
	void setup_headerctrl();
	void get_header_widths(int* pWidths);

	CHeaderCtrl    m_hdr;
	int  m_vHdrWidths[4];

	HWND     m_hwndVList;

	CFont m_font;
	BYTE  m_fFontInited;
	BYTE  m_fWidthsInited;
	UINT   m_timerID;

private:
	void draw_select(LPDRAWITEMSTRUCT lpdis);
	void draw_focus(LPDRAWITEMSTRUCT lpdis);

	void blast_text(LPDRAWITEMSTRUCT lpdis, const TEventEntry* pEntry);
	void get_prio_string(const TEventEntry* pEntry, CString& str);
};
