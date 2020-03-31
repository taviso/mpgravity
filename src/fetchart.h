/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: fetchart.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:24  richard_wood
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

class TArticleHeader;
class TArticleText;

#include "errhndl.h"          // error codes
#include "mplib.h"

// retrieve an article for the Printing Thread
typedef void (*P_OUTBOUNDFUNC)(int status, TArticleHeader* pArtHdr, TArticleText* pArtText);

//
// new interface  - handles both normal reading & blocking
//   JobSuccess (ArticleText)
//       either send msg to mainfrm
//       or set the event
//
//   JobFailed  (errorID, errorString)
//
//

///////////////////////////////////////////////////////////////////////////
// Goal:
//  make the interaction with the newspump seem synchronous
//  the UI waits until the hEvent is set, or the callback function is used
//
// Important: always use 'new' to instantiate one of these. Destructor is
//            private!
class TFetchArticle {
public:
   // uses event during blocking call
   TFetchArticle();

   // uses sendmsg to send article back to UI window
   TFetchArticle(LPCTSTR groupName, LONG GroupID, int artInt, WORD hdrStatus);

   // pump uses these
   int  JobSuccess (TArticleText* pArtText);
   int  JobFailed  (EErrorClass eClass, DWORD dwErrorID, CString & errorString);

public:
   void DestroySelf();

   TArticleText* GetArtText();
   void          SetArtText (TArticleText* pArtText);
   void          SetSuccess (bool ok) { m_fOK = ok; }
   bool          GetSuccess (void)     { return m_fOK; }

   BOOL GetUIContext (LONG* pGroupID = 0, int* partInt = 0, WORD* phdrStatus = 0);
   const CString& GetGroupName(void) { return m_groupName; }

   HANDLE           m_hFetchComplete;

   bool             m_fOK;
   TError           m_mplibError;

private:
   ~TFetchArticle();

protected:
   TArticleText *   m_pArtText;    // return ptr to article body
   CString          m_groupName;   // newsgroup name
   LONG             m_GroupID;     // newsgroup id
   int              m_artInt;      // article id
   WORD             m_hdrStatus;   // status bits of article header
};

/////////////////////////////////////////////////////////////////////////
// Returns:
//   PUMP_NOT_CONNECTED
//   PUMP_ARTICLE_NOT_FOUND
//
//
int BlockingFetchBody(TError        & sError,
                      const CString & groupName,
                      int             groupID,
                      const CString & subject,
                      CPoint &        ptPartID,
                      int             articleNumber,
                      int             iLines,
                      TArticleText*&  rpText,
                      BOOL            fPrioPump,
                      HANDLE          hThrdKillRequest,
                      BOOL            bMarkAsRead = TRUE,
                      BOOL            bEngage = TRUE);
