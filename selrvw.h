/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: selrvw.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 15:48:56  richard_wood
/*  Stopped the Undo buffer in the compose window from filling up with colour wrap operations.
/*  Undo does nothing at all for the moment (needs proper fix).
/*
/*  Revision 1.2  2008/09/19 14:51:49  richard_wood
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

// selrvw.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// TSelectRichEditView view

#include <afxrich.h>

class TSelectRichEditView : public CRichEditView
{
protected:
	TSelectRichEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TSelectRichEditView)

public:
	void SetWrapWidth(BOOL fWrap);
	BOOL IsSelectionColorConsistent(COLORREF color);
	BOOL SetSelectionColor(COLORREF color);
	int  GetParagraphBounds(int iCharPos, CHARRANGE* pchrg);
	void GetPasteRange(CHARRANGE& chrg){ chrg = m_chrgAfterPaste; }

protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual ~TSelectRichEditView();
	DECLARE_MESSAGE_MAP()

	HRESULT QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT* lpcfFormat,
		DWORD dwReco, BOOL bReally, HGLOBAL hMetaPict);

	BOOL m_fWordWrap;
	CHARRANGE m_chrgAfterPaste;
	CRichEditCtrl *m_pRich;
};
