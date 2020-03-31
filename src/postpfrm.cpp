/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postpfrm.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
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

// postpfrm.cpp -- frame window for posting/mailing dialogs
//
// Note:  the cancel warning is now handled by  TPostDoc::CanCloseFrame()
//

#include "stdafx.h"
#include "news.h"
#include "postpfrm.h"
#include "postdoc.h"
#include "postbody.h"
#include "attview.h"
#include "uimem.h"
#include "globals.h"
#include "posttool.h"
#include "warndlg.h"
#include "tglobopt.h"
#include "attdoc.h"
#include "custmsg.h"
#include "resource.h"            // ID_TAB_TO_EDIT
#include "rgwarn.h"
#include "tabedit.h"             // TTabEdit
#include "posthdr.h"             // giMailing
#include "utilsize.h"
#include "nglist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

IMPLEMENT_DYNCREATE(TPostPopFrame, TComposeFrame)

#define WMU_DEFER_MINIMIZE (WM_APP + 453)

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPostPopFrame, TComposeFrame)
	//ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_COMMAND(IDM_CANCEL, OnPostCancel)
	ON_CBN_SELCHANGE(idCompTool_Combo, OnSelchangeSignature)
	ON_UPDATE_COMMAND_UI(IDM_CANCEL, OnUpdatePostCancel)
	ON_COMMAND(WMU_POST_INITIALUPDATE, OnPostInitialUpdate)
	ON_COMMAND(IDM_COMPOSE_ATTACH, OnComposeAttach)
	ON_COMMAND(ID_TAB_TO_EDIT, OnTabToEdit)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WMU_DEFER_MINIMIZE, OnDeferMinimize)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TPostPopFrame::TPostPopFrame ()
: m_iOldSignatureIdx(-1)
{
}

// -------------------------------------------------------------------------
BOOL TPostPopFrame::OnCreateClient(LPCREATESTRUCT lpcs,
								   CCreateContext* pContext)
{
	// create a splitter with 2 rows, 1 column
	if (!m_wndSplit1.CreateStatic(this, 2, 1))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	CSize size_TopLeft(400, 110);         // default height is 110
	CSize size_TopRight(200, 110);        // default height is 110

	// We have a previous setting
	TWindowStatus ws;
	if (gpUIMemory->GetWindowStatus (GetWinType(), ws))
	{
		size_TopLeft = ws.m_sizeSlide2;
	}

	m_wndSplit1.SetRowInfo (0,                // row zero
		size_TopLeft.cy,  // ideal height
		10);              // minimum height

	// virt fn
	if (!CreateBodyView ( pContext ))
		return FALSE;

	// split the top pane
	if (!m_wndSplit2.CreateStatic(
		&m_wndSplit1,
		1, 2,
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		m_wndSplit1.IdFromRowCol(0,0))
		)
	{
		TRACE0("Failed to create nested splitter\n");
		return FALSE;
	}

	// set ideal width, min width
	m_wndSplit2.SetColumnInfo ( 0, 400, 10 );

	//  now create two views inside the nested splitter
	if (!m_wndSplit2.CreateView(0, 0,
		pContext->m_pNewViewClass, size_TopLeft, pContext))
	{
		TRACE0("postmdi: Failed to create top view pane\n");
		return FALSE;
	}
	m_wndSplit2.SetColumnInfo ( 1, 200, 10 );
	if (!m_wndSplit2.CreateView(0,   // row 1
		1,   // col 2
		RUNTIME_CLASS(TAttachView), size_TopRight, pContext))
	{
		TRACE0("postmdi: Failed to create top right pane\n");
		return FALSE;
	}

	SetActiveView((CView*)m_wndSplit2.GetPane(0,0));
	return TRUE;
}

// -------------------------------------------------------------------------
BOOL TPostPopFrame::CreateBodyView(CCreateContext* pContext)
{
	CSize size_Bottom(600, 100);

	// Load the window position
	TWindowStatus ws;
	if (gpUIMemory->GetWindowStatus (GetWinType(), ws))
		size_Bottom  = ws.m_sizeSlide1;

	if (!m_wndSplit1.CreateView(1, 0,
		RUNTIME_CLASS(TPostBody), size_Bottom, pContext))
	{
		TRACE0("postpfrm: failed to create bottom view pane\n");
		return FALSE;
	}

	return TRUE;
}

// -------------------------------------------------------------------------
void TPostPopFrame::SetSignature(const CString & strShortName)
{
	CWnd* pWndView = m_wndSplit1.GetPane(1, 0);
	ASSERT( pWndView->IsKindOf(RUNTIME_CLASS(TPostBody)));

	TPostBody* pBodyView = (TPostBody*) pWndView;

	int iCurrent;
	CString strCur;

	if (strShortName.IsEmpty())
	{
		m_pToolbar -> GetCurrent (iCurrent);
	}
	else
	{
		// there is a per-newsgroup sig
		m_pToolbar -> SelectSig (strShortName);
		m_pToolbar -> GetCurrent (iCurrent);
	}

	m_pToolbar -> GetSignatureText (iCurrent, strCur);

	m_iOldSignatureIdx = iCurrent;
	pBodyView->SetSignature(strCur);
}

