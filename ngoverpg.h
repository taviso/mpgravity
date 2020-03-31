/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngoverpg.h,v $
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
/*  Revision 1.2  2008/09/19 14:51:36  richard_wood
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

// ngoverpg.h -- per-group option overrides

#pragma once

// ngoverpg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupOverrideOptions dialog

class TNewsGroupOverrideOptions : public CPropertyPage
{
	DECLARE_DYNCREATE(TNewsGroupOverrideOptions)

public:
	TNewsGroupOverrideOptions();
	~TNewsGroupOverrideOptions();

	enum { IDD = IDD_NGOPT_OVERRIDES };
	CEdit	m_sLimitHeaders;
	CEdit	m_sFullName;
	CEdit	m_sEmail;
	CEdit	m_sCustomHeadersCtrl;
	CEdit	m_sDecodeDir;
	CString	m_strDecodeDir;
	CString	m_strEmail;
	CString	m_strFullName;
	BOOL	m_bOverrideCustomHeaders;
	BOOL	m_bOverrideDecodeDir;
	BOOL	m_bOverrideEmail;
	BOOL	m_bOverrideFullName;
	UINT	m_iHeaderLimit;
	BOOL	m_fOverrideLimitHeaders;

	CStringList m_sCustomHeaders;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnOverrideCustomHeaders();
	afx_msg void OnOverrideDecodeDir();
	afx_msg void OnOverrideEmail();
	afx_msg void OnOverrideFullName();
	virtual BOOL OnInitDialog();
	afx_msg void OnOverrideLimitHeaders();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void GreyControls ();
};
