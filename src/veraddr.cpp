/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: veraddr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:25  richard_wood
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

// veraddr.cpp : Handles parsing errors for comma separated email addrs.
//               Dialog Box shows Original text vs. addrs we parsed OK
//

#include "stdafx.h"
#include "resource.h"         // IDD_VERIFY_ADDR
#include "veraddr.h"
#include "article.h"          // TEmailHeader

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TVerifyAddressDlg dialog

TVerifyAddressDlg::TVerifyAddressDlg(CWnd* pParent, const CString & origText,
									 const CStringList & errList, const TStringList & parsedList)
									 : CDialog(TVerifyAddressDlg::IDD, pParent),
									 m_strOriginalText(origText), m_lstErrors(errList), m_lstParsed(parsedList)
{
}

void TVerifyAddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_SHOWADDR, m_strOriginalText);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		CString txt;

		// append error to label
		CWnd* pStatic = GetDlgItem(IDC_STATIC_ERRDESC);
		pStatic->GetWindowText (txt);
		if (!m_lstErrors.IsEmpty()) {
			txt += m_lstErrors.GetHead();
			pStatic->SetWindowText (txt);
		}

		// listbox shows the addresses we parsed OK
		CListBox * plbxGoodAdr = static_cast<CListBox*>(GetDlgItem(IDC_GOODADR_LBX));
		int idx = 0;
		POSITION pos = m_lstParsed.GetHeadPosition ();
		while (pos)
		{
			const CString & rStr = m_lstParsed.GetNext (pos);
			plbxGoodAdr->InsertString(idx++, rStr);
		}
	}
}

BEGIN_MESSAGE_MAP(TVerifyAddressDlg, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TVerifyAddressDlg message handlers

// ------------------------------------------------------------------------
// Allow user to go back if there is any parsing error
// iTO =  0 means To:
//        1 means Cc:
//        2 means Bc:
// fCommit   if True, install data into MailHdr
// Returns TRUE to continue. FALSE to stop & allow correction
BOOL fnVerifyParse(CWnd * pWnd, const CString & comsepString, int iTO,
				   TEmailHeader * pMailHdr, BOOL fCommit)
{
	CStringList errList;
	TStringList outList;

	// define a ptr to a member function.  function has Default args .
	void (TEmailHeader::*pMemFn) (const CString & to, TStringList * pOutList, CStringList * pErrList);

	switch (iTO)
	{
	case 0:
		pMemFn = &TEmailHeader::ParseTo;
		break;
	case 1:
		pMemFn = &TEmailHeader::ParseCC;
		break;
	case 2:
		pMemFn = &TEmailHeader::ParseBCC;
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	// Try this out first.           tech note:  parens necessary?
	(pMailHdr->*pMemFn) ( comsepString, &outList, &errList );

	if (errList.IsEmpty())
	{
		if (fCommit)
		{
			// do it for real
			(pMailHdr->*pMemFn) ( comsepString, 0, 0 );
		}
		return TRUE;
	}
	else
	{
		TVerifyAddressDlg sDlg(pWnd,          // anchor
			comsepString,  // orig text
			errList,       // errors
			outList);      // parse output

		// dialog box shows Original text versus what we parsed out.
		sDlg.DoModal ();
	}
	return FALSE;
}

