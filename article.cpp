/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: article.cpp,v $
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
/*  Revision 1.7  2008/09/19 14:51:10  richard_wood
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

///////////////////////////////////////////////////////////////////////////
//  article.cpp - The outer object relays requests to the
//                inner object. This module is mostly fluff.
//
#include "stdafx.h"
#include "afxmt.h"
#include "article.h"
#include "utilstr.h"
#include "codepg.h"
#include "tglobopt.h"
#include "rgcomp.h"
#include "8859x.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// live and die at global scope
TMemShack TArticleHeader::m_gsMemShack(sizeof(TArticleHeader), "Hdr");
static CCriticalSection sArtCritical;

extern ULONG String_base64_decode (LPTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz);
extern ULONG String_QP_decode (LPCTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz);

int magic_isohdr_translate (LPCTSTR text, CString & strOut);

// Used for RFC 2047 decoding in headers
class  TAnsiCodePage
{
public:
	TAnsiCodePage()
	{
		// Old Wisdom
		// ISO-8859-1  ==  LATIN 1  ==  CP 1252
		// ISO-8859-2  ==  LATIN 2  ==  CP 1250
	}

	// this is used strictly for SUBJECT and FROM lines
	BOOL CanTranslate(LPCTSTR pszText, int & iType, GravCharset * & rpCharset)
	{
		int unusedLen = 0;
		int ret = this->FindISOString ( pszText, unusedLen, rpCharset );
		if (0==ret)
			return FALSE;
		else
		{
			iType = ret;
			return TRUE;
		}
	}

	// ===========================================================
	// return 1 for QP, 2 for B64, 0 for 'can not handle it'
	//
	// update ln - offset to start of good data (after 8859-15?Q?)
	//
	// example:
	//    =?iso-8859-1?q?this=20is=20some=20text?=
	BOOL FindISOString (LPCTSTR cpIn, int & ln, GravCharset * & rpCharset)
	{
		CString strcp = cpIn;
		strcp.MakeLower();
		LPCTSTR cp = strcp;
		LPTSTR pRes = 0;

		pRes = (LPSTR)strstr (cp, "=?");
		if (NULL == pRes)
			return FALSE;
		if (NULL == *(pRes+2))
			return FALSE;
		else
		{
			CString charsetName;
			LPTSTR pTravel = pRes + 2;
			int n=0;
			LPTSTR pTxt = charsetName.GetBuffer(strcp.GetLength());
			while (*pTravel && ('?' != *pTravel))
			{
				*pTxt++ = *pTravel++;
				n++;
			}
			charsetName.ReleaseBuffer(n);

			// use the map for a fast lookup
			rpCharset = gsCharMaster.findByName( charsetName );
			if (NULL == rpCharset)
				return FALSE;

			if (NULL == *pTravel)  // pTravel should point to the ?q?  or  ?b?
				return FALSE;
			ASSERT('?' == *pTravel);

			++pTravel;
			TCHAR cEncoding = *pTravel;
			int ret = 0;
			if (NULL == cEncoding)
				return 0;

			if ('q' == cEncoding)
				ret = 1;
			else if ('b' == cEncoding)
				ret = 2;
			else
				return ret;

			++pTravel;  // point to 2nd ?
			if ((NULL == *pTravel) || ('?' != *pTravel))
				return 0;

			++pTravel; // point to start of data

			ln = pTravel - cp;
			return ret;
		}
	}
};

static  TAnsiCodePage gsCodePage;

///////////////////////////////////////////////////////////////////////////
// TPersist822Header
//
#if defined(_DEBUG)
void TPersist822Header::Dump(CDumpContext& dc) const
{
	TBase822Header::Dump( dc );
	m_pRep->Dump (dc);
	dc << "TPersist822Hdr\n" ;
}
#endif

///////////////////////////////////////////////////////////////////////////
// Destructor
TPersist822Header::TPersist822Header()
{
	m_pRep = 0;
	// the derived classes must instantiate the Inner representation
}

///////////////////////////////////////////////////////////////////////////
// copy-constructor
TPersist822Header::TPersist822Header(const TPersist822Header& src)
{
	m_pRep = src.m_pRep;
	m_pRep->AddRef ();
}

///////////////////////////////////////////////////////////////////////////
// Destructor
TPersist822Header::~TPersist822Header()
{
	try
	{
		ASSERT(m_pRep);

		m_pRep->DeleteRef ();
	}
	catch(...)
	{
		// catch everything - no exceptions can leave a destructor
	}
}

void TPersist822Header::copy_on_write()
{
	if (m_pRep->iGetRefCount() == 1)
		return;

	// call virt function to make the right kind of object
	TPersist822HeaderInner * pCpyInner = m_pRep->duplicate();

	TPersist822HeaderInner * pOriginal = m_pRep;

	m_pRep = pCpyInner;         // I now have a fresh copy all to myself

	pOriginal->DeleteRef();     // you have 1 less client
}

///////////////////////////////////////////////////////////////////////////
// NOTE: ask Al before using this function
//
//  5-01-96  amc  Created
void TPersist822Header::PrestoChangeo_AddReferenceCount()
{
	ASSERT(m_pRep);
	m_pRep->AddRef ();   // you have 1 more client
}

void TPersist822Header::PrestoChangeo_DelReferenceCount()
{
	ASSERT(m_pRep);
	m_pRep->DeleteRef ();   // you have 1 less client
}

void TPersist822Header::SetNumber(LONG n)
{
	copy_on_write();
	m_pRep->SetNumber(n);
}

void TPersist822Header::SetLines (int lines)
{
	copy_on_write();
	m_pRep->SetLines ( lines );
}

void TPersist822Header::SetLines (const CString& body)
{
	copy_on_write();
	m_pRep->SetLines ( body );
}

void TPersist822Header::StampCurrentTime()
{
	copy_on_write();
	m_pRep->StampCurrentTime();
}

void TPersist822Header::SetMimeLines(LPCTSTR ver, LPCTSTR type, LPCTSTR encode,
									 LPCTSTR desc)
{
	copy_on_write();

	// the inner function is virtual
	m_pRep->SetMimeLines (ver, type, encode, desc);
}

void TPersist822Header::GetMimeLines(CString* pVer, CString* pType,
									 CString* pCode, CString* pDesc)
{
	// the inner function is virtual
	m_pRep->GetMimeLines (pVer, pType, pCode, pDesc);
}

void TPersist822Header::Serialize(CArchive & archive)
{
	// the inner function is virtual
	m_pRep->Serialize ( archive );
}

TPersist822Header & TPersist822Header::operator=(const TPersist822Header &rhs)
{
	if (&rhs == this)
		return *this;

	m_pRep->DeleteRef();  // he has one less client

	m_pRep = rhs.m_pRep;  // copy ptr

	m_pRep->AddRef ();    // you have 1 more client

	return *this;
}

///// end TPersist822Header ///////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// TArticleHeader
//
TArticleHeader::TArticleHeader()
{
	// set object version
	m_pRep = new TArticleHeaderInner(TArticleHeaderInner::kVersion);
}

// Destructor
TArticleHeader::~TArticleHeader() { /* empty */ }

