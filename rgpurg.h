/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgpurg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

#pragma once

#include "tglobdef.h"
#include "rgbase.h"

class TRegPurge : public TRegistryBase {
public:
   TRegPurge();
   ~TRegPurge();
   int Load();
   int Save();

   void SetPurgeRead(BOOL fPurgeRead)   { m_fPurgeRead = fPurgeRead; }
   BOOL GetPurgeRead(void)              { return m_fPurgeRead; }
   void SetPurgeReadLimit(int iLimit)   { m_iPurgeReadLimit =  iLimit; }
   int  GetPurgeReadLimit(void)         { return m_iPurgeReadLimit; }

   void SetPurgeUnread(BOOL fPrgUnread)  { m_fPurgeUnread = fPrgUnread; }
   BOOL GetPurgeUnread(void)             { return m_fPurgeUnread; }
   void SetPurgeUnreadLimit(int iLimit)  { m_iPurgeUnreadLimit =  iLimit; }
   int  GetPurgeUnreadLimit(void)        { return m_iPurgeUnreadLimit; }

   void SetPurgeOnHdrs(BOOL fPrgOnHdr)   { m_fPurgeOnHdrs = fPrgOnHdr; }
   BOOL GetPurgeOnHdrs(void)             { return m_fPurgeOnHdrs; }
   void SetPurgeOnHdrsEvery(int every)   { m_iPurgeOnHdrsEvery = every; }
   int  GetPurgeOnHdrsEvery(void)        { return m_iPurgeOnHdrsEvery; }

   void SetCompactOnExit(BOOL fCmOnExit) { m_fCompactOnExit = fCmOnExit; }
   BOOL GetCompactOnExit(void)           { return m_fCompactOnExit; }
   void SetCompactOnExitEvery(int every) { m_iCompactOnExitEvery = every; }
   int  GetCompactOnExitEvery(void)      { return m_iCompactOnExitEvery; }

   int  GetSkipPercent ()                {return m_iSkipPercent;}
   void SetSkipPercent (int percent)     {m_iSkipPercent = percent;}

   int  GetSkipAmount ()                 {return m_iSkipAmount;}
   void SetSkipAmount (int amount)       {m_iSkipAmount = amount;}

   CTime    m_lastPurgeTime;     // manual or by m_fPurgeOnHdrs
   CTime    m_lastCompactTime;   // manual or by m_fCompactOnExit

protected:
   void read();
   void write();
   void default_values();

protected:
   BOOL     m_fPurgeRead;
   int      m_iPurgeReadLimit;

   BOOL     m_fPurgeUnread;
   int      m_iPurgeUnreadLimit;

   BOOL     m_fPurgeOnHdrs;
   int      m_iPurgeOnHdrsEvery;   // Purge when retrieving hdrs every X days

   BOOL     m_fCompactOnExit;
   int      m_iCompactOnExitEvery; // Compact on exit every X days

   int      m_iSkipPercent;
   int      m_iSkipAmount;
};
