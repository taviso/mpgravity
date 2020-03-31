/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: evtlog.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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
#include "evtlog.h"
#include "critsect.h"
#include "tglobopt.h"
#include "rgsys.h"
#include "utilrout.h"         // PostMainWndMsg

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions* gpGlobalOptions;

static TProcessLog  sTheProcessLog;
TProcessLog * gpProcessLog = &sTheProcessLog;

TEventEntry::TEventEntry(
						 EPriority      ePriority,
						 ESource        eSource,
						 const CTime&   time,
						 LPCTSTR        desc,
						 LPCTSTR        extDesc)
						 : m_ePriority(ePriority), m_eSource(eSource), m_time(time), m_desc(desc)
{
	if (extDesc)
		m_pExtendedDesc = new CString(extDesc);
	else
		m_pExtendedDesc = 0;
}

TEventEntry::~TEventEntry()
{
	delete m_pExtendedDesc;
}

// copy ctor
TEventEntry::TEventEntry(const TEventEntry& src)
{
	m_ePriority = src.m_ePriority;
	m_eSource   = src.m_eSource;
	m_time      = src.m_time;
	m_desc      = src.m_desc;

	m_pExtendedDesc = 0;
	if (src.m_pExtendedDesc)
		m_pExtendedDesc = new CString( *src.m_pExtendedDesc );
}

// assignment
TEventEntry&
TEventEntry::operator= (const TEventEntry& rhs)
{
	if (this == &rhs)
		return *this;
	delete m_pExtendedDesc;
	m_pExtendedDesc = 0;
	m_ePriority = rhs.m_ePriority;
	m_eSource   = rhs.m_eSource;
	m_time      = rhs.m_time;
	m_desc      = rhs.m_desc;

	if (rhs.m_pExtendedDesc)
		m_pExtendedDesc = new CString(*rhs.m_pExtendedDesc);

	return *this;
}

TEventLog::TEventLog()
{
	InitializeCriticalSection( &m_critSect );
	m_fViewIsOutofdate = FALSE;
}

TEventLog::~TEventLog()
{
	DeleteCriticalSection( &m_critSect );
}

void TEventLog::Add(
					TEventEntry::EPriority ePriority,
					TEventEntry::ESource   eSource,
					LPCTSTR  desc,
					LPCTSTR  extDesc /* == NULL */
					)
{
	int iLOG_MAX_ENTRIES;
	gpGlobalOptions->GetRegSystem()->GetEventLogMax ( &iLOG_MAX_ENTRIES );
	TEnterCSection mgr(&m_critSect);
	m_fViewIsOutofdate = TRUE;
	TEventEntry entry(ePriority, eSource, CTime::GetCurrentTime(),
		desc, extDesc);

	m_vEvents.InsertAt (0, entry );
	if (m_vEvents.GetSize() > iLOG_MAX_ENTRIES)
		m_vEvents.RemoveAt(iLOG_MAX_ENTRIES-1);
}

// --------------------------------------------------------------------------
// AddShowError -- add the event to the log, then force display of the
//                 EventLog window
//

void TEventLog::AddShowError (TEventEntry::ESource  eSource,
							  LPCTSTR               desc,
							  LPCTSTR               extDesc)
{

	this->AddError (eSource, desc, extDesc);

	// present log window to user
	PostMainWndMsg (WM_COMMAND, ID_VIEW_EVENTLOG);
}

void TEventLog::Lock()
{
	EnterCriticalSection ( &m_critSect );
}

void TEventLog::Unlock()
{
	LeaveCriticalSection ( &m_critSect );
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

TProcessLog::TProcessLog()
{
	m_fActive = false;
	InitializeCriticalSection( &m_critSect );
}

TProcessLog::~TProcessLog()
{
	DeleteCriticalSection( &m_critSect );
}

bool TProcessLog::IsActive ()
{
	TEnterCSection mgr(&m_critSect);
	return m_fActive;
}

int TProcessLog::ActivateLog (bool fActive, LPCTSTR serverName)
{
	TEnterCSection mgr(&m_critSect);
	m_fActive = fActive;

	m_memFile.SeekToBegin ();
	m_memFile.SetLength ( 0 );
	if (fActive)
	{
		CTime now = CTime::GetCurrentTime();
		CString timeStamp
			= now.Format (" -- Log Started %A, %B %d, %Y - %H:%M:%S\r\n");

		m_memFile.Write (serverName, lstrlen(serverName));
		m_memFile.Write (timeStamp, timeStamp.GetLength());
	}
	return 0;
}

int TProcessLog::AddString (const CString& msg)
{
	TEnterCSection mgr(&m_critSect);

	if (m_fActive)
		m_memFile.Write (LPCTSTR(msg), msg.GetLength());

	return 0;
}

static int proclog_create_name (CString & name)
{
	CFileStatus sStatus;

	for (int i = 0; i < 1000; i++)
	{
		name.Format("GPROC%03d.TRC", i);
		if (!CFile::GetStatus (name, sStatus))
			return 0;
	}
	return 1;
}

int TProcessLog::SaveLog ()
{
	TEnterCSection mgr(&m_critSect);

	if (!m_fActive)
		return 0;

	m_memFile.SeekToBegin();
	DWORD dwLen = m_memFile.GetLength();

	CString         fname;
	CFileException  sExcept;
	CFile           sFile;

	if (proclog_create_name(fname))
		return 1;

	// open a real file
	if (!sFile.Open (fname, CFile::modeCreate | CFile::modeWrite, &sExcept))
	{
		sExcept.ReportError();
		return 1;
	}

	try
	{
		int   iChunk;
		TCHAR rcBuf[1024];
		DWORD dwBytesLeft = dwLen;

		while (dwBytesLeft > 0)
		{
			if (dwBytesLeft < sizeof(rcBuf) )
				iChunk = (int)dwBytesLeft;
			else
				iChunk = sizeof(rcBuf);

			m_memFile.Read ( rcBuf, iChunk );

			sFile.Write ( rcBuf, iChunk );

			dwBytesLeft -= iChunk;
		}
	}
	catch(CFileException *pFE)
	{
		pFE->ReportError();
		pFE->Delete();
		return 1;
	}
	return 0;
}

