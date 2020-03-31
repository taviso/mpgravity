/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: hierlbx.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.8  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.7  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/09/19 14:51:27  richard_wood
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

// hierlbx.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include <winver.h>
#include "resource.h"
#include "hierlbx.h"

#include "tglobopt.h"
#include "warndlg.h"
#include "thrdlvw.h"
#include "tthredpg.h"
#include "utilrout.h"
#include "custmsg.h"
#include "taglist.h"
#include "rgbkgrnd.h"
#include "rgswit.h"
#include "rgfont.h"
#include "sysclr.h"
#include "artnode.h"

#include "timeutil.h"
#include "rgui.h"
#include "genutil.h"             // EnsureVisibleSelection
#include "globals.h"
#include "decoding.h"            // ViewArticleBinary()
#include "tmandec.h"             // TManualDecode
#include "statchg.h"             // queue a status change
#include "rglaymdi.h"
#include "autoptr.h"
#include "autodrw.h"
#include "newsdb.h"              // gpStore
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "tbozobin.h"            // BozoCallback, ...
#include "thread.h"
#include "thrdact.h"             // TThreadActionList::Add()
#include "intvec.h"              // TIntVector
#include "tmanrule.h"            // TManualRule
#include "ruleutil.h"            // GetRuleIndex()
#include "newsurl.h"             // TProxicomIconMgr
#include "tscoring.h"            // ScoreSetBackground()
#include "printing.h"            // QueueArticleForPrinting()
#include "tnews3md.h"
#include "navigdef.h"
#include "rgwarn.h"
#include "utilpump.h"
#include "bits.h"                // THeapBitmap
#include "superstl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TSystem gSystem;
extern TGlobalOptions *gpGlobalOptions;
extern CFont* gpVariableFont;

CString THierDrawLbx::m_className;

#include "hierlbxw.h"

#define COLUMN_BUFFER  3

// this is the buffer between the open/close indicator and the start of text
#define TREE_OPENBMP_BUFFER 2

const UINT THierDrawLbx::idSelchangeTimer = 2413;
const UINT THierDrawLbx::idDragTimer = 2427;

// some custom msgs
#define WMU_LBUTTONUP      (WM_USER + 1000)

//static LPCTSTR ThousandsSeparator (LPCTSTR rcLines) ;

/////////////////////////////////////////////////////////////////////////////
// THierDrawLbx
//
// MouseDown                           MouseUp
//
//    WM_LBUTTONDOWN                     NM_CLICK  or  WM_LBUTTONUP
//
//                                       either will PostMsg WMU_CUSTOM_MOUSEUP
//
//
//
//
//
static  int gnMsgCount = 0;

// ------------------------------------------------------------------------
THierDrawLbx::THierDrawLbx()
{
	m_DS.bLines = TRUE;

	m_popUpMenu.LoadMenu (IDR_POPUP_THREADLIST);

	// the open/close node indicator
	m_imgListOpen.Create (IDB_TREE_OPEN,
		TREE_WIDTH_OPEN,// width
		1,              // grow factor
		RGB(255,0,255)  // transparent color (hot purple)
		);
	// tag for Retrieval
	m_imgListTag.Create (IDB_TREE_TAG, TREE_WIDTH_TAG,
		1, RGB(255, 0, 255));

	// decoding status.  Position 0 = waiting
	//                            1 = success
	//                            2 = decode error
	//                            3 = ignore article
	//                            4 = watch article
	//  The Watch/Ignore icons are piggy backing in this image list
	m_imgListDecode.Create (IDB_TREE_DEC, TREE_WIDTH_DEC,
		1, RGB(255, 0, 255));

	// Combination Important + Shield indicator && Document.  (they are
	//   all the same width.  Small version
	VERIFY (m_imgListImpShieldDoc.Create (IDB_TREE_IMPSHIELD_DOC,
		TREE_WIDTH_EXCLAM,
		1, RGB(255, 0, 255)));

	m_parentThreadView = 0;

	// handy - dandy clipping region
	m_rgnClip.CreateRectRgn(0,0,0,0);

	m_pCtlColorBrush = NULL;
	if (!gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG())
		m_pCtlColorBrush = new CBrush (gpGlobalOptions->GetBackgroundColors()
		-> GetTreeBackground());

	m_hSelchangeTimer = 0;
	m_hDragTimer = 0;
	m_iDLLVersionKey = 0;
}

// ------------------------------------------------------------------------
THierDrawLbx::~THierDrawLbx()
{
	delete m_pCtlColorBrush;
}

BEGIN_MESSAGE_MAP(THierDrawLbx, CListCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_NOTIFY_REFLECT(NM_CLICK,  OnLeftClick)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_WM_CHAR()
	ON_COMMAND(ID_THREADLIST_POSTNEWARTICLE,                 OnPost)
	ON_COMMAND(ID_THREADLIST_POSTFOLLOWUP,                   OnFollowup)
	ON_COMMAND(ID_THREADLIST_REPLYBYMAIL,                    OnReplyByMail)
	ON_COMMAND(ID_ARTICLE_CHANGESTATUSTO_READ, OnChangeArticle_Read)
	ON_COMMAND(ID_ARTICLE_CHANGESTATUSTO_NEW, OnChangeArticle_Unread)
	ON_COMMAND(ID_ARTICLE_CHANGESTATUSTO_IMPORTANT, OnChangeArticle_Important)
	ON_COMMAND(ID_ARTICLE_CHANGESTATUSTO_NORMALIMP, OnChangeArticle_NormalImp)
	ON_COMMAND(ID_ARTICLE_CHANGESTATUSTO_PERMANENT, OnChangeArticle_Permanent)
	ON_COMMAND(ID_ARTICLE_CHANGESTATUSTO_DELETABLE, OnChangeArticle_Deletable)
	ON_COMMAND(ID_ARTICLE_DELETE_SELECTED, OnChangeArticle_DeleteNow)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_READ, OnChangeThread_Read)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_NEW, OnChangeThread_Unread)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_IMPORTANT, OnChangeThread_Important)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_NORMALIMP, OnChangeThread_NormalImp)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_PERMANENT, OnChangeThread_Permanent)
	ON_COMMAND(ID_THREAD_CHANGETHREADSTATUSTO_DELETABLE, OnChangeThread_Deletable)
	ON_COMMAND(IDC_DECODE,        OnDecode)
	ON_COMMAND(ID_DECODE_TO,      OnDecodeTo)
	ON_COMMAND(IDC_MANUAL_DECODE, OnManualDecode)
	ON_COMMAND(ID_THREAD_FORWARD_SELECTED,                   OnForwardSelectedArticles)
	ON_COMMAND(ID_THREADLIST_SEARCH,                             OnSearch)
	ON_COMMAND(ID_THREAD_SAVE_SELECTED,                      OnSaveSelected)
	ON_COMMAND(ID_THREADLIST_PROPERTIES,                     OnProperties)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_ARTICLE_TAGFOR_FETCH, OnArticleTagRetrieve)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_KILLFOCUS()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDC_VIEW_BINARY, OnViewBinary)
	ON_COMMAND(ID_ARTICLE_BOZO, OnArticleBozo)
	ON_COMMAND(          ID_ADD_TO_WATCH, OnAddToWatch)
	ON_COMMAND(          ID_ADD_TO_IGNORE, OnAddToIgnore)
	ON_COMMAND(ID_FILE_PRINT, OnPrint)
	ON_COMMAND(ID_FILE_PRINT_SETUP, OnPrintSetup)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(WMU_LBUTTONUP, OnCustomLButtonUp)
	ON_MESSAGE(WMU_CUSTOM_MOUSEUP, OnCustomMouseUp)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_MESSAGE(WMU_SELCHANGE_OPEN, OnSelChangeOpen)
	ON_WM_MEASUREITEM_REFLECT()
	ON_UPDATE_COMMAND_UI(IDC_MANUAL_DECODE, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_DELETE_SELECTED, SelectionOK)
	ON_WM_ACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CHANGESTATUSTO_READ, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CHANGESTATUSTO_NEW, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CHANGESTATUSTO_IMPORTANT, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CHANGESTATUSTO_NORMALIMP, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CHANGESTATUSTO_PERMANENT, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CHANGESTATUSTO_DELETABLE, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_READ, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_NEW, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_IMPORTANT, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_NORMALIMP, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_PERMANENT, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_CHANGETHREADSTATUSTO_DELETABLE, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_FORWARD_SELECTED,        SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_THREAD_SAVE_SELECTED,            SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_TAGFOR_FETCH, SelectionOK)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_BINARY, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_BOZO, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ADD_TO_WATCH, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_ADD_TO_IGNORE, SelectionOK)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, SelectionOK)
	ON_WM_MOUSEMOVE()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// THierDrawLbx message handlers

// ------------------------------------------------------------------------
void THierDrawLbx::DrawItem(LPDRAWITEMSTRUCT lpDraw)
{
	if (lpDraw->itemID == -1)
		return;

	CDC * pDC = CDC::FromHandle (lpDraw->hDC);

	CFont* pPrevFont = pDC->SelectObject ( &m_font );

	switch (lpDraw->itemAction)
	{
	case ODA_DRAWENTIRE:
	case ODA_SELECT:

		// Draw Text normally or Selected Text
		OnDrawSelect ( pDC, lpDraw );

		if ((lpDraw->itemState & ODS_FOCUS)  && (m_hWnd == ::GetFocus()))
		{
			OnDrawFocus ( pDC, lpDraw );
		}
		break;
	}

	if (pPrevFont)
		pDC->SelectObject (pPrevFont);
}

void THierDrawLbx::OnDrawFocus (CDC * pDC,  LPDRAWITEMSTRUCT lpDraw)
{
	// we are using the artnode w/o locking it.  I think
	//  we would have to get a read lock on the NG.
	//
	// As a partial fix, catch all exceptions.
	try
	{
		CRect  rctCLIP(0,0,0,0);
		pDC->GetClipBox ( &rctCLIP);

		int nWidth = 7;
		int vWidth[7];

		m_pHeader->GetWidthArray (nWidth, vWidth);
		int  totalWidth = m_pHeader->GetRightMost ();
		//TRACE("ClipRect = %d  %d\n",   rctCLIP.left, rctCLIP.Width());

		TArtNode* pNode = (TArtNode*) lpDraw->itemData;

		int indent;
		if (m_parentThreadView->IsThreaded())
			indent = (pNode->m_depth + 1) * TREE_WIDTH_OPEN + TREE_OPENBMP_BUFFER;
		else
			indent = TREE_OPENBMP_BUFFER;

		RECT tmpRect;
		tmpRect.top    = lpDraw->rcItem.top;
		tmpRect.bottom = lpDraw->rcItem.bottom;
		tmpRect.left   = lpDraw->rcItem.left + indent;

		// highlight is determined by the tree depth or the clipping column
		if (m_parentThreadView->IsThreaded() &&
			m_pHeader->GetTemplate().GetLength() >= 2)
			tmpRect.left = min (tmpRect.left,  lpDraw->rcItem.left + vWidth[0] + vWidth[1]);

		tmpRect.right  = totalWidth;

		pDC->DrawFocusRect ( &tmpRect );
	}
	catch(...)
	{
	}
}

TArtNode*  THierDrawLbx::GetpNode(int idx)
{
	return (TArtNode*) GetItemDataPtr(idx);
}

///////////////////////////////////////////////////////////////////////
// wrap this in exception handler               -amc 4-9-96
void THierDrawLbx::OnDrawSelect (CDC* pDC, LPDRAWITEMSTRUCT lpDraw)
{
	try
	{
		DrawSelect ( pDC, lpDraw );
	}
	catch(...)
	{
	}
}

