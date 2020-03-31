/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tmpllbx.cpp,v $
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

// tmpllbx.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "tmpllbx.h"
#include "globals.h"
#include "custview.h"
#include "tglobopt.h"
#include "autofont.h"
#include "rgbkgrnd.h"
#include "tdlgtmpv.h"
#include "sysclr.h"    // TSystem

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TTemplateLbx

TTemplateLbx::TTemplateLbx()
{
   m_pParentView = 0;
}

TTemplateLbx::~TTemplateLbx()
{
}

BEGIN_MESSAGE_MAP(TTemplateLbx, CListBox)
		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TTemplateLbx message handlers

void TTemplateLbx::DrawItem(LPDRAWITEMSTRUCT lpDraw)
{
	// TODO: Add your message handler code here and/or call default
   if (lpDraw->itemID == -1)
      return;

   if (lpDraw->itemAction & ODA_DRAWENTIRE)
      {
      //TRACE0("DRAWENTIRE\n");
      // Draw Text normally or Selected Text
      OnDrawSelect( lpDraw );
      if (lpDraw->itemState & ODS_FOCUS)
         OnDrawFocus ( lpDraw );
      return;
      }

   // Draw hilite color, draw text
   if (lpDraw->itemAction & ODA_SELECT)
      {
      //TRACE1("DRAWSELECT %d\n", lpDraw->itemID);
      OnDrawSelect ( lpDraw );
      return;
      }

   // Draw focus rect
   if (lpDraw->itemAction & ODA_FOCUS)
      {
      //TRACE2("DRAWFOCUS %d %d\n", lpDraw->itemID,
      //             lpDraw->itemState & ODS_FOCUS);
      OnDrawFocus ( lpDraw );
      return;
      }
}

void TTemplateLbx::OnDrawSelect(LPDRAWITEMSTRUCT lpDraw)
{
   DWORD dwBack;
   DWORD dwText;

   TArticleItemViewUI* pUI = (TArticleItemViewUI*)  lpDraw->itemData;

   if (lpDraw->itemState & ODS_SELECTED)
   {
      dwBack = gSystem.Highlight();
      dwText = gSystem.HighlightText();
   }
   else
   {
      TBackgroundColors *pBack = gpGlobalOptions->GetBackgroundColors();

      dwText = pUI->m_item.GetColor();
      dwBack = gSystem.Window();

      if (m_pParentView && !m_pParentView->m_defBackground)
         dwBack = pBack->GetArtviewBackground();
   }

   CDC* pDC = CDC::FromHandle (lpDraw->hDC);

   // Fill the background
   CBrush br(dwBack);
   pDC->FillRect (&lpDraw->rcItem, &br);

	//POINT pt;
	// Add 2 pixel margin from the edge.
	//pt.x = lpDraw->rcItem.left + 2;
	//pt.y = lpDraw->rcItem.top;
	//m_imageList.Draw ( pDC, 1, pt, ILD_NORMAL );

   // Set the textcolor & background mode
   COLORREF oldTextColor = pDC->SetTextColor ( dwText );
   int oldMode = pDC->SetBkMode (TRANSPARENT);

   this->BlastText ( pDC, lpDraw );          // utility func
   pDC->SetBkMode (oldMode);
   pDC->SetTextColor ( oldTextColor );
}

void TTemplateLbx::OnDrawFocus(LPDRAWITEMSTRUCT lpDraw)
{
   CDC* pDC;
   pDC = CDC::FromHandle (lpDraw->hDC);
   pDC->DrawFocusRect ( &lpDraw->rcItem );
}

void TTemplateLbx::BlastText (CDC * pDC, LPDRAWITEMSTRUCT lpDraw)
{

   TArticleItemViewUI* pUI = (TArticleItemViewUI*)  lpDraw->itemData;
   CString& rText = pUI->text;

   TAutoFont fontmgr(pDC, pUI->GetFont() );

   pDC->TextOut (lpDraw->rcItem.left + 2, lpDraw->rcItem.top, rText, rText.GetLength() );
}

void TTemplateLbx::CalcFontheight (
TArticleItemViewUI* pUI,
int& y
)
{
   CDC* pDC	= GetDC();
   TEXTMETRIC  tm;

   {
      TAutoFont fontmgr(pDC, pUI->GetFont());

      // get height of this font
      pDC->GetTextMetrics ( &tm );
	   y = tm.tmHeight + tm.tmExternalLeading;
   }

   ReleaseDC( pDC );
}

void TTemplateLbx::CorrectItemHeight(int idx)
{
   TArticleItemViewUI* pUI = GetUIDataPtr(idx);
   int height;
   CalcFontheight ( pUI, height );
   SetItemHeight ( idx, height );
}

void TTemplateLbx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
   int cnt;
   if ((cnt = GetCount()) == 0)
      return;

   TArticleItemViewUI* pUI = (TArticleItemViewUI*)  lpMeasureItemStruct->itemData;
   ASSERT(pUI);

   CalcFontheight ( pUI, cnt );

   lpMeasureItemStruct->itemHeight = cnt;
}

int TTemplateLbx::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	
	//return CListBox::OnCompareItem(nIDCtl, lpCompareItemStruct);
   return 0;
}

void TTemplateLbx::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
   TArticleItemViewUI* pUI = (TArticleItemViewUI*)  lpDeleteItemStruct->itemData;
   delete pUI;
}

// the key point. after exchanging, Windows does not send us another WM_MEASUREITEMDATA.
void TTemplateLbx::Exchange(int lo, int hi)
{
   // swap the dataptrs and then calculate
   TArticleItemViewUI* pUIone;
   TArticleItemViewUI* pUItwo;

   // just swap dataptrs, but also set height.

   pUIone = GetUIDataPtr (lo);
   pUItwo = GetUIDataPtr (hi);

   int Yone, Ytwo;
   Yone = GetItemHeight(lo);
   Ytwo = GetItemHeight(hi);

   SetItemDataPtr( hi, pUIone);
   SetItemDataPtr( lo, pUItwo);

   SetItemHeight (lo, Ytwo);
   SetItemHeight (hi, Yone);
}

void TTemplateLbx::SmartRefresh(int idx)
{
   RECT both;
   RECT     rct1, rct2;

   GetItemRect ( idx, &rct1 );
   GetItemRect ( 1 + idx, &rct2 );

   UnionRect (&both, &rct1, &rct2);
   InvalidateRect ( &both );
}
