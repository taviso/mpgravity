/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rglaymdi.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:45  richard_wood
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

/////////////////////////////////////////////////////////////////
// Load and Save layout info to the Registry
//
// I think there will be several layouts in here:
//   mdi window
//   popup view window
//   post & followup
//   reply
//   main
//   bugreport
//   product idea
//   send to friend

#include "stdafx.h"
#include "mplib.h"

#include "rglaymdi.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegLayoutMdi::MdiSync TRegLayoutMdi::m_map[] = {
             {_T("2t"),    TUIMemory::LAY3_2TOP       },
             {_T("2b"),    TUIMemory::LAY3_2BOT       },
             {_T("2l"),    TUIMemory::LAY3_2RIGHT     },
             {_T("2r"),    TUIMemory::LAY3_2LEFT      },
             {_T("3h"),    TUIMemory::LAY3_3HIGH      },
             {_T("3w"),    TUIMemory::LAY3_3WIDE      }  };

TRegLayoutMdi::TRegLayoutMdi(BOOL fFillDefault)
{
    m_fSizeInfo = FALSE;
    default_values();
}

int TRegLayoutMdi::Load()
{
   // inherited
   return TRegistryBase::Load ( GetGravityRegKey()+_T("Layout\\MDI") );
}

void TRegLayoutMdi::read()
{
   // the m_hkKey is opened
   LONG  lRet;
   DWORD dwValueType;
   DWORD dwValueSize;
   TCHAR rcBuf[50];
   int   i;
   BOOL bUseNTAOrder = FALSE;
   dwValueSize = sizeof(m_place);
   lRet = RegQueryValueEx(m_hkKey, _T("Place"), 0, &dwValueType, (BYTE*)&m_place, &dwValueSize);

   // layout
   dwValueSize = sizeof(rcBuf);
   lRet = RegQueryValueEx(m_hkKey, _T("Layout"), 0, &dwValueType, (BYTE*)rcBuf, &dwValueSize);
   int tot = sizeof(m_map)/sizeof(m_map[0]);
   for (i = 0; i < tot; ++i)
      {
      if (!lstrcmpi(rcBuf, m_map[i].name))
         {
         m_layout = m_map[i].lo;
         break;
         }
      }
   if (tot == i) {
      // probably upgrading from v1.0 where it was configured for a 4-pane view
      // just set to some reasonable default
      default_values ();
      bUseNTAOrder = TRUE;
      }

   // read rect #1
   dwValueSize = sizeof(rcBuf);
   lRet = RegQueryValueEx(m_hkKey, _T("Pane1"), 0, &dwValueType, (BYTE*)rcBuf, &dwValueSize);
   str_to_rect(rcBuf, m_rct1);

   // read rect #2
   dwValueSize = sizeof(rcBuf);
   lRet = RegQueryValueEx(m_hkKey, _T("Pane2"), 0, &dwValueType, (BYTE*)rcBuf, &dwValueSize);
   str_to_rect(rcBuf, m_rct2);

   // read rect #3
   dwValueSize = sizeof(rcBuf);
   lRet = RegQueryValueEx(m_hkKey, _T("Pane3"), 0, &dwValueType, (BYTE*)rcBuf, &dwValueSize);
   str_to_rect(rcBuf, m_rct3);

   // see if we have a valid layout, in general
   if (0 == m_rct1.top && 0 == m_rct1.left && m_rct1.bottom >= 0 && m_rct1.right >= 0)
      m_fSizeInfo = TRUE;

   RECT rctZ; SetRectEmpty(&rctZ);
   if (::EqualRect (&rctZ, &m_rct1) && ::EqualRect(&rctZ, &m_rct2) &&
       ::EqualRect (&rctZ, &m_rct3))
      m_fSizeInfo = FALSE;

   // read order
   dwValueSize = sizeof(rcBuf);
   lRet = RegQueryValueEx(m_hkKey, _T("Order"), 0, &dwValueType, (BYTE*)rcBuf, &dwValueSize);

   if (bUseNTAOrder)
      _tcscpy (rcBuf, _T("nta"));

   for (i = 0; i < (sizeof(m_vPanes)/sizeof(m_vPanes[0])); ++i)
      {
      switch(rcBuf[i])
         {
         case 'n':
            m_vPanes[i] = TUIMemory::PANE_NEWSGROUPS;
            break;
         case 't':
            m_vPanes[i] = TUIMemory::PANE_THREADVIEW;
            break;
         case 'a':
            m_vPanes[i] = TUIMemory::PANE_ARTVIEW;
            break;
         }
      }

   // zoomCode
   dwValueSize = sizeof(rcBuf);
   lRet = rgReadInt (_T("ZoomCode"), (BYTE*)rcBuf, (int)dwValueSize, m_iZoomCode);
}

