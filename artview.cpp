/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artview.cpp,v $
/*  Revision 1.2  2010/09/11 19:27:20  richard_wood
/*  Fixed XFace split over multiple lines in header not showing properly bug.
/*  V3.0.3 Release - the first STABLE release of V3.0
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.12  2009/01/28 22:45:49  richard_wood
/*  Added border round article pane in main window.
/*  Cleaned up some memory leaks.
/*
/*  Revision 1.11  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.10  2009/01/11 22:28:40  richard_wood
/*  Fixed the XFace corruption on scroll bug for keyboard Down, Up, Page Down, Page Up, mouse wheel scroll, space bar scroll.
/*
/*  Revision 1.9  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.8  2008/09/19 14:51:12  richard_wood
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

// 6/20/95 - make this derive from TBaseFormView

// artview.cpp : implementation file
//
//
// ClassWizard is not too friendly with us - hand code ON_COMMAND
//
#include "stdafx.h"
#include "News.h"

#include "artview.h"

#include "article.h"

#include "arttext.h"

#include "triched.h"
#include "rtfspt.h"
#include "hints.h"
#include "thrdlvw.h"
#include "newsview.h"
#include "tglobopt.h"
#include "custview.h"
#include "tpopfrm.h"
#include "urlsppt.h"
#include "mainfrm.h"
#include "artdisp.h"
#include "tnews3md.h"
#include "tbozobin.h"            // AddBozo()
#include "timpword.h"            // AddWord()
#include "newsgrp.h"
#include "decoding.h"            // DecodeArticleCallback(), ...
#include "tdlgtmpv.h"
#if !defined(CUSTMSG_H)
#include "custmsg.h"
#endif
#include "utilrout.h"
#include "nglist.h"
#include "rgbkgrnd.h"
#include "rgswit.h"
#include "log.h"
#include "sysclr.h"
#include "globals.h"             // TNewsServer
#include "rgui.h"
#include "tmandec.h"             // TManualDecode
#include "nglist.h"
#include "server.h"              // TNewsServer
#include "newsdb.h"              // gpStore
#include "thrdact.h"             // TThreadActionList::Add()
#include "strext.h"
#include "tcharcoding.h"
#include "rgsys.h"
#include "8859x.h"

// I hate this but I need GetThreadView()

#include "genutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BOOL TArticleFormView::m_fThreadListViewAlive;
extern TGlobalOptions*  gpGlobalOptions;
extern TSystem gSystem;

/////////////////////////////////////////////////////////////////////////////
// TArticleFormView

IMPLEMENT_DYNCREATE(TArticleFormView, TBaseFormView)

TArticleFormView::TArticleFormView()
{
	m_fShowingFullHeader = FALSE;
	m_fShowingQuotedText = true;
	m_fShowingMsgSource  = false;

	InstallAcceltable();

	m_pCtlBrush = 0;
	m_dwBrush = (COLORREF) -1;
}

TArticleFormView::~TArticleFormView()
{
	delete m_pCtlBrush;
}

void TArticleFormView::DoDataExchange(CDataExchange* pDX)
{
	TBaseFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TArticleFormView, TBaseFormView)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_CONTROL(EN_SETFOCUS, IDC_FORMVIEW_RICHEDIT, OnFocusNotify)
	ON_MESSAGE(WMU_CHILD_TAB, OnChildTab)
	ON_MESSAGE(WMU_CHILD_ESC, OnChildEsc)
	ON_MESSAGE(WM_NEXTDLGCTL, OnNextDlgCtl)
	ON_MESSAGE(WMU_MBUTTON_DOWN, OnMButtonDown)
	ON_MESSAGE(WMU_ARTVIEW_UPDATEPOPMENU, OnUpdatePopup)
	ON_COMMAND(ID_CMD_TABBACK, OnCmdTabBack)
	ON_COMMAND(ID_CMD_TABAROUND, OnCmdTabaround)
	ON_COMMAND(ID_ARTICLE_SAVE_AS, OnArticleSaveAs)
	ON_COMMAND(IDR_ARTPOP_CHOOSEFONT, OnProperties)
	ON_COMMAND(ID_HELLO_POSTNEWARTICLE, OnPostMessage)
	ON_COMMAND(ID_ARTICLE_CHARCODING, OnArticleCharcoding)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_SAVE_AS, OnUpdateArticleSaveAs)
	ON_COMMAND(ID_ARTICLE_ROT13, OnRot13)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_ROT13, OnUpdateArticleRot13)
	ON_COMMAND(ID_SEARCH_FIND, OnSearchFind)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FIND, OnUpdateSearchFind)
	ON_COMMAND(ID_ARTICLE_BOZO, OnBozo)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_BOZO, EnableIfHeaderPresent)
	ON_COMMAND(ID_HELLO_MAILREPLY, OnMailReply)
	ON_UPDATE_COMMAND_UI(ID_HELLO_MAILREPLY, OnUpdateArticleMailreply)
	ON_COMMAND(ID_HELLO_POSTFOLLOWUP, OnPostFollowup)
	ON_UPDATE_COMMAND_UI(ID_HELLO_POSTFOLLOWUP, OnUpdatePostFollowup)
	ON_COMMAND(IDR_ARTPOP_FORWARDBYMAIL, OnForwardArticleByMail)
	ON_UPDATE_COMMAND_UI(IDR_ARTPOP_FORWARDBYMAIL, OnUpdateForwardbymail)
	ON_COMMAND(IDC_DECODE, OnDecode)
	ON_UPDATE_COMMAND_UI(IDC_DECODE, EnableIfMessagePresent)
	ON_COMMAND(ID_ARTICLE_TOGGLE_FULL_HDR, OnToggleFullHeader)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_TOGGLE_FULL_HDR, OnUpdateToggleFullHeader)
	ON_COMMAND(ID_ARTICLE_DELETE_SELECTED, OnDeleteSelected)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_DELETE_SELECTED, OnUpdateDeleteSelected)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_READ, OnKillThread)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_READ, OnUpdateKillThread)
	ON_COMMAND(ID_ARTICLE_TOGGLE_QUOTEDTEXT, OnArticleToggleQuotedtext)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_TOGGLE_QUOTEDTEXT, OnUpdateArticleToggleQuotedtext)
	ON_COMMAND(ID_ARTICLE_SHOW_SOURCE, OnArticleShowSource)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_SHOW_SOURCE, OnUpdateArticleShowSource)
	ON_COMMAND(ID_ARTICLE_REPAIRURL, OnArticleRepairURL)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_REPAIRURL, OnUpdateArticleRepairURL)
	ON_COMMAND(IDC_MANUAL_DECODE, OnManualDecode)
	ON_UPDATE_COMMAND_UI(IDC_MANUAL_DECODE, EnableIfMessagePresent)
	ON_COMMAND(ID_ARTICLE_IMPWORD, OnImpWord)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_IMPWORD, EnableIfHeaderPresent)
	ON_COMMAND(IDC_VIEW_BINARY, OnViewBinary)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_BINARY, EnableIfMessagePresent)
	ON_COMMAND(ID_FORWARD_SELECTED, OnForwardArticleByMail)
	ON_UPDATE_COMMAND_UI(ID_FORWARD_SELECTED, OnUpdateForwardbymail)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_MORE, OnUpdateArticleMore)
END_MESSAGE_MAP()

BOOL TArticleFormView::OnCmdMsg(UINT nID, int nCode, void* pExtra,
								AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// 9-3-95  Tnews3md.cpp window does the 4way dispatch.

	// then let us Try
	return TBaseFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/////////////////////////////////////////////////////////////////////////////
// TArticleFormView diagnostics

#ifdef _DEBUG
void TArticleFormView::AssertValid() const
{
	TBaseFormView::AssertValid();
}

void TArticleFormView::Dump(CDumpContext& dc) const
{
	TBaseFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TArticleFormView message handlers
int TArticleFormView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	LogString("article view start create");
	if (TBaseFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_wndRich.m_pDoc = GetDocument ();
	m_wndRich.m_pParentFormView = this;

	m_fShowingFullHeader = gpGlobalOptions->GetRegUI()->GetShowFullHeader();
	m_fShowingQuotedText = gpGlobalOptions->GetRegUI()->GetShowQuotedText();

	LogString("article view end create");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void TArticleFormView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	CWnd * pRich = GetRichEdit();

	// oversize it, to hide the border of richedit wnd.
	if (pRich)
	{
		pRich->MoveWindow(2, 2, cx-4, cy-4);
	}
}

/////////////////////////////////////////////////////////////////////////////
void TArticleFormView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	switch (lHint)
	{
	case VIEWHINT_SAVESEL:
	case VIEWHINT_SHOWGROUP:
	case VIEWHINT_SHOWGROUP_NOSEL:
	case VIEWHINT_STATUS_CHANGE:
	case VIEWHINT_ZOOM:
	case VIEWHINT_UNZOOM:
	case VIEWHINT_SERVER_SWITCH:
		return;

	case VIEWHINT_SHOWARTICLE:
		OnUpdateHelper();
		break;

	case VIEWHINT_EMPTY:
		Clear ();
		break;

	default:
		// base class does another Invalidate
		TBaseFormView::OnUpdate ( pSender, lHint, pHint );
		break;
	}
}

// ------------------------------------------------------------------------
// RLW : Fixed bug where if the XFace string was split over multiple
// lines we only picked up the first line, producing a corrupt XFace.
bool TArticleFormView::display_xface ( TPersist822Text* pText )
{
	const CString&  strFullHeader = pText->GetHeader();

	CString hdr(strFullHeader);

	TStringEx smart(&hdr);
	int pos = 0;
	CString strLine;
	bool bFound = false, bProcessingXFace = false;
	CString strXFace, strTemp;

	// Iterate through all the header lines
	while (smart.GetLine(pos, strLine))
	{
		// If we have NOT found the XFace header
		if (!bFound)
		{
			// See if this line is it?
			if (strLine.Left(8).CompareNoCase("X-Face: ") == 0)
			{
				// Yes, chop off the 'X-Face: ', tidy up and append the rest to the XFace string
				bFound = true;
				strLine = strLine.Mid(8);
				strLine.Trim();
				strXFace += strLine;
			}
		}
		else
		{
			// As the XFace line can continue over more than one line, see if the next line is a continuation?
			if ((strLine[0] == ' ') || (strLine[0] == '\t'))
			{
				// Yes, tidy it up and append to the XFace string
				strLine.Trim();
				strXFace += strLine;
			}
			else
				break;
		}
	}

	// Set the XFace
	if (bFound && !strXFace.IsEmpty())
		m_wndRich.SetXFace(strXFace);

	return bFound;
}

///////////////////////////////////////////////////////////////////////////
//
//
void TArticleFormView::OnUpdateHelper(void)
{
	TPersist822Header * pHdr   = 0;
	TPersist822Text   * pText  = 0;

	bool found = false;
	GetCurrentMessagePtrs ( pHdr, pText );

	if (pHdr && pText)
	{
		CWnd * pRich = GetRichEdit ();
		int viewingCharsetId = gpGlobalOptions->GetRegSystem()->GetViewingCharsetID();
		GravCharset * pCharset = gsCharMaster.findById (viewingCharsetId);

		if (0 == pCharset)
			pCharset = gsCharMaster.findById (1);		// fallback position

		RichDisplayArticle( GetRichEdit (), pHdr, pText,
			m_fShowingFullHeader,
			m_fShowingQuotedText,
			m_fShowingMsgSource,
			pCharset );

		// maybe the user doesn't want to show xfaces
		if (gpGlobalOptions->GetRegSwitch()->GetXFaces())
		{
			found = display_xface ( pText );
		}
	}
	else if (pHdr)
	{
		TArticleHeader * pArtHdr = pHdr->CastToArticleHeader ();
		if (pArtHdr)
		{
			// we only have the Header (no Body). So show a summary
			RichDisplaySummary ( GetRichEdit(), pHdr );
		}
	}
	if (false==found)
		m_wndRich.HideXFace();
}

/////////////////////////////////////////////////////////////////////////////
void TArticleFormView::OnInitialUpdate()
{
	LogString ("article view start initial update");

	CWnd * pRich = GetRichEdit();
	if (pRich)
	{
		m_wndRich.SubclassWindow(pRich->GetSafeHwnd());
		m_wndRich.PostSubclass();

		// Dammit. Turn off WS_EX_CLIENTEDGE
		LONG lStyle = GetWindowLong(m_wndRich.m_hWnd, GWL_EXSTYLE);
		SetWindowLong(m_wndRich.m_hWnd, GWL_EXSTYLE,
			lStyle & ~WS_EX_CLIENTEDGE);

		// please send WM_NOTIFY:EN_SELCHANGE to the parent
		// we are looking to trap VK_ENTER, also
		DWORD dwEvntMask = m_wndRich.SendMessage(EM_GETEVENTMASK);
		dwEvntMask |= ENM_SELCHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_SCROLLEVENTS;
		m_wndRich.SendMessage(EM_SETEVENTMASK, 0, dwEvntMask);

		SetInitialFont(pRich);

		// auto vscroll (duh) + make it Read-Only
		m_wndRich.SendMessage(EM_SETOPTIONS, ECOOP_OR, ECO_AUTOVSCROLL | ECO_READONLY);

		// ECO_AUTOVSCROLL somehow creates a phantom scrollbar
		//   do a little detour to crush the vertical scrollbar
		SCROLLINFO sScrollInfo;
		sScrollInfo.cbSize = sizeof(sScrollInfo);
		sScrollInfo.fMask = SIF_RANGE;
		sScrollInfo.nMin = sScrollInfo.nMax = 0;
		m_wndRich.SetScrollInfo(SB_VERT, &sScrollInfo, TRUE);

		TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
		COLORREF crf = pBackgrounds->GetEffectiveArtviewBackground();

		m_wndRich.SendMessage(EM_SETBKGNDCOLOR, FALSE, LPARAM(crf));

		m_wndRich.SetupXFaceWindow();
	}

	TBaseFormView::OnInitialUpdate();

	LogString ("article view end initial update");
}

// richedit has focus now
afx_msg void TArticleFormView::OnFocusNotify()
{
	//((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->SetActiveView(this, FALSE);

	// let the mdichild window know that we (index 2) have the focus
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->SendMessage ( WMU_CHILD_FOCUS, 0, 2L );
}

/////////////////////////////////////////////////////////////////////////////
// If we are clearing the pane, then close the full window also.
//
void TArticleFormView::Clear(void)
{
	if (IsWindow(m_wndRich.m_hWnd))
	{
		if (m_wndRich.SendMessage(WM_GETTEXTLENGTH) > 0)
			m_wndRich.SendMessage(WM_SETTEXT, 0, (LPARAM)"");
		m_wndRich.HideXFace();
	}
}

void TArticleFormView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
		SetFocus();   // set focus to self;

	TBaseFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// Pass this up so we can tab around
LRESULT TArticleFormView::OnChildTab (WPARAM wParam, LPARAM lParam)
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->PostMessage ( WMU_CHILD_TAB, wParam, 2L );
	return 0;
}

LRESULT TArticleFormView::OnChildEsc (WPARAM wParam, LPARAM lParam)
{
	return 0;
}

afx_msg LRESULT TArticleFormView::OnNextDlgCtl(WPARAM wParam, LPARAM lParam)
{
	// In a dialog box, by default, as you switch to an edit box, the entire text is selected.
	// I want to squish this behavior

	return 0;
}

void TArticleFormView::OnSetFocus(CWnd* pOldWnd)
{
	TBaseFormView::OnSetFocus(pOldWnd);

	m_wndRich.SetFocus ();
	// since we are in a dlgbox, this selects everything by default
	// but the TRichEd handles 'OnGetDlgCode'
}

void TArticleFormView::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	TBaseFormView::OnClose();
}

/////////////////////////////////////////////////////////////////////
//
//
//
//
void TArticleFormView::OnArticleSaveAs(void)
{
	TPersist822Header * pHdr;
	TPersist822Text   * pText;

	GetCurrentMessagePtrs ( pHdr, pText );

	if (pHdr && pText)
		GetTitleView()->SaveOneArticle ( this, pHdr, pText );
	else
		ASSERT(0);
}

void TArticleFormView::OnUpdateArticleSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsMessagePresent());
}

//////////////////////////////////////////////////////////////////////
// Protected
//  fill in the ptrs
void TArticleFormView::GetCurrentMessagePtrs (
	TPersist822Header*& pHdr,
	TPersist822Text*&   pText)
{
	CNewsView* pNV = GetTitleView()->GetNewsView();
	if (NULL == pNV)
	{
		ASSERT(0);
	}
	else
	{
		pHdr = pNV->GetBrowseHeader();
		pText = pNV->GetBrowseText();
	}
}

/////////////////////////////////////////////////////////////////////
//  - I'm going to change the actual text in the TArticleText & redraw
//    thus the decoded text will have the correct font and the user can
//    save the decoded text to file.
//    this is better than changing the RichEdit text alone.
void TArticleFormView::OnRot13(void)
{
	TPersist822Header * pHdr;
	TPersist822Text   * pText;

	long nStart = 0, nEnd = 0;
	m_wndRich.GetSel(nStart, nEnd);
	if ((nStart == nEnd) ||
		(((nStart == 0)||(nStart == -1)) && (nEnd == -1)))
	{
		// No selection or everything selected
		GetCurrentMessagePtrs ( pHdr, pText );

		if (pHdr && pText)
		{
			CString strBody = pText->GetBody();
			// decode text
			TextRotate13 ( strBody );

			// plop back in
			pText->SetBody ( strBody );

			// redisplay & Invalidate
			RichDisplayArticle ( GetRichEdit (), pHdr, pText);
		}
	}
	else
	{
		// Part of article selected
		CString strWord;
		m_wndRich.SelectedText(strWord);

		// decode text
		TextRotate13(strWord);

		// Set it back into window
		m_wndRich.ReplaceSel(strWord);
	}
}

void TArticleFormView::OnUpdateArticleRot13(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsMessagePresent());
}

void TArticleFormView::OnSearchFind()
{
	m_wndRich.OnSearchFind();
}

void TArticleFormView::OnUpdateSearchFind(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsMessagePresent());
}

int TArticleFormView::RequestPageDown(void)
{
	return m_wndRich.RequestPageDown();
}

BOOL TArticleFormView::CanRequestPageDown(void)
{
	return m_wndRich.CanRequestPageDown();
}

////////////////////////////////////////////////////////////////////////
// OnBozo -- add word to bozo bin
void TArticleFormView::OnBozo ()
{
	TPersist822Header *pHdr;
	TPersist822Text *pText;
	GetCurrentMessagePtrs (pHdr, pText);

	if (!pHdr)
		return;

	TBozoBin::AddBozo (pHdr->CastToArticleHeader()->GetFrom ());
}

////////////////////////////////////////////////////////////////////////
// OnImpWord -- add word to important-word list
void TArticleFormView::OnImpWord ()
{
	CString strWord;
	m_wndRich.SelectedText (strWord);
	if (strWord.IsEmpty ())
		m_wndRich.GetWordUnderCaret (strWord);
	if (!strWord.IsEmpty ())
		TImpWord::AddImportantWord (strWord);
}

////////////////////////////////////////////////////////////////////////
void TArticleFormView::OnPostMessage ()
{ AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_NEWSGROUP_POSTARTICLE); }

////////////////////////////////////////////////////////////////////////
void TArticleFormView::OnUpdateArticleMailreply(CCmdUI* pCmdUI)
{ pCmdUI->Enable (IsMessagePresent()); }

void TArticleFormView::OnMailReply ()
{ AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ARTICLE_REPLYBYMAIL); }

////////////////////////////////////////////////////////////////////////
void TArticleFormView::OnUpdatePostFollowup(CCmdUI* pCmdUI)
{  pCmdUI->Enable (IsMessagePresent()); }

void TArticleFormView::OnPostFollowup ()
{ AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ARTICLE_FOLLOWUP); }

void TArticleFormView::OnProperties ()
{
	// TODO: Add your command handler code here
	CPropertySheet       dlgOptions(IDS_CAPTION_ARTVIEW);
	TDlgTemplateView     pgViewTemplate;
	TBackgroundColors  * pBackgroundColors;

	pgViewTemplate.InitializeCustomView (gpGlobalOptions->GetCustomView());

	pgViewTemplate.m_fGetFullHeader = gpGlobalOptions->DownloadFullHeader ();
	pgViewTemplate.m_fUsePopup = gpGlobalOptions->UsingPopup ();
	pgViewTemplate.m_kSortThread = gpGlobalOptions->GetThreadSort ();
	CopyMemory (&pgViewTemplate.m_treeFont, gpGlobalOptions->GetTreeFont(), sizeof(LOGFONT));

	pBackgroundColors = gpGlobalOptions->GetBackgroundColors();
	pgViewTemplate.m_background = pBackgroundColors->GetArtviewBackground();
	pgViewTemplate.m_defBackground = gpGlobalOptions->GetRegSwitch()->IsDefaultArticleBG();
	pgViewTemplate.m_iQuotedTextMax = gpGlobalOptions->GetRegUI()->GetShowQuotedMaxLines();

	pgViewTemplate.m_fSkipHTMLPart = gpGlobalOptions->GetRegSwitch()->GetSkipHTMLPart();

	//////////////////////////////////////
	// Add all pages to the property sheet

	dlgOptions.AddPage ( &pgViewTemplate );

	if (IDOK == dlgOptions.DoModal())
	{
		gpGlobalOptions->SetCustomView ( pgViewTemplate.GetCustomView() );

		gpGlobalOptions->SetDlFullHeader(pgViewTemplate.m_fGetFullHeader);
		gpGlobalOptions->SetUsePopup (pgViewTemplate.m_fUsePopup);
		gpGlobalOptions->SetThreadSort ((TGlobalDef::EThreadSort) pgViewTemplate.m_kSortThread);
		gpGlobalOptions->SetTreeFont (&pgViewTemplate.m_treeFont);
		pBackgroundColors->SetArtviewBackground(pgViewTemplate.m_background);
		gpGlobalOptions->GetRegSwitch()->SetDefaultArticleBG(pgViewTemplate.m_defBackground);
		gpGlobalOptions->GetRegSwitch()->SetSkipHTMLPart(pgViewTemplate.m_fSkipHTMLPart);
		gpGlobalOptions->GetRegUI()->SetShowQuotedMaxLines(pgViewTemplate.m_iQuotedTextMax);

		gpStore->SaveGlobalOptions ();
		GetDocument()->UpdateAllViews(NULL, VIEWHINT_SHOWARTICLE);
	}
}

void TArticleFormView::OnForwardArticleByMail ()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ARTICLE_MAILTOFRIEND);
}

void TArticleFormView::OnUpdateForwardbymail(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsMessagePresent());
}

///////////////////////////////////////////////////////////////////////////
//
// LPARAM is really a CMenu*
LRESULT TArticleFormView::OnUpdatePopup(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);
	ProbeCmdUI (this,  (CMenu*) lParam);
	return TRUE;
}

BOOL TArticleFormView::IsMessagePresent()
{
	TPersist822Header * pHdr;
	TPersist822Text   * pText;

	GetCurrentMessagePtrs ( pHdr, pText );

	if (pHdr && pText)
		return TRUE;
	return FALSE;
}

void TArticleFormView::EnableIfHeaderPresent (CCmdUI* pCmdUI)
{
	TPersist822Header *pHdr;
	TPersist822Text *pText;
	GetCurrentMessagePtrs (pHdr, pText);

	pCmdUI->Enable (pHdr != 0);
}

void TArticleFormView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	BOOL fOn = IsMessagePresent();
	if (fOn)
	{
		// furthermore, check for a non-null Caret selection
		CHARRANGE cr;
		m_wndRich.SendMessage(EM_EXGETSEL, 0, LPARAM(&cr));
		if (cr.cpMin == cr.cpMax)
			fOn = FALSE;
	}
	pCmdUI->Enable(fOn);
}

void TArticleFormView::OnUpdateSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsMessagePresent());
}

// tnews3md.cpp does the real work.  The each active view must have
//   its own handler for this
void TArticleFormView::OnUpdateArticleMore(CCmdUI* pCmdUI)
{
	CMDIChildWnd* pMDIChild = ((CMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive();
	BOOL fOK = ((TNews3MDIChildWnd*)pMDIChild)->CanArticleMore();
	pCmdUI->Enable ( fOK );
}

// ------------------------------------------------------------------------
//
void TArticleFormView::OnToggleFullHeader ()
{
	m_fShowingFullHeader = !m_fShowingFullHeader;

	// Save it immediately
	gpGlobalOptions->GetRegUI()->SetShowFullHeader( m_fShowingFullHeader );
	gpGlobalOptions->GetRegUI()->Save ();

	OnUpdateHelper ();
}

void TArticleFormView::OnUpdateToggleFullHeader(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck ( m_fShowingFullHeader );
}

// this is a real accelerator
void TArticleFormView::OnCmdTabaround()
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
		PostMessage (WMU_CHILD_TAB, 0, 2L);
}

void TArticleFormView::OnCmdTabBack()
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
		PostMessage (WMU_CHILD_TAB, TRUE, 2L);
}

HBRUSH TArticleFormView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = TBaseFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	if (nCtlColor != CTLCOLOR_EDIT)
		return hbr;

	// Get background color
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
	COLORREF crf = pBackgrounds->GetEffectiveArtviewBackground();

	pDC->SetBkColor (crf);

	if (0 == m_pCtlBrush)
	{
		m_pCtlBrush = new CBrush (crf);
		m_dwBrush = crf;
	}

	// check for different color
	if (m_dwBrush != crf)
	{
		if (m_pCtlBrush)
			delete m_pCtlBrush;
		m_pCtlBrush = new CBrush (crf);
		m_dwBrush = crf;
	}

	return (*m_pCtlBrush);
}

void TArticleFormView::NonBlockingCursor(BOOL fOn)
{
	CWnd* pWnd = GetRichEdit();
	HCURSOR cursor = LoadCursor(NULL, fOn ? IDC_APPSTARTING : IDC_IBEAM);
	SetClassLong(pWnd->m_hWnd, GCL_HCURSOR, (LONG) cursor);
}

// -------------------------------------------------------------------------
// Utilize the new view templat
void TArticleFormView::ApplyViewTemplate(void)
{
	OnUpdateHelper ();
}

// -------------------------------------------------------------------------
// returns a copy of selected text
void TArticleFormView::SelectedText (CString &str)
{
	m_wndRich.SelectedText (str);
}

// -------------------------------------------------------------------------
void TArticleFormView::EnableIfMessagePresent (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsMessagePresent ());
}

// -------------------------------------------------------------------------
LONG TArticleFormView::GetCurrentNGID ()
{
	LONG gid = GetTitleView ()->GetNewsView ()->GetCurNewsGroupID ();
	return gid;
}

// -------------------------------------------------------------------------
void TArticleFormView::OnViewBinary ()
{
	TPersist822Header *pHeader;
	TPersist822Text *pText;
	GetCurrentMessagePtrs (pHeader, pText);
	ASSERT (pHeader && pText);

	TServerCountedPtr cpNewsServer;

	BOOL fUseLock;
	TNewsGroup *pNG;
	LONG gid = GetCurrentNGID ();
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return;
	ASSERT (pNG);

	CString dir;

	DecodeArticleCallbackData sData(TRUE, pText->CastToArticleText(), dir);

	NormalDecodeCallback (NULL,
		(TArticleHeader *) pHeader,
		pNG,
		(DWORD) &sData);
}

// -------------------------------------------------------------------------
void TArticleFormView::OnDecode ()
{
	TPersist822Header *pHeader;
	TPersist822Text *pText;
	GetCurrentMessagePtrs (pHeader, pText);
	ASSERT (pHeader && pText);

	TServerCountedPtr cpNewsServer;  // smart-pointer

	LONG gid = GetCurrentNGID ();
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return;
	ASSERT (pNG);

	CString dir;

	DecodeArticleCallbackData sData(FALSE, pText->CastToArticleText(), dir);

	NormalDecodeCallback (NULL, // artnode
		(TArticleHeader *) pHeader,
		pNG,
		(DWORD) &sData);
}

// -------------------------------------------------------------------------
void TArticleFormView::OnManualDecode ()
{
	TPersist822Header *pHeader;
	TPersist822Text *pText;
	GetCurrentMessagePtrs (pHeader, pText);
	ASSERT (pHeader && pText);

	TServerCountedPtr cpNewsServer;  // smart-pointer

	BOOL fUseLock;
	LONG gid = GetCurrentNGID ();
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
		return;
	ASSERT (pNG);

	TManualDecode sManualDecodeDlg;
	gpsManualDecodeDlg = &sManualDecodeDlg;

	QueueArticleForManualDecoding ((TArticleHeader *) pHeader,
		TRUE /* iSelected */,
		pNG,
		0 /* dwData */);
	sManualDecodeDlg.DoModal();
}

