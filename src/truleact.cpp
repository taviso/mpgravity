/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: truleact.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:17  richard_wood
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

// truleact.cpp -- rule actions property page

#include "stdafx.h"              // Windows API, MFC, ...
#include "resource.h"            // ID*
#include "pobject.h"             // for rules.h
#include "mplib.h"               // for rules.h
#include "article.h"             // for rules.h
#include "newsgrp.h"             // for rules.h
#include "rules.h"               // Rule
#include "truleact.h"            // this file's prototypes
#include "dirpick.h"             // DirectoryPicker()

static CString gstrSoundDir;     // current sound directory

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TRuleActions::TRuleActions () : CPropertyPage(TRuleActions::IDD, 0)
{

	m_strAlertText = _T("");
	m_bDecode = FALSE;
	m_strUUDecodeDir = _T("");
	m_strSaveFilename = _T("");
	m_bSaveToFile = FALSE;
	m_bShowAlert = FALSE;
	m_bPlaySound = FALSE;
	m_bForwardTo = FALSE;
	m_strForwardToName = _T("");
	m_bGetBody = FALSE;
	m_bTag = FALSE;
	m_bDiscard = FALSE;
	m_bImportantEnable = FALSE;
	m_bProtectedEnable = FALSE;
	m_bReadEnable = FALSE;
	m_iImportant = -1;
	m_iProtected = -1;
	m_iRead = -1;
	m_bIgnore = FALSE;
	m_bWatch = FALSE;
	m_bBozo = FALSE;
	m_bAddToScore = FALSE;
	m_lScore = 0;
}

