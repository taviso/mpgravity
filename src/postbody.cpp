/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postbody.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.12  2009/06/07 20:11:04  richard_wood
/*  More spelling bug fixes.
/*  Moved global options to Tools menu.
/*
/*  Revision 1.11  2009/05/27 21:15:52  richard_wood
/*  More fixes for spell checker.
/*
/*  Revision 1.10  2009/05/27 19:52:04  richard_wood
/*  Fixed bug in spell checker where I changed everything to lower case before checking.
/*  Added highlighting of the misspelled word.
/*
/*  Revision 1.9  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.8  2009/01/30 08:32:54  richard_wood
/*  Compose window - moved Copy & Select All in context menu up to top level.
/*
/*  Revision 1.7  2008/09/19 14:51:40  richard_wood
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

// postbody.cpp -- body view in posting/mailing windows

#include "stdafx.h"
#include "news.h"
#include "postdoc.h"
#include "postbody.h"
#include "custmsg.h"
#include "tglobopt.h"
#include "posttmpl.h"
#include "genutil.h"             // MsgResource(), fnFetchBody()
#include "richedit.h"            // SF_*
#include "rtfspt.h"              // RTF_TextIn()
#include "tnews3md.h"            // TNews3MDIChildWnd
#include "mlayout.h"             // TMdiLayout
#include "thrdlvw.h"             // TThreadListView
#include "mainfrm.h"             // CMainFrame
#include "artview.h"             // TArticleFormView
#include "richedit.h"            // SF_*
#include "hints.h"               // SENDHINT_80CHARPERLINE
#include "fileutil.h"            // NewsMessageBox
#include "licutil.h"             // TLicenseSystem
#include "hunspell\hunspell.hxx"
#include "arttext.h"             // ArticleCreateField
#include "rgswit.h"              // TRegSwitch object
#include "rgsys.h"				 // TRegSystem object
#include "sysclr.h"              // TSystem object
#include "rgfont.h"
#include "tspelldlg.h"
#include "mywords.h"

extern TGlobalOptions* gpGlobalOptions;
static TCHAR rcSignatureToken[] = "-- \r\n";

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(TPostBody, TComposeFormView)

// -------------------------------------------------------------------------
TPostBody::TPostBody()
{
	m_Body = _T("");

	m_pDiffDataStart = 0;
	m_iCCIntroLen = m_iIntroFinish = 0;
}

// -------------------------------------------------------------------------
TPostBody::~TPostBody()
{
	Compose_FreeDiffData (m_pDiffDataStart);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPostBody, TComposeFormView)
	ON_COMMAND(IDM_POST_QUOTEPARA, OnPostQuotepara)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
TPostDoc * TPostBody::GetDocument(void)
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(TPostDoc)));
	return (TPostDoc*) m_pDocument;
}

static CRichEditCtrl * gpEdit;
static TPostBody *gpPostBody;
static CString gstrAlternateInitialText;
static bool    gfUseAltText = false;

// -------------------------------------------------------------------------
static void fnEmptyAlternateText ()
{
	gfUseAltText = false;

	gstrAlternateInitialText.Empty ();
}

// -------------------------------------------------------------------------
// InsertArticle
static void InsertArticle (TArticleHeader *pHeader, TArticleText *pText,
						   TPostTemplate *pTemplate, BOOL fAddSeparator, const CString &strAuthor,
						   const CString &strAddress)
{
	// assume the phrase is the name
	CString strIntro;
	if (pTemplate -> TestFlag (TPT_USE_REPLY_INTRO))
		strIntro = gpGlobalOptions -> GetReplyIntro ();
	if (pTemplate -> TestFlag (TPT_USE_FOLLOWUP_INTRO))
		strIntro = gpGlobalOptions -> GetFollowupIntro ();

	CString strIndent;
	if (!pTemplate -> TestFlag (TPT_DONT_QUOTE))
		strIndent = gpGlobalOptions -> GetIndentString ();
	BOOL bWrap = gpGlobalOptions -> GetWordWrap ();
	if (pTemplate -> TestFlag (TPT_DONT_WRAP))
		bWrap = FALSE;

	if (pTemplate -> TestFlag (TPT_USE_FORWARD_INTRO)) {
		// if forwarding, insert the article's headers (same headers that
		// would be displayed on the screen when viewing the article)
		CString strHeader;
		SelectedHeaders (pHeader, pText, strHeader);
		RTF_TextIn (gpEdit, strHeader, SF_TEXT | SFF_SELECTION);
	}

	// NOTE: when you insert text, the text inherits the wrap property
	//       from the previous paragraph.  So we apply the formatting for
	//       section 1 at the end of the func.

	LONG lDummy, lIntroStart, lIntroEnd;
	BOOL fPrevColorWrapVal;

	// section 1 - The Intro Line
	{
		// in article <msgid>, you say....

		gpEdit -> GetSel (lIntroStart, lDummy);
		fPrevColorWrapVal = gpPostBody -> SetColorWrap (FALSE);
		/* gpPostBody -> TurnWordWrap (TRUE); */
		gpPostBody -> GenerateSaysLine (strIntro, pHeader->GetMessageID(),
			pHeader->GetDate(), strAddress, strAuthor,
			pTemplate -> m_NewsGroupID);
		gpPostBody -> SetColorWrap (fPrevColorWrapVal);
		gpEdit -> GetSel (lDummy, lIntroEnd);
	}

	gpPostBody -> m_iIntroFinish = gpEdit -> GetTextLength ();

	// Turn off automatic color-wrap processing while we do this
	fPrevColorWrapVal = gpPostBody -> SetColorWrap (FALSE);

	if (!gfUseAltText)
		pText -> ParseFullHeader ();

	const CString & strRawBody = pText -> GetBody ();

	if (pTemplate -> TestFlag (TPT_DONT_QUOTE))
	{
		// for example, when forwarding, don't quote stuff
		if (gfUseAltText)
		{
			gpEdit->SendMessage (EM_REPLACESEL, FALSE, LPARAM(LPCTSTR(gstrAlternateInitialText)));
			fnEmptyAlternateText ();
		}
		else
		{
			// use mostly to strip out multipart/alternative  text/plain + text/html
			CString strPretty;
			BOOL fCreate = ArticleCreateField ( pHeader,
				pText,
				IDS_TMPL_BODY,
				FALSE,         // show field name
				strPretty,
				kRenderPretty );        // render pretty mime

			gpEdit->SendMessage( EM_REPLACESEL, FALSE,
				LPARAM(LPCTSTR(fCreate ? strPretty : strRawBody)) );
		}
	}
	else
	{
		// quote the text
		if (gfUseAltText)
		{
			gpPostBody -> quote_the_original (gstrAlternateInitialText,
				strIndent, bWrap);
			fnEmptyAlternateText ();
		}
		else
		{
			// use mostly to strip out multipart/alternative  text/plain + text/html
			CString strPretty, strStripped;
			BOOL fCreate = ArticleCreateField ( pHeader,
				pText,
				IDS_TMPL_BODY,
				FALSE,         // show field name
				strPretty,
				kRenderPretty );        // render pretty mime

			if (gpGlobalOptions->GetRegSwitch()->GetStripSignature())
				// strip signature after "-- \r\n"
				gpPostBody -> strip_signature ( fCreate ? strPretty : strRawBody, strStripped );
			else
				strStripped = fCreate ? strPretty : strRawBody;

			// RLW - trim off all blank lines poster may have added to start of post
			strStripped.TrimLeft("\r\n");
			// Leave one starting blank line
			strStripped = "\r\n" + strStripped;
			// RLW - trim off all blank lines poster may have added before sig
			strStripped.TrimRight("\r\n");

			gpPostBody -> quote_the_original ( strStripped, strIndent, bWrap );
		}
	}

	if (fAddSeparator)
	{
		// make a string "\r\n" + 60 dashes + "\r\n"
		char rcSep[65]; FillMemory(&rcSep[2], 60, '-');
		rcSep[0] = rcSep[62] = '\r';
		rcSep[1] = rcSep[63] = '\n';
		rcSep[64] = '\0';

		// a separator between messages
		RTF_TextIn (gpEdit, rcSep, SF_TEXT | SFF_SELECTION);
	}

	// apply wrap to section 1
	CHARRANGE crCurrent;
	gpEdit -> GetSel ( crCurrent );                  // save current pos
	gpEdit -> SetSel ( lIntroStart, lIntroEnd );     // select intro
	gpPostBody -> TurnWordWrap ( TRUE );             // format selection
	gpEdit -> SetSel ( crCurrent );                  // restore position

	// automatic color-wrap processing is back on
	gpPostBody -> SetColorWrap (fPrevColorWrapVal);
}

