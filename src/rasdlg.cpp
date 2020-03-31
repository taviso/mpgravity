/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rasdlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.2  2008/09/19 14:51:42  richard_wood
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

// rasdlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "rasdlg.h"
#include "ras.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TRasDialDlg dialog

TRasDialDlg::TRasDialDlg(CWnd* pParent /*=NULL*/)
: CDialog(TRasDialDlg::IDD, pParent)
{
}

void TRasDialDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BUT_REDIAL, m_butRedial);
}

BEGIN_MESSAGE_MAP(TRasDialDlg, CDialog)
	ON_BN_CLICKED(IDC_BUT_REDIAL, OnButRedial)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUT_NO, OnButNo)
	ON_WM_TIMER()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TRasDialDlg message handlers

// ----------------------------------------------------------
BOOL TRasDialDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	Sleep(1000);

	// TODO: Add extra initialization here
	m_uTimer = SetTimer (666, 1000, NULL);
	m_iSecsLeft = 31;

	update_button_text ();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------
void TRasDialDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	if (m_uTimer)
		KillTimer (m_uTimer);
}

// ----------------------------------------------------------
void TRasDialDlg::update_button_text ()
{
	if (--m_iSecsLeft <= 0)
	{
		OnButRedial ();
		return;
	}

	CString str;

	str.Format (IDS_UTIL_REDIAL, m_iSecsLeft);

	m_butRedial.SetWindowText ( str );   
}

// ----------------------------------------------------------
void TRasDialDlg::OnButNo() 
{
	m_fRedial = false;
	CDialog::OnOK();
}

// ----------------------------------------------------------
void TRasDialDlg::OnButRedial() 
{
	m_fRedial = true;
	CDialog::OnOK();
}

void TRasDialDlg::OnTimer(UINT nIDEvent) 
{
	update_button_text ();   
	CDialog::OnTimer(nIDEvent);
}