TArticleHeader& TArticleHeader::operator=(const TArticleHeader &rhs)
{
	if (&rhs == this) return *this;

	TPersist822Header::operator=(rhs);

	return *this;
}

///////////////////////////////////////////////////////////////////////////
ULONG TArticleHeader::ShrinkMemPool()
{
	if (true)
	{
		CSingleLock sLock(&sArtCritical, TRUE);

		ULONG u1 = TArticleHeader::m_gsMemShack.Shrink ();
	}

	ULONG u2 = TArticleHeaderInner::ShrinkMemPool();

	return u2;
}

#if defined(_DEBUG)
void TArticleHeader::Dump(CDumpContext& dc) const
{
	// base class version
	TPersist822Header::Dump(dc);
	dc << "ArtHdr\n" ;
}
#endif

///// member functions
void TArticleHeader::FormatExt(const CString& control, CString& strLine)
{
	GetPA()->FormatExt (control, strLine);
}

void TArticleHeader::SetArticleNumber (LONG artInt)
{
	copy_on_write();
	m_pRep->SetNumber(artInt);
}

void TArticleHeader::SetDate(LPCTSTR date_line)
{
	copy_on_write();
	GetPA()->SetDate ( date_line );
}

LPCTSTR TArticleHeader::GetDate(void)
{
	return GetPA()->GetDate ();
}

void TArticleHeader::AddNewsGroup (LPCTSTR ngroup)
{
	copy_on_write();
	GetPA()->AddNewsGroup ( ngroup );
}

