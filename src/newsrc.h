/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsrc.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

// newsrc.h -- newsrc stuff

#pragma once

class TNewsGroupArray;

CString GetDefaultNewsrc ();
int ExportNewsrcUseDefaults ();
int ImportNewsrcUseDefaults ();
int ExportNewsrc (LPCTSTR pchFile, BOOL bSubscribedOnly);
int ImportNewsrc (LPCTSTR pchFile);
int EnsureUnsubscribed (const CString &strGroup, LPCTSTR pchQuestion,
						TNewsGroupArray *prsSubscribed, BOOL &bUnsubscribeYes, BOOL &bUnsubscribeNo);

// -------------------------------------------------------------------------
class TExportNewsrc : public CDialog
{
public:
	TExportNewsrc(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_NEWSRC_EXPORT };
	CButton	m_sOK;
	CString	m_strFile;
	BOOL	m_bSubscribedOnly;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFile();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

	void GreyButtons ();
};

// -------------------------------------------------------------------------
class TImportNewsrc : public CDialog
{
public:
	TImportNewsrc(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_NEWSRC_IMPORT };
	CButton	m_sOK;
	CString	m_strFile;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFile();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

	void GreyButtons ();
};

// -------------------------------------------------------------------------
class TYesNoDlg : public CDialog
{
public:
	TYesNoDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_YES_NO_ALL };
	CString	m_strMessage;

	CString m_strTitle;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnNo();
	afx_msg void OnNoAll();
	afx_msg void OnYes();
	afx_msg void OnYesAll();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
