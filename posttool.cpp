/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: posttool.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:41  richard_wood
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

// posttool.cpp -- toolbar for posting/mailing dialogs
// Remember that ToolBars send the commands to the Frame Window first

#include "stdafx.h"
#include "news.h"
#include "posttool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// arrays of IDs used to initialize control bars
static UINT BASED_CODE rMailID[] =
{
   // same order as in the bitmap 'IDB_COMPOSE_SMALL2'
   ID_SEPARATOR,           // for combo box (placeholder)
   ID_SEPARATOR,
   ID_SEPARATOR,           // MIME checkbox
   ID_SEPARATOR,
   ID_EDIT_CUT,
   ID_EDIT_COPY,
   ID_EDIT_PASTE,
   ID_SEPARATOR,
   ID_SEPARATOR,
   ID_CHECK_SPELLING,
   IDM_COMPOSE_ATTACH,
   ID_SEPARATOR,
   ID_SEPARATOR,
   IDM_POST_SEND,
   IDM_CANCEL,
   ID_SEPARATOR,
   ID_SEPARATOR,
   IDM_DRAFT,
};

static UINT BASED_CODE rPostID[] =
{
   // same order as in the bitmap 'IDB_COMPOSE_SMALL2'
   ID_SEPARATOR,           // for combo box (placeholder)
   ID_SEPARATOR,
   ID_SEPARATOR,           // will be checkbox
   ID_SEPARATOR,
   ID_SEPARATOR,
   ID_EDIT_CUT,
   ID_EDIT_COPY,
   ID_EDIT_PASTE,
   ID_SEPARATOR,
   ID_SEPARATOR,
   ID_CHECK_SPELLING,
   IDM_COMPOSE_ATTACH,
   ID_SEPARATOR,
   ID_SEPARATOR,
   IDM_POST_SEND,
   IDM_CANCEL,
   ID_SEPARATOR,
   ID_SEPARATOR,
   IDM_DRAFT,
};

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPostToolBar, TCompToolBar)
      
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TPostToolBar::Create(CWnd* pParentWnd)
{
   const int nDropHeight = 100;
   int iMailingButtons = sizeof (rMailID) / sizeof (UINT);
   int iPostingButtons = sizeof (rPostID) / sizeof (UINT);

   if (!CToolBar::Create (pParentWnd,
                          WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY,
                          idPostToolbar) ||
        !LoadBitmap (IDB_COMPOSE_SMALL2) ||
       !SetButtons (m_bMailing ? rMailID : rPostID,
                    m_bMailing ? iMailingButtons : iPostingButtons))
      {
      TRACE0("Failed to create post toolbar\n");
      return FALSE;       // fail to create
      }

   SIZE image;
   image.cx = image.cy = 24;
   SIZE butn;
   butn.cx = 31; butn.cy = 30;

   // switch to larget size
   SetSizes (butn, image);
   TCompToolBar::Create (pParentWnd,
                         0,           // index to place the ComboBox
                         2);
   return TRUE;
}
