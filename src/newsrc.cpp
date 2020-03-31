/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsrc.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

// newsrc.cpp -- newsrc stuff

#include "stdafx.h"              // precompiled header
#include "resource.h"            // needed by newsrc.h
#include "newsrc.h"              // this file's prototypes
#include "genutil.h"             // GetStartupDir(), ...
#include "tglobopt.h"            // gpGlobalOptions
#include "server.h"              // TNewsServer, ...
#include "tasker.h"              // gpTasker
#include "news.h"                // CNewsApp
#include "newsdoc.h"             // CNewsDoc
#include "newsview.h"            // TGroupIDPair, ...
#include "kidsecgt.h"            // TKidSecurityGetPwd

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
CString GetDefaultNewsrc ()
{
	TServerCountedPtr cpNewsServer;

	// default newsrc location is in the server's database directory
	CString strFile = cpNewsServer -> GetServerDatabasePath ();

	// make sure path is null-terminated
	if (!strFile.IsEmpty () && strFile [strFile.GetLength () - 1] != '\\')
		strFile += "\\";

	strFile += "newsrc";
	return strFile;
}

// -------------------------------------------------------------------------
TExportNewsrc::TExportNewsrc(CWnd* pParent /*=NULL*/)
: CDialog(TExportNewsrc::IDD, pParent)
{

	m_strFile = _T("");
	m_bSubscribedOnly = FALSE;
}

// -------------------------------------------------------------------------
void TExportNewsrc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Text(pDX, IDC_FILE, m_strFile);
	DDX_Check(pDX, IDC_SUBSCRIBED_ONLY, m_bSubscribedOnly);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TExportNewsrc, CDialog)
		ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_FILE, OnChangeFile)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TExportNewsrc::OnBrowse()
{
	UpdateData ();
	Browse (m_strFile, FALSE /* bOpen */);
	UpdateData (FALSE /* bSave */);
	GreyButtons ();
}

// -------------------------------------------------------------------------
BOOL TExportNewsrc::OnInitDialog()
{
	TServerCountedPtr cpNewsServer;

	m_strFile = cpNewsServer -> GetExportFile ();
	m_bSubscribedOnly = cpNewsServer -> GetExportSubscribedOnly ();
	CDialog::OnInitDialog();
	GreyButtons ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TExportNewsrc::GreyButtons ()
{
	UpdateData ();
	m_sOK.EnableWindow (!m_strFile.IsEmpty ());
}

// -------------------------------------------------------------------------
void TExportNewsrc::OnChangeFile()
{
	GreyButtons ();
}

// -------------------------------------------------------------------------
void TExportNewsrc::OnOK()
{
	UpdateData ();
	if (ExportNewsrc (m_strFile, m_bSubscribedOnly))
		return;
	CDialog::OnOK();
}

// -------------------------------------------------------------------------
TImportNewsrc::TImportNewsrc(CWnd* pParent /*=NULL*/)
: CDialog(TImportNewsrc::IDD, pParent)
{

	m_strFile = _T("");
}

// -------------------------------------------------------------------------
void TImportNewsrc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Text(pDX, IDC_FILE, m_strFile);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TImportNewsrc, CDialog)
		ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_FILE, OnChangeFile)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TImportNewsrc::OnBrowse()
{
	UpdateData ();
	Browse (m_strFile, TRUE /* bOpen */);
	UpdateData (FALSE /* bSave */);
	GreyButtons ();
}