void TRegLayoutMdi::default_values()
{
   ZeroMemory(&m_place, sizeof(m_place));
   m_layout = TUIMemory::LAY3_3HIGH;
   m_vPanes[0] = TUIMemory::PANE_NEWSGROUPS;
   m_vPanes[1] = TUIMemory::PANE_THREADVIEW;
   m_vPanes[2] = TUIMemory::PANE_ARTVIEW;

   DestroyPaneSizes ();

   m_iZoomCode = 0;
}

int TRegLayoutMdi::Save()
{
   return TRegistryBase::Save ( GetGravityRegKey()+_T("Layout\\MDI") );
}

void TRegLayoutMdi::write()
{
   DWORD dwValueType, dwValueSize;
   LONG lRet;
   TCHAR rcBuf[80];

   // output window placement
   lRet = RegSetValueEx (m_hkKey, _T("Place"), 0, REG_BINARY, (BYTE*) &m_place,
                                                             sizeof(m_place));
   // output layout
   int tot = sizeof(m_map)/sizeof(m_map[0]);
   int i;
   for (i = 0; i < tot; ++i)
      {
      if (m_layout == m_map[i].lo)
         {
         lRet = RegSetValueEx (m_hkKey, _T("Layout"), 0, REG_SZ,
                               (BYTE*) m_map[i].name, sizeof(m_map[i].name));
         break;
         }
      }
   CString str;
   rect_to_string (m_rct1, str);
   lRet = RegSetValueEx(m_hkKey, _T("Pane1"), 0, REG_SZ, (BYTE*) (LPCTSTR) str,
                        str.GetLength() + 1);

   rect_to_string (m_rct2, str);
   lRet = RegSetValueEx(m_hkKey, _T("Pane2"), 0, REG_SZ, (BYTE*) (LPCTSTR) str,
                        str.GetLength() + 1);

   rect_to_string (m_rct3, str);
   lRet = RegSetValueEx(m_hkKey, _T("Pane3"), 0, REG_SZ, (BYTE*) (LPCTSTR) str,
                        str.GetLength() + 1);

   // read order
   dwValueSize = sizeof(rcBuf);
   lRet = RegQueryValueEx(m_hkKey, _T("Order"), 0, &dwValueType, (BYTE*)rcBuf, &dwValueSize);
   for (i = 0; i < (sizeof(m_vPanes)/sizeof(m_vPanes[0])); ++i)
      {
      switch(m_vPanes[i])
         {
         case TUIMemory::PANE_NEWSGROUPS:
            rcBuf[i] = 'n';
            break;
         case TUIMemory::PANE_THREADVIEW:
            rcBuf[i] = 't';
            break;
         case TUIMemory::PANE_ARTVIEW:
            rcBuf[i] = 'a';
            break;
         }
      }
   rcBuf[i] = '\0';
   lRet = RegSetValueEx(m_hkKey, _T("Order"), 0, REG_SZ, (BYTE*) rcBuf, i+1);

   // zoomCode
   rgWriteNum (_T("ZoomCode"), m_iZoomCode);
}

void TRegLayoutMdi::DestroyPaneSizes()
{
   m_fSizeInfo = FALSE;

   // Fill with explicit junk values
   m_rct1.top   = m_rct1.bottom =  -1;
   m_rct1.right = m_rct1.left   =  -1;
   m_rct3 = m_rct2 = m_rct1;
}

// ------------------------------------------------------------------------
// used during dynamic layout change
TRegLayoutMdi* TRegLayoutMdi::Copy()
{
   // Note: Right now, this only copies what I need.

   TRegLayoutMdi * pOther = new TRegLayoutMdi;
   pOther->m_vPanes[0] = m_vPanes[0];
   pOther->m_vPanes[1] = m_vPanes[1];
   pOther->m_vPanes[2] = m_vPanes[2];

   CopyRect (&pOther->m_rct1, &m_rct1);
   CopyRect (&pOther->m_rct2, &m_rct2);
   CopyRect (&pOther->m_rct3, &m_rct3);
   return pOther;
}

