/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: article2.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
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
#include "article.h"
#include "mplib.h"
#include "fileutil.h"
#include "superstl.h"
#include "coding.h"
#include "resource.h"
#include "mailadr.h"
#include "tstrlist.h"
#include "sortlst.h"
#include "rgui.h"
#include "tglobopt.h"
#include "evtlog.h"
#include "rgswit.h"            // boolean switches
#include "codepg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

extern TGlobalOptions* gpGlobalOptions;
extern TEventLog*    gpEventLog;
extern HWND          ghwndMainFrame;

CMap<CString, LPCTSTR, TMimeThread, TMimeThread&> TPersist822Text::m_sequencer;

static const int kBASE64 = 10;
static const int kQP     = 11;
static const int kUU     = 12;
static const int kXX     = 13;
static const int k7bit   = 14;
static const int kYenc   = 15;

static BOOL fnIsBlankLine (LPCTSTR ptr);

static bool myFindNoCase (const CString & haystack, const CString & needle)
{
	CString lowHay = haystack;
	CString lowNd = needle;

	lowHay.MakeLower();
	lowNd.MakeLower();

	return (lowHay.Find (lowNd) == -1) ? false : true;
}

class TPunchOut {
public:
	TPunchOut(LPTSTR pdata, int idx)
	{
		// plop in temporary null
		m_idx = idx;
		m_pdata = pdata;
		m_holdchar = pdata[idx];
		pdata[idx] = '\0';
	}
	~TPunchOut(void)
	{
		// replace with orig character
		m_pdata[m_idx] = m_holdchar;
	}
protected:
	TCHAR  m_holdchar;
	LPTSTR m_pdata;
	int    m_idx;
};

static BOOL fnDecode_known_type(const CString& decodeSpec,
								int            type,
								TCHAR *        data,
								int            dataLen,
								ULONG *        puResultLen,
								BOOL  *        pfIOError);

IMPLEMENT_SERIAL(TBase822Text,    PObject, 1);
IMPLEMENT_SERIAL(TPersist822Text, TBase822Text, 2);
IMPLEMENT_SERIAL(TArticleText,    TPersist822Text, 1);
IMPLEMENT_SERIAL(TEmailText,      TPersist822Text, 1);

// ===========================================================================
// History of the Combined TPersist822Text && TArticleText Serialize
//    version 1 - your baseline object
//    version 2 - TPersist822Text adds m_off_Content_Dispo
//    version 3 - TArticleText doesn't need to write out m_Header,
//                 the parent class already does this. duh.
//    version 4 - TPersist822Text adds m_off_UserAgent and m_off_XNewsreader
//
static const BYTE COMBINEDTEXT_VERSION = 4;

//---------------------------------------------------------------------------
// this constructor is protected, so it acts like an abstract base class
TBase822Text::TBase822Text(BYTE byVersion)
	: PObject(byVersion)
{
}

#if defined(_DEBUG)
void TBase822Text::Dump(CDumpContext& dc) const
{
	// base class version
	PObject::Dump(dc);
	dc << "Base822Text ";
}
#endif

static void fnString_Cat(CString& str, LPCTSTR src, int len);

static void fnString_Cat(CString& str, LPCTSTR src, int len)
{
	if (len <= 0) return;
	int curlen = str.GetLength();
	LPTSTR p = str.GetBuffer(curlen + len);
	memcpy( p + curlen, src, len * sizeof(TCHAR) );
	str.ReleaseBuffer(curlen+len);
}

const WORD OFFSET_NONE = 0xFFFE;

void TBase822Text::Serialize (CArchive & ar)
{
	PObject::Serialize ( ar );

	if (ar.IsStoring())
	{
		ar << m_Body;
	}
	else
	{
		ar >> m_Body;
	}
}

