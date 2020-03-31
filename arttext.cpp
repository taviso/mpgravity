/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: arttext.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.8  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.7  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/09/19 14:51:11  richard_wood
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
#include "superstl.h"      // STL headers
#include "arttext.h"
#include "article.h"
#include "resource.h"
#include "msgid.h"
#include "tglobopt.h"
#include "licutil.h"
#include "globals.h"
#include "tglobopt.h"
#include "rgsys.h"
#include "server.h"        // TNewsServer
#include "servcp.h"        // TServerCountedPtr
#include "coding.h"        // String_base64_decode
#include "codepg.h"        // CP_Out.bound

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static BOOL sfnArticleInsertField (
								   TPersist822Header* pBaseHdr,
								   TPersist822Text*   pBaseText,
								   WORD       wFieldId,
								   CString&   fldData,
								   E_RENDER   eRender,
								   bool       fPosting,
								   GravCharset * pCharset
								   );

static BOOL fnPrepareReplyMsg (TEmailHeader * pHdr, TEmailText * pText, CFile* pFile);

static int ReplyCreateField (
							 TEmailHeader * pHdr,
							 TEmailText * pText,
							 UINT        idString,
							 BOOL        fShowField,
							 CString&    line );

int CreateDateLine (CString& line);

static int  PreparePost(TArticleHeader * pArt, TArticleText * pText,
						CFile * pFile, BOOL fGenMsgID, bool * pfNewTopic);
static BOOL PreparePostOptional(int count, int* pStrIds, TArticleHeader* pArt, CFile* pFile);

static int NNTP_Syntax(CFile * pSrc, CFile* pDst);
static int NNTP_Syntax_Write(CString& data, CFile* pDst);

static void sfnHandleLongRefs (CStringList & refs, CString & fldData);
static void sfnHandle1KLines ( CString & fldData, TCHAR cBreakChar,
							  LPCTSTR rcBreakToken );

static void WriteMIMEHeaderLines ( TPersist822Header* pHdr, CFile* pFile );

static BOOL  PrepareReplyOptional(
								  int count,
								  int* pStrIds,
								  TEmailHeader* pEmail,
								  CFile* pFile);

static void WriteUserAgent(CFile* pFile);

static void translate_encoded_words (CString& strEncoded, CString & strOut);

/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//  Returns 0 or AT_BAD_FROM
//               AT_BAD_NEWSGROUPS
//               AT_BAD_SUBJECT
//               AT_BAD_MSGID
//               AT_BAD_BODY
//

int ArticlePreparePost (TArticleHeader * pArt,
						TArticleText * pText,
						CFile * pFile,
						BOOL   fAddMsgID,
						bool * pfNewTopic)

{
	int iRet = PreparePost ( pArt, pText, pFile, fAddMsgID, pfNewTopic );
	if (0 == iRet)
		pFile->SeekToBegin();

	return iRet;
}

static
int PreparePost(TArticleHeader * pArt, TArticleText * pText, CFile * pFile,
				BOOL fAddMsgID, bool * pfNewTopic)
{
	int     iStatus = 0;
	CString line;
	BOOL    fRet;
	do
	{
		fRet = ArticleCreateField( pArt, pText, IDS_TMPL_FROM, TRUE, line, kRenderRaw, true);
		if (!fRet) {
			iStatus = IDS_POST_BADFROM;
			break;
		}
		line += "\r\n";
		pFile->Write(line, line.GetLength());

		fRet = ArticleCreateField( pArt, pText, IDS_POST_NEWSGROUPS, TRUE, line, kRenderRaw, true);
		if (!fRet) {
			iStatus = IDS_POST_BADNG;
			break;
		}
		line += "\r\n";
		pFile->Write(line, line.GetLength());

		fRet = ArticleCreateField( pArt, pText, IDS_TMPL_SUBJECT, TRUE, line, kRenderRaw, true);
		if (!fRet) {
			iStatus = IDS_POST_BADSUBJ;
			break;
		}
		line += "\r\n";
		pFile->Write(line, line.GetLength());

		// date
		line.LoadString (IDS_TMPL_DATE);
		line += ": ";
		pFile->Write( line, line.GetLength());
		CreateDateLine ( line );           line += "\r\n";
		pFile->Write( line, line.GetLength());

		// add our own message-id or not

		if (fAddMsgID)
		{
			line.LoadString (IDS_TMPL_MSGID);
			line += ": ";
			pFile->Write( line, line.GetLength());

			line.Empty(); line = pArt->GetMessageID();
			if (line.IsEmpty()) {
				iStatus = IDS_POST_BADMSGID;
				break;
			}
			line += "\r\n";
			pFile->Write( line, line.GetLength());
		}

		line.Empty();
		fRet = ArticleCreateField (pArt, pText, IDS_TMPL_REFS, TRUE, line, kRenderRaw, true);
		if (line.IsEmpty())
			*pfNewTopic = true;
		else
		{
			line += "\r\n";
			pFile->Write ( line, line.GetLength() );

			*pfNewTopic = false;
		}
		int vOptionalIds[] = { IDS_TMPL_DISTRIBUTION,
			IDS_TMPL_EXPIRES,
			IDS_TMPL_FOLLOWUP,
			IDS_TMPL_KEYWORDS,
			IDS_TMPL_ORG,
			IDS_TMPL_REPLYTO,
			IDS_TMPL_SENDER,
			IDS_TMPL_SUMMARY,
			IDS_TMPL_CONTROL };

		PreparePostOptional(sizeof(vOptionalIds)/sizeof(vOptionalIds[0]),
			vOptionalIds, pArt, pFile);

		WriteMIMEHeaderLines ( pArt, pFile );

		// Our User-Agent: id line
		WriteUserAgent( pFile );

		// write custom header fields
		POSITION pos = pArt->GetCustomHeaders ().GetHeadPosition ();
		while (pos)
		{
			const CString &strLine = pArt->GetCustomHeaders ().GetNext (pos);
			pFile->Write (strLine, strLine.GetLength ());
			pFile->Write ("\r\n", 2);
		}

		// BLANK line
		pFile->Write("\r\n", 2);

		fRet = ArticleCreateField (pArt, pText, IDS_TMPL_BODY, FALSE, line, kRenderRaw, true);
		if (!fRet)
		{
			iStatus = IDS_POST_BADBODY;
			break;
		}

		NNTP_Syntax_Write ( line, pFile );

		iStatus = 0;
	} while (FALSE);

	return iStatus;
}

