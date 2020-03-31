/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mlayout.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:31  richard_wood
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

#pragma once

#include "tglobdef.h"
#include "uimem.h"
#include "splitter.h"

class CNewsView;
class TThreadListView;
class TArticleFormView;
class TRegLayoutMdi;

class TMdiLayout {

friend class TNews3MDIChildWnd;  //  access ZoomTo()

public:
   TMdiLayout(int count);
   virtual ~TMdiLayout();
   BOOL m_fFreemem;
   virtual int Save(TRegLayoutMdi* pLayMdi);
   void OnSize(UINT nType, int cx, int cy);

   BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
                 AFX_CMDHANDLERINFO* pHandlerInfo, CView* pActiveView,
                 int iMRUTitleView);

   void ApplyNewNGFont();
   void ApplyNewTreeFont();
   void ApplyViewTemplate();
   int  NextPane(int iCurPane, BOOL fForward);
   virtual void SetActiveView (CFrameWnd* pFrame, int iFocusWnd);
   virtual void UserActionArrange(TGlobalDef::EUserAction eAction,
                                  CFrameWnd* pFrame, int * piFocus, int iNewFocus);

   // zooming function
   BOOL    IsZoomed ();
   virtual int Zoom (int iFocus, BOOL fZoom, BOOL fIsVisible = TRUE);
   int     ZoomEscape(CFrameWnd * pFrame, int * piFocus);

   // Info before and after, dynamic layout change
   int     RecalcPaneSizes (TRegLayoutMdi* pMdi);

protected:
   int     ZoomTo (CFrameWnd* pFrame, int * piFocus, int iCode);

protected:
   BOOL make_a_view(CSplitterWnd* splitWnd, SIZE& size,
               CCreateContext* pContext, int iWhich,
               int row, int col);
   void pre_create_view();

   void three_pane_linkup(void);
   void transfer_pane(int iWhichSplitter, int row, int col, RECT& rct);

   void fill_pane_defsize(SIZE* pSZ, int Count);

   CView * get_view_from_code(int iFocusWnd);

public:
   CNewsView*        m_pInitNewsView;     // setup during OnCreateClient
   TThreadListView * GetThreadView () { return m_pInitThreadView; }
   TArticleFormView*  GetArtFormView() { return  m_pInitArtView; }

protected:
   TSplitterWnd*   m_pwndSplit1;
   TSplitterWnd*   m_pwndSplit2;

   // setup during OnCreateClient
   TThreadListView*  m_pInitThreadView;
   TArticleFormView* m_pInitArtView;
};

class TMdiLayout3_2Top : public TMdiLayout {
public:
   TMdiLayout3_2Top(CWnd* pWnd, CCreateContext* pContext);
   ~TMdiLayout3_2Top();
   int Save(TRegLayoutMdi* pLayMdi);
};

class TMdiLayout3_2Bottom : public TMdiLayout {
public:
   TMdiLayout3_2Bottom(CWnd* pWnd, CCreateContext* pContext);
   ~TMdiLayout3_2Bottom();
   int Save(TRegLayoutMdi* pLayMdi);
};

class TMdiLayout3_2Left : public TMdiLayout {
public:
   TMdiLayout3_2Left(CWnd* pWnd, CCreateContext* pContext);
   ~TMdiLayout3_2Left();
   int Save(TRegLayoutMdi* pLayMdi);
};

class TMdiLayout3_2Right : public TMdiLayout {
public:
   TMdiLayout3_2Right(CWnd* pWnd, CCreateContext* pContext);
   ~TMdiLayout3_2Right();
   int Save(TRegLayoutMdi* pLayMdi);
};

class TMdiLayout3_3High : public TMdiLayout {
public:
   TMdiLayout3_3High(CWnd* pWnd, CCreateContext* pContext);
   ~TMdiLayout3_3High();
   int Save(TRegLayoutMdi* pLayMdi);
};

class TMdiLayout3_3Wide  : public TMdiLayout {
public:
   TMdiLayout3_3Wide(CWnd* pWnd, CCreateContext* pContext);
   ~TMdiLayout3_3Wide();
   int Save(TRegLayoutMdi* pLayMdi);
};