// -------------------------------------------------------------------------
void TPostPopFrame::SetCCIntro (BOOL bInsert)
{
	CWnd* pWndView = m_wndSplit1.GetPane(1, 0);
	ASSERT( pWndView->IsKindOf(RUNTIME_CLASS(TPostBody)));
	TPostBody* pBodyView = (TPostBody*) pWndView;
	pBodyView -> SetCCIntro (bInsert);
}

// -------------------------------------------------------------------------
//  GetWinType is a virtual function that returns
//      TUIMemory::WND_POST   - followup just copies WND_POST
//      TUIMemory::WND_REPLY
void TPostPopFrame::OnDestroy()
{
	// Save the window position
	TWindowStatus ws;
	gpUIMemory->GetWindowStatus (GetWinType(), ws);

	ws.m_place.length = sizeof(ws.m_place);
	VERIFY (GetWindowPlacement ( &ws.m_place ));

	int cyCur, cyMin;
	int cxCur, cxMin;
	// big bottom pane
	m_wndSplit1.GetRowInfo (1, cyCur, cyMin );
	m_wndSplit1.GetColumnInfo (0, cxCur, cxMin );
	ws.m_sizeSlide1.cx = cxCur;
	ws.m_sizeSlide1.cy = cyCur;

	// top left
	m_wndSplit2.GetRowInfo (0, cyCur, cyMin);
	m_wndSplit2.GetColumnInfo (0, cxCur, cxMin);
	ws.m_sizeSlide2.cx = cxCur;
	ws.m_sizeSlide2.cy = cyCur;

	gpUIMemory->SetWindowStatus (GetWinType(), ws);

	ASSERT (m_pToolbar -> m_bAutoDelete == TRUE);
	m_pToolbar -> DestroyWindow ();
	m_pToolbar = 0;

	TComposeFrame::OnDestroy();
}

// -------------------------------------------------------------------------
BOOL TPostPopFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// this class has an icon predefined
	m_className.LoadString (giMailing ? IDS_CLASS_MAIL : IDS_CLASS_POST);
	cs.lpszClass = (LPTSTR)(LPCTSTR) m_className;

	return TComposeFrame::PreCreateWindow(cs);
}

// -------------------------------------------------------------------------
int TPostPopFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (TComposeFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	TWindowStatus ws;
	BOOL fPrevSettings = gpUIMemory->GetWindowStatus (GetWinType(), ws);
	if (fPrevSettings)
	{
		// utilsize.cpp
		Utility_SetWindowPlacementPreCreate ( ws.m_place, this );

		// ignore restore-to-minimized
		if (ws.m_place.showCmd == SW_SHOWMAXIMIZED)
			PostMessage (WMU_DEFER_MINIMIZE, ws.m_place.showCmd);
	}
	else
	{
		// some default position
	}

	CreateCustomToolbar();
	return 0;
}

// -------------------------------------------------------------------------
LRESULT TPostPopFrame::OnDeferMinimize (WPARAM wParam, LPARAM)
{
	ShowWindow (wParam);
	return 0;
}

