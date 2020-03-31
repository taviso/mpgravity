/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgxcopy.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.6  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.5  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.4  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.3  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.2  2008/09/19 14:51:47  richard_wood
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
#include "rgxcopy.h"       // our header file
#include "regutil.h"       // UtilRegDelKeyTree
#include "resource.h"      // IDS_ERR_BRANCH_REGISTRY
#include "tmsgbx.h"        // NewsMessageBox
#include "fileutil.h"
#include "genutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Private function prototypes
bool FixInstallDir();
bool FixStorageKey(bool &bRootExist);
bool FixServerKeys();
LONG FixAllServerKeys(HKEY hKey);
LONG FixOneServerKey(HKEY hDad, LPTSTR rcKeyName);
LONG ChangePathforValue(HKEY hKey, LPCTSTR valueName);

extern DWORD GetInstalledBuildNumber ();        // from news.cpp

#define MAX_STR   400
const DWORD uMyAccess = (KEY_ALL_ACCESS & ~KEY_CREATE_LINK);



//
// Public functions
//

bool RenameBranchAnawaveToMicroplanet()
{
	LONG lRet;
	HKEY hKeyMP, hKeyMPGrav;
	HKEY hKeyAWGrav;
	DWORD dwDisposition = 0;
	const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;

	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Anawave\\Gravity"),
		NULL, uAccess, &hKeyAWGrav);
	if (ERROR_SUCCESS != lRet)
		// could be a real, virgin MicroPlanet 2.1 install
		return true;
	RegCloseKey(hKeyAWGrav);

	// Test to see if MicroPlanet\Grav is here
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, GetGravityRegKey(),
		NULL, uAccess, &hKeyMPGrav);
	if (ERROR_SUCCESS == lRet)
	{
		RegCloseKey (hKeyMPGrav);

		// look at the version set under the MicroPlanet branch
		DWORD dwBuild = GetInstalledBuildNumber();

		if (dwBuild >= 900)  // 2.1 starts at 900
			return true;       // Conversion is already done

		// else proceed with copy.  We know that Anawave branch is here.
		//   we know that the MicroPlanet branch is old
	}
	else
	{
		// Before we copy Anawave\Gravity\*.* to MicroPlanet\Gravity\*.*
		//   create HKEY_CURRENT_USER\Software\MicroPlanet
		lRet = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\MicroPlanet"),
			NULL, _T("prefdata"), REG_OPTION_NON_VOLATILE, uAccess,
			NULL, &hKeyMP, &dwDisposition);
		if (ERROR_SUCCESS != lRet)
		{
			CString msg; msg.Format(IDS_BRANCH_ERR1, lRet);
			NewsMessageBox(AfxGetMainWnd(), msg, MB_OK | MB_ICONSTOP);
			return false;
		}
		RegCloseKey (hKeyMP);
	}

	// copy  Software\Anawave\Gravity
	//   to  Software\MicroPlanet\Gravity <version number>
	bool bSuccess = CopyRegistryBranch("Software\\Anawave\\Gravity", GetGravityRegKey());
	if (bSuccess)
	{
		// if the copy went ok, destroy the Anawave\Gravity key
		lRet = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Anawave"),
			(DWORD) NULL, uAccess, &hKeyAWGrav);
		if (ERROR_SUCCESS == lRet)
		{
			UtilRegDelKeyTree(hKeyAWGrav, _T("Gravity"));
			RegCloseKey(hKeyAWGrav);
		}
	}
	else
	{
		AfxMessageBox(IDS_ERR_BRANCH_REGISTRY, MB_OK | MB_ICONSTOP);
	}

	return FixDatabaseValues();
}

