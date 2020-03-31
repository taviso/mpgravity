/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: smtp.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:51  richard_wood
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
#include "mplib.h"
#include "newssock.h"
#include "arttext.h"

#include "smtp.h"
#include "tglobopt.h"
#include "rgstor.h"
#include "evtlog.h"
#include "server.h"            // TNewsServer
#include "servcp.h"            // TServerCountedPtr
#include "globals.h"
#include "fileutil.h"          // GetWinsockErrorID

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TEventLog*    gpEventLog;
extern TGlobalOptions *gpGlobalOptions;

static int push_smtp_error (TErrorList * pErrList,
							const CString & cmd,
							int serverRet,
							const CString & strAck);
static int log_smtp_error (TErrorList * pErrList, const CString & str);

/**************************************************************************
Thread function to send mail out
**************************************************************************/
//  History
//  5-5-97   amc  Some servers have multiple response lines after HELO,
//                so I changed GetAnswer to loop thru comment lines
// 12-16-97  amc  HELO line uses (numeric) dotted address if it can

UINT SMTP_Mailer(LPVOID pVoid)
{
	TServerCountedPtr cpNewsServer;
	TSMTPSocket sk;
	int         iRet;
	CString     line;
	TErrorList  sErrList;

	sk.SetPort (cpNewsServer->GetSmtpServerPort ());

	TEmailMsgPool* pMsgPool = (TEmailMsgPool*) pVoid;

	pMsgPool->m_Completed = FALSE;
	iRet = sk.Connect ( &sErrList, pMsgPool->m_smtpServer );
	if (0 != iRet)
	{
		CString str; str.LoadString (IDS_ERR_SMTP_CONNECTION);

		log_smtp_error ( &sErrList, str );

		SetEvent (pMsgPool->m_hEventDone);
		pMsgPool->m_eError = kConnectFailure;
		return 0;
	}

	// read the welcome line... keep reading until we get a non-comment line
	do {
		if (sk.ReadLine ( &sErrList, line ))
		{
			CString strContext;
			log_smtp_error ( &sErrList, strContext );

			SetEvent (pMsgPool->m_hEventDone);
			pMsgPool->m_eError = kConnectFailure;
			return 0;
		}
	} while (line.GetLength() >= 4 && line [3] == '-');

	LPCTSTR pstr = line;
	int code = atoi(pstr);
	if (code != 220)  // code441 "retry later; close connection"
	{
		sk.WriteLine( &sErrList, "QUIT", 4);
		SetEvent (pMsgPool->m_hEventDone);
		pMsgPool->m_eError = kMTA_Unavailable;
		return 0;      // end thread
	}

	// Ok the socket is online
	try
	{
		TSMTPMailer mailer( &sErrList, &sk );
		CString strHELOanswer;
		CString strEHLOanswer;
		int code = 0;
		CString ack;

		// see if we need SMTP logon
		int iNeedLogon = cpNewsServer->GetSmtpLogonStyle();
		CString usr = cpNewsServer->GetSmtpLogonUser();
		CString pwd = cpNewsServer->GetSmtpLogonPass();

		if (0 == iNeedLogon)
		{

			if (mailer.Hello( pMsgPool->m_domain, code, strHELOanswer ) ||
				(code != 250))
			{
				CString msg = "HELO";
				log_smtp_error (&sErrList, msg);

				pMsgPool->m_eError     = kHelloFailure;
				pMsgPool->m_serverCode = code;
				pMsgPool->m_strServerAck = strHELOanswer;
				SetEvent(pMsgPool->m_hEventDone);
				return 0;
			}
		}
		else
		{
			CString  strAuthLine;

			if (mailer.EHLO ( pMsgPool->m_domain, code, strEHLOanswer, strAuthLine ) ||
				(code != 250))
			{
				CString msg = "EHLO";
				log_smtp_error (&sErrList, msg);

				pMsgPool->m_serverCode = code;
				pMsgPool->m_strServerAck = strEHLOanswer;
				SetEvent(pMsgPool->m_hEventDone);
				return 0;
			}

			if (0 != mailer.CanSupportLogin(strAuthLine))
			{
				CString desc = "Gravity doesn't understand these login protocols: ";
				desc += strAuthLine;
				desc.TrimRight();

				TError sError(desc, kError, kClassSMTP);
				sErrList.PushError (sError);

				log_smtp_error (&sErrList, "SMTP server login");

				pMsgPool->m_serverCode = 0;
				pMsgPool->m_strServerAck = strAuthLine;
				SetEvent(pMsgPool->m_hEventDone);

				return 0;
			}

			code = 0;
			if (mailer.Login (usr, pwd, strAuthLine, code, ack) || (code != 235))
			{
				CString msg = "Authorization";
				log_smtp_error (&sErrList, msg);

				pMsgPool->m_serverCode = code;
				pMsgPool->m_strServerAck = ack;

				SetEvent(pMsgPool->m_hEventDone);
				return 0;
			}

			// I guess we are logged in
		}

		bool fSent;
		for (int i = 0; i < pMsgPool->m_bindArray.GetSize(); ++i)
		{
			TEmailMsgBind& rBind = pMsgPool->m_bindArray.ElementAt(i);

			int r = mailer.SendOneMessage (cpNewsServer,
				rBind.m_pHdr,
				rBind.m_pText,
				&rBind.m_error,
				&rBind.m_ret,
				&rBind.m_errString,
				fSent);
			if (1 == r)
			{
				line = "";
				log_smtp_error (&sErrList, line);
			}
		}
	}
	catch (TException *except)
	{
		except->Display();
		except->Delete();
		SetEvent(pMsgPool->m_hEventDone);
		return 0;
	}

	sk.WriteLine( &sErrList, "QUIT", 4);
	TRACE0("smtp thread ended successfully\n");

	pMsgPool->m_Completed = TRUE;
	pMsgPool->m_eError    = kOK;
	SetEvent (pMsgPool->m_hEventDone);
	return 0;
}

