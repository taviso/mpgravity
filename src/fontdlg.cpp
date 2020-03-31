/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fontdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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

#include "stdafx.h"
#include "resource.h"
#include <dlgs.h>
#include "fontdlg.h"
#include "autofont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP (CNewFontDialog, CFontDialog)
   ON_WM_PAINT()
END_MESSAGE_MAP()

int CNewFontDialog::DoModal()
{
//   m_cf.Flags |= CF_ENABLETEMPLATE;
//   m_cf.hInstance = AfxGetResourceHandle ();
//   m_cf.lpTemplateName = MAKEINTRESOURCE(FORMATDLGORD31);
   return CFontDialog::DoModal ();
}

void CNewFontDialog::OnPaint ()
{
   CPaintDC dc(this);
   CString FaceName;
   GetDlgItem(1136)->GetWindowText (FaceName);
   CString Text;
   GetDlgItem (1137)->GetWindowText (Text);
   int nWeight = FW_NORMAL;

   BOOL bItalic = FALSE;
   Text.MakeUpper ();
   CString strBold; strBold.LoadString (IDS_BOLD);
   CString strItalic; strBold.LoadString (IDS_ITALIC);
   CString strBoldItalic; strBold.LoadString (IDS_BOLD_ITALIC);
   if (Text == strBold)
      nWeight = FW_BOLD;
   else if (Text == strItalic)
      bItalic = TRUE;
   else if (Text == strBoldItalic)
      {
      nWeight = FW_BOLD;
      bItalic = TRUE;
      }
   GetDlgItem (1138)->GetWindowText (Text);
   int nSize = atoi ((char *) (const char *) Text);
   BOOL bStrikeout = ((CButton *) GetDlgItem (1040))->GetCheck ();
   BOOL bUnderline = ((CButton *) GetDlgItem (1041))->GetCheck ();
   CComboBox * pCombo = (CComboBox *) GetDlgItem (1139);
   int nCurSel = pCombo->GetCurSel ();
   DWORD dwColor = pCombo->GetItemData (nCurSel);
   GetDlgItem(stc5)->GetWindowText (Text);
   CFont theFont;
   int nLogicalPixelsY = dc.GetDeviceCaps (LOGPIXELSY);
   nSize = -1 * (nLogicalPixelsY * nSize / 72);
   theFont.CreateFont (nSize, 0, 0, 0, nWeight, bItalic,
                       bUnderline, bStrikeout, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FaceName);

   TAutoFont fontmgr (&dc, &theFont);     // destructor restores old font

   CRect rect;
   GetDlgItem (1092)->GetWindowRect (&rect);
   ScreenToClient (&rect);
   CBrush brush(m_background);
   dc.FillRect (&rect, &brush);
   dc.SetBkColor (m_background);
   dc.SetTextColor (dwColor);
   dc.DrawText (Text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

