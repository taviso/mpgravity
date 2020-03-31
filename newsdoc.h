/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsdoc.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2009/10/04 21:04:10  richard_wood
/*  Changes for 2.9.13
/*
/*  Revision 1.3  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.2  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:33  richard_wood
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

// Newsdoc.h : interface of the CNewsDoc class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "declare.h"
#include "grplist.h"
#include "statchg.h"
#include "tobserv.h"
#include <list>
#include <string>

// forward declaration
//class MySpell;
class Hunspell;
class TMyWords;

class TObserverWatchesPump : public TObserver
{
public:
	virtual void UpdateObserver ();
};

class TGroupList;

class CNewsSrvrItem;
class CNewsDoc : public CDocument
{
public:
	static CNewsDoc* m_pDoc;

protected: // create from serialization only
	CNewsDoc();
	DECLARE_DYNCREATE(CNewsDoc)

	// Attributes
public:

	// Operations
public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewsDoc)

public:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

public:
	int SaveToFile(TPersist822Header* pBaseHdr, TPersist822Text* pBaseText,
		BOOL fFullHdr, CFile& file);

	void AutoDelete(BOOL fOn);

	void AddArticleStatusChange ( TStatusChg* pStatusChange );
	BOOL DequeueArticleStatusChange (void);
	void EmptyArticleStatusChange (void);
	void UtilRetrieveRecentNewsgroups (bool   fRequestGetEntireList,
		BOOL   fShowSubscribeDialog,
		BOOL   fPopupEmptyList = FALSE);

	void CancelMessage (   const CString& newsGroup, const CString& oldmsgid, const CString& oldsubject);

	void CallSubscribeDlg (TGroupList * pFreshGroups);

	int  GetStatusChangeCount () { return m_iStatChangeCount; }

	static void DocGetHeadersAllGroups (bool fForceRetrieveCycle,  bool fUserAction);

	void SubscribeGroupUpdate ();

	int  HandleNewsUrl (CString*& rpstrNewsURL);

	// panic key aka Boss-Key
	bool PanicMode () { return m_fPanicMode; }
	void PanicMode (bool fPanic) { m_fPanicMode = fPanic; }

	void ConfigurePanicKey (BOOL fEnabled);

	TObserver* GetPumpObserver () { return &m_sPumpObserver; }

	// util function for VCR dlg
	void getGroup (const CString & groupName);

	void DisplayConnectError (HWND hWnd, bool fRelease, LPARAM lParam);

	int CheckSpelling_Word (const CString & testWord, std::list<std::string> & suggestList);

	TMyWords* GetMyWords() { return m_pMyWords; }

	// Implementation
public:

	virtual ~CNewsDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CNewsDoc)
	afx_msg void OnNewsgroupSubscribe();
	afx_msg void OnUpdateNewsgroupSubscribe(CCmdUI* pCmdUI);
	afx_msg void OnNewsgroupRecent();
	afx_msg void OnUpdateNewsgroupRecent(CCmdUI* pCmdUI);
	afx_msg void OnNewsgroupGetall();
	afx_msg void OnUpdateNewsgroupGetall(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int ForceSubscribe (const CString & newsGroup, int & iGroupID);

private:
	CRITICAL_SECTION                m_csStatChg;
	TStatusChgList                  m_lstStatChg;   // list of Status Changes
	int                             m_iStatChangeCount;

	bool m_fPanicHookRunning ;     // true if Panic Key Hook is activated
	bool m_fPanicMode;             // true if we are in Panic mode

	// observes Pump, acts when Connected
	TObserverWatchesPump m_sPumpObserver;

//	MySpell *m_pMS;
	Hunspell *m_pMS;
	friend class CNewsApp;
	TMyWords *m_pMyWords;
};