// ------------------------------------------------------------------------
// since the DEL key is an accelerator - it gets passed to all
// views.
void TArticleFormView::OnUpdateDeleteSelected(CCmdUI* pCmdUI)
{
	BOOL fEnableDelete = FALSE;

	if (GetThreadView() && 1 == GetThreadView()->GetSelCount())
		fEnableDelete = TRUE;

	pCmdUI->Enable ( fEnableDelete );
}

// ------------------------------------------------------------------------
void TArticleFormView::OnDeleteSelected(void)
{
	// manually re-route to thread pane
	if (GetThreadView() )
		GetThreadView()->PostMessage (WM_COMMAND, ID_ARTICLE_DELETE_SELECTED);
}

// ------------------------------------------------------------------------
void TArticleFormView::OnUpdateKillThread(CCmdUI* pCmdUI)
{
	BOOL fEnableDelete = FALSE;

	if (GetThreadView() && 1 <= GetThreadView()->GetSelCount())
		fEnableDelete = TRUE;

	pCmdUI->Enable ( fEnableDelete );
}

// ------------------------------------------------------------------------
void TArticleFormView::OnKillThread(void)
{
	// manually re-route to thread pane
	if (GetThreadView() )
		GetThreadView()->PostMessage (WM_COMMAND, ID_THREAD_CHANGETHREADSTATUSTO_READ);
}

