/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rglaywn.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:45  richard_wood
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
#include "rglaywn.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegLayoutWin::TRegLayoutWin()
{
	default_values();
}
TRegLayoutWin::~TRegLayoutWin()
{
}

int TRegLayoutWin::Load()
{
	// inherited
	// use preprocessor strcat
	return TRegistryBase::Load ( GetGravityRegKey()+"Layout\\Win" );
}

int TRegLayoutWin::Save()
{
	// inherited
	// preprocessor strcat
	return TRegistryBase::Save ( GetGravityRegKey()+"Layout\\Win" );
}

void TRegLayoutWin::read()
{
	LONG lRet;
	BYTE buf[64];
	int sz = sizeof(buf);
	CString s;
	try {
		lRet = rgReadString("Post", buf, sz, s);
		m_wndStatusPost.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}
	try {
		lRet = rgReadString("Reply", buf, sz, s);
		m_wndStatusReply.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadString("View", buf, sz, s);
		m_wndStatusView.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadString("Main", buf, sz, s);
		m_wndStatusMain.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadString("Report", buf, sz, s);
		m_wndStatusBugReport.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadString("Idea", buf, sz, s);
		m_wndStatusProductIdea.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}

	try {
		lRet = rgReadString("Friend", buf, sz, s);
		m_wndStatusSendToFriend.StringInit(s);
	}
	catch (TException *pE) {pE->Delete();}
}

void TRegLayoutWin::write()
{
	LONG lRet;
	CString s;
	m_wndStatusPost.Stringize(s);
	lRet = rgWriteString("Post", s);

	m_wndStatusReply.Stringize(s);
	lRet = rgWriteString("Reply", s);

	m_wndStatusView.Stringize(s);
	lRet = rgWriteString("View", s);

	m_wndStatusMain.Stringize(s);
	lRet = rgWriteString("Main", s);

	m_wndStatusBugReport.Stringize(s);
	lRet = rgWriteString("Report", s);

	m_wndStatusProductIdea.Stringize(s);
	lRet = rgWriteString("Idea", s);

	m_wndStatusSendToFriend.Stringize(s);
	lRet = rgWriteString("Friend", s);
}

void TRegLayoutWin::default_values()
{
}

