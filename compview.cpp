/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: compview.cpp,v $
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
/*  Revision 1.7  2009/02/19 13:02:02  richard_wood
/*  Enabled optimisations in whole program.
/*  Optimised Compose dialog - creating long articles should be a lot better now.
/*  Increased Ver to 2.7.1.11
/*
/*  Revision 1.6  2009/02/17 14:28:23  richard_wood
/*  Deleted old commented out code.
/*
/*  Revision 1.5  2009/01/28 15:48:55  richard_wood
/*  Stopped the Undo buffer in the compose window from filling up with colour wrap operations.
/*  Undo does nothing at all for the moment (needs proper fix).
/*
/*  Revision 1.4  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
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

// compview.cpp -- view for posting/mailing windows
//
// Important:  when using ReplaceSel to insert text into the RichEditCntrl
//             convert CRLF to LF. I do this only during OnEditPaste.
//
#include "stdafx.h"
#include "news.h"
#include "compview.h"
#include "richedit.h"
#include "rtfspt.h"
#include "mainfrm.h"
#include "tglobopt.h"
#include "strext.h"
#include "fileutil.h"
#include "rgcomp.h"              // count Quoted lines (Indent String)
#include "rgfont.h"
#include "nglist.h"              // TNewsGroupUseLock
#include "server.h"              // TNewsServer
#include "servcp.h"              // TServerCountedPtr
#include "superstl.h"            // auto_ptr
#include "sysclr.h"
#include "gdiutil.h"             // setMarginRichEdit
#include "crc32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define TYPE_TMR 34

// special signature Token (see son-of-Rfc1036, $4.3.2)
static TCHAR rcSignatureToken[] = "-- \r\n";

extern TGlobalOptions *gpGlobalOptions;

static const int FAR_OUT_TWIPS = 50000;

class TMyDiffData
{
public:
	vector<int> vKeys;
};

IMPLEMENT_DYNCREATE(TComposeFormView, TSelectRichEditView)

// -------------------------------------------------------------------------
TComposeFormView::TComposeFormView()
{
	m_fFontReady = FALSE;
	m_hDCRtf = 0;
	m_fColorWrap = TRUE;
	m_iClipDataLen = 0;
	m_fRestoreDraw = FALSE;
}

// -------------------------------------------------------------------------
TComposeFormView::~TComposeFormView()
{
	if (m_hDCRtf)
		DeleteDC (m_hDCRtf);
}

// -------------------------------------------------------------------------
void TComposeFormView::DoDataExchange(CDataExchange* pDX)
{
	TSelectRichEditView::DoDataExchange(pDX);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TComposeFormView, TSelectRichEditView)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(IDM_INSERT_TEXT_FILE, OnInsertTextFile)
	ON_COMMAND(ID_ARTICLE_ROT13, OnArticleRot13)
	ON_CONTROL_REFLECT(EN_CHANGE, OnRichEditUpdate)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
#ifdef _DEBUG
void TComposeFormView::AssertValid() const
{
	TSelectRichEditView::AssertValid();
}

void TComposeFormView::Dump(CDumpContext& dc) const
{
	TSelectRichEditView::Dump(dc);
}
#endif //_DEBUG

void TComposeFormView::GetPureText (CString * psText)
{
	RTF_TextOut ( m_pRich, psText );
}

// -------------------------------------------------------------------------
// Get the text from the editbox, as the user sees it.
// also return the number of Quoted lines.
//
// if fOutputString==FALSE, then calculate CRC for each line
//
void TComposeFormView::GetLineBrokenText(BOOL fOutputString, CString& strOut,
										 BOOL fCRC, PCHAR & rpVoid)
{
	strOut = "";
	int iLines = m_pRich->SendMessage (EM_GETLINECOUNT);
	int iLength, iIndex;
	const int sz = 1021;
	char rchLine[sz+3];
	TMyDiffData * pDiffs = 0;

	if (fCRC)
		pDiffs = new TMyDiffData;

	// automatic wait-cursor
	CWaitCursor sWait;

	for (int iLine = 0; iLine < iLines; iLine++)
	{
		// get a line from the RTF
		* (WORD *) &rchLine [0] = sz;
		m_pRich->SendMessage (EM_GETLINE, iLine, (LPARAM) (LPCSTR) rchLine);

		iIndex = m_pRich->SendMessage (EM_LINEINDEX, iLine);
		iLength = m_pRich->SendMessage (EM_LINELENGTH, iIndex);

		// null-terminate the line - but reserve room for CRLF
		if (iLength >= sz)
			iLength = sz;
		rchLine [iLength] = 0;

		// make sure each line is terminated with a \n.
		//  however.. the last line can terminate without it.
		//  note we have reserved room for this
		if (iLine != iLines - 1)
		{
			if (((iLength >= 1) && rchLine [iLength - 1] != '\n') || (0 == iLength))
			{
				lstrcat (rchLine, "\r\n");
				iLength += 2;
			}
		}

		if (fOutputString)
			strOut += rchLine;

		if (fCRC)
		{
			int crc = CRC32 ((PBYTE)rchLine, iLength);
			pDiffs->vKeys.insert (pDiffs->vKeys.end(), crc);
		}
	}

	rpVoid = (PCHAR) pDiffs;
}

// -------------------------------------------------------------------------
// Return 1 if we should display warning about "too much quoted text"
//
int  TComposeFormView::EstimateDifferences (PCHAR pMsg1, PCHAR pMsg2)
{
	TMyDiffData * pDiff1 = (TMyDiffData *) pMsg1;
	TMyDiffData * pDiff2 = (TMyDiffData *) pMsg2;

	// STL sort
	sort( pDiff1->vKeys.begin(), pDiff1->vKeys.end() );
	sort( pDiff2->vKeys.begin(), pDiff2->vKeys.end() );

	int totKeys2 = pDiff2->vKeys.size();
	int totsize = pDiff1->vKeys.size() + totKeys2 + 10;

	vector<int> result(totsize);
	vector<int>::iterator itOut
		= set_symmetric_difference (pDiff1->vKeys.begin(), pDiff1->vKeys.end(),
		pDiff2->vKeys.begin(), pDiff2->vKeys.end(),
		result.begin());
	totsize = 0;
	for ( vector<int>::iterator x = result.begin(); x != itOut; x++)
		totsize++;

	// at least 5%
	if (20 * totsize >= totKeys2)
		return 0;

	return 1;
}

// -------------------------------------------------------------------------
void Compose_FreeDiffData (PCHAR & pData)
{
	if (0 == pData)
		return;

	delete ( (TMyDiffData*) pData);

	pData = 0;
}

// -------------------------------------------------------------------------
void TComposeFormView::OnInitialUpdate()
{
	BOOL fOldWrapVal = SetColorWrap(FALSE);

	TSelectRichEditView::OnInitialUpdate();

	// please send us EN_CHANGE messages via WM_COMMAND
	m_pRich->SetEventMask( ENM_CHANGE | m_pRich->GetEventMask() );

	m_pRich->SetOptions (ECOOP_OR,
		ECO_WANTRETURN /* | ECO_AUTOVSCROLL | ECO_AUTOHSCROLL */
		);

	LOGFONT lf;
	int PointSize;
	gpGlobalOptions->GetPostingFont( &lf, &PointSize);

	// new code to set background color   4-7-98
	m_pRich->SendMessage (EM_SETBKGNDCOLOR, TRUE /* use system color */, NULL);

	// install fixed font
	SetWordWrap ( &lf, PointSize );

	SetWrapWidth ( TRUE );

	SetColorWrap (fOldWrapVal);
}