// ------------------------------------------------------------------------
void TArticleFormView::OnArticleToggleQuotedtext()
{
	// change value
	m_fShowingQuotedText = !m_fShowingQuotedText;

	// save it immediately
	gpGlobalOptions->GetRegUI()->SetShowQuotedText( m_fShowingQuotedText );
	gpGlobalOptions->GetRegUI()->Save ();

	OnUpdateHelper ();
}

// Menu-item says "Mute Quoted Text"
void TArticleFormView::OnUpdateArticleToggleQuotedtext(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (m_fShowingQuotedText ? 0 : 1);
}

// ------------------------------------------------------------------------
LRESULT TArticleFormView::OnMButtonDown (WPARAM wParam, LPARAM lParam)
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->SendMessage ( WMU_MBUTTON_DOWN );
	return 0;
}

// ------------------------------------------------------------------------
// called from MDI
BOOL TArticleFormView::handleMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	return m_wndRich.handleMouseWheel (nFlags, zDelta, pt);
}

// ------------------------------------------------------------------------
void TArticleFormView::RichEditEnterKey ()
{
	HWND hLbx =
		GetThreadView()->GetInternalLBX();

	::SendMessage (hLbx, WM_LBUTTONDBLCLK, 0, 0);
}

// ------------------------------------------------------------------------
//
void TArticleFormView::OnArticleShowSource()
{
	m_fShowingMsgSource = !m_fShowingMsgSource;

	if (m_fShowingMsgSource)
		m_fShowingFullHeader = TRUE;
	else
		m_fShowingFullHeader = FALSE;

	OnUpdateHelper ();
}

void TArticleFormView::OnUpdateArticleShowSource(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck ( m_fShowingMsgSource );
}

void TArticleFormView::OnArticleRepairURL()
{
	m_wndRich.ArticleRepairURL ();
}

void TArticleFormView::OnUpdateArticleRepairURL(CCmdUI* pCmdUI)
{
	CString txt;
	m_wndRich.SelectedText (txt);

	pCmdUI->Enable (!txt.IsEmpty());
}

// force a character set
void TArticleFormView::OnArticleCharcoding() 
{
	TCharCoding sDlg(GetRichEdit());

	if (IDCANCEL == sDlg.DoModal())
		return;

	OnUpdateHelper();
}

BOOL TArticleFormView::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
	COLORREF crf = pBackgrounds->GetEffectiveArtviewBackground();
	pDC->FillSolidRect(rect, crf);

	return true;
}