// -------------------------------------------------------------------------
static BOOL InsertViewArticle (TArticleHeader *pHeader, int iSelected,
							   TNewsGroup *pNG, DWORD dwCookie)
{
	if (!iSelected)
		return 0;

	TError         sErrorRet;
	CPoint         ptPartID(0,0);
	CPoint         ptFoundLocal(0,0);
	TArticleText * pText = 0;

	BOOL fMarkAsRead = gpGlobalOptions->GetRegSwitch()->GetMarkReadForward();

	if (fnFetchBody (sErrorRet,
		pNG,
		pHeader,
		pText,
		ptPartID,
		fMarkAsRead /* bMarkAsRead */,
		TRUE /* bTryFromNewsfeed*/,
		FALSE /* force connect */,
		&ptFoundLocal))
	{
		MsgResource (IDS_BODY_FAIL_NOT_CONNECTED);
		return 1;
	}

	if (ptFoundLocal.x  &&  fMarkAsRead)
	{
		pNG->ReadRangeAdd ( pHeader );
	}

	ASSERT (pText);

	CString strAuthor, strAddress;

	pHeader -> ParseFrom (strAuthor, strAddress);

	InsertArticle (pHeader,
		pText,
		(TPostTemplate *) dwCookie,
		TRUE /* add separator */,
		strAuthor,
		strAddress);
	delete pText;
	return 0;
}

