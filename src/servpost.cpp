/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: servpost.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:51  richard_wood
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

// servpost.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "servpost.h"
#include "genutil.h"
#include "nameutil.h"
#include "helpcode.h"            // HID_OPTIONS_*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TServerPostPage property page

IMPLEMENT_DYNCREATE(TServerPostPage, CPropertyPage)

TServerPostPage::TServerPostPage() : CPropertyPage(TServerPostPage::IDD)
{

	m_replyTo = _T("");
	m_fGenMsgID = TRUE;

}

TServerPostPage::~TServerPostPage()
{
}

void TServerPostPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_SERVER_POST_ADVANCED_MSGID, m_fGenMsgID);
	DDV_MaxChars(pDX, m_replyTo, 128);

	DDX_EmailName(pDX, IDC_SERVER_POST_ADVANCED_REPLYTO, m_replyTo);
	DDV_EmailName(pDX, IDC_SERVER_POST_ADVANCED_REPLYTO, m_replyTo, FALSE);
}

BEGIN_MESSAGE_MAP(TServerPostPage, CPropertyPage)
	ON_WM_HELPINFO()

	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

BOOL TServerPostPage::OnHelpInfo(HELPINFO*)
{
	AfxGetApp () -> HtmlHelp((DWORD)"servers-posting-tab.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_NG_POSTING);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TServerPostPage::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
