/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: pumpjob.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:42  richard_wood
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

//TPumpJob
//  TPumpJobPingGroup
//    TPumpJobEnterGroup
//  TPumpJobArticle
//  TPumpJobPost
//  TPumpJobOverview
//  TPumpJobEndOverview
//  TPumpJobHelp
//  TPumpJobConnect
//  TPumpJobBigList
//  TPumpJobIconTime      - Proxicom Forum thing
//  TPumpJobIconData      - Proxicom Forum thing

#include "tstrlist.h"
#include "callbks.h"
#include "mplib.h"
#include "pobject.h"
#include "enumjob.h"

#include "superstl.h"

class TNewsGroup;
class TArticleHeader;
class TArticleText;
class TFetchArticle;
class TArtBasket;

// PJF_BLOCKING             synchronous. release waiting thread
// PJF_SEND_SCRIBE          send text to scribe
// PJF_SEND_UPPERLAYER      send text to ui, search thread
// PJF_RUN_RULES            apply rules since text is Not going to scribe
// PJF_MARK_READ            mark as read after downloading
// PJF_MARK_DOCERROR        failure should get pink X document icon kSendError
// PJF_TAG_JOB              tag job. untag when done or failure

///#define PJF_MARK_DOCERROR        0x0020   /* upper layer should do this */

//efine PJF_BLOCKING             0x0001
//efine PJF_SEND_UPPERLAYER      0x0004  pFetchObj
#define PJF_SEND_SCRIBE          0x0002

#define PJF_RUN_RULES            0x0008
#define PJF_MARK_READ            0x0010
#define PJF_TAG_JOB              0x0040

class TPumpJob : public CObject
{
public:
	enum EPumpJobDone {
		kDoneShow,            // redisplay group when done
		kDoneNothing          // download group and do nothing
	};

	enum EPumpJobType {
		kJobGroup,              // look at group, start retrieving
		kJobArticle,            // retreive body as text
		kJobPost,               // post an article
		kJobOverview,           // get overview
		kJobEndOverview,        // end of overviews.
		kJobHelp,               // send "help" command
		kJobConnect,            // asynchronous connect
		kJobPingGroup,          // look at group - don't retrieve
		kJobBigList,            // fetch the big list of newsgroups
		kJobPingMultiGroup,     // ping groups as a set
		kJobPingArticles        // test if local articles have expired on server
	};

#if defined(_DEBUG)
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	TPumpJob ();
	virtual ~TPumpJob() {}
	virtual EPumpJobType GetType(void) = 0;
	virtual TPumpJob *  Resubmit () { return 0; }

	TPumpJob(const TPumpJob& src);
	TPumpJob& operator=(const TPumpJob& rhs);

	EPumpJobDone GetDisposition() { return m_eJobDone; }
	void CopyDisposition (TPumpJob * pSrcJob)
	{
		m_eJobDone = pSrcJob->m_eJobDone;
	}

	// make this a little more object oriented

protected:
	EPumpJobType   m_kJobType;
	EPumpJobDone   m_eJobDone;
};

// enter group and get the range
class TPumpJobPingGroup : public TPumpJob
{
	friend class TPump;

protected:         // used only by TPump
	TPumpJobPingGroup(const CString & grp);
	TPumpJobPingGroup(const TPumpJobPingGroup& src);
	TPumpJobPingGroup& operator=(const TPumpJobPingGroup& rhs);

public:
	virtual ~TPumpJobPingGroup() {}

	const CString & GetGroup(void) { return m_NewsGroup; }
	EPumpJobType   GetType(void) { return m_kJobType; }
	void SetGroupResults(BOOL fOK, int ret, int first, int last);
	BOOL  IsOK(void) { return m_fOK; }

	void GetGroupRange(int& first, int& last)
	{
		first = m_GroupFirst; last = m_GroupLast;
	}
	//void SetOldNew(BOOL  fOld, BOOL  fNew);
	//void GetOldNew(BOOL& fOld, BOOL& fNew);
	BOOL fRefreshUI;

