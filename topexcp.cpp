/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: topexcp.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:11  richard_wood
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
#include "topexcp.h"
#include "resource.h"
#include "autoprio.h"            // auto_prio object
#include "tmsgbx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern HWND ghwndMainFrame;

struct CTopLevelWorker
{
   void Init ();

   CString  strDownload;
   CString  strTasker;
   CString  strDatabase;
   CString  strSearch;
   CString  strUtility;
   CString  m_strCaption;

private:
   void my_load_string (CString & strError, LPCTSTR fmt, int iStringID);
};

// -------------------------------------------------------------------------
// Setup the strings at app init time
void CTopLevelWorker::Init ()
{
   CString str1, str2, strFormat;

   str1.LoadString (IDS_ERR_THREAD_CRASHED1);
   str2.LoadString (IDS_ERR_THREAD_CRASHED2);

   strFormat = str1 + str2;

   my_load_string (strDownload, strFormat, IDS_DOWNLOAD);
   my_load_string (strTasker,   strFormat, IDS_TASKER);
   my_load_string (strDatabase, strFormat, IDS_DATABASE);
   my_load_string (strSearch,   strFormat, IDS_SEARCH);
   my_load_string (strUtility,  strFormat, IDS_UTILITY);

   m_strCaption.LoadString (IDS_APPLICATION_TITLE);
}

// -------------------------------------------------------------------------
void CTopLevelWorker::my_load_string (CString &  strError,
                                      LPCTSTR    fmt,
                                      int        iStringID)
{
   CString strThread;

   strThread.LoadString (iStringID);
   strError.Format (fmt , LPCTSTR(strThread));
}

static CTopLevelWorker  gsTLW;

// -------------------------------------------------------------------------
void TopLevelExceptionInit (void)
{

   gsTLW.Init ();
}

// -------------------------------------------------------------------------
void TopLevelException (EThreadID  eThreadNameID)
{
   // my goal here is to have everything setup beforehand, so we don't have
   //   to make any calls to new() or malloc() or GlobalAlloc(), or the
   //   stack, since all of 'em could be corrupted.

   // boost priority of thread displaying msgbox
   auto_prio boost(auto_prio::kNormal);

   switch (eThreadNameID)
      {
      case kThreadDownload:
         NewsMessageBox  (CWnd::FromHandle (ghwndMainFrame), gsTLW.strDownload,
                          MB_OK, gsTLW.m_strCaption);
         break;

      case kThreadTasker:
         NewsMessageBox  (CWnd::FromHandle (ghwndMainFrame), gsTLW.strTasker,
                          MB_OK, gsTLW.m_strCaption);
         break;

      case kThreadDatabase:
         NewsMessageBox  (CWnd::FromHandle (ghwndMainFrame), gsTLW.strDatabase,
                          MB_OK, gsTLW.m_strCaption);
         break;

      case kThreadSearch:
         NewsMessageBox  (CWnd::FromHandle (ghwndMainFrame), gsTLW.strSearch,
                          MB_OK, gsTLW.m_strCaption);
         break;

      case kThreadUtility:
         NewsMessageBox  (CWnd::FromHandle (ghwndMainFrame), gsTLW.strUtility,
                          MB_OK, gsTLW.m_strCaption);
         break;
      }
}