// -------------------------------------------------------------------------
//  Find the old signature and replace with the new one.
//   if not found, then plop it at the end.
void TComposeFormView::SwapSignatures(const CString & strPrevIn, const CString & strInNew)
{
	CString strNew = strInNew.IsEmpty()
		? strInNew : rcSignatureToken + strInNew;
	CString strPrev(strPrevIn);

	FINDTEXTEX sFind;
	ZeroMemory(&sFind, sizeof(sFind));

	// search everything
	sFind.chrg.cpMin = 0;
	sFind.chrg.cpMax = -1;
	sFind.lpstrText = LPSTR(LPCTSTR(strPrev));

	LRESULT lRes = m_pRich->SendMessage(EM_FINDTEXTEX, 0/*FT_MATCHCASE*/, LPARAM(&sFind));
	if (-1 == lRes)
	{
		int end = m_pRich->GetWindowTextLength();

		if (end >= 2)
		{
			//figure out if we should start a new line.
			TCHAR  buf[3];
			TEXTRANGE tr;
			ZeroMemory(buf, sizeof(buf));
			tr.chrg.cpMin = end-2;
			tr.chrg.cpMax = end;
			tr.lpstrText = buf;

			m_pRich->SendMessage(EM_GETTEXTRANGE, 0, LPARAM(&tr));

			// if we are not on a new line, start a new line
			if (!('\r' == buf[0] && '\n' == buf[1]))
				strNew = "\r\n" + strNew;
		}

		// move insertpoint to very end
		sFind.chrgText.cpMin = sFind.chrgText.cpMax = end;
	}

	// found it. now do a setsel - replacesel
	m_pRich->SetRedraw (FALSE);

	if (sFind.chrgText.cpMin > 4)
	{
		// hunt for signature token
		TCHAR rcHuntForToken[6];
		TEXTRANGE tr;
		ZeroMemory(rcHuntForToken, sizeof rcHuntForToken);
		tr.chrg.cpMin = sFind.chrgText.cpMin - 5;
		tr.chrg.cpMax = sFind.chrgText.cpMin;
		tr.lpstrText  = rcHuntForToken;
		m_pRich->SendMessage(EM_GETTEXTRANGE, 0, LPARAM(&tr));

		if (0 == _tcscmp(rcHuntForToken, rcSignatureToken))
		{
			// include the sig separator token in the wipeout
			sFind.chrgText.cpMin -= 5;
		}
	}

	m_pRich->SendMessage(EM_EXSETSEL, 0, LPARAM(&sFind.chrgText));
	m_pRich->SetRedraw (TRUE);

	m_pRich->ReplaceSel(strNew);

	// since we had turned redraw off, we manually repaint.  Sometimes,
	//   (with a long message), the window has to scroll before the signature
	//   is appended.  So we redraw to clear out any auto-scroll side effects
	m_pRich->Invalidate();
}

// -------------------------------------------------------------------------
void TComposeFormView::SetSignature(const CString & strSig0)
{
	CString strSig(strSig0);

	if (FALSE == strSig.IsEmpty())
	{
		// restore this position.
		CHARRANGE rng;
		m_pRich->GetSel( rng);

		// When doing followups Tony likes to have a blank line
		m_pRich->ReplaceSel( "\r\n" );

		// insert special signature Token (see son-of-Rfc1036, $4.3.2)
		m_pRich->ReplaceSel( rcSignatureToken );

		// insert text at current selection-point
		m_pRich->ReplaceSel(strSig);

		m_pRich->SetSel( rng );
	}
}

