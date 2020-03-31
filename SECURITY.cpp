/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: SECURITY.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:07  richard_wood
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

/*++

Copyright (c) 1995-98 MicroPlanet Inc.

Module Name:

security.cpp

Abstract:

Handles communication with the SSP package.

Revision History:

--*/

#include "stdafx.h"

#include <winsock.h>
#define SECURITY_WIN32
#include <sspi.h>

#include "issperr.h"

#include "security.h"
#include "genutil.h"                // GetOS
#include "coding.h"
#include "resource.h"               // damn string ids

// clever Macro  #f turns f into a string
#define SCIFF(dw, f)  if ((dw) & (f)) TRACE0( #f "\n")
#define SEC_SUCCESS(Status) ((Status) >= 0)

#define WIN95_DLL_NAME		"secur32.dll"
#define WINNT_DLL_NAME		"security.dll"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/* -------------------------------------------------------------------------

Routine Description:

Finds, loads and initializes the security package

Return Value:

Returns 0 if successful; non-zero otherwise

--*/
int TSPASession::InitPackage (LPCTSTR package)
{
	FARPROC pInit;
	SECURITY_STATUS ss;
	PSecPkgInfo pkgInfo;

	// load and initialize the ntlm ssp
	//
	m_hLib = LoadLibrary ((GetOS() == RUNNING_ON_WIN95)
		? WIN95_DLL_NAME : WINNT_DLL_NAME);
	if (NULL == m_hLib)
		m_hLib = LoadLibrary (WIN95_DLL_NAME);

	if (NULL == m_hLib)
	{
		TRACE1("Couldn't load dll: %u\n", GetLastError ());
		return 1;
	}

	pInit = GetProcAddress (m_hLib, SECURITY_ENTRYPOINT);
	if (NULL == pInit)
	{
		TRACE1("Couldn't get sec init routine: %u\n", GetLastError ());
		FreeLibrary (m_hLib); m_hLib = NULL;
		return 2;
	}

	m_pFuncs = (PSecurityFunctionTable) pInit ();
	if (NULL == m_pFuncs)
	{
		TRACE0("Couldn't init package\n");
		FreeLibrary (m_hLib); m_hLib = NULL;
		return 3;
	}

	// Query for the package we're interested in
	//
	ss = m_pFuncs->QuerySecurityPackageInfo ((char*)package, &pkgInfo);
	if (!SEC_SUCCESS(ss))
	{
		TRACE2("Couldn't query package info for %s, error %u\n", package, ss);
		return 4;
	}

	// save package name
	m_strPackage = package;

	m_dwMaxBuffer = pkgInfo->cbMaxToken;

	m_pInBuf = new BYTE[m_dwMaxBuffer];
	m_pOutBuf = new BYTE[m_dwMaxBuffer];

#if defined(_DEBUG)
	TRACE1("pack name %s\n", pkgInfo->Name);
	TRACE1("pack cmt %s\n", pkgInfo->Comment);

	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_INTEGRITY        );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_PRIVACY          );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_TOKEN_ONLY       );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_DATAGRAM         );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_CONNECTION       );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_MULTI_REQUIRED   );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_CLIENT_ONLY      );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_EXTENDED_ERROR   );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_IMPERSONATION    );
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_ACCEPT_WIN32_NAME);
	SCIFF(pkgInfo->fCapabilities,SECPKG_FLAG_STREAM           );

	TRACE0("-- End of package capabilities\n");
#endif
	m_pFuncs->FreeContextBuffer (pkgInfo);

	return 0;
}