// ------------------------------------------------------------------------
void THierDrawLbx::DrawSelect (CDC* pDC, LPDRAWITEMSTRUCT lpDraw)
{
	int nWidth = 7;
	int vWidth[7];
	TArtNode* pNode = (TArtNode*) lpDraw->itemData;
	int depth = pNode->m_depth;
	int lineCY;

	DWORD dwConnectLevel = 0L;

	if (m_parentThreadView->IsThreaded())
	{
		// Figure out which connecting lines are needed.
		// If this item is the last kid or one it parents
		// is a last kid, then it does not need a connector at that level
		if (0 == depth)
			dwConnectLevel = 0L;
		else
			dwConnectLevel = SetupConnectionBits ( pNode );
	}

	m_pHeader->GetWidthArray (nWidth, vWidth);
	lineCY = lpDraw->rcItem.bottom - lpDraw->rcItem.top;

	int textStart;
	if (m_parentThreadView->IsThreaded())
		textStart = ((1+depth) * TREE_WIDTH_OPEN) + TREE_OPENBMP_BUFFER;
	else
		textStart = TREE_OPENBMP_BUFFER;

	RECT rctBack = lpDraw->rcItem;

	rctBack.left += TREE_OPENBMP_BUFFER;
	rctBack.right = m_pHeader->GetRightMost ();

	WORD status_bits = 0;
	BOOL fHave = FALSE;
	BOOL fOK = m_parentThreadView->GetArticleStatus(pNode->GetArticleNumber(),
		status_bits, fHave);
	if (fOK)
	{
		// update display info
		pNode->SetStatusBits (status_bits);
	}
	else
	{
		// use whatever we have in the node object
		status_bits = pNode->GetStatusBits ();
	}

	// first, draw background color for this row
	DWORD dwBack;
	{
		if (gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG())
			dwBack = gSystem.Window();
		else
			dwBack = gpGlobalOptions->GetBackgroundColors()->GetTreeBackground();

		// give scoring system a chance to set the background color
		long lScore;
		if (m_parentThreadView->IsThreaded ()) {
			lScore = pNode->GetpRandomHeader ()->GetPileScore ();
			if (pNode->GetpRandomHeader ()->GetPileZeroPresent ())
				lScore = max (0, lScore);
		}
		else
			lScore = pNode->GetpRandomHeader ()->GetScore ();
		ScoreSetBackground (dwBack, lScore);

		rctBack.left = lpDraw->rcItem.left;
		CBrush br(dwBack);
		pDC->FillRect(&rctBack, &br);
	}

	// Selection  & winFocus   -  Use Highlight Color
	// !Selection & winFocus   -  nothing
	// Selection  & !winFocus  -  FrameRect
	// !Selection & !winFocus  -  nothing

	DWORD dwText;
	BOOL f_win_focus =  (::GetFocus() == m_hWnd);
	if (f_win_focus && (lpDraw->itemState & ODS_SELECTED))
	{

		dwBack = gSystem.Highlight();
		rctBack.left = lpDraw->rcItem.left + textStart;
		if (m_parentThreadView->IsThreaded() &&
			m_pHeader->GetTemplate().GetLength() >= 2)
			rctBack.left = min (rctBack.left, lpDraw->rcItem.left + vWidth[0] + vWidth[1]);

		::FillRect ( pDC->m_hDC, &rctBack, (HBRUSH)(COLOR_HIGHLIGHT + 1));
		dwText = gSystem.HighlightText();
	}
	else
	{
		if (lpDraw->itemState & ODS_SELECTED)
		{
			COLORREF clrFrame = gSystem.Highlight();
			CBrush brFrame(clrFrame);
			rctBack.left = lpDraw->rcItem.left + textStart;
			if (m_parentThreadView->IsThreaded() &&
				m_pHeader->GetTemplate().GetLength() >= 2)
				rctBack.left = min (rctBack.left, lpDraw->rcItem.left + vWidth[0] + vWidth[1]);
			pDC->FrameRect(&rctBack, &brFrame);
		}

		if (status_bits & TStatusUnit::kNew)
			dwText = gpGlobalOptions->GetRegFonts()->GetTreeFontColor();
		else
			dwText = gpGlobalOptions->GetRegFonts()->GetNewArticleColor();
	}

	{
		// new - more efficient ?
		if (m_imgListImpShieldDoc.GetBkColor() != dwBack)
		{
			m_imgListTag.SetBkColor ( dwBack );
			m_imgListDecode.SetBkColor ( dwBack );
			m_imgListImpShieldDoc.SetBkColor ( dwBack );
		}
		COLORREF crfBack  = pDC->GetBkColor ();
		if (m_imgListOpen.GetBkColor () != crfBack)
			m_imgListOpen.SetBkColor ( crfBack );
	}

	// draw "tree-stuff" only if we are threaded
	COLORREF oldTextColor;
	int oldMode;
	if (m_parentThreadView->IsThreaded())
	{
		const int iPictureCY =  TREE_PIC_CY_NORMAL;
		RECT& rcItem = lpDraw->rcItem;
		POINT pt;

		pt.x = rcItem.left + ( depth * TREE_WIDTH_OPEN );
		pt.y = rcItem.top;
		if (rcItem.bottom - rcItem.top > iPictureCY)
			pt.y += ((rcItem.bottom - rcItem.top) - iPictureCY) / 2;

		// clip to first column (virtual) + 2nd column

		clipRect (pDC, rcItem.left, rcItem.top, vWidth[0] +vWidth[1] -2, rcItem.bottom, TRUE);
		DrawIndicatorOpenClose(pDC, pt, pNode);

		oldTextColor = pDC->SetTextColor (dwText);
		oldMode = pDC->SetBkMode (TRANSPARENT);

		if (m_DS.bLines && depth > 0)
			DrawConnectionLines ( pDC, &lpDraw->rcItem, depth, dwConnectLevel );
		clipUndo (pDC);
	}
	else
	{
		oldTextColor = pDC->SetTextColor (dwText);
		oldMode = pDC->SetBkMode (TRANSPARENT);
	}

	RECT backup = lpDraw->rcItem;

	//lpDraw->rcItem.left += textStart;

	if (/* Draw text even if the NG statusbit has been deleted */  TRUE )
	{
		BOOL fTag = m_parentThreadView->IsArticleTagged( pNode->GetArticleNumber() );
		HIERBLASTTEXT sBlast;

		// fill the Blast data structure
		sBlast.pDC         = pDC;
		sBlast.lpDraw      = lpDraw;
		sBlast.depth       = depth;
		sBlast.nWidth      = nWidth;
		sBlast.vWidth      = vWidth;
		sBlast.status_bits = status_bits;
		sBlast.fTagged     = fTag;
		sBlast.fHave       = fHave;
		sBlast.textStart   = textStart;

		this->BlastText (  &sBlast );
	}

	pDC->SetBkMode ( oldMode );
	pDC->SetTextColor ( oldTextColor );
	lpDraw->rcItem = backup;
}

// $$ #define DEBUG_SHOW_LINE_NUMBER

///////////////////////////////////////////////////////////////////////////
//
//  10-21-97  these flags may not be honored by ExtTextOut!!

#define DT_L_VCENTER (DT_VCENTER | DT_SINGLELINE | DT_LEFT  | DT_NOPREFIX)
#define DT_R_VCENTER (DT_VCENTER | DT_SINGLELINE | DT_RIGHT | DT_NOPREFIX)

