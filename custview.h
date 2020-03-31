/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: custview.h,v $
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

#include "pobject.h"

class TArticleItemView : public PObject
{
public:
	DECLARE_SERIAL(TArticleItemView)

	TArticleItemView();
	TArticleItemView(int stringID, BYTE red, BYTE green, BYTE blue);
	TArticleItemView(const TArticleItemView& src);
	~TArticleItemView(void);

	TArticleItemView& operator= (const TArticleItemView& rhs);

	virtual void Serialize (CArchive & archive);

	void SetStringID(int n) { m_itemID = n; }
	int  GetStringID(void) const   { return m_itemID; }

	COLORREF GetColor() const { return m_color; }
	void SetColor(COLORREF ref) { m_color = ref; }

	BOOL   SetNewFont(LPLOGFONT pLogFont, int PointSizeTwips);

	int    GetSizeTwips(void) const
	{
		// The font dialog gives us tenths of a point. We want twentieths
		return m_PointSize + m_PointSize;
	}

	static void InitLogfont(LOGFONT * pLF);
	const LOGFONT* GetLogfont(void) const { return &m_lf; }

	LOGFONT m_lf;

protected:
	void ClearFont();

	int       m_itemID;       // really a stringid
	COLORREF  m_color;
	LONG      m_PointSize;
};

// wrapper with extra stuff to handle self-drawn code
class TArticleItemViewUI
{
public:
	TArticleItemViewUI(const CString& str, int stringID);
	TArticleItemViewUI(const TArticleItemView& item);
	TArticleItemViewUI(const TArticleItemViewUI& src);
	~TArticleItemViewUI();

	TArticleItemViewUI& operator= (const TArticleItemViewUI& rhs);

	void ClearFont();

	BOOL SetNewFont (LPLOGFONT plf, int PointSize);
	CFont* GetFont(void) { return m_pFont; }

	CString text;
	TArticleItemView m_item;

protected:
	CFont*    m_pFont;
};

//////////////////////////////////////////////////////////////////
//void ConstructElements(TArticleItemView* pNewItemView, int nCount);
//void DestructElements(TArticleItemView* pNewItemView, int nCount);

// new STL syntax
template<> void AFXAPI SerializeElements<TArticleItemView>
(CArchive& ar, TArticleItemView* pItemView, int nCount);

class TCustomArticleView : public PObject
{
public:
	DECLARE_SERIAL(TCustomArticleView)
	TCustomArticleView();
	~TCustomArticleView() {}

	void Add(TArticleItemView& src)				{ m_set.Add(src); }
	void RemoveAll()							{ m_set.RemoveAll(); }
	int  GetSize() const						{ return m_set.GetSize(); }
	const TArticleItemView& operator[](int i)	{ return m_set.ElementAt(i); }

	TCustomArticleView& operator=(const TCustomArticleView& rhs);

	virtual void Serialize (CArchive & archive);

protected:
	void AddDefaultItems(void);

public:
	CArray<TArticleItemView, TArticleItemView&> m_set;
};
