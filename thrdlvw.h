/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: thrdlvw.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:52:05  richard_wood
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

// thrdlvw.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TThreadListView view

#pragma once

#include   "statvec.h"
#include   "newsview.h"
#include   "titlevw.h"
#include "newsdoc.h"
#include "declare.h"
#include "callbks.h"
#include "article.h"
#include "gotoart.h"
#include "hierlbx.h"
#include "treehctl.h"
#include "stldeclare.h"
#include "decoding.h"

// used in OnUpdate.  Saves selection during refresh current newsgroup
class TSaveSelHint : public CObject
{
public:
   TSaveSelHint() {
      m_fApply = false;              // true applies data to UI
      m_iFilterSaveArtInt = -1;
      m_iIndex = 0;
      }
   int   m_iFilterSaveArtInt;
   int   m_iIndex;
   bool  m_fApply;
};

class TThreadListView : public TTitleView
{
friend TArticleFormView;
friend THierDrawLbx;

protected:
   TThreadListView();           // protected constructor used by dynamic creation
   DECLARE_DYNCREATE(TThreadListView)

   typedef CTypedPtrArray<CPtrArray, TArticleHeader*> TPArticleArray;
   enum ESeen { kNew, kRead, kNormalImp };

// Attributes
public:
   // newsstor callback
   static void ArticleStatusCallback(const CString & group,
                                     const CString & articleNumber,
                                     TStatusUnit::EStatus kOldStatus,
                                     TStatusUnit::EStatus kNewStatus);

   void ApplyNewFont(void);

   // an override
   // void FillSmallTree (TThreadPile* pThreadPile, CStringList * pIDList);

   int  ApplyToArticles (
        int  (*pfn) (TArticleHeader *, int, TNewsGroup *, DWORD dwCookie),
        BOOL fRepaint = FALSE,
        DWORD cookie = 0,
        BOOL fBackwards = FALSE);

   // it's more efficient
   int  ApplyToSelectedItems (
        int  (*pfn) (TArticleHeader *, TNewsGroup *, DWORD dwCookie),
        BOOL fRepaint = FALSE,
        DWORD cookie = 0,
        BOOL fBackwards = FALSE);

   //  when you need the node
   int  ApplyToSelectedNodes (
        int  (*pfn) (TArtNode *, TArticleHeader *, TNewsGroup *, DWORD dwCookie),
        BOOL fRepaint        = FALSE,
        DWORD dwMagicCookie  = 0);

   void RedrawItem_Artint (int artInt);

   void ChangeStatusUtil(TStatusUnit::EStatus eStatus, BOOL fOn);
   void SetThreadStatus(TStatusUnit::EStatus eStatus, BOOL fOn);

   void DeleteArticlesNow();

   // is any thread currently selected in the view?
   BOOL IsAThreadSelected ();

   // overridden
   BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

   BOOL GetArticleStatus(int artInt, WORD& wStatus, BOOL& fHave);
   BOOL IsNextArticle(); // used by spacebar logic
   void ResetHeader();
   BOOL IsThreaded();
   int GetFirstSelectedHeader (TPersist822Header *&pHeader);
   int GetTopIndex () const;
   void SetTopIndex (int iIndex);
   int GetSelCount () const;
   int SetSel (int iIndex, BOOL bSelect = TRUE);
   int GetSelItems (int iMaxItems, int *piItems) const;
   int GetCount () const;
   void SetLbxFocus ();

   // used by filterbar
   void SortBy (TTreeHeaderCtrl::EColumn eCol, bool fAscending);

   // used from tree hdr ctrl
   void RefillTree (bool fFromUpdate);

// Operations
public:
   int  vAnythingSelected();  // override

   BOOL ArticleMatchesContext ( TFetchArticle* pFetchArticle );
   HWND GetInternalLBX(void);
   void EmptyView (void);

   int  ResetContent(TTreeHeaderCtrl::EColumn eOldCol,
                     TTreeHeaderCtrl::EColumn eNewCol,
                     BOOL fOldSortAscending, BOOL fNewSortAscending);

   int  ChangeArticle_Permanent();
   int  ChangeThread_Permanent();

   void AdjustForSmallerWidth()
      { m_header.AdjustForSmallerWidth (); }

   int  Nav_SkipNextUnread ();        // used by tnews3md.cpp
   int  Nav_ViewNextUnreadLocal ();   // used by tnews3md.cpp

   BOOL handleMouseWheel (UINT nFlags, short zDelta, CPoint pt)
      { return m_hlbx.handleMouseWheel (nFlags, zDelta, pt); }