void TArticleHeader::GetNewsGroups (CStringList* pList) const
{
	GetPA()->GetNewsGroups ( pList );
}

int TArticleHeader::GetNumOfNewsGroups () const
{
	return GetPA()->GetNumOfNewsGroups ();
}

void TArticleHeader::ClearNewsGroups ()
{
	copy_on_write();
	GetPA()->ClearNewsGroups ();
}

BOOL TArticleHeader::operator<= (const TArticleHeader & rhs)
{
	return (*GetPA()) <= ( *(rhs.GetPA()) );
}

void TArticleHeader::SetControl (LPCTSTR control)
{
	copy_on_write();
	GetPA()->SetControl ( control );
}

LPCTSTR TArticleHeader::GetControl ()
{
	return GetPA()->GetControl ();
}

void    TArticleHeader::SetDistribution(LPCTSTR dist)
{
	copy_on_write();
	GetPA()->SetDistribution (dist);
}

LPCTSTR TArticleHeader::GetDistribution()
{
	return GetPA()->GetDistribution();
}

void TArticleHeader::SetExpires(LPCTSTR dist)
{
	copy_on_write();
	GetPA()->SetExpires (dist);
}

LPCTSTR TArticleHeader::GetExpires()
{
	return GetPA()->GetExpires();
}

// user sets what groups to Followup-To
void TArticleHeader::SetFollowup(LPCTSTR follow)
{
	copy_on_write();
	GetPA()->SetFollowup(follow);
}

LPCTSTR TArticleHeader::GetFollowup(void) const
{
	return GetPA()->GetFollowup();
}

void TArticleHeader::SetOrganization(LPCTSTR org)
{
	copy_on_write();
	GetPA()->SetOrganization(org);
}

LPCTSTR TArticleHeader::GetOrganization()
{
	return GetPA()->GetOrganization();
}

void TArticleHeader::SetKeywords(LPCTSTR keywords)
{
	copy_on_write();
	GetPA()->SetKeywords(keywords);
}

LPCTSTR TArticleHeader::GetKeywords(void)
{
	return GetPA()->GetKeywords();
}

void     TArticleHeader::SetSender(LPCTSTR sndr)
{
	copy_on_write();
	GetPA()->SetSender(sndr);
}

LPCTSTR  TArticleHeader::GetSender(void)
{
	return GetPA()->GetSender();
}

void    TArticleHeader::SetReplyTo(LPCTSTR replyto)
{
	copy_on_write();
	GetPA()->SetReplyTo(replyto);
}

LPCTSTR TArticleHeader::GetReplyTo()
{
	return GetPA()->GetReplyTo();
}

void    TArticleHeader::SetSummary(LPCTSTR sum)
{
	copy_on_write();
	GetPA()->SetSummary (sum);
}

LPCTSTR TArticleHeader::GetSummary(void)
{
	return GetPA()->GetSummary();
}

void TArticleHeader::SetXRef(LPCTSTR xref)
{
	copy_on_write();
	GetPA()->SetXRef(xref);
}

LPCTSTR TArticleHeader::GetXRef(void)
{ return GetPA()->GetXRef(); }

// ------------------------------------------------------------------
//
//
void TArticleHeader::SetQPFrom (const CString & from)
{
	copy_on_write();

	// put it in
	GetPA()->SetFrom (from);
}

void TArticleHeader::SetFrom (const CString & from)
{
	GetPA()->SetFrom (from);
}

CString TArticleHeader::GetFrom ()
{
	return GetPA()->GetFrom ();
}

const CString & TArticleHeader::GetOrigFrom(void)
{
	return GetPA()->GetOrigFrom ();
}

void TArticleHeader::SetOrigFrom(const CString & f)
{
	GetPA()->SetOrigFrom(f);
}

const CString & TArticleHeader::GetPhrase(void)
{
	return GetPA()->GetPhrase();
}

void TArticleHeader::SetMessageID (LPCTSTR msgid)
{
	copy_on_write();
	GetPA()->SetMessageID(msgid);
}

LPCTSTR TArticleHeader::GetMessageID(void)
{ return GetPA()->GetMessageID(); }

