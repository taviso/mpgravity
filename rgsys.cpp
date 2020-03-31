/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgsys.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/08/27 15:29:22  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.4  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.3  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
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
/*  Revision 1.6  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.5  2008/09/19 14:51:46  richard_wood
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
#include "resource.h"
#include "mplib.h"
#include "rgsys.h"
#include "regutil.h"
#include "licutil.h"
#include "superstl.h"            // istrstream, ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const LONG TRegSystem::m_giDefThreadPrio = 0;

// Explanation :  we want a lot of printable chars ( exclude Period )
//     ! thru ,
//      dash         (syntactically by itself)
//     [skip the period]
//     /  thru ~
//

TRegSystem::TRegSystem()
{
	default_values();
}

TRegSystem::~TRegSystem()
{
}

int TRegSystem::GetPrio(EThread eThread)
{
	if (m_lThreadPrio & eThread)
		return THREAD_PRIORITY_NORMAL;
	else
		return THREAD_PRIORITY_BELOW_NORMAL;
}

int TRegSystem::Load()
{
	return TRegistryBase::Load (GetGravityRegKey()+"System");
}

int TRegSystem::Save()
{
	return TRegistryBase::Save (GetGravityRegKey()+"System");
}

void TRegSystem::read()
{
	BYTE rcBuf[80];
	LONG lRet = rgReadInt("EventLogMax", rcBuf, sizeof(rcBuf), m_iEventLogMax);
	CString s;
	try {
		lRet = rgReadString("PumpHalt", rcBuf, sizeof(rcBuf), s);
		istrstream inp((LPTSTR)(LPCTSTR)s);
		inp >> m_iPumpHaltMin >> m_iPumpHaltMax;
	}
	catch (TException *pE) {pE->Delete();}

	lRet = rgReadLong("EngineExt", rcBuf, sizeof(rcBuf), m_lThreadPrio);
	lRet = rgReadInt("SubjMatch", rcBuf, sizeof(rcBuf), m_iSubjMatchLen);
	lRet = rgReadInt("LongHdrLines", rcBuf, sizeof(rcBuf), m_iHdrFieldBreakLen);
	LONG lTmpKey = 0;
	if (ERROR_SUCCESS == rgReadLong("PanicKey", rcBuf, sizeof(rcBuf), lTmpKey))
		m_uPanicKey = UINT(lTmpKey);
	rgReadBool ("PanicEnabled", rcBuf, sizeof(rcBuf), m_fPanicEnabled);
	rgReadTime ("LastUpdateCheck", rcBuf, sizeof (rcBuf), &m_lastUpdateCheck);
	rgReadInt ("UpdateCheckDays", rcBuf, sizeof (rcBuf), m_iUpdateCheckDays);
	rgReadBool ("CheckForUpdates", rcBuf, sizeof (rcBuf), m_fCheckForUpdates);

	try
	{
		rgReadString ("DecodeRE-Parts", rcBuf, sizeof (rcBuf), m_strFractionRE);
	}
	catch (TException *pE)
	{
		pE->Delete();
		m_strFractionRE = FRACTION_DEF_RE;
		rgWriteString ("DecodeRE-Parts",    m_strFractionRE);
	}
	try
	{
		rgReadString ("DecodeRE-Filename", rcBuf, sizeof (rcBuf), m_strFilenameRE);
	}
	catch (TException *pE)
	{
		pE->Delete();
		m_strFilenameRE = FILENAME_DEF_RE;
		rgWriteString ("DecodeRE-Filename", m_strFilenameRE);
	}

	try
	{
		rgReadString ("DejaNewsUrl", rcBuf, sizeof (rcBuf), m_strDejaNews);
	}
	catch (TException *pE)
	{
		pE->Delete();
		rgWriteString ("DejaNewsUrl", m_strDejaNews);
	}

	try {
		rgReadString ("SpellDict", rcBuf, sizeof(rcBuf), m_strDictionary);
	}
	catch(...)
	{
	}

	try {
		rgReadString ("SpellAff", rcBuf, sizeof(rcBuf), m_strAffinity);
	}
	catch(...)
	{
	}

	rgReadBool ("SpellIgNum", rcBuf, sizeof(rcBuf), m_fSpellIgnoreNumbers);
	rgReadInt ("ViewCharset", rcBuf, sizeof(rcBuf), m_iViewingCharset);
}

void TRegSystem::write()
{
	LONG lRet;
	CString s;
	s.Format ("%d %d", m_iPumpHaltMin, m_iPumpHaltMax);
	lRet = rgWriteString("PumpHalt",  s);
	lRet = rgWriteNum ("EventLogMax", m_iEventLogMax);
	lRet = rgWriteNum ("EngineExt", m_lThreadPrio);
	lRet = rgWriteNum ("SubjMatch", m_iSubjMatchLen);
	lRet = rgWriteNum ("Version",
		MAKELONG(m_verInfo.m_nBuild, m_verInfo.m_nMinor*10 + m_verInfo.m_nMajor*100));
	lRet = rgWriteNum ("LongHdrLines", m_iHdrFieldBreakLen);
	lRet = rgWriteNum ("PanicKey",  m_uPanicKey);
	rgWriteNum ("PanicEnabled", m_fPanicEnabled);
	rgWriteTime ("LastUpdateCheck", &m_lastUpdateCheck);
	rgWriteNum ("UpdateCheckDays", m_iUpdateCheckDays);
	rgWriteNum ("CheckForUpdates", m_fCheckForUpdates);

	rgWriteString ("DecodeRE-Parts",    m_strFractionRE);
	rgWriteString ("DecodeRE-Filename", m_strFilenameRE);

	rgWriteString ("SpellDict", m_strDictionary);
	rgWriteString ("SpellAff", m_strAffinity);

	rgWriteNum ("SpellIgNum", m_fSpellIgnoreNumbers);
	rgWriteNum ("ViewCharset", m_iViewingCharset);
}

void TRegSystem::default_values()
{
	m_iEventLogMax = 500;
	m_iPumpHaltMin = 1;
	m_iPumpHaltMax = 5000;
	m_lThreadPrio = m_giDefThreadPrio;

	m_iSubjMatchLen = 20;
	m_iHdrFieldBreakLen = 1000;

	TLicenseSystem::GetVersionInt(m_verInfo);

	m_uPanicKey = VK_F9;
	m_fPanicEnabled = false;

	m_lastUpdateCheck = CTime::GetCurrentTime();
	m_iUpdateCheckDays = 7;
	m_fCheckForUpdates = TRUE;

	m_strFractionRE = FRACTION_DEF_RE;
	m_strFilenameRE = FILENAME_DEF_RE;
	m_strDejaNews.LoadString (IDS_DEJANEWS_URL);

	m_strDictionary = "en_US.dic";
	m_strAffinity   = "en_US.aff";

	m_fSpellIgnoreNumbers = TRUE;

	m_iViewingCharset = 1;
}

// ------------------------------------------------------------------------
void TRegSystem::UpdateBuildNumber(int iCur)
{
	Save();
}

// ------------------------------------------------------------------------
LPCTSTR TRegSystem::GetFindFractionRE(bool fDefaultVersion)
{
	if (fDefaultVersion)
		return FRACTION_DEF_RE;

	return m_strFractionRE;
}

// ------------------------------------------------------------------------
LPCTSTR TRegSystem::GetFindFilenameRE(bool fDefaultVersion)
{
	if (fDefaultVersion)
		return FILENAME_DEF_RE;

	return m_strFilenameRE;
}

// ------------------------------------------------------------------------
void TRegSystem::SetFindFractionRE (LPCTSTR  pRE)

{
	m_strFractionRE = pRE;
}