// -------------------------------------------------------------------------
void TPostBody::DoDataExchange(CDataExchange* pDX)
{
	CWaitCursor wait;
	TComposeFormView::DoDataExchange(pDX);

	TPostDoc *pDoc = GetDocument ();
	if (pDX->m_bSaveAndValidate)
	{
		CString str;
		PCHAR   pDiff2 = 0;
		bool    fWarning = true;

		if (TestFlag (TPT_COPY_MAIL_HEADER) || TestFlag (TPT_COPY_ARTICLE_HEADER))
		{
			// re-editing outbox message - they probably got checked
			//   once already. let them go by
			fWarning = false;
		}

		TAutoFreeDiff  sAuto (pDiff2);

		// EVEN for Save-Draft msgs  add in the hard CRLFs
		GetLineBrokenText (TRUE, str, !pDoc->m_bMailing, pDiff2);

		// double check follow-ups for REAL content.
		// in other words, skip this check if this is Mail or a re-edited post

		if (FALSE == pDoc->m_bMailing)
		{
			// from base class
			if (fWarning && EstimateDifferences (m_pDiffDataStart , pDiff2))
			{
				// "this article seems to consist mostly of text from another article
				//  are you sure you want to send this?"
				if (IDNO == NewsMessageBox(this, IDS_WARN_ALLQUOTEDTEXT,
					MB_YESNO | MB_ICONWARNING))
					pDX->Fail();   // ::UpdateData should return FALSE
			}
		}

		pDoc -> Message_SetBody (pDoc->m_bMailing ? true : false, str);
	}
	else
		DataExchangeIn ( pDX, pDoc );
}

