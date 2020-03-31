/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: DkToolBar.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/26 08:34:45  richard_wood
/*  The buttons are back!!!
/*
/*  Revision 1.2  2008/09/19 14:51:06  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

// DkToolBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDkToolBar window
#pragma once

class CToolBarInfo
{
public:
	TBBUTTON		 tbButton;		// information regarding the button
	LPCTSTR		 btnText;		// text for the button
};

class CDkToolBar : public CToolBar
{
public:
	CDkToolBar();
	virtual ~CDkToolBar();

	// overridden Create(...) allows initialization of toolbar
	// information which allows user-customization; allows
	// specification of registry key which allows toolbar
	// state persistance
	BOOL Create(CWnd *pParentWnd,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		UINT nID = AFX_IDW_TOOLBAR,
		int   totalButtonCount = 0,
		CToolBarInfo *pAllButtons = NULL,
		CString regSubKey = "",
		CString regValue = "",
		HKEY regKey = HKEY_CURRENT_USER);

	// overridden LoadToolBar allows automatic restoration
	// of toolbar information and calculation of total
	// button count
	inline BOOL LoadToolBar(UINT idResource, BOOL restore = FALSE)
	{
		BOOL		 success;		// indicates success;

		// load the toolbar bitmap
		success = CToolBar::LoadToolBar(idResource);

		// count the buttons
		nButtons = GetToolBarCtrl().GetButtonCount();

		// if we're supposed to restore last setting
		if (restore)
		{
			RestoreState();
		}
		return success;
	}

public:
	virtual BOOL	 PreTranslateMessage(MSG* pMsg);
	void			 SaveState();
	void			 RestoreState();
	int				 NButtons();			// number of buttons on toolbar

protected:
	CToolBarInfo	*pToolBarInfo;		// table of buttons and text - CURRENT TOOLBAR
	int				 nButtons;			// number of buttons on toolbar - CURRENT TOOLBAR
	CSize			 defBtnSize;		// default button size
	CSize			 defImgSize;		// default image size
	CSize			 txtBtnSize;		// size of buttons with text
	BOOL			 iconsAndText;		// indicates if text is shown under icons
	HKEY			 registryKey;		// key where toolbar information is kept
	CString			 registrySubKey;	// key where toolbar information is kept
	CString			 registryValue;		// value where toolbar information is kept

	afx_msg void OnPopupCustomize();
	afx_msg void OnDestroy();
	afx_msg void OnToolBarQueryDelete(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarQueryInsert(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarChange(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarBeginDrag(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarEndDrag(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarBeginAdjust(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarCustomHelp(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarEndAdjust(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarGetButtonInfo(NMHDR *notify, LRESULT *result);
	afx_msg void OnToolBarReset(NMHDR *notify, LRESULT *result);
	afx_msg void OnContextMenu(CWnd*, CPoint point);

	DECLARE_MESSAGE_MAP()
};
