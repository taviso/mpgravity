/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsgrp.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

#include "mplib.h"
#include "pobject.h"
#include "article.h"
#include "nglib.h"
#include "statvec.h"             // TStatusMgr
#include "sharesem.h"
#include "tglobdef.h"
#include "grpdb.h"
#include "navigdef.h"
#include "artbank.h"
#include "thredtyp.h"               // holds enums
#include "resource.h"               // IDD_RENUMBER_WARNING

class THeapBitmap;
class TIntVector;
class TArticleIndexList;
class TGlobalOptions;
class TExpiryData;
class TThreadList;
class TPileIDS;
class TRangeSetReadLock;

/////////////////////////////////////////////////////////////////////////////
// TNewsGroup - News group type - encapsulates everything about a newsgroup.
/////////////////////////////////////////////////////////////////////////////

class TNewsGroup : public PObject, public TNewsGroupDB /*, public TArtBasket*/
{
public:
friend class TArticleBank;

public:
   DECLARE_SERIAL(TNewsGroup)

   enum ENewsGroupType { kSubscribed    = 0x1,
                         kUnsubscribed  = 0x2
                       };

   enum EStorageOption { kNothing, kHeadersOnly, kStoreBodies};

   enum EPosting       { kPostInvalid    = 0x0,
                         kPostAllowed    = 0x1,
                         kPostNotAllowed = 0x2,
                         kPostModerated  = 0x3 };

   enum EPurge { /*m_byPurgeFlags*/ kPurgeOverride=0x1, kPurgeRead=0x2,
                 kPurgeUnread=0x4, kPurgeOnHdrs=0x8, kCompactOnExit=0x10 };

   // this enum collection is really bitflags for m_byProxicomFlags
   enum EFlagProxicom { kProxValidated=0x80, kProxPostingAllowed=0x40,
                        kProxNewTopicAllowed=0x20,
                        kGravInactiveGroup=0x10 };

   virtual  void Serialize (CArchive & archive);

   static TNewsGroupLibrary m_library;

public:
   static TViewFilter * GetpCurViewFilter ();

   TNewsGroup();  // constructor for serialization

   TNewsGroup(TNewsServer *               pServer,
              LPCTSTR                     lpstrName,
              TGlobalDef::ENewsGroupType  eType,
              EStorageOption*             peStorage,
              int                         iGoBack);

   ~TNewsGroup(void);

   const CString & GetName () { return m_name; }

   void  serialization_SetName (const CString & n) { m_name = n; }

   int ReloadArticles(BOOL fEmptyFirst);      // reload articles from store

   int CatchUpArticles(void);

   // dump current contents, load different type
   int LoadArticles(BOOL fFilter /*, WORD filter */);

   void NeedsRedraw(void);

   BOOL HdrRangeHave(int artInt);
   void HdrRangeAdd(int artInt);
   void HdrRangeSave(void);
   void HdrRangeSubtract(int artInt);
   void HdrRangeEmpty(void);
   void HdrRangeConvert(CList<int, int&>* pList);
   int  HdrRangeCount(void);

   BOOL TextRangeHave(int artInt);
   void TextRangeAdd(int artInt);
   void TextRangeSave(void);
   void TextRangeDump(CString & str);
   void TextRangeSubtract(TRangeSet& sub, TRangeSet& result);
   void TextRangeSubtract(int artInt);
   BOOL TextRangeDirty(void) { return BOOL(m_byRangeDirty); }
   void TextRangeEmpty(void);

   void HTRangeSubtract(int artInt);
   void HTRangeAdd(int artInt);

   BOOL ReadRangeHave(int artInt);
   void ReadRangeAdd(TArticleHeader* pHdr, KThreadList::EXPost eXPost = KThreadList::kHandleCrossPost);
   void ReadRangeAdd(int artInt, KThreadList::EXPost eXPost = KThreadList::kHandleCrossPost);

