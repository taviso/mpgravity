/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecjob.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/02/19 11:24:17  richard_wood
/*  Re-enabled optimisations in classes newssock.cpp, rulesdlg.cpp, server.cpp and tdecjob.cpp
/*
/*  Revision 1.3  2008/09/19 14:51:59  richard_wood
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

// tdecjob.cpp -- decode job

#include "stdafx.h"              // standard stuff
#include <string.h>              // strncpy()
#include <memory.h>              // memset()
#include <io.h>                  // _access()
#include "tdecjob.h"             // this file's prototypes
#include "coding.h"              // TCodeMgr
#include "tglobopt.h"            // TGlobalOptions
#include "nglist.h"              // Lookup()
#include "tdecthrd.h"            // gpsDecodeDialog
#include "resource.h"            // IDS_*
#include "genutil.h"             // SetStatusBit(), ViewBinary(), ...
#include "server.h"              // TNewsServer
#include "nglist.h"              // TNewsGroupUseLock
#include "thredlst.h"            // TThreadList
#include "newsdb.h"              // THeaderIterator
#include "uipipe.h"              // gpUIPipe
#include "rgswit.h"              // TRegSwitch
#include "rgsys.h"               // TRegSystem
#include "tdecutil.h"            // GetDecodeDirectory(), ...
#include "custmsg.h"             // WMU_LOW_SPACE
#include "utilrout.h"            // PostMainWndMsg()
#include "evtlog.h"              // TEventLog

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
IMPLEMENT_SERIAL (TDecodeJob, PObject, DECODE_JOB_VERSION_NUMBER)

// -------------------------------------------------------------------------
// TDecodeJob -- default constructor
TDecodeJob::TDecodeJob () : TUtilityJob (DECODE_JOB_VERSION_NUMBER)
{
	m_riFoundParts = NULL;
	m_rsPartHeaders = NULL;
	m_pText = NULL;
	m_bLaunchViewer = FALSE;
	m_iPreProcessError = 0;
	m_wFlags = 0;
	m_iNumParts = 0;
}

// -------------------------------------------------------------------------
TDecodeJob::TDecodeJob (const TDecodeJob *pCopy)
: TUtilityJob (DECODE_JOB_VERSION_NUMBER, pCopy),
m_pText(NULL)
{
	*this = *pCopy;
}

// -------------------------------------------------------------------------
TDecodeJob &TDecodeJob::operator= (const TDecodeJob &src)
{
	CopyCStringList (m_rstrFilenames, src.Filenames ());
	m_strDirectory = src.m_strDirectory;
	m_iNumParts = src.m_iNumParts;
	m_iGotParts = src.m_iGotParts;
	m_bLaunchViewer = src.m_bLaunchViewer;
	m_iThisPart = src.m_iThisPart;
	m_iPreProcessError = src.m_iPreProcessError;
	m_wFlags = src.m_wFlags;

	m_riFoundParts = 0;
	m_rsPartHeaders = 0;
	if (m_iNumParts)
	{
		m_riFoundParts = new int [m_iNumParts];
		m_rsPartHeaders = new TArticleHeader [m_iNumParts];
		for (int i = 0; i < m_iNumParts; i++)
		{
			m_riFoundParts [i] = src.m_riFoundParts [i];
			m_rsPartHeaders [i] = src.m_rsPartHeaders [i];
		}
	}

	if (src.m_pText)
	{
		ASSERT(0 == m_pText);
		m_pText = new TArticleText;
		if (m_pText)
			*m_pText = *src.m_pText;
	}

	return *this;
}

// -------------------------------------------------------------------------
TUtilityJob * TDecodeJob::Clone()
{
	TDecodeJob * pXerox = new TDecodeJob ( this );

	return pXerox;
}

// -------------------------------------------------------------------------
// TDecodeJob -- constructor. CString objects are passed in to take advantage
//               of MFC reference counting
TDecodeJob::TDecodeJob (LONG lGroupID,
						const CString & rstrGroupName,
						const CString & rstrGroupNickname,
						TArticleHeader *psHdr,
						const CString & strDirectory,
						int iRuleBased /* = FALSE */,
						TArticleText *pText /* = NULL */,
						char *pchSubject /* = NULL */,
						BOOL bLaunchViewer /* = FALSE */)

						: TUtilityJob (DECODE_JOB_VERSION_NUMBER, lGroupID, rstrGroupName,
						rstrGroupNickname, psHdr, iRuleBased)
{
	// mark this article as having been queued for decoding
	GenSetStatusBit (lGroupID, psHdr -> GetArticleNumber (),
		TStatusUnit::kQDecode);

	m_strJobType.LoadString (IDS_DECODE_STRING);
	m_strDirectory = strDirectory;
	m_pText = NULL;
	m_iNumParts = 0;
	m_iGotParts = 0;
	m_riFoundParts = NULL;
	m_rsPartHeaders = NULL;
	m_bLaunchViewer = bLaunchViewer;
	m_wFlags = 0;
	if (pchSubject)
		m_strSubject = pchSubject;
	if (pText) {
		m_pText = new TArticleText;
		if (m_pText)
			*m_pText = *pText;
	}
}

// -------------------------------------------------------------------------
// ~TDecodeJob -- destructor
TDecodeJob::~TDecodeJob ()
{
	// exceptions must not leave the destructor. If one does, C++ will
	//   call abort()!!
	try {
		if (m_riFoundParts)
			delete [] m_riFoundParts;
		if (m_rsPartHeaders)
			delete [] m_rsPartHeaders;
		if (m_pText)
			delete m_pText;
	}
	catch(...) {
		// trap everything
	}
}

