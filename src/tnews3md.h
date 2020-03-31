/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tnews3md.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:11  richard_wood
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

// tnews3md.h : header file
//

#pragma once

class TMdiLayout;
class TRegLayoutMdi;

class TReadAdvancer;

/////////////////////////////////////////////////////////////////////////////
// TNews3MDIChildWnd frame

class TNews3MDIChildWnd : public CMDIChildWnd
{
	DECLARE_DYNCREATE(TNews3MDIChildWnd)
protected:
	TNews3MDIChildWnd();           // protected constructor used by dynamic creation

	// Attributes
public:

	// Operations
public:
	friend class CMainFrame;

	void RedrawTitles(int groupID, int artInt, BOOL fStatusOnly);
	LRESULT Send_NewsView(UINT message, WPARAM wParam, LPARAM lParam);
	TMdiLayout *GetLayoutWnd () { return m_pLayout; }

	BOOL CanAdvanceNextTitle(); // called by ReadAdvancer
	int  AdvanceNextMessage();  // called by ReadAdvancer
	BOOL CanArticleMore();      // top level

	void ProcessPumpArticle ( TFetchArticle * pFetchArticle );
	int GetFocusCode () { return m_iFocusWnd; }

	BOOL handleMouseWheel (UINT nFlags, short zDelta, CPoint pt) ;

	void SetAbsolutePane (int iPane);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TNews3MDIChildWnd)
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void ActivateFrame(int nCmdShow = -1);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	// Implementation
protected:
	virtual ~TNews3MDIChildWnd();

	// Generated message map functions
	//{{AFX_MSG(TNews3MDIChildWnd)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnChildFocus(WPARAM wP, LPARAM lParam);
	afx_msg LRESULT OnChildTab (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewTreeFont (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewNGFont (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewLayout (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTreeviewRefill (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT GotoArticle (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT ProcessMailTo (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewViewTemplate (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNonBlockingCursor(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnResetTreeHeader(WPARAM wParam, LPARAM lParam);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnClose();
	afx_msg void OnArticleMore();
	afx_msg LRESULT OnUserAction(WPARAM wParam, LPARAM lParam);
	afx_msg void OnZoomKey();
	afx_msg void OnZoomEscape();
	afx_msg LRESULT OnMButtonDown (WPARAM, LPARAM);
	afx_msg void OnViewNewsgroupPane();
	afx_msg void OnViewArticlePane();
	afx_msg void OnViewThreadPane();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void FocusOnOnePane();

	BOOL build_new_layout(CWnd* pwnd, CCreateContext* pContext);
	void save_layout(void);

	TMdiLayout* m_pLayout;

	TRegLayoutMdi* m_pLayMdi;

	TReadAdvancer* m_pReadAdvancer;

	int m_iMRU_Titleview;
	int m_iFocusWnd;

	BYTE m_byCheckZoom;
};
