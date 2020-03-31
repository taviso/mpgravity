/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: laylbx.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:29  richard_wood
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

// laylbx.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "laylbx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TLayoutLbx

TLayoutLbx::TLayoutLbx(int idBitmap)
{
   m_images.Create (idBitmap,
                 48, // frame width
                 1,
                 RGB(255,0,255) /*hot purple is transparent*/);
}

TLayoutLbx::~TLayoutLbx()
{
}

BEGIN_MESSAGE_MAP(TLayoutLbx, CListBox)
		ON_WM_CREATE()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TLayoutLbx message handlers

int TLayoutLbx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
   FillPictures();
	return 0;
}

void
TLayoutLbx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
   IMAGEINFO imi;

   BOOL fRet = m_images.GetImageInfo ( 0, &imi );

   lpMeasureItemStruct->itemHeight = (imi.rcImage.bottom -  imi.rcImage.top);
   lpMeasureItemStruct->itemWidth = (imi.rcImage.right -  imi.rcImage.left);
}

void
TLayoutLbx::DrawItem(LPDRAWITEMSTRUCT lpDraw)
{
   // empty listbox
   if (lpDraw->itemID == -1)
      return;

	//
   if (lpDraw->itemAction & ODA_DRAWENTIRE)
      {
      basic_draw ( lpDraw );
      if (lpDraw->itemState & ODS_SELECTED)
         OnDrawSelect( lpDraw );
      if (lpDraw->itemState & ODS_FOCUS)
         OnDrawFocus ( lpDraw );
      return;
      }

   // Draw hilite color, draw text
   if (lpDraw->itemAction & ODA_SELECT)
      {
      OnDrawSelect ( lpDraw );
      return;
      }

   // Draw focus rect
   if (lpDraw->itemAction & ODA_FOCUS)
      {
      OnDrawFocus ( lpDraw );
      return;
      }
}

void TLayoutLbx::basic_draw(LPDRAWITEMSTRUCT lpDraw)
{
   TUIMemory::EMdiLayout theLayout = (TUIMemory::EMdiLayout) lpDraw->itemData;

   int pix_idx = get_picture_index(theLayout);
   if (pix_idx >= 0)
      {
      CDC theDC;
      theDC.Attach (lpDraw->hDC);
      POINT pt; pt.x = lpDraw->rcItem.left; pt.y = lpDraw->rcItem.top;
      m_images.Draw ( &theDC, pix_idx, pt, ILD_NORMAL );
      theDC.Detach ();
      }
   else
      {
      ASSERT(0);
      }
   return;
}

void TLayoutLbx::OnDrawSelect(LPDRAWITEMSTRUCT lpDraw)
{
// doesn't matter if it is On or Off, invert, re-invert to normal
   CDC theDC;
   theDC.Attach (lpDraw->hDC);
   theDC.InvertRect ( &lpDraw->rcItem );
   theDC.Detach ();
   return;
}

void TLayoutLbx::OnDrawFocus(LPDRAWITEMSTRUCT lpDraw)
{
   CDC theDC;
   theDC.Attach (lpDraw->hDC);
   theDC.DrawFocusRect ( &lpDraw->rcItem );
   theDC.Detach ();
   return;
}

int TLayoutLbx::get_picture_index(TUIMemory::EMdiLayout theLayout)
{
   int total;
   const LAYSYNC* pSync;
   GetLaySync(pSync, total);

   //  pictIdx;
   //  layout;
   for (int i = 0; i < total; ++i, pSync++)
      {
      if (pSync->layout == theLayout)
         return pSync->pictIdx;
      }
   return -1;
}

void TLayoutLbx::FillPictures(void)
{
   int total;
   const LAYSYNC* pSync;
   GetLaySync(pSync, total);

   IMAGEINFO imi;
   BOOL fRet = m_images.GetImageInfo ( 0, &imi );

   //  pictIdx;
   //  layout;
   CString msg;
   for (int i = 0; i < total; ++i, pSync++)
      {
      msg.Format("%d", i+1);

      // If LBS_HASSTRINGS is Off, then LB_GETITEMDATA returns 
      // whatever you use in InsertString

      //idx = InsertString(-1, "");
      //ASSERT(idx != LB_ERR);
      // If LBS_HASSTRINGS is ON, then you must use SETITEMDATA
      
      //if (LB_ERR == SetItemData(idx, pSync->layout))
      //   ASSERT(0);
      SendMessage(LB_INSERTSTRING, WPARAM(-1), 
                                     LPARAM(pSync->layout));
      //idx = AddString ( LPCTSTR(pSync->layout));

      SetItemHeight (i, imi.rcImage.bottom -  imi.rcImage.top); 
      }

   // Can't use MEASUREITEMSTRUCT.  Windows measures before Creation. We subclass
   //   after creation.  This adjusts the height.

   SetColumnWidth (imi.rcImage.right - imi.rcImage.left);
}
