/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: attview.cpp,v $
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
/*  Revision 1.4  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
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

// attview.cpp : implementation file
//

#include "stdafx.h"
#include <cderr.h>
#include "News.h"
#include "attview.h"
#include "utilsize.h"

#include "tstrlist.h"
#include "globals.h"                // gUIMemory
#include "attinfo.h"
#include "mimatd.h"
#include "uimem.h"                  // TUIMemory
#include "tglobopt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions* gpGlobalOptions;

typedef struct {
	CString contentType;
	CString contentDesc;
	TMime::ECode eCode;
	int     size;
} LV_ITEM_BAGGAGE;

/////////////////////////////////////////////////////////////////////////////
// TAttachView

IMPLEMENT_DYNCREATE(TAttachView, CFormView)

TAttachView::TAttachView()
: CFormView(TAttachView::IDD)
{



	// columns will be configured on WM_SIZE
	m_fColumnsSet = FALSE;
	m_ContextMenu.LoadMenu (IDR_POPUP_ATTVIEW);
}

TAttachView::~TAttachView()
{
}

void TAttachView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);



	if (pDX->m_bSaveAndValidate)
	{
		// transfer contents to the Document
		CListCtrl* pList = (CListCtrl*) GetDlgItem (IDC_LST_ATTACH);
		int count = pList->GetItemCount();
		LV_ITEM lvi;
		const int bufsize = 512;
		LPTSTR buf = new char [bufsize];
		try
		{
			// make sure we start with a clean slate.  Fix the
			//  send attachment twice, after "empty subject" warning.
			GetDocument()->Att_RemoveAll();

			for (int i = 0; i < count; ++i)
			{
				ZeroMemory (&lvi, sizeof(lvi));
				lvi.iItem = i;
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText = buf;
				lvi.cchTextMax = bufsize;

				if (pList->GetItem ( &lvi ))
				{
					LV_ITEM_BAGGAGE* pBaggage = (LV_ITEM_BAGGAGE*) lvi.lParam;
					GetDocument()->Att_Add (LPCTSTR(lvi.pszText),  // attachment name
						pBaggage->size,
						pBaggage->contentType,
						pBaggage->contentDesc,
						pBaggage->eCode );
				}
			}
		}
		catch(CException *pE)
		{
			delete [] buf;
			pE->Delete();
		}
		delete [] buf;
	}
}

BEGIN_MESSAGE_MAP(TAttachView, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_KEYDOWN, IDC_LST_ATTACH, OnKeydownLstAttach)
	ON_NOTIFY(NM_RCLICK, IDC_LST_ATTACH, OnRightClick)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LST_ATTACH, OnDeleteitemLstAttach)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_COMMAND(IDM_POP_COMPOSE_DETACH, OnCmdDetach)
	ON_COMMAND(IDM_POP_COMPOSE_ATTACH, OnPopupAttach)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TAttachView diagnostics

#ifdef _DEBUG
void TAttachView::AssertValid() const
{
	CFormView::AssertValid();
}

void TAttachView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TAttachView message handlers

int TAttachView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void TAttachView::OnKeydownLstAttach(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;

	if (VK_INSERT == pLVKeyDown->wVKey)
	{
		QueryAttachments();
	}
	else if (VK_DELETE == pLVKeyDown->wVKey)
	{
		DeletedSelectedAttachments();
	}

	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
void TAttachView::QueryAttachments()
{
	// if
	if (GetDocument()->Att_UsingMime())
	{
		QueryMimeAttachment();
		return ;
	}

	LPTSTR buf = 0;
	const int bufsz = 2048;
	LPTSTR pFilter = 0;

	try
	{
		DWORD dwError;

		// Allocate buffer for multi-select filenames
		buf = new char[bufsz];

		// this is really important! null terminate!
		buf[0] = '\0';

		// Install filter string
		CString filtr;
		filtr.LoadString (IDS_ATTACHMENT_FILTER);
		int len = filtr.GetLength();

		pFilter = new char[len + 1];
		strcpy ( pFilter, filtr );
		for (--len; len >= 0; --len)
			if ('^' == pFilter[len])
				pFilter[len] = '\0';

		// load title
		CString title;
		title.LoadString (IDS_ATTACHMENT_TITLE);

		// Try to load
		LPCTSTR pInitialDir = NULL;
		TPath   initialDir;
		gpUIMemory->GetPath(TUIMemory::DIR_ATTACH_SRC, initialDir);
		if (!initialDir.IsEmpty())
			pInitialDir = initialDir;

		CFileDialog sDlg(TRUE, 
			NULL, 
			NULL, 
			OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_EXPLORER,
			pFilter,
			this);

		OPENFILENAME & ofn = sDlg.m_ofn;

		ofn.hwndOwner   = this->GetSafeHwnd();
		ofn.hInstance   = AfxGetInstanceHandle();
		ofn.lpstrFilter = pFilter;
		ofn.lpstrFile   = buf;
		ofn.nMaxFile    = bufsz;
		ofn.lpstrInitialDir = pInitialDir;
		ofn.lpstrTitle  = title;

		//  CFileDialog constructor has set some flags already, so OR these in
		ofn.Flags       |= OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		if (TRUE == sDlg.DoModal())
		{
			AddFilenamesToList ( sDlg );
			GetDocument()->Att_MimeLock(TRUE);
		}
		else if ((dwError = CommDlgExtendedError()) == FNERR_BUFFERTOOSMALL)
		{
			TRACE0("buffer too small");
		}
	}
	catch(CException *pE)
	{
		delete [] buf;
		delete [] pFilter;
		throw pE;
	}
	//cleanup
	delete [] buf;
	delete [] pFilter;
	return;
}

/////////////////////////////////////////////////////////////////////////////
BOOL TAttachView::QueryMimeAttachment()
{
	TMimeAttachmentDlg dlg(this);

	// these are defaults
	dlg.m_contentType = gpGlobalOptions->Get_MIME_ContentType();
	dlg.m_encoding    = gpGlobalOptions->Get_MIME_TransferEncoding();

	if (IDOK == dlg.DoModal())
	{
		AddFilenameToList (dlg.m_fileName,
			dlg.m_contentType,
			dlg.m_contentDesc,
			dlg.m_eCode);
		GetDocument()->Att_MimeLock(TRUE);
		return TRUE;
	}
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//  AddFilenamesToList -- 3-17-99 Al changed this to use sDlg.GetStartPos
//
void TAttachView::AddFilenamesToList (CFileDialog & sDlg)
{
	// Add these items to the List Control
	CListCtrl* pList = (CListCtrl*) GetDlgItem (IDC_LST_ATTACH);

	POSITION pos = sDlg.GetStartPosition();
	bool pathStored = false;
	while (pos)
	{
		CString fname = sDlg.GetNextPathName (pos);
		AddFilenameToList (fname, NULL, NULL, TMime::CODE_NONE);

		if (!pathStored)
		{
			TPath spec = fname;
			TPath dir;
			spec.GetDirectory ( dir );

			gpUIMemory->SetPath(TUIMemory::DIR_ATTACH_SRC, dir);
			pathStored = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void TAttachView::AddFilenameToList(LPCTSTR lpstrFile,
									LPCTSTR contentType,
									LPCTSTR desc,
									TMime::ECode eCode)
{
	CListCtrl* pList = (CListCtrl*) GetDlgItem (IDC_LST_ATTACH);
	LV_ITEM lvi;
	LV_ITEM_BAGGAGE* pBaggage = new LV_ITEM_BAGGAGE;

	if (NULL == contentType)
	{
		pBaggage->eCode = eCode;
	}
	else
	{
		pBaggage->contentType = contentType;
		pBaggage->contentDesc = desc;
		pBaggage->eCode = eCode;
	}

	int count = pList->GetItemCount();

	ZeroMemory( &lvi, sizeof(lvi));

	lvi.mask    = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem   = count;
	lvi.pszText = LPTSTR(lpstrFile);
	lvi.lParam  = LPARAM(pBaggage);

	pList->InsertItem ( &lvi );

	// Get the size of the file
	CFileStatus fileStatus;
	CFile::GetStatus ( lpstrFile, fileStatus );
	CString rcSize;

	pBaggage->size = fileStatus.m_size;
	rcSize.Format("%d", fileStatus.m_size);
	pList->SetItemText ( count,
		1, // subitem
		LPTSTR(LPCTSTR(rcSize)) );
}

/////////////////////////////////////////////////////////////////////////////
void TAttachView::DeletedSelectedAttachments()
{
	CListCtrl* pList = GetListCtrl();
	if (NULL == pList)
		return;

	int Count = (int) pList->GetSelectedCount();

	if (0 == Count)
		return;

	int idx;
	while ((idx = pList->GetNextItem (-1, LVNI_SELECTED)) != -1)
	{
		pList->DeleteItem ( idx );
	}

	// any left?
	if (0 == pList->GetItemCount())
		GetDocument()->Att_MimeLock(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
void TAttachView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	CListCtrl* pList = GetListCtrl();
	if (NULL == pList)
		return;

	LPTSTR p;
	int iLen;
	CString s;
	if (FALSE == m_fColumnsSet)
	{
		LV_COLUMN  lvc;

		s.LoadString(IDS_UTIL_ATTNAME);
		iLen = s.GetLength();
		p = s.GetBuffer(iLen);
		lvc.mask      = LVCF_FMT | LVCF_TEXT;
		lvc.fmt       = LVCFMT_LEFT;
		lvc.cx        = 0;
		lvc.pszText   = p;
		lvc.cchTextMax = 0;
		lvc.iSubItem   = 0;

		pList->InsertColumn(0, &lvc);
		s.ReleaseBuffer(iLen);

		s.LoadString(IDS_UTIL_SIZE);
		iLen = s.GetLength();
		p = s.GetBuffer(iLen);

		lvc.mask      = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT;
		lvc.fmt       = LVCFMT_RIGHT;
		lvc.cx        = 0;
		lvc.pszText   = p;
		lvc.cchTextMax = 0;
		lvc.iSubItem   = 1;

		pList->InsertColumn (1, &lvc);
		s.ReleaseBuffer(iLen);

		pList->SetColumnWidth (1, LVSCW_AUTOSIZE_USEHEADER);

		m_fColumnsSet = TRUE;
	}

	int newWidth  = Utility_InsetWindow ( pList );
	int col1, col2;

	// Adjust for the inset border
	int iBorder = ::GetSystemMetrics(SM_CXBORDER) * 4;
	if (newWidth > iBorder)
		newWidth -= iBorder;

	col1 = 4 * newWidth / 5;
	col2 = newWidth - col1;

	pList->SetColumnWidth (0, col1);
	pList->SetColumnWidth (1, col2);
}

// free up the extra memory
void TAttachView::OnDeleteitemLstAttach(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	LV_ITEM_BAGGAGE* pBaggage = (LV_ITEM_BAGGAGE*) pNMListView->lParam;
	delete pBaggage;

	*pResult = 0;
}

afx_msg void
TAttachView::OnRightClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}

// this is less than ideal, since it normally happens on the UP click
afx_msg LRESULT
TAttachView::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
	// sent by TAttachListCtrl
	CMenu* pTrackMenu = m_ContextMenu.GetSubMenu (0);

	CListCtrl* pList = GetListCtrl();
	UINT flag = (pList->GetItemCount() > 0) ? MF_ENABLED : MF_GRAYED;
	pTrackMenu->EnableMenuItem(IDM_POP_COMPOSE_DETACH, flag);

	// pass in screen coordinates
	pTrackMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		LOWORD(lParam),HIWORD(lParam), this);
	return 0;
}

void TAttachView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
}

afx_msg void TAttachView::OnPopupAttach()
{
	QueryAttachments();
}

afx_msg void TAttachView::OnCmdDetach()
{
	DeletedSelectedAttachments();
}

// -------------------------------------------------------------------------
void TAttachView::SetAttachments (int iNumAttachments,
								  const TAttachmentInfo *psAttachments)
{
	for (int i = 0; i < iNumAttachments; i++)
		AddFilenameToList (psAttachments [i].m_att,
		psAttachments [i].m_contentType, psAttachments [i].m_contentDesc,
		psAttachments [i].m_eCode);
}
