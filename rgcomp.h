/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgcomp.h,v $
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
/*  Revision 1.2  2008/09/19 14:51:44  richard_wood
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
#include "tglobdef.h"

class TRegCompose : public TRegistryBase {
public:
	typedef struct {
		TCHAR name[10];
		TGlobalDef::EEncoding m_encoding;
	} ENCSYNC;

public:
	TRegCompose();
	~TRegCompose();
	int Load();
	int Save();

	const CString&  GetDistribution () { return m_distribution; }
	void            SetDistribution (LPCTSTR dist) { m_distribution = dist; }

	const CString&  GetReplyTo () { return m_replyTo; }
	void            SetReplyTo (LPCTSTR rplyto) { m_replyTo = rplyto; }

	const CStringList& GetCustomHeaders () { return m_sCustomHeaders; }
	void               SetCustomHeaders (CStringList &sCustomHeaders);

	const int GetSendCharset () { return m_iSendCharset; }
	void      SetSendCharset (int n) { m_iSendCharset = n; }

	BOOL  GetAllow8Bit ()	{ return m_fAllow8Bit; }
	void  SetAllow8Bit (BOOL f) { m_fAllow8Bit = f; }

	BOOL  Get8BitHdrs ()       { return m_f8BitHdrs; }
	void  Set8BitHdrs (BOOL f) { m_f8BitHdrs = f; }

public:
	BOOL                  m_fIncludeOriginalMail;
	BOOL                  m_fIncludeOriginalFollowup;
	BOOL                  m_fLimitQuotedLines;
	BOOL                  m_fattachSeparateArt;             // start attachments in separate article
	int                   m_encodeSplitLen;                 // 0 = no limit.  else in KB
	int                   m_WordWrap;                       // WordWrap setting for posts
	int                   m_IDsToRemember;                  // number of sent post IDs to remember
	LONG                  m_maxQuotedLines;
	LONG                  m_maxEncodedLines;                // max lines in encoded post
	CString               m_folIntro;                       // followup tag line
	CString               m_CCIntro;                        // CC Intro tag line
	CString               m_rplyIntro;                      // email tag line
	CString               m_MIME_ContentTransferEncoding;
	CString               m_MIME_ContentType;
	CString               m_IndentString;                   // indent string ex: '> '
	CString               m_attachSubjectTemplate;          // attachment subject template
	TGlobalDef::EEncoding m_kEncodingType;                  // type of encoding (UU or MIME)

protected:
	static ENCSYNC m_EncMap[];

	void read();
	void write();
	void default_values();

private:
	CString     m_distribution;
	CString     m_replyTo;
	CStringList m_sCustomHeaders;
	CString     m_strCustomHeaders;  // used only for registry I/O
	int         m_iSendCharset;      // iso charset used for outbound msgs
	BOOL		m_fAllow8Bit;		 // allow 8Bit text messages over Quoted-Printable?
	BOOL        m_f8BitHdrs;         // allow 8Bit subject and from?  FALSE implies RFC2047 encoding
};
