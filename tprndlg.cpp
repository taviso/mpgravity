/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tprndlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:14  richard_wood
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

// tprndlg.cpp -- dialog to view print jobs

#include "stdafx.h"
#include "tprndlg.h"          // this file's prototypes
#include "tprnthrd.h"         // gpPrintThread
#include "tprnq.h"            // TPrintQueue
#include "resource.h"         // IDS_*
#include "tglobopt.h"         // WarnOnDeleteBinary(), ...
#include "rgui.h"             // SaveUtilDlg(), ...
#include "critsect.h"         // TEnterCSection
#include "tprnthrd.h"         // gpsPrintDialog
#include "genutil.h"          // ComposeSizePos()

extern TGlobalOptions *gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TPrintDialog::TPrintDialog (CWnd *pParent /* = NULL */)
	: TUtilityDialog (IDD_FACTORY_PRINT, pParent)
{
	// NOTE: the ClassWizard will add member initialization here
}

// -------------------------------------------------------------------------
BOOL TPrintDialog::OnInitDialog ()
{
   m_pThread = gpPrintThread;

   m_psQueue = (TUtilityQueue *) &m_sQueue;
   m_psCompleted = (TUtilityQueue *) &m_sCompleted;
   m_psCurrent = (TUtilityQueue *) &m_sCurrent;

   m_strWindowTitle.LoadString (IDS_PRINT_TITLE);

   // remove the unwanted buttons
   GetDlgItem (IDC_VIEW_BINARY) -> DestroyWindow ();
   GetDlgItem (IDC_DELETE_BINARY_FROM_LIST) -> DestroyWindow ();
   GetDlgItem (IDC_DELETE_BINARY_FROM_DISK) -> DestroyWindow ();
   GetDlgItem (IDC_DELETE_ALL_FROM_DISK) -> DestroyWindow ();
   GetDlgItem (IDC_RENAME) -> DestroyWindow ();

   // set the window's position
   CString strSizePos = gpGlobalOptions -> GetRegUI () -> GetPrintSizePos ();
   int dx, dy, x, y;
   if (!DecomposeSizePos (strSizePos, dx, dy, x, y))
      SetWindowPos (NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

   // this sets the window's size & column sizes, etc...
   BOOL RC = TUtilityDialog::OnInitDialog ();

   return RC;
}

// -------------------------------------------------------------------------
void TPrintDialog::GetCompletedJobStatus (CString &str)
{
   m_psCompleted -> GetCurrentLineStatus (str);
}

// -------------------------------------------------------------------------
void TPrintDialog::SaveHeaderSizes ()
{
   int riSizes [8];

   // get header widths
   int iWidth = 0;
   int i;
   for (i = 0; i < 2; i++)
      riSizes [iWidth++] = m_sQueue.GetColumnWidth (i);
   for (i = 0; i < 3; i++)
      riSizes [iWidth++] = m_sCurrent.GetColumnWidth (i);
   for (i = 0; i < 3; i++)
      riSizes [iWidth++] = m_sCompleted.GetColumnWidth (i);

   // save widths
   TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
   pRegUI -> SaveUtilHeaders ("Print", riSizes, sizeof riSizes / sizeof (int));
}

// -------------------------------------------------------------------------
void TPrintDialog::SetHeaderSizes ()
{
   // get widths
   int riSizes [8];
   TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
   if (pRegUI -> LoadUtilHeaders ("Print", riSizes,
      sizeof riSizes / sizeof (int)))
      return;

   // set header widths
   int iWidth = 0;
   int i;
   for (i = 0; i < 2; i++)
      m_sQueue.SetColumnWidth (i, riSizes [iWidth++]);
   for (i = 0; i < 3; i++)
      m_sCurrent.SetColumnWidth (i, riSizes [iWidth++]);
   for (i = 0; i < 3; i++)
      m_sCompleted.SetColumnWidth (i, riSizes [iWidth++]);
}

// -------------------------------------------------------------------------
void TPrintDialog::SetGlobalPointer ()
{
   EnterCriticalSection (&gcritPrintDialog);
   gpsPrintDialog = this;
   LeaveCriticalSection (&gcritPrintDialog);
};

// -------------------------------------------------------------------------
void TPrintDialog::ClearGlobalPointer ()
{
   EnterCriticalSection (&gcritPrintDialog);
   gpsPrintDialog = NULL;
   LeaveCriticalSection (&gcritPrintDialog);
};

// -------------------------------------------------------------------------
void TPrintDialog::RememberPos (CRect & rct)
{
   WINDOWPLACEMENT wp;

   GetWindowPlacement (&wp);

   rct = wp.rcNormalPosition;

   CString strSizePos = ComposeSizePos (0, 0, rct.left, rct.top);
   gpGlobalOptions -> GetRegUI () -> SetPrintSizePos (strSizePos);
}