TBase822Text& TBase822Text::operator=(const TBase822Text & rhs)
{
	if (this == &rhs)
		return *this;

	PObject::operator=(rhs);

	m_Body = rhs.m_Body;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// set the body from an open Cfile or CMemFile
void
TBase822Text::SetBody(CFile& fl, int len)
{
	if (-1 == len)
	{
		int sz = (int) fl.GetLength();
		LPTSTR buf = m_Body.GetBuffer (sz);

		// fill the string
		fl.SeekToBegin();
		fl.Read ( buf, sz );
		m_Body.ReleaseBuffer ( sz );
	}
	else
	{
		LPTSTR buf = m_Body.GetBuffer ( len );
		fl.Read ( buf, len );
		m_Body.ReleaseBuffer ( len );
	}
}

void
TBase822Text::Pad_CRLF(void)
{
	int n = m_Body.GetLength();
	if (0 == n)
	{
		return;
	}
	else if (1 == n)
	{
		if (m_Body[0] == '\r')
			m_Body += "\n";
		else
			m_Body += "\r\n";
		return;
	}
	else
	{
		if (m_Body[n-2] == '\r' && m_Body[n-1] == '\n')
			return;

		if (m_Body[n-1] == '\r')
			m_Body += "\n";
		else
			m_Body += "\r\n";
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG)
void TPersist822Text::Dump(CDumpContext& dc) const
{
	// base class version
	PObject::Dump(dc);
	dc << "Persist822Text ";
}
#endif

//-------------------------------------------------------------------------
// Version 2 - also store offset of the 'Content-Disposition'
TPersist822Text::TPersist822Text()
	: TBase822Text(COMBINEDTEXT_VERSION)
{
	m_UniqueNumber = 0;
	m_fParsed = FALSE;
	m_fMime   = FALSE;
	m_off_Distribution    = OFFSET_NONE;
	m_off_Content_Desc    = OFFSET_NONE;
	m_off_Content_Enc     = OFFSET_NONE;
	m_off_Content_Type    = OFFSET_NONE;
	m_off_Expires         = OFFSET_NONE;
	m_off_Keywords        = OFFSET_NONE;
	m_off_Followup        = OFFSET_NONE;
	m_off_Newsgroups      = OFFSET_NONE;
	m_off_Organization    = OFFSET_NONE;
	m_off_Path            = OFFSET_NONE;
	m_off_PostingHost     = OFFSET_NONE;
	m_off_ReplyTo         = OFFSET_NONE;
	m_off_Sender          = OFFSET_NONE;
	m_off_Summary         = OFFSET_NONE;
	m_off_Content_Dispo   = OFFSET_NONE;    // new in Version 2
    m_off_UserAgent       = OFFSET_NONE;	// New in Version 4
    m_off_XNewsreader     = OFFSET_NONE;	// New in Version 4
}

TPersist822Text::~TPersist822Text()
{
}

// ------------------------------------------------------------------------
// SizeHint -- estimate of the disk space we need
DWORD TPersist822Text::SizeHint ()
{
	return         m_Body.GetLength()  +     // from base class
		sizeof(m_fArticle)         +
		sizeof(m_UniqueNumber)     +
		m_Header.GetLength()+
		sizeof(m_fParsed)          +
		sizeof(m_fMime)            +
		sizeof(m_off_Distribution) +
		sizeof(m_off_Content_Desc) +
		sizeof(m_off_Content_Enc)  +
		sizeof(m_off_Content_Type) +
		sizeof(m_off_Expires)      +
		sizeof(m_off_Keywords)     +
		sizeof(m_off_Followup)     +
		sizeof(m_off_Newsgroups)   +
		sizeof(m_off_Organization) +
		sizeof(m_off_Path)         +
		sizeof(m_off_PostingHost)  +
		sizeof(m_off_ReplyTo)      +
		sizeof(m_off_Sender)       +
		sizeof(m_off_Summary)      +
		sizeof(m_off_Content_Dispo)+
    	sizeof(m_off_UserAgent)    +
    	sizeof(m_off_XNewsreader);
}

void TPersist822Text::Serialize (CArchive & archive)
{
	BYTE byVersion;

	TBase822Text::Serialize ( archive );

	byVersion = GetObjectVersion ();

	if (archive.IsStoring())
	{
		archive << m_fArticle;
		archive << m_UniqueNumber;
		archive << m_Header;
		archive << m_fParsed;
		archive << m_fMime;
		archive << m_off_Distribution   ;
		archive << m_off_Content_Desc   ;
		archive << m_off_Content_Enc    ;
		archive << m_off_Content_Type   ;
		archive << m_off_Expires        ;
		archive << m_off_Keywords       ;
		archive << m_off_Followup       ;
		archive << m_off_Newsgroups     ;
		archive << m_off_Organization   ;
		archive << m_off_Path           ;
		archive << m_off_PostingHost    ;
		archive << m_off_ReplyTo        ;
		archive << m_off_Sender         ;
		archive << m_off_Summary        ;

		archive << m_off_Content_Dispo  ;    // new in Version 2

    	archive << m_off_UserAgent;          // New in ver 4
    	archive << m_off_XNewsreader;        // New in ver 4
	}
	else
	{
		archive >> m_fArticle;
		archive >> m_UniqueNumber;
		archive >> m_Header;

		archive >> m_fParsed;
		archive >> m_fMime;

		archive >> m_off_Distribution   ;
		archive >> m_off_Content_Desc   ;
		archive >> m_off_Content_Enc    ;
		archive >> m_off_Content_Type   ;
		archive >> m_off_Expires        ;
		archive >> m_off_Keywords       ;
		archive >> m_off_Followup       ;
		archive >> m_off_Newsgroups     ;
		archive >> m_off_Organization   ;
		archive >> m_off_Path           ;
		archive >> m_off_PostingHost    ;
		archive >> m_off_ReplyTo        ;
		archive >> m_off_Sender         ;
		archive >> m_off_Summary        ;

		if (byVersion >= 2)
			archive >> m_off_Content_Dispo;

		if (byVersion >= 4)
		{
	    	archive >> m_off_UserAgent;
			archive >> m_off_XNewsreader;
		}
	}
}

TPersist822Text & TPersist822Text::operator=(const TPersist822Text & rhs)
{
	if (this != &rhs)
	{
		TBase822Text::operator= (rhs);

		m_fArticle     = rhs.m_fArticle;
		m_UniqueNumber = rhs.m_UniqueNumber;
		m_Header       = rhs.m_Header;

		m_fParsed      = rhs.m_fParsed;
		m_fMime        = rhs.m_fMime;
		m_off_Distribution   = rhs.m_off_Distribution;
		m_off_Content_Desc   = rhs.m_off_Content_Desc;
		m_off_Content_Enc    = rhs.m_off_Content_Enc;
		m_off_Content_Type   = rhs.m_off_Content_Type;
		m_off_Expires        = rhs.m_off_Expires;
		m_off_Keywords       = rhs.m_off_Keywords;
		m_off_Followup       = rhs.m_off_Followup;
		m_off_Newsgroups     = rhs.m_off_Newsgroups;
		m_off_Organization   = rhs.m_off_Organization;
		m_off_Path           = rhs.m_off_Path;
		m_off_PostingHost    = rhs.m_off_PostingHost;
		m_off_ReplyTo        = rhs.m_off_ReplyTo;
		m_off_Sender         = rhs.m_off_Sender;
		m_off_Summary        = rhs.m_off_Summary;
		m_off_Content_Dispo  = rhs.m_off_Content_Dispo;

		m_off_UserAgent      = rhs.m_off_UserAgent;
    	m_off_XNewsreader    = rhs.m_off_XNewsreader;

		m_decodeDir          = rhs.m_decodeDir;
	}
	return *this;
}

void TPersist822Text::SetHeader (LPCTSTR hdr)
{
	LPTSTR p = m_Header.GetBuffer(lstrlen(hdr));
	LPCTSTR s = hdr;

	// strip out CRLF followed by WhiteSpace
	while (*s)
	{
		if ('\r' == *s && '\n' == *(s+1) && *(s+2) &&
			(' ' == *(s+2) || '\t' == *(s+2)))
		{
			*p++ = *(s+2);
			s += 3;
		}
		else
		{
			*p++ = *s++;
		}
	}
	*p = '\0';
	m_Header.ReleaseBuffer();
}

///////////////////////////////////////////////////////////////
// Code handles cases where the line is terminated with '\n' instead
// of '\r\n'
//

void TPersist822Text::ParseFullHeader (CString * pstrUserAgent /* =NULL */)
{
	// for decoding sometimes we really need the user-agent line,
	//   so re-parse dammit.
	if (m_fParsed && (NULL == pstrUserAgent))
		return;

	if (m_Header.IsEmpty())
		return ;

	int     idxColon;

	LPCTSTR hdr = m_Header;
	LPCTSTR ptr = hdr;

	LPTSTR  pLine = new TCHAR[m_Header.GetLength()+1];
	auto_ptr<TCHAR> deleter(pLine);

	while (*ptr != NULL)
	{
		LPCTSTR ptrStart = ptr;       // remember start of line
		LPTSTR  pLn = pLine;

		// extract one line
		while (*ptr != '\0'  && *ptr != '\r' && *ptr != '\n')
			*pLn++ = *ptr++;

		// cap off line
		*pLn = 0;

		CString oneLine = pLine;

		// there must be a Colon in here
		idxColon = oneLine.Find (':');

		if (idxColon  != -1)
		{
			CString field_name = oneLine.SpanExcluding(":");
			field_name.MakeLower();

			int fld_name_len = field_name.GetLength() + 1;

			int idx = (ptrStart - hdr);

			WORD wDataOffset = WORD(idx + fld_name_len);

#if defined(_DEBUG)
			LPCTSTR pLook = hdr + wDataOffset;
#endif

			// Analyze field name
			switch (field_name[0])
			{
			case 'c':
				if (field_name == "content-description")
					m_off_Content_Desc = wDataOffset;
				else if (field_name == "content-transfer-encoding")
					m_off_Content_Enc = wDataOffset;
				else if (field_name == "content-type")
					m_off_Content_Type = wDataOffset;
				else if (field_name == "content-disposition")
					m_off_Content_Dispo = wDataOffset;
				break;

			case 'd':
				if (field_name == "distribution")
					m_off_Distribution = wDataOffset;
				break;

			case 'e':
				if (field_name == "expires")
					m_off_Expires = wDataOffset;
				break;

			case 'f':
				if (field_name == "followup-to")
					m_off_Followup = wDataOffset;
				break;

			case 'k':
				if (field_name == "keywords")
					m_off_Keywords = wDataOffset;
				break;

			case 'm':
				if (field_name == "mime-version")
					m_fMime = TRUE;
				break;

			case 'n':
				if (field_name == "newsgroups")
					m_off_Newsgroups = wDataOffset;
				else if (field_name == "nntp-posting-host")
					m_off_PostingHost = wDataOffset;
				break;

			case 'o':
				if (field_name == "organization")
					m_off_Organization = wDataOffset;
				break;

			case 'p':
				if (field_name == "path")
					m_off_Path = wDataOffset;
				break;

			case 'r':
				if (field_name == "reply-to")
					m_off_ReplyTo = wDataOffset;
				break;

			case 's':
				if (field_name == "sender")
					m_off_Sender = wDataOffset;
				else if (field_name == "summary")
					m_off_Summary = wDataOffset;
				break;

			case 'u':
				if ((field_name == "user-agent"))
				{
					m_off_UserAgent = wDataOffset;
					if (pstrUserAgent)
					{
						*pstrUserAgent = oneLine.Mid (idxColon + 1);
					}
				}
				break;

			case 'x':
				if (field_name == "x-newsreader")
					m_off_XNewsreader = wDataOffset;
				break;
			}
		}

		// advance past line separator
		while (*ptr == '\r' || *ptr == '\n')
			ptr++;
	}
	m_fParsed = TRUE;   // when do we write this out?
}

///////////////////////////////////////////////////////////////
// series of accessors for the Full-Header fields
void TPersist822Text::util_fullheader_snip(WORD offset, CString& result)
{
	result = (m_Header.Mid(offset)).SpanExcluding("\r\n");
	result.TrimLeft();
}

BOOL TPersist822Text::GetDistribution(CString& dist)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Distribution)
		return FALSE;
	util_fullheader_snip(m_off_Distribution, dist);
	return TRUE;
}

BOOL TPersist822Text::GetContentDesc(CString& ngroups)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Content_Desc)
		return FALSE;
	util_fullheader_snip(m_off_Content_Desc, ngroups);
	return TRUE;
}

BOOL TPersist822Text::GetContentEncoding(CString& encoding)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Content_Enc)
		return FALSE;
	util_fullheader_snip(m_off_Content_Enc,encoding );
	return TRUE;
}

BOOL TPersist822Text::GetContentType(CString& type)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Content_Type) return FALSE;
	util_fullheader_snip(m_off_Content_Type, type);
	return TRUE;
}

BOOL TPersist822Text::GetContentDisposition(CString& type)
{
	ParseFullHeader ();
	if (OFFSET_NONE == m_off_Content_Dispo) return FALSE;
	util_fullheader_snip(m_off_Content_Dispo, type);
	return TRUE;
}

BOOL TPersist822Text::GetExpires(CString& expires)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Expires) return FALSE;
	util_fullheader_snip(m_off_Expires, expires);
	return TRUE;
}

BOOL TPersist822Text::GetFollowup(CString& followup)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Followup) return FALSE;
	util_fullheader_snip(m_off_Followup, followup);
	return TRUE;
}

BOOL TPersist822Text::GetKeywords(CString& keywords)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Keywords) return FALSE;
	util_fullheader_snip(m_off_Keywords, keywords);
	return TRUE;
}

