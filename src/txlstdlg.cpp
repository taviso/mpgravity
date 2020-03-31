/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: txlstdlg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:23  richard_wood
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

// txlstdlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "txlstdlg.h"
#include "mplib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TExceptionList dialog

TExceptionList::TExceptionList(CWnd* pParent /*=NULL*/)
   : CDialog(TExceptionList::IDD, pParent)
{
   
   
}

void TExceptionList::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   
   
}

BEGIN_MESSAGE_MAP(TExceptionList, CDialog)
      
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TExceptionList::AdjustListboxHorizSize (const char *pchToken)
{
   CListBox * pListBox = (CListBox *) GetDlgItem (IDC_XLIST);
   CDC *psCDC = pListBox->GetDC ();
   CSize sSize = psCDC -> GetTextExtent (pchToken, strlen (pchToken));
   int iExtent = pListBox->GetHorizontalExtent ();
   if (iExtent < sSize.cx)
      pListBox->SetHorizontalExtent (sSize.cx);
}

/////////////////////////////////////////////////////////////////////////////
// TExceptionList message handlers

BOOL TExceptionList::OnInitDialog() 
{

   // fill the listbox with the rest of the errors
   CString        errString;
   ESeverity      kSeverity;

   TErrorIterator iterate = m_pExcept->CreateIterator ();
   CListBox *pLB = (CListBox *) GetDlgItem (IDC_XLIST);

   // skip over the error that is already in the dialog
   iterate.Next (errString, kSeverity);
   while (iterate.Next (errString, kSeverity))
      {
      AdjustListboxHorizSize (errString);
      pLB->InsertString (-1, errString);
      }
      
   CDialog::OnInitDialog();

   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}
