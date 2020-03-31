/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: genutil.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.8  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.7  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.6  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.5  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.4  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.3  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.9  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.8  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.7  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.6  2009/01/20 12:05:39  richard_wood
/*  Fixed the bug introduced by VS2005 conversion into the Rules and Filters creation.
/*  Details:-
/*  CRichEditCtrl::GetLine(nLine, pBuf, nLen)
/*  wants the buffer to be at least sizeof(int) long cause it copies
/*  the buffer length into the first bytes. God knows why cause it then
/*  immediately overwrites it. Duh Microsoft!
/*
/*  Revision 1.5  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.4  2008/09/19 14:51:26  richard_wood
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

// genutil.cpp -- generic utilities

#include "stdafx.h"              // for genutil.h
#include "genutil.h"             // this file's prototypes
#include "fetchart.h"            // BlockingFetchBody()
#include "ngutil.h"              // UtilGetStorageOption()
#include "names.h"               // FORMAT_BODIES
#include "globals.h"             // TNewsServer
#include "tnews3md.h"            // TNews3MDIChildWnd
#include "mlayout.h"             // TMdiLayout
#include "mainfrm.h"             // CMainFrame
#include "resource.h"            // IDS_*
#include "tglobopt.h"            // GetRegWarn(), ...
#include "rgswit.h"              // TRegSwitch
#include "thrdlvw.h"             // TThreadListView
#include "nglist.h"              // TNewsGroupUseLock
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "custview.h"            // TCustomArticleView, ...
#include "arttext.h"             // ArticleCreateField()
#include "licutil.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// indentation increment for condition listbox
#define CONDITION_INDENT_INCREMENT 3

// -------------------------------------------------------------------------
// getGetMainWnd --  don't call AfxGetMainWnd(), it's not thread safe
static CWnd * genGetMainWnd()
{
	extern HWND  ghwndMainFrame;

	if (NULL == ghwndMainFrame)
		return AfxGetMainWnd();

	return CWnd::FromHandle (  ghwndMainFrame );
}

// -------------------------------------------------------------------------
// MsgResource -- loads a message from the resource file and displays it.
// Returns the result of AfxMessageBox
int MsgResource (int iResourceID, CWnd *pWnd /* = 0 */,
				 LPCTSTR pchCaption /* = 0 */, UINT nType /* = MB_OK */)
{
	CString strCaption;

	if (pchCaption)
		strCaption = pchCaption;
	else
		strCaption.LoadString (IDS_PROG_NAME);

	if (!pWnd)
		pWnd = genGetMainWnd ();

	CString str;
	str.LoadString (iResourceID);
	return pWnd->MessageBox (str, strCaption, nType);
}

// -------------------------------------------------------------------------
// DeleteSelected -- deletes selected items from a listbox
void DeleteSelected (CListBox *pListbox)
{
	int iLen = pListbox->GetCount();
	if (iLen != LB_ERR)
		for (int i = 0; i < iLen; i++) {
			int iSel = pListbox->GetSel (i);
			if (iSel == LB_ERR)
				continue;
			if (iSel && pListbox->DeleteString (i) != LB_ERR) {
				// deleted the item, so the listbox is smaller now
				i--;
				iLen--;
			}
		}
}

// -------------------------------------------------------------------------
// DDX_LBStringList -- performs dialog-data-exchange between a CListBox and
// a CStringList
void DDX_LBStringList (CDataExchange* pDX, int nIDC, CStringList &sList)
{
	CListBox *pListbox = (CListBox *) pDX->m_pDlgWnd->GetDlgItem (nIDC);
	if (pDX->m_bSaveAndValidate) {
		sList.RemoveAll ();
		int iLen = pListbox->GetCount ();
		CString str;
		for (int i = 0; i < iLen; i++) {
			pListbox->GetText (i, str);
			sList.AddHead (str);
		}
	}
	else {
		pListbox->ResetContent ();
		POSITION iPos = sList.GetHeadPosition ();
		while (iPos)
			pListbox->AddString (sList.GetNext (iPos));
	}
}

