/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgbase.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
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
#include "mplib.h"
#include "rgbase.h"
#include "regutil.h"
#include <stdio.h>  // for sscanf
#include "resource.h"
#include "superstl.h"            // istrstream, ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// disable warning about CException has no copy-ctor. (Must catch ptr or reference)
#pragma warning( disable : 4671 4673 )

int TRegistryBase::Load(LPCTSTR path)
{
	DWORD dwAction;
	LONG lRet = UtilRegCreateKey(
		path,
		&m_hkKey,
		&dwAction);
	if (ERROR_SUCCESS != lRet)
	{
		throw(new TException(IDS_REGISTRY_ERROR, kError));
		return 0;
	}
	if (REG_CREATED_NEW_KEY == dwAction)
		// let derived class do it's thing
		default_values();
	else
		// let derived class do it's thing
		read();

	RegCloseKey ( m_hkKey );
	return 0;
}

int TRegistryBase::Save(LPCTSTR path)
{
	// TViewFilter needs Query access; everyone needs WRITE access

	LONG lRet = UtilRegOpenKey(
		path,
		&m_hkKey, KEY_ALL_ACCESS);
	if (ERROR_SUCCESS != lRet)
	{
		DWORD disposition;
		lRet = UtilRegCreateKey(
			path,
			&m_hkKey, &disposition);
		if (ERROR_SUCCESS != lRet)
		{
			throw(new TException(IDS_REGISTRY_ERROR, kError));
			return 0;
		}
	}

	// let derived class do it's thing
	write();

	RegCloseKey ( m_hkKey );
	return 0;
}

// ------------------------------------------------------------------------
// preload some data from the Install Script
int TRegistryBase::PreLoad(LPCTSTR path)
{
	bool fOpened = false;
	LONG lRet = UtilRegOpenKey(
		path,
		&m_hkKey, KEY_READ);
	if (ERROR_SUCCESS == lRet)
		fOpened = true;
	else
	{
		DWORD disposition;
		lRet = UtilRegCreateKey(
			path,
			&m_hkKey, &disposition);
		if (ERROR_SUCCESS == lRet)
			fOpened = true;
		else
		{
			CString msg;  msg.Format(IDS_REGISTRY_OPEN, path);

			AfxMessageBox (msg, MB_OK | MB_ICONWARNING);
			return 0;
		}
	}

	if (fOpened)
	{
		// let derived class do it's thing
		preload();

		RegCloseKey ( m_hkKey );
	}
	return 0;
}

void TRegistryBase::rect_to_string(RECT& rct, CString& out)
{
	// should we write out width ? or Left,Right?
	// use whitespace as delimiter, to make it easier to read back in
	out.Format("%d %d %d %d", rct.left, rct.top, rct.right, rct.bottom);
}

void TRegistryBase::str_to_rect(LPTSTR pStr, RECT& rct)
{
	istrstream inp(pStr);
	inp >> rct.left >> rct.top >> rct.right >> rct.bottom;
}

LONG TRegistryBase::rgSetValue(LPCTSTR       lpValueName,
							   DWORD         dwType,
							   CONST BYTE*   lpData,
							   DWORD         cbData)
{
	return RegSetValueEx(m_hkKey, lpValueName, 0, dwType, lpData, cbData);
}

LONG TRegistryBase::rgWriteString(LPCTSTR lpValueName, const CString& str)
{
	// you must write out the NULL explicitly
	return RegSetValueEx(m_hkKey, lpValueName, 0, REG_SZ,
		(BYTE*)(LPCTSTR) str, (str.GetLength()+1) * sizeof(TCHAR));
}

LONG TRegistryBase::rgWriteMultiString(LPCTSTR lpValueName, const CString& str)
{
	CString strCopy(str);
	strCopy.TrimRight("\r\n");

	strCopy.Replace("\\", "\\\\");
	// Change "CR" to "C\\R"
	strCopy.Replace("CR", "C\\R");
	// Change "LF" to "L\\F"
	strCopy.Replace("LF", "L\\F");
	// Replace 0x0D with "CR"
	strCopy.Replace("\r", "CR");
	// Replace 0x0A with "LF"
	strCopy.Replace("\n", "LF");

	return RegSetValueEx(m_hkKey, lpValueName, 0, REG_SZ,
		(BYTE*)(LPCTSTR)strCopy, (strCopy.GetLength()+1) * sizeof(TCHAR));
}

