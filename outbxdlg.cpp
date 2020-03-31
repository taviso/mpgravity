/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: outbxdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.8  2009/01/28 14:56:04  richard_wood
/*  Fixed SF bug 2536614 - "Save Message To Draft" from the Outbox lost the message body.
/*
/*  Revision 1.7  2009/01/28 13:16:09  richard_wood
/*  Source formatting changes
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/09/19 14:51:39  richard_wood
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
#include "OutbxDlg.h"
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
COutboxDlg::COutboxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COutboxDlg::IDD, pParent)
{
	m_bPurge = FALSE;
	m_iPurgeDays = 0;

	LONG lBase = GetDialogBaseUnits();
	m_xMargin = MulDiv(7,LOWORD(lBase),4);
	m_yMargin = MulDiv(7,HIWORD(lBase),8);
	m_iDisplayedArticle = 0;
	m_fInitialLoad = FALSE;
}

// ------------------------------------------------------------------------
void COutboxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_OUTBOX_EXPORT, m_sExport);
	DDX_Control(pDX, IDC_PURGE_DAYS_STATIC, m_sPurgeDaysStatic);
	DDX_Control(pDX, IDC_PURGE, m_sPurge);
	DDX_Control(pDX, IDC_PURGE_DAYS, m_sPurgeDays);
	DDX_Control(pDX, IDC_OUTBOX_CANCEL_ART, m_sCancel);
	DDX_Control(pDX, IDC_OUTBOX_DELETE, m_sDelete);
	DDX_Control(pDX, IDC_OUTBOX_RETRY, m_sRetrySend);
	DDX_Control(pDX, IDC_OUTBOX_LISTCTRL, m_listCtrl);
	DDX_Check(pDX, IDC_PURGE, m_bPurge);
	DDX_Text(pDX, IDC_PURGE_DAYS, m_iPurgeDays);
	DDV_MinMaxUInt(pDX, m_iPurgeDays, 1, 999);
}

// ------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(COutboxDlg, CDialog)
	ON_BN_CLICKED(IDC_OUTBOX_DELETE, OnOutboxDelete)
	ON_NOTIFY(LVN_DELETEITEM, IDC_OUTBOX_LISTCTRL, OnDeleteitemOutboxListctrl)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_OUTBOX_LISTCTRL, OnDblclkOutboxListctrl)
	ON_MESSAGE (WMU_REFRESH_OUTBOX, RefreshOutbox)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_OUTBOX_LISTCTRL, OnItemChangeListctrl)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_OUTBOX_LISTCTRL, OnItemGetDisplayInfo)
	ON_BN_CLICKED(IDC_OUTBOX_CANCEL_ART, OnOutboxCancelArt)
	ON_BN_CLICKED(IDC_PURGE, OnPurge)
	ON_BN_CLICKED(IDC_OUTBOX_RETRY, OnOutboxRetry)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_OUTBOX_EXPORT, OnOutboxExport)
	ON_BN_CLICKED(IDC_OUTBOX_EDIT, OnOutboxEdit)
END_MESSAGE_MAP()

// ------------------------------------------------------------------------
void COutboxDlg::MakeColumns()
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
BOOL COutboxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pServer = GetCountedActiveServer ();
	m_pServer->AddRef ();

	// open the outbox and pin it...

	// make sure the outbox is open and pinned...
	m_pOutbox = m_pServer->GetOutbox ();
	m_pOutbox->Open ();

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
		m_wndRich.PostSubclass ();

		// pleas send WM_NOTIFY:EN_SELCHANGE to the parent
		DWORD dwEvntMask = m_wndRich.SendMessage ( EM_GETEVENTMASK );
		m_wndRich.SendMessage ( EM_SETEVENTMASK, 0, dwEvntMask | ENM_SELCHANGE);

		// auto vscroll (duh)
		m_wndRich.SendMessage ( EM_SETOPTIONS, ECOOP_OR,
			ECO_AUTOVSCROLL | ECO_READONLY);

		SetInitialFont(pRich);
	}

	m_bPurge = m_pServer->GetPurgeOutbox();
	m_iPurgeDays = m_pServer->GetPurgeOutboxDays();

	UpdateData (FALSE /* bSaveAndValidate */);

	// set dialog pos
	CString strSizePos = gpGlobalOptions->GetRegUI ()->GetOutboxSizePos ();
	int dx, dy, x, y;
	if (!DecomposeSizePos (strSizePos, dx, dy, x, y))
		SetWindowPos (NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// set dialog size and header widths
	int iX, iY;
	BOOL bMax, bMin;
	TRegUI *pRegUI = gpGlobalOptions->GetRegUI ();
	if (!pRegUI->LoadUtilDlg ("Outbox", iX, iY, bMax, bMin))
	{
		SetWindowPos (NULL, 0, 0, iX, iY, SWP_NOZORDER | SWP_NOMOVE);
		if (bMax)
			ShowWindow (SW_SHOWMAXIMIZED);
	}

	int riSizes [4];
	if (!pRegUI->LoadUtilHeaders ("Outbox", riSizes,
		sizeof riSizes / sizeof (int)))
		for (int i = 0; i < 4; i++)
			m_listCtrl.SetColumnWidth (i, riSizes [i]);

	// fill the list view
	LoadOutbox ();

	// set up notification

	// setup the correct icon
	//m_hOutboxIcon = AfxGetApp()->LoadIcon (IDR_OUTBOX);
	//m_hOldIcon = SendMessage(WM_SETICON, TRUE, (LPARAM)m_hOutboxIcon);

	m_hTimer = SetTimer (100, 600, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
}

// ------------------------------------------------------------------------
void COutboxDlg::RemoveOutboxItem (int artInt, int fArticle)
{
	m_pOutbox->WriteLock ();
	m_pOutbox->PurgeHeader (artInt);
	m_pOutbox->PurgeBody (artInt);
	m_pOutbox->UnlockWrite ();
	(*m_pOutbox)->RemoveItem (artInt);
}

// ------------------------------------------------------------------------
// DeleteOutboxMessage -- call FinishDeleting() after done deleting one or more
// outbox messages
void COutboxDlg::DeleteOutboxMessage (int iIndex,
									  BOOL bSelectAnother /* = TRUE */, BOOL bSetFocus /* = TRUE */)
{
	TOutboxDlgStatus *pStatus = GetpStatus (iIndex);
	int iArtInt = pStatus->m_articleInt;

	// if this article is currently displayed, clear the display
	if (iArtInt == m_iDisplayedArticle)
		GetRichEdit ()->SendMessage (WM_SETTEXT, 0, (LPARAM) "");

	// ????? warnCBX thing...
	RemoveOutboxItem (iArtInt, pStatus->IsArticle ());
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
void COutboxDlg::FinishDeleting ()
{
	// save the outbox
	m_pOutbox->SaveHeaders ();
	m_pOutbox->SaveBodyMetaData ();
	m_pServer->SaveOutboxState ();

	gpsOutboxBar->UpdateDisplay ();
}

// ------------------------------------------------------------------------
void COutboxDlg::OnOutboxDelete()
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
void COutboxDlg::OnDeleteitemOutboxListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	delete GetpStatus(pNMListView->iItem);
	*pResult = 0;
}

// ------------------------------------------------------------------------
void COutboxDlg::OnClose()
{
	if (!UpdateData ())
		return;  // DDV error
	m_pServer->SetPurgeOutbox (m_bPurge);
	m_pServer->SetPurgeOutboxDays (m_iPurgeDays);
	m_pServer->SaveSettings ();

	// save window size and header widths
	TRegUI *pRegUI = gpGlobalOptions->GetRegUI ();
	int riSizes [4];
	for (int i = 0; i < 4; i++)
		riSizes [i] = m_listCtrl.GetColumnWidth (i);
	pRegUI->SaveUtilHeaders ("Outbox", riSizes, sizeof riSizes / sizeof (int));

	BOOL bZoomed = IsZoomed ();
	BOOL bIconic = IsIconic ();

	WINDOWPLACEMENT wp;
	RECT & rct = wp.rcNormalPosition;

	GetWindowPlacement (&wp);

	// save window width and state
	pRegUI->SaveUtilDlg ("Outbox", (int) (rct.right - rct.left),
		(int) (rct.bottom - rct.top), bZoomed, bIconic);

	// save window pos
	CString strSizePos = ComposeSizePos (0, 0, rct.left, rct.top);
	gpGlobalOptions->GetRegUI ()->SetOutboxSizePos (strSizePos);

	// close outbox object
	m_pOutbox->Close ();

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
void COutboxDlg::OnCancel()
{
	OnClose();
}

// ------------------------------------------------------------------------
void COutboxDlg::OnOK()
{
	CDialog::OnOK();
	OnClose();
}

// ------------------------------------------------------------------------
int  COutboxDlg::GetItemIndex (TPersist822Header *pItem)

{
	int               index;
	int               numItems = m_listCtrl.GetItemCount();
	TOutboxDlgStatus  *pCurrStat;

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
int CALLBACK  OutboxCompareFunc (LPARAM param1, LPARAM param2, LPARAM cookie)
{
	TOutboxDlgStatus * pStatus1 = (TOutboxDlgStatus *) (void*)param1;
	TOutboxDlgStatus * pStatus2 = (TOutboxDlgStatus *) (void*)param2;

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
static void make_recips (TPersist822Header * pItem, TStringList & recipients, CString & temp)
{
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
}

// ------------------------------------------------------------------------
void COutboxDlg::InsertOutboxItem (TPersist822Header *pItem)
{
	// binary search for created date
	int               index = 0;

	// get either the newsgroups or To: field recipients...
	CString  temp;
	TStringList recipients;
	make_recips ( pItem, recipients, temp);

	int idxAt;

	StatusUnitI stat;

	(*m_pOutbox)->GetStatus (pItem->GetNumber(), &stat);

	// To:
	// the correct icon is determined via a WM_NOTIFY callback
	idxAt = m_listCtrl.InsertItem (index, temp, I_IMAGECALLBACK);

	// subject
	m_listCtrl.SetItemText (idxAt, 1, pItem->IsArticle()
		? pItem->CastToArticleHeader()->GetSubject()
		: pItem->CastToEmailHeader()->GetSubject());

	CString str; str.LoadString (IDS_OUTBOX_TIME_FORMAT);
	CString var = (pItem->GetTime()).Format(str);
	m_listCtrl.SetItemText (idxAt, 2, var);  // created time

	// the status timeRead member contains the time sent for
	// outbox items.  If it has been sent, put it in,
	// otherwise leave the column blank

	TOutboxDlgStatus  *pStat = new TOutboxDlgStatus;

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
void COutboxDlg::LoadOutbox()
{
	DWORD dwTickCount = GetTickCount();
	m_fInitialLoad = TRUE;
	THeaderIterator it (m_pOutbox);

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
	m_listCtrl.SortItems (OutboxCompareFunc, NULL);

	TRACE ("outbox items : %d, average milliseconds per item : %u\n",
		count,
		count ? (GetTickCount() - dwTickCount)/count : 0);

	// enable/disable the buttons
	UpdateGrayState ();
	m_fInitialLoad = FALSE;
}

// ------------------------------------------------------------------------
void COutboxDlg::PostNcDestroy()
{
	m_imageList.m_hImageList = 0;
	CDialog::PostNcDestroy();
}

// ------------------------------------------------------------------------
void COutboxDlg::OnSize(UINT nType, int cx, int cy)
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
		MoveOutboxButton ( IDC_OUTBOX_EXPORT,     &clientRect );
		CRect rctB;
		int   listCY;
		Utility_GetPosParent ( this, IDC_OUTBOX_DELETE, rctB );
		CRect sRect;
		m_sPurgeDays.GetClientRect (&sRect);
		int iPurgeDaysHeight = sRect.bottom - sRect.top;
		listCY = (clientRect.bottom - (4*m_yMargin) - iPurgeDaysHeight) / 2;
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
		m_sPurge.SetWindowPos (NULL, m_xMargin, 3 * m_yMargin + 2 * listCY,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
		m_sPurge.GetClientRect (&sRect);
		int iXPos = sRect.right - sRect.left + m_xMargin * 2;
		m_sPurgeDays.SetWindowPos (NULL, iXPos, 3 * m_yMargin + 2 * listCY,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
		m_sPurgeDays.GetClientRect (&sRect);
		iXPos += m_xMargin + sRect.right - sRect.left;
		m_sPurgeDaysStatic.SetWindowPos (NULL, iXPos, 3 * m_yMargin + 2 * listCY,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
}

// ------------------------------------------------------------------------
void COutboxDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 300;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// ------------------------------------------------------------------------
// handles a custom message - probably sent from another thread
LRESULT COutboxDlg::RefreshOutbox(WPARAM wP, LPARAM lP)
{
	int iTotal = 0;
	{
		THeaderIterator it (m_pOutbox);
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
void COutboxDlg::OnDblclkOutboxListctrl(NMHDR* pNMHDR, LRESULT* pResult)
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
	TOutboxDlgStatus *pData = GetpStatus(index);

	TPersist822Header * pHeader;

	m_iDisplayedArticle = pData->m_articleInt;
	pHeader = m_pOutbox->GetHeader (m_iDisplayedArticle);

	// ask header to generate the right Text type (ArticleText or EmailText)
	TPersist822Text * pText = pHeader->AllocText();

	m_pOutbox->LoadBody (pData->m_articleInt, pText);

	// does the Invalidate too
	RichDisplayArticle (GetRichEdit(), pHeader, pText);

	delete pText;
	*pResult = 0;
}

// ------------------------------------------------------------------------
//  We are only interested in the SelChange
void COutboxDlg::OnItemChangeListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	// enable/disable the buttons

	if (m_fInitialLoad)
		return;
	UpdateGrayState ();
	*pResult = 0;
}

// ------------------------------------------------------------------------
// Handles WM_NOTIFY callback for display info
void COutboxDlg::OnItemGetDisplayInfo (NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDisp = reinterpret_cast<LV_DISPINFO*>(pNMHDR);

	if (pDisp->item.mask & LVIF_IMAGE)
	{
		// get the datastruct stored in the lParam
		TOutboxDlgStatus* pStat = GetpStatus (pDisp->item.iItem);
		StatusUnitI stat;

		(*m_pOutbox)->GetStatus (pStat->m_articleInt, &stat);

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
TOutboxDlgStatus* COutboxDlg::GetpStatus(int index)
{
	return (TOutboxDlgStatus *) m_listCtrl.GetItemData(index);
}

// ------------------------------------------------------------------------
TOutboxDlgStatus::TOutboxDlgStatus(void)
{
	m_byFlags = 0;
	m_pCancelMsgID = m_pCancelSubj = m_pCancelNG = 0;
}

// ------------------------------------------------------------------------
TOutboxDlgStatus::~TOutboxDlgStatus(void)
{
	delete m_pCancelMsgID;
	delete m_pCancelSubj;
	delete m_pCancelNG;
}

// ------------------------------------------------------------------------
BOOL COutboxDlg::MoveOutboxButton (int iButtonID, LPRECT pClientRect)
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

// ------------------------------------------------------------------------
void COutboxDlg::OnOutboxCancelArt()
{
	CNewsDoc* pDoc = ((CNewsApp*)AfxGetApp())->GetGlobalNewsDoc();
	if (0 == pDoc)
		return;

	int index = GetFirstSelectedIndex ();
	if (-1 == index)
	{
		CString str; str.LoadString (IDS_ERR_NO_ITEM_SELECTED);
		NewsMessageBox (this, str);
		return;
	}

	TOutboxDlgStatus* pStatus = GetpStatus (index);
	ASSERT( pStatus );

	if (pStatus->m_pCancelMsgID && pStatus->m_pCancelSubj &&
		pStatus->m_pCancelNG && !pStatus->m_pCancelNG->IsEmpty())
	{
		if (IDNO == NewsMessageBox(this, IDS_WARN_CANCELMSG, MB_YESNO | MB_ICONQUESTION))
			return;
		pDoc->CancelMessage (*pStatus->m_pCancelNG,
			*pStatus->m_pCancelMsgID,
			*pStatus->m_pCancelSubj);
	}
}

// -------------------------------------------------------------------------
int COutboxDlg::GetFirstSelectedIndex ()
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
int COutboxDlg::NumSelected ()
{
	return m_listCtrl.GetSelectedCount ();
}

// -------------------------------------------------------------------------
void COutboxDlg::UpdateGrayState ()
{
	UpdateData ();

	m_sPurgeDays.EnableWindow (m_bPurge);

	int iSelected = NumSelected ();
	int iIndex = GetFirstSelectedIndex ();

	// enable the delete button if one or more items are selected
	m_sDelete.EnableWindow (iSelected);

	// enable cancel button if only one item is selected, it is an article,
	// has been sent, and is not a cancel article

	TOutboxDlgStatus *pStatus = 0;

	BOOL bCancelOn = FALSE;
	if (iSelected == 1)
	{
		pStatus = GetpStatus (iIndex);
		ASSERT (pStatus);
		if (pStatus->IsArticle () && pStatus->IsSent () &&
			!pStatus->IsCancelMsg ())
			bCancelOn = TRUE;
	}
	m_sCancel.EnableWindow (bCancelOn);

	UpdateRetryButton ();

	m_sExport.EnableWindow (iSelected);
}

void COutboxDlg::UpdateRetryButton ()
{
	int iSelected = NumSelected ();
	BOOL  fRetryOn = FALSE;

	if (1 == iSelected)
	{
		TOutboxDlgStatus *pStatus = 0;
		int iIndex = GetFirstSelectedIndex ();

		pStatus = GetpStatus (iIndex);
		if (pStatus)
		{
			StatusUnitI stat;

			(*m_pOutbox)->GetStatus (pStatus->m_articleInt, &stat);

			if (stat.m_status & TStatusUnit::kSendErr)
				fRetryOn = TRUE;
		}
	}
	m_sRetrySend.EnableWindow (fRetryOn);
}

// TOutboxBar dialog
TOutboxBar::TOutboxBar()
	: CDialogBar ()
{
	m_strNumErrors = _T("");
	m_strNumWaiting = _T("");
}

void TOutboxBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_NUM_ERRORS, m_strNumErrors);
	DDX_Text(pDX, IDC_NUM_WAITING, m_strNumWaiting);
}

BEGIN_MESSAGE_MAP(TOutboxBar, CDialogBar)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// global pointer to the one outbox-bar
TOutboxBar *gpsOutboxBar;

void TOutboxBar::UpdateDisplay ()
{
	// can't do anything if there's no server
	if (!IsActiveServer())
		return;

	TServerCountedPtr cpNewsServer;        // smart pointer
	int iNumWaiting = 0;
	int nSending = 0;
	int iNumErrors = 0;

	TOutbox *pOutbox = cpNewsServer->GetOutbox ();
	ASSERT (pOutbox);
	pOutbox->CountWaitingAndSending (iNumWaiting, nSending, iNumErrors);

	m_strNumWaiting.Format ("%d", iNumWaiting + nSending);
	m_strNumErrors.Format ("%d", iNumErrors);
	UpdateData (FALSE);
}

void COutboxDlg::set_senttime (int iIndex, time_t aTime)
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

void COutboxDlg::OnPurge()
{
	UpdateGrayState ();
}

void COutboxDlg::OnOutboxRetry()
{
	int iSelected = NumSelected ();
	int iIndex = GetFirstSelectedIndex ();

	// for now, we only do 1 at a time
	if (iSelected != 1)
		return;

	// get the datastruct stored in the lParam
	TOutboxDlgStatus* pStat = GetpStatus (iIndex);
	StatusUnitI stat;

	(*m_pOutbox)->GetStatus (pStat->m_articleInt, &stat);

	if (stat.m_status & TStatusUnit::kSendErr)
	{
		if ((*m_pOutbox)->RetrySend (pStat->m_articleInt))
		{
			// redraw item so it shows 'waiting'
			RECT rct;
			m_listCtrl.GetItemRect (iIndex, &rct, LVIR_ICON);

			m_listCtrl.InvalidateRect (&rct);
		}
	}
}

void COutboxDlg::OnTimer (UINT nIDEvent)
{
	UpdateRetryButton ();

	// catch items that become "SendError", so we can enable the retry button
}

// dump to text file
void COutboxDlg::OnOutboxExport()
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();
	if (NULL == pos)
		return;

	// Ask for a filename
	CString  fileName;
	CFile    file;

	CNewsApp* pApp = (CNewsApp*) AfxGetApp();

	// this function gives us an open file
	BOOL fRet = pApp->GetSpec_SaveArticle (this, &fileName, &file);
	if (FALSE == fRet)
		return;

	// the file is open - yay!
	//BOOL fFullHdr = gpGlobalOptions->GetRegUI()->GetShowFullHeader();

	BOOL fFullHdr = TRUE;      // just default to this

	bool fFirstItem = true;
	CString strDash('=', 78);
	CString strSeparator; strSeparator.Format("\r\n%s\r\n", LPCTSTR(strDash));

	while (pos)
	{
		if (fFirstItem)
		{
			fFirstItem = false;
		}
		else
		{
			file.Write ( strSeparator, strSeparator.GetLength() );
		}

		int index = m_listCtrl.GetNextSelectedItem(pos);

		TOutboxDlgStatus *pData = GetpStatus(index);

		TPersist822Header * pHeader = m_pOutbox->GetHeader (pData->m_articleInt);

		CString strRecipients;
		TStringList recips;
		make_recips ( pHeader, recips, strRecipients);

		// subject
		CString strSubj =  pHeader->IsArticle()
			? pHeader->CastToArticleHeader()->GetSubject()
			: pHeader->CastToEmailHeader()->GetSubject();

		// created time
		CString str; str.LoadString (IDS_OUTBOX_TIME_FORMAT);
		CString var = (pHeader->GetTime()).Format(str);

		// ask header to generate the right Text type (ArticleText or EmailText)
		TPersist822Text * pText = pHeader->AllocText();
		m_pOutbox->LoadBody (pData->m_articleInt, pText);

		try
		{
			file.Write ("Subject: ", 9);
			file.Write(strSubj, strSubj.GetLength()); file.Write ("\r\n",2);

			file.Write ("To: ", 4);
			file.Write (strRecipients, strRecipients.GetLength()); file.Write ("\r\n",2);

			file.Write ("Date: ", 6);
			file.Write (var, var.GetLength());
			file.Write ("\r\n\r\n",4);

			const CString & body = pText->GetBody();
			file.Write (body, body.GetLength());

			// can't do this since full header doesn't really exist
			//CNewsDoc::m_pDoc->SaveToFile (pHeader, pText, fFullHdr, file);
		}
		catch (CFileException * pfe)
		{
			pfe->ReportError (MB_OK | MB_ICONSTOP);
			pfe->Delete ();
			delete pText;
			return;
		}

		delete pText;
	}
}

void COutboxDlg::OnOutboxEdit ()
{
	int iIndex = GetFirstSelectedIndex ();
	if (-1 == iIndex)
		return;

	TOutboxDlgStatus* pStatus = GetpStatus (iIndex);
	ASSERT (pStatus);
	BOOL fMailing = !pStatus->IsArticle ();
	LONG lArtInt = pStatus->m_articleInt;

	TAutoClose sCloser(m_pOutbox);

	// delete the status unit for this guy
	(*m_pOutbox)->RemoveItem (lArtInt);

	TPersist822Header *pHeader = m_pOutbox->GetHeader (lArtInt);

	// make a fair copy of the Header and Body
	TPersist822Header * pCopyHdr;
	TPersist822Text *   pCopyText;
	if (fMailing)
	{
		pCopyHdr = new TEmailHeader;

		*pCopyHdr = *(pHeader->CastToEmailHeader());
	}
	else
	{
		pCopyHdr = new TArticleHeader;

		*pCopyHdr = *(pHeader->CastToArticleHeader());
	}

	pCopyText = pCopyHdr->AllocText();
	m_pOutbox->LoadBody (lArtInt, pCopyText);

	m_pOutbox->PurgeHeader (lArtInt);
	m_pOutbox->PurgeBody (lArtInt);

	// go from 7bit QP text  to  8bit text --> since we are treating this
	//  as an internal message again

	CString text = pCopyText->GetBody ();

	CString ver, type, encoding, desc;
	pCopyHdr->GetMimeLines(&ver, &type, &encoding, &desc);

	int idx;

	encoding.MakeLower();
	if (encoding.Find ("quoted-printable") != -1)
	{
		int ln = text.GetLength();
		LPTSTR pBuf = new TCHAR[ln + 1];
		auto_ptr<TCHAR> sDeleter(pBuf);

		// this gets us from 7 bit data to 8 bit bytes
		ULONG uLen = String_QP_decode (text,
			ln,
			pBuf,
			ln + 1);
		LPTSTR p = text.GetBuffer((int)uLen);
		_tcsncpy (p, pBuf, (int)uLen);
		text.ReleaseBuffer((int)uLen);
	}
	idx = type.Find ("charset=");
	if (idx >= 0)
	{
		CString textFinal;

		// RLW : If CP_Inbound does not convert anything (returns != 0),
		// we must copy input text to output.
		if (CP_Inbound( type.Mid(idx),
							 text,
							 text.GetLength(),
							 textFinal))
		{
			textFinal = text;
		}

		pCopyText->SetBody (textFinal);
	}

	if (true)
	{
		TArticleHeader sHdr;
		TArticleHeader sHdrR2;  // handles Reply-To:

		// using a temp sHdr to get back to 8bit
		CString origFrom;
		if (fMailing)
		{
			// do the from
			origFrom = pCopyHdr->CastToEmailHeader()->GetFrom();
			origFrom = strip_newline( origFrom );

			sHdr.SetQPFrom( origFrom );
			pCopyHdr->CastToEmailHeader()->SetFrom( sHdr.GetFrom() );

			// do the subject
			CString subj = pCopyHdr->CastToEmailHeader()->GetSubject();
			subj = strip_newline( subj );
			sHdr.SetQPSubject ( subj );
			pCopyHdr->CastToEmailHeader()->SetSubject( sHdr.GetSubject() );

			// do the reply to
			CString replyTo = pCopyHdr->CastToEmailHeader()->GetReplyTo();
			sHdrR2.SetQPFrom( replyTo );
			pCopyHdr->CastToEmailHeader()->SetReplyTo( sHdrR2.GetFrom() );
		}
		else
		{
			// do the from
			origFrom = pCopyHdr->CastToArticleHeader()->GetFrom();
			origFrom = strip_newline( origFrom );

			sHdr.SetQPFrom( origFrom );
			CString tmp = sHdr.GetFrom();
			pCopyHdr->CastToArticleHeader()->SetOrigFrom( tmp );

			// do the subject
			CString subj = pCopyHdr->CastToArticleHeader()->GetSubject();
			subj = strip_newline( subj );
			sHdr.SetQPSubject ( subj );
			tmp = sHdr.GetSubject();
			pCopyHdr->CastToArticleHeader()->SetSubject( tmp );

			// do the reply to
			CString replyTo = pCopyHdr->CastToArticleHeader()->GetReplyTo();
			sHdrR2.SetQPFrom( replyTo );
			pCopyHdr->CastToArticleHeader()->SetReplyTo( sHdrR2.GetFrom() );
		}
	}

	TServerCountedPtr cpServer(m_pOutbox->GetParentServer());
	TDraftMsgs* pDrafts = cpServer->GetDraftMsgs();

	pDrafts->SaveDraftMessage ( pCopyHdr , pCopyText );

	cpServer->SaveOutboxState();

	pCopyHdr = 0;
	delete pCopyText;

	// refresh the outbox display
	AfxGetMainWnd ()->PostMessage (WMU_REFRESH_OUTBOX);
	AfxGetMainWnd ()->PostMessage (WMU_REFRESH_OUTBOX_BAR);
}