// ctor
TSMTPMailer::TSMTPMailer(TErrorList * pErrList, TSMTPSocket* pSoc)
: m_pSoc(pSoc)
{
	m_pErrList = pErrList;
}

// dtor
TSMTPMailer::~TSMTPMailer(void)
{
}

// ------------------------------------------------------------------------
// Returns 0 for success
int  TSMTPMailer::Hello(const CString& domain, int& code, CString& strHELOanswer)
{
	CString line = "HELO ";
	CString strIPAddr;

	// Try to get the numeric IP addr of local host
	if (0 == m_pSoc->GetLocalIPAddress (strIPAddr))
		line += strIPAddr;
	else
		line += domain;        //  ex:  "MicroPlanet-Gravity.microsoft.com"

	if (m_pSoc->WriteLine( m_pErrList, line, line.GetLength() ))
		return 1;

	return GetAnswer ( code, strHELOanswer, NULL );
}

// ------------------------------------------------------------------------
// Returns 0 for success
int  TSMTPMailer::EHLO(const CString& domain, int& code, CString& strEHLOanswer,
					   CString & strAuthLine)
{
	CString line = "EHLO ";
	CString strIPAddr;

	// Try to get the numeric IP addr of local host
	if (0 == m_pSoc->GetLocalIPAddress (strIPAddr))
		line += strIPAddr;
	else
		line += domain;        //  ex:  "MicroPlanet-Gravity.microsoft.com"

	if (m_pSoc->WriteLine( m_pErrList, line, line.GetLength() ))
		return 1;

	return GetAnswer ( code, strEHLOanswer, &strAuthLine );
}

