/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ipcgal.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:27  richard_wood
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
#include <afxmt.h>
#include "ipcgal.h"
#include "resource.h"
#include "gallery.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#pragma pack(1)

struct TGALLERY_CLIPBOARD
{
	TCHAR rcFileName[1024];
	TCHAR rcFileName2[1024];
	TCHAR rcFileName3[1024];

	LONG  iFileCount;

	LONG  iProgress;

	LONG  iImagesWaiting;
	LONG  iImagesCompleted;
};

typedef TGALLERY_CLIPBOARD*  LPTGALLERY_CLIPBOARD;

LPTGALLERY_CLIPBOARD  gpGalleryClipboard = NULL;

HANDLE  ghIpcGallery = NULL;

CMutex  sGalleryMutex(FALSE, TEXT("GRAVITY_GALLERY_MUTEX"));

BOOL IPC_Gallery_Startup (void)
{
	HANDLE hMappedObject;
	TCHAR szMappedObjectName[] = TEXT("GRAVITY_GALLERY_MEMCHUNK");

	// initial value
	gpGalleryClipboard = NULL;

	hMappedObject = CreateFileMapping ((HANDLE) 0xFFFFFFFF,
		NULL,
		PAGE_READWRITE,
		0,
		4096,
		szMappedObjectName);
	if (NULL == hMappedObject)
	{
		TRACE1 ("Could not create mapped object for Gallery %x\n",
			GetLastError ());
		gpGalleryClipboard = NULL;
	}
	else
	{
		// mapped object created okay
		//
		// map the section and assign the counter block pointer
		// to this section of memory
		gpGalleryClipboard = (LPTGALLERY_CLIPBOARD)
			MapViewOfFile (hMappedObject,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0);
		if (NULL == gpGalleryClipboard)
		{
			TRACE1 ("Failed to Map View of File %x\n", GetLastError());

			// cleanup
			CloseHandle (hMappedObject);

			return FALSE;
		}

		ZeroMemory (gpGalleryClipboard, sizeof(TGALLERY_CLIPBOARD));

		ghIpcGallery = hMappedObject;
	}

	return TRUE;
}

BOOL IPC_Gallery_Shutdown ()
{
	// undo MapViewOfFile
	if (gpGalleryClipboard)
	{
		UnmapViewOfFile (gpGalleryClipboard);
		gpGalleryClipboard = NULL;
	}

	if (ghIpcGallery)
	{
		// undo CreateFileMapping
		CloseHandle (ghIpcGallery);

		ghIpcGallery = NULL;
	}

	// force Gallery to read.  Subsequent failure will clear miniprogess bar

	HWND hWndGallery = GetGalleryWindow ();
	if (hWndGallery)
		::PostMessage (hWndGallery, WM_COMMAND, ID_PING_PROGRESS, 0);

	return TRUE;
}

int  IPC_Gallery_ViewImage (LPCTSTR pchFileName)
{
	if (true)
	{
		CSingleLock sLock(&sGalleryMutex, TRUE);

		LPTSTR pDest;

		// Find an open slot, we have 3 slots

		if (gpGalleryClipboard->rcFileName[0] == NULL)
			pDest = &gpGalleryClipboard->rcFileName[0];

		else if (gpGalleryClipboard->rcFileName2[0] == NULL)
			pDest = &gpGalleryClipboard->rcFileName2[0];

		else if (gpGalleryClipboard->rcFileName3[0] == NULL)
			pDest = &gpGalleryClipboard->rcFileName3[0];
		else
			pDest = NULL;

		if (pDest)
			_tcsncpy (pDest, pchFileName, 1024);
	}

	return 0;
}

int  IPC_Gallery_Progress (HWND hWnd, int iProgress)
{
	if (true)
	{
		CSingleLock sLock(&sGalleryMutex, TRUE);

		gpGalleryClipboard->iProgress = iProgress;
	}

	::PostMessage (hWnd, WM_COMMAND, ID_PING_PROGRESS, 0);

	return 0;
}

int  IPC_Gallery_QueueChange (HWND hWnd, int iWaiting, int iCompleted)
{
	if (true)
	{
		CSingleLock sLock(&sGalleryMutex, TRUE);

		gpGalleryClipboard->iImagesWaiting   = iWaiting;
		gpGalleryClipboard->iImagesCompleted = iCompleted;
	}

	PostMessage (hWnd, WM_COMMAND, ID_PING_COUNTS, 0);

	return 0;
}
