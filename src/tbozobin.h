/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tbozobin.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.2  2008/09/19 14:51:58  richard_wood
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

// tbozobin.h -- Bozo Bin dialog

#pragma once

#include "resource.h"
#include "expirable.h"           // TExpirable
#include "pobject.h"             // PObject
#include "tsearch.h"             // TSearch

class TNewsGroup;
class TArticleHeader;

int BozoCallback (TArticleHeader *pHeader, int iSelected,
				  TNewsGroup *pNG, DWORD dwData);
void BozoCallbackCommit ();

// -------------------------------------------------------------------------
#define BOZO_VERSION 1

class TBozo : public PObject, public TExpirable {
public:
	DECLARE_SERIAL (TBozo)
	TBozo () {};
	TBozo (const TBozo &src) { *this = src; }
	TBozo &operator= (const TBozo &src)
	{
		TExpirable::operator=(src);

		m_strName = src.m_strName;
		return *this;
	}
	void Serialize (CArchive &archive);

	CString m_strName;
	TSearch m_sSearch;
};

// -------------------------------------------------------------------------
#define BOZO_LIST_VERSION 1

class TBozoList : public CObList {
public:
	DECLARE_SERIAL (TBozoList)
	TBozoList();
	~TBozoList();
	TBozo &GetNext (POSITION &pos)
	{ return *(TBozo *) CObList::GetNext (pos); }
	POSITION AddTail (const TBozo &item)
	{
		TBozo *pNew = new TBozo (item);
		return CObList::AddTail (pNew);
	}
	void RemoveAll ();
	BOOL BozoExists (const CString &strBozo);
};

// -------------------------------------------------------------------------
class TBozoBin : public CDialog
{
public:
	TBozoBin(CWnd* pParent = NULL);   // standard constructor
	static void TBozoBin::AddBozo (const char *pchBozo);

	enum { IDD = IDD_BOZO };
	CEdit m_sPurgeDays;
	CButton  m_sEdit;
	CListCtrl   m_sNames;
	CButton  m_sDelete;
	CButton  m_sAdd;
	CString  m_strNames;
	BOOL  m_bPurge;
	UINT  m_iPurgeDays;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemchangedNames(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnEdit();
	afx_msg void OnDblclkNames(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPurge();
	DECLARE_MESSAGE_MAP()

	void GrayControls ();
	void AddRow (LPCTSTR pchName, const CTime &sLastSeen, int iIndex = -1);
	void ShowColumn2 (int iIndex);
};

// -------------------------------------------------------------------------
class TBozoEdit : public CDialog
{
public:
	TBozoEdit(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_EDIT_BOZO };
	CButton  m_sOK;
	CString  m_strName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeName();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

	void GrayControls ();
};

void CheckForBozo (TArticleHeader *pHeader, TNewsGroup *pNG);
void ApplyBozoToCurrentGroup ();
void ConvertBozos ();
