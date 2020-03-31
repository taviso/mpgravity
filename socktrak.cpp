/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: socktrak.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:52  richard_wood
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

// ------------------------------------------------------------------------
// Module to log socket communication to a file
//
//
#include "stdafx.h"
#include "socktrak.h"
#include "fileUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
TSockTrack::TSockTrack()
{
	m_fRead = true;
	m_fReady = false;
	m_nChar = 0;
	m_pArc = 0;

	CString fname;
	if (create_name(fname))
		return;

	// open the file
	CFileException sExcept;
	if (!m_fl.Open (fname, CFile::modeCreate | CFile::modeWrite, &sExcept))
	{
		sExcept.ReportError ();
		return;
	}

	m_pArc = new CArchive (&m_fl, CArchive::store);

	m_fReady = true;
	CTime now = CTime::GetCurrentTime();
	fname = now.Format ("Trace Started %A, %B %d, %Y - %H:%M:%S\r\n");

	m_pArc->WriteString (LPCTSTR(fname));
}

// ------------------------------------------------------------------------
//
void TSockTrack::AppRead (PBYTE pData, int iCount)
{
	if (!m_fReady)
		return ;

	switch_mode (true);

	add_buffer (pData, iCount);

	dos_flush ();
}

// ------------------------------------------------------------------------
//
void TSockTrack::AppSend (PBYTE pData, int iCount)
{
	if (!m_fReady)
		return ;

	switch_mode (false);

	add_buffer (pData, iCount);

	dos_flush ();
}

// ------------------------------------------------------------------------
TSockTrack::~TSockTrack()
{
	write_remainder ();

	if (m_pArc)
	{
		m_pArc->Flush();
		m_pArc->Close();

		delete m_pArc;
		m_pArc = NULL;
	}

	m_fl.Flush();
	m_fl.Close();
	// close files
}

// ------------------------------------------------------------------------
// look for name that doesn't exist yet
int TSockTrack::create_name(CString & name)
{
	CFileStatus sStatus;

	for (int i = 0; i < 1000; i++)
	{
		// Put the trace file in APPDATA\Gravity\.
		name.Format(_T("%s\\Gravity\\GRAV%03d.TRC"), TFileUtil::GetAppData());
		if (!CFile::GetStatus (name, sStatus))
			return 0;
	}
	return 1;
}

// ------------------------------------------------------------------------
int TSockTrack::switch_mode (bool fSwitchingToRead)
{
	if (fSwitchingToRead == m_fRead)
		return 0;

	write_remainder ();

	if (fSwitchingToRead)
	{
		if (m_pArc)
			m_pArc->Write("\r\nread-\r\n", 9);  // start new section
		m_fRead = true;
	}
	else
	{
		// switching to send (we _were_ reading)
		if (m_pArc)
			m_pArc->Write("\r\nsend:\r\n", 9);  // start new section
		m_fRead = false;
	}

	return 0;
}

// ------------------------------------------------------------------------
int TSockTrack::add_buffer(PBYTE pData, int iCount)
{
	CString strLine;
	int i = 0;
	for (i = 0; i < iCount; i++)
	{
		rcBuf[m_nChar++] = *pData++;
		if (0 == (m_nChar % 16))
		{
			// we have a full 16 bytes

			// don't write out CRLF
			char rcNice[32];
			int k = 0;
			for (k = 0; k < 16; k++)
				rcNice[k] = isspace((BYTE)rcBuf[k]) ? ' ' : rcBuf[k];
			rcNice[k] = 0;

			strLine.Format(_T("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x   %s\r\n"),
				(BYTE) rcBuf[0],   (BYTE) rcBuf[1],
				(BYTE) rcBuf[2],   (BYTE) rcBuf[3],
				(BYTE) rcBuf[4],   (BYTE) rcBuf[5],
				(BYTE) rcBuf[6],   (BYTE) rcBuf[7],
				(BYTE) rcBuf[8],   (BYTE) rcBuf[9],
				(BYTE) rcBuf[10],  (BYTE) rcBuf[11],
				(BYTE) rcBuf[12],  (BYTE) rcBuf[13],
				(BYTE) rcBuf[14],  (BYTE) rcBuf[15],
				rcNice);
			if (m_pArc)
				m_pArc->WriteString (LPCTSTR(strLine));

			m_nChar = 0;
		}
	}
	return 0;
}

// write_remainder the <16 bytes we have in the buffer
int TSockTrack::write_remainder()
{
	TCHAR rcNice[20];
	if (0 == m_nChar)
		return 0;

	int k = 0;
	for (k = 0; k < m_nChar; k++)
		rcNice[k] = isspace(rcBuf[k]) ? _T(' ') : rcBuf[k];
	rcNice[k++] = _T('\r');
	rcNice[k++] = _T('\n');
	rcNice[k] = 0;

	CString s;
	int nWritten = 0;
	int i = 0;
	for (i = 0; i < m_nChar; i++)
	{
		s.Format (_T("%02x "), (BYTE) rcBuf[i]);
		m_pArc->WriteString (LPCTSTR(s));
		nWritten += 3;
	}
	CString t(_T(' '), 50-nWritten);
	if (m_pArc)
	{
		m_pArc->WriteString(t);

		m_pArc->WriteString (rcNice);
	}

	m_nChar = 0;
	return 0;
}

// ------------------------------------------------------------------------
void TSockTrack::dos_flush ()
{
	if (m_pArc)
		m_pArc->Flush ();
	m_fl.Flush ();
}