// -------------------------------------------------------------------------
// DDX_CEditStringList -- performs dialog-data-exchange between a CEdit and
// a CStringList
void DDX_CEditStringList(CDataExchange* pDX, int nIDC, CStringList &sList)
{
	CEdit *pEdit = (CEdit *) pDX->m_pDlgWnd->GetDlgItem(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		sList.RemoveAll();
		int iLines = pEdit->GetLineCount();
		char rchLine[998];
		for (int i = 0; i < iLines; i++)
		{
			// RLW - This is used for custom headers, so adjusted the max length of custom headers to 998 bytes
			// (USEFOR v12)
			memset(rchLine, 0, 998);
			if (pEdit->GetLine(i, rchLine, 997) < 1)
				continue;
			CString strLine = rchLine;
			sList.AddTail(strLine);
		}
	}
	else
	{
		// clear the current contents
		pEdit->SetSel(0, -1);
		pEdit->Clear();

		POSITION iPos = sList.GetHeadPosition();
		while (iPos)
		{
			CString str = sList.GetNext(iPos);
			pEdit->ReplaceSel(str);
			pEdit->ReplaceSel("\r\n");
		}
	}
}

// -------------------------------------------------------------------------
// EscapeString -- takes a string and escapes the metacharacters used in
// regular-expression search
CString EscapeString (const CString &strInput)
{
	CString strOutput;
	int iLen = strInput.GetLength ();
	CString strMetachars = "()[]{}<>|+*.,\\^-$~";
	int iMetachars = strMetachars.GetLength ();
	for (int i = 0; i < iLen; i++) {
		for (int j = 0; j < iMetachars; j++)
			if (strInput [i] == strMetachars [j])
				strOutput += '\\';
		strOutput += strInput [i];
	}

	return strOutput;
}

// -------------------------------------------------------------------------
// RegisterSimilarDialogClass -- copies the class data and registers a
//   new class. The beauty of this is we can have different icons per class
BOOL RegisterSimilarDialogClass (
								 UINT   idClassnameStr,
								 UINT   idIcon
								 )
{
	CString newClassName;
	VERIFY ( newClassName.LoadString ( idClassnameStr ) );

	// Get data for old class
	WNDCLASS wc;

	// use null to access normal dialog class
	VERIFY( GetClassInfo(NULL, MAKEINTRESOURCE(0x8002), &wc) );

	// Change name & icon
	wc.lpszClassName = newClassName;
	wc.style         &= ~CS_GLOBALCLASS;
	wc.hIcon         = AfxGetApp()->LoadIcon (idIcon);
	ASSERT(wc.hIcon);
	BOOL bSuccess = AfxRegisterClass(&wc) ;
	ASSERT( bSuccess );
	return bSuccess;
}

// -------------------------------------------------------------------------
// fnFetchBody -- gets the body either from the database or from the newsfeed.
// Returns 0 for success, and non-0 for failure
int fnFetchBody (
				 TError     &      sErrorRet,
				 TNewsGroup *      psNG,
				 TArticleHeader *  psHeader,
				 TArticleText *&   psBody,
				 CPoint        &   ptPartID,
				 BOOL              bMarkAsRead,
				 BOOL              bTryFromNewsfeed,
				 BOOL              bEngage /* = FALSE */,
				 CPoint       *    pptLocal /* = NULL */)
{
	int iLines  = psHeader->GetLines ();
	int iArtInt = psHeader->GetArticleNumber ();
	CString strSubject = psHeader->GetSubject();
	int iResult = 1;

	{
		TAutoClose sCloser(psNG);

		// first, Try getting from the database...
		psBody = new TArticleText;
		try
		{
			psNG->LoadBody (iArtInt, psBody);

			if (pptLocal)
				pptLocal->x = pptLocal->y = 1;

			iResult = 0;
		}
		catch(TGroupDBErrors *pE)
		{
			pE->Delete();
		}
		catch(TException *pE)
		{
			pE->Delete();
		}
		catch(CException *pE)
		{
			pE->Delete();
		}

		if (0 == iResult)
			return iResult;
	}

	// delete the body we've allocated
	delete psBody;
	psBody = 0;

	if (bTryFromNewsfeed)
	{
		// Try getting from newsfeed
		iResult = BlockingFetchBody ( sErrorRet,
			psNG->GetName (),
			psNG->m_GroupID,
			strSubject,
			ptPartID,
			iArtInt,
			iLines,
			psBody,
			TRUE /* fPrioPump */,
			(HANDLE) NULL,  // hKillEvent
			bMarkAsRead,
			bEngage);
	}
	else
		iResult = 1;

	return iResult;
}

