/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgui.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.6  2009/08/27 15:29:22  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.5  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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
/*  Revision 1.3  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.2  2008/09/19 14:51:46  richard_wood
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

#include "stdafx.h"
#include "resource.h"
#include "mplib.h"
#include "rgui.h"
#include "regutil.h"
#include "statvec.h"
#include "gdiutil.h"
#include "tglobdef.h"
#include "superstl.h"            // istrstream, ...
#include "navigdef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

#define DEFAULT_TREE_COLUMNS "yFyIySyLyDyR"

TRegUI::TRegUI()
{
	default_values();
}
TRegUI::~TRegUI()
{
}

int TRegUI::Load()
{
	DWORD dwAction;
	LONG lRet = my_create_key (&dwAction);
	if (ERROR_SUCCESS != lRet)
	{
		throw(new TException(IDS_REGISTRY_ERROR, kError));
		return 0;
	}
	if (REG_CREATED_NEW_KEY == dwAction)
		default_values();
	else
		read();

	RegCloseKey ( m_hkKey );
	return 0;
}

int TRegUI::Save()
{
	LONG lRet = UtilRegOpenKey(
		GetGravityRegKey()+"UI",  // preprocessor strcat
		&m_hkKey, KEY_WRITE);
	if (ERROR_SUCCESS != lRet)
	{
		DWORD disposition;
		lRet = UtilRegCreateKey(
			GetGravityRegKey()+"UI",  // preprocessor strcat
			&m_hkKey, &disposition);
		if (ERROR_SUCCESS != lRet)
		{
			throw(new TException(IDS_REGISTRY_ERROR, kError));
			return 0;
		}
	}

	write();

	RegCloseKey ( m_hkKey );
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// in Ver1.00 we had an enum
// in Ver1.01 we have a format string
void TRegUI::load_dateformat ()
{
	BYTE buf[80];
	int sz = sizeof(buf);
	LONG val;

	if (ERROR_SUCCESS == rgReadLong("DateFmt", buf, sz, val))
		m_iDateFormat = val;

	try
	{
		rgReadString("DateFmtStr", buf, sz, m_strDateFormat);
	}
	catch (TException *pE)
	{
		pE->Delete();
	}
}

void TRegUI::read()
{
	BYTE buf[32];
	int sz = sizeof(buf);
	LONG lRet;
	LONG val;

	lRet = rgReadLong("Sort", buf, sz, val);
	if (ERROR_SUCCESS == lRet)
	{
		TGlobalDef::EThreadSort eThreadSort = TGlobalDef::EThreadSort(val);

		// validate this
		if (TGlobalDef::kSortDate    == eThreadSort ||
			TGlobalDef::kSortSubject == eThreadSort ||
			TGlobalDef::kSortScore   == eThreadSort ||
			TGlobalDef::kSortAuthor  == eThreadSort )
			m_kThreadSortOrder = eThreadSort;
	}

	lRet = rgReadLong("Within", buf, sz, val);
	if (ERROR_SUCCESS == lRet)
		m_kThreadSortWithin = TGlobalDef::EThreadSortWithin(val);

	lRet = rgReadLong("View", buf, sz, val);
	m_kThreadView = TGlobalDef::EThreadViewType(val);

	lRet = rgReadWord("Filter", buf, sz, m_viewFilter);
	lRet = rgReadLong("TreeIndent", buf, sz, val);
	m_treeIndent = val;
	lRet = rgReadLong("CurGroup",   buf, sz, val);
	m_gid = val;

	try
	{
		lRet = rgReadWord("ThrdZip", buf, sz, m_wShowThreadCollapsed);
	}
	catch (TException *pE)
	{
		pE->Delete();
	}

	load_dateformat ();

	try
	{
		rgReadString("TreeCols", buf, sz, m_TreeColumns);

		// RLW - Error checking - check for duplicate columns
		bool bFixed = false;
		int nPos = m_TreeColumns.Find("R");
		if (nPos != -1)
		{
			while (m_TreeColumns.ReverseFind('R') != nPos)
			{
				m_TreeColumns = m_TreeColumns.Left(m_TreeColumns.GetLength()-2);
				bFixed = true;
			}
		}
		if (bFixed)
			rgWriteString("TreeCols", m_TreeColumns);

		// 11/12/97 -- added score column so we need to add this to the end of
		// the column string
		if (m_TreeColumns.Find("R") == -1)
			m_TreeColumns += "yR";
	}
	catch (TException *pE)
	{
		pE->Delete();
	}

	if (ERROR_SUCCESS == rgReadLong("OptionsPg", buf, sz, val))
		m_iActiveOptionsPage = val;

	if (ERROR_SUCCESS == rgReadLong("DefPtSize", buf, sz, val))
		m_iDefaultPointSize = val;
	if (ERROR_SUCCESS == rgReadLong("HdrPtSize", buf, sz, val))
		m_iHdrCtrlPointSize = val;

	try
	{
		lRet = rgReadWord("OneClkGrp", buf, sz, m_f1ClickGroup);
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		lRet = rgReadWord("OneClkArt", buf, sz, m_f1ClickArt);
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		lRet = rgReadWord("FullHdr", buf, sz, m_fShowFullHeader);
	}
	catch (TException *pE) {pE->Delete();}

	// Read mime types we don't decode
	CString line;
	try
	{
		TCHAR rcMime[80];
		rgReadString("SkipMIME", buf, sz, line);

		m_IgnoreMIMETypes.RemoveAll();
		int len = line.GetLength();
		LPTSTR pBuf = line.GetBuffer(len);
		istrstream inp(pBuf);
		while (inp.getline(rcMime, sizeof rcMime, ','))
		{
			CString s = rcMime;
			s.TrimRight(); s.TrimLeft();
			if (!s.IsEmpty()) {
				s.MakeLower();
				m_IgnoreMIMETypes.AddTail (s);
			}
		}
		line.ReleaseBuffer(len);
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		if (ERROR_SUCCESS == rgReadLong("MaxHiliteSeconds", buf, sz, val))
			m_iMaxHiliteSeconds = val;
	}
	catch (TException *pE) {pE->Delete();}

	m_kThreadSortWithin = TGlobalDef::EThreadSortWithin(val);

	try
	{
		rgReadString("FilterName", buf, sz, m_strFilter);
	}
	catch (TException *pE) {pE->Delete();}

	if (ERROR_SUCCESS == rgReadLong("FilterID", buf, sz, val))
		m_iFilterID = val;

	try
	{
		rgReadString("ScoreMRU", buf, sz, m_strScoreMRU);
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		rgReadWord("ShowQuoted", buf, sz, m_wShowQuotedText);
	}
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadLong("RuleEditIndex", buf, sz, val);
		if (lRet == ERROR_SUCCESS)
			m_iRuleEditIndex = val;
	}
	catch (TException *pE) {pE->Delete();}

	try { rgReadString("FactorySizePos", buf, sz, m_strFactorySizePos); }
	catch (TException *pE) {pE->Delete();}

	try { rgReadString("PrintSizePos", buf, sz, m_strPrintSizePos); }
	catch (TException *pE) {pE->Delete();}

	try { rgReadString("SearchSizePos", buf, sz, m_strSearchSizePos); }
	catch (TException *pE) {pE->Delete();}

	try { rgReadString("OutboxSizePos", buf, sz, m_strOutboxSizePos); }
	catch (TException *pE) {pE->Delete();}

	try { rgReadString("SubscribeSizePos", buf, sz, m_strSubscribeSizePos); }
	catch (TException *pE) {pE->Delete();}

	try { rgReadString("ScoringSizePos", buf, sz, m_strScoringSizePos); }
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadLong ("LastQuickScore", buf, sz, val);
		if (lRet == ERROR_SUCCESS)
			m_iLastQuickScore = val;
	}
	catch (TException *pE) {pE->Delete();}

	rgReadInt ("SeeQuotedLines", buf, sz, m_iShowQuotedMax);

	try { rgReadBool ("PinFilter", buf, sz, m_fPinFilter); }
	catch (TException *pE) {pE->Delete();}

	try
	{
		if (rgReadLong ("PrintLeft", buf, sz, val) == ERROR_SUCCESS)
			m_iPrintLeft = val;
		if (rgReadLong ("PrintRight", buf, sz, val) == ERROR_SUCCESS)
			m_iPrintRight = val;
		if (rgReadLong ("PrintBottom", buf, sz, val) == ERROR_SUCCESS)
			m_iPrintBottom = val;
		if (rgReadLong ("PrintTop", buf, sz, val) == ERROR_SUCCESS)
			m_iPrintTop = val;
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		if (rgReadLong ("PromptGetAll", buf, sz, val) == ERROR_SUCCESS)
			m_iGetHdrCount = val;
		if (rgReadLong ("PromptGetX",   buf, sz, val) == ERROR_SUCCESS)
			m_fGetAllHdrs = val ? true : false;
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		if (rgReadLong ("MaxLinesToShow", buf, sz, val) == ERROR_SUCCESS)
			m_iMaxLines = val;
		if (rgReadLong ("MaxLinesCmd",   buf, sz, val) == ERROR_SUCCESS)
			m_iMaxLinesCmd = val;
	}
	catch (TException *pE) {pE->Delete();}

	try
	{
		if (ERROR_SUCCESS == rgReadLong ("Nav1KR", buf, sz, val))
			m_dwNavig1KeyRead = val;
		if (ERROR_SUCCESS == rgReadLong  ("NavIgn", buf, sz, val))
			m_dwNavigIgnore= val;
		if (ERROR_SUCCESS == rgReadLong  ("NavKA",  buf, sz, val))
			m_dwNavigKillArt= val;
		if (ERROR_SUCCESS == rgReadLong  ("NavKT",  buf, sz, val))
			m_dwNavigKillThrd= val;
		if (ERROR_SUCCESS == rgReadLong  ("NavTag", buf, sz, val))
			m_dwNavigTag= val;
		if (ERROR_SUCCESS == rgReadLong  ("NavWat", buf, sz, val))
			m_dwNavigWatch= val;
	}
	catch (TException *pE) {pE->Delete();}
}

void TRegUI::write()
{
	LONG lRet;
	lRet = rgWriteNum("Sort",   (DWORD) m_kThreadSortOrder);
	lRet = rgWriteNum("Within", (DWORD) m_kThreadSortWithin);
	lRet = rgWriteNum("View",   (DWORD) m_kThreadView);
	lRet = rgWriteNum("Filter", m_viewFilter);
	lRet = rgWriteNum("TreeIndent", m_treeIndent);
	lRet = rgWriteNum("CurGroup", (DWORD) m_gid);
	lRet = rgWriteNum("ThrdZip", (DWORD)m_wShowThreadCollapsed);

	lRet = rgWriteString("TreeCols", m_TreeColumns);

	if (!m_strDateFormat.IsEmpty())
		lRet = rgWriteString("DateFmtStr", m_strDateFormat);

	lRet = rgWriteNum("OptionsPg", m_iActiveOptionsPage);

	lRet = rgWriteNum("DefPtSize", m_iDefaultPointSize);
	lRet = rgWriteNum("HdrPtSize", m_iHdrCtrlPointSize);

	lRet = rgWriteNum("OneClkGrp", m_f1ClickGroup);
	lRet = rgWriteNum("OneClkArt", m_f1ClickArt);
	lRet = rgWriteNum("FullHdr",   m_fShowFullHeader);
	// leave "Header" empty

	CString strMIME;
	POSITION p = m_IgnoreMIMETypes.GetHeadPosition();

	while (p)
	{
		if (!strMIME.IsEmpty())
			strMIME += ",";
		strMIME += m_IgnoreMIMETypes.GetNext(p);
	}
	lRet = rgWriteString("SkipMIME", strMIME);
	lRet = rgWriteNum("MaxHiliteSeconds", m_iMaxHiliteSeconds);
	lRet = rgWriteString("FilterName", m_strFilter);
	lRet = rgWriteNum("FilterID", m_iFilterID);
	lRet = rgWriteString("ScoreMRU", m_strScoreMRU);
	lRet = rgWriteNum ("ShowQuoted", m_wShowQuotedText);
	lRet = rgWriteNum("RuleEditIndex", (DWORD) m_iRuleEditIndex);

	rgWriteString("FactorySizePos", m_strFactorySizePos);
	rgWriteString("PrintSizePos", m_strPrintSizePos);
	rgWriteString("SearchSizePos", m_strSearchSizePos);
	rgWriteString("OutboxSizePos", m_strOutboxSizePos);
	rgWriteString("SubscribeSizePos", m_strSubscribeSizePos);
	rgWriteString("ScoringSizePos", m_strScoringSizePos);

	rgWriteNum ("SeeQuotedLines", (DWORD)m_iShowQuotedMax);

	lRet = rgWriteNum("LastQuickScore", (DWORD) m_iLastQuickScore);

	rgWriteNum("PinFilter", m_fPinFilter);

	rgWriteNum ("PrintLeft", m_iPrintLeft);
	rgWriteNum ("PrintRight", m_iPrintRight);
	rgWriteNum ("PrintTop", m_iPrintTop);
	rgWriteNum ("PrintBottom", m_iPrintBottom);

	rgWriteNum ("PromptGetAll", m_iGetHdrCount);
	rgWriteNum ("PromptGetX",   m_fGetAllHdrs ? 1 : 0);

	rgWriteNum ("MaxLinesToShow",  m_iMaxLines);
	rgWriteNum ("MaxLinesCmd",     m_iMaxLinesCmd);

	rgWriteNum ("Nav1KR",     m_dwNavig1KeyRead);
	rgWriteNum ("NavIgn",     m_dwNavigIgnore);
	rgWriteNum ("NavKA",      m_dwNavigKillArt);
	rgWriteNum ("NavKT",      m_dwNavigKillThrd);
	rgWriteNum ("NavTag",     m_dwNavigTag);
	rgWriteNum ("NavWat",     m_dwNavigWatch);
}

void TRegUI::default_values()
{
	m_kThreadSortOrder  = TGlobalDef::kSortDate;
	m_kThreadSortWithin = TGlobalDef::kSortDateWithin;
	m_kThreadView       = TGlobalDef::kThreadTree;

	m_viewFilter        = TStatusUnit::kNew;
	m_treeIndent        = 32;
	m_gid               = -1;

	m_TreeColumns = DEFAULT_TREE_COLUMNS;
	m_strDateFormat.LoadString (IDS_DATE_FORMAT);
	m_iActiveOptionsPage = 0;

	m_iDefaultPointSize = NEWS32_BASE_FONT_PTSIZE;
	m_iHdrCtrlPointSize = 9;

	m_f1ClickGroup = TRUE;
	m_f1ClickArt   = TRUE;
	m_fShowFullHeader = 0;
	m_wShowQuotedText = 1;

	m_wShowThreadCollapsed = 1;

	m_IgnoreMIMETypes.AddTail("text/plain");
	m_IgnoreMIMETypes.AddTail("application/applefile");
	m_iMaxHiliteSeconds = 3; // max seconds for URL hiliting

	m_strFilter.LoadString (IDS_FLTNAME_UNREAD);
	m_iFilterID = 0;

	m_iRuleEditIndex = 0;

	m_iShowQuotedMax = 5;    // at minimum show X lines of quoted text

	m_iLastQuickScore = 0;

	m_fPinFilter = FALSE;

	m_iPrintRight = m_iPrintLeft = m_iPrintTop = m_iPrintBottom = 25;

	m_iGetHdrCount  = 300;
	m_fGetAllHdrs = FALSE;

	m_iMaxLines     = 0;
	m_iMaxLinesCmd  = MAXLINES_OPEN;

	//                           ThreadedOffline,            ThreadedOnline,         SortedOffline,               SortedOnline
	//
	m_dwNavig1KeyRead=pack_dword(ACTION_VIEWNEXTUNREADLOCAL, ACTION_VIEWNEXTUNREADINTHREAD,  ACTION_VIEWNEXTUNREADLOCAL,  ACTION_VIEWNEXTUNREAD);
	m_dwNavigIgnore  =pack_dword(ACTION_SKIPNEXTUNREAD,      ACTION_SKIPNEXTUNREAD,  ACTION_SKIPNEXTUNREAD,       ACTION_SKIPNEXTUNREAD);
	m_dwNavigKillArt =pack_dword(ACTION_SKIPNEXTUNREAD,      ACTION_SKIPNEXTUNREAD,  ACTION_SKIPNEXTUNREAD,       ACTION_SKIPNEXTUNREAD);
	m_dwNavigKillThrd=pack_dword(ACTION_SKIPNEXTUNREAD,      ACTION_SKIPNEXTUNREAD,  ACTION_SKIPNEXTUNREAD,       ACTION_SKIPNEXTUNREAD);
	m_dwNavigTag     =pack_dword(ACTION_SKIPNEXT,            ACTION_SKIPNEXT,        ACTION_SKIPNEXT,             ACTION_SKIPNEXT      );
	m_dwNavigWatch   =pack_dword(ACTION_SKIPNEXTUNREAD,      ACTION_SKIPNEXTUNREAD,  ACTION_SKIPNEXTUNREAD,       ACTION_SKIPNEXTUNREAD);
}

///////////////////////////////////////////////////////////////////////
int TRegUI::LoadTreehdrCtrlWidths(CString& rStr)
{
	return load_named_header_ctrl ("TreeHdr", rStr);
}

int TRegUI::SaveTreehdrCtrlWidths(const CString& rStr)
{
	return save_named_header_ctrl ("TreeHdr", rStr);
}

///////////////////////////////////////////////////////////////////////
int  TRegUI::LoadTreehdrCtrlZoomWidths(CString& rStr)
{
	return load_named_header_ctrl ("TreeHdrZ", rStr);
}

int  TRegUI::SaveTreehdrCtrlZoomWidths(const CString& rStr)
{
	return save_named_header_ctrl ("TreeHdrZ", rStr);
}

///////////////////////////////////////////////////////////////////////
int TRegUI::save_named_header_ctrl (LPCTSTR value, const CString& rStr)
{
	my_open_for_write();
	int stat = rgWriteString (value, rStr);
	RegCloseKey ( m_hkKey );
	return stat;
}

int TRegUI::load_named_header_ctrl (LPCTSTR value, CString& rStr)
{
	DWORD dwAction;
	LONG lRet = my_create_key(&dwAction);
	if (ERROR_SUCCESS != lRet)
	{
		throw(new TException(IDS_REGISTRY_ERROR, kError));
		return 0;
	}

	BYTE rcBuf[128];
	int stat;
	try
	{
		stat = rgReadString(value, rcBuf, sizeof(rcBuf), rStr);
	}
	catch (TException *pE)
	{
		pE->Delete();
		RegCloseKey ( m_hkKey );
		return -1;
	}
	ASSERT(ERROR_SUCCESS == stat);

	RegCloseKey ( m_hkKey );
	return stat;
}

int TRegUI::LoadEventLogData(CString& rStr)
{
	DWORD dwAction;
	LONG lRet = my_create_key(&dwAction);
	if (ERROR_SUCCESS != lRet) {
		throw(new TException(IDS_REGISTRY_ERROR, kError));
		return 0;
	}
	BYTE rcBuf[64];
	int stat;
	try
	{
		stat = rgReadString("EventLog", rcBuf, sizeof(rcBuf), rStr);
	}
	catch (TException *pE)
	{
		pE->Delete();
		RegCloseKey ( m_hkKey );
		return -1;
	}
	ASSERT(ERROR_SUCCESS == stat);

	RegCloseKey ( m_hkKey );
	return stat;
}

int TRegUI::SaveEventLogData(const CString& rStr)
{
	my_open_for_write();
	int stat = rgWriteString ("EventLog", rStr);
	RegCloseKey ( m_hkKey );
	return stat;
}

LONG TRegUI::my_create_key (DWORD* pdwAction)
{
	LONG lRet = UtilRegCreateKey(
		GetGravityRegKey()+"UI",  // preprocessor strcat
		&m_hkKey,
		pdwAction);
	return lRet;
}

LONG TRegUI::my_open_for_write ()
{
	LONG lRet = UtilRegOpenKey(
		GetGravityRegKey()+"UI",  // preprocessor strcat
		&m_hkKey, KEY_WRITE);
	if (ERROR_SUCCESS != lRet)
	{
		DWORD disposition;
		lRet = my_create_key ( &disposition );
		if (ERROR_SUCCESS != lRet)
		{
			throw(new TException(IDS_REGISTRY_ERROR, kError));
			return 0;
		}
	}
	return lRet;
}

int  TRegUI::GetLastGroupID(void)
{
	return m_gid;
}

void TRegUI::SetLastGroupID(int gid)
{
	m_gid = gid;
}

// -------------------------------------------------------------------------
// LoadUtilDlg -- loads a utility-dialog's dimensions.  Returns 0 for success,
// non-0 otherwise
int TRegUI::LoadUtilDlg (const CString &strDlgName, int &iX, int &iY,
						 BOOL &bMax, BOOL &bMin)
{
	int iReturn = 0;

	DWORD dwAction;
	if (my_create_key (&dwAction) != ERROR_SUCCESS)
		return 1;

	BYTE rchBuf [64];
	LONG lTemp;
	try {
		if (rgReadLong (strDlgName + " X", rchBuf, sizeof (rchBuf), lTemp) !=
			ERROR_SUCCESS) {
				iReturn = 1;
				goto end;
		}
		iX = (int) lTemp;
		if (rgReadLong (strDlgName + " Y", rchBuf, sizeof (rchBuf), lTemp) !=
			ERROR_SUCCESS) {
				iReturn = 1;
				goto end;
		}
		iY = (int) lTemp;
		if (rgReadLong (strDlgName + " max", rchBuf, sizeof (rchBuf), lTemp) !=
			ERROR_SUCCESS) {
				iReturn = 1;
				goto end;
		}
		bMax = lTemp ? TRUE : FALSE;
		if (rgReadLong (strDlgName + " min", rchBuf, sizeof (rchBuf), lTemp) !=
			ERROR_SUCCESS) {
				iReturn = 1;
				goto end;
		}
		bMin = lTemp ? TRUE : FALSE;
	}
	catch (TException *pE)
	{
		pE->Delete();
		iReturn = 1;
	}

end:
	RegCloseKey (m_hkKey);
	return iReturn;
}

// -------------------------------------------------------------------------
// SaveUtilDlg -- saves a utility-dialog's dimensions
void TRegUI::SaveUtilDlg (const CString &strDlgName, const int iX, const int iY,
						  BOOL bMax, BOOL bMin)
{
	my_open_for_write ();
	rgWriteNum (strDlgName + " X", iX);
	rgWriteNum (strDlgName + " Y", iY);
	rgWriteNum (strDlgName + " max", bMax ? 1 : 0);
	rgWriteNum (strDlgName + " min", bMin ? 1 : 0);
	RegCloseKey (m_hkKey);
}

// -------------------------------------------------------------------------
// LoadUtilHeaders -- loads header width values for a utility-dialog.
// Returns 0 for success, non-0 otherwise
int TRegUI::LoadUtilHeaders (const CString &strDlgName, int *riSizes,
							 int iNumSizes)
{
	int iReturn = 1;

	DWORD dwAction;
	if (my_create_key (&dwAction) != ERROR_SUCCESS)
		return 1;

	BYTE rchBuf [64];
	CString data;
	try
	{
		// the string has space delimited integers
		if (0 == rgReadString ( strDlgName + " headers",
			rchBuf, sizeof rchBuf, data ))
		{
			istrstream iss((LPTSTR)(LPCTSTR) data);
			for (int i = 0; i < iNumSizes; ++i)
				iss >> riSizes[i];
			iReturn = 0;
		}
	}
	catch (TException *pE)
	{
		pE->Delete();
		// silent catch
	}

	RegCloseKey (m_hkKey);
	return iReturn;
}

// -------------------------------------------------------------------------
// SaveUtilHeaders -- saves header width values for a utility-dialog
void TRegUI::SaveUtilHeaders (const CString &strDlgName, int *riSizes,
							  int iNumSizes)
{
	my_open_for_write ();

	char szBuf[200];
	ZeroMemory (szBuf, sizeof(szBuf));

	ostrstream oss(szBuf, sizeof(szBuf), ios::out);

	for (int i = 0; i < iNumSizes; i++)
	{
		char szNum [20];
		_itoa (riSizes[i], szNum, 10);
		oss << szNum << " ";
	}

	rgWriteString (strDlgName + " headers", szBuf);

	RegCloseKey (m_hkKey);
}

int  TRegUI::GetTreeColumnsRoles (CString& rStr)
{
	rStr = m_TreeColumns;
	return 0;
}

int  TRegUI::SetTreeColumnsRoles (const CString& rStr)
{
	m_TreeColumns = rStr;
	return 0;
}

void TRegUI::GetActiveOptionsPage (int *piIndex)
{
	*piIndex = m_iActiveOptionsPage;
}

void TRegUI::SetActiveOptionsPage (int  iIndex)
{
	m_iActiveOptionsPage = iIndex;
}

int TRegUI::GetDefaultPointSize()
{ return m_iDefaultPointSize; }

int TRegUI::GetHdrCtrlPointSize()
{ return m_iHdrCtrlPointSize; }

void TRegUI::SetShowFullHeader (BOOL fFull)
{
	m_fShowFullHeader = (WORD) fFull;
}

void TRegUI::SetShowQuotedText (bool fShowQuoted)
{
	m_wShowQuotedText = WORD(fShowQuoted ? 1 : 0);
}

void TRegUI::GetIgnoreMimeTypes (CStringList& input)
{
	input.AddTail ( &m_IgnoreMIMETypes );
}

//-------------------------------------------------------------------------
// Pass in a default value for the string
BOOL TRegUI::GetTreePaneSort(CString& str, LPCTSTR lpszDefault)
{
	int iRet = -1;
	try
	{
		iRet = load_named_header_ctrl("ThreadPaneSort", str);
	}
	catch(...)
	{
		// was not there
	}
	if (ERROR_SUCCESS != iRet)
		str = lpszDefault;
	return TRUE;
}

void TRegUI::SetTreePaneSort(CString& str)
{
	int ret = save_named_header_ctrl("ThreadPaneSort", str);
	ASSERT(ERROR_SUCCESS == ret);
}

void TRegUI::Upgrade(int iCurBuild)
{
	// going from build 464 to build 500
	if (iCurBuild < 500)
	{
		// DateFormat - translate enum value from ver 1.00

		CString str;
		if (TGlobalDef::kFormatDate == m_iDateFormat)
		{
			str.LoadString (IDS_ALTERNATE_DATE_1);
			SetDateFormatStr (str);
		}
		else if (TGlobalDef::kFormatDateTime24 == m_iDateFormat)
		{
			str.LoadString (IDS_ALTERNATE_DATE_2);
			SetDateFormatStr (str);
		}
		else /* if (TGlobalDef::kFormatDateTimePM == m_iDateFormat) */
		{
			str.LoadString (IDS_ALTERNATE_DATE_3);
			SetDateFormatStr (str);
		}
	}
}

