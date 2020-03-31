/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdlgtmpv.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:01  richard_wood
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

// tdlgtmpv.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TDlgTemplateView dialog

#include "tmpllbx.h"
#include "custview.h"

class TDlgTemplateView : public CPropertyPage
{
	DECLARE_DYNCREATE(TDlgTemplateView)

	// Construction
public:
	TDlgTemplateView();
	~TDlgTemplateView();
	void InitializeCustomView (const TCustomArticleView& src);
	const TCustomArticleView& GetCustomView() { return m_CustomView; }

	enum { IDD = IDD_VIEW_TEMPLATE };
	BOOL  m_fUsePopup;
	int      m_kSortThread;
	BOOL  m_fGetFullHeader;
	BOOL	m_defBackground;              // use system background color in article pane?
	BOOL	m_fSkipHTMLPart;
	int		m_iQuotedTextMax;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	afx_msg void OnDblclkVtAvail();
	afx_msg void OnDblclkVtChosen();
	afx_msg void OnVtAddbut();
	afx_msg void OnVtDelbut();
	afx_msg void OnVtMovedown();
	afx_msg void OnVtMoveup();
	virtual BOOL OnInitDialog();
	afx_msg void OnVtFont();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnViewDlfullhdr();
	afx_msg void OnVtBackgroundColor();
	afx_msg void OnCancel();
	afx_msg void OnVtDefback();
	afx_msg void OnVtQuotedColor();
	afx_msg void OnVtSignatureColour();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

protected:
	CBrush   m_greyBrush;

private:
	CListBox * GetAvail()   { return (CListBox*) GetDlgItem (IDC_VT_AVAIL);  }
	TTemplateLbx* GetChosen()  { return (TTemplateLbx*) GetDlgItem (IDC_VT_CHOSEN); }

public:
	TCustomArticleView  m_CustomView;
	LOGFONT             m_treeFont;
	BOOL                m_fChanged;
	COLORREF            m_background;
	COLORREF            m_originalBackground;
	COLORREF            m_clrQuoted;
	COLORREF            m_originalQuoted;

	COLORREF			m_clrSignature;
	COLORREF			m_originalSignature;

	LOGFONT             m_quotedFont;
	LOGFONT             m_origQuotedFont;

	LOGFONT				m_signatureFont;
	LOGFONT				m_origSignatureFont;

	int                 m_origQuotedPtSize;
	int                 m_origSignaturePtSize;
	TTemplateLbx m_TemplateLbx;      // self-draw listbox.  hook this in
};
