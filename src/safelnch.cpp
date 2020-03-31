/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: safelnch.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:49  richard_wood
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

#include "stdafx.h"
#include "safelnch.h"
#include "tstrlist.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------

CSafeLaunch::CSafeLaunch()
{
	m_bInitialised = false;
	m_fProcessingQueue = false;
	m_launchMutex = CreateMutex (NULL, NULL, NULL);
}

// -------------------------------------------------------------------------
CSafeLaunch::~CSafeLaunch ()
{
	CloseHandle(m_launchMutex);
}

//
// Public functions
//

// -------------------------------------------------------------------------
void CSafeLaunch::Launch (LPCTSTR program, LPCTSTR file)
{
	if (!m_bInitialised)
		Initialise();
	CString     extension = file;
	int         pos;

	pos = extension.ReverseFind('.');
	if (pos >= 0)
		extension = extension.Right(extension.GetLength() - pos - 1);
	else
		extension = "";

	QueueFile(program, file);

	if (!m_fProcessingQueue)
		ProcessQueue();
}

// -------------------------------------------------------------------------
CString  CSafeLaunch::GetExtensionList ()
{
	if (!m_bInitialised)
		Initialise();
	CString  extList;
	STRINGMAP::iterator  it;

	for (it = m_extensions.begin(); it != m_extensions.end(); it++)
		extList += *it + ",";

	extList.TrimRight(',');

	return extList;
}

// -------------------------------------------------------------------------
void CSafeLaunch::ReplaceAll(LPCTSTR pExtList)
{
	if (!m_bInitialised)
		Initialise();
	m_extensions.clear();
	StringToMap (pExtList);
	WriteToRegistry (pExtList);
}

// -------------------------------------------------------------------------
bool CSafeLaunch::IsSafe (LPCTSTR pExt)
{
	if (!m_bInitialised)
		Initialise();

	CString upper = pExt;
	upper.MakeUpper();
	if (m_extensions.find(upper) != m_extensions.end())
		return true;

	return false;
}

// -------------------------------------------------------------------------
void CSafeLaunch::ApproveExtension (LPCTSTR  pExt)
{
	if (!m_bInitialised)
		Initialise();

	CString  exts = pExt;

	exts.MakeUpper();

	m_extensions.insert (exts);
	WriteToRegistry(GetExtensionList());
}


//
// Private functions
//

void CSafeLaunch::Initialise()
{
	if (m_bInitialised)
		return;

	CString  extString;
	if (!ReadRegString(extString))
	{
		extString = INIT_SAFELAUNCH;
		StringToMap(extString);
		WriteToRegistry (extString);
	}
	else
	{
		StringToMap(extString);
	}

	m_bInitialised = true;
}

// -------------------------------------------------------------------------
void CSafeLaunch::RemoveApprovedExtension(LPCTSTR  pExt)
{
	CString  upper = pExt;
	upper.MakeUpper();
	m_extensions.erase(m_extensions.find (upper));
	WriteToRegistry(GetExtensionList());
}

// -------------------------------------------------------------------------
void CSafeLaunch::Reset()
{
	m_extensions.clear();
	StringToMap(INIT_SAFELAUNCH);
	WriteToRegistry(INIT_SAFELAUNCH);
}

// -------------------------------------------------------------------------
bool CSafeLaunch::WriteToRegistry(LPCTSTR pExtString)
{
	HKEY  hKey;
	LONG  rc;

	rc = RegOpenKey(HKEY_CURRENT_USER, GetGravityRegKey()+"system", &hKey);

	if (ERROR_SUCCESS != rc)
	{
		rc = RegCreateKey(HKEY_CURRENT_USER, GetGravityRegKey()+"system", &hKey);

		if (ERROR_SUCCESS != rc)
			return false;
	}

	rc = RegSetValueEx(hKey, SAFELAUNCH_VALUE, 0, REG_SZ,
		LPBYTE(pExtString), DWORD(_tcslen (pExtString) + 1));

	RegCloseKey(hKey);

	if (rc != ERROR_SUCCESS)
		return false;

	return true;
}

