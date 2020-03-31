/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: sortart.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.2  2008/09/19 14:51:52  richard_wood
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

// SortArt.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SortArt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
TSortArticlesDlg::TSortArticlesDlg(CWnd* pParent /*=NULL*/)
: CDialog(TSortArticlesDlg::IDD, pParent)
{
}

// ------------------------------------------------------------------------
void TSortArticlesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);


	if (FALSE == pDX->m_bSaveAndValidate)
	{
		read_from_template ();
	}
	else
	{
		write_to_template ();
	}
}

// ------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TSortArticlesDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_SORTBY_LBX, OnSelchangeSortbyLbx)

END_MESSAGE_MAP()

// ------------------------------------------------------------------------
// setup according to m_strInputTemplate
void TSortArticlesDlg::read_from_template ()
{
	CListBox* pLbx = (CListBox*) GetDlgItem(IDC_SORTBY_LBX);
	CString s;
	bool selectionOK = false;
	for (int i = 0; i < m_strInputTemplate.GetLength(); ++i)
	{
		TCHAR cKey = m_strInputTemplate[i];
		int iStrId = char_to_strid(cKey);
		VERIFY (s.LoadString(iStrId));
		int idxItem = pLbx->AddString ( s );

		// keep char index as DATA
		pLbx->SetItemData(idxItem, i);

		// put selection on current sort
		if (i == m_idxTemplate)
		{
			pLbx->SetCurSel( idxItem );
			selectionOK = true;
		}
	}
	if (!selectionOK)
		pLbx->SetCurSel (0);

	CheckRadioButton(IDC_SORTBY_ASCENDING, IDC_SORTBY_DESCENDING,
		m_fSortAscending ? IDC_SORTBY_ASCENDING : IDC_SORTBY_DESCENDING);
	OnSelchangeSortbyLbx();
}

// ------------------------------------------------------------------------
void TSortArticlesDlg::write_to_template ()
{
	CListBox* pLbx = (CListBox*) GetDlgItem(IDC_SORTBY_LBX);
	int idx = pLbx->GetCurSel();
	if (LB_ERR != idx)
	{
		m_fSortAscending = IsDlgButtonChecked(IDC_SORTBY_ASCENDING);
		m_idxTemplate = (int) pLbx->GetItemData(idx);
	}
}

// ------------------------------------------------------------------------
typedef struct
{
	TCHAR c;
	int   strId;
}  STUPID_SORTBY_STRUCT;

// ------------------------------------------------------------------------
static STUPID_SORTBY_STRUCT gsSortByMap[] =
{ {'T', IDS_SORT_THREAD },
{'I', IDS_SORT_STATUS },
{'F', IDS_SORT_FROM   },
{'S', IDS_SORT_SUBJECT},
{'L', IDS_SORT_LINES  },
{'D', IDS_SORT_DATE   },
{'R', IDS_SORT_SCORE  },
};

// ------------------------------------------------------------------------
int   TSortArticlesDlg::char_to_strid (TCHAR c)
{
	int tot = sizeof(gsSortByMap) / sizeof(gsSortByMap[0]);
	for (--tot; tot >= 0; --tot)
	{
		if (c == gsSortByMap[tot].c)
			return gsSortByMap[tot].strId;
	}
	ASSERT(0);
	return 0;
}

// ------------------------------------------------------------------------
TCHAR TSortArticlesDlg::strid_to_char (int iStrID)
{
	int tot = sizeof(gsSortByMap) / sizeof(gsSortByMap[0]);
	for (--tot; tot >= 0; --tot)
	{
		if (iStrID == gsSortByMap[tot].strId)
			return gsSortByMap[tot].c;
	}
	ASSERT(0);
	return 0;
}

// ------------------------------------------------------------------------
void TSortArticlesDlg::OnSelchangeSortbyLbx()
{
}
