/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: warndlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:26  richard_wood
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
#include "resource.h"
#include "warndlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
// pass in message string
BOOL  WarnWithCBX (LPCTSTR    pWarning,
				   BOOL *     pTurnOffWarning,
				   CWnd *     pParentWnd, /* = NULL */
				   BOOL       iNotifyOnly, /* = FALSE */
				   BOOL       bDefaultToNo,  /* = FALSE */
				   BOOL       bDisableCheckbox) /* = FALSE */
{
	BOOL fTurnOffWarning = FALSE;
	int dlgRet = internal_do_warndlg (pWarning, &fTurnOffWarning,
		pParentWnd, iNotifyOnly, bDefaultToNo,
		bDisableCheckbox);
	if (IDOK == dlgRet)
	{
		*pTurnOffWarning = fTurnOffWarning;
		return TRUE;
	}
	else
	{
		*pTurnOffWarning = FALSE;
		return FALSE;
	}
}

// ------------------------------------------------------------------------
BOOL  WarnWithCBX_Ex (UINT       resourceID,
					  BOOL      *pTurnOffWarning,
					  CWnd      *pParentWnd,
					  BOOL       iNotifyOnly,
					  BOOL       bDefaultToNo,
					  BOOL       bDisableCheckbox)
{
	CString strWarning; strWarning.LoadString(resourceID);
	BOOL fTurnOffWarning = FALSE;
	int dlgRet = internal_do_warndlg (strWarning, &fTurnOffWarning,
		pParentWnd, iNotifyOnly, bDefaultToNo,
		bDisableCheckbox);

	// this is slightly different. Even if user exits with "No", honor
	//   the warning boolean

	*pTurnOffWarning = fTurnOffWarning;

	return (IDOK == dlgRet);
}

// ------------------------------------------------------------------------
// Entry point with StringID
BOOL  WarnWithCBX (UINT       resourceID,
				   BOOL *     pTurnOffWarning,
				   CWnd *     pParentWnd, /* = NULL */
				   BOOL       iNotifyOnly, /* = FALSE */
				   BOOL       bDefaultToNo,  /* = FALSE */
				   BOOL       bDisableCheckbox) /* = FALSE */
{
	CString  warning;
	warning.LoadString (resourceID);

	// call version that  takes a String
	return WarnWithCBX (warning, pTurnOffWarning, pParentWnd, iNotifyOnly,
		bDefaultToNo);
}

// ------------------------------------------------------------------------
static int internal_do_warndlg (LPCTSTR pWarning,
								BOOL      *pTurnOffWarning,
								CWnd      *pParentWnd,
								BOOL       iNotifyOnly,
								BOOL       bDefaultToNo,
								BOOL       bDisableCheckbox)
{
	TWarnWithCbx   warnDlg (pParentWnd);

	warnDlg.m_warning = pWarning;
	warnDlg.m_iNotifyOnly = iNotifyOnly;
	warnDlg.m_bDefaultToNo = bDefaultToNo;
	warnDlg.m_bDisableCheckbox = bDisableCheckbox;
	int dlgRet;
	HANDLE hThread = GetCurrentThread();
	int    oldPrio = GetThreadPriority(hThread);
	try
	{
		SetThreadPriority ( hThread, THREAD_PRIORITY_NORMAL );
		dlgRet = warnDlg.DoModal();
	}
	catch(...)
	{
		SetThreadPriority (hThread, oldPrio);
		throw;
	}
	SetThreadPriority (hThread, oldPrio);

	// always pass this back
	*pTurnOffWarning = warnDlg.m_fTurnOffWarning;

	return dlgRet;
}

/////////////////////////////////////////////////////////////////////////////
TWarnWithCbx::TWarnWithCbx(CWnd* pParent /*=NULL*/)
: CDialog(TWarnWithCbx::IDD, pParent)
{

	m_warning = _T("");
	m_fTurnOffWarning = FALSE;


	m_bDefaultToNo = FALSE;
	m_bDisableCheckbox = FALSE;
}

void TWarnWithCbx::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_WARN_WITHCBX_TEXT, m_warning);
	DDX_Check(pDX, IDC_WARN_WITHCBX_TURNOFFWARNING, m_fTurnOffWarning);
}

BEGIN_MESSAGE_MAP(TWarnWithCbx, CDialog)
		ON_WM_PAINT()
	ON_BN_CLICKED(IDC_WARN_WITHCBX_TURNOFFWARNING, OnWarnWithcbxTurnoffwarning)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void TWarnWithCbx::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	HICON hExclamation = LoadIcon (NULL, IDI_EXCLAMATION);
	dc.DrawIcon (15, 15, hExclamation);
	// Do not call CDialog::OnPaint() for painting messages
}

BOOL TWarnWithCbx::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_iNotifyOnly) {

		// get rid of the "no" button
		GetDlgItem (IDCANCEL) -> DestroyWindow ();

		// change the "yes" button's text to "OK"
		CWnd *pYes = GetDlgItem (IDOK);
		ASSERT (pYes);
		CString str; str.LoadString (IDS_UTIL_OK);
		pYes -> SetWindowText (str);

		// also, center the "OK" button
		RECT sRect, sDlgRect;
		pYes -> GetWindowRect (&sRect);
		ScreenToClient (&sRect);
		GetWindowRect (&sDlgRect);
		int iX = (sDlgRect.right - sDlgRect.left) / 2 -
			(sRect.right - sRect.left) / 2;
		pYes -> SetWindowPos (0, iX, sRect.top, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	if (m_bDisableCheckbox)
		GetDlgItem (IDC_WARN_WITHCBX_TURNOFFWARNING) -> EnableWindow (FALSE);

	if (m_bDefaultToNo)
		GotoDlgCtrl (GetDlgItem (IDCANCEL));

	// return TRUE unless you set the focus to a control
	return m_bDefaultToNo ? FALSE : TRUE;
}

void TWarnWithCbx::OnWarnWithcbxTurnoffwarning() 
{
	// Set this even if the User exits with cancel
	m_fTurnOffWarning = 
		(IsDlgButtonChecked (IDC_WARN_WITHCBX_TURNOFFWARNING)) 
		? TRUE : FALSE;
}
