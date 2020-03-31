/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: attdoc.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:12  richard_wood
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

#include "tstrlist.h"
#include "attinfo.h"
#include "mime.h"
#include "coding.h"
#include "servcp.h"
#include "codepg.h"

class TEncodingYesNo
{
public:
   virtual bool operator()(TCHAR c, CString & qpData) = 0;
   void add (TCHAR c, CString & out)
      {
      if (' ' == c)
         out += '_';
      else
         {
         CString tmp;
         tmp.Format ("=%02X", c);
         out += tmp;
         }
      }
      
};

//
//
//  A CDocument that has an attachment list  - ABSTRACT BASE CLASS

class TAttachmentDoc : public CDocument
{
public:
   enum EAttachType { kAttachContinue, kAttachSeparate };

protected:
   TAttachmentDoc();           // protected constructor used by dynamic creation
   DECLARE_DYNCREATE(TAttachmentDoc)

// Attributes
public:

// Operations
public:
   int     Att_Add (LPCTSTR att, int size, LPCTSTR conType, LPCTSTR conDesc,
                     TMime::ECode eCode);
   void    Att_RemoveAll ();
   void    Att_Remove (int n);
   void    Att_GetName (int n, CString& att);
   BOOL    Att_UsingMime();
   void    Att_SetUsingMime(BOOL fOn);
   void    Att_MimeLock (BOOL fLock);
   BOOL    Att_IsMimeLocked (void);
   int     Att_Count (void) { return m_AttArray.GetSize(); }
   BOOL    HasBeenSaved () {return m_fSent;}
   void    SetSavedStatus (BOOL fSent) {m_fSent = fSent;}

   void SaveAttachmentInfo(TPersist822Header * p822Hdr);

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(TAttachmentDoc)
   protected:
   virtual BOOL OnNewDocument();
   //}}AFX_VIRTUAL

   // these two functions overwrite the Body of a message, but keep
   //  the attributes in the header
   void ChunkifyAttachments (int AttCount);

   void ChunkifyOneAttachment (
         const CString& rAttName,
         const CString& origSubj,
         int   startNum,
         BOOL  fBase64,
         bool  fMailing);

   void ChunkifyAllAttachments (
      const CString& rbod,
      const CString& rAttName,
      const CString& origSubj,
      BOOL           fFirst,
      bool           fMailing
      );

   // interface with derived classes (quasi-pure-virtual)
   virtual LPCTSTR        Message_GetSubject (bool fMailing) { ASSERT(0); return 0; }
   virtual void           Message_SetSubject (bool fMailing, LPCTSTR subj) { ASSERT(0); }
   virtual const CString& Message_GetBody (bool fMailing) { ASSERT(0); return m_useless; }
   virtual void           Message_SetBody (bool fMailing, CFile& body, int len = -1) { ASSERT(0); }
   virtual void           Message_SetBody (bool fMailing, const CString& str) { ASSERT(0); }

   virtual void Message_GetMimeLines(bool fMailing, CString* pVer, CString* pTyp, CString* pEnc, CString* pDesc) {ASSERT(0);}
   virtual void Message_SetMimeLines(bool fMailing, LPCTSTR  ver,  LPCTSTR  typ,  LPCTSTR  enc,  LPCTSTR  desc) {ASSERT(0);}

   virtual CString Message_GetFrom(bool fMailing) { ASSERT(0); return m_useless; }
   virtual void Message_SetFrom(bool fMailing, const CString & from) {ASSERT(0); }

// Implementation
public:
   virtual ~TAttachmentDoc();
   virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif

   // Generated message map functions
protected:
   //{{AFX_MSG(TAttachmentDoc)
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()

protected:
   BOOL UpdateData(void);
   void CloseFrameWnd (void);
   virtual void SendTheMessage()  { ASSERT(0); }

   int CalcEncodedParts(const CString & rAttName, int& parts);
   int CalcEncodedParts(int origSize, int& parts);
   int CalcPlainParts (int origSize, int attsize, int& partTotal);

   void CalcSubjectByTemplate(const CString& origSub,
                            const CString& fname,
                            int   part,
                            int   total,
                            CString& subj);

   int  make_temp_filename(CString&  result);

   int  AppendBody_and_Attachment(const CString& rBody,
                                  const CString& attFile,
                                  CString&  result);

   void MIME_ChunkifyAttachments (bool fMailing);
   int  MIME_CalcNumParts (bool fMailing, int& type);

   void MIME_Att_Original(int parts, bool fMailing);
   void MIME_Att_Multipart(int parts, bool fMailing);
   void MIME_Att_Partial(int parts, bool fMailing);

   void MakeABoundaryString(CString& bound);
   void Formalize_SubDescription(void);

   void AddNormalBoundary(CFile& file, BOOL fFirstLine = FALSE);
   void AddEndBoundary(CFile& file);

   void Add_Partial_Headers(bool fMailing, const CString& composite,
                            int part, int total);

   void PostArticle (TArticleHeader* pArtHdr, TArticleText* pArtTxt);
      

   // write the message to newsstore
   void FinalizeMessageForOutbox();
   void CompleteArticle();

   void SendEmail (TEmailHeader* pEmHdr, TEmailText* pEmText);
      
   void FinalizeEmailForOutbox();
   void Finalize822Message(bool fMailing);

   int  CalcSplitLen();

   int  report_fopen_error(const CString& rAttName, CFileException *pe);

   void insert_qp_entity(const CString & rbod, CFile * pFile);
   void use_qp_on_text();

	void make_qp_hdr (int iCharset, const CString & line, CString & out,
      TEncodingYesNo & sEnc);

protected:
   CString          m_useless;
   TStringList      m_Attlist;
   CString          m_boundary;
   TAttachmentArray<TAttachmentInfo, TAttachmentInfo&>   m_AttArray;

   // header usurped by Message/partial
   CString work_Type;           // multipart/mixed Info
   CString work_Desc;
   CString work_Encode;
   TCodeMgr m_codeMgr;
   BOOL     m_fSent;
   bool     m_fEmail;

private:
   TArticleHeader* m_pArtHeader;
   TArticleText*   m_pArtText;
   TEmailHeader*   m_pEmailHeader;
   TEmailText*     m_pEmailText;
   int             m_destGroupID;

   TServerCountedPtr m_cpServer;
};
