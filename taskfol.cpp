/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: taskfol.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:58  richard_wood
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

// TAskFol.cpp : dialog box asks the user what to do when encountering
//               Followup-To: poster

#include "stdafx.h"
#include "news.h"
#include "TAskFol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TAskFollowup dialog

TAskFollowup::TAskFollowup(CWnd* pParent /*=NULL*/)
	: CDialog(TAskFollowup::IDD, pParent)
{


   m_eAction = kCancel;
}

void TAskFollowup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TAskFollowup, CDialog)
		ON_BN_CLICKED(IDC_ASKFOLLOW_EMAIL, OnAskfollowEmail)
	ON_BN_CLICKED(IDC_ASKFOLLOW_POST, OnAskfollowPost)
	ON_BN_CLICKED(IDC_ASKFOLLOW_POSTCC, OnAskfollowPostcc)
	ON_WM_PAINT()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TAskFollowup message handlers

void TAskFollowup::OnAskfollowEmail() 
{
	m_eAction = kSendEmail;
   EndDialog(1);
}

void TAskFollowup::OnAskfollowPost() 
{
   m_eAction = kPost;
   EndDialog(1);
}

void TAskFollowup::OnAskfollowPostcc() 
{
	m_eAction = kPostWithCC;
   EndDialog(1);
}

void TAskFollowup::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

   HICON hicn = AfxGetApp()->LoadStandardIcon(IDI_QUESTION);
   
   dc.DrawIcon (m_ptIconOrigin, hicn);

	// Do not call CDialog::OnPaint() for painting messages
}

BOOL TAskFollowup::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// get the location for the icon, and
   // then destroy the static

   CRect rctIcon;
   CWnd * pStatic = GetDlgItem(IDC_ASKFOLLOW_ORIGIN);
	pStatic->GetWindowRect ( &rctIcon );
   pStatic->DestroyWindow ();

   m_ptIconOrigin.x = rctIcon.left;
   m_ptIconOrigin.y = rctIcon.top;

   // we are going to use this during WM_PAINT
   ScreenToClient (&m_ptIconOrigin);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
