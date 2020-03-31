/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TSigPage.cpp,v $
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

// TSigPage.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "TSigPage.h"
#include "tmsgbx.h"
#include "custsig.h"
#include "tglobopt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TSigPage property page

IMPLEMENT_DYNCREATE(TSigPage, CPropertyPage)

TSigPage::TSigPage() : CPropertyPage(TSigPage::IDD)
{

	m_fCustomSig = FALSE;
}

TSigPage::~TSigPage()
{
}

void TSigPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CMB_NGSIGFILE, m_cmb);
	DDX_Check(pDX, IDC_CBX_NGSIGFILE, m_fCustomSig);


	if (FALSE==pDX->m_bSaveAndValidate)
	{
		TNewsSigList* pList = gpGlobalOptions->GetpSignatureList();
		bool fSet=false;

		for (int i = 0; i < pList->GetSize(); i++)
		{
			TCustomSignature & pSig = pList->GetAt (i);

			int idx = m_cmb.AddString (pSig.GetShortName());
			if (pSig.GetShortName() == m_strShortName)
			{
				m_cmb.SetCurSel (idx);
				fSet=true;
			}
		}
		if (!fSet && m_cmb.GetCount() > 0)
			m_cmb.SetCurSel (0);
		EnableControls (m_fCustomSig);
	}
	else
	{
		int idx = m_cmb.GetCurSel();
		if (idx >= 0)
			m_cmb.GetLBText (idx, m_strShortName);
		else
			m_strShortName.Empty();
	}
}

BEGIN_MESSAGE_MAP(TSigPage, CPropertyPage)
	ON_BN_CLICKED(IDC_CBX_NGSIGFILE, OnUseSignatureFile)
	ON_WM_HELPINFO()
	ON_NOTIFY(PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSigPage message handlers

void TSigPage::OnUseSignatureFile() 
{
	EnableControls (IsDlgButtonChecked (IDC_CBX_NGSIGFILE));
}

void TSigPage::EnableControls(BOOL fEnable)
{
	GetDlgItem (IDC_STATIC_FNAME)->EnableWindow(fEnable);
	GetDlgItem (IDC_CMB_NGSIGFILE)->EnableWindow(fEnable);
}

BOOL TSigPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AfxGetApp()->HtmlHelp((DWORD)"group-properties-signature.html", HH_DISPLAY_TOPIC);
	return 1;
}

afx_msg void TSigPage::OnPSNHelp(NMHDR *, LRESULT *)
{
	OnHelpInfo(NULL);
}