////////////////////////////////////////////////////////////////////////////
// Returns 0 for success.
//         1  -  caller should call log_smtp_error
//         2  -  caller should fail
//
BOOL TSMTPMailer::SendOneMessage (
								  TNewsServer*     pServer,
								  TEmailHeader*    pHdr,
								  TEmailText*      pText,
								  ESmtpError*  peError,
								  int*         pCode,
								  CString*     pErrString,
								  bool &       fSent
								  )
{
	CString target;
	int i;
	fSent = false;

	CMemFile memFile;
	fnPrepareReply ( pHdr, pText, &memFile );         // final form

	CString ack;
	CString line = "MAIL FROM: ";
	int serverCode;

	///  from  <jsmith@netcom.com>

	CString clean;

	CString strAddress = pServer -> GetEmailAddress ();

	make_clean_email_addr(strAddress, clean);

	line += clean;

	if (m_pSoc->WriteLine (m_pErrList, line, line.GetLength()))
		return 1;

	if (GetAnswer ( serverCode, ack ))
		return 1;

	if (250 != serverCode)
	{
		*peError = kFromFailure;
		*pCode = serverCode;
		*pErrString = ack;

		push_smtp_error (m_pErrList, line, serverCode, ack);

		write_file( &memFile );
		return 2;
	}

	///  to
	TStringList strList;
	pHdr->GetDestinationList(TBase822Header::kAddrTo, &strList);
	POSITION pos = strList.GetHeadPosition();
	while (pos)
	{
		line = "RCPT TO: ";
		target = strList.GetNext (pos);
		if (target.IsEmpty())
			continue;

		make_clean_email_addr(target, clean);
		line += clean;

		if (m_pSoc->WriteLine (m_pErrList, line, line.GetLength()))
			return 1;

		if (GetAnswer ( serverCode, ack ))
			return 1;

		if ( serverCode / 10 != 25 )
		{
			*peError = kRecipFailure;
			*pCode = serverCode;
			*pErrString = ack;

			push_smtp_error (m_pErrList, line, serverCode, ack);

			VoidThisTransaction();
			write_file( &memFile );
			return 2;
		}
	}

	///  carbon-copy
	pHdr->GetDestinationList(TBase822Header::kAddrCC, &strList);
	pos = strList.GetHeadPosition();
	while (pos)
	{
		line = "RCPT TO: ";
		target = strList.GetNext (pos);

		make_clean_email_addr(target, clean);
		line += clean;

		if (m_pSoc->WriteLine (m_pErrList, line, line.GetLength()))
			return 1;

		if (GetAnswer ( serverCode, ack ))
			return 1;

		if ( serverCode / 10 != 25 )
		{
			*peError = kRecipCCFailure;
			*pCode = serverCode;
			*pErrString = ack;

			push_smtp_error (m_pErrList, line, serverCode, ack);

			VoidThisTransaction();
			write_file( &memFile );
			return 2;
		}
	}

	/// secret blind carbon copy
	pHdr->GetDestinationList(TBase822Header::kAddrBCC, &strList);
	pos = strList.GetHeadPosition();
	while (pos)
	{
		line = "RCPT TO: ";
		target = strList.GetNext (pos);

		make_clean_email_addr (target, clean);
		line += clean;

		if (m_pSoc->WriteLine (m_pErrList, line, line.GetLength()))
			return 1;

		if (GetAnswer ( serverCode, ack ))
			return 1;

		if ( serverCode / 10 != 25 )
		{
			*peError = kRecipBCCFailure;
			*pCode = serverCode;
			*pErrString = ack;

			push_smtp_error (m_pErrList, line, serverCode, ack);

			VoidThisTransaction();
			write_file( &memFile );
			return 2;
		}
	}

	/// DATA cmd
	line = "DATA";
	if (m_pSoc->WriteLine (m_pErrList, line, line.GetLength()))
		return 1;

	if (GetAnswer (serverCode, ack))   /// 354 means "Ready for Content"
		return 1;

	if (354 != serverCode)
	{
		*peError = kDataFailure;
		*pCode = serverCode;
		*pErrString = ack;

		push_smtp_error (m_pErrList, line, serverCode, ack);

		VoidThisTransaction();
		write_file( &memFile );
		return FALSE;
	}

	// ready to blast the message
	int total = int(memFile.GetLength());
	memFile.SeekToBegin();
	int chunk = 0;
	for (i = 0; i < total; i+= chunk)
	{
		if (total - i <= sizeof(buf))
			chunk = total - i;
		else
			chunk = sizeof(buf);
		memFile.Read(buf, chunk);
		if (m_pSoc->Write (m_pErrList, buf, chunk))
			return 1;
	}
	//m_pSoc->Flush();

	if (GetAnswer (serverCode, ack))
		return 1;

	if (250 != serverCode)
	{
		*peError = kFinalFailure;
		*pCode = serverCode;
		*pErrString = ack;

		CString cmd = "Sending mail msg";

		push_smtp_error (m_pErrList, cmd, serverCode, ack);

		write_file( &memFile );
		return 2;
	}

	*peError = kOK;
	*pCode   = 250;
	fSent    = true;

#if defined(_DEBUG)
	write_file( &memFile );
#endif

	// all done
	return 0;
}

// utility function

