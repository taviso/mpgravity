/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ngstat.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:37  richard_wood
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
#include "ngstat.h"
#include "pobject.h"
#include "critsect.h"

#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(TNewsGroupStatus, PObject, 1);

LONG  TNewsGroupStatus::m_BaseMailID = 300000000;

TNewsGroupStatus::TNewsGroupStatus(void)
   : m_NewsGroup_Check(1972, 1, 1, 0, 0, 0)
{
   // default time is wa-a-ay back in the 70's
   m_NextGroupID = 1;
   m_fGroupListExist = FALSE;
   m_NextPostID  = 10000000;
   m_NextMailID  = m_BaseMailID;
   m_fMemoryExist = TRUE;
   InitializeCriticalSection ( &m_critSection );

#if defined(_DEBUG_SHIT)
      CString res1 = m_NewsGroup_Check.Format ("%m %d %Y %H:%M:%S");
      afxDump << res1 << "\n";
#endif
}

TNewsGroupStatus::~TNewsGroupStatus()
{
   DeleteCriticalSection ( &m_critSection );
}

void TNewsGroupStatus::Serialize (CArchive & archive)
{
   PObject::Serialize( archive );

   TEnterCSection mgr(&m_critSection);

   if (archive.IsStoring())
      {
      archive << m_NextGroupID;
      archive << m_fGroupListExist;
      archive << m_NextPostID;
      archive << m_NextMailID;
      archive << m_NewsGroup_Check;
      archive << m_fMemoryExist;
      }
   else
      {
      archive >> m_NextGroupID;
      archive >> m_fGroupListExist;
      archive >> m_NextPostID;
      archive >> m_NextMailID;
      archive >> m_NewsGroup_Check;
      archive >> m_fMemoryExist;
      }

  // newsgroups are stored separately
}

void  TNewsGroupStatus::SetNewsgroupCheck(const CTime& tm)
{
// Put new time;
m_NewsGroup_Check = tm;

// this is called from SaveAllGroups 10-23-95
//SaveSelf();
}

void TNewsGroupStatus::MemoryExist(BOOL fExist)
{
   m_fMemoryExist = BYTE(fExist);
   SaveSelf();
}

void TNewsGroupStatus::SaveSelf()
{
   // write it to database
   // gpStore->ReplaceObject ( this );
}

TNewsGroupStatus& TNewsGroupStatus::operator= (const TNewsGroupStatus& rhs)
{
   if (this == &rhs)
      return (*this);

   PObject::operator=(rhs);

   TEnterCSection mgr( &m_critSection);

   m_NextGroupID     = rhs.m_NextGroupID;
   m_fGroupListExist = rhs.m_fGroupListExist;
   m_NextPostID      = rhs.m_NextPostID;
   m_NextMailID      = rhs.m_NextMailID;
   m_NewsGroup_Check = rhs.m_NewsGroup_Check;
   m_fMemoryExist    = rhs.m_fMemoryExist;

   return *this;
}

