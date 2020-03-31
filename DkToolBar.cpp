/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: DkToolBar.cpp,v $
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
/*  Revision 1.3  2009/01/26 08:34:45  richard_wood
/*  The buttons are back!!!
/*
/*  Revision 1.2  2008/09/19 14:51:06  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

// DkToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "DkToolBar.h"
#include "resource.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDkToolBar
// DKTODO:
// In the implementation file(s) of the owner(s) of your toolbars, you must
// add an array of static CToolBarInfo objects.  The CToolBarInfo array must
// be populated as follows:
// CToolBarInfo CParentWnd::toolBarInfoTable[] =
// {{ TBBUTTON }, Text to be displayed in customize dialog for this button},
// {{ TBBUTTON }, Text to be displayed in customize dialog for this button},
// ...

/////////////////////////////////////////////////////////////////////////////
// CDkToolBar message map
BEGIN_MESSAGE_MAP(CDkToolBar, CToolBar)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_POPUP_CUSTOMIZE, OnPopupCustomize)
	ON_WM_DESTROY()

	ON_NOTIFY_REFLECT(TBN_BEGINADJUST,     OnToolBarBeginAdjust)
	ON_NOTIFY_REFLECT(TBN_BEGINDRAG,       OnToolBarBeginDrag)
	ON_NOTIFY_REFLECT(TBN_CUSTHELP,        OnToolBarCustomHelp)
	ON_NOTIFY_REFLECT(TBN_ENDADJUST,       OnToolBarEndAdjust)
	ON_NOTIFY_REFLECT(TBN_ENDDRAG,         OnToolBarEndDrag)
	ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO,   OnToolBarGetButtonInfo)
	ON_NOTIFY_REFLECT(TBN_QUERYDELETE,     OnToolBarQueryDelete)
	ON_NOTIFY_REFLECT(TBN_QUERYINSERT,     OnToolBarQueryInsert)
	ON_NOTIFY_REFLECT(TBN_RESET,           OnToolBarReset)
	ON_NOTIFY_REFLECT(TBN_TOOLBARCHANGE,   OnToolBarChange)
END_MESSAGE_MAP()

// This function calculates the number of buttons on the toolbar.
int CDkToolBar::NButtons()
{
	return nButtons;
}

CDkToolBar::CDkToolBar()
{
	// CDkToolBar TODO:  add extra initialization code here
}

CDkToolBar::~CDkToolBar()
{
	// CDkToolBar TODO:  add extra uninitialization code here
}

// This function creates the toolbar and associates it with its parent.  Also,
// the registry key to be used for saving and restoring the toolbar's state is
// stored for later use.
BOOL CDkToolBar::Create(CWnd *pParentWnd,
						DWORD dwStyle,
						UINT  nID,
						int   totalButtonCount,
						CToolBarInfo *pAllButtons,
						CString regSubKey,
						CString regValue,
						HKEY regKey)
{
	BOOL		 success;		// indicates if the toolbar was created

	// if default processing is ok
	if (CToolBar::CreateEx(pParentWnd, TBSTYLE_FLAT, dwStyle))
	{
		// indicate success
		success = TRUE;

		// modify the style to include adjustable
		ModifyStyle(0, CCS_ADJUSTABLE);

		// keep the pointer to the toolbar information
		pToolBarInfo = pAllButtons;

		// if there are buttons to customize
		if (pAllButtons)
			nButtons = totalButtonCount;
		else
			nButtons = 0;
	}
	else // else default processing failed
	{
		TRACE0("Failed to create toolbar\n");
		success = FALSE;
	}

	// keep record of where our registry entry lives
	registryKey = regKey;
	registrySubKey = regSubKey;
	registryValue = regValue;

	// indicate success
	return success;
}

// This function saves the state (visible buttons, toolbar position, etc.)
// of the toolbar, using the registry key provided to the Create(...) function.
void CDkToolBar::SaveState()
{
	// if there is an associated registry subkey
	if (registrySubKey.GetLength())
	{
		// save the toolbar state to the registry
		GetToolBarCtrl().SaveState(registryKey, registrySubKey, registryValue);
	}
}

// This function restores the state (visible buttons, toolbar position, etc.)
// of the toolbar, using the registry key provided to the Create(...) function.
void CDkToolBar::RestoreState()
{
	// if there is an associated registry subkey
	if (registrySubKey.GetLength())
	{
		// RLW : If settings for the toolnar buttons
		// do not exist, create default ones.

		// Check if the reg key is there
		bool bCreate = true;
		try
		{
			CRegKey rg;
			if (ERROR_SUCCESS == rg.Open(HKEY_CURRENT_USER,
										 GetGravityRegKey()+_T("DkToolbar"),
										 KEY_READ))
			{
				// Yep, check if the value is there
				BYTE *pData;
				DWORD nDataLength;
				if (nButtons)
					nDataLength = (nButtons+1)*4+1;
				else
					nDataLength = 4096;

				pData = new BYTE[nDataLength];
				ZeroMemory(pData, nDataLength);

				if (ERROR_SUCCESS == rg.QueryBinaryValue(_T("ToolBar Settings"), pData, &nDataLength))
					bCreate = false; // Yes, don't need to create.

				delete [] pData;

				rg.Close();
			}
		}
		catch(...)
		{
			// If we couldn't open the key or the key was far longer than
			// we expected, clobber it and create again.
		}

		if (bCreate)
		{
			// Initialise it to the default values

			// Create or open the DkToolbar key.
			CRegKey rg;
			if (ERROR_SUCCESS == rg.Create(HKEY_CURRENT_USER,
										 GetGravityRegKey()+_T("DkToolbar")))
			{
				// This is the default data to create.
				// Each group of 8 hex digits is a button ID:-
				// 39800000 06810000 3A810000 04810000 05810000 FFFFFFFF 0E800000 05800000
				// 04800000 FFFFFFFF 32800000 7F800000 85800000 86800000 FFFFFFFF 38810000
				// 39810000 07800000 F4810000 27810000 A3800000 BC800000 FD800000 FFFFFFFF
				// C1800000 12800000 70800000
				BYTE pData[] = {
				 0x39, 0x80, 0x00, 0x00, 0x06, 0x81, 0x00, 0x00, 0x3A, 0x81, 0x00, 0x00, 0x04, 0x81, 0x00, 0x00,
				 0x05, 0x81, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x0E, 0x80, 0x00, 0x00, 0x05, 0x80, 0x00, 0x00,
				 0x04, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x32, 0x80, 0x00, 0x00, 0x7F, 0x80, 0x00, 0x00,
				 0x85, 0x80, 0x00, 0x00, 0x86, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x38, 0x81, 0x00, 0x00,
				 0x39, 0x81, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0xF4, 0x81, 0x00, 0x00, 0x27, 0x81, 0x00, 0x00,
				 0xA3, 0x80, 0x00, 0x00, 0xBC, 0x80, 0x00, 0x00, 0xFD, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
				 0xC1, 0x80, 0x00, 0x00, 0x12, 0x80, 0x00, 0x00, 0x70, 0x80, 0x00, 0x00
				};

				DWORD nDataLength = sizeof(pData);

				// Create the value and its data
				if (ERROR_SUCCESS != rg.SetBinaryValue(_T("ToolBar Settings"), pData, nDataLength))
				{
					// error
				}

				rg.Close();
			}
			else
			{
				// error
			}
		}

		// restore the toolbar state from the registry
		GetToolBarCtrl().RestoreState(registryKey, registrySubKey, registryValue);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDkToolBar message handlers

// This function is called when the user begins dragging a toolbar
// button or when the customization dialog is being populated with
// toolbar information.  Basically, *result should be populated with
// your answer to the question, "is the user allowed to delete this
// button?".
void CDkToolBar::OnToolBarQueryDelete(NMHDR *notify, LRESULT *result)
{
	// if we're not floating - user can delete anything
	*result = !IsFloating();
}

// This function is called when the user begins dragging a toolbar
// button or when the customization dialog is being populated with
// toolbar information.  Basically, *result should be populated with
// your answer to the question, "is the user allowed to insert a
// button to the left of this one?".
void CDkToolBar::OnToolBarQueryInsert(NMHDR *notify, LRESULT *result)
{
	// if we're not floating - user can insert anywhere
	*result = !IsFloating();
}

// This function is called whenever the user makes a change to the
// layout of the toolbar.  Calling the mainframe's RecalcLayout forces
// the toolbar to repaint itself.
void CDkToolBar::OnToolBarChange(NMHDR *notify, LRESULT *result)
{
	// force the frame window to recalculate the size
	GetParentFrame()->RecalcLayout();
}

// This function is called when the user begins dragging a toolbar button.
void CDkToolBar::OnToolBarBeginDrag(NMHDR *notify, LRESULT *result)
{
}

// This function is called when the user has completed a dragging operation.
void CDkToolBar::OnToolBarEndDrag(NMHDR *notify, LRESULT *result)
{
}

// This function is called when the user initially calls up the toolbar
// customization dialog box.
void CDkToolBar::OnToolBarBeginAdjust(NMHDR *notify, LRESULT *result)
{
}

// This function is called when the user clicks on the help button on the
// toolbar customization dialog box.
void CDkToolBar::OnToolBarCustomHelp(NMHDR *notify, LRESULT *result)
{
}

// This function is called when the user dismisses the toolbar customization
// dialog box.
void CDkToolBar::OnToolBarEndAdjust(NMHDR *notify, LRESULT *result)
{
	// save the state of the toolbar for reinitialization
	SaveState();
}

// This function is called to populate the toolbar customization dialog box
// with information regarding all of the possible toolbar buttons.
void CDkToolBar::OnToolBarGetButtonInfo(NMHDR *notify, LRESULT *result)
{
	TBNOTIFY		*tbStruct;		// data needed by customize dialog box

	// init the pointer
	tbStruct = (TBNOTIFY *)notify;

	// if the index is valid
	if (0 <= tbStruct->iItem && tbStruct->iItem < NButtons())
	{
		// copy the stored button structure
		tbStruct->tbButton = pToolBarInfo[tbStruct->iItem].tbButton;
		TRACE("%s\n",pToolBarInfo[tbStruct->iItem].btnText);
		// copy the text for the button label in the dialog
		strcpy(tbStruct->pszText, pToolBarInfo[tbStruct->iItem].btnText);

		// indicate valid data was sent
		*result = TRUE;
	}
	// else there is no button for this index
	else
	{
		*result = FALSE;
	}
}

// This function is called when the user clicks on the reset button on the
// toolbar customization dialog box.
void CDkToolBar::OnToolBarReset(NMHDR *notify, LRESULT *result)
{
	// restore the toolbar to the way it was before entering customization
	RestoreState();
}

// This function initializes and tracks the toolbar pop-up menu
void CDkToolBar::OnContextMenu(CWnd*, CPoint point)
{
	CMenu		 menu;		// toolbar right-click menu

	// if we have extensive information regarding the toolbar
	if (pToolBarInfo && !IsFloating())
	{
		// load the menu from resources
		VERIFY(menu.LoadMenu(IDR_POPUP_TOOLBAR));

		/// track the menu as a pop-up
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		// force all message in this menu to be sent here
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			this);
	}
}

// This function checks for shift-F10 to pop-up the right-click menu in the
// toolbar.
BOOL CDkToolBar::PreTranslateMessage(MSG* pMsg)
{
	BOOL		 handled;		// indicates if message was handled

	// if user hit shift-F10 or if he hit the context menu key on his keyboard
	if ((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
		(pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||
		(pMsg->message == WM_CONTEXTMENU))
	{
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);

		CPoint point = rect.TopLeft();
		point.Offset(5, 5);
		OnContextMenu(NULL, point);

		handled = TRUE;
	}

	// else let the base class handle this one
	else
	{
		handled = CToolBar::PreTranslateMessage(pMsg);
	}
	return handled;
}

// This function is called when the user clicks on the 'customize' menu item of
// the toolbar's right-click menu.
void CDkToolBar::OnPopupCustomize()
{
	// let user play with customization dialog
	GetToolBarCtrl().Customize();
}

void CDkToolBar::OnDestroy()
{
	// save the current state of the toolbar
	SaveState();

	// default processing
	CToolBar::OnDestroy();
}