/* --------------------------------------------------------------------------

Routine Description:

Optionally takes an input buffer coming from the server and returns
a buffer of information to send back to the server.  Also returns
an indication of whether or not the context is complete.

Return Value:

Returns 0 if successful; non-zero otherwise.

--*/
SECURITY_STATUS
TSPASession::GenClientContext (
							   CHAR * pTargetServer,           // don't completely understand
							   BYTE *pIn,
							   DWORD cbIn,
							   BYTE *pOut,
							   DWORD *pcbOut,
							   BOOL *pfDone)
{
	SECURITY_STATUS	ss;
	TimeStamp		Lifetime;
	SecBufferDesc	OutBuffDesc;
	SecBuffer		OutSecBuff;
	SecBufferDesc	InBuffDesc;
	SecBuffer		InSecBuff;
	ULONG			uContextAttributes = 0;

	if (m_fNewConversation)  {
		ss = m_pFuncs->AcquireCredentialsHandle (
			NULL,	// principal
			(char*)(LPCTSTR) m_strPackage,
			SECPKG_CRED_OUTBOUND,
			NULL,	// LOGON id
			NULL,	// auth data
			NULL,	// get key fn
			NULL,	// get key arg
			&m_hcred,
			&Lifetime
			);
		if (SEC_SUCCESS (ss))
			m_fHaveCredHandle = true;
		else {
			TRACE1("AcquireCreds failed: %u\n", ss);
			return ss;
		}
	} // new conversation

	// prepare output buffer
	//
	OutBuffDesc.ulVersion = 0;
	OutBuffDesc.cBuffers = 1;
	OutBuffDesc.pBuffers = &OutSecBuff;

	OutSecBuff.cbBuffer = *pcbOut;
	OutSecBuff.BufferType = SECBUFFER_TOKEN;
	OutSecBuff.pvBuffer = pOut;

	// prepare input buffer
	//
	if (!m_fNewConversation)
	{
		InBuffDesc.ulVersion = 0;
		InBuffDesc.cBuffers = 1;
		InBuffDesc.pBuffers = &InSecBuff;

		InSecBuff.cbBuffer = cbIn;
		InSecBuff.BufferType = SECBUFFER_TOKEN;
		InSecBuff.pvBuffer = pIn;
	}

	ULONG uPass1 = ISC_REQ_MUTUAL_AUTH;
	ULONG uPass2 = ISC_REQ_MUTUAL_AUTH ;

StartPrompt:
	ss = m_pFuncs -> InitializeSecurityContext (
		&m_hcred,
		m_fNewConversation ? NULL : &m_hctxt,
		pTargetServer,
		m_fNewConversation ? uPass1 : uPass2,
		0,	// reserved1
		SECURITY_NATIVE_DREP,
		m_fNewConversation ? NULL : &InBuffDesc,
		0,	// reserved2
		&m_hctxt,
		&OutBuffDesc,
		&uContextAttributes,
		&Lifetime
		);

	if ((SEC_E_NO_CREDENTIALS == ss) && (0 == (uPass1 & ISC_REQ_PROMPT_FOR_CREDS)))
	{
		// jump back! once and only once
		uPass1 |= ISC_REQ_PROMPT_FOR_CREDS;
		goto StartPrompt;
	}

	if (!SEC_SUCCESS (ss))
	{
		TRACE1("init context failed: %lx\n", ss);
		return ss;
	}

	m_fHaveCtxtHandle = true;

	if (m_fNewConversation)
		TRACE0("--pass1\n");
	else
		TRACE0("--pass2\n");

	SCIFF(uContextAttributes,ISC_RET_MUTUAL_AUTH);
	SCIFF(uContextAttributes,ISC_RET_REPLAY_DETECT);
	SCIFF(uContextAttributes,ISC_RET_SEQUENCE_DETECT);
	SCIFF(uContextAttributes,ISC_RET_CONFIDENTIALITY);
	SCIFF(uContextAttributes,ISC_RET_USE_SESSION_KEY);
	SCIFF(uContextAttributes,ISC_RET_USED_COLLECTED_CREDS);
	SCIFF(uContextAttributes,ISC_RET_USED_SUPPLIED_CREDS);
	SCIFF(uContextAttributes,ISC_RET_ALLOCATED_MEMORY);
	SCIFF(uContextAttributes,ISC_RET_USED_DCE_STYLE);
	SCIFF(uContextAttributes,ISC_RET_DATAGRAM);
	SCIFF(uContextAttributes,ISC_RET_CONNECTION);
	SCIFF(uContextAttributes,ISC_RET_INTERMEDIATE_RETURN);
	SCIFF(uContextAttributes,ISC_RET_CALL_LEVEL);
	SCIFF(uContextAttributes,ISC_RET_EXTENDED_ERROR);
	SCIFF(uContextAttributes,ISC_RET_STREAM);
	SCIFF(uContextAttributes,ISC_RET_INTEGRITY);

	switch (ss)
	{
	case SEC_E_OK:
		TRACE0("Return SEC_E_OK\n");
		break;

	case SEC_I_CONTINUE_NEEDED:
		TRACE0("pt SEC_I_CONTINUE_NEEDED\n");
		break;

	case SEC_I_COMPLETE_NEEDED:
		TRACE0("pt SEC_I_COMPLETE_NEEDED\n");
		break;

	case SEC_I_COMPLETE_AND_CONTINUE:
		TRACE0("pt SEC_I_COMPLETE_AND_CONTINUE\n");
		break;
	}
	// Complete token -- if applicable
	//
	if ((SEC_I_COMPLETE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss))  {
		if (m_pFuncs->CompleteAuthToken) {
			ss = m_pFuncs->CompleteAuthToken (&m_hctxt, &OutBuffDesc);
			if (!SEC_SUCCESS(ss))  {
				TRACE1("complete failed: %u\n", ss);
				return ss;
			}
		}
		else {
			TRACE0("Complete not supported.\n");
			return ss;
		}
	}

	*pcbOut = OutSecBuff.cbBuffer;

	if (m_fNewConversation)
		m_fNewConversation = false;

	// check 'done' status
	*pfDone = !((SEC_I_CONTINUE_NEEDED == ss) ||
		(SEC_I_COMPLETE_AND_CONTINUE == ss));

	return 0;
}

