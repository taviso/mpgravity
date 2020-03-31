/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: hints.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:27  richard_wood
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

#pragma once

typedef enum
{
   VIEWHINT_SHOWARTICLE = 300,
   VIEWHINT_SHOWGROUP,
   VIEWHINT_RELOAD_NEWSGROUPS,
   VIEWHINT_OTHER,
   VIEWHINT_ARTHDR_DESTROYED,
   VIEWHINT_STATUS_CHANGE,
   VIEWHINT_EMPTY,
   VIEWHINT_REFILLTREE,
   VIEWHINT_GOTOARTICLE,
   VIEWHINT_ERASE_OLDTHREADS,
   VIEWHINT_NEWSVIEW_UNSUBSCRIBE,
   SENDHINT_80CHARPERLINE,
   SENDHINT_SETFOCUS_SUBJ,
   SENDHINT_SETFOCUS_NG,
   SENDHINT_SPELLCHECK,
   VIEWHINT_SAVESEL,
   VIEWHINT_ZOOM,
   VIEWHINT_UNZOOM,
   VIEWHINT_SERVER_SWITCH,
   VIEWHINT_SHOWGROUP_NOSEL,
   SENDHINT_SELECTIONWRAP
} TYPE_NEWS_VIEW_HINTS;

class TUnZoomHintInfo : public CObject
{
public:
   TUnZoomHintInfo(BOOL fVis, CView * pTravelView)
      {
      m_pTravelView = pTravelView;
      m_byWindowIsVisible = static_cast<BYTE>(fVis);
      }
   CView * m_pTravelView;
   BYTE m_byWindowIsVisible;
};
