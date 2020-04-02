/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsubscri.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:20  richard_wood
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

// tsubscri.h -- subscribe dialog

#pragma once

#include "ngdlg.h"               // TNewsGroupDlg
#include "odlist.h"              // COwnerDrawListView

class CNewsDoc;
class TGroupList;
class TNewsServer;

// -------------------------------------------------------------------------
class TSelectedGroups : public COwnerDrawListView
{
public:
	TSelectedGroups() {};

public:
	virtual ~TSelectedGroups() {};

protected:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};

// -------------------------------------------------------------------------
class TSubscribeDlg : public CDialog
{
	friend class TSelectedGroups;

public:
	TSubscribeDlg(CWnd* pParent = NULL, TGroupList * pNewGroups = NULL);
	CNewsDoc *pDoc;

	CStringList m_OutputGroupNames;

	enum { IDD = IDD_SUBSCRIBE };
	CStatic	m_sGobackDesc2;
	CStatic	m_sGobackDesc1;
	CStatic	m_sGoBackLow;
	CStatic	m_sGoBackHigh;
	CSliderCtrl	m_sGoBackSlider;
	CButton	m_sReset;
	CButton	m_sDone;
	CStatic	m_sStoremodeDesc;
	CStatic	m_sChosenDesc;
	CStatic	m_sSearchDesc;
	CEdit	m_sFilter;
	CEdit	m_sGoBack;
	CComboBox	m_sStorageMode;
	CButton	m_sSample;
	CButton	m_sUnsubscribe;
	CButton	m_sSubscribe;
	CString	m_searchString;
	CString	m_strGoBack;
	BOOL	m_bSample;
	int		m_iStorageMode;
	TSelectedGroups	m_sSelected;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnDblclkGroupList();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDone();
	afx_msg void OnButtonSubscribe();
	afx_msg int  OnVKeyToItem(UINT key, CListBox* pListBox, UINT nIndex);
	afx_msg void OnDestroy();
	afx_msg void OnSubscribeSearch();
	afx_msg void OnChangeSubscribeFilterSearch();
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
	afx_msg void OnSetfocusSubscribeSearchstring();
	afx_msg void OnSubscribeResetsearch();
	afx_msg void OnButtonRemove();
	afx_msg void OnDblclkSelected(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeGoBack();
	afx_msg void OnSample();
	afx_msg void OnSelchangeStorageMode();
	afx_msg void OnItemchangedSelected(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnGroupListSelchange ();

	CListBox* GetGroupList ()
		{ return (CListBox*) GetDlgItem (IDC_GROUP_LIST); }
	void UpdateGroupRatio ();
	void UpdateGrayState ();
	BOOL OnNavigate ();
	void FillList ();
	void UpdateEditValues ();
	void FillSelColumn (int iColumn, LPCTSTR pchStr);
	void MoveControls ();
	void UpdateSliderValue ();

private:
	TGroupList     *m_pAllGroups;    // container for all of the groups (A)
	TGroupList     *m_pNewGroups;    // container for new groups (B)
	TGroupList     *m_pCurGroups;    // points to A or B
	CString         m_currGroup;     // the "root that we're looking at now
	TNewsGroupDlg   m_lbxMgr;        // virtual listbox mgr object
	int             m_totalGroups;   // total groups before any searching
	BOOL            m_fShowAllGroups;// False means incremental

	TNewsServer    *m_pNewsServer;   // setup during OnInitDialog
};
