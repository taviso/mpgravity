/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fltbar.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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

#include "transparentbitmapbutton.h"

class TViewFilterBar : public CDialogBar
{
public:
	TViewFilterBar();   // standard constructor
	~TViewFilterBar();   // standard dtor

	void Initialize ();
	void DefineFilters ();
	void ChooseFilter (CWnd * pAnchor);
	int  ForceShowAll (); // forcibly switch to 'All Articles'
	int  SelectFilter (int iFilterID); // switch to specified filter
	BOOL IsFilterLocked () { return m_fPinned; }
	void SetFilterLocked (BOOL b) { m_fPinned = b; }

	void UpdatePin (BOOL fPin);

	// toggle between showall and default filter
	void ToggleFilterShowall (int iPreferredFilter);	

	enum { IDD = IDD_FILTER_DIALOGBAR };

protected:
	void OnUpdateConfigButton(CCmdUI* pCmdUI);
	void OnUpdateDefaultButton(CCmdUI* pCmdUI);         // LINK button

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnDestroy();
	afx_msg void OnMakeDefault();
	afx_msg void OnViewfilterCfg();
	afx_msg void OnBothPushpinAction();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeViewfilterCombo();
	afx_msg LRESULT OnDelayInitTooltips (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

private:
	int selectString (const int iFilterID, bool fSetUIMemory);
	void enableTooltips (bool on);

	CButton	m_btnEdit;
	CComboBox   m_combo;
	CTransparentBitmapButton  m_Link;
	CTransparentBitmapButton  m_pin1;
	CTransparentBitmapButton  m_pin2;
	BOOL m_fPinned ;
	CToolTipCtrl *    m_pToolTip;
	UINT              m_iTimerId;
	CImageList        m_ImageList;
	bool              m_fIsShowingAll;  // toggle between showall & preferred
};

class TSelectDisplayFilterDlg : public CDialog
{
public:
	TSelectDisplayFilterDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_PICK_DISPLAYFILTER };

	int     m_iFilterID;

protected:
	void execute ();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	afx_msg void OnDblclkList1();
	DECLARE_MESSAGE_MAP()

	UINT   m_iTimerId;
	CString m_newFilterName;
	CListBox m_lbx;
};