void TSMTPMailer::VoidThisTransaction(void)
{
	int serverCode;
	CString line;

	m_pSoc->WriteLine(m_pErrList, "RSET", 4);
	GetAnswer ( serverCode, line);
}

// --------------------------------------------------------------------------
//  5-5-97  amc ignore comment lines
int  TSMTPMailer::GetAnswer (int& code, CString& response,
							 CString * pStrAuthLine /* =0 */)
{
	CString tmp;

	// keep reading until we get a non-comment line
	do
	{
		response.Empty ();

		if (m_pSoc->ReadLine (m_pErrList, response ))
			return 1;

		if (pStrAuthLine)
		{
			tmp = response.Mid (4, 4);

			if (tmp.CompareNoCase("AUTH") == 0)
				*pStrAuthLine = response.Mid (4);
		}

	} while ((response.GetLength() >= 4) && ('-' == response[3]));

	if (pStrAuthLine)
	{
		tmp = response.Mid (4, 4);

		if (tmp.CompareNoCase("AUTH") == 0)
			*pStrAuthLine = response.Mid (4);
	}

	LPCTSTR pstr = response;
	code = atoi(pstr);

	// purely cosmetic
	response.TrimRight();
	return 0;
}

/////////////////////////////////////////////////////////////

TEmailMsgPool::TEmailMsgPool(void)
{
	m_serverCode = 0;
	m_Completed = FALSE;
}

TEmailMsgPool::~TEmailMsgPool(void)
{
	Reset();
}

void TEmailMsgPool::Reset()
{
	m_bindArray.RemoveAll();

	m_serverCode = 0;
	m_strServerAck.Empty();

	m_Completed = FALSE;
}

///////////////////////////////////////////////////////////////////////////
//
//
void TEmailMsgPool::AddMsg(TEmailHeader* pHdr, TEmailText* pText)
{
	TEmailMsgBind sBind(pHdr, pText);
	m_bindArray.Add( sBind );
	sBind.ReleaseOwnership(); // array owns the pointers

	// destructor happens on sBind
}

///////////////////////////////////////////////////////////////////////////
// Flesh out the TEmailMsgBind class
//
TEmailMsgBind::TEmailMsgBind()
{
	m_pHdr = 0;
	m_pText = 0;
	m_error = kInitial;
	m_ret   = 1000;
}

// ctor with arguments
TEmailMsgBind::TEmailMsgBind(TEmailHeader* pHdr, TEmailText* pText)
: m_pHdr(pHdr), m_pText(pText)
{
	ASSERT(pHdr);
	ASSERT(pText);
	m_error = kInitial;
	m_ret   = 1000;
}

// destructor
TEmailMsgBind::~TEmailMsgBind()
{
	delete m_pHdr;
	delete m_pText;
}

