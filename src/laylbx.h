/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: laylbx.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

// laylbx.h : header file
//
#pragma once

#include "uimem.h"

typedef struct {
   int                   pictIdx;
   TUIMemory::EMdiLayout layout;
} LAYSYNC;

/////////////////////////////////////////////////////////////////////////////
// TLayoutLbx window

class TLayoutLbx : public CListBox
{
// Construction
public:
	TLayoutLbx(int idBitmap);

// Attributes
public:

// Operations
public:
   void FillPictures(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TLayoutLbx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TLayoutLbx();
 	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// Generated message map functions
protected:
	//{{AFX_MSG(TLayoutLbx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
   CImageList m_images;

	DECLARE_MESSAGE_MAP()

protected:
   virtual int GetLaySync(const LAYSYNC* & pSync, int& total) {ASSERT(0); return 0;}

   void basic_draw (LPDRAWITEMSTRUCT lpDraw);
   void OnDrawSelect(LPDRAWITEMSTRUCT lpDraw);
   void OnDrawFocus(LPDRAWITEMSTRUCT lpDraw);
   int  get_picture_index(TUIMemory::EMdiLayout theLayout);
};
