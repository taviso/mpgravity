/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgserv.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:45  richard_wood
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
#include "appversn.h"
#include "resource.h"
#include "mplib.h"
#include "rgserv.h"
#include "regutil.h"
#include <memory>                   // STL header
#include "crypt.h"                  // simple encryption

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// disable warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

TRegServer::GRPSYNC  TRegServer::m_grpMap[] =
{
	{ IDS_REG_GRPSONDEMAND,   TGlobalDef::kGroupsOnDemand},
	{ IDS_REG_GRPSONCONNECT,  TGlobalDef::kGroupsUponConnecting}
};

TRegServer::TRegServer()
: m_NewsGroup_Check(1972, 1, 1, 0, 0, 0)
{
	default_values();
	m_itemCount = 59;

	m_pTable = new TableItem[m_itemCount];
	SetItem(m_pTable + 0, "DatabasePath",   kREGString, &m_serverDatabasePath);
	SetItem(m_pTable + 1, "NewsServer",     kREGString, &m_newsServerName);
	SetItem(m_pTable + 2, "NewsServerAddr", kREGString, &m_newsServerAddr, TRUE);
	SetItem(m_pTable + 3, "NewsServerPort", kREGLong,   &m_newsServerPort);
	SetItem(m_pTable + 4, "SMTPServer",     kREGString, &m_smtpServer);
	SetItem(m_pTable + 5, "SMTPServerPort", kREGLong,   &m_smtpServerPort);
	SetItem(m_pTable + 6, "FullName",       kREGString, &m_fullName);
	SetItem(m_pTable + 7, "Email",          kREGString, &m_emailAddress);
	SetItem(m_pTable + 8, "ReplyTo",        kREGString, &m_replyTo, TRUE); // optional
	SetItem(m_pTable + 9, "Organization",   kREGString, &m_organization);
	SetItem(m_pTable +10, "NewsAccount",    kREGString, &m_accountName);
	SetItem(m_pTable +11, "NewsPswd",       kREGString, &m_passwordBuffer);
	SetItem(m_pTable +12, "ConnectAtStartup", kREGBool, &m_fConnectAtStartup);
	SetItem(m_pTable +13, "CheckInterval",  kREGInt,    &m_newsCheckInterval);
	SetItem(m_pTable +14, "ShowNewGroups",  kREGBool,   &m_fDisplayIncomingGroups);
	SetItem(m_pTable +15, "Retrieve",       kREGInt,    &m_iGoBack);
	SetItem(m_pTable +16, "KeepAlive",      kREGBool,   &m_fSendKeepAliveMsgs);
	SetItem(m_pTable +17, "AliveMins",      kREGInt,    &m_iKeepAliveMinutes);
	SetItem(m_pTable +18, "Cycle",          kREGBool,   &m_fAutomaticCycle);
	SetItem(m_pTable +19, "GroupListExist", kREGBool,   &m_fGroupListExist);
	SetItem(m_pTable +20, "MemoryExist",    kREGBool,   &m_fMemoryExist);
	SetItem(m_pTable +21, "GroupCheckTime", kREGTime,   &m_NewsGroup_Check);
	SetItem(m_pTable +22, "DialupConnection", kREGString, &m_connectionName);
	SetItem(m_pTable +23, "DialupForceConnect", kREGBool,   &m_fForceConnection);
	SetItem(m_pTable +24, "DialupUseExisting",  kREGBool,   &m_fUseExistingConnection);
	SetItem(m_pTable +25, "DialupPromptConnect", kREGBool,   &m_fPromptBeforeConnecting);
	SetItem(m_pTable +26, "DialupPromptDisconnect", kREGBool,   &m_fPromptBeforeDisconnecting);
	SetItem(m_pTable +27, "DialupDisconnectOurs", kREGBool,   &m_fDisconnectIfWeOpened);
	SetItem(m_pTable +28, "DialupForceDisconnect", kREGBool,   &m_fForceDisconnect);
	SetItem(m_pTable +29, "Flags",           kREGLong, &m_dwServerFlags);
	SetItem(m_pTable +30, "AutoDisconnect", kREGBool,   &m_fAutoQuit, TRUE);
	SetItem(m_pTable +31, "DisconnectMins", kREGInt,    &m_iAutoQuitMinutes, TRUE);
	SetItem(m_pTable +32, "RetrieveTags", kREGBool, &m_fAutoRetrieveTagged, TRUE);
	SetItem(m_pTable +33, "UpdateGroupCount", kREGInt, &m_iUpdateGroupCount, TRUE);
	SetItem(m_pTable +34, "Import",              kREGBool, &m_fImport, TRUE);
	SetItem(m_pTable +35, "Export",              kREGBool, &m_fExport, TRUE);
	SetItem(m_pTable +36, "ExportSubscribedOnly",kREGBool, &m_fExportSubscribedOnly, TRUE);
	SetItem(m_pTable +37, "ImportFile",          kREGString, &m_strImportFile);
	SetItem(m_pTable +38, "ExportFile",          kREGString, &m_strExportFile);
	SetItem(m_pTable +39, "OptionsPg",           kREGInt, &m_iPropPage);
	SetItem(m_pTable +40, "FullCrossPost",kREGBool, &m_fFullCrossPost, TRUE);
	SetItem(m_pTable +41, "ExpirePhase", kREGBool, &m_fExpirePhase, TRUE);
	SetItem(m_pTable +42, "SubscribeDL", kREGBool, &m_fSubscribeDL, TRUE);
	SetItem(m_pTable +43, "NewsServerLogonStyle", kREGInt, &m_iLogonStyle);
	SetItem(m_pTable +44, "EmailForPosting",     kREGString, &m_emailAddressForPost, TRUE);
	SetItem(m_pTable +45, "LimitHdrs",        kREGBool, &m_fLimitHeaders, TRUE);
	SetItem(m_pTable +46, "LimitHdrsN",       kREGInt, &m_lHeadersLimit, TRUE);
	SetItem(m_pTable +47, "GenMsgID",         kREGBool, &m_fGenerateMsgID, TRUE);
	SetItem(m_pTable +48, "NewsFarm",         kREGBool, &m_fServerFarm, TRUE);
	SetItem(m_pTable +49, "PurgeOutbox",      kREGBool, &m_fPurgeOutbox, TRUE);
	SetItem(m_pTable +50, "PurgeOutboxDays",  kREGInt,  &m_iPurgeOutboxDays, TRUE);
	SetItem(m_pTable +51, "fRetrying",        kREGBool, &m_fRetrying,   TRUE);
	SetItem(m_pTable +52, "RetryCount",       kREGInt,  &m_iRetryCount, TRUE);
	SetItem(m_pTable +53, "fPausing",         kREGBool, &m_fPausing,    TRUE);
	SetItem(m_pTable +54, "iPauseCount",      kREGInt,  &m_iPauseCount, TRUE);
	SetItem(m_pTable +55, "ModeReader",       kREGBool, &m_fModeReader, TRUE);

	SetItem(m_pTable +56, "MailServerLogonStyle", kREGInt, &m_iSmtpLogonStyle, TRUE);
	SetItem(m_pTable +57, "MailLogonUser",        kREGString, &m_strSmtpLogonUser, TRUE);
	SetItem(m_pTable +58, "MailLogonPswd",        kREGString, &m_passwordBuffer2, TRUE);

	ASSERT(59 == m_itemCount);
}