int handle_2hex_digits (LPCTSTR cp, TCHAR & cOut)
{
	BYTE e;

	TCHAR c, c2;
	if ((NULL == (c = cp[1])) || (NULL == (c2 = cp[2])))
		return 1;

	if (!isxdigit (c))   /* must be hex! */
		return 1;

	if (isdigit (c))
		e = c - '0';
	else
		e = c - (isupper (c) ? 'A' - 10 : 'a' - 10);

	c = c2;  /* snarf next character */

	if (!isxdigit (c))   /* must be hex! */
		return 1;

	if (isdigit (c))
		c -= '0';
	else
		c -= (isupper (c) ? 'A' - 10 : 'a' - 10);

	cOut = TCHAR( BYTE( (e << 4) + c) ); /* merge the two hex digits */

	return 0;
}

// ----------------------------------------------------------
// handle 2047 translation
//
struct T2047Segment
{
	T2047Segment(LPCTSTR txt, bool fWS0)
		: fWS(fWS0), text(txt)
	{
		iEncodingType = 0;
		len = 0;
	}
	bool     fWS;
	CString  text;
	int      iEncodingType;
	int      len;
};

typedef T2047Segment*  P2047Segment;

inline bool is_lwsp(TCHAR c)
{
	if (' ' == c || '\t' == c)
		return true;
	else
		return false;
}

LPCTSTR read_ws_token (LPCTSTR pTrav, LPTSTR pToken)
{
	while (*pTrav && is_lwsp(*pTrav))
		*pToken++ = *pTrav++;
	*pToken = 0;
	return pTrav;
}

LPCTSTR read_black_token (LPCTSTR pTrav, LPTSTR pToken)
{
	while (*pTrav && !is_lwsp(*pTrav))
		*pToken++ = *pTrav++;
	*pToken = 0;
	return pTrav;
}

// ----------------------------------------------------------
int  magic_hdr2047_segmentize (
							   LPCSTR                                    subject,
							   CTypedPtrArray<CPtrArray, P2047Segment> & listSegments,
							   LPTSTR                                    rcToken)
{
	LPCTSTR pTrav = subject;

	while (*pTrav)
	{
		if (is_lwsp(*pTrav))
		{
			pTrav = read_ws_token (pTrav, rcToken);
			listSegments.Add ( new T2047Segment(rcToken, true) );
		}
		else
		{
			pTrav = read_black_token (pTrav, rcToken);
			listSegments.Add ( new T2047Segment(rcToken, false) );
		}
	}
	return 0;
}

// ----------------------------------------------------------
//  Example 1:
//  Subject: Hobbits v Sm=?ISO-8859-1?B?6Q==?=agol
//  This is incorrectly encoded
//
//  Subject: =?Windows-1252?Q?Re:_Hobbits_v_Sm=E9agol?=
//
//  this is better, since the encoded word is 1 ATOM (see rfc2047 section 5)
//
// HOWEVER, to be generous, I handle the top case now
//
int  segment_2047_translate (T2047Segment * pS1, LPTSTR rcToken)
{
	LPCTSTR cp = pS1->text;
	TCHAR c;
	int ret = 0;
	std::istrstream in_stream(const_cast<LPTSTR>(cp));
	std::ostrstream out_stream(rcToken, 4096 * 4);

	std::vector<BYTE> vEncodedBytes;
	bool insideEncodedWord = false;
	int questionCount;
	while (in_stream.get(c))
	{
		if (!insideEncodedWord)
		{
			if ('=' == c && '?' == TCHAR(in_stream.peek()))
			{
				insideEncodedWord = true;
				questionCount = 0;
				ret = 1;  // set return to error
			}
			else
			{
				out_stream.put( c );
			}
		}
		else
		{
			if (questionCount < 3)
			{
				if (c == '?')
				{
					questionCount++;
				}
			}
			else
			{
				if (c == '?' && TCHAR(in_stream.peek()) == '=')
				{
					in_stream.get(c); //eat =
					TCHAR rcDecoded[4024];
					ULONG uLen;  vEncodedBytes.push_back(0);
					LPTSTR psz=reinterpret_cast<LPTSTR>(&vEncodedBytes[0]);

					if (2 == pS1->iEncodingType)
						uLen = String_base64_decode (psz, vEncodedBytes.size()-1, rcDecoded, sizeof(rcDecoded));
					else
						uLen = String_QP_decode (psz, vEncodedBytes.size()-1, rcDecoded, sizeof(rcDecoded));
					for (int n = 0; n < uLen; ++n)
						out_stream.put (rcDecoded[n]);
					insideEncodedWord = false;
					vEncodedBytes.clear();
					ret = 0;
				}
				else
				{
					vEncodedBytes.push_back((1 == pS1->iEncodingType && '_' == c) ? ' ' : c);
				}
			}
		} // end insideEW
	} // while

	// final cleanup
	if (0 == ret)
	{
		out_stream.put(0);
		pS1->text = rcToken;
	}
	return ret;
}