   void ReadRangeSave(void);
   void ReadRangeSubtract(TRangeSet& sub, TRangeSet& result);
   void ReadRangeSubtract(TArticleHeader* pHdr, KThreadList::EXPost eXPost);
   void ReadRangeSubtract_NoCPM (int iArtInt);

   void ReadRangeEmpty(void);

   void MissingRangeAdd(int artInt);
   BOOL MissingRangeHave(int artInt);
   void MissingRangeEmpty ();

   // both reading from Database and reading from Socket
   void ReceiveHeader (TArticleHeader* pHdr);

   // inspect newsgrp to see if (next) "unread local" exists
   BOOL QueryExistArticle (EJmpQuery eQuery);

   void UpdateFilterCount (bool fBoth = false);

public:
   int  Mode1_Thread (BOOL fOld, BOOL fNew);

   void InstallGroupID (LONG iGroupID);

   EStorageOption GetStorageOption () {return m_kStorageOption;}
   void SetStorageOption (EStorageOption kOption)
      {m_kStorageOption = kOption;}

   BOOL GetOverrideDecodeDir () { return m_bOverrideDecodeDir; }
   BOOL GetOverrideEmail () { return m_bOverrideEmail; }
   BOOL GetOverrideFullName () { return m_bOverrideFullName; }
   BOOL GetOverrideCustomHeaders () { return m_bOverrideCustomHeaders; }
   BOOL GetOverrideLimitHeaders () { return m_fOverrideLimitHeaders; }

   const CString& GetNickname () {return m_nickname;}
   const CString& GetDecodeDir () {return m_strDecodeDir;}
   const CString& GetEmail () {return m_strEmail;}
   const CString& GetFullName () {return m_strFullName;}
   const CStringList& GetCustomHeaders () { return m_sCustomHeaders; }
   int  GetHeadersLimit () { return m_iGetHeadersLimit; }

   void SetOverrideDecodeDir (BOOL bOverride) { m_bOverrideDecodeDir = bOverride; }
   void SetOverrideEmail (BOOL bOverride) { m_bOverrideEmail = bOverride; }
   void SetOverrideFullName (BOOL bOverride) { m_bOverrideFullName = bOverride; }
   void SetOverrideCustomHeaders (BOOL bOverride) { m_bOverrideCustomHeaders = bOverride; }
   void SetOverrideLimitHeaders (BOOL fOverride) { m_fOverrideLimitHeaders = fOverride; }
   void SetNickname (LPCTSTR nickname) {m_nickname = nickname;}
   void SetDecodeDir (LPCTSTR str) {m_strDecodeDir = str;}
   void SetEmail (LPCTSTR str) {m_strEmail = str;}
   void SetFullName (LPCTSTR str) {m_strFullName = str;}
   void SetCustomHeaders (const CStringList &sCustomHeaders);
   void SetHeadersLimit (int n) { m_iGetHeadersLimit = n; }

   const CString&  GetBestname ();

   // this is the initial 'go back' 2000
   void     SetRetrieveRange (int range) { m_iRetrieveRange = range; }
   int      GetRetrieveRange ()          { return m_iRetrieveRange; }

   int      GetLowwaterMark();
   void     SetLowwaterMark(int n);
   void     CalcLowwaterMark(int iServerLo, int iServerHi);

   TGlobalDef::EPurgeType GetPurgeType () {return m_kPurgeType;}
   void SetPurgeType (TGlobalDef::EPurgeType kPurgeType)
      {m_kPurgeType = kPurgeType;}
   LONG     GetPurgeDays () {return m_lPurgeDays;}
   void     SetPurgeDays (LONG lPurgeDays) {m_lPurgeDays = lPurgeDays;}

   BOOL     UseGlobalPurge () {return m_fUseGlobalPurge;}
   void     SetUseGlobalPurge (LONG fUseGlobal) {m_fUseGlobalPurge = fUseGlobal;}

   BOOL     UseGlobalStorageOptions () {return m_fUseGlobalStorageOptions;}
   void     SetUseGlobalStorageOptions (BOOL fUse)
      {m_fUseGlobalStorageOptions = fUse;}