// ------------------------------------------------------------------------
// TSPASession constructor
TSPASession::TSPASession()
{
	default_values ();
}

// ------------------------------------------------------------------------
// TSPASession destructor
TSPASession::~TSPASession()
{
	try
	{
		TermPackage ();
	}
	catch(...)
	{
	}
}

void TSPASession::default_values ()
{
	m_dwMaxBuffer = 0;
	m_pInBuf = m_pOutBuf = 0;
	m_fNewConversation = true;
	m_fHaveCredHandle  = false;
	m_fHaveCtxtHandle = false;
	m_pFuncs = 0;
	m_hLib = 0;
	m_strPackage.Empty();
}

void TSPASession::TermPackage ()
{
	if (m_fHaveCtxtHandle)
		m_pFuncs->DeleteSecurityContext (&m_hctxt);

	if (m_fHaveCredHandle)
		m_pFuncs->FreeCredentialHandle (&m_hcred);

	if (m_hLib)
		FreeLibrary (m_hLib);

	delete m_pInBuf;
	delete m_pOutBuf;

	m_strPackage.Empty();
	default_values ();
}

// ------------------------------------------------------------------------
//
int TSPASession::CreateLeg1 (const CString& server, CString & strEncodedLeg1,
							 DWORD & dwError)
{
	CString strServer(server);

	DWORD cbOut = m_dwMaxBuffer;
	BOOL done = FALSE;

	SECURITY_STATUS ss =
		GenClientContext ((char*) (LPCTSTR) strServer,
		NULL,      // InBuffer
		0,         // cbIn
		m_pOutBuf, // OutBuffer,
		&cbOut,    // cbOut
		&done);
	if (!SEC_SUCCESS(ss))
	{
		dwError = DWORD(ss);
		return 1;
	}

	String_base64_encode (m_pOutBuf, cbOut, strEncodedLeg1);
	return 0;
}

// ------------------------------------------------------------------------
//
int TSPASession::CreateLeg3 (const CString& server, CString & ack,
							 CString & strEncodedLeg3, DWORD & dwError)
{
	CString strServer(server);
	ULONG uBinaryLen;
	char rcBinary[1000];
	DWORD cbOut = m_dwMaxBuffer;
	BOOL done = FALSE;
	SECURITY_STATUS ss;
	int ln = ack.GetLength();
	LPTSTR pAck = ack.GetBuffer(ln);

	ack.TrimRight ();

	// fill rcBinary
	uBinaryLen = String_base64_decode (pAck, ln, rcBinary, 1000);

	ack.ReleaseBuffer (ln);

	// Important second call

	ss = GenClientContext ((char*) (LPCTSTR) strServer,
		(PBYTE)rcBinary,
		uBinaryLen,
		m_pOutBuf,
		&cbOut,
		&done);

	if (!SEC_SUCCESS(ss))
	{
		dwError = DWORD(ss);
		return 1;
	}

	// back to base64
	String_base64_encode (m_pOutBuf, cbOut, strEncodedLeg3);

	return 0;
}

