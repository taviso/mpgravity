/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: genutil.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/27 15:29:21  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.2  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/01/01 13:57:19  richard_wood
/*  Bug fix for build version code (build number now includes version number internally).
/*  Bug fix for beta version indicatiin (made the same in both places).
/*  These two fix the user date format and newsgroup header changes being lost.
/*
/*  Revision 1.4  2008/09/19 14:51:26  richard_wood
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

// genutil.h -- generic utilities

#pragma once

#include "statunit.h"            // TStatusUnit
#include "mplib.h"               // TError

class TThreadListView;
class CNewsView;
class TNewsGroup;
class TArticleHeader;
class TArticleText;

// return codes for GetOS()
#define RUNNING_ON_UNKNOWN 0
#define RUNNING_ON_WINNT 1
#define RUNNING_ON_WIN95 2
#define RUNNING_ON_VISTA 3
#define RUNNING_ON_WIN7 4

int MsgResource (int iResourceID, CWnd *pWnd = 0, LPCTSTR pchCaption = 0,
   UINT nType = MB_OK);
void DeleteSelected (CListBox *pListbox);
void DDX_LBStringList (CDataExchange* pDX, int nIDC, CStringList &sList);
void DDX_CEditStringList (CDataExchange* pDX, int nIDC, CStringList &sList);
CString EscapeString (const CString &strInput);
BOOL RegisterSimilarDialogClass(UINT idClassnameStr, UINT idIcon);

int fnFetchBody (
   TError      & sErrorRet,
   TNewsGroup *      psNG,
   TArticleHeader *  psHeader,
   TArticleText *&   psBody,
   CPoint        &   ptPartID,
   BOOL              bMarkAsRead,
   BOOL              bTryFromNewsfeed,
   BOOL              bEngage = FALSE,
   CPoint       *    pptLocal = NULL);

void GenSetStatusBit (LONG                 lGroupID,
                      LONG                 lArticleNum,
                      TStatusUnit::EStatus iBit,
                      TNewsGroup *         pNG = NULL,
                      BOOL                 bValue = TRUE);

void GenSetManyStatusBit (LONG                 lGroupID,
                          CDWordArray &        vArticleNum,
                          TStatusUnit::EStatus iBit,
                          TNewsGroup *         pNG = NULL,
                          BOOL                 bValue = TRUE);

void GenSetManyObjStatusBit (LONG                 lGroupID,
                          CPtrArray &             vArtHeader,
                          TStatusUnit::EStatus iBit,
                          TNewsGroup *         pNG = NULL,
                          BOOL                 bValue = TRUE);

void fnKillPrefix (CString& subj);
CNewsView *GetNewsView ();
TThreadListView *GetThreadView ();

int CenterListboxSelection(CListBox * pLbx, BOOL fMovingDown);
int CenterListboxSelection(CListCtrl * pLC, BOOL fMovingDown);

int EnsureVisibleSelection(CListBox * pLbx);
int GetOS ();
int ListboxAddAdjustHScroll (CListBox &box, LPCTSTR pchString,
   BOOL bCheckListbox = FALSE);
void SelectedHeaders (TArticleHeader *pHdr, TArticleText *pText, CString &str);
void CopyCStringList (CStringList &dest, const CStringList &src);
CString EscapeQuotes (LPCTSTR pch);
CString UnescapeQuotes (LPCTSTR pch);
void GetLine (const CRichEditCtrl &sEdit, int iLine, CString &strLine);
void CheckIndentation (CRichEditCtrl &sEdit);
int CountLeadingSpaces (LPCTSTR str);
int CountLeadingSpaces (const CString &str);
CString GetStartupDir ();
void Browse (CString &strFile, BOOL bOpen);
bool IsPaneZoomed ();
void WildmatToRE (const CString &strWildmat, CString &strRE, BOOL bMatchAll = TRUE);
BOOL IsAProxicomWebServer ();
int DecomposeSizePos (const CString &strSizePos, int &dx, int &dy, int &x,
   int &y);
const CString ComposeSizePos (int dx, int dy, int x, int y);
const CString &DaysTillExpiration (const CTime &sLastSeen, int iExpirationDays);

CString strip_newline(const CString & in);

int GetInstallDir (CString & installDir);