TEmailMsgBind& TEmailMsgBind::operator=(const TEmailMsgBind& rhs)
{
	if (this != &rhs)
	{
		m_pHdr = rhs.m_pHdr;
		m_pText = rhs.m_pText;
		m_error = rhs.m_error;
		m_ret   = rhs.m_ret;
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////
// ??????? should use straight C for this instead of CString crap
//
//
void TSMTPMailer::make_clean_email_addr(
										const CString& emailAddr,
										CString& rClean
										)
{
	CString temp2;
	CString no_comment;

	int      i;

	// remove any spaces
	for (i = 0; i < emailAddr.GetLength(); i++)
	{
		if (emailAddr[i] != ' ')
			temp2 += emailAddr[i];
	}

	// hack off any comment

	if ((i = temp2.Find ('(')) >= 0)
		no_comment = temp2.Left(i);
	else
		no_comment = temp2;

	make_bracket_addr(no_comment, rClean);
}

///////////////////////////////////////////////////////////////////////////
// change   al@microplanet.com
// to:      <al@microplanet.com>
//
void TSMTPMailer::make_bracket_addr(
									const CString& addr,
									CString& rBracket
									)
{
	int iLen = addr.GetLength();
	if (iLen >= 2)
	{
		if (('<' == addr[0]) && ('>' == addr[iLen-1]))
			rBracket = addr;
		else
		{
			rBracket = "<";
			rBracket += addr;
			rBracket += ">";
		}
	}
	else
		rBracket = addr;
}

void TSMTPMailer::write_file(CFile* pFile)
{
	CString dumpFile = gpGlobalOptions->GetRegStorage()->GetMailDumpFile();
	if (!dumpFile.IsEmpty())
	{
		pFile->SeekToBegin();
		CFile fl;

		if (fl.Open(dumpFile,
			CFile::modeCreate | CFile::modeReadWrite
			| CFile::shareDenyWrite))
		{
			char c;
			while (pFile->Read(&c, 1))
				fl.Write(&c, 1);
		}
		pFile->SeekToBegin();
	}
}

// -------------------------------------------------------------------------
int push_smtp_error (
					 TErrorList *    pErrList,
					 const CString & cmd,
					 int             serverCode,
					 const CString & strAck)
{
	CString rbuild; rbuild.Format ("%d %s", serverCode, LPCTSTR(strAck));

	TError sErr(rbuild, kError, kClassSMTP);
	pErrList->PushError (sErr);

	return log_smtp_error (pErrList, cmd);
}

// -------------------------------------------------------------------------
int log_smtp_error (TErrorList * pErrList, const CString & strContext)
{
	if (pErrList->GetSize() == 0)
		return 0;

	ESeverity   eSeverity;
	EErrorClass eClass;
	DWORD       dwError;
	CString     desc;

	int         iStringID;
	CString     details;             // event log details

	pErrList->GetErrorByIndex (0, eSeverity, eClass, dwError, desc);

	switch (eClass)
	{
	case kClassWinsock:
		iStringID = GetWinsockErrorID (int(dwError));

		if (iStringID)
			details.LoadString (iStringID);
		break;

	default:
		if (desc.IsEmpty())
			details.Format (IDS_SMTP_ERROR, (int) dwError);  // just show #
		else
			details = desc;
		break;
	}

	gpEventLog->AddError (TEventEntry::kGeneral, strContext, details);

	return 0;
}

// dirty prototype
extern ULONG String_base64_encode (PBYTE pInBuf, DWORD nInSz, CString & strOutput);

// ------------------------------------------------------------------------
int TSMTPMailer::Login (
						CString & usr,
						CString & pwd,
						CString & strAuthLine,
						int & code,
						CString & ack)
{
	strAuthLine.MakeLower();

	TCHAR rcBuf[800];

	CString strB64;
	CString line;
	int    ret;

	if (strAuthLine.Find("plain") >= 0)
	{
		// we want  NULL  username  NULL  password

		// do plain
		wsprintf(rcBuf, "%c%s%c%s", 1, LPCTSTR(usr), 1, (LPCTSTR) pwd);

		int ln = lstrlen(rcBuf);

		int i;
		for (i = 0; i < ln; i++)
			if (rcBuf[i] == 1)
				rcBuf[i] = 0;

		String_base64_encode (PBYTE(rcBuf), ln,  strB64);

		line = "AUTH PLAIN ";
		line += strB64;

		if (m_pSoc->WriteLine ( m_pErrList, line, line.GetLength() ))
			return 1;

	}
	else if (strAuthLine.Find("login") >= 0)
	{
		// do login

		line = "AUTH LOGIN";

		if (m_pSoc->WriteLine ( m_pErrList, line, line.GetLength() ))
			return 1;

		ret = GetAnswer ( code, ack, NULL );

		if (ret || (code != 334))
			return ret;

		// send user name

		strB64.Empty();
		String_base64_encode (PBYTE(LPCTSTR(usr)), usr.GetLength(),  strB64);

		if (m_pSoc->WriteLine ( m_pErrList, strB64, strB64.GetLength() ))
			return 1;

		ret = GetAnswer ( code, ack, NULL );
		if (ret || (code != 334))
			return ret;

		// send passwd

		strB64.Empty();
		String_base64_encode (PBYTE(LPCTSTR(pwd)), pwd.GetLength(), strB64);

		if (m_pSoc->WriteLine ( m_pErrList, strB64, strB64.GetLength() ))
			return 1;
	}
	else
	{

	}

	return GetAnswer (code, ack, NULL);
}

int TSMTPMailer::CanSupportLogin (const CString & strAuthline)
{
	CString tst = strAuthline;
	tst.MakeLower();

	if (tst.Find ("plain") >= 0)
		return 0;

	if (tst.Find ("login") >= 0)
		return 0;

	return 1;
}