BOOL TPersist822Text::GetNewsgroups(CString& newsgroups)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Newsgroups) return FALSE;
	util_fullheader_snip(m_off_Newsgroups, newsgroups);
	return TRUE;
}

BOOL TPersist822Text::GetOrganization(CString& org)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Organization) return FALSE;
	util_fullheader_snip(m_off_Organization, org);
	return TRUE;
}

BOOL TPersist822Text::GetPath(CString& path)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Path) return FALSE;
	util_fullheader_snip(m_off_Path, path);
	return TRUE;
}

BOOL TPersist822Text::GetPostingHost(CString& host)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_PostingHost) return FALSE;
	util_fullheader_snip(m_off_PostingHost, host);
	return TRUE;
}

BOOL TPersist822Text::GetReplyTo(CString& replyto)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_ReplyTo) return FALSE;
	util_fullheader_snip(m_off_ReplyTo, replyto);
	return TRUE;
}

BOOL TPersist822Text::GetSender(CString& sender)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Sender) return FALSE;
	util_fullheader_snip(m_off_Sender, sender);
	return TRUE;
}

BOOL TPersist822Text::GetSummary(CString& summary)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_Summary) return FALSE;
	util_fullheader_snip(m_off_Summary, summary);
	return TRUE;
}

BOOL TPersist822Text::GetUserAgent(CString& strUserAgent)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_UserAgent) return FALSE;
	util_fullheader_snip(m_off_UserAgent, strUserAgent);
	return TRUE;
}

BOOL TPersist822Text::GetXNewsreader(CString& strXNewsreader)
{
	ParseFullHeader();
	if (OFFSET_NONE == m_off_XNewsreader) return FALSE;
	util_fullheader_snip(m_off_XNewsreader, strXNewsreader);
	return TRUE;
}

static CODE_EVENT_MONITOR gpfnCodeEventMonitor;

///////////////////////////////////////////////////////////////////
//
// Short-term goal - I can't think of a decent way of handling
//                   Message/Partial.
//                   I will handle Multipart/Mixed as best I can
//
TPersist822Text::ERenderStat
TPersist822Text::RenderBody_MIME(
								 const CString& decodeDirectory,
								 CString& bod,
								 E_RENDER eRender,
								 const CString& subjline_filename,
								 CODE_EVENT_MONITOR pEventMonitor,
								 bool fFromCache_NoSKIP)
{
	ASSERT(!decodeDirectory.IsEmpty());
	m_decodeDir = decodeDirectory;
	gpfnCodeEventMonitor = pEventMonitor;

	CString content_type;

	if (FALSE == m_fMime ||
		!GetContentType ( content_type ))
	{
		bod = GetBody();
		m_decodeDir.Empty();
		m_decodeDir.FreeExtra();
		if (!m_fMime)
			return TPersist822Text::kNotMIME;
		else
			return TPersist822Text::kNoContentType;
	}

	//content_type.MakeLower(); - might have case-sensitive Boundary string

	CString content_desc;
	GetContentDesc ( content_desc );

	CString content_enc;
	GetContentEncoding ( content_enc );
	content_enc.MakeLower();

	CString content_dispo;
	GetContentDisposition ( content_dispo );
	content_dispo.MakeLower();

	CString content_id;

	CString& rbod = m_Body;
	int b_len = rbod.GetLength();
	LPTSTR data = rbod.GetBuffer(b_len);

	ERenderStat eStat =
		Render_Entity (subjline_filename,
		&content_type, &content_desc, &content_enc,
		&content_dispo, &content_id,
		data, 0, b_len, eRender, bod,
		fFromCache_NoSKIP);
	rbod.ReleaseBuffer(b_len);

	// don't need this anymore
	m_decodeDir.Empty();
	m_decodeDir.FreeExtra();

	return eStat;
}

//-------------------------------------------------------------------------
BOOL  TPersist822Text_IsMultipart(const CString& s)
{
	CString lwr = s;
	lwr.MakeLower();
	return (0 == lwr.Find("multipart"));
}

//-------------------------------------------------------------------------
TPersist822Text::ERenderStat
TPersist822Text::Render_Entity(
							   const CString& subjline_filename,
							   CString* pContent_type,
							   CString* pContent_desc,
							   CString* pContent_enc,
							   CString* pContent_disposition,
							   CString* pContent_id,
							   LPCTSTR  data,
							   int      start,
							   int      end,           // plop in temp null?
							   E_RENDER eRender,
							   CString& bod,            // output body if viewing. not used in decode
							   bool     fFromCache_NoSKIP
							   )
{
	//TPunchOut mgr(data, end);
	data += start;
	T_MIME_HEADER_AREA sArea;

	LPCTSTR pAdvance;
	// get information on body-part
	if (!pContent_type && !pContent_desc && !pContent_enc && !pContent_disposition)
	{
		pAdvance = read_header_area(data, &sArea);
		pContent_type = &sArea.type;   // setup working ptrs
		pContent_desc = &sArea.description;
		pContent_enc  = &sArea.encoding;
		pContent_disposition = &sArea.disposition;
	}
	else
	{
		sArea.type        = *pContent_type;
		sArea.description = *pContent_desc;
		sArea.encoding    = *pContent_enc;
		sArea.disposition = *pContent_disposition;
		sArea.contentId   = *pContent_id;
		pAdvance = data;
	}

	CString type_subtype = pContent_type->SpanExcluding(";");

	CString charset = pContent_type->Mid(type_subtype.GetLength() + 1);

	charset.MakeLower();
	type_subtype.MakeLower();

	int adv_len = pAdvance - data;

	if (type_subtype == "text/plain")
	{
		switch (eRender)
		{
		case kRenderDecode:
			{
				// Whereas before we tried to pass a block of text through
				//  the UU decoder, now we strictly believe the content-encoding

				if (TCodeMgr::uuencode_histogram(pAdvance, end - adv_len))
				{
					// why decode this? this is text. It might be something

					return TPersist822Text::kTreatAsUU;
				}
				else
				{
					// if it is encoded as 7bit, just write it out.
					Render_opaque_binary(subjline_filename,
						&sArea, type_subtype,
						pAdvance, start, end - adv_len, eRender, bod,
						fFromCache_NoSKIP);
				}

			}
			break;

		case kRenderPretty:
			{
				// check the content-encoding

				CString lowcaseEncoding = *pContent_enc;
				lowcaseEncoding.MakeLower ();

				// handle qp Text
				if (lowcaseEncoding.Find ("quoted-printable") != -1)
				{
					end -= adv_len;
					int ln = end - start;
					LPTSTR pBuf = new TCHAR[ln + 1];
					auto_ptr<TCHAR> sDeleter(pBuf);

					// this gets us from 7 bit data to 8 bit bytes
					ULONG uLen = String_QP_decode (pAdvance,
						ln,
						pBuf,
						ln + 1);

					CString strTranslated;

					if (0 == CP_Inbound ( charset, pBuf, uLen, strTranslated ))
					{
						fnString_Cat ( bod, strTranslated, strTranslated.GetLength() );
					}
					else
						fnString_Cat ( bod, pBuf, uLen );
				}

				// handle base64 Text
				else if (lowcaseEncoding.Find ("base64") != -1)
				{
					end -= adv_len;
					int ln = end - start;
					LPTSTR pBuf = new TCHAR[ln + 1];
					auto_ptr<TCHAR> sDeleter(pBuf);

					// this gets us from 7 bit data to 8 bit bytes
					ULONG uLen = String_base64_decode ((LPTSTR)(LPCTSTR)pAdvance,
						ln,
						pBuf,
						ln + 1);

					CString strTranslated;

					if (0 == CP_Inbound ( charset, pBuf, uLen, strTranslated ) )
					{
						fnString_Cat ( bod, strTranslated, strTranslated.GetLength() );
					}
					else
						fnString_Cat ( bod, pBuf, uLen );
				}
				else
				{
					end -= adv_len;
					int ln = end - start;
					CString strTranslated;

					if (0 == CP_Inbound ( charset, pAdvance, ln, strTranslated ))
					{
						// data is in strTranslated
						fnString_Cat ( bod, strTranslated, strTranslated.GetLength() );
					}
					else
						fnString_Cat ( bod, pAdvance, ln );
				}
			}
			break;

		default:
			{
				end -= adv_len;
				int ln = end - start;

				fnString_Cat ( bod, pAdvance, ln );

				break;
			}
		} // switch
	} // text/plain
	else if (TPersist822Text_IsMultipart(type_subtype))
	{
		end -= adv_len;
		int ln = end - start;

		// type_subtype has been made lowercase
		if ("multipart/alternative" == type_subtype)
			Render_multipart_alternative (subjline_filename,
			&sArea.type, pAdvance,
			0, ln, eRender, bod, fFromCache_NoSKIP);
		else
			Render_multipart_mixed(subjline_filename,
			&sArea.type, pAdvance,
			0, ln, eRender, bod, fFromCache_NoSKIP);
	}
	else if (type_subtype == "application/octet-stream" ||
		(type_subtype.GetLength() >= 5 &&
		type_subtype[0] == 'i' &&
		type_subtype[1] == 'm' &&
		type_subtype[2] == 'a' &&
		type_subtype[3] == 'g' &&
		type_subtype[4] == 'e'))
	{
		end -= adv_len;
		int ln = end - start;
		Render_opaque_binary(subjline_filename,
			&sArea, type_subtype,
			pAdvance, 0, ln,
			eRender, bod,
			fFromCache_NoSKIP);
	}
	else if (type_subtype == "message/partial")
	{
		end -= adv_len;
		int ln = end - start;

		if (kRenderDecode == eRender)
		{
			// HURM.. save to file and then analyze?
			//        check the type now?  We can't - might not be first section
			// - add to high level sequencer
			save_partial_mime_chunk (subjline_filename,
				&sArea.type, &sArea.description,
				&sArea.encoding,
				pAdvance, 0, ln,
				fFromCache_NoSKIP);
		}
		else
			fnString_Cat ( bod, pAdvance, ln );
	}
	else if (type_subtype == "message/rfc822")
	{
		end -= adv_len;
		int ln = end - start;

		// section added on Nov 10 1997
		if (kRenderDecode == eRender)
		{
			Render_Entity ( subjline_filename,
				0,0,0,0,0,
				pAdvance,
				0,
				ln,
				eRender,
				bod,
				fFromCache_NoSKIP);
		}
		else
			fnString_Cat ( bod, pAdvance, ln );
	}
	else
	{
		end -= adv_len;
		int ln = end - start;

		// unrecognized
		if (kRenderDecode == eRender)
			Render_opaque_binary(subjline_filename,
			&sArea, type_subtype,
			pAdvance, 0, ln,
			eRender, bod, fFromCache_NoSKIP);
		else
			fnString_Cat ( bod, pAdvance, ln );
	}

	return TPersist822Text::kNoError;
}