// -------------------------------------------------------------------------
bool CSafeLaunch::ReadRegString(CString & extString)
{
	HKEY  hKey;
	LONG  rc;
	DWORD dwType = REG_SZ;
	TCHAR extensions[1024];
	DWORD extSize = sizeof (extensions);

	rc = RegOpenKey(HKEY_CURRENT_USER, GetGravityRegKey()+"system", &hKey);

	if (ERROR_SUCCESS != rc)
		return false;

	rc = RegQueryValueEx(hKey, SAFELAUNCH_VALUE, NULL,
		&dwType, LPBYTE(extensions), &extSize);

	RegCloseKey(hKey);

	if (rc != ERROR_SUCCESS)
		return false;

	extString = extensions;

	return true;
}

// -------------------------------------------------------------------------
void CSafeLaunch::StringToMap(LPCTSTR pExtList)
{
	TStringList strList;
	POSITION    pos;
	CString     upper;

	strList.FillByCommaList(pExtList);

	pos = strList.GetHeadPosition();

	while (pos != NULL)
	{
		upper = strList.GetNext (pos);
		upper.MakeUpper();
		m_extensions.insert(upper);
	}
}

void CSafeLaunch::QueueFile(LPCTSTR program, LPCTSTR file)
{
	CLaunch  launch;

	launch.m_program = program;
	launch.m_file    = file;

	m_waiting.push_back (launch);
}

void CSafeLaunch::LaunchFile(LPCTSTR program, LPCTSTR file)
{
	LPCTSTR pchCommandLine;
	LPCTSTR pchFileToOpen;
	CString strDirectory;

	if (NULL == program || (*program == 0))
	{
		pchCommandLine = NULL;
		pchFileToOpen  = file;
	}
	else
	{
		pchCommandLine = file;
		pchFileToOpen = program;

		// get directory name
		strDirectory = file;
		int i = 0;
		for (i = strDirectory.GetLength () - 1;
			i && strDirectory [i] != '\\'; i--)
			;
		if (strDirectory [i] == '\\')
			strDirectory = strDirectory.Left (i);
	}

	// not checking return code of ShellExecute() since it was returning an
	// error code in some cases even though the viewer was started okay
	// The SW_SHOW is for LVIEW.EXE which can be launched, but is hidden.
	ShellExecute(GetDesktopWindow (),     // parent window
		_T("open"),                  // command to send
		pchFileToOpen,           // file to open
		pchCommandLine,          // command line if file is exe
		strDirectory,            // directory name
		SW_SHOW);                // nCmdShow
}

CString CSafeLaunch::GetExtension(LPCTSTR pFile)
{
	CString     file;
	CString     ext;
	int         pos;

	file = pFile;

	pos = file.ReverseFind('.');
	if (pos >= 0)
		ext = file.Right(file.GetLength() - pos - 1);

	return ext;
}

typedef struct launchRec
{
	TCHAR    rchProgram[1024];
	TCHAR    rchFile   [1024];
	HANDLE   launchMutex;
} LAUNCHREC;

DWORD WINAPI BackLaunch(LPVOID pLaunch)
{
	LAUNCHREC * p = (LAUNCHREC *) pLaunch;

	LPCTSTR pchCommandLine;
	LPCTSTR pchFileToOpen;
	CString strDirectory;

	if (*p->rchProgram == 0)
	{
		pchCommandLine = NULL;
		pchFileToOpen  = p->rchFile;
	}
	else
	{
		pchCommandLine = p->rchFile;
		pchFileToOpen = p->rchProgram;

		// get directory name
		strDirectory = p->rchFile;
		int i = 0;
		for (i = strDirectory.GetLength () - 1;
			i && strDirectory [i] != '\\'; i--)
			;
		if (strDirectory [i] == '\\')
			strDirectory = strDirectory.Left (i);

	}

	// not checking return code of ShellExecute() since it was returning an
	// error code in some cases even though the viewer was started okay
	// The SW_SHOW is for LVIEW.EXE which can be launched, but is hidden.
	WaitForSingleObject(p->launchMutex, INFINITE);
	ShellExecute(GetDesktopWindow (),     // parent window
		_T("open"),                  // command to send
		pchFileToOpen,           // file to open
		pchCommandLine,          // command line if file is exe
		strDirectory,            // directory name
		SW_SHOW);                // nCmdShow
	ReleaseMutex(p->launchMutex);
	free(p);

	return 0;   
}

