/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: 8859x.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.9  2008/09/19 14:51:05  richard_wood
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

#include "stdafx.h"
#include "8859x.h"
#include "coding.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// <prototype>
extern ULONG String_base64_decode (LPTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz);
// </prototype>

static void setup_koi8_r();

CharsetMaster gsCharMaster;

// ===================================================================
CharsetMaster::~CharsetMaster()
{
	list<GravCharset*>::iterator it = m_sCharsets.begin();
	for (;it != m_sCharsets.end(); it++)
	{
		GravCharset * pCharset = *it;
		delete pCharset;
	}
	m_mapNames.clear();
}

// ===================================================================
GravCharset* CharsetMaster::findById(int id)
{
	list<GravCharset*>::iterator it = m_sCharsets.begin();
	for (; it != m_sCharsets.end(); it++)
	{
		GravCharset* pCS = *it;
		if (id == pCS->GetId())
			return pCS;
	}
	return NULL;
}

// ===================================================================
void CharsetMaster::SetupNames()
{
	list<GravCharset*>::iterator it = m_sCharsets.begin();
	for (; it != m_sCharsets.end(); it++)
	{
		GravCharset* pCharset =  *it;
		CStringList list;
		pCharset->MakeNamelist (list);

		POSITION pos = list.GetHeadPosition();
		while (pos)
		{
			CString name = list.GetNext (pos);
			m_mapNames.insert(map<CString, GravCharset*>::value_type(name, pCharset));
		}
	}
}

// ===================================================================
GravCharset* CharsetMaster::findByName(const CString & charset)
{
	map<CString, GravCharset*>::iterator it = m_mapNames.find(charset);
	if (it == m_mapNames.end())
		return NULL;
	GravCharset* pCharset = (*it).second;
	return pCharset;
}

// ===================================================================
void  init_my_chartables()
{
	CString name;
	WORD id;

	id = 1;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_1, true) );
	id = 2;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_2, true) );
	id = 3;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_3, true) );
	id = 4;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_4, true) );
	id = 5;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_5, true) );
	id = 6;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_6, true) );
	id = 7;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_7, true) );
	id = 8;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_8, true) );
	id = 9;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_9, true) );
	id = 10;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_10,true) );
	id = 11;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_11,true) );
	id = 13;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_13,true) );
	id = 14;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_14,true) );
	id = 15;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_15,true) );
	id = 16;		name.Format ("iso-8859-%d", id);		gsCharMaster.Add( new GravTableCharset (id, id, name, ISO59_16,true) );

	setup_koi8_r();

	id = 1250;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1250, false) );
	id = 1251;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1251, false) );
	id = 1252;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1252, false) );
	id = 1253;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1253, false) );
	id = 1254;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1254, false) );
	id = 1255;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1255, false) );
	id = 1256;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1256, false) );
	id = 1257;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1257, false) );
	id = 1258;  name.Format("windows-%d", id);      gsCharMaster.Add( new GravTableCharset (id, id, name, WIN_1258, false) );

	id = 65;    name = "utf-8"; gsCharMaster.Add( new GravFuncUTF8Charset (id, name, UTF_8, true) );
	id = 66;    name = "utf-7"; gsCharMaster.Add( new GravFuncUTF7Charset (id, name, UTF_7, false) );

	gsCharMaster.SetupNames();
}

