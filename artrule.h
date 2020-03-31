/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artrule.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:11  richard_wood
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

// artrule.h -- per-article rule-related information

#pragma once

#include "sharesem.h"            // TSynchronizable
#include "pobject.h"             // PObject

// disable warnings about "synonym"
#pragma warning ( disable : 4097 )

// for versioning
#define ART_RULE_INFO_VERSION_NUMBER 1

// flag bits -- don't rearrange (could be headers storing these numbers)
const USHORT ACTION_DISCARD =             0x0001;// OBSOLETE! Can reuse this bit
const USHORT ACTION_ICON_OFF =            0x0002;// OBSOLETE! Can reuse this bit
const USHORT ACTION_TAG =                 0x0004;// OBSOLETE! Can reuse this bit
const USHORT BODY_ACTION_SAVE_TO_FILE =   0x0008;
const USHORT BODY_ACTION_FORWARD_TO =     0x0010;

// -------------------------------------------------------------------------
// TArtRuleInfo -- per-article rule-related information
class TArtRuleInfo : public PObject {
public:

#define MAX_BODY_RULES 5  /* max of 5 rules can require seeing body */

	TArtRuleInfo::TArtRuleInfo ();
	TArtRuleInfo::~TArtRuleInfo () {}
	void AddBodyRule (UCHAR iRule);
	void DelBodyRule (UCHAR iRule);
	UCHAR *GetBodyRules () { return m_riBodyRules; }
	void ClearBodyRules ()
	{ for (int i = 0; i < MAX_BODY_RULES; i++) m_riBodyRules [i] = 0; }

	void SetRuleFlag (USHORT iFlag) { m_iRuleFlags |= iFlag; }
	void ClearRuleFlag (USHORT iFlag) { m_iRuleFlags &= ~iFlag; }
	void ClearRuleFlags () { m_iRuleFlags = 0; }
	BOOL TestRuleFlag (USHORT iFlag) { return (m_iRuleFlags & iFlag); }
	USHORT GetRuleFlags () { return m_iRuleFlags; }
	void SetDecodeRule (UCHAR iRule) { m_iDecode = iRule; }
	UCHAR GetDecodeRule () { return m_iDecode; }
	void SetForwardRule (UCHAR iRule) { m_iForward = iRule; }
	UCHAR GetForwardRule () { return m_iForward; }
	void SetSaveRule (UCHAR iRule) { m_iSave = iRule; }
	UCHAR GetSaveRule () { return m_iSave; }
	BOOL NonEmpty ();
	void Serialize (CArchive& sArchive);

private:
	USHORT   m_iRuleFlags;        // see flag definitions below
	UCHAR    m_iDecode;           // rule wanting to decode article
	UCHAR    m_iForward;          // rule wanting to forward article
	UCHAR    m_iSave;             // rule wanting to save article
	UCHAR    m_riBodyRules [MAX_BODY_RULES];  // rules that want body
};

// -------------------------------------------------------------------------
// ArtRuleInfoMap -- rule-related information for all articles in a newsgroup
typedef CMap <LONG,LONG&,TArtRuleInfo*,TArtRuleInfo*&> CMapIntsToRuleInfo;

// destructElements is done automatically now.
//void __stdcall DestructElements( TArtRuleInfo ** pElements, int nCount );

class TArtRuleInfoMap : public CMapIntsToRuleInfo, public TSynchronizable {
public:
	TArtRuleInfoMap();
	~TArtRuleInfoMap ();
	void Empty ();
	void Serialize (CArchive& sArchive);
};