   int      StatusAdd_New(int artInt, WORD status = TStatusUnit::kNew);

   void     Collect_Old(CTime threshold, TRangeSet* pRangeSet);

   BOOL     IsStatusBitOn(int artInt, TStatusUnit::EStatus status);
   int      iStatusDirect(int artInt, WORD& wResult);
   int      iStatusRepDirect(int artInt, WORD& wResult);
   void     StatusSet(int artInt, WORD status);
   void     StatusBitSet(int                    artInt,
                         TStatusUnit::EStatus   kStatus,
                         BOOL                   fOn);
   void     StatusMarkRead(int artInt);
   void     StatusDestroy (int artInt);
   void     StatusDestroyAll(void);
   void     StatusCountNew(int& totNew, int& tot);
   void     StatusCountProtected (int & iProtected);
   BOOL     ArticleStatus (int artInt, WORD& wStatus, BOOL& fHave);

   void     SetDirty();
   BOOL     GetDirty(int* pCount = NULL);
   void     ClearDirty();

   void     Empty();

   BOOL     FindThread (TArticleHeader* pArtHdr, TThread*& pFamily);
   BOOL     FindThreadPile (TArticleHeader* pArtHdr, TThreadPile*& pPile);

   EPosting GetPostingState() const { return m_PostingState; }

   BOOL     GetpHeader (int artInt, TArticleHeader*& rpHeader);

   int  FlatListLength();

   int  CreateArticleIndex (TArticleIndexList * pIndexList);
   int  LoadForArticleIndex (TViewFilter * pFilter, BOOL fCreateStati, BOOL fHitServer,
                             TArticleIndexList * pIndexList);

   void MarkThreadRead (TArtNode* pArtNode);
   int  MarkThreadStatus (TArticleHeader* pArtHdr, TStatusUnit::EStatus eStatus,
                          BOOL fBitOn = TRUE);

   int  MarkPileStatus   (TArticleHeader* pArtHdr, TStatusUnit::EStatus eStatus,
                          BOOL fBitOn = TRUE);

   // moved from tasker.cpp
   void adjust_bound(BOOL fGetAccordingToGroup, int iHdrsLimit,
                     const POINT & ptServer,
                     TRangeSet* pLumper, TRangeSet* pArtPing);

   // show user what's new right after connecting
   void SetServerBounds(int server_low, int server_hi);

   void GetServerBounds(int& lo, int& hi)
      { lo = m_iServerLow; hi = m_iServerHi; }

   void FormatNewArticles(int& iLocalNew, int& iServerNew, int & iLocalTotal);
   BOOL CheckHeadersAgainstServerLowRange(TRangeSet* pRange);

   BOOL NeedsPurge();
   void SetPurgeTime(const CTime& currentPurgeTime);

   BOOL IsPurgeOverride();
   void SetPurgeOverride(BOOL fOver);

   BOOL GetPurgeRead();
   void SetPurgeRead(BOOL fPrgRead);
   int  GetPurgeReadLimit ()   { return (int)m_wPurgeReadLimit; }
   void SetPurgeReadLimit (int lmt)  { m_wPurgeReadLimit = (WORD) lmt; }

   BOOL GetPurgeUnread();
   void SetPurgeUnread(BOOL fPrgUnread);
   int  GetPurgeUnreadLimit ()   { return (int)m_wPurgeUnreadLimit; }
   void SetPurgeUnreadLimit (int lmt)  { m_wPurgeUnreadLimit = (WORD) lmt; }

   BOOL GetPurgeOnHdrs();
   void SetPurgeOnHdrs(BOOL fPrgOnHdrs);
   int  GetPurgeOnHdrsEvery ()   { return (int)m_wPurgeOnHdrsEvery; }
   void SetPurgeOnHdrsEvery (int ev)  { m_wPurgeOnHdrsEvery = (WORD) ev; }