   int  OpenSelectedArticle ();

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(TThreadListView)
   public:
   virtual void OnInitialUpdate();
   protected:
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   virtual void OnDraw(CDC* pDC);      // overridden to draw this view
   virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
   //}}AFX_VIRTUAL

// Implementation
protected:
   virtual ~TThreadListView();
#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif
   void PriorityArticle(LPCTSTR newsgroup,
                        LONG    groupID,
                        TArticleHeader * pArtHdr,
                        HTREEITEM hti);

   void MarkItemRead_and_redraw(DWORD dwCookie);
   void RedrawItem (DWORD dwCookie);
   BOOL GetHdr_for_SelectedItem(TPersist822Header* & rpHdr);
   void SelectAndShowArticle (int iArticleNumber, BOOL fLoad,
      BOOL bRemember = TRUE);
   BOOL IsArticleInThreadPane (LONG lArticle);
   void FilterOutReadThreads(void);
   void FilterOutNewThreads(void);
   void FilterOutNormalImpThreads(void);
   void RepaintArticle (int artInt);
   void vCancelArticle ();  // override
   void Followup(TArtNode*  pNode);
   void ReplyByMail(TArtNode* pNode);

   BOOL confirmKillThread();

   // override
   BOOL vMarkBrowseHeader (int artInt, TArticleText * pText);
   int  OnHLbxSelChange (TArticleHeader * pHdr);

   // Generated message map functions
protected:
   //{{AFX_MSG(TThreadListView)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnArticleSkipNextUnread();
   afx_msg void OnArticleSkipNextUnreadInThread();
   afx_msg void OnArticleSkipNextUnreadLocal();
   afx_msg void OnArticleViewNextUnread();
   afx_msg void OnArticleViewNextUnreadInThread();
   afx_msg void OnArticleViewNextUnreadLocal();
   afx_msg void OnArticleViewNextlocal();
   afx_msg void OnArticleNext();
   afx_msg void OnUpdateArticleNext(CCmdUI* pCmdUI);
   afx_msg void OnArticlePrevious();
   afx_msg void OnUpdateArticlePrevious(CCmdUI* pCmdUI);
   afx_msg void OnTreeViewDisplayNotify (NMHDR* pNHdr, LRESULT* pLResult);
   afx_msg void OnTreeViewClickNotify (NMHDR* pNHdr, LRESULT* pLResult);
   afx_msg void OnTreeViewReturnNotify (NMHDR* pNHdr, LRESULT* pLResult);
   afx_msg void OnArticleMarkread();
   afx_msg void EnableIfSelectionExists(CCmdUI* pCmdUI);
   afx_msg void OnThreadNext();
   afx_msg void OnThreadPrevious();
   afx_msg void OnThreadKillthread();
   afx_msg void OnDestroy();
   afx_msg void OnArticleRetreivetagged();
   afx_msg void OnUpdateThreadNext(CCmdUI* pCmdUI);
   afx_msg void OnUpdateThreadPrevious(CCmdUI* pCmdUI);
   afx_msg void OnUpdateKillThread (CCmdUI* pCmdUI);
   afx_msg void OnForwardSelected();
   afx_msg void OnUpdateThreadlistSearch(CCmdUI* pCmdUI);
   afx_msg void OnCmdTabaround();
   afx_msg void OnCmdTabBack();
   afx_msg LRESULT OnFinalResize(WPARAM wParam, LPARAM lParam);
   afx_msg void OnEditSelectAll();
   afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
   afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
   afx_msg void OnThreadExpandCollapse();
   afx_msg void OnUpdateThreadExpandCollapse(CCmdUI* pCmdUI);
   afx_msg void OnNewsgroupSort();
   afx_msg void OnUpdateNewsgroupSort(CCmdUI* pCmdUI);
   afx_msg void OnPriorityDecode();
   afx_msg void OnDecode(void);
   afx_msg void OnDecodeTo(void);
   afx_msg void SelectionOK(CCmdUI* pCmdUI);
   afx_msg void OnNavigateSkipdown();
   afx_msg void OnNavigateSkipdownNoExpand();
   afx_msg void OnExpandThread();
   afx_msg void OnUpdateExpandThread(CCmdUI* pCmdUI);
   afx_msg void OnCollapseThread();
   afx_msg void OnUpdateCollapseThread(CCmdUI* pCmdUI);
	afx_msg void OnDisplayRawArticle();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()