// -------------------------------------------------------------------------
static void gen_SetStatusWorker (TNewsGroup *         pNG,
								 LONG                 lArticleNum,
								 TStatusUnit::EStatus iBit,
								 BOOL                 bValue /* = TRUE */)
{

	try
	{
		// if we're setting a decode-related bit, first clear all decode-related
		// bits
		if (iBit == TStatusUnit::kDecoded ||
			iBit == TStatusUnit::kQDecode ||
			iBit == TStatusUnit::kDecodeErr)
		{
			pNG->StatusBitSet (lArticleNum, TStatusUnit::kQDecode, 0);
			pNG->StatusBitSet (lArticleNum, TStatusUnit::kDecoded, 0);
			pNG->StatusBitSet (lArticleNum, TStatusUnit::kDecodeErr, 0);
		}

		pNG->StatusBitSet (lArticleNum, iBit, bValue);

		// set the group as "dirty" to make sure our changes are saved
		pNG->SetDirty ();
	}
	catch(TException *pE)
	{
		pE->Delete();
		// probably newsgroup's statvec hasn't been loaded yet... not much we
		// can do about it here...
	}
}

// -------------------------------------------------------------------------
// GenSetStatusBit -- sets the status bit for an article in a newsgroup
void GenSetStatusBit (LONG lGroupID,
					  LONG lArticleNum,
					  TStatusUnit::EStatus iBit,
					  TNewsGroup *pNG /* = NULL */,
					  BOOL bValue /* = TRUE */)
{
	CDWordArray  vArtNums;

	vArtNums.Add (lArticleNum);

	// hand off to big brother function

	GenSetManyStatusBit (lGroupID, vArtNums, iBit, pNG, bValue);
}

// -------------------------------------------------------------------------
// GenSetManyStatusBit -- open the group once and then set the status for
//                        several articles.
//
void GenSetManyStatusBit (LONG                 lGroupID,
						  CDWordArray &        vArticleNum,
						  TStatusUnit::EStatus iBit,
						  TNewsGroup *         pNG /*= NULL*/,
						  BOOL                 bValue /*= TRUE*/)
{
	TServerCountedPtr cpNewsServer;
	BOOL fLocked = FALSE;
	bool fOpened = false;
	TNewsGroupUseLock * pUseLock = 0;

	// if newsgroup wasn't given us, look it up
	if (!pNG)
	{
		pUseLock = new TNewsGroupUseLock (cpNewsServer, lGroupID, &fLocked, pNG);
		if (!fLocked)
		{
			delete pUseLock;
			return;
		}
	}

	bool fDecodeMarksRead = false;

	// user may prefer that Decoded items be marked Read

	if ((bValue && iBit == TStatusUnit::kDecoded) &&
		TRegSwitch::kReadAfterDecode ==
		gpGlobalOptions->GetRegSwitch()->GetDecodeMarksRead())
	{

		// the newsgroup needs to be opened only if we are doing
		//    crosspost management  -amc

		pNG->Open ();   // this is needed for the "bad" form of ReadRangeAdd
		fOpened = true;

		fDecodeMarksRead = true;
	}

	for (int i = 0 ; i < vArticleNum.GetSize(); i++)
	{
		LONG lArticleNum = (LONG) vArticleNum[i];

		gen_SetStatusWorker ( pNG, lArticleNum, iBit, bValue );

		if (fDecodeMarksRead)
			pNG->ReadRangeAdd (lArticleNum);   /* bad form !! */
	}

	if (fOpened)
		pNG->Close ();

	delete pUseLock;
}

