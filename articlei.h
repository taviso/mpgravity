/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: articlei.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2008/09/19 14:51:11  richard_wood
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
Inner representation

TBase822Header
   TPersist822Header
      TEmailHeader
      TArticleHeader

********************************************************************/

#include "pobject.h"
#include "tstrlist.h"
#include "article2.h"
#include "artrefs.h"
#include "memshak.h"

class TArticleHeader;
class TEmailHeader;
class TAttachmentInfo;
class TArtUIData;

class TBase822Header : public CObject
{
public:
   enum EAddrType { kAddrTo, kAddrCC, kAddrBCC };
};

// Pretty pure
class TBase822HeaderInner : public PObject
{
protected:
   TBase822HeaderInner() {};     // for serialization

   TBase822HeaderInner(BYTE objectVersion)
      : PObject(objectVersion)
      {}

   // copy-constructor
   TBase822HeaderInner (const TBase822HeaderInner & src);

public:
typedef enum {kAverageHeaderSize = 150};

#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif
   virtual ~TBase822HeaderInner() {}
   DECLARE_SERIAL(TBase822HeaderInner)
   virtual void Serialize (CArchive & archive);
   WORD SizeHint() {return kAverageHeaderSize;}
   TBase822HeaderInner & operator=(const TBase822HeaderInner &);
   virtual void AssertValid(void);
};

// has some variables that deal with sorting and saving
class TPersist822HeaderInner : public TBase822HeaderInner
{
protected:
   TPersist822HeaderInner() {}  // for serialization
   TPersist822HeaderInner(BYTE objectVersion);

   // copy-constructor
   TPersist822HeaderInner(const TPersist822HeaderInner & src);

public:
   static int DecodeElectronicAddress(const CString & rawAddress, CString & prettyAddress);

   virtual TPersist822HeaderInner* duplicate();

   void AddRef ();
   void DeleteRef ();
   LONG iGetRefCount ();

   LONG HashKey () { return m_UniqueNumber; }
   virtual ~TPersist822HeaderInner();

   //DECLARE_SERIAL(TPersist822HeaderInner)
#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif
   virtual void Serialize (CArchive & archive);
   TPersist822HeaderInner & operator=(const TPersist822HeaderInner &);

   virtual TArticleHeader* CastToArticleHeader() { return 0; }
   virtual TEmailHeader*   CastToEmailHeader()   { return 0; }

   virtual BOOL  IsArticle() { return m_fArticle; }

   TPersist822Text * AllocText(void)
      {
      if (m_fArticle)
         return new TArticleText;
      else
         return new TEmailText;
      }

   void SetNumber(LONG n) { m_UniqueNumber = n; }
   LONG GetNumber(void)   { return m_UniqueNumber; }

   // number of lines in the body
   void    SetLines (const CString& body);
   void    SetLines (int lines)        { m_Lines = lines; }
   int     GetLines (void)             { return m_Lines;  }

   const CTime& GetTime(void)    { return m_time; }
   void StampCurrentTime(void)   { m_time = CTime::GetCurrentTime(); }

   virtual LPCTSTR  GetMessageID(void) { return 0; }
   virtual int  ParseFrom (CString& phrase, CString& address)
                  { ASSERT(0); return 0;}
   virtual void SetMimeLines(LPCTSTR ver, LPCTSTR type, LPCTSTR encode, LPCTSTR desc)
                 { ASSERT(0); }
   virtual void GetMimeLines(CString* pVer, CString* pType, CString* pCode, CString* pDesc)
                 { ASSERT(0); }

   virtual void    GetDestList(TStringList* pList) const
                 { ASSERT(0); }

   // attachment access
   int NumAttachments () const { return m_iNumAttachments; }
   const TAttachmentInfo *Attachments () { return m_psAttachments; }
   // the below parameter is void* to avoid include file dependencies. It
   // should be a pointer to an object of the following type:
   //       TAttachmentArray<TAttachmentInfo, TAttachmentInfo&>
   void SetAttachments (void *psAttachmentsVoid);

protected:
   LONG        m_Lines;
   CTime       m_time;           // sort by this
   BYTE        m_fArticle;
   LONG        m_UniqueNumber;
   LONG        m_iRefCount;

