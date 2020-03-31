/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artview.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.7  2009/01/28 22:45:49  richard_wood
/*  Added border round article pane in main window.
/*  Cleaned up some memory leaks.
/*
/*  Revision 1.6  2009/01/11 22:28:40  richard_wood
/*  Fixed the XFace corruption on scroll bug for keyboard Down, Up, Page Down, Page Up, mouse wheel scroll, space bar scroll.
/*
/*  Revision 1.5  2008/09/19 14:51:12  richard_wood
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

// artview.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TArticleFormView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "richedit.h"         // from chicago SDK

#include "newsdoc.h"
#include "triched.h"
#include "declare.h"

#include "basfview.h"         // base class: TBaseFormView

class TTitleView;
class TArticleItemView;
class TNewsGroup;
class GravCharset;

class TArticleFormView : public TBaseFormView
{
public:
	TArticleFormView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TArticleFormView)

public:
	/*enum { IDD = IDD_FORM_ARTVIEW };*/
	// NOTE: the ClassWizard will add data members here

	CNewsDoc* GetDocument(void)
	{
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNewsDoc)));
		return (CNewsDoc*) m_pDocument;
	}

	virtual int GetAccelTableID(void)
	{
		return IDR_ARTVIEW;
	}

public:
	void ApplyViewTemplate(void);
	void OnUpdateHelper();

	void Clear(void);   // clear our contents
	void SetTitleView (TTitleView* sib)
	{
		m_pTitleView = sib;
		TArticleFormView::ThreadListView (TRUE);
	}
	TTitleView* GetTitleView(void) { return m_pTitleView; }

	int RequestPageDown(void);
	BOOL CanRequestPageDown(void);

	BOOL IsMessagePresent();

	void NonBlockingCursor(BOOL fOn);

	void SelectedText (CString &str);   // returns a copy of selected text

	BOOL handleMouseWheel (UINT nFlags, short zDelta, CPoint pt);

	void RichEditEnterKey ();

public:
	virtual void OnInitialUpdate();
	///virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

public:
	static void ThreadListView(BOOL fActive) { m_fThreadListViewAlive = fActive; }
	static BOOL IsThreadViewActive(void)       { return m_fThreadListViewAlive; }

protected:
	virtual ~TArticleFormView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFocusNotify();
	afx_msg LRESULT OnChildTab(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNextDlgCtl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChildEsc (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChildSpace(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdatePopup(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnArticleSaveAs();
	afx_msg void OnUpdateArticleSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnRot13 ();
	afx_msg void OnUpdateArticleRot13(CCmdUI* pCmdUI);
	afx_msg void OnSearchFind();
	afx_msg void OnUpdateSearchFind(CCmdUI* pCmdUI);
	afx_msg void OnImpWord ();
	afx_msg void OnBozo ();
	afx_msg void OnPostMessage ();
	afx_msg void OnMailReply ();
	afx_msg void OnPostFollowup ();
	afx_msg void OnForwardArticleByMail ();
	afx_msg void OnDecode ();
	afx_msg void OnManualDecode ();
	afx_msg void OnProperties ();
	afx_msg void OnUpdateForwardbymail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateArticleDecode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateArticleMailreply(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePostFollowup(CCmdUI* pCmdUI);
	afx_msg void EnableIfHeaderPresent (CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateArticleMore(CCmdUI* pCmdUI);
	afx_msg void OnToggleFullHeader ();
	afx_msg void OnUpdateToggleFullHeader(CCmdUI* pCmdUI);
	afx_msg void OnCmdTabaround();
	afx_msg void OnCmdTabBack();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void EnableIfMessagePresent(CCmdUI* pCmdUI);
	afx_msg void OnViewBinary();
	afx_msg void OnDeleteSelected(void);
	afx_msg void OnUpdateDeleteSelected(CCmdUI* pCmdUI);
	afx_msg void OnKillThread(void);
	afx_msg void OnUpdateKillThread(CCmdUI* pCmdUI);
	afx_msg void OnArticleToggleQuotedtext();
	afx_msg void OnUpdateArticleToggleQuotedtext(CCmdUI* pCmdUI);
	afx_msg LRESULT OnMButtonDown (WPARAM wParam, LPARAM lParam);
	afx_msg void OnArticleShowSource();
	afx_msg void OnUpdateArticleShowSource(CCmdUI* pCmdUI);
	afx_msg void OnArticleRepairURL();
	afx_msg void OnUpdateArticleRepairURL(CCmdUI* pCmdUI);
	afx_msg void OnArticleCharcoding();
	DECLARE_MESSAGE_MAP()

protected:
	void GetCurrentMessagePtrs (TPersist822Header*& pHdr, TPersist822Text*& pText);
	void UtilSetArticleStatus(int iStatus, BOOL fOn);
	// BOOL UtilGetArticleStatus (WORD& wStatus);
	LONG GetCurrentNGID ();

protected:
	static BOOL m_fThreadListViewAlive;

	bool display_xface ( TPersist822Text* pText ) ;

private:
	HWND   m_hwndRTF;
	TRichEd m_wndRich;

	// allows us to get ptr to CNewsView
	TTitleView* m_pTitleView;

	BOOL     m_fShowingFullHeader;
	bool     m_fShowingQuotedText;
	bool     m_fShowingMsgSource;
	CBrush*  m_pCtlBrush;
	DWORD    m_dwBrush;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
