/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tbozobin.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.3  2009/02/17 19:31:03  richard_wood
/*  Tidied up classes
/*
/*  Revision 1.2  2008/09/19 14:51:58  richard_wood
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

// tbozobin.cpp -- Bozo Bin dialog

#include "stdafx.h"              // windows API
#include "tbozobin.h"            // this file's prototypes
#include "newsdb.h"              // GetBozoList(), ...
#include "globals.h"             // gpStore
#include "genutil.h"             // DaysTillExpiration(), ...
#include "server.h"              // needed by newsdb.h
#include "rgswit.h"              // TRegSwitch
#include "tglobopt.h"            // gpGlobalOptions
#include "thredlst.h"            // TThreadList
#include "idxlst.h"              // TArticleIndexList
#include "newsview.h"            // CNewsView
#include "uipipe.h"              // gpUIPipe
#include "rules.h"               // Rule, ...
#include "ruleutil.h"            // GetRule(), ...
#include "custmsg.h"             // WMU_BOZO_CONVERTED

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
// the VERSIONABLE_SCHEMA thing is needed because it's being serialized through
// a CObList
IMPLEMENT_SERIAL(TBozo, PObject, VERSIONABLE_SCHEMA | BOZO_VERSION)

// -------------------------------------------------------------------------
void TBozo::Serialize(CArchive &archive)
{
	PObject::Serialize(archive);
	TExpirable::Serialize(archive);

	if (archive.IsStoring())
	{
		archive << m_strName;
		return;
	}

	archive >> m_strName;
	TRACE(m_strName);
	TRACE("\r\n");
	TRACE(this->LastSeen().Format("%c\r\n"));
}

IMPLEMENT_SERIAL(TBozoList, CObList, BOZO_LIST_VERSION)

TBozoList::TBozoList()
{
}

TBozoList::~TBozoList()
{
	RemoveAll();
}

// -------------------------------------------------------------------------
void TBozoList::RemoveAll()
{
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		TBozo *pItem = (TBozo *) CObList::GetNext(pos);
		delete pItem;
	}
	CObList::RemoveAll();
}

// -------------------------------------------------------------------------
BOOL TBozoList::BozoExists(const CString &strBozo)
{
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		TBozo *pItem = (TBozo *) CObList::GetNext(pos);
		if (!pItem->m_strName.CompareNoCase(strBozo))
			return TRUE;
	}
	return FALSE;
}

// -------------------------------------------------------------------------
// TBozoBin -- constructor
TBozoBin::TBozoBin(CWnd* pParent /*=NULL*/)
	: CDialog(TBozoBin::IDD, pParent)
{
	m_strNames = _T("");
	m_bPurge = FALSE;
	m_iPurgeDays = 0;
}

// -------------------------------------------------------------------------
// DoDataExchange -- transfers data to/from dialog
void TBozoBin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PURGE_DAYS, m_sPurgeDays);
	DDX_Control(pDX, IDC_EDIT, m_sEdit);
	DDX_Control(pDX, IDC_NAMES, m_sNames);
	DDX_Control(pDX, IDC_DELETE, m_sDelete);
	DDX_Control(pDX, IDC_ADD, m_sAdd);
	DDX_Check(pDX, IDC_PURGE, m_bPurge);
	DDX_Text(pDX, IDC_PURGE_DAYS, m_iPurgeDays);
	DDV_MinMaxUInt(pDX, m_iPurgeDays, 1, 999);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TBozoBin, CDialog)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_NAMES, OnItemchangedNames)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_NAMES, OnDblclkNames)
	ON_BN_CLICKED(IDC_PURGE, OnPurge)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TBozoBin::ShowColumn2(int iIndex)
{
	TBozo *pBozo = (TBozo *) m_sNames.GetItemData(iIndex);

	CString str = "-";
	if (gpGlobalOptions->GetRegSwitch()->m_fBozoExpireMRU)
	{
		int iExpirationDays =
			gpGlobalOptions->GetRegSwitch()->m_iBozoExpirationDaysMRU;
		str = DaysTillExpiration(pBozo->LastSeen(), iExpirationDays);
	}
	m_sNames.SetItemText(iIndex, 1 /* column */, str);
}

