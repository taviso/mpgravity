/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fileutil.h,v $
/*  Revision 1.2  2010/07/24 21:57:03  richard_wood
/*  Bug fixes for Win7 executing file ops out of order.
/*  V3.0.1 RC2
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/07/26 18:35:43  richard_wood
/*  Changed back from ShellExecute to system as ShellExecute doesn't work from program files?
/*
/*  Revision 1.4  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
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
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
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

#pragma once

#include "mplib.h"
#include "tmsgbx.h"

#define GETFILE_FLG_TEXT            0x01

#define GETFILE_ACT_ASKUSER         1
#define GETFILE_ACT_SKIP            2
#define GETFILE_ACT_OVERWRITE       3
#define GETFILE_ACT_ALTNAME         4

struct TYP_GETOUTPUTFILE
{
	WORD  flags;
	WORD  action;

	TYP_GETOUTPUTFILE()
	{
		flags = 0;
		action = 0;
	}

	void SetAction ();
};

class TFileUtil
{
public:
	// Get various user environment variables that point into user profile storage space
	static CString GetAppData();
	static CString GetLocalAppData();
	static CString GetWinDir();

	enum EAccess { kEXIST, kREAD, kWRITE, kREADWRITE };
	enum EDisposition { kFileCancel, kFileOverwrite, kFileAppend };

	static BOOL FileExist(LPCTSTR fileName);
	static BOOL FileAccess(LPCTSTR fileName, TFileUtil::EAccess eAccess);

	// Compress or extract a cabinet format zip file
	static bool Unzip(CString strSourceFile, CString strDestDir);
	static bool Zip(CString strDestFile, CString strSourceDir);

	static BOOL GetOutputFilename(
		CString*			pOutName,
		CWnd*				pParentWnd = NULL,
		UINT				stringIDTitle = 0,
		const CString*		pInitDir = 0,
		DWORD				moreFlags = 0,
		int					filterStringID = 0,
		LPCTSTR				pchOriginalFilename = NULL);

	static int CreateOutputFile(
		CWnd*				pParentWnd,
		CString&			fileName,
		const CString&		defaultDir,
		UINT				defaultTitleStrID,
		CFile&				fl,
		TYP_GETOUTPUTFILE * psMode,
		bool				fFromCache_NoSKIP,
		int					filterStringID = 0);

	static TFileUtil::EDisposition CheckFileExist(
		CString &           fileName,
		CWnd *              pParentWnd,
		const CString&      defaultDir,
		UINT                defaultTitleStrID,
		int                 filterStringID,
		TYP_GETOUTPUTFILE * psMode,
		bool                fFromCache_NoSKIP);

	// create a path adjacent to .EXE
	static BOOL UseProgramPath(LPCTSTR fileName, TPath& spec);

	static BOOL GetInputFilename(
		CString*			pOutName,
		CWnd*				pParentWnd = NULL,
		UINT				stringIDTitle = 0,
		const CString*		pInitDir = 0,
		DWORD				moreFlags	= 0,
		int					filterStringID = 0);
};

int NewsShowCException (
						CWnd* pWnd,
						CException * pException,
						UINT nType = MB_OK,
						LPCTSTR caption = NULL );

// always uses hwnd of MainFrame
int fnNNTPError ( const CString & strWhen, int err, LPCTSTR msg);

int make_temp_filename(CString& result);

int make_arbitrary_filename(const CString& path, CString& result);

int file_append(const CString& name1, const CString& name2, BOOL fDelete = FALSE);

// Recursively remove directory - doesn't handle read only or hidden
BOOL TrashDirectory (LPCTSTR directory);

bool CopyDirectory(CString strFrom, CString strTo, bool bRecurse);
bool WaitForFile(CString &strFilename);

int GetWinsockErrorID (int iLastError);

/////////////////////////////////////////////////////////////////////////////
// TAskOverwrite dialog

#ifndef IDD_ASKOVERWRITE
#include "resource.h"      // make sure resource.h is included by this point
#endif

class TAskOverwrite : public CDialog
{
public:
	TAskOverwrite(CWnd* pParent, const CString & msg);   // standard constructor

	enum { IDD = IDD_ASKOVERWRITE };
	CString  m_strMessage;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSaveAs();
	DECLARE_MESSAGE_MAP()
};