// -------------------------------------------------------------------------
// GenSetManyObjStatusBit -- open the group once and then set the status for
//                        several articles.
//
void GenSetManyObjStatusBit (LONG              lGroupID,
							 CPtrArray &          vArtHeader,
							 TStatusUnit::EStatus iBit,
							 TNewsGroup *         pNG /*= NULL*/,
							 BOOL                 bValue /*= TRUE*/)
{
	TServerCountedPtr cpNewsServer;
	BOOL fLocked = FALSE;
	TNewsGroupUseLock * pUseLock = 0;

	// if newsgroup wasn't given us, look it up
	if (!pNG)
	{
		pUseLock = new TNewsGroupUseLock (cpNewsServer, lGroupID, &fLocked, pNG);
		if (!fLocked)
		{
			delete pUseLock;
			return;
		}
	}

	bool fDecodeMarksRead = false;

	// user may prefer that Decoded items be marked Read

	if ((bValue && iBit == TStatusUnit::kDecoded) &&
		TRegSwitch::kReadAfterDecode ==
		gpGlobalOptions->GetRegSwitch()->GetDecodeMarksRead())
	{
		fDecodeMarksRead = true;
	}

	for (int i = 0 ; i < vArtHeader.GetSize(); i++)
	{
		TArticleHeader * pHdr  = (TArticleHeader*) vArtHeader[i];

		gen_SetStatusWorker ( pNG, pHdr->GetNumber(), iBit, bValue );

		// preferred form doesn't need newsgroup to be opened
		if (fDecodeMarksRead)
			pNG->ReadRangeAdd ( pHdr );
	}

	delete pUseLock;
}

// -------------------------------------------------------------------------
static TMdiLayout *GetLayoutWindow ()
{
	BOOL fMax;
	CMainFrame *pMainFrame = (CMainFrame *) genGetMainWnd ();
	if (!pMainFrame)
		return 0;
	TNews3MDIChildWnd *pActiveKid = (TNews3MDIChildWnd *)pMainFrame->MDIGetActive(&fMax);
	if (!pActiveKid)
		return 0;
	TMdiLayout *psLayout = pActiveKid->GetLayoutWnd ();
	return psLayout;
}

// -------------------------------------------------------------------------
// GetNewsView -- returns a pointer to the main CNewsView window, or 0 for
// failure
CNewsView *GetNewsView ()
{
	TMdiLayout *pLayout = GetLayoutWindow ();
	if (!pLayout)
		return 0;
	return pLayout->m_pInitNewsView;
}

// -------------------------------------------------------------------------
// GetThreadView -- returns a pointer to the thread pane, or 0 for failure
TThreadListView *GetThreadView ()
{
	TMdiLayout *pLayout = GetLayoutWindow ();
	if (!pLayout)
		return 0;
	return pLayout->GetThreadView();
}

// -------------------------------------------------------------------------
// IsPaneZoomed -- returns true if any pane is zoomed
bool IsPaneZoomed ()
{
	TMdiLayout * pLayout = GetLayoutWindow ();
	if (!pLayout || !pLayout->IsZoomed())
		return false;
	return true;
}

// -------------------------------------------------------------------------
// CenterListboxSelection -- tries to keep the selection in the
// middle half of the listbox.  Returns 0 for success, 2 for too-small
// -1 for error.  Written for multi-select listboxes
int CenterListboxSelection(CListBox * pLbx, BOOL fMovingDown)
{
	int totSel = pLbx->GetSelCount();
	if (LB_ERR == totSel || 1 != totSel)
		return -1;

	CRect rct;
	CRect rctItem;
	int   idx;
	pLbx->GetSelItems(1, &idx);

	int estCount;
	pLbx->GetClientRect( &rct );
	pLbx->GetItemRect(idx, &rctItem);
	int itemCY = rctItem.Height();

	if (rct.Height() < 5*itemCY)
		return -1;                    // window too small

	estCount = rct.Height() / itemCY;
	if (fMovingDown && rctItem.top > (4*rct.bottom/5))
	{
		// the guy 1/4 above me should the TopIndex
		estCount = estCount / 4;
		totSel = max(idx - estCount + 1, 0);
		pLbx->SetTopIndex (totSel);
	}
	else if (!fMovingDown && rctItem.top < (rct.bottom/4))
	{
		// the guy 3/4 above me should be TopIndex
		estCount = 3*estCount/4;
		totSel = max(idx - estCount + 1, 0);
		pLbx->SetTopIndex (totSel);
	}

	return 0;
}

