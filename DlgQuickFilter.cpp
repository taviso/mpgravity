/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: DlgQuickFilter.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:06  richard_wood
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

// DlgQuickFilter.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DlgQuickFilter.h"
#include "custmsg.h"
#include "tsearch.h"
#include "tmsgbx.h"
#include "mplib.h"            // TException

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgQuickFilter dialog

DlgQuickFilter::DlgQuickFilter(CWnd* pParent /*=NULL*/)
: CDialog(DlgQuickFilter::IDD, pParent)
{

	m_fFrom = FALSE;
	m_fSubj = TRUE;
	m_fRegExpr = FALSE;
	m_strSearchText = _T("");
}

void DlgQuickFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CBX_INFROM, m_fFrom);
	DDX_Check(pDX, IDC_CBX_INSUBJ, m_fSubj);
	DDX_Check(pDX, IDC_CBX_REGEXP, m_fRegExpr);
	DDX_Text(pDX, IDC_EDIT1, m_strSearchText);
}

BEGIN_MESSAGE_MAP(DlgQuickFilter, CDialog)
		ON_WM_CLOSE()
	ON_MESSAGE(WMU_QUICK_FILTER, OnQuickFilter)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgQuickFilter message handlers

LRESULT DlgQuickFilter::OnQuickFilter(WPARAM, LPARAM)
{
	CWnd * pWnd = GetDlgItem (IDC_EDIT1);
	if (pWnd)
		pWnd->SetFocus ();
	return 0;
}

void DlgQuickFilter::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	DestroyWindow ();
	CDialog::OnClose();
}

void DlgQuickFilter::OnOK()
{
	UpdateData (TRUE);

	TQuickFilterData * pData =  new TQuickFilterData;

	pData->m_fSubj   = m_fSubj;
	pData->m_fFrom   = m_fFrom;
	pData->m_fRE     = m_fRegExpr;
	pData->m_strText = m_strSearchText;

	// test for invalid reg-expression 

	try
	{
		TSearch      search;

		if (pData->m_fRE)
			search.SetPattern (pData->m_strText, FALSE, TSearch::RE);
		else
			search.SetPattern (pData->m_strText, FALSE, TSearch::NON_RE);

		AfxGetMainWnd()->PostMessage (WMU_QUICK_FILTER, 0, (LPARAM) pData);
	}
	catch(TException *pE)
	{
		int iStringID = IDS_QFLT_BADSEARCHTEXT;
		if (pData->m_fRE)
			iStringID = IDS_QFLT_BADRETEXT;

		NewsMessageBox (this, iStringID, MB_OK | MB_ICONWARNING);
		pE->Delete();
	}

	//CDialog::OnOK(); keep dialog visible
}
