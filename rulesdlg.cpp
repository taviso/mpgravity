/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rulesdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/02/19 11:24:17  richard_wood
/*  Re-enabled optimisations in classes newssock.cpp, rulesdlg.cpp, server.cpp and tdecjob.cpp
/*
/*  Revision 1.2  2008/09/19 14:51:48  richard_wood
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

// rulesdlg.cpp -- main rules dialog

#include "stdafx.h"              // windows API, MFC, ...
#include "resource.h"            // ID*
#include "rulesdlg.h"            // this file's prototypes
#include "trulegen.h"            // TRuleGeneral
#include "trulecon.h"            // TRuleConditions
#include "truleact.h"            // TRuleActions
#include "tmrbar.h"              // TManualRuleBar
#include "newsdb.h"              // TNewsDB
#include "server.h"              // TNewsServer
#include "genutil.h"             // MsgResource ()
#include "tglobopt.h"            // gpGlobalOptions, ...
#include "rgui.h"                // TRegUI, ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//#pragma optimize("", off)

// -------------------------------------------------------------------------
TRulesDlg::TRulesDlg(CWnd* pParent /*=NULL*/) : CDialog(TRulesDlg::IDD, pParent)
{
}

// -------------------------------------------------------------------------
void TRulesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_RULE_LIST, m_sRuleList);
	DDX_Control(pDX, IDC_RULE_DOWN, m_sMoveDown);
	DDX_Control(pDX, IDC_RULE_UP, m_sMoveUp);
	DDX_Control(pDX, IDC_RENAME, m_sRename);
	DDX_Control(pDX, IDC_ENABLE_DISABLE, m_sEnable);
	DDX_Control(pDX, IDC_EDIT, m_sEdit);
	DDX_Control(pDX, IDC_DELETE, m_sDelete);
	DDX_Control(pDX, IDC_COPY, m_sCopy);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRulesDlg, CDialog)
		ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_ENABLE_DISABLE, OnEnableDisable)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_RULE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_RULE_DOWN, OnMoveDown)
	ON_NOTIFY(NM_DBLCLK, IDC_RULE_LIST, OnDblclkRuleList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RULE_LIST, OnItemchangedRuleList)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
int TRulesDlg::GetCurrentRuleIndex ()
{
   int iLen = m_sRuleList.GetItemCount ();
   for (int i = 0; i < iLen; i++)
      if (m_sRuleList.GetItemState (i, LVIS_SELECTED))
         return i;
   return -1;
}

// -------------------------------------------------------------------------
void TRulesDlg::SetColumn3 (int iIndex, Rule *psRule)
{
   CString str;
   if (psRule -> iExpirationType == Rule::NO_EXPIRATION ||
       !psRule -> bEnabled)
      str = "-";
   else
      str = DaysTillExpiration (psRule -> LastSeen (),
         psRule -> iExpirationDays);
   m_sRuleList.SetItemText (iIndex, 2 /* column */, str);
}

// -------------------------------------------------------------------------
void TRulesDlg::AddToListbox (Rule *psRule, int iSetFocus /* = FALSE */,
   int iPos /* = -1 */)
{
   // add item
   int iIndex = iPos == -1 ? m_sRuleList.GetItemCount () : iPos;
   LV_ITEM sItem;
   sItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
   sItem.iItem = iIndex;
   sItem.iSubItem = 0;
   sItem.state = iSetFocus ? LVIS_SELECTED | LVIS_FOCUSED : 0;
   sItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
   sItem.pszText = (LPTSTR) (LPCTSTR) psRule -> strRuleName;
   sItem.cchTextMax = psRule -> strRuleName.GetLength ();
   sItem.lParam = (DWORD) psRule;
   m_sRuleList.InsertItem (&sItem);

   // set column 2
   CString str; str.LoadString (psRule -> bEnabled ? IDS_YES : IDS_NO);
   m_sRuleList.SetItemText (iIndex, 1 /* column */, str);

   // set column 3
   SetColumn3 (iIndex, psRule);
}

// -------------------------------------------------------------------------
Rule *TRulesDlg::GetCurrentRule ()
{
	int iIndex = GetCurrentRuleIndex ();
   Rule *psRule = (Rule *) m_sRuleList.GetItemData (iIndex);
   ASSERT (psRule);
   return psRule;
}

