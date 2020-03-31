/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TDraftsDlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 13:16:09  richard_wood
/*  Source formatting changes
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:08  richard_wood
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

// OutbxDlg.cpp : implementation file

#include "stdafx.h"
#include "news.h"
#include "tdraftsdlg.h"
#include "pobject.h"
#include "names.h"
#include "article.h"
#include "globals.h"
#include "fileutil.h"
#include "article.h"
#include "article.h"
#include "tglobopt.h"
#include "arttext.h"
#include "rtfspt.h"
#include "utilsize.h"
#include "artdisp.h"
#include "fetchart.h"
#include "ngutil.h"
#include "fileutil.h"
#include "custmsg.h"
#include "newsdoc.h"
#include "rgui.h"                // SaveUtilDlg(), ...
#include "posttmpl.h"            // TPostTemplate
#include "autodrw.h"             // TAutoDraw
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "newsdb.h"              // THeaderIterator
#include "genutil.h"             // DecomposeSizePos(), ...
#include "rgswit.h"              // TRegSwitch

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
TDraftsDlg::TDraftsDlg(CWnd* pParent /*=NULL*/)
: CDialog(TDraftsDlg::IDD, pParent)
{
	LONG lBase = GetDialogBaseUnits();
	m_xMargin = MulDiv(7,LOWORD(lBase),4);
	m_yMargin = MulDiv(7,HIWORD(lBase),8);
	m_iDisplayedArticle = 0;
	m_fInitialLoad = FALSE;
}

// ------------------------------------------------------------------------
void TDraftsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_OUTBOX_DELETE, m_sDelete);
	DDX_Control(pDX, IDC_OUTBOX_EDIT, m_sEdit);
	DDX_Control(pDX, IDC_OUTBOX_LISTCTRL, m_listCtrl);
}

// ------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TDraftsDlg, CDialog)
	ON_BN_CLICKED(IDC_OUTBOX_DELETE, OnOutboxDelete)
	ON_NOTIFY(LVN_DELETEITEM, IDC_OUTBOX_LISTCTRL, OnDeleteitemOutboxListctrl)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_OUTBOX_LISTCTRL, OnDblclkOutboxListctrl)
	ON_MESSAGE (WMU_REFRESH_OUTBOX, RefreshDraftDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_OUTBOX_LISTCTRL, OnItemChangeListctrl)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_OUTBOX_LISTCTRL, OnItemGetDisplayInfo)
	ON_BN_CLICKED(IDC_OUTBOX_EDIT, OnOutboxEdit)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// ------------------------------------------------------------------------
void TDraftsDlg::MakeColumns()
{
	// create the columns in the list control...
	RECT  rct;
	m_listCtrl.GetClientRect (&rct);

	CString temp;
	temp.LoadString (IDS_REPLY_TO);
	m_listCtrl.InsertColumn (0, temp, LVCFMT_LEFT, rct.right*3 / 10, 1);
	temp.LoadString (IDS_HDRCTRL_SUBJECT);
	m_listCtrl.InsertColumn (1, temp, LVCFMT_LEFT, rct.right*3 / 10, 2);
	temp.LoadString (IDS_HDRCTRL_CREATED);
	m_listCtrl.InsertColumn (2, temp, LVCFMT_LEFT, rct.right*2 / 10, 3);
	temp.LoadString (IDS_HDRCTRL_SENT);
	m_listCtrl.InsertColumn (3, temp, LVCFMT_LEFT, rct.right*2 / 10, 4);
}