void CSafeLaunch::ProcessQueue()
{
	CLaunch                    launch;
	CString                    ext;
	list<CLaunch>::iterator    it;
	CRITICAL_SECTION           crit;
	DWORD                      threadID;
	CWnd *                     pParent;
	DWORD                      dwProcId;

	pParent = CWnd::GetForegroundWindow();
	if (pParent)
	{
		::GetWindowThreadProcessId(pParent->m_hWnd, &dwProcId);
		if (GetCurrentProcessId() != dwProcId)
			pParent = NULL;
	}

	CFileWarn warnDlg(pParent);

	InitializeCriticalSection(&crit);

	EnterCriticalSection(&crit);
	m_fProcessingQueue = true;

	while (!m_waiting.empty())
	{
		launch = m_waiting.front();
		ext = GetExtension(launch.m_file);
		ext.MakeUpper();
		if (IsSafe(ext))
		{
			LAUNCHREC *p = (LAUNCHREC *) malloc (sizeof (LAUNCHREC));
			_tcscpy(p->rchProgram, launch.m_program);
			_tcscpy(p->rchFile, launch.m_file);
			p->launchMutex = m_launchMutex;
			CreateThread(NULL, 2048, BackLaunch, p, 0, &threadID);
		}
		else
		{
			int pos;
			pos = launch.m_file.ReverseFind ('\\');
			if (pos >= 0)
				warnDlg.m_file = launch.m_file.Right(launch.m_file.GetLength() - pos - 1);
			else
				warnDlg.m_file = launch.m_file;

			warnDlg.m_extension  = ext;
			warnDlg.pSafeLaunch  = this;
			warnDlg.m_fWarn      = FALSE;

			if (IDOK == warnDlg.DoModal())
				LaunchFile(launch.m_program, launch.m_file);
		}
		m_waiting.pop_front();
	}

	m_fProcessingQueue = false;
	LeaveCriticalSection(&crit);
}

/////////////////////////////////////////////////////////////////////////////
// CFileWarn dialog

CFileWarn::CFileWarn(CWnd* pParent /*=NULL*/)
	: CDialog(CFileWarn::IDD, pParent)
{
	m_fWarn = FALSE;
	m_extension = _T("");
	m_file = _T("");
}

void CFileWarn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_DONT_WARN, m_fWarn);
	DDX_Text(pDX, IDC_EXTENSION, m_extension);
	DDX_Text(pDX, IDC_FILE, m_file);
}

BEGIN_MESSAGE_MAP(CFileWarn, CDialog)
		ON_BN_CLICKED(IDC_EDIT, OnEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileWarn message handlers

BOOL CFileWarn::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);
	ASSERT(pSafeLaunch);

	return TRUE;
}

void CFileWarn::OnEdit() 
{
	CFileExtDlg extEdit;

	extEdit.pSafeLaunch = pSafeLaunch;

	if (extEdit.DoModal() == IDOK)
	{
		m_fWarn = pSafeLaunch->IsSafe(m_extension);
		UpdateData(FALSE);
	}
}

void CFileWarn::OnOK() 
{
	UpdateData (TRUE);

	if (m_fWarn)
		pSafeLaunch->ApproveExtension(m_extension);
	CDialog::OnOK();
}

void CFileWarn::OnCancel() 
{
	CDialog::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
// CFileExtDlg dialog

CFileExtDlg::CFileExtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileExtDlg::IDD, pParent)
{
	m_extensionList = _T("");
}

void CFileExtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EXTENSION_LIST, m_extensionList);
}

BEGIN_MESSAGE_MAP(CFileExtDlg, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileExtDlg message handlers

BOOL CFileExtDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ASSERT(pSafeLaunch);
	m_extensionList = pSafeLaunch->GetExtensionList();

	UpdateData(FALSE);

	return TRUE;
}

void CFileExtDlg::OnOK() 
{
	UpdateData(TRUE);
	pSafeLaunch->ReplaceAll(m_extensionList);

	CDialog::OnOK();
}