bool FixDatabaseValues()
{
	TRACE("FixDatabaseValues >\n");
	bool bRV = true;
	bool bPath = false;
	bool bRootExist = false;
	bool bServerKeysFixed = false;

	// Fix InstallDir value
	bPath = FixInstallDir();
	if (!bPath)
	{
		TRACE("FixDatabaseValues : FixInstallDir failed\n");
		ASSERT(FALSE);
	}

	// fix one value under Storage
	bPath &= FixStorageKey(bRootExist);
	if (!bPath)
	{
		TRACE("FixDatabaseValues : FixStorageKey failed\n");
		ASSERT(FALSE);
	}

	if (bPath && bRootExist)
	{
		// fix each server
		bServerKeysFixed = FixServerKeys();
		if (!bServerKeysFixed)
		{
			TRACE("FixDatabaseValues : FixServerKeys failed\n");
			ASSERT(FALSE);
		}
	}

	if (bPath && (bServerKeysFixed || !bRootExist))
		TRACE("FixDatabaseValues : Succeeded <\n");
	else
		TRACE("FixDatabaseValues : Failed <");
	return (bPath && (bServerKeysFixed || !bRootExist));
}



//
// Private functions
//

bool FixInstallDir()
{
	bool bRV = true;
	const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;
	HKEY hKey = 0;

	// First of all get path to our executable file.
	CString strInstallDir = GetStartupDir()+"\\";

	// Open key and overwrite with this value
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, GetGravityRegKey(), 0, uAccess, &hKey))
	{
		// Write this out as "InstallDir"
		DWORD dwcOut = strInstallDir.GetLength()+1;
		BYTE *pByte = (BYTE*)strInstallDir.GetBuffer();
		if (ERROR_SUCCESS != RegSetValueEx(hKey, "InstallDir", 0, REG_SZ, pByte, dwcOut))
			bRV = false;
		RegCloseKey(hKey);
	}
	else
		bRV = false;

	return bRV;
}

// ------------------------------------------------------------------------
// HKEY_CURRENT_USER\Software\MicroPlanet\Gravity\Storage
bool FixStorageKey(bool &bRootExist)
{
	TRACE("FixStorageKey >\n");
	HKEY hKey = 0;
	LONG lRet;

	// First of all, just see if the root key is here
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, GetGravityRegKey(), 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lRet)
	{
		bRootExist = true;
		RegCloseKey(hKey);
	}
	else
	{
		bRootExist = false;
		TRACE("FixStorageKey : Failed : Unable to open key '%s'<\n", GetGravityRegKey());
		ASSERT(FALSE);
		return false;
	}

	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, GetGravityRegKey()+"Storage", 0, uMyAccess, &hKey);
	if (ERROR_SUCCESS != lRet)
	{
		TRACE("FixStorageKey : Failed : Unable to open key '%s'<\n", GetGravityRegKey()+"Storage");
		ASSERT(FALSE);
		return false;
	}

	// pass in open key, change the value
	lRet = ChangePathforValue(hKey, _T("DatabasePath"));
	if (lRet != ERROR_SUCCESS)
		TRACE("FixStorageKey : Warning : Could not update DatabasePath in key '%s'<\n", GetGravityRegKey()+"Storage");
	RegCloseKey(hKey);

	// change the decode dir
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, GetGravityRegKey()+"Dirs", 0, uMyAccess, &hKey);
	if (ERROR_SUCCESS == lRet)
	{
		lRet = ChangePathforValue(hKey, _T("Decode"));
		if (lRet != ERROR_SUCCESS)
			TRACE("FixStorageKey : Warning : Could not update Decode in key '%s'<\n", GetGravityRegKey()+"Dirs");
		RegCloseKey(hKey);
	}
	else
	{
		TRACE("FixStorageKey : Warning : Unable to open key '%s'<\n", GetGravityRegKey()+"Dirs");
		ASSERT(FALSE);
	}

	TRACE("FixStorageKey <\n");
	return true;
}

