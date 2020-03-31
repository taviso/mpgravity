/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: gdiutil.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:25  richard_wood
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
#include "gdiutil.h"
#include "autofont.h"

#include "tglobopt.h"
#include "rgui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void fnCalcPointSize(CDC* pdc, LPLOGFONT plf, int* pPointSize)
{
   int Ypels = pdc->GetDeviceCaps (LOGPIXELSY);

   if (plf->lfHeight < 0)
      {
      // this calculation is very accurate
      *pPointSize = MulDiv(plf->lfHeight, -72, Ypels);
      }
   else
      {
      // I don't know if this clause ever happens
      CFont aFont;
      TEXTMETRIC tm;
      aFont.CreateFontIndirect ( plf );

      TAutoFont  restoreFont(pdc, &aFont);
      pdc->GetTextMetrics (&tm);

      int Pels = tm.tmHeight;

      *pPointSize = MulDiv(72, Pels, Ypels);
      }
}

void setupSansSerifFont(int pointSize, HDC hdc,
 LPLOGFONT plf, BOOL fArial/*=FALSE*/)
{
   ZeroMemory(plf, sizeof(*plf));
   plf->lfHeight = -MulDiv(pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
   plf->lfWeight = 400;
   plf->lfCharSet        = ANSI_CHARSET;
   plf->lfOutPrecision   = OUT_DEFAULT_PRECIS;
   plf->lfClipPrecision  = CLIP_DEFAULT_PRECIS;
   plf->lfQuality        = DEFAULT_QUALITY;
   plf->lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
   strcpy(plf->lfFaceName, fArial ? "Arial" : "MS Sans Serif");
}

void setupCourierFont(int pointSize, HDC hdc, LPLOGFONT plf)
{
   ZeroMemory(plf, sizeof(*plf));
   plf->lfHeight = -MulDiv(pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
   plf->lfWeight = 400;
   plf->lfCharSet        = ANSI_CHARSET;
   plf->lfOutPrecision   = OUT_DEFAULT_PRECIS;
   plf->lfClipPrecision  = CLIP_DEFAULT_PRECIS;
   plf->lfQuality        = DEFAULT_QUALITY;
   plf->lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
   strcpy(plf->lfFaceName, "Courier New");
}

TAutoClipRect::TAutoClipRect (CDC* pDC, CRgn* pRegion, int left, int top, int right, int bottom)
   : m_pDC(pDC), m_pRegion(pRegion)
{
   CRect rctInput(left, top, right, bottom);

   pDC->GetClipBox( &m_rctCur );
   pDC->IntersectClipRect ( &rctInput );
}

TAutoClipRect::~TAutoClipRect ()
{
   // restore original
   m_pRegion->SetRectRgn(m_rctCur.left, m_rctCur.top, m_rctCur.right, m_rctCur.bottom);
   m_pDC->SelectClipRgn ( m_pRegion );
}

/////////////////////////////////////////////////////////////////////////////
void setMarginRichEdit (CRichEditCtrl & rich, int cx, int cy)
{
   if (cx < 13)
      return;

   // add a decent Left margin

   CRect rct(5, 0, cx-7, cy);

   rich.SetRect ( rct );
}

/////////////////////////////////////////////////////////////////////////////
// returns true for success, false for user canceled
bool gdiCustomColorDlg (CWnd * pParentWnd, COLORREF crf, COLORREF & crfOutput)
{
   CColorDialog colorDlg (crf, 0, pParentWnd);

   COLORREF vCustomColors[16];

   gpGlobalOptions->GetRegUI()->LoadCustomColors (16, vCustomColors);
   colorDlg.m_cc.lpCustColors = vCustomColors;

   if (IDOK != colorDlg.DoModal ())
      return false;
   else
      {
      crfOutput = colorDlg.GetColor ();	

      gpGlobalOptions->GetRegUI()->SaveCustomColors (16, vCustomColors);
      return true;
      }
}