// ------------------------------------------------------------------------
TRegServer::~TRegServer()
{
	delete [] m_pTable;
}

// ------------------------------------------------------------------------
int TRegServer::Load(LPCTSTR server)
{
	CString  keyPath;
	keyPath.Format ("%s%s\\%s", GetGravityRegKey(), "Servers", server);
	return TRegistryBase::Load (keyPath);
}

// ------------------------------------------------------------------------
int TRegServer::Save(LPCTSTR server)
{
	CString  keyPath;
	keyPath.Format ("%s%s\\%s", GetGravityRegKey(), "Servers", server);
	return TRegistryBase::Save (keyPath);
}

// ------------------------------------------------------------------------
void TRegServer::read()
{
	upgrade ();

	LONG lRet;
	DWORD dwType;
	DWORD dwValueSize;
	TCHAR rcBuf[50];
	int   i, tot;

	lRet = rgReadTable (m_itemCount, m_pTable);

#ifdef LITE
	// lite users have no choice
	m_fFullCrossPost = FALSE;
#endif

	dwValueSize=sizeof(rcBuf);
	lRet = RegQueryValueEx(m_hkKey, "GroupSchedule", 0, &dwType,
		(BYTE*) rcBuf, &dwValueSize);
	tot = sizeof(m_grpMap)/sizeof(m_grpMap[0]);
	CString s;
	for (i=0; i < tot; i++)
	{
		s.LoadString(m_grpMap[i].strID);
		if (0 == s.CompareNoCase((LPCTSTR)rcBuf))
		{
			m_kGroupSchedule = m_grpMap[i].m_eGrpSched;
			break;
		}
	}

	// Be Robust
	if (i >= tot)
		m_kGroupSchedule = TGlobalDef::kGroupsUponConnecting;

	// NewsPswd is stored with simple encryption
	make_plain_text (m_passwordBuffer, m_accountPassword);
	m_passwordBuffer.Empty();

	// smtp passwd is encrypted too, so decrypt it
	make_plain_text (m_passwordBuffer2, m_strSmtpLogonPwd);
	m_passwordBuffer2.Empty();
}