/**************************************************************************
if fPosting is true, then limit lines to 1000K, trim References

called from    ArticlePreparePost           to post
InsertArticleField           to display
genutil - SelectedHeaders    to print, fwd article, save to file
newsdoc - SaveToFile         to file
postbody - InsertArticle     to post, strip out html

note: Agent - forward verbatim - attaches the article as an attachment
message/rfc822
forward unquoted
forward quoted    both show the pretty form
**************************************************************************/
BOOL ArticleCreateField(
						TPersist822Header* pBaseHdr,
						TPersist822Text*   pBaseText,
						WORD      wFieldId,
						BOOL      fShowFieldName,
						CString&  result,

						E_RENDER  eRender /*= kRenderRaw */,

						bool      fPosting /* =false*/,
						GravCharset * pCharset /* =NULL */)
{
	CString fldData;
	BOOL present = sfnArticleInsertField ( pBaseHdr, pBaseText, wFieldId,
		fldData, eRender, fPosting, pCharset );

	if (!present)
		return FALSE;

	if (fShowFieldName)
	{
		WORD wDisplayID;

		// Note:   IDS_TMPL_FOLLOWUP = "Followup-To (optional)"
		//         IDS_TMPLD_FOLLOWUP= "Followup-To"
		//
		//         IDS_TMPL_REPLYTO = "Reply-To (optional)"
		//         IDS_TMPLD_REPLYTO= "Reply-To"
		// .. this is to get the Good NetKeeping Seal of Approval...
		//
		if (IDS_TMPL_FOLLOWUP == wFieldId)
			wDisplayID = IDS_TMPLD_FOLLOWUP;
		else if (IDS_TMPL_REPLYTO == wFieldId)
			wDisplayID = IDS_TMPLD_REPLYTO;
		else
			wDisplayID = wFieldId;

		result.LoadString (wDisplayID);
		result += ": ";
		result += fldData;
	}
	else
		result = fldData;

	if (IDS_TMPL_NGROUP == wFieldId && -1 != result.Find(','))
	{
		// change all "," to COMMA+SPACE. Helps with window word wrapping
		//  on long Newsgroups: lines.
		auto_ptr<TCHAR> sBuf(new TCHAR[result.GetLength() * 2]);
		LPTSTR p = sBuf.get();
		for (int i = 0; i < result.GetLength(); i++)
		{
			TCHAR c = result[i];
			*p++ = c;

			if (',' == c)
				*p++ = ' ';
		}
		*p = '\0';
		result = sBuf.get();
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////
//
// called from ArticleCreateField
//
static BOOL
sfnArticleInsertField (
					   TPersist822Header* pArt,
					   TPersist822Text*   pBaseText,
					   WORD               wFieldId,
					   CString&           fldData,
					   E_RENDER           eRender,
					   bool               fPosting,
					   GravCharset *      pCharset
					   )
{
	/// NEED SOME MAP shit
	BOOL ret = TRUE;
	CString  strInit;

	switch (wFieldId)
	{
	case IDS_TMPL_SUBJECT:
		fldData = (pArt->IsArticle()) ? pArt->CastToArticleHeader()->GetSubject()
			: pArt->CastToEmailHeader()->GetSubject();

		//translate_encoded_words (strInit, fldData);
		break;

	case IDS_TMPL_FROM:
		if (pArt->IsArticle())
		{
			if (kRenderRaw == eRender)
				fldData = pArt->CastToArticleHeader()->GetOrigFrom();
			else
			{
				CString phrase, addr;
				pArt->CastToArticleHeader()->ParseFrom (phrase, addr);
				if (phrase.IsEmpty())
					fldData = addr;
				else if (phrase == addr)
					fldData = "<" + addr + ">";
				else
					fldData = phrase + " <" + addr +">";
			}
		}
		else
			fldData = pArt->CastToEmailHeader()->GetFrom();
		break;

	case IDS_TMPL_DATE:        // needs work
		if (pArt->IsArticle())
			fldData = pArt->CastToArticleHeader()->GetDate();
		else
			ret = FALSE;
		break;

	case IDS_TMPL_MSGID:
		fldData = pArt->GetMessageID();
		break;

	case IDS_TMPL_REFS:
		{
			if (!pArt->IsArticle())
				return FALSE;

			// make sure you handle both cases
			if (fPosting)
			{
				// move data from StringList to fldData
				CStringList refs;
				pArt->CastToArticleHeader()->GetReferencesStringList( &refs );
				sfnHandleLongRefs ( refs, fldData );
				sfnHandle1KLines ( fldData, ' ',  "\r\n ");
			}
			else
				pArt->CastToArticleHeader()->GetReferencesWSList( fldData );

			// output fldData
			ret = !fldData.IsEmpty();
			break;
		}

	case IDS_TMPL_BODY:
		if (kRenderPretty == eRender)
		{
			// a fancier version - Try to hide encodings of attachments
			CString decodeDir = "dummy";
			CString subjline_filename;

			TPersist822Text::ERenderStat eStat = 
				pBaseText->RenderBody_MIME (decodeDir,
				fldData,
				eRender,
				subjline_filename,
				NULL,   // no event monitor,
				false); // fFromCache_NoSKIP

			if (pCharset && (eStat == TPersist822Text::kNotMIME || eStat == TPersist822Text::kNoContentType))
			{
				CString in = fldData;
				CP_Util_Inbound (pCharset, in, in.GetLength(), fldData);
			}
			else
			{
				/* fldData = fldData */
			}
		}
		else
		{
			// kRenderDecode or kRenderRaw
			fldData = pBaseText->GetBody();
		}
		break;

		// dont confuse this with IDS_TMPL_NGROUP= newgroups an article WAS posted to.
	case IDS_POST_NEWSGROUPS:  // newsgroups we WILL post this article to
		{
			if (!pArt->IsArticle())
				return FALSE;
			TStringList grps;
			pArt->CastToArticleHeader()->GetNewsGroups( &grps);
			if (grps.IsEmpty())
				return FALSE;

			{
				// Note - strictly speaking, the delimiter must be "," and not ", "
				CString oneString;
				grps.CommaList ( oneString, "," );
				fldData = oneString;
			}
			if (fPosting)
				sfnHandle1KLines ( fldData,  ','  ,  ",\r\n " );
		}
		break;

		//////////////////////////////
		// fields from the Full Header

	case IDS_TMPL_CONTENT_DESC:
		ret = pBaseText->GetContentDesc ( fldData );
		break;
	case IDS_TMPL_CONTENT_ENCODING:
		ret = pBaseText->GetContentEncoding ( fldData );
		break;
	case IDS_TMPL_CONTENT_TYPE:
		ret = pBaseText->GetContentType( fldData );
		break;
	case IDS_TMPL_DISTRIBUTION:
		ret = pBaseText->GetDistribution ( fldData );
		break;
	case IDS_TMPL_EXPIRES:
		ret = pBaseText->GetExpires( fldData );
		break;
	case IDS_TMPL_FOLLOWUP:
		ret = pBaseText->GetFollowup ( fldData );
		break;
	case IDS_TMPL_KEYWORDS:
		ret = pBaseText->GetKeywords ( fldData );
		break;

	case IDS_TMPL_NGROUP:
		ret = pBaseText->GetNewsgroups ( fldData );
		break;
	case IDS_TMPL_ORG:
		ret = pBaseText->GetOrganization ( fldData );
		break;
	case IDS_TMPL_PATH:
		ret = pBaseText->GetPath ( fldData );
		break;
	case IDS_TMPL_NNTP_POSTINGHOST:
		ret = pBaseText->GetPostingHost ( fldData );
		break;
	case IDS_TMPL_REPLYTO:
		{
			CString rawReplyTo;
			ret = pBaseText->GetReplyTo ( rawReplyTo );

			if (!ret || (kRenderRaw == eRender))
			{
				fldData = rawReplyTo;
			}
			else
			{
				// put the cleaned up ReplyTo  in fldData
				TArticleHeader::DecodeElectronicAddress (rawReplyTo, fldData);
			}
		}
		break;
	case IDS_TMPL_SENDER:
		ret = pBaseText->GetSender ( fldData );
		break;
	case IDS_TMPL_SUMMARY:
		ret = pBaseText->GetSummary ( fldData );
		break;
	case IDS_TMPL_USERAGENT:
		ret = pBaseText->GetUserAgent( fldData );
		break;
	case IDS_TMPL_X_NEWSREADER:
		ret = pBaseText->GetXNewsreader( fldData );
		break;
	default:
		ret= FALSE;
		ASSERT(0);
		break;
	}
	return ret;
} // sfnArticleInsertField

int CreateDateLine (CString& line)
{
	CTime now = CTime::GetCurrentTime();

	// get Day of Week
	line = now.Format("%a, ");

	CString day = now.Format("%d ");
	LPCTSTR szDay = day;

	if (*szDay == '0')
		line += szDay + 1;
	else
		line += szDay;

	// month year HH:MM:SS
	CString mon_year = now.Format("%b %Y %H:%M:%S");
	line += mon_year;

	// Result -  Mon, 19 Jun 1995 20:10:00
	TIME_ZONE_INFORMATION sZone;
	DWORD dwRet = GetTimeZoneInformation ( &sZone );
	if (dwRet != -1)
	{
		// this is a count of minutes
		LONG minutes;
		if (TIME_ZONE_ID_STANDARD == dwRet)
			minutes = sZone.StandardBias + sZone.Bias;
		else if (TIME_ZONE_ID_DAYLIGHT == dwRet)
			minutes = sZone.DaylightBias + sZone.Bias;
		else
			minutes = sZone.Bias;

		// utc = local + bias
		// local = utc - bias;
		CString zone;

		if (minutes >= 0)
			zone.Format(" -%02d%02d", minutes/60, minutes % 60);
		else
		{
			minutes = -minutes;
			zone.Format(" +%02d%02d", minutes/60, minutes % 60);
		}

		line += zone;
	}

	return 0;
}

/**************************************************************************

**************************************************************************/
BOOL fnPrepareReply (TEmailHeader* pHdr, TEmailText* pText, CFile* pFile)
{
	//CMemFile memFile;

	if (fnPrepareReplyMsg ( pHdr, pText, /*&memFile*/ pFile) )
	{
		//NNTP_Syntax (&memFile, pFile);

		pFile->SeekToBegin();
		return TRUE;
	}
	return FALSE;
}

/**************************************************************************
???? do we have to worry about folding?
**************************************************************************/

// merging
BOOL fnPrepareReplyMsg (TEmailHeader* pHdr, TEmailText* pText, CFile* pFile)
{
	CString line;
	BOOL    fRet = FALSE;
	int     stat;
	do
	{
		// 1 - date
		line.LoadString ( IDS_TMPL_DATE );
		line += ": ";
		pFile->Write ( line, line.GetLength() );
		CreateDateLine ( line );   line += "\r\n";
		pFile->Write ( line, line.GetLength() );

		// 2 - from
		stat = ReplyCreateField ( pHdr, pText, IDS_TMPL_FROM, TRUE, line );
		if (stat) break;
		line += "\r\n";
		pFile->Write ( line, line.GetLength() );

		// 3 - subject
		stat = ReplyCreateField ( pHdr, pText, IDS_TMPL_SUBJECT, TRUE, line );
		if (stat < 0) break;
		line += "\r\n";
		pFile->Write ( line, line.GetLength() );

		// 4 - to
		stat = ReplyCreateField ( pHdr, pText, IDS_REPLY_TO, TRUE, line );
		if (stat) break;
		line += "\r\n";
		pFile->Write ( line, line.GetLength() );

		// 5 and 6
		int vOptional1[] = { IDS_REPLY_CC /* IDS_REPLY_BCC*/  };
		PrepareReplyOptional (sizeof(vOptional1)/sizeof(vOptional1[0]),
			vOptional1, pHdr, pFile);
		// 7 - message id
		line.LoadString (IDS_TMPL_MSGID);
		line += ": ";
		pFile->Write( line, line.GetLength());
		line.Empty(); line = pHdr->GetMessageID();
		ASSERT(FALSE == line.IsEmpty());
		line += "\r\n";
		pFile->Write ( line, line.GetLength() );

		// 8 - in reply to.  Omit the line if the data is blank
		stat = ReplyCreateField ( pHdr, pText, IDS_REPLY_INREPLYTO, TRUE, line );
		if (stat < 0)
			break;
		else if (0 == stat)
		{
			line += "\r\n";
			pFile->Write ( line, line.GetLength() );
		}

		// 9 - optional fields
		int vOptional2[] = { IDS_TMPL_REPLYTO,
			IDS_TMPL_SENDER,
			IDS_TMPL_KEYWORDS };
		PrepareReplyOptional (sizeof(vOptional2)/sizeof(vOptional2[0]),
			vOptional2, pHdr, pFile);

		// 10- oops forgot this!  added 10-25-95
		WriteMIMEHeaderLines ( pHdr, pFile );

		// 11 User Agent
		WriteUserAgent( pFile );

		// 12  BLANK line
		pFile->Write ( "\r\n", 2 );

		// 13 message body
		stat = ReplyCreateField ( pHdr, pText, IDS_TMPL_BODY, FALSE, line );
		//pFile->Write ( line, line.GetLength() );
		NNTP_Syntax_Write (line, pFile);

		fRet = TRUE;
	} while (FALSE);
	return fRet;
}

//-------------------------------------------------------------------------
// Returns   0 for success
//           1 for empty field
//          -1 for error
static int ReplyCreateField (
							 TEmailHeader*  pHdr,
							 TEmailText*    pText,
							 UINT        idString,
							 BOOL        fShowField,
							 CString&    line )
{
	CString info;
	int ret = 0;
	switch (idString)
	{
	case IDS_TMPL_FROM:        info = pHdr->GetFrom();          break;
	case IDS_TMPL_SUBJECT:     info = pHdr->GetSubject();       break;
	case IDS_REPLY_INREPLYTO:  info = pHdr->Get_InReplyTo();    break;
	case IDS_TMPL_BODY:        info = pText->GetBody();         break;

	case IDS_REPLY_TO:
		pHdr->GetToText( info );
		break;

	default:
		// unknown field!
		ASSERT(0);
		return -1;
	}

	if (fShowField)
	{
		line.LoadString ( idString );
		line += ": ";
		line += info;
	}
	else
		line = info;
	if (info.IsEmpty())
		ret = 1;
	return ret;
}

/////////////////////////////////////////////////////////////////
// creates:          Albert Choy <alchoy@nando.net>
//    or  :          alchoy@nando.net (Albert Choy)
void fnFromLine (CString & strOutput,
				 LONG      lGroupID,
				 BOOL      fAddressFirst /* = FALSE */,
				 BOOL      fMailing      /* = FALSE */)
{
	CString addr, fullName;

	TServerCountedPtr cpServer;      // this is a smart pointer

	// Part 1) the 'real' address
	addr = cpServer->GetEmailAddress();
	fullName = cpServer->GetFullName();

	// Part 2) if posting and there is a posting mail-address override, use that
	if (!fMailing)
	{
		CString strPostAddress = cpServer->GetEmailAddressForPost ();
		if (!strPostAddress.IsEmpty ())
			addr = strPostAddress;
	}

	// Part 3) use the per-group overrides ?
	if (lGroupID)
	{
		BOOL fUseLock;
		TNewsGroup *psNG;
		TNewsGroupUseLock useLock (cpServer, lGroupID, &fUseLock, psNG);
		if (fUseLock)
		{
			psNG->Open ();
			if (psNG->GetOverrideEmail ())
				addr = psNG->GetEmail ();
			if (psNG->GetOverrideFullName ())
				fullName = psNG->GetFullName ();
			psNG->Close ();
		}
		else
		{
			ASSERT(0);
		}
	}

	// If required, quote fullName
	static char cQuote[] = "()<>@,;:\\.[]";
	if (fullName.FindOneOf(cQuote) != -1)
	{
		fullName.Trim('"');
		fullName = "\"" + fullName + "\"";
	}

	// now the tricky stuff is done, just format it the way we want
	if (fAddressFirst)
	{
		if (fullName.IsEmpty())
			strOutput = addr;
		else
			strOutput = addr + " (" + fullName + ")";
	}
	else
	{
		int len = addr.GetLength();
		// address last
		if ((len >= 1) && addr[0] == '<' && addr[len-1] == '>')
			strOutput = fullName + " " + addr;
		else
			strOutput = fullName + " <" + addr + ">";
	}
}

void WriteMIMEHeaderLines ( TPersist822Header* pHdr, CFile* pFile )
{
	CString ver;
	CString typ;
	CString encode;
	CString desc;
	CString result;
	pHdr->GetMimeLines(&ver, &typ, &encode, &desc);
	if (FALSE == ver.IsEmpty())
	{
		AfxFormatString1 (result, IDS_MIME_HDR_VER, ver);
		pFile->Write (result, result.GetLength());
		pFile->Write ("\r\n", 2);

		AfxFormatString1 (result, IDS_MIME_HDR_CONTYPE, typ);
		pFile->Write (result, result.GetLength());
		pFile->Write ("\r\n", 2);

		// encoding may be empty for Message/partial
		if (!encode.IsEmpty())
		{
			AfxFormatString1 (result, IDS_MIME_HDR_CONENCODE, encode);
			pFile->Write (result, result.GetLength());
			pFile->Write ("\r\n", 2);
		}

		if (!desc.IsEmpty())
		{
			AfxFormatString1 (result, IDS_MIME_HDR_CONDESC, desc);
			pFile->Write (result, result.GetLength());
			pFile->Write ("\r\n", 2);
		}

	}
}

static BOOL PreparePostOptional(
								int count,
								int* pStrIds,
								TArticleHeader* pArt,
								CFile* pFile)
{
	int iLabel;
	CString data;
	for (int i = 0; i < count; ++i)
	{
		iLabel = pStrIds[i];
		data.Empty();
		switch (pStrIds[i])
		{
		case  IDS_TMPL_DISTRIBUTION:
			data = pArt->GetDistribution();
			break;
		case  IDS_TMPL_EXPIRES:
			data = pArt->GetExpires();
			break;
		case  IDS_TMPL_FOLLOWUP:
			iLabel = IDS_TMPLD_FOLLOWUP;
			data = pArt->GetFollowup();
			break;
		case  IDS_TMPL_KEYWORDS:
			data = pArt->GetKeywords();
			break;
		case  IDS_TMPL_ORG:
			data = pArt->GetOrganization();
			break;
		case  IDS_TMPL_REPLYTO:
			{
				iLabel = IDS_TMPLD_REPLYTO;
				data = pArt->GetReplyTo();
			}
			break;
		case  IDS_TMPL_SENDER:
			data = pArt->GetSender();
			break;
		case  IDS_TMPL_SUMMARY:
			data = pArt->GetSummary();
			break;
		case  IDS_TMPL_CONTROL:
			data = pArt->GetControl();
			break;
		default:
			ASSERT(0);
			break;
		}  // switch
		if (!data.IsEmpty())
		{
			CString desc;
			VERIFY ( desc.LoadString ( iLabel ) );
			desc += ": ";
			desc += data;
			desc += "\r\n";
			pFile->Write (desc, desc.GetLength() );
		}
	} // for loop
	return TRUE;
}

static BOOL  PrepareReplyOptional(
								  int count,
								  int* pStrIds,
								  TEmailHeader* pEmail,
								  CFile* pFile)
{
	CString data;
	int     iLabel;
	for (int i = 0; i < count; ++i)
	{
		data.Empty();
		iLabel = pStrIds[i];
		switch (pStrIds[i])
		{
		case  IDS_TMPL_KEYWORDS:
			data = pEmail->GetKeywords();
			break;
		case  IDS_TMPL_REPLYTO:
			iLabel = IDS_TMPLD_REPLYTO;
			data = pEmail->GetReplyTo();
			break;
		case  IDS_TMPL_SENDER:
			data = pEmail->GetSender();
			break;
		case  IDS_REPLY_CC:
			data.Empty();
			pEmail->GetCCText( data );
			break;
		case  IDS_REPLY_BCC:
			data.Empty();
			pEmail->GetBCCText( data );
			break;
		default:
			ASSERT(0);
			break;
		}  // switch
		if (!data.IsEmpty())
		{
			CString desc;
			VERIFY ( desc.LoadString ( iLabel ) );
			desc += ": ";
			desc += data;
			desc += "\r\n";
			pFile->Write (desc, desc.GetLength() );
		}
	} // for loop
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// Created  11-24-95  amc
//
// Handles doubling of PERIODS and the <PERIOD><CRLF> at the end
//
static int NNTP_Syntax_Write (CString& data, CFile* pDst)
{
	int len = data.GetLength();
	int i;
	LPCTSTR ptr = (LPCTSTR) data;
	TCHAR   pr = 0;                 // prev
	TCHAR   prpr = 0;               // previous previous char

	int delta = 1;
	if (0 == *ptr)
		delta = 0;
	else if ('.' == *ptr)
	{
		pDst->Write("..", 2);
		pr = '.';
	}
	else
	{
		pDst->Write(ptr, 1);
		pr = *ptr;
	}
	ptr+=delta;
	for (i = delta; i < len;)
	{
		if ( ('.' == *ptr) && ('\r' == prpr && '\n' == pr) )
		{
			pDst->Write("..", 2);
			delta = 1;
			prpr = pr;
			pr   = *ptr;
		}
		else if ('\r' == *ptr || '\n' == *ptr)
		{
			pDst->Write(ptr, 1);
			delta = 1;
			prpr = pr;
			pr   = *ptr;
		}
		else
		{
			// boring character
			LPCTSTR fwd = ptr;
			while (*fwd && (*fwd != '\r'))
				++fwd;
			delta = fwd-ptr;
			ASSERT(delta >= sizeof(TCHAR));
			pDst->Write(ptr, delta);
			delta /= sizeof(TCHAR);
			// can reset state vars
			prpr = 0;
			pr   = 0;
		}
		i += delta;
		ptr += delta;
	}
	// cap off message with <CR><LF>.<CR><LF>
	if (0 == i || ('\r' == prpr && '\n' == pr))
		pDst->Write(".\r\n", 3);
	else
		pDst->Write("\r\n.\r\n", 5);
	return 0;
}

extern TLicenseSystem *gpLicenseDope;

///////////////////////////////////////////////////////////////////////////
//  Write out User Agent: MicroPlanet-Gravity/2.60.2057
//
static void WriteUserAgent( CFile* pFile)
{
	CString line;

	line.LoadString ( IDS_USER_AGENT );
	line += ": ";
	pFile->Write( line, line.GetLength() );

	line = TLicenseSystem::GetUserAgentDescription();
	line += "\r\n";
	pFile->Write( line, line.GetLength() );
}

// ------------------------------------------------------------------------
static BOOL sfnGetFirstPart(CString & data, TCHAR cBreakChar, CString * pPart)
{
	if (data.IsEmpty())
		return FALSE;
	TCHAR rcBreakString[2];
	rcBreakString[0] = cBreakChar;
	rcBreakString[1] = '\0';

	(*pPart) = data.SpanExcluding(rcBreakString);
	int iPartLen = pPart->GetLength();
	int myLen = data.GetLength();
	data = data.Right(myLen - iPartLen);

	while (!data.IsEmpty())
	{
		if (data[0] == cBreakChar)
		{
			data = data.Right (data.GetLength() - 1);
			data.TrimRight();
		}
		else
			break;
	}
	return TRUE;
}

// ------------------------------------------------------------------------
// Break up really long lines, so the news server will accept them
//  Semantically, <CRLF><SPACE> is equiv to <SPACE>
static void sfnHandle1KLines ( CString & fldData, TCHAR cBreakChar, LPCTSTR
							  rcBreakToken )
{
	int kBreakLen = gpGlobalOptions->GetRegSystem()->GetHeaderFieldBreakLen();
	int iLen = fldData.GetLength();
	if (iLen < kBreakLen)
		return;

	CString unit, out;

	int iCurLineLen = 0;
	int iTokLen = lstrlen(rcBreakToken);

	while (sfnGetFirstPart (fldData, cBreakChar, &unit))
	{
		if (iCurLineLen + unit.GetLength() >= kBreakLen)
		{
			if (0 == iCurLineLen)
			{
				// this is a Really Big token
				out += unit;
				iCurLineLen = unit.GetLength();
			}
			else
			{
				// cap off line
				out += rcBreakToken;

				// another line is born
				out += unit;
				iCurLineLen = unit.GetLength();
			}
		}
		else
		{
			if (!out.IsEmpty())
			{
				out += cBreakChar;
				++iCurLineLen;
			}
			out += unit;
			iCurLineLen += unit.GetLength();
		}
	}

	fldData = out;
}

// ------------------------------------------------------------------------
void   elim_malformed_refs (CStringList& refsIn, CStringList & refsOut)
{
	POSITION pos = refsIn.GetHeadPosition ();
	while (pos)
	{
		CString oneRef = refsIn.GetNext (pos);
		oneRef.TrimLeft();
		oneRef.TrimRight();

		do
		{
			int ln = oneRef.GetLength();
			if (ln < 5)  // asume <e@x>
				break;

			// begin and end
			if ('<' != oneRef[0] && '>' != oneRef[ln-1])
				break;

			// EXACTLY one at sign, 2 braces
			int nAT = 0;
			int nAB1 = 0;     // check opening angle bracket
			int nAB2 = 0;
			for (int i = 0; i < ln; i++)
			{
				switch (oneRef[i])
				{
				case '<':      nAB1++;  break;
				case '>':      nAB2++;  break;
				case '@':      nAT++;   break;
				}
			}
			if (1 != nAT || 1 != nAB1 || 1 != nAB2)
				break;

			// looks pretty decent
			refsOut.AddTail (oneRef);

		} while (0);
	}
}

// ------------------------------------------------------------------------
int refs_final_len (CStringList & refs, CString* pOut)
{
	POSITION p = refs.GetHeadPosition();
	int n = 0;
	while (p)
	{
		const CString & oneRef = refs.GetNext (p);
		n += oneRef.GetLength() + 1;
		if (pOut)
			(*pOut) += oneRef + ' ';
	}

	if (pOut)
		pOut->TrimRight();

	return n - 1;
}

// ------------------------------------------------------------------------
// Before this function would create wrap long References. Due to GNKSA 2.0
//   we are following their algorithm, which is to
//   1) filter out malformed items
//   2) keep #1 and the last 3
//   3) respect 995chars + EOL length limit
static void sfnHandleLongRefs ( CStringList & refsIn, CString & fldData )
{
	CStringList refs;

	elim_malformed_refs (refsIn, refs);
	const int nLimit = 983;

	// our limit is 995+CRLF.
	//  so that would really be  995 - "References: " (12) = 983

	while ((refs.GetCount() > 4) && refs_final_len(refs,NULL) > nLimit)
	{
		refs.RemoveAt (refs.FindIndex(1));  // remove #2
	}

	refs_final_len (refs, &fldData);
}

// -------------------------------------------------------------------------
static int Decode_Base64 (CString & strDataIn, CString & strDataOut)
{
	int ln = strDataIn.GetLength();
	LPTSTR pIn = strDataIn.GetBuffer(ln);

	LPTSTR pOut = strDataOut.GetBuffer(ln);

	ULONG uLen = String_base64_decode (pIn, ln, pOut, ln);

	strDataIn.ReleaseBuffer (ln);
	strDataOut.ReleaseBuffer (uLen);
	return 0;
}

// -------------------------------------------------------------------------
// Returns 0 for success
//
static int Decode_QP (CString & strDataIn, CString & strDataOut)
{
	int len = strDataIn.GetLength();
	for (int i = 0; i < len; )
	{
		TCHAR c = strDataIn[i];
		switch (c)
		{
		case '=':
			{
				if (i + 2 >= len)
					return 1;

				c = strDataIn[i + 1];

				if (!isxdigit (c))         // must be hex!
					return 1;

				BYTE e, e2;

				if (isdigit (c))
					e = c - '0';
				else
					e = c - (isupper (c) ? 'A' : 'a')  + 10;

				// get next char
				c = strDataIn[i + 2];
				if (!isxdigit (c))
					return 1;

				if (isdigit (c))
					e2 = c - '0';
				else
					e2 = 10 + c - (isupper (c) ? 'A' : 'a');

				strDataOut += TCHAR(16 * e + e2);

				i += 3;
			}
			break;

		case '_':
			strDataOut += (TCHAR)0x20;
			i ++;
			break;

		default:
			strDataOut += c;
			i ++;
			break;
		}

	}
	return 0;
}

// -------------------------------------------------------------------------
static void read_encoded_word (LPCTSTR & pEncoded, CString & strOut)
{
	CString temp = "=?";
	pEncoded += 2;

	LPCTSTR pStart = pEncoded;

	CString strCharset;
	TCHAR   cEncoding;
	CString strDataEncoded;
	TCHAR   cTemp;

	bool fFoundEnd = false;

	bool fOK = false;

	// read the charset

	while (*pEncoded)
	{
		if (*pEncoded == '?')
		{
			pEncoded++;
			fOK = true;
			break;
		}
		strCharset += *pEncoded++;
	}

	if (!fOK)
	{
		strOut = temp;
		pEncoded = pStart;
		return;
	}

	cTemp = *pEncoded++;
	if (cTemp != 'Q' && cTemp != 'B')
	{
		strOut = temp;
		pEncoded = pStart;
		return;
	}
	else
		cEncoding = cTemp;

	cTemp = *pEncoded++;

	if (cTemp != '?')
	{
		strOut = temp;
		pEncoded = pStart;
		return;
	}

	cTemp = 0;
	fOK = false;
	while (*pEncoded)
	{
		if ('?' == *pEncoded  &&  '=' == *(pEncoded + 1))
		{
			fOK = true;
			pEncoded += 2;
			break;
		}
		strDataEncoded += *pEncoded++;
	}

	if (!fOK)
	{
		strOut = temp;
		pEncoded = pStart;
		return;
	}

	CString strDataDecoded;

	TRACE("charset %s, %c %s\n",
		LPCTSTR(strCharset),
		cEncoding,
		LPCTSTR(strDataEncoded));

	int stat;

	if ('Q' == cEncoding)
		stat = Decode_QP (strDataEncoded, strDataDecoded);
	else
		stat = Decode_Base64 (strDataEncoded, strDataDecoded);

	if (stat)
	{
		// abort
		strOut = temp;
		pEncoded = pStart;
		return;
	}

	// now do character set mapping
	strOut = strDataDecoded;
}

// -------------------------------------------------------------------------
// translate_encoded_words -- deal with mime-encoded header lines, rfc-1522
//
static void translate_encoded_words (CString& strEncoded, CString & strOut)
{
	// check for simple case - not a special mime string
	bool fStart = strEncoded.Find ("=?") > -1;
	bool fEnd   = strEncoded.Find ("?=") > -1;

	if (! (fStart && fEnd) )
	{
		strOut = strEncoded;
		return;
	}

	bool fStringEmpty = true;
	TCHAR c;

	int iPos = 0;
	LPCTSTR pEncoded = strEncoded;
	while (*pEncoded)
	{
		c = *pEncoded;

		if (*pEncoded == '=' && *(pEncoded + 1) == '?')
		{
			CString strWord;
			read_encoded_word (pEncoded, strWord);
			strOut += strWord;
		}
		else
		{
			strOut += c;
			pEncoded++;
		}
	}
}