   // attachment array used only when saving drafts to outbox
   int m_iNumAttachments;
   TAttachmentInfo *m_psAttachments;
};

class TBlobMaster
{
public:
   enum EFlatField {
      kFrom = 0,                    // index to our flattened area
      kMsgID,
      kSubject,
      kKeywords,
      kSender,
      kReplyTo,
      kMimeVer,
      kMimeContentType,
      kMimeContentEncoding,
      kMimeContentDesc,
      kDate,
      kDistribution,
      kExpires,
      kOrganization,
      kSummary,
      kControl,
      kFollow,
      kCrossPost };

public:
   TBlobMaster();
   ~TBlobMaster();

   void SetAtGrow (TBlobMaster::EFlatField eIdx, LPCTSTR strInput, BYTE & phraseAvail);

   LPCTSTR GetBlobString(EFlatField eIdx) const;

   TBlobMaster &  operator= (const TBlobMaster & rhs);
   TBlobMaster(const TBlobMaster & src);

   void Serialize (CArchive & archive);

   void RemoveField (EFlatField eIdx, BYTE & phraseAvailable)
      {
      remove_fld (m_riOffsets[eIdx], phraseAvailable);
      }

protected:
   void remove_fld(LONG& idx, BYTE & phraseAvailable);

protected:
   static TCHAR rcEmptyString[2];

   LONG        m_riOffsets[18];

   LONG        m_iBlobCur;
   LONG        m_iBlobMax;

   LPTSTR      m_pBlob;
};

/////////////////////////////////////////////////////////////////////////////
// TArticleHeaderInner      - Attributes that are vital.
//
/////////////////////////////////////////////////////////////////////////////
class TArticleHeaderInner : public TPersist822HeaderInner
{
public:
   // version history:
   //  1 and 2: don't know
   //  3: introduced the attachment stuff in TPersist822HeaderInner
   //  4: add vRandomStrings for ProxicomIconName
   //  5: added m_lScore
   //  6: added m_lBodyScore
   //  7: (get rid of FlatStorage) store From in a CString again
   //  8: store RFC2047 decoded phrase in 'm_strPhrase'
   //  9: store RFC2047 decoded phrase in m_vRandomStrings[3]

   enum { kVersion = 9 };

public:
   static ULONG  ShrinkMemPool();
   static TMemShack m_gsMemShack;

public:
   TArticleHeaderInner() {}  // for serialization
   TArticleHeaderInner(BYTE objectVersion);
   ~TArticleHeaderInner();

   virtual TPersist822HeaderInner* duplicate();

#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif
   virtual void Serialize (CArchive & archive);
   TArticleHeaderInner & operator=(const TArticleHeaderInner &);
   TArticleHeaderInner(const TArticleHeaderInner& src);

public:
   // return 1 if dirty
   int  MakeUIData (TArtUIData *& rpUIData);
   void FormatExt(const CString& control, CString& strLine);

   void SetArticleNumber (LONG articleNumber) { SetNumber(articleNumber); }
   LONG GetArticleNumber (void) {return GetNumber(); }

   void     SetDate(LPCTSTR date_line);
   LPCTSTR  GetDate(void);

   // used when saving off the Create Window
   void AddNewsGroup (LPCTSTR ngroup);
   void GetNewsGroups (CStringList* pList) const;
   int GetNumOfNewsGroups () const;
   void ClearNewsGroups ();

   BOOL  operator<= (const TArticleHeaderInner & rhs);

   void           SetControl (LPCTSTR control);
   LPCTSTR        GetControl ();

   void           SetDistribution(LPCTSTR dist);
   LPCTSTR        GetDistribution();

   void           SetExpires(LPCTSTR dist);
   LPCTSTR        GetExpires();

   // user sets what groups to Followup-To
   void           SetFollowup(LPCTSTR follow);
   LPCTSTR        GetFollowup(void) const;

   void           SetOrganization(LPCTSTR org);
   LPCTSTR        GetOrganization();

   void            SetKeywords(LPCTSTR keywords);
   LPCTSTR         GetKeywords(void);

   void            SetSender(LPCTSTR sndr);
   LPCTSTR         GetSender(void);

   void           SetReplyTo(LPCTSTR replyto);
   LPCTSTR        GetReplyTo(void);

