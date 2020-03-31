/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: comptool.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:15  richard_wood
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

// comptool.cpp -- toolbar used by posting/mailing windows

#include "stdafx.h"
#include "News.h"
#include "comptool.h"

#include "utilsize.h"
#include "tglobopt.h"
#include "attdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

const BYTE kUNKNOWN = 200;

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TCompToolBar, CToolBar)
		ON_WM_DESTROY()
	
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL TCompToolBar::Create(
						  CWnd* pParentWnd,
						  int ComboBox_PlacementIndex,
						  int cbxMIME_Index)
{
	const int nDropHeight = 500;

	// Try to account for the size of the system font
	int cxSignature = 100;
	{
		CClientDC sDC(pParentWnd);
		CSize sz = sDC.GetTextExtent("No Signature M", 14);
		sz.cx = sz.cx * 5 / 4;
		cxSignature = max(cxSignature, sz.cx);
	}

	// Create the sig combo box
	SetButtonInfo(ComboBox_PlacementIndex,
		idCompTool_Combo, TBBS_SEPARATOR, cxSignature);

	// Design guide advises 12 pixel gap between combos and buttons
	SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
	CRect rect;
	GetItemRect(0, &rect);
	rect.top = 3;
	rect.bottom = rect.top + nDropHeight;
	if (!m_comboBox.Create(CBS_DROPDOWNLIST|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
		rect, this, idCompTool_Combo))
	{
		TRACE0("Failed to create compose combo-box\n");
		return FALSE;
	}

	SetButtonInfo(2, ID_OPTIONS_USEMIME, TBBS_SEPARATOR, 100);
	GetItemRect (2, &rect);
	rect.top = 3;
	rect.bottom = rect.top + nDropHeight;
	if (!m_cbxMIME.Create (CBS_DROPDOWNLIST | WS_VISIBLE | WS_TABSTOP,
		rect, this, idCompTool_cbxMime))
	{
		TRACE0("Failed to create mime combobox\n");
		return FALSE;
	}
	CString str;
	int iUUDecodeIndex;
	str.LoadString (IDS_USE_UUENCODE);
	iUUDecodeIndex = m_cbxMIME.AddString (str);
	str.LoadString (IDS_USE_MIME);
	m_iMimeIndex = m_cbxMIME.AddString (str);

	if (gpGlobalOptions->GetEncodingType() == TGlobalDef::kUUENCODE)
		m_cbxMIME.SetCurSel (iUUDecodeIndex);
	else
		m_cbxMIME.SetCurSel (m_iMimeIndex);

	SetButtonInfo (3, ID_SEPARATOR, TBBS_SEPARATOR, 12);

#if 1
	//  Fill the combo box

	// This separate copy of the Signatures may seem wasteful, but
	// the user could reconfigure the list, and the Create window would
	// be hosed... at least it will always be handy
	TNewsSigList* pSignatureList = gpGlobalOptions->GetpSignatureList();
	int iDefIndex = pSignatureList->GetDefaultIndex();
	int total = pSignatureList->GetSize();
	for (int i = 0; i < total; ++i)
	{
		TCustomSignature* pSig = pSignatureList->CopyAt(i);
		int iAdded = m_comboBox.AddString ( pSig->GetShortName() );
		m_comboBox.SetItemDataPtr( iAdded, pSig );
		if (iDefIndex == i)
		{
			m_comboBox.SetCurSel (iAdded);
			m_comboBox.SetCurrentIndex (iAdded);       // the combobox will track Current/Previous
		}
	}

#else
	CString szStyle;
	if (szStyle.LoadString(IDS_LEFT))
		m_wndStyleBar.m_comboBox.AddString((LPCTSTR)szStyle);
	if (szStyle.LoadString(IDS_CENTERED))
		m_wndStyleBar.m_comboBox.AddString((LPCTSTR)szStyle);
	if (szStyle.LoadString(IDS_RIGHT))
		m_wndStyleBar.m_comboBox.AddString((LPCTSTR)szStyle);
	if (szStyle.LoadString(IDS_JUSTIFIED))
		m_wndStyleBar.m_comboBox.AddString((LPCTSTR)szStyle);
#endif
	//  Create a font for the combobox
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));

	if (!::GetSystemMetrics(SM_DBCSENABLED))
	{
		// Since design guide says toolbars are fixed height so is the font.
		logFont.lfHeight = -12;
		logFont.lfWeight = FW_BOLD;
		logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		CString strDefaultFont;
		strDefaultFont.LoadString(IDS_COMBO_DEFFONT);
		lstrcpy(logFont.lfFaceName, strDefaultFont);
		if (!m_font.CreateFontIndirect(&logFont))
			TRACE0("Could Not create font for combo\n");
		else {
			m_comboBox.SetFont(&m_font);
			m_cbxMIME.SetFont(&m_font);
		}
	}
	else
	{
		m_font.Attach(::GetStockObject(SYSTEM_FONT));
		m_comboBox.SetFont(&m_font);
		m_cbxMIME.SetFont(&m_font);
	}

	m_byMimeLocked = kUNKNOWN;

	return TRUE;
}

// -------------------------------------------------------------------------
void TCompToolBar::SelectSig (const CString & str)
{
	for (int i = 0; i < m_comboBox.GetCount(); i++)
	{
		TCustomSignature* pSig = (TCustomSignature*) m_comboBox.GetItemDataPtr(i);

		if (pSig->GetShortName() == str)
		{
			m_comboBox.SetCurSel (i);
			break;
		}
	}
}

// -------------------------------------------------------------------------
void TCompToolBar::OnDestroy()
{
	CToolBar::OnDestroy();

	// clean up stored Signature structs
	int total = m_comboBox.GetCount();
	for (int j = 0; j < total; ++j)
	{
		TCustomSignature* pSig = (TCustomSignature*)m_comboBox.GetItemDataPtr(j);
		delete pSig;
		m_comboBox.SetItemDataPtr(j, NULL);
	}
}

// -------------------------------------------------------------------------
BOOL TCompToolBar::GetSignatureText(int iSel, CString& str)
{
	if (iSel < 0)
		return FALSE;

	TCustomSignature* pSig = 0;

	pSig = (TCustomSignature*)m_comboBox.GetItemDataPtr(iSel);

	ASSERT(pSig);

	pSig->GetSignature( str, this );
	return TRUE;
}

// -------------------------------------------------------------------------
void TCompToolBar::GetCurrent(int& iCurrent)
{
	iCurrent = m_comboBox.GetCurSel();
}

// -------------------------------------------------------------------------
BOOL TCompToolBar::MimeButtonChecked(void)
{
	int iIndex = m_cbxMIME.GetCurSel ();
	return iIndex == m_iMimeIndex;
}