// ------------------------------------------------------------------------
// 12-04-97  amc  Robust; no throw exception
void TRegServer::write()
{
	// do simple encryption.  passwordBuffer is used by Read/WriteTable
	m_passwordBuffer.Empty ();
	make_cipher_text (m_accountPassword, m_passwordBuffer);

	// encrypt smtp pwd too
	m_passwordBuffer2.Empty ();
	make_cipher_text (m_strSmtpLogonPwd, m_passwordBuffer2);

	// write out
	LONG lRet = rgWriteTable (m_itemCount, m_pTable);

	m_passwordBuffer.Empty ();

	int i, tot, iStrID = 0;
	tot = sizeof(m_grpMap)/sizeof(m_grpMap[0]);
	for (i = 0; i < tot; ++i)
	{
		if (m_kGroupSchedule == m_grpMap[i].m_eGrpSched)
		{
			iStrID = m_grpMap[i].strID;
			break;
		}
	}

	// be Robust
	if (0 == iStrID)
		iStrID = IDS_REG_GRPSONCONNECT;

	CString s;
	s.LoadString(iStrID);
	lRet = rgWriteString("GroupSchedule", s);
}

///////////////////////////////////////////////////////////////////////////
// 2-4-96  Connect at Startup is now the default
//
void TRegServer::default_values()
{
	m_newsServerPort           = 119;
	m_smtpServerPort           = 25;
	m_kGroupSchedule           = TGlobalDef::kGroupsOnDemand;
	m_fDisplayIncomingGroups   = FALSE;
	m_iGoBack                  = 1000;
	m_fConnectAtStartup        = TRUE;
	m_fSendKeepAliveMsgs       = TRUE;
	m_iKeepAliveMinutes        = 15;
	m_fAutomaticCycle          = FALSE;
	m_newsCheckInterval        = 30;

	m_fGroupListExist          = FALSE;
	m_fMemoryExist             = FALSE;

	m_fForceConnection            = FALSE;
	m_fUseExistingConnection      = TRUE;
	m_fPromptBeforeConnecting     = TRUE;
	m_fPromptBeforeDisconnecting  = TRUE;
	m_fDisconnectIfWeOpened       = TRUE;
	m_fForceDisconnect            = FALSE;

	// m_NewsGroup_Check is set in constructor

	m_dwServerFlags = (DWORD) TRegServer::kPostOK;

	m_fAutoQuit = FALSE;
	m_iAutoQuitMinutes = 5;       // disconnect after 5 minutes of No traffic

	m_fAutoRetrieveTagged = FALSE; // False is consistent with 1.01.500
	m_iUpdateGroupCount = 2;       // "upon connecting"

	m_fImport               = FALSE;
	m_fExport               = FALSE;
	m_fExportSubscribedOnly = TRUE;
	m_iPropPage = 0;
	m_iProxicomServer       = 0;      // not serialized !
	m_fFullCrossPost        = FALSE;
	m_fExpirePhase          = FALSE;
	m_fSubscribeDL          = TRUE;
	m_iLogonStyle           = 0;      // authentication not required

	m_fLimitHeaders         = FALSE;
	m_lHeadersLimit         = 500;
	m_fGenerateMsgID        = TRUE;
	m_fServerFarm           = FALSE;

	m_fPurgeOutbox          = FALSE;
	m_iPurgeOutboxDays      = 7;

	m_fPausing              = TRUE;
	m_fRetrying             = TRUE;
	m_iPauseCount           = 10;
	m_iRetryCount           = 5;
	m_fModeReader           = TRUE;

	m_iSmtpLogonStyle       = 0;
}

int TRegServer::GetGoBackArtcount()
{
	return m_iGoBack;
}

void TRegServer::SetGoBackArtcount(int count)
{  m_iGoBack = count; }

void TRegServer::SetSendKeepAliveMsgs(BOOL fKeepAlive)
{
	m_fSendKeepAliveMsgs = fKeepAlive;
}

int  TRegServer::GetKeepAliveMinutes()
{
	return m_iKeepAliveMinutes;
}

void TRegServer::SetKeepAliveMinutes(int iDelayMinutes)
{
	m_iKeepAliveMinutes = iDelayMinutes;
}

BOOL TRegServer::GetAutomaticCycle ()
{
	return m_fAutomaticCycle;
}

