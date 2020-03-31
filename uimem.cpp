/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: uimem.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:23  richard_wood
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
#include "uimem.h"
#include "fileutil.h"
#include "statvec.h"

#include "tglobopt.h"
#include "rgdir.h"
#include "rglaywn.h"
#include "rgui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions* gpGlobalOptions;

TUIMemory::~TUIMemory()
{
}

TUIMemory::TUIMemory()
{
	m_byDirtyBits = 0;
}

void TUIMemory::GetPath(TUIMemory::EDirType eDirType, TPath& tpath)
{
	switch (eDirType)
	{
	case DIR_DECODE_DEST:
		tpath = gpGlobalOptions->GetRegDirs()->m_DecodeDestination;
		break;
	case DIR_ATTACH_SRC:
		tpath = gpGlobalOptions->GetRegDirs()->m_AttachmentSource;
		break;
	case DIR_ARTICLE_DEST:
		tpath = gpGlobalOptions->GetRegDirs()->m_SaveArticleDestination;
		break;
	case DIR_PGP_HOME:
		tpath = gpGlobalOptions->GetRegDirs()->m_pgpHome;
		break;
	default:
		ASSERT(0);
		break;
	}
}

void TUIMemory::SetPath (TUIMemory::EDirType eDirType, const TPath& tpath)
{
	m_byDirtyBits |= DIRTY_DIR;
	switch (eDirType)
	{
	case DIR_DECODE_DEST:
		gpGlobalOptions->GetRegDirs()->m_DecodeDestination = LPCTSTR(tpath);
		break;
	case DIR_ATTACH_SRC:
		gpGlobalOptions->GetRegDirs()->m_AttachmentSource = LPCTSTR(tpath);
		break;
	case DIR_ARTICLE_DEST:
		gpGlobalOptions->GetRegDirs()->m_SaveArticleDestination = LPCTSTR(tpath);
		break;
	case DIR_PGP_HOME:
		gpGlobalOptions->GetRegDirs()->m_pgpHome = LPCTSTR(tpath);
		break;
	default:
		ASSERT(0);
		break;
	}
}

BOOL TUIMemory::GetWindowStatus (TUIMemory::EWinType eWinType, TWindowStatus& wndStatus)
{
	TRegLayoutWin*  pLW = gpGlobalOptions->GetRegLayoutWin();
	switch (eWinType)
	{
	case WND_POST:
		wndStatus = pLW->m_wndStatusPost; break;
	case WND_REPLY:
		wndStatus = pLW->m_wndStatusReply ; break;
	case WND_VIEW:
		wndStatus = pLW->m_wndStatusView ; break;
	case WND_MAIN:
		wndStatus = pLW->m_wndStatusMain ; break;
	case WND_BUGREPORT:
		wndStatus = pLW->m_wndStatusBugReport ; break;
	case WND_PRODUCTIDEA:
		wndStatus = pLW->m_wndStatusProductIdea ; break;
	case WND_SENDTOFRIEND:
		wndStatus = pLW->m_wndStatusSendToFriend ; break;
	default:
		ASSERT(0);
		break;
	}

	// check for emptiness
	if (0 == wndStatus.m_place.length)
		return FALSE;
	else
		return TRUE;
}

void TUIMemory::SetWindowStatus (TUIMemory::EWinType eWinType, const TWindowStatus& wndStatus)
{
	m_byDirtyBits |= DIRTY_LAYOUTWIN;
	TRegLayoutWin*  pLW = gpGlobalOptions->GetRegLayoutWin();
	switch (eWinType)
	{
	case WND_POST:
		pLW->m_wndStatusPost = wndStatus;
		break;
	case WND_REPLY:
		pLW->m_wndStatusReply = wndStatus;
		break;
	case WND_VIEW:
		pLW->m_wndStatusView = wndStatus;
		break;
	case WND_MAIN:
		pLW->m_wndStatusMain = wndStatus;
		break;
	case WND_BUGREPORT:
		pLW->m_wndStatusBugReport = wndStatus;
		break;
	case WND_PRODUCTIDEA:
		pLW->m_wndStatusProductIdea = wndStatus;
		break;
	case WND_SENDTOFRIEND:
		pLW->m_wndStatusSendToFriend = wndStatus;
		break;
	default:
		ASSERT(0);
		break;
	}
}

BOOL TUIMemory::FormulatePath (TPath& spec)
{
	return TFileUtil::UseProgramPath ( "TWINDOW.POS", spec );
}

int  TUIMemory::LayoutPaneCount(EMdiLayout eLayout)
{
	return 3;
}

// ------------------------------------------------------------------------
// act as a wrapper around RegUI
const CString & TUIMemory::Deprecated_GetViewFilterName()
{
	return gpGlobalOptions->GetRegUI()->Deprecated_GetViewFilterName();
}

// ------------------------------------------------------------------------
int  TUIMemory::GetViewFilterID ()
{
	return gpGlobalOptions->GetRegUI()->GetViewFilterID ();
}

void TUIMemory::SetViewFilterID (int iFilterID)
{
	m_byDirtyBits |= DIRTY_UI;
	gpGlobalOptions->GetRegUI()->SetViewFilterID ( iFilterID );
}

// ------------------------------------------------------------------------
// Called from ExitInstance ()
void TUIMemory::Save(void)
{
	if (m_byDirtyBits & DIRTY_DIR)
	{
		gpGlobalOptions->GetRegDirs()->Save();
		m_byDirtyBits &= ~DIRTY_DIR;
	}
	if (m_byDirtyBits & DIRTY_LAYOUTWIN)
	{
		gpGlobalOptions->GetRegLayoutWin()->Save();
		m_byDirtyBits &= ~DIRTY_LAYOUTWIN;
	}
	if (m_byDirtyBits & DIRTY_UI)
	{
		gpGlobalOptions->GetRegUI()->Save();
		m_byDirtyBits &= ~DIRTY_UI;
	}
}

BOOL TUIMemory::Load(void)
{
	// members are actually embedded in TGlobalOptions
	//   we are just a liason
	return TRUE;
}

void TUIMemory::SetLastGroup(int gid)
{
	m_byDirtyBits |= DIRTY_UI;
	gpGlobalOptions->GetRegUI()->SetLastGroupID(gid);
}
