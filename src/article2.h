/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: article2.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.2  2008/09/19 14:51:11  richard_wood
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

/********************************************************************
TBase822Text
   TPersist822Text
      TEmailText
      TArticleText

********************************************************************/

#include "pobject.h"
#include "codeevt.h"

class TArticleHeader;
class TArticleText;
class TEmailText;

#include "e_render.h"

class TMimeSequence {
public:
   TMimeSequence() {}
   ~TMimeSequence() {}
   TMimeSequence& operator=(const TMimeSequence& rhs)
   {
      if (this == &rhs) return *this;
      m_id    = rhs.m_id;
      m_fname = rhs.m_fname;
      m_part  = rhs.m_part;
      m_total = rhs.m_total;
      return *this;
   }

   CString  m_id;
   CString  m_fname;
   int      m_part;
   int      m_total;
};

class TMimeThread {
public:
   TMimeThread() { m_fFileOwnership = TRUE;}
   ~TMimeThread();
   void Add(TMimeSequence& seq);
   BOOL IsComplete(void);
   BOOL SuperAppend(CString& result);
   TMimeThread& operator= (const TMimeThread& rhs);
   CArray<TMimeSequence, TMimeSequence&>  m_set;
   BOOL  m_fFileOwnership;
};

//void ConstructElements(TMimeSequence* pNewItem, int nCount);
//void DestructElements(TMimeSequence* pNewItem, int nCount);

//void ConstructElements(TMimeThread* pNewItem, int nCount);
//void DestructElements(TMimeThread* pNewItem, int nCount);

typedef struct
{
   CString type;
   CString description;
   CString encoding;
   CString disposition;          // content-disposition
   CString contentId;
} T_MIME_HEADER_AREA, * LPT_MIME_HEADER_AREA;

class TBase822Text : public PObject
{
protected:
   TBase822Text(BYTE byVersion = 1);    // like an abstract base class

public:
   virtual ~TBase822Text() {}

   DECLARE_SERIAL(TBase822Text)
#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif
   virtual void Serialize (CArchive & archive);
   TBase822Text & operator=(const TBase822Text &);

   void     SetBody(LPCTSTR body) { m_Body = body;}
   void     SetBody(CFile& fl, int len = -1);

   const CString&  GetBody(void)         { return m_Body; }
   CString&  AccessBody(void) { return m_Body; }

   void     Pad_CRLF(void);

protected:
   CString     m_Body;
};

class TPersist822Text : public TBase822Text
{
public:
   enum ERenderStat { kNoError, kNotMIME, kNoContentType, kTreatAsUU };

protected:
   TPersist822Text();    // protected construct - like an abstract base class

public:
   LONG HashKey() { return m_UniqueNumber; }

   // returns an estimate of the disk space we need
   virtual DWORD SizeHint ();

   virtual ~TPersist822Text();

   DECLARE_SERIAL(TPersist822Text)
#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif
   virtual void Serialize (CArchive & archive);
   TPersist822Text & operator=(const TPersist822Text &);

   virtual TArticleText*  CastToArticleText() { return 0; }
   virtual TEmailText*    CastToEmailText()   { return 0; }

   LONG GetNumber()       { return m_UniqueNumber; }
   void SetNumber(LONG n) { m_UniqueNumber = n; }

   const CString&  GetHeader(void)        { return m_Header; }
   void            SetHeader(LPCTSTR hdr);

   void            ParseFullHeader(CString* pstrUserAgent = 0);
   BOOL            GetDistribution(CString& dist);
   BOOL            GetContentDesc(CString& desc);
   BOOL            GetContentEncoding(CString& encoding);
   BOOL            GetContentType(CString& type);
   BOOL            GetContentDisposition(CString& type);
   BOOL            GetExpires(CString& expires);
   BOOL            GetFollowup(CString& followup);
   BOOL            GetNewsgroups(CString& ngroups);
   BOOL            GetOrganization(CString& org);
   BOOL            GetPath(CString& path);
   BOOL            GetPostingHost(CString& postinghost);
   BOOL            GetReplyTo(CString& replyto);
   BOOL            GetKeywords(CString& expires);
   BOOL            GetSender(CString& sender);
   BOOL            GetSummary(CString& summary);
   BOOL            GetUserAgent(CString& strUserAgent);
   BOOL            GetXNewsreader(CString& strXNewsreader);

   TPersist822Text::ERenderStat
           RenderBody_MIME(const CString& decodeDir,
                           CString& bod,
                           E_RENDER eRender,
                           const CString& subjline_filename,
                           CODE_EVENT_MONITOR pEventMonitor,
                           bool  fFromCache_NoSKIP);

   BOOL        IsBodyMime() const { return m_fMime; }

protected:
   TPersist822Text::ERenderStat
               Render_Entity(const CString& subjline_filename,
                             CString* pContent_type,
                             CString* pContent_desc,
                             CString* pContent_enc,
                             CString* pContent_disposition,
                             CString* pContent_id,
                             LPCTSTR  data,
                             int      start,
                             int      end,
                             E_RENDER eRender,
                             CString& bod,
                             bool  fFromCache_NoSKIP);

