/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: laytlbx.cpp,v $
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

// laytlbx.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "laytlbx.h"
#include "globals.h"
#include "sysclr.h"   // TSystem

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TLayoutTextLbx

TLayoutTextLbx::TLayoutTextLbx()
{
}

TLayoutTextLbx::~TLayoutTextLbx()
{
}

BEGIN_MESSAGE_MAP(TLayoutTextLbx, CListBox)
	// NOTE - the ClassWizard will add and remove mapping macros here.

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TLayoutTextLbx message handlers

void TLayoutTextLbx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	int h;
	CalcItemHeight (h);
	lpMeasureItemStruct->itemHeight = h;
}

void TLayoutTextLbx::CalcItemHeight(int& h, CFont* pFont)
{
	TEXTMETRIC  tm;
	CDC* pDC	= GetDC();
	CFont * pOldFont;

	if (pFont)
		pOldFont = pDC->SelectObject ( pFont );

	// get height of this font
	pDC->GetTextMetrics ( &tm );
	h = tm.tmHeight + tm.tmExternalLeading;

	if (pFont)
		pDC->SelectObject ( pOldFont );

	ReleaseDC( pDC );
}

void TLayoutTextLbx::DrawItem(LPDRAWITEMSTRUCT lpDraw)
{
	// empty listbox
	if (lpDraw->itemID == -1)
		return;

	//
	if (lpDraw->itemAction & ODA_DRAWENTIRE)
	{
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

void TLayoutTextLbx::OnDrawSelect(LPDRAWITEMSTRUCT lpDraw)
{
	DWORD dwBack;
	DWORD dwText;

	if (lpDraw->itemState & ODS_SELECTED)
	{
		dwBack = gSystem.Highlight();
		dwText = gSystem.HighlightText();
	}
	else
	{
#if 1
		dwBack = gSystem.Window();
#else
		// force the grey background
		dwBack = gSystem.ColorScrollbar();
#endif
		dwText = gSystem.WindowText();
	}

	CDC theDC;
	theDC.Attach (lpDraw->hDC);

	// Fill the background
	CBrush br(dwBack);
	theDC.FillRect (&lpDraw->rcItem, &br);

	// Set the textcolor & background mode
	COLORREF oldTextColor = theDC.SetTextColor ( dwText );
	int oldMode = theDC.SetBkMode (TRANSPARENT);

	this->BlastText ( &theDC, lpDraw );          // utility func

	theDC.SetBkMode (oldMode);
	theDC.SetTextColor ( oldTextColor );
	theDC.Detach ();
}

void TLayoutTextLbx::OnDrawFocus(LPDRAWITEMSTRUCT lpDraw)
{
	DrawFocusRect (lpDraw->hDC, &lpDraw->rcItem );
}

void TLayoutTextLbx::BlastText (CDC * pDC, LPDRAWITEMSTRUCT lpDraw)
{
	CString final;
	CString txt;
	GetText (lpDraw->itemID, txt);

	final.Format ("%d %s", 1 + lpDraw->itemID, (LPCTSTR) txt);

	int sz = final.GetLength();

	RECT rct;
	rct = lpDraw->rcItem;

	//pDC->TextOut (lpDraw->rcItem.left + iconWidth, lpDraw->rcItem.top, rName, sz);
	pDC->DrawText (final, sz, &rct, DT_LEFT | DT_NOPREFIX | DT_VCENTER);
}

