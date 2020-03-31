/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: layoutpg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:29  richard_wood
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

// layoutpg.h : header file
//

#pragma once

#include "uimem.h"
#include "lay3lbx.h"
#include "laytlbx.h"

class TRegLayoutMdi;

/////////////////////////////////////////////////////////////////////////////
// TOptionsLayoutPage dialog

class TOptionsLayoutPage : public CPropertyPage
{
	DECLARE_DYNCREATE(TOptionsLayoutPage)

public:
	TOptionsLayoutPage();
	~TOptionsLayoutPage();

	void DataIn(TRegLayoutMdi* pRegLayoutMdi);
	void DataOut(TRegLayoutMdi* pRegLayoutMdi);

	enum { IDD = IDD_OPTIONS_LAYOUT };
	CButton	m_s3Up;
	CButton	m_s3Down;

	TUIMemory::EMdiLayout m_eLayout;
	TUIMemory::EPaneType  m_vPanes[3];

	TUIMemory::EMdiLayout m_bakLayout;    //  copy of original so
	TUIMemory::EPaneType  m_bakPanes[3];  //     we know if anything changed

	BOOL IsDifferent();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
	virtual BOOL OnInitDialog();
	afx_msg void On3Down();
	afx_msg void On3Up();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()

	LRESULT  lbxBeginDrag(WPARAM wParam, DRAGLISTINFO* pInfo);
	LRESULT  lbxCancelDrag(WPARAM wParam, DRAGLISTINFO* pInfo);
	LRESULT  lbxDragging(WPARAM wParam, DRAGLISTINFO* pInfo);
	LRESULT  lbxDropped(WPARAM wParam, DRAGLISTINFO* pInfo);

protected:
	TLayout3Lbx m_lbx3;
	TLayoutTextLbx  m_lbxText3;

	UINT  uDragListboxMessage;
	int   m_iDragCtrlId;
	int   m_iDragIndex;
};