int CenterListboxSelection(CListCtrl * pLC, BOOL fMovingDown)
{
	int estCount = pLC->GetCountPerPage ();

	if (estCount < 5)
		return -1;     // window too small

	estCount /= 4;

	POSITION pos = pLC->GetFirstSelectedItemPosition ();

	if (NULL == pos)
		return -1;      // no items selected

	int idx = pLC->GetNextSelectedItem (pos);

	if (fMovingDown)
	{
		idx = min (idx + estCount, pLC->GetItemCount() - 1);
		pLC->EnsureVisible ( idx , TRUE );
	}
	else
	{
		idx = max (0, idx - estCount);
		pLC->EnsureVisible ( idx, TRUE );
	}
	return 0;
}

// -------------------------------------------------------------------------
// EnsureVisibleSelection -- make sure the selection is completely visible
//   Returns 0 for success, non-zero for error. Written for multi-select
int EnsureVisibleSelection(CListBox * pLbx)
{
	int totSel = pLbx->GetSelCount();
	if (LB_ERR == totSel || 1 != totSel)
		return -1;

	// make sure it is completely visible
	int cy = pLbx->GetItemHeight(0);
	if (0 == cy)
		return -1;

	CRect rct;
	pLbx->GetClientRect(&rct);
	int visibleSpan = rct.Height() / cy;
	int idx, top;
	pLbx->GetSelItems(1, &idx);

	if (0 == visibleSpan)
	{
		top = idx;
		pLbx->SetTopIndex(top);
	}
	else
	{
		int topIdx = pLbx->GetTopIndex();
		if (idx < topIdx)
			pLbx->SetTopIndex(idx);
		else if (idx >= topIdx && idx < topIdx + visibleSpan)
			;
		else
		{
			top = max(idx - visibleSpan + 1, 0);
			pLbx->SetTopIndex(top);
		}
	}

	return 0;
}

// -------------------------------------------------------------------------
int GetOS()
{
	OSVERSIONINFOEX sVerInfo;
	sVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&sVerInfo);
	if (sVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		return RUNNING_ON_WIN95;
	else if (sVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (sVerInfo.dwMajorVersion >= 6)
		{
			if (sVerInfo.dwMinorVersion == 0)
			{
				return RUNNING_ON_VISTA; // Or Server 2008
			}
			else if (sVerInfo.dwMinorVersion == 1)
			{
				return RUNNING_ON_WIN7; // Or Server 2008 R2
			}
		}
		return RUNNING_ON_WINNT;
	}
	return RUNNING_ON_UNKNOWN;
}

// -------------------------------------------------------------------------
// ListboxAddAdjustHScroll -- returns the new item's index, or <0 for error
int ListboxAddAdjustHScroll (CListBox &box, LPCTSTR pchString,
							 BOOL bCheckListbox /* = FALSE */)
{
	int iResult = box.AddString (pchString);

	CDC *pDC = box.GetDC ();
	if (!pDC) {
		ASSERT (0);
		return -1;
	}

	static CFont font;
	static int iFontInitialized;
	if (!iFontInitialized) {
		// assuming system font... better to get it from the system though
		font.CreatePointFont (8, "MS Sans Serif");
		iFontInitialized = TRUE;
	}
	pDC->SelectObject (&font);

	CSize size = pDC->GetTextExtent (pchString, strlen (pchString));
	box.ReleaseDC (pDC);

	int iStringLen = size.cx + 10;      // +10 for extra slack
	if (bCheckListbox)
		iStringLen += 10;    // more space to account for the check

	int iExtent = box.GetHorizontalExtent ();
	if (iStringLen > iExtent)
		box.SetHorizontalExtent (iStringLen);

	return iResult;
}

// -------------------------------------------------------------------------
void SelectedHeaders (TArticleHeader *pHdr, TArticleText *pText, CString &str)
{
	str = "";

	TCustomArticleView &sCustomView = gpGlobalOptions->GetCustomView ();

	int iTotal = sCustomView.GetSize();
	for (int i = 0; i < iTotal; i++) {
		const TArticleItemView &sItem = sCustomView [i];
		int iID = sItem.GetStringID ();
		if (iID == IDS_TMPL_BODY)
			continue;
		if (iID == IDS_TMPL_BLANK) {
			str += "\r\n";
			continue;
		}
		CString strField;
		ArticleCreateField (pHdr, pText, (WORD) iID, TRUE /* fShowFieldName */,
			strField);
		if (!strField.IsEmpty ())
			str += strField + "\r\n";
	}
}