	virtual BOOL Release() { return FALSE; }
	int      m_iRet;
	CString  m_Ack;

protected:
	// kJobGroup
	CString        m_NewsGroup;
	BYTE  m_OldNew;

	// -- results of Group
	BOOL  m_fOK;
	int   m_GroupFirst;
	int   m_GroupLast;
};

// Semantic differenct: enter group, get range and begin retrieving
class TPumpJobEnterGroup : public TPumpJobPingGroup
{
public:
	TPumpJobEnterGroup (const CString& grp,
		EPumpJobDone eDone,
		BOOL         fGetAll,
		int          iHdrLimit);
	//HANDLE hEventDone);
	~TPumpJobEnterGroup() {}

	// override
	BOOL Release();

public:
	BOOL   m_fGetAll;
	int    m_iHdrLimit;

protected:
	HANDLE m_hEventDone;
};

// -------------------------------------------------------------------------
// one job packages several newsgroup names. Processed together
class TPumpJobPingMultiGroup : public TPumpJob
{
public:
	TPumpJobPingMultiGroup (CStringList * pGroupNames)
		: m_pGroupNames(pGroupNames)
	{ }

	TPumpJobPingMultiGroup& operator= (const TPumpJobPingMultiGroup& rhs);

	virtual ~TPumpJobPingMultiGroup () { delete m_pGroupNames; }

	EPumpJobType   GetType(void) { return kJobPingMultiGroup; }

	CStringList * m_pGroupNames;
};

// full article

class TPumpJobArticle : public TPumpJob
{
public:
	// retreive full article
	TPumpJobArticle(const            CString &  grp,
		LONG             groupID,
		const            CString & subject,
		CPoint &          ptPartID,
		int              artnum,
		int              bodyLines,
		DWORD            jobFlags,
		TArticleHeader * pHeader,     // take ownership
		TFetchArticle  * pFetchArt);

	virtual ~TPumpJobArticle();

	EPumpJobType      GetType(void)     { return kJobArticle; }
	const CString &   GetGroup(void)    { return m_NewsGroup; }
	LONG              GetGroupID(void)  { return m_GroupID; }
	int               GetArtInt(void)   { return m_artNumber; }

	int   GetBodyLines(void) { return m_Lines; }
	TArticleHeader *  GetArticleHeader() { return m_pArtHeader; }

	DWORD Flags ()         { return m_dwJobFlags; }
	bool  TestFlag (DWORD flg) { return (m_dwJobFlags & flg) ? true : false; }

	TFetchArticle * GetFetchObject() { return m_pFetchArticle; }
	TFetchArticle * ReleaseFetchObject();

	virtual TPumpJob *  Resubmit ();

	bool IsSameAs (TPumpJobArticle * pJobArt)
	{
		if (m_GroupID    == pJobArt->m_GroupID  &&
			m_dwJobFlags == pJobArt->m_dwJobFlags  &&
			m_artNumber  == pJobArt->m_artNumber)
			return true;

		return false;
	}

	CString FormatStatusString ();

protected:
	CString      m_NewsGroup;
	LONG         m_GroupID;
	CString      m_subject;
	CPoint       m_ptPartID;
	int          m_artNumber;
	int          m_Lines ;
	DWORD        m_dwJobFlags;

	TFetchArticle *  m_pFetchArticle;
	TArticleHeader * m_pArtHeader;
};

class TPumpJobPost : public TPumpJob
{
public:
	TPumpJobPost( LONG artInt,
		CMemFile* pmemFile,         // post article
		const CString& groupName,
		bool  fNewTopic
		);
	~TPumpJobPost();

	CMemFile * GetFile(void) { return m_pmemFile;  }
	BOOL  IsOK(void)         { return m_fOK; }
	void SetPostResult(BOOL fOK) { m_fOK = fOK; }
	EPumpJobType   GetType(void) { return m_kJobType; }

	void GetJobID(LONG& artInt) { artInt  = m_ArtInt; }

	CString    m_groupName;      // only necessary for Status Bar message
	bool IsNewTopic () { return m_fNewTopic; }

protected:

	// kJobPost
	CMemFile * m_pmemFile;
	BOOL       m_fOK;
	LONG       m_ArtInt;
	bool       m_fNewTopic;
};