// ----------------------------------------------------------
int  magic_hdr2047_translate (LPCSTR subject, CString & strOut)
{
	TCHAR      rcToken[4096 * 4];
	CTypedPtrArray<CPtrArray, P2047Segment> listSegments;
	int        iStat=0;

	magic_hdr2047_segmentize (subject, listSegments, rcToken);

	int sz = listSegments.GetSize();
	int i;
	for (i = 0; i < sz; i++)
	{
		T2047Segment * pSeg  = listSegments[i];
		GravCharset * pCharsetDummy = 0;
		int encType = gsCodePage.FindISOString (pSeg->text, pSeg->len, pCharsetDummy);
		pSeg->iEncodingType = encType;
	}

	// drop LWSP segments in between encoded words
	for (i = 0; i < listSegments.GetSize(); i++)
	{
		T2047Segment* pS1 = listSegments[i];
		if ((i+2) < listSegments.GetSize())
		{
			T2047Segment* pS2 = listSegments[i+1];
			T2047Segment* pS3 = listSegments[i+2];

			if (pS1->iEncodingType && !pS1->text.IsEmpty() &&
				pS2->fWS && pS3->iEncodingType && !pS3->text.IsEmpty())
			{
				delete pS2;
				listSegments.RemoveAt (i+1);
			}
		}
	}

	// per segment translate
	for (i = 0; i < listSegments.GetSize(); i++)
	{
		T2047Segment* pS1 = listSegments[i];

		if (pS1->iEncodingType)
		{
			iStat = segment_2047_translate (pS1, rcToken);
			if (iStat)
				break;
		}
	}

	// cleanup and build final string
	for (i = 0; i < listSegments.GetSize(); i++)
	{
		T2047Segment* pS1 = listSegments[i];
		if (0 == iStat)
			strOut += pS1->text;

		delete pS1;
	}

	return iStat;
}

// ----------------------------------------------------------
int  magic_isohdr_translate (LPCSTR subject, CString & strOut)
{
	// do QP translation here
	// ex:
	// =?iso-8859-1?q?this=20is=20some=20text?=

	int iType = 0;
	GravCharset * pCharset = 0;

	if (FALSE == gsCodePage.CanTranslate(subject, iType, pCharset))
	{
		// this may be untagged eight bit
		if ( using_hibit(subject) )
		{
			int iSendCharset = gpGlobalOptions->GetRegCompose()->GetSendCharset();
			GravCharset* pCharsetUser = gsCharMaster.findById( iSendCharset );

			return CP_Util_Inbound (pCharsetUser, subject, lstrlen(subject), strOut);
		}
		else
			return 1;
	}

	CString strBytes;
	int stat = magic_hdr2047_translate (subject,  strBytes);

	if (stat)
		return stat;

	// go from bytes to chars
	return CP_Util_Inbound ( pCharset, strBytes, strBytes.GetLength(), strOut);
}

// ----------------------------------------------------------
void TArticleHeader::SetQPSubject(LPCTSTR subject, LPCTSTR pszFrom)
{
	copy_on_write();

	CString strOut;

	// do QP translation here

	if (0 == magic_isohdr_translate (subject, strOut))
		GetPA()->SetSubject(strOut);
	else
		GetPA()->SetSubject(subject);
}

void TArticleHeader::SetSubject(LPCTSTR subject)
{
	copy_on_write();
	GetPA()->SetSubject(subject);
}
LPCTSTR TArticleHeader::GetSubject (void)
{ return GetPA()->GetSubject(); }

void TArticleHeader::GetDestList(TStringList* pList) const
{ GetPA()->GetDestList (pList); }

void TArticleHeader::Format (CString& strLine)
{ GetPA()->Format(strLine); }

int  TArticleHeader::ParseFrom (CString& phrase, CString& address)
{
	return GetPA()->ParseFrom(phrase, address);
}

