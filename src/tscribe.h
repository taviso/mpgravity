/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tscribe.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:18  richard_wood
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

// tscribe.h : header file
//

#pragma once

#include "declare.h"
#include "pobject.h"
#include "enumjob.h"
#include "servcp.h"     // TServerCountedPtr
#include "humble.h"

class TScribePrep;
class TPrepHeader;
class TPrepBody;
class TPrepFullHeader;
class TPrepBatchHeader;
class TNewsServer;

/////////////////////////////////////////////////////////////////////////////
// the newspump passes results to the scribe.
//   - xhdr info
//   - fullheader
//   - body
//
//   TScribePrep
//       TPrepHeader
//       TPrepFullHeader
//       TPrepBody
//
class TScribePrep : public CObject
{
public:
	enum EPrepDone { kDoneDisplay, kDoneNothing };

	enum EPrepType {
		kPrepHdr,
		kPrepBody,
		kPrepBatchHdr
	} ;

public:
	TScribePrep(const CString& newsGroup, int artNum)
		: m_newsGroup(newsGroup), m_articleNumber(artNum)
	{
	}
	virtual ~TScribePrep() {}
	const CString & GetGroup(void) { return m_newsGroup; }
	int  GetArticleInt(void) { return m_articleNumber; }
	virtual EPrepType GetType(void) = 0;       // abstract

	virtual TPrepHeader*       CastToPrepHeader(void) = 0;
	virtual TPrepBody*         CastToPrepBody(void) = 0;
	virtual TPrepFullHeader*   CastToPrepFullHeader(void) = 0;
	virtual TPrepBatchHeader*  CastToPrepBatchHeader(void) = 0;
	virtual int                GetByteCount() = 0;

protected:
	CString m_newsGroup;
	int     m_articleNumber;
};

class TPrepHeader : public TScribePrep
{
public:
	TPrepHeader(LPCTSTR newsGroup, TArticleHeader* pArtHeader,  int artNum);
	~TPrepHeader(void);
	TArticleHeader * GetpHeader(void) { return m_pArtHeader; }
	TArticleHeader * RemovepHeader(void)
	{
		TArticleHeader * pHdr = m_pArtHeader;
		m_pArtHeader = 0;
		return pHdr;
	}

	EPrepType GetType () { return kPrepHdr; }

	TPrepHeader* CastToPrepHeader(void) { return this; }

	TPrepBody*          CastToPrepBody(void)        { return 0; }
	TPrepFullHeader*    CastToPrepFullHeader(void)  { return 0; }
	TPrepBatchHeader*   CastToPrepBatchHeader(void) { return 0; }
	virtual int         GetByteCount();

protected:
	TArticleHeader * m_pArtHeader;
};

// Save a body - it actually starts a batch save
class TPrepBody : public TScribePrep
{
public:
	TPrepBody(const CString& newsGroup, TArticleText* pText, int artNum,
		EPumpJobFlags eFlags);
	~TPrepBody();
	TArticleText * GetBody(void) { return m_pText; }
	EPrepType GetType () { return kPrepBody; }

	TPrepHeader* CastToPrepHeader(void) { return 0; }
	TPrepBody*   CastToPrepBody(void) { return this; }
	TPrepFullHeader*   CastToPrepFullHeader(void) { return 0; }
	TPrepBatchHeader*   CastToPrepBatchHeader(void) { return 0; }

	virtual int         GetByteCount();

	EPumpJobFlags m_eFlags;

protected:
	TArticleText * m_pText;
};

//  5-19-97  amc  Removed 'CDWordArray* m_pMarkers', it was unused
class TPrepBatchHeader : public TScribePrep
{
public:
	TPrepBatchHeader*   CastToPrepBatchHeader(void) { return this; }

	TPrepHeader*      CastToPrepHeader(void)        { return 0; }
	TPrepBody*        CastToPrepBody(void)          { return 0; }
	TPrepFullHeader*  CastToPrepFullHeader(void)    { return 0; }

	EPrepType GetType () { return kPrepBatchHdr; }

	TPrepBatchHeader(LPCTSTR            groupName,
		LONG               groupID,
		TArtHdrArray *     pHdrArray,
		TScribePrep::EPrepDone eDone);

	~TPrepBatchHeader(void);
	virtual int         GetByteCount();
	TScribePrep::EPrepDone GetDisposition() { return m_eDone; }

public:
	LONG               m_groupID;
	TArtHdrArray *     m_pHdrArray;
	TScribePrep::EPrepDone m_eDone;
};

/////////////////////////////////////////////////////////////////////////////
// TNewsScribe thread

class TNewsScribe : public CHumbleThread
{
	DECLARE_DYNCREATE(TNewsScribe)
public:
	TNewsScribe(void);
	TNewsScribe(HANDLE evt, TNewsServer* pCurrentServer);
	virtual ~TNewsScribe();

	BOOL IsRunning(void) { return m_fScribeRunning; }

	//  Cancel jobs pertaining to a newsgroup
	int CancelNewsgroup (LPCTSTR groupName);

	// Attributes
public:

	// Operations
public:
	void Delete ();      // thread cleanup
	DWORD ResumeScribe(void);
	void AddPrep(TScribePrep* pPrep);
	int JobCount(void);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TNewsScribe)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	// Implementation
protected:
	void SingleStep (void);

	TScribePrep* GetPrep(void);
	void PutbackPrep(TScribePrep* pPrep);
	void QueueBodyCount(int& n);
	void MultiGetPrep(int n, CTypedPtrArray<CObArray, TScribePrep*>& set);

	void SaveFullHeader(TPrepFullHeader* pHdr);
	void SaveHeader(TPrepHeader* pHdr);
	void SaveBody(TPrepBody* pBody);
	void SaveBatchHeader ( TPrepBatchHeader* pBatchHdr );
	void PostCheckQ (void);
	int  ScribeEvalRules (TNewsGroup * pNG, int iArticleInt,
		TPrepBody* pPrepBody, TArticleHeader*& rpHdr);

	void discard_local_bodies(CTypedPtrArray<CObArray, TScribePrep*>& set,
		TNewsGroup* pNG, const CString & groupName);
	void save_body_util(TPrepBody* pBody, TNewsGroup* pNG, int artInt,
		bool & diskLow);

	void convert_c_excp(LPCTSTR descOuter, CException * pCExcp);

public: // data
	HANDLE m_KillRequest;
	HANDLE m_Killed;

private: // data
	CTypedPtrArray<CObArray, TScribePrep*> m_Bucket;

	HANDLE m_hEventArticleReady;    // synchronize with pump
	HANDLE m_hMutexScribeBucket;    // synchronize with pump

	BOOL   m_fScribeRunning;
	int    m_instanceRet;
	HANDLE m_evtScribe;             // pump can be 50 messages ahead, no more

	CTime  m_lastFlushTime;         // controls how often we flush
	int    m_iFlushCount;

	TServerCountedPtr m_cpNewsServer;    // smart pointer
};
