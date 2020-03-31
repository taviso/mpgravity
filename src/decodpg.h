/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: decodpg.h,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:21  richard_wood
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

// decodpg.h : header file

#pragma once

// -------------------------------------------------------------------------
class TDecodePage : public CPropertyPage
{
	DECLARE_DYNCREATE(TDecodePage)

public:
	TDecodePage();

	enum { IDD = IDD_OPTIONS_DECODE };
	CComboBox	m_cmbFilename;
	CButton	m_sViewerBrowse;
	CEdit	m_sOtherViewer;
	CString  m_decodeDirectory;
	BOOL	m_fAutomaticManualDecode;
	BOOL	m_fAutomaticRuleDecode;
	CString	m_strOtherViewer;
	BOOL	m_bRestartPausedJobs;
	BOOL	m_bDeleteOnExit;
	BOOL	m_bWriteDescription;
	int		m_iViewMethod;
	UINT	m_uiPauseSpace;
	BOOL	m_fPausingSpace;

	int   m_iAfterDecode;
	int   m_iFilenameConflict;       // what action to take

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	afx_msg void OnAttachDecodeBrowse();
	afx_msg void OnOtherViewerBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnDecodepgAfterdecode();
	afx_msg void OnUseAlternate();
	afx_msg void OnUseImageGallery();
	afx_msg void OnUseRegistry();
	afx_msg void OnCbxPauselowdisk();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void transfer_postprocessing(CDataExchange* pDX);
	void enable_afterdecode(BOOL fEnable);
	void UpdateGreyState ();
};
