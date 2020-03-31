/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: AtlRegKey.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:05  richard_wood
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

class TRegKey : public CRegKey
{
	// Operations
public:
	LONG Create(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired = (KEY_ALL_ACCESS & ~KEY_CREATE_LINK))
	{
		return CRegKey::Create (hKeyParent, 
			lpszKeyName, 
			REG_NONE,         // class
			REG_OPTION_NON_VOLATILE,
			samDesired);
	}

	// I just re-ordered the parameters to something logical
public:
	LONG SetValue(LPCTSTR lpszValueName, DWORD dwValue)
	{
		return CRegKey::SetDWORDValue (lpszValueName, dwValue);
	}

	LONG SetValue(LPCTSTR lpszValueName, LPCTSTR lpszValue)
	{
		return CRegKey::SetStringValue (lpszValueName, lpszValue);
	}

	LONG GetValue(LPCTSTR lpszValueName, DWORD& dwValue)
	{
		return QueryDWORDValue(lpszValueName, dwValue);
	}

	LONG GetValue(LPCTSTR lpszValueName, CString & rstrValue)
	{
		DWORD dwSize = 4096 * 2;

		LPTSTR pBuf = rstrValue.GetBuffer(dwSize);
		LONG lRet = QueryStringValue( lpszValueName, pBuf, &dwSize);
		rstrValue.ReleaseBuffer();
		return lRet;
	}
};