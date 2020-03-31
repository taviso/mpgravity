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
#include "resource.h"
// navigdef.h

   enum EJmpQuery  {  kQueryDoNotMove,
                      kQueryNext,
                      kQueryNextUnread,
                      kQueryNextLocal,
                      kQueryNextUnreadLocal,
                      kQueryNextUnreadInThread,
                      kQueryNextUnreadWatchFree,
                      kQueryNextUnreadIgnoreFree,
                      kQueryNextUnreadTagFree,
                   };


#define  ACTION_DONTMOVE                   ((BYTE)  0)
#define  ACTION_SKIPNEXT                   ((BYTE)  1)
#define  ACTION_VIEWNEXT                   ((BYTE)  2)
#define  ACTION_SKIPNEXTUNREAD             ((BYTE)  3)
#define  ACTION_SKIPNEXTUNREADINTHREAD     ((BYTE)  4)
#define  ACTION_SKIPNEXTUNREADLOCAL        ((BYTE)  5)
#define  ACTION_VIEWNEXTUNREAD             ((BYTE)  6)
#define  ACTION_VIEWNEXTUNREADINTHREAD     ((BYTE)  7)
#define  ACTION_VIEWNEXTUNREADLOCAL        ((BYTE)  8)
#define  ACTION_SKIPNEXT_NOEXPAND          ((BYTE)  9)

// hmm , we have ACTION_VIEWNEXT which is View-Next-Article
///       we sort of need an plain old SkipTo-Next-Article
struct NAVTRIPLET
{
   BYTE       byAction;
   int        idCmd;
   EJmpQuery  eQuery;

};

static NAVTRIPLET  vNavMap[10] = {
    {ACTION_DONTMOVE,                ID_NAVIGATE_DONTMOVE,                 kQueryDoNotMove},
    {ACTION_SKIPNEXT,                ID_NAVIGATE_SKIPDOWN,                 kQueryNext},
    {ACTION_SKIPNEXT_NOEXPAND,       ID_NAVIGATE_SKIPDOWN_NOEXP,           kQueryNext},
    {ACTION_VIEWNEXT,                ID_ARTICLE_NEXT,                      kQueryNext},
    {ACTION_SKIPNEXTUNREAD,          ID_ARTICLE_SKIPNEXTUNREAD,            kQueryNextUnread},
    {ACTION_SKIPNEXTUNREADINTHREAD,  ID_ARTICLE_SKIPNEXTUNREADINTHREAD,    kQueryNextUnreadInThread},
    {ACTION_SKIPNEXTUNREADLOCAL,     ID_ARTICLE_SKIPNEXTUNREADLOCAL,       kQueryNextUnreadLocal},
    {ACTION_VIEWNEXTUNREAD,          ID_ARTICLE_VIEWNEXTUNREAD,            kQueryNextUnread},
    {ACTION_VIEWNEXTUNREADINTHREAD,  ID_ARTICLE_VIEWNEXTUNREADINTHREAD,    kQueryNextUnreadInThread},
    {ACTION_VIEWNEXTUNREADLOCAL,     ID_ARTICLE_VIEWNEXTUNREADLOCAL,       kQueryNextUnreadLocal}
    };

// input action codes
void fnNavigate_Fire_Event (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
EJmpQuery fnNavigate_GetQuery (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn, bool * pfViewArt);

