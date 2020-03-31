/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artdisp.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:51:10  richard_wood
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
#include "superstl.h"         // aggregate STL headers
#include "artdisp.h"
#include "News.h"
#include "article.h"
#include "arttext.h"
#include "triched.h"
#include "rtfspt.h"
#include "hints.h"
#include "thrdlvw.h"
#include "newsview.h"
#include "tglobopt.h"
#include "custview.h"
#include "urlsppt.h"
#include "rxsearch.h"
#include "rgbkgrnd.h"
#include "rgswit.h"
#include "rgurl.h"
#include "rgfont.h"
#include "rgui.h"
#include "sysclr.h"
#include "gdiutil.h"
#include "strext.h"           // TStringEx

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TSystem gSystem;
extern TGlobalOptions *gpGlobalOptions;
extern RxSearch urlSearch;

static void ChangeDefaultFont(CWnd *pRich, const TArticleItemView& rItem);

static void ApplyURLHighlighting(CWnd *pRich, BOOL* pRedraw);
static void convert_to_charformat(const LOGFONT* pLF,  int iTwips, COLORREF color, CHARFORMAT * pcf);

static void rich_display_article(BOOL fSummary, CWnd *pRich,
								  TPersist822Header* pArt, TPersist822Text*   pText, BOOL fFullHeader,
								  bool fShowQuotedText, bool fShowMsgSource, GravCharset * pCharset);

static int quoted_text_filter(CString& in, CString& out, DWORD cookie);

static void display_xface(CWnd * pRich, TPersist822Text* pText);

// ------------------------------------------------------------------------
// Show summary on LB_SELCHANGE
void RichDisplaySummary(CWnd * pRich, TPersist822Header* pArt)
{
	rich_display_article(TRUE, pRich, pArt, NULL, FALSE, false, false, NULL);
}

///////////////////////////////////////////////////////////////////////
//  FYI: NetScape has - SUBJ, DATE, FROM, ORG, NEWGROUP, REFS: BODY
//
//  4-26-96  amc Changed this to always handle the Invalidate
void RichDisplayArticle (CWnd *pRich,
						 TPersist822Header* pArt,
						 TPersist822Text*   pText,
						 BOOL fFullHeader /* FALSE */,
						 bool fShowQuotedText /* = true */,
						 bool fShowMsgSource /* = false */,
						 GravCharset * pCharset /* = NULL */
						 )
{
	rich_display_article(FALSE, pRich, pArt, pText, fFullHeader, fShowQuotedText, fShowMsgSource, pCharset);
}

