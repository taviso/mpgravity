/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artdisp.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:10  richard_wood
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

#pragma once

#include "richedit.h"
#include "triched.h"
#include "custview.h"
#include "article.h"

class GravCharset;

typedef int (*P_ARTDISPFILTERFUNC)(CString & in, CString & out, DWORD dwCookie);
struct ARTDISP_INFO
{
   BOOL                 m_fOptimize;
   P_ARTDISPFILTERFUNC  m_pFunc;
   DWORD                m_dwFuncData;

   // constructor  (structs can have ctors, too!)
   ARTDISP_INFO(BOOL fOptimize)
   {
      m_fOptimize = fOptimize;
      m_pFunc = 0;
      m_dwFuncData = 0;
   }
} ;

void RichDisplayArticle(CWnd *pRich, TPersist822Header * pArt,
                        TPersist822Text * pText,
                        BOOL fFullHeader = FALSE,
                        bool fShowQuotedText = true,
                        bool fShowMsgSource = false,
								GravCharset * pCharset = NULL);

void RichDisplaySummary(CWnd * pRich, TPersist822Header * pArt);

int IsURL (LPCTSTR  word, int *len = NULL);
BOOL GetWord ( LPCTSTR     line,
               LPTSTR      word,
               int *       pos);

struct T_ADHOC_IAF
{
   CWnd              * pRich;
   TPersist822Header * pArt;
   TPersist822Text *   pText;
   BOOL              fFullHeader;
   bool              fMsgSource;
};

static BOOL InsertArticleField (T_ADHOC_IAF & iaf,
                                 WORD wField,
                                 CHARRANGE* pRange,
                                 ARTDISP_INFO* pInfo,
											GravCharset * pCharset = 0);

void SetInitialFont(CWnd *pRich);

static void ColorizeQuotedText(CWnd *pRich, LONG low, LONG high,
                               const LOGFONT* pLF,
                               COLORREF ref,
                               int  iTwips,
                               const CString & strQuoteChars);
