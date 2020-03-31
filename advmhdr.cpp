/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: advmhdr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
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

// advmhdr.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "advmhdr.h"
#if !defined (NEWSDB_H)
#include "newsdb.h"
#endif
#include "nameutil.h"               // DDV_EmailName()
#include "article.h"                // TEmailHeader
#include "arttext.h"                // fnFromLine()
#include "veraddr.h"                // fnVerifyParse
#include "server.h"                 // TNewsServer

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsDB*      gpStore;

/////////////////////////////////////////////////////////////////////////////
// TAdvMailHeaders dialog

TAdvMailHeaders::TAdvMailHeaders(CWnd* pParent,
								 TNewsServer* pServer,
								 TEmailHeader * pMailHdr,
								 LONG lGroupID)
								 : CDialog(TAdvMailHeaders::IDD, pParent), m_pServer(pServer)
{
	// use ptr in DoDataExchange
	m_pMailHdr = pMailHdr;

	m_keywords = _T("");
	m_replyto = _T("");
	m_sender = _T("");
	m_from = _T("");
	m_cc = _T("");
	m_bcc = _T("");

	m_replyto = m_pServer->GetReplyTo();
	fnFromLine (m_from, lGroupID, FALSE /* fAddressFirst */, TRUE /* fMailing */);
}

void TAdvMailHeaders::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// (Step #1) import data from object to data members
	if (!pDX->m_bSaveAndValidate)
	{
		m_from     = m_pMailHdr->GetFrom ();
		m_keywords = m_pMailHdr->GetKeywords ();
		m_replyto  = m_pMailHdr->GetReplyTo ();
		m_sender   = m_pMailHdr->GetSender ();
		m_pMailHdr->GetCCText (m_cc);
		m_pMailHdr->GetBCCText (m_bcc);

	}

	// (Step #2) move from data members to Line Edit Boxes
	// class-wizard chokes on this
	DDX_EmailName(pDX, IDC_HDR_REPLYTO, m_replyto);
	DDV_EmailName(pDX, IDC_HDR_REPLYTO, m_replyto, FALSE);
	DDX_EmailName(pDX, IDC_HDR_SENDER, m_sender);
	DDV_EmailName(pDX, IDC_HDR_SENDER, m_sender, FALSE);
	DDX_EmailName(pDX, IDC_HDR_FROM, m_from);
	DDV_EmailName(pDX, IDC_HDR_FROM, m_from, TRUE);

	DDX_Text(pDX, IDC_HDR_KEYWORDS, m_keywords);

	DDX_Text(pDX, IDC_HDR_CC, m_cc);

	// set last focus for pDX->Fail?
	if (pDX->m_bSaveAndValidate)
		if (!fnVerifyParse (this, m_cc, 1, m_pMailHdr, FALSE))
			pDX->Fail (); // abort

	DDX_Text(pDX, IDC_HDR_BCC, m_bcc);
	if (pDX->m_bSaveAndValidate)
		if (!fnVerifyParse (this, m_bcc, 2, m_pMailHdr, FALSE))
			pDX->Fail (); // abort
}

BEGIN_MESSAGE_MAP(TAdvMailHeaders, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TAdvMailHeaders message handlers

void TAdvMailHeaders::ExportDlgData (TEmailHeader * pMailHdr)
{
	pMailHdr->SetFrom      ( m_from );
	pMailHdr->SetKeywords  ( m_keywords );
	pMailHdr->SetReplyTo   ( m_replyto );
	pMailHdr->SetSender    ( m_sender );
	pMailHdr->ParseCC      ( m_cc );
	pMailHdr->ParseBCC     ( m_bcc );
}
