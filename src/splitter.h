/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: splitter.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:53  richard_wood
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

// splitter.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TSplitter window
const int PANE_CX_MIN = 10;
const int PANE_CY_MIN = 10;
const int PANE_CX_DEF = 130;
const int PANE_CY_DEF = 100;

class TSplitterWnd : public CSplitterWnd
{
// Construction
public:
	TSplitterWnd();

// Attributes
public:
   BOOL HaveView (const CView * pView, POINT * ppt);
   BOOL IsZoomed ()  { return m_byZoomed; }

   void ResizeZoomPane (int cx, int cy);

// Operations
public:
   void my_get_info(int row, int col, SIZE& sz);
   void my_set_info(int row, int col, RECT& rct, SIZE& sz);

   // set to some default
   void SetRowHeight(int row, int cy)  { SetRowInfo(row, cy, PANE_CY_MIN); }
   void SetColWidth(int col,  int cx)  { SetColumnInfo(col, cx, PANE_CX_MIN); }

   int  ZoomView (CView * pView, POINT & pt);
   int  UnZoomView (BOOL fWindowIsVisible);

protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TSplitterWnd)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TSplitterWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(TSplitterWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
   BYTE m_byZoomed;
   CView * m_pTravellingView;
};

/////////////////////////////////////////////////////////////////////////////
