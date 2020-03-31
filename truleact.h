/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: truleact.h,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:17  richard_wood
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

// truleact.h -- rule actions property page

#pragma once

class TRuleActions : public CPropertyPage
{
public:
	TRuleActions ();
	void SetRule (Rule *psRule) { m_psRule = psRule; }
	BOOL m_bDirty;

	enum { IDD = IDD_RULE_ACTIONS };
	CEdit	m_sScore;
	CButton	m_sDiscard;
	CEdit	m_sUUDecodeDir;
	CButton	m_sInTrash;
	CEdit	m_sForwardToName;
	CEdit	m_sSaveFilename;
	CEdit	m_sAlertText;
	CButton	m_sSoundTest;
	CComboBox	m_sSoundToPlay;
	CButton	m_sSoundBrowse;
	CButton	m_sSaveBrowse;
	CButton	m_sDecodeBrowse;
	CString	m_strAlertText;
	BOOL	m_bDecode;
	CString	m_strUUDecodeDir;
	CString	m_strSaveFilename;
	BOOL	m_bSaveToFile;
	BOOL	m_bShowAlert;
	BOOL	m_bPlaySound;
	BOOL	m_bForwardTo;
	CString	m_strForwardToName;
	int		m_iInTrash;
	BOOL	m_bGetBody;
	BOOL	m_bTag;
	BOOL	m_bDiscard;
	BOOL	m_bImportantEnable;
	BOOL	m_bProtectedEnable;
	BOOL	m_bReadEnable;
	int		m_iImportant;
	int		m_iProtected;
	int		m_iRead;
	BOOL	m_bIgnore;
	BOOL	m_bWatch;
	BOOL	m_bBozo;
	BOOL	m_bAddToScore;
	long	m_lScore;
	CButton m_sImportant;
	CButton m_sNormal;
	CButton m_sProtected;
	CButton m_sDeletable;
	CButton m_sRead;
	CButton m_sUnread;
	CButton m_sWatchIcon;
	CButton m_sIgnoreIcon;
	CButton m_sIconOff;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnSoundBrowse();
	afx_msg void OnSoundTest();
	afx_msg void OnSelchangeSoundToPlay();
	afx_msg void OnSaveBrowse();
	afx_msg void OnDecodeBrowse();
	afx_msg void OnChangeSoundToPlay();
	afx_msg void SetDirty();
	afx_msg void SetDirtyUpdateGrey();
	afx_msg void OnAddToScore();
	afx_msg void OnChangeScore();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void GetEditContents (int iControlID, CString& strString);
	void DisplayCurrentRule ();
	int UpdateRule ();
	void OnOK ();
	void UpdateGreyStates ();

	Rule *m_psRule;
};
