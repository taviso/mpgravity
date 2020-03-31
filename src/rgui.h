/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgui.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:46  richard_wood
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

#include "tglobdef.h"
#include "rgbase.h"

#define  MAXLINES_OPEN     0
#define  MAXLINES_DECODE   1
#define  MAXLINES_VIEW     2
#define  MAXLINES_IGNORE   3

class TRegUI : public TRegistryBase {
public:

public:
   TRegUI();
   ~TRegUI();
   int Load();
   int Save();

public:
   TGlobalDef::EThreadSort GetThreadSort() { return m_kThreadSortOrder; }
   void SetThreadSort (TGlobalDef::EThreadSort kSort)
      {m_kThreadSortOrder = kSort;}

   TGlobalDef::EThreadSortWithin GetThreadSortWithin () {
      return m_kThreadSortWithin;
      }
   void SetThreadSortWithin (TGlobalDef::EThreadSortWithin kSort)
      {m_kThreadSortWithin = kSort;}

   TGlobalDef::EThreadViewType GetThreadViewType () {return m_kThreadView;}
   void SetThreadViewType (TGlobalDef::EThreadViewType kViewType) {m_kThreadView = kViewType;}

// Obsolete - use GetViewFilterName
   WORD Obsolete_GetViewFilter() { return m_viewFilter; }
   void Obsolete_SetViewFilter(WORD filter)  {m_viewFilter=filter;}

   // deprecated!  8-29-2001
   const CString & Deprecated_GetViewFilterName() { return m_strFilter; }
   void            Deprecated_SetViewFilterName(const CString& s) { m_strFilter = s; }

   int  GetViewFilterID()               { return m_iFilterID; }
   void SetViewFilterID(int iFilterID)  { m_iFilterID =  iFilterID; }

   int  LoadTreehdrCtrlWidths(CString& rStr);
   int  SaveTreehdrCtrlWidths(const CString& rStr);

   int  LoadTreehdrCtrlZoomWidths(CString& rStr);
   int  SaveTreehdrCtrlZoomWidths(const CString& rStr);

   int  GetTreeColumnsRoles (CString& rStr);
   int  SetTreeColumnsRoles (const CString& rStr);

   // proxy for EventLog viewer
   int LoadEventLogData (CString& rStr);
   int SaveEventLogData (const CString& str);

   // utility dialogs dimensions (print and decode dialogs)
   int LoadUtilDlg (const CString &strDlgName, int &iX, int &iY,
      BOOL &bMax, BOOL &bMin);
   void SaveUtilDlg (const CString &strDlgName, const int iX, const int iY,
      BOOL bMax, BOOL bMin);
   int LoadUtilHeaders (const CString &strDlgName, int *riSizes, int iNumSizes);
   void SaveUtilHeaders (const CString &strDlgName, int *riSizes, int iNumSizes);

   int  GetTreeIndent(void)      { return m_treeIndent; }
   void SetTreeIndent(int ti)    { m_treeIndent = ti; }

   int  GetLastGroupID(void);
   void SetLastGroupID(int gid);

   const CString& GetDateFormatStr() { return m_strDateFormat; }
   void  SetDateFormatStr(const CString& str) { m_strDateFormat = str; }

   void GetActiveOptionsPage (int *piIndex);  // restore last property page
   void SetActiveOptionsPage (int  iIndex);

   int GetDefaultPointSize();
   int GetHdrCtrlPointSize();

   BOOL GetOneClickGroup()          { return (BOOL)m_f1ClickGroup; }
   void SetOneClickGroup(BOOL fClk) { m_f1ClickGroup = (WORD) fClk; }

   BOOL GetOneClickArt ()           { return (BOOL) m_f1ClickArt; }
   void SetOneClickArt(BOOL fClk)   { m_f1ClickArt = (WORD) fClk; }

   BOOL GetShowFullHeader () { return m_fShowFullHeader; }
   void SetShowFullHeader (BOOL fFull);

   bool GetShowQuotedText () { return m_wShowQuotedText ? true : false; }
   void SetShowQuotedText (bool fShow);

   int  GetShowQuotedMaxLines () { return m_iShowQuotedMax; }
   void SetShowQuotedMaxLines (DWORD n) { m_iShowQuotedMax = (int)n; }

   BOOL GetShowThreadsCollapsed () { return m_wShowThreadCollapsed; }
   void SetShowThreadsCollapsed (BOOL fCol) { m_wShowThreadCollapsed = (WORD)fCol; }

   void GetIgnoreMimeTypes (CStringList& input);

   BOOL GetTreePaneSort(CString& str, LPCTSTR def);
   void SetTreePaneSort(CString& str);

   int   GetMaxHiliteSeconds () {return m_iMaxHiliteSeconds;}

   const CString &GetScoreMRU () { return m_strScoreMRU; }
   void SetScoreMRU (const CString &str) { m_strScoreMRU = str; }

   int  GetRuleEditIndex () { return m_iRuleEditIndex; }
   void SetRuleEditIndex (int index) { m_iRuleEditIndex = index; }

   void Upgrade (int iCurBuild);

   const CString &GetFactorySizePos () { return m_strFactorySizePos; }
   void SetFactorySizePos (const CString &str) { m_strFactorySizePos = str; }

   const CString &GetPrintSizePos () { return m_strPrintSizePos; }
   void SetPrintSizePos (const CString &str) { m_strPrintSizePos = str; }

   const CString &GetSearchSizePos () { return m_strSearchSizePos; }
   void SetSearchSizePos (const CString &str) { m_strSearchSizePos = str; }

   const CString &GetOutboxSizePos () { return m_strOutboxSizePos; }
   void SetOutboxSizePos (const CString &str) { m_strOutboxSizePos = str; }