// -------------------------------------------------------------------------
// Note - when inserting text, we generally turn-of the automatic ColorWrap
// processing, which is hooked into EN_CHANGE. This is done by SetColorWrap(F)
//
void TPostBody::DataExchangeIn(CDataExchange* pDX, TPostDoc* pDoc)
{
	CHARRANGE range;
	CHARRANGE crEndMsg;
	CString dummy;
	int iChars = 0;
	int iLineCount;

	SetRedraw(FALSE);

	// insert contents of the original article if needed
	CWnd* pEdit = &GetRichEditCtrl();
	CRichEditCtrl & rRich = GetRichEditCtrl();

	ASSERT (pEdit);
	TPostTemplate *pTemplate =
		(TPostTemplate *) pDoc -> GetDocTemplate ();

	// make a copy of the flags
	m_iTFlags = pTemplate->m_iFlags;

	if (pTemplate -> TestFlag (TPT_USE_FORWARD_INTRO)) {
		CString str, strIntro = "\r\n\r\n";
		str.LoadString (IDS_FWD_THIS_MESSAGE);
		strIntro += str + " \"" + pTemplate -> m_strExtraTextFile +
			"\".\r\n\r\n----------- ";
		str.LoadString (IDS_FWD_BEGIN);
		strIntro += str + " -----------\r\n\r\n";
		RTF_TextIn (pEdit, strIntro, SF_TEXT | SFF_SELECTION);
	}

	if (pTemplate -> TestFlag (TPT_INSERT_SELECTED_ARTICLES)) {
		gpEdit = &GetRichEditCtrl ();
		gpPostBody = this;

		CHARRANGE range;

		BeginColorWrap ( range );

		CMDIFrameWnd *pMainWnd = (CMDIFrameWnd *) AfxGetMainWnd ();
		TNews3MDIChildWnd *pChildWnd =
			(TNews3MDIChildWnd *) pMainWnd -> MDIGetActive ();
		TMdiLayout *pLayout = pChildWnd -> GetLayoutWnd ();
		if (pChildWnd -> GetFocusCode () == 1) // 1 == thread-view
			pLayout -> GetThreadView () ->
			ApplyToArticles (InsertViewArticle, FALSE /* fRepaint */,
			(DWORD) pTemplate);

		EndColorWrap ( range );
		range.cpMin = range.cpMax = 0;
		rRich.SetSel(range);
	}

	if (pTemplate -> TestFlag (TPT_INSERT_ARTICLE) &&
		(
		(gpGlobalOptions -> IncludingOriginalFollowup () && !pDoc -> m_bMailing)
		||
		(gpGlobalOptions -> IncludingOriginalMail () && pDoc -> m_bMailing)
		)) {

			ASSERT (pTemplate -> m_pArtText && pTemplate -> m_pArtHdr);
			gpEdit = &GetRichEditCtrl ();
			gpPostBody = this;

			// if the article-view has a region highlighted, use it instead
			gstrAlternateInitialText.Empty();
			gfUseAltText = false;

			BOOL fMax;
			TNews3MDIChildWnd *pActiveKid = (TNews3MDIChildWnd *)
				((CMainFrame *) AfxGetMainWnd ()) -> MDIGetActive (&fMax);
			if (pActiveKid) {
				TMdiLayout *pLayout = pActiveKid -> GetLayoutWnd ();
				if (pLayout) {
					TArticleFormView *pArtView = pLayout -> GetArtFormView ();
					if (pArtView) {
						pArtView -> SelectedText (gstrAlternateInitialText);

						// ignore selected space char
						CString strTest = gstrAlternateInitialText;
						strTest.TrimLeft();
						strTest.TrimRight();
						if (!strTest.IsEmpty())
						{
							gfUseAltText = true;

							// the CRLF separates the selected text from the
							//  blank line and the .sig
							gstrAlternateInitialText += "\r\n";
						}
					}
				}
			}

			// set m_strOriginalAuthor and m_strOriginalAddress
			pTemplate -> m_pArtHdr -> ParseFrom (m_strOriginalAuthor,
				m_strOriginalAddress);

			BeginColorWrap (range);
			InsertArticle (pTemplate -> m_pArtHdr, pTemplate -> m_pArtText,
				pTemplate, FALSE /* don't add separator */, m_strOriginalAuthor,
				m_strOriginalAddress);
			EndColorWrap (range);

			range.cpMin = range.cpMax = 0;
			rRich.SetSel( range );
	}

	if (pTemplate -> TestFlag (TPT_INSERT_FILE)) {
		try {
			InsertTextFile (pTemplate -> m_strExtraTextFile);
		}
		catch (CFileException * pfe) {
			MsgResource (IDS_ERR_NO_INFO_FILE);
			pfe->Delete ();
		}
	}

	// Re-editing message in the outbox - insert original
	if (pTemplate -> TestFlag (TPT_INSERT_TEXT))
	{
		BOOL fPrevWrap = SetColorWrap (FALSE);
		LONG lDummy, lStart, lEnd;
		rRich.GetSel (lStart, lDummy);
		rRich.ReplaceSel ( pTemplate -> m_strText );
		rRich.GetSel (lDummy, lEnd);

		CHARRANGE chrg;
		chrg.cpMin = lStart; chrg.cpMax = lEnd;
		PerformColorWrapRange ( chrg );
		SetColorWrap (fPrevWrap);
	}

	// put cursor at the end.
	iChars = rRich.GetTextLength();
	iLineCount = rRich.GetLineCount();

	range.cpMin = range.cpMax = iChars - iLineCount + 2;  // for W2K
	rRich.SetSel ( range );

	range.cpMin = range.cpMax = iChars ;
	rRich.SetSel ( range );

	rRich.GetSel ( crEndMsg );   // memorize this

	// Add the signature, unless we are re-editing a message from the Outbox
	if (!pTemplate -> TestFlag (TPT_COPY_MAIL_HEADER) &&
		!pTemplate -> TestFlag (TPT_COPY_ARTICLE_HEADER))
	{
		// RLW - Add two blank lines before the sig (sig has its own single blank line before it)
		char rcSep[3] = {'\r','\n','\0'};
		RTF_TextIn (pEdit, rcSep, SF_TEXT | SFF_SELECTION);
		RTF_TextIn (pEdit, rcSep, SF_TEXT | SFF_SELECTION);
		// And move insert position down one line, past first blank line
		crEndMsg.cpMin ++;
		crEndMsg.cpMax ++;

		// The last thing we do is insert the Sig. msg will route to Frame
		//   -- this does not move the selection
		GetParent () -> SendMessage (WM_COMMAND, WMU_POST_INITIALUPDATE, 0);
	}

	// this only applies to BugReports via e-mail
	if (pTemplate->TestFlag(TPT_INSERT_MACHINEINFO))
	{
		CString machineInfo;            // added 6-15-98

		if (0 == gSystem.GetMachineInfo (machineInfo))
		{
			int iEnd = pEdit -> SendMessage (WM_GETTEXTLENGTH);
			range.cpMin = range.cpMax = iEnd;
			rRich.SetSel( range );
			rRich.ReplaceSel ( machineInfo );
		}
	}

	// restore caret pos
	if (pTemplate ->TestFlag (TPT_FOLLOWUP))
	{
		// put cursor above the signature

		rRich.SetSel ( crEndMsg );  rRich.SetSel ( crEndMsg );
	}
	else
	{
		// put cursor at start
		range.cpMin = range.cpMax = 0;
		rRich.SetSel ( range );
	}

	// get a count of the lines.  We want to measure what the user adds
	// and prevent postings that have no NEW content.

	// get starting snapshot into pointer
	GetLineBrokenText (FALSE, dummy, TRUE, m_pDiffDataStart);

	{
		// please recalculate the Horizontal scrollbar, and make it appear
		CRect rctWnd;
		GetWindowRect( &rctWnd );
		SetWindowPos ( NULL, 0, 0, rctWnd.Width(), rctWnd.Height(),
			SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE );
	}

	SetRedraw(TRUE);
	Invalidate ();

	// from base class
	PostInit ();
}

// -------------------------------------------------------------------------
#ifdef _DEBUG
void TPostBody::AssertValid() const
{
	TComposeFormView::AssertValid();
}

