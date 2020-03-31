/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artbank.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:09  richard_wood
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

#include "afxmt.h"

//////////////////////////////////////////////////////////////////////////////
// This class acts as an intermediate between the TNewsGroup and the actual
//   news source (be it the database or the newsserver).  The unified API
//   should make Mode-1 code a little simpler
//
// When we build a thread list, the TArticleBank holds a reference to the
//   articles.
//
// Example: the filter is NEW
// The ArticleBank will hold the New&Read messages, but will only give
//   access to New messages. So only new messages are added into the
//   thread list.
//
// Filtering by Important:  the bank is full, and we just access the
//   important articles
//
// Deferred Rules: right now we filter/thread/deferred-rule
//   the DR may mark something as read, and to the user, it looks like
//   the filter did not work.
//   note - this works ok if the action is to mark something important
//
// I.  Goals
//   A)  Simplify mode 1. Make the NewsBank responsible for getting
//       what it needs.
//      1. the database and the mode-1 action should look the same to
//         clients of the NewsBank object
//      2. localize the code for calculating "What I have/what I need"
//   B)  Don't reload articles for Mode 1. They are cached in the newsbank
//   C)  make deferred rules work
//
// II. Behavior
//
//     the filter buttons just filter the "view" on the NewsBank.
//     a dblclik on the mode 1 newsgroup will fetch incrementally from the
//     server
//     - if you keep an article on screen for 24 hours, yes the body may
//       expire from the server
//     - see paper diagram
//
// III.  Example Interaction
//      1 NG:    View New & Read
//      2 Bank:  I have new. I need read.  What do you have
//      3 NG:    <returns a range>
//      4 Bank:  figures.  "Get These for me"
//      5 NG:    tasker fills the Bank with Headers
//      6 Bank:  ok.
//      7 NG:    I will thread now. <uses Bank + filter + NG>
//      8 NG:    run deferred rules on threads
//      9 NG:    2nd pass thru filter?
//     10 NG:    use threadlist to fill UI
//

#include "declare.h"
#include "mplib.h"
#include "servcp.h"
#include "superstl.h"

class ob_artnode_ptr
{
public:
   ob_artnode_ptr()
      {
      pNode = 0;
      }

    ob_artnode_ptr(TArtNode * p)
      {
      pNode = p;
      }

   ob_artnode_ptr(const ob_artnode_ptr & src)
      {
      pNode = src.pNode;
      }

   ~ob_artnode_ptr()
      {
      }

   BOOL operator <(const ob_artnode_ptr & rhs)
      {
      return DWORD(pNode) < DWORD(rhs.pNode);
      }

   operator TArtNode*() {return pNode; }
   TArtNode * pNode;
};

class ob_less_artnodeptr {
public:
   bool operator()(const ob_artnode_ptr & x,
                   const ob_artnode_ptr & y) const
   {
      return DWORD(x.pNode) < DWORD(y.pNode);
   }
};

typedef set<ob_artnode_ptr, ob_less_artnodeptr>  STLMapArtNode;

class TArticleBank : public CObject /*, public TArtBasket */{
friend class TThreadList;

public:
   // this typedef is based in this class to keep global namespace clean
   typedef map<CString, TArtNode*> NodeMap;

public:
   // serialization ctor
   TArticleBank();

   // ctor - dtor
   TArticleBank(TNewsServer* pServer);
   ~TArticleBank();

   int CreateArticleIndex (TArticleIndexList * pIndexList);

   BOOL insert_headers(PVOID pVoid,
                      TArticleHeader* pArtHdr,
                      TNewsGroup * pNG);

   bool GetIteratorEx( STLMapArtNode::iterator & itBegin,
                       STLMapArtNode::iterator & itEnd );

   static void translate_filter(TViewFilter * pFilter, BOOL& fOld, BOOL& fNew);

   int  ActiveFilter (TViewFilter* pFilter, TNewsGroup* pNG);

   int LoadFromDB (TViewFilter* pFilter, TNewsGroup* pNG);

   // fills bank with articles according to filter
   int  ChangeContents(BOOL fHitServer,
                       TViewFilter * pFilter, TNewsGroup* pNG,
                       BOOL& fLink);

   void TagContents(BOOL fOld, BOOL fNew);

   BOOL StageLookup(TArticleHeader* pArtHdr);

   int  Empty ();

   bool    GetStartPositionEx(TViewFilter* pFilter);

   BOOL     Lookup(CString& key, TArtNode*& rpNode);
   BOOL     UnhookNode ( TArtNode * pNode );
   void     DeleteNode ( TArtNode * & rpNode );

   void InitHashTable(UINT uSize);
   int  GetCount();

   void Lock () { m_crit.Lock(); }
   void Unlock () { m_crit.Unlock(); }

   void SetServerName (const CString& strServerName)
      {
      m_ServerName = strServerName;
      }

   void PuntLinkInformation ();
   int  SanityCheck_CountNodes ();

private:
   int load_from_db (TViewFilter * pViewFilter, TNewsGroup* pNG);
   int artid_we_need(BOOL fOld, BOOL fNew, TNewsGroup* pNG, int lo, int hi,
                     int iHdrLimit, TRangeSet* pSet);

   int change_contents_mode2 (TViewFilter * pFilter, TNewsGroup* pNG);

   int change_contents_mode1_server(TViewFilter * pFilter, TNewsGroup* pNG);
   int change_contents_mode1_internal(TViewFilter * pFilter, TNewsGroup* pNG);
   BOOL need_server(TViewFilter * pFilter);
   void isolate_all_nodes(void);

   BOOL filter_important(TNewsGroup* pNG, int iArtInt);

   void replace_filter (TViewFilter * pFilter);
   TNewsServer* getp_server ();
protected:
   CCriticalSection m_crit;      // define me first

   NodeMap   m_Stage;
   DWORD     m_dwFlags;
   TRangeSet m_ids;
   TViewFilter * m_pMyViewFilter;

   BOOL      m_fHaveOld;
   BOOL      m_fHaveNew;
   BYTE      m_fShowImportant;

   TNewsServer* m_pServer;       // cached pointer
   CString   m_ServerName;

   STLMapArtNode   m_setNodes;
};
