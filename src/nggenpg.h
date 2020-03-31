/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: nggenpg.h,v $
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
/*  Revision 1.3  2009/01/28 14:53:39  richard_wood
/*  Tidying up formatting
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

// nggenpg.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TNewGroupGeneralOptions dialog

class TNewGroupGeneralOptions : public CPropertyPage
{
	DECLARE_DYNCREATE(TNewGroupGeneralOptions)

public:
	TNewGroupGeneralOptions();
	~TNewGroupGeneralOptions();

	enum { IDD = IDD_NGOPT_GENERAL_PAGE };
	CSliderCtrl m_highArtSlider;
	CString  m_nickname;
	int      m_kStorageOption;
	int      m_iOriginalMode;
	CString  m_strGroupName;
	int      m_iServerLow;          // lowest article on NewsServer
	int      m_iServerHi;           // highest article on server
	int      m_iHighestArticleRead; // number of highest article user has read
	int      m_iHighestArticleRead0;// original value
	BOOL     m_fStorageBodies;
	LOGFONT  m_ngFont;
	COLORREF m_newsgroupColor;
	BOOL     m_fCustomNGFont;
	COLORREF m_newsgroupBackground;
	BOOL     m_fDefaultBackground;
	BOOL     m_fDiv100;
	int      m_iBarHi;
	int      m_lowWater;
	BOOL	m_bSample;
	BOOL	m_fInActive;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(void);
	virtual BOOL OnInitDialog();
	afx_msg void OnNgoptFont();
	afx_msg void OnNgoptDeffont();
	afx_msg void OnNgoptBackground();
	afx_msg void OnNgoptDefbkgrnd();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	void EnableDisable (BOOL fEnable);
	BOOL IsGobackValid ();

	void ExchangeStorageMode (CDataExchange * pDX);

	void warn_about_transition (CDataExchange *pDX);

	// warn about transitions from Mode2->Mode1
};
