/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postpfrm.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:41  richard_wood
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

// postpfrm.h -- frame window for posting/mailing dialogs

#pragma once

#include "compfrm.h"
#include "uimem.h"
#include "posttool.h"

class TAttachmentInfo;

// -------------------------------------------------------------------------
class TPostPopFrame : public TComposeFrame
{
   DECLARE_DYNCREATE(TPostPopFrame)
protected:
   TPostPopFrame();        // protected constructor used by dynamic creation
   ~TPostPopFrame() {};

public:
   virtual TUIMemory::EWinType GetWinType() { return TUIMemory::WND_POST; }

public:
   void SetSignature (const CString & shortName);
   virtual BOOL SendWarning ();
   virtual BOOL CancelWarning ();
   void SetCCIntro (BOOL bInsert);
   void SetAttachments (int iNumAttachments,
      const TAttachmentInfo *psAttachments);

   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(TPostPopFrame)
   public:
   virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
   protected:
   virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
   //}}AFX_VIRTUAL
   virtual BOOL CreateBodyView(CCreateContext* pContext);

protected:
   void CreateCustomToolbar();

   // Generated message map functions
   //{{AFX_MSG(TPostPopFrame)
   afx_msg void OnDestroy();
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnPostCancel();
   afx_msg void OnSelchangeSignature();
   afx_msg void OnUpdatePostCancel(CCmdUI* pCmdUI);
   afx_msg void OnClose();
   afx_msg void OnPostInitialUpdate();
   afx_msg void OnComposeAttach();
   afx_msg void OnChooseGroup();
   afx_msg void OnTabToEdit ();
   afx_msg void OnEditCopy();
   afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
   afx_msg void OnEditCut();
   afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
   afx_msg void OnEditPaste();
   afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
   //}}AFX_MSG
   afx_msg LRESULT OnDeferMinimize(WPARAM, LPARAM);
   DECLARE_MESSAGE_MAP()

   int EditSelectionExists ();

protected:
   CSplitterWnd   m_wndSplit1;
   CSplitterWnd   m_wndSplit2;
   CString        m_className;
   int            m_iOldSignatureIdx;
};
