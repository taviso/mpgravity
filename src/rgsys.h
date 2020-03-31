/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgsys.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/27 15:29:22  richard_wood
/*  Updates for 2.9.10.
/*  Fixed : Unable to download a single article (if just one new article in a group)
/*  Fixed : Crash when trying to close down if a DB compact started (after new version detected)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2008/09/19 14:51:46  richard_wood
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

#include "rgbase.h"
#include "licutil.h" // for VERINFO

#define FILENAME_DEF_RE   _T("[!-,-/-~]+(\\.)+[!-,-/-~]+")
#define FRACTION_DEF_RE   _T("([0-9]+)/([0-9]+)")

class TRegSystem : public TRegistryBase {
public:
   enum EThread { kTasker = 0x1, kPumpN = 0x2, kPumpE = 0x4, kScribe = 0x8,
                  kPrint  = 0x10, kDecode = 0x20 };

public:
   TRegSystem();
   ~TRegSystem();
   int Load();
   int Save();

public:
   void  GetPumpHalt(int* pMin, int* pMax)
      { *pMin = m_iPumpHaltMin; *pMax = m_iPumpHaltMax; }

   void  GetEventLogMax(int* pEventLogMax)
      { *pEventLogMax = m_iEventLogMax; }

   int  GetPrio(EThread eThread);

   int  GetSubjectMatchCutoff() { return m_iSubjMatchLen; }

   int  GetHeaderFieldBreakLen() { return m_iHdrFieldBreakLen; }

   void UpdateBuildNumber (int iCur);

   UINT PanicKey () { return m_uPanicKey; }
   void PanicKey (UINT uVirtKey) { m_uPanicKey = uVirtKey; }

   void EnablePanicKey (BOOL fOn) { m_fPanicEnabled = fOn; }
   BOOL EnablePanicKey ()  { return m_fPanicEnabled; }

   CTime GetLastUpdateCheck () {
      return m_lastUpdateCheck;
      }

   void SetLastUpdateCheck (const CTime & time) {
      m_lastUpdateCheck = time;
      }

   BOOL GetCheckForUpdates () {
      return m_fCheckForUpdates;
      }

   void SetCheckForUpdates (BOOL fCheck) {
      m_fCheckForUpdates = fCheck;
      }

   int GetUpdateCheckDays () {
      return m_iUpdateCheckDays;
      }

   void SetUpdateCheckDays (int  iCheckDays) {
      m_iUpdateCheckDays = iCheckDays;
      }

   // used by decoding for subject line analysis
   LPCTSTR  GetFindFractionRE (bool fDefaultVersion);
   LPCTSTR  GetFindFilenameRE (bool fDefaultVersion);
   void     SetFindFractionRE (LPCTSTR pRE);

   const CString & GetDejanewsURL () { return m_strDejaNews; }

   const CString & GetSpellingDictionary() { return m_strDictionary; }
   void            SetSpellingDictionary(const CString & d) { m_strDictionary = d; }

   const CString & GetSpellingAffinity() { return m_strAffinity; }
   void            SetSpellingAffinity(const CString & d) { m_strAffinity = d; }

   BOOL  GetSpelling_IgnoreNumbers()       { return m_fSpellIgnoreNumbers; }
   void  SetSpelling_IgnoreNumbers(BOOL b) { m_fSpellIgnoreNumbers = b; }

   int   GetViewingCharsetID()      { return m_iViewingCharset; }
   void  SetViewingCharsetID(int n) { m_iViewingCharset = n; }

protected:

   void read();
   void write();
   void default_values();

   int  m_iPumpHaltMax;          // in Kbytes
   int  m_iPumpHaltMin;          // in Kbytes
   int  m_iEventLogMax;
   LONG m_lThreadPrio;           // bitmap
   // bit 0  tasker               [1|0]   1 == normal prio, 0 == below_normal
   //     1  normal pump
   //     2  emerg  pump
   //     3  tscribe
   //     4  print
   //     5  decode

   int m_iSubjMatchLen;          // threading by subject - cutoff len
   int m_iHdrFieldBreakLen;

   static const LONG m_giDefThreadPrio;
   //int   m_iVersion;
   VERINFO m_verInfo;
   UINT  m_uPanicKey;
   BOOL  m_fPanicEnabled;
   CTime m_lastUpdateCheck;
   int   m_iUpdateCheckDays;
   BOOL  m_fCheckForUpdates;

   CString  m_strFractionRE;
   CString  m_strFilenameRE;

   CString  m_strDejaNews;

   CString  m_strDictionary;
   CString  m_strAffinity;

   BOOL m_fSpellIgnoreNumbers;

   int  m_iViewingCharset;  // number of GravCharset to use when viewing
};
