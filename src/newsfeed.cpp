/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsfeed.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/08/27 15:29:22  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.4  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.3  2009/08/18 22:05:02  richard_wood
/*  Refactored XOVER and XHDR commands so they fetch item data in batches of 300 (or less) if we want > 300 articles.
/*
/*  Revision 1.2  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

#include "newsfeed.h"

#include "newssock.h"
#include "mplib.h"

#include "custmsg.h"
#include "usrdisp.h"
#include "resource.h"

#include "ecpcomm.h"       // communication TExceptions
#include "memspot.h"
#include "autoptr.h"

#include <strstream>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Uses  kClassNNTP
//
//

// warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

extern TUserDisplay* gpUserDisplay;

const int TUserDisplay_Frequency = 5;

static char szNone[] = "(none)";

TNewsFeed::TNewsFeed(int portNumber, FARPROC pBlockingHook, HANDLE hStopEvent,
					 bool * pfProcessJobs, bool bSecure)
{
	m_pSocket = new TNewsSocket (portNumber,
		pBlockingHook,
		hStopEvent,
		pfProcessJobs,
		bSecure);

	m_fValid = m_pSocket ? true : false;
}

TNewsFeed::~TNewsFeed(void)
{
	delete m_pSocket;
	m_pSocket = NULL;
}

////////////////////////////////////////////////////////////////////////////////////
//
// estimateLines - provides a range for the statusbar
// fPriority     - indicates a priority article - (Red StatusBar)
//
int TNewsFeed::Article (TErrorList * pErrList,
						int art_number,
						CString& Str,
						CString& ack,
						BOOL& fOK,
						int estimateLines,
						BOOL fPriority)

{
	int r =
		m_iEstimateLines = estimateLines;

	r = ArtCmd (pErrList,
		"ARTICLE", art_number,
		Str,
		ack,
		fOK,
		fPriority,
		TRUE          // user feedback
		);

	m_iEstimateLines  = 0;

	return r;
}

///////////////////////////////////////////////////////////////////////////
//  5-7-96  amc  Pass in a struct, pass out more info
//  Returns 0 for success
//
int  TNewsFeed::SetGroup (TErrorList * pErrList, LPFEED_SETGROUP psSG)
{
	CString cmdline = "GROUP ";

	psSG->fOK = FALSE;
	psSG->est = 0;
	psSG->first = 0;
	psSG->last = 0;

	cmdline += psSG->lpszNewsGroup;
	cmdline += "\r\n";

	// issue command
	if (m_pSocket->Write( pErrList, cmdline, cmdline.GetLength() ))
		return 1;

	// read the ack-line
	if (ReadLine ( pErrList, psSG->strAck ))
		return 1;

	if ( psSG->parseResults ( m_pSocket->getDottedAddr() ) )
	{
		return 0;
	}
	else
	{
		CString cleanAck(psSG->strAck);
		cleanAck.TrimRight();

		TError sErr (cleanAck, kError, kClassNNTP);

		sErr.SetDWError ( psSG->iRet );
		pErrList->PushError (sErr);

		return 1;
	}
}

// #define PULL_FROM_FILE 1

#ifdef PULL_FROM_FILE
CFile groupsFromFile ("groups", CFile::modeRead);
CArchive groupArc (&groupsFromFile, CArchive::load);
#endif

// ------------------------------------------------------------------------
//
int  TNewsFeed::StartList (TErrorList * pErrList, LPCTSTR cmd /* =NULL */,
						   int* piRet /*=0*/, CString* pAck/*=0*/)
{
	CString cmdline = (0==cmd) ? "LIST" : cmd;
	int     channel = 9999;
	int     ret;
	CString  ack;

#ifdef PULL_FROM_FILE
	return TRUE;
#endif

	// issue command
	cmdline += "\r\n";
	if (m_pSocket->Write(pErrList, cmdline, cmdline.GetLength() ))
		return 1;

	// read the ack-line
	if (ReadLine ( pErrList, ret, ack ))
		return 1;

	if (piRet)
		*piRet = ret;
	if (pAck)
		*pAck = ack;

	if (ret >=200 && ret <= 299)
		return 0;

	return 1;
}

// ------------------------------------------------------------------------
//
int  TNewsFeed::NextListLine(TErrorList * pErrList, CString * pStr)
{
#ifdef PULL_FROM_FILE
	BOOL result;
	result = groupArc.ReadString (*pStr);

	if (result)
		*pStr += "\r\n";
	return result;
#endif

	if (m_pSocket->ReadLine (pErrList, *pStr ))
		return 1;

	return 0;
}

