/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artrule.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.6  2009/01/30 00:08:25  richard_wood
/*  Fixed memory leaks.
/*
/*  Revision 1.5  2009/01/28 22:45:48  richard_wood
/*  Added border round article pane in main window.
/*  Cleaned up some memory leaks.
/*
/*  Revision 1.4  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
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

// artrule.cpp -- per-article rule-related information

#include "stdafx.h"              // precompiled header
#include "artrule.h"             // this file's prototypes
#include "globals.h"             // TNewsServer, gpStore
#include "mplib.h"               // TPath

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TArtRuleInfo::TArtRuleInfo() : PObject(ART_RULE_INFO_VERSION_NUMBER)
{
	m_iRuleFlags = 0;
	m_iDecode = 0;
	m_iForward = 0;
	m_iSave = 0;
	for (int i = 0; i < MAX_BODY_RULES; i++)
		m_riBodyRules[i] = 0;
}

// -------------------------------------------------------------------------
// AddBodyRule, DelBodyRule -- add/delete a rule to/from the list of rules
// waiting for the article's body
void TArtRuleInfo::AddBodyRule(UCHAR iRule)
{
	for (int i = 0; i < MAX_BODY_RULES; i++)
	{
		if (!m_riBodyRules[i])
		{
			m_riBodyRules[i] = iRule;
			return;
		}
	}
	// may want to set a special status bit here to indicate that we've run
	// out of slots for body-rules
}

void TArtRuleInfo::DelBodyRule(UCHAR iRule)
{
	for (int i = 0; i < MAX_BODY_RULES; i++)
	{
		if (m_riBodyRules[i] == iRule)
		{
			m_riBodyRules[i] = 0;
			return;
		}
	}
	ASSERT(0);
}

// -------------------------------------------------------------------------
BOOL TArtRuleInfo::NonEmpty()
{
	if (m_iRuleFlags)
		return TRUE;

	for (int i = 0; i < MAX_BODY_RULES; i++)
		if (m_riBodyRules[i])
			return TRUE;

	return FALSE;
}

// -------------------------------------------------------------------------
void TArtRuleInfo::Serialize(CArchive& sArchive)
{
	PObject::Serialize(sArchive);

	if (sArchive.IsStoring())
	{
		sArchive << m_iRuleFlags;
		sArchive << m_iDecode;
		sArchive << m_iSave;
		sArchive << m_iForward;
		for (int i = 0; i < MAX_BODY_RULES; i++)
			sArchive << m_riBodyRules[i];
	}
	else
	{
		sArchive >> m_iRuleFlags;
		sArchive >> m_iDecode;
		sArchive >> m_iSave;
		sArchive >> m_iForward;
		for (int i = 0; i < MAX_BODY_RULES; i++)
			sArchive >> m_riBodyRules[i];
	}
}

// -------------------------------------------------------------------------
TArtRuleInfoMap::TArtRuleInfoMap()
{
	InitHashTable(4909);
}

// -------------------------------------------------------------------------
TArtRuleInfoMap::~TArtRuleInfoMap()
{
	// Clean up and delete the objects allocated
	LONG nKey;
	TArtRuleInfo *pRecord;
	POSITION pos = GetStartPosition();
	while(pos)
	{
		GetNextAssoc(pos, nKey, pRecord);

		if (pRecord)
			delete pRecord;
	}

	//   CMap destructor automatically calls RemoveAll();
}

// -------------------------------------------------------------------------
void TArtRuleInfoMap::Empty()
{
	TSyncWriteLock sWLock(this);
	// Clean up and delete the objects allocated
	LONG nKey;
	TArtRuleInfo *pRecord;
	POSITION pos = GetStartPosition();
	while(pos)
	{
		GetNextAssoc(pos, nKey, pRecord);

		if (pRecord)
			delete pRecord;
	}
	RemoveAll ();
}

// -------------------------------------------------------------------------
void TArtRuleInfoMap::Serialize(CArchive& sArchive)
{
	if (sArchive.IsStoring())
	{
		// store only those entries that contain nonzero information
		POSITION iPos = GetStartPosition();
		LONG lArtNum;
		TArtRuleInfo *pRecord;
		while (iPos)
		{
			GetNextAssoc(iPos, lArtNum, pRecord);
			if (pRecord->NonEmpty()) {
				sArchive << (UCHAR) 1;  // to indicate the presence of a record
				sArchive << lArtNum;
				pRecord->Serialize(sArchive);
			}
		}
		sArchive << (UCHAR) 0;  // to indicate the end of file
	}
	else
	{
		Empty();

		UCHAR iMore;
		TArtRuleInfo *pRecord;
		LONG lArtNum;
		while (1)
		{
			sArchive >> iMore;
			if (!iMore)
				break;

			// create a new record and read its info
			pRecord = new TArtRuleInfo;
			sArchive >> lArtNum;
			pRecord->Serialize(sArchive);

			// add the new record to the map
			SetAt(lArtNum, pRecord);
		}
	}
}
