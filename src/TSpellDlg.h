/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TSpellDlg.h,v $
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
/*  Revision 1.4  2008/09/19 14:51:08  richard_wood
/*  Updated for VS 2005
/*
/*****************************************************************************/

#pragma once

#include <list>
#include <string>
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// TSpellDlg dialog

class TSpellDlg : public CDialog
{
public:
	std::list<std::string> m_suggestList;

	TSpellDlg(CWnd* pParent = NULL);   // standard constructor

	void SetBadWord(CString strWord) { m_strWord = strWord; };
	CString GetCorrectedWord() { return m_strCorrect; };
	enum { IDD = IDD_CHECK_SPELL };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnCorrect();
	afx_msg void OnAddWord();
	afx_msg void OnIgnore();
	afx_msg void OnDblclkSuggestions();
	afx_msg void OnAbort();
	afx_msg void OnBnClickedOptions();
	DECLARE_MESSAGE_MAP()

private:
	void RememberLocation();
	void SetState();

	BOOL m_bEditing;
	CString m_strWord;
	CString m_strCorrect;

	CEdit m_badWord;
	CButton	m_btnCorrect;

	CListBox m_lbxSuggestions;

	static CPoint pos;
public:
	afx_msg void OnEnChangeBadWord();
	afx_msg void OnLbnSelchangeSuggestions();
};