// -------------------------------------------------------------------------
void TRuleActions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SCORE, m_sScore);
	DDX_Control(pDX, IDC_DISCARD, m_sDiscard);
	DDX_Control(pDX, IDC_UUDECODE_DIRECTORY, m_sUUDecodeDir);
	DDX_Control(pDX, IDC_FORWARD_TO_NAME, m_sForwardToName);
	DDX_Control(pDX, IDC_SAVE_FILENAME, m_sSaveFilename);
	DDX_Control(pDX, IDC_ALERT_TEXT, m_sAlertText);
	DDX_Control(pDX, IDC_SOUND_TEST, m_sSoundTest);
	DDX_Control(pDX, IDC_SOUND_TO_PLAY, m_sSoundToPlay);
	DDX_Control(pDX, IDC_SOUND_BROWSE, m_sSoundBrowse);
	DDX_Control(pDX, IDC_SAVE_BROWSE, m_sSaveBrowse);
	DDX_Control(pDX, IDC_DECODE_BROWSE, m_sDecodeBrowse);
	DDX_Text(pDX, IDC_ALERT_TEXT, m_strAlertText);
	DDX_Check(pDX, IDC_DECODE, m_bDecode);
	DDX_Text(pDX, IDC_UUDECODE_DIRECTORY, m_strUUDecodeDir);
	DDX_Text(pDX, IDC_SAVE_FILENAME, m_strSaveFilename);
	DDX_Check(pDX, IDC_SAVE_TO_FILE, m_bSaveToFile);
	DDX_Check(pDX, IDC_SHOW_ALERT, m_bShowAlert);
	DDX_Check(pDX, IDC_PLAY_SOUND, m_bPlaySound);
	DDX_Check(pDX, IDC_FORWARD_TO, m_bForwardTo);
	DDX_Text(pDX, IDC_FORWARD_TO_NAME, m_strForwardToName);
	DDX_Check(pDX, IDC_RETRIEVE, m_bGetBody);
	DDX_Check(pDX, IDC_TAG, m_bTag);
	DDX_Check(pDX, IDC_DISCARD, m_bDiscard);
	DDX_Check(pDX, IDC_IMPORTANT_ENABLE, m_bImportantEnable);
	DDX_Check(pDX, IDC_PROTECTED_ENABLE, m_bProtectedEnable);
	DDX_Check(pDX, IDC_READ_ENABLE, m_bReadEnable);
	DDX_Radio(pDX, IDC_IMPORTANT, m_iImportant);
	DDX_Radio(pDX, IDC_PROTECTED, m_iProtected);
	DDX_Radio(pDX, IDC_READ, m_iRead);
	DDX_Check(pDX, IDC_ADD_TO_IGNORE, m_bIgnore);
	DDX_Check(pDX, IDC_ADD_TO_WATCH, m_bWatch);
	DDX_Check(pDX, IDC_AUTHOR_BOZO, m_bBozo);
	DDX_Check(pDX, IDC_ADD_TO_SCORE, m_bAddToScore);
	DDX_Text(pDX, IDC_SCORE, m_lScore);

	DDX_Control (pDX, IDC_IMPORTANT, m_sImportant);
	DDX_Control (pDX, IDC_NORMAL, m_sNormal);
	DDX_Control (pDX, IDC_PROTECTED, m_sProtected);
	DDX_Control (pDX, IDC_DELETABLE, m_sDeletable);
	DDX_Control (pDX, IDC_READ, m_sRead);
	DDX_Control (pDX, IDC_UNREAD, m_sUnread);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleActions, CPropertyPage)
	ON_BN_CLICKED(IDC_SOUND_BROWSE, OnSoundBrowse)
	ON_BN_CLICKED(IDC_SOUND_TEST, OnSoundTest)
	ON_BN_CLICKED(IDC_SAVE_BROWSE, OnSaveBrowse)
	ON_BN_CLICKED(IDC_DECODE_BROWSE, OnDecodeBrowse)
	ON_CBN_SELCHANGE(IDC_SOUND_TO_PLAY, OnChangeSoundToPlay)
	ON_BN_CLICKED(IDC_DISCARD, SetDirty)
	ON_BN_CLICKED(IDC_DECODE, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_ADD_TO_SCORE, OnAddToScore)
	ON_BN_CLICKED(IDC_FORWARD_TO, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_PLAY_SOUND, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_SAVE_TO_FILE, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_SHOW_ALERT, SetDirtyUpdateGrey)
	ON_EN_CHANGE(IDC_SAVE_FILENAME, SetDirty)
	ON_EN_CHANGE(IDC_ALERT_TEXT, SetDirty)
	ON_EN_CHANGE(IDC_FORWARD_TO_NAME, SetDirty)
	ON_EN_CHANGE(IDC_UUDECODE_DIRECTORY, SetDirty)
	ON_BN_CLICKED(IDC_RETRIEVE, SetDirty)
	ON_BN_CLICKED(IDC_TAG, SetDirty)
	ON_BN_CLICKED(IDC_DELETABLE, SetDirty)
	ON_BN_CLICKED(IDC_IMPORTANT_ENABLE, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_NORMAL, SetDirty)
	ON_BN_CLICKED(IDC_PROTECTED, SetDirty)
	ON_BN_CLICKED(IDC_PROTECTED_ENABLE, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_READ, SetDirty)
	ON_BN_CLICKED(IDC_READ_ENABLE, SetDirtyUpdateGrey)
	ON_BN_CLICKED(IDC_UNREAD, SetDirty)
	ON_BN_CLICKED(IDC_IMPORTANT, SetDirty)
	ON_BN_CLICKED(IDC_ADD_TO_IGNORE, SetDirty)
	ON_BN_CLICKED(IDC_ADD_TO_WATCH, SetDirty)
	ON_BN_CLICKED(IDC_AUTHOR_BOZO, SetDirty)
	ON_EN_CHANGE(IDC_SCORE, OnChangeScore)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TRuleActions::UpdateGreyStates ()
{
	UpdateData ();
	m_sSoundToPlay.EnableWindow (m_bPlaySound);
	m_sSoundBrowse.EnableWindow (m_bPlaySound);
	m_sSoundTest.EnableWindow (m_bPlaySound);
	m_sAlertText.EnableWindow (m_bShowAlert);
	m_sSaveFilename.EnableWindow (m_bSaveToFile);
	m_sSaveBrowse.EnableWindow (m_bSaveToFile);
	m_sForwardToName.EnableWindow (m_bForwardTo);
	m_sUUDecodeDir.EnableWindow (m_bDecode);
	m_sDecodeBrowse.EnableWindow (m_bDecode);
	m_sImportant.EnableWindow (m_bImportantEnable);
	m_sNormal.EnableWindow (m_bImportantEnable);
	m_sRead.EnableWindow (m_bReadEnable);
	m_sUnread.EnableWindow (m_bReadEnable);
	m_sProtected.EnableWindow (m_bProtectedEnable);
	m_sDeletable.EnableWindow (m_bProtectedEnable);
	m_sScore.EnableWindow (m_bAddToScore);
}