LONG TRegistryBase::rgWriteNum(LPCTSTR       lpValueName,
							   DWORD         dwVal)
{
	return RegSetValueEx(m_hkKey, lpValueName, 0, REG_DWORD,
		(BYTE*)&dwVal, sizeof(dwVal));
}

LONG TRegistryBase::rgWriteLogfont(LPCTSTR lpValueName,
							   LPLOGFONT pLF,
							   int points)
{
	// this is a LOGFONT
	CString s;
	//  Convert to a big string
	//
	//          0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
	s.Format ("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s",
		points,
		pLF->lfHeight,
		pLF->lfWidth,
		pLF->lfEscapement,
		pLF->lfOrientation,
		pLF->lfWeight,
		pLF->lfItalic,
		pLF->lfUnderline,
		pLF->lfStrikeOut,
		pLF->lfCharSet,
		pLF->lfOutPrecision,
		pLF->lfClipPrecision,
		pLF->lfQuality,
		pLF->lfPitchAndFamily,
		pLF->lfFaceName);

	return rgWriteString ( lpValueName, s );
}

LONG TRegistryBase::rgReadLogfont(LPCTSTR lpValueName, LPLOGFONT pLF, int& points)
{
	TCHAR rcBuf[128];
	LONG lRet;
	DWORD dwType, dwValueSize;

	dwValueSize = sizeof(rcBuf);
	ZeroMemory(pLF, sizeof(LOGFONT));
	lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwType, (BYTE*)rcBuf, &dwValueSize);
	if (ERROR_SUCCESS == lRet)
	{
		istrstream iss(rcBuf);
		iss >>    points;
		iss >>    pLF->lfHeight;
		iss >>    pLF->lfWidth;
		iss >>    pLF->lfEscapement;
		iss >>    pLF->lfOrientation;
		iss >>    pLF->lfWeight;
		int i;
		iss >> i; pLF->lfItalic     = BYTE(i);
		iss >> i; pLF->lfUnderline  = BYTE(i);
		iss >> i; pLF->lfStrikeOut  = BYTE(i);
		iss >> i; pLF->lfCharSet    = BYTE(i);
		iss >> i; pLF->lfOutPrecision = BYTE(i);
		iss >> i; pLF->lfClipPrecision= BYTE(i);
		iss >> i; pLF->lfQuality = BYTE(i);
		iss >> i; pLF->lfPitchAndFamily = BYTE(i);
		iss >> ws;   // eat whitespace
		iss.get(pLF->lfFaceName, sizeof(pLF->lfFaceName));
	}
	return lRet;
}

LONG TRegistryBase::rgReadBool(LPCTSTR lpValueName, BYTE* buf, int size, BOOL& fOn)
{
	LONG lRet;
	DWORD dwValueType;
	DWORD dwValueSize = size;
	lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwValueType,
		buf, &dwValueSize);
	if (ERROR_SUCCESS == lRet)
		fOn = *((BOOL*) buf);
	return lRet;
}

LONG TRegistryBase::rgReadInt(LPCTSTR lpValueName, BYTE* buf, int size, int& n)
{
	LONG lRet;
	DWORD dwValueType;
	DWORD dwValueSize = size;
	lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwValueType,
		buf, &dwValueSize);
	if (ERROR_SUCCESS == lRet)
		n = *((int*) buf);
	return lRet;
}

LONG TRegistryBase::rgReadLong(LPCTSTR lpValueName, BYTE* buf, int size, LONG& ln)
{
	LONG lRet;
	DWORD dwValueType;
	DWORD dwValueSize = size;
	lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwValueType,
		buf, &dwValueSize);
	if (ERROR_SUCCESS == lRet)
		ln = *((LONG*) buf);
	return lRet;
}

