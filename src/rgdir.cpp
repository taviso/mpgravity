/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgdir.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.4  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.3  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
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
#include "resource.h"
#include "mplib.h"
#include "rgdir.h"
#include "regutil.h"
#include "genutil.h"          // GetStartupDir
#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegDirectory::TRegDirectory()
{
	default_values();
}

TRegDirectory::~TRegDirectory()
{
}

int TRegDirectory::Load()
{
	CString regPath;
	make_regpath ( regPath );
	return TRegistryBase::Load ( regPath );
}

int TRegDirectory::Save()
{
	CString regPath;
	make_regpath ( regPath );
	return TRegistryBase::Save ( regPath );
}

int TRegDirectory::PreLoad()
{
	CString regPath;
	make_regpath ( regPath );
	return TRegistryBase::PreLoad ( regPath );
}

void TRegDirectory::read()
{
	BYTE buf[200];
	int sz = sizeof(buf);
	LONG lRet;
	lRet = rgReadString("Decode",   buf, sz, m_DecodeDestination);
	lRet = rgReadString("Dec2",     buf, sz, m_DecodeDest2);
	lRet = rgReadString("Dec3",     buf, sz, m_DecodeDest3);

	lRet = rgReadString("Attach",   buf, sz, m_AttachmentSource);
	lRet = rgReadString("SaveFile", buf, sz, m_SaveArticleDestination);
	lRet = rgReadString("PGP",      buf, sz, m_pgpHome);

	lRet = rgReadString("OtherViewer", buf, sz, m_strOtherViewer);
}

void TRegDirectory::write()
{
	LONG lRet;
	lRet = rgWriteString("Decode",    m_DecodeDestination);
	lRet = rgWriteString("Dec2",      m_DecodeDest2);
	lRet = rgWriteString("Dec3",      m_DecodeDest3);

	lRet = rgWriteString("Attach",    m_AttachmentSource);
	lRet = rgWriteString("SaveFile",  m_SaveArticleDestination);
	lRet = rgWriteString("PGP",       m_pgpHome);

	lRet = rgWriteString("OtherViewer", m_strOtherViewer);
}

// ------------------------------------------------------------------------
// read any hints laid down by the Install Script
void TRegDirectory::preload()
{
	BYTE buf[200];
	int sz = sizeof(buf);
	try
	{
		rgReadString("Decode",     buf, sz, m_DecodeDestination);
	}
	catch (TException *pE)
	{
		pE->Delete();
	}
}

void TRegDirectory::default_values()
{
	m_DecodeDestination = TFileUtil::GetAppData() + "\\Gravity\\" + "download";
}

void TRegDirectory::make_regpath (CString & rStr)
{
	rStr = GetGravityRegKey() + "Dirs";
}
