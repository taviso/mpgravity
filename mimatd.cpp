/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mimatd.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:30  richard_wood
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

// mimatd.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "mimatd.h"

#include "fileutil.h"
#include "globals.h"
#include "uimem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TMimeAttachmentDlg dialog

TMimeAttachmentDlg::TMimeAttachmentDlg(CWnd* pParent /*=NULL*/)
: CDialog(TMimeAttachmentDlg::IDD, pParent)
{

	m_contentDesc = _T("");
	m_fileName = _T("");
	m_contentType = _T("");
	m_encoding = _T("");
}

void TMimeAttachmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MIME_DESCRIPTION, m_contentDesc);
	DDX_CBString(pDX, IDC_MIME_TYPE, m_contentType);
	DDX_CBString(pDX, IDC_MIME_ENCODING, m_encoding);

	DDX_Text(pDX, IDC_FILENAME, m_fileName);
	DDV_Filename(pDX, IDC_FILENAME, m_fileName);

	if (pDX->m_bSaveAndValidate)
	{
		CComboBox* pCB = (CComboBox*) GetDlgItem(IDC_MIME_ENCODING);
		int icursel = pCB->GetCurSel();
		if (CB_ERR != icursel)
			m_eCode = (TMime::ECode) pCB->GetItemData(icursel);
	}
}

BEGIN_MESSAGE_MAP(TMimeAttachmentDlg, CDialog)
		ON_BN_CLICKED(IDC_FOLD, OnFold)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TMimeAttachmentDlg message handlers

BOOL TMimeAttachmentDlg::OnInitDialog()
{
	// bitmap buttons - load images
	m_btnFold.AutoLoad ( IDC_FOLD, this );

	int i;
	CString str;
	// fill MIME content types
	CComboBox* pCB = (CComboBox*) GetDlgItem (IDC_MIME_TYPE);
	// Fill with Content Type
	for (i = int(IDS_MIMECONTENT_START+1);
		i < int(IDS_MIMECONTENT_END);
		++i)
	{
		if (str.LoadString(i))
			pCB->AddString ( str );
	}

	MIME_Pair riPairs[] = { IDS_ENCODE_BASE64,       TMime::CODE_BASE64,
		IDS_ENCODE_UU,           TMime::CODE_UU
		/* IDS_ENCODE_XX,           TMime::CODE_XX, */
	};
	// fill with encoding choices
	pCB = (CComboBox*) GetDlgItem (IDC_MIME_ENCODING);
	for (i = 0;
		i < sizeof(riPairs)/sizeof(riPairs[0]);
		++i)
	{
		if (str.LoadString(riPairs[i].str_id))
		{
			int add_idx = pCB->AddString (str);
			pCB->SetItemData(add_idx,  DWORD( riPairs[i].code ));
		}
	}
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// browse for a file
void TMimeAttachmentDlg::OnFold()
{
	OSVERSIONINFO verinfo;
	verinfo.dwOSVersionInfoSize = sizeof(verinfo);
	GetVersionEx( &verinfo );

	DWORD dwMoreFlags = 0;
	// Win95 Explorer dialog boxes have null terminated strings
	if (VER_PLATFORM_WIN32_WINDOWS == verinfo.dwPlatformId)
	{
		dwMoreFlags |= 0x80000;  // OFN_EXPLORER;
	}
	CString result;

	// Try to load initial dir
	TPath   initialDir;
	gpUIMemory->GetPath(TUIMemory::DIR_ATTACH_SRC, initialDir);

	if (TFileUtil::GetInputFilename(&result,
		this,
		IDS_ATTACHMENT_TITLE,
		&initialDir,
		dwMoreFlags,
		IDS_ATTACHMENT_FILTER))
	{
		GetDlgItem (IDC_FILENAME)->SetWindowText ( result );

		TPath spec = result;
		TPath dir ;
		spec.GetDirectory (dir);

		gpUIMemory->SetPath (TUIMemory::DIR_ATTACH_SRC, dir);
	}
}

void AFXAPI DDV_Filename(CDataExchange* pDX, int nIDC, CString& str)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	HWND hWndParent = GetParent ( hWndCtrl );
	if (pDX->m_bSaveAndValidate)
	{
		CWnd * pAnchor = CWnd::FromHandlePermanent( hWndParent );
		CString fname;
		int len = GetWindowTextLength (hWndCtrl);
		if (0 == len)
		{
			NewsMessageBox(pAnchor, IDS_ERR_FILENONAME, MB_OK | MB_ICONWARNING);
			pDX->Fail();
		}
		else
		{
			LPTSTR p = fname.GetBuffer(len);
			GetWindowText (hWndCtrl, p, len+1);
			fname.ReleaseBuffer();

			if (FALSE == TFileUtil::FileExist(fname))
			{
				CString msg;
				AfxFormatString1(msg, IDS_ERR_FILENOTEXIST, LPCTSTR(fname));

				NewsMessageBox(pAnchor, msg, MB_OK | MB_ICONWARNING);
				pDX->Fail();
			}
			if (!TFileUtil::FileAccess(fname, TFileUtil::kREAD))
			{
				NewsMessageBox(pAnchor, IDS_ERR_FILEOPEN, MB_OK | MB_ICONWARNING);
				pDX->Fail();
			}
		}
	}
	else
	{
		// do nothing
	}
}

