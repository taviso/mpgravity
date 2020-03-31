/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: 8859x.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.10  2008/09/19 14:51:05  richard_wood
/*  Updated for VS 2005
/*  */
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

/****
#include "8859-1.h"
#include "8859-2.h"
#include "8859-3.h"
#include "8859-4.h"
#include "8859-5.h"
#include "8859-6.h"
#include "8859-7.h"
#include "8859-8.h"
#include "8859-9.h"
#include "8859-10.h"
#include "8859-13.h"
#include "8859-14.h"
#include "8859-15.h"
*****/

#include "superstl.h"

//forward declaration
class TCodeMgr;

void init_my_chartables();

enum GRAV_ISO { ISO59_1,
                ISO59_2,
                ISO59_3,
                ISO59_4,
                ISO59_5,
                ISO59_6,
                ISO59_7,
                ISO59_8,
                ISO59_9,
                ISO59_10,
                ISO59_13,
                ISO59_14,
                ISO59_15,
                UTF_8,
					 KOI8_R,
                WIN_1250,
                WIN_1251,
                WIN_1252,
                WIN_1253,
                WIN_1254,
                WIN_1255,
                WIN_1256,
                WIN_1257,
                WIN_1258,
                UTF_7,
                ISO59_11,
                ISO59_16
};

typedef map<WCHAR, BYTE> GRAVCSETMAP;

/// =============================
class GravCharset {
public:
	GravCharset(int serializedId, const CString & name, GRAV_ISO eIso, bool allow);
	virtual ~GravCharset() = 0 { }

	// operations
	CString  GetName()    { return m_name; }
	int      GetId()      { return m_id; }
   GRAV_ISO GetIsoEnum() { return m_eIso; }

	bool AllowPosting() { return m_fAllowPosting; }

   // used for posting
	virtual bool TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes) = 0;

	// converts from 8bit bytes to wide character string. Used for reading
	virtual void Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars) = 0;

   void MakeNamelist(CStringList & list);

private:
	CString  m_name;
	int	   m_id;
	GRAV_ISO m_eIso;
	bool     m_fAllowPosting;
};

class GravTableCharset : public GravCharset {
public:
	GravTableCharset(WORD wResourceID, int serializedId, const CString & name, GRAV_ISO eIso, bool allow);
	GravTableCharset(int serializedId, const CString & name, int count, WCHAR * pWCTable, GRAV_ISO eIso, bool allow);

   // used for posting
	virtual bool TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes);

	// converts from 8bit bytes to wide character string. Used for reading
	virtual void Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars);

private:
	void make_reverse_lookup();
   int  read_res_isomap(WORD wID, PVOID pMap, int bufsz);

private:
	WCHAR   m_vUpperChars[128];

	GRAVCSETMAP m_uniMap;
		
};

/// =============================
class GravFuncUTF8Charset : public GravCharset {
public:
   GravFuncUTF8Charset(int serializedId, const CString & name, GRAV_ISO eIso, bool allow);

   // override
   // used for posting
	virtual bool TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes);

	// override
   // converts from 8bit bytes to wide character string. Used for reading
	virtual void Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars);
};

/// =============================
class GravFuncUTF7Charset : public GravCharset {
public:
   GravFuncUTF7Charset(int serializedId, const CString & name, GRAV_ISO eIso, bool allow);
	~GravFuncUTF7Charset();

   // override
   // used for posting
	virtual bool TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes);

	// override
   // converts from 8bit bytes to wide character string. Used for reading
	virtual void Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars);

private:
	void process_base64buffer (std::vector<WCHAR> & vWideChars);

	std::vector<BYTE> m_b64;

	TCodeMgr * m_pCodeMgr;
};

/// =============================
class CharsetMaster {
public:
	~CharsetMaster();
	GravCharset* findById(int id);  // this may return NULL

   void Add(GravCharset* pCS) { m_sCharsets.push_back(pCS); }
	list<GravCharset*>  m_sCharsets;

	void SetupNames();

	GravCharset* findByName(const CString & charset);
   map<CString, GravCharset*> m_mapNames;
};

extern CharsetMaster gsCharMaster;
