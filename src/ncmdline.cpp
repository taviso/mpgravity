/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ncmdline.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:33  richard_wood
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
#include "ncmdline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern bool gfLogSocketDataToFile;              // from newssock.cpp

// ------------------------------------------------------------------------
// constructor
TNewsCommandLineInfo::TNewsCommandLineInfo ()
	: m_bLogStartup(FALSE)
	, m_bSafeStart(FALSE)
{
	m_bNewsURL = m_bVCRFile = m_bGSXFile = false;
}

// --------------------------------------------------------------------------
// TNewsCommandLineInfo - so we can override this v.func
//
//  7-29-97  amc  copy news url string from command line
void
TNewsCommandLineInfo::ParseParam (LPCTSTR lpszParam, BOOL fFlag,
								  BOOL /*bLast*/)
{
	static bool fVCRFileParamComing = false;

	if (fFlag)
	{
		if (0 == _tcsicmp(lpszParam, _T("logo")))
			m_bShowSplash = TRUE;
		else if (0 == _tcsicmp(lpszParam, _T("log")))
			m_bLogStartup = TRUE;
		else if (0 == _tcsicmp(lpszParam, _T("safestart")))
			m_bSafeStart = TRUE;
		else if (0 == _tcsnicmp(lpszParam, _T("newsurl:"), 8))
		{
			CString tmp(lpszParam);

			// trim off our flag
			m_strNewsURL = tmp.Mid(8);
			if (!m_strNewsURL.IsEmpty())
			{
				m_bNewsURL = true;
			}
		}
		else if (0 == _tcsicmp(lpszParam, "socket"))
		{
			// start loggin socket data to GRAV000.TRC file
			gfLogSocketDataToFile = true;
		}
		else if (0 == _tcsnicmp(lpszParam, "vcrfile:", 8))
		{
			CString tmp(lpszParam);

			// trim off our flag
			m_strVCRFile = tmp.Mid(8);
			m_strVCRFile.TrimLeft();
			m_strVCRFile.TrimRight();
			if (m_strVCRFile.IsEmpty())
				fVCRFileParamComing = true;
			else
				m_bVCRFile = true;
		}
		else if (0 == _tcsnicmp(lpszParam, "gsxfile:", 8))
		{
			CString tmp(lpszParam);

			// trim off our flag
			m_strGSXFile = tmp.Mid(8);
			m_strGSXFile.TrimLeft();
			m_strGSXFile.TrimRight();
			if (m_strGSXFile.IsEmpty())
				m_bGSXFile = true;
			else
				m_bGSXFile = true;
		}
	}
	else
	{
		if (fVCRFileParamComing)
		{
			fVCRFileParamComing = false;
			m_strVCRFile = lpszParam;
			m_bVCRFile = true;
		}
	}
}
