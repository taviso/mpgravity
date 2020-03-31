/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: update.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2010/01/07 17:35:52  richard_wood
/*  Updates for 2.9.14.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.3  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.2  2008/09/19 14:52:24  richard_wood
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
#include "rgsys.h"
#include "tglobopt.h"
#include "update.h"
#include "urlsppt.h"
#include <afxinet.h>
#include "custmsg.h"
#include "genutil.h"
#include "helpcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

/////////////////////////////////////////////////////////////////////////////
// CCheckingForUpdateDlg dialog
//CCheckingForUpdateDlg::CCheckingForUpdateDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CCheckingForUpdateDlg::IDD, pParent)
//{}
//
//void CCheckingForUpdateDlg::DoDataExchange(CDataExchange* pDX)
//{
//	CDialog::DoDataExchange(pDX);
//}
//
//BEGIN_MESSAGE_MAP(CCheckingForUpdateDlg, CDialog)
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateDlg property page
IMPLEMENT_DYNCREATE(CUpdateDlg, CPropertyPage)

CUpdateDlg::CUpdateDlg() : CPropertyPage(CUpdateDlg::IDD)
{
	m_fCheckForUpdates = FALSE;
}

CUpdateDlg::~CUpdateDlg()
{}

void CUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_FOR_UPDATES, m_fCheckForUpdates);
}

BEGIN_MESSAGE_MAP(CUpdateDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_CHECKNOW, OnCheckNow)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateDlg message handlers
void CUpdateDlg::OnCheckNow()
{
	::AfxGetMainWnd()->PostMessage(WMU_CHECKFORUPDATE);
}

BOOL CUpdateDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	return TRUE;
}

BOOL CUpdateDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"maintenance-check-for-updates.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_UPDATES);
	return 1;
}

afx_msg void CUpdateDlg::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
