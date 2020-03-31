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

Copyright (c) 1995-98 MicroPlanet, Inc.

Module Name:

    security.h

Abstract:

    Functions to handle communication with the security package

Revision History:

--*/

#pragma once

#define SECURITY_WIN32
#include <sspi.h>

// ------------------------------------------------------------------------
class TSPASession
{
public:
   TSPASession();
   ~TSPASession();

   int     InitPackage (LPCTSTR package);
   void    TermPackage ();

   int CreateLeg1 (const CString& server, CString & strEncodedLeg1,
                   DWORD & dwError);
   int CreateLeg3 (const CString& server, CString & ack,
                   CString & strEncodedLeg3, DWORD & dwError);

   // a string to show the user. 0 for success.
   int  FormatMessage (DWORD dwError, CString & message);

private:
   void default_values ();

   SECURITY_STATUS
   TSPASession::GenClientContext (
         CHAR * pTargetServer,           // don't completely understand
			BYTE *pIn,
			DWORD cbIn,
			BYTE *pOut,
			DWORD *pcbOut,
			BOOL *pfDone);

public:
   DWORD    m_dwMaxBuffer;
   PBYTE    m_pInBuf;
   PBYTE    m_pOutBuf;

   bool                   m_fNewConversation;
   bool                   m_fHaveCredHandle;
   CredHandle             m_hcred;

   bool                   m_fHaveCtxtHandle;
   struct _SecHandle      m_hctxt;

   PSecurityFunctionTable m_pFuncs;
   HINSTANCE              m_hLib;

   CString                m_strPackage;
};