   void           SetSummary(LPCTSTR sum);
   LPCTSTR        GetSummary(void);

   void     SetXRef(LPCTSTR xref);
   LPCTSTR  GetXRef(void);

   void            SetFrom (const CString & from);
   CString         GetFrom ();

   const CString & GetOrigFrom(void);
   void            SetOrigFrom(const CString & f);

   const CString & GetPhrase(void);
   void            SetPhrase(const CString & p);

   void            SetMessageID (LPCTSTR msgid);
   virtual LPCTSTR GetMessageID(void);

   void            SetSubject(LPCTSTR subject);
   LPCTSTR         GetSubject (void);

   void    GetDestList(TStringList* pList) const;

   void SetCustomHeaders (const CStringList &sCustomHeaders);
   const CStringList &GetCustomHeaders () const;

   void Format (CString& strLine);

   // used by GenerateSaysLine.  This is NOT meant to be a bulletproof func.
   int  ParseFrom (CString& phrase, CString& address);

   // References Line
   void   SetReferences (const CString& refs);
   BOOL   AtRoot(void);
   BOOL   FindInReferences(const CString& msgID);
   int    GetReferencesCount();
   void   CopyReferences(const TArticleHeaderInner & src);
   void   ConstructReferences(const TArticleHeaderInner & src, const CString& msgid);
   void   GetDadRef(CString& str) ;
   void   GetFirstRef(CString& str);
   void   GetReferencesWSList(CString& line);
   void   GetReferencesStringList(CStringList* pList);
   const TFlatStringArray &GetReferences ();
   void SetMimeLines(LPCTSTR ver, LPCTSTR type, LPCTSTR encode, LPCTSTR desc);
   void GetMimeLines(CString* pVer, CString* pType, CString* pCode, CString* pDesc);

   long GetScore () { return m_lScore + m_lBodyScore; }
   void SetScore (long lScore) { m_lScore = lScore; }
   void AddToScore (long lAdd) { m_lScore += lAdd; }
   void SetBodyScore (long lScore) { m_lBodyScore = lScore; }
   void AddToBodyScore (long lAdd) { m_lBodyScore += lAdd; }

   // just a long string. From the Post window, used to save the Email
   //   addresses of folks, during Save-To-Draft operation.
   int  GetCCEmailString (CString & strOutput) const;
   void SetCCEmailString (const CString & strInput);

   void SetPileScore (long lScore) { m_lPileScore = lScore; }
   long GetPileScore () { return m_lPileScore; }
   void SetPileZeroPresent (BOOL bZeroPresent) { m_bPileZeroPresent = bZeroPresent ? true : false; }
   long GetPileZeroPresent () { return m_bPileZeroPresent; }

protected:
   BOOL  ParseUsenetDate();
   BOOL  parse_date_helper (LPCTSTR lpszDate);

   void  common_construct();
   void  partSerialize(CArchive & ar);
   void  parse_from(CString& phrase, CString& address);

   void     SetAtGrow(TBlobMaster::EFlatField eIdx, LPCTSTR strInput);
   LPCTSTR  GetBlobString(TBlobMaster::EFlatField eIdx) const;
   //void     remove_fld(LONG& idx);

   void  serialize_CustomHeaders ( CArchive & ar );

protected:

   WORD                 m_phraseIdx1;
   WORD                 m_phraseCnt;
   WORD                 m_addrIdx1;
   WORD                 m_addrCnt;
   BYTE                 m_fAddrAvailable;

   TBlobMaster m_sBlob;

   TFlatStringArray  m_DestArray;
   T822References    m_822References;

   CStringList * m_psCustomHeaders;
   CStringArray m_vRandomStrings;
      // index-0 is ProxicomIconName
      // index-1 is Protected
      // index-2 is used for SetCCEmailString
      // index-3 is used for SetPhrase

      // rest is reserved for
      //  the future

   long m_lScore;
   long m_lBodyScore;

//   LONG m_lDisplayFromStart;  ??  jan-20-1999 amc
//   LONG m_lDisplayFromLen;    ??  jan-20-1999 amc

   CString  m_strFrom;  // new in version 7

private:
   long m_lPileScore;            // not serialized
   bool m_bPileZeroPresent;      // not serialized
};

