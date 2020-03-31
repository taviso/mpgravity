/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: licutil.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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
/*  Revision 1.4  2009/03/18 15:08:07  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.3  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.2  2008/09/19 14:51:29  richard_wood
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
#include "tglobopt.h"
#include "licutil.h"
#include "mpserial.h"
#include "globals.h"
#include "netcfg.h"
#include "server.h"        // TNewsServer
#include "servcp.h"        // TServerCountedPtr
#include "newsdb.h"        // TNewsDB

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

/////////////////////////////////////////////////////////////////////
// TLicenseSystem ctor - Sets up the strings that splash screen
//                       boy uses...
/////////////////////////////////////////////////////////////////////

//
// PLEASE NOTE from version 2.8 the third sub number has been removed.
//
// So its now 2.8.<build number>
//
void TLicenseSystem::GetVersionInt(VERINFO &verInfo)
{
	char  fileName[256];
	DWORD dreck;
	GetModuleFileName (NULL, fileName, 255);
	int size = GetFileVersionInfoSize (fileName, &dreck);

	if (size)
	{
		LPVOID   pInfo = (LPVOID) new char[size];
		LPVOID   pValue = NULL;
		UINT     valueLen = 0;
		GetFileVersionInfo (fileName, 0, size, pInfo);

		// get fixed version info
		VerQueryValue (pInfo, TEXT("\\"), &pValue, &valueLen);

		VS_FIXEDFILEINFO *pFixed = (VS_FIXEDFILEINFO *) pValue;

		verInfo.m_nMajor = HIWORD(pFixed->dwProductVersionMS);
		verInfo.m_nMinor = LOWORD(pFixed->dwProductVersionMS);
		verInfo.m_nBuild = HIWORD(pFixed->dwProductVersionLS);
		//*version = HIWORD(pFixed->dwProductVersionMS) * 100 +
		//	LOWORD(pFixed->dwProductVersionMS) * 10 +
		//	HIWORD(pFixed->dwProductVersionLS);

//		if (beta)
//		{
			if (pFixed->dwFileFlags & VS_FF_PRERELEASE)
				verInfo.m_bBeta = true;
				//*beta = TRUE;
			else
				verInfo.m_bBeta = false;
				//*beta = FALSE;
//		}

		delete [] pInfo;
	}
}

///////////////////////////////////////////////////////////////////////////
//  Get description for the (X-Newsreader | X-Mailer) header line
//
CString TLicenseSystem::GetUserAgentDescription()
{
	CString usragent;
	VERINFO verInfo;

	GetVersionInt(verInfo);

	usragent.Format ("MicroPlanet-Gravity/%d.%d.%d",
		verInfo.m_nMajor, verInfo.m_nMinor, verInfo.m_nBuild);

	return usragent;
}

CString TLicenseSystem::FormatVersionString()
{
	CString verString;
	VERINFO verInfo;

	GetVersionInt(verInfo);

	verString.Format ("Version %d.%d.%d %s%s",
		verInfo.m_nMajor, verInfo.m_nMinor, verInfo.m_nBuild,
		verInfo.m_bBeta ? "Beta ":"", GRAVITY_VERSION);

	return verString;
}
