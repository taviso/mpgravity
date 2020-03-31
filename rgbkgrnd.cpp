/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgbkgrnd.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:44  richard_wood
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
#include "tglobopt.h"
#include "rgswit.h"
#include "sysclr.h"
#include "rgbkgrnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions* gpGlobalOptions;
extern TSystem         gSystem;

TBackgroundColors & TBackgroundColors::operator=(const TBackgroundColors &rhs)
{
	m_flatviewBackground  = rhs.m_flatviewBackground;
	m_artviewBackground   = rhs.m_artviewBackground;
	m_newsgroupBackground = rhs.m_newsgroupBackground;
	return *this;
}

TBackgroundColors::TBackgroundColors()
{
	default_values();

	// setup the table that controls i/o
	m_itemCount = 4;
	m_pTable = new TableItem[m_itemCount];
	SetItem (m_pTable + 0, "Subjects",  kREGRGB, &m_flatviewBackground);
	SetItem (m_pTable + 1, "Articles",  kREGRGB, &m_artviewBackground);
	SetItem (m_pTable + 2, "Newsgroups",kREGRGB, &m_newsgroupBackground);
	SetItem (m_pTable + 3, "Threads",   kREGRGB, &m_threadviewBackground);
}

TBackgroundColors::~TBackgroundColors ()
{
	delete [] m_pTable;
}

int TBackgroundColors::Load()
{
	CString aggregate = GetGravityRegKey();
	aggregate += "Backgrounds";

	return TRegistryBase::Load ( aggregate );
}

int TBackgroundColors::Save()
{
	CString aggregate = GetGravityRegKey();
	aggregate += "Backgrounds";

	return TRegistryBase::Save ( aggregate );
}

void TBackgroundColors::default_values()
{
	// 192,192,192 is Grey
	// 255,255,255 is White
	m_flatviewBackground  =
		m_artviewBackground   =
		m_newsgroupBackground =
		m_threadviewBackground = RGB(255,255,255);
}

void TBackgroundColors::write()
{
	LONG lRet = rgWriteTable (m_itemCount, m_pTable);
}

void TBackgroundColors::read()
{
	LONG lRet = rgReadTable (m_itemCount, m_pTable);
}

COLORREF TBackgroundColors::GetEffectiveArtviewBackground()
{
	COLORREF crf;

	if (gpGlobalOptions->GetRegSwitch()->IsDefaultArticleBG())
		crf = gSystem.Window();
	else
		crf = m_artviewBackground;

	return crf;
}