/***  ===================================================================
<code_set_name> KOI8-R
<comment_char> %
<escape_char> /
% version: 1.0
% repertoiremap: mnemonic,ds
%  source: RFC1489 via Gabor Kiss <kissg@sztaki.hu>

%  and Andrey Chernov <ache@[NOSPAM]>
***/
static void setup_koi8_r()
{
	int K = 0x80;
	WCHAR wc[128];

	wc[0x80 - K] = 0x2500;    // BOX DRAWINGS LIGHT HORIZONTAL
	wc[0x81 - K] = 0x2502;    // BOX DRAWINGS LIGHT VERTICAL
	wc[0x82 - K] = 0x250C;    // BOX DRAWINGS LIGHT DOWN AND RIGHT
	wc[0x83 - K] = 0x2510;    // BOX DRAWINGS LIGHT DOWN AND LEFT
	wc[0x84 - K] = 0x2514;    // BOX DRAWINGS LIGHT UP AND RIGHT
	wc[0x85 - K] = 0x2518;    // BOX DRAWINGS LIGHT UP AND LEFT
	wc[0x86 - K] = 0x251C;    // BOX DRAWINGS LIGHT VERTICAL AND RIGHT
	wc[0x87 - K] = 0x2524;    // BOX DRAWINGS LIGHT VERTICAL AND LEFT
	wc[0x88 - K] = 0x252C;    // BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
	wc[0x89 - K] = 0x2534;    // BOX DRAWINGS LIGHT UP AND HORIZONTAL
	wc[0x8A - K] = 0x253C;    // BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
	wc[0x8B - K] = 0x2580;    // UPPER HALF BLOCK
	wc[0x8C - K] = 0x2584;    // LOWER HALF BLOCK
	wc[0x8D - K] = 0x2588;    // FULL BLOCK
	wc[0x8E - K] = 0x258C;    // LEFT HALF BLOCK
	wc[0x8F - K] = 0x2590;    // RIGHT HALF BLOCK
	wc[0x90 - K] = 0x2591;    // LIGHT SHADE
	wc[0x91 - K] = 0x2592;    // MEDIUM SHADE
	wc[0x92 - K] = 0x2593;    // DARK SHADE
	wc[0x93 - K] = 0x2320;    // TOP HALF INTEGRAL
	wc[0x94 - K] = 0x25A0;    // BLACK SQUARE
	wc[0x95 - K] = 0x2219;    // BULLET OPERATOR
	wc[0x96 - K] = 0x221A;    // SQUARE ROOT
	wc[0x97 - K] = 0x2248;    // ALMOST EQUAL TO
	wc[0x98 - K] = 0x2264;    // LESS-THAN OR EQUAL TO
	wc[0x99 - K] = 0x2265;    // GREATER-THAN OR EQUAL TO
	wc[0x9A - K] = 0x00A0;    // NO-BREAK SPACE
	wc[0x9B - K] = 0x2321;    // BOTTOM HALF INTEGRAL
	wc[0x9C - K] = 0x00B0;    // DEGREE SIGN
	wc[0x9D - K] = 0x00B2;    // SUPERSCRIPT TWO
	wc[0x9E - K] = 0x00B7;    // MIDDLE DOT
	wc[0x9F - K] = 0x00F7;    // DIVISION SIGN
	wc[0xA0 - K] = 0x2550;    // BOX DRAWINGS DOUBLE HORIZONTAL
	wc[0xA1 - K] = 0x2551;    // BOX DRAWINGS DOUBLE VERTICAL
	wc[0xA2 - K] = 0x2552;    // BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
	wc[0xA3 - K] = 0x0451;    // CYRILLIC SMALL LETTER IO
	wc[0xA4 - K] = 0x2553;    // BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
	wc[0xA5 - K] = 0x2554;    // BOX DRAWINGS DOUBLE DOWN AND RIGHT
	wc[0xA6 - K] = 0x2555;    // BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
	wc[0xA7 - K] = 0x2556;    // BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
	wc[0xA8 - K] = 0x2557;    // BOX DRAWINGS DOUBLE DOWN AND LEFT
	wc[0xA9 - K] = 0x2558;    // BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
	wc[0xAA - K] = 0x2559;    // BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
	wc[0xAB - K] = 0x255A;    // BOX DRAWINGS DOUBLE UP AND RIGHT
	wc[0xAC - K] = 0x255B;    // BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
	wc[0xAD - K] = 0x255C;    // BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
	wc[0xAE - K] = 0x255D;    // BOX DRAWINGS DOUBLE UP AND LEFT
	wc[0xAF - K] = 0x255E;    // BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
	wc[0xB0 - K] = 0x255F;    // BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
	wc[0xB1 - K] = 0x2560;    // BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
	wc[0xB2 - K] = 0x2561;    // BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
	wc[0xB3 - K] = 0x0401;    // CYRILLIC CAPITAL LETTER IO
	wc[0xB4 - K] = 0x2562;    // BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
	wc[0xB5 - K] = 0x2563;    // BOX DRAWINGS DOUBLE VERTICAL AND LEFT
	wc[0xB6 - K] = 0x2564;    // BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
	wc[0xB7 - K] = 0x2565;    // BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
	wc[0xB8 - K] = 0x2566;    // BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
	wc[0xB9 - K] = 0x2567;    // BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
	wc[0xBA - K] = 0x2568;    // BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
	wc[0xBB - K] = 0x2569;    // BOX DRAWINGS DOUBLE UP AND HORIZONTAL
	wc[0xBC - K] = 0x256A;    // BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
	wc[0xBD - K] = 0x256B;    // BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
	wc[0xBE - K] = 0x256C;    // BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
	wc[0xBF - K] = 0x00A9;    // COPYRIGHT SIGN
	wc[0xC0 - K] = 0x044E;    // CYRILLIC SMALL LETTER YU
	wc[0xC1 - K] = 0x0430;    // CYRILLIC SMALL LETTER A
	wc[0xC2 - K] = 0x0431;    // CYRILLIC SMALL LETTER BE
	wc[0xC3 - K] = 0x0446;    // CYRILLIC SMALL LETTER TSE
	wc[0xC4 - K] = 0x0434;    // CYRILLIC SMALL LETTER DE
	wc[0xC5 - K] = 0x0435;    // CYRILLIC SMALL LETTER IE
	wc[0xC6 - K] = 0x0444;    // CYRILLIC SMALL LETTER EF
	wc[0xC7 - K] = 0x0433;    // CYRILLIC SMALL LETTER GHE
	wc[0xC8 - K] = 0x0445;    // CYRILLIC SMALL LETTER HA
	wc[0xC9 - K] = 0x0438;    // CYRILLIC SMALL LETTER I
	wc[0xCA - K] = 0x0439;    // CYRILLIC SMALL LETTER SHORT I
	wc[0xCB - K] = 0x043A;    // CYRILLIC SMALL LETTER KA
	wc[0xCC - K] = 0x043B;    // CYRILLIC SMALL LETTER EL
	wc[0xCD - K] = 0x043C;    // CYRILLIC SMALL LETTER EM
	wc[0xCE - K] = 0x043D;    // CYRILLIC SMALL LETTER EN
	wc[0xCF - K] = 0x043E;    // CYRILLIC SMALL LETTER O
	wc[0xD0 - K] = 0x043F;    // CYRILLIC SMALL LETTER PE
	wc[0xD1 - K] = 0x044F;    // CYRILLIC SMALL LETTER YA
	wc[0xD2 - K] = 0x0440;    // CYRILLIC SMALL LETTER ER
	wc[0xD3 - K] = 0x0441;    // CYRILLIC SMALL LETTER ES
	wc[0xD4 - K] = 0x0442;    // CYRILLIC SMALL LETTER TE
	wc[0xD5 - K] = 0x0443;    // CYRILLIC SMALL LETTER U
	wc[0xD6 - K] = 0x0436;    // CYRILLIC SMALL LETTER ZHE
	wc[0xD7 - K] = 0x0432;    // CYRILLIC SMALL LETTER VE
	wc[0xD8 - K] = 0x044C;    // CYRILLIC SMALL LETTER SOFT SIGN
	wc[0xD9 - K] = 0x044B;    // CYRILLIC SMALL LETTER YERU
	wc[0xDA - K] = 0x0437;    // CYRILLIC SMALL LETTER ZE
	wc[0xDB - K] = 0x0448;    // CYRILLIC SMALL LETTER SHA
	wc[0xDC - K] = 0x044D;    // CYRILLIC SMALL LETTER E
	wc[0xDD - K] = 0x0449;    // CYRILLIC SMALL LETTER SHCHA
	wc[0xDE - K] = 0x0447;    // CYRILLIC SMALL LETTER CHE
	wc[0xDF - K] = 0x044A;    // CYRILLIC SMALL LETTER HARD SIGN
	wc[0xE0 - K] = 0x042E;    // CYRILLIC CAPITAL LETTER YU
	wc[0xE1 - K] = 0x0410;    // CYRILLIC CAPITAL LETTER A
	wc[0xE2 - K] = 0x0411;    // CYRILLIC CAPITAL LETTER BE
	wc[0xE3 - K] = 0x0426;    // CYRILLIC CAPITAL LETTER TSE
	wc[0xE4 - K] = 0x0414;    // CYRILLIC CAPITAL LETTER DE
	wc[0xE5 - K] = 0x0415;    // CYRILLIC CAPITAL LETTER IE
	wc[0xE6 - K] = 0x0424;    // CYRILLIC CAPITAL LETTER EF
	wc[0xE7 - K] = 0x0413;    // CYRILLIC CAPITAL LETTER GHE
	wc[0xE8 - K] = 0x0425;    // CYRILLIC CAPITAL LETTER HA
	wc[0xE9 - K] = 0x0418;    // CYRILLIC CAPITAL LETTER I
	wc[0xEA - K] = 0x0419;    // CYRILLIC CAPITAL LETTER SHORT I
	wc[0xEB - K] = 0x041A;    // CYRILLIC CAPITAL LETTER KA
	wc[0xEC - K] = 0x041B;    // CYRILLIC CAPITAL LETTER EL
	wc[0xED - K] = 0x041C;    // CYRILLIC CAPITAL LETTER EM
	wc[0xEE - K] = 0x041D;    // CYRILLIC CAPITAL LETTER EN
	wc[0xEF - K] = 0x041E;    // CYRILLIC CAPITAL LETTER O
	wc[0xF0 - K] = 0x041F;    // CYRILLIC CAPITAL LETTER PE
	wc[0xF1 - K] = 0x042F;    // CYRILLIC CAPITAL LETTER YA
	wc[0xF2 - K] = 0x0420;    // CYRILLIC CAPITAL LETTER ER
	wc[0xF3 - K] = 0x0421;    // CYRILLIC CAPITAL LETTER ES
	wc[0xF4 - K] = 0x0422;    // CYRILLIC CAPITAL LETTER TE
	wc[0xF5 - K] = 0x0423;    // CYRILLIC CAPITAL LETTER U
	wc[0xF6 - K] = 0x0416;    // CYRILLIC CAPITAL LETTER ZHE
	wc[0xF7 - K] = 0x0412;    // CYRILLIC CAPITAL LETTER VE
	wc[0xF8 - K] = 0x042C;    // CYRILLIC CAPITAL LETTER SOFT SIGN
	wc[0xF9 - K] = 0x042B;    // CYRILLIC CAPITAL LETTER YERU
	wc[0xFA - K] = 0x0417;    // CYRILLIC CAPITAL LETTER ZE
	wc[0xFB - K] = 0x0428;    // CYRILLIC CAPITAL LETTER SHA
	wc[0xFC - K] = 0x042D;    // CYRILLIC CAPITAL LETTER E
	wc[0xFD - K] = 0x0429;    // CYRILLIC CAPITAL LETTER SHCHA
	wc[0xFE - K] = 0x0427;    // CYRILLIC CAPITAL LETTER CHE
	wc[0xFF - K] = 0x042A;    // CYRILLIC CAPITAL LETTER HARD SIGN

	int id = 64;      // number is written out the registry
	CString name = "koi8-r";

	GravCharset * pKOICharset = new GravTableCharset (id, name, 128, &wc[0], KOI8_R, true);
	gsCharMaster.Add ( pKOICharset );
}

