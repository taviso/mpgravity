/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: pikngdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:39  richard_wood
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

// pikngdlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "pikngdlg.h"
#include "grplist.h"
#include "vlist.h"
#include "tstrlist.h"
#include "globals.h"
#include "server.h"     // TNewsServer

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TPickNewsgroupDlg dialog

TPickNewsgroupDlg::TPickNewsgroupDlg(
TStringList* pList,
CWnd* pParent /*=NULL*/)
	: CDialog(TPickNewsgroupDlg::IDD, pParent),
     m_pStringList(pList)
{
   RegisterVListBox(AfxGetInstanceHandle());
   m_pGroups = 0;
}

void TPickNewsgroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);


   if (!pDX->m_bSaveAndValidate)
      {
      CListBox* pChosen = GetChosenLbx();
      // take items from stringlist and jam them into
      POSITION pos = m_pStringList->GetHeadPosition();
      while (pos)
         {
         pChosen->AddString ( m_pStringList->GetNext (pos) );
         }
      if (pChosen->GetCount() == 0)
         GetDlgItem(IDC_BUT_REMOVE)->EnableWindow(FALSE);
      }
   else
      {
      m_pStringList->RemoveAll();
      CListBox* pChosen = GetChosenLbx();
      // move items from listbox into the stringlist
      CString item;
      int tot = pChosen->GetCount();

      for (int j = 0; j < tot; ++j)
         {
         pChosen->GetText ( j, item );
         m_pStringList->AddTail (item);
         }
      }
}

BEGIN_MESSAGE_MAP(TPickNewsgroupDlg, CDialog)
	   ON_WM_VKEYTOITEM()
	ON_WM_DESTROY()
   ON_LBN_DBLCLK(IDC_GROUP_LIST,  OnDblclkGroupList)
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
	ON_EN_CHANGE(IDC_SUBSCRIBE_SEARCHSTRING, OnChangeSearchstring)
 	ON_EN_SETFOCUS(IDC_SUBSCRIBE_SEARCHSTRING, OnSetfocusSearchstring)

	ON_BN_CLICKED(IDC_SUBSCRIBE_RESETSEARCH, OnResetsearch)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, OnButtonSelect)
	ON_BN_CLICKED(IDC_BUT_REMOVE, OnButRemove)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TPickNewsgroupDlg message handlers

