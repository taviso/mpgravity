/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsurl.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.4  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.3  2008/09/19 14:51:35  richard_wood
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
#include "SECReg.h"
#include <stdio.h>            // tmpfile
#include "newsurl.h"
#include "regutil.h"          // UtilRegDelKeyTree
#include "server.h"
#include "newsdb.h"
#include "autoptr.h"
#include "atlregkey.h"        // TRegKey

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*************************************
Set HKLM/Software/Clients/News  (default)   Gravity

Friendly name
HKLM/Software/Clients/News/Gravity (default)  MicroPlanet Gravity
{special}                                                              #1

HKLM/Software/Clients/News/Gravity/Protocols/nntp    (tree)            #2
HKLM/Software/Clients/News/Gravity/Protocols/news    (tree)            #3

HKCR/nntp  (tree)                                                      #4
HKCR/news  (tree)                                                      #5

--------------------------------------------------------------------------

(tree) ==      shell/open/command                    <- command-line-to-launch
d:\progra~1\aaaa\gravity.exe /newsurl:%1

DefaultIcon
d:\progra~1\aaaa\gravity.exe,-3

{special} ==     shell/open/command
d:\progra~1\aaaa\gravity.exe /newsurl:%1

**************************/

static int Install_NewsURL_RegistryKeys (void);
static int newsurl_write_branch (const CString& strSubkeyIn,
								 const CString& strExename, const CString& cmd);

static void newsurl_create_opencmd (CString& strExename, CString& cmd);
static int newsurl_write_error (const CString& strBranch);

// ------------------------------------------------------------------------
// return 0 on success
int Install_NewsURL_RegistryKeys (void)
{
	TRegKey regHKCU;
	int ret;

	CString openCmd, exeName, branch;

	newsurl_create_opencmd (exeName, openCmd);

	ret = regHKCU.Create (HKEY_CURRENT_USER, "Software\\Clients\\News", KEY_WRITE);
	if (ERROR_SUCCESS != ret)
	{
		branch = "HKEY_CURRENT_USER\\Software\\Clients\\News";
		return newsurl_write_error (branch);
	}
	else
	{
		ret = regHKCU.SetValue ("", "Gravity");
		regHKCU.Close ();
	}

	branch = "Software\\Clients\\News\\Gravity";

	ret = regHKCU.Create(HKEY_CURRENT_USER, branch, KEY_WRITE);
	if (ERROR_SUCCESS != ret)
		return newsurl_write_error (branch);
	else
	{
		// set friendly name
		regHKCU.SetValue ("", "MicroPlanet Gravity");
		regHKCU.Close ();
	}

	// protocols-news #5

	branch = "Software\\Clients\\News\\Gravity\\Protocols\\news";

	ret = regHKCU.Create(HKEY_CURRENT_USER, branch, KEY_WRITE);
	if (ERROR_SUCCESS != ret)
		return newsurl_write_error (branch);
	else
	{
		newsurl_write_branch (branch, exeName, openCmd);
		regHKCU.Close ();
	}

	// protocols-nntp #4

	branch = "Software\\Clients\\News\\Gravity\\Protocols\\nntp";
	ret = regHKCU.Create(HKEY_CURRENT_USER, branch, KEY_WRITE);
	if (ERROR_SUCCESS != ret)
		return newsurl_write_error (branch);
	else
	{
		newsurl_write_branch (branch, exeName, openCmd);
		regHKCU.Close ();
	}

	// Requirement #1
	branch = "Software\\Clients\\News\\Gravity\\shell\\open\\command";

	ret = regHKCU.Create(HKEY_CURRENT_USER, branch, KEY_WRITE);
	if (ERROR_SUCCESS != ret)
		return newsurl_write_error (branch);
	else
	{
		regHKCU.SetValue ("", openCmd);
		regHKCU.Close ();
	}

	// CLASSES - news #5
	ret = regHKCU.Create (HKEY_CURRENT_USER, "Software\\Classes\\news", KEY_WRITE);
	if (ERROR_SUCCESS != ret)
		return newsurl_write_error ("HKEY_CURRENT_USER\\Software\\Classes\\news");
	else
	{
		regHKCU.SetValue ("URL Protocol", "");
		newsurl_write_branch ("Software\\Classes\\news", exeName, openCmd);
		regHKCU.Close ();
	}

	// CLASSES - nntp  #4
	ret = regHKCU.Create (HKEY_CURRENT_USER, "Software\\Classes\\nntp", KEY_WRITE);
	if (ERROR_SUCCESS != ret)
		return newsurl_write_error ("HKEY_CURRENT_USER\\Software\\Classes\\nntp");
	else
	{
		regHKCU.SetValue ("URL Protocol", "");
		newsurl_write_branch ("Software\\Classes\\nntp", exeName, openCmd);
		regHKCU.Close ();
	}

	return 0;
}

// -------------------------------------------------------------------
static int newsurl_write_branch (const CString& strSubkeyIn,
								 const CString& strExename, const CString& openCmd)
{
	// create two subkeys
	CString strSubkey;

	CString errMsg = "HKEY_CURRENT_USER\\";

	TRegKey reg0;

	strSubkey = strSubkeyIn + "\\shell\\open\\command";

	if (ERROR_SUCCESS == reg0.Create (HKEY_CURRENT_USER,strSubkey, KEY_WRITE))
	{
		reg0.SetValue ("", openCmd);
		reg0.Close ();
	}
	else
		return newsurl_write_error (errMsg + strSubkey);

	strSubkey = strSubkeyIn + "\\DefaultIcon";

	if (ERROR_SUCCESS == reg0.Create (HKEY_CURRENT_USER, strSubkey, KEY_WRITE))
	{
		// set the DefaultIcon
		CString strIcon = strExename;
		strIcon += ",-1";
		reg0.SetValue ("", strIcon);
		reg0.Close ();
	}
	else
		return newsurl_write_error (errMsg + strSubkey);

	// destroy any info under ddeexec, since we don't use it.
	CString clean = strSubkeyIn;
	clean += "\\shell\\open\\command\\ddeexec";

	UtilRegDelKeyTree (HKEY_CURRENT_USER, clean);

	// alternate
	clean = strSubkeyIn;
	clean += "\\shell\\open\\ddeexec";

	UtilRegDelKeyTree (HKEY_CURRENT_USER, clean);

	return 0;
}

