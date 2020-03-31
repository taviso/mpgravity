/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rtfspt.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:47  richard_wood
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

typedef struct tagSTREAMIN_DATA
{
   LPCTSTR   pStr;
   LONG      index;
   LONG      total;
   BOOL      end;
} STREAMIN_DATA, * LPSTREAMIN_DATA;

typedef struct tagSTREAMOUT_DATA
{
   CString *  pString;
   BOOL       end;
} STREAMOUT_DATA, * LPSTREAMOUT_DATA;

typedef CTypedPtrArray<CPtrArray, CHARRANGE*> PARAVEC;

typedef struct tagPARAFIND_STREAMIN_DATA
{
   PARAVEC*    pVec;
   LPCTSTR     pPrefix;
   int         iPrefixLen;
   LONG        iStartIdx;
   LONG        iFoundCRLF;
   LONG        iIndex;
   int         fBegun;
   BYTE        cLast;
   BOOL        end;
   BOOL        fNewline;
} PARAFIND_STREAMIN_DATA, * LPPARAFIND_STREAMIN_DATA;

void RTF_TextIn(CWnd* pWnd, LPCTSTR pText, WPARAM wFlags = 0);
void RTF_TextOut(CWnd* pWnd, CString*  pString, WPARAM wFlags = SF_TEXT);

// special support for Quoting original text
void RTF_QuotedTextIn (CWnd* pWnd,
                       const CString& prefix,
                       const CString& orig,
                       WPARAM wFlags = SF_TEXT);

// OR in these styles with EM_SETEVENTMASK
void RTF_EventMask (CWnd* pWnd, DWORD style);

void RTF_TextRotate13(CString & strText);

int RTF_GetParagraphBounds(CWnd* pRich, int iCharPos, CHARRANGE* pchrg);
int RTF_CollectParagraphs(CWnd* pRich, CHARRANGE& chrg, PARAVEC* pVec);

// stupid repetitive stuff
int RTF_FindChar (CRichEditCtrl & sRich, int iStartPos, TCHAR  cKey,
                  BOOL & fEOF);
