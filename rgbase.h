/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgbase.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
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
/*  Revision 1.2  2008/09/19 14:51:44  richard_wood
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

class TWindowStatus;

typedef enum  { kREGString, kREGNum, kREGRGB, kREGLogfont,
	kREGBool, kREGInt, kREGLong, kREGWord, kREGTime, kREGMultiString, kREGInvalid } ERegistryType;

class TRegistryBase {
public:
	~TRegistryBase() {}
	struct TableItem
	{
		TableItem::TableItem()
		{
			pValueName = NULL;
			inputType = kREGInvalid;
			m_fOptional = FALSE;
			pBool = NULL;
		};
		LPCTSTR         pValueName;
		ERegistryType   inputType;
		BOOL            m_fOptional;
		union
		{
			BOOL*    pBool;
			int*     pInt;
			LONG*    pLong;
			CString* pStr;
			WORD*    pWord;
			DWORD*   pRGB;
			LOGFONT* pLF;
			CTime*   pTime;
		};
	};

protected:
	TRegistryBase() {}          // protected constructor
	void rect_to_string(RECT& rct, CString& out);
	void str_to_rect(LPTSTR pStr, RECT& rct);
	HKEY m_hkKey;
	int  Load(LPCTSTR path);
	virtual void default_values() {}
	virtual void read() {}
	int  Save(LPCTSTR path);
	virtual void write() {}

	int  PreLoad(LPCTSTR path);
	virtual void preload() {}

	LONG rgSetValue(LPCTSTR       lpValueName,
		DWORD         dwType,
		CONST BYTE*   lpData,
		DWORD         cbData);
	LONG rgWriteString(LPCTSTR lpValueName, const CString& str);
	LONG rgWriteMultiString(LPCTSTR lpValueName, const CString& str);
	LONG rgWriteNum(LPCTSTR lpValueName, DWORD dwVal);
	LONG rgWriteRGB(LPCTSTR lpValueName, DWORD dwColor);
	LONG rgWriteLogfont(LPCTSTR lpValueName, LPLOGFONT pLF, int points);
	LONG rgWriteTime(LPCTSTR lpValueName, CTime* pTime);

	LONG rgReadBool(LPCTSTR lpValueName, BYTE* buf, int size, BOOL& fOn);
	LONG rgReadInt(LPCTSTR vn, BYTE* buf, int sz, int& n);
	LONG rgReadLong(LPCTSTR vn, BYTE* buf, int sz, LONG& ln);
	LONG rgReadString(LPCTSTR vn, BYTE* buf, int sz, CString& str);
	LONG rgReadMultiString(LPCTSTR vn, BYTE* buf, int sz, CString& str);
	LONG rgReadWord(LPCTSTR vn, BYTE* buf, int sz, WORD& w);
	LONG rgReadRGB(LPCTSTR vn, BYTE* buf, int sz, DWORD& dw);
	LONG rgReadLogfont(LPCTSTR lpValueName, LPLOGFONT pLF, int& points);
	LONG rgReadTime(LPCTSTR vn, BYTE* buf, int sz, CTime* pTime);

	LONG rgWriteTable(int count, TableItem* pItems);
	LONG rgReadTable(int count,  TableItem* pItems);
	void SetItem(TableItem*    pItem,
		LPCTSTR       ValueName,
		ERegistryType eType,
		void*         pData,
		BOOL          fOptional = FALSE);

	void EnsureStringExists (LPCTSTR pchName);

protected:
	TableItem*  m_pTable;
	int         m_itemCount;
};

#define COUNT(x)     (sizeof(x)/sizeof(x[0]))
