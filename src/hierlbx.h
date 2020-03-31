/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: hierlbx.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.3  2008/09/19 14:51:27  richard_wood
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

// hierlbx.h : header file
//
#pragma once

#include "declare.h"
#include "superstl.h"

class TTreeHeaderCtrl;

typedef struct _HierDrawStruct {
	BOOL      bLines;
} HEIRDRAWSTRUCT;

typedef HEIRDRAWSTRUCT *  LPHEIRDRAWSTRUCT ;

typedef struct tagHIERBLASTTEXT
{
	CDC*             pDC;
	LPDRAWITEMSTRUCT lpDraw;
	int              depth;
	int              nWidth;
	int*             vWidth;
	WORD             status_bits;
	BOOL             fTagged;
	BOOL             fHave;

	int              textStart;
} HIERBLASTTEXT, * LPHIERBLASTTEXT;

class TArtNode;

enum ThreadActionType { WATCH_ACTION, IGNORE_ACTION };

typedef set<DWORD> STLUniqSet;

// -------------------
struct  THierUtlDrag
{
	STLUniqSet  m_stlCtrlDragSet;

	bool        m_fCapture;
	bool        m_fCtrlKey ;            // not used yet
	int         m_iStartDragIndex;
	bool        m_fDragPositive;        // not used yet

	THierUtlDrag()
	{
		m_fCapture =  m_fDragPositive = false;
		m_fCtrlKey = false;
		m_iStartDragIndex = -1;
	}
};

/////////////////////////////////////////////////////////////////////////////
// THierDrawLbx window

class THierDrawLbx : public CListCtrl
{
public:
	THierDrawLbx();
	void  AddInfo(void * pData, int uniq, BOOL fOpen);
	int   InsertInfo(int idx, LPCTSTR pText, void * pData, int uniq);

	void  NodeExpand(BOOL fExpand);

	const CString&  GetTemplate(void);

	enum { kTokFrom, kTokSubj, kTokLines, kTokDate } EHierToken;

	TThreadListView* m_parentThreadView;
	CImageList m_imgListOpen;               // open/close indicator
	CImageList m_imgListTag;
	CImageList m_imgListDecode;
	CImageList m_imgListImpShieldDoc;

	TTreeHeaderCtrl* m_pHeader;

	static int CALLBACK fnCompareDate(LPARAM lParam1, LPARAM lParam2, LPARAM dummy);
	static int CALLBACK fnCompareSubj(LPARAM lParam1, LPARAM lParam2, LPARAM dummy);

	void ApplyNewFont(BOOL fRedraw = TRUE);
	void ServerSwitch ();
	BOOL IsSelection();

	BOOL GetThreadRoot(int idx, int& rootIdx);
	BOOL GetNextThread(int idx, int& sibIdx, BOOL fForward);

	BOOL Find(int artInt, int* pIndex,
		TPersist822Header** ppHdr = 0);
	BOOL FindCollapsed(int artInt, int* pIndex, bool* pfVisible);
	BOOL FindFrom(int iStart, int iArtInt, int * pIndex,
		TPersist822Header** ppHdr = 0);

	int  OpenArticleByIndex (UINT nIndex, BOOL bCheckMaxLines = TRUE);
	void SetOneSelection(int idx);
	int  GetFirstSelection();
	void DeleteAll();
	void RepaintItem (LPVOID pVoid);      // pass in pArtNode
	void RepaintArticle (int artInt);
	void RepaintIndex (int index);

	TArtNode*  GetpNode(int idx);
	TPersist822Header* Getp_Header(int idx);

	BOOL IsOpened(int artInt);
	void OpenItem(int artInt);
	void CloseItem(int artInt);
	void CloseBranch(int idx, int* piSelIdx = 0);
	void OpenBranch(int idx);
	void ShowKids(int iCurrentSelection, int Kids);
	void ExpandAll (BOOL fExpand);

	void RemoveThread (BOOL fTreeDisplay, TThread* pThread, int i,
		int* piSelIdx);

	BOOL handleMouseWheel (UINT nFlags, short zDelta, CPoint pt);

	virtual void DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct);