void TPostBody::Dump(CDumpContext& dc) const
{
	TComposeFormView::Dump(dc);
}
#endif //_DEBUG

// -------------------------------------------------------------------------
void TPostBody::OnInitialUpdate()
{
	TComposeFormView::OnInitialUpdate();

	// we are no longer derived from CFormView. Call this ourselves
	//  to reach the DoDataExchange code
	UpdateData(FALSE);
}

// -------------------------------------------------------------------------
void TPostBody::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (SENDHINT_80CHARPERLINE == lHint)
	{
		TPostDoc * pPostDoc = static_cast<TPostDoc*>(pHint);
		BOOL fValid = check_format();

		if (fValid)
			pPostDoc->m_fBodyFormatValid = TRUE;
		else
		{
			// "Some lines are longer than 80 chars. Do you
			//  want to continue sending?"
			if (IDYES == NewsMessageBox(this, IDS_WARN_LONGBODYLINE,
				MB_YESNO | MB_ICONQUESTION))
			{
				// stupid user wants to go ahead anyway
				pPostDoc->m_fBodyFormatValid = TRUE;
			}
			else
				pPostDoc->m_fBodyFormatValid = FALSE;
		}

		return;
	}

	if (SENDHINT_SPELLCHECK == lHint)
	{
		TPostDoc * pDoc = static_cast<TPostDoc*>(pHint);
		CheckSpelling (pDoc -> m_fShowSpellComplete);
		pDoc -> m_fBodyFormatValid = TRUE;
		return;
	}

	if (SENDHINT_SELECTIONWRAP == lHint)
	{
		TPostDoc * pDoc = static_cast<TPostDoc*>(pHint);
		WrapSelection();
		return;
	}

	TComposeFormView::OnUpdate(pSender, lHint, pHint);
}

// -------------------------------------------------------------------------
void TPostBody::CheckSpelling (BOOL bShowComplete)
{
	BOOL bError = FALSE;

	// loop through lines, ignoring quoted text and intro
	CString strQuote = gpGlobalOptions -> GetIndentString ();
	CRichEditCtrl &ctrl = GetRichEditCtrl ();
	int iLines = ctrl.GetLineCount ();
	int iFirstLine = ctrl.LineFromChar (m_iIntroFinish + m_iCCIntroLen);
	bool bSigSepReached = false;

	for (int i = iFirstLine; i < iLines; i++)
	{
		bError = CheckLineSpelling (i, strQuote, strQuote.GetLength (), bSigSepReached);
		if (bError || bSigSepReached)
			break;
	}

	if (bShowComplete && !bError)
		MsgResource (IDS_SPELL_COMPLETE, this);
}

