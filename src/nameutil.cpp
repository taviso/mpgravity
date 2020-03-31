/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: nameutil.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:32  richard_wood
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
#include "resource.h"
#include "nameutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static void  remove_phrase(CString& emailAddr);

static BOOL NU_CheckWord(const CString & s);
static BOOL NU_CheckLocal (const CString & local);
static BOOL NU_CheckDomain (const CString & domain);

///////////////////////////////////////////////////////////////////////////
// This basically checks for:
//   word@word.word and doesn't handle the weird stuff in RFC822 like:
//     quoted-strings, special chars...
//
BOOL CheckEmailSyntax(const CString & InEmailAddr, int * piErrorID)
{
   CString emailAddr = InEmailAddr;

   remove_phrase(emailAddr);

   // "an email address should be similar to: Jane@AOL.COM"
   //  this message is generic enough
   *piErrorID = IDS_NOAT_EMAIL;

   // local-part @ domain
   // local-part = word * ("." word)
   // domain = sub-domain * ("." sub-domain)

   // check for minimum length
   if (emailAddr.GetLength() <= 2)
      {
      *piErrorID = IDS_REQUIRE_EMAIL;
      return FALSE;
      }

   int iAT = emailAddr.Find('@');
   if (-1 == iAT)
      {
      *piErrorID = IDS_NOAT_EMAIL;
      return FALSE;
      }

   CString local = emailAddr.Left(iAT);

   if (!NU_CheckLocal(local))
      return FALSE;

   // minimum al@X.Y

   CString domain = emailAddr.Mid(iAT + 1);
   CString subd;

   // should be no more @ signs
   if (-1 != domain.Find ('@'))
      return FALSE;

   int iDot = domain.Find('.');
   if (-1 == iDot)
      return FALSE;

   return NU_CheckDomain(domain);
}

static char rcNormal[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~!#$%^&*-+=_/";

static BOOL NU_CheckLocal (const CString & local)
{
   int iLen = local.GetLength();
   if (0 == iLen)
      return FALSE;
   if ('.' == local[0] || '.' == local[iLen-1])
      return FALSE;

   return NU_CheckWord( local );
}

static BOOL NU_CheckDomain (const CString & domain)
{
   CString dom = domain;
   int iDot = dom.Find('.');

   // should be at least 2 subdomains
   CString left = dom.Left(iDot);
   CString right = dom.Mid(iDot + 1);
   if (!NU_CheckWord(left) || !NU_CheckWord(right))
      return FALSE;

   dom = dom.Mid(iDot + 1);
   while (TRUE)
      {
      if (!NU_CheckWord(left))
         return FALSE;
      if ((iDot = dom.Find('.')) == -1)
         return NU_CheckWord(dom);
      else
         {
         left = dom.Left(iDot);
         dom = dom.Mid(iDot + 1);
         }
      }
   return TRUE;
}

// do some really basic
static BOOL NU_CheckWord(const CString & s)
{
   if (0 == s.GetLength())
      return FALSE;

   CString found = s.SpanIncluding(rcNormal);

   if (0 == found.GetLength())
      return FALSE;
   return TRUE;
}

void AFXAPI DDX_EmailName(CDataExchange* pDX, int nIDC, CString& str)
{
   DDX_Text(pDX, nIDC, str);
}

void AFXAPI DDV_EmailName(CDataExchange* pDX, int nIDC, CString& str,
                          BOOL fRequired)
{
   HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
   if (pDX->m_bSaveAndValidate)
      {
      if (!fRequired && str.IsEmpty())
         {
         // some fields are optional , like 'Sender'
         }
      else
         {
         int iErrorStringID;

         if (!CheckEmailSyntax(str, &iErrorStringID))
            {
            AfxMessageBox(iErrorStringID);
            pDX->Fail();
            }
         }
      }
   else
      {
      // do nothing
      }
}

static void  remove_phrase(CString& emailAddr)
{
   CString addr;
   int i =  emailAddr.Find('<');
   if (-1 != i)
      {
      // take stuff within angle brackets
      addr = (emailAddr.Mid(i+1)).SpanExcluding(">");
      emailAddr = addr;
      return;
      }

   // hack off the comment
   int iParenOpen = emailAddr.Find('(');
   int iParenCloz = emailAddr.ReverseFind(')');

   if (-1 == iParenOpen)
      {
      return;
      }

   if (iParenCloz > iParenOpen)
      {
      CString left_part = emailAddr.Left(iParenOpen);
      CString right_part = emailAddr.Mid(iParenCloz + 1);
      emailAddr = left_part + right_part;
      }
}

///////////////////////////////////////////////////////////////////////////
void AFXAPI DDX_HostName(CDataExchange* pDX, int nIDC, CString& str)
{
   DDX_Text(pDX, nIDC, str);

   // folks like to copy & paste hostnames, so trim off any trailing spaces
   str.TrimLeft();
   str.TrimRight();
}

///////////////////////////////////////////////////////////////////////////
void AFXAPI DDV_HostName(CDataExchange* pDX, int nIDC, CString& str,
                          BOOL fRequired)
{
   HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
   if (pDX->m_bSaveAndValidate)
      {
      if (!fRequired)
         return;

      if (str.IsEmpty())
         {
         AfxMessageBox (IDS_WARN_NOSERVER_NAME, MB_ICONWARNING);
         pDX->Fail();
         }
      }
   else
      {
      // do nothing
      }
}

