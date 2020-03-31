/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postmdi.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:40  richard_wood
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

// postmdi.cpp -- MDI child window for posting/mailing windows

#include "stdafx.h"
#include "news.h"
#include "postmdi.h"
#include "posthdr.h"
#include "postbody.h"
#include "attview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(TPostMDIChildWnd, CMDIChildWnd)

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPostMDIChildWnd, CMDIChildWnd)
			// NOTE - the ClassWizard will add and remove mapping macros here.
	
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TPostMDIChildWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// create a splitter with 2 rows, 1 column
	if (!m_wndSplit1.CreateStatic(this, 2, 1))
   	{
	   TRACE0("Failed to CreateStaticSplitter\n");
	   return FALSE;
	   }

   m_wndSplit1.SetRowInfo (0,    // row zero
                           100,  // ideal height
                           10);  // minimum height

	// virt fn
	if (!CreateBodyView ( pContext ))
		return FALSE;

   // split the top pane
   if (!m_wndSplit2.CreateStatic(
       &m_wndSplit1,
       1, 2,
       WS_CHILD | WS_VISIBLE | WS_BORDER,
       m_wndSplit1.IdFromRowCol(0,0)))
      {
      TRACE0("Failed to create nested splitter\n");
      return FALSE;
      }

   m_wndSplit2.SetColumnInfo ( 0, 400, 10 );

   //  now create two views inside the nested splitter
   if (!m_wndSplit2.CreateView(0, 0,
       pContext->m_pNewViewClass, CSize(400, 100), pContext))
      {
      TRACE0("postmdi: Failed to create top view pane\n");
      return FALSE;
      }
   m_wndSplit2.SetColumnInfo ( 1, 200, 10 );
   if (!m_wndSplit2.CreateView(0,   // row 1
                               1,   // col 2
       RUNTIME_CLASS(TAttachView), CSize(200, 100), pContext))
      {
      TRACE0("postmdi: Failed to create top right pane\n");
      return FALSE;
      }

	SetActiveView((CView*)m_wndSplit2.GetPane(0,0));
	return TRUE;
}

// -------------------------------------------------------------------------
BOOL TPostMDIChildWnd::CreateBodyView(CCreateContext* pContext)
{
   if (!m_wndSplit1.CreateView(1, 0,
         RUNTIME_CLASS(TPostBody), CSize(50, 50), pContext))
      {
      TRACE0("postmdi: failed to create bottom view pane\n");
      return FALSE;
      }

	return TRUE;
}
