/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tdlgtmpv.cpp,v $
/*  Revision 1.2  2010/08/07 20:48:20  richard_wood
/*  Updated all WinHelp calls to HTMLHelp.
/*  Added help funcs to property pages that were missing them.
/*
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:52:01  richard_wood
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

// tdlgtmpv.cpp : implementation file
//
//
//
// Chosen Listbox -- each item has a TArticleItemViewUI*
// Available lbx  -- each item has a string-id
//

#include "stdafx.h"
#include <dlgs.h>
#include "fontdlg.h"
#include "resource.h"
#include "tdlgtmpv.h"
#include "tmpllbx.h"          // template listbox
#if !defined(CUSTVIEW_H)
#include "custview.h"
#endif
#if !defined(TGLOBOPT_H)
#include "tglobopt.h"
#endif
#if !defined(FILEUTIL_H)
#include "fileutil.h"
#endif
#include "rgbkgrnd.h"
#include "sysclr.h"
#include "helpcode.h"            // HID_OPTIONS_*
#include "rgfont.h"
#include "superstl.h"            // auto_ptr, ...
#include "gdiutil.h"             // gdiCustomColor

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;
extern TSystem        gSystem;

/////////////////////////////////////////////////////////////////////////////
// TDlgTemplateView property page

IMPLEMENT_DYNCREATE(TDlgTemplateView, CPropertyPage)

TDlgTemplateView::TDlgTemplateView() : CPropertyPage(TDlgTemplateView::IDD)
{
	m_fGetFullHeader = FALSE;
	m_defBackground = FALSE;
	m_fSkipHTMLPart = FALSE;
	m_background = RGB(255,255,255);
	m_iQuotedTextMax = 0;

	m_clrQuoted = gpGlobalOptions->GetRegFonts()->GetQuotedTextColor();
	m_clrSignature = gpGlobalOptions->GetRegFonts()->GetSignatureTextColor();
	m_fChanged = FALSE;
	m_greyBrush.CreateStockObject(LTGRAY_BRUSH);
}

void TDlgTemplateView::OnCancel()
{
	TBackgroundColors *pBackgrounds = gpGlobalOptions->GetBackgroundColors();
	COLORREF currCR = pBackgrounds->GetArtviewBackground();
	if (currCR != m_originalBackground)
		pBackgrounds->SetArtviewBackground(m_originalBackground);

	// turn these back to original settings
	gpGlobalOptions->GetRegFonts()->SetQuotedTextColor(m_originalQuoted);
	gpGlobalOptions->GetRegFonts()->SetQuotedTextFont(&m_origQuotedFont);
	gpGlobalOptions->GetRegFonts()->m_QuotedTextPtSize = m_origQuotedPtSize ;

	gpGlobalOptions->GetRegFonts()->SetSignatureTextColor(m_originalSignature);
	gpGlobalOptions->GetRegFonts()->SetSignatureTextFont(&m_origSignatureFont);
	gpGlobalOptions->GetRegFonts()->m_SignatureTextPtSize = m_origSignaturePtSize ;
}

TDlgTemplateView::~TDlgTemplateView()
{
}

void TDlgTemplateView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_VT_DEFBACK, m_defBackground);
	DDX_Check(pDX, IDC_VT_SHOWHTMLTXT, m_fSkipHTMLPart);
	DDX_Text(pDX, IDC_VIEW_QT_SUPPRESS, m_iQuotedTextMax);
	DDV_MinMaxInt(pDX, m_iQuotedTextMax, 0, 9000);

	int idx;

	CListBox * plbxAvail = GetAvail();
	TTemplateLbx* plbxChosen = GetChosen();

	if (pDX->m_bSaveAndValidate)
	{
		m_CustomView.RemoveAll();
		TTemplateLbx* plbxChosen = GetChosen();

		int total = plbxChosen->GetCount();
		for (int i = 0; i < total; ++i)
		{
			TArticleItemViewUI* pUI = plbxChosen->GetUIDataPtr(i);

			// copy that sub struct in
			m_CustomView.Add ( pUI->m_item );
		}

		//m_kSortThread = ((CComboBox *) GetDlgItem (IDC_VT_SORT_THREAD))->GetCurSel ();
	}
	else
	{
		do {
			m_fChanged = TRUE;  // Not strictly true, but at least we came to this dlg.

			// vc4.0 calls DoDataExchange more than once ?? -amc
			// guard against this
			if (plbxAvail->GetCount() > 0)
				break;
			char rcLoaded[IDS_TMPL_EX_LAST - IDS_TMPL_FIRST];
			memset (rcLoaded, 0, sizeof(rcLoaded));

			// initialize the chosen lbx first
			int total = m_CustomView.m_set.GetSize();
			for (int i = 0; i < total; ++i)
			{
				const TArticleItemView & rItem = m_CustomView.m_set.GetAt(i);
				TArticleItemViewUI* pUI = new TArticleItemViewUI ( rItem );
				int pos = plbxChosen->SendMessage(LB_INSERTSTRING, WPARAM(-1), LPARAM(pUI));

				plbxChosen->CorrectItemHeight (pos);

				// mark this as used, so it won't go in the Available lbx.
				int id = rItem.GetStringID();
				if (IDS_TMPL_FIRST < id && id < IDS_TMPL_EX_LAST)
					rcLoaded[id - IDS_TMPL_FIRST] = TRUE;
			}

			CString str;
			UINT u;
			// the first item is not a valid string, so plus one
			for (u = IDS_TMPL_FIRST + 1; u < IDS_TMPL_LAST; u++)
			{
				if ( (IDS_TMPL_BLANK == u || !rcLoaded[u - IDS_TMPL_FIRST]) && str.LoadString ( u ) )
				{
					idx = plbxAvail->AddString ( str );
					plbxAvail->SetItemData ( idx, u );          // store as extra data
				}
			}
			if (m_fGetFullHeader)
			{
				// by downloading the full header, you get more choices
				for (u = IDS_TMPL_LAST + 1; u < IDS_TMPL_EX_LAST; u++)
				{
					if ( !rcLoaded[u - IDS_TMPL_FIRST] && str.LoadString (u) )
					{
						idx = plbxAvail->AddString ( str );
						plbxAvail->SetItemData ( idx, u );
					}
				}
			}

			//((CComboBox *) GetDlgItem (IDC_VT_SORT_THREAD))->SetCurSel (m_kSortThread);
		} while (FALSE);

		OnVtDefback();
	}
}

BEGIN_MESSAGE_MAP(TDlgTemplateView, CPropertyPage)
	ON_LBN_DBLCLK(IDC_VT_AVAIL, OnDblclkVtAvail)
	ON_LBN_DBLCLK(IDC_VT_CHOSEN, OnDblclkVtChosen)
	ON_BN_CLICKED(IDC_VT_ADDBUT, OnVtAddbut)
	ON_BN_CLICKED(IDC_VT_DELBUT, OnVtDelbut)
	ON_BN_CLICKED(IDC_VT_MOVEDOWN, OnVtMovedown)
	ON_BN_CLICKED(IDC_VT_MOVEUP, OnVtMoveup)
	ON_BN_CLICKED(IDC_VT_FONT, OnVtFont)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_VIEW_DLFULLHDR, OnViewDlfullhdr)
	ON_BN_CLICKED(IDC_VT_BACKGROUND_COLOR, OnVtBackgroundColor)
	ON_BN_CLICKED(IDC_VT_DEFBACK, OnVtDefback)
	ON_BN_CLICKED(IDC_VT_QUOTED_COLOR, OnVtQuotedColor)
	ON_BN_CLICKED(IDC_VT_SIGNATURE_COLOR, OnVtSignatureColour)
	ON_WM_HELPINFO()
	ON_NOTIFY (PSN_HELP, 0, OnPSNHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TDlgTemplateView message handlers

void TDlgTemplateView::OnDblclkVtAvail()
{
	OnVtAddbut();
}

void TDlgTemplateView::OnDblclkVtChosen()
{
	OnVtDelbut();
}

void TDlgTemplateView::OnVtAddbut()
{
	CListBox * plbxAvail = GetAvail();
	CListBox * plbxChosen = GetChosen();
	CString  str;
	CList<int, int> idxList;               // transfer list
	int i, idx;

	int total = plbxAvail->GetCount();
	for (i = 0; i < total; i++)
	{
		if (plbxAvail->GetSel(i) > 0)
		{
			plbxAvail->GetText (i, str);

			int stringID = (int) plbxAvail->GetItemData(i);
			// transfer the string id. and the text
			TArticleItemViewUI* pUI = new TArticleItemViewUI (str, stringID);

			// this listbox has NO strings.  so this ptr is IT.
			idx = plbxChosen->SendMessage(LB_INSERTSTRING, WPARAM(-1), LPARAM(pUI));

			// the blank string choice, never goes on the kill-list
			if (IDS_TMPL_BLANK != stringID)
			{
				idxList.AddTail( i );
			}
		}
	}

	// delete them from the Available list
	while (idxList.GetCount())
	{
		i = idxList.RemoveTail();
		plbxAvail->DeleteString ( i );
	}
}

void TDlgTemplateView::OnVtDelbut()
{
	CListBox * plbxAvail = GetAvail();
	CListBox * plbxChosen = GetChosen();
	CString  str;
	CList<int, int> idxList;
	int i;

	int total = plbxChosen->GetCount();
	for (i = 0; i < total; i++)
	{
		if (plbxChosen->GetSel(i) > 0)
		{
			TArticleItemViewUI* pUI = 0;

			// retrieve our dataptr
			plbxChosen->SendMessage(LB_GETTEXT, i, LPARAM(&pUI));

			int stringID = pUI->m_item.GetStringID();
			if (IDS_TMPL_BLANK != stringID)
			{
				// add back to available
				int pos = plbxAvail->AddString(pUI->text);

				// transfer string id back.
				plbxAvail->SetItemData (pos, stringID );
			}

			idxList.AddTail(i );
		}
	}
	while (idxList.GetCount())
	{
		i = idxList.RemoveTail();

		plbxChosen->DeleteString ( i );
		// memory freed by owner-drawn code DeleteItem(LPDELETEITEMSTRUCT)
	}
}

void TDlgTemplateView::OnVtMovedown()
{
	TTemplateLbx* plbxChosen = GetChosen();
	int idx = plbxChosen->GetSelCount();
	if (1 != idx)
		return;
	int total = plbxChosen->GetCount();
	plbxChosen->GetSelItems(1, &idx);
	if (idx == total -1)
		return;     // already at bottom

	plbxChosen->Exchange (idx, idx + 1);
	plbxChosen->SetSel ( -1, FALSE );
	plbxChosen->SetSel ( 1 + idx, TRUE );

	// redraw those two lines
	plbxChosen->SmartRefresh ( idx );
}

void TDlgTemplateView::OnVtMoveup()
{
	TTemplateLbx* plbxChosen = GetChosen();
	int idx = plbxChosen->GetSelCount();
	if (1 != idx)
		return;
	plbxChosen->GetSelItems(1, &idx);
	if (idx == 0)
		return;     // already at top

	plbxChosen->Exchange (idx - 1, idx);

	plbxChosen->SmartRefresh (idx - 1);

	plbxChosen->SetSel ( -1, FALSE );
	plbxChosen->SetSel ( idx - 1, TRUE );
}

// --------------------------------------------------------------------------
BOOL TDlgTemplateView::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// hook in the TTemplateLbx class. It implements the owner (self-drawn)
	// functionality

	VERIFY(m_TemplateLbx.SubclassDlgItem (IDC_VT_CHOSEN, this));

	// if needed, we restore these values in OnCancel()
	m_originalBackground = m_background;
	m_originalQuoted = m_clrQuoted;
	m_originalSignature = m_clrSignature;

	CopyMemory(&m_quotedFont,
		gpGlobalOptions->GetRegFonts()->GetQuotedTextFont (),
		sizeof (m_quotedFont));
	CopyMemory(&m_signatureFont,
		gpGlobalOptions->GetRegFonts()->GetSignatureTextFont (),
		sizeof (m_signatureFont));

	CopyMemory(&m_origQuotedFont, &m_quotedFont, sizeof m_origQuotedFont);
	CopyMemory(&m_origSignatureFont, &m_signatureFont, sizeof m_origSignatureFont);

	m_origQuotedPtSize = gpGlobalOptions->GetRegFonts()->m_QuotedTextPtSize;
	m_origSignaturePtSize = gpGlobalOptions->GetRegFonts()->m_SignatureTextPtSize;

	// make sure selection is set.
	TTemplateLbx* plbxChosen = GetChosen();
	if (plbxChosen->GetCount() > 0)
		plbxChosen->SetSel(0);

	plbxChosen->m_pParentView = this;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

///////////////////////////////////////////////////////////////////////////
// 2-7-95  Added support for multiple select
//
void TDlgTemplateView::OnVtFont()
{
	COLORREF current = gSystem.Window();
	if (!m_defBackground)
		current = m_background;

	CNewFontDialog dlgFont(current);

	TTemplateLbx* plbxChosen = GetChosen();

	int selCount = plbxChosen->GetSelCount();
	if (selCount <= 0)
		return;

	int* pIdx = new int[selCount];
	auto_ptr<int> deleter(pIdx);

	plbxChosen->GetSelItems(selCount, pIdx);

	TArticleItemViewUI* pUI = (TArticleItemViewUI*) plbxChosen->GetItemDataPtr ( pIdx[0] );

	LOGFONT lf;

	CopyMemory (&lf, &pUI->m_item.m_lf, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.rgbColors = pUI->m_item.GetColor();

	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY | CF_FORCEFONTEXIST ;
	if (IDOK == dlgFont.DoModal())
	{
		int newHeight;

		// install new font into each one of the Multiple Selection
		for (int i = 0; i < selCount; ++i)
		{
			pUI = (TArticleItemViewUI*) plbxChosen->GetItemDataPtr ( pIdx[i] );

			pUI->SetNewFont ( &lf, dlgFont.GetSize() );
			pUI->m_item.SetColor ( dlgFont.GetColor() );

			plbxChosen->CalcFontheight ( pUI, newHeight );
			plbxChosen->SetItemHeight ( pIdx[i], newHeight );
		}

		plbxChosen->InvalidateRect( NULL );
	}
}

void TDlgTemplateView::InitializeCustomView (const TCustomArticleView& src)
{
	m_CustomView = src;
}

HBRUSH TDlgTemplateView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: Change any attributes of the DC here

	// TODO: Return a different brush if the default is not desired
	if (nCtlColor == CTLCOLOR_LISTBOX && (GetChosen() == (CListBox*) pWnd))
	{
		COLORREF current = gSystem.Window();

		if (!m_defBackground)
			current = m_background;
		pDC->SetBkColor( current );
		m_greyBrush.Detach();
		m_greyBrush.CreateSolidBrush (current);
		return (HBRUSH) m_greyBrush.GetSafeHandle();
	}
	return hbr;
}

void TDlgTemplateView::OnViewDlfullhdr()
{
	UINT checked =  IsDlgButtonChecked ( IDC_VIEW_DLFULLHDR );
	CListBox * plbxAvail = GetAvail();
	TTemplateLbx* plbxChosen = GetChosen();
	UINT u;
	CString str;
	int     idx;

	if (checked)
	{
		// add extras
		for (u = IDS_TMPL_LAST + 1; u < IDS_TMPL_EX_LAST; u++)
		{
			if ( str.LoadString (u) )
			{
				idx = plbxAvail->AddString ( str );
				plbxAvail->SetItemData ( idx, u );
			}
		}
	}
	else
	{
		// check if user has selected any Extended fields
		BOOL fExtended = FALSE;
		int iStringID;
		for (idx = plbxChosen->GetCount()-1; idx >= 0; idx--)
		{
			TArticleItemViewUI* pUI = plbxChosen->GetUIDataPtr(idx);
			iStringID = pUI->m_item.GetStringID();
			if ( iStringID > IDS_TMPL_LAST && iStringID < IDS_TMPL_EX_LAST)
			{
				fExtended = TRUE; break;
			}
		}
		if (fExtended)
		{
			CString one_fld;
			one_fld.LoadString (iStringID);
			CString msg; AfxFormatString1(msg, IDS_TMPL_LOSS, LPCTSTR(one_fld));

			// warn user about loss. Continue?
			if (IDNO == NewsMessageBox (this, msg, MB_YESNO | MB_ICONWARNING))
			{
				CheckDlgButton(IDC_VIEW_DLFULLHDR, 1);
				return;
			}
		}
		// remove extras from Chosen Side
		for (idx = plbxChosen->GetCount()-1; idx >= 0; idx--)
		{
			TArticleItemViewUI* pUI = plbxChosen->GetUIDataPtr(idx);
			iStringID = pUI->m_item.GetStringID();
			if ( iStringID > IDS_TMPL_LAST && iStringID < IDS_TMPL_EX_LAST)
			{
				plbxChosen->DeleteString ( idx );
			}
		}

		// remove goodies from the Available side
		for (idx = plbxAvail->GetCount()-1; idx >= 0; idx--)
		{
			iStringID = (int) plbxAvail->GetItemData(idx);
			if ( iStringID > IDS_TMPL_LAST && iStringID < IDS_TMPL_EX_LAST)
				plbxAvail->DeleteString (idx);
		}
	}
}

// 4-24-98  allow custom colors
void TDlgTemplateView::OnVtBackgroundColor()
{
	COLORREF output;

	if (gdiCustomColorDlg(this, m_background, output))
	{
		m_background = output;

		TTemplateLbx* plbxChosen = GetChosen();
		TBackgroundColors *pBack = gpGlobalOptions->GetBackgroundColors();
		pBack->SetArtviewBackground(m_background);
		plbxChosen->InvalidateRect( NULL, TRUE );
	}
}

void TDlgTemplateView::OnVtDefback()
{
	CWnd* pButton = GetDlgItem(IDC_VT_BACKGROUND_COLOR);
	if (pButton)
	{
		if (IsDlgButtonChecked(IDC_VT_DEFBACK))
		{
			m_defBackground = TRUE;
			pButton->EnableWindow(FALSE);
		}
		else
		{
			m_defBackground = FALSE;
			pButton->EnableWindow(TRUE);
		}

		// let lbx display new background color
		GetChosen()->Invalidate();
	}
}

void TDlgTemplateView::OnVtQuotedColor()
{
	COLORREF current = gSystem.Window();
	if (!m_defBackground)
		current = m_background;

	CNewFontDialog dlgFont(current);

	LOGFONT lf;

	CopyMemory (&lf, &m_quotedFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.rgbColors = m_clrQuoted;

	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY | CF_FORCEFONTEXIST;
	if (IDOK == dlgFont.DoModal())
	{
		// all this is corrected by OnCancel() if user cancels
		CopyMemory (&m_quotedFont, &lf, sizeof(m_quotedFont));
		m_clrQuoted = dlgFont.m_cf.rgbColors;

		// set it  immediately

		// this is PointSize * 10
		gpGlobalOptions->GetRegFonts()->m_QuotedTextPtSize = dlgFont.GetSize();
		gpGlobalOptions->GetRegFonts()->SetQuotedTextColor ( dlgFont.m_cf.rgbColors );
		gpGlobalOptions->GetRegFonts()->SetQuotedTextFont ( &lf );
	}
}

void TDlgTemplateView::OnVtSignatureColour()
{
	COLORREF current = gSystem.Window();
	if (!m_defBackground)
		current = m_background;

	CNewFontDialog dlgFont(current);

	LOGFONT lf;

	CopyMemory (&lf, &m_signatureFont, sizeof(lf));

	dlgFont.m_cf.lpLogFont = &lf;
	dlgFont.m_cf.rgbColors = m_clrSignature;

	dlgFont.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_ANSIONLY | CF_FORCEFONTEXIST;
	if (IDOK == dlgFont.DoModal())
	{
		// all this is corrected by OnCancel() if user cancels
		CopyMemory (&m_signatureFont, &lf, sizeof(m_quotedFont));
		m_clrSignature = dlgFont.m_cf.rgbColors;

		// set it immediately

		// this is PointSize * 10
		gpGlobalOptions->GetRegFonts()->m_SignatureTextPtSize = dlgFont.GetSize();
		gpGlobalOptions->GetRegFonts()->SetSignatureTextColor ( dlgFont.m_cf.rgbColors );
		gpGlobalOptions->GetRegFonts()->SetSignatureTextFont ( &lf );
	}
}

BOOL TDlgTemplateView::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AfxGetApp () -> HtmlHelp((DWORD)"customize-article.html", HH_DISPLAY_TOPIC);//HtmlHelp(HID_OPTIONS_ARTICLE_LAYOUT);
	return 1;
}

// -------------------------------------------------------------------------
afx_msg void TDlgTemplateView::OnPSNHelp (NMHDR *, LRESULT *)
{
	OnHelpInfo (NULL);
}