// -------------------------------------------------------------------------
void TComposeFormView::SetWordWrap (LOGFONT* pLF, int Tenths_Of_A_Point)
{
	int nWordWrapChars = gpGlobalOptions->GetWordWrap();

	HFONT hFixedFont;
	LOGFONT fixedLF;  CopyMemory(&fixedLF, pLF, sizeof fixedLF);
	hFixedFont = CreateFontIndirect ( &fixedLF );

	// Find out the length in Twips
	m_hDCRtf = CreateCompatibleDC( NULL );

	CString strTest('x', nWordWrapChars);
	RECT rct; ZeroMemory(&rct, sizeof rct);

	// Calculate the rect w/o drawing
	HGDIOBJ hPrevFont = SelectObject(m_hDCRtf, hFixedFont);

	int ret = DrawText (m_hDCRtf, strTest, strTest.GetLength(), &rct,
		DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
	SelectObject(m_hDCRtf, hPrevFont);
	DeleteObject ( hFixedFont );

	// remember this for H-scroll calculations
	m_iCharCX = (rct.right-rct.left) / nWordWrapChars;

	// Convert from MM_TEXT to MM_TWIPS
	SetMapMode(m_hDCRtf, MM_TWIPS);

	POINT pt;
	pt.x =rct.right;
	pt.y =rct.bottom;
	DPtoLP (m_hDCRtf, &pt, 1);

	// we are accounting for Logical Screen Inches vs. Real Inches
	int iHORZSIZE = GetDeviceCaps( m_hDCRtf, HORZSIZE ); // Width in mm
	int iHORZRES  = GetDeviceCaps( m_hDCRtf, HORZRES );  // Width in pixels
	int iLOGPX    = GetDeviceCaps( m_hDCRtf, LOGPIXELSX ); // pixels per logical inch

	// iHORZRES * 25.4 / iHORZSIZE     -> pixels per real inch
	ret = (int) ((pt.x * iHORZRES / iLOGPX) * 25.4 / iHORZSIZE);

	m_i80charTwips = ret;
	DeleteObject(m_hDCRtf);

	m_hDCRtf = CreateCompatibleDC( NULL );

	// should word wrap
	// default is unlimited
	// special is based on window position
	ret = FAR_OUT_TWIPS;
	VERIFY (m_pRich->SendMessage(EM_SETTARGETDEVICE, (WPARAM) m_hDCRtf,
		(LPARAM) ret ));

	m_nWordWrap = WrapToTargetDevice;

	// set default font
	DWORD fx = 0;
	DWORD mask =  CFM_SIZE | CFM_FACE;

	// pay attention to these settings... even if the are NOT on
	mask |= CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;

	if (FW_BOLD == pLF->lfWeight) {
		fx |= CFE_BOLD;         //mask |= CFM_BOLD;
	}
	if (pLF->lfItalic) {
		fx |= CFE_ITALIC;       //mask |= CFM_ITALIC;
	}
	if (pLF->lfUnderline) {
		fx |= CFE_UNDERLINE;    //mask |= CFM_UNDERLINE;
	}
	if (pLF->lfStrikeOut) {
		fx |= CFE_STRIKEOUT;    //mask |= CFM_STRIKEOUT;
	}

	mask |= CFM_COLOR;         // i want to use the System color  4/8/98
	fx  |=  CFE_AUTOCOLOR;

	// CFM_OFFSET controls super/sub script
	CHARFORMAT2& cf = m_defCharFormat;

	ZeroMemory (&cf, sizeof cf);
	cf.cbSize          = sizeof(cf);
	cf.dwMask          = mask;
	cf.dwEffects       = fx;
	// use device units - see description of the LOGFONT height field
	cf.yHeight         = Tenths_Of_A_Point * (2);
	cf.yOffset         = 0;
	cf.crTextColor     = 0;
	cf.bCharSet        = pLF->lfCharSet;
	cf.bPitchAndFamily = pLF->lfPitchAndFamily;
	cf.szFaceName[LF_FACESIZE - 1] = '\0';
	strncpy (cf.szFaceName, pLF->lfFaceName, LF_FACESIZE-1);

	m_pRich->SetDefaultCharFormat( cf );
	SetCharFormat( cf );

	m_fFontReady = TRUE;
}

// -------------------------------------------------------------------------
// used to use RTF_QuotedTextIn, but the unbounded margins would not
//   take effect.  So identify each paragraph separately, and Unwrap/Color it
void TComposeFormView::quote_the_original(
	const CString& inOrig,
	const CString& prefix,
	int   wrap)
{
	CHARRANGE crBegin, crEnd;
	int iMax = 1000000;
	int iLines = 0;
	CString orig(inOrig);

	convert_crlf (orig);    // convert CRLF to LF

	if (gpGlobalOptions->IsLimitingQuoteLines())
		iMax = gpGlobalOptions->GetMaxQuotedLines();

	int iLen = orig.GetLength();
	LPTSTR pBuf, pStart;
	LPTSTR pFwd;

	m_pRich->HideSelection(TRUE, FALSE);
	pStart = pBuf = orig.GetBuffer(iLen);
	while (iLines < iMax)
	{
		pFwd = _tcsstr(pBuf, "\n");
		if (pFwd)
		{
			// view pBuf

			TCHAR c = *(pFwd + 1);
			// cap off one paragraph with a temporary NULL
			*(pFwd + 1) = 0;

			m_pRich->GetSel(crBegin);

			// insert prefix string and the paragraph
			if (prefix.GetLength() > 0)
				m_pRich->ReplaceSel (prefix);
			m_pRich->ReplaceSel (pBuf);
			m_pRich->GetSel(crEnd);

			crBegin.cpMax = crEnd.cpMin;
			PerformColorWrapPara (crBegin, 0);  // definitely no wrap

			++iLines;

			// uninstall NULL
			*(pFwd + 1) = c;
			pBuf = pFwd + 1;
		}
		else
		{
			m_pRich->GetSel(crBegin);
			if (prefix.GetLength() > 0)
				m_pRich->ReplaceSel (prefix);
			m_pRich->ReplaceSel (pBuf);
			m_pRich->ReplaceSel ("\n");       // looks better
			m_pRich->GetSel(crEnd);

			crBegin.cpMax = crEnd.cpMin;
			PerformColorWrapPara (crBegin, 0);
			break;
		}
	}

	m_pRich->HideSelection(FALSE, FALSE);

	orig.ReleaseBuffer(iLen);
}

// -------------------------------------------------------------------------
void TComposeFormView::SubstituteMacros (
	const CString& templat,
	CString &result,
	const CString& msgid,
	const CString& date,
	const CString& addr,
	const CString& name,
	long lNewsGroupID)
{
	result = "";
	LPCTSTR p = templat;
	while (*p)
	{
		if ('%' == *p)
		{
			p++;
			switch (*p)
			{
			case '%':
				result += '%';
				break;
			case 'i':
				result += msgid;
				break;
			case 'd':
				result += date;
				break;
			case 'a':
				result += addr;
				break;
			case 'n':
				result += name;
				break;
			case 'g':
				{
					TServerCountedPtr cpNewsServer;
					CString strGroup;
					BOOL fUseLock;
					TNewsGroup* pNG;
					TNewsGroupUseLock useLock(cpNewsServer, lNewsGroupID,
						&fUseLock, pNG);
					if (fUseLock)
						strGroup = pNG->GetName ();

					result += strGroup;
				}
				break;
			case 'b':
				result += "\n";
				break;
			default:
				result += '%';
				if (*p)
					result += *p;            // copy unrecognized
				else
					return;
				break;
			}
			++p;        // skip recognized control char after %
		}
		else
			result += *p++;
	}
}

// -------------------------------------------------------------------------
// Fill out the template string, and insert it into the BODY
void TComposeFormView::GenerateSaysLine(
										const CString& templat,
										const CString& msgid,
										const CString& date,
										const CString& addr,
										const CString& name,
										long lNewsGroupID)
{
	CString result;
	CString strName = name;
	// RLW - if name is blank, make same as addr
	if (name.IsEmpty())
		strName = addr;
	SubstituteMacros (templat, result, msgid, date, addr, strName, lNewsGroupID);

	if (0 < result.GetLength())
	{
		result += "\r\n"; // was commented out? - fixed the missing newline after this line
		m_pRich->ReplaceSel ( result );
	}
}

// -------------------------------------------------------------------------
void TComposeFormView::InsertTextFile(const CString& strFileName)
{
	CFile file (strFileName, CFile::typeBinary | CFile::modeRead);

	char rchLine[256];
	UINT iRead;
	do
	{
		iRead = file.Read (rchLine, sizeof(rchLine) - 1);
		if (iRead > 0)
		{
			rchLine[iRead] = 0;

			convert_crlf (rchLine);

			m_pRich->ReplaceSel ( rchLine );
		}
	} while (iRead > 0);
}

// -------------------------------------------------------------------------
void TComposeFormView::UserInsertTextFile()
{
	CString fileName;
	try
	{
		if (TFileUtil::GetInputFilename ( &fileName, this,
			IDS_INSERT_TXTFILE,      // stringid of Caption
			NULL,                    // initial dir
			0,                       // more flags
			IDS_FILTER_TXT ))
			InsertTextFile ( fileName );
	}
	catch(CFileException *pFE)
	{
		TCHAR rcBuf[256];
		CString front;  front.LoadString (IDS_ERR_FILEOPEN);
		if (pFE->GetErrorMessage(rcBuf, sizeof rcBuf))
		{
			front += " ";
			front += rcBuf;
		}
		pFE->Delete();

		NewsMessageBox (this, front, MB_OK | MB_ICONEXCLAMATION);
	}
}

// -------------------------------------------------------------------------
void TComposeFormView::OnInsertTextFile()
{
	UserInsertTextFile();
}

// -------------------------------------------------------------------------
// Could be slow for large articles
void TComposeFormView::OnArticleRot13()
{
	CString text;
	PCHAR   pDummy;
	GetLineBrokenText (TRUE, text, FALSE /*fCRC*/, pDummy);

	// in rtfspt.cpp
	RTF_TextRotate13 ( text );

	// replace all contents
	RTF_TextIn (m_pRich, (LPCTSTR) text, SF_TEXT);
}

// -------------------------------------------------------------------------
// Return FALSE if any line is greater than nChar
BOOL TComposeFormView::CheckLineLength(int nChar)
{
	int iTotLines = m_pRich->SendMessage (EM_GETLINECOUNT);
	BOOL fRet = TRUE;

	// hourglass cursor
	CWaitCursor wait;

	for (int i = 0; i < iTotLines; ++i)
	{
		int iLen;
		int iCharPos = m_pRich->SendMessage (EM_LINEINDEX, i);
		iLen = m_pRich->SendMessage(EM_LINELENGTH, iCharPos, 0);

		// an 80 character line + CRLF is ok, since the CRLF is not visible
		if (iLen > nChar)
		{
			// definitely too long
			fRet = FALSE;
			break;
		}
	}
	return fRet;
}

// -------------------------------------------------------------------------
// see if characters typed indicate quoted text, which should not be word
//   wrapped
//
//  pasting - handled elsewhere
//  typing  - see if start of paragraph
//  EnterKey - see if new paragraph starts with QuotePrefix (QPF)
void TComposeFormView::OnRichEditUpdate(void)
{
	if (!m_fColorWrap)
		return;

	CHARRANGE sel;

	m_pRich->GetSel ( sel );
	if (sel.cpMin != sel.cpMax)
		return;

	CHARRANGE para;

	int iBack = 0;
	int iFwd  = 0;
	BOOL fEOF = FALSE;
	iBack = backup_from (sel.cpMin);
	iFwd  = forward_from (sel.cpMin, fEOF);  // Tag #1 

	para.cpMin = iBack;
	para.cpMax = iFwd;

	if (fEOF && (para.cpMin == para.cpMax))
	{
		// blank line
	}
	else if (!fEOF && (para.cpMin == para.cpMax - 2))
	{
		// para is just a CRLF
	}
	else
		PerformColorWrapPara ( para, -1, &sel );
}

// -------------------------------------------------------------------------
//  Check this paragraph for the indent string.  Wrap it. and Add the
//   correct color
//
//  iWrap:  1 for wrap it, 0 don't wrap it,  -1 check prefix for ourselves
int TComposeFormView::PerformColorWrapPara(CHARRANGE& para,
										   int iWrapMode /* == -1 */, CHARRANGE* pOrig /* = 0 */)
{
	// turn off my own automatic processing, while I manipulate my own text
	BOOL fOldWrapVal = SetColorWrap ( FALSE );

	CHARRANGE orig;
	if (0 == pOrig)
		m_pRich->GetSel( orig );
	else
		orig = *pOrig;

	BOOL fWrap = (iWrapMode > -1) ? iWrapMode : HasQuotePrefix( para );

	PARAFORMAT pf;
	pf.cbSize = sizeof pf;
	pf.dwMask = PFM_RIGHTINDENT;

	// ping the right margin. (Skip work if wrap-setting is correct)
	m_pRich->GetParaFormat( pf );

	//  if wrap and indent is set to default
	if (fWrap && pf.dxRightIndent == 0)
		TurnWordWrap(fWrap);
	// Unwrap and indent is not set to default
	else if (!fWrap && pf.dxRightIndent != 0)
	{
#if defined(_DEBUG)
		afxDump << "Wrapoff (" << para.cpMin << "," << para.cpMax << ")\n";
#endif
		TurnWordWrap(fWrap);
	}

	// secretly hide the selection while we tweak this
	m_pRich->HideSelection( TRUE, FALSE );

	correct_color (/*&para*/ pOrig);

	// restore original selection, unhide selection
	m_pRich->HideSelection( FALSE, FALSE );

	// turn my own automatic processing back on
	SetColorWrap ( fOldWrapVal );

	m_pRich->EmptyUndoBuffer();

	return 0;
}

// -------------------------------------------------------------------------
BOOL TComposeFormView::HasQuotePrefix(CHARRANGE& para)
{
	BOOL fWrap = TRUE;    // 1 char lines means wrap

	const CString& strIndent = gpGlobalOptions->GetIndentString();

	TCHAR rcBuf[60]; rcBuf[0] = '\0';
	if (para.cpMax - para.cpMin >= strIndent.GetLength())
	{
		TEXTRANGE tr;
		int iRetLen;

		// Fetch the start of the paragraph
		tr.chrg.cpMin = para.cpMin;
		tr.chrg.cpMax = para.cpMin + strIndent.GetLength();
		tr.lpstrText = rcBuf;
		iRetLen = m_pRich->SendMessage(EM_GETTEXTRANGE, 0, (LPARAM) &tr);
		rcBuf[iRetLen] = '\0';

		// does it match the prefix string?
		fWrap = (0 != strIndent.Compare(rcBuf));
	}
	return fWrap;
}

// -------------------------------------------------------------------------
int TComposeFormView::TurnWordWrap(BOOL fOn)
{
	BOOL fOldState = SetColorWrap( FALSE );

	PARAFORMAT pf;
	ZeroMemory(&pf, sizeof pf);
	pf.cbSize = sizeof pf;

	if (!fOn)
	{
		// Turn off word wrapping for this paragraph

		//             "Right Edge of Window"
		//                    ||
		// |--- 1440          ||   negative numbers

		pf.dwMask = PFM_RIGHTINDENT;
		pf.dxRightIndent = 0;   // use width defined by target device
		// which is way out there.

	}
	else
	{
		// Turn on word wrapping for this paragraph
		pf.dwMask = PFM_RIGHTINDENT;
		int iPW = FAR_OUT_TWIPS;
		if (iPW >= m_i80charTwips)
			pf.dxRightIndent = iPW - m_i80charTwips;
		else
			pf.dxRightIndent = iPW - m_i80charTwips;
	}
	m_pRich->SendMessage (EM_SETPARAFORMAT, 0, (LPARAM) &pf);

	if (fOn)
		m_pRich->PostMessage(WM_HSCROLL, SB_TOP);

	SetColorWrap( fOldState );

	return 0;
}

// -------------------------------------------------------------------------
//
void TComposeFormView::OnEditPaste()
{
	m_fRestoreDraw = FALSE;
	m_pRich->HideSelection(TRUE, FALSE);

	CWaitCursor wait;

	if (::OpenClipboard (m_hWnd))
	{
		HANDLE hClipData = 0;
		LPTSTR pClipData = 0;
		try
		{
			hClipData = ::GetClipboardData (CF_TEXT);

			if (hClipData)
			{
				LPTSTR pClipData = static_cast<LPTSTR>(GlobalLock (hClipData));
				if (pClipData)
				{
					m_iClipDataLen = lstrlen(pClipData);
					auto_ptr<TCHAR> MyCopy(new TCHAR[m_iClipDataLen + 1]);
					lstrcpy (MyCopy.get(), pClipData);
					convert_crlf (MyCopy.get());

					// if we have lots of data, turn off redraw and do a big
					// Invaliate at the end
					if (m_iClipDataLen > 256)
					{
						m_pRich->SetRedraw(FALSE);
						m_fRestoreDraw = TRUE;
					}

					BOOL fPrevVal = SetColorWrap(FALSE);
					// with plain CF_TEXT we don't inherit Source Font/Color
					paste_incremental (MyCopy.get());

					SetColorWrap (fPrevVal);
					correct_color(NULL);
				}
				GlobalUnlock (hClipData);
				pClipData = 0;
			}
			m_iClipDataLen = 0;
			::CloseClipboard ();

			if (m_fRestoreDraw)
			{
				m_pRich->SetRedraw(TRUE);
				m_pRich->Invalidate();
				m_fRestoreDraw = FALSE;
			}
		}
		catch(...)
		{
			if (pClipData && hClipData)
				GlobalUnlock (hClipData);
			m_iClipDataLen = 0;
			::CloseClipboard ();

			if (m_fRestoreDraw)
			{
				m_pRich->SetRedraw(TRUE);
				m_fRestoreDraw = FALSE;
			}

			throw;
		}
	}
	m_pRich->HideSelection(FALSE, FALSE);
}

// -------------------------------------------------------------------------
LPTSTR get_eop (LPTSTR pData)
{
	if (!pData) return NULL;
	
	//LPTSTR pFwd = pData;

	// hunt for end of string, or end of paragraph
	// Optimisation of loop below
	LPTSTR pEOP = _tcschr(pData, '\n');
	if (pEOP == NULL)
		pEOP = pData + _tcslen(pData);
	else
		pEOP++;

	//while (*pFwd)
	//{
	//	if (*pFwd == '\n')
	//	{
	//		pFwd ++;
	//		break;
	//	}
	//	pFwd++;
	//}
	return pEOP;
}

// -------------------------------------------------------------------------
// Analyze the data we are pasting  $$
void TComposeFormView::paste_incremental (LPTSTR pData)
{
	if (*pData == 0)
		return;

	LPTSTR pStart = pData;
	int    iDataLen = lstrlen(pData);

	CHARRANGE chrg;
	m_pRich->GetSel ( chrg );

	int iBackup = backup_from (chrg.cpMin);

	int iSel = chrg.cpMin;
	if (iBackup == chrg.cpMin)
	{
		paste_rest ( pData, iSel );
	}
	else
	{
		LPTSTR pFwd = get_eop(pData);
		TCHAR cTmp = *pFwd;
		*pFwd = 0;

		int iChunkLen = lstrlen(pData);

		m_pRich->ReplaceSel (pData, FALSE);
		CHARRANGE para;
		para.cpMin = iBackup; para.cpMax = iSel + iChunkLen;

		PerformColorWrapPara ( para );

		*pFwd = cTmp;
		pData = pFwd;
		iSel += iChunkLen;
		paste_rest ( pData, iSel );
	}

	// We must change the attributes of the following paragraph
	if (iSel + 1 < m_pRich->GetTextLength())
	{
		BOOL fEOF = FALSE;
		chrg.cpMin = backup_from (iSel + 1);
		chrg.cpMax = forward_from (iSel + 1, fEOF);  // Tag #2

		PerformColorWrapPara ( chrg );
	}
}

// -------------------------------------------------------------------------
void TComposeFormView::paste_rest (LPTSTR & pData, int & iSel)
{
	const CString& strIndent = gpGlobalOptions->GetIndentString();
	while (*pData)
	{
		// we start our own paragraph
		if (lstrlen (pData) >= strIndent.GetLength() &&
			(0 == strncmp(strIndent, pData, strIndent.GetLength())))
			paste_incre(pData, iSel, FALSE);    // long line
		else
			paste_incre(pData, iSel, TRUE);     // must wrap
	}
}

// -------------------------------------------------------------------------
void TComposeFormView::paste_incre(LPTSTR & pData, int & iSel, BOOL fWrap)
{
	LPTSTR pFwd = get_eop(pData);
	TCHAR cTmp;

	cTmp = *pFwd;

	*pFwd = 0;

	TCHAR rcBuf[2]; rcBuf[0] = *pData; rcBuf[1] = 0;
	m_pRich->ReplaceSel(rcBuf, FALSE);

	// set attributes of this little chunk/stub
	TurnWordWrap (fWrap);

	CHARRANGE para;
	para.cpMin = iSel;
	iSel += lstrlen (pData);
	para.cpMax = iSel;

	// the rest will inherit from the stub
	m_pRich->ReplaceSel (pData + 1, FALSE);

	//correct_color(fWrap, para);

	para.cpMin = para.cpMax;
	m_pRich->SetSel (para);

	*pFwd = cTmp;

	// advance
	pData = pFwd;

	m_pRich->EmptyUndoBuffer();
}

// -------------------------------------------------------------------------
// Wrap this section of text. Probably has multiple paragraphs
//
void TComposeFormView::PerformColorWrapRange (CHARRANGE& chrg)
{
	// wrap it correctly, colorize it

	// get bounds of each paragraph
	PARAVEC vec;
	RTF_CollectParagraphs(m_pRich, chrg, &vec);

	int i, tot = vec.GetSize();

	for (i = 0; i < tot; ++i)
	{
		CHARRANGE * pOnePara = vec[i];

		PerformColorWrapPara ( *pOnePara );
	}

	for (i = 0; i < tot; ++i)
	{
		CHARRANGE * pOnePara = vec[i];
		delete pOnePara;
	}
}

// -------------------------------------------------------------------------
int  TComposeFormView::GetMaxLineLength ()
{
	int i = 0, iMax = 0, iCount = m_pRich->GetLineCount();
	int len = 0;
	// Quickly find the maximum
	for (; i < iCount; ++i)
	{
		len = m_pRich->LineLength ( i );
		iMax = (len > iMax) ? len : iMax;
	}
	return iMax;
}

// -------------------------------------------------------------------------
void TComposeFormView::OnSize(UINT nType, int cx, int cy)
{
	TSelectRichEditView::OnSize(nType, cx, cy);

	setMarginRichEdit ( *m_pRich, cx, cy );
}

// -------------------------------------------------------------------------
BOOL TComposeFormView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	// allow tooltip messages to be filtered
	if (CView::PreTranslateMessage(pMsg))
	{
		//TRACE("CView::PreTranslateMessage ate message\n");
		return TRUE;
	}

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
	{
		//TRACE("No pFrameWnd or its showing help\n");
		return FALSE;
	}

	// since 'IsDialogMessage' will eat frame window accelerators,
	//   we call all frame windows' PreTranslateMessage first
	pFrameWnd = GetParentFrame();   // start with first parent frame
	while (pFrameWnd != NULL)
	{
		// allow owner & frames to translate before IsDialogMessage does
		if (pFrameWnd->PreTranslateMessage(pMsg))
		{
			//TRACE("pFrameWnd ate message\n");
			return TRUE;
		}

		// Try parent frames until there are no parent frames
		pFrameWnd = pFrameWnd->GetParentFrame();
	}

	::TranslateMessage(pMsg);
	::DispatchMessage(pMsg);
	//TRACE("TranslateMessage + DispatchMessage message\n");

	return TRUE; // message handled
}

