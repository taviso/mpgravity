/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: outbox.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:39  richard_wood
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

#include "pobject.h"
#include "statsrch.h"
#include "declare.h"
#include "grpdb.h"

// ----------------------------------------
class TOutboxInternal : public PObject
{
public:
   //DECLARE_SERIAL(TOutboxInternal);
   TOutboxInternal();
   ~TOutboxInternal();
   virtual void Serialize(CArchive& ar);

   void AddItem(int artInt, WORD status, time_t readTime = 0);
   void RemoveItem(int artInt);
   BOOL PeekArticle(WORD status,int* pArtInt);
   BOOL PeekEmails(WORD status,CDWordArray* pIntArray);

   // return 0 for success, -1 for not found, +1 for bitset already
   int  Mark(int artInt, WORD status);

   BOOL ResetInterruptedMessages(void);
   BOOL GetStatus (int artInt, StatusUnitI *pStatus);
   BOOL RetrySend (int artInt);
   void GetAll (CDWordArray * pIntArray);

   void CountWaitingAndSending(int& wait, int& sending, int& errors);

protected:  // Functions
   BOOL peek_util(WORD status, BOOL fCountArticles, int* pArtInt);

protected:
   TStatusSearch m_StatusSearch;
};

// abstract base class for (TOutbox,  TDraftMsgs)
// smart pointer class
class TMetaOutbox : public CObject, public TNewsGroupDB
{
public:
   TMetaOutbox(TNewsServer *pServer);
   virtual ~TMetaOutbox() = 0;

   // override TNewsGroupDB function
   virtual const CString & GetName () {return m_name;}
   void SetName (LPCTSTR name)        {m_name = name;}

   TOutboxInternal* operator->() { return m_pRep; }

   void Serialize(CArchive& arc);

protected:
	   void SaveMessage (TPersist822Header * pHdr, TPersist822Text *pBody);

protected:
   TOutboxInternal* m_pRep;

   CString         m_name;
};

class TOutbox : public TMetaOutbox
{
public:
   TOutbox(TNewsServer *pServer);
   ~TOutbox();

   // the following two funcs are because of our derivation
   // from TNewsGroupDB - it isn't serialized, but is used
   // to determine file names for the outbox...

   void SaveEmail (TEmailHeader *& pHdr, TEmailText * pText);

   void SaveArticle (TArticleHeader *& pHdr, TArticleText * pText);

   void Open ();
   void DoPurging ();

   // shell so we can verify once
   BOOL PeekArticle(WORD status,int* pArtInt);
   BOOL PeekEmails(WORD status,CDWordArray* pIntArray);
   void CountWaitingAndSending(int& wait, int& sending, int& errors);

protected:

   void InitOnce ();
   void ResetInterruptedMessages ();
   void Verify ();

   // controls how often we call ResetInterruptedMessages()
   static BYTE     m_byResetForInstance;
};

// ==========================================================
class TDraftMsgs : public TMetaOutbox
{
public:
      TDraftMsgs(TNewsServer *pServer)
         : TMetaOutbox(pServer)
         {
         // empty
         SetName(_T("drafts"));
         }

   void SavingNewCopy (LONG lArticle);

   void EditingArticle (LONG lArticle);
   BOOL IsEditingArticle (LONG lArticle);
   void NotEditingArticle (LONG lArticle);
   void AbortingEdit (LONG lArticle);

   void SaveDraftMessage (TPersist822Header * pHdr, TPersist822Text *pBody);

private:
	   std::set<int>  m_stlsetEditing;     // list of article IDs being edited
};
