/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thotlink.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:04  richard_wood
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

// thotlink.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// THotlinkPage dialog

#include "turldde.h"

class THotlinkPage : public CPropertyPage
{
	DECLARE_DYNCREATE(THotlinkPage)

public:
	THotlinkPage();
	~THotlinkPage();

	enum { IDD = IDD_OPTIONS_HOTLINK };
	BOOL     m_fUnderline;
	int      m_fMailUseReg;
	BOOL     m_fWebUseReg;
	BOOL     m_fTelnetUseReg;
	BOOL     m_fGopherUseReg;
	BOOL     m_fFtpUseReg;
	COLORREF m_colorRef;
	TUrlDde  m_webHelper;
	TUrlDde  m_ftpHelper;
	TUrlDde  m_gopherHelper;
	TUrlDde  m_telnetHelper;

	CString  m_webPattern;
	CString  m_ftpPattern;
	CString  m_gopherPattern;
	CString  m_telnetPattern;
	CString  m_mailToPattern;

	LONG     m_fHighlightMail;
	LONG     m_fHighlightWeb;
	LONG     m_fHighlightFtp;
	LONG     m_fHighlightGopher;
	LONG     m_fHighlightTelnet;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnOptionsHotlinkColor();
	afx_msg void OnOptionsHotlinkFtpCustom();
	afx_msg void OnOptionsHotlinkFtpUsereg();
	afx_msg void OnOptionsHotlinkGopherCustom();
	afx_msg void OnOptionsHotlinkGopherUsereg();
	afx_msg void OnOptionsHotlinkMailCustom();
	afx_msg void OnOptionsHotlinkMailUsereg1();
	afx_msg void OnOptionsHotlinkTelnetCustom();
	afx_msg void OnOptionsHotlinkTelnetUsereg();
	afx_msg void OnOptionsHotlinkWebCustom();
	afx_msg void OnOptionsHotlinkWebUsereg();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnOptionsHotlinkUnderline();
	afx_msg void OnOptionsHotlinkRecognize();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()
	void DoCustom(const CString &caption, TUrlDde *pDDE);
};
