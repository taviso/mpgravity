/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: friched.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/11 22:28:40  richard_wood
/*  Fixed the XFace corruption on scroll bug for keyboard Down, Up, Page Down, Page Up, mouse wheel scroll, space bar scroll.
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

// FRICHED.H : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TFormRichEdit window

class TFormRichEdit : public CRichEditCtrl
{
	// Construction
public:
	TFormRichEdit();

	void SetWrapWidth(BOOL fWrap);
	BOOL IsSelectionColorConsistent(COLORREF color);
	BOOL SetSelectionColor(COLORREF color);
	int  GetParagraphBounds(int iCharPos, CHARRANGE* pchrg);

	void GetPasteRange(CHARRANGE& chrg){ chrg = m_chrgAfterPaste; }

public:
	virtual ~TFormRichEdit();

protected:
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnRequestResize(NMHDR* pNMHDR, LRESULT* pLResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	void calculateFormatRect (int cx, int cy, BOOL fSizeMessage);
	BOOL m_fWordWrap;
	int m_lastWidth;
	int m_lastHeight;

	CHARRANGE m_chrgAfterPaste;
};
