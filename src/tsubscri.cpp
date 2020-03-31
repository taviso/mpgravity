/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsubscri.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:20  richard_wood
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

// tsubscri.cpp -- subscribe dialog

#include "stdafx.h"
#include "News.h"
#include "tsubscri.h"
#include "globals.h"
#include "tmutex.h"
#include "hints.h"
#include "pobject.h"
#include "grplist.h"
#include "vlist.h"
#include "tglobopt.h"
#include "rgconn.h"
#include "server.h"
#include "newsdoc.h"
#include "rgui.h"                // SaveUtilDlg(), ...
#include "genutil.h"             // DecomposeSizePos(), ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define SUBSCRIBE_TITLE "Subscribe" /* for saving/retrieving window size */

// -------------------------------------------------------------------------
TSubscribeDlg::TSubscribeDlg(CWnd* pParent /*=NULL*/,
   TGroupList* pNewGroups /*=NULL*/)
   : CDialog(TSubscribeDlg::IDD, pParent)
{
   RegisterVListBox(AfxGetInstanceHandle());

   m_pCurGroups = 0;
   m_pAllGroups = 0;
   m_pNewsServer = 0;

   // we do not own this pointer
   m_pNewGroups = pNewGroups;

   // are we showing All Groups, or just the new ones.
   if (m_pNewGroups)
      m_fShowAllGroups = FALSE;
   else
      m_fShowAllGroups = TRUE;

   
   m_searchString = _T("");
	m_strGoBack = _T("");
	m_bSample = FALSE;
	// m_iStorageMode = -1;
   m_currGroup   = _T("");

   // pre-select the global default

   TNewsGroup::EStorageOption eMode = gpGlobalOptions -> GetStorageMode ();
   if (eMode == TNewsGroup::kHeadersOnly)
      m_iStorageMode = 0;
   else
      m_iStorageMode = 1;
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TSubscribeDlg, CDialog)
      ON_LBN_DBLCLK(IDC_GROUP_LIST,       OnDblclkGroupList)
   ON_BN_CLICKED(IDC_BUTTON_DONE,      OnButtonDone)
   ON_BN_CLICKED(IDC_BUTTON_SUBSCRIBE, OnButtonSubscribe)
   ON_WM_VKEYTOITEM()
   ON_WM_DESTROY()
   ON_BN_CLICKED(IDC_SUBSCRIBE_SEARCH, OnSubscribeSearch)
   ON_EN_CHANGE(IDC_SUBSCRIBE_SEARCHSTRING, OnChangeSubscribeFilterSearch)
   ON_MESSAGE(VLB_RANGE,      On_vlbRange)
   ON_MESSAGE(VLB_PREV,       On_vlbPrev)
   ON_MESSAGE(VLB_FINDPOS,    On_vlbFindPos)
   ON_MESSAGE(VLB_FINDITEM,   On_vlbFindItem)
   ON_MESSAGE(VLB_FINDSTRING, On_vlbFindString)
   ON_MESSAGE(VLB_SELECTSTRING, On_vlbSelectString)
   ON_MESSAGE(VLB_NEXT,       On_vlbNext)
   ON_MESSAGE(VLB_FIRST,      On_vlbFirst)
   ON_MESSAGE(VLB_LAST,       On_vlbLast)
   ON_MESSAGE(VLB_GETTEXT,    On_vlbGetText)
   ON_EN_SETFOCUS(IDC_SUBSCRIBE_SEARCHSTRING, OnSetfocusSubscribeSearchstring)
   ON_BN_CLICKED(IDC_SUBSCRIBE_RESETSEARCH, OnSubscribeResetsearch)
	ON_BN_CLICKED(IDC_BUTTON_SUBSCRIBE2, OnButtonRemove)
	ON_NOTIFY(NM_DBLCLK, IDC_SELECTED, OnDblclkSelected)
	ON_EN_CHANGE(IDC_GO_BACK, OnChangeGoBack)
	ON_BN_CLICKED(IDC_SAMPLE, OnSample)
	ON_CBN_SELCHANGE(IDC_STORAGE_MODE, OnSelchangeStorageMode)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SELECTED, OnItemchangedSelected)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_WM_HSCROLL()

   ON_LBN_SELCHANGE (IDC_GROUP_LIST, OnGroupListSelchange)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TSubscribeDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   
	DDX_Control(pDX, IDC_SUBSCRIBE_GOBACK_DESC2, m_sGobackDesc2);
	DDX_Control(pDX, IDC_SUBSCRIBE_GOBACK_DESC1, m_sGobackDesc1);
	DDX_Control(pDX, IDC_GOBACK_LOW, m_sGoBackLow);
	DDX_Control(pDX, IDC_GOBACK_HIGH, m_sGoBackHigh);
	DDX_Control(pDX, IDC_GOBACK_SLIDER, m_sGoBackSlider);
	DDX_Control(pDX, IDC_SUBSCRIBE_RESETSEARCH, m_sReset);
	DDX_Control(pDX, IDC_BUTTON_DONE, m_sDone);
	DDX_Control(pDX, IDC_SUBSCRIBE_STOREMODE_DESC, m_sStoremodeDesc);
	DDX_Control(pDX, IDC_SUBSCRIBE_CHOSEN_DESC, m_sChosenDesc);
	DDX_Control(pDX, IDC_SUBSCRIBE_SEARCH_DESC, m_sSearchDesc);
	DDX_Control(pDX, IDC_SUBSCRIBE_SEARCHSTRING, m_sFilter);
	DDX_Control(pDX, IDC_GO_BACK, m_sGoBack);
	DDX_Control(pDX, IDC_STORAGE_MODE, m_sStorageMode);
	DDX_Control(pDX, IDC_SAMPLE, m_sSample);
	DDX_Control(pDX, IDC_BUTTON_SUBSCRIBE2, m_sUnsubscribe);
	DDX_Control(pDX, IDC_BUTTON_SUBSCRIBE, m_sSubscribe);
	DDX_Text(pDX, IDC_GO_BACK, m_strGoBack);
	DDX_Check(pDX, IDC_SAMPLE, m_bSample);
	DDX_CBIndex(pDX, IDC_STORAGE_MODE, m_iStorageMode);
	DDX_Control(pDX, IDC_SELECTED, m_sSelected);
}