LONG TRegistryBase::rgReadString(LPCTSTR       lpValueName,
								 BYTE*         buf,
								 int           size,
								 CString&       str)
{
	LONG lRet;
	DWORD dwValueType;
	DWORD dwValueSize = size;
	lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwValueType,
		buf, &dwValueSize);

	if (ERROR_SUCCESS == lRet)
	{
		str = (LPCTSTR) buf;
	}
	else if (ERROR_MORE_DATA == lRet)
	{
		LPTSTR p = str.GetBuffer(dwValueSize);
		lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwValueType,
			(BYTE*)p, &dwValueSize);
		str.ReleaseBuffer(dwValueSize-1);
	}
	else
	{
		throw(new TException(IDS_REGISTRY_ERROR, kError));
		return 0;
	}
	return lRet;
}

LONG TRegistryBase::rgReadMultiString(LPCTSTR  lpValueName,
								 BYTE*         buf,
								 int           size,
								 CString&      str)
{
	LONG lRet = rgReadString(lpValueName, buf, size, str);

	if (ERROR_SUCCESS == lRet)
	{
		// Replace 0x0D with "CR"
		str.Replace("CR", "\r");
		// Replace 0x0A with "LF"
		str.Replace("LF", "\n");
		// Change "CR" to "C\\R"
		str.Replace("C\\R", "CR");
		// Change "LF" to "L\\F"
		str.Replace("L\\F", "LF");

		str.Replace("\\\\", "\\");

		str.TrimRight("\r\n");
	}

	return lRet;
}

LONG TRegistryBase::rgReadWord(LPCTSTR lpValueName, BYTE* buf, int size, WORD& w)
{
	LONG lRet;
	DWORD dwValueType;
	DWORD dwValueSize = size;
	lRet = RegQueryValueEx(m_hkKey, lpValueName, 0, &dwValueType,
		buf, &dwValueSize);

	if (ERROR_SUCCESS == lRet)
	{
		w = *((WORD*) buf);
	}
	else
	{
		throw(new TException(IDS_REGISTRY_ERROR, kError));
		return 0;
	}
	return lRet;
}

LONG TRegistryBase::rgWriteRGB(LPCTSTR lpValueName, DWORD dwColor)
{
	CString rgb;
	rgb.Format ("%d,%d,%d",
		GetRValue(dwColor),
		GetGValue(dwColor),
		GetBValue(dwColor));
	return rgWriteString (lpValueName, rgb);
}

LONG TRegistryBase::rgReadRGB(LPCTSTR vn, BYTE* buf, int size, DWORD& dw)
{
	CString color;
	LONG lRet = rgReadString (vn, buf, size, color);
	int red, green, blue;
	sscanf (LPCTSTR(color), "%d,%d,%d", &red, &green, &blue);
	dw = RGB(red, green, blue);
	return lRet;
}

LONG TRegistryBase::rgWriteTime(LPCTSTR vn, CTime* pTime)
{
	CString strData;
	strData.Format("%d %d %d %d %d %d", pTime->GetYear(), pTime->GetMonth(),
		pTime->GetDay(), pTime->GetHour(), pTime->GetMinute(),
		pTime->GetSecond() );
	return rgWriteString(vn, strData);
}

LONG TRegistryBase::rgReadTime(LPCTSTR vn, BYTE* buf, int size, CTime* pTime)
{
	LONG  lRet;
	int yr, mon, day, hr, min, sec;
	CString data;
	DWORD dwType, dwValueSize;
	dwValueSize = size;

	lRet = RegQueryValueEx(m_hkKey, vn, 0, &dwType, buf, &dwValueSize);
	if (ERROR_SUCCESS == lRet)
	{
		istrstream iss((char*)buf);
		iss >> yr >> mon >> day >> hr >> min >> sec;
		CTime tmp(yr, mon, day, hr, min, sec);
		*pTime = tmp;
	}
	return lRet;
}

