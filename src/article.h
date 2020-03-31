/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: article.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.4  2008/09/19 14:51:10  richard_wood
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

// disable warnings about "unreferenced formal parameter"
#pragma warning ( disable : 4100 )
#include "articlei.h"
#include "memshak.h"

/********************************************************************
Outer representations

TBase822Header
   TPersist822Header
      TEmailHeader
      TArticleHeader

********************************************************************/

class TArticleHeader;
class TArtUIData;

///////////////////////////////////////////////////////////////////////////
//  Outer representation
//
class TPersist822Header : public TBase822Header
{
protected:
   TPersist822Header();          // protected constructor!

   // copy-constructor
   TPersist822Header(const TPersist822Header& src);

   void copy_on_write();         // part of reference counting

public:
   static int DecodeElectronicAddress(const CString & rawAddress, CString & prettyAddress)
		{
		return TPersist822HeaderInner::DecodeElectronicAddress( rawAddress, prettyAddress );
		}

   LONG HashKey () { return m_pRep->HashKey(); }
   virtual ~TPersist822Header();

#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif

   // this is not virtual, but the inner function is.
   void Serialize (CArchive & archive);

   TPersist822Header & operator=(const TPersist822Header &rhs);

   virtual TArticleHeader* CastToArticleHeader() { return 0; }
   virtual TEmailHeader*   CastToEmailHeader()   { return 0; }

   virtual BOOL  IsArticle() { return m_pRep->IsArticle(); }

   TPersist822Text * AllocText(void) { return m_pRep->AllocText(); }

   void SetNumber(LONG n);
   LONG GetNumber(void)   { return m_pRep->GetNumber(); }

   // number of lines in the body
   void    SetLines (int lines);
   void    SetLines (const CString& body);
   int     GetLines (void)             { return m_pRep->GetLines();  }

   const CTime& GetTime(void)    { return m_pRep->GetTime(); }
   void StampCurrentTime(void);

   virtual LPCTSTR  GetMessageID(void) { return m_pRep->GetMessageID(); }

   virtual int ParseFrom (CString& phrase, CString& address)
                  { ASSERT(0); return 0;}
   void SetMimeLines(LPCTSTR ver, LPCTSTR type, LPCTSTR encode, LPCTSTR desc);
   void GetMimeLines(CString* pVer, CString* pType, CString* pCode, CString* pDesc);

   // attachment access
   int NumAttachments () const { return m_pRep->NumAttachments (); }
   const TAttachmentInfo *Attachments () { return m_pRep->Attachments (); }
   // the below parameter is void* to avoid include file dependencies. It
   // should be a pointer to an object of the following type:
   //       TAttachmentArray<TAttachmentInfo, TAttachmentInfo&>
   void SetAttachments (void *psAttachmentsVoid)
      { m_pRep->SetAttachments (psAttachmentsVoid); }

   // removed 5-10-96 amc
   //virtual void GetDestList(TStringList* pList) const
   //              { ASSERT(0); }

protected:
   union
      {
      TPersist822HeaderInner * m_pRep;
      TArticleHeader * m_pNext;           // used by TArticleHeader::operator delete
      };

private:
   // NOTE: ask Al before using this function
   void PrestoChangeo_AddReferenceCount();   // BEWARE! reference counting!
   void PrestoChangeo_DelReferenceCount();   // BEWARE! reference counting!
};

///////////////////////////////////////////////////////////////////////////
// Article object outer representation
//
class TArticleHeader : public TPersist822Header
{
public:
   static ULONG ShrinkMemPool();
   static TMemShack m_gsMemShack;

   PVOID  m_pvBinary; //BYTE   m_byMemberCompleteBinary;
public:
   TArticleHeader(void);
   ~TArticleHeader();
   TArticleHeader & operator=(const TArticleHeader &rhs);

public:
#if defined(_DEBUG)
   virtual void Dump(CDumpContext& dc) const;
#endif

public:
   TArticleHeader* CastToArticleHeader() { return this; }
   TEmailHeader*   CastToEmailHeader()   { return 0; }

   // return 1 if arthdr becomes dirty
   int  MakeUIData (TArtUIData * & rpUIData)
         { return GetPA()->MakeUIData(rpUIData); }

   void FormatExt(const CString& control, CString& strLine);

   void SetArticleNumber (LONG artInt);
   LONG GetArticleNumber (void) {return m_pRep->GetNumber(); }

   void     SetDate(LPCTSTR date_line);
   LPCTSTR  GetDate(void);

   // used when saving off the Create Window
   void AddNewsGroup (LPCTSTR ngroup);
   void GetNewsGroups (CStringList* pList) const;
   int GetNumOfNewsGroups () const;
   void ClearNewsGroups ();