   BOOL NeedsCompact();
   BOOL GetCompactOnExit();
   void SetCompactOnExit(BOOL fCompact);
   int  GetCompactOnExitEvery ()   { return (int)m_wCompactOnShutdownEvery; }
   void SetCompactOnExitEvery (int ev)  { m_wCompactOnShutdownEvery = (WORD)ev; }

   void GetLastPurgeTime (CTime* pTime)   { *pTime = m_lastPurgeTime; }
   void GetLastCompactTime (CTime* pTime) { *pTime = m_lastCompactTime; }
   void SetLastCompactTime (const CTime& t) { m_lastCompactTime = t; }

   // if changing this prototype, change grpdb.h also
   virtual void PurgeByDate (bool fWriteOutSubscribedGroups = true);
   void ExpireArticles (TExpiryData* pExpiryData);

   // added 4-8-98
   virtual void MonthlyMaintenance ();

   void StorageTransition (TNewsGroup::EStorageOption eOldStoreMode);

   CString GetType () {return CString ("TNewsGroup");}

   void DisplayMode1_CalculateWhatToGet();

   // override a virtual
   virtual void PurgeHeader (int artNum);

   BOOL FindNPileContents (TPileIDS* psIDS);

   BOOL GetHighestReadArtInt (int* piHigh);
   BOOL ResetHighestArticleRead (int iNewHi, int iOldHi);

   void UpgradeNeedsGlobalOptions (TGlobalOptions * pOptions);

   int GetNumArticles ()
      { return m_statusMgr.GetStatusVectorLength (); }

   void Sample (BOOL bSample) { m_bSample = bSample; }
   BOOL IsSampled () { return m_bSample; }

   virtual void SetServerName (const CString& strServerName);

   int GetFilterID () { return m_iDefaultFilter; }
   void SetFilterID (int ID) { m_iDefaultFilter = ID; }

   TThreadList *  GetpThreadList () { return m_pThreadList; }

   void CalculatePingSet (bool fOnlyUnread, TRangeSet * pPing);

   // repairing
   int ValidateStati ();

   // this is Public, but only should be used with caution
   int validate_status_vector ();

   bool IsActiveGroup ()
      { return (m_byProxicomFlags & kGravInactiveGroup) ? false : true; }
   void SetActiveGroup (BOOL fActive)
      { if (fActive)
           m_byProxicomFlags &= ~kGravInactiveGroup;
        else
           m_byProxicomFlags |= kGravInactiveGroup;
      }

   int  GetLocalBodies (CDWordArray & rNums);

   BOOL  GetUseSignature () { return m_fOverrideSig; }
   void  SetUseSignature (BOOL f) { m_fOverrideSig = f; }
   const CString &  GetSigShortName () { return m_strSigShortName; }
   void  SetSigShortName (const CString & s) { m_strSigShortName = s; }

private:
   BOOL read_range_have (TRangeSetReadLock * psLock, int artInt);

   void storeSerialize (CArchive & ar);
   void loadSerialize  (CArchive & ar, BYTE byObjectVersion);

protected:
   void Common_Construct (int iGoBack);
   int ForegroundLoadArticles (EStorageOption eStor, BOOL fHitServer,
                               TViewFilter * pVF);

   // void wait_for_headers (BOOL fCreateStatus, TThreadList &sThreadList);
   void load_headers (TThreadList& sThreadList, WORD iFilter,
                      BOOL bCreateStati, void* pVoid);

   void excluded_add (int i, TRangeSet* pRangeSet);

   // override
   void PreDeleteHeaders ();

   int  adjust_bound_headers_only(int first, int last, int iHdrLimit,
                                  TRangeSet* pLumper,
                                  BOOL fPing, TRangeSet* pArtPing);
   int  adjust_bound_store_bodies(int first, int last, int iHdrLimit,
                                  TRangeSet* pLumper,
                                  BOOL fPing, TRangeSet* pArtPing);

   void read_range_util (int artInt, TArticleHeader* pHdr, KThreadList::EXPost eXPost,
                         BOOL fMarkSeen);
   void read_range_chglocal (int artInt, BOOL fMarkSeen);

