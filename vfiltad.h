/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vfiltad.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:52:25  richard_wood
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

#pragma once

// vfiltad.h : header file
//

#include "statunit.h"

/////////////////////////////////////////////////////////////////////////////
// TViewFilterAddDlg dialog

class TViewFilterAddDlg : public CDialog
{
public:
	TViewFilterAddDlg(CListCtrl* pList, CWnd* pParent = NULL);   // standard constructor
	~TViewFilterAddDlg ();

	BOOL m_fEdit ;

	enum { IDD = IDD_VIEWFILTER_ADD };
	CString	m_strName;
	BOOL	m_fShowEntireThread;
	BOOL	m_fCompleteBinaries;
	BOOL	m_fSkipThreads;
	WORD    m_wRequired;
	WORD    m_wFilter;
	CStringArray m_rstrCondition;
	DWORD   m_dwSortCode;

	DWORD GetDWordData () { return (DWORD)MAKELONG(m_wFilter, m_wRequired); }
	void  SetDWordData (DWORD dw) {
		m_wRequired = HIWORD(dw);
		m_wFilter = LOWORD(dw);
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnVfiltImp();
	afx_msg void OnVfiltLocal();
	afx_msg void OnVfiltNew();
	afx_msg void OnVfiltTag();
	afx_msg void OnVfiltDecoded();
	afx_msg void OnVfiltProtected();
	virtual void OnOK();
	afx_msg void OnAdvanced();
	afx_msg void OnClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCmbFltsort();
	afx_msg void OnRadioFltsortAsend();
	afx_msg void OnRadioFltsortDsend();
	DECLARE_MESSAGE_MAP()

	CListCtrl * m_pList;
	CRichEditCtrl m_sDescription;
	CComboBox   m_cmbSort;

private:
	void config_bits(BOOL fSave, TStatusUnit::EStatusFilter eStatus, int iCtrlID);
	void config_all_bits (BOOL fSave);
	void config_sort_combo (BOOL fSave);
	void GetDescription (CString &str);
	void update_banner();
	int  GetState(TStatusUnit::EStatusFilter eStatus);
	int  CountNameMatches (const CString & name);
};