// -------------------------------------------------------------------------
void CopyCStringList (CStringList &dest, const CStringList &src)
{
	dest.RemoveAll ();
	POSITION pos = src.GetHeadPosition ();
	while (pos) {
		const CString &str = src.GetNext (pos);
		dest.AddTail (str);
	}
}

// -------------------------------------------------------------------------
// EscapeQuotes -- escapes any double-quotes in a string
CString EscapeQuotes (LPCTSTR pch)
{
	CString str;

	int iLen = strlen (pch);
	for (int i = 0; i < iLen; i++)
		if (pch [i] != '"')
			str += pch [i];
		else
			str += CString ("\\\"");

	return str;
}

// -------------------------------------------------------------------------
// UnescapeQuotes -- un-escapes any double-quotes in a string
CString UnescapeQuotes (LPCTSTR pch)
{
	int iLen = strlen (pch);
	CString str;
	LPTSTR pchDest = str.GetBufferSetLength (iLen + 1);
	int iDest = 0;

	for (int i = 0; i < iLen; i++) {
		if (pch [i] == '\\' && i < iLen - 1 && pch [i+1] == '"') {
			pchDest [iDest++] = '"';
			i++;
		}
		else
			pchDest [iDest++] = pch [i];
	}
	pchDest [iDest] = 0;

	str.ReleaseBuffer ();
	return str;
}

// -------------------------------------------------------------------------
void GetLine (const CRichEditCtrl &sEdit, int iLine, CString &strLine)
{
	int iLineIndex = sEdit.LineIndex (iLine);
	int iNextLineIndex = sEdit.LineIndex (iLine + 1);
	if (iLineIndex < 0 || iNextLineIndex < 0) {
		ASSERT (0);
		strLine = "";
		return;
	}
	int iLineLen = iNextLineIndex - iLineIndex;
	// RLW - CRichEditCtrl::GetLine(nLine, pBuf, nLen)
	// wants the buffer to be at least sizeof(int) long cause it copies
	// the buffer length into the first bytes. God knows why cause it then
	// immediately overwrites it. Duh Microsoft!
	LPTSTR pchLine = strLine.GetBuffer(iLineLen + 2 + sizeof(iLineLen));
	sEdit.GetLine (iLine, pchLine, iLineLen + sizeof(iLineLen));
	strLine.ReleaseBuffer(iLineLen);

	// strip off the "\r\n" from the end
	strLine.TrimRight();
}

// -------------------------------------------------------------------------
void CheckIndentation (CRichEditCtrl &sEdit)
{
	int iSpaces = 0;
	CString strLine;
	CString strNewLine;
	int iExistingSpaces;
	char chFirstChar;
	int iLines = sEdit.GetLineCount() - 1;
	for (int i = 0; i < iLines; i++) {
		GetLine (sEdit, i, strLine);
		strLine += "\r\n";
		iExistingSpaces = CountLeadingSpaces (strLine);
		if (strLine.IsEmpty ())
			continue;
		chFirstChar = strLine [iExistingSpaces];
		if (chFirstChar == '(')
			iSpaces += CONDITION_INDENT_INCREMENT;
		if (iExistingSpaces != iSpaces) {
			strNewLine = "";
			for (int j = 0; j < iSpaces; j++)
				strNewLine += " ";
			strNewLine += & ((LPCTSTR) strLine) [iExistingSpaces];
			sEdit.SetSel (sEdit.LineIndex (i), sEdit.LineIndex (i + 1));
			sEdit.ReplaceSel (strNewLine);
		}
		if (chFirstChar == ')')
			iSpaces = max (iSpaces - CONDITION_INDENT_INCREMENT, 0);
	}
}

// -------------------------------------------------------------------------
// CountLeadingSpaces -- tells how many spaces are at the start of a CString
int CountLeadingSpaces (LPCTSTR str)
{
	int i = 0;
	for (i = 0; i < (int) strlen (str) && str [i] == ' '; i++);
	return i;
}

// -------------------------------------------------------------------------
// CountLeadingSpaces -- tells how many spaces are at the start of a CString
int CountLeadingSpaces (const CString &str)
{
	int i = 0;
	for (i = 0; i < str.GetLength () && str [i] == ' '; i++);
	return i;
}

