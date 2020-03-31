/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsfeed.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:34  richard_wood
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

#include "mplib.h"               // TErrorList
#include "tstrlist.h"
#include "callbks.h"

class TNewsSocket;

struct MPBAR_INFO
{
   int   m_iLogSubRangeLen;        // check for -1

   int   m_iLogPos;                // absolute index
   int   m_iTotalLogRange;         // absolute range

   int   m_nPos;                   // start subrange iPos = 0
   int   m_nSubRangeLen;           // length of current subrange
   int   m_nDivide;                // divisor  XOVER=50%, XHDR-XREF=25%
                                   //    XHDR-Newsgroups 25%

   int   m_nGrandTotal;
};

// ------------------------------------------------------------------------
struct FEED_SETGROUP
{
   CString lpszNewsGroup;

   CString strAck;

   BOOL    fOK;

   int     iRet;
   int     est;
   int     first;
   int     last;

   bool    fNormalPump;
   CString strLogFile;
   CString strAddr;
   CString strGroupName;  // returned from server

public:

   FEED_SETGROUP(const CString & logFile, BOOL fPrioPump)
      : strLogFile (logFile)
      {
      fOK = FALSE;
      est = first = last = iRet = -1;

      fNormalPump = fPrioPump ? false : true;
      }

   BOOL parseResults (const CString & strDottedAddr);

   ~FEED_SETGROUP();
};

typedef FEED_SETGROUP * LPFEED_SETGROUP;

class TMemorySpot;
/**************************************************************************
- Sends commands to the NNTP Server.
- Deals with NNTP peculiarities.
- Handles success codes.
- Locks & releases channels.
**************************************************************************/
class TNewsFeed {
public:
   TNewsFeed(int portNumber, FARPROC pBlockingHook, HANDLE hStopEvent,
               bool * pfProcessJobs);

   ~TNewsFeed(void);

   bool m_fValid;

   int   Init (TErrorList * pErrList, LPCTSTR  hostAddress, CString& strWelcome);

   int   ReadLine (TErrorList * pErrList, CString & str);
   int   ReadLine (TErrorList * pErrList, int& ret, CString& str);

   int   WriteLine (TErrorList * pErrList, LPCTSTR  buf, int bytes);

   // CMD
   int   SetGroup (TErrorList * pErrList, LPFEED_SETGROUP psSetGroup);

   // CMD article - issue cmd. Parse retcode. hand off.
	int   Article (TErrorList * pErrList, int art_number, CString& Str,
                  CString & ack,
                  BOOL& fOK, int estimateLines, BOOL fPrio);

   // variation that passes each line back
   int   StartList (TErrorList * pErrList, LPCTSTR cmd = NULL,
                     int* piRet = 0, CString* pAck = 0);
   int   NextListLine (TErrorList * pErrList, CString* pStr);

   // check for fresh newsgroups
   //    pass in the GMT time, since we checked last
   int  Newgroups (TErrorList * pErrList, CTime& time, CString & ack);

   // CMD post an article
   int   PostArticle (TErrorList * pErrList, CFile* pFile,
                      const CString& groupName, BOOL& fOK,
                      CString & Ack);

   // CMD grab subject from an article
   void  XSubject    (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);
   void  XFrom       (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);
   void  XReferences (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);
   void  XMessageID  (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);
   void  XDate       (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);
   void  XFollow     (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);
   void  XNewsgroups (TErrorList * pErrList, int art_number, CString& Str, BOOL& fOK);

   // CMD overview
   int  Overview (TErrorList * pErrList,
                   int low, int high,
                   T2StringList& StrList, BOOL& fOK,
                   int* pRetVal, BOOL  fPriorityStatus,
                   MPBAR_INFO * pBarInfo);

   // CMD mode reader
   int  ModeReader (TErrorList * pErrList, int & iRet, CString & strAck);

   // flexible command
   int  Direct(TErrorList * pErrList,
               const CString& cmdLine, int * piRet = 0, CString * pAck = 0);

   int  DirectNext (TErrorList * pErrList, CString& rOutStr);

   int Quit (TErrorList * pErrList);

   static void progress_norm_cb(int lineCount, DWORD dwdummy);
   static void progress_prio_cb(int lineCount, DWORD dwdummy);

private:

   // unused private constructor;
   TNewsFeed(void);
   void  X_Command (TErrorList * pErrList, LPCTSTR cmd,
                     int art_number, CString& Str, BOOL& fOK);

	int   ArtCmd (TErrorList * pErrList,
                 LPCTSTR cmd,
                 int art_number,
                 CString& Str,
                 CString& ack,
                 BOOL& fOK,
	              BOOL fPriority = 0,
                 BOOL fUserFeedback = FALSE);

   int  ReadAnswer ( TErrorList * pErrList,
                       CString& strData,
                       bool fHandleDoublePeriods,
                       BOOL fPriority, BOOL fStatusLine);

   int  ReadOverviewStringList (TErrorList * pErrList,
                                 T2StringList& StrList,
                                 BOOL fStatusPrio, MPBAR_INFO * psBarInfo);

   // void remove_periods(TMemorySpot* pSpot);

   int  read_line (TErrorList * pErrList, int & ret, int & est, int & first,
                   int & last, CString & ack);

   // private data-members
   TNewsSocket * m_pSocket;

   int      m_iEstimateLines;
};
