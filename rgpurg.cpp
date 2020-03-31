/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgpurg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
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

#include "stdafx.h"
#include "tglobdef.h"
#include "mplib.h"
#include "rgpurg.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegPurge::TRegPurge()
   : m_lastPurgeTime(1973, 1, 1, 0, 0, 0), m_lastCompactTime(1973, 1, 1, 0,0,0)
{
    default_values();
    m_itemCount = 12;
    m_pTable = new TableItem[m_itemCount];
    SetItem(m_pTable + 0, "PurgeRead",         kREGBool,  &m_fPurgeRead);
    SetItem(m_pTable + 1, "PurgeReadLimit",    kREGLong,  &m_iPurgeReadLimit);
    SetItem(m_pTable + 2, "PurgeUnread",       kREGBool,  &m_fPurgeUnread);
    SetItem(m_pTable + 3, "PurgeUnreadLimit",  kREGLong,  &m_iPurgeUnreadLimit);
    SetItem(m_pTable + 4, "PurgeOnHdr",        kREGBool,  &m_fPurgeOnHdrs);
    SetItem(m_pTable + 5, "PurgeOnHdrsEvery",  kREGLong,  &m_iPurgeOnHdrsEvery);
    SetItem(m_pTable + 6, "CompactExit",       kREGBool,  &m_fCompactOnExit);
    SetItem(m_pTable + 7, "CompactEvery",      kREGLong,  &m_iCompactOnExitEvery);
    SetItem(m_pTable + 8, "LastPurge",         kREGTime,  &m_lastPurgeTime);
    SetItem(m_pTable + 9, "LastCompact",       kREGTime,  &m_lastCompactTime);
    SetItem(m_pTable + 10, "SkipPercent",      kREGLong,  &m_iSkipPercent, TRUE);
    SetItem(m_pTable + 11, "SkipAmount",       kREGLong,  &m_iSkipAmount, TRUE);
    ASSERT(m_itemCount == 12);
}

TRegPurge::~TRegPurge()
{
   delete [] m_pTable;
}

int TRegPurge::Load()
{
   return TRegistryBase::Load ( GetGravityRegKey()+"Purge" );
}

int TRegPurge::Save()
{
   return TRegistryBase::Save ( GetGravityRegKey()+"Purge" );
}

void TRegPurge::read()
{
   LONG lRet = rgReadTable (m_itemCount, m_pTable);
}

void TRegPurge::write()
{
   LONG lRet;
   lRet = rgWriteTable (m_itemCount, m_pTable);
}

void TRegPurge::default_values()
{
   m_fPurgeRead = TRUE;
   m_iPurgeReadLimit = 7;

   m_fPurgeUnread = TRUE;
   m_iPurgeUnreadLimit = 14;

   m_fPurgeOnHdrs = TRUE;
   m_iPurgeOnHdrsEvery = 1;

   m_fCompactOnExit = TRUE;
   m_iCompactOnExitEvery = 7;

   m_iSkipAmount  = 5;
   m_iSkipPercent = 10;

   // last times are set in Constructor, but reset here.
   CTimeSpan spanOneDay(1, 0, 0, 0);

   // the goal is NOT to compact after the very 1st run, but 7 days from
   //   the install day. I want this serialized as the baseline date.
   m_lastCompactTime = m_lastPurgeTime = CTime::GetCurrentTime();
}