// -------------------------------------------------------------------------
void TComposeFormView::BeginColorWrap(CHARRANGE& rng)
{
	// remember the start point
	m_pRich->GetSel ( rng );
}

// -------------------------------------------------------------------------
void TComposeFormView::EndColorWrap(CHARRANGE& start)
{
	OSVERSIONINFO  sVerInfo;
	sVerInfo.dwOSVersionInfoSize = sizeof(sVerInfo);

	GetVersionEx ( &sVerInfo );

	bool fColorize = true;

	if ((VER_PLATFORM_WIN32_NT == sVerInfo.dwPlatformId) &&
		(sVerInfo.dwMajorVersion > 4))
	{
		// for NT 2000 (aka NT 5.0) skip the heart of this function
		fColorize = false;
	}

	if (fColorize)
	{
		CHARRANGE end;
		CHARRANGE area;

		// get the end point
		m_pRich->GetSel( end );

		area.cpMin = start.cpMin;
		area.cpMax = end.cpMax;

		PerformColorWrapRange ( area );
	}

	// automatic colorwrap is ON
	SetColorWrap(TRUE);
}

// -------------------------------------------------------------------------
BOOL TComposeFormView::SetColorWrap(BOOL fOn)
{
	BOOL fOldValue = m_fColorWrap;

	m_fColorWrap = fOn;

	return fOldValue;
}