   void        Render_multipart_mixed( const CString& subjline_filename,
                                       const CString* pContent_type,
                                       LPCTSTR        data,
                                       int            start,
                                       int            end,
                                       E_RENDER       eRender,
                                       CString& bod,
                                       bool  fFromCache_NoSKIP);

   void        Render_multipart_alternative(const CString& subjline_filename,
                                            const CString* pContent_type,
                                            LPCTSTR        data,
                                            int            start,
                                            int            end,
                                            E_RENDER       eRender,
                                            CString& bod,
                                            bool    fFromCache_NoSKIP);

   void        Render_opaque_binary(const CString& subjline_filename,
                                    const T_MIME_HEADER_AREA * pArea,
                                    const CString& type_subtype,
                                    LPCTSTR  data,
                                    int     start,
                                    int     end,
                                    E_RENDER  eRender,
                                    CString& bod,
                                    bool    fFromCache_NoSKIP);

   LPCTSTR     read_header_area(LPCTSTR src, LPT_MIME_HEADER_AREA pArea);
   LPCTSTR     read_line(LPCTSTR src, TCHAR* buf, int buf_size, int* res_len);
   void        Render_summary(const CString* pContent_desc,const CString& type_subtype, CString& bod);
   void        util_fullheader_snip(WORD offset, CString& result);
   void        save_partial_mime_chunk (const CString& subjline_filename,
                                        CString* ptype, CString* pdesc,
                                        CString* penc,
                                        LPCTSTR data, int start, int end,
                                        bool fFromCache_NoSKIP);

   void        presto_chango_memmapfile (const CString& subjline_filename,
                                         const CString& fname,
                                         bool fFromCache_NoSKIP);

   void        Decode_All (const CString& subjline_filename,
                           const T_MIME_HEADER_AREA * pArea,
                           LPCTSTR        data,
                           int            len,
                           bool fFromCache_NoSKIP);

   void        Decode_known_type (const CString& subjline_filename,
                                  int   type,
                                  const T_MIME_HEADER_AREA * pArea,
                                  LPCTSTR data,
                                  int len,
                                  bool fFromCache_NoSKIP);

   int         Decode_section_fname (const T_MIME_HEADER_AREA * pArea,
                                     const CString & subjline_filename,
                                     TPath& output_spec);

   void        Decode_section_fext (const T_MIME_HEADER_AREA * pArea,
                                     CString& output_spec);

protected:
   static      CMap<CString, LPCTSTR, TMimeThread, TMimeThread&> m_sequencer;

   BYTE        m_fArticle;
   BYTE        m_fParsed;
   BYTE        m_fMime;
   LONG        m_UniqueNumber;
   CString     m_Header;            // full article header

   // offsets
   WORD        m_off_Distribution;
   WORD        m_off_Content_Desc;
   WORD        m_off_Content_Enc;
   WORD        m_off_Content_Type;
   WORD        m_off_Expires;
   WORD        m_off_Followup;
   WORD        m_off_Keywords;
   WORD        m_off_Newsgroups;
   WORD        m_off_Organization;
   WORD        m_off_Path;
   WORD        m_off_PostingHost;
   WORD        m_off_ReplyTo;
   WORD        m_off_Sender;
   WORD        m_off_Summary;
   WORD        m_off_UserAgent;
   WORD        m_off_XNewsreader;

   WORD        m_off_Content_Dispo;

   // for Decoding.  this is not serialized
   CString     m_decodeDir;
};

/////////////////////////////////////////////////////////////////////////////
// TArticleText        - Attributes that are not necessary for displaying
//                       a threadview.  These will be stored offline
//                       in a less accessible format.
/////////////////////////////////////////////////////////////////////////////

class TArticleText : public TPersist822Text
{
public:
   DECLARE_SERIAL (TArticleText)
   TArticleText ();
   ~TArticleText ();
   TArticleText & operator=(const TArticleText & );
   virtual void Serialize (CArchive & archive);

   // Posting Followup articles - we must download this information
   void            SetPostToLine(LPCTSTR postTo) { m_PostTo = postTo; }
   const CString&  GetPostToLine(void)           { return m_PostTo; }

   TArticleText*  CastToArticleText() { return this; }
   TEmailText*    CastToEmailText()   { return 0; }

   void TransferXRef (TArticleHeader * pHdr);

   // Good NetKeeping
   BOOL FollowupSame ();
   BOOL ReplyToSame (CString & address);

   // returns an estimate of the disk space we need
   virtual DWORD SizeHint ();

protected:
   LONG        m_fBody;             // just a boolean
   LONG        m_fHeader;           // just a boolean

   CString     m_PostTo;            // Newsgroups or Followup-To
};

class TEmailText : public TPersist822Text
{
public:
   DECLARE_SERIAL (TEmailText)
   TEmailText()
      {
      m_fArticle = FALSE;
      }
   ~TEmailText() {}

   TArticleText*  CastToArticleText() { return 0; }
   TEmailText*    CastToEmailText()   { return this; }
};
