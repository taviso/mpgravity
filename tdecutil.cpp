/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdecutil.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.3  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:00  richard_wood
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

// tdecutil.cpp -- decoding utils

#include "stdafx.h"              // precompiled header
#include "tdecutil.h"            // this file's prototypes
#include "server.h"              // TNewsServer
#include "nglist.h"              // TNewsGroupUseLock
#include "tglobopt.h"            // TGlobalOptions, ...
#include "warndlg.h"             // WarnWithCBX()
#include "genutil.h"             // GetStartupDir(), ...
#include "gallery.h"             // GetImageGalleryWindow()
#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern HWND fnAccessSafeFrameHwnd(BOOL fLock);

// -------------------------------------------------------------------------
void GetDecodeDirectory (CString &strDir, LONG lGroupID)
{
	// first Try to get the group-specific directory
	TServerCountedPtr cpNewsServer;
	BOOL fUseLock;
	TNewsGroup *psNG;
	TNewsGroupUseLock useLock (cpNewsServer, lGroupID, &fUseLock, psNG);
	if (fUseLock)
	{
		// i don't think we need to open the newsgroup for only this! -amc

		if (psNG -> GetOverrideDecodeDir ())
			strDir = psNG -> GetDecodeDir ();

		if (!strDir.IsEmpty ())
			return;
	}

	// get global directory
	strDir = gpGlobalOptions -> GetDecodeDirectory();
	if (!strDir.IsEmpty ())
		return;

	// if no global directory was specified, use the directory that this
	// program was started from
	strDir = TFileUtil::GetAppData() + "\\Gravity\\" + "download";
}

// -------------------------------------------------------------------------
// returns 0 for success
int  EnsureDirExists (CString &strDir)
{
	if (0 == _taccess (strDir, 0))
		return 0;   // dir exists

	if ( CreateDirectory (strDir, NULL) )
		return 0;   // success

	return 1;
}

// -------------------------------------------------------------------------
static CString EscapeBackslashesAndCommas (LPCTSTR pch)
{
	CString str;
	int iLen = _tcslen (pch);
	for (int i = 0; i < iLen; i++) {
		if (pch [i] == ',' || pch [i] == '\\')
			str += '\\';
		str += pch [i];
	}
	return str;
}

// -------------------------------------------------------------------------
static void WriteSessionStartTime ()
{
	static BOOL sbDone;  // do only once a session
	if (sbDone)
		return;
	sbDone = TRUE;

	CString strFullKey = _T("Software\\MicroPlanet\\Image Gallery\\General");
	HKEY hKey;
	DWORD dwDisposition;
	if (RegCreateKeyEx (HKEY_CURRENT_USER, strFullKey, 0 /* reserved */,
		_T("prefdata") /* lpClass */, 0 /* dwOptions */, KEY_ALL_ACCESS,
		NULL /* lpSecurityAttributes */, &hKey, &dwDisposition) != ERROR_SUCCESS)
		return;
	DWORD t = time (NULL);
	RegSetValueEx (hKey, _T("Session Start Time"), 0 /* reserved */, REG_DWORD,
		(const BYTE *) &t, sizeof t);
	RegCloseKey (hKey);
}

// -------------------------------------------------------------------------
void NotifyImageGallery (LPCTSTR pchFilename, LPCTSTR pchSubject,
						 LPCTSTR pchGroupNickname)
{
	try
	{
		WriteSessionStartTime ();

//		CString strFile = GetStartupDir () + "add.txt";
		CString strFile = TFileUtil::GetAppData() + "\\Gravity\\" + "add.txt";

		// get decoded file's size
		int iSize = 0;
		CFileStatus sStatus;
		if (CFile::GetStatus (pchFilename, sStatus))
			iSize = sStatus.m_size;

		// construct the line to add to add.txt
		DWORD t = time (NULL);
		CString strLine;
		strLine.Format (_T("%s,%s,%s,%d,%d\n"),
			LPCTSTR(EscapeBackslashesAndCommas (pchFilename)),
			LPCTSTR(EscapeBackslashesAndCommas (pchSubject)),
			LPCTSTR(EscapeBackslashesAndCommas (pchGroupNickname)),
			iSize, t);

		// Try writing to add.txt ten times, pausing between each time and the
		// next.  To avoid synchronization problems, we open it in exclusive mode
		int i;
		for (i = 0; i < 10; i++) {
			try {
				CStdioFile sFile (strFile, CFile::modeWrite | CFile::modeCreate |
					CFile::modeNoTruncate | CFile::shareExclusive);

				// if add.txt is more than 500K in size, remove it (older versions
				// of Gravity didn't do the size checking so it may have grown
				// quite large)
				// also, if the file is more than 100K in size, don't write to it
				CFileStatus sStatus;
				BOOL bWrite = TRUE;
				BOOL bRemove = FALSE;
				if (sFile.GetStatus (sStatus)) {
					LONG lSize = sStatus.m_size;
					if (lSize > 100*1024)
						bWrite = FALSE;
					if (lSize > 500*1024)
						bRemove = TRUE;
				}

				if (bWrite) {
					sFile.SeekToEnd ();
					sFile.WriteString (strLine);
				}
				sFile.Close ();

				if (bRemove)
					DeleteFile (strFile);

				break;         // success... don't Try anymore
			}
			catch(...) {
				Sleep (1000);  // wait a second
			}
		}

		if (i >= 10 && gpGlobalOptions -> WarnOnErrorDuringDecode ()) 
		{

			HWND hWndMF = fnAccessSafeFrameHwnd(TRUE);

			if (hWndMF)
			{
				BOOL bDoNotWarn;
				CString str; str.LoadString (IDS_ERR_NOTIFY_IMAGE_GALLERY);
				WarnWithCBX (str, &bDoNotWarn, CWnd::FromHandle(hWndMF), TRUE);
				gpGlobalOptions -> SetWarnOnErrorDuringDecode (!bDoNotWarn);
			}

			fnAccessSafeFrameHwnd(FALSE);
			return;
		}

		// also notify running instance of image gallery
		HWND hWnd = GetGalleryWindow ();
		if (hWnd)
			PostMessage (hWnd, WM_COMMAND, ID_PING_IMAGE_GALLERY, 0);
	}
	catch(...)
	{
		// one bug report indicated that 'pchSubject' was passed in as 0x007
		//  which is odd since it comes from a CString.

		// wants %x %x %x, so pass in 3 integers
		CString errorMsg;
		errorMsg.Format (IDS_ERR_EXCEPT_NOTIFY_GALLERY, DWORD(pchFilename),
			DWORD(pchSubject), DWORD(pchGroupNickname));
		AfxMessageBox (errorMsg);
	}
}
