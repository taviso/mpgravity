/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsrchdlg.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:19  richard_wood
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

// tsrchdlg.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TSearchDialog dialog
#include "resource.h"
#include "pobject.h"
#include "richedit.h"
#include "triched.h"
#include "custview.h"

#include "srchfrm.h"             // TSearchFrame

class TNewsServer;
class TArticleHeader;            // declare the class

typedef  struct SearchArticleResult
{
	SearchArticleResult::SearchArticleResult()
	{m_header=0;m_groupID=0;}
	CString  m_from;
	CString  m_subject;
	CString  m_date;
	CString  m_newsgroup;
	int      m_groupID;
	TArticleHeader *m_header;
} SEARCH_ARTICLE_RESULT;

class TResultItemData
{
public:
	TResultItemData(){m_header=0;m_groupID=0;}
	virtual ~TResultItemData();

	LONG  m_groupID;
	TArticleHeader *m_header;
};

class TGroupNameID
{
public:
	TGroupNameID(){m_groupID=0;}
	~TGroupNameID(){}
	CString  m_name;
	LONG     m_groupID;
};

class TProgressMessage
{
public:
	TProgressMessage () {m_curr=0;m_total=0;}
	~TProgressMessage() {}
	CString  m_group;
	DWORD m_curr;
	DWORD m_total;
};

UINT BackgroundSearchThread (LPVOID pParam);
BOOL GetGroupList (TNewsServer * pServer,
				   CArray<TGroupNameID, TGroupNameID &> & groupList);

typedef struct SearchThreadParams
{
	SearchThreadParams::SearchThreadParams()
	{
		m_fSearchAll=FALSE;
		m_fSearchHeaders=FALSE;
		m_fSearchBodies=FALSE;
		m_fSearchFrom=FALSE;
		m_fSearchSubject=FALSE;
		m_fLocalOnly=FALSE;
		m_fRE=FALSE;
		m_groupID=0;
		m_stopEvent=0;
		m_hWnd=0;
		m_pNewsServer=NULL;
	}
	BOOL           m_fSearchAll;           // search all newsgroups?
	BOOL           m_fSearchHeaders;       // should we search article headers?
	BOOL           m_fSearchBodies;        // should we search article bodies?
	BOOL           m_fSearchFrom;          // search the from
	BOOL           m_fSearchSubject;       // search the
	BOOL           m_fLocalOnly;           // search only local headers/bodies
	BOOL           m_fRE;                  // use regular-expression search
	CString        m_newsgroup;            // current newsgroup being viewed
	LONG           m_groupID;              // group ID of current newsgroup
	HANDLE         m_stopEvent;            // thread dies when this is signaled
	HWND           m_hWnd;                 // window handle for Dialog
	CString        m_searchFor;            // string we're searching for
	CString        m_timeFormat;           // custom time format
	TNewsServer  * m_pNewsServer;          // ptr to our newsserver
} SEARCH_THREAD_PARMS;

class TSearchDialog : public CDialog
{
protected:
	static CString  s_strMRUText;

public:
	enum EThreadMessage {kSearchFinished,
		kSearchAddResult,
		kThreadDead,
		kSearchAborted,
		kSearchProgress,
		kPreserveStatus};
	TSearchDialog(CWnd* pParent = NULL);   // standard constructor

	static LPCTSTR s_winClassName;         // holds registered class name

	enum { IDD = IDD_SEARCH };

	CTabCtrl          m_myTabs;

	CString           m_searchFor;
	BOOL              m_fSearchHeaders;
	BOOL              m_fSearchBodies;
	int               m_fSearchThisNewsgroup;
	BOOL              m_fLocalOnly;
	BOOL              m_fRE;

	CString           m_currGroup;
	LONG              m_currGroupID;

	HANDLE            m_stopEvent;
	HANDLE            m_workerThread;

	CStatusBarCtrl    m_statusBar;
	CProgressCtrl     m_progress;
	HWND              m_hwndRTF; // rich edit control

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnSearchSearchNow();
	afx_msg void OnSearchStop();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnDblclkSearchResults(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClkSearchResults(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSearchJump();
	afx_msg void OnDeleteitemSearchResults(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSearchOptionsGoto();
	afx_msg LRESULT OnThreadTalk (WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	void EnableDisable(BOOL fRunning);
	void GetDialogParms(SEARCH_THREAD_PARMS *pParms);
	void AddResult(SEARCH_ARTICLE_RESULT *pResult);
	TRichEd *GetRichEdit();
	BOOL LoadArticleForDisplay(LONG GroupID,
		TArticleHeader* pHdr,
		TArticleText*&  pArtText);
	int  GetCurSel(void);
	void SetProgress(TProgressMessage *pProgress);
	void OnCancel();
	void DisplaySelectedArticle();

	void on_size(UINT nType, int cx, int cy);

protected:
	HANDLE   m_hSearchIcon;
	DWORD    m_hOldIcon;
	BOOL     m_bPreserveStatus;

	TSearchFrame * m_pResultFrame;

	TNewsServer * m_pNewsServer;
};