// -------------------------------------------------------------------------
int TComposeFormView::backup_from (int iIdx)
{
	TCHAR rcBuf[128], *tcFound;
	BOOL fDone = FALSE;
	int iPos = iIdx;
	int iChunk, iRetLen;
	TEXTRANGE tr;

	while (!fDone && iPos > 0)
	{
		ZeroMemory(rcBuf, sizeof rcBuf);
		if (iPos > (sizeof(rcBuf) - 1))
			iChunk = sizeof(rcBuf) - 1;
		else
			iChunk = iPos;

		tr.chrg.cpMin = iPos - iChunk;
		tr.chrg.cpMax = iPos;
		tr.lpstrText = rcBuf;

		// Note - this copies in a terminating NULL
		iRetLen = (int)m_pRich->SendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr);

		//for (int i = iRetLen - 1; i >= 0; --i)
		//{
		//	if (rcBuf[i] == '\n')
		//	{
		//		fDone = TRUE;
		//		break;
		//	}
		//	--iPos;
		//}

		// Optimisation of above
		tcFound = _tcsrchr(rcBuf, '\n');
		if (tcFound == NULL)
		{
			iPos -= iRetLen;
		}
		else
		{
			iPos -= (iRetLen - (tcFound - &rcBuf[0]))-1;
			fDone = TRUE;
		}
	}
	ASSERT(iPos >= 0);
	return iPos;
}

