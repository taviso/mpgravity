/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecq.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:00  richard_wood
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

// tdecq.cpp -- list of queued decode jobs

#include "stdafx.h"              // precompiled header
#include "resource.h"            // IDS_*
#include "tdecq.h"               // this file's prototypes
#include "tdecjob.h"             // TDecodeJob
#include "custmsg.h"             // WMU_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
void TDecodeQueue::SetDefaultWidths ()
{
	RECT rct;
	GetClientRect (&rct);

	SetColumnWidth (0, rct.right / 4);
	SetColumnWidth (1, (m_iMyPosition == WAIT_Q ?
		rct.right * 3 / 4 - SCROLL_BAR_WIDTH:
	rct.right / 4));
	if (m_iMyPosition == CURRENT)
		// column 3 -- status
		SetColumnWidth (2, rct.right / 2 - SCROLL_BAR_WIDTH);

	if (m_iMyPosition == COMPLETED) {
		// column 3 -- file name
		SetColumnWidth (2, rct.right / 4);

		// column 4 -- status
		SetColumnWidth (3, rct.right / 4 - SCROLL_BAR_WIDTH);
	}
}

// -------------------------------------------------------------------------
// MakeColumns -- sets up the listbox
void TDecodeQueue::MakeColumns ()
{
	RECT rct;
	LV_COLUMN lvc;
	GetClientRect (&rct);
	ZeroMemory (&lvc, sizeof(lvc));
	CString str;

	// column 1 -- newsgroup's name
	lvc.mask       = LVCF_FMT | LVCF_TEXT;
	str.LoadString (IDS_UTIL_NEWSGROUP);
	lvc.pszText    = (LPSTR) (LPCSTR) str;
	lvc.iSubItem   = 0;
	InsertColumn (0, &lvc);

	// column 2 -- article subject
	lvc.mask       |= LVCF_SUBITEM;
	str.LoadString (IDS_UTIL_SUBJECT);
	lvc.pszText    = (LPSTR) (LPCSTR) str;
	lvc.iSubItem   = 1;
	InsertColumn (1, &lvc);

	if (m_iMyPosition == CURRENT) {
		// column 3 -- status
		str.LoadString (IDS_UTIL_STATUS);
		lvc.pszText    = (LPSTR) (LPCSTR) str;
		lvc.iSubItem   = 2;
		InsertColumn (2, &lvc);
	}

	if (m_iMyPosition == COMPLETED) {
		// column 3 -- file name
		str.LoadString (IDS_UTIL_FILENAME);
		lvc.pszText    = (LPSTR) (LPCSTR) str;
		lvc.iSubItem   = 2;
		InsertColumn (2, &lvc);

		// column 4 -- status
		str.LoadString (IDS_UTIL_STATUS);
		lvc.pszText    = (LPSTR) (LPCSTR) str;
		lvc.iSubItem   = 3;
		InsertColumn (3, &lvc);
	}

	if (m_iMyPosition == WAIT_Q)
		m_iNumColumns = 2;
	else if (m_iMyPosition == CURRENT)
		m_iNumColumns = 3;
	else
		m_iNumColumns = 4;

	SetDefaultWidths ();
}

// -------------------------------------------------------------------------
// FillRow -- fills a row
void TDecodeQueue::FillRow (void *pJob, LV_ITEM *pLVI)
{
	TDecodeJob *pDecodeJob = (TDecodeJob *) pJob;

	pLVI -> pszText = (char *) (LPCTSTR) pDecodeJob -> NewsgroupNickname ();
	InsertItem (pLVI);

	SetItemText (pLVI -> iItem, 1 /* subitem */,
		(char *) (LPCTSTR) pDecodeJob -> Subject ());

	if (m_iMyPosition == CURRENT)
		SetItemText (pLVI -> iItem, 2 /* subitem */,
		(char *) (LPCTSTR) pDecodeJob -> Status ());

	if (m_iMyPosition == COMPLETED) {
		SetItemText (pLVI -> iItem, 2 /* subitem */,
			(char *) (LPCTSTR) pDecodeJob -> FirstFilename ());
		SetItemText (pLVI -> iItem, 3 /* subitem */,
			(char *) (LPCTSTR) pDecodeJob -> Status ());
	}
}

// -------------------------------------------------------------------------
void TDecodeQueue::GetCurrentLineStatus (CString &str)
{
	// first make sure something is selected
	if (0 == GetSelectedCount())
		return;

	// get the selected item
	int iIndex = -1;
	int iNum = GetItemCount ();
	for (int i = 0; i < iNum; i++)
		if (GetItemState (i, LVIS_SELECTED)) {
			iIndex = i;
			break;
		}

		if (iIndex >= 0)
			// get the item's text
			str = GetItemText (iIndex, 3 /* nSubItem */);
}

// -------------------------------------------------------------------------
void TDecodeQueue::CheckForHorizontalScrollbar ()
{
}
