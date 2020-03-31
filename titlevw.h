/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: titlevw.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:09  richard_wood
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

// titlevw.h : header file

#pragma once

#include "declare.h"
#include "newsdoc.h"
#include "artchain.h"
#include "statvec.h"
#include "gotoart.h"

class TArticleFormView;
class TTitleView;
typedef struct tagTITLSaveArt
{
   const CString* pFileName;
   CFile * pFile;
   BOOL  fRetrieve;

   CNewsView* pNV;
   TTitleView* pSelf;
   int   missing;
   BOOL  fErrorShown;
} TITLSaveArt, * LPTITLSaveArt;

class TArtHdrDestroyedInfo : public CObject
{
public:
   TArtHdrDestroyedInfo(int count);
   ~TArtHdrDestroyedInfo();

   int * m_vID;
   int m_total;
};

#define TITLEVIEW_BASE   CView

/////////////////////////////////////////////////////////////////////////////
// TTitleView view
class TTitleView : public TITLEVIEW_BASE
{
protected:
   TTitleView();           // protected constructor used by dynamic creation
   DECLARE_DYNCREATE(TTitleView)

// Attributes
public:
   CNewsDoc* GetDocument(void)
      {
      ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNewsDoc)));
      return (CNewsDoc*) m_pDocument;
      }

   void SetNewsView(CNewsView* pNewsView) { m_pNewsView = pNewsView; }
   CNewsView* GetNewsView(void)           { return m_pNewsView; }

   void SetArtView(TArticleFormView* pArtView) { m_pArtView = pArtView; }
   TArticleFormView* GetArtView(void)          { return m_pArtView; }

   virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
                         AFX_CMDHANDLERINFO* pHandlerInfo);

   // save article to file.
   BOOL     SaveArticle (CFile & file,
                         TPersist822Header* pHdr, TPersist822Text* pText,
                         BOOL fAppend);
   BOOL     SaveOneArticle (CWnd* pAnchor, TPersist822Header* pHdr, TPersist822Text* pText);

   BOOL     IsArticleTagged(int artInt);

   // must be called when switching servers
   void            NotifySwitchServers ();

// Operations
public:
   void UpdateContextMenu (CMenu* pSubmenu);

   int MarkThreadStatus (TArticleHeader* pArtHdr,
                              TStatusUnit::EStatus kStatus,
                              BOOL fOn = TRUE);

   int SaveArticleDriver (LPTITLSaveArt pSaveInfo, TArticleHeader* pHdr,
                          int fSelected, TNewsGroup* pNG);

   virtual int  vAnythingSelected()                 { ASSERT(0); return 0; }

   void ProcessPumpArticle (TFetchArticle* pFetchArticle);
   void NonBlockingCursor(BOOL fOn);
   virtual HWND GetInternalLBX(void) { return 0; }
// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(TTitleView)
   protected:
   virtual void OnDraw(CDC* pDC);      // overridden to draw this view
   //}}AFX_VIRTUAL

// Implementation
protected:
   virtual ~TTitleView();
#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif

   virtual void    RedrawItem(DWORD dwCookie)   { ASSERT(0); }
   virtual void    MarkItemRead_and_redraw(DWORD dwCookie) { ASSERT(0); }
   virtual BOOL    GetHdr_for_SelectedItem(TPersist822Header* & rpHdr) {
                   ASSERT(0);
                   return FALSE;
                   }
   virtual void    SetFocusToAndShowArticle (TGotoArticle *pGoto)
                   {ASSERT(0);}
   virtual BOOL    IsArticleInThreadPane (LONG lArticle)
                   {
                   ASSERT(0);
                   return FALSE;
                   }

   virtual void    vCancelArticle () { ASSERT(0); }
   virtual BOOL    vMarkBrowseHeader (int artInt, TArticleText * pText) { ASSERT(0); return FALSE; }

   int             ArtPaneSummarize ( TArticleHeader * pHdr );

   // Generated message map functions
protected:
   //{{AFX_MSG(TTitleView)
   afx_msg void OnArticleSaveAs();
   afx_msg void OnUpdateArticleSaveAs(CCmdUI* pCmdUI);
   afx_msg void OnUpdateArticleForward(CCmdUI* pCmdUI);
   afx_msg void OnArticleCancelmsg();
   afx_msg void OnUpdateArticleCancelmsg(CCmdUI* pCmdUI);
   afx_msg void OnUpdateArticleMore(CCmdUI* pCmdUI);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnUpdateBackward(CCmdUI* pCmdUI);
   afx_msg void OnBackward();
   afx_msg void OnUpdateForward(CCmdUI* pCmdUI);
   afx_msg void OnForward();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()

protected:
   int  LoadFullArticle(TNewsGroup* pNG, TPersist822Header* pArtHdr,
      DWORD dwRedrawCookie, BOOL bRemember = TRUE);

   BOOL LoadFullArticleHelper(
      TNewsGroup*        pNG,
      TPersist822Header* pBaseHdr,
      TPersist822Text*   pBaseText);

   // puts the article text into the Popup Window
   void FillViewWindow (void);

   int priority_article(
      LPCTSTR newsgroup,
      LONG groupID,
      BOOL fSaveBody,
      TArticleHeader* pArtHdr,
      DWORD dwRedrawCookie,
      WORD  wStatus);

   void SetItemStatus (TNewsGroup* pNewsGrp,
                       int msgInt,
                       TStatusUnit::EStatus eStatus);

   virtual BOOL vSelectedArticlesAvailable(int& count){ ASSERT(0); return 0; }
   virtual int  vSaveSelectedArticles( const CString& fileName, CFile & file,
                                        BOOL fRetrieve)
                                           { ASSERT(0); return 0; }

   void    doCancelArticle (TArticleHeader* pArtHdr);
   void    Util_ArticleNGread (TArticleHeader* pArtHdr);

   void    EmptyBrowse_ClearArtPane ();
   int     GoToArticle (long lGroup, long lArticle, BOOL bRemember = FALSE);

protected:   // protected data

   CDWordArray m_hItemRedrawList;  // PriorityArticle

private:
   CNewsView *         m_pNewsView;    // use the accessor dummy
   TArticleFormView *  m_pArtView;     // only I have direct access! ha ha ha
   CArticleChain       m_chain;       // for back & forward keys
};