LONG TRegistryBase::rgWriteTable(int count, TableItem* pItems)
{
	int i;
	LONG lRet;
	for (i = 0; i < count; i++)
	{
		TableItem* pItem = pItems + i;
		switch (pItem->inputType)
		{
		case kREGString:
			lRet = rgWriteString(pItem->pValueName, *pItem->pStr);
			break;
		case kREGMultiString:
			lRet = rgWriteMultiString(pItem->pValueName, *pItem->pStr);
			break;
		case kREGNum:
		case kREGInt:
		case kREGLong:
		case kREGWord:
		case kREGBool:
			lRet = rgWriteNum(pItem->pValueName, (DWORD)*pItem->pLong);
			break;
		case kREGRGB:
			rgWriteRGB(pItem->pValueName, *pItem->pRGB);
			break;
		case kREGLogfont:
			ASSERT(0);
			break;

		case kREGTime:
			rgWriteTime(pItem->pValueName, pItem->pTime);
			break;

		case kREGInvalid:
			break;
		default:
			ASSERT(0);
			break;
		}
	}
	return 0;
}

LONG TRegistryBase::rgReadTable(int count, TableItem* pItems)
{
	BYTE buf[80];
	int i;
	LONG lRet;
	for (i = 0; i < count; i++)
	{
		TableItem* pItem = pItems + i;
		if (pItem->pValueName)
		{
			switch (pItem->inputType)
			{
			case kREGString:
				try
				{
					lRet = rgReadString(pItem->pValueName, buf, sizeof(buf), *pItem->pStr);
				}
				catch (TException *pE)
				{
					if (!pItem->m_fOptional)
					{
						TException *ex = new TException(*pE);
						pE->Delete();
						throw(ex);
						return 0;
					}
					else
						pE->Delete();
				}
				break;
			case kREGMultiString:
				try
				{
					lRet = rgReadMultiString(pItem->pValueName, buf, sizeof(buf), *pItem->pStr);
				}
				catch (TException *pE)
				{
					if (!pItem->m_fOptional)
					{
						TException *ex = new TException(*pE);
						pE->Delete();
						throw(ex);
						return 0;
					}
					else
						pE->Delete();
				}
				break;
			case kREGInt:
				lRet = rgReadInt(pItem->pValueName, buf, sizeof(buf), *pItem->pInt);
				break;
			case kREGLong:
				lRet = rgReadLong(pItem->pValueName, buf, sizeof(buf), *pItem->pLong);
				break;
			case kREGWord:
				lRet = rgReadWord(pItem->pValueName, buf, sizeof(buf), *pItem->pWord);
				break;
			case kREGBool:
				lRet = rgReadBool(pItem->pValueName, buf, sizeof(buf), *pItem->pBool);
				break;
			case (kREGRGB):
				lRet = rgReadRGB(pItem->pValueName, buf, sizeof(buf), *pItem->pRGB);
				break;
			case kREGNum:
			case kREGLogfont:
				ASSERT(0);
				break;
			case kREGTime:
				lRet = rgReadTime(pItem->pValueName, buf, sizeof(buf), pItem->pTime);
				break;
			default:
				ASSERT(0);
				break;
			}
		}
		// else key missing from registry, we'll write it out on exit
	}
	return lRet;
}

void TRegistryBase::SetItem(TableItem*    pItem,
							LPCTSTR       ValueName,
							ERegistryType eType,
							void*         pData,
							BOOL          fOptional)
{
	pItem->pValueName = ValueName;
	pItem->inputType  = eType;
	pItem->m_fOptional = fOptional;

	// this cast is not so great
	pItem->pBool = (BOOL*) pData;
}

void TRegistryBase::EnsureStringExists(LPCTSTR pchName)
{
	DWORD dwValueType;
	DWORD dwValueSize;
	LONG lRet = RegQueryValueEx (m_hkKey, pchName, 0, &dwValueType,
		NULL, &dwValueSize);
	if ((lRet == ERROR_SUCCESS) || (lRet == ERROR_MORE_DATA))
		return;

	// write empty string
	rgWriteString (pchName, CString (""));
}