// -------------------------------------------------------------------------
static void WriteDescription (const CString &strDir,
							  const CString &strFilePath, const CString &strGroup,
							  const CString &strSubject, TArticleHeader &sHeader)
{
	CString strDescriptionFile = strDir + "\\descript.ion";

	CString strFilename;
	int i = 0;
	for (i = strFilePath.GetLength () - 1; i >= 0; i--)
		if (strFilePath [i] == '\\')
			break;
	strFilename = strFilePath.Right (strFilePath.GetLength () - i - 1);

	if (strFilename.Find (" "))
		strFilename = CString ("\"") + strFilename + "\"";

	CString strLine;
	strLine.Format ("%s %s (%s, %s, %s)\n", LPCTSTR(strFilename),
		LPCTSTR(strSubject), LPCTSTR(strGroup), LPCTSTR(sHeader.GetFrom ()),
		LPCTSTR(sHeader.GetDate ()));

	try {
		BOOL bExist = (_access (strDescriptionFile, 0) == 0);

		CStdioFile sFile (strDescriptionFile,
			CFile::modeWrite | CFile::modeNoTruncate | CFile::modeCreate);
		sFile.SeekToEnd ();
		sFile.WriteString (strLine);
		sFile.Close ();

		// if it didn't exist, make it hidden
		if (!bExist) {
			CFileStatus sStatus;
			CFile::GetStatus (strDescriptionFile, sStatus);
			sStatus.m_attribute |= 2 /* hidden */;
			CFile::SetStatus (strDescriptionFile, sStatus);
		}
	}
	catch(...) {
		return;  // no need for error message...
	}
}

// -------------------------------------------------------------------------
static BOOL IsSlash (char ch, const CString &strSubject, int &i, int iLen)
{
	if (ch == '/')
		return TRUE;

	if (i + 1 < iLen &&
		toupper (ch) == 'O' && toupper (strSubject [i + 1]) == 'F') {
			i++;
			return TRUE;
	}

	return FALSE;
}

// -------------------------------------------------------------------------
// ScanSubjectDigits -- scans one or two digits from a subject line. If it
// scans two digits, it advances the loop counter (i)
static int ScanSubjectDigits (const CString &strSubject, int &i, int &iNum,
							  int iLen)
{
	char ch = strSubject [i];
	if (!isdigit (ch))
		return 1;
	iNum = ch - '0';

	// if we've reached the end of the string, return
	if (iLen <= i + 1)
		return 0;

	// look at the next character, which is possibly another digit
	ch = strSubject [i + 1];
	if (isdigit (ch)) {
		i++;
		iNum = iNum * 10 + (ch - '0');
	}

	// if we've reached the end of the string, return
	if (iLen <= i + 1)
		return 0;

	// look at the next character, which is possibly another digit
	ch = strSubject [i + 1];
	if (isdigit (ch)) {
		i++;
		iNum = iNum * 10 + (ch - '0');
	}

	return 0;
}

// -------------------------------------------------------------------------
// ScanBinarySubjectSegment -- returns 0 for success, non-0 for failure
static int ScanBinarySubjectSegment (const CString &strSubject, int &iThis,
									 int &iTotal, BOOL &bImpliedPart1, int &i, int &iIndexStart, int &iIndexEnd)
{
	int iLen = strSubject.GetLength ();
	enum State {INIT, FOUND_LEFT, FOUND_NUM1, FOUND_SLASH};
	State iState = INIT;
	char ch;
	iThis = iTotal = 0;

	// look for first occurrence of <num> ['/'|'of'] <num>
	while (i < iLen) {
		ch = strSubject [i];
		switch (iState) {
		 case INIT:
			 iIndexStart = i;
			 if (isdigit (ch)) {
				 iState = FOUND_LEFT;
				 i--;  // read this digit again
			 }
			 break;
		 case FOUND_LEFT:
			 if (!ScanSubjectDigits (strSubject, i, iThis, iLen))
				 iState = FOUND_NUM1;
			 else {
				 iIndexStart = -1;
				 iState = INIT;
			 }
			 break;
		 case FOUND_NUM1:
			 if (IsSlash (ch, strSubject, i, iLen))
				 iState = FOUND_SLASH;
			 else {
				 iIndexStart = -1;
				 iState = INIT;
			 }
			 break;
		 case FOUND_SLASH:
			 if (!ScanSubjectDigits (strSubject, i, iTotal, iLen)) {
				 iIndexEnd = i;
				 goto end;
			 }
			 else {
				 iIndexStart = -1;
				 iState = INIT;
			 }
			 break;
		}
		i++;
	}

end:
	return iState == FOUND_SLASH ? 0 : 1;
}

