/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: posthdr.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:40  richard_wood
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

// posthdr.h -- header dialog for mailing/posting windows

#pragma once

#include <afxext.h>
#include "postdoc.h"
#include "tpostbtn.h"
#include "tabedit.h"
#include "noselebx.h"
#include "servcp.h"

// -------------------------------------------------------------------------
class TPostHdrView : public CFormView
{
protected:
	TPostHdrView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TPostHdrView)

public:
	enum { IDD = IDD_FORM_POSTHDR };
	CString  m_Groups;
	CString  m_Subject;
	CString  m_CC;

	TPostDoc * GetDocument(void);
	virtual void OnInitialUpdate();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	~TPostHdrView() {};
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHdrMorefields();
	afx_msg void OnChooseGroups();
	afx_msg void OnChangePostEditsubj();
	afx_msg LRESULT OnVerifyPostHdr(WPARAM wParam, LPARAM lParam);
	afx_msg void OnAccelCCAuthor();
	afx_msg void OnCCToAuthor ();
	afx_msg void OnCCToSelf ();
	afx_msg void OnDestroy();
	afx_msg void OnJumpToEditBox(UINT nID);
	DECLARE_MESSAGE_MAP()

protected:
	BOOL AddNewsgroups(TArticleHeader * pHdr);
	void SetWindowTitle (LPCTSTR pchSubject);
	void choose_groups(int idEbx);
	void size_mail(UINT nType, int cx, int cy);
	void size_post(UINT nType, int cx, int cy);
	void move_ebx_button(int iRightSide, int iCX, int idEbx, int idBut);
	void OnCCButton (CButton *pButton, const CString &strAddress);

	int  m_EditFieldLeft;
	int  m_EditFieldLen;
	TPostBtn m_sMoreFieldsButton;
	TTabEdit m_sSubjectEbx;
	TNoSelectEdit m_sNewsGroupsEbx;

	BOOL m_bInitializing;         // TRUE before OnInitialUpdate() is called
	CString m_strReplyTo;         // 'reply to' address, used in OnCCToAuthor()
	TServerCountedPtr m_cpServer; // smart pointer
};

// used to communicate between the posting template and both
// TPostHdrView and TPostToolBar
extern int giMailing;   // very ugly, I know
