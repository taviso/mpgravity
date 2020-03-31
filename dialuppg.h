/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: dialuppg.h,v $
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

// DialupPg.H : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TDialupPage dialog

#include "dialman.h"

class TConnectionDelta
{
public:
	TConnectionDelta  () {}
	~TConnectionDelta () {}
public:
	CString  m_connectionName;
	CString  m_connectionUser;
	CString  m_connectionPassword;
};

class TDialupPage : public CPropertyPage
{
	DECLARE_DYNCREATE(TDialupPage)

public:
	TDialupPage();
	~TDialupPage();

	enum { IDD = IDD_OPTIONS_DIALUP };
	CString  m_connectionName;
	BOOL     m_fForceConnection;
	BOOL     m_fUseExistingConnection;
	BOOL     m_fPromptBeforeConnecting;
	BOOL     m_fPromptBeforeDisconnecting;
	BOOL     m_fDisconnectIfWeOpened;
	BOOL     m_fForceDisconnect;

	CMap <CString, LPCTSTR, TConnectionDelta, TConnectionDelta &> m_deltaMap;
	BOOL     m_fConnectionSet;
	CString  m_connection;
	CString  m_userName;
	CString  m_password;

public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	afx_msg void OnDialupNewConnection();
	afx_msg void OnDialupConnectionProps();
	afx_msg void OnDialupForceConnection();
	afx_msg void OnDialupDisconnect();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeDialupConnectionCombo();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void EnableDisable ();
	void FillConnectionList(BOOL fUpdate = FALSE);
	void UpdateNamePassword();
	void ReflectRasChanges ();
	void UpdateConnectionDelta ();
};