// ------------------------------------------------------------------------
// helper function
void rich_display_article(BOOL fSummary,
						   CWnd *pRich,
						   TPersist822Header *pArt,
						   TPersist822Text *pText,
						   BOOL fFullHeader,
						   bool fShowQuotedText,
						   bool fShowMsgSource,
						   GravCharset *pCharset
						   )
{
	pCharset=NULL;
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();

	static COLORREF   lastBackground = RGB(192,192,192);

	COLORREF          currBackground;

	currBackground = pBackgrounds->GetEffectiveArtviewBackground();

	if (currBackground != lastBackground)
	{
		pRich->SendMessage(EM_SETBKGNDCOLOR, FALSE, LPARAM(currBackground));
		lastBackground = currBackground;
	}

	TCustomArticleView& rCustomView = gpGlobalOptions->GetCustomView();
	HWND  hwndRTF = pRich->GetSafeHwnd();

	pRich->SendMessage(WM_SETREDRAW, FALSE, 0L);

	// empty it out
	pRich->SendMessage(WM_SETTEXT, 0, (LPARAM)"");

	CHARRANGE range;

	T_ADHOC_IAF  iaf;

	iaf.pRich = pRich;
	iaf.pArt  = pArt;
	iaf.pText = pText;

	iaf.fFullHeader = fFullHeader;
	iaf.fMsgSource  = fShowMsgSource;

	if (fSummary)
	{
		ARTDISP_INFO sInfo(TRUE);
		SetInitialFont(pRich);

		iaf.pText  = NULL;
		InsertArticleField(iaf, IDS_TMPL_SUBJECT, &range, &sInfo);
		InsertArticleField(iaf, IDS_TMPL_FROM, &range, &sInfo);
	}
	else
	{
		ASSERT(pText);
		// Parse the header, so we know if it is MIME.  (If multipart/alternative,
		//   we can skip displaying the text/html part and just show the text/plain
		pText->ParseFullHeader();

		if (fFullHeader)
		{
			SetInitialFont(pRich);

			ARTDISP_INFO sInfo(FALSE);

			// insert full header (inherit initial font)
			InsertArticleField(iaf, IDS_TMPL_FULLHDR, &range, &sInfo);
			InsertArticleField(iaf, IDS_TMPL_BLANK,   &range, &sInfo);
		}
		int total = rCustomView.GetSize();
		int stringID, i;

		ZeroMemory(&range, sizeof(range));

		// setup two configurations
		ARTDISP_INFO sNormal(TRUE);
		ARTDISP_INFO sNoQuoted(TRUE);
		sNoQuoted.m_pFunc = quoted_text_filter;  // setup ptr to function
		sNoQuoted.m_dwFuncData = 0;

		LONG low, high;
		low = 0;

		for (i = 0; i < total; ++i)
		{
			const TArticleItemView& rItem = rCustomView[i];

			stringID = rItem.GetStringID();

			// we display the full header without any fancy formatting

			if (!fFullHeader || (fFullHeader && IDS_TMPL_BODY == stringID))
			{
				// change the default font
				ChangeDefaultFont(pRich, rItem);

				// insert text, using the font we just set
				// optimize Followup-To & Reply-To
				if (InsertArticleField(iaf, WORD(stringID), &range,
					fShowQuotedText ? &sNormal : &sNoQuoted, pCharset))
				{
					high = range.cpMax;

					// we might want to colorize the 4 lines of quoted text
					if (IDS_TMPL_BODY == stringID)
					{
						CString strQuoteChars = gpGlobalOptions->GetRegFonts()->GetQuoteChars();

						// format low-high
						ColorizeQuotedText (pRich, low, high,
							gpGlobalOptions->GetRegFonts()->GetQuotedTextFont(),
							gpGlobalOptions->GetRegFonts()->GetQuotedTextColor(),
							gpGlobalOptions->GetRegFonts()->m_QuotedTextPtSize * 2,  // send in Twips
							strQuoteChars);
					}

					low = high;
				}
			}
		}
	}
	// reset selection back to 0th char
	range.cpMin = range.cpMax = 0;
	::SendMessage(hwndRTF, EM_EXSETSEL, 0, LPARAM(&range));

	CWaitCursor waiting;

	BOOL fRedraw = TRUE;
	// Try to highlight the URLs
	if (AnyURLHighlighting())
	{
		::SendMessage(hwndRTF, EM_HIDESELECTION, TRUE, FALSE);
		ApplyURLHighlighting(pRich, &fRedraw);
		::SendMessage(hwndRTF, EM_EXSETSEL, 0, LPARAM(&range));
		::SendMessage(hwndRTF, EM_HIDESELECTION, FALSE, FALSE);
	}

	if (fRedraw)
	{
		::SendMessage(hwndRTF, WM_SETREDRAW, TRUE, 0L);
		pRich->Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////
// pRange returns the current position
//
// fOptimize - if TRUE, show Followup-To and Reply-To only if different
//             from Newsgroups & From
static BOOL InsertArticleField (T_ADHOC_IAF       & iaf,
								WORD                wField,
								CHARRANGE         * pRange,
								ARTDISP_INFO      * pInfo,
								GravCharset * pCharset /* = NULL */)
{
	/// NEED SOME MAP shit
	// See if the data is here
	CString field;
	BOOL    fSimpleInsert = TRUE;
	BOOL    ret = TRUE;
	if (IDS_TMPL_BLANK == wField)
	{
		field = "\r\n";
	}
	else if (IDS_TMPL_FULLHDR == wField)
	{
		RTF_TextIn ( iaf.pRich, iaf.pText->GetHeader(), SF_TEXT | SFF_SELECTION );
		return ret;
	}
	else if (IDS_TMPL_BODY == wField)
	{
		ArticleCreateField ( iaf.pArt, iaf.pText, wField, FALSE /*show field name*/, field,
			(iaf.fMsgSource ? kRenderRaw : kRenderPretty),
			false, // fPosting
			pCharset );

		// we should apply the function-ptr (generically) to all Fields
		if (pInfo->m_pFunc)
		{
			CString strIn = field;
			field.Empty();

			// apply magick filtre
			pInfo->m_pFunc ( strIn, field, pInfo->m_dwFuncData );
		}
		RTF_TextIn ( iaf.pRich, field, SF_TEXT | SFF_SELECTION);
		fSimpleInsert = FALSE;
	}
	else
	{
		if (pInfo->m_fOptimize && iaf.pArt->IsArticle())
		{
			if (IDS_TMPL_FOLLOWUP==wField &&
				iaf.pText->CastToArticleText()->FollowupSame())
				return ret;

			if (IDS_TMPL_REPLYTO==wField)
			{
				CString phrase;
				CString address;
				iaf.pArt->ParseFrom (phrase, address);

				if (iaf.pText->CastToArticleText()->ReplyToSame(address))
					return ret;
			}
		}

		ret = ArticleCreateField ( iaf.pArt, iaf.pText, wField, TRUE, field,
			(iaf.fMsgSource ? kRenderRaw : kRenderPretty) );
		if (!ret)
			return ret;
		field += "\r\n";
	}

	if (fSimpleInsert)
		iaf.pRich->SendMessage ( EM_REPLACESEL, FALSE, (LPARAM)(LPCTSTR)field );

	iaf.pRich->SendMessage ( EM_EXGETSEL, 0, LPARAM(pRange) );
	return ret;
} // InsertArticleField

//-------------------------------------------------------------------------
static void apply_font_cf (const TArticleItemView& rItem, CHARFORMAT* pcf )
{
	const LOGFONT* pLF = rItem.GetLogfont();

	convert_to_charformat( pLF, rItem.GetSizeTwips(), // want TWIPS
		rItem.GetColor(), pcf);
}

//-------------------------------------------------------------------------
static void convert_to_charformat(const LOGFONT* pLF, int iTwips, COLORREF color, CHARFORMAT * pcf)
{
	DWORD fx = 0;
	DWORD mask = CFM_SIZE | CFM_COLOR | CFM_FACE
		| CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;

	if (FW_BOLD == pLF->lfWeight) {
		fx |= CFE_BOLD;         mask |= CFM_BOLD;
	}
	if (pLF->lfItalic) {
		fx |= CFE_ITALIC;       mask |= CFM_ITALIC;
	}
	if (pLF->lfUnderline) {
		fx |= CFE_UNDERLINE;    mask |= CFM_UNDERLINE;
	}
	if (pLF->lfStrikeOut) {
		fx |= CFE_STRIKEOUT;    mask |= CFM_STRIKEOUT;
	}

	// CFM_OFFSET controls super/sub script
	pcf->cbSize          = sizeof(*pcf);
	pcf->dwMask          = mask;
	pcf->dwEffects       = fx;
	pcf->yHeight         = iTwips;  // want TWIPS
	pcf->yOffset         = 0;
	pcf->crTextColor     = color;
	pcf->bCharSet        = pLF->lfCharSet;
	pcf->bPitchAndFamily = pLF->lfPitchAndFamily;
	pcf->szFaceName[LF_FACESIZE - 1] = '\0';
	strncpy (pcf->szFaceName, pLF->lfFaceName, LF_FACESIZE-1);
}

///////////////////////////////////////////////////////////////////////////
// This is a function, so I can profile it.
static void ChangeDefaultFont(CWnd *pRich, const TArticleItemView& rItem)
{
	CHARFORMAT cf;
	apply_font_cf ( rItem, &cf );

	pRich->SendMessage (EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cf));
}

///////////////////////////////////////////////////////////////////////////
// This is a function, so I can profile it.
static LRESULT GetOneLine(int iLine, CWnd *pRiched, int bufsize, LPTSTR buf)
{
	// this is how you pass in the buffer size to EM_GETLINE
	*((WORD *) buf) = WORD(bufsize - 1);

	// copy the line into a buffer
	return pRiched->SendMessage(EM_GETLINE, (WPARAM) iLine, (LPARAM) (LPTSTR) buf);
}

void ApplyURLHighlighting(CWnd *pRiched, BOOL* pfRedraw)
{
	int				lines=0;
	TCHAR			lineToParse[2000]={};
	LRESULT			lineStart=0;
	TURLSettings *	pSettings = gpGlobalOptions->GetURLSettings();
	BOOL			fUnderline = pSettings->UnderliningLinks();
	COLORREF		linkColor = pSettings->GetHotlinkColor ();
	LPCTSTR			result;
	LPCTSTR         curr;
	int				length=0;

	// remove selection
	lineStart = pRiched->SendMessage(EM_SETSEL, WPARAM(-1), 0);
	lines = pRiched->SendMessage(EM_GETLINECOUNT, 0, 0);
	int pos = 0;

	DWORD	currTime = GetTickCount();
	DWORD	max      = gpGlobalOptions->GetRegUI()->GetMaxHiliteSeconds() * 1000 + currTime;
	DWORD	currCount=0;
	LRESULT	len;

	for (int i = 0; i < lines; i++)
	{
		if ((currCount = GetTickCount()) > max)
			continue;

		ZeroMemory(lineToParse, sizeof lineToParse);

		// copy the line into a buffer (max line size here is 199)
		GetOneLine(i, pRiched, sizeof (lineToParse), lineToParse);

		lineStart = pRiched->SendMessage(EM_LINEINDEX, WPARAM (i), 0);
		len = pRiched->SendMessage(EM_LINELENGTH, (WPARAM) lineStart, 0);

		if (len > 0)
		{
			// this is needed since Windows2000 copies too much data in via our
			// call to GetOneLine
			if (len < sizeof (lineToParse))
				lineToParse[len] = 0;

			// Try to detect a UUENCODED article, we don't want to search it
			// needlessly.  I'm worried about the cost of this test on
			// each line, but it should short circuit if the first char
			// is not 'M' - it's about the minimal cost I can think of
			// to tell

			if ('M' == lineToParse[0] &&
				(len > 40) &&
				(NULL == _tcschr (lineToParse, ' ')))
				continue;

			curr = lineToParse;
			while ((result = urlSearch.Search(curr, &length)) != NULL)
			{
				CHARRANGE   select;
				CHARFORMAT  currFormat;

				// lineStart is still set from above
				ASSERT(pRiched->SendMessage(EM_LINEINDEX, WPARAM (i), 0) ==  lineStart);

				select.cpMin = lineStart + (result - lineToParse);
				select.cpMax = select.cpMin + length;
				pRiched->SendMessage(EM_EXSETSEL, WPARAM(0), LPARAM(&select));

				currFormat.cbSize = sizeof(currFormat);
				// retrieve the current character formatting
				pRiched->SendMessage(EM_GETCHARFORMAT,
					WPARAM(BOOL(TRUE)),
					LPARAM((CHARFORMAT FAR *) &currFormat));

				currFormat.crTextColor = linkColor;

				if (fUnderline)
				{
					currFormat.dwMask |= CFM_UNDERLINE;
					currFormat.dwEffects |= CFE_UNDERLINE;
				}
				pRiched->SendMessage(EM_SETCHARFORMAT,
					WPARAM(UINT(SCF_SELECTION)),
					LPARAM((CHARFORMAT FAR *) &currFormat));
				curr = result + length;
			}
		}

		// any work we do after his point is probably offscreen, so
		//  allow the window to paint, and let the user begin reading
		//  (while we continue work)
		if (50 == i)
		{
			CHARRANGE range;
			range.cpMin = range.cpMax = 0;
			pRiched->SendMessage(EM_EXSETSEL, 0, LPARAM(&range));
			pRiched->SetRedraw(TRUE);
			pRiched->Invalidate();

			*pfRedraw = FALSE;
		}
		if (i > 50)
		{
			MSG msg;
			while (::PeekMessage(&msg, NULL, WM_ERASEBKGND, WM_ERASEBKGND, PM_REMOVE))
				::DispatchMessage(&msg);
			while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
				::DispatchMessage(&msg);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void SetInitialFont(CWnd *pRich)
{
	// Get background color
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
	COLORREF currBK = pBackgrounds->GetEffectiveArtviewBackground();

	pRich->SendMessage (EM_SETBKGNDCOLOR, FALSE, LPARAM(currBK));

	// MS Sans Serif, 9pt, Regular
	CHARFORMAT cf;
	ZeroMemory (&cf, sizeof(cf));
	cf.cbSize = sizeof(cf);

	// wParam == 0 sets the default format
	cf.dwMask  =  CFM_FACE | CFM_SIZE | CFM_COLOR
		| CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
	cf.yHeight = 20 * NEWS32_BASE_FONT_PTSIZE;
	cf.crTextColor = gSystem.WindowText();
	cf.bPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	lstrcpy(cf.szFaceName, "MS Sans Serif");

	pRich->SendMessage ( EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cf) );
}

//-------------------------------------------------------------------------
//
static void ColorizeQuotedText(CWnd *pRich, LONG low, LONG high,
							   const LOGFONT* pLF,
							   COLORREF clrref,
							   int iTwips,
							   const CString & strQuoteChars)
{
	TRACE("\nColorizeQuotedText >\n");
	CHARRANGE orgSel;
	// select our range
	pRich->SendMessage (EM_EXGETSEL, 0, LPARAM(&orgSel));
	pRich->SendMessage (EM_HIDESELECTION, TRUE, FALSE);

	// apply this font & color
	CHARFORMAT cf; ZeroMemory(&cf, sizeof cf);
	convert_to_charformat(pLF, iTwips, clrref, &cf);

	// RLW - create a set font & colour for signatures ALA Xananews
	CHARFORMAT cfSig; ZeroMemory(&cfSig, sizeof(cfSig));
	convert_to_charformat(gpGlobalOptions->GetRegFonts()->GetSignatureTextFont(),
		gpGlobalOptions->GetRegFonts()->m_SignatureTextPtSize * 2,
		gpGlobalOptions->GetRegFonts()->GetSignatureTextColor(),
		&cfSig);
	//// MS Sans Serif, 9pt, Regular, grey
	//cfSig.cbSize = sizeof(cfSig);
	//// wParam == 0 sets the default format
	//cfSig.dwMask  =  CFM_FACE | CFM_SIZE | CFM_COLOR
	//	| CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
	//cfSig.dwEffects = 0;
	//cfSig.yHeight = 20 * 8;
	//cfSig.crTextColor = RGB(128,128,128);
	//cfSig.bPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	//lstrcpy(cfSig.szFaceName, "MS Sans Serif");

	PARAVEC vec;

	CHARRANGE cr;
	cr.cpMin = low;
	cr.cpMax = high;
	RTF_CollectParagraphs(pRich, cr, &vec);

	TCHAR rcBuf[1024];
	int i, tot = vec.GetSize();
	bool bInSig = false;
	for (i = 0; i < tot; ++i)
	{
		CHARRANGE* pElem = vec[i];
		TRACE("Paragraph (%d - %d) :", pElem->cpMin, pElem->cpMax);

		TEXTRANGE tr;
		ZeroMemory(rcBuf, sizeof(rcBuf));
		tr.chrg.cpMin = pElem->cpMin;
		tr.chrg.cpMax = min(pElem->cpMin+1022, pElem->cpMax)+1;
		tr.lpstrText  = rcBuf;

		pRich->SendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr);

		TRACE (" '%s'\n", rcBuf);

		if (bInSig)
		{
			pRich->SendMessage (EM_EXSETSEL, 0, LPARAM(pElem));
			pRich->SendMessage (EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfSig));
		}
		else
		{
			if (rcBuf[0] && (strQuoteChars.Find(rcBuf[0]) > -1))
			{
				// select our range
				pRich->SendMessage (EM_EXSETSEL, 0, LPARAM(pElem));
				pRich->SendMessage (EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cf));
			}
			if (   (strncmp(rcBuf, "-- \r\n", 5) == 0)
				|| (strncmp(rcBuf, "--\r\n", 4) == 0))
			{
				bInSig = true;
				pRich->SendMessage (EM_EXSETSEL, 0, LPARAM(pElem));
				pRich->SendMessage (EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfSig));
			}
		}
		delete pElem;
	}

	// put selection back
	pRich->SendMessage (EM_HIDESELECTION, FALSE, FALSE);
	pRich->SendMessage (EM_EXSETSEL, 0, LPARAM(&orgSel));
}

