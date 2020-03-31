/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vcrrun.cpp,v $
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

// vcrrun.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "newsdoc.h"
#include "vcrrun.h"
#include "tasker.h"     // gpTasker->IsConnected
#include "custmsg.h"
#include "mainfrm.h"
#include "tmsgbx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TNewsTasker* gpTasker;

#define  VACT_SETIDLE      0x80
#define  VACT_DELJOB       0x40

#define TIMEREVENT_DELAY   33
#define TIMEREVENT_IDLE    44

#define WMU_EXIT_WINDOW    (WM_APP + 10)

// ------------------------------------------------------------------------
bool TVCRActionServer::Proceed (bool  fAdvance, bool fIdle)
{
   return fIdle && fAdvance;
}

// ------------------------------------------------------------------------
// Returns 0 for success.
int TVCRActionServer::DoYourJob (HWND hDlg)
{
   // switch over to this server and connect

   int iRet = gptrApp->ForceSwitchServer ( m_strServerName );

   // 1 means that the current server matched the target server

   if (0 == iRet || 1 == iRet)
      {
      if (!gpTasker->IsConnected())
         AfxGetMainWnd() -> PostMessage (WM_COMMAND, ID_FILE_SERVER_CONNECT);
      else
         {
         CMainFrame * pMF =  (CMainFrame*)    AfxGetMainWnd();

         HWND hDlg = pMF->LockVCRWindow ();
         if (hDlg)
            CWnd::FromHandle(hDlg)->PostMessage (WMU_VCR_NEXTSTEP);
         pMF->LockVCRWindow(false);
         }
      }

   return 0;
}

// ------------------------------------------------------------------------
bool TVCRActionGroup::Proceed ( bool fAdvance, bool fIdle)
{
   return fAdvance;
}

// ------------------------------------------------------------------------
// Returns 0 for success.
int TVCRActionGroup::DoYourJob (HWND hDlg)
{
   // submit this group to the pump

   gptrApp->GetGlobalNewsDoc()->getGroup ( getDescription() );

   return 0;
}

// -----------------------------------------------------------------------
// Action Exit
bool TVCRActionExit::Proceed (bool fAdvance, bool fIdle)
{
   // must be idle to disconnect and exit
   return fIdle && fAdvance;
}

TVCRActionExit::TVCRActionExit ()
{
   m_strDesc.LoadString (IDS_VCR_EXITGRAVITY);
}

int TVCRActionExit::DoYourJob (HWND hDlg)
{
   AfxGetMainWnd()->PostMessage (WM_CLOSE);
   return VACT_DELJOB;
}

// -----------------------------------------------------------------------
// Action Start
bool TVCRActionStart::Proceed (bool fAdvance, bool fIdle)
{
   return (COleDateTime::GetCurrentTime() > m_oleTime) ? true : false;
}

TVCRActionStart::TVCRActionStart (const COleDateTime & oleTime)
      : m_oleTime(oleTime)
{
   m_strDesc.LoadString (IDS_VCR_STARTAT);  // string has trailing space
   m_strDesc += m_oleTime.Format ();
}

int TVCRActionStart::DoYourJob (HWND hDlg)
{
   // delete self
   CWnd::FromHandle (hDlg)->PostMessage (WMU_VCR_NEXTSTEP);
   return VACT_SETIDLE;
}

/////////////////////////////////////////////////////////////////////////////
// TVCRRunDlg dialog

TVCRRunDlg::TVCRRunDlg(CWnd* pParent, ListVCRAction * pListAction,
                       bool          fSpecificTime,
                       COleDateTime  oleTime,
                       BOOL          fExitApp)
	: CDialog(TVCRRunDlg::IDD, pParent),
     m_pListAction (pListAction),
     m_fSpecificTime (fSpecificTime),
     m_oleTime (oleTime),
     m_fExitApp (fExitApp)
{
	// NOTE: the ClassWizard will add member initialization here
}

void TVCRRunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_LIST, m_lbx);
	
}

