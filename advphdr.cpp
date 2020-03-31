/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: advphdr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:09  richard_wood
/*  Updated for VS 2005
/*  */
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

// advphdr.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "advphdr.h"
#include "newsdb.h"
#include "nameutil.h"            // DDV_EmailName()
#include "arttext.h"             // fnFromLine()
#include "server.h"              // TNewsServer
#include "genutil.h"             // DDX_CEditStringList

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsDB*      gpStore;

/////////////////////////////////////////////////////////////////////////////
// TAdvancedPostHdr dialog

TAdvancedPostHdr::TAdvancedPostHdr(CWnd* pParent /*=NULL*/,
								   TNewsServer * pServer,
								   LONG lGroupID)
								   : CDialog(TAdvancedPostHdr::IDD, pParent),
								   m_pServer(pServer)
{
	m_dist = _T("");
	m_expires = _T("");
	m_followup = _T("");
	m_keywords = _T("");
	m_organization = _T("");
	m_replyto = _T("");
	m_sender = _T("");
	m_summary = _T("");
	m_from = _T("");

	m_replyto = m_pServer->GetReplyTo();
	m_dist    = gpStore->GetGlobalOptions()->GetDistribution();
	fnFromLine(m_from, lGroupID);
}

void TAdvancedPostHdr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_HDR_DISTRIBUTION, m_dist);
	DDX_Text(pDX, IDC_HDR_EXPIRES, m_expires);
	DDX_Text(pDX, IDC_HDR_FOLLOWUP, m_followup);
	DDX_Text(pDX, IDC_HDR_KEYWORDS, m_keywords);
	DDX_Text(pDX, IDC_HDR_ORGANIZATION, m_organization);
	DDX_Text(pDX, IDC_HDR_SUMMARY, m_summary);

	// class-wizard chokes on this
	DDX_EmailName(pDX, IDC_HDR_REPLYTO, m_replyto);
	DDV_EmailName(pDX, IDC_HDR_REPLYTO, m_replyto, FALSE);  // not required ADVPHDR.CPP
	DDX_EmailName(pDX, IDC_HDR_SENDER, m_sender);
	DDV_EmailName(pDX, IDC_HDR_SENDER, m_sender, FALSE);    // ADVP2 not required
	DDX_EmailName(pDX, IDC_HDR_FROM, m_from);
	DDV_EmailName(pDX, IDC_HDR_FROM, m_from, TRUE);

	DDX_CEditStringList (pDX, IDC_HDR_CUSTOM, m_sCustomHeaders);
}

BEGIN_MESSAGE_MAP(TAdvancedPostHdr, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TAdvancedPostHdr message handlers
