/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgwarn.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:47  richard_wood
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
#include "resource.h"
#include "mplib.h"
#include "rgwarn.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegWarn::TRegWarn()
{
    default_values();
}
TRegWarn::~TRegWarn()
{
}

int TRegWarn::Load()
{
   return TRegistryBase::Load ( GetGravityRegKey()+"Warnings" );
}

int TRegWarn::Save()
{
   return TRegistryBase::Save ( GetGravityRegKey()+"Warnings" );
}

void TRegWarn::read()
{
   LONG lRet;
   BYTE buf[30];
   int sz = sizeof(buf);
   lRet = rgReadBool("CatchUp",      buf, sz, m_fWarnOnCatchup);
   lRet = rgReadBool("Compose",      buf, sz, m_fWarnOnExitCompose);
   lRet = rgReadBool("Exit",         buf, sz, m_fWarnOnExitNews32);
   lRet = rgReadBool("MarkRead",     buf, sz, m_fWarnOnMarkRead);
   lRet = rgReadBool("Unsubscribe",  buf, sz, m_fWarnOnUnsubscribe);
   lRet = rgReadBool("Sending",      buf, sz, m_fWarnOnSending);
   lRet = rgReadBool("DeleteBinary", buf, sz, m_fWarnOnDeleteBinary);
   lRet = rgReadBool("ErrorDuringDecode", buf, sz, m_fWarnOnErrorDuringDecode);
   lRet = rgReadBool("RunExe",       buf, sz, m_fWarnOnRunExe);
   lRet = rgReadBool("Overwrite",    buf, sz, m_fWarnOnOverwrite);
   lRet = rgReadBool("ManualRuleOffline", buf, sz, m_fWarnOnManualRuleOffline);
   lRet = rgReadBool("NewsURL",      buf, sz, m_fWarnAboutNewsURL);
   lRet = rgReadBool("DeleteArticle", buf, sz, m_fWarnOnDeleteArticle);
}

void TRegWarn::write()
{
   LONG lRet;
   lRet = rgWriteNum("CatchUp",      m_fWarnOnCatchup);
   lRet = rgWriteNum("Compose",      m_fWarnOnExitCompose);
   lRet = rgWriteNum("Exit",         m_fWarnOnExitNews32);
   lRet = rgWriteNum("MarkRead",     m_fWarnOnMarkRead);
   lRet = rgWriteNum("Unsubscribe",  m_fWarnOnUnsubscribe);
   lRet = rgWriteNum("Sending",      m_fWarnOnSending);
   lRet = rgWriteNum("DeleteBinary", m_fWarnOnDeleteBinary);
   lRet = rgWriteNum("ErrorDuringDecode", m_fWarnOnErrorDuringDecode);
   lRet = rgWriteNum("RunExe",       m_fWarnOnRunExe);
   lRet = rgWriteNum("Overwrite",    m_fWarnOnOverwrite);
   lRet = rgWriteNum("ManualRuleOffline", m_fWarnOnManualRuleOffline);
   lRet = rgWriteNum("NewsURL",      m_fWarnAboutNewsURL);
   lRet = rgWriteNum("DeleteArticle", m_fWarnOnDeleteArticle);
}

void TRegWarn::default_values()
{
   m_fWarnOnCatchup      = TRUE;
   m_fWarnOnExitCompose  = TRUE;
   m_fWarnOnExitNews32   = TRUE;
   m_fWarnOnMarkRead     = TRUE;
   m_fWarnOnUnsubscribe  = TRUE;
   m_fWarnOnSending      = TRUE;
   m_fWarnOnDeleteBinary = TRUE;
   m_fWarnOnErrorDuringDecode = TRUE;
   m_fWarnOnRunExe       = TRUE;
   m_fWarnOnOverwrite    = TRUE;
   m_fWarnOnManualRuleOffline = TRUE;
   m_fWarnAboutNewsURL   = TRUE;
   m_fWarnOnDeleteArticle = TRUE;
}
