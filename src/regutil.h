/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: regutil.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.4  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
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

#pragma once

#include <winreg.h>

CString GetGravityRegKey();
CString GetGravityHKCURegKey();
CString GetGravityNewsDBDir();
CString GetOldGravityRegKey();

LONG UtilRegCreateKey(LPCTSTR lpszSubKey, PHKEY phkResult, LPDWORD lpdwDisposition);

////////////////////////////////////////////////
// most common access modes   KEY_ALL_ACCESS
//                            KEY_READ
//                            KEY_WRITE
LONG UtilRegOpenKey(LPCTSTR lpszSubKey, PHKEY phkResult, REGSAM sam = KEY_ALL_ACCESS);

////////////////////////////////////////////////
// Delete a key and all of its subtrees...
BOOL UtilRegDelKeyTree(HKEY  hKey, LPCTSTR  lpSubKey);

bool CopyRegistryBranch(CString strFromBranch, CString strToBranch);

DWORD GetInstalledBuildNumber();
bool GetUACEnabled();

class CStdioFileEx;

class TRegExport
{
public:
	TRegExport(CString strBranch, CString strFilename);

	bool Export();

protected:
	TRegExport() {};
	bool UtilRegExport(CString strKey, HKEY hKey, CStdioFileEx *pfOutput);
	bool UtilRegExportValues(HKEY hKey, CStdioFileEx *pfOutput, DWORD dwcValues, DWORD dwcMaxValueData);
	bool UtilRegExportKeys(CString strKey, HKEY hKey, CStdioFileEx *pfOutput, DWORD dwcSubKeys, DWORD dwcMaxSubKey);

private:
	CString m_strBranch;
	CString m_strFilename;
};

class TRegImport
{
public:
	TRegImport(CString strFilename);

	bool Import();

protected:
	TRegImport() {};
	bool ImportSection(CStdioFileEx *pfInput);
	
	bool CreateKey(HKEY &hKey, CString strKeyName);
	bool CreateValue(HKEY hKey, CString strLine);

	bool CreateString(HKEY hKey, CString strName, CString strValue);
	bool CreateDWORD(HKEY hKey, CString strName, DWORD dwValue);
	bool CreateHex(HKEY hKey, CString strName, BYTE *pData, DWORD pDataSize);

private:
	CString m_strFilename;
};
