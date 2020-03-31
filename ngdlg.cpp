/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngdlg.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:35  richard_wood
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

// ngdlg.cpp : implementation file
//

#include "stdafx.h"
#include "News.h"
#include "ngdlg.h"
#include "grplist.h"

#include "vlist.h"
#include "tglobdef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupDlg dialog

TNewsGroupDlg::TNewsGroupDlg(void)
{
}

TNewsGroupDlg::~TNewsGroupDlg(void)
{
}

/////// Virtual Listbox Stuff
LRESULT TNewsGroupDlg::On_vlbRange   (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT  lpvlbInStruct = (LPVLBSTRUCT)lParam;

   lpvlbInStruct->lIndex = m_pCurGroups->NumGroups();
   lpvlbInStruct->nStatus = VLB_OK;
   return TRUE;
}

LRESULT TNewsGroupDlg::On_vlbPrev    (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT  lpvlbInStruct = (LPVLBSTRUCT)lParam;

   int   tot = m_pCurGroups->NumGroups();

   if (0 == tot) {
      lpvlbInStruct->nStatus = VLB_ENDOFFILE;
      return TRUE;
   }

   if ( lpvlbInStruct->lIndex > 0 ) {

      lpvlbInStruct->nStatus = VLB_OK;
      lpvlbInStruct->lIndex--;
      lpvlbInStruct->lData = lpvlbInStruct->lIndex;
      m_pCurGroups->GetItem (lpvlbInStruct->lIndex, m_group) ;
      lstrcpyn (m_szText, m_group, sizeof m_szText);
      lpvlbInStruct->lpTextPointer = m_szText;
      return TRUE;
   }
   else {
      lpvlbInStruct->nStatus = VLB_ENDOFFILE;
      return TRUE;
   }
}

LRESULT TNewsGroupDlg::On_vlbFindPos (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

   if (0 == lpvlbInStruct->lIndex)
      return On_vlbFirst (wP, lParam);
   else if (100 == lpvlbInStruct->lIndex)
      return On_vlbLast (wP, lParam);
   else
      {
      int groupCount = m_pCurGroups->NumGroups();

      if (0 == groupCount)
         {
         lpvlbInStruct->nStatus = VLB_ENDOFFILE;
         return TRUE;
         }

      ASSERT(lpvlbInStruct->lIndex <= 100);

      lpvlbInStruct->lIndex = (lpvlbInStruct->lIndex * groupCount) / 100 ;

      // sanity chk
      if (lpvlbInStruct->lIndex >= groupCount)
         lpvlbInStruct->lIndex = groupCount-1;

      if (lpvlbInStruct->lIndex < 0)
         {
         lpvlbInStruct->nStatus = VLB_ENDOFFILE;
         return TRUE;
         }

      lpvlbInStruct->nStatus = VLB_OK;
      m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group);
      lstrcpyn(m_szText,  m_group, sizeof m_szText);
      lpvlbInStruct->lpTextPointer = m_szText;
      return TRUE;
      }
}

LRESULT TNewsGroupDlg::On_vlbFindItem(WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;
   int         tot = m_pCurGroups->NumGroups();

   lpvlbInStruct->lIndex = lpvlbInStruct->lData;

   if (tot == 0 ||
       lpvlbInStruct->lIndex >= tot ||
       lpvlbInStruct->lIndex < 0)
      {
      lpvlbInStruct->nStatus = VLB_ERR;
      return TRUE;
      }

   lpvlbInStruct->nStatus = VLB_OK;

   m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group );

   lstrcpyn (m_szText, m_group, sizeof m_szText );
   lpvlbInStruct->lpTextPointer = m_szText;
   return TRUE;
}

// Needs work
LRESULT TNewsGroupDlg::On_vlbFindString(WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

   lstrcpyn(m_szText,lpvlbInStruct->lpFindString, sizeof m_szText);
   lpvlbInStruct->lIndex = atol(m_szText);

   m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group );
   lstrcpyn (m_szText, m_group, sizeof m_szText );
   lpvlbInStruct->lpTextPointer = m_szText;
   lpvlbInStruct->lData = lpvlbInStruct->lIndex;
   lpvlbInStruct->nStatus = VLB_OK;
   return TRUE;
}

LRESULT TNewsGroupDlg::On_vlbSelectString(WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

   lstrcpyn(m_szText,lpvlbInStruct->lpFindString, sizeof m_szText);
   lpvlbInStruct->lIndex = atol(m_szText);
   m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group );
   lstrcpyn (m_szText, m_group, sizeof m_szText);
   lpvlbInStruct->lpTextPointer = m_szText;
   lpvlbInStruct->lData = lpvlbInStruct->lIndex;
   lpvlbInStruct->nStatus = VLB_OK;
   return TRUE;
}

LRESULT TNewsGroupDlg::On_vlbNext    (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT  lpvlbInStruct = (LPVLBSTRUCT)lParam;

   if ( lpvlbInStruct->lIndex < m_pCurGroups->NumGroups() - 1 ) {
      lpvlbInStruct->nStatus = VLB_OK;
      lpvlbInStruct->lIndex++;
      lpvlbInStruct->lData = lpvlbInStruct->lIndex;
      m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group );

      lstrcpyn (m_szText, m_group, sizeof m_szText);

      lpvlbInStruct->lpTextPointer = m_szText;
      return TRUE;
   }
   else {
      lpvlbInStruct->nStatus = VLB_ENDOFFILE;
      return TRUE;
   }
}

LRESULT TNewsGroupDlg::On_vlbFirst   (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

   if ( m_pCurGroups->NumGroups() == 0 )
      {
      lpvlbInStruct->nStatus = VLB_ENDOFFILE;
      }
   else
      {
      lpvlbInStruct->nStatus = VLB_OK;
      lpvlbInStruct->lIndex = 0L;
      m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group );
      lstrcpyn (m_szText, m_group, sizeof m_szText);

      lpvlbInStruct->lpTextPointer = m_szText;
      lpvlbInStruct->lData = lpvlbInStruct->lIndex;
      }
   return TRUE;
}

LRESULT TNewsGroupDlg::On_vlbLast    (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

   int tot     = m_pCurGroups->NumGroups();
   if (0 == tot)
      {
      lpvlbInStruct->nStatus = VLB_ENDOFFILE;
      return TRUE;
      }

   lpvlbInStruct->nStatus = VLB_OK;
   lpvlbInStruct->lIndex = tot - 1;
   m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group);
   lstrcpyn (m_szText, m_group, sizeof m_szText);
   lpvlbInStruct->lpTextPointer = m_szText;
   lpvlbInStruct->lData = lpvlbInStruct->lIndex;
   return TRUE;
}

LRESULT TNewsGroupDlg::On_vlbGetText (WPARAM wP, LPARAM lParam)
{
   LPVLBSTRUCT lpvlbInStruct = (LPVLBSTRUCT)lParam;

   lpvlbInStruct->nStatus = VLB_OK;
   lpvlbInStruct->lData = lpvlbInStruct->lIndex;
   m_pCurGroups->GetItem ( lpvlbInStruct->lIndex, m_group );
   lstrcpyn (m_szText, m_group, sizeof m_szText);
   lpvlbInStruct->lpTextPointer = m_szText;
   return TRUE;
}

