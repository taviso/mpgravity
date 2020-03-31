/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: odlist.cpp,v $
/*  Revision 1.2  2010/07/24 21:57:03  richard_wood
/*  Bug fixes for Win7 executing file ops out of order.
/*  V3.0.1 RC2
/*
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:38  richard_wood
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
#include "odlist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////
// COwnerDrawListView

COwnerDrawListView::COwnerDrawListView()
{
}

COwnerDrawListView::~COwnerDrawListView()
{
}

///////////////////////////////////////////////////////////////////
// Override. This routine is called by CDialog::OnDrawItem()
//
void COwnerDrawListView::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	// Note that 'lpDIS->itemID' = item id of the current row being
	// painted.

	if(-1 == lpDIS->itemID)
		return;

	// You should get the pointer to the device context from the "lpDIS" ptr.
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	switch(lpDIS->itemAction)
	{
	case  ODA_SELECT:
	case  ODA_DRAWENTIRE:
		////////
		// 1 - Get and Set the listview text items font.
		// 2 - Set the forground and background colors. For
		//     ODS_SELECTED = bkgd(COLOR_HIGHLIGHT),
		//                    fgd(COLOR_HIGHLIGHTTEXT)
		//     otherwise    = bkgd(COLOR_WINDOW),
		//                    fgd(COLOR_WINDOWTEXT)
		// 3 - Save the old font, "forground" and "background"
		//     colors of the device context in the pOldfont and
		//     "crlXXXXXX" variables.
		// YOU MUST RESTORE THE DEVICE CONTEXT TO ITS ORIGINAL
		// STATE by restoring the old font and colors.
		////////
		CFont ItemsTextFont;
		GetItemsFont(ItemsTextFont);
		CFont* pOldFont = (CFont*) pDC->SelectObject(&ItemsTextFont);
		ASSERT(pOldFont);
		COLORREF clrForground, clrBackground;
		clrForground =
			pDC->SetTextColor (GetSysColor(lpDIS->itemState & ODS_SELECTED ?
COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

		clrBackground = pDC->SetBkColor
			(GetSysColor(lpDIS->itemState & ODS_SELECTED ?
COLOR_HIGHLIGHT : COLOR_WINDOW));

		// Populate the listview with column text
		DrawOneRow (lpDIS);

		// REQUIRED: Restore the original device context colors
		//           and font
		pDC->SetTextColor(clrForground);
		pDC->SetBkColor(clrBackground);
		pDC->SelectObject(pOldFont);

		////////
		// Is the item selected? Then draw a rectangle around the
		// entire selection. Note that I'm using 'lpDIS->rcItem'.
		//

		if ((lpDIS->itemState & ODS_FOCUS) &&  (m_hWnd == ::GetFocus()))
		{
			pDC->DrawFocusRect(&lpDIS->rcItem);
		}
		break;
	}
}

///////////////////////////////////////////////////////////////////
// void COwnerDrawListView::_InsertColItem(int iItem, LPARAM lParam)
// {
//    LV_ITEM lvi;
//    lvi.mask       = LVIF_TEXT | LVIF_PARAM;
//    lvi.iItem      = iItem;
//    lvi.iSubItem   = 0;
//    lvi.pszText    = LPSTR_TEXTCALLBACK;
//    lvi.cchTextMax = 0;
//    lvi.lParam     = lParam;
//
//    // Insert the item
//    int nRtnValue = InsertItem(&lvi);
//    ASSERT(-1 != nRtnValue);
//
//    // calc the number of columns
//    int iCols = 0;
//    LV_COLUMN sColumn;
//    sColumn.mask = LVCF_WIDTH;
//    while (GetColumn(iCols, &sColumn))
//       iCols++;
//
//    // Note that we're starting from iSubItem '1'
//    for (int iSubItem = 1; iSubItem < iCols; iSubItem++)
//    {
//       SetItemText(iItem, iSubItem, LPSTR_TEXTCALLBACK);
//    }
// }

///////////////////////////////////////////////////////////////////
//  1-21-97  amc  Check with SystemParametersInfo
//
void COwnerDrawListView::GetItemsFont(CFont& rFont)
{
	LOGFONT sLF;                                // Logical font struct

	// a list control is essentially a smallicon + icon title font
	//  Get the details on the icon title font.

	if (!SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(sLF), &sLF, 0))
	{
		ASSERT(0);
	}
	else
		VERIFY(rFont.CreateFontIndirect ( &sLF ));
}

///////////////////////////////////////////////////////////////////
void COwnerDrawListView::DrawOneRow (LPDRAWITEMSTRUCT lpDIS)
{
	//TRACE("[%s - %d] - COwnerDrawListView::SetRowText().....\n", __FILE__, __LINE__);
	ASSERT_VALID(this);

	////////
	// Retrieve the first column item. Validate 'lParam' mask
	// so that we can retrieve a pointer to the appropriate
	// item list data.
	//

	LV_ITEM  lvi;

	lvi.mask     = LVIF_PARAM;
	lvi.iItem    = lpDIS->itemID;
	lvi.iSubItem = 0;

	VERIFY(GetItem(&lvi));

	////////
	// Retrieve the item rectangle size. The reason I'm setting
	// '.right' to '.left' because of the way 'DrawColItem()'
	// is implemented. A lazy way of avoiding an 'if' statement
	// in it.  [Setting up the zero-th  item]
	//

	CRect rctText  = lpDIS->rcItem;
	rctText.right  = rctText.left;   // see OJO!!

	////////
	// Now, start displaying the columns text.
	//

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	int iColumn = 0;

	// calc the number of columns
	int       iTotCols = getColumnCount ();

	for (iColumn = 0; iColumn < iTotCols; iColumn++)
	{

		// GetColumnWidth is a standard CListCtrl  member function

		rctText.left   = rctText.right;   //  OJO!!
		rctText.right  = rctText.left + GetColumnWidth(iColumn);

		DrawColItem ( pDC, rctText, lvi.iItem, iColumn,  lvi.lParam );

	}
}

///////////////////////////////////////////////////////////////////
// provide implementation of a virtual function
void COwnerDrawListView::DrawColItem (
									  CDC*     pDC,
									  CRect &  rctText,
									  int      iItem,
									  int      iColumn,
									  LPARAM   lParam )
{
	CString strColText;

	// v-func
	getItemText (iItem, iColumn, lParam, strColText );

	DrawTruncatedColItemText (pDC, strColText, rctText);
}

// -------------------------------------------------------------------------
void COwnerDrawListView::getItemText (int iItem, int iCol, LPARAM , CString & outText)
{
	// by default use the standard CListCtrl method
	outText = GetItemText ( iItem , iCol );
}

///////////////////////////////////////////////////////////////////
void COwnerDrawListView::DrawTruncatedColItemText(
	CDC *          pDC,
	CString &      stColText,
	const CRect &  rectText)
{
	////////
	// 1 - Calculate the width of the column in pixels and subtract
	//     '6' from it to take into account the edges of the column
	//     and the added '2' to the left margin in the ExtTextOut()'
	//     call.
	//
	// 2 - Get the size of the text in pixels.
	//

	int nColWidth     = rectText.right - rectText.left - 6;
	int nColTextWidth = GetStringWidth(stColText);

	////////
	// Now, if the width of the colum is LESS than the length of
	// the string in pixels, truncate the string and add '...' to
	// the end of the string to let the user know of it.
	//

	if (nColWidth < nColTextWidth)
	{
		int nDotsTextWidth = GetStringWidth(_T("..."));

		while (nColWidth < (nColTextWidth + nDotsTextWidth))
		{
			// if nothing left, exit before doing the "getlength()-1"
			if (!stColText.GetLength())
				break;

			stColText.GetBufferSetLength(stColText.GetLength()-1);
			stColText.ReleaseBuffer();
			nColTextWidth = GetStringWidth(stColText);
		}
		stColText += _T("...");
	}

	pDC->ExtTextOut (rectText.left + 2,
		rectText.top + 1,
		ETO_OPAQUE | ETO_CLIPPED,
		rectText,
		stColText,
		stColText.GetLength(),
		NULL);
}

// ------------------------------------------------------------------------
int  COwnerDrawListView::getColumnCount ()
{
	// calc the number of columns
	int       iTotCols = 0;
	LV_COLUMN sColumn;

	sColumn.mask = LVCF_WIDTH;

	while (GetColumn (iTotCols, &sColumn))
		iTotCols++;

	return iTotCols;
}