// -------------------------------------------------------------------------
// FillSoundList -- fills the sound combobox with the list of sounds
static void FillSoundList (CComboBox * box)
{
	box -> BeginWaitCursor();

	// empty the current contents of the combobox, and put in a "(None)" option
	CString str; str.LoadString (IDS_NONE);
	box -> ResetContent ();
	box -> AddString (str);

	// look in the directory in gstrSoundDir for sound files, and display their
	// long-names
	WIN32_FIND_DATA sData;
	CString strSearch = gstrSoundDir + "\\*.*";
	HANDLE hResult = FindFirstFile ((LPCTSTR) strSearch, &sData);
	BOOL bMore;
	if (hResult == INVALID_HANDLE_VALUE)
		goto end;
	do {
		// we want to make sure case-sensitive compares don't mess us up.

		CString  strAltName  = (LPCTSTR) sData.cAlternateFileName;
		CString  strFileName = (LPCTSTR) sData.cFileName;

		strAltName.MakeUpper ();
		strFileName.MakeUpper ();

		// add the file's long-name to the combobox, but only if it's a sound file
		if (strAltName.Find (".WAV") > 0 || strFileName.Find (".WAV") > 0)
		{
			char rchName [100];
			// copy the file's long-name until we encounter a period, a null, or
			// run over the buffer's length
			int i = 0;
			for (i = 0; (i < sizeof (rchName) - 1) && (sData.cFileName[i] != '\0')
				&& (sData.cFileName[i] != '.'); i++)
				rchName [i] = (i == 0 ? toupper (sData.cFileName [i]) :
				tolower (sData.cFileName [i]));
			rchName [i] = '\0';
			box -> AddString ((LPCTSTR) rchName);
		}
		bMore = FindNextFile (hResult, &sData);
	} while (bMore);
	FindClose (hResult);

end:
	box -> EndWaitCursor();
}

// -------------------------------------------------------------------------
// OnInitDialog -- called once to initialize the page
BOOL TRuleActions::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// initialize the sound directory to <windows-dir>\media
	GetDefaultSoundDirectory (gstrSoundDir);

	// add list of sounds to the sound combobox
	FillSoundList ((CComboBox *) GetDlgItem (IDC_SOUND_TO_PLAY));

	DisplayCurrentRule ();
	m_bDirty = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TRuleActions::OnOK ()
{
	if (m_bDirty)
		UpdateRule ();
	CPropertyPage::OnOK();
}

// -------------------------------------------------------------------------
void TRuleActions::DisplayCurrentRule ()
{
	// extract the sound-name and sound-directory from the strSoundToPlay field
	CString strSoundName;
	CString strSoundDir;
	SoundLongnameFromFile (m_psRule -> pathSoundToPlay, strSoundName,
		strSoundDir);
	// if the sound-directory is different from the current one, select the rule's
	// sound-directory
	CComboBox * psSoundBox = (CComboBox *) GetDlgItem (IDC_SOUND_TO_PLAY);
	if (_stricmp ((LPCTSTR) strSoundDir, (LPCTSTR) gstrSoundDir)) {
		gstrSoundDir = strSoundDir;
		FillSoundList (psSoundBox);
	}
	// if the sound-name is null, use "(None)"
	if (strSoundName == "")
		strSoundName.LoadString (IDS_NONE);
	// select the sound in the sound-combobox
	psSoundBox -> SelectString (-1, strSoundName);

	// copy fields from current rule to the dialog's variables
	m_strAlertText = m_psRule -> strAlertText;
	m_strSaveFilename = m_psRule -> strSaveFilename;
	m_strForwardToName = m_psRule -> strForwardToName;
	m_strUUDecodeDir = m_psRule -> strUUDecodeDirectory;

	m_bPlaySound = m_psRule -> bPlaySound;
	m_bShowAlert = m_psRule -> bShowAlert;
	m_bSaveToFile = m_psRule -> bSaveToFile;
	m_bForwardTo = m_psRule -> bForwardTo;
	m_bDecode = m_psRule -> bDecode;
	m_bTag = m_psRule -> bTag;
	m_bGetBody = m_psRule -> bGetBody;
	m_bDiscard = m_psRule -> bDiscard;
	m_bIgnore = m_psRule -> bAddToIgnore;
	m_bWatch = m_psRule -> bAddToWatch;
	m_bBozo = m_psRule -> bAddToBozo;

	m_bImportantEnable = m_psRule -> bImportantEnable;
	m_iImportant = (m_psRule -> bImportant ? 0 : 1);
	m_bProtectedEnable = m_psRule -> bProtectedEnable;
	m_iProtected = (m_psRule -> bProtected ? 0 : 1);
	m_bReadEnable = m_psRule -> bReadEnable;
	m_iRead = (m_psRule -> bRead ? 0 : 1);

	m_bAddToScore = m_psRule -> bAddToScore;
	m_lScore = m_psRule -> lScore;

	// radio buttons must be set explicitly
	m_sImportant.SetCheck (m_iImportant);
	m_sNormal.SetCheck (!m_iImportant);
	m_sProtected.SetCheck (m_iProtected);
	m_sDeletable.SetCheck (!m_iProtected);
	m_sRead.SetCheck (m_iRead);
	m_sUnread.SetCheck (!m_iRead);

	UpdateData (FALSE);
	UpdateGreyStates ();
}

