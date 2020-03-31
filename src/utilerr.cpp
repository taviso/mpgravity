/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: utilerr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

// utilerr.cpp : implementation file
//

#include "stdafx.h"
#include "utilerr.h"
#include "utilsize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TNNTPErrorDialog dialog

TNNTPErrorDialog::TNNTPErrorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TNNTPErrorDialog::IDD, pParent)
{

	m_strWhen = _T("");
	m_strReason = _T("");


   m_hTimer = 0;
   m_fTimer = false;
   m_iSeconds = 0;
}

void TNNTPErrorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_TEXT_WHEN, m_strWhen);
	DDX_Text(pDX, IDC_TEXT_REASON, m_strReason);
}

BEGIN_MESSAGE_MAP(TNNTPErrorDialog, CDialog)
		ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TNNTPErrorDialog message handlers

BOOL TNNTPErrorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

   // standard icon, do not free
   m_hIcon = LoadIcon(NULL, IDI_EXCLAMATION);

   if (m_fTimer && m_iSeconds)
      m_hTimer = SetTimer (666, 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Gotta draw the Icon by hand
void TNNTPErrorDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Locate our placment window - steal his position

   CWnd * pHidden = GetDlgItem(IDC_STATIC_PLACEMENT);
   if (pHidden && m_hIcon)
      {
      CRect rct;

      Utility_GetPosParent(this, pHidden, rct);

      // draw icon just Under the placement window
      dc.DrawIcon (rct.left, rct.bottom, m_hIcon);
      }

	// Do not call CDialog::OnPaint() for painting messages
}

void TNNTPErrorDialog::OnDestroy() 
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
   if (m_hTimer)
      KillTimer (m_hTimer);
}

void TNNTPErrorDialog::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (--m_iSeconds <= 0)
      PostMessage (WM_COMMAND, IDOK);
	CDialog::OnTimer(nIDEvent);
}