// ===================================================================
// base class
GravCharset::GravCharset(int serializedId, const CString & name, GRAV_ISO eIso, bool allowPost)
: m_name(name), m_id(serializedId), m_eIso(eIso), m_fAllowPosting(allowPost)
{
	// empty
}

// ===================================================================
// This should really be a virtual function in derived classes, but
// I am lazy
//
void GravCharset::MakeNamelist(CStringList & slist)
{
	switch (GetIsoEnum())
	{
	case ISO59_1:
		slist.AddTail ("us-ascii");
		slist.AddTail (GetName());
		break;
	case ISO59_2:
	case ISO59_3:
	case ISO59_4:
	case ISO59_5:
	case ISO59_6:
	case ISO59_7:
	case ISO59_8:
	case ISO59_9:
	case ISO59_10:
	case ISO59_11:
	case ISO59_13:
	case ISO59_14:
	case ISO59_15:
	case ISO59_16:
	case UTF_8:
	case KOI8_R:
	case UTF_7:
		slist.AddTail (GetName());
		break;

	case WIN_1250:
	case WIN_1251:
	case WIN_1252:
	case WIN_1253:
	case WIN_1254:
	case WIN_1255:
	case WIN_1256:
	case WIN_1257:
	case WIN_1258:
		{
			CString s2; s2.Format("cp%d", GetId());
			CString s3; s3.Format("cp-%d", GetId());
			slist.AddTail (GetName());
			slist.AddTail (s2);
			slist.AddTail (s3);
			break;
		}
	}
}