// -------------------------------------------------------------------------
// GetStartupDir -- returns the directory that the program was started from,
// ending in a backslash
CString GetStartupDir ()
{
	CString strDir;
	char rchDir [256];
	HMODULE hModule = GetModuleHandle (NULL);
	int RC = GetModuleFileName (hModule, rchDir, sizeof rchDir);
	if (!RC) {
		// GetModuleFileName() failed
		strDir = ".\\";
		return strDir;
	}

	// strip the filename and copy the path
	for (int i = strlen (rchDir) - 1; i >= 0; i--)
		if (rchDir [i] == '\\') {
			rchDir [i+1] = 0;
			break;
		}
		strDir = rchDir;
		return strDir;
}

// -------------------------------------------------------------------------
// Browse -- lets the user choose a file using the save-as dialog or file-open
// dialog
void Browse (CString &strFile, BOOL bOpen)
{
	CFileDialog dlg (bOpen /* bOpenFileDialog */, NULL /* lpszDefExt */,
		strFile);
	if (dlg.DoModal () != IDOK)
		return;
	strFile = dlg.GetPathName ();
}

// -------------------------------------------------------------------------
// WildmatToRegex - Convert wildmat format to a full blown regular expression
// according to these rules:
//   If not in a character set expression or if the special character
//   is not escaped by a backslash
//     '*' becomes ".*"
//     '?' becomes '.'
//     '.' becomes "\."
//     '+' becomes "\+"
// Note: If a ']' immediately follows a '[', then it is considered
// a literal.  Written by Tony
static BOOL WildmatToRegex (const TCHAR * pSrc, TCHAR * pDest, int buffSize)
{
	const TCHAR *  pCurr       = pSrc;
	int      currDest    = 0;
	BOOL     fInCharSet  = FALSE;
	TCHAR    prev        = 0;
	BOOL     fEscaped    = FALSE;

	while (*pSrc && (currDest < (buffSize - 1)))
	{
		if (fEscaped)
		{
			pDest[currDest++] = *pSrc;
			fEscaped = FALSE;
		}
		else
		{
			switch (*pSrc)
			{
			case '*':
				if (!fInCharSet)
				{
					pDest[currDest++] = '.';
					if (currDest < (buffSize - 1))
						pDest[currDest++] = '*';
				}
				break;

			case '?':
				if (!fInCharSet)
					pDest[currDest++] = '.';
				break;

			case '[':
				pDest[currDest++] = '[';
				if (!fInCharSet)
					fInCharSet = TRUE;
				break;
			case ']':
				pDest[currDest++] = ']';
				if (fInCharSet)
					if ('[' != prev)
						fInCharSet = FALSE;
				break;
			case '\\':
				fEscaped = TRUE;
				break;
			case '^':
			case '.':
			case '+':
			case '(':
			case ')':
			case '|':
			case '$':
				if (!fInCharSet)
					pDest[currDest++] = '\\';

				if (currDest < (buffSize - 1))
					pDest[currDest++] = *pSrc;
				break;

			default:
				pDest[currDest++] = *pSrc;
				break;
			}
		}

		prev = *pSrc;
		pSrc++;
	}

	if (currDest == buffSize)
		return FALSE;
	else
		pDest[currDest] = 0;

	return TRUE;
}

// -------------------------------------------------------------------------
// WildmatToRE -- converts a wildmat to an RE and appends <0> to the start of
// the RE, and appends <~0> to the end of the RE, so that the RE can only
// match the whole string
void WildmatToRE (const CString &strWildmat, CString &strRE,
				  BOOL bMatchAll /* = TRUE */)
{
	int iSize = strWildmat.GetLength () * 2 + 1;
	char *pchTemp = new char [iSize];
	WildmatToRegex (strWildmat, pchTemp, iSize);
	if (bMatchAll)
		strRE = CString ("^") + pchTemp + "$";
	else
		strRE = pchTemp;
	delete [] pchTemp;
}

// -------------------------------------------------------------------------
// IsAProxicomServer -- this is used so much, it should be a function
BOOL IsAProxicomWebServer ()
{
	TServerCountedPtr cpNewsServer;
	return 1 == cpNewsServer->ProxicomWebServer ();
}

