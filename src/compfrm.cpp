/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: compfrm.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:15  richard_wood
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

// compfrm.cpp -- frame window for posting/mailing windows

#include "stdafx.h"
#include "news.h"
#include "compfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(TComposeFrame, CFrameWnd)

// -------------------------------------------------------------------------
TComposeFrame::TComposeFrame()
{
}

// -------------------------------------------------------------------------
TComposeFrame::~TComposeFrame()
{
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TComposeFrame, CFrameWnd)
			// NOTE - the ClassWizard will add and remove mapping macros here.
      ON_COMMAND(IDM_MESSAGE_CANCEL, OnMessageCancel)
	
	ON_CBN_SELCHANGE (idCompTool_cbxMime, OnMIMEClicked)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TAttachmentDoc* TComposeFrame::GetAttDoc(void)
{
   CDocument* pDoc = GetActiveDocument();
   if (pDoc)
      {
      ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(TAttachmentDoc)));
      return (TAttachmentDoc*) pDoc;
      }
   else
      {
      ASSERT(0);
      return NULL;
      }
}

// -------------------------------------------------------------------------
void TComposeFrame::OnMIMEClicked(void)
{
   TAttachmentDoc* pAttDoc = GetAttDoc();
   if (!pAttDoc)
      return;

   int iNewSetting = m_pToolbar->MimeButtonChecked ();

   // if there already exists an attachment, don't let the user change
   // the state of this combobox
   if (pAttDoc->Att_IsMimeLocked ()) {
      int iUsingMime = pAttDoc->Att_UsingMime ();
      if ((iUsingMime && !iNewSetting) || (!iUsingMime && iNewSetting)) {

         // user tried to change the setting
         CString str; str.LoadString (IDS_ERR_DEL_ATTACHMENTS);
         CString name; name.LoadString (IDS_PROG_NAME);
         MessageBox (str, name);

         // set the setting back to the proper value...
         // first combobox item --> uuencode
         // second combobox item --> mime
         m_pToolbar->m_cbxMIME.SetCurSel (iUsingMime ? 1 : 0);
         }
      return;
      }

   pAttDoc->Att_SetUsingMime (iNewSetting);
}

// -------------------------------------------------------------------------
void TComposeFrame::OnMessageCancel(void)
{
   PostMessage(WM_CLOSE);
}
