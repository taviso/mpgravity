/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgstor.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/20 21:04:55  richard_wood
/*  Updated splash screen component so it works properly.
/*  Fixed crash from new splash screen.
/*  Updated setver setup dialogs, changed into a Wizard with a lot more verification and "user helpfulness".
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
/*  Revision 1.2  2008/09/19 14:51:46  richard_wood
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
#include "mplib.h"
#include "newsgrp.h"
#include "rgstor.h"
#include "regutil.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Disable warning about CException has no copy-ctor.
//  Must catch TException with a ptr or reference
#pragma warning( disable : 4671 4673 )

TRegStorage::STORSYNC TRegStorage::m_mapStorageSync[] =
{
	{"Nothing",       TNewsGroup::kNothing},
	{"Headers",       TNewsGroup::kHeadersOnly},
	{"Bodies",        TNewsGroup::kStoreBodies},
};

TRegStorage::TRegStorage()
{
	default_values();
}
TRegStorage::~TRegStorage()
{
}

int TRegStorage::Load()
{
	return TRegistryBase::Load ( GetGravityRegKey()+"Storage" );
}

int TRegStorage::Save()
{
	return TRegistryBase::Save ( GetGravityRegKey()+"Storage" );
}

void TRegStorage::read()
{
	BYTE buf[64];
	int  sz=sizeof(buf);
	LONG lRet;

	CString strStorageScheme;
	lRet = rgReadString("Scheme", buf, sz, strStorageScheme);
	int tot = sizeof(m_mapStorageSync)/sizeof(m_mapStorageSync[0]);
	for (--tot; tot >= 0; --tot)
	{
		if (0 == strStorageScheme.CompareNoCase( m_mapStorageSync[tot].name ) ) {
			m_kStorageMode = m_mapStorageSync[tot].kStorageMode;
			break;
		}
	}

	//   rgReadString ("DatabasePath", buf, sizeof (buf), m_databasePath);
	if (tot < 0)
		m_kStorageMode = TNewsGroup::kHeadersOnly;

	if (TNewsGroup::kNothing == m_kStorageMode)
		m_kStorageMode = TNewsGroup::kHeadersOnly;

	try
	{
		rgReadString ("MailError", buf, sizeof(buf), m_mailDumpFile);
	}
	catch(TException *pE)
	{
		pE->Delete();
	}
	catch(...)
	{
	}
}

void TRegStorage::write()
{
	LONG lRet;

	int tot = sizeof(m_mapStorageSync)/sizeof(m_mapStorageSync[0]);
	for (--tot; tot >= 0; --tot)
	{
		if (m_kStorageMode == m_mapStorageSync[tot].kStorageMode ) {
			lRet = rgWriteString ("Scheme", m_mapStorageSync[tot].name);
			break;
		}
	}

	if (tot < 0)
		lRet =  rgWriteString ("Scheme", "Headers");
}

void TRegStorage::default_values()
{
	m_kStorageMode = TNewsGroup::kHeadersOnly;
}