public:
	virtual ~THierDrawLbx();

	// stuff to mimic a pure listbox
	void * GetItemDataPtr (int idx) { return (void*) GetItemData (idx); }
	int    GetCount ()              { return GetItemCount(); }
	int    GetSelCount ()           { return GetSelectedCount(); }
	int    GetSelItems (int max, int * pIdx) const ;
	int    GetCaretIndex()  { return GetFocusIndex(); }
	int    GetFocusIndex ();
	int    RepaintSelected ();
	int    EnsureVisibleSelection ();
	int    SetSel (int idx, BOOL fSelected = TRUE);
	BOOL   GetSel (int idx);
	int    DeleteString (int idx);

	void   SetTopIndex (int idx);
	void   SetItemDataPtr (int idx, void * pData);
	int    AddString(LPCTSTR str);
	int    getColumnCount ();

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLeftClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPost(void);
	afx_msg void OnFollowup(void);
	afx_msg void OnReplyByMail(void);
	afx_msg void OnChangeArticle_Read(void);
	afx_msg void OnChangeArticle_Unread(void);
	afx_msg void OnChangeArticle_Important(void);
	afx_msg void OnChangeArticle_NormalImp(void);
	afx_msg void OnChangeArticle_Permanent(void);
	afx_msg void OnChangeArticle_Deletable(void);
	afx_msg void OnChangeArticle_DeleteNow(void);
	afx_msg void OnChangeThread_Read(void);
	afx_msg void OnChangeThread_Unread(void);
	afx_msg void OnChangeThread_Important(void);
	afx_msg void OnChangeThread_NormalImp(void);
	afx_msg void OnChangeThread_Permanent(void);
	afx_msg void OnChangeThread_Deletable(void);
	afx_msg void OnManualDecode(void);
	afx_msg void OnDecode(void);
	afx_msg void OnDecodeTo(void);
	afx_msg void OnForwardSelectedArticles(void);
	afx_msg void OnSearch(void);
	afx_msg void OnSaveSelected (void);
	afx_msg void OnProperties(void);
	afx_msg void SelectionOK(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnArticleTagRetrieve();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnViewBinary();
	afx_msg void OnArticleBozo(void);
	afx_msg void OnAddToWatch(void);
	afx_msg void OnAddToIgnore(void);
	afx_msg void OnPrint(void);
	afx_msg void OnPrintSetup(void);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg LRESULT OnCustomMouseUp (WPARAM wP, LPARAM lP);
	afx_msg LRESULT OnCustomLButtonUp (WPARAM wP, LPARAM lP);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnSelChangeOpen (WPARAM wParam, LPARAM lParam);
	afx_msg void MeasureItem (LPMEASUREITEMSTRUCT  lpMeasure);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	HEIRDRAWSTRUCT m_DS;       // draw struct

	void OnDrawFocus ( CDC* pDC, LPDRAWITEMSTRUCT lpDraw);
	void OnDrawSelect ( CDC* pDC, LPDRAWITEMSTRUCT lpDraw);
	void DrawSelect ( CDC* pDC, LPDRAWITEMSTRUCT lpDraw);
	void BlastText (LPHIERBLASTTEXT lpBlastText);

	void FastRect(HDC hdc, int x, int y, int cx, int cy);

	void DrawIndicatorOpenClose(CDC* pDC, POINT& pt, TArtNode* pNode);
	DWORD  SetupConnectionBits(TArtNode* pNode);
	void DrawConnectionLines (CDC* pDC, LPRECT prct, int depth, DWORD dwConnection);

	void clipRect(CDC* pdc, int left, int top, int right, int bottom, BOOL fSaveOld);
	void clipUndo(CDC* pdc);

	int  CalcFontHeight ( CFont* pFont, int& h );

	void BlastToken(int& iX, int iField, LPCTSTR pTok, LPHIERBLASTTEXT pBlast, UINT uFlag);
	void BlastIndicators(int iX, int iField, LPHIERBLASTTEXT pBlast);

	void ThreadAction (ThreadActionType iAction);     // used for watch/ignore

	int selection_delete_string (int i, int* piSelIdx);

	void custDrawText (CDC* pDC, LPCTSTR pTok, int iLen,
		LPRECT pRect, UINT uFlag);
	void adjust_item_height ();

	void smartSelect (int lo, int hi, int current);

protected:
	//  stuff to read version
	int setupVersionInfo ();
	int m_iDLLVersionKey;
	int getNotifyIndex (NMHDR* pNMHDR);

	void start_selchange_timer ();
	void stop_selchange_timer ();

	void dragselect_timer (bool fTurnOn);
	void handlePanning();

	POINT  getMessagePosition();

public:
	static const UINT idSelchangeTimer;
	static const UINT idDragTimer;

protected:
	CMenu  m_popUpMenu;
	RECT   m_rctOldClip;
	CRgn   m_rgnClip;
	CFont  m_font;
	CFont  m_boldFont;
	int    m_fontCY;

	STLUniqSet  m_stlOpenSet;

	CBrush*    m_pCtlColorBrush;
	static CString m_className;

	CPoint      m_ptLastClick;
	int         m_iWindowCX;

	UINT        m_hSelchangeTimer;
	UINT        m_hDragTimer;

	THierUtlDrag m_sDrag;
};
