/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fileutil.cpp,v $
/*  Revision 1.2  2010/07/24 21:57:03  richard_wood
/*  Bug fixes for Win7 executing file ops out of order.
/*  V3.0.1 RC2
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.10  2009/08/29 00:56:29  richard_wood
/*  Fixed import previous version on startup bug (TrashDirectory failed if directory didn't exist)
/*
/*  Revision 1.9  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.8  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.7  2009/07/26 18:35:43  richard_wood
/*  Changed back from ShellExecute to system as ShellExecute doesn't work from program files?
/*
/*  Revision 1.6  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.5  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.4  2009/06/14 20:47:36  richard_wood
/*  Vista/Win7 compatible.
/*  Multi user.
/*  Multi version side-by-side.
/*  Import/Export DB & settings.
/*  Updated credits file for the UFT CStdioFileEx and compression code.
/*
/*  Revision 1.3  2009/06/14 13:17:22  richard_wood
/*  Added side by side installation of Gravity.
/*  Adding (WORK IN PORGRESS!!!) DB export/import.
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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
#include <winsock.h>
#include <direct.h>
#include "resource.h"
#include "fileutil.h"
#include "dlgappnd.h"
#include "tglobopt.h"            // TGlobalOptions
#include "superstl.h"            // STL headers
#include "autoprio.h"
#include "superstl.h"            // ifstream, ...
#include "utilerr.h"             // TNNTPErrorDlg

#include "Cabinet\Compress.hpp"
#include "Cabinet\Extract.hpp"

#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// prototypes
static void LoadFilterString (int stringID, LPTSTR str, int bufsize);

static TFileUtil::EDisposition getfile_ask_user (
	CWnd *                pParentWnd,
	TYP_GETOUTPUTFILE *   psMode,
	const CString&        defaultDir,
	UINT                  defaultTitleStrID,
	int                   filterStringID,
	CString &             fileName);

TFileUtil::EDisposition getfile_ask_altname (
	CWnd *                pParentWnd,
	TYP_GETOUTPUTFILE *   psMode,
	const CString&        defaultDir,
	UINT                  defaultTitleStrID,
	int                   filterStringID,
	CString &             fileName);

/////////////////////////////////////////////////////////////////////////////
BOOL TFileUtil::FileExist (LPCTSTR fileName)
{
	return TFileUtil::FileAccess (fileName, TFileUtil::kEXIST);
}

BOOL TFileUtil::FileAccess(LPCTSTR fileName, TFileUtil::EAccess eAccess)
{
	int mode;
	switch (eAccess)
	{
	case TFileUtil::kEXIST:
		mode = 0;
		break;
	case TFileUtil::kWRITE:
		mode = 2;
		break;
	case TFileUtil::kREAD:
		mode = 4;
		break;
	case TFileUtil::kREADWRITE:
		mode = 6;
		break;
	default:
		ASSERT(0);
		break;
	}
	return (0 == _access( fileName, mode ));
}

// structure  member function
void TYP_GETOUTPUTFILE::SetAction ()
{
	if (flags & GETFILE_FLG_TEXT)
	{
		// Article -> File Save As...    is an interactive chore
		action = GETFILE_ACT_ASKUSER;
	}
	else
		switch (gpGlobalOptions->FilenameConflict ())
	{
		case 0:   action =  GETFILE_ACT_ASKUSER;     break;
		case 1:   action =  GETFILE_ACT_SKIP;        break;
		case 2:   action =  GETFILE_ACT_OVERWRITE;   break;
		case 3:   action =  GETFILE_ACT_ALTNAME;     break;
	}
}

////////////////////////////////////////////////////////////////////////////
//  GetOutputFilename -- Wrapper for  GetSaveFileName
//
BOOL TFileUtil::GetOutputFilename(
								  CString* pOutName,
								  CWnd* pParentWnd,
								  UINT stringIDTitle,
								  const CString* pInitDir,
								  DWORD moreFlags,
								  int   filterStringID,
								  LPCTSTR pchOriginalFilename /* = NULL */)
{
	OPENFILENAME ofn;

	TCHAR        szFile[MAX_PATH] = "\0";

	LPTSTR  pFilter = 0;

	//if (!pOutName->IsEmpty())
	// strncpy (szFile, *pOutName, sizeof(szFile) - 1);
	memset ( &ofn, 0, sizeof(ofn) );

	CString title;
	CString filter;
	if (stringIDTitle)
		title.LoadString(stringIDTitle);

	if (filterStringID)
	{
		pFilter = new TCHAR[256];
		LoadFilterString (filterStringID, pFilter, 256);
	}

	// Try to find the original extension
	ofn.lpstrDefExt = NULL;
	if (pchOriginalFilename)
		for (int i = strlen (pchOriginalFilename); i > 0; i--)
			if (pchOriginalFilename [i] == '.') {
				ofn.lpstrDefExt = &pchOriginalFilename [i+1];
				break;
			}

			// Set line edit box to original filename, pchOriginalFilename
			if (pchOriginalFilename)
				lstrcpy (szFile, pchOriginalFilename);

			ofn.lStructSize      = sizeof(ofn);
			ofn.hwndOwner        = (pParentWnd) ? pParentWnd->GetSafeHwnd() : AfxGetMainWnd()->GetSafeHwnd();
			ofn.hInstance        = AfxGetInstanceHandle();
			ofn.lpstrFilter      = pFilter;
			ofn.lpstrCustomFilter= NULL;
			ofn.nMaxCustFilter   = NULL;
			ofn.nFilterIndex     = NULL;
			ofn.lpstrFile        = szFile;
			ofn.nMaxFile         = sizeof(szFile);
			ofn.lpstrFileTitle   = NULL;
			ofn.nMaxFileTitle    = 0;
			ofn.lpstrInitialDir  = NULL;
			if (pInitDir && !pInitDir->IsEmpty())
				ofn.lpstrInitialDir = LPCTSTR(*pInitDir);
			ofn.lpstrTitle       = stringIDTitle ? LPCTSTR(title) : NULL;
			ofn.nFileOffset      = 0;
			ofn.nFileExtension   = 0;
			ofn.lCustData        = NULL;
			ofn.lpfnHook         = NULL;
			ofn.lpTemplateName   = NULL;
			ofn.Flags            = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST |
				OFN_NOREADONLYRETURN | moreFlags;

			BOOL ret = GetSaveFileName ( &ofn );

			if (ret)
				(*pOutName) = ofn.lpstrFile;

			if (pFilter)
				delete [] pFilter;

			return ret;
}