// -------------------------------------------------------------------------
// CheckLineSpelling -- returns 0 for success, non-0 for failure
int TPostBody::CheckLineSpelling (int iLine, LPCTSTR pchQuote, int iQuoteLen, bool &bSigSepReached)
{
	CRichEditCtrl &ctrl = GetRichEditCtrl ();
	int iCharIndex = ctrl.LineIndex (iLine);   // get character index
	int iLineLen = ctrl.LineLength (iCharIndex);   // yep, it's weird, it takes a character index
	if (iLineLen <= 0)
		return 0;

	// RLW - GetLine wants the output string 3 more than the line length
	LPTSTR pszLine = new TCHAR[iLineLen+3];
	ctrl.GetLine (iLine, pszLine, iLineLen+3);
	CString strLine(pszLine, iLineLen);
	delete [] pszLine;

	// check for quoted line
	if (strLine.Left (iQuoteLen) == pchQuote)
		return 0;

	// check for sig sep
	// Length is 3 chars for sig sep line
	CString strSigSep(rcSignatureToken);
	strSigSep = strSigSep.Left(3);
	if ((strLine.GetLength() == 3) && (strLine == strSigSep))
	{
		bSigSepReached = true;
		return 0;
	}

	// check each word
	int iPos = 0;
	int iEnd = 0, iTmp;
	CString strWord, strTmp, strTmp2, strTmp3;

	// We want to split up the line into words.
	// Works are seperated by whitespace.
	// Once we've got a work, strip off any beginning and ending punctuation marks.
	// Then, if ignore words with numbers in is set, check if non-alpha chars in
	// word and ignore if there are.

	// Whitespace = space & tab
	// Punctuation = . , ; : - " ' ` ? ! & ( ) < > [ ] { }
	// Alpha = a - z, A - Z
	// Number = 0 - 9
	// Others = anything else left
	//
	// Break sentence into tokens, token seperators are whitespace.
	// If the token then has anything other than alphas in it, skip spelling if this option is set.
	static CString strWhitespace = " \t\r\n";
	static CString strPunctuation = "¬¦£$%^*_=+#~@/\\|0123456789.,;:-\"'`?!&()<>[]{}";
	static CString strNumbers = "0123456789";

	bool bSkipNums = gpGlobalOptions->GetRegSystem()->GetSpelling_IgnoreNumbers() == TRUE ? true : false;
	bool bCheck;

	do
	{
		bCheck = false;
		// Remember where this token started
		iPos = iEnd;
		// iEnd might point at 1 or more whitespace chars if there were > 1 in sequence
		// If it does we want to move iPos over the white space chars
		strTmp2 = strTmp3 = strLine.Mid(iEnd);
		strTmp2.TrimLeft(strWhitespace);
		iPos += strTmp3.GetLength() - strTmp2.GetLength();
		// Get the next token
		strTmp = strLine.Tokenize(strWhitespace, iEnd);

		if (!strTmp.IsEmpty())
		{
			iTmp = strTmp.GetLength();
			// Strip off any non-alpha at start
			strTmp.TrimLeft(strPunctuation);
			// Adjust word start position if we ate any chars at start
			iPos += (iTmp - strTmp.GetLength());
			// And end
			strTmp.TrimRight(strPunctuation);

			// If there's anything left
			if (!strTmp.IsEmpty())
			{
				// If we're set to ignore words with nums in, check
				if (bSkipNums)
				{
					// Any non alpha chars in it?
					iTmp = strTmp.FindOneOf(strNumbers);
					// There are none, ok to check.
					if (iTmp == -1)
						bCheck = true;
				}
				else
					bCheck = true;

				if (bCheck)
				{
					// Check for common things we should ignore
					// These include
					// http:
					// url:
					// mailto:
					// https:
					// telnet:
					// gopher:
					// news:
					// nntp:
					strTmp2 = strTmp;
					strTmp2.MakeLower();
					iTmp = strTmp2.Find(':');
					if (   (iTmp != -1)
						&& (   (strTmp2.Find("http") == 0)
							|| (strTmp2.Find("url") == 0)
							|| (strTmp2.Find("mailto") == 0)
							|| (strTmp2.Find("https") == 0)
							|| (strTmp2.Find("telnet") == 0)
							|| (strTmp2.Find("gopher") == 0)
							|| (strTmp2.Find("news") == 0)
							|| (strTmp2.Find("nntp") == 0)))
					{
						//if (   (strTmp2.GetLength() > 9)
						//	&& (strTmp2[iTmp] == ':'))
							//&& ((strTmp2[iTmp+1] == '/') || (strTmp2[iTmp+1] == '\\'))
							//&& ((strTmp2[iTmp+2] == '/') || (strTmp2[iTmp+2] == '\\')))
						{
							// Ignore it
							bCheck = false;
						}
					}
				}
				if (bCheck)
				{
					int newIndex = 0;
					ctrl.SetSel(iCharIndex + iPos, iCharIndex + iPos + strTmp.GetLength());
					int status = CheckWordSpelling (strTmp, iCharIndex + iPos, newIndex);
					if (status == 2) // Abort
					{
						ctrl.SetSel(0,0);
						return 2;
					}
					else if (status == 1) // Word corrected, update our position
					{
						iEnd += (newIndex-(iCharIndex+iPos+strTmp.GetLength()));

						// get the line again
						iCharIndex = ctrl.LineIndex (iLine);
						iLineLen = ctrl.LineLength (iCharIndex);
						pszLine = new TCHAR[iLineLen+3];
						ctrl.GetLine (iLine, pszLine, iLineLen+3);
						strLine.SetString(pszLine, iLineLen);
						delete [] pszLine;
					}
					// else Spelling OK
					ctrl.SetSel(0,0);
				}
			}
		}
	}
	while (iEnd != -1);

	return 0;
}

// -------------------------------------------------------------------------
// CheckWordSpelling -- returns 0 for success, 1 for reset index, 2 for abort
int TPostBody::CheckWordSpelling (const CString &strWord, int iIndex, int & newIndex)
{
	int ret = 0;
	CRichEditCtrl &ctrl = GetRichEditCtrl ();

	TSpellDlg dlg;
	ret = (CNewsDoc::m_pDoc)->CheckSpelling_Word( strWord, dlg.m_suggestList );

	if (1 == ret)
	{
		dlg.SetBadWord(strWord);
			//m_strWord = strWord;
		switch (dlg.DoModal())
		{
		case IDOK:
			// ignore this word
			ret = 0;
			break;

		case IDC_CORRECT:
			// use dlg.m_strCorrect;
			ctrl.SetSel (iIndex, iIndex + strWord.GetLength());
			ctrl.ReplaceSel(dlg.GetCorrectedWord());
			newIndex = iIndex + dlg.GetCorrectedWord().GetLength();
			ret = 1;
			break;

		case IDC_ADD_WORD:
			(CNewsDoc::m_pDoc)->GetMyWords()->AddWord ( strWord );
			ret = 0;
			break;

		case IDC_CORRECT_AND_ADD:
			ctrl.SetSel(iIndex, iIndex + strWord.GetLength());
			ctrl.ReplaceSel(dlg.GetCorrectedWord());
			newIndex = iIndex + dlg.GetCorrectedWord().GetLength();
			(CNewsDoc::m_pDoc)->GetMyWords()->AddWord(dlg.GetCorrectedWord());
			ret = 1;
			break;

		case IDC_ABORT:
			ret = 2;
			break;

			// User pressed "Esc" - treat this as "abort"
		case IDCANCEL:
			ret = 2;
			break;
		}

		ctrl.SetSel (iIndex, iIndex);    // remove the selection
	}
	return ret;
}

