/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ecpcomm.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:23  richard_wood
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

#include "mplib.h"

/////////////////////////////////////////////////////////////////////////////
// Base communication exception type.
/////////////////////////////////////////////////////////////////////////////

typedef enum { NNTP_BAD_ARTICLE } ENntp_Error;

class TCommExcept : public TException
{
public:
   TCommExcept(const CString &description, const ESeverity &kSeverity)
      : TException(description, kSeverity) {}
};

class TSmtpExcept : public TCommExcept {
public:
   TSmtpExcept(const CString &description, const ESeverity &kSeverity)
      : TCommExcept(description, kSeverity) {}
};

class TNntpExcept : public TCommExcept {
public:
   TNntpExcept(const CString &description,    // normal stuff for TException
               const ESeverity &kSeverity,    // normal stuff for TException
               ENntp_Error eError,
               int         iNntpRet,
               int         iArtInt)
      : TCommExcept(description, kSeverity)
      {
      m_eError   = eError;
      m_iNntpRet = iNntpRet;
      m_ArtInt   = iArtInt;
      }

   TNntpExcept(const TNntpExcept& src)
      : TCommExcept(src)
      {
      m_eError   = src.m_eError;
      m_ArtInt   = src.m_ArtInt;
      m_iNntpRet = src.m_iNntpRet;
      }

   ENntp_Error m_eError;
   int         m_ArtInt;
   int         m_iNntpRet;
   CString     m_groupName;
};