void TArticleHeader::SetReferences (const CString& refs)
{
	copy_on_write();
	GetPA()->SetReferences (refs);
}

void TArticleHeader::SetCustomHeaders (const CStringList &sCustomHeaders)
{
	copy_on_write();
	GetPA()->SetCustomHeaders (sCustomHeaders);
}

const CStringList &TArticleHeader::GetCustomHeaders () const
{ return GetPA()->GetCustomHeaders(); }

BOOL TArticleHeader::AtRoot(void)
{ return GetPA()->AtRoot(); }

BOOL TArticleHeader::FindInReferences(const CString& msgID)
{ return GetPA()->FindInReferences(msgID); }

int  TArticleHeader::GetReferencesCount()
{ return GetPA()->GetReferencesCount(); }

void TArticleHeader::CopyReferences(const TArticleHeader & src)
{
	copy_on_write();
	GetPA()->CopyReferences( *(src.GetPA()) );
}

void TArticleHeader::ConstructReferences(const TArticleHeader & src,
										 const CString& msgid)
{ GetPA()->ConstructReferences(*(src.GetPA()), msgid); }

void TArticleHeader::GetDadRef(CString& str)
{ GetPA()->GetDadRef (str); }

void TArticleHeader::GetFirstRef(CString& str)
{ GetPA()->GetFirstRef(str); }

void TArticleHeader::GetReferencesWSList(CString& line)
{ GetPA()->GetReferencesWSList(line); }

void TArticleHeader::GetReferencesStringList(CStringList* pList)
{ GetPA()->GetReferencesStringList(pList); }

const TFlatStringArray &TArticleHeader::GetReferences ()
{ return GetPA()->GetReferences(); }

// end of TArticle Implementation
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// TEmailHeader
//
TEmailHeader::TEmailHeader()
{
	m_pRep = new TEmailHeaderInner(TEmailHeaderInner::kVersion); // object version is 1
}

// Empty destructor
TEmailHeader::~TEmailHeader() { /* empty */ }

TEmailHeader& TEmailHeader::operator=(const TEmailHeader &rhs)
{
	if (&rhs == this) return *this;

	TPersist822Header::operator=(rhs);             // do normal stuff

	return *this;
}

void TEmailHeader::Set_InReplyTo (LPCTSTR irt)
{
	copy_on_write();
	GetPE()->Set_InReplyTo(irt);
}

LPCTSTR TEmailHeader::Get_InReplyTo ()
{ return GetPE()->Get_InReplyTo(); }

int TEmailHeader::GetDestinationCount(TBase822Header::EAddrType eAddr)
{ return GetPE()->GetDestinationCount(eAddr); }

void TEmailHeader::ParseTo(const CString & to, TStringList * pOutList /* = 0 */,
						   CStringList * pErrList /* = 0 */)
{
	if (0 == pOutList)
		copy_on_write();

	GetPE()->ParseTo (to, pOutList, pErrList);
}

void TEmailHeader::GetToText(CString& txt)
{ GetPE()->GetToText (txt); }

// CC - store the comma separated list of addresses
void TEmailHeader::ParseCC(const CString& comma_sep_CC_list,
						   TStringList * pOutList /* = 0 */,
						   CStringList * pErrList /* = 0 */)
{
	if (0 == pOutList)
		copy_on_write();
	GetPE()->ParseCC (comma_sep_CC_list, pOutList, pErrList);
}

// CC - get the displayable text, separated by commas
void TEmailHeader::GetCCText(CString& txt)
{ GetPE()->GetCCText ( txt ); }

// BCC - store the comma separated list of addresses
void TEmailHeader::ParseBCC(const CString& comma_sep_BCC_list,
							TStringList * pOutList /* = 0 */,
							CStringList * pErrList /* = 0 */)
{
	if (0 == pOutList)
		copy_on_write();
	GetPE()->ParseBCC ( comma_sep_BCC_list, pOutList, pErrList );
}

// BCC - get the displayable text, separated by commas
void TEmailHeader::GetBCCText(CString& txt)
{ GetPE()->GetBCCText ( txt ); }

void TEmailHeader::GetDestinationList (TBase822Header::EAddrType eAddr,
									   TStringList * pstrList) const
{ GetPE()->GetDestinationList(eAddr, pstrList); }

// end of file