// -------------------------------------------------------------------------
void TPostBody::WrapLines (CRichEditCtrl &ctrl, long firstLine, long lastLine, CString prefix, long lineLength)
{
	CString text ("");

	// Non-empty line introducers should have a single space added
	if (!prefix.IsEmpty())
	{
		prefix += ' ';
	}
	long prefixLength = prefix.GetLength();

	// Make sure we get at least one character per line - if not,
	// clear the prefix! (There's no "nice" thing to do here anyway.)
	if (prefixLength >= lineLength-1)
	{
		prefixLength=0;
		prefix.Empty();
	}

	// Work out the complete text. Extra spaces here don't matter.
	int i = 0;
	for (i=firstLine; i <= lastLine; i++)
	{
		long curLineLength = ctrl.LineLength(ctrl.LineIndex(i));

		if (curLineLength > 0)
		{
			// Buffer must be slightly oversized to prevent corruption
			LPTSTR pszLine = new TCHAR[curLineLength+3];
			ctrl.GetLine (i, pszLine, curLineLength+3);

			CString strLine(&pszLine[prefixLength], curLineLength-prefixLength);
			delete [] pszLine;
			text+=strLine;
			text+=' ';
		}

		//TCHAR* buffer = new TCHAR[curLineLength+4];
		//ctrl.GetLine(i, buffer, curLineLength);
		//buffer[curLineLength]=0;
		//TCHAR* actualText=buffer+prefixLength;
		//text+=actualText;
		//text+=' ';
		//delete (buffer);
	}

	long startOfFirstLine = ctrl.LineIndex(firstLine);
	long startOfLastLine = ctrl.LineIndex(lastLine);
	long endOfLastLine = startOfLastLine + ctrl.LineLength(startOfLastLine);

	ctrl.SetSel(startOfFirstLine, endOfLastLine);

	CString replacement ("");
	long textLength = text.GetLength();

	long wordStart=0;
	long wordLength=0; // Make sure we write a new line immediately
	bool inWord=false;
	bool firstInLine=true;
	long curLineLength=lineLength;
	bool startNewLineNext=true;
	bool firstLineOutput=true;
	for (i=0; i < textLength; i++)
	{
		if (text[i]==' ')
		{
			if (inWord)
			{
				if (firstInLine)
				{
					firstInLine=false;
				}
				else
				{
					replacement+=' ';
					curLineLength++;
				}
				for (int j=wordStart; j < i; j++)
				{
					replacement += text[j];
					curLineLength++;
				}
				inWord=false;
				wordLength=1; // Allow for the space after this word
			}
		}
		else
		{
			if (!inWord)
			{
				inWord=true;
				wordStart=i;
			}
			wordLength++;

			// Do we need a new line for this character?
			if (wordLength + curLineLength > lineLength)
			{
				// Really long word? If so, copy everything we've
				// got so far, and then we'll start a new line
				if (firstInLine && !firstLineOutput)
				{
					for (int j=wordStart; j < i; j++)
					{
						replacement += text[j];
					}
					wordStart=i;
					wordLength=0;
				}

				if (firstLineOutput)
				{
					firstLineOutput=false;
				}
				else
				{
					replacement += '\n';
				}
				replacement += prefix;
				curLineLength=prefixLength;

				firstInLine=true;
			}
		}
	}
	ctrl.ReplaceSel(replacement);
}

