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

// CUpdateCheck - Update checking system

// GPSUtil - Utility structure
#pragma once

struct GPSUtil 
{
	char *     key;
	CString  * var;
};

class CUpdateCheckResults
{
public:
	CUpdateCheckResults() : m_fCheckSucceeded(FALSE) {}

	HWND     m_hFrameWnd;
	BOOL     m_fCheckSucceeded;
	int      m_iAvailableBuild;
	CString  m_name;
	CString  m_date;
	CString  m_info;
};

// --------------------------------------------------------------
// CUpdateCheck - Class for managing Gravity update checks.
// --------------------------------------------------------------
class CUpdateCheck
{
public:
	static BOOL CheckNeeded ();

	static void BackgroundUpdateCheck (CUpdateCheckResults *pResults);

	static BOOL  GetUpdateInfo(LPCTSTR   updateURL,
		LPCTSTR   product,
		int     & build,
		CString & name,
		CString & date,
		CString & info
		);

private:
	static BOOL GPSAssign (LPCTSTR   app,
		LPCTSTR   key,
		LPCTSTR   def,
		LPTSTR    buff,
		int       size,
		LPCTSTR   path,
		CString * var);
};

/////////////////////////////////////////////////////////////////////////////
// CCheckingForUpdateDlg dialog
class CCheckingForUpdateDlg : public CDialog
{
public:
	CCheckingForUpdateDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_CHECKING_FOR_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CUpdateDlg dialog
class CUpdateDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CUpdateDlg)

public:
	CUpdateDlg();
	~CUpdateDlg();

	enum { IDD = IDD_OPTIONS_UPDATES };
	BOOL  m_fCheckForUpdates;
	CString  m_daysBetweenChecks;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckNow();
	afx_msg void OnCheckForUpdates();
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CUpdateNotify dialog
class CUpdateNotify : public CDialog
{
public:
	CUpdateNotify(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_UPDATE_AVAILABLE };

	BOOL     m_fModeless;
	CString  m_runningVersion;
	CString  m_newVersion;
	CString  m_releaseDate;
	CString  m_desc;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnGetIt();
	DECLARE_MESSAGE_MAP()
};