// -------------------------------------------------------------------
//  output strExename
//  output cmd
static void newsurl_create_opencmd (CString& strExename, CString& cmd)
{
	// get our pathname
	LPTSTR pExename = strExename.GetBuffer(512);
	GetModuleFileName (NULL, pExename, 512);
	strExename.ReleaseBuffer ();

	// convert to ShortPathname
	TCHAR rcShort[1024];

	GetShortPathName (strExename, rcShort, sizeof rcShort);
	strExename = rcShort;

	// since we have short path form, we dont' need quotes
	cmd = strExename;

	cmd += " /newsurl:%1";

	cmd.MakeLower ();
}

// ------------------------------------------------------------------------
// Test HKLM/Software/Clients/News  (default)   Gravity
//
// I am tigtening this up to account for the AnawaveGravity change
// 
// RLW - Fixed the missing "ERROR_SUCCESS !=" on the Opens.
bool TNewsUrl::IsDefaultReader ()
{
	CString diskOpenCmd(_T("")), strExename, correctOpenCmd;
	TRegKey rg;

	if (	(ERROR_SUCCESS != rg.Open(HKEY_CURRENT_USER, "Software\\Clients\\News", KEY_READ))
			||
			(ERROR_SUCCESS != rg.GetValue("", diskOpenCmd)))
		return false;

	rg.Close();

	// test for basic stuff - shiould point to our Friendly name
	if (diskOpenCmd.CompareNoCase("Gravity"))
		return false;

	// the correct open string is this
	newsurl_create_opencmd(strExename, correctOpenCmd);

	///// Test Requirement #1
	diskOpenCmd.Empty();

	if (	(ERROR_SUCCESS != rg.Open(HKEY_CURRENT_USER, 
				"Software\\Clients\\News\\Gravity\\shell\\open\\command",
				KEY_READ))
			||
			(ERROR_SUCCESS != rg.GetValue("", diskOpenCmd)))
		return false;

	rg.Close();

	if (diskOpenCmd.CompareNoCase(correctOpenCmd))
		return false;

	///// Test Requirement #2
	diskOpenCmd.Empty();

	if (	(ERROR_SUCCESS != rg.Open(HKEY_CURRENT_USER,
				"Software\\Clients\\News\\Gravity\\Protocols\\nntp\\shell\\open\\command",
				KEY_READ))
			||
			(ERROR_SUCCESS != rg.GetValue("", diskOpenCmd)))
		return false;
	
	rg.Close();

	if (diskOpenCmd.CompareNoCase(correctOpenCmd))
		return false;

	///// Test Requirement #3
	diskOpenCmd.Empty();

	if (	(ERROR_SUCCESS != rg.Open(HKEY_CURRENT_USER,
				"Software\\Clients\\News\\Gravity\\Protocols\\news\\shell\\open\\command",
				KEY_READ))
			||
			(ERROR_SUCCESS != rg.GetValue("", diskOpenCmd)))
		return false;
	
	rg.Close();

	if (diskOpenCmd.CompareNoCase(correctOpenCmd))
		return false;

	///// Test Requirement #4
	diskOpenCmd.Empty();

	if (	(ERROR_SUCCESS != rg.Open(HKEY_CURRENT_USER, "Software\\Classes\\nntp\\shell\\open\\command", KEY_READ))
			||
			(ERROR_SUCCESS != rg.GetValue("", diskOpenCmd)))
		return false;
	
	rg.Close();

	if (diskOpenCmd.CompareNoCase(correctOpenCmd))
		return false;

	///// Test Requirement #5
	diskOpenCmd.Empty();

	if (	(ERROR_SUCCESS != rg.Open(HKEY_CURRENT_USER, "Software\\Classes\\news\\shell\\open\\command", KEY_READ))
			||
			(ERROR_SUCCESS != rg.GetValue("", diskOpenCmd)))
		return false;
	
	rg.Close();

	if (diskOpenCmd.CompareNoCase(correctOpenCmd))
		return false;

	// we passed all 5 tests
	return true;
}

// ------------------------------------------------------------------------
//
int TNewsUrl::SetDefaultReader (BOOL fOn /* = TRUE */)
{
	if (fOn)
		return Install_NewsURL_RegistryKeys ();
	else
	{
		// explicitly turning us off.
		TRegKey rgHKCU;

		// this is the key that MS internet-explorer cares about
		int ret = rgHKCU.Open (HKEY_CURRENT_USER, "Software\\Clients\\News", KEY_WRITE);
		if (!ret)
			return 1;  // can't proceed

		// set the <default> value to empty
		rgHKCU.DeleteValue ("");

		return 0;
	}
}

// ------------------------------------------------------------------------
static int newsurl_write_error (const CString& strBranch)
{
	CString errMsg;
	errMsg.Format (IDS_ERR_WRITEREG1, LPCTSTR(strBranch));
	AfxMessageBox (errMsg, MB_OK | MB_ICONWARNING);
	return 1;
}