///////////////////////////////////////////////////////////////////////////
// Header for Email Messages
//
///////////////////////////////////////////////////////////////////////////

class TEmailHeaderInner : public TPersist822HeaderInner
{
public:
   // version history:
   //  1: don't know
   //  3: (jumped straight to version 3) introduced the attachment stuff
   //     in TPersist822HeaderInner
   enum { kVersion = 3 };
   virtual TPersist822HeaderInner* duplicate();

public:
   TEmailHeaderInner() {}   // for serialization
   TEmailHeaderInner(BYTE objectVersion);
   ~TEmailHeaderInner() {}

#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif
   virtual void Serialize (CArchive & archive);

   // assignment
   TEmailHeaderInner & operator=(const TEmailHeaderInner & rhs);

   // copy-constructor
   TEmailHeaderInner(const TEmailHeaderInner & src);

// Methods

   void    Set_InReplyTo (LPCTSTR irt) { m_in_replyTo = irt; }
   LPCTSTR Get_InReplyTo ()            { return m_in_replyTo; }

   // Get count of address in TO, CC, or BCC
   int     GetDestinationCount(TBase822Header::EAddrType eAddr);

   // TO - take comma list and put items into string list
   void    ParseTo (const CString & to, TStringList * pOutList = 0,
                    CStringList * pErrList = 0);

   // TO - get the displayable text, separated by commas
   void    GetToText (CString& txt);

   // CC - store the comma separated list of addresses
   void ParseCC(const CString& comma_sep_CC_list, TStringList * pFinal = 0,
                CStringList * pErrList = 0);

   // CC - get the displayable text, separated by commas
   void GetCCText(CString& txt);

   // BCC - store the comma separated list of addresses
   void ParseBCC(const CString& comma_sep_BCC_list, TStringList * pOutList = 0,
                 CStringList * pErrList = 0);

   // BCC - get the displayable text, separated by commas
   void GetBCCText(CString& txt);

   void GetDestinationList (TBase822Header::EAddrType eAddr, TStringList * pstrList) const;

   void            SetFrom(const CString & from)  { m_From = from; }
   const CString&  GetFrom(void)          { return m_From; }

   void            SetKeywords(LPCTSTR key) { m_Keywords = key; }
   const CString&  GetKeywords(void)        { return m_Keywords; }

   void            SetMessageID(LPCTSTR msgid)  { m_MsgID = msgid; }
   LPCTSTR         GetMessageID(void)     { return m_MsgID; }

   void SetMimeLines(LPCTSTR ver, LPCTSTR type, LPCTSTR encode, LPCTSTR desc);
   void GetMimeLines(CString* pVer, CString* pType, CString* pCode, CString* pDesc);

   void            SetSubject(LPCTSTR subj)     { m_Subject = subj; }
   const CString&  GetSubject(void)       { return m_Subject; }

   void            SetSender(LPCTSTR sender) { m_Sender = sender; }
   const CString&  GetSender(void)        { return m_Sender; }

   void            SetReplyTo(LPCTSTR replyTo) { m_replyTo = replyTo; }
   const CString&  GetReplyTo(void)       { return m_replyTo; }

protected:
   void parse_into_stringlist (const CString& comma_list, TStringList& tstrList);

   // parse comma separated list
   void parse_addr (LPTSTR strAddr, TStringList* pstrList, CStringList * pErrList);

   // get the displayable text, separated by commas
   void get_addr_text (TStringList & strList, CString& str);
   BOOL format_address(LPTSTR mailbox, LPTSTR host, LPTSTR personal,
                                  CString& outStr);

protected:
   CString     m_From;
   CString     m_MsgID;
   CString     m_Subject;
   CString     m_Keywords;    // added 10-14-95
   CString     m_Sender;
   CString     m_replyTo;
   TStringList  m_DestList;

   CString     m_MimeVersion;
   CString     m_MimeContentType;
   CString     m_MimeContentEncoding;
   CString     m_MimeContentDescription;

   CString  m_in_replyTo;

         //  email has in addition  : in reply to <msgid8234@xx>
   // make this a 'String'
   TStringList    m_CC;
   TStringList    m_BCC;
};
