/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ndirpick.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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
/*  Revision 1.2  2008/09/19 14:51:33  richard_wood
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

#include "stdafx.h"
#include "dlgs.h"
#include "ndirpick.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CNTDirDlg::CNTDirDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
					 LPCSTR lpszDefExt,
					 LPCSTR lpszFileName,
					 DWORD dwFlags,
					 LPCSTR lpszFilter,
					 CWnd* pParentWnd) : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
					 dwFlags, lpszFilter, pParentWnd)
{
}

BEGIN_MESSAGE_MAP(CNTDirDlg, CFileDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirpkrApp commands

BOOL CNTDirDlg::OnInitDialog()
{
	CenterWindow();

	//Let's hide these windows so the user cannot tab to them.  Note that in
	//the private template (in cddemo.dlg) the coordinates for these guys are
	//*outside* the coordinates of the dlg window itself.  Without the following
	//ShowWindow()'s you would not see them, but could still tab to them.

	GetDlgItem(stc2)->ShowWindow(SW_HIDE);
	GetDlgItem(stc3)->ShowWindow(SW_HIDE);
	GetDlgItem(edt1)->ShowWindow(SW_HIDE);
	GetDlgItem(lst1)->ShowWindow(SW_HIDE);
	GetDlgItem(cmb1)->ShowWindow(SW_HIDE);

	//We must put something in this field, even though it is hidden.  This is
	//because if this field is empty, or has something like "*.txt" in it,
	//and the user hits OK, the dlg will NOT close.  We'll jam something in
	//there (like "Junk") so when the user hits OK, the dlg terminates.
	//Note that we'll deal with the "Junk" during return processing (see below)

	SetDlgItemText(edt1, "Junk");

	//Now set the focus to the directories listbox.  Due to some painting
	//problems, we *must* also process the first WM_PAINT that comes through
	//and set the current selection at that point.  Setting the selection
	//here will NOT work.  See comment below in the on paint handler.

	GetDlgItem(lst2)->SetFocus();

	m_bDlgJustCameUp=TRUE;

	CFileDialog::OnInitDialog();

	return(FALSE);
}

void CNTDirDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here

	//This code makes the directory listbox "highlight" an entry when it first
	//comes up.  W/O this code, the focus is on the directory listbox, but no
	//focus rectangle is drawn and no entries are selected.  Ho hum.

	if (m_bDlgJustCameUp)
	{
		m_bDlgJustCameUp=FALSE;
		SendDlgItemMessage(lst2, LB_SETCURSEL, 0, 0L);
	}

	// Do not call CFileDialog::OnPaint() for painting messages
}
