/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TSpellDlg.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/05/27 19:52:04  richard_wood
/*  Fixed bug in spell checker where I changed everything to lower case before checking.
/*  Added highlighting of the misspelled word.
/*
/*  Revision 1.4  2008/09/19 14:51:08  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

// TSpellDlg.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "TSpellDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPoint TSpellDlg::pos(0, 0);

/////////////////////////////////////////////////////////////////////////////
// TSpellDlg dialog

TSpellDlg::TSpellDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TSpellDlg::IDD, pParent)
	, m_bEditing(FALSE)
{
}

void TSpellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BAD_WORD, m_badWord);
	DDX_Control(pDX, IDC_CORRECT, m_btnCorrect);
	DDX_Control(pDX, IDC_SUGGESTIONS, m_lbxSuggestions);

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		m_badWord.SetWindowText( m_strWord );

		std::list<std::string>::iterator it = m_suggestList.begin();
		for (; it != m_suggestList.end(); it++)
		{
			m_lbxSuggestions.AddString ( (*it).c_str() );
		}
		int count = m_lbxSuggestions.GetCount();
		if (1 == count)
			m_lbxSuggestions.SetCurSel(0);
		m_btnCorrect.EnableWindow(count > 0);
	}
}

BEGIN_MESSAGE_MAP(TSpellDlg, CDialog)
	ON_BN_CLICKED(IDC_CORRECT, OnCorrect)
	ON_BN_CLICKED(IDC_ADD_WORD, OnAddWord)
	ON_BN_CLICKED(IDOK, OnIgnore)
	ON_LBN_DBLCLK(IDC_SUGGESTIONS, OnDblclkSuggestions)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	ON_BN_CLICKED(IDC_OPTIONS, OnBnClickedOptions)
	ON_EN_CHANGE(IDC_BAD_WORD, &TSpellDlg::OnEnChangeBadWord)
	ON_LBN_SELCHANGE(IDC_SUGGESTIONS, &TSpellDlg::OnLbnSelchangeSuggestions)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TSpellDlg message handlers

// Correct with either the chosen list box item
// or the edited word in the edit box
void TSpellDlg::OnCorrect()
{
	// Correct word
	if (m_bEditing)
	{
		// Corrected word comes from badWord edit box
		m_badWord.GetWindowText(m_strCorrect);
		m_strCorrect.Trim();
	}
	else
	{
		// Corrected word comes from listbox
		int index = m_lbxSuggestions.GetCurSel();
		if (LB_ERR == index)
			return;
		else
			m_lbxSuggestions.GetText (index, m_strCorrect);
	}
	RememberLocation();
	EndDialog(IDC_CORRECT);
}

// If user has typed a new word in then use this to correct and
void TSpellDlg::OnAddWord()
{
	if (m_bEditing)
	{
		// Correct word and then add to user dictionary
		m_badWord.GetWindowText(m_strCorrect);
		m_strCorrect.Trim();
		RememberLocation();
		EndDialog(IDC_CORRECT_AND_ADD);
	}
	else
	{
		// Add word to dictionary
		RememberLocation();
		EndDialog(IDC_ADD_WORD);
	}
}

void TSpellDlg::OnIgnore()
{
	if (m_bEditing)
	{
		// Undo
		m_badWord.SetWindowText(m_strWord);
		m_bEditing = FALSE;
		SetState();
	}
	else
	{
		// Ignore
		RememberLocation();
		EndDialog(IDOK);
	}
}

// Open the spell checker options dialog
void TSpellDlg::OnBnClickedOptions()
{
}

void TSpellDlg::OnDblclkSuggestions() 
{
	if (m_bEditing) // Cannot perform this command when editing bad word
	{
		ASSERT(FALSE);
		return;
	}

	OnCorrect();
}

void TSpellDlg::OnAbort() 
{
	RememberLocation();
	EndDialog(IDC_ABORT);
}

void TSpellDlg::RememberLocation()
{
	RECT rct;
	GetWindowRect (&rct);

	pos.x = rct.left; pos.y = rct.top;
}

BOOL TSpellDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// test if static position is valid
	if (pos.x && pos.y)
		SetWindowPos (NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// Set dialogs initial state
	m_bEditing = FALSE;
	SetState();

	return TRUE;
}

void TSpellDlg::SetState()
{
	m_badWord.GetWindowText(m_strCorrect);
	m_strCorrect.Trim();
	bool bEditedWordInvalid = false;
	bool bEditedWordCannotBeAdded = false;

	if (m_strCorrect.GetLength() == 0)
	{
		// User has deleted bad word, so we are in edit mode but correct buttons are disabled
		m_bEditing = TRUE;
		bEditedWordInvalid = true;
	}
	else if (m_strCorrect.Compare(m_strWord) != 0)
	{
		// Word has been changed so we are in edit mode
		m_bEditing = TRUE;
		if (m_strCorrect.FindOneOf(" \t") != -1)
		{
			// Edited word has spaces/tabs in - can use it to correct but cannot add to dictionary
			bEditedWordCannotBeAdded = true;
		}
	}
	else
		m_bEditing = FALSE;

	if (bEditedWordInvalid)
		bEditedWordCannotBeAdded = true;

	if (!m_bEditing)
	{
		GetDlgItem(IDC_SUGGESTIONS)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->SetWindowText("&Ignore Once");
		GetDlgItem(IDC_ADD_WORD)->SetWindowText("&Add to Dictionary");
		GetDlgItem(IDC_ADD_WORD)->EnableWindow(TRUE);
		GetDlgItem(IDC_CORRECT)->EnableWindow(m_lbxSuggestions.GetCurSel() == -1 ? FALSE : TRUE);
		m_badWord.SetSel(-1,-1);
	}
	else
	{
		GetDlgItem(IDC_SUGGESTIONS)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->SetWindowText("&Undo");
		GetDlgItem(IDC_ADD_WORD)->SetWindowText("Correct and &Add");
		GetDlgItem(IDC_ADD_WORD)->EnableWindow(bEditedWordCannotBeAdded ? FALSE : TRUE);
		GetDlgItem(IDC_CORRECT)->EnableWindow(bEditedWordInvalid ? FALSE : TRUE);
	}
}

void TSpellDlg::OnEnChangeBadWord()
{
	SetState();
}

void TSpellDlg::OnLbnSelchangeSuggestions()
{
	SetState();
}
