/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: Humble.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/10/03 08:21:06  richard_wood
/*  Tidying up code and comments.
/*
/*  Revision 1.2  2008/09/19 14:51:06  richard_wood
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

// Humble.cpp : implementation file
//

#include "stdafx.h"
#include "Humble.h"

extern HWND ghwndMainFrame;         // this is a global variable

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHumbleThread

IMPLEMENT_DYNCREATE(CHumbleThread, CWinThread)

CHumbleThread::CHumbleThread()
{
	m_fSelfAllocated = false;
}

CHumbleThread::~CHumbleThread()
{
}

//-------------------------------------------------------------------------
//
int CHumbleThread::ExitInstance()
{
	if (m_fSelfAllocated && m_pMainWnd)
	{
		m_pMainWnd->Detach ();
		delete m_pMainWnd;
		m_pMainWnd = NULL;
	}
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHumbleThread, CWinThread)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHumbleThread message handlers

//-------------------------------------------------------------------------
// Due to server switching code, the tasker is started before the FrameWnd
// is up.  So set ptr up on demand
CWnd* CHumbleThread::GetMainWnd()
{
	// setup m_pMainWnd

	if (NULL == m_pMainWnd)
	{
		// this stuff is cleaned up in ExitInstance
		m_pMainWnd = new CWnd;
		m_pMainWnd->Attach (ghwndMainFrame);
		m_fSelfAllocated = true;
	}

	return CWinThread::GetMainWnd();
}
