/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: licutil.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
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
/*  Revision 1.3  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.2  2008/09/19 14:51:29  richard_wood
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

#include <winver.h>
#include "mpserial.h"

struct VERINFO
{
	VERINFO::VERINFO()
	{
		m_nMajor=0;
		m_nMinor=0;
		m_nBuild=0;
		m_bBeta=0;
	};
	int m_nMajor;
	int m_nMinor;
	int m_nBuild;
	bool m_bBeta;
};

class TLicenseSystem
{
public:
	static void GetVersionInt (VERINFO &verInfo);
	static CString GetUserAgentDescription(); // for UserAgent
	static CString FormatVersionString(); // for About dialog
};