   BOOL  operator<= (const TArticleHeader & rhs);

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

   void            SetQPFrom (const CString & from);

   void            SetMessageID (LPCTSTR msgid);
   LPCTSTR         GetMessageID(void);

   void            SetSubject(LPCTSTR subject);
   void            SetQPSubject(LPCTSTR subject, LPCTSTR pszFrom = NULL);
   LPCTSTR         GetSubject (void);

   void    GetDestList(TStringList* pList) const;

   void SetCustomHeaders (const CStringList &sCustomHeaders);
   const CStringList &GetCustomHeaders () const;

   void Format (CString& strLine);

   // used by GenerateSaysLine.  This is NOT meant to be a bulletproof func.
   // returns 1 if we are dirty
   int  ParseFrom (CString& phrase, CString& address);

   // References Line
   void   SetReferences (const CString& refs);
   BOOL   AtRoot(void);
   BOOL   FindInReferences(const CString& msgID);
   int    GetReferencesCount();
   void   CopyReferences(const TArticleHeader & src);
   void   ConstructReferences(const TArticleHeader & src, const CString& msgid);
   void   GetDadRef(CString& str) ;
   void   GetFirstRef(CString& str);
   void   GetReferencesWSList(CString& line);
   void   GetReferencesStringList(CStringList* pList);
   const TFlatStringArray &GetReferences ();

   long GetScore () { return GetPA ()->GetScore (); }
   void SetScore (long lScore) { GetPA ()->SetScore (lScore); }
   void AddToScore (long lAdd) { GetPA ()->AddToScore (lAdd); }
   void SetBodyScore (long lScore) { GetPA ()->SetBodyScore (lScore); }
   void AddToBodyScore (long lAdd) { GetPA ()->AddToBodyScore (lAdd); }

   void SetPileScore (long lScore) { GetPA()->SetPileScore (lScore); }
   long GetPileScore () { return GetPA()->GetPileScore(); }
   void SetPileZeroPresent (BOOL bZeroPresent) { GetPA()->SetPileZeroPresent( bZeroPresent ); }
   long GetPileZeroPresent () { return GetPA()->GetPileZeroPresent(); }

   // just a long string. From the Post window, used to save the Email
   //   addresses of folks, during Save-To-Draft operation.

   int  GetCCEmailString (CString & strOutput) const
           { return  GetPA()->GetCCEmailString (strOutput); }

   void SetCCEmailString (const CString & strInput)
           { GetPA()->SetCCEmailString (strInput); }

private:
   TArticleHeaderInner * GetPA()
      { return static_cast<TArticleHeaderInner*> (m_pRep); }

   const TArticleHeaderInner * GetPA() const
      { return static_cast<const TArticleHeaderInner*> (m_pRep); }

   long m_lPileScore;            // not serialized
   bool m_bPileZeroPresent;      // not serialized
};

///////////////////////////////////////////////////////////////////////////
//
//
class TEmailHeader : public TPersist822Header
{
public:
   TEmailHeader ();
   ~TEmailHeader ();

   TEmailHeader & operator=(const TEmailHeader & rhs);

// Methods
   TArticleHeader* CastToArticleHeader() { return 0; }
   TEmailHeader*   CastToEmailHeader()   { return this; }

   void    Set_InReplyTo (LPCTSTR irt);
   LPCTSTR Get_InReplyTo ();

   // override
   int ParseFrom (CString& phrase, CString& address);

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

   void            SetFrom(const CString & from)  { GetPE()->SetFrom(from); }
   const CString&  GetFrom(void)          { return GetPE()->GetFrom(); }

   void            SetKeywords(LPCTSTR key) { GetPE()->SetKeywords(key); }
   const CString&  GetKeywords(void)        { return GetPE()->GetKeywords(); }

   void            SetMessageID(LPCTSTR msgid)  { GetPE()->SetMessageID (msgid); }
   LPCTSTR         GetMessageID(void)     { return GetPE()->GetMessageID (); }

   void            SetSubject(LPCTSTR subj) { GetPE()->SetSubject (subj); }
   const CString&  GetSubject(void)         { return GetPE()->GetSubject(); }

   void            SetSender(LPCTSTR sender) { GetPE()->SetSender(sender); }
   const CString&  GetSender(void)           { return GetPE()->GetSender(); }

   void            SetReplyTo(LPCTSTR replyTo) { GetPE()->SetReplyTo(replyTo); }
   const CString&  GetReplyTo(void)            { return GetPE()->GetReplyTo(); }

private:
   TEmailHeaderInner * GetPE()
      { return static_cast<TEmailHeaderInner*>(m_pRep); }

   const TEmailHeaderInner * GetPE() const
      { return static_cast<const TEmailHeaderInner*>(m_pRep); }
};
