/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: wndstat.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:26  richard_wood
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
#include "wndstat.h"
#include "superstl.h"            // istrstream, ...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TWindowStatus::TWindowStatus()
{
	ZeroMemory (&m_place, sizeof(m_place));
	ZeroMemory (&m_sizeSlide1, sizeof(m_sizeSlide1));
	ZeroMemory (&m_sizeSlide2, sizeof(m_sizeSlide2));
}

TWindowStatus::~TWindowStatus()
{
}

TWindowStatus& TWindowStatus::operator=(const TWindowStatus& rhs)
{
	if (this == &rhs)
		return *this;
	m_place     = rhs.m_place;
	m_sizeSlide1= rhs.m_sizeSlide1;
	m_sizeSlide2= rhs.m_sizeSlide2;

	return *this;
}

void TWindowStatus::Stringize(CString& s)
{
	s.Empty();
	//          L  F  S mx my Mx My <RECT-----> cx cy cx cy
	s.Format ("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		m_place.length,
		m_place.flags,
		m_place.showCmd,
		m_place.ptMinPosition.x,
		m_place.ptMinPosition.y,
		m_place.ptMaxPosition.x,
		m_place.ptMaxPosition.y,
		m_place.rcNormalPosition.left,
		m_place.rcNormalPosition.top,
		m_place.rcNormalPosition.right,
		m_place.rcNormalPosition.bottom,
		m_sizeSlide1.cx,
		m_sizeSlide1.cy,
		m_sizeSlide2.cx,
		m_sizeSlide2.cy);
}

void TWindowStatus::StringInit(CString& s)
{
	int len = s.GetLength();
	LPTSTR p = s.GetBuffer(len);
	istrstream iss(p);
	iss >> m_place.length
		>>  m_place.flags
		>>   m_place.showCmd
		>>  m_place.ptMinPosition.x
		>>   m_place.ptMinPosition.y
		>>     m_place.ptMaxPosition.x
		>>       m_place.ptMaxPosition.y;
	iss >> m_place.rcNormalPosition.left
		>>   m_place.rcNormalPosition.top
		>>     m_place.rcNormalPosition.right
		>>       m_place.rcNormalPosition.bottom;
	iss >> m_sizeSlide1.cx >> m_sizeSlide1.cy;
	iss >> m_sizeSlide2.cx >> m_sizeSlide2.cy;
	s.ReleaseBuffer();
	s.Empty();
}

