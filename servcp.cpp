/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: servcp.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:50  richard_wood
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
#include "servcp.h"
#include "server.h"
#include "newsdb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsDB*      gpStore;

static CCriticalSection gcritCountedServer;
static CString          gstrCountedServerName;

#if defined(_DEBUG)
void TServerCountedPtr::Dump(CDumpContext& dc) const
{
	CObject::Dump( dc );
	dc << "=TServerCountedPtr\n" ;
}
#endif

TServerCountedPtr::TServerCountedPtr(TNewsServer* pServer)
	: m_pRep(pServer)
{
	ASSERT(pServer);
	if (m_pRep)
		m_pRep->AddRef ();
}

// dtor removes 1 reference
TServerCountedPtr::~TServerCountedPtr()
{
	if (m_pRep)
		m_pRep->Release ();
}

// ------------------------------------------------------------------------
// C-function. We do no reference counting
TNewsServer * GetCountedActiveServer ()
{
	gcritCountedServer.Lock ();

	// based on the CString, get the pointer
	TNewsServer * pServer =
		gpStore->GetServerByName (gstrCountedServerName);

	gcritCountedServer.Unlock ();

	return pServer;
}

// ------------------------------------------------------------------------
void SetCountedActiveServerName (const CString& serverName)
{
	gcritCountedServer.Lock ();

	gstrCountedServerName = serverName;

	gcritCountedServer.Unlock ();
}

// ------------------------------------------------------------------------
void GetCountedActiveServerName (CString& serverName)
{
	gcritCountedServer.Lock ();

	serverName = gstrCountedServerName;

	gcritCountedServer.Unlock ();
}

// ------------------------------------------------------------------------
CString GetCountedActiveServerName (void)
{
	gcritCountedServer.Lock ();

	CString serverName = gstrCountedServerName;

	gcritCountedServer.Unlock ();

	return serverName;
}

// ------------------------------------------------------------------------
bool IsActiveServer ()
{
	bool fRet = false;
	gcritCountedServer.Lock ();
	fRet = !gstrCountedServerName.IsEmpty();
	gcritCountedServer.Unlock ();
	return fRet;
}

// ------------------------------------------------------------------------
TServerCountedPtr::TServerCountedPtr (void)
{
	m_pRep = GetCountedActiveServer ();
	if (m_pRep)
		m_pRep->AddRef ();
}

// ------------------------------------------------------------------------
// C-function. We do no reference counting
bool TestCountedActiveServer ()
{
	bool fOK = false;
	gcritCountedServer.Lock ();

	// based on the CString, get the pointer
	TNewsServer * pServer =
		gpStore->GetServerByName (gstrCountedServerName);

	fOK = pServer->GetRefcount() <= 2;

	gcritCountedServer.Unlock ();

	return fOK;
}