// map error to string-id
static DWORD  rcSPAErrors[] =
{
	SEC_E_INSUFFICIENT_MEMORY,            IDS_SPA_SYSERR_NOMEM            ,
	SEC_E_INVALID_HANDLE,                 IDS_SPA_SYSERR_EHANDLE          ,
	SEC_E_UNSUPPORTED_FUNCTION,           IDS_SPA_SYSERR_UFUNC            ,
	SEC_E_TARGET_UNKNOWN,                 IDS_SPA_SYSERR_UNKTARG          ,
	SEC_E_INTERNAL_ERROR,                 IDS_SPA_SYSERR_INTERNAL         ,
	SEC_E_SECPKG_NOT_FOUND,               IDS_SPA_SYSERR_PKG              ,
	SEC_E_NOT_OWNER,                      IDS_SPA_SYSERR_NOTOWNER         ,
	SEC_E_CANNOT_INSTALL,                 IDS_SPA_SYSERR_EINSTALL         ,
	SEC_E_INVALID_TOKEN,                  IDS_SPA_SYSERR_TOKEN            ,
	SEC_E_CANNOT_PACK,                    IDS_SPA_SYSERR_PACK             ,
	SEC_E_QOP_NOT_SUPPORTED,              IDS_SPA_SYSERR_QOP              ,
	SEC_E_NO_IMPERSONATION,               IDS_SPA_SYSERR_EIMPERSONATE     ,
	SEC_E_LOGON_DENIED,                   IDS_SPA_SYSERR_LOGDENIED        ,
	SEC_E_UNKNOWN_CREDENTIALS,            IDS_SPA_SYSERR_CRED             ,
	SEC_E_NO_CREDENTIALS,                 IDS_SPA_SYSERR_NOCRED           ,
	SEC_E_MESSAGE_ALTERED,                IDS_SPA_SYSERR_MSGALT           ,
	SEC_E_OUT_OF_SEQUENCE,                IDS_SPA_SYSERR_SEQ              ,
	SEC_E_NO_AUTHENTICATING_AUTHORITY,    IDS_SPA_SYSERR_AUTHAUTH         ,
	SEC_I_CONTINUE_NEEDED,                IDS_SPA_SYSERR_CONTINUE_NEEDED  ,
	SEC_I_COMPLETE_NEEDED,                IDS_SPA_SYSERR_COMPLETE_NEEDED  ,
	SEC_I_COMPLETE_AND_CONTINUE,          IDS_SPA_SYSERR_CC               ,
	SEC_I_LOCAL_LOGON,                    IDS_SPA_SYSERR_LOCAL_LOGON      ,
	SEC_E_BAD_PKGID,                      IDS_SPA_SYSERR_BAD_PKGID        ,
	SEC_E_CONTEXT_EXPIRED,                IDS_SPA_SYSERR_CONTEXT_EXPIRED  ,
	SEC_E_INCOMPLETE_MESSAGE,             IDS_SPA_SYSERR_MSG_INCOMPLETE
};

// ------------------------------------------------------------------------
// FormatMessage -- a string to show the user. Returns 0 for success
//
int TSPASession::FormatMessage (const DWORD dwError, CString & message)
{
	// I tried the Win32 call ::FormatMessage, but it doesn't seem
	//    to work. it may only work with GetLastError()
	int k = sizeof(rcSPAErrors) / sizeof(rcSPAErrors[0]);
	for (int i = 0; i < k; i+=2)
	{
		if (dwError == rcSPAErrors[i])
		{
			message.LoadString (IDS_SPA_ERROR);
			message += "\r\n";

			CString hx; hx.LoadString (rcSPAErrors[i+1]);
			message += hx;

			hx.Format(" (%x)", dwError);
			message += hx;
			return 0;
		}
	}
	return 1;
}