// -------------------------------------------------------------------------
// LoadCustomColors -- this is instant Open/Close

#define STR_COLORS_KEY "BkCustClrs"

int TRegUI::LoadCustomColors(int count, COLORREF * pData)
{
	int i;
	ASSERT(16 == count);

	// fill with default white
	for (i = 0; i < count; i++)
		pData[i] = RGB(255,255,255);

	int iRet = 1;
	DWORD dwAction;
	if (my_create_key (&dwAction) != ERROR_SUCCESS)
		return 0;

	BYTE    rcBuf[100];
	CString data;
	try
	{
		if (0 == rgReadString ( STR_COLORS_KEY, rcBuf, sizeof rcBuf, data ))
		{
			// string is space delimited numbers
			istrstream iss((LPTSTR)(LPCTSTR) data);
			int i = 0;
			while (iss && i++ < count)
				iss >> *pData++;
			iRet = 0;
		}
	}
	catch (TException *pE)
	{
		pE->Delete();
		// silent catch
	}

	RegCloseKey (m_hkKey);

	return iRet;
}

// -------------------------------------------------------------------------
// SaveCustomColors -- this is instant Open/Close
int TRegUI::SaveCustomColors(int count, COLORREF * pData)
{
	ASSERT(16 == count);

	my_open_for_write ();

	int sz = 400;
	auto_ptr<TCHAR> sBuf(new TCHAR[sz]);            // Al loves STL
	ZeroMemory (sBuf.get(), sz*sizeof(TCHAR));

	ostrstream oss(sBuf.get(), sz, ios::out);

	for (int i = 0; i < count; i++)
	{
		char rcNum[20];
		_itoa (pData[i], rcNum, 10);
		oss << rcNum << " ";
	}

	rgWriteString (STR_COLORS_KEY, sBuf.get());
	return 0;
}