// -------------------------------------------------------------------------
void TBozoBin::AddRow(LPCTSTR pchName, const CTime &sLastSeen,
					   int iIndex /* = -1 */)
{
	if (iIndex == -1)
		iIndex = m_sNames.GetItemCount();
	iIndex = m_sNames.InsertItem(iIndex, pchName);
	m_sNames.EnsureVisible(iIndex, FALSE /* bPartialOK */);

	TBozo *pNew = new TBozo;
	pNew->m_strName = pchName;
	pNew->SetLastSeen(sLastSeen.GetTime());
	m_sNames.SetItemData(iIndex, (DWORD) pNew);
	ShowColumn2(iIndex);
}

// -------------------------------------------------------------------------
// OnAdd -- add a phrase to the phrase list
void TBozoBin::OnAdd()
{
	TBozoEdit sDlg;
	if (sDlg.DoModal() != IDOK)
		return;

	LV_FINDINFO sFind;
	sFind.flags = LVFI_STRING;
	sFind.psz = sDlg.m_strName;
	if (m_sNames.FindItem(&sFind) != -1)
	{
		CString str; str.LoadString(IDS_DUPLICATE_BOZO);
		MessageBox(str);
		return;
	}

	AddRow(sDlg.m_strName, CTime::GetCurrentTime());
}

// -------------------------------------------------------------------------
void TBozoBin::OnEdit() 
{
	// find the selected item
	int iIndex = -1;
	int iSize = m_sNames.GetItemCount();
	for (int i = 0; i < iSize; i++)
	{
		if (m_sNames.GetItemState(i, LVIS_SELECTED))
		{
			iIndex = i;
			break;
		}
	}
	if (iIndex == -1)
		return;

	TBozo *pBozo = (TBozo *) m_sNames.GetItemData(iIndex);
	TBozoEdit sDlg;
	sDlg.m_strName = pBozo->m_strName;

	if (sDlg.DoModal() != IDOK)
		return;

	// if name has changed, make sure it's unique
	if (sDlg.m_strName.CompareNoCase(pBozo->m_strName))
	{
		LV_FINDINFO sFind;
		sFind.flags = LVFI_STRING;
		sFind.psz = sDlg.m_strName;
		if (m_sNames.FindItem(&sFind) != -1)
		{
			CString str; str.LoadString(IDS_DUPLICATE_BOZO);
			MessageBox(str);
			return;
		}
	}

	m_sNames.DeleteItem(iIndex);
	delete pBozo;
	AddRow(sDlg.m_strName, CTime::GetCurrentTime(), iIndex);
}

