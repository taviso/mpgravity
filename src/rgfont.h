/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgfont.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
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

#pragma once

#include "rgbase.h"

class TRegFonts : public TRegistryBase {
public:

public:
   TRegFonts();
   ~TRegFonts();
   int Load();
   int Save();

   void  SetNewsgroupFontColor (DWORD crf) { m_clrNewsgroup = crf; }
   DWORD GetNewsgroupFontColor (void) { return m_clrNewsgroup; }

   void     SetTreeFontColor (DWORD crf) { m_clrTree = crf; }
   DWORD GetTreeFontColor (void) { return m_clrTree; }

   void     SetNewArticleColor (DWORD crf) { m_clrNewArticle = crf; }
   DWORD    GetNewArticleColor (void)      { return m_clrNewArticle; }

   const LOGFONT*  GetTreeFont ();
   void  SetTreeFont (const LOGFONT* pLF);

   const LOGFONT*  GetNewsGroupFont ();
   void  SetNewsGroupFont (const LOGFONT* pLF);

   void  SetQuotedTextColor (DWORD crf) { m_clrQuotedText = crf; }
   DWORD GetQuotedTextColor (void)      { return m_clrQuotedText; }

   const LOGFONT*  GetQuotedTextFont ();
   void  SetQuotedTextFont (const LOGFONT* pLF);

   void  SetSignatureTextColor (DWORD crf) { m_clrSignatureText = crf; }
   DWORD GetSignatureTextColor (void)      { return m_clrSignatureText; }

   const LOGFONT*  GetSignatureTextFont ();
   void  SetSignatureTextFont (const LOGFONT* pLF);

   const CString& GetQuoteChars (void) { return m_QuoteChars; }
   void  SetQuoteChars (LPCTSTR s) { m_QuoteChars = s; }

   void  SetPrintTextColor (DWORD crf) { m_dwPrintColor = crf; }
   DWORD GetPrintTextColor (void)      { return m_dwPrintColor; }

   const LOGFONT* GetPrintFont ();
   void  SetPrintFont (const LOGFONT* pLF);

   int GetPrintPointSize () { return m_iPrintPointSize; }
   void SetPrintPointSize (int iSize) { m_iPrintPointSize = iSize; }

public:
   int     m_treePtSize;

   int     m_ngPtSize;

   LOGFONT m_postFont;         // a fixed font for posting
   int     m_postPtSize;       //  the point size * 10

   int     m_QuotedTextPtSize; // size of quoted text font, point size * 10
   int     m_SignatureTextPtSize; // size of signature text font, point size * 10

protected:
   LOGFONT m_ngFont;
   LOGFONT m_treeFont;

   DWORD   m_clrNewsgroup;
   DWORD   m_clrTree;
   DWORD   m_clrNewArticle;   // color in listbox

   LOGFONT m_lfQuotedText;    // font for quoted text
   DWORD   m_clrQuotedText;   // FG color for quoted text

   LOGFONT m_lfSignatureText;    // font for Signature text
   DWORD   m_clrSignatureText;   // FG color for Signature text

   CString m_QuoteChars;      // when viewing, these chars indicate Quoted text

   LOGFONT m_printFont;       // for printing
   int     m_iPrintPointSize;
   DWORD   m_dwPrintColor;

   void read();
   void write();
   void default_values();
};
