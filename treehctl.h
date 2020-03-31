/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: treehctl.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.6  2009/03/18 15:08:08  richard_wood
/*  Added link to SF Gravity web page from Help menu.
/*  Added "Wrap" command to compose menu.
/*  Changed version number (dropped minor version, now major, middle, build)
/*  Fixed bug where app would lock up if downloading & user tried to exit.
/*  Fixed bozo bin memory leak.
/*  Fixed "Sort by From" bug.
/*  Added "sort ascending" and "sort descending" arrows to thread header.
/*  Fixed width of thread header "threaded" arrow.
/*
/*  Revision 1.5  2009/02/17 14:28:23  richard_wood
/*  Deleted old commented out code.
/*
/*  Revision 1.4  2009/01/30 00:09:18  richard_wood
/*  Removed redundant files.
/*  Fixed header on thread view.
/*
/*  Revision 1.3  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.2  2008/09/19 14:52:16  richard_wood
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

// treehctl.h : header file
//

#pragma once

class TThreadListView;
class TRegUI;

const int giMinimumHdrCtrlSectionWidth = 8;

/////////////////////////////////////////////////////////////////////////////
// TTreeHeaderCtrl window

class TTreeHeaderCtrl : public CHeaderCtrl
{
public:
	TTreeHeaderCtrl();
	virtual ~TTreeHeaderCtrl();


	enum EColumn { kThread=0, kIndicator=1, kFrom=2, kSubject=3,
		kLines=4, kDate=5, kScore=6 };

	TThreadListView* m_pParentThreadView;

	BOOL IsSortAscend() { return m_fSortAscend; }
	void SetSortAscend(bool asnd) { m_fSortAscend = asnd; }

	BOOL IsThreaded()   { return kThread == m_eSortColumn ; }
	TTreeHeaderCtrl::EColumn GetSortColumn() { return m_eSortColumn; }
	void SetSortColumn (TTreeHeaderCtrl::EColumn eCol);

	static int Upgrade (int iCurBuild, TRegUI * pRegUI);

	int LoadSettings(int ParentWidth);
	int SaveSettings(bool fZoomed);
	int ResetSettings(int ParentWidth);

	int GetWidthArray(int n,  int* pWidths);  // 4 of them
	int GetOptional(CString& templat);
	int GetRightMargin(int iItem);
	int GetRightMost ();
	const CString& GetTemplate() { return m_template; }

	BOOL EmptySize()         { return m_template.IsEmpty(); }

	void OnNewsgroupSort(CWnd* pParent);    // presents Sort dialog box

	void AdjustForSmallerWidth ();

	void OnZoom (bool fZoom);

protected:
	afx_msg void OnItemChange(NMHDR* pNMHdr, LRESULT* pResult);
	afx_msg void OnItemClick(NMHDR* pNMHdr, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

	void  set_widths_profile(int ParentWidth);
	void  set_widths_average(int ParentWidth);
	void  set_widths_registry(CString& strWidths);
	static void  build_template(CString& columns, CString& strTemplate);
	void  template_insert();
	void  add_virtual_column(BOOL fUseDefault, CString& columns);
	int  insert_item(int StrID, int width, DWORD flags = 0,
		LPARAM* plAppData = 0);

	void DispatchToParent(TTreeHeaderCtrl::EColumn eOldCol,
		TTreeHeaderCtrl::EColumn eNewCol,
		BOOL fOldSortA, BOOL fSortA);

	int SaveWidthSettings(bool fZoomed);
	int SaveSortSettings();

	TTreeHeaderCtrl::EColumn GetPrevCol();

	void invalidate_column(TTreeHeaderCtrl::EColumn eCol);

	void column_change ( TTreeHeaderCtrl::EColumn eCol, BOOL* pfAscend);

	TTreeHeaderCtrl::EColumn set_column (EColumn eCol, bool draw,
		bool fAllowThreadToggle);

	int  SaveZoomedWidthSettings ();
	void ApplyWidthsFromString (CString & strWidths);

protected:
	int                          m_vWidth[7];

	CString m_template;      // looks like "FISLD"
	TTreeHeaderCtrl::EColumn     m_eSortColumn;
	TTreeHeaderCtrl::EColumn     m_ePrevSortColumn;
	BYTE                         m_fSortAscend;
	CImageList                   m_imgList;

private:
	void fnUtilHdrProportionalWidth(int nCol, int * piCX, int iTooSmall);
	void UpdateThreadedIcon();
};