// -------------------------------------------------------------------------
// OnDelete -- delete a phrase from one of the "Subject" or "From" phrase lists
void TBozoBin::OnDelete()
{
	int iLen = m_sNames.GetItemCount();
	for (int i = iLen - 1; i >= 0; i--)
	{
		if (m_sNames.GetItemState(i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			TBozo *pBozo = (TBozo *) m_sNames.GetItemData(i);
			delete pBozo;
			m_sNames.DeleteItem(i);

			// select something else
			int iIndex = i;
			if (iIndex > m_sNames.GetItemCount() - 1)
				iIndex --;
			m_sNames.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	m_sNames.SetFocus();
	GrayControls();
}

// -------------------------------------------------------------------------
BOOL TBozoBin::OnInitDialog()
{
	m_iPurgeDays = gpGlobalOptions->GetRegSwitch()->m_iBozoExpirationDaysMRU;
	m_bPurge = gpGlobalOptions->GetRegSwitch()->m_fBozoExpireMRU;

	CDialog::OnInitDialog();   // performs DDX

	// initialize list control
	CRect sRect;
	m_sNames.GetClientRect(&sRect);
	int iTotalWidth = sRect.right - sRect.left;
	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	m_sNames.InsertColumn(0 /* column */, strHeader, LVCFMT_LEFT,
		iTotalWidth - 85 - 4 - GetSystemMetrics(SM_CXVSCROLL) /* width */,
		0 /* subitem */);
	strHeader.LoadString(IDS_DAYS_LEFT_HDR);
	m_sNames.InsertColumn(1 /* column */, strHeader, LVCFMT_LEFT,
		85 /* width */, 1 /* subitem */);

	// add bozos to list control
	TBozoList &sBozos = gpStore->GetBozoList();
	POSITION pos = sBozos.GetHeadPosition();
	while (pos)
	{
		TBozo &sBozo = sBozos.GetNext(pos);
		AddRow(sBozo.m_strName, sBozo.LastSeen());
	}

	// set initial enabled state
	GrayControls();

	return FALSE; // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TBozoBin::OnOK()
{
	CDialog::OnOK();  // performs DDX

	// save bozos to global list
	TBozoList &sBozos = gpStore->GetBozoList();
	sBozos.RemoveAll();

	int iLen = m_sNames.GetItemCount();
	for (int i = 0; i < iLen; i++)
	{
		TBozo *pBozo = (TBozo *) m_sNames.GetItemData(i);
		sBozos.AddTail(*pBozo);
	}

	gpGlobalOptions->GetRegSwitch()->m_iBozoExpirationDaysMRU = m_iPurgeDays;
	gpGlobalOptions->GetRegSwitch()->m_fBozoExpireMRU = m_bPurge;
	gpStore->SaveGlobalOptions();
}

// -------------------------------------------------------------------------
void TBozoBin::OnDestroy() 
{
	int iLen = m_sNames.GetItemCount();
	for (int i = 0; i < iLen; i++)
	{
		TBozo *pBozo = (TBozo *) m_sNames.GetItemData(i);
		delete pBozo;
	}
	CDialog::OnDestroy();
}

// -------------------------------------------------------------------------
void TBozoBin::OnItemchangedNames(NMHDR*, LRESULT* pResult) 
{
	GrayControls();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TBozoBin::GrayControls()
{
	UpdateData();

	int iSelected = m_sNames.GetSelectedCount();
	m_sDelete.EnableWindow(iSelected);
	m_sEdit.EnableWindow(iSelected == 1);
	m_sPurgeDays.EnableWindow(m_bPurge);
}

// -------------------------------------------------------------------------
static void AddBozos(const CStringList &rstrBozos)
{
	// save bozos to global list
	TBozoList &sBozos = gpStore->GetBozoList();

	POSITION pos = rstrBozos.GetHeadPosition();
	while (pos)
	{
		const CString &strBozo = rstrBozos.GetNext(pos);
		if (sBozos.BozoExists(strBozo))
			continue;
		TBozo sBozo;
		sBozo.m_strName = strBozo;
		sBozos.AddTail(sBozo);
	}
}

// -------------------------------------------------------------------------
void TBozoBin::AddBozo(const char *pchBozo)
{
	CStringList rstr;
	rstr.AddTail(pchBozo);
	AddBozos(rstr);
}

// -------------------------------------------------------------------------
static CStringList grstr;
int BozoCallback(TArticleHeader *pHeader, int iSelected, TNewsGroup *pNG,
				  DWORD dwData)
{
	if (!iSelected)
		return 0;
	grstr.AddTail(pHeader->GetFrom());
	return 0;
}
void BozoCallbackCommit()
{
	AddBozos(grstr);
	grstr.RemoveAll();
}

// -------------------------------------------------------------------------
TBozoEdit::TBozoEdit(CWnd* pParent /*=NULL*/)
	: CDialog(TBozoEdit::IDD, pParent)
{

	m_strName = _T("");
}

// -------------------------------------------------------------------------
void TBozoEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Text(pDX, IDC_NAME, m_strName);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TBozoEdit, CDialog)
		ON_EN_CHANGE(IDC_NAME, OnChangeName)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TBozoEdit::GrayControls()
{
	UpdateData();
	m_sOK.EnableWindow(!m_strName.IsEmpty());
}

// -------------------------------------------------------------------------
BOOL TBozoEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	GrayControls();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TBozoEdit::OnChangeName() 
{
	GrayControls();
}

// -------------------------------------------------------------------------
void TBozoEdit::OnOK() 
{
	CDialog::OnOK();  // does data exchange
	gpStore->SaveGlobalOptions();
}

// -------------------------------------------------------------------------
void CheckForBozo(TArticleHeader *pHeader, TNewsGroup *pNG)
{
	TBozoList &sBozos = gpStore->GetBozoList();
	POSITION pos = sBozos.GetHeadPosition();
	CPtrList sRemoveList;
	while (pos)
	{
		POSITION sOriginalPos = pos;
		TBozo &sBozo = sBozos.GetNext(pos);

		// check for expiration
		if (gpGlobalOptions->GetRegSwitch()->m_fBozoExpireMRU)
		{
			int iExpirationDays =
				gpGlobalOptions->GetRegSwitch()->m_iBozoExpirationDaysMRU;
			CTimeSpan sDays(iExpirationDays /* days */,
				0 /* hours */, 0 /* minutes */, 0 /* seconds */);
			CTime sSeenCutoff = CTime::GetCurrentTime() - sDays;
			time_t lCutoff = sSeenCutoff.GetTime();
			if (sBozo.NotSeenSince(lCutoff))
			{
				sRemoveList.AddTail((void *) sOriginalPos);
				continue;
			}
		}

		TSearch &sSearch = sBozo.m_sSearch;

		// compile the score's pattern if needed
		if (!sSearch.HasPattern())
		{
			try
			{
				sSearch.SetPattern(sBozo.m_strName, FALSE /* fCaseSensitive */,
					TSearch::NON_RE);
			}
			catch(...)
			{
				ASSERT(0);
				continue;
			}
		}

		// test for the bozo
		int iResultLen;
		DWORD iPos;
		if (sSearch.Search(pHeader->GetFrom(), iResultLen, iPos))
		{
			// dirties group automatically
			pNG->ReadRangeAdd(pHeader);

			// mark bozo entry as seen
			TRACE("Bozo Seen : %s, Time : %s\r\n",
				sBozo.m_strName,
				sBozo.LastSeen().Format("%c\r\n"));
			sBozo.Seen();
		}
	}

	// remove expired bozo entries
	pos = sRemoveList.GetHeadPosition();
	while (pos)
	{
		POSITION sTempPos = (POSITION) sRemoveList.GetNext(pos);
		sBozos.RemoveAt(sTempPos);
	}
}

// -------------------------------------------------------------------------
void ApplyBozoToCurrentGroup()
{
	CNewsView *pView = GetNewsView();
	LONG lGroup = pView->GetCurNewsGroupID();

	// initialize pNG
	TNewsGroup *pNG = 0;
	BOOL fUseLock;
	TServerCountedPtr cpNewsServer;
	TNewsGroupUseLock(cpNewsServer, lGroup, &fUseLock, pNG);
	if (!fUseLock)
	{
		ASSERT(0);
		return;
	}

	CWaitCursor wait;

	pNG->Open();
	pNG->WriteLock();

	// initialize group's info
	TThreadList *psThreadList = 0;// thread-list to go through

	// if this newsgroup's thread-list is already constructed, use it.
	// Otherwise, make our own threadlist
	TArticleIndexList sIndexList; // index of articles to go through
	if (pNG->FlatListLength())
	{
		psThreadList = pNG->GetpThreadList();
		psThreadList->CreateArticleIndex(&sIndexList);
	}
	else
	{
		pNG->LoadForArticleIndex(TNewsGroup::GetpCurViewFilter(),
			FALSE /* fCreateStati */, TRUE /* fHitServer */, &sIndexList);
	}

	POSITION pos = sIndexList->GetHeadPosition();
	while (pos)
	{
		TArticleHeader *pHdr = sIndexList->GetNext(pos);
		CheckForBozo(pHdr, pNG);
	}

	// must be done before closing the group, because closing the group will
	// free the headers
	sIndexList.Empty();

	pNG->UnlockWrite();
	pNG->Close();

	pView->RefreshCurrentNewsgroup();

	// tell the UI to refresh this group's read count on-screen
	gpUIPipe->NewRedrawGroup(pNG->m_GroupID);
}

// -------------------------------------------------------------------------
void ConvertBozos()
{
	// get the list of names from the bozo rule, disable the rule, and add
	// the bozos to the data structure

	CString strRule = "Bozo Rule";
	Rule *pRule = GetRule(strRule);
	if (!pRule || !pRule->bEnabled)
		return;

	CStringList rstr;
	AddCondToStringList(pRule, ADD_COND_SUBJECT, rstr);
	AddBozos(rstr);

	GetGlobalRules()->WriteLock();
	pRule->bEnabled = FALSE;
	GetGlobalRules()->UnlockWrite();
	gpStore->SaveRules();

	// inform user
	AfxGetMainWnd()->PostMessage(WMU_BOZO_CONVERTED);
}

// -------------------------------------------------------------------------
void TBozoBin::OnDblclkNames(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnEdit();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TBozoBin::OnPurge() 
{
	GrayControls();  // does DDX

	gpGlobalOptions->GetRegSwitch()->m_iBozoExpirationDaysMRU = m_iPurgeDays;
	gpGlobalOptions->GetRegSwitch()->m_fBozoExpireMRU = m_bPurge;

	// show correct expiration info for rows
	int iLen = m_sNames.GetItemCount();
	for (int i = 0; i < iLen; i++)
		ShowColumn2(i);
}
