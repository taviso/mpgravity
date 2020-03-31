/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgurl.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.3  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.2  2008/09/19 14:51:47  richard_wood
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
#include "pobject.h"
#include "rgurl.h"
#include <stdio.h>  // for sscanf
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TURLSettings & TURLSettings::operator=(const TURLSettings &rhs)
{
	m_fMailUseRegistry   = rhs.m_fMailUseRegistry;
	m_fWebUseRegistry    = rhs.m_fWebUseRegistry;
	m_fFtpUseRegistry    = rhs.m_fFtpUseRegistry;
//	m_fGopherUseRegistry = rhs.m_fGopherUseRegistry;
//	m_fTelnetUseRegistry = rhs.m_fTelnetUseRegistry;
	m_hotlinkColor       = rhs.m_hotlinkColor;
	m_fUnderlineLinks    = rhs.m_fUnderlineLinks;
	m_webPattern         = rhs.m_webPattern;
	m_ftpPattern         = rhs.m_ftpPattern;
//	m_gopherPattern      = rhs.m_gopherPattern;
//	m_telnetPattern      = rhs.m_telnetPattern;
	m_mailToPattern      = rhs.m_mailToPattern;
	m_fHighlightMail     = rhs.m_fHighlightMail;
	m_fHighlightWeb      = rhs.m_fHighlightWeb;
	m_fHighlightFtp      = rhs.m_fHighlightFtp;
//	m_fHighlightGopher      = rhs.m_fHighlightGopher;
//	m_fHighlightTelnet      = rhs.m_fHighlightTelnet;
	return *this;
}

TURLSettings::TURLSettings()
{
	default_values();
	// Setup the table
	m_itemCount = 18;
	m_pTable = new TableItem[m_itemCount];
	SetItem (m_pTable + 0, "MailUseReg",     kREGLong,  &m_fMailUseRegistry);
	SetItem (m_pTable + 1, "WebUseReg",      kREGLong,  &m_fWebUseRegistry);
	SetItem (m_pTable + 2, "FtpUseReg",      kREGLong,  &m_fFtpUseRegistry);
//	SetItem (m_pTable + 3, "GopherUseReg",   kREGLong,  &m_fGopherUseRegistry);
//	SetItem (m_pTable + 4, "TelnetUseReg",   kREGLong,  &m_fTelnetUseRegistry);
	SetItem (m_pTable + 5, "Underline",      kREGLong,  &m_fUnderlineLinks);
	SetItem (m_pTable + 6, "Color",          kREGRGB,   &m_hotlinkColor);
	SetItem (m_pTable + 7, "HighlightMail",     kREGLong, &m_fHighlightMail);
	SetItem (m_pTable + 8, "HighlightWeb",      kREGLong, &m_fHighlightWeb);
	SetItem (m_pTable + 9, "HighlightFtp",      kREGLong, &m_fHighlightFtp);
	//SetItem (m_pTable +10, "HighlightGopher",   kREGLong, &m_fHighlightGopher);
	//SetItem (m_pTable +11, "HighlightTelnet",   kREGLong, &m_fHighlightTelnet);
	SetItem (m_pTable +12, "WebPattern",     kREGString, &m_webPattern);
	SetItem (m_pTable +13, "FtpPattern",     kREGString, &m_ftpPattern);
	//SetItem (m_pTable +14, "GopherPattern",  kREGString, &m_gopherPattern);
	//SetItem (m_pTable +15, "TelnetPattern",  kREGString, &m_telnetPattern);
	SetItem (m_pTable +16, "MailToPattern",  kREGString, &m_mailToPattern);
	SetItem (m_pTable +17, "NewsPattern",    kREGString, &m_newsPattern, TRUE);
	// if you add stuff here, please change the size of m_pTable[]
}

TURLSettings::~TURLSettings ()
{
	delete [] m_pTable;
}

int TURLSettings::Load()
{
	CString aggregate = GetGravityRegKey();
	aggregate += "URL\\Settings";
	int nRetVal = TRegistryBase::Load ( aggregate );
	// RLW : If the WebPattern is old style (and unchanged), replace with new style
	CString strOldStyle;
	strOldStyle.LoadString(IDS_OLD_DEFAULT_URL_HTTP);

	if (m_webPattern.CompareNoCase(strOldStyle) == 0)
	{
		m_webPattern.LoadString(IDS_DEFAULT_URL_HTTP);
		Save();
	}

	return nRetVal;
}

int TURLSettings::Save()
{
	CString aggregate = GetGravityRegKey();
	aggregate += "URL\\Settings";

	return TRegistryBase::Save ( aggregate );
}

void TURLSettings::default_values()
{
	m_fMailUseRegistry   = FALSE;
	m_fWebUseRegistry    = TRUE;
	m_fFtpUseRegistry    = TRUE;
//	m_fGopherUseRegistry = TRUE;
//	m_fTelnetUseRegistry = TRUE;
	m_hotlinkColor       = RGB(0,0,255);
	m_fUnderlineLinks    = FALSE;
	m_webPattern.LoadString(IDS_DEFAULT_URL_HTTP);
	m_ftpPattern.LoadString(IDS_DEFAULT_URL_FTP);
//	m_gopherPattern.LoadString(IDS_DEFAULT_URL_GOPHER);
//	m_telnetPattern.LoadString(IDS_DEFAULT_URL_TELNET);
	m_mailToPattern.LoadString(IDS_DEFAULT_URL_MAILTO);
	m_newsPattern.LoadString(IDS_DEFAULT_URL_NEWS);
	m_fHighlightMail   = TRUE;    // for the time being, just highlight these two
	m_fHighlightWeb    = TRUE;
	m_fHighlightNews   = TRUE;
	m_fHighlightFtp    = FALSE;
	//m_fHighlightGopher = FALSE;
	//m_fHighlightTelnet = FALSE;
	m_newsPattern.LoadString(IDS_DEFAULT_URL_NEWS);
}

void TURLSettings::write()
{
	LONG lRet = rgWriteTable(m_itemCount, m_pTable);
}

void TURLSettings::read()
{
	LONG lRet = rgReadTable(m_itemCount, m_pTable);
}