// ------------------------------------------------------------------------
// Albert's 1st use of STL
class TFalloffQ
{
protected:
	int m_iLimit;
	int m_iCount;
	bool  m_fStored;            // special handling for zero
	deque<CString> m_dq;

public:
	// ctor
	TFalloffQ (int iLimit) : m_iLimit(iLimit), m_iCount(0), m_fStored(false) { }

	// Store string
	void Store (const CString & str)
	{
		m_fStored = true;
		if (0 == m_iLimit)
			return;
		else if (m_iCount < m_iLimit)
		{
			m_dq.push_back (str);  // store item
		}
		else
		{
			m_dq.pop_front();      // this item falls off
			m_dq.push_back (str);
		}
		m_iCount++;
	}

	// flush contents
	bool Flush (CString & accumulator)
	{
		accumulator.Empty();
		while (!m_dq.empty())
		{
			accumulator += m_dq.front ();
			m_dq.pop_front ();
		}
		m_iCount = 0;

		bool  fRet = m_fStored;
		m_fStored = false;
		return fRet;
	}

	BOOL QuotedTextLimitReached()
	{
		return (m_iCount > m_iLimit);
	}
};

// ------------------------------------------------------------------------
static int  quoted_text_flushq (TFalloffQ & sFQ, CString & strQuoteChars,
								CString & out)
{
	CString strQContents;

	// add an indicator that Muting is On. Prepend some Quoted char
	//    so the block of text gets the right color
	CString strIndicator;
	strIndicator.Format(IDS_MUTE_QUOTED, LPCTSTR(strQuoteChars.Mid(0,1)));

	// get contents into strIndicator

	BOOL bPrependMessage = FALSE;
	if (sFQ.QuotedTextLimitReached())
		bPrependMessage = TRUE;

	if (sFQ.Flush (strQContents))
	{
		if (bPrependMessage)
			out += strIndicator;
		out += strQContents;
	}

	return 0;
}

// ------------------------------------------------------------------------
// Function helps to filter out QuotedText lines
static int  quoted_text_filter (CString& in, CString& out, DWORD cookie)
{
	CString strQuoteChars =
		gpGlobalOptions->GetRegFonts()->GetQuoteChars();

	TStringEx sSmart(& in);
	int pos = 0;
	CString strLine;
	bool fInQuoted = false;
	TFalloffQ  fq(gpGlobalOptions->GetRegUI()->GetShowQuotedMaxLines ());
	CString acc;

	while (sSmart.GetLine (pos, strLine))
	{
		// line does not start with a known Quote character
		if (strQuoteChars.Find (strLine[0]) == -1)
		{
			if (fInQuoted)
			{
				fInQuoted = false;

				quoted_text_flushq (fq, strQuoteChars, out);
			}
			out += strLine;
		}
		else
		{
			fInQuoted = true;
			fq.Store (strLine);     // store this quoted text
		}
	}

	// empty whatever is in there.
	quoted_text_flushq (fq, strQuoteChars, out);

	return 0;
}

