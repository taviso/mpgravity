/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: posttmpl.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:41  richard_wood
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

// posttmpl.h -- definitions for all posting/mailing templates

#pragma once

#include "postdoc.h"                // TPostDoc
#include "posthdr.h"                // TPostHdrView
#include "postpfrm.h"               // TPostPopFrame
#include "postmdi.h"                // TPostMDIChildWnd

// -------------------------------------------------------------------------
#define TPT_TO_NEWSGROUP             0x00001  /* m_NewsGroupID */
#define TPT_TO_STRING                0x00002  /* m_strTo */
#define TPT_INSERT_ARTICLE           0x00004  /* m_pArtText */
#define TPT_READ_ARTICLE_HEADER      0x00008  /* m_pArtHdr */
#define TPT_CANCEL_WARNING_ID        0x00010  /* m_iCancelWarnID */
#define TPT_FOLLOWUP                 0x00020  /* to construct references line */
                                              /* and handle extra screen fields */
#define TPT_INIT_SUBJECT             0x00040  /* m_strSubject, m_strSubjPrefix */
#define TPT_POST                     0x00080  /* MUST SET EITHER TPT_POST OR */
#define TPT_MAIL                     0x00100  /* TPT_MAIL */
#define TPT_INSERT_FILE              0x00200  /* m_strExtraTextFile */
#define TPT_USE_FOLLOWUP_INTRO       0x00400  /* intro to paste into body */
#define TPT_USE_REPLY_INTRO          0x00800  /* intro to paste into body */
#define TPT_INSERT_SELECTED_ARTICLES 0x01000  /* inserts selected articles */
                                              /* from view that has focus */
#define TPT_COPY_MAIL_HEADER         0x02000  /* m_pCopyMailHdr */
#define TPT_COPY_ARTICLE_HEADER      0x04000  /* m_pCopyArtHdr */
#define TPT_INSERT_TEXT              0x08000  /* m_strText */
#define TPT_POST_CC_AUTHOR           0x10000  /* check the 'CC author' */
#define TPT_INSERT_MACHINEINFO       0x20000  /* append sysinfo to body */
#define TPT_DONT_WRAP                0x40000  /* don't wrap text */
#define TPT_DONT_QUOTE               0x80000  /* don't quote text */
#define TPT_IGNORE_LINE_LIMIT       0x100000  /* ignore limit on quoted text */
#define TPT_USE_FORWARD_INTRO       0x200000  /* intro to paste into body */
#define TPT_TO_NEWSGROUPS           0x400000  /* m_rNewsGroups */

// -------------------------------------------------------------------------
class TPostTemplate : public CMultiDocTemplate
{
public:
   TPostTemplate (UINT nIDResource)
      : CMultiDocTemplate (nIDResource,
                           RUNTIME_CLASS (TPostDoc),
                           RUNTIME_CLASS (TPostPopFrame),
                           RUNTIME_CLASS (TPostHdrView)) { }
   ~TPostTemplate () { }
   void Launch (int iNewsGroupID)
      {
         m_NewsGroupID = iNewsGroupID;
         giMailing = !(m_iFlags & TPT_POST);
         OpenDocumentFile (NULL);
      }
   BOOL TestFlag (int iFlag) { return (iFlag & m_iFlags) ? TRUE : FALSE; }

public:
   int         m_iFlags;         // see flag bits below
   LONG        m_NewsGroupID;    // newsgroup to show in "to" field
   CString     m_strTo;          // person to show in "to" field
   TArticleHeader * m_pArtHdr;   // to extract followup groups & references
   TArticleText * m_pArtText;    // article text to insert
   int        m_iCancelWarningID;// IDS_*
   CString m_strSubject;         // initial subject
   CString m_strSubjPrefix;      // prefix the subject with this if not there
   CString m_strExtraTextFile;   // file containing extra text to insert
   TArticleHeader *m_pCopyArtHdr;// copy all fields from this article header
   TEmailHeader *m_pCopyMailHdr; // copy all fields from this mail header
   CString m_strText;            // text to insert into body
   CDWordArray m_rNewsgroups;    // list of integers (like m_NewsGroupID)
};
