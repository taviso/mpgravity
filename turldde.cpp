/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: turldde.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:20  richard_wood
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
#include "turldde.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Disable warning about CException has no copy-ctor.
//  Must catch TException with a ptr or reference
#pragma warning( disable : 4671 4673 )

TUrlDde & TUrlDde::operator=(const TUrlDde &rhs)
{
	m_regSubKey       = rhs.m_regSubKey;

	m_application     = rhs.m_application;
	m_fUseDDE         = rhs.m_fUseDDE;
	m_service         = rhs.m_service;
	m_topic           = rhs.m_topic;
	m_transactionType = rhs.m_transactionType;
	m_item            = rhs.m_item;
	m_data            = rhs.m_data;
	return *this;
}

TUrlDde::TUrlDde(LPCTSTR subkey)
: m_regSubKey(subkey)
{
	default_values();
}

int TUrlDde::Load()
{
	DWORD dwAction;
	CString aggregate = GetGravityRegKey();
	aggregate += "URL\\";
	aggregate += m_regSubKey;

	LONG lRet = UtilRegCreateKey(
		aggregate,
		&m_hkKey,
		&dwAction);
	if (ERROR_SUCCESS != lRet)
	{
		throw(new TException(IDS_ERR_REGISTRY, kError));
		return 0;
	}
	if (REG_CREATED_NEW_KEY == dwAction)
		default_values();
	else
		read();

	RegCloseKey ( m_hkKey );
	return 0;
}

int TUrlDde::Save()
{
	CString aggregate = GetGravityRegKey();
	aggregate += "URL\\";
	aggregate += m_regSubKey;

	LONG lRet = UtilRegOpenKey(
		aggregate,
		&m_hkKey, KEY_WRITE);
	if (ERROR_SUCCESS != lRet)
	{
		DWORD disposition;
		lRet = UtilRegCreateKey(
			aggregate,
			&m_hkKey, &disposition);
		if (ERROR_SUCCESS != lRet)
		{
			throw(new TException(IDS_ERR_REGISTRY, kError));
			return 0;
		}
	}

	write();

	RegCloseKey ( m_hkKey );
	return 0;
}

void TUrlDde::default_values()
{
	m_transactionType = kRequest;
	m_fUseDDE = FALSE;
}

void TUrlDde::write()
{
	LONG lRet;

	lRet = rgWriteNum("transaction", m_transactionType);
	lRet = rgWriteNum("dde",         m_fUseDDE);

	lRet = rgWriteString("app",     m_application);
	lRet = rgWriteString("service", m_service);
	lRet = rgWriteString("topic",   m_topic);
	lRet = rgWriteString("item",    m_item);
	lRet = rgWriteString("data",    m_data);
}

void TUrlDde::read()
{
	LONG lRet;
	BYTE buf[80];
	int  size = sizeof(buf);

	lRet = rgReadLong("transaction", buf, size, m_transactionType);
	lRet = rgReadLong("dde",         buf, size, m_fUseDDE);

	lRet = rgReadString("app",     buf, size, m_application);
	lRet = rgReadString("service", buf, size, m_service);
	lRet = rgReadString("topic",   buf, size, m_topic);
	lRet = rgReadString("item",    buf, size, m_item);
	lRet = rgReadString("data",    buf, size, m_data);
}