// ------------------------------------------------------------------------
BOOL TDraftsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pServer = GetCountedActiveServer ();
	m_pServer->AddRef ();

	// open the outbox and pin it...

	// make sure the outbox is open and pinned...
	m_pDrafts = m_pServer->GetDraftMsgs ();
	m_pDrafts->Open ();

	// Owner draw code doesn't handle images...
	// m_listCtrl.SubclassDlgItem (IDC_OUTBOX_LISTCTRL, this);

	// grab image list
	m_imageList.Create (IDB_OUTBOX, 16, 1, RGB(192,192,192));
	m_listCtrl.SetImageList (&m_imageList, LVSIL_SMALL);

	// set up the columns
	MakeColumns();

	// set up the RTF
	CWnd * pRich = GetRichEdit();
	if (pRich)
	{
		m_wndRich.SubclassWindow (pRich->GetSafeHwnd());
		// pleas send WM_NOTIFY:EN_SELCHANGE to the parent
		m_wndRich.SendMessage ( EM_SETEVENTMASK, 0, ENM_SELCHANGE);

		// auto vscroll (duh)
		m_wndRich.SendMessage ( EM_SETOPTIONS, ECOOP_OR,
			ECO_AUTOVSCROLL | ECO_READONLY);

		SetInitialFont(pRich);
	}

	UpdateData (FALSE /* bSaveAndValidate */);

	// set dialog pos
	CString strSizePos = gpGlobalOptions -> GetRegUI () -> GetOutboxSizePos ();
	int dx, dy, x, y;
	if (!DecomposeSizePos (strSizePos, dx, dy, x, y))
		SetWindowPos (NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// set dialog size and header widths
	int iX, iY;
	BOOL bMax, bMin;
	TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
	if (!pRegUI -> LoadUtilDlg ("Outbox", iX, iY, bMax, bMin))
	{
		SetWindowPos (NULL, 0, 0, iX, iY, SWP_NOZORDER | SWP_NOMOVE);
		if (bMax)
			ShowWindow (SW_SHOWMAXIMIZED);
	}

	int riSizes [4];
	if (!pRegUI -> LoadUtilHeaders ("Outbox", riSizes,
		sizeof riSizes / sizeof (int)))
		for (int i = 0; i < 4; i++)
			m_listCtrl.SetColumnWidth (i, riSizes [i]);

	// fill the list view
	LoadOutbox ();

	// set up notification

	// setup the correct icon
	//m_hOutboxIcon = AfxGetApp() -> LoadIcon (IDR_OUTBOX);
	//m_hOldIcon = SendMessage(WM_SETICON, TRUE, (LPARAM)m_hOutboxIcon);

	//m_hTimer = SetTimer (100, 600, NULL);
	m_hTimer = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
}

// ------------------------------------------------------------------------
void TDraftsDlg::RemoveOutboxItem (int artInt, int fArticle)
{
	m_pDrafts->WriteLock ();
	m_pDrafts->PurgeHeader (artInt);
	m_pDrafts->PurgeBody (artInt);
	m_pDrafts->UnlockWrite ();
	(*m_pDrafts)->RemoveItem (artInt);
}

// ------------------------------------------------------------------------
// DeleteOutboxMessage -- call FinishDeleting() after done deleting one or more
// outbox messages
void TDraftsDlg::DeleteOutboxMessage (int iIndex,
									  BOOL bSelectAnother /* = TRUE */, BOOL bSetFocus /* = TRUE */)
{
	TDraftDlgStatus *pStatus = GetpStatus (iIndex);
	int iArtInt = pStatus -> m_articleInt;

	// if this article is currently displayed, clear the display
	if (iArtInt == m_iDisplayedArticle)
		GetRichEdit () -> SendMessage (WM_SETTEXT, 0, (LPARAM) "");

	// ????? warnCBX thing...
	RemoveOutboxItem (iArtInt, pStatus -> IsArticle ());
	m_listCtrl.DeleteItem (iIndex);

	int newCount = m_listCtrl.GetItemCount();
	if (bSelectAnother && newCount)
	{
		int iNewSel = (newCount > iIndex ? iIndex : iIndex - 1);
		m_listCtrl.SetItemState (iNewSel, LVIS_SELECTED, LVIS_SELECTED);
		if (bSetFocus)
			m_listCtrl.SetFocus ();
	}
}

