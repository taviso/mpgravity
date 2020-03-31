/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: attinfo.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:13  richard_wood
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
#include "tglobopt.h"
#include "mime.h"
#include "pobject.h"

// -------------------------------------------------------------------------
#define ATTACHMENTINFO_VERSION_NUMBER 1

// -------------------------------------------------------------------------
class TAttachmentInfo : public PObject
{
public:
   TAttachmentInfo();
   TAttachmentInfo(LPCTSTR att,
                   int size,
                   LPCTSTR conType = _T(""),
                   LPCTSTR conDesc = _T(""),
                   TMime::ECode eCode = TMime::CODE_NONE);

   TAttachmentInfo(const TAttachmentInfo& src);
   TAttachmentInfo& operator= (const TAttachmentInfo& rhs);
   void Serialize (CArchive& sArchive);

   CString m_att;
   int     m_nameOffset;
   CString m_contentType;
   CString m_contentDesc;
   TMime::ECode m_eCode;
   int     m_size;
   CString m_subDesc;

   // encoding will be UUEncode -or- base64, since we are too lazy to offer
   // other options
};

//////////////////////////////////////////////////////////////////
//void ConstructElements(TAttachmentInfo* pNewAttInfo, int nCount);
//void DestructElements(TAttachmentInfo* pNewAttInfo, int nCount);

template<class TYPE, class ARG_TYPE>
class TAttachmentArray : public CArray<TYPE, ARG_TYPE>
{
public:
   friend class TAttachmentDoc;

public:
   TAttachmentArray();
   ~TAttachmentArray() {}

   BOOL IsUsingMIME (void);

protected:
   BOOL m_fEncodingInited;
   TGlobalDef::EEncoding m_eEncoding;
};

// ctor
template<class TYPE, class ARG_TYPE>
TAttachmentArray<TYPE, ARG_TYPE>::TAttachmentArray(void)
{
   m_fEncodingInited = FALSE;
//   m_eEncoding = 600;            // a bogus value
}

template<class TYPE, class ARG_TYPE>
BOOL TAttachmentArray<TYPE, ARG_TYPE>::IsUsingMIME(void)
{
   if (!m_fEncodingInited)
      {
      return FALSE;
      }
   return  (TGlobalDef::kMIME == m_eEncoding);
}