////////////////////////////////////////////////////////////////////////////
// meant to open an output CFile.
// 0 :  success
// 1 :  failure
// 2 :  skip
int TFileUtil::CreateOutputFile (
								 CWnd*                pParentWnd,
								 CString&             fileName,
								 const CString&       defaultDir,
								 UINT                 defaultTitleStrID,
								 CFile&               fl,
								 TYP_GETOUTPUTFILE *  psMode,
								 bool                 fFromCache_NoSKIP,
								 int                  filterStringID /* = 0 */)

{
	auto_prio boost(auto_prio::kNormal);
	CFileException fileExcp;
check_exist_again:
	TFileUtil::EDisposition eDisp = TFileUtil::CheckFileExist (fileName,
		pParentWnd,
		defaultDir,
		defaultTitleStrID,
		filterStringID,
		psMode,
		fFromCache_NoSKIP);

	if (kFileCancel == eDisp)
		return 2;

	// Try to open the file
	BOOL openRet;
	if (kFileOverwrite == eDisp)
		openRet = fl.Open(fileName,
		CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive,
		&fileExcp);
	else
		openRet = fl.Open(fileName, CFile::modeWrite | CFile::shareExclusive, &fileExcp);

	if (openRet)
	{
		if (kFileAppend == eDisp)
			fl.SeekToEnd();

		return 0;   // success
	}
	else
	{
		// we are decoding something (overnight?) can't stop
		if (psMode->action != GETFILE_ACT_ASKUSER)
			return 1;

		const int bufSz = 256;
		auto_ptr<TCHAR> sBuf( new TCHAR[bufSz] );

		if (fileExcp.GetErrorMessage(sBuf.get(), bufSz-1))
		{
			// display Error
			NewsMessageBox (pParentWnd, sBuf.get(), MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			CString msg;
			msg.LoadString (IDS_ERR_FILEOPEN);
			msg += " - ";
			msg += fileName;

			// display Error
			NewsMessageBox (pParentWnd, msg, MB_OK | MB_ICONEXCLAMATION);
		}

		// ask for another
		BOOL fRet = TFileUtil::GetOutputFilename(
			&fileName,
			pParentWnd,
			defaultTitleStrID, &defaultDir, filterStringID);
		if (!fRet)
			return 2;
		else
			goto check_exist_again;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Returns  kFileCancel, kFileOverwrite, kFileAppend
//
// 4-12-96 amc  Now returns an explicit ENUM
//              { kFileCancel, kFileOverwrite, kFileAppend }
//
TFileUtil::EDisposition
TFileUtil::CheckFileExist(CString&              fileName,
						  CWnd*                 pParentWnd,
						  const CString&        defaultDir,
						  UINT                  defaultTitleStrID,
						  int                   filterStringID,
						  TYP_GETOUTPUTFILE *   psMode,
						  bool                  fFromCache_NoSKIP)

{
	psMode->SetAction ();

	auto_prio boost(auto_prio::kNormal);
	if (!pParentWnd)
		pParentWnd = AfxGetMainWnd();

	if (!TFileUtil::FileExist (fileName))
		return kFileOverwrite;

	// this filename already exists, gotta take some other action

	switch (psMode->action)
	{
	case GETFILE_ACT_ASKUSER:
		{
			return getfile_ask_user (pParentWnd, psMode,
				defaultDir,
				defaultTitleStrID,
				filterStringID,
				fileName);
		}

	case GETFILE_ACT_OVERWRITE:
		{
			return kFileOverwrite;
		}

	case GETFILE_ACT_ALTNAME:
		{
			// get base & extension
			CString strBase, strExtension;
			int iLen = fileName.GetLength ();
			int i;
			for (i = iLen - 1; i >= 0; i--) {
				// if found '.', break
				if (fileName [i] == '.')
					break;
				// if we're into the directory name, break
				if (fileName [i] == '\\') {
					i = -1;
					break;
				}
			}
			if (i == -1)
				// no '.' found
				strBase = fileName;
			else {
				strBase = fileName.Left (i);
				strExtension = fileName.Right (iLen - i - 1);
			}

			int iIncrement = 2;
			do
			{
				// prevent infinite loop
				if (iIncrement > 0x7FFF)
					return kFileCancel;

				// append a number to the name
				if (strExtension.IsEmpty ())
					fileName.Format ("%s_%d", LPCTSTR(strBase), iIncrement);
				else
					fileName.Format ("%s_%d.%s", LPCTSTR(strBase), iIncrement, LPCSTR(strExtension));
				iIncrement ++;
			} while (TFileUtil::FileExist (fileName));
			return kFileOverwrite;
		}

	case GETFILE_ACT_SKIP:
		// multi-part article's parts were read from the cache, so it's probably
		// ok to erase the faulty output from the previous decode attempt
		if (fFromCache_NoSKIP)
			return kFileOverwrite;
		else
			return kFileCancel;

	default:
		{
			return kFileCancel;
		}

	}
}

/////////////////////////////////////////////////////////////////////////////
// helper function
// 4-12-96 amc  Now returns an explicit ENUM
//              { kFileCancel, kFileOverwrite, kFileAppend }
//
static TFileUtil::EDisposition getfile_ask_user (
	CWnd *                pParentWnd,
	TYP_GETOUTPUTFILE *   psMode,
	const CString&        defaultDir,
	UINT                  defaultTitleStrID,
	int                   filterStringID,
	CString &             fileName)
{
	BOOL fRet;

	if (psMode->flags & GETFILE_FLG_TEXT)
	{
		// this is a text file, so append is a valid option

		// call up special message box
		CDlgAskAppend dlg(pParentWnd, fileName);

		// 0 - Overwrite
		// 1 - Append
		// 2 - Choose
		dlg.m_RadioButtonValue = 0;
		fRet = dlg.DoModal();

		// 7-19-96 fix bug -amc
		if (IDCANCEL == fRet)
			return TFileUtil::kFileCancel;
		else
		{
			if (0 == dlg.m_RadioButtonValue)
				return TFileUtil::kFileOverwrite;
			else if (1 == dlg.m_RadioButtonValue)
				return TFileUtil::kFileAppend;
			else
				return getfile_ask_altname (pParentWnd, psMode,
				defaultDir,
				defaultTitleStrID,
				filterStringID,
				fileName);
		}
	}
	else
	{
		CString strWriteMsg;

		AfxFormatString1 (strWriteMsg, IDS_OVERWRITE_FILE, LPCTSTR(fileName));
		TAskOverwrite sDlg (pParentWnd, strWriteMsg);

		// Choices:  Overwrite,  Alternate Name,  Skip
		int RC = sDlg.DoModal ();
		if (IDCANCEL == RC)
			return TFileUtil::kFileCancel;

		if (IDOK == RC)
			return TFileUtil::kFileOverwrite;

		return getfile_ask_altname (pParentWnd, psMode,
			defaultDir,
			defaultTitleStrID,
			filterStringID,
			fileName);
	}
}

///////////////////////////////////////////////////////////////////////////
// ask for alternate name
// 4-12-96 amc  Now returns an explicit ENUM
//              { kFileCancel, kFileOverwrite, kFileAppend }
TFileUtil::EDisposition getfile_ask_altname (
	CWnd *                pParentWnd,
	TYP_GETOUTPUTFILE *   psMode,
	const CString&        defaultDir,
	UINT                  defaultTitleStrID,
	int                   filterStringID,
	CString &             fileName)
{
	CString altName = fileName;
	BOOL fRet = TFileUtil::GetOutputFilename(
		&altName,
		pParentWnd,
		defaultTitleStrID,
		&defaultDir,
		0, // more flags
		filterStringID,
		fileName);
	if (!fRet)
		return TFileUtil::kFileCancel;
	fileName = altName;

	return TFileUtil::kFileOverwrite;
}

static void LoadFilterString (int filterStringID, LPTSTR pFilter, int bufsize)
{
	LoadString(AfxGetResourceHandle(), filterStringID, pFilter, bufsize);
	// the special character is ^.  Punch'em in
	int n = lstrlen(pFilter);
	for(--n; n >=0; --n)
		if ('^' == pFilter[n])
			pFilter[n] = NULL;
}

///////////////////////////////////////////////////////////////////////////
//  Pass in lpszFileName.  Function will prepend the application's path
//  and return a complete filespec
//
BOOL TFileUtil::UseProgramPath(LPCTSTR lpszFileName, TPath& spec)
{
	LPTSTR p = spec.GetBuffer(260);
	VERIFY( GetModuleFileName ( NULL, p, 260 ) );
	spec.ReleaseBuffer();

	{
		TPath path;
		spec.GetDirectory ( path );

		// basic filename TWINDOW.POS = adjacent to NEWS.EXE
		spec.FormPath ( path, lpszFileName );
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////
//
//

CString TFileUtil::GetAppData()
{
	CString strAppData("");
	int nLen = GetEnvironmentVariable(_T("APPDATA"), NULL, 0);
	if (!nLen)
		return strAppData;

	LPTSTR p = strAppData.GetBuffer(nLen+1);
	VERIFY((nLen-1) == GetEnvironmentVariable(_T("APPDATA"), p, nLen));
	strAppData.ReleaseBuffer();

	return strAppData;
}


CString TFileUtil::GetLocalAppData()
{
	CString strAppData("");
	int nLen = GetEnvironmentVariable(_T("LOCALAPPDATA"), NULL, 0);
	if (!nLen)
		return strAppData;

	LPTSTR p = strAppData.GetBuffer(nLen+1);
	VERIFY((nLen-1) == GetEnvironmentVariable(_T("LOCALAPPDATA"), p, nLen));
	strAppData.ReleaseBuffer();

	return strAppData;
}

CString TFileUtil::GetWinDir()
{
	CString strAppData("");
	int nLen = GetEnvironmentVariable(_T("WINDIR"), NULL, 0);
	if (!nLen)
		return strAppData;

	LPTSTR p = strAppData.GetBuffer(nLen+1);
	VERIFY((nLen-1) == GetEnvironmentVariable(_T("WINDIR"), p, nLen));
	strAppData.ReleaseBuffer();

	return strAppData;
}

///////////////////////////////////////////////////////////////////////
//
//

BOOL TFileUtil::GetInputFilename(
								 CString* pOutName,
								 CWnd*    pParentWnd,
								 UINT     stringIDTitle,
								 const CString* pInitDir,
								 DWORD moreFlags,
								 int   filterStringID)
{
	OPENFILENAME ofn;

	TCHAR        szFile[MAX_PATH] = "\0";

	LPTSTR  pFilter = 0;

	//if (!pOutName->IsEmpty())
	// strncpy (szFile, *pOutName, sizeof(szFile) - 1);
	memset ( &ofn, 0, sizeof(ofn) );

	CString title;
	CString filter;
	if (stringIDTitle)
		title.LoadString(stringIDTitle);

	if (filterStringID)
	{
		pFilter = new TCHAR[256];
		LoadFilterString (filterStringID, pFilter, 256);
	}

	ofn.lStructSize      = sizeof(ofn);
	ofn.hwndOwner        = (pParentWnd) ? pParentWnd->GetSafeHwnd() : AfxGetMainWnd()->GetSafeHwnd();
	ofn.hInstance        = AfxGetInstanceHandle();
	ofn.lpstrFilter      = pFilter;
	ofn.lpstrCustomFilter= NULL;
	ofn.nMaxCustFilter   = NULL;
	ofn.nFilterIndex     = NULL;
	ofn.lpstrFile        = szFile;
	ofn.nMaxFile         = sizeof(szFile);
	ofn.lpstrFileTitle   = NULL;
	ofn.nMaxFileTitle    = 0;
	ofn.lpstrInitialDir  = NULL;
	if (pInitDir && !pInitDir->IsEmpty())
		ofn.lpstrInitialDir = LPCTSTR(*pInitDir);
	ofn.lpstrTitle       = stringIDTitle ? LPCTSTR(title) : NULL;
	ofn.nFileOffset      = 0;
	ofn.nFileExtension   = 0;
	ofn.lpstrDefExt      = NULL;
	ofn.lCustData        = NULL;
	ofn.lpfnHook         = NULL;
	ofn.lpTemplateName   = NULL;
	ofn.Flags            = OFN_EXPLORER | OFN_FILEMUSTEXIST
		| moreFlags;

	BOOL ret = GetOpenFileName ( &ofn );

	if (ret)
	{
		LPTSTR ptr = pOutName->GetBuffer(strlen(ofn.lpstrFile));
		strcpy (ptr, ofn.lpstrFile);
		pOutName->ReleaseBuffer();
	}

	delete pFilter;
	return ret;
}

int make_temp_filename(CString& result)
{
	TCHAR rcFname[MAX_PATH];
	DWORD dwRet = GetTempPath(1, rcFname);      // see how much we need
	TCHAR* pPath = new TCHAR[dwRet + 2];
	if (pPath)
	{
		GetTempPath ( dwRet, pPath );
		make_arbitrary_filename ( pPath, result );
		delete [] pPath;
	}
	return 0;
}

int make_arbitrary_filename(const CString& path, CString& result)
{
	TCHAR rcFname[MAX_PATH];

	// if you pass in 0 as the unique integer, a File is created,a name is mad
	//  else just the name is made
	UINT u = GetTempFileName ( path,
		"MPI",
		0,
		rcFname );
	result = LPCTSTR(rcFname);

	// I don't want you to create it. Just give me a name
	CFile::Remove ( result );
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// Re-written on 04-11-2001
//
int file_append(const CString& name1, const CString& name2, BOOL fDelete)
{
	HANDLE  hDest;
	HANDLE  hSrc;

	hSrc  = CreateFile (name2, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hSrc)
		return -1;

	hDest = CreateFile (name1, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hDest)
	{
		CloseHandle (hSrc);
		return -1;
	}

	// append to dest

	SetFilePointer (hDest, 0, NULL, FILE_END);

	DWORD  dwSizeLow;
	DWORD  dwSizeHi;

	dwSizeLow = dwSizeHi = 0;

	dwSizeLow = GetFileSize (hSrc, &dwSizeHi);

	TCHAR  rcBuf[4096];

	DWORD  dwBytesLeft = dwSizeLow;
	DWORD  dwBytesToRead, dwBytesRead;
	BOOL   fReadStat, fWriteStat;
	DWORD  dwBytesWritten;

	while (dwBytesLeft > 0)
	{
		if (dwBytesLeft > sizeof(rcBuf))
			dwBytesToRead = sizeof(rcBuf);
		else
			dwBytesToRead = dwBytesLeft;

		fReadStat = ReadFile (hSrc, rcBuf, dwBytesToRead, &dwBytesRead, NULL);

		if (0 == fReadStat || (dwBytesRead != dwBytesToRead))
		{
			CloseHandle (hSrc);
			CloseHandle (hDest);
			return -1;
		}

		fWriteStat = WriteFile (hDest, rcBuf, dwBytesRead, &dwBytesWritten, NULL);

		if (0 == fWriteStat || (dwBytesWritten != dwBytesRead))
		{
			CloseHandle (hSrc);
			CloseHandle (hDest);
			return -1;
		}

		dwBytesLeft -= dwBytesRead;
	}

	CloseHandle (hSrc);
	CloseHandle (hDest);

	if (fDelete)
		CFile::Remove (name2);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// TrashDirectory - Recursively remove directory.  Doesn't handle
//                  read only or hidden files, since I don't anticipate
//                  us creating them.  Making it do that should be
//                  easy, though.
/////////////////////////////////////////////////////////////////////////////
BOOL  TrashDirectory (LPCTSTR directory)
{
	HANDLE            thisDirectory;
	WIN32_FIND_DATA   findData;
	TCHAR             buff[512];
	int               len;

	_tcscpy (buff, directory);

	len = _tcslen (buff);

	if (buff[len - 1] != '\\')
	{
		buff[len] = '\\';
		buff[len + 1] = 0;
	}

	_tcscat (buff, "*.*");

	thisDirectory = FindFirstFile (buff, &findData);

	DWORD dwErr;
	if (thisDirectory == INVALID_HANDLE_VALUE)
	{
		dwErr = GetLastError();
		if (dwErr == ERROR_PATH_NOT_FOUND)
			return TRUE; // Directory doesn't exist - function has succeeded!
		if (dwErr != ERROR_FILE_NOT_FOUND)
			return FALSE; // Abnormal error, failed
	}
	else
	{
		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_tcscmp (findData.cFileName, ".") &&
					_tcscmp (findData.cFileName, ".."))
				{
					// form new subpath and recurse...
					_tcscpy (buff, directory);
					if (buff[_tcslen (buff) - 1] != '\\')
						_tcscat (buff, "\\");
					_tcscat (buff, findData.cFileName);

					if (!TrashDirectory (buff))
					{
						FindClose (thisDirectory);
						return FALSE;
					}
				}
			}
			else
			{
				_tcscpy (buff, directory);
				if (buff[_tcslen (buff) - 1] != '\\')
					_tcscat (buff, "\\");
				_tcscat (buff, findData.cFileName);
				TRACE ("Removing file %s\n", buff);
				if (!DeleteFile (buff))
				{
					FindClose (thisDirectory);
					return FALSE;
				}
			}
		}
		while (FindNextFile (thisDirectory, &findData));

		if (!FindClose (thisDirectory))
			return FALSE;
	}

	TRACE ("Removing directory %s\n", directory);

	if (!RemoveDirectory (directory))
		if (GetLastError() != ERROR_PATH_NOT_FOUND)
			return FALSE; // abnormal error - failed

	return TRUE;
}

//
// This works just like xcopy
//
// e.g.
// strFrom is a directory = c:\temp or c:\temp\
// strTo is an existing directory = c:\blah or c:\blah\
// Result c:\blah\temp\ 
//
// strFrom is a wildcard = c:\temp\*.*
// strTo is an existing directory = c:\blah or c:\blah\
// Result c:\blah\<files in c:\temp>
//
// strFrom is a filename = c:\temp\boo.txt
// strTo is an existing directory = c:\blah or c:\blah\
// Result c:\blah\boo.txt
//
// strFrom MUST exist.
// strTo MUST exist and it must be a directory
//
bool CopyDirectory(CString strFrom, CString strTo, bool bRecurse)
{
	if (!strTo.GetLength() || !strFrom.GetLength())
		return false;

	HANDLE            fromDirectory;
	WIN32_FIND_DATA   findData;

	// MAke sure strTo ends in a slash as it is a directory
	strTo.TrimRight('\\');

	// See if destination directory is there?
	struct _stat buf;
	int result;
	result = _stat(strTo, &buf);
	if (result != 0)
		return false; // Does not exist at all
	if (!(buf.st_mode & _S_IFDIR))
		return false; // Not a directory
	if (!(buf.st_mode & _S_IWRITE))
		return false; // We do not have write permission

	bool bDirectory = false;
	bool bFile = false;
	bool bWildCard = false;

	// If strFrom end in a slash its a directory
	if (strFrom.Right(1) == "\\")
		bDirectory = true;
	else
	{
		// Else it could be a directory, file or wildcard
		result = _stat(strFrom, &buf);
		if (result != 0)
			bWildCard = true;
		else if ((buf.st_mode & _S_IFDIR))
			bDirectory = true; // It is a directory
		else if ((buf.st_mode & _S_IFREG))
			bFile = true; // It is a file
		else
			bWildCard = true;
	}

	// If we cannot determine what the source is, return error
	if (!bDirectory && !bFile && !bWildCard)
		return false;

	if (bFile)
	{
		CString strTmp;
		if (strFrom.ReverseFind('\\') != -1)
			strTmp = strFrom.Mid(strFrom.ReverseFind('\\')+1);
		else
			strTmp = strFrom;
		if (CopyFile(strFrom, strTo+"\\"+strTmp, FALSE))
			return (WaitForFile(strTo+"\\"+strTmp));

		return false;
	}
	else if (bWildCard)
	{
		// Wildcard
		fromDirectory = FindFirstFile(strFrom, &findData);
	}
	else
	{
		// Directory
		strFrom.TrimRight('\\');
		strFrom += "\\";
		fromDirectory = FindFirstFile(strFrom+"*.*", &findData);
	}

	if (fromDirectory == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (bRecurse &&
				(CString(findData.cFileName) != ".") &&
				(CString(findData.cFileName) != ".."))
			{
				// Create the sub dir in the destination
				if (_access(strTo+"\\"+CString(findData.cFileName), 6) == -1)
				{
					if (!CreateDirectory(strTo+"\\"+CString(findData.cFileName), NULL))
						return false;
					else if (!WaitForFile(strTo+"\\"+CString(findData.cFileName)))
						return false;
				}
				// Recurse into sub dir
				if (!CopyDirectory(strFrom+CString(findData.cFileName), strTo+"\\"+CString(findData.cFileName), true))
				{
					FindClose(fromDirectory);
					return false;
				}
			}
		}
		else
		{
			TRACE("Copying file %s\n", findData.cFileName);
			if (!CopyFile(strFrom+CString(findData.cFileName), strTo+"\\"+CString(findData.cFileName), FALSE))
			{
				FindClose(fromDirectory);
				return false;
			}
			else if (!WaitForFile(strTo+"\\"+CString(findData.cFileName)))
			{
				FindClose(fromDirectory);
				return false;
			}
		}
	}
	while (FindNextFile(fromDirectory, &findData));

	if (!FindClose(fromDirectory))
		return false;

	return true;
}

// ------------------------------------------------------------------------
// the first argument is a HWND, since this may be called by different
//  threads
int fnNNTPError(const CString & strWhen, int err, LPCTSTR msg)
{
	extern HWND ghwndMainFrame;

	int strid;
	switch (err)
	{
	case 500:
		strid = IDS_ERR_NNTP500;
		break;
	case 501:
		strid = IDS_ERR_NNTP501;
		break;
	case 502:
		strid = IDS_ERR_NNTP502;
		break;
	case 503:
		strid = IDS_ERR_NNTP503;
		break;
	default:
		strid = IDS_ERR_NNTP501;
		break;
	}
	CString line;
	AfxFormatString1 (line, strid, msg);

	TNNTPErrorDialog sDlg (CWnd::FromHandle (ghwndMainFrame));

	sDlg.m_strWhen = strWhen;
	sDlg.m_strReason = line;
	sDlg.m_fTimer = true;
	sDlg.m_iSeconds = 120;
	sDlg.DoModal ();

	return 0;
	//return NewsMessageBox(TGlobalDef::kMainWnd,
	//LPCTSTR(line), MB_OK | MB_ICONSTOP, NULL);
}

//-------------------------------------------------------------------------
int NewsShowCException (
						CWnd* pWnd,
						CException * pExc,
						UINT  typeMsgBox,
						LPCTSTR caption
						)
{
	static CString strFileError;  // static to avoid using stack

	// use pExc->ReportError ?
	TCHAR rcBuf[256];
	if (FALSE == pExc->GetErrorMessage(rcBuf, sizeof(rcBuf)))
	{
		strFileError.LoadString (IDS_ERR_FILE_UNKNOWN);
		lstrcpy (rcBuf, strFileError);
	}
	return NewsMessageBox(pWnd, rcBuf, typeMsgBox);
}

/////////////////////////////////////////////////////////////////////////////
// TAskOverwrite dialog

TAskOverwrite::TAskOverwrite(CWnd* pParent,  const CString & msg)
: CDialog(TAskOverwrite::IDD, pParent), m_strMessage(msg)
{
}

void TAskOverwrite::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MESSAGE, m_strMessage);
}

BEGIN_MESSAGE_MAP(TAskOverwrite, CDialog)
	ON_BN_CLICKED(IDC_SAVE_AS, OnSaveAs)

END_MESSAGE_MAP()

void TAskOverwrite::OnSaveAs()
{
	EndDialog (IDC_SAVE_AS);
}

int GetWinsockErrorID (int iLastError)
{
	switch (iLastError)
	{
	case WSAEACCES:         return IDS_WSAEACCES;
	case WSAEADDRINUSE:     return IDS_WSAEADDRINUSE;
	case WSAEADDRNOTAVAIL:  return IDS_WSAEADDRNOTAVAIL;
	case WSAENETDOWN:       return IDS_WSAENETDOWN;
	case WSAENETUNREACH:     return IDS_WSANETUNREACH;
	case WSAENETRESET:       return IDS_WSANETRESET;
	case WSAECONNABORTED:   return IDS_WSAECONNABORTED;
	case WSAECONNRESET:     return IDS_WSAECONNRESET;
	case WSAENOBUFS:        return IDS_WSAENOBUFS;
	case WSAEISCONN:        return IDS_WSAEISCONN;
	case WSAENOTCONN:       return IDS_WSAENOTCONN;
	case WSAESHUTDOWN:      return IDS_WSAESHUTDOWN;
	case WSAETIMEDOUT:      return IDS_WSAETIMEDOUT;
	case WSAECONNREFUSED:    return IDS_WSACONNREFUSED;
	case WSAEHOSTDOWN:      return IDS_WSAEHOSTDOWN;
	case WSAEHOSTUNREACH:   return IDS_WSAEHOSTUNREACH;
	case WSAHOST_NOT_FOUND: return IDS_WSAHOST_NOT_FOUND;
	case WSANO_RECOVERY:    return IDS_WSANO_RECOVERY;
	case WSANO_DATA:        return IDS_WSANO_DATA;
	case WSATRY_AGAIN:      return IDS_WSATRY_AGAIN;
	}
	return 0;
}

bool TFileUtil::Unzip(CString strSourceFile, CString strDestDir)
{
	USES_CONVERSION;
	wchar_t* wstr_SourceFile = T2W(strSourceFile.GetBuffer());
	strSourceFile.ReleaseBuffer();
	wchar_t* wstr_command = T2W(strDestDir.GetBuffer());
	strDestDir.ReleaseBuffer();

	Cabinet::CExtract i_Extract;

	if (i_Extract.CreateFDIContext())
		if (i_Extract.ExtractFileW(wstr_SourceFile, wstr_command))
			if (i_Extract.DestroyFDIContext())
				return true;

	return false;
}

bool TFileUtil::Zip(CString strDestFile, CString strSourceDir)
{
	// Compress all the contents of the temp folder
	USES_CONVERSION;
	wchar_t* wstr_DestFile = T2W(strDestFile.GetBuffer());
	strDestFile.ReleaseBuffer();
	wchar_t* wstr_command = T2W(strSourceDir.GetBuffer());
	strSourceDir.ReleaseBuffer();

	Cabinet::CCompress i_Compress;

	if (i_Compress.CreateFCIContextW(wstr_DestFile, TRUE, TRUE, 0x7FFFFFFF, 12345))
		if (i_Compress.AddFolderW(wstr_command))
			if (i_Compress.DestroyFCIContext())
				return true;

	return false;
}

//
// Running under Windows 7 I have seen the following behaviour:-
//
// As the program runs, any (some?) Win32 file handling functions that
// are called do not execute at the time they are called. they only
// execute when this program goes idle (dialog box or whatever).
//
// A bit like Windows messages.
//
// That in itself wouldn't be a problem, what is a problem is that
// sometimes the file functions seem to be executed in a different
// order to the order I called them.
// So a rename followed by an open of the newly renamed file, the rename
// doesn't take place until after the open file has failed.
//
// To get round it I've created this function which simply Sleeps a
// few mS until the file (or directory) specified exists.
//
// Stupid that I need to do this, but it works as a workaround.
//
bool WaitForFile(CString &strFilename)
{
	int nAttempts = 0;
	while (nAttempts < 5)
	{
		if (::GetFileAttributes(strFilename) != INVALID_FILE_ATTRIBUTES)
		{
			nAttempts = -1;
			break;
		}
		else
			Sleep(10);
		nAttempts++;
	}
	if (nAttempts > 0)
		::AfxMessageBox("multiple attempts at opening reg file");
	return (nAttempts == -1);
}
