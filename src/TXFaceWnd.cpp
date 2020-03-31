/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TXFaceWnd.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:08  richard_wood
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

// TXFaceWnd.cpp : implementation file
//
//

#include "stdafx.h"
#include "news.h"
#include "TXFaceWnd.h"

extern "C" {
#include "libwinface.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TXFaceWnd
TXFaceWnd::TXFaceWnd()
{
	m_pixels = NULL;
	m_DIB = NULL;
}

TXFaceWnd::~TXFaceWnd()
{
	if (m_DIB)
		DeleteObject(m_DIB);
	m_DIB = NULL;
	m_pixels = NULL;
}

BEGIN_MESSAGE_MAP(TXFaceWnd, CWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

/* creates a 48x48x1 DIB section for use with the other functions */
HBITMAP create_face_dib(void **pixel_ptr)
{
	BITMAPINFO *bmi;
	HBITMAP b;
	RGBQUAD colors[] = {{0xff,0xff,0xff},{0,0,0,0}};

	bmi = (BITMAPINFO*) malloc(sizeof(BITMAPINFO) + sizeof(colors) * sizeof(RGBQUAD));

	memcpy(&bmi->bmiColors, colors, sizeof(colors) * sizeof(RGBQUAD));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = 48;
	bmi->bmiHeader.biHeight = -48; /* top-down bitmap instead of bottom-up */
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 1;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0;
	bmi->bmiHeader.biXPelsPerMeter = 0;
	bmi->bmiHeader.biYPelsPerMeter = 0;
	bmi->bmiHeader.biClrUsed = 2;
	bmi->bmiHeader.biClrImportant = 2;

	b = CreateDIBSection(0, bmi, 0, pixel_ptr, 0, 0);
	free(bmi);

	return b;
}

/////////////////////////////////////////////////////////////////////////////
// TXFaceWnd message handlers
int TXFaceWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	RECT rct;
	rct.top = rct.left = 0;
	rct.right = rct.bottom = 50;
	wndStatic.Create("", SS_NOTIFY | SS_BITMAP | WS_VISIBLE, rct, this, 1);	

	m_DIB = create_face_dib(&m_pixels);

	wndStatic.SetBitmap(m_DIB);

	char* p =  (char*)m_pixels;
	for (int i=0; i<48*8; i++) 
	{
		*p++= (char)0xaa;
	}

	return 0;
}

void TXFaceWnd::SetXFace (LPCTSTR pszLine) 
{
	if (m_DIB && m_pixels)
	{
		face_to_bitmap(pszLine, m_pixels, 8);
	}

	if (::IsWindow(wndStatic.m_hWnd))
		wndStatic.RedrawWindow();
}