// ===================================================================
// load from our funky binary resources
GravTableCharset::GravTableCharset(WORD wResourceID, int serializedId, const CString & name, GRAV_ISO eIso, bool allowPost)
: GravCharset(serializedId, name, eIso, allowPost)
{
	int ret = read_res_isomap(wResourceID, m_vUpperChars, sizeof(m_vUpperChars));
	ASSERT(0 == ret);

	// build our reverse lookup table
	make_reverse_lookup();
}

// ===================================================================
// just pass in our table
GravTableCharset::GravTableCharset(int serializedId,
								   const CString & name,
								   int count,
								   WCHAR * pWCTable,
								   GRAV_ISO eIso,
								   bool allowPost)
								   : GravCharset(serializedId, name, eIso, allowPost)
{
	ASSERT(count == 128);
	CopyMemory (m_vUpperChars, pWCTable, sizeof(WCHAR) * 128);

	// build our reverse lookup table
	make_reverse_lookup();
}

// ===================================================================
void GravTableCharset::make_reverse_lookup()
{
	WORD b;
	for (b = 0x80; b <= 0xFF; b++)
	{
		WCHAR wc = m_vUpperChars[b - 0x80];

		m_uniMap.insert(GRAVCSETMAP::value_type(wc, BYTE(b)));
	}
}

