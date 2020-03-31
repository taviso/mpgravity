/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tmandec.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:09  richard_wood
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

// tmandec.cpp -- manual decoding dialog

#include "stdafx.h"              // windows API
#include "tmandec.h"             // this file's prototypes
#include "tdecthrd.h"            // gpDecodeThread, ...
#include "tdecjob.h"             // TDecodeJob
#include "newsgrp.h"             // TNewsGroup

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TManualDecode *gpsManualDecodeDlg;

// -------------------------------------------------------------------------
TManualDecode::TManualDecode(CWnd* pParent /* = NULL*/)
   : CDialog(TManualDecode::IDD, pParent)
{


   m_iScrollExtent = 0;
}

// -------------------------------------------------------------------------
void TManualDecode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DELETE, m_sDelete);
	DDX_Control(pDX, IDC_PARTS, m_sParts);
	DDX_Control(pDX, IDC_UP, m_sUp);
	DDX_Control(pDX, IDC_DOWN, m_sDown);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TManualDecode, CDialog)
		ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
void TManualDecode::GetPart (int iIndex, CString &strSubject, DWORD &dwData)
{
   m_sParts.GetText (iIndex, strSubject);
   dwData = m_sParts.GetItemData (iIndex);
}

// -------------------------------------------------------------------------
void TManualDecode::InsertPart (CString strSubject, DWORD dwData,
   int iIndex /* = -1 */)
{
   // m_sParts is a listbox
   if (iIndex == -1)
      iIndex = m_sParts.GetCount ();
   m_sParts.InsertString (iIndex, strSubject);
   m_sParts.SetItemData (iIndex, dwData);

   CClientDC sDC(&m_sParts);
   CSize sSize = sDC.GetTextExtent (strSubject);

   if (sSize.cx > m_iScrollExtent) {
      m_iScrollExtent = sSize.cx;
      // set the horizontal scroll range. Pass in the whole number,
      //  the listbox figures the Delta
      m_sParts.SetHorizontalExtent ( m_iScrollExtent );
      }
}

// -------------------------------------------------------------------------
BOOL TManualDecode::OnInitDialog()
{
	CDialog::OnInitDialog();

   // insert our headers into the listbox
   POSITION iPos = m_rpHeaders.GetHeadPosition ();
   TArticleHeader *pHeader;
   while (iPos) {
      pHeader = (TArticleHeader *) m_rpHeaders.GetNext (iPos);
      InsertPart (pHeader -> GetSubject (), (DWORD) pHeader);
      }

   // if there's only one part, just do the manual decode and get out
   if (m_sParts.GetCount () <= 1)
      PostMessage (WM_COMMAND, IDOK);

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// -------------------------------------------------------------------------
void TManualDecode::AddPart (TArticleHeader *psHeader, TNewsGroup *psNG)
{
   m_psNG = psNG;

   // put the header into the header-list
   m_rpHeaders.AddTail (psHeader);
}

// -------------------------------------------------------------------------
void TManualDecode::OnDown()
{
   int iIndex = m_sParts.GetCurSel ();
   if (iIndex == LB_ERR || iIndex >= m_sParts.GetCount () - 1)
      return; // no item is selected, or the selected item is already at bottom

   CString strSubject;
   DWORD dwData;
   GetPart (iIndex, strSubject, dwData);
   m_sParts.DeleteString (iIndex);
   InsertPart (strSubject, dwData, iIndex + 1);

   m_sParts.SetCurSel (iIndex + 1);
   GotoDlgCtrl (&m_sParts);
}

// -------------------------------------------------------------------------
void TManualDecode::OnUp()
{
   int iIndex = m_sParts.GetCurSel ();
   if (iIndex == LB_ERR || !iIndex)
      return; // no item is selected, or the selected item is already on top

   CString strSubject;
   DWORD dwData;
   GetPart (iIndex, strSubject, dwData);
   m_sParts.DeleteString (iIndex);
   InsertPart (strSubject, dwData, iIndex - 1);

   m_sParts.SetCurSel (iIndex - 1);
   GotoDlgCtrl (&m_sParts);
}

// -------------------------------------------------------------------------
void TManualDecode::OnDelete()
{
   int iIndex = m_sParts.GetCurSel ();
   if (iIndex == LB_ERR)
      return; // no item is selected, or the selected item is already on top

   m_sParts.DeleteString (iIndex);
   if (m_sParts.GetCount () - 1 < iIndex)
      iIndex --;
   m_sParts.SetCurSel (iIndex);
   GotoDlgCtrl (&m_sParts);
}

// -------------------------------------------------------------------------
// fixed up March 19, 2003
void TManualDecode::OnOK()
{
   // make up a new (unique) prefix, and queue each part for decoding, then
   // exit and invoke the image-gallery

   static int iCounter;

   CString           strFirstPartSubject;
   CString           strSubject;
   CString           strDirectory = "";
   DWORD             dwData;
   TDecodeJob *      pJob = NULL;
   iCounter ++;

   int iLen = m_sParts.GetCount ();

   for (int i = 0; i < iLen; i++)
      {
      CString strPartSubject;
      GetPart (i, strPartSubject, dwData);

      VEC_HDRS * pVecHdrs = NULL;

      if (i == 0)
         {
         strFirstPartSubject = strPartSubject;
         }

      TArticleHeader * psHeader = (TArticleHeader *) dwData;

      CString str; str.LoadString (IDS_MANUAL_SUBJECT);
      strSubject.Format (str, iCounter, strFirstPartSubject, i+1, iLen);

      // queue the article
      pJob = new TDecodeJob (m_psNG -> m_GroupID,
                                m_psNG -> GetName(),
                                m_psNG -> GetBestname(),
                                psHeader,
                                strDirectory,
                                TRUE /* iRuleBased */,
                                NULL,
                                (char *) (LPCTSTR) strSubject);

      gpDecodeThread -> AddJob (pJob,
                                pVecHdrs,  // null in this case
                                FALSE /* bEliminateDuplicate */,
                                FALSE /* fPriority */);
      } 

	CDialog::OnOK();
}
