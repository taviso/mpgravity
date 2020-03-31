/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: custsig.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:16  richard_wood
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
#include "news.h"
#include "custsig.h"
#include "pobject.h"
#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(TCustomSignature,     CObject, 1);

TCustomSignature::TCustomSignature ()
: PObject(2)
{
	// this is version 2 - can point to a File
	m_fFlags = 0;      // unlocked, Not file-based
}

TCustomSignature::TCustomSignature (LPCTSTR lpstrName)
: PObject(2), m_shortName(lpstrName)
{
	m_fFlags = 0;
}

// copy ctor
TCustomSignature::TCustomSignature(const TCustomSignature & src)
: PObject(2), m_shortName(src.m_shortName), m_sig(src.m_sig)
{
	m_fFlags = src.m_fFlags;
}

// assignment
TCustomSignature &
TCustomSignature::operator=(const TCustomSignature & rhs)
{
	if (this == &rhs)
		return *this;

	m_shortName  = rhs.m_shortName;
	m_sig        = rhs.m_sig;
	m_fFlags     = rhs.m_fFlags;

	return *this;
}

//-------------------------------------------------------------------------
// updated 7-31-96 amc  Don't use GetObjectVersion when storing. You know
//                      what version you are. You are the latest version.
void TCustomSignature::Serialize (CArchive & archive)
{
	PObject::Serialize ( archive );

	if (archive.IsStoring())
	{
		archive << m_shortName;
		archive << m_sig;
		archive << m_fFlags;
	}
	else
	{
		BYTE byVersion = GetObjectVersion ();

		if (1 == byVersion)
		{
			BYTE byTempCanChange;
			archive >> m_shortName;
			archive >> m_sig;
			archive >> byTempCanChange;
			SetChangeable( byTempCanChange );
		}
		else
		{
			archive >> m_shortName;
			archive >> m_sig;
			archive >> m_fFlags;
		}
	}
}

// -------------------------------------------------------------------
// 01-25-98  amc  New template syntax for serialize
template <> void AFXAPI
SerializeElements <TCustomSignature>(CArchive& ar, TCustomSignature* pSig, int nCount)
{
	for (int i=0; i < nCount; ++i, ++pSig)
		pSig->Serialize ( ar );
}

BOOL TCustomSignature::CanChange() const
{
	return (m_fFlags & kSigLocked) ? FALSE : TRUE;
}

void TCustomSignature::SetChangeable(BOOL fCanChange)
{
	if (fCanChange)
		m_fFlags &= ~kSigLocked;
	else
		m_fFlags |=  kSigLocked;
}

BOOL TCustomSignature::IsFileBased(void) const
{
	return (m_fFlags & kSigFileBased) ? TRUE : FALSE;
}

void TCustomSignature::SetFileBased(BOOL fFileBased, LPCTSTR fileName)
{
	if (fFileBased)
	{
		m_fFlags |= kSigFileBased;
		m_sig = fileName;
	}
	else
		m_fFlags &= ~kSigFileBased;
}

///////////////////////////////////////////////////////////////////////////
//  Set the signature text - if this is file-based, update the file
//
void TCustomSignature::SetSignature (LPCTSTR sigText)
{
	if (IsFileBased())
	{
		// write out to file
		CFile f;
		CFileException excp;
		if (f.Open ( m_sig, CFile::modeWrite | CFile::typeBinary, &excp))
		{
			int iSigLen = _tcslen ( sigText );
			f.Write ( sigText, iSigLen );
			f.SetLength ( iSigLen );
		}
		else
		{
			return ;
		}
	}
	else
	{
		// save the text
		m_sig = sigText;
	}
}

///////////////////////////////////////////////////////////////////////////
//  Get the signature text - if this is file-based, read it from the file
//
int TCustomSignature::GetSignature (CString & s, CWnd * pAnchor) const
{
	// int iTest = m_fFlags;

	if (IsFileBased())
	{
		try
		{
			// m_sig is a filename
			CFile f( m_sig, CFile::modeRead | CFile::typeBinary
				| CFile::shareDenyNone );

			int iLen = (int) f.GetLength();

			LPTSTR pBuf = s.GetBuffer(iLen);
			f.Read( pBuf, iLen );
			s.ReleaseBuffer (iLen);

			return 0;
		}
		catch(CFileException *pExcp)
		{
			CString msg;
			TCHAR szCause[255];   szCause[0] = '\0';

			// get the cause
			pExcp->GetErrorMessage (szCause, sizeof(szCause));
			pExcp->Delete();

			AfxFormatString1 (msg, IDS_WARN_SIGFILEERR, szCause);
			NewsMessageBox (pAnchor, msg, MB_OK | MB_ICONWARNING);

			return -1;
		}
	}
	else
	{
		s = m_sig;
		return 0;
	}
}

const CString& TCustomSignature::GetFileName () const
{
	if (!IsFileBased())
		ASSERT(0);
	return m_sig;
}