// ===================================================================
// override a virtual
bool GravTableCharset::TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes)
{
	bool fFoundHighChar = false;
	WCHAR * pTrav = pWideChars;
	WCHAR wc;
	for (int i = 0; i < count; i++)
	{
		wc = *pTrav++;
		if (wc >= 0 && wc <= 0x7F)
		{
			outBytes.push_back( BYTE(wc) );
		}
		else
		{	
			fFoundHighChar = true;

			GRAVCSETMAP::iterator it = m_uniMap.find(wc);
			if (it == m_uniMap.end())
			{
				// nuts. did not find it
				if (preprocessForQP)
				{
					outBytes.push_back( '=' );
					outBytes.push_back( '3' );
					outBytes.push_back( 'F' );
				}
				else
				{
					outBytes.push_back( '?' );
				}
			}
			else
			{
				outBytes.push_back( (*it).second );
			}
		}
	}
	return fFoundHighChar;
}

// ===================================================================
// virtual override
// converts from 8bit bytes to wide character string
//
void GravTableCharset::Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars)
{
	LPCTSTR pTrav = strIn;
	for (int i = 0; i < len; i++, pTrav++)
	{
		BYTE c = *pTrav;
		if (c < 0x80)
		{
			vWideChars.push_back( c );
		}
		else
		{
			WCHAR wc = m_vUpperChars[c - 0x80];  // shortened table so - 0x80

			if (wc)
			{
				vWideChars.push_back( wc );
			}
			else
			{
				vWideChars.push_back( '?' );
			}
		}
	}
}

