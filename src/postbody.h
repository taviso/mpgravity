/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postbody.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2008/09/19 14:51:40  richard_wood
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

// postbody.h -- body view in posting/mailing windows

#pragma once

#include <afxext.h>
#include "compview.h"

// fwd declaration
class TPostDoc;

// -------------------------------------------------------------------------
class TPostBody : public TComposeFormView
{
protected:
	TPostBody();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TPostBody)

public:
	CString	m_Body;

	TPostDoc* GetDocument(void);
	virtual int GetAccelTableID() { return IDR_POSTTYPE; }
	void SetCCIntro (BOOL bInsert);
	int m_iIntroFinish;        // position in rich-edit where intro is finished

public:
	virtual void OnInitialUpdate();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	~TPostBody();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	BOOL check_format();
	void DataExchangeIn(CDataExchange* pDC, TPostDoc* pDoc);
	BOOL TestFlag (int iFlag) { return (iFlag & m_iTFlags) ? TRUE : FALSE; }
	void CheckSpelling (BOOL bShowComplete);
	void WrapSelection ();
	int CheckLineSpelling (int iLine, LPCTSTR pchQuote, int iQuoteLen, bool &bSigSepReached);
	int CheckWordSpelling (const CString &strWord, int iIndex, int & newIndex);

	afx_msg void OnPostQuotepara();
	DECLARE_MESSAGE_MAP()

	PCHAR  m_pDiffDataStart;      // snapshot of the initial article text

	int m_iTFlags;                // flags from template
	int m_iCCIntroLen;            // length of current CC intro
	CString m_strOriginalAuthor;  // original author's name
	CString m_strOriginalAddress; // original author's address

private:
	void WrapLines (CRichEditCtrl &ctrl, long firstLine, long lastLine, CString prefix, long lineLength);
};