void THierDrawLbx::BlastText (LPHIERBLASTTEXT pBlast)
{
	const CString& myTemplate = GetTemplate();

	TArtNode* pNode = (TArtNode*) pBlast->lpDraw->itemData;

	TArtUIData* pUIData = 0;

	pNode->GetDisplayData (pUIData);

	int iX = 0;
	for (int i = 0; i < myTemplate.GetLength(); ++i)
	{
		switch (myTemplate[i])
		{
		case 'T':
			// virtual column to handle Toggle between threading & flat sorting
			break;

		case 'F':
		case 'f':
			BlastToken (iX, i, pUIData->m_From, pBlast, DT_L_VCENTER);
			break;
		case 'S':
		case 's':
			{
				CFont* pPrevFont;
				BOOL fBold = FALSE;
				TArtNode* pNode = (TArtNode*) pBlast->lpDraw->itemData;
				fBold = m_parentThreadView->IsThreaded() && pNode->FirstInThread();
				if (fBold)
					pPrevFont = pBlast->pDC->SelectObject(&m_boldFont);

#if defined(DEBUG_SHOW_LINE_NUMBER) && defined(_DEBUG)
				{
					// show line numbers
					CString ss; ss.Format("%d^%s",
						pNode->GetArticleNumber (),
						(LPCTSTR) pUIData->m_Subj);
					BlastToken (iX, i, ss, pBlast, DT_L_VCENTER);
				}
#else
				BlastToken (iX, i, pUIData->m_Subj, pBlast, DT_L_VCENTER);
#endif
				if (fBold)
					pBlast->pDC->SelectObject(pPrevFont);

			}
			break;
		case 'L':
		case 'l':
			{
				// RLW : Add space after line count.
				CString strTmp;
				strTmp.Format("%d  ", pUIData->m_iLines);
//				_itot (pUIData->m_iLines, rcLines, 10);
//				LPCTSTR pszNumber = ThousandsSeparator (rcLines);
//				BlastToken (iX, i, pszNumber, pBlast, DT_R_VCENTER);
				BlastToken (iX, i, strTmp, pBlast, DT_R_VCENTER);
				break;
			}
		case 'I':
		case 'i':
			// Indicators
			BlastIndicators(iX, i, pBlast);
			break;
		case 'D':
		case 'd':
			{
				CTime aTime(pUIData->m_localTime);
				CString strDisplayTime;
				GetCustomFormat (gpGlobalOptions->GetRegUI()->GetDateFormatStr(),
					aTime, strDisplayTime);

				BlastToken (iX, i, strDisplayTime, pBlast, DT_L_VCENTER);
			}
			break;
		case 'R':
		case 'r':
			{
//				TCHAR rcScore[30];
//				_itot (pUIData->m_iScore, rcScore, 10);
//				BlastToken (iX, i, rcScore, pBlast, DT_R_VCENTER);
				CString strTmp;
				strTmp.Format("%d  ", pUIData->m_iScore);
				BlastToken(iX, i, strTmp, pBlast, DT_R_VCENTER);
				break;
			}
		default:
			ASSERT(0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//
void THierDrawLbx::BlastToken(int& iX,
							  int iField,
							  LPCTSTR pTok,
							  LPHIERBLASTTEXT pBlast,
							  UINT uFlag)
{
	RECT rct = pBlast->lpDraw->rcItem;
	int  lo = 0;
	int  hi ;

	int k = 0;
	for (k = 0; k < iField; ++k)
	{
		lo += pBlast->vWidth[k];
	}
	hi = lo + pBlast->vWidth[k];

	if (1 == iField)  // #1 is buddy to virtual column
	{
		// use lo bound or the Depth indent
		rct.left  = pBlast->lpDraw->rcItem.left + pBlast->textStart;

		//  LOOK max (0, pBlast->lpDraw->rcItem.left);
	}
	else
	{
		// use lo bound as the left margin
		rct.left  = pBlast->lpDraw->rcItem.left + lo;
	}
	rct.right = pBlast->lpDraw->rcItem.left + hi;

	// this could happen since the Left part is indented for n-Depth levels
	if (rct.left >= rct.right)
		return;

	if (uFlag == DT_R_VCENTER)
		rct.right -= COLUMN_BUFFER;
	else
		rct.left += COLUMN_BUFFER;

	if (rct.left >= rct.right)
		return;

	int iLen = _tcslen(pTok);
	//pBlast->pDC->DrawText ( pTok, iLen, &rct, uFlag );
	custDrawText (pBlast->pDC, pTok, iLen, &rct, uFlag );
}

// ------------------------------------------------------------------------
void THierDrawLbx::custDrawText (CDC* pDC, LPCTSTR pTok, int iLen,
								 LPRECT pRect, UINT uFlag)
{
	if (uFlag & DT_RIGHT)
	{
		// drawText is slower, but will do Right-Align
		pDC->DrawText ( pTok, iLen, pRect, uFlag );
	}
	else
	{
		// Use ExtText out for more SPEED!!

		int x = pRect->left;
		int y = pRect->top;
		int rh = pRect->bottom - pRect->top;

		// vertical center this by hand
		if (rh > m_fontCY)
			y += (rh - m_fontCY) / 2;

		pDC->ExtTextOut ( x, y, ETO_CLIPPED,
			pRect,   // clip rectangle
			pTok,
			iLen,
			NULL );  // default spacing between chars
	}
}

///////////////////////////////////////////////////////////////////////////
// draw Indicators
//
void THierDrawLbx::BlastIndicators(int iX, int iField, LPHIERBLASTTEXT pBlast)
{
	int  lo = 0;
	int  hi = 0;
	// width of column based on header control
	for (int k = 0; k <= iField; ++k)
	{
		if (k < iField)
			lo += pBlast->vWidth[k];
		hi += pBlast->vWidth[k];
	}
	if (lo + COLUMN_BUFFER >= hi)
		return;

	RECT rct = pBlast->lpDraw->rcItem;
	POINT pt;

	// if this is Column 0, then account for the possible depth indent
	if (0 == iField)
		pt.x =  rct.left ;
	else
		pt.x = rct.left + lo;   /* LOOK */
	pt.y = rct.top;

	if (rct.bottom-rct.top > TREE_DOC_CY)
		pt.y += (rct.bottom-rct.top - TREE_DOC_CY) / 2;

	// clip to column
	clipRect (pBlast->pDC, pt.x , rct.top, hi - COLUMN_BUFFER,
		rct.bottom, TRUE);

	// - draw TAG indicator
	if (pBlast->fTagged)
		m_imgListTag.Draw(pBlast->pDC, 0, pt, ILD_NORMAL);
	pt.x += TREE_WIDTH_TAG;

	// - draw Watch/Ignore indicator.  Bitmaps are stored in "m_imgListDecode"
	int iWatchIdx = -1;
	if (pBlast->status_bits & TStatusUnit::kWatch)
		iWatchIdx = 4;
	else if (pBlast->status_bits & TStatusUnit::kIgnore)
		iWatchIdx = 3;
	if (iWatchIdx > 0)
		m_imgListDecode.Draw (pBlast->pDC, iWatchIdx, pt, ILD_NORMAL);
	pt.x += TREE_WIDTH_WATCH;

	// - draw DECODE indicator
	int iDecIdx = -1;
	if (pBlast->status_bits & TStatusUnit::kQDecode)
		iDecIdx = 0;
	else if (pBlast->status_bits & TStatusUnit::kDecoded)
		iDecIdx = 1;
	else if (pBlast->status_bits & TStatusUnit::kDecodeErr)
		iDecIdx = 2;
	if (iDecIdx >=0 )
		m_imgListDecode.Draw (pBlast->pDC, iDecIdx, pt, ILD_NORMAL);
	pt.x += TREE_WIDTH_DEC;

	{
		int iPicIndex = 0;
		// - draw exclamation point & Shield for Important articles
		if (pBlast->status_bits & TStatusUnit::kImportant)
		{
			iPicIndex = (pBlast->status_bits & TStatusUnit::kPermanent) ? 1 : 2;
		}
		else
		{
			if (pBlast->status_bits & TStatusUnit::kPermanent)
				iPicIndex = 3;
		}
		if (iPicIndex)
		{
			// shield stuff comes after Five document icons.

			m_imgListImpShieldDoc.Draw (pBlast->pDC, iPicIndex + 4, pt, ILD_NORMAL);
		}
	}

	pt.x += TREE_WIDTH_EXCLAM;

	// - draw DOCUMENT
	if (pBlast->status_bits & TStatusUnit::kSendErr)
	{
		iDecIdx = 4;  // server don't have it
	}
	else if (pBlast->status_bits & TStatusUnit::kNew)
	{
		if (pBlast->fHave)
			iDecIdx = 0;   // new - in database
		else
			iDecIdx = 2;   // new - but on the server
	}
	else
	{
		if (pBlast->fHave)
			iDecIdx = 1;   // old - in database
		else
			iDecIdx = 3;   // old - up on server
	}
	m_imgListImpShieldDoc.Draw (pBlast->pDC, iDecIdx, pt, ILD_NORMAL);

	clipUndo (pBlast->pDC);
} // BlastIndicators

//-------------------------------------------------------------------------
void THierDrawLbx::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	int idx;

	idx = getNotifyIndex (pNMHDR);

	if (idx < 0)
	{
		idx = GetFirstSelection();

		if (LB_ERR == idx)
			return;
	}

	TArtNode * pNode = GetpNode (idx);
	if (pNode)
	{
		TPersist822Header* p822Hdr = pNode->GetpRandomHeader();

		if (p822Hdr)
			m_parentThreadView->DisplayArticle ( p822Hdr, pNode );
	}
}

//-------------------------------------------------------------------------
void THierDrawLbx::OpenBranch(int idx)
{
	try
	{
		TArtNode* pNode = (TArtNode*) GetItemDataPtr (idx);

		int artInt = pNode->GetArticleNumber();

		// is this open already?
		if (IsOpened(artInt))
			return;

		OpenItem ( artInt );
		int iAdded = 0;
		pNode->InsertKids ( this, idx + 1, FALSE, pNode->m_depth, iAdded );
		ShowKids ( idx, iAdded );
	}
	catch(...)
	{
		// silent catch
	}
}

//-------------------------------------------------------------------------
// pSelInfo - info to help maintain the selection. x indicates the
//            current selection. If we are deleting a listbox line that
//            is above 'x', then increment y;
void THierDrawLbx::CloseBranch(int idx, int* piSelIdx /* = 0 */)
{
	int tot = GetCount();
	int depth;
	int mydepth;
	TArtNode* pNode;

	pNode = (TArtNode*) GetItemDataPtr(idx);
	mydepth = pNode->m_depth;

	CloseItem (pNode->GetArticleNumber());

	idx++;
	BOOL fDone = FALSE;
	if (idx >= tot)
		return;

	while (!fDone)
	{
		// Remove child items, until depth triggers stop
		TArtNode* pNode = (TArtNode*) GetItemDataPtr(idx);
		depth = pNode->m_depth;
		if (depth <= mydepth)
			break;
		int artInt = pNode->GetArticleNumber();
		if (IsOpened(artInt))
			CloseItem (artInt);

		// returns new count
		int n = selection_delete_string (idx, piSelIdx);

		// idx++;   no need for this
		if (idx >= n)
			fDone = TRUE;
	}
}

///////////////////////////////////////////////////////////////////////////
//  implements Expand All Threads, Collapse All Threads
//
void THierDrawLbx::ExpandAll (BOOL fExpand)
{
	BOOL fLock = LockWindowUpdate ();

	TPersist822Header* p822Hdr = 0;

	// slightly different logic to maintain a reasonable
	// selection
	int iSelCount = GetSelCount();
	int idx, i;
	int iArtInt = -1;
	// maintain selection only if there is a single
	if (1==iSelCount &&  1==GetSelItems(1, &idx))
	{
		if (fExpand)
		{
			p822Hdr = Getp_Header (idx);
			if (!p822Hdr)
				return;
			iArtInt = p822Hdr->GetNumber();
		}
		else
		{
			// hunt upwards for a top level node
			for (i = idx; i >= 0; --i)
			{
				TArtNode* pNode = (TArtNode*) GetItemDataPtr(i);
				if (0==pNode->m_depth)
				{
					iArtInt = pNode->GetArticleNumber();
					break;
				}
			}
		}
	}

	// walk backwards
	NodeExpand ( fExpand );

	// reset selection after all the shifting
	if (-1 != iArtInt && 1==iSelCount)
	{
		int tot = GetCount();
		if (fExpand)
		{
			// hunt down from original position
			for (i = idx; i < tot; ++i)
			{
				p822Hdr = Getp_Header(i);
				if (!p822Hdr)
					return;
				if (p822Hdr->GetNumber() == iArtInt)
				{
					SetOneSelection(i);
					break;
				}
			}
		}
		else
		{
			// hunt up from original
			for (i = min(idx, tot-1); i >= 0; --i)
			{
				p822Hdr = Getp_Header(i);
				if (!p822Hdr)
					return;
				if (p822Hdr->GetNumber() == iArtInt)
				{
					SetOneSelection(i);
					break;
				}
			}
		}
	}

	if (fLock)
		UnlockWindowUpdate ();
}

void THierDrawLbx::CloseItem(int uniq)
{
	m_stlOpenSet.erase (uniq);
}

// use a unique number (the Article number)
void THierDrawLbx::OpenItem(int uniq)
{
	m_stlOpenSet.insert (uniq);
}

// use a unique number (the Article number)
BOOL THierDrawLbx::IsOpened(int artInt)
{
	STLUniqSet::iterator it = m_stlOpenSet.find (artInt);

	if (it == m_stlOpenSet.end())
		return FALSE;
	else
		return TRUE;
}

// AMC - looks like a Refinement
void  THierDrawLbx::ShowKids(int iCurrentSelection, int Kids)
{
}

// append to the end of the list. Mark as OPEN
//  fOpen is FALSE if we are showing collapsed
void  THierDrawLbx::AddInfo (void * pData, int uniq, BOOL fOpen)
{
	// store the artnode directly
	int ret = AddString((LPCTSTR) pData);

	ASSERT(LB_ERR      != ret);
	ASSERT(LB_ERRSPACE != ret);

	if (fOpen)
		OpenItem ( uniq );
	else
		CloseItem ( uniq );
}

int THierDrawLbx::AddString(LPCTSTR s)
{
	LVITEM lvi;

	//ZeroMemory (&lvi, sizeof(lvi) );

	lvi.mask     = LVIF_PARAM ;
	lvi.pszText  = 0;
	lvi.iItem    = this->GetItemCount();
	lvi.iSubItem = 0;
	lvi.lParam   = LPARAM(s);

	int iAt = InsertItem ( &lvi );
	VERIFY(iAt != -1);

	return iAt;
}

// this is to Insert at a specific position
int THierDrawLbx::InsertInfo(int idx, LPCTSTR pText, void * pData, int uniq)
{
	LVITEM  lvi;

	lvi.mask = LVIF_PARAM;
	lvi.iItem = idx;
	lvi.iSubItem = 0;
	lvi.lParam   = LPARAM(pData)
		;
	int iInsertedAt = InsertItem ( &lvi );

	OpenItem ( uniq );
	return iInsertedAt;
}

void THierDrawLbx::FastRect(HDC hdc, int x, int y, int cx, int cy)
{
	RECT rc;
	rc.left = x;
	rc.right = x+cx;
	rc.top = y;
	rc.bottom = y+cy;
	//ExtTextOut(hdc, x, y, ETO_OPAQUE, &rc, NULL, 0, NULL);
	FillRect (hdc, &rc, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
}

void THierDrawLbx::DrawIndicatorOpenClose(CDC* pDC, POINT& pt, TArtNode* pNode)
{
	int idxOpen;

	if (IsOpened (pNode->GetArticleNumber()) )
	{
		idxOpen = 0;
	}
	else
	{
		if (0 == pNode->CountChildren())
		{
			// you have no kids! Show a minus.
			idxOpen = 0;
		}
		else
		{
			// decide if we have any unread articles in thread
			if (pNode->FirstInThread () && pNode->HaveUnreadElements())
				idxOpen = 2;
			else
				idxOpen = 1;
		}
	}

	m_imgListOpen.Draw (pDC, idxOpen, pt, ILD_TRANSPARENT);
}

///////////////////////////////////////////////////////////////////////////
//
//
DWORD THierDrawLbx::SetupConnectionBits(TArtNode* pNode)
{
	DWORD      dwConnectLevel = 0L;
	int        depth = pNode->m_depth;
	DWORD      dwMask;
	int        nTempLevel;
	TArtNode * pParent;

	nTempLevel = depth - 1;

	// Check parents ( grand, great, ... ) to see it they are last children
	//
	// Start at parent ( 1 level up from here ).
	pParent = pNode;

	// setup the bit
	dwMask = (DWORD) pow(2.0f, depth - 1);
	while ( nTempLevel >= 0 )
	{
		if (pParent->IsLastChild())
		{
			// Last kid so no connection at this level
			dwConnectLevel &= ~dwMask;
		}
		else
			dwConnectLevel |= dwMask;

		// move Mask bit over
		dwMask /= 2;

		nTempLevel--;
		pParent = pParent->GetParent();
	}

	return dwConnectLevel;
}

///////////////////////////////////////////////////////////////////////////
//
//
void THierDrawLbx::DrawConnectionLines (
										CDC* pDC,
										LPRECT prct,
										int   depth,
										DWORD dwConnectLevel)
{

	DWORD dwMask;
	int x, y, nTempLevel;
	RECT rcTemp = *prct;
	int lineCY = rcTemp.bottom - rcTemp.top;
	int origin = rcTemp.left ;

	//  buffer zone of 4 pixels
	int bmpCX = TREE_WIDTH_OPEN;
	int zone  = TREE_OPENBMP_BUFFER;

	dwMask = 1;

	// Draw a series of | lines for outer levels
	x = origin + bmpCX/2;

	for (nTempLevel = 0; nTempLevel < depth; ++nTempLevel)
	{
		if (dwConnectLevel & dwMask)
			FastRect(pDC->m_hDC, x, rcTemp.top, 1, lineCY);
		x += bmpCX;
		dwMask *= 2;
	}

	// Draw the short vertical line up towards the parent
	nTempLevel = depth - 1;
	dwMask *= 2;
	x = origin + (nTempLevel * bmpCX) + (bmpCX/2);
	if (dwConnectLevel & dwMask)
		y = rcTemp.bottom;
	else
		y = rcTemp.bottom - (lineCY/2);
	FastRect (pDC->m_hDC, x, rcTemp.top, 1, y-rcTemp.top);

	// Draw short horiz bar to right
	FastRect ( pDC->m_hDC, x, rcTemp.bottom-(lineCY/2), bmpCX/2, 1 );
}

///////////////////////////////////////////////////////////////////////////
//
//
const CString&  THierDrawLbx::GetTemplate(void)
{
	return m_pHeader->GetTemplate();
}

void THierDrawLbx::clipRect(
							CDC* pdc, int left, int top,
							int right, int bottom,
							BOOL fSaveOld
							)
{

	CRect rctCur;
	// save off old clip region
	if (fSaveOld)
		pdc->GetClipBox ( &rctCur );

	// get intersection between current ClipRect and the Input
	CRect intersect;
	CRect input(left, top, right, bottom);

	input.NormalizeRect();
	rctCur.NormalizeRect();

	intersect.IntersectRect(&rctCur, &input);

	m_rgnClip.SetRectRgn(intersect.left, intersect.top, intersect.right, intersect.bottom);
	pdc->SelectClipRgn ( &m_rgnClip );

	m_rctOldClip = rctCur;
}

void THierDrawLbx::clipUndo(CDC* pdc)
{
	m_rgnClip.SetRectRgn( &m_rctOldClip ); // restore old clip rect
	pdc->SelectClipRgn ( &m_rgnClip );     // install old clip region
}

int THierDrawLbx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//ghWndHier = m_hWnd;
	ApplyNewFont(FALSE);

	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void THierDrawLbx::ApplyNewFont(BOOL fRedraw /*=TRUE*/)
{
	delete m_pCtlColorBrush;
	m_pCtlColorBrush = 0;

	if (!gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG())
		m_pCtlColorBrush = new CBrush (gpGlobalOptions->GetBackgroundColors()->
		GetTreeBackground());

	if (0 == gpGlobalOptions->m_defTreeFont.lfFaceName[0])
	{
		// show this in the config dialog box
		gpVariableFont->GetObject ( sizeof (LOGFONT), &gpGlobalOptions->m_defTreeFont );
	}
	if (m_font.m_hObject)
	{
		m_font.DeleteObject();
		m_font.m_hObject = 0;
	}
	if (m_boldFont.m_hObject)
	{
		m_boldFont.DeleteObject();
		m_boldFont.m_hObject = 0;
	}
	LPCLOGFONT pLF;
	LOGFONT   info;
	if (gpGlobalOptions->IsCustomTreeFont())
	{
		pLF = gpGlobalOptions->GetTreeFont();
		m_font.CreateFontIndirect ( pLF );
	}
	else
	{
		// default to microplanet defined small font
		gpVariableFont->GetObject ( sizeof (info), &info );
		pLF = &info;
		m_font.CreateFontIndirect ( pLF );
	}

	LOGFONT lfBold;

	CopyMemory(&lfBold, pLF, sizeof(lfBold));
	lfBold.lfWeight = FW_BOLD;

	m_boldFont.CreateFontIndirect( &lfBold );

	adjust_item_height ();

	if (fRedraw)
	{
		// we have to do some junk to force Windows to send us another
		//   WM_MEASUREITEM message.  Hint from www.codeguru.com

		CRect rct(0,0,0,0);

		GetWindowRect (&rct);

		WINDOWPOS    wp;

		wp.hwnd = m_hWnd;
		wp.cx = rct.Width();
		wp.cy = rct.Height();
		wp.flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER |
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW ;

		SendMessage (WM_WINDOWPOSCHANGED, 0, (LPARAM) &wp);
	}

	//SetFont ( &m_font );    // for the list control does this do anything?

	if (fRedraw)
		Invalidate ();   // in lieu of SetFont
}

// ------------------------------------------------------------------------
void THierDrawLbx::MeasureItem (LPMEASUREITEMSTRUCT  lpMeasure)
{
	const int iPictureCY = TREE_PIC_CY_NORMAL;

	ASSERT(ODT_LISTVIEW == lpMeasure->CtlType);
	lpMeasure->itemHeight = max (m_fontCY, iPictureCY);
}

// ------------------------------------------------------------------------
// adjust_item_height -- measure item height and then adjust
void THierDrawLbx::adjust_item_height ()
{
	// store value in member  'm_fontCY'

	CalcFontHeight ( &m_font, m_fontCY );   // m_font is a data member
}

// ------------------------------------------------------------------------
// CalcFontHeight -- now depends on the current server (Proxicom or normal)
int THierDrawLbx::CalcFontHeight (CFont* pFont, int & h)
{
	TEXTMETRIC  tm;
	CClientDC   theDC(this);
	CFont * pOldFont;

	if (pFont)
		pOldFont = theDC.SelectObject ( pFont );

	// get height of this font
	theDC.GetTextMetrics ( &tm );
	h = tm.tmHeight + tm.tmExternalLeading;

	if (pFont)
		theDC.SelectObject ( pOldFont );

	return 0;
}

// ------------------------------------------------------------------------
// handles NM_CLICK
void THierDrawLbx::OnLeftClick(NMHDR* pNMHDR, LRESULT* pResult)
{

	TRACE("recv  NM_CLICK - post wmu_cust_mouseup\n");

	PostMessage (WMU_CUSTOM_MOUSEUP, 0, ++gnMsgCount);

	*pResult = 0;

	// do open/close
	int idx;
	int tot = GetCount();
	if (0 == tot || 0 == GetSelCount())
		return;

	GetSelItems (1, &idx);

	TArtNode* pNode = (TArtNode*) GetItemDataPtr (idx);

	m_ptLastClick = getMessagePosition ();

	int leftSide = pNode->m_depth * TREE_WIDTH_OPEN;

	if (m_ptLastClick.x > leftSide && m_ptLastClick.x < (leftSide + TREE_WIDTH_OPEN))
	{
		SetRedraw(FALSE);

		int uniq = pNode->GetArticleNumber();
		if (IsOpened(uniq))
			CloseBranch (idx);
		else
			OpenBranch (idx);
		SetRedraw(TRUE);
		if (tot != GetCount())
		{
			RECT rctClient, rctItem;
			GetClientRect (&rctClient);
			GetItemRect(idx, &rctItem, LVIR_BOUNDS);

			// redraw from here Down
			rctClient.top = rctItem.top;
			InvalidateRect (&rctClient,FALSE);
		}
	}

	PostMessage (WMU_LBUTTONUP);
}

// --------------------------------------------------------------------------
void THierDrawLbx::RepaintItem(LPVOID pVoid)
{
	try
	{
		TArtNode * pNode = (TArtNode*) pVoid;

		// pass work off to sibling function
		this->RepaintArticle ( pNode->GetArticleNumber() );
	}
	catch(...)
	{
	}
}

// --------------------------------------------------------------------------
void THierDrawLbx::RepaintArticle(int artInt)
{
	int iTop   = GetTopIndex ();
	int iTotal = GetCount();

	if (LB_ERR == iTop || LB_ERR == iTotal)
		return;

	// assume 75 lbx lines fit onscreen

	int iStop  = min (iTotal, iTop + 75);

	for (int i = iTop; i < iStop; i++)
	{
		TPersist822Header* pHdr =  Getp_Header (i);

		if (pHdr && (artInt == pHdr->GetNumber()))
		{
			RepaintIndex ( i );
			break;
		}
	}
}

// --------------------------------------------------------------------------
void THierDrawLbx::RepaintIndex(int idx)
{
	RECT rctItem;
	GetItemRect (idx, &rctItem, LVIR_BOUNDS);
	InvalidateRect (&rctItem);
}

// --------------------------------------------------------------------------
// Calling function must check return code
TPersist822Header* THierDrawLbx::Getp_Header(int idx)
{
	try
	{
		TArtNode*  pNode = GetpNode(idx);
		ASSERT(pNode);
		if (0 == pNode)
			return NULL;

		return pNode->GetpRandomHeader();
	}
	catch(...)
	{
		return NULL;
	}
}

// ------------------------------------------------------------------------
//
void THierDrawLbx::SetOneSelection(int idx)
{
	//   state  :  mask
	UINT  mask =  LVIS_SELECTED | LVIS_FOCUSED;

	SetItemState (-1,           // -1 does all items??
		0,           // put zero in ...
		UINT(-1) );  //  .. all states

	SetItemState  (idx, mask, mask);
	SetSelectionMark (idx);
}

// ------------------------------------------------------------------------
void THierDrawLbx::DeleteAll()
{
	m_stlOpenSet.clear();

	this->DeleteAllItems (); //ResetContent();
}

// ------------------------------------------------------------------------
int THierDrawLbx::GetFirstSelection()
{
	POSITION pos = this->GetFirstSelectedItemPosition ();

	if (NULL == pos)
		return LB_ERR;      // no items selected

	return this->GetNextSelectedItem (pos);
}

///////////////////////////////////////////////////////////////////////////
//
//
BOOL THierDrawLbx::Find(
						int artInt,
						int * pIndex,
						TPersist822Header** ppHdrRet)
{
	try
	{
		return FindFrom ( 0, artInt, pIndex, ppHdrRet );
	}
	catch(...)
	{
		return FALSE;
	}
}

// ------------------------------------------------------------------------
//
BOOL THierDrawLbx::FindFrom(int iStart, int iArtInt, int * pIndex,
							TPersist822Header** ppHdrRet)
{
	int i;
	int tot = GetCount();
	for (i = iStart; i < tot; ++i)
	{
		TPersist822Header* p822Hdr = Getp_Header (i);
		if (p822Hdr)
		{
			if (p822Hdr->GetNumber() == iArtInt)
			{
				*pIndex = i;
				if (ppHdrRet)
					*ppHdrRet = p822Hdr;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ------------------------------------------------------------------------
// Used when Search window jumps to Article that is in a collapsed thread.
BOOL THierDrawLbx::FindCollapsed(int artInt, int * pIndex, bool* pfParent)
{
	*pfParent = false;
	TPersist822Header * p822Hdr = 0;
	try
	{
		// walk up from the bottom
		for (int i = GetCount () - 1; i >= 0; i--)
		{
			TArtNode * pNode = GetpNode (i);
			if (pNode)
			{
				p822Hdr = pNode->GetpRandomHeader ();
				if (p822Hdr)
				{
					int nNodeKey = p822Hdr->GetNumber();
					if (nNodeKey == artInt)
					{
						*pIndex = i;
						return TRUE;
					}

					// find parent thread
					TThread * pThread = pNode->GetThread ();

					if (pThread->IsRoot(nNodeKey))  // optimization
					{
						TArtNode * pJunkNode = 0;

						// this is the thread that contains the article we want
						if (pThread->Have (artInt, pJunkNode))
						{
							// return parent index
							*pIndex = i;
							*pfParent = true;
							return TRUE;
						}
					}
				} // p822Hdr
			} // pNode
		}
	}
	catch(...)
	{
		ASSERT(0);
	}
	return FALSE;
}

BOOL THierDrawLbx::GetThreadRoot(int idx, int& rootIdx)
{
	if (idx >= GetCount())
		return FALSE;
	if (idx < 0)
		throw(new TException (IDS_ERR_TREEVIEW, kError));

	TArtNode* pArtNode = GetpNode (idx);
	while (pArtNode->m_depth > 0)
	{
		if (idx <= 0)
			return FALSE;
		pArtNode = GetpNode (--idx);
	}
	rootIdx = idx;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// Pass in a root, and we will find the Next/Previous root
//
BOOL THierDrawLbx::GetNextThread(int idx, int& sibIdx, BOOL fForward)
{
	TArtNode* pArtNode;
	int limit;
	if (fForward)
	{
		limit = GetCount();

		// keep going down until we find another line with Depth 0

		while (idx < limit-1)
		{
			++idx;
			pArtNode = GetpNode(idx);
			if (0 == pArtNode->m_depth)
			{
				sibIdx = idx;
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		limit = 0;

		// keep going up until we find another line with Depth 0

		while (idx > limit)
		{
			--idx;
			pArtNode = GetpNode(idx);
			if (0 == pArtNode->m_depth)
			{
				sibIdx = idx;
				return TRUE;
			}
		}
		return FALSE;
	}
}

void THierDrawLbx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		// circle around the panes
	case VK_TAB:
		((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->
			PostMessage (WMU_CHILD_TAB, GetKeyState(VK_SHIFT) < 0, 1);
		break;

	default:
		break;
	}
	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}

void THierDrawLbx::OnPost(void)
{ AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_NEWSGROUP_POSTARTICLE); }

void THierDrawLbx::OnFollowup(void)
{
	int idx = GetFirstSelection();
	if (LB_ERR != idx)
	{
		TArtNode* pArtNode = GetpNode(idx);
		TPersist822Header* p822Hdr = pArtNode->GetpRandomHeader();
		// ensure this is not an email message
		if (p822Hdr && p822Hdr->IsArticle())
			m_parentThreadView->Followup( pArtNode );
	}
}

void THierDrawLbx::OnReplyByMail(void)
{
	int idx = GetFirstSelection();
	if (LB_ERR != idx)
	{
		TArtNode* pArtNode = GetpNode(idx);
		TPersist822Header* p822Hdr = pArtNode->GetpRandomHeader();
		// ensure this is not an email message
		if (p822Hdr && p822Hdr->IsArticle())
			m_parentThreadView->ReplyByMail( pArtNode );
	}
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeArticle_Read(void)
{
	CWaitCursor cursor;

	m_parentThreadView->ChangeStatusUtil(TStatusUnit::kNew, FALSE);

	// function will maintain selection
	m_parentThreadView->FilterOutReadThreads ();

	int   iThrdOff, iThrdOn, iSortOff, iSortOn;

	// fancy navigation
	gpGlobalOptions->GetRegUI()->GetNavigKillArt (iThrdOff, iThrdOn, iSortOff, iSortOn);

	fnNavigate_Fire_Event (iThrdOff, iThrdOn, iSortOff, iSortOn);
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeArticle_Unread(void)
{
	CWaitCursor cursor;

	m_parentThreadView->ChangeStatusUtil (TStatusUnit::kNew, TRUE);

	// function will maintain selection
	m_parentThreadView->FilterOutNewThreads ();
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeArticle_Important(void)
{
	CWaitCursor cursor;
	m_parentThreadView->ChangeStatusUtil (TStatusUnit::kImportant, TRUE);
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeArticle_NormalImp(void)
{
	CWaitCursor cursor;
	m_parentThreadView->ChangeStatusUtil (TStatusUnit::kImportant, FALSE);

	// remove from the UI any threads that are completely Normal
	m_parentThreadView->FilterOutNormalImpThreads ();
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeArticle_Permanent(void)
{
	CWaitCursor cursor;

	// need to special function since we might ask
	// "This article is not in the DB, tag it?"
	VERIFY ( 0 == m_parentThreadView->ChangeArticle_Permanent() );
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeArticle_Deletable(void)
{
	CWaitCursor cursor;
	m_parentThreadView->ChangeStatusUtil (TStatusUnit::kPermanent, FALSE);
}

void THierDrawLbx::OnChangeArticle_DeleteNow(void)
{
	CWaitCursor cursor;
	if (gpGlobalOptions->WarnOnDeleteArticle())
	{
		BOOL fDisableWarning = FALSE;

		if (WarnWithCBX (IDS_WARN_ON_DELETE, &fDisableWarning))
		{
			if (fDisableWarning)
			{
				gpGlobalOptions->SetWarnOnDeleteArticle(FALSE);
				TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
				pRegWarning->Save();
			}
			// this drops through to do the deletion as if there
			// wasn't a warning
		}
		else
			return;
	}
	m_parentThreadView->DeleteArticlesNow();
}

//-------------------------------------------------------------------------
void THierDrawLbx::OnChangeThread_Read(void)
{
	try
	{
		// I'm not sure why we have both
		//            ID_THREAD_CHANGETHREADSTATUSTO_READ  (toolbar) and
		//            ID_THREAD_KILLTHREAD                 (accelerator)
		//
		// but the accelerator has fancy navigation afterwards.
		//
		m_parentThreadView->SendMessage (WM_COMMAND, ID_THREAD_KILLTHREAD);
	}
	catch(...)
	{
	}
}

// ------------------------------------------------------------------------
void THierDrawLbx::OnChangeThread_Unread(void)
{
	CWaitCursor cursor;
	m_parentThreadView->SetThreadStatus(TStatusUnit::kNew, TRUE);
	m_parentThreadView->FilterOutNewThreads();
}

// ------------------------------------------------------------------------
void THierDrawLbx::OnChangeThread_Important(void)
{
	CWaitCursor cursor;
	m_parentThreadView->SetThreadStatus(TStatusUnit::kImportant, TRUE);
}

// ------------------------------------------------------------------------
void THierDrawLbx::OnChangeThread_NormalImp(void)
{
	CWaitCursor cursor;
	m_parentThreadView->SetThreadStatus(TStatusUnit::kImportant, FALSE);
	m_parentThreadView->FilterOutNormalImpThreads();
}

// ------------------------------------------------------------------------
void THierDrawLbx::OnChangeThread_Permanent(void)
{
	m_parentThreadView->ChangeThread_Permanent ();
}

// ------------------------------------------------------------------------
void THierDrawLbx::OnChangeThread_Deletable(void)
{
	CWaitCursor cursor;
	m_parentThreadView->SetThreadStatus(TStatusUnit::kPermanent, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnForwardSelectedArticles(void)
{
	m_parentThreadView->SendMessage(WM_COMMAND, ID_FORWARD_SELECTED);
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnSearch(void)
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_SEARCH_SEARCH);
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnSaveSelected (void)
{
	m_parentThreadView->SendMessage (WM_COMMAND, ID_ARTICLE_SAVE_AS);
}

/////////////////////////////////////////////////////////////////////////////
// OnProperties - Article Tree Property Sheet/Page
/////////////////////////////////////////////////////////////////////////////

void THierDrawLbx::OnProperties(void)
{
	TThreadPage       threadPage;
	CPropertySheet    threadSheet (IDS_THREAD_PANE_PROP);
	TGlobalDef::EThreadSort eOldThreadSort;

	threadPage.m_fCustomTreeFont = FALSE;
	if (gpGlobalOptions->IsCustomTreeFont())
	{
		CopyMemory (&threadPage.m_treeFont, gpGlobalOptions->GetTreeFont(), sizeof(LOGFONT));
		threadPage.m_fCustomTreeFont = TRUE;
	}
	else
		CopyMemory (&threadPage.m_treeFont, &gpGlobalOptions->m_defTreeFont, sizeof(LOGFONT));

	threadPage.m_treeColor = gpGlobalOptions->GetRegFonts()->GetTreeFontColor();

	threadPage.m_fDefaultBackground = gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG();
	threadPage.m_treeBackground = gpGlobalOptions->GetBackgroundColors()->GetTreeBackground();
	eOldThreadSort = gpGlobalOptions->GetThreadSort();
	threadPage.m_sortThread = (int) eOldThreadSort;

	threadSheet.AddPage (&threadPage);
	if (IDOK == threadSheet.DoModal())
	{
		CMDIChildWnd * pMDIChild;
		CMDIFrameWnd * pMDIFrame = (CMDIFrameWnd*) AfxGetMainWnd();
		// note this is only 1 of the mdi children. could be more.
		//  we aren't handling them.
		pMDIChild = pMDIFrame->MDIGetActive();

		gpGlobalOptions->CustomTreeFont (threadPage.m_fCustomTreeFont);
		if (threadPage.m_fCustomTreeFont)
		{
			gpGlobalOptions->SetTreeFont ( &threadPage.m_treeFont );
		}
		gpGlobalOptions->GetRegFonts()->SetTreeFontColor (threadPage.m_treeColor);

		gpGlobalOptions->GetBackgroundColors()->SetTreeBackground(threadPage.m_treeBackground);
		gpGlobalOptions->GetRegSwitch()->SetDefaultTreeBG(threadPage.m_fDefaultBackground);

		gpGlobalOptions->SetThreadSort ((TGlobalDef::EThreadSort) threadPage.m_sortThread);
		// make it last forever...
		gpStore->SaveGlobalOptions ();

		// apply the tree font
		pMDIChild->PostMessage ( WMU_TREEVIEW_NEWFONT );

		if (eOldThreadSort != gpGlobalOptions->GetThreadSort())
			pMDIChild->SendMessage (WMU_TREEVIEW_REFILL);
	}
}

void THierDrawLbx::SelectionOK(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelection());
}

BOOL THierDrawLbx::IsSelection()
{
	return (LB_ERR == GetFirstSelection()) ? FALSE : TRUE;
}

int CALLBACK THierDrawLbx::fnCompareDate(LPARAM lParam1, LPARAM lParam2, LPARAM dummy)
{
	TPersist822Header* pHdr1 = (TPersist822Header*) lParam1;
	TPersist822Header* pHdr2 = (TPersist822Header*) lParam2;

	if (pHdr1->GetTime() == pHdr2->GetTime())
		return 0;
	return (pHdr1->GetTime() < pHdr2->GetTime()) ? -1 : 1;
}

int CALLBACK THierDrawLbx::fnCompareSubj(LPARAM lParam1, LPARAM lParam2, LPARAM dummy)
{
	TArticleHeader* pHdr1 = (TArticleHeader*) (lParam1);
	TArticleHeader* pHdr2 = (TArticleHeader*) (lParam2);

	LPCTSTR p1 = pHdr1->GetSubject();
	LPCTSTR p2 = pHdr2->GetSubject();
	if (('r' == p1[0] || 'R' == p1[0]) &&
		('e' == p1[1] || 'E' == p1[1]) &&
		(':' == p1[2]) &&
		(' ' == p1[3]))  p1 += 4;
	if (('r' == p2[0] || 'R' == p2[0]) &&
		('e' == p2[1] || 'E' == p2[1]) &&
		(':' == p2[2]) &&
		(' ' == p2[3]))  p2 += 4;
	int ret = lstrcmpi(p1, p2);
	if (ret)
		return ret;
	const CTime& t1 = pHdr1->GetTime();
	const CTime& t2 = pHdr2->GetTime();
	if (t1 == t2)
		return 0;
	else if (t1 < t2)
		return -1;
	else
		return 1;
}

void THierDrawLbx::OnSetFocus(CWnd* pOldWnd)
{
	int totSel = GetSelCount();
	int idx;
	CListCtrl::OnSetFocus(pOldWnd);

	if (0 == totSel && ((idx = GetTopIndex()) != LB_ERR))
		SetOneSelection ( idx );

	// FOR items that are Selected, but the Listbox doesn't have focus,
	//  they are Framed.
	// NOW that focus is coming back, redraw them (HILIGHT_COLOR)
	this->RepaintSelected ();

	return;
}

//-------------------------------------------------------------------------
// Redone to handle collapsed branches -amc 8-21-96
void THierDrawLbx::OnArticleTagRetrieve()
{
	TServerCountedPtr cpNewsServer;
	int iGroupID = 0;
	MAP_NODES stlMapNodes;
	QUE_HDRS  stlQueHdrs;      // collect output

	// get articles that are selected, handles collapsed branches.
	// note : i can access this since i am a friend
	if (LB_ERR == m_parentThreadView->engine_of_selection ( &stlMapNodes, FALSE ))
		return;

	BOOL fMode1 = FALSE;
	CString groupName;
	// discard any articles that already have local bodies
	m_parentThreadView->discard_local ( &stlMapNodes, &iGroupID, &groupName, &stlQueHdrs );

	TPersistentTags & rTags = cpNewsServer->GetPersistentTags();

	BOOL fUseLock;
	TNewsGroup * pNG = 0;
	TNewsGroupUseLock useLock(cpNewsServer, iGroupID, &fUseLock, pNG);
	if (!fUseLock)
		return;

	// for navigation, see if we are Tagging, Untagging or toggling
	bool fAllAddTag = true;

	QUE_HDRS::iterator itH = stlQueHdrs.begin();
	for (;itH != stlQueHdrs.end(); itH++)
	{
		TArticleHeader * pHdr = (*itH);

		if (rTags.FindTag ( iGroupID, pHdr->GetNumber() ))
		{
			rTags.DeleteTagCancelJob ( pNG->GetName(), iGroupID,
				pHdr->GetNumber(),
				TRUE);  // cancel queued job
			fAllAddTag = false;
		}
		else
		{
			TArticleHeader * pInputHeader = 0;

			int ret = rTags.AddTag ( pNG, pHdr->GetNumber(), pHdr->GetLines(),
				pInputHeader );

			if (ret)
				delete pInputHeader;
		}
	}

	stlMapNodes.clear();
	stlQueHdrs.clear();

	// redraw listbox items - to show Green ball
	int i, tot;
	if ((tot = GetSelCount()) <= 0)
		return;

	int * pIdx = new int[tot];
	auto_ptr<int> sIdxDeleter(pIdx);

	GetSelItems (tot, pIdx);

	for (i = tot - 1; i >= 0; --i)
		RepaintIndex (pIdx[i]);

	// do we want advanced Agent-like navigation?
	if (fAllAddTag)
	{
		if (tot > 1)
			// the intent here is to set the selection to the lowermost selected item
			//   of  a multiselect group.  So that the MoveDown navigation command will
			//   move to an untagged item.
			SetOneSelection (pIdx[tot-1]);

		int   iThrdOff, iThrdOn, iSortOff, iSortOn;

		gpGlobalOptions->GetRegUI()->GetNavigTag (iThrdOff, iThrdOn, iSortOff, iSortOn);

		fnNavigate_Fire_Event (iThrdOff, iThrdOn, iSortOff, iSortOn);

#if defined(OLDER_KODE)
		// this has the power to roll-over to a different NG.

		m_parentThreadView->JumpToEngine (TNewsGroup::kQueryNextUnreadTagFree,
			false,   // fViewArticle
			false);  // fLimitToCurPile

#endif
	}
	else
	{
		// if we un-tagged something, just do simple move (down-1)

		// move down one item
		if (tot == 1)
		{
			int iIndex = pIdx[0];
			if (iIndex != GetCount () - 1) // if not last item
			{
				SetOneSelection ( iIndex + 1 );
			}
		}
	}
}

// ----------------------------------------------------------------------
void THierDrawLbx::OnReturn (NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	int nIndex = GetFirstSelection();

	if (nIndex >= 0)
	{
		// view an article

		OpenArticleByIndex ( nIndex );
	}
}

// ----------------------------------------------------------------------
int THierDrawLbx::OpenArticleByIndex (UINT nIndex, BOOL bCheckMaxLines)
{
	TArtNode* pNode = GetpNode(nIndex);
	if (pNode)
	{
		TPersist822Header * p822Hdr = pNode->GetpRandomHeader ();
		if (p822Hdr)
		{
			m_parentThreadView->DisplayArticle(p822Hdr, pNode, TRUE, bCheckMaxLines);
			return 0;
		}
	}
	return 1;
}

// The WM_CTLCOLOR that is normally sent to the parent, has been
// reflected back down here
HBRUSH THierDrawLbx::CtlColor(CDC* pDC, UINT nCtlColor)
{
	if (gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG())
		return NULL;  // send back to parent and do default

	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
	COLORREF dwBack = pBackgrounds->GetTreeBackground();

	pDC->SetBkColor ( dwBack );
	return *m_pCtlColorBrush;
}

BOOL THierDrawLbx::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL ret;

	ret = CListCtrl::PreCreateWindow(cs);

	if (m_className.IsEmpty())
	{
		WNDCLASS wndcls;

		ZeroMemory (&wndcls, sizeof(wndcls));

		// retrieve WNDCLASS structure for the base window class
		VERIFY ( ::GetClassInfo(NULL,
			cs.lpszClass,
			&wndcls) );

		m_className = "MicroPlanet_TreeLBX";

		// Give new class a unique name
		wndcls.lpszClassName = (LPTSTR)(LPCTSTR) m_className;

		//wndcls.hCursor = 0;
		wndcls.hInstance = NULL;
		VERIFY ( ::RegisterClass (&wndcls) );
	}
	cs.lpszClass = (LPTSTR)(LPCTSTR) m_className;

	return ret;
}

// ------------------------------------------------------------------------
void THierDrawLbx::OnKillFocus(CWnd* pNewWnd)
{
	//CListCtrl::OnKillFocus(pNewWnd);

	this->RepaintSelected ();

	SendMessage (WMU_CUSTOM_MOUSEUP);
}

///////////////////////////////////////////////////////////////////////////
// this happens when the Right Mouse button is released
void THierDrawLbx::OnContextMenu(CWnd* pWnd, CPoint ptScreenCoords)
{
	if (-1 == ptScreenCoords.x && -1 == ptScreenCoords.y)
	{
		// Summoned with Shift-F10. Anchor at top-left.
		ptScreenCoords.x = ptScreenCoords.y = 2;
		ClientToScreen ( &ptScreenCoords );
	}

	CMenu * pTrackMenu = m_popUpMenu.GetSubMenu ( 0 );

	// inherited from TitleView
	m_parentThreadView->UpdateContextMenu ( pTrackMenu );

	ProbeCmdUI (this, pTrackMenu);
	pTrackMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		ptScreenCoords.x, ptScreenCoords.y, this);
}

///////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnViewBinary ()
{
	CString dir;
	DecodeArticleCallbackData sData(TRUE, NULL, dir);

	m_parentThreadView->ApplyToSelectedNodes (NormalDecodeCallback,
		FALSE /* fRepaint */,
		(DWORD) &sData);
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnArticleBozo()
{
	m_parentThreadView->ApplyToArticles (BozoCallback);
	BozoCallbackCommit ();
}

/////////////////////////////////////////////////////////////////////////////
// menu-items from context menu come here, I suppose
void THierDrawLbx::OnDecode()
{
	// we're friends so I can access this function
	m_parentThreadView->OnDecode ();
}

/////////////////////////////////////////////////////////////////////////////
// menu-items from context menu come here, I suppose
void THierDrawLbx::OnDecodeTo()
{
	// we're friends so I can access this function
	m_parentThreadView->OnDecodeTo ();
}

void THierDrawLbx::OnManualDecode()
{
	TManualDecode sManualDecodeDlg;
	gpsManualDecodeDlg = &sManualDecodeDlg;
	m_parentThreadView->ApplyToArticles (QueueArticleForManualDecoding);
	sManualDecodeDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::NodeExpand(BOOL fExpand)
{
	int i;

	// walk backwards
	for (i = GetCount() - 1; i >= 0; --i)
	{
		if (fExpand)
			OpenBranch( i );
		else
			CloseBranch( i );
	}
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnAddToWatch ()
{
	ThreadAction (WATCH_ACTION);
}

/////////////////////////////////////////////////////////////////////////////
void THierDrawLbx::OnAddToIgnore ()
{
	ThreadAction (IGNORE_ACTION);
}

/////////////////////////////////////////////////////////////////////////////
static void AddArticle (TArticleHeader *pHeader, void *pVoid)
{
	CPtrList *pHeaders = (CPtrList *) pVoid;
	if (!pHeaders->Find (pHeader))
		pHeaders->AddHead (pHeader);
}

/////////////////////////////////////////////////////////////////////////////
static int AddThreadHeaders (TArticleHeader *pHeader, int iSelected,
							 TNewsGroup *pNG, DWORD dwData)
{
	if (!iSelected)
		return 0;

	TThread *pThread;
	if (!pNG->FindThread (pHeader, pThread))
	{
		ASSERT (0);
		return 0;
	}

	pThread->OperateOnHeaders (AddArticle, (void *) dwData);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static BOOL gbPositive;    // looking for presence or absence of icon?
static BOOL gbFoundIcon;
static int TestIcon (TArticleHeader *pHeader, int iSelected, TNewsGroup *pNG,
					 DWORD dwData)
{
	if (!iSelected)
		return 0;

	ThreadActionType iTestFor = (ThreadActionType) dwData;

	BOOL bOn = pNG->IsStatusBitOn ( pHeader->GetArticleNumber (),
		iTestFor == WATCH_ACTION
		? TStatusUnit::kWatch
		: TStatusUnit::kIgnore);

	if ((gbPositive && bOn) || (!gbPositive && !bOn))
	{
		gbFoundIcon = TRUE;
		return 1;   // don't continue
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// spawned off.  called from ThreadAction
//
static int fnRemoveWatchIgnoreIcon (
									CPtrList &           sHeaders,
									BOOL                 bWatch,
									TNewsGroup *         pNG,
									TThreadActionList *  pThreadActionList )
{
	// for each article, remove its watch/ignore icon and remove its
	//   thread from the watch/ignore list

	POSITION pos = sHeaders.GetHeadPosition ();
	while (pos)
	{
		TArticleHeader *pHeader = (TArticleHeader *) sHeaders.GetNext (pos);

		pNG->StatusBitSet (pHeader->GetArticleNumber (),
			bWatch
			? TStatusUnit::kWatch
			: TStatusUnit::kIgnore,
			FALSE /* fOn */);

		pThreadActionList->Remove (pNG, pHeader);
	}

	// no Agent like navigation needed

	return 0;
}  // fnRemoveWatchIgnoreIcon

/////////////////////////////////////////////////////////////////////////////
// Marwan wrote this function.  It is protected.
//
//
// Navigation note:
//  even if Agent is configured to use MoveToNextUnread after Ignore
//     the lbx selection does not move if you are toggling Ignore off
//
//
void THierDrawLbx::ThreadAction (ThreadActionType iAction)
{
	TServerCountedPtr cpNewsServer;

	BOOL bWatch = (iAction == WATCH_ACTION);

	// if any selected headers have the opposite icon displayed,
	//   pop up an error message

	gbFoundIcon = FALSE;
	gbPositive  = TRUE;
	m_parentThreadView->ApplyToArticles (
		TestIcon,
		FALSE,  // repaint
		DWORD(bWatch ? IGNORE_ACTION : WATCH_ACTION));
	if (gbFoundIcon)
	{
		MsgResource (bWatch ? IDS_NO_WATCH_ON_IGNORED : IDS_NO_IGNORE_ON_WATCHED,
			this);
		return;
	}

	TThreadActionList *pList =
		(bWatch ? &gpStore->GetWatchList () : &gpStore->GetIgnoreList ());

	// make a list of headers that are in the thread of any selected article
	CPtrList sHeaders;
	m_parentThreadView->ApplyToArticles (
		AddThreadHeaders,
		FALSE,  // repaint
		(DWORD) &sHeaders);

	LONG gid = GetNewsView ()->GetCurNewsGroupID ();
	BOOL fUseLock;
	TNewsGroup *pNG = 0;
	TNewsGroupUseLock useLock (cpNewsServer, gid, &fUseLock, pNG);
	if (!fUseLock)
	{
		ASSERT (0);
		return;
	}

	// if all selected articles are already marked with the watch/ignore icon,
	// user wants to remove them from the watch/ignore list

	gbPositive  = FALSE;           // looking for absence of icon
	gbFoundIcon = FALSE;
	m_parentThreadView->ApplyToArticles (
		TestIcon,
		FALSE,  // repaint
		DWORD(bWatch ? WATCH_ACTION : IGNORE_ACTION));
	BOOL bAdd = gbFoundIcon;

	if (!bAdd)
	{
		fnRemoveWatchIgnoreIcon (sHeaders, bWatch, pNG, pList);
		return;
	}

	// --- From here down we are Adding the watch or ignore icon  ---
	ASSERT(bAdd);

	// add articles to watch/ignore list
	POSITION pos = sHeaders.GetHeadPosition ();
	while (pos)
	{
		TArticleHeader *pHeader = (TArticleHeader *) sHeaders.GetNext (pos);
		pList->Add (pNG, pHeader);
	}

	// run rule manually on the headers
	CString strRule;
	strRule.LoadString (bWatch ? IDS_WATCH_RULE_NAME : IDS_IGNORE_RULE_NAME);
	Rule *psRule = GetRule (strRule);
	ASSERT (psRule);
	int iRuleID = psRule->GetID ();

	LONG lGroupID = GetNewsView ()->GetCurNewsGroupID ();

	// refresh the thread pane only in the case of ignore

	TManualRule sDlg (this,
		lGroupID,
		FALSE,       /* bSelGroups */
		iRuleID,
		TRUE,        /* bRunStraightThrough */
		FALSE,       /* bShowDialog */
		FALSE,       /* bAllArticles */
		FALSE,       /* bRefreshWhenDone */
		&sHeaders);

	if (true)
	{
		// pause redrawing..
		TLockDraw sLockDraw (m_parentThreadView,  TRUE);

		// perform the ignore or watch
		sDlg.DoModal ();
	}

	int   iThrdOff, iThrdOn, iSortOff, iSortOn;

	if (bWatch)
	{
		gpGlobalOptions->GetRegUI()->GetNavigWatch (iThrdOff, iThrdOn, iSortOff, iSortOn);
	}
	else
	{
		gpGlobalOptions->GetRegUI()->GetNavigIgnore (iThrdOff, iThrdOn, iSortOff, iSortOn);
	}

	// before we do the refresh, figure out where we should land
	bool fViewArticle = false;
	EJmpQuery eQuery = fnNavigate_GetQuery (iThrdOff, iThrdOn, iSortOff, iSortOn, &fViewArticle);

	int iOutGID = 0;
	int iOutArtNum = 0;

	int nQueryRet = m_parentThreadView->QueryToEngine ( eQuery,
		false,
		iOutGID,
		iOutArtNum );

	int *piSelected = 0;
	int iNumSelected = m_parentThreadView->GetSelCount();
	int iTopIndex = m_parentThreadView->GetTopIndex ();

	if (-1 == nQueryRet)
	{
		piSelected = new int[iNumSelected];
		if (piSelected)
			m_parentThreadView->GetSelItems (iNumSelected, piSelected);
	}

	if (!bWatch)
	{
		// ok, now do the refresh
		sDlg.DoRefreshNewsgroup ();
	}

	// go to the landing area

	switch (nQueryRet)
	{
	case -1:
		{
			// since there is not landing candidate, stay where we are, more or less,
			//  given that we have moved everything w the Refresh
			int iLen = m_parentThreadView->GetCount();
			for (int i = 0; i < iNumSelected; i++)
				if (i < iLen)
					m_parentThreadView->SetSel (piSelected[i]);

			m_parentThreadView->SetTopIndex (iTopIndex);   // ?? questionable
		}
		break;

	case 0:
		m_parentThreadView->SelectAndShowArticle ( iOutArtNum, fViewArticle );
		break;

	case 1:
		m_parentThreadView->JumpSecondPass (  GetNewsView (),
			eQuery,
			iOutGID,
			fViewArticle,
			false );  // fLimitToPile
		break;
	}

	delete [] piSelected;
} // end ThreadAction

// ------------------------------------------------------------------------
// pSelInfo - info to help maintain the selection. x indicates the
//            current selection. If we are deleting a listbox line that
//            is positioned above 'x', then decrement 'x';
//
void THierDrawLbx::RemoveThread (BOOL fTreeDisplay, TThread* pThread,
								 int i, int* piSelIdx)
{
	try
	{
		if (fTreeDisplay)
		{
			CloseBranch (i, piSelIdx);

			selection_delete_string (i, piSelIdx);
		}
		else // flat-mode
		{
			TArtNode* pNode;

			// we don't have 'i'
			//pNode = (TArtNode*) GetItemDataPtr(i);
			//CloseItem (pNode->GetArticleNumber());

			TIntVector sVec;
			pThread->CollectIds (&sVec);

			for (int k = GetCount() - 1; k >= 0; --k)
			{
				pNode = GetpNode(k);
				if (pNode && sVec.Have(pNode->GetArticleNumber()))
				{
					selection_delete_string (k, piSelIdx);
				}
			}
		}
	}
	catch(...)
	{
		// catch all
	}
}

// ------------------------------------------------------------------------
// figure out where selection should be after deleting the string
int THierDrawLbx::selection_delete_string (int i, int* piSelIdx)
{
	int iTot = GetCount();

	VERIFY(CListCtrl::DeleteItem (i));

	if (piSelIdx)
	{
		if (i > *piSelIdx)   // selection doesn't change
		{
			// do nothing
			;
		}
		else if (i < *piSelIdx)
			--(*piSelIdx);   //  selection shift posn up

		else // (i == *piSelIdx)
		{
			if (i == iTot - 1)
				--(*piSelIdx);   //  selection shift posn up, forced
		}
	}

	return (iTot - 1);   // return new count
}

// -------------------------------------------------------------------------
// OnFilePrint -- gets called by the framework when the File->Print menu
// option is selected
void THierDrawLbx::OnPrint ()
{
	static PRINTDLG gpd;          // used whenever File->Print is called
	static BOOL bInitialized;
	if (!bInitialized) {
		bInitialized = TRUE;
		memset (&gpd, 0, sizeof(PRINTDLG));
		gpd.lStructSize = sizeof (PRINTDLG);
		gpd.hwndOwner = AfxGetMainWnd ()->GetSafeHwnd ();
		gpd.nCopies = 1;
	}

	gpd.Flags = PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
	if (!PrintDlg (&gpd))
		return;     // probably user hit escape

	m_parentThreadView->ApplyToArticles (QueueArticleForPrinting,
		FALSE /* fRepaint */, (DWORD) gpd.hDC /* cookie */);
}

// -------------------------------------------------------------------------
void THierDrawLbx::OnPrintSetup()
{
	CPrintSetup sDlg;
	sDlg.DoModal ();
}

// -------------------------------------------------------------------------
void THierDrawLbx::OnMButtonDown(UINT nFlags, CPoint point)
{
	((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame()->SendMessage ( WMU_MBUTTON_DOWN );

	CListCtrl::OnMButtonDown(nFlags, point);
}

// -------------------------------------------------------------------------
void THierDrawLbx::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CListCtrl::OnMButtonDblClk(nFlags, point);
}

// -------------------------------------------------------------------------
BOOL THierDrawLbx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// let the mdi-child decide which pane will handle this
	CWnd* pMdiChild = ((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame();
	if (!pMdiChild)
		return FALSE;

	if (((TNews3MDIChildWnd*) pMdiChild)->handleMouseWheel(nFlags, zDelta, pt))
		return TRUE;
	else
		return handleMouseWheel (nFlags, zDelta, pt);
}

// -------------------------------------------------------------------------
BOOL THierDrawLbx::handleMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	return CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

// -------------------------------------------------------------------------
int  THierDrawLbx::GetSelItems (int max, int * pIdx) const
{
	POSITION pos = this->GetFirstSelectedItemPosition ();

	if (NULL == pos)
		return 0;      // no items selected

	int i = 0;
	while (pos && (i < max))
	{
		int idx = this->GetNextSelectedItem (pos);
		pIdx[i++] = idx;
	}
	return i;
}

// -------------------------------------------------------------------------
int  THierDrawLbx::GetFocusIndex ()
{
	int n = GetNextItem (-1, LVNI_FOCUSED);

	if (-1 == n) return LB_ERR;
	return n;
}

int THierDrawLbx::RepaintSelected ()
{
	POSITION pos = this->GetFirstSelectedItemPosition ();

	if (NULL == pos)
		return 0;      // no items selected

	while (pos)
	{
		int idx = this->GetNextSelectedItem (pos);

		RECT rct;
		GetItemRect (idx, &rct,LVIR_BOUNDS);
		InvalidateRect (&rct);
	}

	return 0;
}

// -------------------------------------------------------------------------
int THierDrawLbx::EnsureVisibleSelection ( )
{
	int n = GetFirstSelection();

	if (LB_ERR == n) return 0;

	EnsureVisible (n, FALSE);
	return 0;
}

// -------------------------------------------------------------------------
int  THierDrawLbx::SetSel (int idx, BOOL fSelected)
{
	if (fSelected)
		SetItemState (idx, LVIS_SELECTED, LVIS_SELECTED);
	else
		SetItemState (idx, 0,             LVIS_SELECTED);
	return 0;
}

// -------------------------------------------------------------------------
BOOL THierDrawLbx::GetSel (int idx)
{
	if (LVIS_SELECTED == GetItemState (idx, LVIS_SELECTED))
		return TRUE;
	else
		return FALSE;
}

int  THierDrawLbx::DeleteString (int idx)
{

	CListCtrl::DeleteItem (idx);
	return 0;
}

void  THierDrawLbx::SetTopIndex (int idx)
{
	int estCount = GetCountPerPage ();

	if (estCount < 5)
	{
		EnsureVisible (idx, FALSE);
		return;
	}

	int n = max (idx + estCount -1, GetItemCount() - 1);

	EnsureVisible (n, TRUE);
}

void  THierDrawLbx::SetItemDataPtr (int idx, void * pData)
{
	LVITEM lv;

	lv.mask = LVIF_PARAM;
	lv.iItem = idx;
	lv.iSubItem = 0;
	lv.lParam = LPARAM(pData);
	SetItem ( & lv );
}

// ------------------------------------------------------------------------
int  THierDrawLbx::getColumnCount ()
{
	// calc the number of columns
	int       iTotCols = 0;
	LV_COLUMN sColumn;

	sColumn.mask = LVCF_WIDTH;

	while (GetColumn (iTotCols, &sColumn))
	{
		//TRACE ("iColumn %d is %d wide\n", iTotCols, sColumn.cx );
		iTotCols++;
	}
	return iTotCols;
}

// ------------------------------------------------------------------------
// returns 0 and above for success
//         -1 for error
//
int THierDrawLbx::getNotifyIndex (NMHDR* pNMHDR)
{
	if (setupVersionInfo())
		return -1;

	if (m_iDLLVersionKey >= 4071)
	{
		NMLISTVIEW * pNMLV = (NMLISTVIEW*) pNMHDR;

		return pNMLV->iItem;
	}
	else
		return -1;
}

// ------------------------------------------------------------------------
// returns 0 for success
int THierDrawLbx::setupVersionInfo ()
{
	TCHAR     rcFileName[MAX_PATH];
	int       iKey;
	LPVOID    pInfo = 0;
	int       iStatus = 1;
	HINSTANCE hLib;
	HMODULE   hMod;

	if (m_iDLLVersionKey)
		return 0;

	hLib = LoadLibrary ("comctl32.dll");
	if (NULL == hLib)
	{
		//chMB("load lib failed");
		return 1;
	}

	do
	{
		hMod = GetModuleHandle ("comctl32");
		if (NULL == hMod)
		{
			//chMB("get mod handle failed");
			break;
		}

		GetModuleFileName (hMod, rcFileName, MAX_PATH);

		DWORD dwDummy;
		int   iSize = GetFileVersionInfoSize (rcFileName, &dwDummy);
		if (!iSize)
		{
			//chMB("Get Info size failed");
			break;
		}

		pInfo = malloc (iSize);
		VS_FIXEDFILEINFO * pFixed = 0;
		UINT   valueLen = 0;
		GetFileVersionInfo (rcFileName, 0, iSize, pInfo);

		// get VS_FIXEDFILEINFO
		if (FALSE == VerQueryValue (pInfo, _T("\\"), (PVOID*)&pFixed, &valueLen) || NULL == pFixed)
		{
			//chMB("Ver Query failed");
			break;
		}
		iKey =   1000 * HIWORD(pFixed->dwProductVersionMS) +
			LOWORD(pFixed->dwProductVersionMS) ;

		m_iDLLVersionKey = iKey;

		iStatus = 0;  // success

		// we want a version later than 4.70  ==  4070

	} while (FALSE);

	if (pInfo)
		free (pInfo);
	FreeLibrary (hLib);

	return iStatus;
}

// ------------------------------------------------------------------------
// handles WMU_CUSTOM_MOUSEUP - from mouse hooking
LRESULT THierDrawLbx::OnCustomMouseUp (WPARAM wP, LPARAM lP)
{
	TRACE("  recvd  WMU_CUSTOM_MOUSEUP  %d\n", lP);
	if (m_sDrag.m_fCapture)
	{
		dragselect_timer (false);
		TRACE0(" ........  release capture \n");
		ReleaseCapture ();
		m_sDrag.m_fCapture = false;

		m_sDrag.m_stlCtrlDragSet.clear();
	}
	return 0;
}

// ------------------------------------------------------------------------
// handles WMU_LBUTTONUP - custom msg
LRESULT THierDrawLbx::OnCustomLButtonUp(WPARAM wParam, LPARAM lParam)
{
	// if open-on-single-click is enabled, open... but only if there is no
	// extended selection
	if (gpGlobalOptions->GetRegUI()->GetOneClickArt() && GetSelCount() == 1)
	{
		POINT & point = m_ptLastClick;

		int sel = this->GetFocusIndex ();
		if (sel >= 0)
		{
			TArtNode* pNode = GetpNode (sel);
			if (pNode)
			{
				int leftSide = pNode->m_depth * TREE_WIDTH_OPEN;

				if (point.x > leftSide && point.x < (leftSide + TREE_WIDTH_OPEN))
				{
					// the Mouse Down & Mouse Up is probably on the
					//  special open/close area. Ignore this
				}
				else
				{
					TPersist822Header *p822Hdr = pNode->GetpRandomHeader ();
					if (p822Hdr)
						m_parentThreadView->DisplayArticle (p822Hdr, pNode);
				}
			}
		}
	}
	return 0L;
}

BOOL THierDrawLbx::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	DWORD dwBack;
	{
		if (gpGlobalOptions->GetRegSwitch()->IsDefaultTreeBG())
			dwBack = gSystem.Window();
		else
			dwBack = gpGlobalOptions->GetBackgroundColors()->GetTreeBackground();

		RECT rctClient;
		GetClientRect ( &rctClient );

		CBrush br(dwBack);
		pDC->FillRect(&rctClient, &br);
	}

	return TRUE;
}

void THierDrawLbx::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	m_iWindowCX = cx;	
}

///////////////////////////////////////////////////////////////////////////
// Substitute for LBN_SELCHANGE,
//
void THierDrawLbx::OnItemChanged (NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 1;

	LPNMLISTVIEW pnmlv = (LPNMLISTVIEW) pNMHDR;

	if ((pnmlv->uNewState & LVIS_SELECTED)  &&
		!(pnmlv->uOldState & LVIS_SELECTED))
	{
		if (1 == GetSelectedCount())
		{
			// this will pop unless:
			//    a:  another selchange comes
			//    b:  this turns out to be a right click
			//
			start_selchange_timer ();

			//  OnTimer will call
			//   PostMessage (WMU_SELCHANGE_OPEN);

			// 5-25-96 Rather than calling OpenNewsgroup() directly,
			//  post a message to myself.  That way the processing
			//  can finish before we start the big fat OpenNewsgroup function

			*pResult = 0;
		}
	}
}

// ------------------------------------------------------------------------
// Keep trying to set the Caption on the mdichild
void THierDrawLbx::OnTimer(UINT nIDEvent)
{
	if (THierDrawLbx::idSelchangeTimer == nIDEvent)
	{
		// this is related to 1-click open article
		stop_selchange_timer ();
		//TRACE("SelchangeTimer popped\n");
		PostMessage (WMU_SELCHANGE_OPEN);
	}

	if (THierDrawLbx::idDragTimer == nIDEvent)
	{
		handlePanning ();   // for drag select
		return;
	}

	CListCtrl::OnTimer(nIDEvent);
}

// -------------------------------------------------------------------------
void THierDrawLbx::start_selchange_timer ()
{
	stop_selchange_timer ();

	m_hSelchangeTimer = SetTimer (THierDrawLbx::idSelchangeTimer, 333, NULL);
}

// -------------------------------------------------------------------------
void THierDrawLbx::stop_selchange_timer ()
{
	// kill any timer currently running
	if (m_hSelchangeTimer)
		KillTimer (m_hSelchangeTimer);

	m_hSelchangeTimer = 0;
}

//////////////////////////////////////////////////////////////////////////
// Called from OnLbxSelChange(void)
LRESULT THierDrawLbx::OnSelChangeOpen(WPARAM wParam, LPARAM lParam)
{
	int iSelCount = GetSelCount ();
	int iCaret;

	if (1 == iSelCount)
		this->EnsureVisibleSelection();

	if (iSelCount >= 1 && m_parentThreadView &&
		((iCaret = GetFocusIndex()) >= 0) &&
		GetSel (iCaret))
	{
		TPersist822Header * p822Hdr = Getp_Header (iCaret);
		if (p822Hdr)
		{
			TArticleHeader * pHdr = p822Hdr->CastToArticleHeader ();

			if (pHdr)
				m_parentThreadView->OnHLbxSelChange ( pHdr );
		}
	}

	return 0;
}

// ------------------------------------------------------------------------
// Do chores associated with changing news servers.
void THierDrawLbx::ServerSwitch ()
{
	// we need to adjust height since the Proxicom bitmaps are 20 pixels high.
	//adjust_item_height ();
}

// ------------------------------------------------------------------------
//  handles WM_LBUTTONDOWN
void THierDrawLbx::OnLButtonDown(UINT nFlags, CPoint point)
{
	CListCtrl::OnLButtonDown (nFlags, point);
	TRACE0("LButtonDown\n");

	int tot = GetCount();
	if (0 == tot)
		return;

	int idx = GetFocusIndex();
	if (LB_ERR == idx)
		return;

	if (GetSel (idx))
	{
		m_sDrag.m_fDragPositive = true;
		TRACE0("...LButtonDown  drag positive\n");
	}
	else
	{
		m_sDrag.m_fDragPositive = false;
		TRACE0("...LButtonDown  drag negative\n");
	}

	m_sDrag.m_iStartDragIndex = idx;

	if (true)
	{
		if (nFlags & MK_CONTROL)
		{
			m_sDrag.m_fCtrlKey = true;

			m_sDrag.m_stlCtrlDragSet.clear();
			POSITION pos = GetFirstSelectedItemPosition ();
			while (pos)
				m_sDrag.m_stlCtrlDragSet.insert ( GetNextSelectedItem (pos) );
		}
		else
			m_sDrag.m_fCtrlKey = false;

		TRACE0("   ...  lbutdown -- Set capture \n");
		dragselect_timer (true);
		m_sDrag.m_fCapture = true;
		SetCapture ();
	}
}

// ------------------------------------------------------------------------
//  handles WM_LBUTTONDOWN
void THierDrawLbx::OnLButtonUp(UINT nFlags, CPoint point)
{
	TRACE("recv  standard WM_LBUTTONUP - posting\n");
	++gnMsgCount;

	SendMessage (WMU_CUSTOM_MOUSEUP, 0, gnMsgCount);
}

// ------------------------------------------------------------------------
//  handles WM_MOUSEMOVE
void THierDrawLbx::OnMouseMove(UINT nFlags, CPoint point)
{
	CListCtrl::OnMouseMove(nFlags, point);

	if (m_sDrag.m_fCapture)
	{
		//TRACE2("mouse move %d %d\n",  point.x, point.y);
		if (m_sDrag.m_iStartDragIndex >=0 )
		{

			RECT rct;

			GetClientRect (&rct);

			if (PtInRect (&rct, point))
			{
				// make sure everything between  here and here is selected
				UINT flags = 0;
				int idxNow = HitTest (point,  &flags);

				if ( -1  == idxNow )
					return;

				smartSelect ( idxNow, m_sDrag.m_iStartDragIndex, idxNow );
			}

		}
	}
}

// ------------------------------------------------------------------------
void THierDrawLbx::smartSelect (int blo, int bhi, int current)
{
	int lo, hi;
	if (blo < bhi)      //  purify
	{
		lo = blo;  hi = bhi;
	}
	else
	{
		lo = bhi;  hi = blo;
	}

	if (m_sDrag.m_fDragPositive)
	{
		THeapBitmap   sHB(lo, hi);
		POSITION pos = this->GetFirstSelectedItemPosition ();

		// Phase 1 -- turn a bunch of stuff off  '(except original)
		while (pos)
		{
			int idx = GetNextSelectedItem (pos);

			if (idx < lo  ||  idx > hi)
			{
				STLUniqSet::iterator it = m_sDrag.m_stlCtrlDragSet.find (idx);

				if (it != m_sDrag.m_stlCtrlDragSet.end())
				{
					// Found! -- it's one of the original
				}
				else
					SetItemState (idx, 0, LVIS_SELECTED);
			}
			else
				sHB.SetBit ( idx );  // track the fact it is already selected
		}

		// Phase 2 -- turn stuff on
		for (int i = lo; i <= hi; i++)
		{
			if (FALSE == sHB.IsBitOn (i))
				SetItemState (i, LVIS_SELECTED , LVIS_SELECTED);
		}

		// put focus rect on current item
		SetItemState ( current, LVIS_FOCUSED, LVIS_FOCUSED );
	}
	else
	{
		// Phase 1 -- turn a original stuff on
		STLUniqSet::iterator it = m_sDrag.m_stlCtrlDragSet.begin();
		for (; it != m_sDrag.m_stlCtrlDragSet.end(); it++)
		{
			int idx = (int)  *it;

			if (idx < lo  ||  idx > hi)
			{
				SetItemState (idx, LVIS_SELECTED, LVIS_SELECTED /*mask*/);
			}
			else
				SetItemState (idx, 0,  LVIS_SELECTED);   // turn off
		}

		// Phase 2 -- turn  our new set off

		for (int i = lo; i <= hi; i++)
		{
			SetItemState (i, 0, LVIS_SELECTED);
		}

	}
}

// ------------------------------------------------------------------------
void THierDrawLbx::dragselect_timer (bool fTurnOn)
{
	if (fTurnOn)   // 55 msecs just feels right to AMC
		m_hDragTimer = SetTimer (THierDrawLbx::idDragTimer, 55, NULL);
	else
	{
		if (m_hDragTimer )
		{
			//TRACE("KillTimer\n");
			KillTimer (m_hDragTimer);
			m_hDragTimer = 0;
		}
	}
}

// ------------------------------------------------------------------------
// handle drag-Select  and autoScroll
void THierDrawLbx::handlePanning ()
{
	if (false == m_sDrag.m_fCapture)
		return;

	CPoint  pt = getMessagePosition ();

	RECT rct;
	GetClientRect ( &rct );
	int nVert=0;

	if ((rct.left <= pt.x) && (pt.x <= rct.right))
	{
		if ( pt.y < rct.top )
			nVert = 1;   // above

		else if (pt.y > rct.bottom)
			nVert = 2;   // below
	}

	int tot = GetCount();

	if (0 == nVert  ||  0 == tot)
		return;

	int idxTop = GetTopIndex();

	if (1 == nVert)
	{
		if (--idxTop >= 0)
		{
			EnsureVisible (idxTop, TRUE);   // partial is OK
			smartSelect (m_sDrag.m_iStartDragIndex, idxTop, idxTop);
		}
	}
	else
	{  // we are Below
		if (idxTop < tot)
		{
			int last_visible_index = idxTop + GetCountPerPage();

			if (last_visible_index >= tot)
				last_visible_index = tot - 1;

			EnsureVisible (last_visible_index, TRUE );   // partial is OK
			smartSelect (m_sDrag.m_iStartDragIndex, last_visible_index, last_visible_index);
		}
	}
}

// ------------------------------------------------------------------------
// Factored out
POINT THierDrawLbx::getMessagePosition()
{
	POINT pt;

	DWORD  dwPos = GetMessagePos();
	POINTS  px  =  MAKEPOINTS(dwPos);
	pt.x = px.x;
	pt.y = px.y;

	this->ScreenToClient ( &pt );
	return pt;
}

// ------------------------------------------------------------------------
// Factored out
//static LPCTSTR ThousandsSeparator (LPCTSTR rcLines)
//{
//	static TCHAR result[60];
//	static TCHAR rcSep[4];
//	static bool init = false;
//	if (!init)
//	{
//		ZeroMemory(rcSep, sizeof(rcSep));
//		GetLocaleInfo (LOCALE_SYSTEM_DEFAULT,
//			LOCALE_STHOUSAND,
//			rcSep,
//			sizeof(rcSep));
//		init = true;
//	}
//	int len = _tcslen (rcLines);
//	int m = len % 3;
//	LPTSTR dst = result;
//	LPCTSTR src = rcLines;
//	while (--m >= 0)
//	{
//		*dst++ = *src++;
//		len --;
//	}
//	if ((dst != result) && (len >= 3))
//	{
//		*dst++ = rcSep[0];
//	}
//
//	while (*src)
//	{
//		*dst++ = *src++;
//		*dst++ = *src++;
//		*dst++ = *src++;
//		len -= 3;
//		if (len >= 3)
//			*dst++ = rcSep[0];
//	}
//	*dst = 0;
//	return result;
//}