BEGIN_MESSAGE_MAP(TVCRRunDlg, CDialog)
		ON_WM_DESTROY()
	ON_WM_CREATE()
   ON_MESSAGE(WMU_SPLASH_GONE, DoInit)
   ON_MESSAGE(WMU_VCR_NEXTSTEP, OnStep)
   ON_MESSAGE(WMU_VCR_IDLE, OnNotifyIdle)
   ON_MESSAGE(WMU_VCR_BADSERVER, OnBadServer)
   ON_MESSAGE(WMU_EXIT_WINDOW, OnExitWindow)
	ON_WM_TIMER()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TVCRRunDlg message handlers

LRESULT TVCRRunDlg::DoInit(WPARAM, LPARAM)
{
   m_fIdle = true;
   m_fAdvance = true;  // set to go

   // fill m_lbx;

   POSITION pos = m_pListAction->GetHeadPosition();
   while (pos)
      {
      TVCRAction * pAct = m_pListAction->GetNext (pos);

      m_lbx.InsertString (m_lbx.GetCount(), pAct->getDescription());
      }

   // setup a 1 second timer
   m_hDelayStartTimer = SetTimer (TIMEREVENT_DELAY, 1000,  NULL);

   m_hWaitIdleTimer   = SetTimer (TIMEREVENT_IDLE,  1000, NULL);

   m_fStartCountingIdle = false;
   m_byCountIdle = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TVCRRunDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here

   CMainFrame * pMF =  (CMainFrame*)    AfxGetMainWnd();

   pMF->SetVCRWindow (NULL);

   // if they prematurely clicked OK, then clean up the actions here
   while (m_pListAction->GetCount() > 0)
      delete m_pListAction->RemoveHead ();

   delete m_pListAction;

   if (m_hDelayStartTimer)
      {
      KillTimer (m_hDelayStartTimer);
      m_hDelayStartTimer = NULL;
      }

   if (m_hWaitIdleTimer)
      {
      KillTimer (m_hWaitIdleTimer);
      m_hWaitIdleTimer = 0;
      }
}

// do the next thing
// wParam == fKeepTopJob
LRESULT TVCRRunDlg::OnStep (WPARAM fKeepTopJob, LPARAM)
{
   if (!fKeepTopJob)
      {
      delJob ();
      }

   if (0 == m_pListAction->GetCount())
      {
      // exit dialog
      return  my_exit();
      }

   // $ look out for lastgroup -> server2 transition

   POSITION pos =  m_pListAction->GetHeadPosition();

   TVCRAction * pAction = m_pListAction->GetNext (pos);

   m_fAdvance = true;
   if (pAction->Proceed ( m_fAdvance, m_fIdle ))
      {
      m_fIdle = m_fAdvance = false;

      if (0 == pAction->BeforeJob())
         {
         m_fStartCountingIdle = false;
         }

      int flags = pAction->DoYourJob (m_hWnd);

      if (flags & VACT_SETIDLE)
         m_fIdle = true;

      if (flags & VACT_DELJOB)
         delJob ();
      }
   return 0;
}

// ------------------------------------------------------------------------
void TVCRRunDlg::delJob ()
{
   ASSERT(m_lbx.GetCount() == m_pListAction->GetCount());
   m_lbx.DeleteString (0);

   TVCRAction* pAction = m_pListAction->RemoveHead ();
   if (pAction->AfterJob ())
      m_fStartCountingIdle = 1;

   delete pAction;
}

// ------------------------------------------------------------------------
LRESULT TVCRRunDlg::OnNotifyIdle (WPARAM, LPARAM)
{
   m_fIdle = true;
   return OnStep (TRUE, 0);
}

