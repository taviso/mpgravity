/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tabedit.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:57  richard_wood
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

// tabedit.cpp : implementation file
//

#include "stdafx.h"
//#include "news.h"
#include "tabedit.h"
//#include "postpfrm.h"            // TPostPopFrame
//#include "folpfrm.h"             // TFollowPopFrame
//#include "rplypfrm.h"            // TFollowPopFrame
#include "resource.h"            // ID_TAB_TO_EDIT

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TTabEdit

TTabEdit::TTabEdit()
{
}

TTabEdit::~TTabEdit()
{
}

BEGIN_MESSAGE_MAP(TTabEdit, CEdit)
		ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TTabEdit message handlers

void TTabEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	
}

UINT TTabEdit::OnGetDlgCode()
{
	UINT baseFlags = CEdit::OnGetDlgCode();

   baseFlags &= ~DLGC_HASSETSEL;
   baseFlags |= DLGC_WANTTAB;
   return baseFlags;
}

void TTabEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (VK_TAB == nChar)
      {
      if (GetKeyState(VK_SHIFT) < 0)
         {
         // send this weird message and do normal shift-tab behavior
         GetParent()->SendMessage(WM_NEXTDLGCTL, TRUE, 0);
         }
      else
         {
         // go upward until we see the ancestor of class TPostPopFrame,
         //    then send him a message
         CWnd *pSplitter = GetParent()->GetParent();

         //while (pWnd && !pWnd -> IsKindOf (RUNTIME_CLASS (TPostPopFrame)) &&
         //       !pWnd -> IsKindOf (RUNTIME_CLASS (TFollowPopFrame)) &&
         //       !pWnd -> IsKindOf (RUNTIME_CLASS (TReplyPopFrame)))
         //       pWnd = pWnd -> GetOwner ();

         if (!pSplitter)
            return;

         // this should be a splitter Wnd. It should route the message
         //  UP until we get to a T-XXX-PopFrame
         pSplitter -> SendMessage (WM_COMMAND, ID_TAB_TO_EDIT);
         }
      return;
	   }
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