// -------------------------------------------------------------------------
BOOL TImportNewsrc::OnInitDialog()
{
	TServerCountedPtr cpNewsServer;

	m_strFile = cpNewsServer -> GetImportFile ();
	CDialog::OnInitDialog();
	GreyButtons ();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TImportNewsrc::GreyButtons ()
{
	UpdateData ();
	m_sOK.EnableWindow (!m_strFile.IsEmpty ());
}

// -------------------------------------------------------------------------
void TImportNewsrc::OnChangeFile()
{
	GreyButtons ();
}

// -------------------------------------------------------------------------
void TImportNewsrc::OnOK()
{
	UpdateData ();
	if (ImportNewsrc (m_strFile))
		return;
	CDialog::OnOK();
}

// -------------------------------------------------------------------------
// ExportNewsrcUseDefaults -- exports a newsrc file. Returns 0 for success,
// non-0 for failure
int ExportNewsrcUseDefaults ()
{
	TServerCountedPtr cpNewsServer;

	CString strFile = cpNewsServer -> GetExportFile ();
	BOOL bSubscribedOnly = cpNewsServer -> GetExportSubscribedOnly ();
	return ExportNewsrc (strFile, bSubscribedOnly);
}

// -------------------------------------------------------------------------
// ImportNewsrcUseDefaults -- imports a newsrc file. Returns 0 for success,
// non-0 for failure
int ImportNewsrcUseDefaults ()
{
	TServerCountedPtr cpNewsServer;

	CString strFile = cpNewsServer -> GetImportFile ();
	return ImportNewsrc (strFile);
}

// -------------------------------------------------------------------------
static void WriteSubscribedGroup (CStdioFile &sFile, TNewsGroup *pNG)
{
	CString strLine = pNG -> GetName () + ": ";

	TServerCountedPtr cpNewsServer;

	TRangeSetReadLock sLock (cpNewsServer, pNG -> GetName ());
	TRangeSet *pRangeSet = sLock.m_pRangeSet;

	ASSERT (RangeSetOK (*pRangeSet));

	if (!pRangeSet)
		strLine += "0";      // single "0" if range set is empty
	else {
		BOOL bFirst = TRUE;
		int iRanges = pRangeSet -> RangeCount ();

		if (!iRanges)
			strLine += "0";

		for (int j = 0; j < iRanges; j++) {
			if (!bFirst)
				strLine += ",";
			bFirst = FALSE;

			int low, high;
			pRangeSet -> GetRange (j, low, high);
			CString strAdd;
			if (low == high)
				strAdd.Format ("%d", low);
			else
				strAdd.Format ("%d-%d", low, high);

			strLine += strAdd;
		}
	}

	strLine += "\n";
	sFile.WriteString (strLine);
}

// -------------------------------------------------------------------------
// ExportNewsrc -- exports a newsrc file. Returns 0 for success, non-0 for
// failure
int ExportNewsrc (LPCTSTR pchFile, BOOL bSubscribedOnly)
{
	CWaitCursor wait;
	int iResult = 0;

	TServerCountedPtr cpNewsServer;

	try {
		CStdioFile sFile (pchFile, CFile::modeCreate | CFile::modeWrite);

		TNewsGroupArray &rsSubscribed = cpNewsServer -> GetSubscribedArray ();
		TNewsGroupArrayReadLock ngMgr (rsSubscribed);

		// write out the subscribed groups
		int iNum = rsSubscribed -> GetSize ();
		int i = 0;
		for (i = 0; i < iNum; i++) {
			TNewsGroup *pNG = rsSubscribed [i];
			WriteSubscribedGroup (sFile, pNG);
		}

		if (!bSubscribedOnly) {
			// write out the unsubscribed groups
			TGroupList *pGroups = cpNewsServer -> LoadServerGroupList ();
			iNum = pGroups -> NumGroups ();
			for (i = 0; i < iNum; i++) {
				WORD wArticles;
				TGlobalDef::ENewsGroupType iType;
				CString strLine; pGroups -> GetItem (i, strLine, wArticles, iType);

				// if subscribed to this group, ignore it
				if (rsSubscribed.Exist (strLine))
					continue;

				strLine += CString ("! 0\n");
				sFile.WriteString (strLine);
			}
			delete pGroups;
		}

		sFile.Close ();
	}
	catch(...) {
		MsgResource (IDS_ERR_EXPORT_NEWSRC);
		iResult = 1;
	}

	return iResult;
}

// -------------------------------------------------------------------------
static void UnsubscribeFromGroup (LPCTSTR pchGroup)
{
	CPtrArray sTemp;
	TServerCountedPtr cpNewsServer;

	{
		BOOL fUseLock;
		TNewsGroup *pNG;
		TNewsGroupUseLock useLock (cpNewsServer, pchGroup, &fUseLock, pNG);
		if (!fUseLock)
			return;
		sTemp.Add (new TGroupIDPair (pNG -> m_GroupID, pNG -> GetBestname ()));
	}

	GetNewsView () -> UnsubscribeGroups (sTemp);
	FreeGroupIDPairs (&sTemp);
}

// -------------------------------------------------------------------------
// EnsureUnsubscribed -- returns 0 for success, non-0 for failure
int EnsureUnsubscribed (const CString &strGroup, LPCTSTR pchQuestion,
						TNewsGroupArray *prsSubscribed, BOOL &bUnsubscribeYes, BOOL &bUnsubscribeNo)
{
	if (!prsSubscribed -> Exist (strGroup))
		return 0;

	if (bUnsubscribeNo)
		return 0;

	BOOL bUnsubscribe = bUnsubscribeYes;
	if (!bUnsubscribe) {
		TYesNoDlg dlg;
		dlg.m_strTitle.LoadString (IDS_EVENT_GENERAL);
		dlg.m_strMessage.Format (pchQuestion, strGroup);
		switch (dlg.DoModal ()) {
		 case IDC_YES_ALL:
			 bUnsubscribeYes = TRUE;
			 bUnsubscribe = TRUE;
			 break;
		 case IDC_NO_ALL:
			 bUnsubscribeNo = TRUE;
			 bUnsubscribe = FALSE;
			 break;
		 case IDC_ANSWER_YES:
			 bUnsubscribe = TRUE;
			 break;
		 default:    // includes IDC_ANSWER_NO and IDCANCEL
			 bUnsubscribe = FALSE;
			 break;
		}
	}

	if (!bUnsubscribe)
		return 0;

	// unsubscribe the group
	UnsubscribeFromGroup (strGroup);
	return 0;
}

// -------------------------------------------------------------------------
// CheckPassword -- checks the kid-security password if needed. Returns 0 for
// OK and non-0 for non-OK
static int CheckPassword (BOOL &bPasswordOK)
{
	if (bPasswordOK)
		return 0;

	const CString &strPassword = gpGlobalOptions -> GetSubscribePassword ();
	if (strPassword.IsEmpty())
		return 0;

	BOOL fContinue = FALSE;
	TKidSecurityGetPwd dlg (fContinue, strPassword, AfxGetMainWnd ());
	dlg.DoModal();

	bPasswordOK = (fContinue == TRUE);
	return bPasswordOK ? 0 : 1;
}

// -------------------------------------------------------------------------
// EnsureSubscribed -- returns 0 for success, non-0 for failure
static int EnsureSubscribed (const CString &strGroup,
							 TNewsGroupArray *prsSubscribed, BOOL &bPasswordOK)
{
	if (prsSubscribed -> Exist (strGroup))
		return 0;

	if (CheckPassword (bPasswordOK))
		return 1;

	TServerCountedPtr cpNewsServer;

	// find this group in the big group list, and find out its type (moderated,
	// etc.)
	TGroupList *pGroups = cpNewsServer -> LoadServerGroupList ();
	ASSERT (pGroups);
	int iIndex;
	if (!pGroups -> GroupExist (strGroup, &iIndex)) {
		CString str; str.LoadString (IDS_NEWSRC_NO_GROUP);
		CString strError; strError.Format (str, strGroup);
		AfxGetMainWnd () -> MessageBox (strError);
		delete pGroups;
		return 1;
	}
	TGlobalDef::ENewsGroupType iType;
	WORD wArticles;
	CString dummy; pGroups -> GetItem (iIndex, dummy, wArticles, iType);
	delete pGroups;

	BYTE byStorageMode;
	switch (gpGlobalOptions -> GetStorageMode ()) {
	  case TNewsGroup::kNothing:
		  byStorageMode = STORAGE_MODE_STORENOTHING;
		  break;
	  case TNewsGroup::kStoreBodies:
		  byStorageMode = STORAGE_MODE_STOREBODIES;
		  break;
	  default:    // includes kHeadersOnly
		  byStorageMode = STORAGE_MODE_STOREHEADERS;
	}

	TNewsGroup *pNG = cpNewsServer -> SubscribeGroup (strGroup,
		iType, byStorageMode, cpNewsServer -> GetGoBackArtcount ());

	if (!pNG)
		return 1;

	// notify the app that there is a new group
	CNewsApp *pApp = (CNewsApp *) AfxGetApp ();
	pApp -> AddToNewsgroupList (pNG);

	// if we're connected, queue a job to get the number of messages
	extern TNewsTasker *gpTasker;
	if (gpTasker -> IsConnected ()) {
		CStringList sList;
		sList.AddHead (strGroup);
		gpTasker -> PingList (sList);
	}

	return 0;
}

// -------------------------------------------------------------------------
// MergeReadRanges -- returns 0 for success, non-0 for failure
static int MergeReadRanges (const CString &strGroup, const TRangeSet &sAdd)
{
	TServerCountedPtr cpNewsServer;
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock (cpNewsServer, strGroup, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	pNG -> Open ();
	pNG -> WriteLock ();

	int iCount = sAdd.RangeCount ();
	for (int i = 0; i < iCount; i++) {
		int iLow, iHigh;
		sAdd.GetRange (i, iLow, iHigh);
		for (int j = iLow; j <= iHigh; j++)
			pNG -> ReadRangeAdd (j, KThreadList::kIgnoreCrossPost);
	}

	pNG -> UnlockWrite ();
	pNG -> Close ();
	return 0;
}

// -------------------------------------------------------------------------
static void SwallowChar (CString &line)
{
	line = line.Right (line.GetLength () - 1);
	line.TrimLeft ();
}

// -------------------------------------------------------------------------
static int ReadNum (CString &line)
{
	// find the number's last char
	int pos = 0;
	int iLen = line.GetLength ();
	while (pos < iLen && line [pos] >= '0' && line [pos] <= '9')
		pos ++;

	// take the number out of the string
	CString strNum = line.Left (pos);
	line = line.Right (iLen - pos);
	line.TrimLeft ();

	return atoi (strNum);
}

// -------------------------------------------------------------------------
static void ProcessReadLine (CString &line, TNewsGroupArray &rsSubscribed,
							 BOOL &bUnsubscribeYes, BOOL &bUnsubscribeNo, BOOL &bPasswordOK)
{
	CString strGroup;
	BOOL bSubscribed;
	TRangeSet sRangeSet;

	int pos = 0;
	line.TrimLeft ();

	// skip past group name
	int iLen = line.GetLength ();
	while (pos < iLen &&
		line [pos] != ' ' && line [pos] != ':' && line [pos] != '!')
		pos ++;

	// read group name
	strGroup = line.Left (pos);

	// take out group name
	line = line.Right (iLen - pos);
	line.TrimLeft ();

	// read subscribed or not
	BOOL bEmpty = line.IsEmpty ();
	if (!bEmpty && line [0] == ':')
		bSubscribed = TRUE;
	else if (!bEmpty && line [0] == '!')
		bSubscribed = FALSE;
	else
		return;  // bad line

	// read rest of line only if subscribed
	if (bSubscribed) {
		// take out colon or exclamation point
		SwallowChar (line);

		while (!line.IsEmpty ()) {
			// read number or number-number, then read optional comma

			// read first number
			int iNum1 = ReadNum (line);
			int iNum2 = iNum1;

			// read optional "-number"
			if (!line.IsEmpty () && line [0] == '-') {
				SwallowChar (line);
				iNum2 = ReadNum (line);
			}

			// read optional comma
			if (!line.IsEmpty () && line [0] == ',')
				SwallowChar (line);

			sRangeSet.Add (iNum1, iNum2);
		}

		ASSERT (RangeSetOK (sRangeSet));
	}

	// only things we do are:
	//  1. if a group should be subscribed and it's not, subscribe it
	//  2. add read ranges to subscribed groups
	//  3. if a group should not be subscribed and it is, ask whether to
	//     unsubscribe
	BOOL bError = FALSE;
	if (bSubscribed)
		if (EnsureSubscribed (strGroup, &rsSubscribed, bPasswordOK) ||
			MergeReadRanges (strGroup, sRangeSet))
			bError = TRUE;

	CString strQuestion; strQuestion.LoadString (IDS_NEWSRC_ASK_UNSUBSCRIBE);
	if (!bSubscribed)
		if (EnsureUnsubscribed (strGroup, strQuestion, &rsSubscribed,
			bUnsubscribeYes, bUnsubscribeNo))
			bError = TRUE;

	if (bError) {
		CString str; str.LoadString (IDS_ERR_IMPORT_GROUP);
		CString strError; strError.Format (str, strGroup);
		AfxGetMainWnd () -> MessageBox (strError);
	}
}

// -------------------------------------------------------------------------
// ImportNewsrc -- imports a newsrc file. Returns 0 for success, non-0 for
// failure
int ImportNewsrc (LPCTSTR pchFile)
{
	CWaitCursor wait;
	int iResult = 0;
	BOOL bUnsubscribeYes = FALSE;
	BOOL bUnsubscribeNo = FALSE;
	BOOL bPasswordOK = FALSE;

	TServerCountedPtr cpNewsServer;

	try {
		CStdioFile sFile (pchFile, CFile::modeRead);

		TNewsGroupArray &rsSubscribed = cpNewsServer -> GetSubscribedArray ();
		TNewsGroupArrayWriteLock ngMgr (rsSubscribed);

		CString line;
		while (sFile.ReadString (line))
			ProcessReadLine (line, rsSubscribed, bUnsubscribeYes, bUnsubscribeNo,
			bPasswordOK);

		sFile.Close ();
	}
	catch(...) {
		MsgResource (IDS_ERR_IMPORT_NEWSRC);
		iResult = 1;
	}

	// tell the group window to update the newsgroup names and read counts
	CNewsApp *pApp = (CNewsApp *) AfxGetApp ();
	CNewsDoc *pDoc = pApp -> GetGlobalNewsDoc ();

	// it's possible the document object doesn't exist yet (if we're importing
	// on startup)
	if (pDoc)
		pDoc -> SubscribeGroupUpdate ();

	return iResult;
}

// -------------------------------------------------------------------------
TYesNoDlg::TYesNoDlg(CWnd* pParent /*=NULL*/)
: CDialog(TYesNoDlg::IDD, pParent)
{

	m_strMessage = _T("");
}

// -------------------------------------------------------------------------
void TYesNoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MESSAGE, m_strMessage);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TYesNoDlg, CDialog)
		ON_BN_CLICKED(IDC_ANSWER_NO, OnNo)
	ON_BN_CLICKED(IDC_NO_ALL, OnNoAll)
	ON_BN_CLICKED(IDC_ANSWER_YES, OnYes)
	ON_BN_CLICKED(IDC_YES_ALL, OnYesAll)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TYesNoDlg::OnNo()
{
	EndDialog (IDC_ANSWER_NO);
}

// -------------------------------------------------------------------------
void TYesNoDlg::OnNoAll()
{
	EndDialog (IDC_NO_ALL);
}

// -------------------------------------------------------------------------
void TYesNoDlg::OnYes()
{
	EndDialog (IDC_ANSWER_YES);
}

// -------------------------------------------------------------------------
void TYesNoDlg::OnYesAll()
{
	EndDialog (IDC_YES_ALL);
}

// -------------------------------------------------------------------------
BOOL TYesNoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetWindowText (m_strTitle);
	return TRUE;  // return TRUE unless you set the focus to a control
}
