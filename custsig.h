/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: custsig.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.2  2008/09/19 14:51:20  richard_wood
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

#include "resource.h"
#include "pobject.h"

class TCustomSignature : public PObject {
public:
	DECLARE_SERIAL (TCustomSignature)
	TCustomSignature (LPCTSTR lpstrName);

	TCustomSignature ();
	~TCustomSignature () {}

	// copy ctor
	TCustomSignature(const TCustomSignature & src);

	// assignment
	TCustomSignature & operator=(const TCustomSignature & rhs);
	virtual void Serialize (CArchive & archive);

	void           SetShortName (LPCTSTR shortName) { m_shortName = shortName; }
	const CString& GetShortName () const { return m_shortName; }

	void           SetSignature (LPCTSTR sig);
	int            GetSignature (CString & s, CWnd * pAnchor) const;

	// the default signature (None) is not editable!
	BOOL           CanChange() const;
	void           SetChangeable(BOOL chg);
	BOOL           IsFileBased(void) const;
	void           SetFileBased(BOOL fFileBased, LPCTSTR fileName);
	const CString& GetFileName () const;

protected:
	// The sig can be a text or we just store a filename
	enum ECustSigFlags { kSigFileBased = 0x1, kSigLocked = 0x2 };

	CString m_shortName;
	CString m_sig;
	BYTE    m_fFlags;
};

//////////////////////////////////////////////////////////////////
// STL handles ConstructElements(TCustomSignature* pSig, int nCount);
//     and     DestructElements(TCustomSignature* pSig, int nCount);
//

// -------------------------------------------------------------------
// 01-25-98  amc  New template syntax for serialize
template <> void AFXAPI
SerializeElements <TCustomSignature>(CArchive& ar, TCustomSignature* pSig, int nCount);

//////////////////////////////////////////////////////////////////
// Al is nuts. here he is deriving from a template class, again.
//
template<class TYPE, class ARG_TYPE>
class TCustomSignatureList : public CArray<TYPE, ARG_TYPE>
{
public:

	TCustomSignatureList ()
	{
		CString s; s.LoadString(IDS_NO_SIGNATURE);
		TCustomSignature oneSig(s);
		oneSig.SetChangeable(FALSE);
		Add ( oneSig );
		m_DefIndex = 0;
	}
	~TCustomSignatureList () {}

	// assignment ?? IMPURE returns void
	void Set(TCustomSignatureList<TYPE, ARG_TYPE>* pList);

	virtual void Serialize (CArchive & archive);

	void   SetDefaultIndex(int idx) { m_DefIndex = idx; }
	int    GetDefaultIndex(void)         { return m_DefIndex; }
	TCustomSignature* CopyAt(int idx) const
	{
		return new TCustomSignature(GetAt(idx));
	}

protected:
	LONG   m_DefIndex;
};

typedef TCustomSignatureList<TCustomSignature, TCustomSignature&> TNewsSigList ;

// assignment
//
// ??? impure - returns void ("I can't deal with all this syntax")
template<class TYPE, class ARG_TYPE>
void TCustomSignatureList<TYPE, ARG_TYPE>::Set(TCustomSignatureList<TYPE, ARG_TYPE>* pList)
{
	if (pList == this)
		return;

	RemoveAll();

	int max = pList->GetSize();
	for (int i = 0; i < max; ++i)
		Add( pList->ElementAt(i) );

	m_DefIndex = pList->m_DefIndex;
	return;
}

template<class TYPE, class ARG_TYPE>
void TCustomSignatureList<TYPE, ARG_TYPE>::Serialize (CArchive & archive)
{
	if (archive.IsStoring())
	{
		archive << m_DefIndex;
	}
	else
	{
		archive >> m_DefIndex;
	}
	CArray<TYPE, ARG_TYPE>::Serialize ( archive );
}
