/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: attview.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:13  richard_wood
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

// attview.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// TAttachView form view
//  the parent CDocument must be of type CAttachmentDoc

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "attdoc.h"
#include "mime.h"

class TAttachView : public CFormView
{
protected:
	TAttachView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TAttachView)

public:
	enum { IDD = IDD_FORM_ATTACH };

public:
   CListCtrl* GetListCtrl() { return (CListCtrl*) GetDlgItem (IDC_LST_ATTACH); }

public:
   TAttachmentDoc* GetDocument(void)
      {
      ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(TAttachmentDoc)));
      return (TAttachmentDoc*) m_pDocument;
      }

   void QueryAttachments();
   void SetAttachments (int iNumAttachments,
      const TAttachmentInfo *psAttachments);

	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	virtual ~TAttachView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
   BOOL QueryMimeAttachment();
   void DeletedSelectedAttachments();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeydownLstAttach(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDeleteitemLstAttach(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg LRESULT OnContextMenu (WPARAM wParam, LPARAM lParam);
   afx_msg void OnPopupAttach();
   afx_msg void OnCmdDetach();
	DECLARE_MESSAGE_MAP()

protected:
   void AddFilenamesToList(CFileDialog & sDlg);
   void AddFilenameToList(LPCTSTR lpstrFile, LPCTSTR contentType, LPCTSTR desc,
                          TMime::ECode eCode);
   BYTE m_fColumnsSet;
   CMenu m_ContextMenu;
};
