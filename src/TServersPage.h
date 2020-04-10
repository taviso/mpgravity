/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TServersPage.h,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2010/04/20 21:04:55  richard_wood
/*  Updated splash screen component so it works properly.
/*  Fixed crash from new splash screen.
/*  Updated setver setup dialogs, changed into a Wizard with a lot more verification and "user helpfulness".
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:08  richard_wood
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

// TServersPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TServersPage dialog

class TServersPage : public CPropertyPage
{
	DECLARE_DYNCREATE(TServersPage)

public:
	TServersPage();
	~TServersPage();

	enum { IDD = IDD_OPTIONS_SERVERS };
	CString  m_newsServer;
	CString  m_strNNTPAddress;
	int      m_iLogonStyle_NTP;
	CString  m_authName_NTP;
	CString  m_authPass_NTP;

	CString  m_smtpServer;
	int      m_iLogonStyle_STP;
	CString  m_authName_STP;
	CString  m_authPass_STP;

	int      m_iPortNNTP;
	int      m_iPortSMTP;

	BOOL     m_fConnectSecurely;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void TServersPage::EnableAccountName (BOOL fEnable);
	void TServersPage::EnableSmtpAccountName (BOOL fEnable);
	afx_msg void OnRbtSmtpNopwd();
	afx_msg void OnRbtSmtppwd();
	afx_msg void OnOptserverRbtNopwd();
	afx_msg void OnOptserverRbtLogonw();
	afx_msg void OnOptserverRbtSpa();
	afx_msg void OnServerTls();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnEnChangeUpdateButtons();
	afx_msg void OnEnChangeNntpserverAddress();
	afx_msg void OnEnChangeUpdateNNTPAuth();
	virtual BOOL OnSetActive();
	afx_msg void OnBnClickedRbtSmtpUseNntpAuth();
};