////////////////////////////////////////////////////////////////////////
void  TPersist822Text::Render_opaque_binary(
	const CString& subjline_filename,
	const T_MIME_HEADER_AREA * pArea,
	const CString& type_subtype,
	LPCTSTR        data,
	int            start,
	int            end,
	E_RENDER       eRender,
	CString&       bod,
	bool fFromCache_NoSKIP
	)
{
	if (eRender != kRenderDecode)
		Render_summary (&pArea->description, type_subtype, bod);
	else
	{
		// check for B64, Q-P, X-UU  and  REJECT others
		Decode_All (subjline_filename, pArea, data + start, end - start,  fFromCache_NoSKIP);
	}
}

////////////////////////////////////////////////////////////////////////
void  TPersist822Text::Decode_All (
								   const CString& subjline_filename,
								   const T_MIME_HEADER_AREA * pArea,
								   LPCTSTR        data,
								   int            len,
								   bool fFromCache_NoSKIP
								   )
{
	CStringList ignoreList;

	gpGlobalOptions->GetRegUI()->GetIgnoreMimeTypes( ignoreList );

	CString lowcaseContentType = pArea->type;
	lowcaseContentType.MakeLower();

	if (lowcaseContentType.Find("text/plain") != -1)
	{
		// sometimes text is just a description of a JPG. Sometimes it is
		//   a text attachment. (C-code, pascal, whatever)
		TPath tpTest;

		if (0 == Decode_section_fname (pArea, subjline_filename, tpTest))
		{
			// go ahead and decode it
		}
		else
		{
			// no explicit filename. follow default behavior
			goto def_havior;
		}
	}
	else
	{
def_havior:
		POSITION p = ignoreList.GetHeadPosition();
		while (p)
		{
			const CString& ign = ignoreList.GetNext (p);
			if (lowcaseContentType.Find(ign) != -1)
			{
				// skip it & do nothing
				return ;
			}
		}
	}
	// check the encoding type
	if (pArea->encoding.IsEmpty())
	{
		// skip it & do nothing
		return ;
	}

	CString lowcaseEncType = pArea->encoding;
	lowcaseEncType.MakeLower();

	if (lowcaseEncType.Find("base64") != -1)
		Decode_known_type (subjline_filename,
		kBASE64, pArea, data, len, fFromCache_NoSKIP);

	else if (lowcaseEncType.Find("quoted-printable") != -1)
		Decode_known_type (subjline_filename, kQP, pArea, data, len, fFromCache_NoSKIP);

	// should be "x-uuencode" but look for "uuencode"
	else if (lowcaseEncType.Find("uuenco") != -1)
		Decode_known_type (subjline_filename,
		kUU, pArea, data, len, fFromCache_NoSKIP);

	else if (lowcaseEncType.Find("x-xx") != -1)
		Decode_known_type (subjline_filename,
		kXX, pArea, data, len, fFromCache_NoSKIP);

	else if (lowcaseEncType.Find("7bit") != -1)
		Decode_known_type (subjline_filename,
		k7bit, pArea, data, len, fFromCache_NoSKIP);

	else if (lowcaseEncType.Find("x-yenc") != -1)
		Decode_known_type (subjline_filename,
		kYenc, pArea, data, len, fFromCache_NoSKIP);
	else
	{
		//   unhandled encoding type
		CString errMsg;
		AfxFormatString1(errMsg, IDS_ERR_WEIRD_ENCODING, LPCTSTR(pArea->encoding));

		gpEventLog->AddError (TEventEntry::kDecode, errMsg, pArea->type);
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void TPersist822Text::Render_summary (
									  const CString* pContent_desc,
									  const CString& type_subtype,
									  CString&       bod
									  )
{
	CString summary = "\r\n[" + type_subtype ;
	if (pContent_desc->IsEmpty())
		summary += "]\r\n";
	else
		summary += "," + *pContent_desc + "]\r\n";

	bod += summary;
}

void TPersist822Text_ExtractBound(const CString& input, CString& bound)
{
	int i = 0;
	int tot = input.GetLength();

	BOOL iQuote = 0;
	while (i < tot)
	{
		TCHAR tc = input[i];
		if ('"' == tc)
		{
			if (0==iQuote)
			{
				bound += tc;
				iQuote = 1;
			}
			else if (1==iQuote)
			{
				bound += tc;
				break;
			}
		}
		else if ('\r' == tc || '\n' == tc || ';' == tc)
			break;
		else
			bound += tc;

		++i;
	}

	// check for dbl quotes, strip them
	{
		int b_len = bound.GetLength();
		if (bound[0] == '"' && bound[b_len-1] == '"')
			bound = bound.Mid(1, b_len-2);
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void TPersist822Text::Render_multipart_mixed(
	const CString& subjline_filename,
	const CString* pContent_type,
	LPCTSTR        data,
	int            start,
	int            end,
	E_RENDER       eRender,
	CString& bod,
	bool fFromCache_NoSKIP)
{
	CString bound;
	CString lowercaseType = *pContent_type;
	lowercaseType.MakeLower();

	// preserve case of unique Boundary string
	int idx = lowercaseType.Find("boundary=");
	if (-1 == idx)
	{                  // this should have been here
		bod = GetBody();
		return;
	}

	TPersist822Text_ExtractBound( pContent_type->Mid(idx+9), bound );

	if (bound.IsEmpty())
	{
		bod = GetBody();
		return;
	}

	LPCTSTR cptrBody = data;

	//	// RFC says
	//	Overall, the body of a multipart entity may be specified as follows:
	//
	//	multipart-body := preamble 1*encapsulation
	//		close-delimiter epilogue
	//
	//encapsulation := delimiter body-part CRLF
	//
	//delimiter := "--" boundary CRLF ; taken from Content-Type field.
	//		   ; There must be no space
	//		   ; between "--" and boundary.
	//
	//		   close-delimiter := "--" boundary "--" CRLF ; Again, no space by "--",

	// strictly speaking it should be CRLF + "--" + bound,
	//   but the 1st CRLF may not be there
	CString delim_norm = "\r\n--" + bound ;
	int delim_len = delim_norm.GetLength();
	BOOL fFirstDelim = TRUE;
	int  iPushPast = 0;    // how much to push past the delimiter

	//  strictly speaking there should be a "--CRLF" at the end, but we are forgiving
	CString delim_end = "\r\n--" + bound + "--";
	LPTSTR ptrMid ;
	LPTSTR ptrFirst = (LPSTR)_tcsstr(cptrBody, LPCTSTR(delim_norm) + 2);  // dont search for CRLF on first pass
	//LPTSTR ptrEnd = _tcsstr(cptrBody, LPCTSTR(delim_end));

	if (/*0 == ptrEnd || */ 0 == ptrFirst)
	{
		fnString_Cat(bod, data+start, end-start);
		return;
	}

	while ( (ptrMid = _tcsstr(ptrFirst + sizeof(TCHAR), LPCTSTR(delim_norm)))
		!= NULL)
	{
		//   test for blank line. Process data

		//   data is between ptrFound + ptrMid
		if (fFirstDelim)
		{
			fFirstDelim = FALSE;
			iPushPast = 2 + bound.GetLength() + 2;
		}
		else
			iPushPast = delim_len + 2;

		if (fnIsBlankLine (ptrFirst + iPushPast))
		{
			LPCTSTR pSrc = _tcsstr(ptrFirst + iPushPast, "\r\n")+2;

			if (kRenderDecode != eRender)
			{
				//   no header area
				fnString_Cat(bod, pSrc, ptrMid - pSrc);
			}
			else
			{
				//   why decode this? this is text. It might be something
				TCodeMgr aDecoder;
				aDecoder.InstallEventMonitor(gpfnCodeEventMonitor);
				aDecoder.SetDestinationDirectory (m_decodeDir);

				aDecoder.DecodeIdentified(subjline_filename, 1, 1,
					pSrc, ptrMid - pSrc);
			}
		}
		else
		{
			// my trust in recursion

			Render_Entity (subjline_filename,
				0, // type
				0,
				0,
				0,
				0, // id
				ptrFirst + iPushPast,
				0, // start index
				(int)(ptrMid - (ptrFirst + iPushPast)), // end index
				eRender,
				bod,
				fFromCache_NoSKIP);

#if  0
			T_MIME_HEADER_AREA sArea;

			LPCTSTR advance = read_header_area (ptrFirst + iPushPast, &sArea);

			// Concatenate text subtypes. case-insensitive match on "text/????"
			if (NULL != _tcsstr(sArea.type, "text/"))
			{
				if (kRenderDecode != eRender)
				{
					//$2003 needs work
					fnString_Cat(bod, advance, ptrMid - advance);
				}
				else
				{
					CString type_subtype = sArea.type;
					type_subtype.MakeLower();

					// if it is encoded as 7bit, just write it out.
					Render_opaque_binary (subjline_filename, &sArea,
						type_subtype,
						advance,      // ptr to pData
						0,            // start index into pData
						ptrMid - advance, // end idx into pData
						eRender,
						bod,
						fFromCache_NoSKIP);

				}
			}
			else     // everything else
			{
				Render_Entity (subjline_filename,
					&sArea.type, &sArea.description, &sArea.encoding,
					&sArea.disposition, &sArea.contentId,
					advance,
					0,                  // start
					ptrMid - advance,   // end
					eRender,
					bod,
					fFromCache_NoSKIP);
			}
#endif
		}

		/*
		if (ptrMid == ptrEnd)
		break;
		*/
		ptrFirst = ptrMid;
	}
}

// ------------------------------------------------------------------------
// read_header_area -- reads the header area describing a MIME section
LPCTSTR  TPersist822Text::read_header_area(
	LPCTSTR   src,
	LPT_MIME_HEADER_AREA  pArea
	)
{
	PCHAR buf = 0;
	int   bufSz = 256;

	LPCTSTR adv = src;
	int    res_len;
	do
	{
		while (true)
		{
			try
			{
				if (0 == buf)
					buf = new TCHAR[bufSz];

				adv = read_line (adv, buf, bufSz, &res_len);
				break;
			}
			catch(TException *pE)
			{
				// buffer overrun!  increase size.
				delete buf;
				buf = 0;
				bufSz += 1024;
				pE->Delete();
			}
		}

		CString ln = LPCTSTR(buf);
		ln.TrimRight(); ln.TrimLeft();
		// check for blank line
		if (ln.GetLength() == 0)
			break;
		CString typ_subtype = ln.SpanExcluding(":");
		typ_subtype.MakeLower();
		if (typ_subtype == "content-type")
		{
			pArea->type = ln.Mid(typ_subtype.GetLength() + 1);
			// type.MakeLower(); - can't homogenize this. The boundary might be case-sensitive
			pArea->type.TrimLeft();
		}
		else if (typ_subtype == "content-description")
		{
			pArea->description = ln.Mid(typ_subtype.GetLength() + 1);
			pArea->description.TrimLeft();
		}
		else if (typ_subtype == "content-transfer-encoding")
		{
			pArea->encoding = ln.Mid(typ_subtype.GetLength() + 1);
			pArea->encoding.MakeLower();
			pArea->encoding.TrimLeft();
		}
		else if (typ_subtype == "content-disposition")
		{
			// added this on 4-1-96
			pArea->disposition = ln.Mid(typ_subtype.GetLength() + 1);
			pArea->disposition.TrimLeft();
		}
		else if (typ_subtype == "content-id")
		{
			// added this on 10-14
			pArea->contentId = ln.Mid(typ_subtype.GetLength() +1);
			pArea->contentId.TrimLeft();
		}
	} while (TRUE);

	if (buf)
		delete buf;

	if (pArea->type.IsEmpty())
		pArea->type = "text/plain";

	if (pArea->encoding.IsEmpty())
		pArea->encoding = "7Bit";
	return adv;
}

LPCTSTR TPersist822Text::read_line (
									LPCTSTR src,
									TCHAR* buf,
									int buf_size,
									int* res_len
									)
{
	// NOTE:  7-31-96 if we have
	//    Content-Type: image/jpeg;<CR><LF><SPACE>filename="amy.jpg"
	//  then we want to treat the line break as 1 white space character
	//
	//  but if we have <CR><LF><SPACE><CR><LF>  this really is two lines.
	//

	int i = 0;
	while (*src)
	{
		if (i > buf_size)
			throw(new TException("buffer overrun", kFatal));

		if ('\r' == *src && '\n' == *(src+1))
		{
			if (0 == i)
			{
				// these are the first characters. Accept them
				buf[0] = '\r';
				buf[1] = '\n';
				buf[2] = NULL;
				*res_len = 2;
				return src + (2*sizeof(TCHAR));
			}

			if (*(src+2) && (' ' == *(src+2) || '\t' == *(src+2)))
			{
				src += 2;  // this CRLF was followed by WhiteSpace. Ignore.
				while (*src && (' '== *src || '\t'== *src))
					++src;
				if ('\0' == *src)
					break;
			}
			else
			{
				buf[i] = NULL;
				*res_len = i;
				return src + (2*sizeof(TCHAR));
			}
		}
		buf[i++] = *src++;
	}
	buf[i] = '\0';
	*res_len = i;
	return src;
}

//////////////////////////////////////////////////////////////////////////
void
TPersist822Text::save_partial_mime_chunk (
	const CString& subjline_filename,
	CString* ptype,
	CString* pdesc,
	CString* penc,
	LPCTSTR data,
	int start,
	int end,
	bool fFromCache_NoSKIP)
{
	CString param;
	// part, total, msg-id
	int idxID, idxNUM, idxTOT;
	int iNUM = -1;
	int iTOT = -1;
	idxID = ptype->Find("id=");
	if (-1 == idxID) return;
	idxNUM = ptype->Find("number=");
	if (-1 == idxNUM) return;
	idxTOT = ptype->Find("total=");
	if (-1 != idxTOT)
	{
		param = (ptype->Mid(idxTOT + 6)).SpanExcluding(";");
		iTOT = atoi(param);
	}
	param = (ptype->Mid(idxNUM + 7)).SpanExcluding(";");
	iNUM = atoi(param);

	// the id
	param = (ptype->Mid(idxID + 3)).SpanExcluding(";");

	CString filename;
	make_temp_filename( filename );
	{
		CFile fl;
		fl.Open (filename, CFile::modeCreate | CFile::modeWrite);
		fl.Write (data + start, end-start);
	}
	TMimeThread thread;
	TMimeSequence seq;
	seq.m_id = param;
	seq.m_fname = filename;
	seq.m_part  = iNUM;
	seq.m_total = iTOT;

	if (m_sequencer.Lookup(param, thread))
	{
		thread.Add (seq);
		m_sequencer.SetAt(param, thread);
	}
	else
	{
		thread.Add (seq);
		m_sequencer.SetAt(param, thread);
	}
	// turn off before destructor happens
	// version in the CMap owns the files
	thread.m_fFileOwnership = FALSE;

	CString bigFile;
	if (thread.IsComplete())
	{
		thread.SuperAppend(bigFile);
		presto_chango_memmapfile(subjline_filename, bigFile, fFromCache_NoSKIP);
		m_sequencer.RemoveKey(param);
	}
}

////////////////////////////////////
// from Paul Yao's book p746

void TPersist822Text::presto_chango_memmapfile (
	const CString& subjline_filename,
	const CString& fname,
	bool  fFromCache_NoSKIP)
{

	HANDLE hfile = CreateFile(fname,
		GENERIC_READ,       // access mode
		0,                  // share mode (0=none)
		0,                  // security
		OPEN_EXISTING,      // Create flags
		FILE_ATTRIBUTE_NORMAL, // attribute flags
		0);                 // file to emulate

	if (INVALID_HANDLE_VALUE == hfile)
		return;
	DWORD dwFileSize = GetFileSize ( hfile, NULL /* ptr to Hi32bits of filesize*/ );

	HANDLE hmapobj =
		CreateFileMapping ( hfile,
		NULL,           // ptr to security
		PAGE_READONLY,  // page protection
		0,              // max object size is
		0,              //   size of hfile
		0 );            // name goes here

	if (hmapobj)
	{
		void* pVoid =
			MapViewOfFile(hmapobj,
			FILE_MAP_READ,
			0,   // start offset Hi 32bits
			0,   // start offset Lo 32bits
			0);  // map entire file
		if (pVoid)
		{
			CString type;
			CString desc;
			CString enc;
			CString dummy_bod;
			Render_Entity (subjline_filename,
				0,   // type
				0,   // description
				0,   // encoding
				0,   // disposition
				0,   // content-id
				(LPCTSTR) pVoid,
				0, int(dwFileSize), kRenderDecode, dummy_bod,
				fFromCache_NoSKIP);

			UnmapViewOfFile(pVoid);
		}
		CloseHandle ( hmapobj );
	}
	CloseHandle ( hfile );
}

struct T_MIME_ALTERNATIVE_SEGMENT
{
public:
	T_MIME_ALTERNATIVE_SEGMENT()
	{
		m_fText = false;
		m_pData = NULL;
		m_iLen  = 0;
	}

public:
	bool                 m_fText;
	T_MIME_HEADER_AREA   m_sArea;

	LPCTSTR  m_pData;
	int      m_iLen;
};

// ------------------------------------------------------------------------
//
void TPersist822Text::Render_multipart_alternative(
	const CString& subjline_filename,
	const CString* pContent_type,
	LPCTSTR        data,
	int            start,
	int            end,
	E_RENDER        eRender,
	CString&       bod,
	bool fFromCache_NoSKIP)
{
	bool fSeenText = false;
	CString bound;
	CString lowercaseType = *pContent_type;
	lowercaseType.MakeLower();

	// preserve case of unique Boundary string
	int idx = lowercaseType.Find("boundary=");
	if (-1 == idx)
	{                  // this should have been here
		bod = GetBody();
		return;
	}

	TPersist822Text_ExtractBound( pContent_type->Mid(idx+9), bound );

	if (bound.IsEmpty())
	{
		bod = GetBody();
		return;
	}

	LPCTSTR cptrBody = data;

	// strictly speaking it should be CRLF + "--" + bound,
	//   but the 1st CRLF may not be there
	CString delim_norm = "\r\n--" + bound ;
	int delim_len = delim_norm.GetLength();
	BOOL fFirstDelim = TRUE;
	int  iPushPast = 0;    // how much to push past the delimiter

	//  strictly speaking there should be a "--CRLF" at the end, but we are forgiving
	CString delim_end = "\r\n--" + bound + "--";
	LPTSTR ptrMid ;
	LPTSTR ptrFirst = (LPSTR)_tcsstr(cptrBody, LPCTSTR(delim_norm) + 2);  // dont search for CRLF on first pass
	LPTSTR ptrEnd = (LPSTR)_tcsstr(cptrBody, LPCTSTR(delim_end));

	if (0 == ptrEnd || 0 == ptrFirst)
	{
		fnString_Cat(bod, data+start, end-start);
		return;
	}

	// Phase 1 : identify segments

	CPtrArray vSegments;

	while ( (ptrMid = _tcsstr(ptrFirst + sizeof(TCHAR), LPCTSTR(delim_norm)))
		!= NULL)
	{
		//   test for blank line. Process data

		//   data is between ptrFound + ptrMid
		if (fFirstDelim)
		{
			fFirstDelim = FALSE;
			iPushPast = 2 + bound.GetLength() + 2;
		}
		else
			iPushPast = delim_len + 2;

		if (fnIsBlankLine (ptrFirst + iPushPast))
		{
			LPCTSTR src = _tcsstr(ptrFirst + iPushPast, "\r\n")+2;

			T_MIME_ALTERNATIVE_SEGMENT * pAltSegment = new T_MIME_ALTERNATIVE_SEGMENT;

			pAltSegment->m_fText = true;
			pAltSegment->m_pData = src;             // data start
			pAltSegment->m_iLen  = ptrMid - src;    // data length

			vSegments.Add ( pAltSegment );
		}
		else
		{
			T_MIME_ALTERNATIVE_SEGMENT * pAltSegment = new T_MIME_ALTERNATIVE_SEGMENT;
			T_MIME_HEADER_AREA & sArea = pAltSegment->m_sArea;

			LPCTSTR advance = read_header_area (ptrFirst + iPushPast, &sArea);

			pAltSegment->m_pData = advance;
			pAltSegment->m_iLen  = ptrMid - advance;

			vSegments.Add ( pAltSegment );
		} // else clause

		if (ptrMid == ptrEnd)
			break;
		ptrFirst = ptrMid;
	} // while clause

	// Phase 2 : process segments

	int k;
	T_MIME_ALTERNATIVE_SEGMENT * pAltSegment ;

	if (kRenderDecode == eRender)
	{
		for (k = 0; k < vSegments.GetSize(); k++)
		{
			pAltSegment =
				static_cast<T_MIME_ALTERNATIVE_SEGMENT *>(vSegments.GetAt(k));

			if (pAltSegment->m_fText)
				fnString_Cat (bod, pAltSegment->m_pData, pAltSegment->m_iLen);
			else
			{
				T_MIME_HEADER_AREA & sArea = pAltSegment->m_sArea;

				if (myFindNoCase (sArea.type, "text/"))
				{
					// do nothing.  I choose NOT to 2nd guess the sender
					//    don't decode text
				}
				else
				{
					Render_Entity (subjline_filename,
						&sArea.type, &sArea.description, &sArea.encoding,
						&sArea.disposition,
						&sArea.contentId,
						pAltSegment->m_pData,
						0,                         // start
						pAltSegment->m_iLen,       // end
						eRender,
						bod,
						fFromCache_NoSKIP);
				}
			}
		}
	}
	else  // !decode
	{
		// stop after we see text/plain

		bool fSeenText = false;
		for (k = 0; k < vSegments.GetSize(); k++)
		{
			pAltSegment =
				static_cast<T_MIME_ALTERNATIVE_SEGMENT *>(vSegments.GetAt(k));

			if (pAltSegment->m_fText)
			{
				fnString_Cat (bod, pAltSegment->m_pData, pAltSegment->m_iLen);
				fSeenText = true;
			}
			else if (myFindNoCase (pAltSegment->m_sArea.type, "html"))
			{
				// suppress!!
			}
			else if (myFindNoCase (pAltSegment->m_sArea.type, "text/"))
			{
				T_MIME_HEADER_AREA & sArea = pAltSegment->m_sArea;

				if (0 == _tcsnicmp (sArea.type, "text/plain", 10))
					fSeenText = true;

				if (0 == sArea.encoding.CompareNoCase ("quoted-printable"))
				{
					// translate qp stuff.
					TCodeMgr aDecoder;
					CString strQP;

					int iStrmLen = pAltSegment->m_iLen;
					istrstream in_strm(pAltSegment->m_pData, iStrmLen);

					// output to our stream, really our CString
					ULONG uOutLen = 0;
					BOOL  fIOError = FALSE;

					LPTSTR pBuf = strQP.GetBuffer (iStrmLen);
					ostrstream  out_strm(pBuf,iStrmLen);

					// translate qp stuff into the stream
					aDecoder.Decode_QuotedPrintable_to_binary (
						in_strm, iStrmLen, out_strm, &uOutLen, &fIOError );

					strQP.ReleaseBuffer (uOutLen);
					bod += strQP;
				}
				else  // not QP
				{
					// all other cases - just copy the text
					fnString_Cat (bod, pAltSegment->m_pData, pAltSegment->m_iLen);
				}
			}
			if (fSeenText && gpGlobalOptions->GetRegSwitch()->GetSkipHTMLPart())
				break;
		} // for loop
	} // end !decode

	// Phase 3 : cleanup

	for (k = 0; k < vSegments.GetSize(); k++)
	{
		T_MIME_ALTERNATIVE_SEGMENT * pAltSegment =
			static_cast<T_MIME_ALTERNATIVE_SEGMENT *>(vSegments.GetAt(k));

		delete pAltSegment;
	}
} // Render_multipart_alternative

// ------------------------------------------------------------------------
//
void TMimeThread::Add(TMimeSequence& seq)
{
	int n = m_set.GetSize();
	for (int i = 0; i < n; ++i)
	{
		TMimeSequence& aSeq = m_set.ElementAt(i);
		if (seq.m_part == aSeq.m_part)
			return;
		if (seq.m_part < aSeq.m_part)
		{
			m_set.InsertAt(i, seq);
			return;
		}
	}
	m_set.Add(seq); // add to end
}

BOOL TMimeThread::IsComplete(void)
{
	// check if we completed anything
	int iTOT = 0;
	int n = m_set.GetSize();
	for (int i = 0; i < n; ++i)
	{
		TMimeSequence& seq = m_set.ElementAt(i);
		if (seq.m_total > iTOT)
			iTOT  = seq.m_total;
	}
	if (iTOT == n)
		return TRUE;
	else
		return FALSE;
}

BOOL TMimeThread::SuperAppend(CString& result)
{
	TMimeSequence& seq1 = m_set.ElementAt(0);
	int n = m_set.GetSize();
	for (int i = 1; i < n; ++i)
	{
		TMimeSequence& seqN = m_set.ElementAt(i);
		file_append (seq1.m_fname, seqN.m_fname,
			TRUE  /* fDelete */);
	}
	result = seq1.m_fname;
	return TRUE;
}

TMimeThread::~TMimeThread()
{
	if (m_fFileOwnership)
	{
		int total = m_set.GetSize();
		for (int i = 0; i < total; ++i)
		{
			CFileStatus fs;
			TMimeSequence& seq = m_set.ElementAt(i);
			if (CFile::GetStatus(seq.m_fname, fs))
				CFile::Remove(seq.m_fname);
		}
	}
}

TMimeThread& TMimeThread::operator= (const TMimeThread& rhs)
{
	if (this == &rhs)
		return *this;
	m_fFileOwnership = rhs.m_fFileOwnership;
	m_set.RemoveAll();
	int total = rhs.m_set.GetSize();
	for (int i = 0; i < total; ++i)
	{
		TMimeSequence seq;
		seq = rhs.m_set[i];
		m_set.Add (seq);
		//m_set.InsertAt(0, rhs.m_set);
	}
	return *this;
}

//*************************************************************************
// TArticleText - Class for holding extended attributes for an
//                       article.
//*************************************************************************
TArticleText::TArticleText ()
{
	m_fArticle=TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// TArticleText Destructor - Destroys all extended attributes.
/////////////////////////////////////////////////////////////////////////////

TArticleText::~TArticleText ()
{
}

/////////////////////////////////////////////////////////////////////////////
// Assignment operator
/////////////////////////////////////////////////////////////////////////////

TArticleText &
TArticleText::operator=(const TArticleText & rhs)
{
	if (this == &rhs)
		return *this;

	TPersist822Text::operator=(rhs);

	m_fBody     =  rhs.m_fBody;
	m_fHeader   =  rhs.m_fHeader;
	m_Header    =  rhs.m_Header;
	m_PostTo    =  rhs.m_PostTo;
	return *this;
}

// --------------------------------------------------------------------------
// SizeHint -- returns an estimate of the disk space we need. define a virtual
DWORD TArticleText::SizeHint ()
{
	return TPersist822Text::SizeHint() +
		sizeof (m_fBody) +
		sizeof (m_fHeader) +
		m_PostTo.GetLength();
}

/////////////////////////////////////////////////////////////////////////////
// Serialize - Function to serialize a TArticleText object.
/////////////////////////////////////////////////////////////////////////////

void TArticleText::Serialize (CArchive & archive)
{
	// call base class function first
	TPersist822Text::Serialize( archive );

	BYTE byObjectVersion = GetObjectVersion ();

	if ( archive.IsStoring() )
	{
		archive << m_fBody;
		archive << m_fHeader;

		// base class already writes out m_Header

		archive << m_PostTo;           // Newsgroups or Followup-To
	}
	else
	{
		archive >> m_fBody;
		archive >> m_fHeader;

		// versions 1 & 2 both wrote this out. it was a bug - the
		//   base class owns m_Header

		if (byObjectVersion < 3)
			archive >> m_Header;

		archive >> m_PostTo;
	}
}

static BOOL fnIsBlankLine (LPCTSTR ptr)
{
	while (*ptr && *ptr != '\r' && *ptr != '\n')
	{
		if (!isspace(*ptr))
			return FALSE;
		ptr++;
	}
	return TRUE;
}

static void fnExcludePath (CString& fileSpec)
{
	CString rv = fileSpec;
	rv.MakeReverse ();

	// this is to handle any included path - lop it off.
	fileSpec = rv.SpanExcluding ("<>:\"/\\|");
	fileSpec.MakeReverse ();
}

/////////////////////////////////////////////////////////////////////////
// if k7bit just write it out
//
void  TPersist822Text::Decode_known_type (
	const CString& subjline_filename,
	int   type,                                // kUU, kBASE64 etc..
	const T_MIME_HEADER_AREA * pArea,
	LPCTSTR data,
	int  dataLen,
	bool fFromCache_NoSKIP)
{
	TPath decodeSpec;

	// find a workable filename
	Decode_section_fname (pArea, subjline_filename, decodeSpec);

	CString           decodeDir = m_decodeDir;
	TCodeEvent        sEvent;
	TYP_GETOUTPUTFILE sMode;

	TFileUtil::EDisposition eDisp =
		TFileUtil::CheckFileExist (decodeSpec,
		CWnd::FromHandle(ghwndMainFrame),
		decodeDir,
		IDS_SAVE_DECODED_AS,  // title StrID
		IDS_FILTER_DECODE,    // filter StrID
		&sMode,
		fFromCache_NoSKIP);
	sEvent.m_file = decodeSpec;

	if (TFileUtil::kFileCancel == eDisp)
	{
		if (gpfnCodeEventMonitor)
		{
			sEvent.m_kEventType = TCodeEvent::kFileSkipped;
			gpfnCodeEventMonitor ( &sEvent );
		}
		return;   // cancelled
	}

	ULONG uResultLen = 0;
	BOOL fIOError = FALSE;
	BOOL fRet = fnDecode_known_type ( decodeSpec, type, (TCHAR*)data, dataLen,
		&uResultLen, &fIOError );

	if (fIOError && gpfnCodeEventMonitor)
	{
		sEvent.m_kEventType = TCodeEvent::kIOError;
		gpfnCodeEventMonitor ( &sEvent );
	}
	if (fRet && gpfnCodeEventMonitor)
	{
		sEvent.m_kEventType = TCodeEvent::kFileFinished;
		gpfnCodeEventMonitor ( &sEvent );
	}
}

// ------------------------------------------------------------------------
// Returns 0 if a mime section has a filename, 1 if taken from subject
int  TPersist822Text::Decode_section_fname (
	const T_MIME_HEADER_AREA * pArea,
	const CString & subjline_filename,
	TPath& output_spec)
{
	CString dosName;
	int      idx, nlen;
	int      iResult = 0;
	CString  lowcase;

	lowcase = pArea->disposition;
	lowcase.MakeLower();

	idx = lowcase.Find("filename=");
	if (-1 != idx)
	{
		// look for Filename in the Content-Disposition field

		dosName = pArea->disposition.Mid(idx + 9);
		nlen = dosName.GetLength();
		if (dosName[0] == '"' && dosName[nlen-1] == '"')
			dosName = dosName.Mid(1, nlen-2);

		fnExcludePath (dosName);

		// Try adding appropriate file extension
		Decode_section_fext (pArea, dosName);

		dosName = TPath::GenLegalFileName (dosName);
	}

	if (dosName.IsEmpty())
	{
		// look for Filename in the Content-Type field
		lowcase = pArea->type;  lowcase.MakeLower();

		idx = lowcase.Find("name=");
		if (-1 != idx)
		{
			dosName=(pArea->type.Mid(idx + 5)).SpanExcluding(";");
			nlen = dosName.GetLength();
			if (dosName[0] == '"' && dosName[nlen-1] == '"')
				dosName = dosName.Mid(1, nlen-2);
			fnExcludePath (dosName);
			dosName = TPath::GenLegalFileName (dosName);
		}
	}
	if (dosName.IsEmpty())
	{
		// look for Filename in the Content-Description field

		dosName = pArea->description.Mid(0, 100);
		// Disallow   <>:"/\|
		dosName = dosName.SpanExcluding("<>:\"/\\|");
		dosName.TrimLeft();
		dosName.TrimRight();
		dosName = TPath::GenLegalFileName (dosName);
	}

	if (dosName.IsEmpty() && !pArea->contentId.IsEmpty())
	{
		// use content-id field
		dosName = pArea->contentId;
		dosName.TrimLeft();
		dosName.TrimRight();
		fnExcludePath (dosName);

		if (!dosName.IsEmpty())
		{
			// don't add file extenstion to nothing. ".JPG" would circumvent
			//  phase #6

			// Try adding appropriate file extension
			Decode_section_fext (pArea, dosName);

			// final sweep of illegal chars
			dosName = TPath::GenLegalFileName (dosName);
		}
	}

	// Phase #6
	if (dosName.IsEmpty()  &&  !subjline_filename.IsEmpty() )
	{
		CString strInput = subjline_filename;

		// Disallow   <>:"/\|
		strInput = strInput.SpanExcluding("<>:\"/\\|");
		strInput.TrimLeft();
		strInput.TrimRight();

		if (!strInput.IsEmpty())
		{
			// Try adding appropriate file extension
			Decode_section_fext (pArea, strInput);

			// use filename from subject line
			dosName = TPath::GenLegalFileName (strInput);

			iResult = 1;
		}
	}

	// OK now attach the decode-path to the Name

	// RenderBody_Mime sets this
	CString decodeDir = m_decodeDir;
	ASSERT(!decodeDir.IsEmpty());

	if (dosName.IsEmpty())
	{
		// random name
		make_arbitrary_filename(decodeDir, dosName);

		// Try adding appropriate file extension
		Decode_section_fext (pArea, dosName);

		output_spec = dosName;
	}
	else
	{
		output_spec.FormPath(decodeDir, dosName);
	}
	return iResult;
}

//-------------------------------------------------------------------------
void TPersist822Text::Decode_section_fext (const T_MIME_HEADER_AREA * pArea,
										   CString& output_spec)
{
	// hurm. search lower case copies of the strings
	CString strType = pArea->type;  strType.MakeLower();
	CString spec2 = output_spec;    spec2.MakeLower();

	if (strType.Find("image/jpeg") != -1)
	{
		if (-1 == spec2.Find(".jpeg") && -1 == spec2.Find(".jpg"))
			output_spec += ".jpeg";
	}
	else if (strType.Find("image/jpg") != -1)
	{
		if (-1 == spec2.Find(".jpeg") && -1 == spec2.Find(".jpg"))
			output_spec += ".jpg";
	}
	else if (strType.Find("image/gif") != -1)
	{
		if (-1 == spec2.Find(".gif"))
			output_spec += ".gif";
	}
	else if (strType.Find("image/png") != -1)
	{
		if (-1 == spec2.Find(".png"))
			output_spec += ".png";
	}
	else if (strType.Find("text/plain") != -1)
	{
		if (-1 == spec2.Find(".txt"))
			output_spec += ".txt";
	}
	else if (strType.Find("text/html") != -1)
	{
		if (-1 == spec2.Find(".htm"))
			output_spec += ".htm";
	}
}

// ------------------------------------------------------------------------
// kBASE64 case used to goto a fancy Memory-Mapped-File function, but
//    now it goes to a conventional function
//
static BOOL fnDecode_known_type(
								const CString& decodeSpec,
								int            type,
								TCHAR *        data,
								int            dataLen,
								ULONG *        puResultLen,
								BOOL *         pfIOError)
{
	BOOL fRet = FALSE;

	ofstream  out_stream(decodeSpec, ios::out | ios::binary);
	if (!out_stream.is_open())
	{
		*pfIOError = TRUE;
		CString msg; AfxFormatString1(msg, IDS_ERR_FILEOPEN1, (LPCTSTR) decodeSpec);
		NewsMessageBoxTimeout (20,CWnd::FromHandle(ghwndMainFrame), msg, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	istrstream in_stream(data, dataLen);   // input stream (from a string)

	TCodeMgr aCodeMgr;

	aCodeMgr.InstallEventMonitor ( gpfnCodeEventMonitor );

	switch (type)
	{
	case kBASE64:
		try
		{
			fRet = aCodeMgr.Decode_base64_to_binary (in_stream, dataLen, out_stream,
				puResultLen);
		}
		catch(...)
		{
			fRet       = FALSE;
			*pfIOError = TRUE;
		}
		break;

	case kQP:
		fRet = aCodeMgr.Decode_QuotedPrintable_to_binary(in_stream, dataLen, out_stream,
			puResultLen, pfIOError);
		break;
	case kUU:
		fRet = aCodeMgr.Decode_UU_to_binary(in_stream, dataLen, out_stream,
			puResultLen, pfIOError);
		break;
	case kXX:
		fRet = aCodeMgr.Decode_XX_to_binary(in_stream, dataLen, out_stream,
			puResultLen, pfIOError);
		break;

	case kYenc:
		fRet = aCodeMgr.Decode_YEnc_to_binary(in_stream, dataLen, out_stream,
			puResultLen, pfIOError);
		break;

	case k7bit:
		{
			// copy the file as is.
			char c;
			while (in_stream.get(c))
				out_stream.put(c);

			fRet = (out_stream) ? TRUE : FALSE;
		}
		break;

	default:
		ASSERT(0);
		break;
	}

	out_stream.close ();
	return fRet;
}

///////////////////////////////////////////////////////////////////////////
//  If we used XOVER, we didn't get the Xrefs and it's not part of the
//    header.  Fill in the Header object from the Full header string.
//
void TArticleText::TransferXRef (TArticleHeader * pArtHdr)
{
	// if there is nothing there now
	if (0 == _tcslen(pArtHdr->GetXRef()))
	{
		int idx;
		// Try to get XRefs from the article text
		CString &strHeader = m_Header;

		// note - token starts a line, it should/must be a header field

		if ((idx = strHeader.Find("\r\nXref:")) != -1)
		{
			int iLen = strHeader.GetLength();
			LPTSTR pBuf = strHeader.GetBuffer(iLen);
			LPTSTR pStart = pBuf + idx + 7;

			while (*pStart && isspace(*pStart))
				++pStart;

			LPTSTR pEnd = pStart;

			// find the end of this line
			while (*pEnd && (*pEnd != '\r') && (*pEnd != '\n'))
				++pEnd;

			{
				// plop in a temporary null
				TPunchOut punch(pEnd, 0);

				// starts with hostname
				pArtHdr->SetXRef ( pStart );
			}
			strHeader.ReleaseBuffer(iLen);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//
BOOL TArticleText::FollowupSame()
{
	CString followupTo;
	BOOL fPresent = GetFollowup ( followupTo );
	if (!fPresent)
		return TRUE;

	CString ngs;
	GetNewsgroups ( ngs );

	TStringList listF, listN;
	listF.FillByCommaList ( followupTo );

	listN.FillByCommaList ( ngs );

	int count = listF.GetCount();
	if (count != listN.GetCount())
		return FALSE;

	// do full blown comparison
	TSortedPtrArray<CPtrArray, CString*> vF;
	TSortedPtrArray<CPtrArray, CString*> vN;

	CString *pStrF, *pStrN;

	// add to array
	while (!listF.IsEmpty())
	{
		pStrF = new CString(listF.RemoveHead());
		vF.Add( pStrF );

		pStrN = new CString(listN.RemoveHead());
		vN.Add( pStrN );
	}

	// sort alphabetically
	vF.SortItems();
	vN.SortItems();

	// compare one-by-one
	int i;
	BOOL fSame = TRUE;
	count = vF.GetSize();
	for (i = 0; i < count; ++i)
	{
		pStrF = vF[i];
		pStrN = vN[i];
		if (*pStrF != *pStrN)
		{
			fSame = FALSE;
			break;
		}
	}

	for (i = 0; i < count; ++i)
	{
		delete vF[i];
		delete vN[i];
	}

	return fSame;
}

///////////////////////////////////////////////////////////////////////////
//
//
BOOL TArticleText::ReplyToSame(CString& address)
{
	CString replyTo;
	BOOL fPresent = GetReplyTo ( replyTo );
	BOOL fSame = FALSE;

	if (!fPresent)
		return TRUE;

	int len = replyTo.GetLength();
	LPTSTR pBuf = replyTo.GetBuffer(len);

	TCHAR rcDefHost[] = "mplanet";
	MAIL_ADDRESS* pAddr = rfc822_parse_address(&pBuf, rcDefHost);
	if (0==pAddr)
		;
	else
	{
		if (pAddr->fUsedDefaultHost)
			fSame = FALSE;
		else if (0==pAddr->mailbox || 0==pAddr->host)
			fSame = FALSE;
		else
		{
			CString final; final.Format("%s@%s", pAddr->mailbox, pAddr->host);
			if (final.CompareNoCase( address ) == 0)
				fSame = TRUE;
		}
	}
	if (pAddr)
		FreeAddressList (pAddr);

	replyTo.ReleaseBuffer(len);
	return fSame;
}

