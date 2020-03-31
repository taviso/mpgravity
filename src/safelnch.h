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

#include "resource.h"
#include "superstl.h"

#define  INIT_SAFELAUNCH      (LPCTSTR) _T("GIF,JPG,JPEG,MPG,MPEG,BMP,WAV")
#define  SAFELAUNCH_VALUE     _T("SafeLaunch")

typedef set<CString> STRINGMAP;

class CLaunch
{
public:
	CLaunch(){}
	~CLaunch(){}

	CLaunch (const CLaunch & l) 
	{
		m_program = l.m_program;
		m_file    = l.m_file;
	}
	CLaunch & operator=(const CLaunch & l) 
	{
		m_program = l.m_program;
		m_file    = l.m_file;
		return *this;
	}

	CString  m_program;
	CString  m_file;
};

class CSafeLaunch
{
public:
	CSafeLaunch();
	~CSafeLaunch ();
	CString GetExtensionList ();
	void Launch(LPCTSTR program, LPCTSTR  file);
	void ReplaceAll(LPCTSTR pExtList);
	bool IsSafe(LPCTSTR pExt);
	void ApproveExtension(LPCTSTR   pExt);

private:
	void Initialise();

	void RemoveApprovedExtension(LPCTSTR   pExt);
	void Reset();
	CString GetExtension(LPCTSTR   pFile);
	bool WriteToRegistry(LPCTSTR  pExtString);
	bool ReadRegString(CString & extString);
	void StringToMap(LPCTSTR   pExtList);
	void QueueFile (LPCTSTR    program, LPCTSTR file);
	void LaunchFile (LPCTSTR  program, LPCTSTR  file);
	void ProcessQueue ();

	STRINGMAP      m_extensions;
	list<CLaunch>  m_waiting;
	bool           m_fProcessingQueue;
	HANDLE         m_launchMutex;

	bool m_bInitialised;
};

/////////////////////////////////////////////////////////////////////////////
// CFileWarn dialog
class CFileWarn : public CDialog
{
public:
	CFileWarn(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SAFELAUNCH_WARN };
	BOOL  m_fWarn;
	CString  m_extension;
	CString  m_file;
	CSafeLaunch * pSafeLaunch;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnEdit();
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CFileExtDlg dialog

class CFileExtDlg : public CDialog
{
public:
	CFileExtDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SAFELAUNCH_EDIT };
	CString  m_extensionList;
	CSafeLaunch * pSafeLaunch;

protected:
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