// -------------------------------------------------------------------------
BOOL TSubscribeDlg::OnInitDialog()
{
   m_pNewsServer = GetCountedActiveServer();
   m_pNewsServer -> AddRef ();

   // load the group container from the store,
   if (m_fShowAllGroups)
      {
      m_pAllGroups = m_pNewsServer->LoadServerGroupList ();
      m_pCurGroups = m_pAllGroups;
      }
   else
      {
      // this is an incremental update
      ASSERT(m_pNewGroups);
      m_pCurGroups = m_pNewGroups;
      }

   m_lbxMgr.SetGroupList (m_pCurGroups);
   m_totalGroups = m_pCurGroups->NumGroups();

   FillList();
   UpdateGroupRatio();

   CDialog::OnInitDialog();   // does data exchange

   CString str;
   str.LoadString (IDS_SUBSCRIBE_NEWSGROUP);
   m_sSelected.InsertColumn (0, str, LVCFMT_LEFT, 200 /* width */,
      0 /* subitem */);
   str.LoadString (IDS_SUBSCRIBE_SAMPLE);
   m_sSelected.InsertColumn (1, str, LVCFMT_LEFT, 60 /* width */,
      1 /* subitem */);
   str.LoadString (IDS_SUBSCRIBE_STORAGE_MODE);
   m_sSelected.InsertColumn (2, str, LVCFMT_LEFT, 120 /* width */,
      2 /* subitem */);
   str.LoadString (IDS_SUBSCRIBE_GO_BACK);
   m_sSelected.InsertColumn (3, str, LVCFMT_LEFT, 60 /* width */,
      3 /* subitem */);

   //str.LoadString (IDS_STORAGE_NOTHING);
   //m_sStorageMode.AddString (str);

   str.LoadString (IDS_STORAGE_HEADERSONLY);
   m_sStorageMode.AddString (str);
   str.LoadString (IDS_STORAGE_BODIES);
   m_sStorageMode.AddString (str);

   // set window size
   int iX, iY;
   BOOL bMax, bMin;
   TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
   if (!pRegUI -> LoadUtilDlg (SUBSCRIBE_TITLE, iX, iY, bMax, bMin))
      SetWindowPos (NULL, 0, 0, iX, iY, SWP_NOZORDER | SWP_NOMOVE);

   // set window pos
   CString strSizePos = gpGlobalOptions -> GetRegUI () -> GetSubscribeSizePos ();
   int dx, dy, x, y;
   if (!DecomposeSizePos (strSizePos, dx, dy, x, y))
      SetWindowPos (NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

   MoveControls ();
   GotoDlgCtrl (&m_sFilter);
   UpdateGrayState ();

   // initialize slider
   m_sGoBackSlider.SetRange (0, 2000);
   m_sGoBackSlider.SetLineSize (100);
   m_sGoBackSlider.SetTicFreq (100);

   return FALSE;     // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnClose()
{

	CDialog::OnClose();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnDblclkGroupList()
{
   OnButtonSubscribe ();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnGroupListSelchange ()
{
   UpdateGrayState ();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::FillList()
{
   CListBox* pLB = GetGroupList();
   pLB->SendMessage (VLB_INITIALIZE);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::UpdateGroupRatio ()
{
   CWnd *pWnd = GetDlgItem (IDC_SUBSCRIBE_MATCHEDTOTAL);

   if (pWnd)
      {
      CString  format;

      format.Format ("%d/%d",
                     m_pCurGroups->NumGroups(),
                     m_totalGroups);
      pWnd->SetWindowText (format);
      }
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnButtonDone()
{
   CWaitCursor wait;

   CString strMode1; strMode1.LoadString (IDS_STORAGE_NOTHING);
   CString strMode2; strMode2.LoadString (IDS_STORAGE_HEADERSONLY);
   CString strMode3; strMode3.LoadString (IDS_STORAGE_BODIES);
   int iCount = m_sSelected.GetItemCount ();
   for (int i = 0; i < iCount; i++) {
      CString strName = m_sSelected.GetItemText (i, 0 /* column */);
      if (!CNewsApp::InNewsgroupList (strName)) {

         TGlobalDef::ENewsGroupType eType =
            (TGlobalDef::ENewsGroupType) m_sSelected.GetItemData (i);

         CString str;
         str = m_sSelected.GetItemText (i, 2 /* column */);
         BYTE bMode = 255;
         if (str == strMode1)
            bMode = STORAGE_MODE_STORENOTHING;
         else if (str == strMode2)
            bMode = STORAGE_MODE_STOREHEADERS;
         else
            bMode = STORAGE_MODE_STOREBODIES;
         ASSERT (bMode != 255);

         int iGoBack = atoi (m_sSelected.GetItemText (i, 3 /* column */));
         if (iGoBack < 0)
            iGoBack = 1000;

         CString strSample = m_sSelected.GetItemText (i, 1 /* column */);
         BOOL bSample = (!strSample.IsEmpty () && strSample [0] == 'Y');

         TNewsGroup *pOneGroup = m_pNewsServer -> SubscribeGroup (strName,
            eType, (BYTE) bMode, iGoBack, bSample);
         ((CNewsApp *) AfxGetApp ()) -> AddToNewsgroupList (pOneGroup);

         m_OutputGroupNames.AddTail (strName);
         }
      }

   pDoc -> SubscribeGroupUpdate ();

   // update the global 'go back setting' if there is a number in the 'go back'
   // field
   UpdateData ();
   if (!m_strGoBack.IsEmpty ()) {
      int iGoBack = atoi (m_strGoBack);
      if (iGoBack < 0)
         iGoBack = 1000;
      m_pNewsServer -> SetGoBackArtcount (iGoBack);
      m_pNewsServer -> SaveSettings ();
      }

   EndDialog (IDOK);
}

// -------------------------------------------------------------------------
static BOOL ListCtrlContainsString (CListCtrl &sList, LPCSTR pchStr)
{
   LV_FINDINFO sFind = {LVFI_STRING, pchStr, 0};
   return sList.FindItem (&sFind) != -1;
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnButtonSubscribe()
{
   CListBox *pLB = GetGroupList();
   int iIndex = pLB->SendMessage (VLB_GETCURSEL);
   if (iIndex < 0)
      return;

   WORD wNumArticles;
   TGlobalDef::ENewsGroupType eType;
   CString strSelected ;
   m_pCurGroups -> GetItem (iIndex, strSelected, wNumArticles, eType);

   // check for duplicates
   if (ListCtrlContainsString (m_sSelected, strSelected))
      return;

   CString strSample; strSample.LoadString (IDS_NO);

   // get default storage mode
   int iID;
   TNewsGroup::EStorageOption eMode = gpGlobalOptions -> GetStorageMode ();
   if (eMode == TNewsGroup::kHeadersOnly)
      iID = IDS_STORAGE_HEADERSONLY;
   else
      iID = IDS_STORAGE_BODIES;
   CString strMode; strMode.LoadString (iID);

   // get default go-back value
   CString strGoBack; strGoBack.Format ("%d",
      m_pNewsServer -> GetGoBackArtcount ());

   // un-select all selected items
   int iCount = m_sSelected.GetItemCount ();
   for (int i = 0; i < iCount; i++)
      if (m_sSelected.GetItemState (i, LVIS_SELECTED))
         m_sSelected.SetItemState (i, 0, LVIS_SELECTED);

   iIndex = m_sSelected.InsertItem (0 /* index */, strSelected);
   m_sSelected.SetItemText (iIndex, 1 /* column */, strSample);
   m_sSelected.SetItemText (iIndex, 2 /* column */, strMode);
   m_sSelected.SetItemText (iIndex, 3 /* column */, strGoBack);
   m_sSelected.SetItemData (iIndex, (DWORD) eType);

   // select the newly-added item
   m_sSelected.SetItemState (iIndex, LVIS_SELECTED, LVIS_SELECTED);

   // make sure it's visible
   m_sSelected.EnsureVisible (iIndex, FALSE /* bPartialOK */);

   UpdateGrayState ();
}

// -------------------------------------------------------------------------
int TSubscribeDlg::OnVKeyToItem(UINT key, CListBox* pListBox, UINT nIndex)
{
   // not VK_RETURN.  The enter key is handled by the button with the
   // 'Default Button' style

   if (VK_SPACE == key && (pListBox->GetDlgCtrlID() == IDC_GROUP_LIST))
      {
      // simulate a doubleclick
      this->OnDblclkGroupList();
      return -2;
      }

   return -1;
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnDestroy()
{
   // decrement ref count
   m_pNewsServer->Release ();
   m_pNewsServer = 0;

   // save window size
   CRect sRect;
   GetWindowRect (&sRect);
   TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
   pRegUI -> SaveUtilDlg (SUBSCRIBE_TITLE, (int) (sRect.right - sRect.left),
      (int) (sRect.bottom - sRect.top), FALSE, FALSE);

   // save window pos
   GetWindowRect (&sRect);
   CString strSizePos = ComposeSizePos (0, 0, sRect.left, sRect.top);
   gpGlobalOptions -> GetRegUI () -> SetSubscribeSizePos (strSizePos);

   CDialog::OnDestroy();
   if (m_fShowAllGroups)
      delete m_pAllGroups;
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnSubscribeSearch()
{
   CString     searchString;
   CString     header;
   CListBox* pLB = GetGroupList();
   CEdit *pEdit = (CEdit *) GetDlgItem (IDC_SUBSCRIBE_SEARCHSTRING);

   pEdit->GetWindowText(searchString);

   if (searchString.IsEmpty())
      {
      m_pCurGroups->ClearSearch();
      //SetDlgItemText (IDC_SUBSCRIBE_STATIC, "");
      pLB->SendMessage ( VLB_RESETCONTENT );
      FillList ();
      //pLB->SendMessage ( VLB_UPDATEPAGE );
      UpdateGroupRatio();
      return;
      }

   // update Static text
   //header.Format ("Filtering by : %s", searchString);
   //SetDlgItemText (IDC_SUBSCRIBE_STATIC, header);

   //pLB->SetRedraw ( FALSE );
   pLB->SendMessage ( VLB_RESETCONTENT );

   // search for the
   m_pCurGroups->Search (searchString);

   FillList ();

   int total = m_pCurGroups->NumGroups ();
   if (total > 0)
      pLB->SendMessage ( VLB_SETCURSEL, WPARAM(VLB_FIRST), LPARAM(0) );
   else
      pLB->SendMessage ( VLB_UPDATEPAGE );

   UpdateGroupRatio();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnChangeSubscribeFilterSearch()
{
   static CString lastString;

   CString searchString;
   m_sFilter.GetWindowText (searchString);

   if (searchString != lastString) {
      lastString  = searchString;
      OnSubscribeSearch ();
      UpdateGrayState ();
      }
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbRange   (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbRange (wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbPrev    (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbPrev (wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbFindPos (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFindPos (wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbFindItem(WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFindItem(wP, lParam);
}

// -------------------------------------------------------------------------
// Needs work
LRESULT TSubscribeDlg::On_vlbFindString(WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFindString(wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbSelectString(WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbSelectString(wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbNext    (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbNext (wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbFirst   (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFirst(wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbLast    (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbLast(wP, lParam);
}

// -------------------------------------------------------------------------
LRESULT TSubscribeDlg::On_vlbGetText (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbGetText (wP, lParam);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnSetfocusSubscribeSearchstring()
{
   CListBox *pLB = GetGroupList ();
   pLB -> SendMessage (VLB_SETCURSEL, 0, LPARAM (-1));
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnSubscribeResetsearch()
{
   m_sFilter.SetWindowText ("");
   GotoDlgCtrl (&m_sFilter);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::UpdateGrayState ()
{
   BOOL bAvailableSel = GetGroupList() -> SendMessage (VLB_GETCURSEL) >= 0;
   BOOL bSubscribedSel = m_sSelected.GetSelectedCount () > 0;

   m_sSubscribe.EnableWindow (bAvailableSel);
   m_sUnsubscribe.EnableWindow (bSubscribedSel);
   m_sSample.EnableWindow (bSubscribedSel);
   m_sStorageMode.EnableWindow (bSubscribedSel);
   m_sGoBack.EnableWindow (bSubscribedSel);
   m_sGoBackSlider.EnableWindow (bSubscribedSel);
}

// -------------------------------------------------------------------------
// OnButtonRemove -- remove items from the chosen-group set
void TSubscribeDlg::OnButtonRemove ()
{
   // go backwards and remove the selected ones
   for (int i = m_sSelected.GetItemCount () - 1; i >= 0; i--)
      if (m_sSelected.GetItemState (i, LVIS_SELECTED))
         m_sSelected.DeleteItem (i);

   // set focus elsewhere
   GotoDlgCtrl (&m_sSelected);

   UpdateGrayState ();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnDblclkSelected(NMHDR* pNMHDR, LRESULT* pResult)
{
   OnButtonRemove ();
	*pResult = 0;
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnItemchangedSelected(NMHDR* pNMHDR, LRESULT* pResult)
{
   UpdateGrayState ();

   // update the values in the edit controls
   UpdateEditValues ();

   *pResult = 0;
}

// -------------------------------------------------------------------------
void TSubscribeDlg::UpdateEditValues ()
{
   CString strGoBack;
   int iMode = m_iStorageMode;
   BOOL bMultipleGoBack = FALSE, bMultipleMode = FALSE, bMultipleSample = FALSE;
   BOOL bSample = FALSE, bFirstItem = TRUE;

   // go through groups and get their values
   CString strYes; strYes.LoadString (IDS_YES);
   CString strStoringHeaders; strStoringHeaders.LoadString (IDS_STORAGE_HEADERSONLY);
   int iCount = m_sSelected.GetItemCount ();
   for (int i = 0; i < iCount; i++)
      if (m_sSelected.GetItemState (i, LVIS_SELECTED)) {
         CString strThisGoBack;
         int iThisMode;
         BOOL bThisSample;

         // read this group's second column
         CString str;
         str = m_sSelected.GetItemText (i, 1 /* column */);
         bThisSample = (str == strYes);

         // read this group's third column
         str = m_sSelected.GetItemText (i, 2 /* column */);
         iThisMode = -1;
         if (str == strStoringHeaders)
            iThisMode = 0;
         else
            iThisMode = 1;

         ASSERT (iThisMode >= 0);

         // read this group's fourth column
         strThisGoBack = m_sSelected.GetItemText (i, 3 /* column */);

         // figure out multiplicity
         if (!bFirstItem && strThisGoBack != strGoBack)
            bMultipleGoBack = TRUE;

         if (!bFirstItem && iThisMode != iMode)
            bMultipleMode = TRUE;

         if (!bFirstItem && bThisSample != bSample)
            bMultipleSample = TRUE;

         bFirstItem = FALSE;

         // record this group's values
         strGoBack = strThisGoBack;
         iMode = iThisMode;
         bSample = bThisSample;
         }

   ASSERT(iMode == 0 || iMode == 1);

   // adjust values as needed
   if (bMultipleGoBack)
      strGoBack = "";
   if (bMultipleMode)
      iMode = 0;
   if (bMultipleSample)
      bSample = FALSE;

   // write values
   m_strGoBack = strGoBack;
   m_iStorageMode = iMode;
   m_bSample = bSample;
   UpdateData (FALSE /* bSaveAndValidate */);
   UpdateSliderValue ();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::FillSelColumn (int iColumn, LPCTSTR pchStr)
{
   int iCount = m_sSelected.GetItemCount ();
   for (int i = 0; i < iCount; i++)
      if (m_sSelected.GetItemState (i, LVIS_SELECTED))
         m_sSelected.SetItemText (i, iColumn, pchStr);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnSample()
{
   UpdateData ();
   CString str; str.LoadString (m_bSample ? IDS_YES : IDS_NO);
   FillSelColumn (1, str);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnSelchangeStorageMode()
{
   UpdateData ();
   CString str;
   m_sStorageMode.GetLBText (m_iStorageMode, str);
   FillSelColumn (2, str);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::UpdateSliderValue ()
{
   UpdateData (); // make sure m_strGoBack is current
   int iPos = min (atoi (m_strGoBack), 2000);
   if (iPos < 0)
      iPos = 2000;
   m_sGoBackSlider.SetPos (iPos);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnChangeGoBack()
{
   UpdateData ();
   FillSelColumn (3, m_strGoBack);
   UpdateSliderValue ();
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
   MoveControls ();
}

// -------------------------------------------------------------------------
static void Move (CWnd &wnd, int iX, int iY, BOOL bInvalidate = TRUE)
{
   wnd.SetWindowPos (NULL, iX, iY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
   if (bInvalidate)
      wnd.Invalidate ();
}

// -------------------------------------------------------------------------
static void Size (CWnd &wnd, int iX, int iY)
{
   wnd.SetWindowPos (NULL, 0, 0, iX, iY, SWP_NOMOVE | SWP_NOZORDER);
}

// -------------------------------------------------------------------------
static int Width (CWnd &wnd)
{
   RECT sRect;
   wnd.GetClientRect (&sRect);
   return sRect.right;
}

// -------------------------------------------------------------------------
static int Height (CWnd &wnd)
{
   RECT sRect;
   wnd.GetClientRect (&sRect);
   return sRect.bottom;
}

// -------------------------------------------------------------------------
void TSubscribeDlg::MoveControls ()
{
   if (!m_sReset.m_hWnd)      // if not yet initialized, return
      return;

   // get needed measurements
   RECT sRect;
   GetClientRect (&sRect);
   int iRightEdge = sRect.right - 10;
   int iBottomEdge = sRect.bottom - 10;
   int iTopEdge = 10, iLeftEdge = 10;

   // search controls
   int iTemp = iRightEdge - Width (m_sReset);
   Move (m_sReset, iTemp, iTopEdge);
   iTemp -= 10 + Width (m_sFilter);
   Move (m_sFilter, iTemp, iTopEdge);
   iTemp -= 6 + Width (m_sSearchDesc);
   Move (m_sSearchDesc, iTemp, iTopEdge + 3);

   // done button
   Move (m_sDone, iRightEdge - Width (m_sDone), iBottomEdge - Height (m_sDone));

   // setting-change buttons
   Move (m_sGobackDesc1, iLeftEdge, iBottomEdge - Height (m_sGobackDesc1) - 5);
   int iTempWidth = iLeftEdge + Width (m_sGobackDesc1) + 10;
   Move (m_sGoBack, iTempWidth, iBottomEdge - Height (m_sGoBack) - 5);
   iTempWidth += Width (m_sGoBack) + 10;
   Move (m_sGobackDesc2, iTempWidth, iBottomEdge - Height (m_sGobackDesc2) - 5);
   iTempWidth += Width (m_sGobackDesc2) + 10;
   Move (m_sGoBackLow, iTempWidth, iBottomEdge - Height (m_sGoBackLow) - 5);
   iTempWidth += Width (m_sGoBackLow) + 10;
   Move (m_sGoBackSlider, iTempWidth, iBottomEdge - Height (m_sGoBackSlider));
   iTempWidth += Width (m_sGoBackSlider) + 10;
   Move (m_sGoBackHigh, iTempWidth, iBottomEdge - Height (m_sGoBackHigh) - 5);
   iTemp = iBottomEdge - Height (m_sGoBack) - 25;
   Move (m_sStoremodeDesc, iLeftEdge, iTemp - Height (m_sStoremodeDesc) - 3);
   iTempWidth = 10 + Width (m_sStoremodeDesc);
   Move (m_sStorageMode, iTempWidth, iTemp - Height (m_sStorageMode));
   iTemp -= 5 + Height (m_sStorageMode);
   Move (m_sSample, iLeftEdge, iTemp - Height (m_sSample));

   // adjust iTopEdge to top of available-groups list
   iTopEdge += Height (m_sReset) + 10;

   // adjust iBottom to bottom of selected-groups list
   iBottomEdge -= 40 + Height (m_sSample) + Height (m_sGoBack) + Height (m_sStorageMode);

   // top listbox
   int iListHeight = ((iBottomEdge - iTopEdge) - 10 - Height (m_sSubscribe)) / 2;
   CListBox *pAvailable = GetGroupList ();
   Move (*pAvailable, iLeftEdge, iTopEdge, FALSE /* bInvalidate */);
   Size (*pAvailable, iRightEdge - iLeftEdge, iListHeight);

   // bottom listbox
   iTemp = iTopEdge + iListHeight + 10 + Height (m_sSubscribe);
   Move (m_sChosenDesc, iLeftEdge, iTemp - 5 - Height (m_sChosenDesc));
   Size (m_sSelected, iRightEdge - iLeftEdge, iListHeight);
   Move (m_sSelected, iLeftEdge, iTemp, FALSE /* bInvalidate */);

   // size columns
   m_sSelected.SetColumnWidth (0, iRightEdge - iLeftEdge
      - 240 /* other columns' widths */
      - 18 /* slack and room for vertical scrollbar */);

   // add/remove buttons
   int iMiddle = (iRightEdge - iLeftEdge) / 2;
   iTemp = iTopEdge + iListHeight + 5;
   Move (m_sSubscribe, iMiddle - 5 - Width (m_sSubscribe), iTemp);
   Move (m_sUnsubscribe, iMiddle + 5, iTemp);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI -> ptMinTrackSize.x = 620;
	lpMMI -> ptMinTrackSize.y = 370;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// -------------------------------------------------------------------------
void TSubscribeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   if (nSBCode == TB_THUMBTRACK ||
       nSBCode == TB_TOP || nSBCode == TB_BOTTOM ||
       nSBCode == TB_LINEDOWN || nSBCode == TB_LINEUP ||
       nSBCode == TB_PAGEDOWN || nSBCode == TB_PAGEUP) {

      // nPos already correct if nSBCode == TB_THUMBTRACK, but what the hey
      nPos = m_sGoBackSlider.GetPos ();

      m_strGoBack.Format ("%d", nPos);
      UpdateData (FALSE /* bSaveAndValidate */);

      // propagate the value back to the selected groups
      FillSelColumn (3, m_strGoBack);
      }

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TSelectedGroups, CListCtrl)
		ON_WM_KEYDOWN()

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TSelectedGroups::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   if (nChar == VK_DELETE) {
      TSubscribeDlg *pParent = (TSubscribeDlg *) GetParent ();
      pParent -> OnButtonRemove ();
      }

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