// -------------------------------------------------------------------------
// UpdateRule -- takes a rule, and updates it with this dialog's displayed values
int TRuleActions::UpdateRule ()
{
	UpdateData ();

	// takes the displayed sound name, and attaches it to the current
	// sound-directory to make the sound-file to save
	CString strLongname;
	m_sSoundToPlay.GetLBText (m_sSoundToPlay.GetCurSel (), strLongname);
	SoundFileFromLongname (gstrSoundDir, strLongname,
		m_psRule -> pathSoundToPlay);

	// copy values from controls to the current rule's variables
	m_psRule -> bPlaySound = m_bPlaySound;
	m_psRule -> bShowAlert = m_bShowAlert;
	m_sAlertText.GetWindowText (m_psRule -> strAlertText);
	m_psRule -> bSaveToFile = m_bSaveToFile;
	m_sSaveFilename.GetWindowText (m_psRule -> strSaveFilename);
	m_psRule -> bForwardTo = m_bForwardTo;
	m_sForwardToName.GetWindowText (m_psRule -> strForwardToName);
	m_psRule -> bDecode = m_bDecode;
	m_sUUDecodeDir.GetWindowText (m_psRule -> strUUDecodeDirectory);
	m_psRule -> bTag = m_bTag;
	m_psRule -> bGetBody = m_bGetBody;
	m_psRule -> bDiscard = m_bDiscard;
	m_psRule -> bAddToIgnore = m_bIgnore;
	m_psRule -> bAddToWatch = m_bWatch;
	m_psRule -> bAddToBozo = m_bBozo;
	m_psRule -> bAddToScore = m_bAddToScore;
	m_psRule -> lScore = m_lScore;

	m_psRule -> bImportantEnable = m_bImportantEnable;
	m_psRule -> bImportant = (m_iImportant == 0);
	m_psRule -> bProtectedEnable = m_bProtectedEnable;
	m_psRule -> bProtected = (m_iProtected == 0);
	m_psRule -> bReadEnable = m_bReadEnable;
	m_psRule -> bRead = (m_iRead == 0);

	return 0;
}

// -------------------------------------------------------------------------
void TRuleActions::OnSoundBrowse()
{
	CString strDir = gstrSoundDir;

	if (!DirectoryPicker (this, &strDir))
		return;

	// fill the combobox with the contents of the newly-selected directory
	gstrSoundDir = strDir;
	FillSoundList (&m_sSoundToPlay);

	// pick the first element of the combobox
	m_sSoundToPlay.SetCurSel (0);
}

// -------------------------------------------------------------------------
void TRuleActions::OnSoundTest()
{
	TPath pathSoundFile;
	CString strLongName;
	m_sSoundToPlay.GetLBText (m_sSoundToPlay.GetCurSel (), strLongName);
	SoundFileFromLongname (gstrSoundDir, strLongName, pathSoundFile);
	PlaySoundFile (pathSoundFile);
}

// -------------------------------------------------------------------------
void TRuleActions::OnSaveBrowse()
{
	CString strExistingFile;
	m_sSaveFilename.GetWindowText (strExistingFile);

	CString str; str.LoadString (IDS_RULE_SAVE_BROWSE_FILES);
	CFileDialog dlg (TRUE, NULL, (LPCTSTR) strExistingFile, OFN_HIDEREADONLY,
		str, this);

	str.LoadString (IDS_RULE_SELECT_SAVE_FILE);
	dlg.m_ofn.lpstrTitle = str;

	int RC = dlg.DoModal ();
	if (RC == IDCANCEL || !RC)
		return;

	// user selected OK
	CString strFile = dlg.GetPathName();
	m_strSaveFilename = strFile;
	UpdateData (FALSE);  // write m_strSaveFilename to its control
	m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
void TRuleActions::OnDecodeBrowse()
{
	CString strExistingDir;
	m_sUUDecodeDir.GetWindowText (strExistingDir);

	CString strDir;
	if (!DirectoryPicker (this, &strDir))
		return;

	m_strUUDecodeDir = strDir;
	UpdateData (FALSE);  // write m_strUUDecodeDir to its control
	m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
void TRuleActions::OnChangeSoundToPlay()
{
	// set the changed-flag only if the new sound is different from the existing
	// one in the rule
	CString strFile;
	TPath path;
	m_sSoundToPlay.GetWindowText (strFile);
	SoundFileFromLongname (gstrSoundDir, strFile, path);
	if (path != m_psRule -> pathSoundToPlay)
		m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
void TRuleActions::SetDirty ()
{
	m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
void TRuleActions::SetDirtyUpdateGrey ()
{
	m_bDirty = TRUE;
	UpdateGreyStates ();
}

// -------------------------------------------------------------------------
void TRuleActions::OnAddToScore()
{
	m_bDirty = TRUE;
	UpdateGreyStates ();
}

// -------------------------------------------------------------------------
void TRuleActions::OnChangeScore()
{
	m_bDirty = TRUE;
}

BOOL TRuleActions::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TRuleActions::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