BOOL TPickNewsgroupDlg::OnInitDialog()
{
	// load group container
   m_pGroups = m_cpNewsServer->LoadServerGroupList ();
   m_lbxMgr.SetGroupList ( m_pGroups );
   FillList();

	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TPickNewsgroupDlg::FillList()
{
   CListBox* pLB = GetAvailLbx();
   pLB->SendMessage ( VLB_INITIALIZE );
}

void TPickNewsgroupDlg::OnDestroy()
{
	CDialog::OnDestroy();

   // free this pig
	delete m_pGroups;
}

CListBox* TPickNewsgroupDlg::GetAvailLbx()
{
   return (CListBox*) GetDlgItem(IDC_GROUP_LIST);
}

CListBox* TPickNewsgroupDlg::GetChosenLbx()
{
   return (CListBox*) GetDlgItem(IDC_SEL_GROUPS);
}

///////////////////////////////////////////////////////////////
// Virtual listbox stuff
LRESULT TPickNewsgroupDlg::On_vlbRange   (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbRange (wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbPrev    (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbPrev (wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbFindPos (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFindPos (wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbFindItem(WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFindItem(wP, lParam);
}

// Needs work
LRESULT TPickNewsgroupDlg::On_vlbFindString(WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFindString(wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbSelectString(WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbSelectString(wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbNext    (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbNext (wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbFirst   (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbFirst(wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbLast    (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbLast(wP, lParam);
}

LRESULT TPickNewsgroupDlg::On_vlbGetText (WPARAM wP, LPARAM lParam)
{
   return m_lbxMgr.On_vlbGetText (wP, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//
int TPickNewsgroupDlg::OnVKeyToItem(UINT key, CListBox* pListBox, UINT nIndex)
{
   // not VK_RETURN.  The enter key is handled by the
   //   Button with the 'Default Button' style
   if ((VK_SPACE == key || VK_INSERT == key) &&
       pListBox->m_hWnd != GetDlgItem(IDC_SEL_GROUPS)->m_hWnd)
      {
      // add this item
      OnButtonSelect();
      return -2;
      }
   if (VK_DELETE == key && pListBox->m_hWnd == GetDlgItem(IDC_SEL_GROUPS)->m_hWnd)
      {
      // remove item
      OnButRemove();
      return -2;
      }
   return -1;
}

/////////////////////////////////////////////////////////////////////////////
//
void TPickNewsgroupDlg::GetSearchText(CString & str)
{
   CWnd * pEbx = GetDlgItem (IDC_SUBSCRIBE_SEARCHSTRING);
   if (pEbx)
      pEbx -> GetWindowText ( str );
}

/////////////////////////////////////////////////////////////////////////////
//
void TPickNewsgroupDlg::OnChangeSearchstring()
{
CString searchString;

// get text from Line edit box
GetSearchText (searchString);

if (searchString != m_lastString)
   {
   m_lastString  = searchString;
   OnSubscribeSearch ();
   }
}

/////////////////////////////////////////////////////////////////////////////
//
void TPickNewsgroupDlg::OnSetfocusSearchstring()
{
   //CListBox* pLB = GetAvailLbx();
	//pLB->SendMessage(VLB_SETCURSEL, 0, LPARAM(-1));
}

/////////////////////////////////////////////////////////////////////////////
//
void TPickNewsgroupDlg::OnSubscribeSearch()
{
CString     searchString;
CString     header;
CListBox* pLB = GetAvailLbx();

GetSearchText ( searchString );

if (searchString.IsEmpty())
   {
   m_pGroups->ClearSearch();
   pLB->SendMessage ( VLB_RESETCONTENT );
   FillList ();
   TRACE0("Search string is empty - Case 0\n");
   return;
   }

pLB->SendMessage ( VLB_RESETCONTENT );

// search for the substring
m_pGroups->Search (searchString);

FillList ();

int total = m_pGroups->NumGroups ();
if (total > 0)
   {
   pLB->SendMessage ( VLB_SETCURSEL, WPARAM(VLB_FIRST), LPARAM(0) );
   TRACE0("Total is zero - Case 1\n");
   }
else
   {
   pLB->SendMessage ( VLB_UPDATEPAGE );
   TRACE0("update page - Case 2\n");
   }
}

void TPickNewsgroupDlg::OnDblclkGroupList()
{
   OnButtonSelect ();
}

void TPickNewsgroupDlg::OnResetsearch()
{
   GetDlgItem (IDC_SUBSCRIBE_SEARCHSTRING)->SetWindowText(_T(""));
}

/////////////////////////////////////////////////////////////////////
// this is slightly weird. in one case you send a message to the
// ListBox.  In the other, you send VLB_GETTEXT to the dialog (ourselves)
//
void TPickNewsgroupDlg::OnButtonSelect()
{
   CListBox* pLB = GetAvailLbx();
   int idx = pLB->SendMessage ( VLB_GETCURSEL );
   if (idx >= 0)
      {
      VLBSTRUCT sVLB;
      sVLB.lIndex = idx;

      this->SendMessage ( VLB_GETTEXT, 0, LPARAM(&sVLB) );
      if (VLB_OK == sVLB.nStatus)
         {
         // Filter out duplicates
         // this is not case sensitive
         if (LB_ERR == GetChosenLbx()->FindStringExact(-1, sVLB.lpTextPointer ))
            {
            GetChosenLbx()->AddString ( sVLB.lpTextPointer );
            GetDlgItem(IDC_BUT_REMOVE)->EnableWindow(TRUE);
            }
         }
      }
}

// should handle the DEL key also. handled in (ON_VKEY_TO_ITEM)
void TPickNewsgroupDlg::OnButRemove()
{
   CListBox* pChosen = GetChosenLbx();
	int idx = pChosen->GetCurSel();
	if (idx!=LB_ERR)
      {
      int tot;
      pChosen->DeleteString(idx);
      if ((tot = pChosen->GetCount()) == 0)
         GetDlgItem(IDC_BUT_REMOVE)->EnableWindow(FALSE);
      else
         {
         // reselect some item
         if (idx < tot)
            pChosen->SetCurSel(idx);
         else
            pChosen->SetCurSel(idx-1);
         }
      }
}
