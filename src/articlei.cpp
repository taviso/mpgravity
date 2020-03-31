/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: articlei.cpp,v $
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
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.8  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.7  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.6  2008/09/19 14:51:11  richard_wood
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
#include <winnls.h>     // GetDateFormat, DATE_SHORTDATE
#include <time.h>
#include "article.h"
#include "pobject.h"
#include "mplib.h"
#include "timeutil.h"   // GetLocalTime
#include "afxmt.h"
#include "ctype.h"
#include "mailadr.h"
#include "resource.h"   // IDS_*
#include "genutil.h"    // CopyCStringList()
#include "attinfo.h"    // TAttachmentInfo
#include "superstl.h"   // istrstream
#include "artdata.h"    // TArtUIData
#include "critsect.h"   // TEnterCSection

#include "superstl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------
typedef set<CString, less<CString> > STLMapString2;

STLMapString2  gsFromZoo;

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

IMPLEMENT_SERIAL(TBase822HeaderInner,    PObject,                1);

TCHAR TBlobMaster::rcEmptyString[2];

CRITICAL_SECTION gcritMempool;

// this is here to handle the critical section at startup/shutdown
class TCritSectionKeeper {
public:
	TCritSectionKeeper(LPCRITICAL_SECTION pCS)
	{
		m_pMyCritical = pCS;
		InitializeCriticalSection (pCS);
	}

	~TCritSectionKeeper()
	{
		DeleteCriticalSection (m_pMyCritical);
	}
private:
	LPCRITICAL_SECTION m_pMyCritical;
};

// this is here to handle the critical section at startup/shutdown
TCritSectionKeeper  gsTheKeeper(&gcritMempool);

// live and die at global scope
TMemShack TArticleHeaderInner::m_gsMemShack(sizeof(TArticleHeaderInner), "Inner");

///////////////////////////////////////////////////////////////////////////
ULONG TArticleHeaderInner::ShrinkMemPool()
{
	TEnterCSection csAuto(&gcritMempool);

	return TArticleHeaderInner::m_gsMemShack.Shrink ();
}

// ------------------------------------------------------------------------
void fnZooLookup ( const CString & key, CString & out )
{
	TEnterCSection csAuto(&gcritMempool);

	STLMapString2::iterator it = gsFromZoo.find ( key );

	if (it != gsFromZoo.end())
	{
		// forcibly help the CString ref counting

		//  helps immensely with .MP3 groups, since the Author is repeated so
		//    much

		out = *it;
	}
	else
	{
		out = key;
		gsFromZoo.insert (out);
	}
}

// ------------------------------------------------------------------------
void fnZooFlush ()
{
	TEnterCSection csAuto(&gcritMempool);

	gsFromZoo.erase ( gsFromZoo.begin(), gsFromZoo.end() );
}

// ------------------------------------------------------------------------
// articlei_nomem_handler -- throws a TException, with a description and
//    a unique number indicating the callers position
static void articlei_nomem_handler (BOOL fMalloc, int iBytes, int iCodePos)
{
	CString strMsg;
	CString str; str.LoadString (IDS_ERR_CANT_ALLOCATE);
	strMsg.Format(str,
		(fMalloc) ? "malloc" : "realloc",
		iBytes,
		iCodePos);
	throw(new TException(strMsg, kFatal));
}

#if defined(_DEBUG)
void TBase822HeaderInner::Dump(CDumpContext& dc) const
{
	// base class version
	PObject::Dump(dc);
	dc << "Base822HdrInner\t";
}
#endif

// assignment
TBase822HeaderInner & TBase822HeaderInner::operator= (const TBase822HeaderInner & rhs)
{
	if (this == &rhs)
		return *this;

	PObject::operator=(rhs);

	return *this;
}

// copy-constructor
TBase822HeaderInner::TBase822HeaderInner (const TBase822HeaderInner & src)
: PObject(src)
{
}

void TBase822HeaderInner::Serialize (CArchive & archive)
{
	// call base class function first
	PObject::Serialize( archive );

	if ( archive.IsStoring() )
	{

	}
	else
	{

	}
}

void TBase822HeaderInner::AssertValid(void)
{
	//CObject::AssertValid();
}

////////////////////////////////////////////////////////////////////////
// TPersist822HeaderInner
////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
void TPersist822HeaderInner::Dump(CDumpContext& dc) const
{
	// base class version
	TBase822HeaderInner::Dump(dc);
	dc << "Persist822 " <<  m_fArticle;
}
#endif

void TPersist822HeaderInner::AddRef ()
{
	TEnterCSection csAuto(&gcritMempool);

	++m_iRefCount;
}

void TPersist822HeaderInner::DeleteRef ()
{
	TEnterCSection csAuto(&gcritMempool);

	ASSERT(m_iRefCount > 0);

	--m_iRefCount;

	if (m_iRefCount <= 0)
		delete this;
}

LONG TPersist822HeaderInner::iGetRefCount()
{
	return m_iRefCount;
}

TPersist822HeaderInner::TPersist822HeaderInner(BYTE objectVersion)
: TBase822HeaderInner(objectVersion)
{
	m_iRefCount = 1;
	m_Lines = 0;
	m_UniqueNumber = -1;
	m_iNumAttachments = 0;
	m_psAttachments = NULL;
}

///////////////////////////////////////////////////////////////////////////
// Copy-constructor
//
TPersist822HeaderInner::TPersist822HeaderInner (const TPersist822HeaderInner & src)
: TBase822HeaderInner(src), m_time(src.m_time)
{
	m_Lines        = src.m_Lines;
	m_fArticle     = src.m_fArticle;
	m_UniqueNumber = src.m_UniqueNumber;

	// copy attachment info
	m_iNumAttachments = src.m_iNumAttachments;
	m_psAttachments = NULL;
	if (m_iNumAttachments) {
		m_psAttachments = new TAttachmentInfo [m_iNumAttachments];
		for (int i = 0; i < m_iNumAttachments; i++)
			m_psAttachments [i] = src.m_psAttachments [i];
	}

	m_iRefCount = 1;
}

///////////////////////////////////////////////////////////////////////////
TPersist822HeaderInner::~TPersist822HeaderInner()
{
	if (m_psAttachments)
		delete [] m_psAttachments;
}