// -------------------------------------------------------------------------
int TComposeFormView::forward_from (int iIdx, BOOL& fEOF)
{
	int iTL = m_pRich->GetTextLength();
	TCHAR rcBuf[128], *tcFound;
	BOOL  fDone = FALSE;
	int   iPos = iIdx;
	int iChunk, iRetLen;
	TEXTRANGE tr;

	fEOF = TRUE;

	while (!fDone)
	{
		ZeroMemory(rcBuf, sizeof rcBuf);

		if (iPos + static_cast<int>(sizeof(rcBuf) - 1) > iTL)
		{
			iChunk = iTL - iPos;
			fDone = TRUE;
		}
		else
			iChunk = sizeof rcBuf - 1;

		tr.chrg.cpMin = iPos;
		tr.chrg.cpMax = iPos + iChunk;
		tr.lpstrText = rcBuf;

		// Note - this copies in a terminating NULL
		iRetLen = (int)m_pRich->SendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr);

		// w2k workaround  (it can return ZERO here)
		if (0 == iRetLen)
		{
			return RTF_FindChar (*m_pRich, iIdx, '\n', fEOF);
		}

		//for (int i = 0; i < iRetLen; ++i)
		//{
		//	if (rcBuf[i] == '\n')
		//	{
		//		fDone = TRUE;
		//		++iPos;

		//		// note we terminated via CRLF
		//		fEOF = FALSE;
		//		break;
		//	}
		//	++iPos;
		//}

		// Optimisation of above
		tcFound = _tcschr(rcBuf, '\n');
		if (tcFound == NULL)
		{
			iPos += iRetLen;
		}
		else
		{
			iPos += (tcFound - &rcBuf[0])+1;
			fDone = TRUE;
			fEOF = FALSE;
		}
	}

	return iPos;
}

