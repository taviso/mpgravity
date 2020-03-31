/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: basfview.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:13  richard_wood
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

// basefview.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "basfview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TBaseFormView

IMPLEMENT_DYNCREATE(TBaseFormView, CFormView)

TBaseFormView::TBaseFormView()
	: CFormView(TBaseFormView::IDD)
{


   m_hAccel = 0;
}

///////////////////////////////////////////////////////////////////////////
// 12-12-95  this is used only by the TArticleFormView.  The ComposeView
//    family just depends on CFrameWnd::GetDefaultAccelerator.
void TBaseFormView::InstallAcceltable(void)
{
   int SomeAccelTableID = GetAccelTableID();
   // GetAccelTableID must be defined by derived class
   m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(),
                                 MAKEINTRESOURCE(SomeAccelTableID)
                                );
}

TBaseFormView::~TBaseFormView()
{
}

void TBaseFormView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TBaseFormView, CFormView)
	   ON_COMMAND(          ID_EDIT_COPY, OnEditCopy)
   ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
   ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_WM_SIZE()
	ON_COMMAND(          ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(          ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TBaseFormView diagnostics

#ifdef _DEBUG
void TBaseFormView::AssertValid() const
{
	CFormView::AssertValid();
}

void TBaseFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TBaseFormView message handlers

////////////////////////////////////////////////////////////////
void TBaseFormView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
   pCmdUI->Enable ( EditboxHasSelection() );
}

void TBaseFormView::OnEditCut()
{
	// TODO: Add your command handler code here
	CWnd* pRich = GetRichEdit();
   if (pRich)
      pRich->SendMessage(WM_CUT);
}

////////////////////////////////////////////////////////////////
void TBaseFormView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
   pCmdUI->Enable ( EditboxHasSelection() );
}
void TBaseFormView::OnEditCopy()
{
   TRACE0("baseFormview::Copy\n");
   GetRichEdit()->SendMessage (WM_COPY);
}

////////////////////////////////////////////////////////////////
void TBaseFormView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	// See if there is anything in the clipboard
   //    logic taken from example in MSTOOLS\samples\frmwork\editsdi
   //    12/10/95 -amc
   BOOL fEnable = FALSE;
   if (OpenClipboard())
      {
      if (IsClipboardFormatAvailable(CF_TEXT) ||
          IsClipboardFormatAvailable(CF_OEMTEXT))
         fEnable = TRUE;

      CloseClipboard();
      }
   pCmdUI->Enable ( fEnable );
}

void TBaseFormView::OnEditPaste()
{
	CWnd* pRich = GetRichEdit();
   if (pRich)
      pRich->SendMessage(WM_PASTE);
}

void TBaseFormView::OnEditSelectAll()
{
    CHARRANGE sCharRange;
    sCharRange.cpMin = 0;
    sCharRange.cpMax = -1;

    TRACE0("baseFormview::SelectAll\n");
    GetRichEdit()->SendMessage (EM_EXSETSEL, 0, (LPARAM) (CHARRANGE FAR *) &sCharRange);
}

BOOL TBaseFormView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
      {
      if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
         return TRUE;
      }
   return CFormView::PreTranslateMessage( pMsg );   
}

void TBaseFormView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	CWnd * pRich = GetRichEdit();
   if (pRich)
      pRich->MoveWindow ( 5, 5, cx - 10, cy - 10 );
}

BOOL TBaseFormView::EditboxHasSelection()
{
   // furthermore, check for a non-null Caret selection
   CHARRANGE cr;
   BOOL fOn = TRUE;
   GetRichEdit()->SendMessage(EM_EXGETSEL, 0, LPARAM(&cr));
   if (cr.cpMin == cr.cpMax)
      fOn = FALSE;
   return fOn;
}

void TBaseFormView::OnEditUndo()
{
	GetRichEdit()->SendMessage(WM_UNDO);
}

void TBaseFormView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
   pCmdUI->Enable ( GetRichEdit()->SendMessage(EM_CANUNDO) );
}

//////////////////////////////////////////////////////////////////////
// does the ROT13 substitution cipher
//
void TBaseFormView::TextRotate13(CString & strText)
{
   int len = strText.GetLength();
   LPTSTR pText = strText.GetBuffer(len);
   for (int i = 0; i < len; ++i,++pText)
      {
      TCHAR c = *pText;
      if (c >= 'a' && c <= 'm')
         c = c + 13;
      else if (c >= 'n' && c <= 'z')
         c = c - 13;
      else if (c >= 'A' && c <= 'M')
         c = c + 13;
      else if (c >= 'N' && c <= 'Z')
         c = c - 13;
      *pText = c;
      }
   strText.ReleaseBuffer(len);
}

void TBaseFormView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

   // for big messages, give the RTF more space for the user to type into
	GetRichEdit()->SendMessage(EM_EXLIMITTEXT, 0, 300000);
}
