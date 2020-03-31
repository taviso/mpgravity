/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: kidsecch.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:28  richard_wood
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

// kidsecch.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "kidsecch.h"
#include "crypt.h"
#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TKidSecureChgPwd dialog

TKidSecureChgPwd::TKidSecureChgPwd(CWnd* pParent, CString& rCiperText)
	: CDialog(TKidSecureChgPwd::IDD, pParent),
     m_cipherText(rCiperText)
{

	m_oldPass = _T("");
	m_newPass = _T("");
	m_verifyPass = _T("");
}

void TKidSecureChgPwd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_KSEC_OPWD, m_oldPass);
	DDX_Text(pDX, IDC_KSEC_PWD, m_newPass);
	DDX_Text(pDX, IDC_KSEC_VRFY, m_verifyPass);
}

BEGIN_MESSAGE_MAP(TKidSecureChgPwd, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TKidSecureChgPwd message handlers

void TKidSecureChgPwd::OnOK() 
{
	UpdateData(TRUE);

   if (m_newPass != m_verifyPass)
      {
      NewsMessageBox (this, IDS_WARN_PWDNOMATCH, MB_OK | MB_ICONEXCLAMATION);
      return;
      }

   // check correctness of old password
   int ctLen = m_cipherText.GetLength();
   CString dbPass;
   LPTSTR pdbPass = dbPass.GetBuffer(2*ctLen);
   ZeroMemory(pdbPass, 2*ctLen*sizeof(TCHAR));
   MRRDecrypt ((LPTSTR)LPCTSTR(m_cipherText), (BYTE*)pdbPass, 2*ctLen);
   dbPass.ReleaseBuffer();
   if (m_oldPass != dbPass)
      {
      NewsMessageBox (this, IDS_WARN_BADOLDPWD, MB_OK | MB_ICONEXCLAMATION);
      return;
      }

   int plain_len = m_newPass.GetLength();
   if (plain_len == 0)
      m_cipherText.Empty();
   else
      {
      LPTSTR pCipherText = m_cipherText.GetBuffer(2*plain_len + 6);
      // encrypt with rotors
      MRREncrypt((BYTE*)(LPTSTR)LPCTSTR(m_newPass), plain_len, pCipherText);
      m_cipherText.ReleaseBuffer();
      }
   CDialog::OnOK();
}