// ------------------------------------------------------------------------
// FinishDeleting -- called after calling DeleteOutboxMessage() one or more
// times
void TDraftsDlg::FinishDeleting ()
{
	// save the outbox
	m_pDrafts->SaveHeaders ();
	m_pDrafts->SaveBodyMetaData ();
	m_pServer->SaveDrafts ();
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnOutboxDelete()
{
	// turn off redraw
	TAutoDraw sDraw(&m_listCtrl, TRUE);

	// delete all selected outbox messages
	int iSelected = NumSelected ();
	for (int i = 0; i < iSelected; i++)
	{
		int iIndex = GetFirstSelectedIndex ();

		ASSERT (iIndex != -1);
		if (iIndex == -1)
			break;

		DeleteOutboxMessage (iIndex, FALSE /* bSelectAnother */);
	}
	FinishDeleting ();
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnDeleteitemOutboxListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	delete GetpStatus(pNMListView->iItem);
	*pResult = 0;
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnClose()
{
	if (!UpdateData ())
		return;  // DDV error

	// save window size and header widths
	TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
	int riSizes [4];
	for (int i = 0; i < 4; i++)
		riSizes [i] = m_listCtrl.GetColumnWidth (i);
	pRegUI -> SaveUtilHeaders ("Outbox", riSizes, sizeof riSizes / sizeof (int));

	BOOL bZoomed = IsZoomed ();
	BOOL bIconic = IsIconic ();

	WINDOWPLACEMENT wp;
	RECT & rct = wp.rcNormalPosition;

	GetWindowPlacement (&wp);

	// save window width and state
	pRegUI -> SaveUtilDlg ("Outbox", (int) (rct.right - rct.left),
		(int) (rct.bottom - rct.top), bZoomed, bIconic);

	// save window pos
	CString strSizePos = ComposeSizePos (0, 0, rct.left, rct.top);
	gpGlobalOptions -> GetRegUI () -> SetOutboxSizePos (strSizePos);

	// close outbox object
	m_pDrafts->Close ();

	// subtract reference count
	m_pServer->Release ();
	m_pServer = 0;

	//SendMessage(WM_SETICON, TRUE, (LPARAM)m_hOldIcon);

	if (m_hTimer)
		KillTimer (m_hTimer);

	DestroyWindow();
	CDialog::OnClose();
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnCancel()
{
	OnClose();
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnOK()
{
	CDialog::OnOK();
	OnClose();
}

// ------------------------------------------------------------------------
int  TDraftsDlg::GetItemIndex (TPersist822Header *pItem)
{
	int               index;
	int               numItems = m_listCtrl.GetItemCount();
	TDraftDlgStatus  *pCurrStat;

	if (numItems == 0)
		return 0;
	else
	{
		for (index = 0; index < numItems; index++)
		{
			pCurrStat = GetpStatus(index);
			if (pItem->GetTime() > pCurrStat->m_timeRead)
				break;
		}
	}

	return index;
}

// ------------------------------------------------------------------------
int CALLBACK  DraftCompareFunc (LPARAM param1, LPARAM param2, LPARAM cookie)
{
	TDraftDlgStatus * pStatus1 = (TDraftDlgStatus *) (void*)param1;
	TDraftDlgStatus * pStatus2 = (TDraftDlgStatus *) (void*)param2;

	if (pStatus1->m_timeRead  < pStatus2->m_timeRead)
		return 1;

	if (pStatus1->m_timeRead > pStatus2->m_timeRead)
		return -1;

	// use artInt as tie breaker

	if (pStatus1->m_articleInt < pStatus2->m_articleInt)
		return 1;

	if (pStatus1->m_articleInt > pStatus2->m_articleInt)
		return -1;

	return 0;
}

// ------------------------------------------------------------------------
void TDraftsDlg::InsertOutboxItem (TPersist822Header *pItem)
{
	// binary search for created date
	int               index = 0;

	// get either the newsgroups or To: field recipients...
	TStringList recipients;
	CString  temp;
	if (pItem->IsArticle())
	{
		pItem->CastToArticleHeader()->GetDestList( &recipients );
		recipients.CommaList (temp);
	}
	else
	{
		CString cc, bcc;
		TEmailHeader * pEmailHdr = pItem->CastToEmailHeader();
		pEmailHdr->GetToText ( temp );
		pEmailHdr->GetCCText ( cc );
		pEmailHdr->GetBCCText ( bcc );

		// add in CC text
		if (!cc.IsEmpty())
		{
			if (temp.IsEmpty())
				temp = cc;
			else
				temp += ", " + cc;
		}
		if (!bcc.IsEmpty())
		{
			if (temp.IsEmpty())
				temp = bcc;
			else
				temp += ", " + bcc;
		}
	}
	int idxAt;

	StatusUnitI stat;

	(*m_pDrafts)->GetStatus (pItem->GetNumber(), &stat);

	// the correct icon is determined via a WM_NOTIFY callback
	idxAt = m_listCtrl.InsertItem (index, temp, I_IMAGECALLBACK);

	// To:
	m_listCtrl.SetItemText (idxAt, 1, pItem->IsArticle()
		? pItem->CastToArticleHeader()->GetSubject()
		: pItem->CastToEmailHeader()->GetSubject());

	CString str; str.LoadString (IDS_OUTBOX_TIME_FORMAT);
	CString var = (pItem->GetTime()).Format(str);
	m_listCtrl.SetItemText (idxAt, 2, var);  // subject

	// the status timeRead member contains the time sent for
	// outbox items.  If it has been sent, put it in,
	// otherwise leave the column blank

	TDraftDlgStatus  *pStat = new TDraftDlgStatus;

	// show the time the message went out, if any
	if (stat.m_timeRead)
		pStat->m_byFlags |= OUTBOX_FLAG_SENT;
	set_senttime (idxAt, stat.m_timeRead);

	pStat->m_articleInt = pItem->GetNumber();
	if (pItem->IsArticle())
	{
		pStat->m_byFlags |= OUTBOX_FLAG_ARTICLE;

		TArticleHeader* pAH = pItem->CastToArticleHeader();
		// if the "Control" line is here, it's a Cancel message
		if (0 == _tcslen(pAH->GetControl()))
		{
			if (!recipients.IsEmpty())
			{
				pStat->m_pCancelMsgID = new CString(pAH->GetMessageID());
				pStat->m_pCancelSubj  = new CString(pAH->GetSubject());
				// where do we post the death message? use first one
				pStat->m_pCancelNG    = new CString(recipients.GetHead());
			}
		}
		else
			pStat->m_byFlags |= OUTBOX_FLAG_CANCELMSG;
	}

	pStat->m_timeRead = (pItem->GetTime()).GetTime();

	m_listCtrl.SetItemData (idxAt, (DWORD) pStat);
}

// ------------------------------------------------------------------------
void TDraftsDlg::LoadOutbox()
{
	DWORD dwTickCount = GetTickCount();
	m_fInitialLoad = TRUE;
	THeaderIterator it (m_pDrafts);

	TPersist822Header *pHeader;

	// turn off redraw
	TAutoDraw sDraw(&m_listCtrl, TRUE);

	int count = 0;
	while (it.Next (pHeader))
	{
		count++;
		if (pHeader->IsArticle ())
			InsertOutboxItem (pHeader->CastToArticleHeader ());
		else
			InsertOutboxItem (pHeader->CastToEmailHeader());
	}

	// should sort by date descending...
	m_listCtrl.SortItems (DraftCompareFunc, NULL);

	TRACE ("outbox items : %d, average milliseconds per item : %u\n",
		count,
		count ? (GetTickCount() - dwTickCount)/count : 0);

	// enable/disable the buttons
	UpdateGrayState ();
	m_fInitialLoad = FALSE;
}

// ------------------------------------------------------------------------
void TDraftsDlg::PostNcDestroy()
{
	m_imageList.m_hImageList = 0;
	CDialog::PostNcDestroy();
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	RECT  clientRect;
	int   xPos;

	GetClientRect (&clientRect);
	xPos = MoveOutboxButton ( IDOK, &clientRect );
	if (xPos)
	{
		MoveOutboxButton ( IDC_OUTBOX_EDIT,       &clientRect );
		MoveOutboxButton ( IDC_OUTBOX_RETRY,      &clientRect );
		MoveOutboxButton ( IDC_OUTBOX_DELETE,     &clientRect );
		MoveOutboxButton ( IDC_OUTBOX_CANCEL_ART, &clientRect );
		CRect rctB;
		int   listCY;
		Utility_GetPosParent ( this, IDC_OUTBOX_DELETE, rctB );
		listCY = (clientRect.bottom - (4*m_yMargin) ) / 2;
		m_listCtrl.SetWindowPos (NULL,
			m_xMargin,
			m_yMargin,
			rctB.left - (2*m_xMargin),
			listCY,
			SWP_NOZORDER);

		CWnd *pArtWnd = GetDlgItem (IDC_OUTBOX_RICHEDIT);
		pArtWnd->SetWindowPos (NULL,
			m_xMargin,
			2*m_yMargin + listCY,
			rctB.left - (2*m_xMargin),
			listCY,
			SWP_NOZORDER);
	}
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 300;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// ------------------------------------------------------------------------
// handles a custom message - probably sent from another thread
LRESULT TDraftsDlg::RefreshDraftDlg(WPARAM wP, LPARAM lP)
{
	int iTotal = 0;
	{
		THeaderIterator it (m_pDrafts);
		TPersist822Header *pHeader;
		while (it.Next (pHeader))
			++iTotal;
	}

	if (m_listCtrl.GetItemCount() == iTotal)
	{
		// number matches - redraw pretty mini-icon.
		m_listCtrl.Invalidate ();
	}
	else
	{
		// reload everything
		m_listCtrl.DeleteAllItems ();
		LoadOutbox ();
	}
	return 0;
}

// ------------------------------------------------------------------------
void TDraftsDlg::OnDblclkOutboxListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	POINT pt;
	DWORD dwPos;
	LV_HITTESTINFO hti;

	// we must find out what item was clicked
	// Find out where the cursor was for this message
	dwPos = GetMessagePos();
	pt.x = LOWORD(dwPos);
	pt.y = HIWORD(dwPos);

	::MapWindowPoints(HWND_DESKTOP, m_listCtrl.GetSafeHwnd(), &pt, 1);
	hti.pt = pt;

	// Get the oid of the persistent article and retrieve it
	int index = m_listCtrl.HitTest( &hti );
	if (-1 == index)
		return;
	TDraftDlgStatus *pData = GetpStatus(index);

	TPersist822Header * pHeader;

	m_iDisplayedArticle = pData->m_articleInt;
	pHeader = m_pDrafts->GetHeader (m_iDisplayedArticle);

	// ask header to generate the right Text type (ArticleText or EmailText)
	TPersist822Text * pText = pHeader->AllocText();

	m_pDrafts->LoadBody (pData->m_articleInt, pText);

	// does the Invalidate too
	RichDisplayArticle (GetRichEdit(), pHeader, pText);

	delete pText;
	*pResult = 0;
}

// ------------------------------------------------------------------------
//  We are only interested in the SelChange
void TDraftsDlg::OnItemChangeListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	// enable/disable the buttons

	if (m_fInitialLoad)
		return;
	UpdateGrayState ();
	*pResult = 0;
}

// ------------------------------------------------------------------------
// Handles WM_NOTIFY callback for display info
void TDraftsDlg::OnItemGetDisplayInfo (NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDisp = reinterpret_cast<LV_DISPINFO*>(pNMHDR);

	if (pDisp->item.mask & LVIF_IMAGE)
	{
		// get the datastruct stored in the lParam
		TDraftDlgStatus* pStat = GetpStatus (pDisp->item.iItem);
		StatusUnitI stat;

		(*m_pDrafts)->GetStatus (pStat->m_articleInt, &stat);

		// return an index into the imagelist
		if ((stat.m_status & TStatusUnit::kSent))
			pDisp->item.iImage = 2;
		else if ((stat.m_status & TStatusUnit::kSendErr))
			pDisp->item.iImage = 3;
		else if ((stat.m_status & TStatusUnit::kDraft))
			pDisp->item.iImage = 4;
		else if ((stat.m_status & TStatusUnit::kSending))
			pDisp->item.iImage = 1;
		else
			pDisp->item.iImage = 0;

		// additionally, track changes from "Unsent" to "Sent"
		if (!(pStat->m_byFlags & OUTBOX_FLAG_SENT)  &&
			stat.m_timeRead)
		{
			pStat->m_byFlags |= OUTBOX_FLAG_SENT;
			// put some text in the "Sent" column
			set_senttime (pDisp->item.iItem,  stat.m_timeRead);
		}
	}
}

// ------------------------------------------------------------------------
TDraftDlgStatus* TDraftsDlg::GetpStatus(int index)
{
	return (TDraftDlgStatus *) m_listCtrl.GetItemData(index);
}

// ------------------------------------------------------------------------
TDraftDlgStatus::TDraftDlgStatus(void)
{
	m_byFlags = 0;
	m_pCancelMsgID = m_pCancelSubj = m_pCancelNG = 0;
}

// ------------------------------------------------------------------------
TDraftDlgStatus::~TDraftDlgStatus(void)
{
	delete m_pCancelMsgID;
	delete m_pCancelSubj;
	delete m_pCancelNG;
}

// ------------------------------------------------------------------------
BOOL TDraftsDlg::MoveOutboxButton (int iButtonID, LPRECT pClientRect)
{
	CWnd * pWnd = GetDlgItem (iButtonID);
	if (0 == pWnd)
		return 0;
	CRect rct;
	pWnd->GetWindowRect( rct );
	ScreenToClient( rct );
	return pWnd->SetWindowPos (NULL,
		pClientRect->right - m_xMargin - (rct.right-rct.left),
		rct.top,
		NULL,
		NULL,
		SWP_NOZORDER | SWP_NOSIZE);
}

// -------------------------------------------------------------------------
int TDraftsDlg::GetFirstSelectedIndex ()
{
	int iRet = m_listCtrl.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

#if defined(_DEBUG)

	// verify with boring algorithm

	int total = m_listCtrl.GetItemCount();
	int nRet = -1;
	for (int i = 0; i < total; ++i)
	{
		if (m_listCtrl.GetItemState(i, LVIS_SELECTED))
		{
			nRet = i;
			break;
		}
	}

	ASSERT(iRet == nRet);
#endif
	return iRet;
}

// -------------------------------------------------------------------------
int TDraftsDlg::NumSelected ()
{
	return m_listCtrl.GetSelectedCount ();
}

// -------------------------------------------------------------------------
void TDraftsDlg::OnOutboxEdit ()
{
	int iIndex = GetFirstSelectedIndex ();
	if (-1 == iIndex)
		return;

	TDraftDlgStatus *pStatus = GetpStatus (iIndex);
	ASSERT (pStatus);
	BOOL bMailing = !pStatus->IsArticle ();
	LONG lArtInt = pStatus->m_articleInt;

	TAutoClose sCloser(m_pDrafts);

	// if the selected article's "outbox ID" is non-zero, it's already being
	// edited, so refuse to launch another edit window for it
	TPersist822Header *pHeader = m_pDrafts->GetHeader (lArtInt);
	if (m_pDrafts->IsEditingArticle (lArtInt)) {
		CString str; str.LoadString (IDS_ERR_ALREADY_EDITING);
		MessageBox (str);
		return;
	}

	// remember that we're editing this outbox item
	m_pDrafts->EditingArticle (lArtInt);

	// start a compose window and initialize it with the contents of the article
	TPostTemplate *pTemplate =
		bMailing ? gptrApp->GetMailToTemplate () : gptrApp->GetPostTemplate ();
	pTemplate->m_iFlags =
		TPT_TO_STRING |
		TPT_INSERT_TEXT |
		TPT_CANCEL_WARNING_ID |
		TPT_INIT_SUBJECT;
	pTemplate->m_iCancelWarningID = IDS_WARNING_MESSAGECANCEL;
	pTemplate->m_strSubjPrefix = "";

	if (pStatus->IsArticle ())
	{
		pTemplate->m_iFlags |= TPT_COPY_ARTICLE_HEADER | TPT_POST;
		pTemplate->m_pCopyArtHdr = (TArticleHeader *) pHeader;
		pTemplate->m_strSubject = pTemplate->m_pCopyArtHdr->GetSubject();
		TStringList rstrTo;
		pTemplate->m_pCopyArtHdr->GetNewsGroups(&rstrTo);
		pTemplate->m_strTo = "";
		rstrTo.CommaList(pTemplate->m_strTo);
		TArticleText *pText = new TArticleText;
		m_pDrafts->LoadBody(lArtInt, pText);
		pTemplate->m_strText = pText->GetBody();
		// OK, we're done with pText
		delete pText;
	}
	else
	{
		pTemplate->m_iFlags |= TPT_COPY_MAIL_HEADER | TPT_MAIL;
		pTemplate->m_pCopyMailHdr = (TEmailHeader *) pHeader;
		pTemplate->m_strSubject = pTemplate->m_pCopyMailHdr->GetSubject();
		TStringList rstrTo;
		pTemplate->m_pCopyMailHdr->GetDestinationList(TBase822Header::kAddrTo,
			&rstrTo);
		pTemplate->m_strTo = "";
		rstrTo.CommaList(pTemplate->m_strTo);
		TEmailText *pText = new TEmailText;
		m_pDrafts->LoadBody(lArtInt, pText);
		pTemplate->m_strText = pText->GetBody ();
		// OK, we're done with pText
		delete pText;
	}

	// OK, now launch the compose dialog
	pTemplate->Launch(0 /* newsgroupID */);

	// change this outbox entry's status to "draft"
	(*m_pDrafts)->Mark(lArtInt, TStatusUnit::kDraft);
}

// -------------------------------------------------------------------------
void TDraftsDlg::UpdateGrayState ()
{
	UpdateData();

	int iSelected = NumSelected();
	int iIndex = GetFirstSelectedIndex();

	// enable the delete button if one or more items are selected
	m_sDelete.EnableWindow(iSelected);

	// enable the edit button only if there is one item selected
	m_sEdit.EnableWindow(iSelected == 1);

	// enable cancel button if only one item is selected, it is an article,
	// has been sent, and is not a cancel article

	TDraftDlgStatus *pStatus = 0;
}

// ------------------------------------------------------------------------
void TDraftsDlg::set_senttime (int iIndex, time_t aTime)
{
	CString var;
	if (aTime)
	{
		CTime timeSent = aTime;
		var = timeSent.Format("%x %I:%M %p");
	}

	// show the sent time. Unsent items are just blank
	m_listCtrl.SetItemText (iIndex, 3, var);
}

