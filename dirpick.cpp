/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: dirpick.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:51:21  richard_wood
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
#include "resource.h"
#include "dlgs.h"       // system defined cmndlg ctrl ids like   stc1, edt1
#include "globals.h"

#include "mplib.h"         // TPath
#include "dirpick.h"
#include "ndirpick.h"
#include "uimem.h"
#include "dllver.h"
#include <winnetwk.h>

#include "shlobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BOOL CALLBACK CustomDirPickDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static BOOL Shell_DirectoryPicker ( DWORD dwVer, CWnd* pParentWnd, CString* pChosenDir );

////////////////////////////////////////////////////////////////////////
// Note that NT 3.51 still does not have the Explorer-type dialog box
//
BOOL DirectoryPicker(CWnd* pParentWnd, CString* pChosenDir)
{
	// use the NT picker on versions of NT less than 4.0
	if (VER_PLATFORM_WIN32_NT == gdwPlatform &&
		(GetVersion () & 0xFF) < 4)
	{
		return NT_DirectoryPicker(pParentWnd, pChosenDir);
	}

	DWORD verShell = GetDllVersion (_T("shell32.dll"));

	// Try out the new Shell-based browse function
	return Shell_DirectoryPicker ( verShell, pParentWnd, pChosenDir );
}

BOOL CALLBACK CustomDirPickDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		//CommDlg_OpenSave_HideControl ( GetParent(hDlg), edt1 );
		CommDlg_OpenSave_SetControlText ( GetParent(hDlg), edt1, "junk" );
		return FALSE;

	case WM_NOTIFY:
		switch (((LPOFNOTIFY)lParam)->hdr.code)
		{
		case CDN_FOLDERCHANGE:
			{
				char szFolderPath[MAX_PATH];
				BOOL fErr = TRUE;
				if (
					CommDlg_OpenSave_GetFolderPath ( GetParent(hDlg),
					szFolderPath,
					sizeof(szFolderPath))
					<= sizeof(szFolderPath)
					)
				{
					if (GetDlgItem (hDlg, IDC_DIRPICK_DIRPATH))
					{
						SetDlgItemText (hDlg, IDC_DIRPICK_DIRPATH, szFolderPath);
						fErr = FALSE;
						//LPTSTR ptr = gptrDirString->GetBuffer (1024);
						//if (CommDlg_OpenSave_GetFolderPath ( GetParent(hDlg), ptr, 1024 ) <= 1024)
						//{
						//}
						//gptrDirString->ReleaseBuffer();
					}
				}

				if (fErr)
					SetDlgItemText (hDlg, IDC_DIRPICK_DIRPATH, "");

			} // end case
			break;

		} // end switch
		break; // end wm_notify

	default:
		return FALSE;

	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// Note that NT 3.51 still does not have the Explorer-type dialog box
//   customize the old 3.1 version
BOOL NT_DirectoryPicker(CWnd* pParentWnd, CString* pChosenDir)
{
	// TODO: Add your command handler code here

	CNTDirDlg  cfdlg(FALSE, NULL, NULL, OFN_SHOWHELP | OFN_HIDEREADONLY |
		OFN_OVERWRITEPROMPT | OFN_ENABLETEMPLATE, NULL,
		pParentWnd);

	cfdlg.m_ofn.hInstance = AfxGetInstanceHandle();
	cfdlg.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIRPICK_NT);

	TPath initDir;
	gpUIMemory->GetPath (TUIMemory::DIR_DECODE_DEST, initDir);
	if (initDir.IsEmpty())
		cfdlg.m_ofn.lpstrInitialDir = NULL;
	else
		cfdlg.m_ofn.lpstrInitialDir = LPCTSTR(initDir);

	if (IDOK==cfdlg.DoModal())
	{
		WORD wFileOffset;

		wFileOffset = cfdlg.m_ofn.nFileOffset;  //for convenience

		cfdlg.m_ofn.lpstrFile[wFileOffset-1]=0; //Nuke the "Junk"
		LPTSTR p = pChosenDir->GetBuffer(wFileOffset);
		strcpy(p , cfdlg.m_ofn.lpstrFile );
		pChosenDir->ReleaseBuffer();

		gpUIMemory->SetPath (TUIMemory::DIR_DECODE_DEST, *pChosenDir);
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// I don't know if I can set the initial directory (TUIMemory) ?????
//
BOOL Shell_DirectoryPicker ( DWORD dwVerShell, CWnd* pParentWnd, CString* pChosenDir )
{
	BROWSEINFO     bi;
	LPITEMIDLIST   pIDList;
	char           rcBuf[MAX_PATH];
	BOOL           fRet = FALSE;

	// newer versions require CoInitialize
	HRESULT hr = CoInitialize (NULL);

	memset (&bi, 0, sizeof(bi));
	memset (rcBuf, 0, sizeof(rcBuf));

	bi.hwndOwner      = pParentWnd->GetSafeHwnd();
	bi.pszDisplayName = rcBuf;
	CString str; str.LoadString (IDS_MAKE_DIR_CHOICE);
	bi.lpszTitle      = str;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS;

	if (dwVerShell >= PACKVERSION(5,0))
		bi.ulFlags |= 0x40; //BIF_NEWDIALOGSTYLE;

	pIDList = SHBrowseForFolder (&bi);

	if (pIDList)
	{
		if (SHGetPathFromIDList ( pIDList, rcBuf ))
		{
			*pChosenDir = LPCTSTR(rcBuf);

			fRet = TRUE;
		}

		// free up the mem that OLE allocated for us
		CoTaskMemFree ( pIDList );
	}

	if (SUCCEEDED(hr))
		CoUninitialize();
	return fRet;
}
