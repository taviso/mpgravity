/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: triched.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/01/11 22:28:40  richard_wood
/*  Fixed the XFace corruption on scroll bug for keyboard Down, Up, Page Down, Page Up, mouse wheel scroll, space bar scroll.
/*
/*  Revision 1.4  2008/09/19 14:52:17  richard_wood
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

// triched.h : header file
//
// Classes Defined:
//    TFormRicheEdit
//       TRichEd
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// TRichEd window
#pragma once

#include "friched.h"
#include "TXFaceWnd.h"

class TArticleFormView;

class TRichEd : public TFormRichEdit
{
public:
	TRichEd();
	virtual ~TRichEd();

	void PostSubclass ();

	int ArticleRepairURL();

	// Movement
	BOOL handleMouseWheel (UINT nFlags, short zDelta, CPoint pt) ;
	int  RequestPageDown();
	BOOL CanRequestPageDown();

	// Text
	void GetWordUnderCaret (CString &strWord);
	int GetWordAtCharPos (int iCharPos, char *pchLine, int iSize, int &iLeft,
		int &iLine, BOOL & fAngleBrackets);
	int GetWordAtPoint (CPoint point, char *pchLine, int iSize, int &iLeft,
		int &iLine, BOOL & fAngleBrackets);
	void SelectedText (CString &str);

	// XFace
	void SetXFace (LPCTSTR line);
	void HideXFace ();
	void SetupXFaceWindow();

	CDocument *m_pDoc;
	TArticleFormView* m_pParentFormView;

protected:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

public:
	afx_msg void OnSearchFind();
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMsgFilterReflect (NMHDR* pNotifyStruct, LRESULT* plResult);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnRtfYield(WPARAM, LPARAM);
	afx_msg LRESULT OnXFaceOn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchFindCB(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	void DoPageDown ();
	bool XFaceShowing ();
	int ClickHandler (CPoint * ptButtonDown, CPoint * ptButtonUp);

private:
	CMenu                m_popUpMenu;         // popup context menu
	CFindReplaceDialog * m_pFindDialog;       // find dialog
	LRESULT              m_searchPos;         // current search position
	CHARRANGE            m_selected;          // current range selected
	HCURSOR              m_hArrowCursor;      // cursor shape over URLs
	TXFaceWnd            m_wndXFace;
	CPoint               m_ptLastLeftClick;   // coordinates of last click
	bool				 m_bTurnXFaceOn;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
