/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: names.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:32  richard_wood
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

//////////////////////////////////////////////////////////////////////
// these are the names of the root collections

#define  NAME_GLOBAL_OPTIONS           (_T("Global Options"))
#define  NAME_NEWSGROUPS               (_T("NewsGroups"))
#define  NAME_SUBSCRIBED_NEWSGROUPS    (_T("Subscribed"))
#define  NAME_RULES                    (_T("Rules"))
#define  NAME_PENDING_DECODE_JOBS      (_T("Pending Decode Jobs"))
#define  NAME_POSTED_ARTICLE_IDS       (_T("Posted Article IDs"))

// -- NGSTAT.H --
// -  keeps the next post id,
//    whether the master nglist exists
//    next groupID
//    last time we checked for fresh Newsgroups
#define  NAME_GROUP_MANAGER            (_T("Group Manager"))

// - keeps email messages (queued-up, sending, and sent)
#define  NAME_OUTBOX_STATUS            (_T("Out Status"))
#define  NAME_OUTBOX_MAIL_HEADERS      (_T("Out Mail Hdrs"))
#define  NAME_OUTBOX_MAIL_BODIES       (_T("Out Mail Bods"))
#define  NAME_OUTBOX_ART_HEADERS       (_T("Out Art Hdrs"))
#define  NAME_OUTBOX_ART_BODIES        (_T("Out Art Bods"))

// - collection to store window positions, commonly used paths, etc.
#define  NAME_UI_MEMORY                (_T("UI memory"))

// - collection for Purge Status
#define  NAME_PURGE_STATUS             (_T("Purge status"))

/////////////////////////////////////////////////////////////////////
#define FORMAT_STATI       (_T("%d/stati"))
#define FORMAT_HEADERS     (_T("%d/headers"))
#define FORMAT_BODIES      (_T("%d/bodies"))

//efine FORMAT_MAIL_HEADERS (_T("%d/mail headers"))
//efine FORMAT_MAIL_BODIES  (_T("%d/mail bodies"))
