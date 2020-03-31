/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tglobdef.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:04  richard_wood
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

class TGlobalDef
{
public:
   // !!!! these enums map directly onto the radio buttons in the
   //      wastebasket options dialog - don't change the constants

   enum EPurgeType {kNever       = 0,
                    kOnShutDown  = 1,
                    kElapsedTime = 2};

   enum EThreadSort {kSortDate = 0, kSortSubject = 1,
      kSortScore = 2, kSortAuthor = 3};
   enum EThreadSortWithin {kSortDateWithin = 0, kSortSubjectWithin = 1,
      kSortScoreWithin = 2, kSortAuthorWithin = 3};
   enum EGroupSchedule {kGroupsOnDemand = 0, kGroupsUponConnecting = 1};
   enum EEncoding {kUUENCODE = 0, kMIME = 1};
   enum EThreadViewType {kThreadTree = 0, kTranscriptView = 1};

   enum ENewsGroupType { kPostAllowed = 1,
                         kPostNotAllowed = 2,
                         kPostModerated = 3};
   enum EDateFormat { kFormatDate=0, kFormatDateTime24=1, kFormatDateTimePM=2 };

   enum EUserAction { kActionOpenGroup, kActionOpenArticle, kActionTabAround };

   enum EMsgBoxAnchor { kMainWnd };
public:
   TGlobalDef() {}
   ~TGlobalDef(){}
};