   void    GetExpandState (bool & bExpanded, bool & bOneSelected);
   afx_msg LRESULT OnTreeItemClk (WPARAM wP, LPARAM lParam);

   int  DisplayArticle(HTREEITEM hti);
   int  DisplayArticle(TPersist822Header * p822Hdr,
                       LPVOID pVoid,
                       BOOL   bRemember = TRUE,
                       BOOL   bCheckMaxLines = TRUE);

   // display the next article, possibly moving onto the next thread
   void NextArticle(BOOL fNext);
   void NextArticleIndex(int index, BOOL bRemember = TRUE);

   void UpdateSelection ( LPCTSTR msgid );
   int  NextItem(bool fExpand = true);
   int  PrevItem(void);

   HTREEITEM NextTreeItem(HTREEITEM item);
   void Util_ArticleMarkread( TArtNode* pNode );
   void Util_ArticleMarkread( TArtNode* pNode, TPersist822Header* pHdr );
   void Util_ArticleMarkIndexRead(int index);
   void Util_ThreadNext (BOOL fForward);
   void Util_KillTree ( int rootIdx );
   void UpdateThreadMoves (CCmdUI *pCmdUI, BOOL fForward);

   void UpdateSortFromTreeheader (void);

   // override virt funcs from TTitleView
   BOOL vSelectedArticlesAvailable(int& count);
   int  vSaveSelectedArticles(const CString& fileName,
                              CFile & file,
                              BOOL fRetrieve);

   int  engine_of_selection(MAP_NODES * pstlMapNodes, BOOL fClose);
   void engine_of_destruction(QUE_NODES * pstlQueNodes, TNewsGroup* pNG);
   int  discard_local(MAP_NODES* stlMapNodes, int* pgid, CString* pgrpName,
                      QUE_HDRS* stlQueHdrs );

   void resize(int cx, int cy);

   int  ChangeThreadDisplay(BOOL fThread);
   int  UseFlatList(TTreeHeaderCtrl::EColumn eNewCol, BOOL fSortAscend);
   int  SortFlatList(TThreadList& rList, TTreeHeaderCtrl::EColumn eNewCol);
   int  ChangeSortOrder(TThreadList& rList, BOOL fNewSortAscending, BOOL fKeepSel);
   int  ChangeColumn(TTreeHeaderCtrl::EColumn eNewCol, BOOL fNewSortAscending);
   BOOL update_expand_collapse();
   BOOL filter_save_selection (CObject * pObSel);

   void filter_out_threads(ESeen eSeen);
   BOOL needs_filter_out(ESeen eSeen);
   void filter_out_threads_two (ESeen eSeen, TNewsGroup* pNG, int* piSelIdx);

   void filter_out_threads_end (BOOL fLockUpdate);
   BOOL filter_out_can_remove_thrd (int idx, TNewsGroup* pNG,
      ESeen eSeen, TThread*& rpThread);
   BOOL filter_out_test_remove_thrd (TNewsGroup* pNG, ESeen eSeen, TThread* pThread);

   void set_thread_statbit(TStatusUnit::EStatus eStatus, BOOL fOn, BOOL fPile);

   void handle_zoom(BOOL fZoom, CObject * pObj);
   int  jump_engine (int  (*pfnTest)(TArticleHeader * pHdr, void * pData),
                     bool fViewArticle,
                     bool fLimitToCurPile,
                     bool fTestCurrent,
                     int * pOutArtNum = NULL );

   void update_ascend_from_filter ();

   int  JumpToEngine (EJmpQuery eQuery,
                      bool fViewArticle,
                      bool fLimitToCurPile);

   int JumpSecondPass (CNewsView * pNV,
                       EJmpQuery  eQuery,
                       int   gid,
                       bool fViewArticle,
                       bool fLimitToPile);

   int  QueryToEngine (EJmpQuery eQuery,
                       bool fLimitToCurPile,
                       int & iOutGID,
                       int & iOutArtNum);

private:
   TTreeHeaderCtrl m_header;
   THierDrawLbx  m_hlbx;

   void FillPane(bool fSort = true, bool fResetMetrics = false);
   void FillTree(bool fSort = true, bool fResetMetrics = false);

   BOOL m_fCollapsed;         // threads collapsed

   HANDLE   m_hEventFetchArticle;

   int   m_iFilterSaveArtInt;
   bool  m_fTrackZoomed;
   bool  m_fTestCurrentNavItem;

   DecodeArticleCallbackData  m_decodeCBdata;
};