// -------------------------------------------------------------------------
// Caller must HideSelection to prevent blinking
// Caller must restore selection when done
//
// RLW : Rewritten to simply iterate through all the lines in the edit box
// and check for the quote char at the start of each line.
// The old method stopped working with the port to Visual Studio 2005 and
// I couldn't work out why, so here is the brute force method :-)
//
// RLW : Optimised so if pPara is passed in, it indicates the range we
// should colourize and we restrict ourselves to that range.
// Also optimised as much as I can.
//
void TComposeFormView::correct_color(CHARRANGE *pPara)
{
	CHARRANGE para, origPara;
	if (!m_fRestoreDraw)
		m_pRich->SetRedraw(FALSE);
	int nFirstVisisble = m_pRich->GetFirstVisibleLine(); // Remember what line was at the top of rich edit control

	m_pRich->GetSel( origPara );

	DWORD dwNormalText = gSystem.WindowText();
	COLORREF clrQuotedText = gpGlobalOptions->GetRegFonts()->GetQuotedTextColor();
	int nStartLine, nEndLine, nLines = m_pRich->GetLineCount(), nLineLen;
	char cLine[6];

	if (!pPara)
	{
		nStartLine = 0;
		nEndLine = nLines;
	}
	else
	{
		nStartLine = m_pRich->LineFromChar(pPara->cpMin)-1;
		nEndLine = m_pRich->LineFromChar(pPara->cpMax)+1;
		if (nStartLine < 0)
			nStartLine = 0;
		if (nEndLine > nLines)
			nEndLine = nLines;
	}

	CHARFORMAT cfm;
	// strictly speaking we shouldn't need to Zero it out. But it works!!
	ZeroMemory(&cfm, sizeof(cfm));
	cfm.cbSize = sizeof(cfm);
	cfm.dwMask = CFM_COLOR;

	for (int i = nStartLine; i < nEndLine; i++)
	{
		nLineLen = m_pRich->GetLine(i, cLine, 5);
		if (nLineLen >= 2)
		{
			cLine[2] = 0;
			para.cpMin = m_pRich->LineIndex(i);
			if ((i+1) < nLines)
				para.cpMax = m_pRich->LineIndex(i+1)-1;
			else
				para.cpMax = -1;
			m_pRich->SetSel( para );

			if (cLine[0] == '>' && cLine[1] == ' ')
			{
				cfm.crTextColor = clrQuotedText;
				m_pRich->SetSelectionCharFormat(cfm);
			}
			else
			{
				cfm.crTextColor = dwNormalText;
				m_pRich->SetSelectionCharFormat(cfm);
			}
		}
	}

	// restore original selection, scroll to where it was and unlock window update
	m_pRich->SetSel( origPara );
	m_pRich->LineScroll(-m_pRich->GetLineCount(), 0);
	m_pRich->LineScroll(nFirstVisisble, 0);
	if (!m_fRestoreDraw)
	{
		m_pRich->SetRedraw(TRUE);
		m_pRich->Invalidate(FALSE);
	}
	// Stop undo from doing anything
	m_pRich->EmptyUndoBuffer();
}