// -------------------------------------------------------------------------
static int GetNumber (CString &str, int &num)
{
	str.TrimLeft ();
	if (str.IsEmpty ())
		return 1;   // failure
	BOOL bNegative = FALSE;
	while (!str.IsEmpty () && str [0] != ' ') {
		char ch = str [0];
		str = str.Right (str.GetLength () - 1);
		if (ch == '-') // assume '-' only occurs at the start of the string
			bNegative = TRUE;
		else
			num = num * 10 + ch - '0';
	}
	if (bNegative)
		num = -num;

	return 0;   // success
}

// -------------------------------------------------------------------------
int DecomposeSizePos (const CString &strSizePos, int &dx, int &dy, int &x,
					  int &y)
{
	dx = dy = x = y = 0;

	// go through strSizePos and scan the numbers
	CString str = strSizePos;
	if (GetNumber (str, dx) || GetNumber (str, dy) || GetNumber (str, x) ||
		GetNumber (str, y))
		return 1;

	// Win95 uses this configuration for Iconized windows
	if (0 == dx && 0 == dy && 3000 == x && 3000 == y)
	{
		// sometimes the caller uses the X,Y position only, so clean it up.
		x = y = 1;
		dx = 630;
		dy = 480;
	}

	// make sure none of these numbers is negative
	dx = max (dx, 1);
	dy = max (dy, 1);
	x = max (x, 1);
	y = max (y, 1);

	return 0;   // success
}

// -------------------------------------------------------------------------
const CString ComposeSizePos (int dx, int dy, int x, int y)
{
	CString str;
	str.Format ("%d %d %d %d", dx, dy, x, y);
	return str;
}

// -------------------------------------------------------------------------
const CString &DaysTillExpiration (const CTime &sLastSeen, int iExpirationDays)
{
	static CString strResult;
	int iDaysTillExpiration = 0;
	CTime sExpirationTime (sLastSeen);
	CTimeSpan sPeriod (iExpirationDays /* days */,
		0 /* hours */, 0 /* minutes */, 0 /* seconds */);
	sExpirationTime += sPeriod;
	time_t lTimeTillExpiration =
		sExpirationTime.GetTime () - CTime::GetCurrentTime ().GetTime ();
	CTimeSpan sTimeTillExpiration (lTimeTillExpiration);
	if (lTimeTillExpiration > 0) {
		int iDays = sTimeTillExpiration.GetDays ();
		int iHours = sTimeTillExpiration.GetHours ();
		int iMinutes = sTimeTillExpiration.GetMinutes ();
		int iSeconds = sTimeTillExpiration.GetSeconds ();
		iDaysTillExpiration = sTimeTillExpiration.GetDays ();
		if (iHours || iMinutes || iSeconds)
			iDaysTillExpiration ++;
	}
	strResult.Format ("%d", iDaysTillExpiration);
	return strResult;
}

/////////////////////////////////////////////////////////////////////////////
// we probably inserted \r\n at the end of each window line. so this is
// heavy handed but remove them all
CString strip_newline(const CString & in)
{
	CString out;
	int    outLen = 0;
	LPTSTR pOutText = out.GetBuffer(in.GetLength());
	for (int i = 0; i < in.GetLength(); i++)
	{
		TCHAR c = in[i];
		if (c != '\r' && c != '\n')
		{
			*pOutText++ = c;
			outLen++;
		}
	}
	out.ReleaseBuffer(outLen);
	return out;
}

/////////////////////////////////////////////////////////////////////////////
int GetInstallDir (CString & installDir)
{
	CRegKey      rg;
	TPath        path;

	LONG  lRet = rg.Open (HKEY_CURRENT_USER,
		GetGravityRegKey(),
		KEY_READ);

	if (ERROR_SUCCESS == lRet)
	{
		TCHAR   rcDir[MAX_PATH];
		DWORD   dwSize = sizeof(rcDir);

		ZeroMemory (rcDir, dwSize);

		lRet = rg.QueryStringValue (_T("InstallDir"), rcDir, &dwSize);
		if (ERROR_SUCCESS == lRet)
			installDir = rcDir;
	}
	else
		installDir = GetStartupDir();

	return lRet;
}
