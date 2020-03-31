/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: gallery.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/26 11:25:38  richard_wood
/*  Updated "Compress" icon.
/*  Updated "DejaNews" icon and URL to Google Groups.
/*  Added error message for invalid date time format string.
/*  Fixed function ID for Run Rule Manually button.
/*  Updated greyscale buttons to look a lot more ghost like (less contrast, easier to tell they're inactive)
/*
/*  Revision 1.2  2008/09/19 14:51:25  richard_wood
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

// gallery.cpp -- image-gallery functions

#include "stdafx.h"              // precompiled header
#include "gallery.h"             // this file's prototypes
#include "genutil.h"             // GetStartupDir(), ...
#include "resource.h"            // IDS_IMAGE_GALLERY, ...
#include "tglobopt.h"            // gpGlobalOptions, ...
#include "rgwarn.h"              // TRegWarn
#include "warndlg.h"             // WarnWithCBX()
#include "safelnch.h"            // CFileWarn, CSafeLaunch
#include "globals.h"             // gSafeLaunch
#include "ipcgal.h"
#include "utlmacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
HWND GetGalleryWindow ()
{
	CString strClass; strClass.LoadString (IDS_CLASS_GALLERY);
	return FindWindow (strClass, NULL);
}

// -------------------------------------------------------------------------
static void GalleryViewBinary (LPCTSTR pchFilename)
{
	// write the file name to view.txt, then either ping the image gallery if
	// it's running, or start it up

	IPC_Gallery_ViewImage ( pchFilename );

	HWND hWnd = GetGalleryWindow ();
	if (hWnd)
		PostMessage (hWnd, WM_COMMAND, ID_PING_VIEW_IMAGE, 0);
	else
		StartGallery ();
}

// -------------------------------------------------------------------------
// ViewBinary -- for safety's sake, only the UI thread should call this.
// ShellExecute may use DDE internally and the UI thread has already called
// DdeInitialize
void ViewBinary (bool fPanicMode, const CString &strFullName_In)
{
	// if the boss is nearby, do nothing
	if (fPanicMode)
		return;

	if (gpGlobalOptions->IsUseImageGallery ()) {
		GalleryViewBinary (strFullName_In);
		return;
	}

	CString  program;

	if (gpGlobalOptions->IsUseOtherViewer ())
		program = gpGlobalOptions->GetOtherViewer();
	gSafeLaunch.Launch (program, strFullName_In);
}

struct SShellExecuteErrors
{
	int nErrCode;
	CString strErrorText;
};

static SShellExecuteErrors ShellExecuteErrors[] =
{
	{0, "The operating system is out of memory or resources."},
	{ERROR_FILE_NOT_FOUND, "The specified file was not found."},
	{ERROR_PATH_NOT_FOUND, "The specified path was not found."},
	{ERROR_BAD_FORMAT, "The .exe file is invalid (non-Microsoft Win32 .exe or error in .exe image)."},
	{SE_ERR_ACCESSDENIED, "The operating system denied access to the specified file."},
	{SE_ERR_ASSOCINCOMPLETE, "The file name association is incomplete or invalid."},
	{SE_ERR_DDEBUSY, "The Dynamic Data Exchange (DDE) transaction could not be completed because other DDE transactions were being processed."},
	{SE_ERR_DDEFAIL, "The DDE transaction failed."},
	{SE_ERR_DDETIMEOUT, "The DDE transaction could not be completed because the request timed out."},
	{SE_ERR_DLLNOTFOUND, "The specified dynamic-link library (DLL) was not found."},
	{SE_ERR_FNF, "The specified file was not found."},
	{SE_ERR_NOASSOC, "There is no application associated with the given file name extension. This error will also be returned if you attempt to print a file that is not printable."},
	{SE_ERR_OOM, "There was not enough memory to complete the operation."},
	{SE_ERR_PNF, "The specified path was not found."},
	{SE_ERR_SHARE, "A sharing violation occurred."},
};

// -------------------------------------------------------------------------
void StartGallery ()
{
	CString strPath = GetStartupDir () + "gallery.exe";
	HINSTANCE RC = ShellExecute (::GetDesktopWindow (), "open",
		strPath, NULL /* command line */, NULL /* directory */, SW_SHOW);
	if ((int) RC <= 32)
	{
		for (int i = 0; i < ELEM(ShellExecuteErrors); i++)
		{
			if (ShellExecuteErrors[i].nErrCode == (int)RC)
			{
				CString str; str.LoadString (IDS_ERR_LAUNCH_IG);
				CString strError; strError.Format (str, ShellExecuteErrors[i].strErrorText);
				AfxGetMainWnd()->MessageBox (strError);
				return;
			}
		}
		CString str; str.LoadString (IDS_ERR_LAUNCH_IG);
		CString strError; strError.Format (str, "Unknown error");
		AfxGetMainWnd ()->MessageBox (strError);
		return;
	}

	// wait until the image gallery has put up its main window... this prevents
	// multiple image galleries from being launched if the decoding is fast and
	// the image gallery is slow to start up.  Wait up to 10 seconds
	for (int i = 0; i < 10; i++)
	{
		if (GetGalleryWindow ())
			break;
		Sleep (1000);     // wait a second
	}
}
