/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TrcDlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:09  richard_wood
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

// TrcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "trcDlg.h"

#include "tglobopt.h"
#include "rgswit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern bool gfLogSocketDataToFile;              // from newssock.cpp

/////////////////////////////////////////////////////////////////////////////
// TTraceDlg dialog

TTraceDlg::TTraceDlg(CWnd* pParent /*=NULL*/)
: CDialog(TTraceDlg::IDD, pParent)
{
	m_fGroupCmd = FALSE;
}

void TTraceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate)
	{
		m_fGroupCmd = gpGlobalOptions->GetRegSwitch()->GetLogGroupCmds();
	}

	DDX_Control(pDX, IDC_STOP_TRACE, m_butStop);
	DDX_Control(pDX, IDC_START_TRACE, m_butStart);
	DDX_Check(pDX, IDC_TRACE_GROUPCMD, m_fGroupCmd);

	if (!pDX->m_bSaveAndValidate)
	{
		m_butStart.EnableWindow (!gfLogSocketDataToFile);
		m_butStop.EnableWindow (gfLogSocketDataToFile);
	}
	else
	{
		gpGlobalOptions->GetRegSwitch()->SetLogGroupCmds(m_fGroupCmd);
	}
}

BEGIN_MESSAGE_MAP(TTraceDlg, CDialog)
	ON_BN_CLICKED(IDC_START_TRACE, OnStartTrace)
	ON_BN_CLICKED(IDC_STOP_TRACE, OnStopTrace)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TTraceDlg message handlers

void TTraceDlg::OnStartTrace() 
{
	gfLogSocketDataToFile = true;
	UpdateData (FALSE);
}

void TTraceDlg::OnStopTrace() 
{
	gfLogSocketDataToFile = false;
	UpdateData (FALSE);
}