// ------------------------------------------------------------------------
void TRegUI::SetNavig1KeyRead (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn) {
	m_dwNavig1KeyRead = pack_dword (iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::SetNavigIgnore   (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn) {
	m_dwNavigIgnore = pack_dword (iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::SetNavigKillArt  (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn) {
	m_dwNavigKillArt = pack_dword (iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::SetNavigKillThrd (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn) {
	m_dwNavigKillThrd = pack_dword (iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::SetNavigTag      (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn) {
	m_dwNavigTag = pack_dword (iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::SetNavigWatch    (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn) {
	m_dwNavigWatch = pack_dword (iThrdOff, iThrdOn, iSortOff, iSortOn);
}

void TRegUI::GetNavig1KeyRead (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn) {
	unpack_dword (m_dwNavig1KeyRead, iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::GetNavigIgnore   (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn) {
	unpack_dword (m_dwNavigIgnore, iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::GetNavigKillArt  (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn) {
	unpack_dword (m_dwNavigKillArt, iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::GetNavigKillThrd (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn) {
	unpack_dword (m_dwNavigKillThrd, iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::GetNavigTag      (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn) {
	unpack_dword (m_dwNavigTag, iThrdOff, iThrdOn, iSortOff, iSortOn);
}
void TRegUI::GetNavigWatch    (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn) {
	unpack_dword (m_dwNavigWatch, iThrdOff, iThrdOn, iSortOff, iSortOn);
}

// ------------------------------------------------------------------------
// A is high, b low, c high, d low
DWORD  TRegUI::pack_dword (int a, int b, int c, int d)
{
	WORD wHigh = MAKEWORD(b, a);
	WORD wLow  = MAKEWORD(d, c);

	return (DWORD) MAKELONG(wLow, wHigh);
}

// ------------------------------------------------------------------------
void   TRegUI::unpack_dword (DWORD dwFat, int & a, int & b, int & c, int & d)
{
	WORD wHigh = HIWORD(dwFat);
	WORD wLow  = LOWORD(dwFat);

	a = HIBYTE(wHigh);
	b = LOBYTE(wHigh);
	c = HIBYTE(wLow);
	d = LOBYTE(wLow);
}

