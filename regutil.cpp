/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: regutil.cpp,v $
/*  Revision 1.2  2010/07/24 21:57:03  richard_wood
/*  Bug fixes for Win7 executing file ops out of order.
/*  V3.0.1 RC2
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.10  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.9  2009/08/27 15:29:22  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.8  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.7  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
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
/*  Revision 1.4  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.3  2009/06/14 13:17:22  richard_wood
/*  Added side by side installation of Gravity.
/*  Adding (WORK IN PORGRESS!!!) DB export/import.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
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
/*  Revision 1.2  2008/09/19 14:51:43  richard_wood
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
#include "regutil.h"
#include "genutil.h"
#include "StdioFileEx.h"
#include "licutil.h"
#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

int RegCopyBranch(HKEY hSrcDad, HKEY hDstDad, LPCTSTR pszSubkey);
int UtilRegXCopyKeys(HKEY hSrc, HKEY  hDst, DWORD dwcSubKeys, DWORD dwcMaxSubKey);
int UtilRegXCopyValues(HKEY hSrc, HKEY hDst, DWORD dwcValues, DWORD dwcMaxValueData);
int UtilRegXCopy(HKEY hSrc, HKEY hDst);

static const DWORD MAX_VALUE_NAME = 256;

static CString gStrNewsDBDir(_T(""));
static CString gStrRegGravityKey(_T(""));
static CString gStrRegGravityHKCUKey(_T(""));
static bool gbRegKeysSet = false;

void UtilSetRegKeys()
{
	gbRegKeysSet = true;

	VERINFO verInfo;
	TLicenseSystem::GetVersionInt(verInfo);

	gStrRegGravityKey.Format("Software\\Microplanet\\Gravity %d.%d\\",
		verInfo.m_nMajor, verInfo.m_nMinor);
	gStrRegGravityHKCUKey.Format("HKEY_CURRENT_USER\\Software\\Microplanet\\Gravity %d.%d\\",
		verInfo.m_nMajor, verInfo.m_nMinor);
	gStrNewsDBDir.Format("newsdb%d.%d\\", verInfo.m_nMajor, verInfo.m_nMinor);
}

CString GetGravityRegKey()
{
	if (!gbRegKeysSet) UtilSetRegKeys();

	return gStrRegGravityKey;
}

CString GetGravityHKCURegKey()
{
	if (!gbRegKeysSet) UtilSetRegKeys();

	return gStrRegGravityHKCUKey;
}

CString GetGravityNewsDBDir()
{
	if (!gbRegKeysSet) UtilSetRegKeys();

	return gStrNewsDBDir;
}

CString GetOldGravityRegKey()
{
	return "Software\\Microplanet\\Gravity\\";
}


// ------------------------------------------------------------------------
// GetInstalledBuildNumber - Gets the build number of the
// version in the registry.
//   works with TRegSystem::UpdateBuildNumber
DWORD GetInstalledBuildNumber()
{
	HKEY     regKey;
	DWORD    type;
	DWORD    version;
	DWORD    len;

	if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_CURRENT_USER,
		GetGravityRegKey()+"System",
		DWORD(0),
		KEY_READ,
		&regKey))
		return 100;

	len = sizeof (version);
	type = REG_DWORD;

	if (ERROR_SUCCESS != RegQueryValueEx (regKey,
		"Version",
		LPDWORD (0),
		&type,
		(LPBYTE) &version,
		&len))
	{
		RegCloseKey (regKey);
		return 100;
	}

	RegCloseKey (regKey);
	return version;
}

bool GetUACEnabled()
{
	int nOS = GetOS();
	if (nOS != RUNNING_ON_VISTA && nOS != RUNNING_ON_WIN7)
		return false;

	HKEY     regKey;
	DWORD    type;
	DWORD    version;
	DWORD    len;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
		0, KEY_READ, &regKey))
		return false;

	len = sizeof (version);
	type = REG_DWORD;

	if (ERROR_SUCCESS != RegQueryValueEx (regKey, "EnableLUA",
		0, &type, (LPBYTE) &version, &len))
	{
		RegCloseKey (regKey);
		return false;
	}

	RegCloseKey (regKey);
	return (version != 0);
}


/////////////////////////////////////////////////////////////////////////////
// UtilRegCreateKey - Create a registry key under HKEY_CURRENT_USER.
/////////////////////////////////////////////////////////////////////////////
LONG UtilRegCreateKey(LPCTSTR lpszSubKey, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	return RegCreateKeyEx (
		HKEY_CURRENT_USER,      // handle of open key
		lpszSubKey,             // subkey
		0,                      // reserved
		_T("prefdata"),             // string specifies object type
		REG_OPTION_NON_VOLATILE,//  makes sense!
		KEY_ALL_ACCESS,         // access mode
		NULL,                   // security mode
		phkResult,              // receives open key
		lpdwDisposition );      // action result
}

/////////////////////////////////////////////////////////////////////////////
// UtilRegCreateKey - Open a registry key under HKEY_CURRENT_USER.
/////////////////////////////////////////////////////////////////////////////
LONG UtilRegOpenKey(LPCTSTR lpszSubKey, PHKEY phkResult, REGSAM sam)
{
	return RegOpenKeyEx (
		HKEY_CURRENT_USER,      // handle of open key
		lpszSubKey,             // subkey
		0,                      // reserved
		sam,                    // security access mask
		phkResult);             // receives open key
}

/////////////////////////////////////////////////////////////////////////////
// UtilRegDelKeyTree - Delete a key and all of it's subkeys and values.
/////////////////////////////////////////////////////////////////////////////
BOOL UtilRegDelKeyTree(HKEY hKey, LPCTSTR lpSubKey)
{
	LONG  rc;
	int   i = 0;
	DWORD len, classlen;
	TCHAR name[512];
	TCHAR rcClass[512];
	HKEY  ourKey;
	FILETIME ft;
	DWORD subkeyCount, maxSubKeyLen, maxClassLen;
	DWORD maxValueNameLen, nValues, maxValueLen, szSecurityDescriptor;

	// open the current key we're trying to kill
	rc = RegOpenKeyEx (hKey,
		lpSubKey,
		(DWORD) 0,
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
		&ourKey);

	if (ERROR_SUCCESS != rc)
		return FALSE;

	// figure out how many subkeys we have
	classlen = sizeof(rcClass);
	rc = RegQueryInfoKey (ourKey,
		rcClass,           // receives class name
		&classlen,         // size of class buffer,
		NULL,              // reserved,
		&subkeyCount,      // COUNT of SUBKEYS
		&maxSubKeyLen,     // longest subkey name length
		&maxClassLen,      // longest class string length
		&nValues,          // number of value entries
		&maxValueNameLen,
		&maxValueLen,      // longest value data length
		&szSecurityDescriptor,
		&ft);

	if (ERROR_SUCCESS != rc)
		return FALSE;

	// enumerate backwards through the subkeys, kill them all!
	for (i = (int) subkeyCount - 1; i >= 0; --i)
	{
		// reset len
		name[0] = 0;
		len = sizeof (name);

		rc = RegEnumKeyEx (ourKey,             // open key
			(DWORD) i,    // index of entry
			name,         // buffer for subkey
			&len,         // length of subkey
			NULL,         // reserved, must be null
			NULL,         // buffer for class name
			NULL,         // size of class name buffer
			&ft);         // returned file time
		if (ERROR_SUCCESS == rc || ERROR_MORE_DATA == rc)
		{
			if (!UtilRegDelKeyTree (ourKey, name))
				return FALSE;
		}
	}

	// close the key and kill it...
	RegCloseKey (ourKey);

	rc = RegDeleteKey (hKey, lpSubKey);
	if (ERROR_SUCCESS != rc)
		return FALSE;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//  Stuff to copy a branch
/////////////////////////////////////////////////////////////////////////////

//
// E.g
// strFromBranch = "Software\\Microplanet\\Gravity"
// strToBranch   = "Software\\Microplanet\\Gravity2.9"
//
bool CopyRegistryBranch(CString strFromBranch, CString strToBranch)
{
	bool bRV = true;
	HKEY hKeyTo, hKeyFrom;
	DWORD dwDisposition = 0;
	const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;

	// If nothing to do, do nothing!
	if (strFromBranch.CompareNoCase(strToBranch) == 0)
		return true;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, strFromBranch, 0, uAccess, &hKeyFrom))
		return false;

	// Test to see if MicroPlanet\Grav is here
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, strToBranch, 0, uAccess, &hKeyTo))
	{
		RegCloseKey(hKeyFrom);
		return false;
	}

	if (0 != UtilRegXCopy(hKeyFrom, hKeyTo))
		bRV = false;

	RegCloseKey(hKeyFrom);
	RegCloseKey(hKeyTo);

	return bRV;
}

//
// This function copies a sub branch between two parents.
//
// hSrc, hDst - opened keys
// pszSubkey  - a subkey (segment) that you want copied
//
int RegCopyBranch(HKEY hSrcDad, HKEY hDstDad, LPCTSTR pszSubkey)
{
	HKEY  hSrc, hDst;
	DWORD dwDisposition;
	LONG  lRet;

	// open child-1
	lRet = RegOpenKeyEx (hSrcDad, pszSubkey, 0, KEY_ALL_ACCESS, &hSrc);
	if (ERROR_SUCCESS != lRet)
		return 1;

	// create child-2
	lRet = RegCreateKeyEx (hDstDad, pszSubkey, 0, _T("prefdata"),
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hDst,
		&dwDisposition);
	if (ERROR_SUCCESS != lRet)
	{
		RegCloseKey (hSrc);
		return 1;
	}

	int stat = UtilRegXCopy(hSrc, hDst);

	RegCloseKey(hDst);
	RegCloseKey(hSrc);

	return stat;
}

// ------------------------------------------------------------------------
//
int UtilRegXCopy (HKEY hSrc, HKEY hDst)
{
	DWORD dwcSubKeys, dwcMaxSubKey;
	DWORD dwcValues, dwcMaxValueData;

	LONG lRet =
		RegQueryInfoKey (hSrc,
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
		return 1;

	int stat = 0;
	// Enumerate and copy the Values
	if (dwcValues)
		stat = UtilRegXCopyValues(hSrc, hDst, dwcValues, dwcMaxValueData);

	if (0 == stat && dwcSubKeys > 0)
		UtilRegXCopyKeys(hSrc, hDst, dwcSubKeys, dwcMaxSubKey);

	return stat;
}

// ------------------------------------------------------------------------
//
int UtilRegXCopyValues (HKEY hSrc, HKEY hDst, DWORD dwcValues,
						DWORD dwcMaxValueData)
{
	int j;
	DWORD retValue;
	TCHAR rcValueName[MAX_VALUE_NAME];
	DWORD dwcValueName;

	LPTSTR rbData = new TCHAR[dwcMaxValueData + 1];

	for (j = 0, retValue = ERROR_SUCCESS; j < (int)dwcValues; ++j)
	{
		DWORD dwType;
		dwcValueName = MAX_VALUE_NAME;
		rcValueName[0] = '\0';

		DWORD dwcData = dwcMaxValueData;
		rbData[0] = '\0';

		retValue = RegEnumValue (hSrc, j,
			rcValueName,
			&dwcValueName,
			NULL,
			&dwType,
			(PBYTE)rbData,
			&dwcData);

		if (ERROR_SUCCESS == retValue)
		{
			// write this out
			DWORD dwcOut = dwcData;

			RegSetValueEx (hDst, rcValueName, (DWORD) 0, dwType,
				(PBYTE)rbData, dwcOut);
		}
	}

	delete [] rbData;

	return 0;
}

// ------------------------------------------------------------------------
// Get the name of each child key and recurse
int  UtilRegXCopyKeys (HKEY hSrc, HKEY  hDst, DWORD dwcSubKeys,
					   DWORD dwcMaxSubKey)
{
	// we need N+1, but add one more for luck
	dwcMaxSubKey += 2;

	LPBYTE rbChildName = new BYTE[dwcMaxSubKey];
	int i, ret = 0;
	DWORD retCode;

	// Loop until RegEnumKey fails, get the name of each child

	for (i = 0, retCode = ERROR_SUCCESS;
		ERROR_SUCCESS == retCode && i < (int)dwcSubKeys;
		++i)
	{
		rbChildName[0] = '\0';
		retCode = RegEnumKey (hSrc, i, (LPTSTR)rbChildName, dwcMaxSubKey);
		if (ERROR_SUCCESS == retCode)
		{
			ret = RegCopyBranch (hSrc, hDst, (LPTSTR)rbChildName);
			if (ret)
				break;
		}
	}

	delete [] rbChildName;
	return ret;
}

//
// Export a branch to a file
//
// ONLY works in HKEY_CURRENT_USER
//
// If strFilename exists it will be overwritten (if we can, if not, error)
//
TRegExport::TRegExport(CString strBranch, CString strFilename)
{
	m_strBranch = strBranch;
	m_strFilename = strFilename;

	m_strBranch.TrimRight('\\');
}

bool TRegExport::Export()
{
	bool bRV = true;
	HKEY hKey;
	DWORD dwDisposition = 0;
	const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;

	if (m_strBranch.IsEmpty() || m_strFilename.IsEmpty())
		return false;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, m_strBranch, 0, uAccess, &hKey))
		return false;

	CStdioFileEx fOutput;

	if (!fOutput.Open(m_strFilename, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeText))
		return false;

	// Write the header lines to the reg file
	fOutput.WriteString("Windows Registry Editor Version 5.00\r\n\r\n");

	if (!UtilRegExport(m_strBranch+"\\", hKey, &fOutput))
		bRV = false;

	RegCloseKey(hKey);
	fOutput.Close();

	return bRV;
}

//
// Start of the recursive loop.
// hKey and pfOutput are open and ready.
//
bool TRegExport::UtilRegExport(CString strKey, HKEY hKey, CStdioFileEx *pfOutput)
{
	bool bRV = true;

	// Iterate through the sub keys in hKey
	DWORD dwcSubKeys, dwcMaxSubKey;
	DWORD dwcValues, dwcMaxValueData;

	LONG lRet =
		RegQueryInfoKey (hKey,
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
		return false;

	// Write out this keys name
	pfOutput->WriteString("[HKEY_CURRENT_USER\\"+strKey+"]\r\n");

	// Enumerate and copy the Values
	if (dwcValues)
		bRV = UtilRegExportValues(hKey, pfOutput, dwcValues, dwcMaxValueData);

	pfOutput->WriteString("\r\n");

	if (bRV && dwcSubKeys)
		bRV = UtilRegExportKeys(strKey, hKey, pfOutput, dwcSubKeys, dwcMaxSubKey);

	return bRV;
}

bool TRegExport::UtilRegExportValues(HKEY hKey, CStdioFileEx *pfOutput, DWORD dwcValues, DWORD dwcMaxValueData)
{
	bool bRV = true;

	int i = 0, j, k;
	DWORD retValue = ERROR_SUCCESS, dwType, dwcData;
	TCHAR rcValueName[MAX_VALUE_NAME];
	DWORD dwcValueName;
	CString strTmp, strTmp2;
	LPTSTR rbData = new TCHAR[dwcMaxValueData + 1];

	for (; i < (int)dwcValues; ++i)
	{
		dwcValueName = MAX_VALUE_NAME;
		rcValueName[0] = '\0';
		dwcData = dwcMaxValueData;
		rbData[0] = '\0';

		if (ERROR_SUCCESS == RegEnumValue(hKey, i, rcValueName, &dwcValueName,
			NULL, &dwType, (PBYTE)rbData, &dwcData))
		{
			// write this out
			switch(dwType)
			{
			case REG_BINARY:
				strTmp.Format("\"%s\"=hex:", rcValueName); // hex:....
				pfOutput->WriteString(strTmp);
				// Binary data in hex form, so take each byte of data, convert
				// to hex string and write out.
				j = strTmp.GetLength(); // Count of chars written so far - line wrap at 80 or less.
				for (k = 0; k < (int)dwcData; k++)
				{
					strTmp.Format("%02x", (BYTE)rbData[k]);
					j += 2;
					if (k < ((int)dwcData)-1)
					{
						strTmp += ",";
						j++;
					}
					if (j > 78)
					{
						pfOutput->WriteString("\\\r\n  ");
						pfOutput->WriteString(strTmp);
						j = 2+strTmp.GetLength();
					}
					else
						pfOutput->WriteString(strTmp);
				}
				pfOutput->WriteString("\r\n");
				break;
			case REG_DWORD:
				// dword:...
				strTmp.Format("\"%s\"=dword:%08x\r\n", rcValueName, (*(DWORD*)rbData));
				pfOutput->WriteString(strTmp);
				break;
			case REG_SZ:
				// "string"
				strTmp2 = rbData;
				strTmp2.Replace("\\", "\\\\"); // Change escape slashes to double escape slashes
				strTmp2.Replace("\"", "\\\""); // Escape double quote chars
				strTmp.Format("\"%s\"=\"%s\"\r\n", rcValueName, strTmp2);
				pfOutput->WriteString(strTmp);
				break;
//			case REG_MULTI_SZ:
//				break;
//			case REG_EXPAND_SZ:
//				break;
			default:
				ASSERT(FALSE);
				break;
			}
		}
		else
			bRV = false;
	}

	delete [] rbData;

	return bRV;
}

bool TRegExport::UtilRegExportKeys(CString strKey, HKEY hKey, CStdioFileEx *pfOutput, DWORD dwcSubKeys, DWORD dwcMaxSubKey)
{
	bool bRV = true;

	// we need N+1, but add one more for luck
	dwcMaxSubKey += 2;

	LPBYTE rbChildName = new BYTE[dwcMaxSubKey];
	int i = 0;
	CString strSubKey;

	// Loop until RegEnumKey fails, get the name of each child
	for (; i < (int)dwcSubKeys; ++i)
	{
		rbChildName[0] = '\0';
		if (ERROR_SUCCESS == RegEnumKey(hKey, i, (LPTSTR)rbChildName, dwcMaxSubKey))
		{
			strSubKey = strKey+"\\"+CString(rbChildName);

			HKEY hSubKey;
			const DWORD uAccess = KEY_ALL_ACCESS & ~KEY_CREATE_LINK;

			if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, strSubKey, 0, uAccess, &hSubKey))
				bRV = false;
			else if (!UtilRegExport(strSubKey, hSubKey, pfOutput))
				bRV = false;
		}
		else
			bRV = false;
	}

	delete [] rbChildName;

	return bRV;
}



TRegImport::TRegImport(CString strFilename)
{
	m_strFilename = strFilename;
}

bool TRegImport::Import()
{
	TRACE("TRegImport::Import '%s' >\n", m_strFilename);
	if (m_strFilename.IsEmpty())
	{
		TRACE("TRegImport::Import : Failed : No filename specified <\n");
		ASSERT(FALSE);
		return false;
	}
	if (!WaitForFile(m_strFilename))
	{
		TRACE("TRegImport::Import : Failed : File not found <\n");
		ASSERT(FALSE);
		return false;
	}

	CStdioFileEx fInput;
	if (!fInput.Open(m_strFilename, CFile::modeRead|CFile::shareExclusive|CFile::typeText))
	{
		TRACE("TRegImport::Import : Failed unable to open file <\n");
		ASSERT(FALSE);
		return false;
	}

	// Read the first line in and verify it is correct
	CString strLine;
	if (fInput.ReadString(strLine))
	{
		if (strLine.Compare("Windows Registry Editor Version 5.00") != 0)
		{
			fInput.Close();
			TRACE("TRegImport::Import : Failed : File is not a valid registry file '%s' <\n", strLine);
			ASSERT(FALSE);
			return false;
		}
	}
	else
	{
		fInput.Close();
		TRACE("TRegImport::Import : Failed : File is empty <\n");
		ASSERT(FALSE);
		return false; // Empty file!
	}

	// Call the recursive func to import the reg file.
	bool bRV = ImportSection(&fInput);

	fInput.Close();

	TRACE("TRegImport::Import <\n");
	return bRV;
}

bool TRegImport::ImportSection(CStdioFileEx *pfInput)
{
	TRACE("TRegImport::ImportSection >\n");
	CString strLine(""), strTmp;

	// Skip over blank lines and get first non-blank line
	while (strLine.IsEmpty())
	{
		if (!pfInput->ReadString(strLine))
		{
			TRACE("TRegImport::ImportSection (EOF) <\n");
			return true; // EOF
		}
	}

	// Check for correct start of section
	if (strLine[0] != '[')
	{
		TRACE("TRegImport::ImportSection : Failed : First non empty line is not a start of key <\n");
		return false;
	}
	else
		TRACE("TRegImport::ImportSection : Creating key '%s'\n", strLine);

	HKEY hKey = NULL;
	if (!CreateKey(hKey, strLine))
	{
		TRACE("TRegImport::ImportSection : Failed trying to create key '%s' <\n", strLine);
		return false;
	}

	if (!pfInput->ReadString(strLine))
	{
		TRACE("TRegImport::ImportSection : EOF <\n");
		return true; // EOF
	}

	while (!strLine.IsEmpty())
	{
		// For versions between 2.9.0 and 2.9.14, fix the CustomHeaders line
		//{
		//	strTmp = strLine;
		//	strTmp.MakeLower();
		//	if (strTmp.Find("customheaders") != -1)
		//	{
		//		while (strLine.Right(1) != "\"")
		//		{
		//			if (!pfInput->ReadString(strTmp))
		//			{
		//				// Odd, line says it continues but end of file found.
		//				// Try to create it anyway
		//				bool bRV = CreateValue(hKey, strLine);
		//				if (bRV)
		//					TRACE("TRegImport::ImportSection : Succeeded CreateValue '%s' <\n", strLine);
		//				else
		//					TRACE("TRegImport::ImportSection : Failed trying to CreateValue '%s' <\n", strLine);
		//				return bRV;
		//			}
		//			strLine += "\r\n" + strTmp;
		//		}
		//		// We have now read the whole line and can create the key properly
		//	}
		//}
		// Is this line escaped at the end (i.e. it continues over more than 1 line)?
		while (strLine.Right(1) == '\\')
		{
			strLine = strLine.Left(strLine.GetLength()-1);
			if (!pfInput->ReadString(strTmp))
			{
				// Odd, line says it continues but end of file found.
				// Try to create it anyway
				bool bRV = CreateValue(hKey, strLine);
				if (bRV)
					TRACE("TRegImport::ImportSection : Succeeded CreateValue '%s' <\n", strLine);
				else
					TRACE("TRegImport::ImportSection : Failed trying to CreateValue '%s' <\n", strLine);
				return bRV;
			}
			strLine += strTmp;
		}

		if (!CreateValue(hKey, strLine))
		{
			TRACE("TRegImport::ImportSection : Failed trying to CreateValue '%s' <\n", strLine);
			return false;
		}
		else
			TRACE("TRegImport::ImportSection : Creating value '%s'\n", strLine);

		if (!pfInput->ReadString(strLine))
		{
			TRACE("TRegImport::ImportSection : EOF <\n");
			return true; // EOF
		}
	}

	bool bRV = ImportSection(pfInput);
	if (bRV)
		TRACE("TRegImport::ImportSection : Succeeded <\n");
	else
		TRACE("TRegImport::ImportSection : Failed <\n");
	return bRV;
}

bool TRegImport::CreateKey(HKEY &hKey, CString strKeyName)
{
	DWORD disposition;
	strKeyName.MakeLower();
	strKeyName.TrimRight("]");
	strKeyName.TrimLeft("[");
	strKeyName.TrimLeft("hkey_current_user");
	strKeyName.TrimLeft("\\");
	return (ERROR_SUCCESS == UtilRegCreateKey(strKeyName, &hKey, &disposition) ? true : false);
}

bool TRegImport::CreateValue(HKEY hKey, CString strLine)
{
	// Parse the line and see what type it is
	DWORD dwType = 0;
	CString strName, strRHS, strTmp;

	if (strLine.Find('=') != -1)
	{
		strName = strLine.Left(strLine.Find('='));
		strName.Trim('"');
		strRHS = strLine.Mid(strLine.Find('=')+1);
		if (strRHS[0] == '"')
		{
			// String
			strRHS.Trim('"');
			strRHS.Replace("\\\\", "\\"); // Change double escape slashes back to single escape slashes
			strRHS.Replace("\\\"", "\""); // Un-escape double quote chars
			return CreateString(hKey, strName, strRHS);
		}
		else
		{
			if (strRHS.Left(3).CompareNoCase("hex") == 0)
			{
				// A binary string
				if (strRHS.Find(':') != -1)
				{
					strRHS = strRHS.Mid(strRHS.Find(':')+1);
					DWORD dwLength = 0, dwValue;
					BYTE *pData = NULL;
					int i = 0, j = 0;

					// The string we have is complete

					// Length is comma count
					dwLength = strRHS.Replace(',', ';')+1;
					// Allocate a byte array of the correct size
					pData = new BYTE[dwLength+1];
					
					// Convert the string into the byte array
					strTmp = strRHS.Tokenize(";", i);
					while(!strTmp.IsEmpty() && (j < (int)dwLength))
					{
						if (sscanf(strTmp, "%x", &dwValue))
						{
							pData[j++] = (BYTE)dwValue;
						}
						strTmp = strRHS.Tokenize(";", i);
					}

					// Write out to the registry
					bool bRV = CreateHex(hKey, strName, pData, dwLength);

					delete [] pData;
					pData = NULL;

					return bRV;
				}
			}
			else
			{
				// A DWORD

				DWORD dwValue = 0;
				// Convert 8 char hexadecimal string to number
				if (strRHS.Find(':') != -1)
				{
					strRHS = strRHS.Mid(strRHS.Find(':')+1);
					if (sscanf(strRHS, "%x", &dwValue))
						return CreateDWORD(hKey, strName, dwValue);
				}
			}
		}
	}

	return false;
}

bool TRegImport::CreateString(HKEY hKey, CString strName, CString strValue)
{
	return (ERROR_SUCCESS == RegSetValueEx(hKey, strName, 0, REG_SZ,
		(BYTE *)(LPCTSTR) strValue, strValue.GetLength()+1) ? true : false);
}

bool TRegImport::CreateDWORD(HKEY hKey, CString strName, DWORD dwValue)
{
	return (ERROR_SUCCESS == RegSetValueEx(hKey, strName, 0, REG_DWORD,
		(BYTE *) &dwValue, sizeof(DWORD)) ? true : false);
}

bool TRegImport::CreateHex(HKEY hKey, CString strName, BYTE *pData, DWORD pDataSize)
{
	return (ERROR_SUCCESS == RegSetValueEx(hKey, strName, 0, REG_BINARY,
		pData, pDataSize) ? true : false);
}