// ------------------------------------------------------------------------
bool FixServerKeys()
{
	TRACE("FixServerKeys >\n");
	HKEY hKey;
	LONG lRet = RegOpenKeyEx(HKEY_CURRENT_USER, GetGravityRegKey()+"Servers", 0, uMyAccess, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		lRet = FixAllServerKeys(hKey);
		if (lRet != ERROR_SUCCESS)
			TRACE("FixServerKeys : Error FixAllServerKeys failed in key '%s'<\n", GetGravityRegKey()+"Servers");
		RegCloseKey(hKey);
	}
	else
		TRACE("FixServerKeys : Error could not open key '%s'<\n", GetGravityRegKey()+"Servers");

	TRACE("FixServerKeys <\n");
	return (ERROR_SUCCESS == lRet);
}

// ------------------------------------------------------------------------
LONG FixAllServerKeys(HKEY hKey)
{
	DWORD dwcSubKeys, dwcMaxSubKey;
	DWORD dwcValues, dwcMaxValueData;
	TCHAR rcKeyName[100];
	int i;
	DWORD retCode;
	int iSubKeys;

	LONG lRet =
		RegQueryInfoKey(hKey,
		NULL,              // Buffer for class name.
		NULL,              // Length of class string.
		NULL,              // Reserved.
		&dwcSubKeys,       // Number of sub keys.
		&dwcMaxSubKey,     // Longest subkey name length.
		NULL,              // Longest class string.
		&dwcValues,        // Number of values for this key.
		NULL,              // Longest Value name.
		&dwcMaxValueData,  // Longest Value data.
		NULL,              // Security descriptor.
		NULL);             // Last write time.
	if (ERROR_SUCCESS != lRet)
		return lRet;

	iSubKeys = (int) dwcSubKeys;

	// Loop until RegEnumKey fails, get the name of each child
	for (i = 0, retCode = ERROR_SUCCESS;
		ERROR_SUCCESS == retCode && i < iSubKeys;
		++i)
	{
		rcKeyName[0] = '\0';
		retCode = RegEnumKey(hKey, i, rcKeyName, sizeof(rcKeyName));
		if (ERROR_SUCCESS == retCode)
		{
			FixOneServerKey(hKey, rcKeyName);
		}
	}
	return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------
// rcKeyName is probably server1, server2, server4, etc...
LONG FixOneServerKey(HKEY hDad, LPTSTR rcServerN)
{
	HKEY hKey;
	LONG lRet = RegOpenKeyEx(hDad, rcServerN,
		0, uMyAccess, &hKey);
	if (ERROR_SUCCESS == lRet)
	{
		lRet = ChangePathforValue(hKey, _T("DatabasePath2"));
		lRet = ChangePathforValue(hKey, _T("ExportFile"));
		lRet = ChangePathforValue(hKey, _T("ImportFile"));
		RegCloseKey(hKey);
	}

	return lRet;
}

// ------------------------------------------------------------------------
// hKey is opened to some key (either HKCU\Software\MicroPlanet\Gravity\Storage
//   or serverN).
//  Change the indicated value
//
LONG ChangePathforValue(HKEY hKey, LPCTSTR valueName)
{
	TCHAR rcBuf[MAX_PATH] = {0};
	DWORD dwType(0), dwValueSize(0);
	LONG  lRet(0);
	CString strOutput(""), strValue(valueName), strInput, strTmp, strRegKey(valueName);
	if (strRegKey.CompareNoCase("DatabasePath2") == 0)
		strRegKey = "DatabasePath";

	dwType = (DWORD) -1;
	dwValueSize = MAX_PATH;

	lRet = RegQueryValueEx (hKey, strRegKey, 0, &dwType, (BYTE*) rcBuf, &dwValueSize );

	if (ERROR_SUCCESS == lRet)
	{
		strInput = rcBuf;
		strInput.TrimRight('\\');
		strInput.MakeLower();

		if (!strValue.CompareNoCase("DatabasePath"))
		{
			strOutput = TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir();
		}
		else if (!strValue.CompareNoCase("Decode"))
		{
			// Try to guess if user has changed the download folder
			if (strInput.Find("\\gravity\\download") != -1)
				// We think not, so update it
				strOutput = TFileUtil::GetAppData()+"\\Gravity\\"+"download";
		}
		else if (!strValue.CompareNoCase("DatabasePath2"))
		{
			// Try to guess if user has changed the download folder
			if (strInput.Find("\\gravity\\newsdb") != -1)
			{
				// We think not, so update it
				strTmp = strInput.Mid(1+strInput.ReverseFind('\\'));
				strOutput = TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir()+strTmp;
			}
		}
		else if (!strValue.CompareNoCase("ExportFile"))
		{
			// Try to guess if user has changed the download folder
			if (strInput.Find("\\gravity\\newsdb") != -1)
			{
				// We think not, so update it
				strTmp = strInput.Mid(1+strInput.ReverseFind('\\'));
				strOutput = TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir()+strTmp;
			}
		}
		else if (!strValue.CompareNoCase("ImportFile"))
		{
			// Try to guess if user has changed the download folder
			if (strInput.Find("\\gravity\\newsdb") != -1)
			{
				// We think not, so update it
				strTmp = strInput.Mid(1+strInput.ReverseFind('\\'));
				strOutput = TFileUtil::GetAppData()+"\\Gravity\\"+GetGravityNewsDBDir()+strTmp;
			}
		}
		else
			lRet = -1; // error

		if (!strOutput.IsEmpty())
		{
			DWORD dwLen = strOutput.GetLength();
			BYTE *rcOutput = (BYTE*)strOutput.LockBuffer();

			// write it out
			lRet = RegSetValueEx ( hKey, strRegKey,
				0,
				REG_SZ,
				(BYTE*) rcOutput,
				dwLen + 1);

			strOutput.ReleaseBuffer();
		}
	}

	return lRet;
}

// ------------------------------------------------------------------------
// if short dirname change "ANAWAV~1"  to "Gravity"
// if long  dirname change "ANAWAVE GRAVITY" to "Gravity"
//int PatchString (LPTSTR pInput, int iSize, LPTSTR pOutput, int iOutSize)
//{
//	int iRet = 1;
//	LPTSTR pStartAW = 0;
//	LPTSTR pUpperCopy = new TCHAR[iSize];
//	int  iExciseLen = 0;
//	if (0 == pUpperCopy)
//		return iRet;
//
//	_tcscpy ( pUpperCopy, pInput);
//
//	// upper case it, for an easy compare
//	_tcsupr ( pUpperCopy );
//
//	pStartAW = _tcsstr ( pUpperCopy, _T("ANAWAVE GRAVITY") );
//	if (pStartAW)
//	{
//		iExciseLen = 15;
//	}
//	else
//	{
//		CString shortForm; 
//		int n = 0;
//		do 
//		{
//			shortForm.Format (_T("ANAWAV~%d"), ++n);
//			// try short form (NT has short form in the registry?)
//			pStartAW = _tcsstr ( pUpperCopy, LPCTSTR(shortForm));
//		} while ((0==pStartAW) && n < 9);
//		iExciseLen = 8;
//	}
//
//	if (pStartAW)
//	{
//		int cbFirstPart = 0;
//
//		memset (pOutput, _T('\0'), iOutSize);
//
//		cbFirstPart = (int) (pStartAW - pUpperCopy);
//
//		// copy the first part
//		_tcsncpy (pOutput, pInput, cbFirstPart);
//
//		_tcscat  (pOutput, _T("Gravity"));
//
//		// copy the rest from the original string (it's still mixed case)
//		_tcscat  (pOutput, pInput + cbFirstPart + iExciseLen);
//
//		iRet = 0;
//	}
//	else
//	{
//		_tcscpy (pOutput, pInput);
//		iRet = 0;
//	}
//	delete [] pUpperCopy ;
//
//	return iRet;
//}
//
