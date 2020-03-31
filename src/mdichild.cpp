/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mdichild.cpp,v $
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

// mdichild.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "mdichild.h"
#include "thrdlvw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TNewsMDIChildWnd

IMPLEMENT_DYNCREATE(TNewsMDIChildWnd, CMDIChildWnd)

TNewsMDIChildWnd::TNewsMDIChildWnd()
{
}

TNewsMDIChildWnd::~TNewsMDIChildWnd()
{
}

BEGIN_MESSAGE_MAP(TNewsMDIChildWnd, CMDIChildWnd)
			// NOTE - the ClassWizard will add and remove mapping macros here.
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TNewsMDIChildWnd message handlers

BOOL TNewsMDIChildWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	  // create a splitter with 1 row, 2 columns
	  if (!m_wndSplitter.CreateStatic(this, 1, 2))
	  {
	  	  TRACE0("Failed to CreateStaticSplitter\n");
	  	  return FALSE;
	  }

	  // add the first splitter pane - the default view in column 0
     //  the document is CNewsDoc
	  if (!m_wndSplitter.CreateView(0, 0,
	  	  pContext->m_pNewViewClass, CSize(130, 50), pContext))
	  {
	  	  TRACE0("Failed to create first pane\n");
	  	  return FALSE;
	  }

	  // add the Second splitter pane - an threadlist view in column 1
     //  the document is _again_ CNewsDoc
	  if (!m_wndSplitter.CreateView(0, 1,
	  	  RUNTIME_CLASS(TThreadListView), CSize(0, 0), pContext))
	  {
	  	  TRACE0("Failed to create second pane\n");
	  	  return FALSE;
	  }

	  // activate the input view
	  SetActiveView((CView*)m_wndSplitter.GetPane(0,0));

	//return CMDIChildWnd::OnCreateClient(lpcs, pContext);
   return TRUE;
}