////////////////////////////////////////////////////////////////////////
//  This uses a T2StringList because the normal CStringList has a free-list
//    and doesn't actually free memory on RemoveHead()
//
class TPumpJobOverview : public TPumpJob
{
public:
	// ctor 1/2
	TPumpJobOverview(const CString & grp, LONG groupID,
		int iExpireLow, TRangeSet * pRange,
		bool fExpire,
		TRangeSet* pArtPing, TPumpJob::EPumpJobDone eDone);

	// ctor 2/2
	TPumpJobOverview(const CString & grp, LONG groupID, int low, int hi);

	~TPumpJobOverview();
	EPumpJobType   GetType(void) { return m_kJobType; }
	const CString & GetGroup(void) { return m_NewsGroup; }
	T2StringList & GetList(void) { return m_StrList; };
	void          GetGroupRange (int& lo, int& hi) { lo = m_low; hi = m_high; }

	LONG          GetGroupID() { return m_GroupID; }

	TRangeSet *   GetRangeSet() { return m_pRange; }
	TRangeSet *   GetPingRangeSet() { return m_pPingRange; }
	//int           GetRangeSetTotal();
	//int           GetPingRangeTotal();
	int           GetExpireLow() { return m_iExpireLow; }
	bool          GetExpirePhase () { return m_fExpirePhase; }

	virtual void Abort () {}
	virtual TPumpJob * Resubmit ();

private:
	//int total_rangeset(TRangeSet* pRange);

protected:
	T2StringList   m_StrList;
	CString       m_NewsGroup;
	int           m_low;
	int           m_high;
	TRangeSet *   m_pRange;
	TRangeSet *   m_pPingRange;
	LONG          m_GroupID;
	int           m_iExpireLow;
	bool          m_fExpirePhase;
};

class TPumpJobHelp : public TPumpJob
{
public:
	TPumpJobHelp() {}
	~TPumpJobHelp() {}
	EPumpJobType GetType() { return kJobHelp; }
};

class TPumpJobConnect : public TPumpJob
{
public:
	TPumpJobConnect(BOOL fPrio, BOOL fDeath = FALSE)
	{
		m_fDeath = fDeath;
		m_kJobType = kJobConnect;
		m_fSuccess = FALSE;
		m_fEmergency = fPrio;
		m_fUserCancel = FALSE;

	}

	~TPumpJobConnect() {}
	EPumpJobType GetType() { return m_kJobType; }

	EErrorClass m_eErrorClass;
	CString     m_errorMessage;
	DWORD       m_dwErrorID;

	BOOL    m_fSuccess;
	BOOL    m_fEmergency;
	BOOL    m_fDeath;
	BOOL    m_fUserCancel;
};

class TPumpJobBigList : public TPumpJob
{
public:
	TPumpJobBigList (BOOL fRecent, WPARAM wParam,
		CTime& prevCheck);
	~TPumpJobBigList();

	EPumpJobType GetType() { return kJobBigList; }

public:
	BOOL     m_fRecent;              // Get All or Get Recent
	WPARAM   m_wParam;               // post processing options
	CTime    m_prevCheck;             // if Get Recent, the last check time
};

class TPumpJobPingArticles : public TPumpJob
{
public:
	TPumpJobPingArticles(int iGroupID, const CString& grpName,
		const CString& nickName, int iServerLow)
		: m_groupName(grpName), m_groupNickName(nickName)
	{
		m_iGroupID = iGroupID;
		m_pPing = new TRangeSet;
		m_kJobType = kJobPingArticles;
		m_iServerLow = iServerLow;
	}
	~TPumpJobPingArticles() { delete m_pPing; }

	virtual EPumpJobType GetType(void) { return m_kJobType; }
	int GetGroupID() { return m_iGroupID; }
	const CString & GroupName() { return m_groupName; }
	const CString & GroupNickName() { return m_groupNickName; }

	TRangeSet * m_pPing;
	int         m_iServerLow;

protected:  // data members
	int         m_iGroupID;
	CString     m_groupName;
	CString     m_groupNickName;
};