   const CString &GetSubscribeSizePos () { return m_strSubscribeSizePos; }
   void SetSubscribeSizePos (const CString &str) { m_strSubscribeSizePos = str; }

   const CString &GetScoringSizePos () { return m_strScoringSizePos; }
   void SetScoringSizePos (const CString &str) { m_strScoringSizePos = str; }

   int  GetLastQuickScore () { return m_iLastQuickScore; }
   void SetLastQuickScore (int i) { m_iLastQuickScore = i; }

   BOOL GetPinFilter () { return m_fPinFilter; }
   void SetPinFilter (BOOL pin) { m_fPinFilter = pin; }

   void GetPrintMargins (int &iLeft, int &iRight, int &iTop, int &iBottom)
      {
      iLeft = m_iPrintLeft;
      iRight = m_iPrintRight;
      iTop = m_iPrintTop;
      iBottom = m_iPrintBottom;
      }
   void SetPrintMargins (int iLeft, int iRight, int iTop, int iBottom)
      {
      m_iPrintLeft = iLeft;
      m_iPrintRight = iRight;
      m_iPrintTop = iTop;
      m_iPrintBottom = iBottom;
      }

   int LoadCustomColors(int count, COLORREF * pData);
   int SaveCustomColors(int count, COLORREF * pData);

   void GetLimitHeadersMRU (bool & fGetAll, int & nCount) {
          fGetAll = m_fGetAllHdrs ;
          nCount  = m_iGetHdrCount;
        }
   void SetLimitHeadersMRU (bool   fGetAll, int   nCount) {
          m_fGetAllHdrs = fGetAll;
          m_iGetHdrCount = nCount;
        }

   int GetMaxLines () {return m_iMaxLines;}
   void SetMaxLines (int iMax) {m_iMaxLines = iMax;}

   int GetMaxLinesCmd () {return m_iMaxLinesCmd;}
   void SetMaxLinesCmd (int iCmd) {m_iMaxLinesCmd = iCmd;}

   void SetNavig1KeyRead (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
   void SetNavigIgnore   (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
   void SetNavigKillArt  (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
   void SetNavigKillThrd (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
   void SetNavigTag      (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
   void SetNavigWatch    (int iThrdOff, int iThrdOn, int iSortOff, int iSortOn);
   void GetNavig1KeyRead (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn);
   void GetNavigIgnore   (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn);
   void GetNavigKillArt  (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn);
   void GetNavigKillThrd (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn);
   void GetNavigTag      (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn);
   void GetNavigWatch    (int & iThrdOff, int & iThrdOn, int & iSortOff, int & iSortOn);

protected:
   void read();
   void write();
   void default_values();
   LONG my_create_key (DWORD* pdwAction);
   LONG my_open_for_write ();

   int save_named_header_ctrl (LPCTSTR value, const CString& rStr);
   int load_named_header_ctrl (LPCTSTR value, CString& rStr);

   void load_dateformat ();

   DWORD  pack_dword (int a, int b, int c, int d);
   void   unpack_dword (DWORD, int & a, int & b, int & c, int & d);

protected:
   TGlobalDef::EThreadSort        m_kThreadSortOrder;
   TGlobalDef::EThreadSortWithin  m_kThreadSortWithin;
   TGlobalDef::EThreadViewType    m_kThreadView;        // tree or Transcript
   WORD                           m_viewFilter;
   int              m_treeIndent;         // pixel indent on tree control
   int              m_gid;                // restore current newsgroup
   CString  m_TreeColumns;
   int              m_iActiveOptionsPage;  // index
   int              m_iDefaultPointSize;
   int              m_iHdrCtrlPointSize;

   // ui preferences
   WORD             m_f1ClickGroup;     // open NG with 1 click
   WORD             m_f1ClickArt;       // open NG with 1 click
   WORD             m_fShowFullHeader;  // persistent state
   WORD             m_wShowQuotedText;

   int              m_iDateFormat;      // corresponds to TGlobDef::EDateFormat
   CString          m_strDateFormat;

   WORD             m_wShowThreadCollapsed;

   CStringList      m_IgnoreMIMETypes;  // types we don't decode

   // DAF = "Date Ascending Flat"   FDT = "From Descending Thread"
   CString          m_TreePaneSort;

   int              m_iMaxHiliteSeconds;

   CString          m_strFilter;        // name of current Customizable view deprecated
   int              m_iFilterID;        // id of current filter

   CString          m_strScoreMRU;      // MRU scores for quick-score dialog
   int              m_iRuleEditIndex;   // last active page in rule-edit dlg
   int              m_iLastQuickScore;  // last value in quick-score dialog

   CString          m_strFactorySizePos, m_strPrintSizePos,
                    m_strSearchSizePos, m_strOutboxSizePos,
                    m_strSubscribeSizePos, m_strScoringSizePos;

   // show last 5 lines of quoted text, even when suppressing quoted text
   int              m_iShowQuotedMax;

   BOOL             m_fPinFilter;       // filter is pinned, or locked

   // printing margins, in hundredths of an inch
   int              m_iPrintRight, m_iPrintLeft, m_iPrintTop, m_iPrintBottom;

   // dlg that prompt to get next 'X' headers
   int              m_iGetHdrCount;
   bool             m_fGetAllHdrs;

   // large article display handling

   int              m_iMaxLines;
   int              m_iMaxLinesCmd;

   DWORD            m_dwNavig1KeyRead;
   DWORD            m_dwNavigIgnore;
   DWORD            m_dwNavigKillArt;
   DWORD            m_dwNavigKillThrd;
   DWORD            m_dwNavigTag;
   DWORD            m_dwNavigWatch;
};
