/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: compview.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/02/19 13:02:02  richard_wood
/*  Enabled optimisations in whole program.
/*  Optimised Compose dialog - creating long articles should be a lot better now.
/*  Increased Ver to 2.7.1.11
/*
/*  Revision 1.3  2009/01/28 15:48:56  richard_wood
/*  Stopped the Undo buffer in the compose window from filling up with colour wrap operations.
/*  Undo does nothing at all for the moment (needs proper fix).
/*
/*  Revision 1.2  2008/09/19 14:51:15  richard_wood
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

// compview.h -- view for posting/mailing windows
// This Compose-Form View window is a base class for Posting, Replying
// and follow up.  The goal is to move some Fixed-Font, Wordwrap code to
// here

#pragma once

#include "basfview.h"
#include "friched.h"
#include "selrvw.h"
#include <afxext.h>
#include <afxrich.h>

// -------------------------------------------------------------------------
class TComposeFormView : public TSelectRichEditView
{
protected:
	TComposeFormView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TComposeFormView)

public:
	void SetSignature(const CString & strSig);
	void SwapSignatures(const CString & strPrev, const CString & strNew);

	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	int  TurnWordWrap(BOOL fOn);
	int  PostInit ();
	void quote_the_original(const CString& orig, const CString& pre, int wrap);
	void GenerateSaysLine(const CString& templat,
		const CString& msgid,
		const CString& date,
		const CString& addr,
		const CString& name,
		long lNewsGroupID);
	void SubstituteMacros (const CString& templat,
		CString &result,
		const CString& msgid,
		const CString& date,
		const CString& addr,
		const CString& name,
		long lNewsGroupID);

	BOOL SetColorWrap(BOOL fOn);
	void strip_signature (const CString& orig, CString& output);

protected:
	virtual ~TComposeFormView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnEditPaste();
	afx_msg void OnInsertTextFile();
	afx_msg void OnArticleRot13();
	afx_msg void OnRichEditUpdate(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

	void SetWordWrap (LOGFONT* pLF, int Tenths_Of_A_Point);
	void SetWordwrapRect();

	void GetPureText (CString * psText );

	// returns handle to utility diff data.
	void GetLineBrokenText(BOOL fOutputString, CString& strOut,
		BOOL fCRC, PCHAR & rpVoid);
	BOOL CheckLineLength(int nChar);
	void InsertTextFile(const CString& fileName);
	void UserInsertTextFile(void);

	BOOL HasQuotePrefix(CHARRANGE& para);
	int  PerformColorWrapPara(CHARRANGE& chrg, int iWrap = -1,
		CHARRANGE* pOrig = 0);
	void PerformColorWrapRange(CHARRANGE& chrg);

	int  EstimateDifferences (PCHAR pMsg1, PCHAR pMsg2);

	int  GetMaxLineLength ();

	void BeginColorWrap(CHARRANGE& chrg);
	void EndColorWrap(CHARRANGE& start);

	int backup_from (int iIdx);
	int forward_from (int iIdx, BOOL& fEOF);

	// returns new length
	int convert_crlf (LPTSTR pString);
	void convert_crlf (CString & str);

	int   m_i80charTwips;
	int   m_iClipDataLen;

private:
	void paste_incremental (LPTSTR pData);
	void paste_incre(LPTSTR & pData, int & iSel, BOOL fWrap);
	void paste_rest (LPTSTR & pData, int & iSel);
	void correct_color (CHARRANGE *pPara);

	BOOL  m_fFontReady;
	int   m_rtfCX;          // correct width for wordwrapping
	HDC   m_hDCRtf;         // target dc for the RichEdit
	int   m_iCharCX;
	CHARFORMAT2 m_defCharFormat;
	BOOL  m_fColorWrap;     // colorwrap on EN_CHANGE
	BOOL  m_fRestoreDraw;
};

// normal C function
void Compose_FreeDiffData (PCHAR & pData);

// ----------------------------------
class TAutoFreeDiff
{
public:
	TAutoFreeDiff(PCHAR & pData)
		: m_pData(pData)
	{
		// empty
	}

	~TAutoFreeDiff ()
	{
		Compose_FreeDiffData ( m_pData );
	}

protected:
	PCHAR & m_pData;
};
