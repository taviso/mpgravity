/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TOptionsMark.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:08  richard_wood
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

// TOptionsMark.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "TOptionsMark.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TOptionsMark property page

IMPLEMENT_DYNCREATE(TOptionsMark, CPropertyPage)

TOptionsMark::TOptionsMark() : CPropertyPage(TOptionsMark::IDD)
{
	
	m_fMR_display = FALSE;
	m_fMR_filesave = FALSE;
	m_fMR_followup = FALSE;
	m_fMR_forward = FALSE;
	m_fMR_reply = FALSE;
	
}

TOptionsMark::~TOptionsMark()
{
}

void TOptionsMark::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	
	DDX_Check(pDX, IDC_MR_DISPLAY, m_fMR_display);
	DDX_Check(pDX, IDC_MR_FILESAVE, m_fMR_filesave);
	DDX_Check(pDX, IDC_MR_FOLLOWUP, m_fMR_followup);
	DDX_Check(pDX, IDC_MR_FORWARD, m_fMR_forward);
	DDX_Check(pDX, IDC_MR_REPLY, m_fMR_reply);
	
}

BEGIN_MESSAGE_MAP(TOptionsMark, CPropertyPage)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TOptionsMark message handlers

BOOL TOptionsMark::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"about.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TOptionsMark::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