// ===================================================================
int GravTableCharset::read_res_isomap(WORD wID, PVOID pMap, int bufsz)
{
	HRSRC hFind = FindResource(NULL, MAKEINTRESOURCE(wID), "ISOMAP");

	if (hFind)
	{
		HANDLE hRes = LoadResource (NULL, hFind);

		if (hRes)
		{
			DWORD sz = SizeofResource (NULL, hFind);

			ASSERT(256 == sz);
			ASSERT(bufsz == sz);
			LPVOID pVoid = LockResource (hRes);

			if (pVoid)
			{
				CopyMemory (pMap, pVoid, sz);
				FreeResource (hRes);
				return 0;
			}
		}
	}
	return 1;
}

// ===================================================================
GravFuncUTF8Charset::GravFuncUTF8Charset(int serializedId, const CString & name, GRAV_ISO eIso, bool allowPost)
: GravCharset(serializedId, name, eIso, allowPost)
{
}

// ===================================================================
// virtual override - for posting
//
//
//   OOOO OOOO thru OOOO OO7F   0xxxxxxx  (7 bits)
//
//                                   5        6 bits
//   OOOO OO80 thru OOOO O7FF   110xxxxx 10xxxxxx
//
//												 4			 6			6
//   OOOO O800 thru OOOO FFFF   1110xxxx 10xxxxxx 10xxxxxx
//
//  returns true if we found an high bit character
bool GravFuncUTF8Charset::TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes)
{
	WCHAR* pTrav = pWideChars;
	bool   fFoundHighChar = false;
	BYTE   vBytes[3];	
	WCHAR wc;
	for (int i = 0; i < count; i++)
	{
		wc = *pTrav++;

		if (wc <= 0x007F)
		{
			// we need 1 output byte
			outBytes.push_back( BYTE(wc) );
		}
		else if (wc <= 0x07FF)
		{
			fFoundHighChar = true;
			// using 2 output bytes
			vBytes[0] = 0xC0 | (0x1F & (wc >> 6));
			vBytes[1] = 0x80 | (0x3F & wc);

			outBytes.push_back( vBytes[0] );
			outBytes.push_back( vBytes[1] );
		}
		else
		{	
			fFoundHighChar = true;

			// using 3 output bytes
			vBytes[2] = 0x80 | (0x3F & wc);

			wc >>= 6;
			vBytes[1] = 0x80 | (0x3F & wc);

			wc >>= 6;
			vBytes[0] = 0xE0 | (0x0F & wc);

			outBytes.push_back( vBytes[0] );
			outBytes.push_back( vBytes[1] );
			outBytes.push_back( vBytes[2] );
		}
	}
	return fFoundHighChar;
}