////////////////////////////////////////////////////////////////////////
// Direct -- returns 0 for success.  used for "xhdr FROM 10-120"
//           roll your own cmd
int TNewsFeed::Direct(TErrorList * pErrList,
					  const CString& cmdLineIn,
					  int * piRet /* =0 */,
					  CString * pAck /* =0 */)
{
	CString cmdLine = cmdLineIn;
	CString ack;
	int     ret;

	// issue command
	cmdLine += "\r\n";
	if (m_pSocket->Write(pErrList, cmdLine, cmdLine.GetLength() ))
		return 1;

	// read the ack-line
	if (ReadLine ( pErrList, ack ))
		return 1;

	ret = atoi (LPCTSTR(ack));

	if (piRet)
		*piRet = ret;

	if (pAck)
		*pAck = ack;

	if (ret >= 200 && ret <= 299)
	{
		return 0;
	}

	TError sErr(ack, kError, kClassNNTP);

	pErrList->PushError (sErr);

	return 1;
}

////////////////////////////////////////////////////////////////////////
// DirectNext -- returns 0 for success
int TNewsFeed::DirectNext (TErrorList * pErrList, CString& rOutStr)
{
	return m_pSocket->ReadLine ( pErrList, rOutStr );
}

// ------------------------------------------------------------------
// ReadAnswer -- returns 0 for success
int TNewsFeed::ReadAnswer(
						  TErrorList * pErrList,
						  CString& strData,
						  bool fHandleDoublePeriods,
						  BOOL fPriority,
						  BOOL fUserFeedback
						  )
{
	int estLines = max(m_iEstimateLines, 4);
	estLines = min(estLines, 20000);
	int estBytes = estLines * 80;
	int ret;

	TMemorySpot spot(32768, estBytes);

	CString line;
	int count = 0;
	if (fPriority)
	{
		TUserDisplay_Auto statusAutoRefresh(TUserDisplay_Auto::kClearDisplay,
			TUserDisplay_Auto::kPriority);
		ret = m_pSocket->ReadAnswer( pErrList,
			&spot, fHandleDoublePeriods,
			progress_prio_cb, DWORD(0x00000) );
	}
	else if (fUserFeedback)
	{
		TUserDisplay_Auto statusAutoRefresh(TUserDisplay_Auto::kClearDisplay,
			TUserDisplay_Auto::kNormal);
		ret = m_pSocket->ReadAnswer( pErrList,
			&spot, fHandleDoublePeriods, progress_norm_cb );
	}
	else
		ret = m_pSocket->ReadAnswer( pErrList, &spot, fHandleDoublePeriods );

	// process the ..  (newssock does this for us now)
	//remove_periods(&spot);

	LPTSTR pdata = strData.GetBuffer(spot.size());
	CopyMemory(pdata, spot.getData(), spot.size());
	strData.ReleaseBuffer(spot.size());

	return ret;
}