// -------------------------------------------------------------------------
void TPostPopFrame::CreateCustomToolbar()
{
	TPostToolBar *pDerived = new TPostToolBar;
	m_pToolbar = pDerived;

	// giMailing is set in TPostTemplate::Launch ()
	pDerived -> m_bMailing = giMailing;
	m_pToolbar -> m_bAutoDelete = TRUE;
	pDerived -> Create (this);
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnPostCancel()
{
	PostMessage (WM_CLOSE);
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnSelchangeSignature()
{
	TRACE0("PostPFrm.Cpp - Selchange!\n");

	int iCurrent;

	m_pToolbar->GetCurrent (iCurrent);

	if (m_iOldSignatureIdx != iCurrent)
	{
		CString strCur, strPrev;

		m_pToolbar->GetSignatureText(iCurrent, strCur);
		m_pToolbar->GetSignatureText(m_iOldSignatureIdx,    strPrev);

#ifdef _DEBUG
		afxDump << "Presig: " << strPrev << "\n";
		afxDump << "Cursig: " << strCur  << "\n";
#endif

		CWnd* pWndView = m_wndSplit1.GetPane(1, 0);
		ASSERT(pWndView->IsKindOf(RUNTIME_CLASS(TComposeFormView)));

		TComposeFormView* pComposeView = (TComposeFormView*) pWndView;
		pComposeView -> SwapSignatures (strPrev, strCur);
		m_iOldSignatureIdx = iCurrent;
	}
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnUpdatePostCancel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// -------------------------------------------------------------------------
// SendWarning - Warn the user before they send something.
BOOL  TPostPopFrame::SendWarning()
{
	return TRUE;
}

// -------------------------------------------------------------------------
// CancelWarning - Warn the user before they lose work.
//                 Return TRUE  if ok to exit
//                        FALSE if user does not want to exit
BOOL TPostPopFrame::CancelWarning ()
{
	if (gpGlobalOptions->WarnOnExitCompose())
	{
		BOOL  fDisableWarning = FALSE;
		TPostDoc *pDoc = static_cast<TPostDoc *>(GetActiveDocument ());

		if (WarnWithCBX (pDoc -> m_iCancelWarningID,
			&fDisableWarning,
			this,
			FALSE /* iNotifyOnly */,
			TRUE /* bDefaultToFalse */))
		{
			if (fDisableWarning)
			{
				gpGlobalOptions->SetWarnOnExitCompose(FALSE);
				TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
				pRegWarning->Save();
			}
			return TRUE;
		}
		else
			return FALSE;
	}
	return TRUE;
}

// -------------------------------------------------------------------------
afx_msg void TPostPopFrame::OnPostInitialUpdate()
{
	CString strSigShortName;
	TServerCountedPtr servcp;

	CView * pView = GetActiveView();
	if (pView)
	{
		CDocument * pDoc = pView->GetDocument();
		if (pDoc->IsKindOf (RUNTIME_CLASS(TPostDoc)))
		{
			TPostDoc * pPostDoc = static_cast<TPostDoc*>(pDoc);

			LONG lGroupID = pPostDoc->m_lGroupID;

			TNewsGroup* pNG = 0;
			BOOL fUseLock=FALSE;
			TNewsGroupUseLock useLock((TNewsServer*)servcp, lGroupID, &fUseLock, pNG);

			if (fUseLock)
			{
				if (pNG->GetUseSignature())
					strSigShortName = pNG->GetSigShortName();
			}
		}
	}
	SetSignature (strSigShortName);
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnComposeAttach()
{
	CWnd* pWndView = m_wndSplit2.GetPane(0, 1);
	ASSERT(pWndView->IsKindOf(RUNTIME_CLASS(TAttachView)));

	((TAttachView*)pWndView)->QueryAttachments();
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnChooseGroup()
{
	// pass to header view
	m_wndSplit2.GetPane(0,0)->SendMessage(WM_COMMAND, IDM_CHOOSE_GROUP);
}

// -------------------------------------------------------------------------
// OnTabToEdit -- you can't just set focus, you must make it the "Active
//    Pane" (which makes it the active View, which gets the Cmd-routing)
void TPostPopFrame::OnTabToEdit()
{
	m_wndSplit1.SetActivePane (1, 0, NULL);
}

// -------------------------------------------------------------------------
int TPostPopFrame::EditSelectionExists ()
{
	CWnd *pWnd = GetFocus ();
	if (pWnd) {
		DWORD iStart = 0, iEnd = 0;
		pWnd -> SendMessage (EM_GETSEL, (WPARAM) &iStart, (LPARAM) &iEnd);
		return iStart != iEnd;
	}

	return FALSE;
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnEditCopy()
{
	CWnd *pWnd = GetFocus ();
	if (pWnd)
		pWnd -> SendMessage (WM_COPY);
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI -> Enable (EditSelectionExists ());
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnEditCut()
{
	CWnd *pWnd = GetFocus ();
	if (pWnd)
		pWnd -> SendMessage (WM_CUT);
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	pCmdUI -> Enable (EditSelectionExists ());
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnEditPaste()
{
	CWnd *pWnd = GetFocus ();
	if (pWnd)
		pWnd -> SendMessage (WM_PASTE);
}

// -------------------------------------------------------------------------
void TPostPopFrame::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI -> Enable (TRUE);
}

// -------------------------------------------------------------------------
BOOL TPostPopFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	CWnd * pWnd;

	if (EN_CHANGE == nCode)
	{
		pWnd = m_wndSplit1.GetPane (1, 0);

		if (::IsWindow(pWnd->m_hWnd) && (nID == (UINT) pWnd->GetDlgCtrlID()))
		{
			if (pWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
				return TRUE;
		}
	}
	else if (ID_JUMPTO_SUBJ <= nID && nID <= ID_JUMPTO_CC_AUTHOR)
	{
		pWnd = m_wndSplit2.GetPane (0, 0);
		if (pWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return TComposeFrame::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// -------------------------------------------------------------------------
void TPostPopFrame::SetAttachments (int iNumAttachments,
									const TAttachmentInfo *psAttachments)
{
	TAttachView *pAttachView = (TAttachView *) m_wndSplit2.GetPane (0, 1);
	pAttachView -> SetAttachments (iNumAttachments, psAttachments);
}
