/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vcrrun.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:25  richard_wood
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

// vcrrun.h : header file
//

// ////////////////////////////////////////////////////////////////////////
// Simple hierarchy
// TVCRAction
//    TVCRActionServer
//    TVCRActionGroup
//    TVCRActionStart
//    TVCRActionExit
//

// download action,   an abstract base class

enum VCRAct { kVCRServer, kVCRGroup, kVCRStart, kVCRExit };

class TVCRAction
{
public:

	// virt destructor
	virtual ~TVCRAction () {}

	virtual const CString & getDescription ()  =0;

	virtual bool Proceed (bool fAdvance, bool fIdle) = 0;
	virtual int DoYourJob (HWND hDlg) = 0;

	virtual int BeforeJob () = 0;
	virtual int AfterJob  () = 0;
	virtual VCRAct GetType () = 0;
};

class TVCRActionServer : public TVCRAction
{
public:
	// override the virtual
	virtual int DoYourJob (HWND hDlg);

	TVCRActionServer (const CString & svr)
		: m_strServerName(svr)
	{
		m_desc = "connect to " + svr;
	}

	virtual const CString & getDescription ()
	{ return m_desc; }

	virtual bool Proceed (bool fAdvance, bool fIdle);

	virtual int BeforeJob () { return 0; }
	virtual int AfterJob  () { return 0; }

	virtual VCRAct GetType () { return kVCRServer; }

public:
	CString m_strServerName;
	CString m_desc;
};

class TVCRActionGroup  : public TVCRAction
{
public:
	// override the virtual
	virtual int DoYourJob (HWND hDlg);

	TVCRActionGroup (const CString & grp)
		: m_strGroupName(grp) { }

	virtual const CString & getDescription ()
	{ return m_strGroupName; }

	virtual bool Proceed (bool fAdvance, bool fIdle);

	//  penultimate Group  B4     countingIdle = 0
	//                     After  countingIdle = 1
	//  immediately afterwards
	//  ultimateGroup      B4     countingIdle  = 0
	//                     After  countingIdle  = 1
	//   count 7 seconds
	//  then proceed to next server

	virtual int BeforeJob () { return 0; }
	virtual int AfterJob  () { return 1; }  // start counting idle

	virtual VCRAct GetType () { return kVCRGroup; }

public:
	CString  m_strGroupName;
};

/////////////////
class TVCRActionStart  : public TVCRAction
{
public:
	// override the virtual
	virtual int DoYourJob (HWND hDlg);

	TVCRActionStart (const COleDateTime & oleTime);

	virtual const CString & getDescription ()
	{ return m_strDesc; }

	virtual bool Proceed (bool fAdvance, bool fIdle);

	virtual int BeforeJob () { return 0; }
	virtual int AfterJob  () { return 0; }

	virtual VCRAct GetType () { return kVCRStart; }

public:
	CString        m_strDesc;
	COleDateTime   m_oleTime;
};

/////////////////
class TVCRActionExit  : public TVCRAction
{
public:
	// override the virtual
	virtual int DoYourJob (HWND hDlg);

	TVCRActionExit ();

	virtual const CString & getDescription ()
	{ return m_strDesc; }

	virtual bool Proceed (bool fAdvance, bool fIdle);

	virtual int BeforeJob () { return 0; }
	virtual int AfterJob  () { return 0; }

	virtual VCRAct GetType () { return kVCRExit; }

public:
	CString  m_strDesc;
};

typedef CTypedPtrList<CPtrList, TVCRAction*> ListVCRAction;

/////////////////

struct TServerDownloadInfo
{
	CString      m_strServerName;
	CStringList  m_lstGroups;

	TServerDownloadInfo(const CString & serverName)
		: m_strServerName(serverName) {}
};

typedef CTypedPtrList<CPtrList, TServerDownloadInfo*> ListDownloadInfo;

int  GetServersDownloadInfo (ListDownloadInfo & lst);

/////////////////////////////////////////////////////////////////////////////
// TVCRRunDlg dialog

class TVCRRunDlg : public CDialog
{
public:
	TVCRRunDlg (CWnd          * pParent,
		ListVCRAction * pListAction,
		bool          fSpecificTime,
		COleDateTime  oleTime,
		BOOL          fExitApp);

	enum { IDD = IDD_VCR_RUNNING };
	CListBox	m_lbx;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT DoInit(WPARAM, LPARAM);
	afx_msg LRESULT OnStep (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyIdle (WPARAM, LPARAM);
	afx_msg LRESULT OnBadServer (WPARAM, LPARAM);
	afx_msg LRESULT OnExitWindow (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	void delJob ();

	LRESULT my_exit ();

	ListVCRAction * m_pListAction;
	bool            m_fIdle;
	bool            m_fAdvance;

	bool          m_fSpecificTime;
	COleDateTime  m_oleTime;
	BOOL          m_fExitApp;

	UINT          m_hDelayStartTimer;
	UINT          m_hWaitIdleTimer;
	bool          m_fStartCountingIdle;
	int           m_byCountIdle;

	CStringList   m_ServerErrors;  // those we couldn't connect to.
};