// -------------------------------------------------------------------------
void TRulesDlg::RefreshCurrentLine ()
{
   int iLine = GetCurrentRuleIndex ();
   Rule *psRule = GetCurrentRule ();
   m_sRuleList.DeleteItem (iLine);
   AddToListbox (psRule, TRUE, iLine);
}

// -------------------------------------------------------------------------
void TRulesDlg::OnAdd()
{
	Rule *psRule = new Rule (&m_rsRules);

   // get new rule's name
   if (GetNewName (psRule -> strRuleName) != IDOK) {
      delete psRule;
      return;
      }

   // check for duplicate rule name
   Rule *psTemp;
   if (m_rsRules.Lookup (psRule -> strRuleName, psTemp)) {
      CString str; str.LoadString (IDS_DUPLICATE_RULE_NAME);
      MessageBox (str);
      return;
      }

	// add to rule list
	m_rsRules.SetAt (psRule -> strRuleName, psRule);

	// add to listbox
   AddToListbox (psRule, TRUE /* iSetFocus */);

   // call up the edit dialog
   DoEdit (1 /* condition page */);

   // return focus to the listbox
   m_sRuleList.SetFocus ();

   m_bDirty = TRUE;
   UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRulesDlg::OnCopy()
{
	// create the new rule
   Rule *psRule = GetCurrentRule ();
	Rule *psNewRule = new Rule;
	*psNewRule = *psRule;

   // get new name
   CString strNewName;
   if (GetNewName (strNewName) != IDOK) {
      delete psNewRule;
      return;
      }

   // check for duplicate rule name
   Rule *psTemp;
   if (m_rsRules.Lookup (strNewName, psTemp)) {
      CString str; str.LoadString (IDS_DUPLICATE_RULE_NAME);
      MessageBox (str);
      return;
      }

   psNewRule -> strRuleName = strNewName;

	// add the new rule to the rule hashtable
	m_rsRules.SetAt (psNewRule -> strRuleName, psNewRule);

	// add the new rule to the rule listbox
   AddToListbox (psNewRule, TRUE);

   m_bDirty = TRUE;
   UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRulesDlg::OnDelete()
{
   Rule *psRule = GetCurrentRule ();
   int iIndex = GetCurrentRuleIndex ();
	CString &strRuleName = psRule -> strRuleName;

   // confirm OK to delete
   CString strQ; strQ.LoadString (IDS_QUESTION_MARK);
	CString str; str.LoadString (IDS_RULE_OK_TO_DELETE);
	str += ", \"";
	str += strRuleName + "\" " + strQ;
	int RC = AfxMessageBox (str, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2, 0);
	if (RC == IDCANCEL)
		return;

	// delete from the listbox
	m_sRuleList.DeleteItem (iIndex);

	// delete from the rule-list
	m_rsRules.RemoveKey (strRuleName);

   // delete the rule itself
	delete psRule;

   // return focus to the listbox
   m_sRuleList.SetFocus ();

   // select the rule closest to the one deleted
   if (m_sRuleList.GetItemCount () <= iIndex)
      iIndex --;
   m_sRuleList.SetItemState (iIndex, LVIS_SELECTED | LVIS_FOCUSED,
      LVIS_SELECTED | LVIS_FOCUSED);

   m_bDirty = TRUE;
   UpdateGreyState ();
}

// -------------------------------------------------------------------------
// DoEdit -- brings up the edit dialog and sets the page to iPage. If iPage is
// -1, it sets the page to the last open one
void TRulesDlg::DoEdit (int iPage /* = -1 */)
{
   // find the rule being edited
   Rule *psRule = GetCurrentRule ();

   if (0 == psRule)
      return;

	// create and call an Edit-Rule dialog box
   CString str; str.LoadString (IDS_EDIT_RULES_TITLE);
   str += CString (" - ") + psRule -> strRuleName;
	TRuleEdit rulesDlg (str);
   rulesDlg.m_iPage = iPage;
	TRuleGeneral *psGenPage = new TRuleGeneral;
	TRuleConditions *psConPage = new TRuleConditions;
	TRuleActions *psActPage = new TRuleActions;

   Rule *psTempRule = new Rule (GetGlobalRules ());
   *psTempRule = *psRule;
   psGenPage -> SetRule (psTempRule);
   psConPage -> SetRule (psTempRule);
   psActPage -> SetRule (psTempRule);

	rulesDlg.AddPage (psGenPage);
	rulesDlg.AddPage (psConPage);
	rulesDlg.AddPage (psActPage);

	if (rulesDlg.DoModal() == IDOK &&
       (psGenPage -> m_bDirty || psConPage -> m_bDirty ||
        psActPage -> m_bDirty)) {
      *psRule = *psTempRule;
      m_bDirty = TRUE;
      SetColumn3 (GetCurrentRuleIndex (), psRule);
      }

   delete psTempRule;
	delete psGenPage;
	delete psConPage;
	delete psActPage;
   UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRulesDlg::OnEdit()
{
   DoEdit ();
}

// -------------------------------------------------------------------------
void TRulesDlg::OnMoveUp()
{
   // move the selected rule up
	int iIndex = GetCurrentRuleIndex ();
   ASSERT (iIndex);
   Rule *psRule = GetCurrentRule ();
	m_sRuleList.DeleteItem (iIndex);
   AddToListbox (psRule, TRUE /* iSetFocus */, iIndex - 1);
   m_bDirty = TRUE;

   // return focus to the listbox
   m_sRuleList.SetFocus ();

   UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRulesDlg::OnMoveDown()
{
   // move the selected rule down
	int iIndex = GetCurrentRuleIndex ();
   ASSERT (iIndex < m_sRuleList.GetItemCount () - 1);
   Rule *psRule = GetCurrentRule ();
	m_sRuleList.DeleteItem (iIndex);
   AddToListbox (psRule, TRUE /* iSetFocus */, iIndex + 1);
   m_bDirty = TRUE;

   // return focus to the listbox
   m_sRuleList.SetFocus ();

   UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRulesDlg::OnEnableDisable()
{
   // toggle the enabled-bit
   Rule *psRule = GetCurrentRule ();
   psRule -> bEnabled = !psRule -> bEnabled;

   // set it as "seen" as of now so that it expires after X days from now
   psRule -> Seen ();

   // refresh the display
   RefreshCurrentLine ();

   m_bDirty = TRUE;
   UpdateGreyState ();
}

// -------------------------------------------------------------------------
int TRulesDlg::GetNewName (CString &strNewName, LPCTSTR pchOldName /* = NULL */,
   int iPrompt /* = 0 */)
{
   TRuleName sDlg;
   if (iPrompt)
      sDlg.m_strPrompt.LoadString (iPrompt);
   else
      sDlg.m_strPrompt.LoadString (IDS_NEW_RULES_NAME);
   if (pchOldName)
      sDlg.m_strName = pchOldName;
   int RC = sDlg.DoModal ();
   if (RC == IDOK)
      strNewName = sDlg.m_strName;
   return RC;
}

// -------------------------------------------------------------------------
void TRulesDlg::OnRename()
{
   Rule *psRule = GetCurrentRule ();
   CString strNewName;
   if (GetNewName (strNewName, psRule -> strRuleName,
         IDS_RULE_NEW_NAME) != IDOK)
      return;

   // check for duplicate rule name
   Rule *psTemp;
   if (m_rsRules.Lookup (strNewName, psTemp)) {
      CString str; str.LoadString (IDS_DUPLICATE_RULE_NAME);
      MessageBox (str);
      return;
      }

   psRule -> strRuleName = strNewName;

   // refresh the display
   RefreshCurrentLine ();

   m_bDirty = TRUE;
   UpdateGreyState ();
}

// -------------------------------------------------------------------------
void TRulesDlg::UpdateGreyState ()
{
   int iIndex = GetCurrentRuleIndex ();
   BOOL bEnable = (iIndex != -1);
   m_sRename.EnableWindow (bEnable);
   m_sEnable.EnableWindow (bEnable);
   m_sEdit.EnableWindow (bEnable);
   m_sDelete.EnableWindow (bEnable);
   m_sCopy.EnableWindow (bEnable);
   m_sMoveUp.EnableWindow (bEnable && iIndex);
   m_sMoveDown.EnableWindow (
      bEnable && iIndex < m_sRuleList.GetItemCount () - 1);
}

// -------------------------------------------------------------------------
void TRulesDlg::OnOK()
{
   if (m_bDirty) {
      BeginWaitCursor ();

      // read the rules from the dialog to get their order
      m_rsRules.RemoveAll ();
      int iNum = m_sRuleList.GetItemCount ();
      for (int i = 0; i < iNum; i++) {
         Rule *pRule = (Rule *) m_sRuleList.GetItemData (i);
         m_rsRules.SetAt (pRule -> strRuleName, pRule);
         }

      Rules *pOrigRules = GetGlobalRules ();
      pOrigRules -> WriteLock ();
      *pOrigRules = m_rsRules;
      pOrigRules -> UnlockWrite ();

      // save new rules to DB
      gpStore -> SaveRules ();

      // also update the rule list in the manual-rule dialog bar
      gpsManualRuleBar -> UpdateRuleList ();

      EndWaitCursor ();
      }

	CDialog::OnOK();
}

// -------------------------------------------------------------------------
BOOL TRulesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

   // set up columns
   CRect sRect;
   m_sRuleList.GetWindowRect (&sRect);
   int iTotalWidth = sRect.right - sRect.left;
   CString strTitle;
   strTitle.LoadString (IDS_RULE_NAME);
   m_sRuleList.InsertColumn (0, strTitle, LVCFMT_LEFT,
      iTotalWidth - 220 - 20 /* width */, 0 /* sub-item */);
   strTitle.LoadString (IDS_APPLY_TO_INCOMING);
   m_sRuleList.InsertColumn (1, strTitle, LVCFMT_LEFT, 100 /* width */,
      1 /* sub-item */);
   strTitle.LoadString (IDS_DAYS_BEFORE_EXPIRATION);
   m_sRuleList.InsertColumn (2, strTitle, LVCFMT_LEFT, 120 /* width */,
      2 /* sub-item */);

   // make local copy of rules
   Rules *pOrigRules = GetGlobalRules ();
   pOrigRules -> ReadLock ();
   m_rsRules = *pOrigRules;
   pOrigRules -> UnlockRead ();

   m_bDirty = FALSE;
   DisplayRules ();

   UpdateGreyState ();
   return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TRulesDlg::OnExport()
{
   TRuleExportFile dlg;
   CString &strName = dlg.m_strFilename;
   LPTSTR pchDir = strName.GetBuffer (256);
   GetCurrentDirectory (256, pchDir);
   strName.ReleaseBuffer ();
   dlg.m_strPrompt.LoadString (IDS_EXPORT_PROMPT);
   dlg.m_strCaption.LoadString (IDS_RULE_EXPORT_TITLE);

   if (!strName.IsEmpty () && strName [strName.GetLength () - 1] != '\\')
      strName += "\\";
   strName += "gravity.rul";
   if (dlg.DoModal () != IDOK)
      return;

   // if file exists, prompt for overwrite
   CFileStatus status;
   if (CFile::GetStatus (dlg.m_strFilename, status))
      if (MsgResource (IDS_RULE_EXPORT_OVERWRITE, 0, 0, MB_YESNO) != IDYES)
         return;

   if (m_rsRules.Export (dlg.m_strFilename))
      MsgResource (IDS_ERR_RULE_EXPORT);
}

// -------------------------------------------------------------------------
void TRulesDlg::DisplayRules ()
{
   m_sRuleList.DeleteAllItems ();
   TRuleIterator it (m_rsRules, ReadLock);
   Rule *psRule;
   while (0 != (psRule = it.Next ()))
      AddToListbox (psRule);
   m_sRuleList.SetItemState (0, LVIS_SELECTED | LVIS_FOCUSED,
      LVIS_SELECTED | LVIS_FOCUSED);
}

// -------------------------------------------------------------------------
void TRulesDlg::OnImport()
{
   TRuleExportFile dlg;
   LPTSTR pchDir = dlg.m_strFilename.GetBuffer (256);
   GetCurrentDirectory (256, pchDir);
   dlg.m_strFilename.ReleaseBuffer ();
   dlg.m_strFilename += "\\gravity.rul";
   dlg.m_strPrompt.LoadString (IDS_IMPORT_PROMPT);
   dlg.m_strCaption.LoadString (IDS_RULE_IMPORT_TITLE);
   dlg.m_bReading = TRUE;
   if (dlg.DoModal () != IDOK)
      return;

   int iLine;
   if (m_rsRules.Import (dlg.m_strFilename, iLine, dlg.m_bDeleteOriginal)) {
      CString str; str.LoadString (IDS_ERR_RULE_IMPORT);
      CString strError;
      strError.Format ("%s %d", str, iLine);
      AfxGetMainWnd () -> MessageBox (strError);
      }

   DisplayRules ();
   m_bDirty = TRUE;
}

// -------------------------------------------------------------------------
void TRulesDlg::OnItemchangedRuleList(NMHDR* pNMHDR, LRESULT* pResult)
{
   UpdateGreyState ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TRulesDlg::OnDblclkRuleList(NMHDR* pNMHDR, LRESULT* pResult)
{
   DoEdit ();
   *pResult = 0;
}

// -------------------------------------------------------------------------
TRuleName::TRuleName(CWnd* pParent /*=NULL*/) : CDialog(TRuleName::IDD, pParent)
{

	m_strName = _T("");
	m_strPrompt = _T("");
}

// -------------------------------------------------------------------------
void TRuleName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_sOK);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_PROMPT, m_strPrompt);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleName, CDialog)
		ON_EN_CHANGE(IDC_NAME, OnChangeName)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TRuleName::OnChangeName()
{
   UpdateData ();
   m_sOK.EnableWindow (m_strName.GetLength () != 0);
}

// -------------------------------------------------------------------------
TRuleExportFile::TRuleExportFile(CWnd* pParent /*=NULL*/)
	: CDialog(TRuleExportFile::IDD, pParent)
{

	m_strFilename = _T("");
	m_strPrompt = _T("");

   m_bReading = 0;
}

// -------------------------------------------------------------------------
void TRuleExportFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_FILENAME, m_strFilename);
	DDX_Text(pDX, IDC_PROMPT, m_strPrompt);
}

// -------------------------------------------------------------------------
void TRuleExportFile::OnBrowse()
{
   CFileDialog dlg (m_bReading /* bOpenFileDialog */, NULL /* extension */,
      NULL /* file name */,
      OFN_HIDEREADONLY | (m_bReading ? 0 : OFN_OVERWRITEPROMPT),
      _T("All Files (*.*)|*.*||"), this);

   if (dlg.DoModal () != IDOK)
      return;

   m_strFilename = dlg.GetPathName ();
   UpdateData (FALSE /* bSaveAndValidate */);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleExportFile, CDialog)
		ON_BN_CLICKED(IDC_BROWSE, OnBrowse)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TRuleExportFile::OnInitDialog()
{
	CDialog::OnInitDialog();
   SetWindowText (m_strCaption);

   // if exporting, remove the 'replace rules with same name' checkbox
   if (!m_bReading)
      GetDlgItem (IDC_DELETE_ORIGINAL) -> DestroyWindow ();

	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TRuleExportFile::OnOK()
{
   if (m_bReading)
      m_bDeleteOriginal =
         ((CButton *) GetDlgItem (IDC_DELETE_ORIGINAL)) -> GetCheck ();

	CDialog::OnOK();
}

// -------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(TRuleEdit, CPropertySheet)

// -------------------------------------------------------------------------
TRuleEdit::TRuleEdit(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
   m_iPage = -1;
}

// -------------------------------------------------------------------------
TRuleEdit::TRuleEdit(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
   m_iPage = -1;
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TRuleEdit, CPropertySheet)
		ON_WM_DESTROY()

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TRuleEdit::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

   int iActive = m_iPage;
   if (m_iPage == -1)
      iActive = gpGlobalOptions -> GetRegUI () -> GetRuleEditIndex ();
   SetActivePage (iActive);

	return bResult;
}

// -------------------------------------------------------------------------
void TRuleEdit::OnDestroy()
{
   gpGlobalOptions -> GetRegUI () -> SetRuleEditIndex (GetActiveIndex ());
	CPropertySheet::OnDestroy();
}
