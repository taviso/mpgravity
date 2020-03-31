/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: attinfo.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
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

#include "stdafx.h"
#include "attinfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// ctor
TAttachmentInfo::TAttachmentInfo()
: PObject (ATTACHMENTINFO_VERSION_NUMBER)
{
	m_eCode = TMime::CODE_NONE;
	m_size = 0;
	m_nameOffset = 0;
}

TAttachmentInfo::TAttachmentInfo(
								 LPCTSTR att,
								 int size,
								 LPCTSTR conType,
								 LPCTSTR conDesc,
								 TMime::ECode eCode)
								 : PObject (ATTACHMENTINFO_VERSION_NUMBER),
								 m_att(att), m_contentType(conType), m_contentDesc(conDesc)
{
	m_eCode = eCode;
	m_size = size;
	TPath wholePath = att;
	TPath fileName;
	wholePath.GetFile (fileName);
	m_nameOffset = wholePath.GetLength() - fileName.GetLength();
}

// copy ctor
TAttachmentInfo::TAttachmentInfo(const TAttachmentInfo& src)
: PObject (ATTACHMENTINFO_VERSION_NUMBER),
m_att(src.m_att), m_contentType(src.m_contentType),
m_contentDesc(src.m_contentDesc)
{
	m_eCode      = src.m_eCode;
	m_size       = src.m_size;
	m_nameOffset = src.m_nameOffset;
}

TAttachmentInfo&
TAttachmentInfo::operator= (const TAttachmentInfo& rhs)
{
	if (this == &rhs)
		return *this;
	m_att          = rhs.m_att;
	m_nameOffset   = rhs.m_nameOffset;

	m_contentType  = rhs.m_contentType;
	m_contentDesc  = rhs.m_contentDesc;
	m_eCode        = rhs.m_eCode;
	m_size         = rhs.m_size;

	return *this;
}

// -------------------------------------------------------------------------
// Serialize -- read/write the object from/to a file
void TAttachmentInfo::Serialize (CArchive& sArchive)
{
	PObject::Serialize (sArchive);

	if (sArchive.IsStoring ()) {
		sArchive << m_att;
		sArchive << m_nameOffset;
		sArchive << m_contentType;
		sArchive << m_contentDesc;
		sArchive << (int) m_eCode;
		sArchive << m_size;
		sArchive << m_subDesc;
	}
	else {
		sArchive >> m_att;
		sArchive >> m_nameOffset;
		sArchive >> m_contentType;
		sArchive >> m_contentDesc;
		sArchive >> (int &) m_eCode;
		sArchive >> m_size;
		sArchive >> m_subDesc;
	}
}