// -------------------------------------------------------------------------
// if !fOK, then pRet contains server error
int  TNewsFeed::PostArticle (TErrorList * pErrList, CFile* pFile,
							 const CString& groupName,
							 BOOL& fOK, CString& strAck)
{
	fOK = FALSE;
	int channel = 999;
	int ret = 0;
	const int bufSz = 1024;
	CString cmdline = "POST\r\n";
	CString statusmsg;
	int bytesSent = 0;
	CString ack;

	pFile->SeekToBegin();

	// clear statusline when dtor
	TUserDisplay_Auto  auto_display(TUserDisplay_Auto::kClearDisplay,
		TUserDisplay_Auto::kNormal);

	CString fmt; fmt.LoadString(IDS_FORMAT_POSTSTATUS);
	DWORD len = pFile->GetLength();

	gpUserDisplay->SetRange(0, (int)len);
	statusmsg.Format (fmt, groupName, (int) len);
	gpUserDisplay->SetText ( statusmsg );

	// be careful, since the line may be dead now
	if (m_pSocket->Write (pErrList, cmdline, cmdline.GetLength() ))
		return 1;

	// read the ack-line
	if (ReadLine (pErrList, ack ))  // should respond 340
		return 1;

	ret = atoi (ack);

	if (!(ret >= 300 && ret <=399))
	{
		strAck = ack;
		TError sErr(ack, kError, kClassNNTP);

		pErrList->PushError (sErr);
		return 1;
	}
	else
	{
		LPTSTR pBuf = new TCHAR[bufSz];
		if (0==pBuf)
			throw(new TException(IDS_ERR_POSTING_OUT_OF_MEMORY, kFatal));
		auto_ptr<TCHAR> deleter(pBuf);

		int  chunk;

		// The file is _fully_ formatted.
		while (len > 0)
		{
			gpUserDisplay->SetPos ( bytesSent, FALSE );

			ZeroMemory(pBuf, bufSz*sizeof(TCHAR));
			if (len > bufSz)
			{
				chunk = pFile->Read(pBuf, bufSz);
				len -= chunk;
			}
			else
			{
				chunk = pFile->Read(pBuf, len);
				len -= chunk;
			}

			if (m_pSocket->Write (pErrList, pBuf, chunk ))
				return 1;

			bytesSent += chunk;
		}

		// read ack (expect - 240)
		if (ReadLine (pErrList, ack ))
			return 1;

		ret = atoi(ack);

		if (240 == ret)
		{
			fOK = TRUE;
			return 0;
		}
		else
		{
			strAck = ack;

			TError sErr(ack, kError, kClassNNTP);
			pErrList->PushError (sErr);

			return 1;
		}
	}
}

void TNewsFeed::XSubject (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR SUBJECT", art_number, Str, fOK);
}

void TNewsFeed::XFrom (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR FROM", art_number, Str, fOK);
}

void TNewsFeed::XReferences (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR References", art_number, Str, fOK);
}

void TNewsFeed::XMessageID (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR Message-ID", art_number, Str, fOK);
}

void TNewsFeed::XDate (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR DATE", art_number, Str, fOK);
}

void TNewsFeed::XFollow (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR Followup-To", art_number, Str, fOK);
	if (Str.CompareNoCase(szNone) == 0)
		Str.Empty();
}

void TNewsFeed::XNewsgroups (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK)
{
	X_Command(pErrList, "XHDR Newsgroups", art_number, Str, fOK);
	if (Str.CompareNoCase(szNone) == 0)
		Str.Empty();
}