//-------------------------------------------------------------------------
int TComposeFormView::PostInit ()
{
	return 0;
}

// ------------------------------------------------------------------------
int TComposeFormView::convert_crlf (LPTSTR pString)
{
	LPTSTR pFwd = pString;
	TCHAR last = 0;
	int len = 0;

	while (*pFwd)
	{
		if (*pFwd == '\n' && last == '\r')
		{
			*(pString - 1) = '\n';
			last = *pFwd;
			pFwd++;
		}
		else
		{
			last = *pFwd;
			pFwd ++;
			*pString++ = last;
			len ++;
		}
	}
	*pString = 0;

	return len;
}

// ------------------------------------------------------------------------
// This is important because CRichEditControl doesn't like it when you do
//    hwndRich.ReplaceSel ("\r\n")
//
void TComposeFormView::convert_crlf (CString & str)
{
	int sz = str.GetLength();
	LPTSTR pBuf = str.GetBuffer(sz);

	int newLength = convert_crlf (pBuf);
	str.ReleaseBuffer (newLength);
}

// ------------------------------------------------------------------------
// the special delimiter is a line with just -- CRLF
//
// Bug:  doesn't handle the case where delimiter is the very first line
void TComposeFormView::strip_signature (const CString& orig, CString& output)
{
	output = orig;

	LPCTSTR pBuf = LPCTSTR(output);

	// see if any occurrence at all.
	LPCTSTR pFound = _tcsstr(pBuf, "\r\n-- \r\n");

	if (!pFound)
	{
		// Try dash dash CRLF (OE special)
		pFound = _tcsstr(pBuf, "\r\n--\r\n");
		if (!pFound)
			return;
	}

	// now, find the last occurrence
	output.MakeReverse ();

	int idx = output.Find ( "\n\r --\n\r" );
	if (idx == -1)
		idx = output.Find ( "\n\r--\n\r" );

	ASSERT(idx > -1);
	output = output.Mid (idx + 5);

	output.MakeReverse ();
}