// ------------------------------------------------------------------------
LRESULT TVCRRunDlg::OnBadServer (WPARAM, LPARAM)
{
   // TODO:  skip this server, and any group jobs that follow it.
   POSITION   pos;
   TVCRAction* pAction;

   if (m_pListAction->GetCount() > 0)
      {
      pos = m_pListAction->GetHeadPosition ();
      pAction = m_pListAction->GetNext (pos);

      if (pAction->GetType() == kVCRServer)
         {
         m_ServerErrors.AddTail ( ((TVCRActionServer*)pAction)->m_strServerName );
         }
      }

   delJob ();

   while (m_pListAction->GetCount() > 0)
      {
      pos = m_pListAction->GetHeadPosition ();
      pAction = m_pListAction->GetNext (pos);

      // $$ this GetType() function is a gross hack

      if (pAction->GetType() == kVCRGroup)
         delJob();
      else
         break;
      }

   if (0 == m_pListAction->GetCount())
      {
      // exit dialog
      return my_exit();
      }

   return 0;
}

// ------------------------------------------------------------------------
LRESULT TVCRRunDlg::my_exit ()
{
   // show errors if there are any.

   if (m_ServerErrors.GetCount() > 0)
      {
      CString mesg;
      POSITION pos = m_ServerErrors.GetHeadPosition ();
      while (pos)
         {
         CString svrName;

         svrName.Format ("Could not connect to %s\n",
                          (LPCTSTR) m_ServerErrors.GetNext (pos));
         mesg += svrName;
         }

      NewsMessageBox (this, mesg, MB_OK | MB_ICONWARNING);
      }

   PostMessage (WMU_EXIT_WINDOW);

   return 0;
}

// ------------------------------------------------------------------------
LRESULT TVCRRunDlg::OnExitWindow (WPARAM, LPARAM)
{
   DestroyWindow ();
   return 0;
}

// ------------------------------------------------------------------------
void TVCRRunDlg::PostNcDestroy()
{
	//  free the c++ object, since this is a modeless dlg

   delete this;	
	CDialog::PostNcDestroy();
}

// ------------------------------------------------------------------------
int TVCRRunDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// equivalent to WM_INITDIALOG
   PostMessage (WMU_SPLASH_GONE)	;

	return 0;
}

// ------------------------------------------------------------------------
void TVCRRunDlg::OnCancel() { OnOK(); }

// ------------------------------------------------------------------------
void TVCRRunDlg::OnOK()
{
	// TODO: Add extra validation here
   CDialog::OnOK();

   DestroyWindow ();
}

// ------------------------------------------------------------------------
// our 1 second timer
void TVCRRunDlg::OnTimer(UINT nIDEvent)
{

   if (TIMEREVENT_DELAY == nIDEvent)
      {
   	bool fStart = false;

      if (m_pListAction->GetCount() == 0)
         return;

      // get first item

      POSITION pos =  m_pListAction->GetHeadPosition();

      TVCRAction * pAction = m_pListAction->GetNext (pos);

      if (pAction->Proceed (m_fAdvance, m_fIdle))
         {
         fStart = true;
         }

      if (fStart)
         {
         KillTimer (m_hDelayStartTimer);
         m_hDelayStartTimer = NULL;
         PostMessage (WMU_VCR_NEXTSTEP,  1);
         }

   	CDialog::OnTimer(nIDEvent);
      }

   else if (TIMEREVENT_IDLE == nIDEvent)
      {
      if (m_fStartCountingIdle)
         {
         // the goal here is we assume we are idle if we
         //  can check the state six times and find 0 jobs each time.

         if (gpTasker->IsSendingEmail())
            m_byCountIdle = 0;  // rset totally
         else
            {
            int nFetch = 0;

            // check number of queued pump jobs
            gpTasker->GetStatusData ( nFetch );

            if (0 == nFetch)
               m_byCountIdle++;
            else
               m_byCountIdle = 0;  // rset totally
            }

         if (m_byCountIdle >= 6)  // seconds
            {
            m_byCountIdle = 0;
            m_fStartCountingIdle = false;

            PostMessage (WMU_VCR_IDLE);   // msg to self
            }
         }
   	CDialog::OnTimer(nIDEvent);
      }
}