void TRegServer::SetAutomaticCycle (BOOL fCycle)
{
	m_fAutomaticCycle = fCycle;
}

void TRegServer::GetNewsGroupCheckTime(CTime& tm)
{
	tm = m_NewsGroup_Check;
}

void TRegServer::SetNewsGroupCheckTime(const CTime& tm)
{
	m_NewsGroup_Check = tm;
}

BOOL TRegServer::GetPostingAllowed ()
{
	return (m_dwServerFlags & TRegServer::kPostOK) ? TRUE : FALSE;
}

void  TRegServer::SetPostingAllowed (BOOL fPostAllowed)
{
	if (fPostAllowed)
		m_dwServerFlags |= kPostOK;
	else
		m_dwServerFlags &= ~kPostOK;
}

// returns true if the XRef information is contained in the XOver information
bool TRegServer::XOverXRef ()
{
	return (m_dwServerFlags & kXOverXRef) ? true : false;
}

void TRegServer::XOverXRef (bool fXRef)
{
	if (fXRef)
		m_dwServerFlags |= kXOverXRef;
	else
		m_dwServerFlags &= ~kXOverXRef;
}

///////////////////////////////////////////////////////////////////////////
// upgrade -- upgrade registry format as needed
void TRegServer::upgrade()
{
	// these two were added after build 602
	EnsureStringExists ("ImportFile");
	EnsureStringExists ("ExportFile");

	// use smart ptr
	const int iBufSz = 128;
	std::auto_ptr<BYTE> sPtr(new BYTE[iBufSz]);
	LONG lStyle = 0;
	LPCTSTR szStyle = "NewsServerLogonStyle";

	// check for "NewsServerLogonStyle", added after build 900
	if (ERROR_SUCCESS != rgReadLong (szStyle, sPtr.get(), iBufSz, lStyle))
	{
		// before now, there were 2 styles, "No Password" & "PlainText Pwd"
		//   we can allow Secure Password Authentication
		CString strNewsAccount;
		try
		{
			rgReadString ("NewsAccount", sPtr.get(), iBufSz, strNewsAccount);
		}
		catch (TException *pE) {pE->Delete();}

		// create this DWORD value
		rgWriteNum (szStyle, strNewsAccount.IsEmpty() ? 0 : 1);
	}

	// change from "NewsPwd" (stored plaintext) to "NewsPswd" (simple encrypt)
	CString strPass;

	bool bPswdExists = false;
	try
	{
		if (0 == rgReadString ("NewsPswd", sPtr.get(), iBufSz, strPass))
			bPswdExists = true;
	}
	catch (TException *pE) {pE->Delete();}

	if (!bPswdExists)
	{
		CString strCipher;
		strPass.Empty ();
		try
		{
			// read plaintext
			rgReadString ("NewsPwd", sPtr.get(), iBufSz, strPass);
		}
		catch (TException *pE) {pE->Delete();}

		make_cipher_text (strPass, strCipher);
		if (0 == rgWriteString ("NewsPswd", strCipher))
			RegDeleteValue (m_hkKey, "NewsPwd");
	}
}

// ------------------------------------------------------------------------
// Created  1-9-98
int TRegServer::make_cipher_text (const CString & strPlain, CString& strCipher)
{
	int iPlainSize = max(30, strPlain.GetLength() + 1);
	int iCipherSize = iPlainSize * 2 + 10;
	std::auto_ptr<BYTE> sPtrPlain(new BYTE[iPlainSize]);
	std::auto_ptr<CHAR> sPtrCipher(new CHAR[iCipherSize]);

	// even is password is empty, we are encrypting a block of 30 bytes
	ZeroMemory (sPtrPlain.get(), iPlainSize);

	lstrcpy (LPTSTR(sPtrPlain.get()), LPCTSTR(strPlain));

	// output is zero-terminated
	MRREncrypt (sPtrPlain.get(), iPlainSize, sPtrCipher.get());

	strCipher = sPtrCipher.get();
	return 0;
}

// ------------------------------------------------------------------------
int TRegServer::make_plain_text (const CString & strCipherIn, CString& strPlain)
{
	CString strCipher(strCipherIn);
	strPlain.Empty();
	int ln = strCipher.GetLength();

	// just to get a non-const ptr
	LPTSTR pCipher = strCipher.GetBuffer(ln);

	LPTSTR pPlain = strPlain.GetBuffer(ln);
	MRRDecrypt (pCipher, PBYTE(pPlain), ln);

	strCipher.ReleaseBuffer (ln);
	strPlain.ReleaseBuffer ();
	return 0;
}
