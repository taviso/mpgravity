/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsigpg.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
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
/*  Revision 1.2  2008/09/19 14:52:19  richard_wood
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

// tsigpg.cpp : implementation file
//
// This operates via Combobox and SetItemDataPtr

// when read-only change it to grey?

#include "stdafx.h"
#include "News.h"
#include "tsigpg.h"
#include "nsigdlg.h"
#include "gdiutil.h"             // setupCourierFont
#include "helpcode.h"            // HID_OPTIONS_*

#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsSignatureDlg property page

IMPLEMENT_DYNCREATE(TOptionsSignatureDlg, CPropertyPage)

#define new DEBUG_NEW

TOptionsSignatureDlg::TOptionsSignatureDlg() : CPropertyPage(TOptionsSignatureDlg::IDD)
{


   m_ReadOnly = FALSE;
}

TOptionsSignatureDlg::~TOptionsSignatureDlg()
{
}

void TOptionsSignatureDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   if (FALSE == pDX->m_bSaveAndValidate)
      {
      if (GetCurrent()->GetCount() == 0)
         {
         // Setup the combo box
         int iAdd;
         int iDefIndex = m_TsigList.GetDefaultIndex();
         int total = m_TsigList.GetSize();

         for (int i = 0; i < total; ++i)
            {
            TCustomSignature* pSig = m_TsigList.CopyAt (i);

            // add this to combobox
            iAdd = GetCurrent()->AddString ( pSig->GetShortName() );

            // store this reference
            GetCurrent()->SetItemDataPtr ( iAdd, pSig );

            if (i == iDefIndex)
               {
               CString sigText;
               GetDefault()->SetWindowText ( pSig->GetShortName() );
               pSig->GetSignature( sigText, this );
               GetEdit()->SetWindowText ( sigText );
               if (FALSE == pSig->CanChange())
                  {
                  SetNoSignature(pSig);
                  }
               // put selection on the default guy
               GetCurrent()->SetCurSel (iAdd);
               m_pDefSig = pSig;
               }
            }
         OnSelchangeSignatureCurrent();
         }
      }
   else
      {
      TRACE0("Saving data\n");
      // move data from dlg to structs
      SaveCurrentScreen();
      m_TsigList.RemoveAll();
      CComboBox* pCmb = GetCurrent();
      int max = pCmb->GetCount();
      for (int i = 0; i < max; ++i)
         {
         int  iAt;
         TCustomSignature* pSig = (TCustomSignature*)pCmb->GetItemDataPtr(i);

         iAt = m_TsigList.Add(*pSig);
         if (m_pDefSig == pSig)
            m_TsigList.SetDefaultIndex(iAt);
         }
      }
}

BEGIN_MESSAGE_MAP(TOptionsSignatureDlg, CPropertyPage)
		ON_CBN_SELCHANGE(IDC_SIGNATURE_CURRENT, OnSelchangeSignatureCurrent)
	ON_BN_CLICKED(IDC_SIGNATURE_NEW, OnSignatureNew)
	ON_BN_CLICKED(IDC_SIGNATURE_REMOVE, OnSignatureRemove)
	ON_BN_CLICKED(IDC_SIGNATURE_SETDEFAULT, OnSignatureSetdefault)
	ON_EN_KILLFOCUS(IDC_SIGNATURE_EDIT, OnKillfocusSignatureEdit)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SIGNATURE_INSERTFILE, OnSignatureInsertfile)
	ON_WM_HELPINFO()

   ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsSignatureDlg message handlers

void TOptionsSignatureDlg::OnKillfocusSignatureEdit()
{
   SaveCurrentScreen(); // can this be so easy?
}

void TOptionsSignatureDlg::OnSelchangeSignatureCurrent()
{
   int iSel = GetCurrent()->GetCurSel();

   // Save the old & display the new.
   // - we already saved the old when the EditBox loses Focus,
   //   so just display new
   TCustomSignature* pSig = GetpSig(iSel);
   ShowThis ( pSig );

   BOOL fCanChange = pSig->CanChange();

   CWnd* pRemove = GetDlgItem(IDC_SIGNATURE_REMOVE);
   CWnd* pInsert = GetDlgItem(IDC_SIGNATURE_INSERTFILE);

   if (!fCanChange && (pRemove == GetFocus() || pInsert == GetFocus()))
      {
      // move focus to a good place
      GetDlgItem(IDC_SIGNATURE_NEW)->SetFocus();
      }
   pRemove->EnableWindow( fCanChange );
   pInsert->EnableWindow( fCanChange );
}

///////////////////////////////////////////////////////////////
// The "No Signature" item must be kept constant
//  this is a button
void TOptionsSignatureDlg::OnSignatureNew()
{
   CStringList list;
   CComboBox* pCmb  = GetCurrent();

   // the dialog box uses this list to check for Uniqueness
   for (int i = 0; i < pCmb->GetCount(); ++i)
      {
      TCustomSignature* pSig = (TCustomSignature*)pCmb->GetItemDataPtr(i);
      list.AddTail ( pSig->GetShortName() );
      }

   TNewSigDlg dlg(&list, this);

   // sub - dialog asks for the logical name
   if (IDOK == dlg.DoModal())
      {
      // ?? warn about duplicate names

      //  make a new entry

      TCustomSignature * pNewSig = new TCustomSignature ( dlg.m_Name );
      int iAddIdx = GetCurrent()->AddString ( dlg.m_Name );
      GetCurrent()->SetItemDataPtr ( iAddIdx, pNewSig );

      GetCurrent()->SetCurSel ( iAddIdx );

      // show it & enable buttons
      OnSelchangeSignatureCurrent();

      GetDlgItem(IDC_SIGNATURE_EDIT)->SetFocus();
      }
}

///////////////////////////////////////////////////////////////
// button
void TOptionsSignatureDlg::OnSignatureRemove()
{
   int Count = GetCurrent()->GetCount();

   int iSel = GetCurrent()->GetCurSel();
   if (CB_ERR == iSel)
      {
      ASSERT(0);
      return;
      }
   TCustomSignature* pCurSig = GetpSig(iSel);

   if (FALSE == pCurSig->CanChange())
      {
      MessageBeep(0);
      return;
      }
   else
      {
      GetCurrent()->DeleteString(iSel);
      delete pCurSig;
      }

   TCustomSignature* pDeadSig = pCurSig;

   // Figure out new selection
   int iNewSel;
   if (iSel == Count - 1)
      iNewSel = Count - 2;
   else
      iNewSel = iSel;

   GetCurrent()->SetCurSel(iNewSel);
   // show it and enable/disable buttons
   OnSelchangeSignatureCurrent();

   if (pDeadSig == m_pDefSig)
      // need new default?
      OnSignatureSetdefault();
}

///////////////////////////////////////////////////////////////
// button
void TOptionsSignatureDlg::OnSignatureSetdefault()
{
   SaveCurrentScreen();

   CString line;
   CComboBox* pCmb = GetCurrent();
   pCmb->GetWindowText( line );

   TCustomSignature* pSig;
   if (Find(line, pSig) < 0)
      return;

   m_pDefSig = pSig;

   GetDefault()->SetWindowText (pSig->GetShortName());

   if (!pSig->CanChange())
      {
      SetNoSignature(pSig);
      return;
      }
}

void TOptionsSignatureDlg::SaveCurrentScreen()
{
   CString line;
   CComboBox* pCmb = GetCurrent();
   pCmb->GetWindowText( line );

   TCustomSignature* pSig;
   if (Find(line, pSig) < 0)
      {
      TRACE0("Not Found - SaveCurrentScreen failed\n");
      return;
      }

   if (!pSig->CanChange())
      {
      // this is the "no signature" item
      }
   else
      {
      CString sig_text;
      CEdit* pEdit = GetEdit();
      pEdit->GetWindowText(sig_text);
      pSig->SetSignature (sig_text);

      // save the short name - Do We Need this ?  We save the shortname on LOSEFOCUS
      //GetCurrent()->GetLBText (iSel, sig_text);
      //pSig->SetShortName (sig_text);
      }
}

//////////////////////////////////////////////////////
// Clean up the data pointers in the combobox
void TOptionsSignatureDlg::OnDestroy()
{
	CPropertyPage::OnDestroy();

   CComboBox* pCmb = GetCurrent();
   int max = pCmb->GetCount();
   for (int i = 0; i < max; ++i)
      {
      TCustomSignature* pSig = (TCustomSignature*)pCmb->GetItemDataPtr(i);
      delete pSig;
      pCmb->SetItemDataPtr(i, NULL);
      }
}

////////////// Utility Function /////////////////////
int  TOptionsSignatureDlg::Find ( const CString & line, TCustomSignature*& rpOutSig )
{
int n = GetCurrent()->GetCount();
for (int i = 0; i < n; ++i)
   {
   TCustomSignature* pSig = GetpSig(i);
   if (pSig->GetShortName() == line)
      {
      rpOutSig = pSig;
      return i;
      }
   }

return -1;
}

////////////// Utility Function /////////////////////
void TOptionsSignatureDlg::ShowThis (TCustomSignature* pSig)
{
   CString sigText;
   pSig->GetSignature( sigText, this );
   GetEdit()->SetWindowText( sigText );

   CWnd* pStatic = GetDlgItem(IDC_SIGNATURE_EDITDESC);
   if (pStatic)
      {
      CString final;
      AfxFormatString1 ( final, IDS_SIGNATURE_UTIL1, pSig->GetShortName() );
      pStatic->SetWindowText ( final );
      }

   if (FALSE == pSig->CanChange())
      {
      m_ReadOnly = TRUE;
      GetEdit()->SetReadOnly(TRUE);

      // disabling this makes the tab-order jump all the way
      // to 'New Sig' button
      }
   else
      {
      m_ReadOnly = FALSE;
      GetEdit()->SetReadOnly(FALSE);
      }

   if (pSig->IsFileBased())
      {
      CString fname = pSig->GetFileName ();
      AfxFormatString1 (sigText, IDS_SIG_CHANGESTOFILE, fname);
      }
   else
      sigText.Empty();

   GetDlgItem(IDC_SIGNATURE_EDITDESC2)->SetWindowText (sigText);
}

////////////// Utility Function /////////////////////
void TOptionsSignatureDlg::SetNoSignature(TCustomSignature* pSig)
{
   GetDefault()->SetWindowText(pSig->GetShortName());
   GetEdit()->SetWindowText(_T(""));
   GetEdit()->SetReadOnly(TRUE);
   m_ReadOnly = TRUE;
}

BOOL TOptionsSignatureDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
   LOGFONT lf;

   // create a fixed point font for the signature edit box
   HDC hdc = ::GetDC(m_hWnd);
   setupCourierFont( 9, hdc, &lf );
   ::ReleaseDC(m_hWnd, hdc);
   m_font.CreateFontIndirect( &lf );

   GetEdit()->SetFont ( &m_font, FALSE );

	GetCurrent()->SetFocus();

   return FALSE;

	//return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

///////////////////////////////////////////////////////////////////////////
//  Pick a file and remember the filename
//
void TOptionsSignatureDlg::OnSignatureInsertfile()
{
   CString fname;
   DWORD   flags = 0;
   BOOL    ret =
	TFileUtil::GetInputFilename(&fname,
                               this,
                               IDS_LOADSIG_TITLE,
                               NULL,               // initDir
                               flags,
                               IDS_FILTER_LOADSIG);

   if (ret)
      {
      CFile fl;
      if (fl.Open (fname, CFile::modeRead | CFile::shareDenyWrite))
         {
         CString data;

         // display text
         UINT len = UINT( fl.GetLength() );
         LPTSTR  ptr = data.GetBuffer(len);
         fl.Read (ptr, len);
         data.ReleaseBuffer(len);
         GetEdit()->SendMessage(EM_REPLACESEL, 0, LPARAM(LPCTSTR(data)));

         // tag this Signature as file-based

         int iSel = GetCurrent()->GetCurSel();
         if (CB_ERR != iSel)
            {
            TCustomSignature* pSig = GetpSig(iSel);
            pSig->SetFileBased (TRUE, fname);
            }
         CString mirrorChanges;
         AfxFormatString1 (mirrorChanges, IDS_SIG_CHANGESTOFILE, fname);
         GetDlgItem(IDC_SIGNATURE_EDITDESC2)->SetWindowText (mirrorChanges);
         }
      }
}

BOOL TOptionsSignatureDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   AfxGetApp () -> HtmlHelp((DWORD)"posting_signatures.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_SIGNATURE);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TOptionsSignatureDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
   OnHelpInfo (NULL);
}