// -------------------------------------------------------------------------
int  ScanForFilename (LPCTSTR pSubject, CString & strFileName)
{
	static RxSearch srName;
	int       iResultLen = 0;
	LPCTSTR   pszResult;
	LPCTSTR   pStart;
	LPCTSTR   pLast;
	int       iLastLen;

	// we want to look for period, so we do a RE  escape to \.
	//   but then we need a 'C' string escape ---  \\.

	if (!srName.HasPattern ())
	{
		// get user's version of RE pattern
		TRegSystem * pRegSys =  gpGlobalOptions->GetRegSystem();

		if (srName.Compile (pRegSys->GetFindFilenameRE (false)))
		{
			// Error Box
			CString errMsg; errMsg.LoadString (IDS_ERR_RE_FILENAME);

			gpEventLog->AddError (TEventEntry::kDecode,
				errMsg,
				pRegSys->GetFindFilenameRE (false));

			VERIFY(0==srName.Compile (pRegSys->GetFindFilenameRE (true)));
		}
	}

	pStart = pSubject;            // begin looking from pStart
	pLast  = 0;

	while ( (pszResult = srName.Search (pStart, &iResultLen)) != 0)
	{
		pLast    = pszResult;
		iLastLen = iResultLen;

		pStart = pszResult + iResultLen;
	}

	if (pLast)
	{
		LPTSTR pBuf = strFileName.GetBuffer(iLastLen + 1);
		lstrcpyn (pBuf, pLast, iLastLen + 1);
		strFileName.ReleaseBuffer(iLastLen);
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
LPCTSTR  RxSearch_FindLast (RxSearch & sSearch,
							LPCTSTR    buffer, int * resultLen,
							int & Num, int & Denom, bool & fValidFrac)
{
	LPCTSTR pSearchFrom = buffer;
	LPCTSTR pResult;
	int     iResLen;
	LPCTSTR pLast = 0;
	int     iLastLen = 0;
	int     n, d;           // numerator , denominator

	n = d = 0;
	// iterate until we find the Last match
	fValidFrac = true;

	while ((pResult = sSearch.Search (pSearchFrom, &iResLen)) != NULL)
	{
		pLast    = pResult;
		iLastLen = iResLen;

		// arggh! depends on the Compiled pattern
		n   = atoi (sSearch.GetAssignment(1));
		d   = atoi (sSearch.GetAssignment(2));

		// advance the search start point
		pSearchFrom = pResult + iResLen;
	}

	// return

	*resultLen = iLastLen;
	Num = n;
	Denom = d;

	// handle cases like:  Hey! party on Saturday 9/8
	if ((Num > Denom) || (Num < 0) || (Denom < 0) || (0 == Num && 0 == Denom))
		fValidFrac = false;

	return pLast;
}

// -------------------------------------------------------------------------
// Identifies four things 1) fraction
//                        2) if fraction not found then ImpliedPart1 is True
//                        3) prefix = subject line - fraction
//                        4) if required, look for a filename in the subject line
//
int  ScanBinarySubject (const CString &    strSubject,
						CString &          strPrefix,
						CString *          pstrFileName,
						int &              iNumerator,
						int &              iTotal,
						BOOL &             fImpliedPart1,
						bool &             fValidFraction)
{

	static RxSearch    srFraction;
	int                iResultLen = 0;
	LPCTSTR            pszResult;

	fValidFraction = true;

	//$b [1]= numerator
	//$c [2]= denominator

	if (!srFraction.HasPattern())
	{
		TRegSystem * pRegSys = gpGlobalOptions->GetRegSystem ();

		LPCTSTR pRE = pRegSys->GetFindFractionRE (false);
		if (srFraction.Compile (pRE))
		{
			CString errMsg; errMsg.LoadString (IDS_ERR_RE_PARTS);

			// Error Box
			gpEventLog->AddError (TEventEntry::kDecode, errMsg, pRE);

			// use default version
			VERIFY(0==srFraction.Compile (pRegSys->GetFindFractionRE (true)));
		}
	}

	LPCTSTR  pszSubject = strSubject;

	pszResult = RxSearch_FindLast (srFraction,
		pszSubject,
		&iResultLen,
		iNumerator,
		iTotal,
		fValidFraction);

	if (NULL == pszResult)
	{
		fImpliedPart1 = TRUE;

		// assume this is part [1/1]
		iNumerator = iTotal = 1;

		fValidFraction = true;  // back to OK.

		strPrefix = strSubject;

		if (pstrFileName)
			// use entire subject to look for filename
			ScanForFilename ( strSubject, *pstrFileName );
		return 0;
	}
	else
	{
		fImpliedPart1 = FALSE;

		int  idx = (pszResult - pszSubject)/sizeof(TCHAR);
		strPrefix = strSubject.Left (idx);
		strPrefix += strSubject.Mid (idx + iResultLen);

		if (!strPrefix.IsEmpty())
		{
			// use remainder to look for filename
			if (pstrFileName)
				ScanForFilename (strPrefix, *pstrFileName);
		}
		else
		{
			// we are just out of luck here
		}
	}

	return 0;
}

// -------------------------------------------------------------------------
// AlphaCharChecksum -- count non digit characters
int AlphaCharChecksum (const CString & str)
{
	// ScanBinarySubject looks for first occurrence of <num> ['/'|'of'] <num>
	//   and we work in parallel, being careful to ignore 'O' and 'F'

	int nChecksum = 0;
	BYTE c, u;  // TODO:

	for (int i = 0; i < str.GetLength(); i++)
	{
		c = str[i];
		u = toupper(c);
		if (isalpha(u))
			nChecksum += BYTE(c);
	}

	return nChecksum;
}

// -------------------------------------------------------------------------
// PrefixMatch -- tells whether an article's subject matches a prefix
//
bool PrefixMatch (
				  const CString &strSubject,
				  const CString & strMasterPrefix,
				  int iPrefixChecksum,
				  int &iPart)
{
	// new algorithm is targeted towards ignoring digit characters.
	//   this is supposed to quickly weed out 90% of the candidates

	if (AlphaCharChecksum (strSubject) != iPrefixChecksum)
		return false;

	// do full blown scan and Try to find the part-number from this subject.

	int  iTotal;
	BOOL bImpliedPart1 = FALSE;
	CString strTestPrefix;
	bool fValidFraction;

	if (ScanBinarySubject (strSubject,
		strTestPrefix,
		NULL /* pstrFileName*/,
		iPart,
		iTotal,
		bImpliedPart1,
		fValidFraction))
		return false;

	if (!fValidFraction)
		return false;

	return (strMasterPrefix == strTestPrefix);
}

// -------------------------------------------------------------------------
// AddPart -- adds a post's part to this post's info
void TDecodeJob::AddPart (TArticleHeader *pHdr, int iPart)
{
	if (iPart < 1 || iPart > m_iNumParts ||   // yeah, it could happen
		m_riFoundParts [iPart-1])
		return;

	ASSERT(pHdr->GetNumber() > 0);

	m_riFoundParts[iPart - 1]  = TRUE;
	m_rsPartHeaders[iPart - 1] = *pHdr;
	m_iGotParts ++;
}

// -------------------------------------------------------------------------
// PreprocessJob -- find sibling parts of an article
//
// CALLED FROM:  decode thread :: AddJob, cuz the newsgroup is available there
//
void TDecodeJob::PreprocessJob (VEC_HDRS * pVecHdrs)
{
	VERIFY(_CrtCheckMemory());

	BOOL    bImpliedPart1 = FALSE;
	bool    fValidFraction;
	CString strSubject;
	CString strPrefix;
	int     iThisPartnum;
	int     ret;

	m_iPreProcessError = 0;

	if (m_strSubject.GetLength ())
		strSubject = m_strSubject;
	else
		strSubject = m_sHdr.GetSubject ();

	// analyze the subject line for fractions

	ret = ScanBinarySubject (strSubject,
		strPrefix,
		&m_strSubjFilename,
		iThisPartnum,
		m_iNumParts,
		bImpliedPart1,
		fValidFraction);

	if (!fValidFraction)
	{
		SetStatus (IDS_ERR_DECODE_BADFRAC, ERROR_STATUS);
		m_iPreProcessError = 1;
		return;
	}

	if (bImpliedPart1)
		SetImpliedPart1 ();

	if (ret)
	{
		SetStatus (IDS_ERR_DECODE_SUBJECT, ERROR_STATUS);
		m_iPreProcessError = 1;
		return;
	}

	// m_strPrefix was already set in ComputePrefix()

	// try to get all parts of the post
	if (LocateParts (m_strPrefix, iThisPartnum,  pVecHdrs))
	{
		m_iPreProcessError = 1;

		// error status already posted by LocateParts()
		return;
	}

	// remember my part number for later
	m_iThisPart = iThisPartnum;

	// this sets the decode bit (icon) for all articles in this job
	SetStatusBits (TStatusUnit::kQDecode);

	VERIFY(_CrtCheckMemory());
}

// -------------------------------------------------------------------------
// LocateParts -- looks up all parts of a post.  If it can't find all parts,
// it returns failure (non-zero)
//
// this is private
//
// CALLED FROM:   PreprocessJob
//
BOOL TDecodeJob::LocateParts (const CString & strPrefix,
							  int iThisPart,
							  VEC_HDRS * pVecHdrs)
{
	int iSetStatus = FALSE;

	// allocate the part-array
	ASSERT (m_iNumParts > 0);

	m_riFoundParts  = new int [m_iNumParts];
	m_rsPartHeaders = new TArticleHeader [m_iNumParts];

	if (!m_riFoundParts || !m_rsPartHeaders)
	{
		CString str; str.LoadString (IDS_ERR_OUT_OF_MEMORY);
		SetStatus (str, ERROR_STATUS);
		return 1;
	}

	// clear the part array's memory
	memset (m_riFoundParts, 0, sizeof (int) * m_iNumParts);

	// put the available header into the part-array
	AddPart (&m_sHdr, iThisPart);

	// if already got all parts, scram
	if (m_iGotParts == m_iNumParts)
		return 0;

	/*
	* need to find the rest of the parts from the newsgroup's flat-list.  Here
	* is my strategy:
	*  - obtain a lock on the global newsgroup-array for the duration of
	*    the operation
	*  - lock the newsgroup's headers for the duration of the operation
	*  - get a pointer to the newsgroup's thread list, to access the flatlist
	*  - locate our article's header in the flat list
	*  - search the flat list backward and forward one article at a
	*    time until we either find all related articles or we search
	*    the whole list
	*/

	// note: manual decode jobs are treated as rule-based  -amc

	if (m_iRuleBased)
	{

		if (0 == iThisPart)
		{
			SetStatus (IDS_DECODE_IGNORE_0);
			iSetStatus = TRUE;
			goto end;
		}

		// rule-based jobs consult the pending-job list.  This will either give
		// us the rest of the headers or start a new pending job
		int RC = ConsultPendingJobs (strPrefix, iThisPart);

		// if ConsultPendingJobs() resulted in an error, then it has already set
		// the error status
		if (RC)
			iSetStatus = TRUE;
	}
	else
	{
		LocatePartsFromUI ( strPrefix, &iSetStatus, pVecHdrs );
	}

end:

	if (m_iNumParts != m_iGotParts)
	{
		if (!iSetStatus)
			SetStatus (IDS_ERR_DECODE_FIND_PARTS, ERROR_STATUS);
		return 1;
	}

	// at this point, iSetStatus tells us whether we've had an error or not
	return iSetStatus;
}

// -------------------------------------------------------------------------
// LocatePartsFromUI -- locates matching parts of a job
//
//
// notes:  - this would be more isolated if instead of implicitly
//         using a newsgroup, it worked against a set of article hdrs.
//
//         - works using PrefixMatch + ScanBinarySubject (many times)
//
//
void TDecodeJob::LocatePartsFromUI (
									const CString & strPrefix,
									int *           piSetStatus,
									VEC_HDRS *      pVecHdrs)           // could be a hint to our siblings
{
	TServerCountedPtr cpNewsServer;     // smart pointer

	// UI-based jobs look at the newsgroup's flat-list
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock(cpNewsServer, m_strGroupName, &fUseLock, pNG);
	if (!fUseLock)
	{
		SetStatus (IDS_ERR_DECODE_FIND_NG, ERROR_STATUS);
		*piSetStatus = TRUE;
		return;
	}

	TAutoClose  sCloser(pNG);     // open+close newsgroup

	if (true)
	{
		TSyncReadLock  sLock(pNG);    // read lock + unlock

		int iPart;  // set by PrefixMatch
		int iTotal; iTotal = pNG -> FlatListLength ();
		int iPrefixChecksum = AlphaCharChecksum (strPrefix);

		if (0 == iTotal)
		{
			// if the newsgroup is not the current one, it has been emptied and the
			// threadlist and the flatlist are gone. No choice - use a linear search

			// I guess you could use a rule to decode multiple newsgroups.
			//   One group would be the current one, the other group would not!

			TPersist822Header* pPHdr = 0;
			THeaderIterator it1(pNG, THeaderIterator::kReadLock);
			BOOL fFound = FALSE;
			while (it1.Next(pPHdr))
			{
				if (pPHdr->GetNumber() == m_sHdr.GetNumber())
				{
					fFound = TRUE;
					break;
				}
			}

			if (!fFound)
				return;

			// found this article .  Need to purge any pending decode-job
			// that matches this article's prefix, just to be safe
			PurgePendingJob (strPrefix);

			THeaderIterator it2(pNG, THeaderIterator::kReadLock);
			while (it2.Next(pPHdr))
			{
				TArticleHeader * pHdr = pPHdr->CastToArticleHeader();
				if (PrefixMatch (pHdr -> GetSubject(),
					strPrefix,
					iPrefixChecksum,
					iPart))
				{
					AddPart (pHdr, iPart);
				}

				if (m_iGotParts >= m_iNumParts)
					break;
			}
		}
		else
		{
			// Need to purge any pending decode-job that matches
			//  this article's prefix, just to be safe
			PurgePendingJob (strPrefix);

			set<DWORD> setSeenAlready;

			// phase 1 -- don't process articles twice

			setSeenAlready.insert (m_sHdr.GetArticleNumber());

			// phase 2 -- best candidates

			VEC_HDRS::iterator itBest = pVecHdrs->begin();
			for (; itBest != pVecHdrs->end(); itBest++)
			{
				TArticleHeader * pHdr =  *itBest;

				if ( PrefixMatch (pHdr -> GetSubject(),
					strPrefix,
					iPrefixChecksum,
					iPart) )
				{
					AddPart (pHdr, iPart);

					if (m_iGotParts >= m_iNumParts)
						return;
				}

				setSeenAlready.insert ( pHdr->GetArticleNumber() );
			}

			// phase 3 -- broader search

			TThreadList *pThreadList = pNG -> GetpThreadList ();

			VEC_NODES::iterator itNodes    = pThreadList->GetFlatBegin();
			VEC_NODES::iterator itNodesEnd = pThreadList->GetFlatEnd();

			for (; itNodes != itNodesEnd; itNodes++) // go through nodes
			{
				TArtNode * pNode = *itNodes;

				STLMapAHdr::iterator it2    = pNode -> GetArtsItBegin();
				STLMapAHdr::iterator it2End = pNode -> GetArtsItEnd();

				for (; it2 != it2End; it2++)
				{
					TArticleHeader * pHdr = *it2;

					// okay, this is silly but we have to watch out for
					//   the guys we did in phase 1 and 2.

					if ( setSeenAlready.find(pHdr->GetArticleNumber())
						== setSeenAlready.end() )
					{

						if ( PrefixMatch (pHdr -> GetSubject(),
							strPrefix,
							iPrefixChecksum,
							iPart) )
						{
							AddPart (pHdr, iPart);

							if (m_iGotParts >= m_iNumParts)
								return;
						}
					}
				} // for hdrs

			} // for nodes

		} // decoding in current group

		// undo read lock on pNG
	}
}

// -------------------------------------------------------------------------
// ComputePrefix -- used by TDecodeThread::AddJob() to weed out new jobs whose
// prefix already occurs in the waiting-job-list
//
void TDecodeJob::ComputePrefix ()
{
	int iThis;
	BOOL bImpliedPart1 = FALSE;
	bool fValidFraction;
	int ret;
	int iNumParts = 1;

	CString str;
	if (m_strSubject.GetLength ())
		str = m_strSubject;
	else
		str = m_sHdr.GetSubject ();

	ret = ScanBinarySubject (str, m_strPrefix, NULL, iThis,
		iNumParts, bImpliedPart1, fValidFraction);

	if (fValidFraction)
		m_iNumParts = iNumParts;

	if (bImpliedPart1)
		SetImpliedPart1 ();

	if (ret)
		return;
}

class TEventMonitorDatum
{
public:
	bool          m_bIOError;
	bool          m_bFileSkipped;
	bool          m_bFileSeemsIncomplete;
	bool          m_bFileFailedCRC;
	CStringList * m_prstrDecodedFiles;

	void Initialize(CStringList *  prstrDecFilesIn)
	{
		m_bIOError = m_bFileSkipped = false;
		m_bFileSeemsIncomplete = false;
		m_bFileFailedCRC = false;
		m_prstrDecodedFiles = prstrDecFilesIn;
	}
};

// -------------------------------------------------------------------------
// DecodeEventMonitor -- gets event messages from a decode function
static TEventMonitorDatum gsEventMonitorDatum;

BOOL DecodeEventMonitor (TCodeEvent *pEvent)
{
	switch (pEvent -> m_kEventType)
	{
	case TCodeEvent::kFileError:
	case TCodeEvent::kIOError:
		gsEventMonitorDatum.m_bIOError = true;
		return TRUE;

	case TCodeEvent::kFileSkipped:
		gsEventMonitorDatum.m_bFileSkipped = true;
		gsEventMonitorDatum.m_prstrDecodedFiles -> AddTail (pEvent -> m_file);
		return TRUE;

	case TCodeEvent::kFileFinished:
		gsEventMonitorDatum.m_prstrDecodedFiles -> AddTail (pEvent -> m_file);
		return TRUE;

	case TCodeEvent::kFileSeemsIncomplete:
	case TCodeEvent::kFileFailedYEncSize:
		gsEventMonitorDatum.m_bFileSeemsIncomplete = true;
		gsEventMonitorDatum.m_prstrDecodedFiles -> AddTail (pEvent -> m_file);
		return TRUE;

	case TCodeEvent::kFileFailedCRC:
		gsEventMonitorDatum.m_bFileFailedCRC = true;
		gsEventMonitorDatum.m_prstrDecodedFiles -> AddTail (pEvent -> m_file);
		return TRUE;

	case TCodeEvent::kPartFinished:
	case TCodeEvent::kStartingFile:
	case TCodeEvent::kFileName:
	default:
		return TRUE;
	}
}

// -------------------------------------------------------------------------
// 6/16/98 -- turning off optimizations fixes a release-mode-only crash
#pragma optimize ("", off)
static BOOL DiskSpaceLow (char chDrive)
{
#define BYTES_IN_A_MEG (1024 * 1024)
	DWORDLONG dwlLowSpace =
		((DWORDLONG) gpGlobalOptions -> GetRegSwitch () -> GetPauseSpace ()) *
		BYTES_IN_A_MEG;
	TServerCountedPtr cpNewsServer;
	CString strDir; strDir.Format ("%c:\\", chDrive);
	DWORDLONG dwlFreeSpace;

	if (cpNewsServer -> GetFreeDiskSpaceEx (dwlFreeSpace, strDir))
		return FALSE;  // pretend there's enough disk space

	return dwlFreeSpace < dwlLowSpace;
}
#pragma optimize ("", on)

// -------------------------------------------------------------------------
// DoYourJob -- decode an image
// 5/14/98 -- turning off optimizations fixes a release-mode-only crash

//#pragma optimize ("", off)

// return true to re-queue the job
bool TDecodeJob::DoYourJob (HANDLE hKillEvent)
{
	VERIFY(_CrtCheckMemory());
	int     iAlreadySetStatus = FALSE;
	int     iFileSkip;
	CString strStatus;

	if (m_iPreProcessError)
		return false;

	// set the destination directory if it's not already set to something
	// (could have already been set in the constructor)
	if (m_strDirectory.IsEmpty ())
		GetDecodeDirectory (m_strDirectory, m_lGroupID);

	// users complained that it doesn't create the directory automatically
	EnsureDirExists (m_strDirectory);

	HWND hWnd = AfxGetMainWnd () -> GetSafeHwnd ();

	// if disk space too low, bring up a warning dialog & set error status
	// (the thread will pause the queue in this case)
	if ( gpGlobalOptions->GetRegSwitch()->GetPausingLowSpace() &&
		DiskSpaceLow (m_strDirectory [0]) )
	{
		SetStatus (IDS_ERR_LOW_SPACE);
		PostMainWndMsg (WMU_LOW_SPACE);
		return true;  // re-queue
	}

	TCodeMgr sCodeMgr;
	sCodeMgr.SetDestinationDirectory (m_strDirectory);

	m_rstrFilenames.RemoveAll ();

	gsEventMonitorDatum.Initialize ( &m_rstrFilenames );
	sCodeMgr.InstallEventMonitor (DecodeEventMonitor);

	TArticleText *pText = 0;

	TError   sErrorRet;
	EArticleSource  eArtSource = SRC_FROM_DB;

	// go through the post's parts, and decode them in order
	for (int i = 0; i < m_iNumParts; i++)
	{

		// for the part that triggered the decode, we may already have the text
		if (i == m_iThisPart - 1)
			pText = m_pText;

		// get the part's text
		if (NULL == pText)
		{
			pText = new TArticleText;
			if (!pText)
			{
				SetStatus (IDS_ERR_OUT_OF_MEMORY, ERROR_STATUS);

				return false;
			}

			// retrieve body - success is 0
			if (PlsFetchArticle (sErrorRet,
				eArtSource,
				&m_rsPartHeaders [i],
				pText,
				true,
				hKillEvent,
				i+1,
				m_iNumParts))
			{
				// error message has been displayed by FetchArticle()
				if (pText == m_pText)   // clean up
					m_pText = 0;
				delete pText;

				EErrorClass eClass ;  sErrorRet.GetClass ( eClass );

				DWORD   dwErr = sErrorRet.GetDWError ( );

				bool fReQJob = false;

				switch (eClass)
				{
				case kClassNNTP:
					{
						CString desc;  sErrorRet.GetDescription ( desc );
						if (0 == dwErr)
							dwErr = atoi(desc);

						if (411 == dwErr)        // catch "no such group"
							fReQJob = false;
						else if (dwErr < 400)    // catch   "no such art"  et al.
							fReQJob = true;
						break;
					}
				case kClassWinsock:
					if (dwErr == WSAEINTR)   // 'Alt-F4, stop
						fReQJob = true;
					else
						fReQJob = true;
					break;
				case kClassUser:
					{
						CString desc;  sErrorRet.GetDescription ( desc );
						CString strNC; strNC.LoadString (IDS_NOT_CONNECTED);
						if (desc == strNC)
							fReQJob = true;
					}
				}
				return fReQJob;
			}
		} // NULL == pText

		if (SRC_FROM_CACHE == eArtSource)
			sCodeMgr.m_fArticleFromCache_NoSKIP = true;
		else
			sCodeMgr.m_fArticleFromCache_NoSKIP = false;

		strStatus.Format ("(%d/%d) ", i + 1, m_iNumParts);
		SetStatus (strStatus, IDS_DECODING);

		// if we are doing a manual-decode, we want the subject within the
		// header to be our special subject, except that the part's number
		// is substituted for this part's number
		if (m_strSubject.GetLength ())
		{
			CString strNewSubject;
			int iManualNumber = 0;

			CString str; str.LoadString (IDS_MANUAL);

			CString strManual = CString ("(")   +   str   +   " ";

			if (m_strSubject.Left (strManual.GetLength ()) == strManual)
			{
				int iSubjectLen = m_strSubject.GetLength ();
				// str still contains "Manual"
				CString str2; str2 = CString ("(") + str + " "; // "(Manual "
				for (int j = str2.GetLength ();
					j < iSubjectLen && m_strSubject [j] >= '0' && m_strSubject [j] <= '9';
					j++)
					iManualNumber = iManualNumber * 10 + m_strSubject [j] - '0';

				CString strOriginalSubject = m_rsPartHeaders [i].GetSubject ();

				// IDS_MANUAL_SUBJECT  ==    "(Manual %d) %s [%d/%d]"

				str.LoadString (IDS_MANUAL_SUBJECT);
				strNewSubject.Format (str,  iManualNumber,
					strOriginalSubject,
					i + 1,  m_iNumParts);
				m_rsPartHeaders [i].SetSubject (strNewSubject);

			}
		}

		try {
			POINT  sFraction;
			sFraction.x = i + 1;
			sFraction.y = m_iNumParts,

				iFileSkip = sCodeMgr.Decode (m_strSubjFilename,
				sFraction,
				&m_rsPartHeaders[i],
				pText,
				hWnd,
				m_strSubject,
				!m_iRuleBased /* fAskOverwrite */);
		}
		catch (TException *sException)
		{
			// decode cancelled by user
			CString strError;
			ESeverity iSeverity;
			sException->GetError (strError, iSeverity);
			SetStatus (strError, 0, ERROR_STATUS);
			iAlreadySetStatus = TRUE;
			sException->Delete();
		}
		catch (CFileException *e)
		{
			SetStatus (IDS_ERR_DECODING, ERROR_STATUS);
			iAlreadySetStatus = TRUE;
			e -> Delete ();
		}
		catch(...)
		{
			SetStatus (IDS_ERR_DECODING, ERROR_STATUS);
			iAlreadySetStatus = TRUE;
		}

		if (pText == m_pText)   // clean up
			m_pText = 0;
		delete pText;
		pText = 0;

		// if we've encountered an error, stop
		if (iAlreadySetStatus || iFileSkip)
			break;

	} // loop through parts

	if (gsEventMonitorDatum.m_bIOError)
		SetStatus (IDS_ERR_DECODE_IOERROR);
	else
	{
		if (!iAlreadySetStatus && !m_rstrFilenames.IsEmpty ())
		{
			if (gsEventMonitorDatum.m_bFileFailedCRC)
				SetStatus ( IDS_DECUTIL_WARN_FAILEDCRC );

			else if (gsEventMonitorDatum.m_bFileSeemsIncomplete)
				SetStatus ( IDS_DECUTIL_WARN_INCOMPLETE );

			else if (gsEventMonitorDatum.m_bFileSkipped)
				SetStatus ( IDS_UTIL_SKIPPED );

			else
				SetStatus ( IDS_UTIL_OK );
		}
		else if (!iAlreadySetStatus)
			SetStatus (IDS_ERR_DECODE_BAD_ARTICLE, ERROR_STATUS);

		if (GetSuccessful () && gpGlobalOptions -> GetWriteDescription ())
			WriteDescription (m_strDirectory, FirstFilename (), m_strGroupName,
			m_strSubject, m_rsPartHeaders [0]);

		// let ImageGallery add the new image(s) ....
		if (GetSuccessful ())
		{
			POSITION pos = m_rstrFilenames.GetHeadPosition ();
			while (pos)
			{
				const CString &strFilename = m_rstrFilenames.GetNext (pos);
				NotifyImageGallery (strFilename, m_strSubject, m_strGroupNickname);
			}
		}

		if (GetSuccessful () && LaunchViewer ())
		{
			POSITION pos = m_rstrFilenames.GetHeadPosition ();
			while (pos)
			{
				const CString & strFilename = m_rstrFilenames.GetNext (pos);
				gpUIPipe -> NewViewBinary ( strFilename );
			}

			if (gpGlobalOptions -> GetRegSwitch () -> GetDecodeMarksRead () ==
				TRegSwitch::kReadAfterView)
				MarkArticlesRead ();
		}

		if (GetSuccessful())
			gpDecodeCache->EraseAll ();
	}

	VERIFY(_CrtCheckMemory());

	return false;
}
#pragma optimize ("", on)

// -------------------------------------------------------------------------
// CopyVisualFields -- copies fields that appear in the decode-jobs dialog
void TDecodeJob::CopyVisualFields (TDecodeJob *pSource)
{
	m_strGroupName = pSource -> m_strGroupName;
	m_strGroupNickname = pSource -> m_strGroupNickname;
	m_strStatus = pSource -> m_strStatus;
	CopyCStringList (m_rstrFilenames, pSource -> m_rstrFilenames);
	m_strDirectory = pSource -> m_strDirectory;
	m_iNumParts = pSource -> m_iNumParts;
	m_iGotParts = pSource -> m_iGotParts;
	m_riFoundParts = new int [pSource -> m_iNumParts];
	if (pSource -> m_strSubject.GetLength ())
		m_sHdr.SetSubject (pSource -> m_strSubject);
	else
		m_sHdr.SetSubject (pSource -> m_sHdr.GetSubject());
	if (!m_riFoundParts)
		return;
	for (int i = 0; i < pSource -> m_iNumParts; i++)
		// could be that pSource -> m_riFoundParts is null at this point
		m_riFoundParts [i] =
		pSource -> m_riFoundParts ? pSource -> m_riFoundParts [i] : 0;
}

// -------------------------------------------------------------------------
// MonitorDialog -- returns a dialog that is monitoring us, or NULL. Use in
//    conjunction with LockDialogPtr
CDialog *TDecodeJob::MonitorDialog ()
{
	return gpsDecodeDialog;
}

// -------------------------------------------------------------------------
// LockDialogPtr -- get exclusive access to critical section that protects
//     gpsDecodeDialog
void TDecodeJob::LockDialogPtr(BOOL fLock)
{
	if (fLock)
		EnterCriticalSection (&gcritDecodeDialog);
	else
		LeaveCriticalSection (&gcritDecodeDialog);
}

// -------------------------------------------------------------------------
// SetSubclassStatus -- does decode-job-specific status setting
void TDecodeJob::SetSubclassStatus (int iResourceID)
{
	// set the decode status bits for each article in this job, unless the
	// status is "waiting on other parts", in which case the bits will be
	// updated in the pending-job code
	switch (iResourceID) {
case IDS_DECODING:
case IDS_DECODE_WAITING_FOR_PARTS:
case IDS_UTIL_FETCH_DB:
case IDS_UTIL_FETCH_NF:
	// ignore
	break;
case IDS_UTIL_OK:
case IDS_UTIL_SKIPPED:
case IDS_DECUTIL_WARN_INCOMPLETE:
	SetSuccessful ();
	SetStatusBits (TStatusUnit::kDecoded);
	break;
default:
	SetStatusBits (TStatusUnit::kDecodeErr);
	break;
	}
}

// -------------------------------------------------------------------------
// SetStatusBits -- sets a status bit for each article in this job
void TDecodeJob::SetStatusBits (TStatusUnit::EStatus iBit,
								BOOL fValue /* = TRUE */)
{
	CPtrArray  vArtHeaders;

	// accumulate  header pointers into the array
	if (m_riFoundParts)
		for (int i = 0; i < m_iNumParts; i++)
			if ( m_riFoundParts [i] )
				vArtHeaders.Add ( &m_rsPartHeaders [i] );

	// vArtHeaders.Add ( &m_sHdr );  should be included already!!

	// pass array into the worker function
	GenSetManyObjStatusBit (m_lGroupID, vArtHeaders, iBit, NULL, fValue);
}

// -------------------------------------------------------------------------
void TDecodeJob::JobKickedBackToWaitQueue ()
{
	SetStatusBits (TStatusUnit::kDecodeErr, FALSE);
	SetStatusBits (TStatusUnit::kQDecode);
}

// -------------------------------------------------------------------------
void TDecodeJob::JobBeingCancelled ()
{
	// get rid of this article's "decoding" icon
	SetStatusBits (TStatusUnit::kQDecode, FALSE);
}

// -------------------------------------------------------------------------
// History:
//         Object version 2 - added m_strSubjFilename
//
void TDecodeJob::Serialize (CArchive& sArchive)
{
	PObject::Serialize (sArchive);

	TUtilityJob::Serialize (sArchive);
	if (sArchive.IsStoring())
	{
		sArchive << m_bLaunchViewer;
		sArchive << FirstFilename ();
		sArchive << m_strDirectory;
		sArchive << m_iNumParts;
		sArchive << m_iGotParts;
		sArchive << m_strSubject;
		int i = 0;
		for (int i = 0; i < m_iNumParts; i++)
			// could be that m_riFoundParts is null at this point
			sArchive << (m_riFoundParts ? m_riFoundParts [i] : 0);
		for (i = 0; i < m_iNumParts; i++)
			// could be that m_rsPartHeaders is null at this point
			if (m_rsPartHeaders)
				m_rsPartHeaders [i].Serialize (sArchive);
			else
			{
				TArticleHeader sTempHeader;
				sTempHeader.Serialize (sArchive);
			}

			sArchive << m_strSubjFilename;
	}
	else
	{

		if (GetObjectVersion () < 1)
		{
			// it's an older version, here we need to read the object in its
			// original format then convert it...
			ASSERT (0);
			return;
		}

		sArchive >> m_bLaunchViewer;
		CString strFilename;
		sArchive >> strFilename;
		SetFilename (strFilename);
		sArchive >> m_strDirectory;
		sArchive >> m_iNumParts;
		sArchive >> m_iGotParts;
		sArchive >> m_strSubject;
		if (m_iNumParts)
		{
			m_riFoundParts = new int [m_iNumParts];
			m_rsPartHeaders = new TArticleHeader [m_iNumParts];
		}
		int i = 0;
		for (i = 0; i < m_iNumParts; i++)
			sArchive >> m_riFoundParts [i];
		for (i = 0; i < m_iNumParts; i++)
			m_rsPartHeaders [i].Serialize (sArchive);

		if (GetObjectVersion() >= DECODE_JOB_VERSION_NUMBER)
			sArchive >> m_strSubjFilename;
	}
}

// -------------------------------------------------------------------------
// ArticleFullPath -- takes an article number and determines whether this job
// contains an article with the same number.  If not found, returns non-zero.
// If found, returns zero and fills in the decoded binary's full-path
int TDecodeJob::ArticleFullPath (LONG lGroupID, LONG lArtNum, CString &strPath)
{
	if (lGroupID != m_lGroupID)
		return 1;

	BOOL bFound = FALSE;
	if (m_sHdr.GetNumber () == lArtNum)
		bFound = TRUE;

	if (!bFound && m_rsPartHeaders)
		for (int i = 0; i < m_iNumParts; i++)
			if (m_rsPartHeaders [i].GetNumber () == lArtNum) {
				bFound = TRUE;
				break;
			}

			if (!bFound)
				return 1;

			strPath = FirstFilename ();
			return 0;
}

// -------------------------------------------------------------------------
BOOL TDecodeJob::MergeJobsInResultPane (TUtilityJob *pOtherJob)
{
	TDecodeJob *pOtherDecodeJob = (TDecodeJob *) pOtherJob;
	return TUtilityJob::MergeJobsInResultPane (pOtherJob) &&
		FirstFilename () == pOtherDecodeJob -> FirstFilename ();
}

// -------------------------------------------------------------------------
// Did we imply part 1 of 1 ?
void TDecodeJob::SetImpliedPart1 ()
{
	m_wFlags |= kImpliedPart1;
}

BOOL TDecodeJob::GetImpliedPart1 ()
{
	return (m_wFlags & kImpliedPart1) ? TRUE : FALSE;
}

// -------------------------------------------------------------------------
void TDecodeJob::SetSuccessful ()
{
	m_wFlags |= kOK;
}

// -------------------------------------------------------------------------
BOOL TDecodeJob::GetSuccessful ()
{
	return (m_wFlags & kOK) ? TRUE : FALSE;
}

// -------------------------------------------------------------------------
BOOL TDecodeJob::LaunchViewer ()
{
	return (m_bLaunchViewer ||
		(m_iRuleBased ? gpGlobalOptions->IsLaunchViewerByRules () :
		gpGlobalOptions->IsLaunchingViewerOnManual ()));
}

// -------------------------------------------------------------------------
const CString TDecodeJob::FirstFilename ()
{
	CString str;
	POSITION pos = m_rstrFilenames.GetHeadPosition ();
	if (pos)
		str = m_rstrFilenames.GetNext (pos);
	return str;
}

// -------------------------------------------------------------------------
// MarkArticlesRead -- mark this job's articles as read
void TDecodeJob::MarkArticlesRead ()
{
	TServerCountedPtr cpNewsServer;
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock (cpNewsServer, m_strGroupName, &fUseLock, pNG);
	if (!fUseLock)
		return;

	if (m_riFoundParts)
		for (int i = 0; i < m_iNumParts; i++)
			if (m_riFoundParts [i])
				pNG -> ReadRangeAdd (&m_rsPartHeaders [i]);
	pNG -> ReadRangeAdd (&m_sHdr);
}

// -------------------------------------------------------------------------
void TDecodeJob::SetFilename (const CString &strNewName)
{
	m_rstrFilenames.RemoveAll ();
	m_rstrFilenames.AddHead (strNewName);
}