   void catchup_local (KThreadList::EXPost eXPost);
   void catchup_server (BOOL fCPM);
   void catchup_status (BOOL fKeepTags);

   int  deferred_thread (BOOL fEmpty);
   void repair_statusunit (int artInt);

   int  build_request_set (TRangeSet* pHave, int first, int last, int iHdrLimit,
                           TRangeSet* pLumper);

   void expire_articles_list (TExpiryData* pExpiryData);

protected:
   // the thread list uses the ArticleBank, So the bank must be
   // defined first.  On shutting down, the thread list is destructed,
   // then the bank is destructed.
   TArticleBank         m_ArtBank;

public:
   LONG              m_GroupID;
   ENewsGroupType    eType;

protected:
   //  this is a ptr now to cut down on dependencies
   TThreadList *        m_pThreadList;      // not serialized
   CString              m_name;
   //BOOL                 m_fLoaded;              // ??
   //BOOL                 m_DatabaseHasMore;      // ??

   TRangeSet            m_HdrRange;     // tells us Hdr is here
   TRangeSet            m_TextRange;    // tells us body is in Database
   TRangeSet            m_MissingRange; // articles that are not on the
                                        //   server.  'holes'
   LONG                 m_byRangeDirty;

   BOOL                 m_bOverrideDecodeDir;
   BOOL                 m_bOverrideEmail;
   BOOL                 m_bOverrideFullName;
   BOOL                 m_bOverrideCustomHeaders;
   BOOL                 m_fOverrideLimitHeaders;
   BOOL                 m_fOverrideSig;

   BYTE                 m_byPurgeFlags;
   EStorageOption       m_kStorageOption;
   CString              m_nickname;
   CString              m_strDecodeDir;
   CString              m_strEmail;
   CString              m_strFullName;
   CStringList          m_sCustomHeaders;
   LONG                 m_iGetHeadersLimit;
   CString              m_strSigShortName;

   LONG                 m_iRetrieveRange;      // "go back" 2000
   LONG                 m_iSubscribeLowMark;   // lowest art number from subscribing
   TGlobalDef::EPurgeType  m_kPurgeType;
   LONG                 m_lPurgeDays;
   LONG                 m_fUseGlobalPurge;
   LONG                 m_fUseGlobalStorageOptions;
   EPosting             m_PostingState;

   // 0x80 = validated this session         (proxicom junk)
   // 0x40 = general posting allowed        (proxicom junk)
   // 0x20 = post/create new topic allowed  (proxicom junk)
   // 0x10 = inactive group                 (our junk added 4/15/99)
   //
   BYTE                 m_byProxicomFlags;
   BYTE                 m_save[2];
   LONG                 m_iDefaultFilter;
   LONG                 m_iServerLow;
   LONG                 m_iServerHi;

   TStatusMgr           m_statusMgr;

   // type is from declare.h  Bucket.H is not appropriate
   TP822HdrArray        m_hdrBucket;

   WORD                 m_wPurgeReadLimit;
   WORD                 m_wPurgeUnreadLimit;
   WORD                 m_wPurgeOnHdrsEvery;
   WORD                 m_wCompactOnShutdownEvery;

   CTime                m_lastPurgeTime;
   CTime                m_lastCompactTime;
   BYTE                 m_byPingReadArticles;   // not serialized

   BOOL                 m_bSample;              // not serialized

private:
   void NextXRefsToken (const char *pchSrc, int &iPos, CString &strDest);
   void ProcessXRefs (LPCTSTR pchRefs, BOOL fMarkSeen);

private:
   BOOL     m_bLoadArticlesMutex;   // used in LoadArticlesIntoThreadList()
};

// utility struct linked with WMU_GROUPRENUMBERED_WARNING

struct TYP_GROUPRENUM
{
   int     m_iServerLow;    // latest lowwater mark from Server
   int     m_iPreviousLow;
   CString m_strNewsGroup;
};

typedef TYP_GROUPRENUM * PTYP_GROUPRENUM;