///////////////////////////////////////////////////////////////////////////
TPersist822HeaderInner & TPersist822HeaderInner::operator= (const TPersist822HeaderInner & rhs)
{
	if (this == &rhs)
		return *this;

	TBase822HeaderInner::operator=(rhs);

	m_Lines        = rhs.m_Lines;
	m_time         = rhs.m_time;
	m_fArticle     = rhs.m_fArticle;
	m_UniqueNumber = rhs.m_UniqueNumber;

	delete [] m_psAttachments;
	m_psAttachments = 0;

	// copy attachment info
	m_iNumAttachments = rhs.m_iNumAttachments;
	if (m_iNumAttachments)
	{
		m_psAttachments = new TAttachmentInfo [m_iNumAttachments];
		for (int i = 0; i < m_iNumAttachments; i++)
			m_psAttachments [i] = rhs.m_psAttachments [i];
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// designed to be overridden
TPersist822HeaderInner*  TPersist822HeaderInner::duplicate()
{
	ASSERT(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////
void TPersist822HeaderInner::Serialize (CArchive & archive)
{
	TBase822HeaderInner::Serialize (archive);

	if ( archive.IsStoring() )
	{
		archive << m_Lines;
		archive << m_time;
		archive << m_fArticle;
		archive << m_UniqueNumber;
		archive << m_iNumAttachments;
		for (int i = 0; i < m_iNumAttachments; i++)
			m_psAttachments [i].Serialize (archive);
	}
	else
	{
		archive >> m_Lines;
		archive >> m_time;
		archive >> m_fArticle;
		archive >> m_UniqueNumber;

		// version 3 of both TArticleHeader and TEmailHeader
		if (GetObjectVersion () >= 3)
		{
			archive >> m_iNumAttachments;

			if (m_psAttachments)
				delete [] m_psAttachments;

			if (0 == m_iNumAttachments)
				m_psAttachments = NULL;
			else
			{
				m_psAttachments = new TAttachmentInfo [m_iNumAttachments];
				for (int i = 0; i < m_iNumAttachments; i++)
					m_psAttachments[i].Serialize (archive);
			}
		}
	}
}

void  TPersist822HeaderInner::SetLines (const CString& body)
{
	int iLFcount = 0;
	LPCTSTR p = body;
	while (*p)
	{
		if (*p++ == '\n')
			iLFcount++;
	}
	if ('\n' != *(p-1))
		iLFcount++;
	SetLines (iLFcount);
}

/////////////////////////////////////////////////////////////////////////////
//
// TArticleHeaderInner - Article object that is the heart of our news program.
//
/////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
void TArticleHeaderInner::Dump(CDumpContext& dc) const
{
	// base class version
	TPersist822HeaderInner::Dump(dc);
	dc << "ArtHdrInner\n" ;
}
#endif

extern int magic_isohdr_translate (LPCTSTR text, CString & strOut);  // from article.cpp

// --------------------------------------------------------------------------
// Return 1 if we calculated the indices, aka we are dirty
int  TArticleHeaderInner::ParseFrom(CString& phrase, CString& address)
{
	LPCTSTR pFrom = GetOrigFrom();
	if (m_fAddrAvailable)
	{
		phrase = GetPhrase();

		LPTSTR buf = address.GetBuffer(m_addrCnt);
		if (buf)
		{
			CopyMemory(buf, pFrom + m_addrIdx1, m_addrCnt*sizeof(TCHAR));
			address.ReleaseBuffer(m_addrCnt);
		}

		return 0;
	}
	else
	{
		// parse once..
		if (m_strFrom.IsEmpty())
			return 0;

		parse_from (phrase, address);
		m_fAddrAvailable = TRUE;

		ASSERT(address.IsEmpty() == FALSE);

		// .. save indices for future reference.
		LPTSTR pAt = (LPSTR)_tcsstr(pFrom, address);
		if (pAt)
		{
			m_addrIdx1 = WORD((pAt - pFrom) / sizeof(TCHAR));
			ASSERT(m_addrIdx1 < 100);

			m_addrCnt  = WORD(address.GetLength());
		}
		else
		{
			// cases like:
			// < alchoy @ yahoo . com >   do not parse well, since they are illegal
			int idxOpen = m_strFrom.ReverseFind('<');
			int idxClose = m_strFrom.ReverseFind('>');

			// this is terribly unscientific, but grab whatever exists between < >
			if ((-1 != idxOpen) && (-1 != idxClose) && (idxClose > idxOpen + 1))
			{
				m_addrIdx1 = idxOpen + 1;
				m_addrCnt  = idxClose - idxOpen - 1;
			}
			else
			{
				// something is weird
				m_addrIdx1 = 0;
				m_addrCnt = WORD(_tcslen(pFrom));
			}
		}

		if (phrase.IsEmpty())
		{
		}
		else
		{
			CString xtmp = phrase;
			phrase.Empty();
			if (magic_isohdr_translate (xtmp, phrase))
			{
				SetPhrase(xtmp);
				phrase=xtmp;
			}
			else
				SetPhrase(phrase);
		}
		return 1;
	}
} // ParseFrom

// ----------------------------------------------------------------------------
void sfnParse_from (const CString & fromIn, CString & phrase, CString & address)
{
	CString from = fromIn;

	// new stuff 5-16-96
	int len = from.GetLength();
	LPTSTR pBuf = from.GetBuffer(len);
	TCHAR rcDefaultHost[] = "mplanet";

	// heh heh from Mark Crispin
	MAIL_ADDRESS* pAddr = rfc822_parse_address( &pBuf, rcDefaultHost );

	if (!pAddr || pAddr->fUsedDefaultHost ||
		(0==pAddr->personal && 0==pAddr->mailbox))
	{
		// what can we do?
		address = from;
	}
	else
	{
		if (pAddr->personal)
			phrase  = (LPCTSTR) pAddr->personal;
		if (pAddr->mailbox && pAddr->host)
			address.Format("%s@%s", pAddr->mailbox, pAddr->host);
		else
			ASSERT(0);
	}
	if (pAddr)
		FreeAddressList ( pAddr );
	from.ReleaseBuffer();
}

//////////////////////////////////////////////////////////////////////////
// used by GenerateSaysLine.  This is NOT meant to be a bulletproof func.
//   - a yacc parser is not robust
//   - Crispin's work is too complex
//   - WinVN uses this method
// note: does not handle multiple mailboxes.
//       does not handle quoted-strings      "aha <tricky> xx"
void
TArticleHeaderInner::parse_from (CString& phrase, CString& address)
{
	CString from = m_strFrom;

	ASSERT(from.IsEmpty() == FALSE);

	sfnParse_from (from, phrase, address);
}

int TEmailHeader::ParseFrom (CString& phrase, CString& address)
{
	CString frum = GetFrom();
	sfnParse_from (frum, phrase, address);
	return 0;
}

// -------------------------------------------------------------------------------------------------
// decode rfc2047 encoding for the Reply-To: line
int TPersist822HeaderInner::DecodeElectronicAddress(const CString & rawAddress, CString & prettyAddress)
{
	CString phraseIn, address, phraseFinal;

	sfnParse_from (rawAddress, phraseIn, address);

	if (0 == magic_isohdr_translate (phraseIn, phraseFinal))
	{
		// we have phraseFinal & address
		if (phraseFinal.IsEmpty())
		{
			prettyAddress = address;
		}
		else
		{
			prettyAddress.Format("%s <%s>", LPCTSTR(phraseFinal), LPCTSTR(address));
		}
	}
	else
	{
		// give up
		prettyAddress = rawAddress;
	}
	return 0;
}

void TArticleHeaderInner::SetAtGrow(TBlobMaster::EFlatField eIdx, LPCTSTR strInput)
{
	m_sBlob.SetAtGrow (eIdx, strInput, m_fAddrAvailable);
}

LPCTSTR TArticleHeaderInner::GetBlobString(TBlobMaster::EFlatField eIdx) const
{
	return m_sBlob.GetBlobString (eIdx);
}

// -----------------------------------------------------------------------
void TArticleHeaderInner::SetFrom (const CString & from)
{
	SetOrigFrom (from);
}

// -----------------------------------------------------------------------
CString TArticleHeaderInner::GetFrom()
{
	CString tmp(_T("")), fullName, addr;

	ParseFrom(fullName, addr);

	if (addr.IsEmpty())
		return tmp;

	if (fullName.IsEmpty())
	{
		tmp = "<" + addr + ">";
	}
	else
	{
		// If required, quote fullName
		static char cQuote[] = "()<>@,;:\\.[]";
		if (fullName.FindOneOf(cQuote) != -1)
		{
			fullName.Trim('"');
			fullName = "\"" + fullName + "\"";
		}
		tmp = fullName + " <" + addr + ">";
	}
	return tmp;
}

const CString & TArticleHeaderInner::GetOrigFrom()
{
	return m_strFrom;
}

void TArticleHeaderInner::SetOrigFrom(const CString & f)
{
	m_strFrom = f;

	m_fAddrAvailable = FALSE;
	m_addrIdx1 = 0;
	m_addrCnt = 0;

	CString emptee;
	SetPhrase(emptee);
}

void TArticleHeaderInner::SetPhrase(const CString & p)
{
	m_vRandomStrings.SetAtGrow (3, p);
}

const CString & TArticleHeaderInner::GetPhrase()
{
	static CString strEmpty;

	if (m_vRandomStrings.GetSize() >= 4)
		return m_vRandomStrings[3];
	else
		return strEmpty;
}

void TArticleHeaderInner::SetMessageID(LPCTSTR msgid)
{
	SetAtGrow( TBlobMaster::kMsgID, msgid);
}

LPCTSTR TArticleHeaderInner::GetMessageID(void)
{
	return GetBlobString( TBlobMaster::kMsgID);
}

void TArticleHeaderInner::SetSubject(LPCTSTR subj)
{
	SetAtGrow(TBlobMaster::kSubject, subj);
}

LPCTSTR TArticleHeaderInner::GetSubject(void)
{
	return GetBlobString(TBlobMaster::kSubject);
}

void TArticleHeaderInner::GetDestList(TStringList* pList) const
{
	GetNewsGroups ( pList );      // a public func
}

void TArticleHeaderInner::SetCustomHeaders (const CStringList &sCustomHeaders)
{
	if (sCustomHeaders.IsEmpty())
	{
		delete m_psCustomHeaders;
		m_psCustomHeaders = NULL;
		return;
	}

	if (NULL == m_psCustomHeaders)
		m_psCustomHeaders = new CStringList;

	CopyCStringList (*m_psCustomHeaders, sCustomHeaders);
}

const CStringList &TArticleHeaderInner::GetCustomHeaders () const
{
	static CStringList m_gsEmptyCustomHdrs;

	if (m_psCustomHeaders)
		return *m_psCustomHeaders;

	return m_gsEmptyCustomHdrs;
}

void TArticleHeaderInner::SetKeywords(LPCTSTR keywords)
{
	SetAtGrow(TBlobMaster::kKeywords, keywords);
}

LPCTSTR TArticleHeaderInner::GetKeywords(void)
{
	return GetBlobString(TBlobMaster::kKeywords );
}

void TArticleHeaderInner::SetSender(LPCTSTR sender)
{
	SetAtGrow(TBlobMaster::kSender, sender);
}

LPCTSTR TArticleHeaderInner::GetSender(void)
{
	return GetBlobString(TBlobMaster::kSender);
}

void TArticleHeaderInner::SetReplyTo(LPCTSTR replyto)
{
	SetAtGrow(TBlobMaster::kReplyTo, replyto);
}

LPCTSTR TArticleHeaderInner::GetReplyTo(void)
{
	return GetBlobString(TBlobMaster::kReplyTo);
}

void TArticleHeaderInner::SetDate(LPCTSTR date_line)
{
	SetAtGrow(TBlobMaster::kDate, date_line);

	BOOL fOK = ParseUsenetDate();

	if (!fOK)
	{
		// last resort
		CTime guessTime(1996, 1, 1, 0, 0, 1);
		m_time = guessTime;
	}

#if defined(_DEBUG)
	if (!fOK || fOK > 1)
	{
		char rcName[] = "C:\\badtime.txt";
		CFileStatus fs;
		CStdioFile f;

		if (CFile::GetStatus ( rcName, fs ))
			f.Open(rcName, CFile::modeReadWrite | CFile::typeText);
		else
			f.Open(rcName, CFile::modeCreate | CFile::modeReadWrite | CFile::typeText);

		CString str;
		if (!fOK)
			str.Format("%d - %s\n", GetNumber(), date_line);
		else
			str.Format("<ok> %d - %s\n", GetNumber(), date_line);

		f.SeekToEnd();
		f.Write(str, str.GetLength());
	}
#endif
}

LPCTSTR TArticleHeaderInner::GetDate(void)
{
	return GetBlobString(TBlobMaster::kDate);
}

void TArticleHeaderInner::SetDistribution(LPCTSTR dist)
{
	SetAtGrow(TBlobMaster::kDistribution, dist);
}

LPCTSTR  TArticleHeaderInner::GetDistribution(void)
{
	return GetBlobString(TBlobMaster::kDistribution);
}

void TArticleHeaderInner::SetExpires(LPCTSTR exp)
{
	SetAtGrow(TBlobMaster::kExpires, exp);
}

LPCTSTR  TArticleHeaderInner::GetExpires(void)
{
	return GetBlobString(TBlobMaster::kExpires);
}

void TArticleHeaderInner::SetOrganization(LPCTSTR org)
{
	SetAtGrow(TBlobMaster::kOrganization, org);
}

LPCTSTR  TArticleHeaderInner::GetOrganization(void)
{
	return GetBlobString(TBlobMaster::kOrganization);
}

// The line has: <GreatGranddad><Grandad><Dad>
void TArticleHeaderInner::SetReferences(const CString& inRefs)
{
	m_822References.SetReferences( inRefs );
}

void TArticleHeaderInner::SetSummary(LPCTSTR sum)
{
	SetAtGrow(TBlobMaster::kSummary, sum);
}

LPCTSTR TArticleHeaderInner::GetSummary(void)
{
	return GetBlobString(TBlobMaster::kSummary);
}

void    TArticleHeaderInner::SetControl (LPCTSTR control)
{
	SetAtGrow(TBlobMaster::kControl, control);
}

LPCTSTR TArticleHeaderInner::GetControl ()
{
	return GetBlobString(TBlobMaster::kControl);
}

void  TArticleHeaderInner::SetFollowup(LPCTSTR follow)
{
	SetAtGrow(TBlobMaster::kFollow, follow);
}

LPCTSTR TArticleHeaderInner::GetFollowup(void) const
{
	return GetBlobString(TBlobMaster::kFollow);
}

BOOL TArticleHeaderInner::AtRoot(void)
{
	return m_822References.GetCount() == 0;
}

BOOL TArticleHeaderInner::FindInReferences(const CString& msgID)
{
	return m_822References.Find(msgID);
}

int TArticleHeaderInner::GetReferencesCount()
{
	return m_822References.GetCount();
}

void  TArticleHeaderInner::CopyReferences(const TArticleHeaderInner& src)
{
	if (this != &src)
		m_822References.CopyReferences( src.m_822References );
}

void  TArticleHeaderInner::ConstructReferences(const TArticleHeaderInner& src,
											   const CString & msgid)
{
	if (this != &src)
		m_822References.ConstructReferences( src.m_822References, msgid );
}

void TArticleHeaderInner::GetDadRef(CString& oneRef)
{
	m_822References.GetLastRef(oneRef);
}

void TArticleHeaderInner::GetFirstRef(CString& oneRef)
{
	m_822References.GetFirstRef(oneRef);
}

void TArticleHeaderInner::GetReferencesWSList(CString& line)
{
	m_822References.MakeWhiteSpaceList(line);
}

void TArticleHeaderInner::GetReferencesStringList(CStringList* pList)
{
	m_822References.FillStringList(pList);
}

const TFlatStringArray &TArticleHeaderInner::GetReferences ()
{
	return m_822References;
}

void
TArticleHeaderInner::SetMimeLines(LPCTSTR ver, LPCTSTR type,
								  LPCTSTR encode, LPCTSTR desc)
{
	SetAtGrow(TBlobMaster::kMimeVer, ver);
	SetAtGrow(TBlobMaster::kMimeContentType, type);
	SetAtGrow(TBlobMaster::kMimeContentEncoding, encode);
	SetAtGrow(TBlobMaster::kMimeContentDesc, desc);
}

void
TArticleHeaderInner::GetMimeLines(CString* pVer, CString* pType,
								  CString* pCode, CString* pDesc)
{
	if (pVer)   *pVer  = GetBlobString(TBlobMaster::kMimeVer);
	if (pType)  *pType = GetBlobString(TBlobMaster::kMimeContentType);
	if (pCode)  *pCode = GetBlobString(TBlobMaster::kMimeContentEncoding);
	if (pDesc)  *pDesc = GetBlobString(TBlobMaster::kMimeContentDesc);
}

void TArticleHeaderInner::Format(CString& strLine)
{
	// empty
}

// ---------------------------------------------------------------------------
// MakeUIData -- structure is used in displaying thread pane. Return 1 if
//               parsing made arthdr dirty
int  TArticleHeaderInner::MakeUIData (TArtUIData *& rpUIData)
{
	// assume "fslkr"
	// from, subject, lines, time, score

	CString phrase;
	CString addr;
	int iDirty = ParseFrom(phrase, addr);

	CTime local;
	GetLocalTime (m_time, local);

	rpUIData =  new TArtUIData (phrase.IsEmpty() ? addr : phrase,
		GetBlobString(TBlobMaster::kSubject),
		GetLines(),
		local.GetTime(),   // uSeconds in time_t
		GetScore());

	return iDirty;
}

// handle d - date
//        j - extended date:   12/95 03:30pm
//        k - raw date - pass back the time_t as a string
//        f - from
//        g - from without separator
//        l - lines
//        m - lines without separator
//        s - subject   result will be separated by ASCII 1
void TArticleHeaderInner::FormatExt (const CString& control, CString& strLine)
{
	try
	{
		if (control.IsEmpty())
		{
			ASSERT(0);
			return;
		}
		else
		{
			int i;
			for (i = 0; i < control.GetLength(); ++i)
			{
				switch (control[i])
				{
				case 'j':
					{
						if (!strLine.IsEmpty())
							strLine += TCHAR(1);
						// Article stores time as GMT. Get the local time
						CTime local;
						GetLocalTime (m_time, local);
						strLine += local.Format("%m/%d %I:%M %p");
					}
					break;

				case 'k':
					{
						if (!strLine.IsEmpty())
							strLine += TCHAR(1);
						// Article stores time as GMT. Get the local time
						CTime local;
						GetLocalTime (m_time, local);
						time_t uSeconds = local.GetTime();
						CString hx;
						hx.Format("%d", uSeconds);

						// pass back hex-string
						strLine += hx;
					}
					break;

				case 'f':   // From
				case 'g':
					{
						CString phrase;
						CString addr;
						ParseFrom(phrase, addr);
						if (!strLine.IsEmpty() && ('f' == control[i]))
							strLine += TCHAR(1);

						if (phrase.IsEmpty())
							strLine += addr;
						else
							strLine += phrase;
						//strLine += m_From;
						break;
					}
				case 'l':      // Lines
				case 'm':
					{
						if (!strLine.IsEmpty() && ('l' == control[i]))
							strLine += TCHAR(1);
						CString num = " ";
						num.Format("%d", GetLines());
						strLine += num;
					}
					break;
				case 's':
					if (!strLine.IsEmpty())
						strLine += TCHAR(1);
					strLine += GetBlobString(TBlobMaster::kSubject);
					break;
				case 'r':
					{
						if (!strLine.IsEmpty())
							strLine += TCHAR(1);
						CString num;
						num.Format("%d", GetScore());
						strLine += num;
					}
					break;
				case ' ':
				case '(':
				case ')':
				case '[':
				case ']':
					strLine += control[i];
					break;
				default:
					throw (new TException(IDS_ERR_UNKNOWN_ART_FORMAT, kFatal));
					break;
				}
			} // for
		}
	}
	catch(...)
	{
		strLine.Empty();
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Default Constructor - doesn't do much.
/////////////////////////////////////////////////////////////////////////////

TArticleHeaderInner::TArticleHeaderInner(BYTE objectVersion)
: TPersist822HeaderInner(objectVersion)
{
	//TRACE0(" TArticleHeaderInner Constructor called\n");
	common_construct();
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////

TArticleHeaderInner::~TArticleHeaderInner()
{
	try
	{
		delete m_psCustomHeaders;
		m_psCustomHeaders = NULL;
	}
	catch(...)
	{
		// catch everything - no exceptions can leave a destructor
	}
}

/////////////////////////////////////////////////////////////////////////////
// Assignment operator - Assign all of the parts.
/////////////////////////////////////////////////////////////////////////////

TArticleHeaderInner& TArticleHeaderInner::operator=(const TArticleHeaderInner& rhs)
{
	if (this == &rhs)
		return *this;

	TPersist822HeaderInner::operator=(rhs);

	m_phraseIdx1            = rhs.m_phraseIdx1;
	m_phraseCnt             = rhs.m_phraseCnt;
	m_addrIdx1              = rhs.m_addrIdx1;
	m_addrCnt               = rhs.m_addrCnt;
	m_fAddrAvailable        = rhs.m_fAddrAvailable;

	m_sBlob                 = rhs.m_sBlob;

	m_DestArray = rhs.m_DestArray;

	CopyReferences(rhs);

	delete m_psCustomHeaders;
	m_psCustomHeaders = NULL;

	if (rhs.m_psCustomHeaders)
	{
		m_psCustomHeaders = new CStringList;

		CopyCStringList (*m_psCustomHeaders, *(rhs.m_psCustomHeaders));
	}

	m_vRandomStrings.Copy (rhs.m_vRandomStrings);

	m_lScore       = rhs.m_lScore;
	m_lBodyScore   = rhs.m_lBodyScore;
	m_strFrom      = rhs.m_strFrom;

	m_lPileScore        = rhs.m_lPileScore;
	m_bPileZeroPresent  = rhs.m_bPileZeroPresent;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
// this defines a virtual function and creates the correct type (Article)
//
TPersist822HeaderInner* TArticleHeaderInner::duplicate()
{
	// call the copy-constructor
	return new TArticleHeaderInner(*this);
}

///////////////////////////////////////////////////////////////////////////
// copy-constructor
//
TArticleHeaderInner::TArticleHeaderInner(const TArticleHeaderInner & src)
: TPersist822HeaderInner(src),
m_sBlob (src.m_sBlob)
{
	m_phraseIdx1      = src.m_phraseIdx1;
	m_phraseCnt       = src.m_phraseCnt;
	m_addrIdx1        = src.m_addrIdx1;
	m_addrCnt         = src.m_addrCnt;
	m_fAddrAvailable = src.m_fAddrAvailable;

	m_DestArray = src.m_DestArray;

	CopyReferences(src);

	m_psCustomHeaders = NULL;
	if (src.m_psCustomHeaders)
	{
		m_psCustomHeaders =  new CStringList;

		CopyCStringList (*m_psCustomHeaders, *(src.m_psCustomHeaders));
	}

	m_vRandomStrings.Copy (src.m_vRandomStrings);

	m_lScore     = src.m_lScore;
	m_lBodyScore = src.m_lBodyScore;
	m_strFrom    = src.m_strFrom;

	m_lPileScore        = src.m_lPileScore;
	m_bPileZeroPresent  = src.m_bPileZeroPresent;
}

/////////////////////////////////////////////////////////////////////////////
// Serialize
/////////////////////////////////////////////////////////////////////////////

void TArticleHeaderInner::Serialize (CArchive & archive)
{
	// call base class function first
	TPersist822HeaderInner::Serialize( archive );

	if( archive.IsStoring() )
	{
		archive << m_phraseIdx1;
		archive << m_phraseCnt;
		archive << m_addrIdx1;
		archive << m_addrCnt;
		archive << m_fAddrAvailable;

		m_sBlob.Serialize ( archive );

		m_DestArray.Serialize ( archive );
		m_822References.Serialize ( archive );
		serialize_CustomHeaders ( archive );
		m_vRandomStrings.Serialize ( archive );

		archive << m_lScore;
		archive << m_lBodyScore;
		archive << m_strFrom;
	}
	else
	{
		archive >> m_phraseIdx1;
		archive >> m_phraseCnt;
		archive >> m_addrIdx1;
		archive >> m_addrCnt;
		archive >> m_fAddrAvailable;

		m_sBlob.Serialize ( archive );

		m_DestArray.Serialize ( archive );
		m_822References.Serialize ( archive );
		BYTE byObjectVersion = GetObjectVersion ();
		if (byObjectVersion > 1)
			serialize_CustomHeaders ( archive );
		if (byObjectVersion > 3)
			m_vRandomStrings.Serialize ( archive );  // introduced in v4
		if (byObjectVersion > 4)
			archive >> m_lScore;
		if (byObjectVersion > 5)
			archive >> m_lBodyScore;

		if (byObjectVersion <= 6)
		{
			CString tmpFrom = GetBlobString(TBlobMaster::kFrom);

			fnZooLookup ( tmpFrom, m_strFrom );

			m_sBlob.RemoveField (TBlobMaster::kFrom, m_fAddrAvailable);
		}

		if (byObjectVersion >= 7)
		{
			CString tmpFrom;
			archive >> tmpFrom;

			fnZooLookup ( tmpFrom, m_strFrom );
		}

		if (byObjectVersion == 8)
		{
			CString strPhrase;
			archive >> strPhrase;
			SetPhrase ( strPhrase ); // new in ver 9
		}
		if (byObjectVersion < 9)
		{
			m_fAddrAvailable = FALSE;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// don't create a CStringList unless we need it.   11-2-99 amc
//
void TArticleHeaderInner::serialize_CustomHeaders (CArchive & archive)
{
	if ( archive.IsStoring() )
	{
		if (m_psCustomHeaders)
		{
			m_psCustomHeaders->Serialize ( archive );
		}
		else
		{
			// dummy

			CStringList  sCustomHeaders;

			sCustomHeaders.Serialize ( archive );
		}
	}
	else
	{
		CStringList  sCustomHeaders;

		sCustomHeaders.Serialize ( archive );

		if (sCustomHeaders.GetCount() > 0)
		{
			delete  m_psCustomHeaders;

			m_psCustomHeaders =  new CStringList;

			CopyCStringList ( *m_psCustomHeaders,  sCustomHeaders );
		}
		else
		{
			delete m_psCustomHeaders;
			m_psCustomHeaders = NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//  protected function
//
//  5-13-96  amc  Handle "1/1/96" format
//  3-31-99  amc
//        Fri, 21 Nov 1997 23:08:34 -0600 (CST)
//        is OK. We ignore the parens.
BOOL TArticleHeaderInner::ParseUsenetDate()
{
	LPCTSTR lpszInput = GetBlobString(TBlobMaster::kDate);

	CString s;

	int len = lstrlen (lpszInput);
	int iParenLevel = 0;
	int nChars = 0;

	LPTSTR  pBuf = s.GetBuffer ( len + 1 );

	// ignore anything in parens

	for (int i = 0; i < len; i++)
	{
		TCHAR c = lpszInput[i];

		if (c == '('  )
			iParenLevel++;
		else if (c == ')'  )
			iParenLevel--;
		else if (0 == iParenLevel)
		{
			*pBuf++ = c;
			nChars ++;
		}
	}

	s.ReleaseBuffer ( nChars );
	s.TrimRight();

	return parse_date_helper ( s );
}

static int window_year (int yr)
{
	if (yr < 100)
	{
		if ((yr >= 0) && (yr <= 69))
			return yr + 2000;
		else
			return yr + 1900;
	}
	else
		return yr;
}

//
// RLW : Added month descriptions in different languages.
// Gravity now recognises 12 different language spellings for the month names.
//
// Also, if any of these don't match, the crash has been fixed.
//
/*
Language  	Lang. code  	01  	02  	03  	04  	05  	06  	07  	08  	09  	10  	11  	12
Croatian 	scr 			sij 	vel 	ožu 	tra 	svi 	lip 	srp 	kol 	ruj 	lis 	stu 	pro
Czech 		cze 			led 	ún	 	b?e 	dub 	kv? 	?er 	?er 	srp 	zá? 	?ij 	lis 	pro
Danish 		dan 			jan 	feb 	mar 	apr 	maj 	jun 	jul 	aug 	sep 	okt 	nov 	dec
Dutch 		dut 			jan 	feb 	maa 	apr 	mei 	jun 	jul 	aug 	sep 	oct 	nov 	dec
English 	eng 			Jan 	Feb 	Mar 	Apr 	May 	Jun 	Jul 	Aug 	Sep 	Oct 	Nov 	Dec
Estonian 	est 			jaa 	vee 	mär 	apr 	mai 	juu 	juu 	aug 	sep 	okt 	nov 	det
French 		fre 			jan 	fév 	mar 	avr 	mai 	jui 	jui 	aoû 	sep 	oct 	nov 	déc
German 		ger 			Jan		Feb 	Mär 	Apr 	Mai 	Jun 	Jul 	Aug 	Sep 	Okt 	Nov 	Dez
Greek, Modern 	gre 		Ian 	Fev 	Mar 	Apr 	Mai 	Iou 	Iou 	Aug 	Sep 	Okt 	Noe 	Dek
Hungarian 	hun 			jan 	feb 	már 	ápr 	máj 	jún 	júl  	aug 	sze 	okt 	nov 	dec
Indonesian 	ind 			Jan		Peb 	Mrt 	Apr 	Mei		Jun		Jul		Ag 		Sep 	Okt 	Nop 	Des
Italian 	ita 			gen 	feb 	mar 	apr 	mag 	giu 	lug 	ag	 	set 	ott 	nov 	dic
Latin 		lat 			Ian 	Feb 	Mar 	Apr 	Mai 	Iun 	Iul 	Aug 	Sep 	Oct 	Nov 	Dec
Latvian 	lav 			jan 	feb 	mar 	apr 	mai 	jun 	jul 	aug 	sep 	okt 	nov 	dec
Lithuanian 	lit 			sau 	vas 	kov 	bal 	geg 	bir 	lie 	rug 	rug 	spa 	lap 	gr
Malaysian 	may 			Jan 	Feb 	Mac 	Apr 	Mei 	Jun 	Jul 	Og	 	Sep 	Okt 	Nov 	Dis
Norwegian 	nor 			jan 	feb 	mar 	apr 	mai 	jun 	jul 	aug 	sep 	okt 	nov 	des
Polish 		pol 			sty 	lut		mar 	kwi 	maj 	cze 	lip 	sie 	wrz 	pa? 	lis 	gru
Portuguese 	por 			jan 	fev 	mar 	abr 	mai 	jun 	jul 	ago 	set 	out 	nov 	dez
Romanian 	rum 			Ian 	Feb 	Mar 	Apr 	Mai 	Iun 	Iul 	Aug 	Sep 	Oct 	Noe 	Dec
Serbian		scc 			jan 	feb 	mar 	apr 	maj 	jun 	jul 	aug 	sep 	okt 	nov 	dec
Slovak 		slo 			jan 	feb 	mar 	apr 	máj 	jún 	júl 	aug 	sep 	okt 	nov 	dec
Slovenian 	slv 			jan 	feb 	mar 	apr 	maj 	jun 	iul 	avg 	sep 	okt 	nov 	dec
Spanish 	spa 			ene		feb 	mar 	abr 	may 	jun 	jul 	ago 	sep		oct 	nov 	dic
Swedish 	swe 			jan 	feb 	mar 	apr 	maj 	jun 	jul 	aug 	sep 	okt 	nov 	dec
Ukrainian 	ukr 			sic 	liu? 	ber 	kvi 	tra 	che 	lyp 	ser 	ver 	z?h?o	lys 	hru
Welsh 		wel 			Ion 	Chw 	Maw 	Ebr 	Mai 	Meh 	Gor 	Aws 	Med 	Hyd 	Tac 	Rha


After removing duplicates:-

Language  	Lang. code  	01  	02  	03  	04  	05  	06  	07  	08  	09  	10  	11  	12
Croatian 	scr 			sij 	vel 	ozu 	tra 	svi 	lip 	srp 	kol 	ruj 	lis 	stu 	pro
Czech 		cze 			led 	un	 	bre 	dub 	kve 	cer 	?er 	srp 	zar 	?ij 	lis 	
Danish 		dan 			jan 	feb 	mar 	apr 	maj 	jun 	jul 	aug 	sep 	okt 	nov 	dec
Dutch 		dut 				 		 	maa 		 	mei 		 		 		 		 	oct 		 	
English 	eng 				 		 		 		 	May 		 		 		 		 		 		 	
Estonian 	est 			jaa 	vee 		 		 	mai 	juu 	juu 		 		 		 		 	det
French 		fre 				 	fev 		 	avr 		 	jui 	jui 	aou 		 		 		 	
German 		ger 						 		 		 		 		 		 		 		 		 		 	Dez
Greek, Modern 	gre 		Ian 		 		 		 		 	Iou 	Iou 		 		 		 	Noe 	Dek
Hungarian 	hun 				 		 		 		 		 		 		  		 	sze 		 		 	
Indonesian 	ind 					Peb 	Mrt 		 							Ag 			 		 	Nop 	Des
Italian 	ita 			gen 		 		 		 	mag 	giu 	lug 		 	set 	ott 		 	dic
Latin 		lat 				 		 		 		 		 	Iun 	Iul 		 		 		 		 	
Latvian 	lav 				 		 		 		 		 		 		 		 		 		 		 	
Lithuanian 	lit 			sau 	vas 	kov 	bal 	geg 	bir 	lie 	rug 	rug 	spa 	lap 	gr
Malaysian 	may 				 		 	Mac 		 		 		 		 	Og	 		 		 		 	Dis
Norwegian 	nor 				 		 		 		 		 		 		 		 		 		 		 	
Polish 		pol 			sty 	lut			 	kwi 		 	cze 	lip 	sie 	wrz 	pa? 		 	gru
Portuguese 	por 				 		 		 	abr 		 		 		 	ago 		 	out 		 	
Romanian 	rum 				 		 		 		 		 		 		 		 		 		 		 	
Serbian		scc 				 		 		 		 		 		 		 		 		 		 		 	
Slovak 		slo 				 		 		 		 		 		 		 		 		 		 		 	
Slovenian 	slv 				 		 		 		 		 		 		 	avg 		 		 		 	
Spanish 	spa 			ene			 		 		 		 		 		 		 				 		 	
Swedish 	swe 				 		 		 		 		 		 		 		 		 		 		 	
Ukrainian 	ukr 			sic 	liu? 	ber 	kvi 	tra 	che 	lyp 	ser 	ver 	z?h?o	lys 	hru
Welsh 		wel 			Ion 	Chw 	Maw 	Ebr 		 	Meh 	Gor 	Aws 	Med 	Hyd 	Tac 	Rha

Putting Eng first and compacting the rest

Language  	Lang. code  	01  	02  	03  	04  	05  	06  	07  	08  	09  	10  	11  	12
English 	eng 			Jan 	Feb 	Mar 	Apr 	May 	Jun 	Jul 	Aug 	Sep 	Oct 	Nov 	Dec

							sij 	vel 	ozu 	tra 	svi 	lip 	srp 	kol 	ruj 	lis 	stu 	pro
							led 	un	 	bre 	dub 	kve 	cer 	cer 	srp 	zar 	rij 	lis 	det
				 			jaa	 	vee	 	maa	 	avr	 	maj 	juu	 	juu	 	aou	 	sre	 	okt		noe		dez
							ian		fev		mrt 	bal	 	mei 	jui	 	jui	 	ag	 	set	 	ott		nop		dek
							gen	 	peb 	kov	 	kwi	 	mai 	iou 	iou 	rug	 	rug	 	spa	 	lap	 	des
							sau		vas 	mac	 	abr 	mag	 	giu 	lug 	og		wrz		paz		lys		dic
				 			sty		lut	 	ber	 	kvi	 	geg	 	iun	 	iul	 	sie	 	ver	 	out	 	tac	 	gr
					 		ene	 	liu 	maw	 	ebr	 	tra	 	bir 	lie 	ago	 	med	 	zho 		 	diz
				 			sic	 		 		 		 		 	cze	 	lip	  	avg	 	 		hyd				gru
				 			ion			 		 		 			che		lyp		ser			 		 		 	hru
				 				 		 		 		 		 	meh 	gor 	aws	 		 		 		 	rha

*/

BOOL TArticleHeaderInner::parse_date_helper (LPCTSTR lpszDate)
{
	char *cp, rcMon[80];
	int dom = 0, yr = 0, hr = 0, iMin = 0, sc = 0, iMonth = 0;
	static char fmtMonthTable01[37] = "janfebmaraprmayjunjulaugsepoctnovdec";
	// RLW : Added month descriptions in different languages.
	// Note that for some languages, if only the first three chars are compared
	// (as we do here) then quite a few months are the same as other months.
	static char fmtMonthTable02[37] = "sijvelozutrasvilipsrpkolrujlisstupro";
	static char fmtMonthTable03[37] = "ledun bredubkvecercersrpzarrijlisdet";
	static char fmtMonthTable04[37] = "jaaveemaaavrmajjuujuuaousreoktnoedez";
	static char fmtMonthTable05[37] = "ianfevmrtbalmeijuijuiag setottnopdek";
	static char fmtMonthTable06[37] = "genpebkovkwimaiiouiourugrugspalapdes";
	static char fmtMonthTable07[37] = "sauvasmacabrmaggiulugog wrzpazlysdic";
	static char fmtMonthTable08[37] = "stylutberkvigegiuniulsieverouttacgr ";
	static char fmtMonthTable09[37] = "eneliumawebrtrabirlieagomedzho   diz";
	static char fmtMonthTable10[37] = "sic            czelipavg   hyd   gru";
	static char fmtMonthTable11[37] = "ion            chelypser         hru";
	static char fmtMonthTable12[37] = "               mehgoraws         rha";

	BOOL fHandleTimeZone = FALSE;
	char *pZone = NULL;
	LPCTSTR s = GetBlobString(TBlobMaster::kDate);
	LPCTSTR postcomma;

	if (!s || !*s)
		return (0);
	if ((cp = strchr ((LPSTR)s, ',')) != 0)
		s = ++cp;
	while (isspace (*s))
		s++;
	postcomma = s;

	rcMon[0] = '\0';
	if (isdigit (*s))
	{
		char* pColon;
		char* pSpace;
		sscanf (s, "%d %s %d %d:%d:%d", &dom, rcMon, &yr, &hr, &iMin, &sc);
		if (yr < 100)  // egads a 2 digit year!
			yr = window_year (yr);

		pColon = strrchr((LPSTR)s, ':');
		pSpace = strrchr((LPSTR)s, ' ');

		if (pColon && pSpace && (pSpace == pColon + 3))
		{
			fHandleTimeZone = TRUE;
			pZone = ++pSpace;
		}
	}
	else
	{
		yr = -1;
		sscanf (s, "%*s %s %d %d:%d:%d %d", rcMon, &dom, &hr, &iMin, &sc, &yr);
		if (-1 == yr)
			yr = 0;
		else
		{
			if (yr < 100)  // egads a 2 digit year!
				yr = window_year (yr);
		}
	}

	BOOL fOK = TRUE;
	do {
		if (!dom || !yr || !*(cp = rcMon)) {
			fOK = FALSE;
			break;
		}
		if ((dom <= 0) || (dom >= 32)) {
			fOK = FALSE;
			break;
		}
		if ((yr < 1980) || (yr > 2069)) {
			fOK = FALSE;
			break;
		}
		if (strlen (rcMon) > 10) {
			fOK = FALSE;
			break;
		}
		if ((hr < 0) || (hr > 23)) {
			fOK = FALSE;
			break;
		}
		if ((iMin < 0) || (iMin > 59)) {
			fOK = FALSE;
			break;
		}
		if ((sc < 0) || (sc > 61)) {     // son-of-1036 says leap seconds happen
			fOK = FALSE;
			break;
		}
	} while (FALSE);

	if (!fOK)
	{
		// this is not a valid USENET date, but what the heck, Try it

		if (3 != sscanf (postcomma, "%d/%d/%d", &iMonth, &dom, &yr))
			return FALSE;

		if (yr < 100)
			yr = window_year (yr);

		if ((dom <= 0) || (dom >= 32))
			return FALSE;
		if ((iMonth <= 0) || (iMonth >= 13))
			return FALSE;
		if ((yr < 1980) || (yr > 2069))
			return FALSE;

		// seems somewhat ok
		CTime myGuessTime( yr, iMonth, dom, 0, 0, 1);

		// store the GMT time
		m_time = myGuessTime;
		return 10;
	}
	else
	{
		for (cp = rcMon; *cp; cp++)
			*cp = char(tolower (*cp));

		rcMon[3] = 0;
		// RLW : Added month descriptions in different languages.
		if ((cp = strstr (fmtMonthTable01, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable01) / 3 );
		else if ((cp = strstr (fmtMonthTable02, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable02) / 3 );
		else if ((cp = strstr (fmtMonthTable03, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable03) / 3 );
		else if ((cp = strstr (fmtMonthTable04, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable04) / 3 );
		else if ((cp = strstr (fmtMonthTable05, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable05) / 3 );
		else if ((cp = strstr (fmtMonthTable06, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable06) / 3 );
		else if ((cp = strstr (fmtMonthTable07, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable07) / 3 );
		else if ((cp = strstr (fmtMonthTable08, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable08) / 3 );
		else if ((cp = strstr (fmtMonthTable09, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable09) / 3 );
		else if ((cp = strstr (fmtMonthTable10, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable10) / 3 );
		else if ((cp = strstr (fmtMonthTable11, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable11) / 3 );
		else if ((cp = strstr (fmtMonthTable12, rcMon)) != 0)
			iMonth = 1 + ( (cp - fmtMonthTable12) / 3 );

		if (yr < 100)
			yr = window_year (yr);

		if ((yr < 1980) || (yr > 2069))
			return FALSE;
		if ((iMonth <= 0) || (iMonth >= 13))
			return FALSE;
		if ((dom <= 0) || (dom >= 32))
			return FALSE;

		/*  Setup a Ctime */

		CTime myTime( yr, iMonth, dom, hr, iMin, sc );

		if (fHandleTimeZone && (NULL != pZone))
		{
			CString strZone;

			// ignore anything in parens.
			int j;
			int nParenLevel = 0;
			int len = strlen (pZone);
			bool fAlpha = false;
			for (j = 0; j < len; j++)
			{
				if (  '(' == pZone[j])
					nParenLevel++;
				else if (  ')' == pZone[j])
					nParenLevel--;
				else if (0 == nParenLevel)
				{
					strZone += pZone[j];
					if (isalpha (pZone[j]))
						fAlpha = true;            // found a yucky Alphabetic Timezone
				}
			}

			if (fAlpha)
			{
				// timezone is specified with letters GMT, EST blah, blah, blah
				//   deal with this later
			}
			else
			{
				len = atoi(strZone);
				CTimeSpan span(0, abs(len)/100, abs(len)%100, 0);

				// EST 2pm is GMT 6pm
				// notation -0500
				if (len < 0)
					myTime += span;
				else
					myTime -= span;
			}
		}

		// store the GMT time
		m_time = myTime;
	}
	return TRUE;
}

BOOL TArticleHeaderInner::operator<= (const TArticleHeaderInner & rhs)
{
	return m_time <= rhs.m_time;
}

void TArticleHeaderInner::common_construct()
{
	m_fArticle = TRUE;

	m_phraseIdx1=0;
	m_phraseCnt=0;
	m_addrIdx1=0;
	m_addrCnt=0;
	m_fAddrAvailable = FALSE;    // tells us if the From line has been analyzed
	m_lScore = 0;
	m_lBodyScore = 0;

	m_psCustomHeaders = 0;

	m_lPileScore = 0;                // not serialized
	m_bPileZeroPresent = false;      // not serialized
}

void TArticleHeaderInner::SetXRef(LPCTSTR xref)
{
	LPCTSTR p = xref;

	// skip the host name
	while (*p && !isspace(*p))
		++p;

	// skip WS after host name
	while (*p && isspace(*p))
		++p;

	SetAtGrow(TBlobMaster::kCrossPost, xref);
}

LPCTSTR TArticleHeaderInner::GetXRef(void)
{
	return GetBlobString(TBlobMaster::kCrossPost);
}

void TArticleHeaderInner::AddNewsGroup(LPCTSTR group)
{
	m_DestArray.AddString (group);
}

void TArticleHeaderInner::GetNewsGroups(CStringList* pList) const
{
	m_DestArray.FillStringList (pList);
}

int TArticleHeaderInner::GetNumOfNewsGroups () const
{
	return m_DestArray.GetSize ();
}

void TArticleHeaderInner::ClearNewsGroups ()
{
	m_DestArray.RemoveAll ();
}

// --------------------------------------------------------------------------
// just a long string. From the Post window, used to save the Email
//   addresses of folks, during Save-To-Draft operation.
int  TArticleHeaderInner::GetCCEmailString (CString & strOutput) const
{
	if (m_vRandomStrings.GetSize() >= 3)
		strOutput = m_vRandomStrings[2];
	return 0;
}

// --------------------------------------------------------------------------
void TArticleHeaderInner::SetCCEmailString (const CString & strInput)
{
	m_vRandomStrings.SetAtGrow (2, strInput);
}

///////////////////////////////////////////////////////////////////////////
//
//  Start of TEmailHeaderInner implementation
//
///////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG)
void TEmailHeaderInner::Dump(CDumpContext& dc) const
{
	// base class version
	PObject::Dump(dc);
	dc << "TEmailHdr " ;
}
#endif

TEmailHeaderInner::TEmailHeaderInner(BYTE objectVersion)
: TPersist822HeaderInner(objectVersion)
{
	m_fArticle = FALSE;
}

// put elements into stringlist
//
// this does not separate the phrase from the address-spec
//
void TEmailHeaderInner::ParseTo (const CString & to, TStringList * pOutList,
								 CStringList * pErrList)
{

	CString tmp = to;  // this is commaseparated
	int iLen = tmp.GetLength();
	LPTSTR  pText = tmp.GetBuffer(iLen);

	if (pErrList)
	{
		// trial
		parse_addr ( pText, pOutList, pErrList);
	}
	else
	{
		// parse and store internally
		parse_addr ( pText, &m_DestList, NULL );
	}
	tmp.ReleaseBuffer (iLen);
}

void TEmailHeaderInner::GetToText (CString& outStr)
{
	if (m_DestList.IsEmpty())
		return ;

	get_addr_text ( m_DestList, outStr );
}

void TEmailHeaderInner::ParseCC (const CString& comma_sep_CC_list,
								 TStringList * pFinal, CStringList * pErrList)
{
	CString tmp = comma_sep_CC_list;
	int iLen = tmp.GetLength();
	LPTSTR  pText = tmp.GetBuffer(iLen);

	if (pErrList)
	{
		ASSERT(pFinal);
		parse_addr ( pText, pFinal, pErrList );
	}
	else
		parse_addr ( pText, &m_CC, NULL );

	tmp.ReleaseBuffer(iLen);
}

void TEmailHeaderInner::GetCCText(CString& txt)
{
	if (m_CC.IsEmpty())
		return ;

	get_addr_text ( m_CC, txt );
}

void TEmailHeaderInner::ParseBCC (const CString& comma_sep_BCC_list,
								  TStringList * pOutList, CStringList * pErrList)
{
	CString tmp = comma_sep_BCC_list;
	int iLen = tmp.GetLength();
	LPTSTR  pText = tmp.GetBuffer(iLen);

	if (pErrList)
	{
		// trial - errors returned to User
		parse_addr ( pText, pOutList, pErrList );
	}
	else
		parse_addr ( pText, &m_BCC, NULL );

	tmp.ReleaseBuffer(iLen);
}

void TEmailHeaderInner::GetBCCText(CString& txt)
{
	if (m_BCC.IsEmpty())
		return ;

	get_addr_text ( m_BCC, txt );
}

int TEmailHeaderInner::GetDestinationCount(TBase822Header::EAddrType eAddr)
{
	switch (eAddr)
	{
	case TBase822Header::kAddrTo:
		return m_DestList.GetCount();
		break;

	case TBase822Header::kAddrCC:
		return m_CC.GetCount();
		break;

	case TBase822Header::kAddrBCC:
		return m_BCC.GetCount();
		break;
	}
	throw(new TException("GetDestinationCount", kFatal));
	return 0;
}

// I really only want the SMTP thread to use this
void TEmailHeaderInner::GetDestinationList (TBase822Header::EAddrType eAddr,
											TStringList * pstrList) const
{
	pstrList->RemoveAll ();
	switch (eAddr)
	{
	case TBase822Header::kAddrTo:
		pstrList->AddTail ( (CStringList*) &m_DestList );
		return;

	case TBase822Header::kAddrCC:
		pstrList->AddTail ( (CStringList*) &m_CC );
		return;

	case TBase822Header::kAddrBCC:
		pstrList->AddTail ( (CStringList*) &m_BCC );
		return;
	}
	throw(new TException("GetDestinationList", kFatal));
}

// ------------------------------------------------------------------------
//
//
void TEmailHeaderInner::parse_addr (LPTSTR strAddr, TStringList* pstrList,
									CStringList * pErrList)
{
	MAIL_ADDRESS * pAddrList = 0;

	rfc822_parse_adrlist ( &pAddrList, strAddr, "", pErrList );

	MAIL_ADDRESS * lst = pAddrList;
	MAIL_ADDRESS * one;
	BOOL fRet;
	CString oneAddr;
	pstrList->RemoveAll();

	while (lst)
	{
		one = lst;

		// get strict form
		fRet = format_address(one->mailbox, one->host, one->personal,  oneAddr);

		if (fRet)
		{
			// store pretty form in one slot of the string list
			pstrList->AddTail ( oneAddr );
		}
		lst = lst->next;
	}
	FreeAddressList ( pAddrList );
}

// get the displayable text, separated by commas
void TEmailHeaderInner::get_addr_text (TStringList & strList, CString& str)
{
	str.Empty();
	strList.CommaList ( str );
}

BOOL TEmailHeaderInner::format_address(LPTSTR mailbox, LPTSTR host, LPTSTR personal,
									   CString& outStr)
{
	if (NULL == mailbox && NULL == host)
		return FALSE;

	if (personal)
		outStr.Format ("%s@%s (%s)", mailbox, host, personal);
	else
		outStr.Format ("%s@%s", mailbox, host);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//  Created 3-20-96
//
TEmailHeaderInner&
TEmailHeaderInner::operator=(const TEmailHeaderInner& rhs)
{
	if (this == &rhs)
		return *this;
	TPersist822HeaderInner::operator=(rhs);

	m_From                        = rhs.m_From;
	m_MsgID                       = rhs.m_MsgID;
	m_Subject                     = rhs.m_Subject;
	m_Keywords                    = rhs.m_Keywords;
	m_Sender                      = rhs.m_Sender;
	m_replyTo                     = rhs.m_replyTo;
	m_DestList                    = rhs.m_DestList;
	m_MimeVersion                 = rhs.m_MimeVersion;
	m_MimeContentType             = rhs.m_MimeContentType;
	m_MimeContentEncoding         = rhs.m_MimeContentEncoding;
	m_MimeContentDescription      = rhs.m_MimeContentDescription;
	m_in_replyTo                  = rhs.m_in_replyTo;
	m_CC                          = rhs.m_CC;
	m_BCC                         = rhs.m_BCC;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
// this defines a virtual function and makes the right type (Email)
TPersist822HeaderInner* TEmailHeaderInner::duplicate()
{
	// call the copy-constructor
	return new TEmailHeaderInner( *this );
}

///////////////////////////////////////////////////////////////////////////
// Copy-constructor  - always a nice thing to have
//
// like Jeff Stevenson we could do "*this = src", but I want the
//  effiency of the initializer list.
// many of the strings are created in the Big initializer list
//
TEmailHeaderInner::TEmailHeaderInner(const TEmailHeaderInner& src)
: TPersist822HeaderInner(src),
m_From(src.m_From),               m_MsgID(src.m_MsgID),
m_Subject(src.m_Subject),         m_Keywords(src.m_Keywords),
m_Sender(src.m_Sender),           m_replyTo(src.m_replyTo),
m_MimeVersion(src.m_MimeVersion),
m_MimeContentType(src.m_MimeContentType),
m_MimeContentEncoding(src.m_MimeContentEncoding),
m_MimeContentDescription(src.m_MimeContentDescription),
m_in_replyTo(src.m_in_replyTo)
{
	// assign the TStringLists
	m_DestList = src.m_DestList;
	m_CC       = src.m_CC;
	m_BCC      = src.m_BCC;
}

void TEmailHeaderInner::Serialize (CArchive & ar)
{
	TPersist822HeaderInner::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_From;
		ar << m_MsgID;
		ar << m_Subject;
		ar << m_Keywords;
		ar << m_Sender;
		ar << m_replyTo;
		m_DestList.Serialize ( ar );
		ar << m_MimeVersion;
		ar << m_MimeContentType;
		ar << m_MimeContentEncoding;
		ar << m_MimeContentDescription;
		ar << m_in_replyTo;
		m_CC.Serialize (ar);
		m_BCC.Serialize (ar);
	}
	else
	{
		ar >> m_From;
		ar >> m_MsgID;
		ar >> m_Subject;
		ar >> m_Keywords;
		ar >> m_Sender;
		ar >> m_replyTo;
		m_DestList.Serialize ( ar );
		ar >> m_MimeVersion;
		ar >> m_MimeContentType;
		ar >> m_MimeContentEncoding;
		ar >> m_MimeContentDescription;
		ar >> m_in_replyTo;
		m_CC.Serialize (ar);
		m_BCC.Serialize (ar);
	}
}

///////////////////////////////////////////////////////////////////////////
// Created 11-24-95
// put elements into stringlist
// this does not separate the phrase from the address-spec
//
// serves as helper function for ParseTo(), ParseCC(), ParseBCC()
void TEmailHeaderInner::parse_into_stringlist   (
	const CString& comma_list, TStringList& tstrList)
{
	CString lst(comma_list);
	CString item;

	tstrList.RemoveAll();               // logical, right?

	int comma = lst.Find ( ',' );       // zero based
	while (comma != -1)
	{
		item = lst.Left(comma);
		// list is smaller
		lst = lst.Mid(comma + 1);

		item.TrimLeft();
		item.TrimRight();
		if (!item.IsEmpty())
		{
			// validate
			tstrList.AddTail (item);
		}

		comma = lst.Find ( ',' );
	}

	lst.TrimLeft();
	lst.TrimRight();
	if (!lst.IsEmpty())
	{
		// validation???
		tstrList.AddTail ( lst );
	}
}

void TEmailHeaderInner::GetMimeLines(CString* pVer, CString* pType, CString* pCode,
									 CString* pDesc)
{
	if (pVer)  *pVer  = m_MimeVersion;
	if (pType) *pType = m_MimeContentType;
	if (pCode) *pCode = m_MimeContentEncoding;
	if (pDesc) *pDesc = m_MimeContentDescription;
}

void TEmailHeaderInner::SetMimeLines(LPCTSTR ver, LPCTSTR type, LPCTSTR encode, LPCTSTR desc)
{
	m_MimeVersion = ver;
	m_MimeContentType = type;
	m_MimeContentEncoding = encode;
	m_MimeContentDescription = desc;
}

// ------------------------------------------------------------------------
void TPersist822HeaderInner::SetAttachments (void *psAttachmentsVoid)
{
	TAttachmentArray<TAttachmentInfo, TAttachmentInfo&> *psAttachments =
		(TAttachmentArray<TAttachmentInfo, TAttachmentInfo&> *) psAttachmentsVoid;

	if (m_psAttachments) {
		delete [] m_psAttachments;
		m_psAttachments = NULL;
	}

	m_iNumAttachments = psAttachments->GetSize ();
	if (!m_iNumAttachments)
		return;
	m_psAttachments = new TAttachmentInfo [m_iNumAttachments];
	for (int i = 0; i < m_iNumAttachments; i++)
		m_psAttachments [i] = psAttachments->ElementAt (i);
}

/////////////////////////

TBlobMaster::TBlobMaster()
{
	int tot = sizeof(m_riOffsets)/sizeof(m_riOffsets[0]);

	for (--tot; tot >= 0; --tot)
		m_riOffsets[tot] = -1;

	m_iBlobCur = 0;
	m_iBlobMax = 0;

	m_pBlob = NULL;
}

TBlobMaster::~TBlobMaster()
{
	try
	{
		if (m_pBlob)
			free (m_pBlob);
	}
	catch(...)
	{
		// catch everything - no exceptions can leave a destructor
	}
}

void TBlobMaster::SetAtGrow(EFlatField eIdx, LPCTSTR strInput, BYTE & phraseAvail)
{
	LONG & idx = m_riOffsets[eIdx];

	if (-1 != idx)
		remove_fld( idx, phraseAvail );

	int addLen = _tcslen( strInput ) + 1;
	int addSize = addLen * sizeof(TCHAR);

	if (addSize + m_iBlobCur > m_iBlobMax)
	{
		LPTSTR pMore = (LPTSTR) realloc(m_pBlob, addSize + m_iBlobCur);
		if (0 == pMore)
		{
			articlei_nomem_handler (FALSE, addSize + m_iBlobCur, 11);
		}
		m_pBlob = (LPTSTR) pMore;
		m_iBlobMax = addSize + m_iBlobCur;
	}
	idx = m_iBlobCur / sizeof(TCHAR);
	_tcscpy (m_pBlob + idx, strInput);
	m_iBlobCur += addSize;
}

void TBlobMaster::remove_fld(LONG& idx, BYTE & phraseAvailable)
{
	int i,iOldFld = idx;

	// bytes to remove
	if (idx > m_iBlobMax)
		throw(new TException(IDS_ERR_INVALID_OFFSET, kFatal));

	LPTSTR p = m_pBlob + idx;
	LPTSTR fwd = p + (_tcslen(p) + 1);
	int iBytesRemoved = fwd - p;
	int iCharsRemoved =  iBytesRemoved / sizeof(TCHAR);

	for (i = 0; i < (sizeof(m_riOffsets)/sizeof(m_riOffsets[0])); ++i)
	{
		if (m_riOffsets[i] >= iOldFld)
			m_riOffsets[i] -= iCharsRemoved;
	}

	// move bytes down
	MoveMemory (p, fwd, (m_pBlob + m_iBlobCur) - fwd);

	m_iBlobCur -= iBytesRemoved;
	idx = -1;

	// to be safe - reparse the from field
	phraseAvailable = FALSE;
}

LPCTSTR TBlobMaster::GetBlobString(EFlatField eIdx) const
{
	int idx = m_riOffsets[eIdx];

	if (-1 == idx)
		return rcEmptyString;
	else
	{
		if (idx > m_iBlobMax)
			throw(new TException(IDS_ERR_INVALID_OFFSET, kFatal));
		return m_pBlob + idx;
	}
}

// copy-constructor
TBlobMaster::TBlobMaster (const TBlobMaster & src)
{
	CopyMemory(&m_riOffsets, &src.m_riOffsets, sizeof(m_riOffsets));

	m_iBlobCur = src.m_iBlobCur;
	m_iBlobMax = src.m_iBlobMax;
	m_pBlob    = NULL;

	m_pBlob = (LPTSTR) malloc( m_iBlobMax);

	if (0==m_pBlob)
		articlei_nomem_handler (TRUE, m_iBlobMax, 17);

	CopyMemory(m_pBlob, src.m_pBlob, m_iBlobCur);
}

// ----------------------------------------------------------------------
TBlobMaster &  TBlobMaster::operator= (const TBlobMaster & rhs)
{
	if (this == &rhs)
		return *this;

	CopyMemory(&m_riOffsets, &rhs.m_riOffsets, sizeof(m_riOffsets));

	m_iBlobCur = rhs.m_iBlobCur;
	m_iBlobMax = rhs.m_iBlobMax;

	if (m_pBlob)
		free (m_pBlob);

	m_pBlob = (LPTSTR)malloc( m_iBlobMax);
	if (0==m_pBlob)
		articlei_nomem_handler (TRUE, m_iBlobMax, 17);

	CopyMemory(m_pBlob, rhs.m_pBlob, m_iBlobCur);

	return *this;
}

// ----------------------------------------------------------------------
void TBlobMaster::Serialize (CArchive & archive)
{
	if (archive.IsStoring())
	{
		archive.Write (m_riOffsets, sizeof(m_riOffsets));

		archive << m_iBlobCur;
		archive.Write(m_pBlob, m_iBlobCur);
	}
	else
	{
		archive.Read (m_riOffsets, sizeof(m_riOffsets));

		archive >> m_iBlobCur;

		LONG lBytesNeeded = m_iBlobCur;

		if (m_iBlobMax < lBytesNeeded)
		{

			if (m_pBlob)
				free (m_pBlob);

			m_pBlob = (LPTSTR) malloc (lBytesNeeded);

			if (0==m_pBlob)
				articlei_nomem_handler (TRUE, lBytesNeeded, 26);

			m_iBlobMax = lBytesNeeded;

		}

		archive.Read (m_pBlob, m_iBlobCur);
	}
}

