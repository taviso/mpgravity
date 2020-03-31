/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdisppg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:52:01  richard_wood
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

// tdisppg.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TOptionsDisplayDlg dialog

class TOptionsDisplayDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(TOptionsDisplayDlg)

public:
	TOptionsDisplayDlg();
	~TOptionsDisplayDlg();

	BOOL ColumnsEquivalent(CString& strOld, CString& strNew);

	COLORREF m_oldArticleColor;
	CString  m_threadCols;  // [Y|N]From Indicators Subject Lines Date

	int m_kSortThread;
	//int m_kDateFormat;
	CString m_strDateFormat;
	int m_iShowThreadCollapsed;

	BOOL     m_fChanged;
	BOOL     m_fPureThreading;

	enum { IDD = IDD_OPTIONS_DISPLAY };
	BOOL	m_fXFaces;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnButOldArticleColor();
	virtual BOOL OnInitDialog();
	afx_msg void OnDisplayOptionsTup();
	afx_msg void OnDisplayOptionsTdn();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnOptdispThredmsgid();
	afx_msg void OnOptdispThredsubj();
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void init_check_lbx(CCheckListBox* pLbx, CString data);
	void save_check_lbx(CCheckListBox* pLbx, CString& data);

	CCheckListBox m_lbxThreadCols;
};
