/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsview.h,v $
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
/*  Revision 1.2  2008/09/19 14:51:35  richard_wood
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

// newsview.h : interface of the cnewsview class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "newsdoc.h"
#include   "gotoart.h"
#include   "newsgrp.h"
#include "utlmacro.h"

class TNewsGroup;

// ---------------------------------------------------------------------------
class TGroupIDPair : public CObject
{
public:
   TGroupIDPair (int idIn, const CString& nameIn)
      : id(idIn), ngName(nameIn) {}

   int     id;
   CString ngName;     // NickName
};

void FreeGroupIDPairs (CPtrArray* pVec);

// ---------------------------------------------------------------------------
class CNewsView : public CListView
{
protected: // create from serialization only
   CNewsView();
   DECLARE_DYNCREATE(CNewsView)

// Attributes
public:
   enum EOpenMode { kOpenNormal, kFetchOnZero };
   enum EUseFilter { kPreferredFilter, kCurrentFilter };

   CNewsDoc* GetDocument();

// Operations
public:
   void OpenNewsgroupEx(CNewsView::EOpenMode eMode);
   int  OpenNewsgroup(CNewsView::EOpenMode eMode,
                      CNewsView::EUseFilter eFilter,
                      int groupID = -1);

   void SetCurNewsGroupID (LONG groupID);

   LONG GetCurNewsGroupID(void);

   void SetBrowseHeader (TPersist822Header * pArt);
   TPersist822Header * GetBrowseHeader(void);

   void SetBrowseText (TPersist822Text* pText);
   TPersist822Text * GetBrowseText(void);

   void ApplyNewFont(void);

   void UseFreshData (BOOL fNew) { m_fLoadFreshData = fNew; }

   // a newsgroup's headers are displayed
   BOOL IsNewsgroupDisplayed ();

   // exactly one newsgroup is selected in the newsgroup view
   bool IsExactlyOneNewsgroupSelected ();

   // a newsgroup is selected in the newsgroup view
   bool IsOneOrMoreNewsgroupSelected ();

   // what newsgroup is currently selected
   LONG GetSelectedGroupID ();
   int  SetSelectedGroupID (int iGroupID);

   // close the current newsgroup - needed by CMainFrm::OnDatePurgeAll
   void CloseCurrentNewsgroup ();

   void UnsubscribeGroups (const CPtrArray &vec);

   // for retrieving info about multiple selected groups
   int NumSelected ();
   void GetSelectedIDs (int *piIDs, int iSize);

   // returns 0 for success
   int GetNextGroup (EJmpQuery eQuery, int * pGroupID);

   // this is a hack-o-rama
   void AddOneClickGroupInterference ();

   int GetPreferredFilter (TNewsGroup * pNG);

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CNewsView)
   public:
   virtual void OnDraw(CDC* pDC);  // overridden to draw this view
   virtual void OnInitialUpdate();
   virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
   protected:
   virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
   virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
   virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
   virtual BOOL PreTranslateMessage(MSG* pMsg);
   //}}AFX_VIRTUAL

// Implementation
public:
   virtual ~CNewsView();
#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif

   // called from MDI child
   BOOL handleMouseWheel(UINT nFlags, short zDelta, CPoint pt) ;

protected:
   bool demote_newsgroup ( TNewsGroup* pNG );
   int  finish_unsubscribe (void);
   void context_menu (CPoint & pt);
   BOOL ShutdownOldNewsgroup (void);

// Generated message map functions
protected:
   //{{AFX_MSG(CNewsView)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnNgpopupOpen();
   afx_msg void OnNewsgroupUnsubscribe();
   afx_msg void OnNgpopupCatchupallarticles();
   afx_msg void OnNgpopupManualRule ();
   afx_msg void OnNewsgroupPostarticle();
   afx_msg void OnArticleFollowup();
   afx_msg void OnArticleReplybymail();
   afx_msg void OnArticleForward();
   afx_msg void OnForwardSelectedArticles ();
   afx_msg void OnUpdateArticleReplybymail(CCmdUI* pCmdUI);
   afx_msg void OnUpdateForwardSelectedArticles(CCmdUI* pCmdUI);
//   afx_msg void OnUpdateHelpSendBugReport(CCmdUI* pCmdUI);
//   afx_msg void OnUpdateHelpSendSuggestion(CCmdUI* pCmdUI);
   afx_msg void OnUpdateSendToFriend(CCmdUI* pCmdUI);
   afx_msg void OnDestroy();
   afx_msg void OnUpdateArticleFollowup(CCmdUI* pCmdUI);
   afx_msg void OnUpdateNewsgroupPostarticle(CCmdUI* pCmdUI);
   afx_msg void OnNgpopupProperties();
//   afx_msg void OnHelpSendBugReport();
//   afx_msg void OnHelpSendSuggestion();
   afx_msg void OnSendToFriend();
   afx_msg void OnTimer(UINT nIDEvent);
   afx_msg LRESULT GotoArticle (WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT ProcessMailTo (WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnDisplayArtcount(WPARAM wParam, LPARAM lParam);
   afx_msg void OnSaveToFile ();
   afx_msg void OnUpdateSaveToFile (CCmdUI* pCmdUI);
   afx_msg void OnThreadChangeToRead ();
   afx_msg void OnUpdateNewsgroupCatchup (CCmdUI* pCmdUI);
   afx_msg void OnUpdateNewsgroupUnsubscribe (CCmdUI* pCmdUI);
   afx_msg void OnUpdateNewsgroupProperties (CCmdUI* pCmdUI);
   afx_msg void OnUpdateThreadChangeToRead (CCmdUI* pCmdUI);
   afx_msg void OnUpdateDisable (CCmdUI* pCmdUI);
   afx_msg void OnCmdTabaround();
   afx_msg void OnCmdTabBack();
   afx_msg void OnGetheadersAllGroups();
   afx_msg void OnUpdateGetheadersAllGroups(CCmdUI* pCmdUI);
   afx_msg void OnGetHeadersMultiGroup();
   afx_msg void OnUpdateGetHeadersMultiGroup(CCmdUI* pCmdUI);
   afx_msg void OnGetheaderLimited();
   afx_msg void OnUpdateGetheaderLimited(CCmdUI* pCmdUI);
   afx_msg void OnDisplayAllArticleCounts();
   afx_msg LRESULT OnMode1HdrsDone(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnNewsgroupHdrsDone(WPARAM wParam, LPARAM lParam);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
   afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
   afx_msg LRESULT OnErrorFromServer(WPARAM, LPARAM);
   afx_msg LRESULT OnSelChangeOpen(WPARAM wParam, LPARAM lParam);
   afx_msg void OnUpdateDeleteSelected(CCmdUI* pCmdUI);
   afx_msg void OnUpdateNgpopupManualRule(CCmdUI* pCmdUI);
   afx_msg void OnUpdateNgpopupOpen(CCmdUI* pCmdUI);
   afx_msg void OnUpdateKeepSampled(CCmdUI* pCmdUI);
   afx_msg void OnKeepSampled();
   afx_msg void OnUpdateKeepAllSampled(CCmdUI* pCmdUI);
   afx_msg void OnKeepAllSampled();
   afx_msg void OnEditSelectAll();
   afx_msg void OnUpdatePostSelected(CCmdUI* pCmdUI);
   afx_msg void OnPostSelected();
   afx_msg void OnArticleDeleteSelected();
   afx_msg void OnVerifyLocalHeaders();
   afx_msg void OnHelpResyncStati();
   afx_msg void OnNewsgroupPinfilter();
   afx_msg void OnUpdateNewsgroupPinfilter(CCmdUI* pCmdUI);
   afx_msg void OnUpdateArticleMore(CCmdUI* pCmdUI);
   afx_msg void OnGetDisplayInfo(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
   //afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
   afx_msg void OnUpdateGettaggedForgroups(CCmdUI* pCmdUI);
   afx_msg void OnGettaggedForgroups();
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg void OnRefreshCurrentNewsgroup();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()

   void GetHeadersOneGroup();

   void newsview_reload(TNewsGroup* pNG);
   void ComposeMessage ();

   ///////
   void EmptyBrowsePointers();
   void Resize(int cx, int cy);

   void protected_open (TNewsGroup* pNG, BOOL fOpen);

   void CatchUp_Helper (int iGroupID, BOOL fAllowMoveNext);

   void GetHeadersMultiGroup (void);
   int  multisel_get (CPtrArray* pVec, int* piSel);
   int  unsubscribe_prelim (CPtrArray* pVec, BOOL bAlwaysAsk);
   void DoUnsubscribe (BOOL bAlwaysAsk);
   int  get_header_multigroup_worker (BOOL fGetAll, int iHdrsLimit);
   int  get_header_worker (int iGroupID, BOOL fCatchUp, BOOL fGetAll, int iHdrsLimit);

   int  catchup_prelim (CPtrArray* pVec);

   void handle_zoom (bool fZoom, CObject* pHint);

   // utility
   void  SetupReportColumns (int iParentWidth);
   int   SetOneSelection (int idx);
   int   AddStringWithData (const CString & groupName, LONG lGroupID,
                            int * pIdxAt = 0);
   void  RedrawByGroupID (int iGroupID);
   void  CNewsView::SaveColumnSettings();
   int   GetSelectedIndex ();

   BOOL OnNeedText (UINT, NMHDR *pNMHdr, LRESULT *);
   void HandleToolTips (CPoint &point);
   CString m_strToolTip;
   CToolTipCtrl        m_sToolTips;         // for tool tips

public:
   // called from outside this class, e.g., tmanrule.cpp
   void RefreshCurrentNewsgroup (bool fMoveSelection = true);
   void EmptyListbox ();

protected:
   BOOL    m_fLoadFreshData;         // if we are switching to a new layout
                                     //   don't reload data. just use old data

private:
   BOOL sync_caption();
   void please_update_views();
   BOOL check_server_posting_allowed();
   BOOL validate_connection ();
   void on_filter_update (CCmdUI* pCmdUI, WORD flag);
   void on_filter_command (TStatusUnit::EStatus flag);
   BOOL validate_next_newsgroup (int * pGrpId, int * pIdx);
   void SetupFont ();
   void SaveColumnSettingsTo (const CString & strLabel);

   void start_selchange_timer ();
   void stop_selchange_timer ();

private:

   CMenu               m_ngPopupMenu;
   CFont               m_font;              // User can choose custom font
   CImageList          m_imageList;

   static const  UINT  idSelchangeTimer;
   static const  UINT  idCaptionTimer;

   // current status
   LONG                m_curNewsgroupID;    // ID of current newsgroup

   TPersist822Header * m_pBrowsePaneHeader;
   TPersist822Text *   m_pBrowsePaneText;

   UINT                m_hCaptionTimer;
   UINT                m_hSelchangeTimer;

   CRITICAL_SECTION    m_dbCritSect;        // controls open of NG
   int                 iLocalOpen;          // copy pNG->Open

   TGotoArticle        m_GotoArticle;       // need our own copy
   BOOL                m_fPinFilter;        // filter is locked?

   bool                m_fTrackZoom;

   int                 m_iOneClickInterference; // mess up single click operation
};

// ---------------------------------------------------------------------------
#ifndef _DEBUG  // debug version in Newsview.cpp
inline CNewsDoc* CNewsView::GetDocument()
   { return (CNewsDoc*)m_pDocument; }
#endif
