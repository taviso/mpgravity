/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: utilrout.cpp,v $
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

#include "stdafx.h"
#include "utilrout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void ProbeCmdUI (CCmdTarget* pTarget, CMenu* pSubmenu)
{
   CWnd* pMain = AfxGetMainWnd();
   CCmdUI state;
   state.m_pMenu = pSubmenu;
   state.m_pParentMenu = pSubmenu;

   state.m_nIndexMax = pSubmenu->GetMenuItemCount();
   for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
      {
      state.m_nID = pSubmenu->GetMenuItemID(state.m_nIndex);
		if (0 == state.m_nID)
			continue; // menu separator or invalid cmd - ignore it
      if ((UINT)-1 == state.m_nID)
         {
#if 1
         // possibly a popup menu
         CMenu* pminiMenu = pSubmenu->GetSubMenu(state.m_nIndex);
         if (pminiMenu)
            {
            // recurse!
            ProbeCmdUI (pTarget, pminiMenu);
            // if all children are grey, grey the stub.
            int subTotal = pminiMenu->GetMenuItemCount();
            BOOL fAllGrey = TRUE;
            for (int j = 0; j < subTotal; ++j)
               {
               UINT uSubId = pminiMenu->GetMenuItemID(j);
               if ((0 == uSubId) || (uSubId == (UINT) -1))
                  continue;
               UINT uRet = pminiMenu->GetMenuState(j, MF_BYPOSITION);
               if (!(uRet & MF_GRAYED))
                  {
                  fAllGrey = FALSE;
                  break;
                  }
               }
            pSubmenu->EnableMenuItem(
                           state.m_nIndex,
                           MF_BYPOSITION |
                            (fAllGrey ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));
            }
#else
			// possibly a popup menu, route to first item of that popup
			state.m_pSubMenu = pSubmenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			   {
				continue;       // first item of popup can't be routed to
			   }
			state.DoUpdate(pTarget, FALSE);    // popups are never auto disabled
#endif
         }
      else
         {
         //  a normal menu item
         // see if CNewsView has anything to say about it...
         state.m_pSubMenu = NULL;
         state.DoUpdate (pMain, FALSE);
         }
      }
}

void SubitemModifyMenu (BOOL fOn, CCmdUI* pCmdUI)
{
   // first menuitem of submenu takes responsibility
   if (pCmdUI->m_pMenu != NULL)
	   {
		if (pCmdUI->m_pSubMenu)
         {
   	   ASSERT(pCmdUI->m_nIndex < pCmdUI->m_nIndexMax);
	      pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION |
		   	   (fOn ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
         }
	   }
}

// ------------------------------------------------------------------------
//
BOOL PostMainWndMsg (UINT msg, WPARAM wParam, LPARAM lParam /* = 0L */)
{
   extern HWND ghwndMainFrame;

   return PostMessage (ghwndMainFrame, msg, wParam, lParam);
}