// ===================================================================
// virtual override
// converts from 8bit bytes to wide character string
//
void GravFuncUTF8Charset::Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars)
{
	LPCTSTR pTrav = strIn;

	// Bytes left for the curent character, or 0 if we're at the start
	// of a new character, or -1 if we just don't know...
	int bytesLeft=0;
	// Data we've already got for the current character
	WCHAR curChar=0;

	for (int i = 0; i < len; i++, pTrav++)
	{
		BYTE c = *pTrav;

		// In the middle of an unknown character...
		if (bytesLeft==-1)
		{
			// If this byte isn't the start of a new character, continue
			if ((c & 0xc0) == 0x80)
				continue;

			// Otherwise, we can start the next character
			bytesLeft=0;
		}

		if (bytesLeft==0)
		{
			if (c < 0x80)
			{
				vWideChars.push_back( c );
			}
			else if ((c & 0xE0) == 0xC0) // test the high three bits, do they equal 110
			{
				curChar = c & 0x1f;  // take the lowest 5 bits
				bytesLeft=1;
			}
			else if ((c & 0xF0) == 0xE0) // test the high byte, should equal 1110
			{
				curChar = c & 0xf;  // take the low byte
				bytesLeft=2;
			}
			else
			{
				// Very wide character or dodgy encoding.
				// Wait until the next decent character comes.
				vWideChars.push_back( '?' );
				bytesLeft=-1;
			}
		}
		else
		{
			if ((c & 0xC0) != 0x80) // if hi 2 bits != (binary) 10 ?
			{
				// We'd expected more data here.
				// Wait until the next decent character comes.
				vWideChars.push_back( '?' );
				bytesLeft=-1;
			}
			else
			{
				curChar = (curChar << 6) | (c & 0x3F);  // take the lowest 6 bits from 'c'
				bytesLeft--;
				// Have we got everything now? If so,
				// write out the character.
				if (bytesLeft==0)
				{
					vWideChars.push_back( curChar );
				}
			}
		}
	}
}

// ===================================================================
GravFuncUTF7Charset::GravFuncUTF7Charset(int serializedId, const CString & name, GRAV_ISO eIso, bool allowPost)
: GravCharset(serializedId, name, eIso, allowPost)
{
	m_pCodeMgr = new TCodeMgr;
}

// ===================================================================
GravFuncUTF7Charset::~GravFuncUTF7Charset()
{
	delete m_pCodeMgr;
}

// ===================================================================
// virtual override (for Posting)
bool GravFuncUTF7Charset::TranslateUnicodeToBytes(BOOL preprocessForQP, int count, WCHAR* pWideChars, vector<BYTE> & outBytes)
{
	ASSERT(0);   // not implemented
	return false;
}

// ===================================================================
// virtual override
// converts from 8bit bytes to wide character string
//
void GravFuncUTF7Charset::Translate (LPCTSTR strIn, int len, std::vector<WCHAR> & vWideChars)
{
	LPCTSTR pTrav = strIn;
	bool shift = false;

	for (int i = 0; i < len; ++i, ++pTrav)
	{
		BYTE c = *pTrav;

		if (false == shift)
		{
			if ('+' == c)
			{
				if ('-' == *(pTrav + 1))
				{
					//  +- yields +
					vWideChars.push_back( '+' );	
				}
				else
				{
					shift = true;
				}
			}
			else
				vWideChars.push_back( c );
		}
		else
		{
			// are we stopping the run of b64 characters?
			if ('\r' == c || '\n' == c || '-' == c)
			{
				shift = false;
				// process base64stuff
				process_base64buffer( vWideChars );

				if (c == '-')
				{
					//absorb the minus sign
				}
				else
					vWideChars.push_back( c );
			}
			else
			{
				// accumulate
				m_b64.push_back ( c );
			}
		}
	}

#if defined(_DEBUG)
	{
		// look at this with debugger
		WCHAR * pWide = &vWideChars[0];
		shift = false;
	}
#endif
}

// ===================================================================
void GravFuncUTF7Charset::process_base64buffer (std::vector<WCHAR> & vWideChars)
{
	// 4 six-bit bytes  ==  3 8bit chars.
	vector<TCHAR> vOutBuf( m_b64.size() );  // this is generous

	m_b64.push_back( 0 ); // null terminate the string, so istrstream knows the EOF

	LPTSTR pText = reinterpret_cast<LPTSTR>(&m_b64[0]);
	istrstream in_stream(pText);
	strstream  out_stream;
	ULONG      ulen = 0;

	m_pCodeMgr->Decode_base64_to_binary ( in_stream, 0, out_stream, &ulen);
	out_stream.seekg(0);

	for (int i = 0; (1+i) < ulen; i+= 2)
	{
		BYTE lo, hi;
		out_stream >> hi;
		out_stream >> lo;
		vWideChars.push_back( static_cast<WCHAR>(MAKEWORD( lo, hi )) );
	}
	m_b64.clear();
}