void TNewsFeed::X_Command(TErrorList * pErrList, LPCTSTR cmd, int art_number,
						  CString& Str, BOOL& fOK)
{
	fOK = FALSE;
	CString cmdline;
	int     ret = 9999;
	CString ack;

	cmdline.Format("%s %d\r\n", cmd, art_number);

	// issue command
	if (m_pSocket->Write ( pErrList, cmdline, cmdline.GetLength() ))
		return;

	// read the ack-line
	if (ReadLine ( pErrList, ret, ack ))
		return;

	if (ret >= 200 && ret <= 299)
	{
		if (m_pSocket->ReadLine (pErrList, ack ))
			return;

		if (ack.GetLength() == 3 && ack[0] == '.')
		{
			// must be bad article
			Str.Empty();
		}
		else
		{
			// read <period><CRLF>
			if (m_pSocket->ReadLine (pErrList, cmdline ))
				return;

			fOK = TRUE;

			Str = ack.Mid(ack.Find(' ') + 1);
			int len = Str.GetLength();
			if (len >= 2)
				Str = Str.Left(len - 2);    // hack off CRLF
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//   4-24-96 amc Investigating the Exception we throw & simplify
//           This is used by Article ().
//
int TNewsFeed::ArtCmd (
					   TErrorList * pErrList,
					   LPCTSTR  cmd,
					   int      art_number,
					   CString& Str,
					   CString& ack,
					   BOOL&    fOK,
					   BOOL     fPriority,
					   BOOL     fUserFeedback /* == FALSE */
					   )
{
	fOK = FALSE;
	CString cmdline;
	int     ret = 9999;

	cmdline.Format("%s %d\r\n", cmd, art_number);

	// issue command
	if (m_pSocket->Write (pErrList, cmdline, cmdline.GetLength() ))
		return 1;

	// read the ack-line
	if (ReadLine (pErrList, ack ))
		return 1;

	ret = atoi(ack);

	if (ret >= 200 && ret <= 299)
	{
		if (ReadAnswer (pErrList, Str,
			true,             // deal with doubled periods
			fPriority, fUserFeedback))
			return 1;

		fOK = TRUE;
		return 0;
	}
	else
	{
		fOK = FALSE;

		CString strCleanAck(ack);  strCleanAck.TrimRight();

		TError sError (strCleanAck, kError, kClassNNTP);
		pErrList->PushError (sError);

		return 1;
	}
}

///////////////////////////////////////////////////////////////////////////
//  pass in the status bar range explicitly.  The caller will handle the
//  status bar. The caller will have to factor in the XHDR x-refs call
//  Caller also sets and clears the status bar
int TNewsFeed::Overview (TErrorList * pErrList, int low, int high,
						 T2StringList& StrList,
						 BOOL& fOK,
						 int*  pRetVal,
						 BOOL  fPriorityStatus,
						 MPBAR_INFO * psBarInfo)
{
	fOK = TRUE;
	CString cmdline;
	int     ret = 9999;
	CString ack;

	// RLW - Modified so we do not ask for more than 300 articles in one go
	int nStart = low;

	while (nStart <= high)
	{
		cmdline.Format("XOVER %d-%d\r\n", nStart, (nStart + 300 < high) ? nStart + 300 : high);
		//TRACE("%s\n", cmdline);

		// issue command
		if (!m_pSocket->Write(pErrList, cmdline, cmdline.GetLength()))
		{
			// read the ack-line
			if (!ReadLine(pErrList, ack))
			{
				ret = atoi(ack);

				if (pRetVal) *pRetVal = ret;

				if (ret >= 200 && ret <= 299)
				{
					ret = ReadOverviewStringList(pErrList, StrList, fPriorityStatus, psBarInfo);

					if (ret)
						fOK = FALSE;
				}
				else // Error
				{
					fOK = FALSE;

					TError sError(ack, kError, kClassNNTP);
					pErrList->PushError(sError);
				}
			}
			else
			{
				fOK = FALSE;
			}
		}
		else
		{
			fOK = FALSE;
		}

		nStart += 301;
	}

	return fOK == TRUE ? 0 : 1;

	//cmdline.Format("XOVER %d-%d\r\n", low, high);

	//// issue command
	//if (m_pSocket->Write ( pErrList, cmdline, cmdline.GetLength() ))
	//	return 1;

	//// read the ack-line
	//if (ReadLine (pErrList, ack ))
	//	return 1;

	//ret = atoi (ack);

	//if (pRetVal) *pRetVal = ret;

	//if (ret >= 200 && ret <= 299)
	//{
	//	ret = ReadOverviewStringList (pErrList, StrList, fPriorityStatus, psBarInfo );

	//	fOK = (ret == 0);

	//	return ret;
	//}
	//else
	//{
	//	fOK = FALSE;

	//	TError sError (ack, kError, kClassNNTP);
	//	pErrList->PushError (sError);

	//	return 1;
	//}
}

///////////////////////////////////////////////////////////////////////////
// Read into a string list.
int TNewsFeed::ReadOverviewStringList (TErrorList * pErrList,
									   T2StringList& StrList,
									   BOOL fPriorityStatus,
									   MPBAR_INFO * psBarInfo)
{
	int ret;
	CString line;
	for (;;)
	{
		ret = m_pSocket->ReadLine (pErrList, line );
		if (ret)
			return ret;

		int len = line.GetLength();
		if (len == 3 && '.' == line[0])
		{
			//  Stop on <PERIOD><CRLF>
			break;
		}
		else
		{
			StrList.push_back ( line );

			psBarInfo->m_nPos ++;

			if (psBarInfo->m_iLogSubRangeLen != -1)
			{
				int fraction = MulDiv (psBarInfo->m_iLogSubRangeLen, psBarInfo->m_nPos,
					psBarInfo->m_nSubRangeLen );
				if (fraction != -1)
				{
					fraction /= psBarInfo->m_nDivide;

					int iLogPos = psBarInfo->m_iLogPos + fraction;
					// user feedback
					gpUserDisplay->SetPos (iLogPos, FALSE, fPriorityStatus);
				}
			}
		}
	} // for loop

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
int TNewsFeed::Quit(TErrorList * pErrList)
{
	int r = 0;

	if (!m_pSocket->TransmitError())
	{
		// issue command
		r = m_pSocket->AsyncQuit (pErrList);

		// don't bother to read the ack-line
	}

	//TRACE0("newsfeed : NNTP Quit-ted\n");

	return r;
}

/////////////////////////////////////////////////////////////////////
//  Get fresh newsgroups
//    partner with TNewsFeed::Newgroups_Next
//
int TNewsFeed::Newgroups (TErrorList * pErrList, CTime& time, CString & ack)
{
	CString cmdline = time.Format("NEWGROUPS %y%m%d %H%M%S GMT\r\n");
	int     ret;

	// issue command
	if (m_pSocket->Write( pErrList, cmdline, cmdline.GetLength() ))
		return 1;

	// read the ack-line - return code should be 231
	if (ReadLine (pErrList, ret, ack ))
		return 1;

	if (ret >= 200 && ret <= 299)
		return 0;

	return 1;
}

// Global func
void TNewsFeed::progress_norm_cb(int lineCount, DWORD /*dw*/)
{
	// 11-14-95 do not force redraw
	gpUserDisplay->SetPos ( lineCount, FALSE );
}

// Global func
void TNewsFeed::progress_prio_cb(int lineCount, DWORD /*dummy*/)
{
	gpUserDisplay->SetPos ( lineCount,
		FALSE,      // don't draw immediately
		TRUE );     // priority
}

/////////////////////////////////////////////////////////////////////////////
int TNewsFeed::ModeReader (TErrorList * pErrList, int& rRet , CString& ack)
{
	CString cmdline = "mode reader\r\n";

	// issue command
	if (m_pSocket->Write (pErrList, cmdline, cmdline.GetLength() ))
		return 1;

	// read the response
	return ReadLine ( pErrList, rRet,  ack );
}

// ------------------------------------------------------------------------
// Returns 0 for success
//
int TNewsFeed::Init(
					TErrorList * pErrList,
					LPCTSTR  hostAddress,
					CString& strWelcome)
{
	int stat = 0;

	// Initialize our socket
	stat = m_pSocket->Connect( pErrList, hostAddress );
	if (stat)
		return stat;

	// Process the "Welcome line"
	m_pSocket->ReadLine ( pErrList, strWelcome );

	return pErrList->GetSize();
}  // Init

// uses kClassWinsock
int   TNewsFeed::ReadLine (TErrorList * pErrList, CString & str)
{
	return m_pSocket->ReadLine (pErrList, str);
}

int  TNewsFeed::ReadLine(TErrorList * pErrList, int& ret, CString& str)
{
	TNewsSocket & rSoc = *m_pSocket;

	if (rSoc.ReadNum (pErrList, ret))
		return 1;

	return m_pSocket->ReadLine (pErrList, str);
}

// uses kClassWinsock
int TNewsFeed::WriteLine (TErrorList * pErrList, LPCTSTR  buf, int bytes)
{
	return m_pSocket->WriteLine (pErrList, buf, bytes);
}

// ------------------------------------------------------------------------
// structure member function
BOOL FEED_SETGROUP::parseResults (const CString & strDottedAddr)
{
	ASSERT( !strAck.IsEmpty() );

	strAck.TrimRight ();

	istrstream strX(strAck, strAck.GetLength() );

	strX >> iRet;

	if ((iRet >= 200) && (iRet <= 299))
	{
		string  group;
		fOK = TRUE;

		strX >> est;

		strX >> first;
		strX >> last;
		strX >> group;

		strGroupName = group.c_str();
	}

	strAddr = strDottedAddr;

	return fOK;
}

// ------------------------------------------------------------------------
FEED_SETGROUP::~FEED_SETGROUP()
{
	if ((false == fNormalPump) || strAck.IsEmpty())
		return;

	// log to file here ??

	if (strLogFile.IsEmpty())
		return;

	CFileStatus    rStat;

	CStdioFile     sFile;
	CFileException sFE;

	bool fOpened = false;

	if (CFile::GetStatus (strLogFile, rStat))
	{

		if (sFile.Open (strLogFile, CFile::modeWrite, &sFE))
		{
			sFile.SeekToEnd ();
			fOpened = true;
		}
	}
	else
	{
		if (sFile.Open (strLogFile, CFile::modeCreate | CFile::modeWrite, &sFE))
		{
			fOpened = true;
		}
	}

	if (fOpened)
	{
		CString msg;

		msg.Format ("%s %sGRP %s >> %s\n",
			CTime::GetCurrentTime().Format ("%m-%d %H:%M:%S"),
			(LPCTSTR) strAddr,
			(LPCTSTR) lpszNewsGroup,
			(LPCTSTR) strAck );

		sFile.WriteString ( msg );
	}
}