// -------------------------------------------------------------------------
void TPostBody::WrapSelection ()
{
	int lineLength = gpGlobalOptions->GetWordWrap() -1;
	CRichEditCtrl &ctrl = GetRichEditCtrl ();

	long selStart;
	long selEnd;

	// Find the selection
	ctrl.GetSel(selStart, selEnd);
	ctrl.HideSelection(true, false);

	long firstLine=ctrl.LineFromChar(selStart);
	long lastLine=ctrl.LineFromChar(selEnd);

	int lastLineOfCurrentBlock=lastLine;

	// Prefix we've been using
	CString prefix ("");
	// Which characters to count as part of the prefix (along with space)
	CString quoteChars = gpGlobalOptions->GetRegFonts()->GetQuoteChars();

	// Work from the far end, so that indexes don't become dodgy
	for (long lineNumber=lastLine; lineNumber >= firstLine; lineNumber--)
	{
		long curLineLength = ctrl.LineLength(ctrl.LineIndex(lineNumber));

		CString strLine;
		if (curLineLength > 0)
		{
			// Buffer must be slightly oversized to prevent corruption
			LPTSTR pszLine = new TCHAR[curLineLength+3];
			ctrl.GetLine (lineNumber, pszLine, curLineLength+3);
			strLine = CString(pszLine, curLineLength);
			delete [] pszLine;
		}

		long j=0;
		for (j=0; j < curLineLength; j++)
		{
			if (strLine.GetAt(j)!=' ' && quoteChars.Find(strLine.GetAt(j))==-1)
				break;
		}

		bool prefixOnly = (j==curLineLength);

		// It's also effectively prefix-only if all we've got on the line is "-- " or "--"
		// so that we don't wrap sigs into the main body.
		// (It should be "-- " and not "--" but OE is rubbish.
		if ((j==curLineLength-2 && strLine.GetAt(j)=='-' && strLine.GetAt(j+1)=='-') ||
			(j==curLineLength-3 && strLine.GetAt(j)=='-' && strLine.GetAt(j+1)=='-' && strLine.GetAt(j+2)==' '))
		{
			prefixOnly=true;
		}

		// Now backtrack over the spaces
		while (j > 0 && strLine.GetAt(j-1)==' ')
			j--;

		CString curPrefix(strLine, j);

		// If this is the last line of the next block, we're really
		// just setting the prefix
		if (lastLineOfCurrentBlock==lineNumber)
		{
			prefix = curPrefix;
		}

		// If the whole line is the prefix, it's a block on
		// its own (needs to be preserved as is, effectively)
		if (prefixOnly)
		{
			// If we've got something to wrap, wrap it
			if (lineNumber != lastLineOfCurrentBlock)
			{
				WrapLines (ctrl, lineNumber+1, lastLineOfCurrentBlock, prefix, lineLength);
			}
			lastLineOfCurrentBlock = lineNumber-1;
			continue;
		}
		if (curPrefix != prefix)
		{
			WrapLines (ctrl, lineNumber+1, lastLineOfCurrentBlock, prefix, lineLength);
			if (lastLineOfCurrentBlock != -1)
				lastLineOfCurrentBlock = lineNumber;
			prefix = curPrefix;
		}
	}
	if (lastLineOfCurrentBlock != firstLine-1)
	{
		WrapLines (ctrl, firstLine, lastLineOfCurrentBlock, prefix, lineLength);
	}
	ctrl.SetSel(selStart, selStart);
	ctrl.HideSelection(false, false);
}

// -------------------------------------------------------------------------
// Hint from the GoodNetKeeping Guide. Warn about lines > 80 chars
BOOL TPostBody::check_format()
{
	// inherited
	return CheckLineLength ( 80 );
}

void TPostBody::OnPostQuotepara()
{
	CHARFORMAT cfm;
	ZeroMemory(&cfm, sizeof cfm);
	cfm.cbSize = sizeof(cfm);
	cfm.dwMask = CFM_FACE | CFM_BOLD;

	GetRichEditCtrl().GetSelectionCharFormat( cfm );

	if (cfm.dwEffects & CFE_BOLD)
		TRACE0("Bold\n");

	// Turn off word wrapping for this paragraph
	PARAFORMAT pf;
	ZeroMemory(&pf, sizeof pf);
	pf.cbSize = sizeof pf;

	//             "Right Edge of Window"
	//                    ||
	// |--- 1440          ||   negative numbers

	pf.dwMask = PFM_RIGHTINDENT;
	pf.dxRightIndent = -28800;  // 10 * 1440. This is way out there

	VERIFY ( GetRichEditCtrl().SendMessage (EM_SETPARAFORMAT, 0, (LPARAM) &pf) );
}

// -------------------------------------------------------------------------
void TPostBody::SetCCIntro (BOOL bInsert)
{
	CRichEditCtrl& rRich = GetRichEditCtrl();
	CHARRANGE sOldRange;
	rRich.GetSel (sOldRange);
	int iOldCCIntroLen = m_iCCIntroLen;

	// remove old CC intro
	CHARRANGE sRange;
	sRange.cpMin = 0;
	sRange.cpMax = m_iCCIntroLen;
	rRich.SetSel (sRange);
	rRich.ReplaceSel ("");

	// get new intro
	CString strCC;
	if (bInsert) {
		TPostDoc *pDoc = GetDocument ();
		TArticleHeader *pHdr = pDoc -> GetHeader ();
		TPostTemplate *pTemplate = (TPostTemplate *) pDoc -> GetDocTemplate ();

		SubstituteMacros (gpGlobalOptions -> GetCCIntro (), strCC,
			pHdr -> GetMessageID (), pHdr -> GetDate (), m_strOriginalAddress,
			m_strOriginalAuthor, pTemplate -> m_NewsGroupID);
		if (strCC.GetLength ())
			strCC += "\r\n\r\n";
	}

	// insert new intro
	sRange.cpMax = 0;
	rRich.SetSel (sRange);
	rRich.ReplaceSel (strCC);
	m_iCCIntroLen = strCC.GetLength ();

	// restore position
	if (bInsert) {
		sOldRange.cpMin += m_iCCIntroLen;
		sOldRange.cpMax += m_iCCIntroLen;
	}
	else {
		sOldRange.cpMin -= iOldCCIntroLen;
		sOldRange.cpMax -= iOldCCIntroLen;
	}
	rRich.SetSel (sOldRange);
}

