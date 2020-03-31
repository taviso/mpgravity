/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment(optional)                          */
/*                                                                           */
/*  $Log: posthdr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
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
/*  Revision 1.2  2008/09/19 14:51:40  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

/**********************************************************************************
Copyright(c) 2003, Albert M. Choy
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
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************/

// posthdr.cpp -- header dialog for mailing/posting windows

#include "stdafx.h"
#include "news.h"
#include "posthdr.h"
#include "postdoc.h"
#include "arttext.h"
#include "posttmpl.h"
#include "advmhdr.h"             // advanced mailing address fields Dialog
#include "advphdr.h"             // advanced posting address fields Dialog
#include "tglobopt.h"
#include "pikngdlg.h"
#include "nglist.h"
#include "server.h"              // TNewsServer
#include "custmsg.h"             // WMU_VERIFY_POSTHDR
#include "fileutil.h"            // NewsMessageBox
#include "veraddr.h"             // TVerifyAddressDlg
#include "postpfrm.h"            // TPostPopFrame
#include "genutil.h"             // CopyCStringList()
#include "article.h"             // TPersist822Header, ...
#include "rgswit.h"              // TRegSwitch
#include "servcp.h"              // TServerCountedPtr

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions* gpGlobalOptions;

IMPLEMENT_DYNCREATE(TPostHdrView, CFormView)

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPostHdrView, CFormView)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_MESSAGE(WMU_VERIFY_POSTHDR, OnVerifyPostHdr)
	ON_COMMAND(IDM_CHOOSE_GROUP, OnChooseGroups)
	ON_COMMAND(ID_JUMPTO_CC_AUTHOR, OnAccelCCAuthor)
	ON_COMMAND(ID_JUMPTO_ADVANCEDHDRS, OnHdrMorefields)     // fake accelerator
	ON_COMMAND(ID_JUMPTO_PICKNEWSGRPS, OnChooseGroups)      // fake accelerator
	ON_BN_CLICKED(IDC_HDR_MOREFIELDS, OnHdrMorefields)
	ON_BN_CLICKED(IDC_CC_TO_AUTHOR, OnCCToAuthor)
	ON_BN_CLICKED(IDC_CC_TO_SELF, OnCCToSelf)
	ON_EN_CHANGE(IDC_REPLY_EDITSUBJ, OnChangePostEditsubj)
	ON_EN_CHANGE(IDC_POST_EDITSUBJ, OnChangePostEditsubj)
	ON_COMMAND_RANGE(ID_JUMPTO_SUBJ, ID_JUMPTO_CC, OnJumpToEditBox)
END_MESSAGE_MAP()

// -------------------------------------------------------------------------
int giMailing;  // very ugly, I know

TPostHdrView::TPostHdrView()
	: CFormView(giMailing ? IDD_FORM_REPLYHDR : IDD_FORM_POSTHDR)
{
	m_Groups = _T("");
	m_Subject = _T("");
	m_CC = _T("");

	m_EditFieldLeft = 0;
	m_EditFieldLen = 0;
	m_bInitializing = TRUE;
}

// -------------------------------------------------------------------------
TPostDoc* TPostHdrView::GetDocument(void)
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(TPostDoc)));
	return(TPostDoc*) m_pDocument;
}

// -------------------------------------------------------------------------
// GetField -- takes a header line and returns the field name
static CString GetField(CString &strLine)
{
	int iPos = strLine.Find(_T(":"));
	if (iPos == -1)
		return _T("");
	CString strField = strLine.Left(iPos);
	strField.TrimLeft();
	strField.TrimRight();
	return strField;
}

// -------------------------------------------------------------------------
static BOOL FieldExists(CStringList &sCustomHeaders, CString &strAdd)
{
	POSITION pos = sCustomHeaders.GetHeadPosition();
	while(pos) 
	{
		CString &strHeader = sCustomHeaders.GetNext(pos);
		CString strField = GetField(strHeader);
		if (!strField.CompareNoCase(strAdd))
			return TRUE;
	}

	return FALSE;
}

// -------------------------------------------------------------------------
// AddCustomHeaders -- takes a list of custom headers and a list of custom
// headers to add to the first list.  Only those which are not duplicate fields
// are added
static void AddCustomHeaders(CStringList &sCustomHeaders, CStringList &sAdd)
{
	POSITION pos = sAdd.GetHeadPosition();
	while(pos) 
	{
		CString &strAdd = sAdd.GetNext(pos);
		CString strField = GetField(strAdd);
		// allow empty fields names for now... they'll be caught later when sending
		if (!strField.IsEmpty() &&
			!FieldExists(sCustomHeaders, strField))
			sCustomHeaders.AddTail(strAdd);
	}
}

// -------------------------------------------------------------------------
// Get custom headers from the Newsgroup and from the Global scope
static void GetCustomHeaders(CStringList &sCustomHeaders, LONG lGroupID)
{
	// first read the per-group headers, then add the global custom headers
	CStringList strListTemp;

	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock(GetCountedActiveServer(), lGroupID, &fUseLock, pNG);
	if (fUseLock)
	{
		// the list could exist, but still be disabled, so check it.
		if (pNG->GetOverrideCustomHeaders())
		{
			CopyCStringList(strListTemp, pNG->GetCustomHeaders());
			AddCustomHeaders(sCustomHeaders, strListTemp);
		}
	}

	CopyCStringList(strListTemp, gpGlobalOptions->GetCustomHeaders());
	AddCustomHeaders(sCustomHeaders, strListTemp);
}

// -------------------------------------------------------------------------
// VerifyCustomHeaders -- returns FALSE if custom headers are okay... prints
// a warning for each bad one
static BOOL VerifyCustomHeaders(const CStringList &sHeaders, CWnd *pWnd)
{
	BOOL bResult = FALSE;

	POSITION pos = sHeaders.GetHeadPosition();
	while(pos)
	{
		const CString &str = sHeaders.GetNext(pos);

		// verify that there is a ':' character
		BOOL bBad = FALSE;

		if (str.Find(_T(":")) == -1)
			bBad = TRUE;

		if (bBad)
		{
			bResult = TRUE;
			CString strError; strError.LoadString(IDS_ERR_BAD_HEADER);
			strError += str;
			pWnd->MessageBox(strError);
		}
	}

	return bResult;
}

// -------------------------------------------------------------------------
void TPostHdrView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	TPostDoc *pDoc = GetDocument();
	TPostTemplate *pTemplate = (TPostTemplate *) pDoc->GetDocTemplate();

	if (FALSE == pDX->m_bSaveAndValidate)
	{
		// set attachments
		TPersist822Header *pHeader = NULL;

		if (pTemplate->TestFlag(TPT_COPY_ARTICLE_HEADER))
			pHeader = (TPersist822Header *) pTemplate->m_pCopyArtHdr;

		if (pTemplate->TestFlag(TPT_COPY_MAIL_HEADER))
			pHeader = (TPersist822Header *) pTemplate->m_pCopyMailHdr;

		if (pHeader && pHeader->NumAttachments()) 
		{
			CWnd *pParent = GetParent();       // splitter 1
			pParent = pParent->GetParent();  // splitter 2
			TPostPopFrame *pFrame = (TPostPopFrame *) pParent->GetParent();
			pFrame->SetAttachments(pHeader->NumAttachments(),
				pHeader->Attachments());
		}

		// remember the 'reply to' address, used in OnCCToAuthor()
		if (pTemplate->TestFlag(TPT_FOLLOWUP)) 
		{
			pTemplate->m_pArtText->GetReplyTo(m_strReplyTo);
			if (m_strReplyTo.IsEmpty()) 
			{
				CString strAuthorName;   // not used
				pTemplate->m_pArtHdr->ParseFrom(strAuthorName, m_strReplyTo);
			}
		}

		// initialize groups
		if (pTemplate->TestFlag(TPT_FOLLOWUP))
			if (!pTemplate->m_pArtText->GetFollowup(m_Groups) ||
				m_Groups == "poster") 
			{
				if (!pTemplate->m_pArtText->GetNewsgroups(m_Groups))
					m_Groups = pTemplate->m_pArtText->GetPostToLine();
			}

			if (m_Groups.IsEmpty() &&
				pTemplate->TestFlag(TPT_TO_NEWSGROUP) &&
				pTemplate->m_NewsGroupID) 
			{
				BOOL fUseLock;
				TNewsGroup* pNG;
				TNewsGroupUseLock useLock(m_cpServer, pTemplate->m_NewsGroupID,
					&fUseLock, pNG);
				if (fUseLock)
					m_Groups = pNG->GetName();
			}

			if (m_Groups.IsEmpty() &&
				pTemplate->TestFlag(TPT_TO_NEWSGROUPS)) 
			{
				for(int i = 0; i < pTemplate->m_rNewsgroups.GetSize(); i++) 
				{
					BOOL fUseLock;
					TNewsGroup* pNG;
					TNewsGroupUseLock useLock(m_cpServer, pTemplate->m_rNewsgroups [i],
						&fUseLock, pNG);
					if (fUseLock) 
					{
						if (!m_Groups.IsEmpty())
							m_Groups += ", ";
						m_Groups += pNG->GetName();
					}
				}
			}

			if (pTemplate->TestFlag(TPT_TO_STRING))
				m_Groups = pTemplate->m_strTo;

			// initialize the subject
			if (pTemplate->TestFlag(TPT_INIT_SUBJECT))
				if (!_tcsnicmp(pTemplate->m_strSubjPrefix,
					pTemplate->m_strSubject,
					pTemplate->m_strSubjPrefix.GetLength()))
					m_Subject = pTemplate->m_strSubject;
				else
					m_Subject = pTemplate->m_strSubjPrefix + pTemplate->m_strSubject;

			CString friendly_from;

			// if we are re-hydrating a draft msg, then we should use the from already set.
			CString old_from;

			if (pDoc->m_bMailing)
				old_from = pDoc->GetEmailHeader()->GetFrom();
			else
				old_from = pDoc->GetHeader()->GetOrigFrom();  // skip translation, go direct

			if (FALSE == old_from.IsEmpty() && 0 == pTemplate->m_NewsGroupID)
			{
				friendly_from = old_from;
			}
			else
			{
				fnFromLine(friendly_from, pTemplate->m_NewsGroupID,
					FALSE /* fAddressFirst */, pDoc->m_bMailing /* fMailing */);
			}

			if (pDoc->m_bMailing) 
			{
				// set these fields unless the header is initialized elsewhere
				if (!pTemplate->TestFlag(TPT_COPY_MAIL_HEADER)) 
				{
					TEmailHeader *pHdr = pDoc->GetEmailHeader();
					pHdr->SetFrom(friendly_from);
					pHdr->SetReplyTo(m_cpServer->GetReplyTo());
				}
				else
				{
#if defined(_DEBUG)
					// test
					TEmailHeader *pHdrEmail = pDoc->GetEmailHeader();

					CString testFrom    = pHdrEmail->GetFrom();
					CString testReplyTo = pHdrEmail->GetReplyTo();
#endif
				}
			}
			else // we are posting
			{
				// set these fields unless the header is initialized elsewhere
				if (!pTemplate->TestFlag(TPT_COPY_ARTICLE_HEADER)) 
				{
					TArticleHeader *pHdr = pDoc->GetHeader();
					pHdr->SetFrom(friendly_from);
					pHdr->SetOrganization(m_cpServer->GetOrganization());
					pHdr->SetReplyTo(m_cpServer->GetReplyTo());
					pHdr->SetDistribution(gpGlobalOptions->GetDistribution());

					CStringList sCustomHeaders;
					GetCustomHeaders(sCustomHeaders, pTemplate->m_NewsGroupID);
					pHdr->SetCustomHeaders(sCustomHeaders);
				}
				else
				{
#if defined(_DEBUG)
					TArticleHeader *pHdr = pDoc->GetHeader();

					CString testFrom    = pHdr->GetFrom();
					int i = 0;
#endif
				}

				if (true)
				{
					CString strDraftCC;
					TArticleHeader *pHdr = pDoc->GetHeader();

					pHdr->GetCCEmailString(strDraftCC);

					// set this up, so it will be transferred to the EditBox
					if (!strDraftCC.IsEmpty())
						m_CC += strDraftCC;
				}

				if (pTemplate->TestFlag(TPT_FOLLOWUP) &&
					pTemplate->TestFlag(TPT_POST_CC_AUTHOR)) 
				{

					CheckDlgButton(IDC_CC_TO_AUTHOR, 1);

					// This will cause the cc field to be updated correctly. We can't
					// call OnCCToAuthor() from here because the ddx stuff in this
					// function interferes
					PostMessage(WM_COMMAND, IDC_CC_TO_AUTHOR);
				}

				// read cc-to-self setting
				if (gpGlobalOptions->GetRegSwitch()->m_fCCSelf) 
				{

					CheckDlgButton(IDC_CC_TO_SELF, 1);

					// This will cause the cc field to be updated correctly. We can't
					// call OnCCToAuthor() from here because the ddx stuff in this
					// function interferes
					PostMessage(WM_COMMAND, IDC_CC_TO_SELF);
				}
			}

			// ugly!! needs cleanup(consolidate with other sections in this if-case)
			if (pTemplate->TestFlag(TPT_READ_ARTICLE_HEADER)) 
			{
				TEmailHeader *pHdr = GetDocument()->GetEmailHeader();
				pHdr->Set_InReplyTo(pTemplate->m_pArtHdr->GetMessageID());
			}
	} // SaveAndValidate == false

	if (pDoc->m_bMailing)
	{
		DDX_Text(pDX, IDC_REPLY_EDITSUBJ, m_Subject);
		DDX_Text(pDX, IDC_REPLY_EDITTO, m_Groups);
	}
	else
	{
		DDX_Text(pDX, IDC_POST_EDITSUBJ, m_Subject);
		DDX_Text(pDX, IDC_POST_EDITNGRP, m_Groups);
		DDX_Text(pDX, IDC_POST_CC, m_CC);
	}

	if (pDX->m_bSaveAndValidate)
	{
		if (pDoc->m_bMailing)
		{
			TEmailHeader *pMailHdr = GetDocument()->GetEmailHeader();
			pMailHdr->SetSubject(m_Subject);

			if (!fnVerifyParse(this, m_Groups, 0 /* TO: */, pMailHdr, TRUE))
			{
				pDX->Fail(); // abort
			}

			if (0 == pMailHdr->GetDestinationCount(TBase822Header::kAddrTo))
			{
				CString msg; msg.LoadString(IDS_WARN_NORECIPIENTS);
				MessageBox( msg );
				pDX->Fail();   // abort
			}

		}
		else  // we are posting
		{
			TArticleHeader *pHdr = GetDocument()->GetHeader();

			pHdr->SetSubject(m_Subject);

			// deal with the comma list
			AddNewsgroups(pHdr);

			if (!m_CC.IsEmpty())
			{
				TEmailHeader *pMailHdr = GetDocument()->GetEmailHeader();
				CString strMe;
				fnFromLine(strMe, pTemplate->m_NewsGroupID,
					FALSE /* fAddressFirst */, TRUE /* fMailing */);
				pMailHdr->SetFrom(strMe);
				pMailHdr->SetSubject(m_Subject);
				if (!fnVerifyParse(this, m_CC, 0 /* TO: */, pMailHdr, TRUE))
					pDX->Fail(); // abort
			}

			// in case we save a Draft, store the MailTo('CC') list in the ArtHdr
			pHdr->SetCCEmailString(m_CC);

			if (VerifyCustomHeaders(pHdr->GetCustomHeaders(), this))
				pDX->Fail();
		}
	}
}

// -------------------------------------------------------------------------
//
void TPostHdrView::OnAccelCCAuthor()
{
	CButton *psCCToAuthor = (CButton *) GetDlgItem(IDC_CC_TO_AUTHOR);

	// if activated via accelerator(ID_JUMPTO_CC_AUTHOR = Alt-T)
	//  make sure check box is not hidden. E.g. Hidden by Post Window.
	if (!psCCToAuthor || !psCCToAuthor->IsWindowVisible())
		return;

	// manually toggle auto-check box
	CheckDlgButton(IDC_CC_TO_AUTHOR,
		IsDlgButtonChecked(IDC_CC_TO_AUTHOR) ? 0 : 1);

	// real processing
	OnCCToAuthor();
}

// -------------------------------------------------------------------------
void TPostHdrView::OnCCButton(CButton *pButton, const CString &strAddress)
{
	TPostTemplate *pTemplate =
		(TPostTemplate *) GetDocument()->GetDocTemplate();

	int iPutIn = pButton->GetState() & 3;   // is it checked?
	CEdit *psCC = (CEdit *) GetDlgItem(IDC_POST_CC);
	int iSize = psCC->LineLength();
	CString strCC; psCC->GetWindowText(strCC);
	int iOffset = strCC.Find(strAddress);

	if (iPutIn) 
	{
		if (iOffset != -1)
			return;     // already there
		if (strCC.GetLength())
			strCC += ", ";
		strCC += strAddress;
	}
	else 
	{
		if (iOffset == -1)
			return;     // not there
		CString strNewCC = strCC.Left(iOffset);
		int iRightEdge = iOffset + strAddress.GetLength();
		for(; iRightEdge < strCC.GetLength(); iRightEdge++)
			if (strCC [iRightEdge] != ' ' && strCC [iRightEdge] != ',')
				break;
		strNewCC += strCC.Right(strCC.GetLength() - iRightEdge);

		// clean up... take out any spaces and commas at the right
		while(!strNewCC.IsEmpty() &&
			(strNewCC.Right(1) == " " ||
			strNewCC.Right(1) == ","))
			strNewCC = strNewCC.Left(strNewCC.GetLength() - 1);

		strCC = strNewCC;
	}

	psCC->SetWindowText(strCC);

	CWnd *pParent = GetParent();       // splitter 1
	pParent = pParent->GetParent();  // splitter 2
	TPostPopFrame *pFrame = (TPostPopFrame *) pParent->GetParent();
	pFrame->SetCCIntro(iPutIn);
}

// -------------------------------------------------------------------------
//  5-20-96  amc  Check 'replyto' field
void TPostHdrView::OnCCToAuthor()
{
	CButton *psCCToAuthor = (CButton *) GetDlgItem(IDC_CC_TO_AUTHOR);
	OnCCButton(psCCToAuthor, m_strReplyTo);
}

// -------------------------------------------------------------------------
void TPostHdrView::OnCCToSelf()
{
	TServerCountedPtr cpNewsServer;
	CString strMyAddress = cpNewsServer->GetEmailAddress();
	CButton *psCCToSelf = (CButton *) GetDlgItem(IDC_CC_TO_SELF);
	OnCCButton(psCCToSelf, strMyAddress);
}

// -------------------------------------------------------------------------
// add each item in the comma separated list
BOOL TPostHdrView::AddNewsgroups(TArticleHeader* pHdr)
{
	TStringList lst;

	lst.FillByCommaList( m_Groups, TRUE ); // strip white space

	pHdr->ClearNewsGroups();
	POSITION pos = lst.GetHeadPosition();
	while(pos)
		pHdr->AddNewsGroup( lst.GetNext(pos) );

	return TRUE;
}

// -------------------------------------------------------------------------
#ifdef _DEBUG
void TPostHdrView::AssertValid() const
{
	CFormView::AssertValid();
}

void TPostHdrView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// -------------------------------------------------------------------------
void TPostHdrView::OnSize(UINT nType, int cx, int cy)
{
	// if m_bInitializing is true, then the document won't be valid, and we must
	// read giMailing
	BOOL bMailing = m_bInitializing ? giMailing : GetDocument()->m_bMailing;

	CFormView::OnSize(nType, cx, cy);

	if (bMailing)
		size_mail( nType, cx, cy );
	else
		size_post( nType, cx, cy );
}

// -------------------------------------------------------------------------
void TPostHdrView::size_mail(UINT nType, int cx, int cy)
{
	CWnd* pEdit;
	pEdit = (CWnd*) GetDlgItem(IDC_REPLY_EDITTO);
	if (NULL == pEdit)
		return ;

	RECT rct;

	// remember the initial width
	if (m_EditFieldLeft == 0)
	{
		pEdit->GetClientRect( &rct );
		pEdit->MapWindowPoints( this,   // destination coordinates
			&rct );
		m_EditFieldLeft = rct.left;
		m_EditFieldLen = rct.right - rct.left;
	}

	GetClientRect( &rct );

	// what width can we have?
	int newWidth;
	if (rct.right >= m_EditFieldLeft + m_EditFieldLen + 10)
	{
		newWidth = rct.right - 10 - m_EditFieldLeft;
	}
	else
		newWidth = m_EditFieldLen;

	RECT winRct;

	// resize all three
	pEdit->GetWindowRect( &winRct );
	// set new width, keep orig height, don't move X-Y
	pEdit->SetWindowPos(NULL, NULL, NULL, newWidth,
		winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);

	pEdit = GetDlgItem(IDC_REPLY_EDITSUBJ);
	if (pEdit)
	{
		pEdit->GetWindowRect( &winRct );
		pEdit->SetWindowPos(NULL, NULL, NULL, newWidth,
			winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);
	}

	CWnd* pBut = GetDlgItem( IDC_HDR_MOREFIELDS );
	if (pBut)
	{
		pBut->GetWindowRect( &winRct );
		pBut->SetWindowPos(NULL, NULL, NULL, newWidth,
			winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);
	}
}

// -------------------------------------------------------------------------
void TPostHdrView::size_post(UINT nType, int cx, int cy)
{
	// we want to resize the 4 edit fields 'to fit'
	RECT rct;
	CRect winRct;  // stores orig height
	CEdit * pEdit;

	pEdit = (CEdit*) GetDlgItem(IDC_POST_EDITSUBJ);
	if (NULL == pEdit)
		return ;

	// remember the initial width
	if (m_EditFieldLeft == 0)
	{
		pEdit->GetClientRect( &rct );
		pEdit->MapWindowPoints( this,   // destination coordinates
			&rct );
		m_EditFieldLeft = rct.left;
		m_EditFieldLen = rct.right - rct.left;
	}

	// what width can we have?
	int newWidth;
	if (cx >= m_EditFieldLeft + m_EditFieldLen + 10)
		newWidth = cx - 10 - m_EditFieldLeft;
	else
		newWidth = m_EditFieldLen;

	// resize all three
	pEdit->GetWindowRect( &winRct );
	// set new width, keep orig height, don't move X-Y
	pEdit->SetWindowPos(NULL, NULL, NULL, newWidth,
		winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);

	// "newsgroups" editbox and a button
	int iSubjectRT = winRct.left + newWidth;  // right side of Subject ebx
	move_ebx_button( iSubjectRT, newWidth, IDC_POST_EDITNGRP, IDM_CHOOSE_GROUP );

	// move cc-to-self checkbox
	int iCCToSelfWidth = 0;
	CWnd *pCCToSelf = GetDlgItem(IDC_CC_TO_SELF);
	CRect rcTemp;
	pCCToSelf->GetClientRect(&rcTemp);
	iCCToSelfWidth = rcTemp.right - rcTemp.left;
	pCCToSelf->GetWindowRect(&rcTemp);
	CPoint sPoint(iSubjectRT - iCCToSelfWidth, rcTemp.top);
	ScreenToClient(&sPoint);
	pCCToSelf->SetWindowPos(NULL, sPoint.x, sPoint.y,
		0, 0, SWP_NOSIZE | SWP_NOZORDER);

	CButton *psCCButton = (CButton *) GetDlgItem(IDC_CC_TO_AUTHOR);
	if (psCCButton)
	{
		psCCButton->EnableWindow(FALSE);
		psCCButton->ShowWindow(SW_HIDE);
	}

	// disable this button for TPostHdrView... it will be enabled in a subclass's
	// on_size if the subclass wants it
	pEdit = (CEdit*) GetDlgItem(IDC_POST_CC);
	if (pEdit)
	{
		pEdit->GetWindowRect( &winRct );
		pEdit->SetWindowPos(NULL, NULL, NULL, newWidth - iCCToSelfWidth - 8,
			winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);
	}

	CWnd* pButton = GetDlgItem(IDC_HDR_MOREFIELDS);
	if (pButton)
	{
		pButton->GetWindowRect( &winRct );
		pButton->SetWindowPos(NULL, NULL, NULL, newWidth,
			winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);
	}

	// if following up, handle the extra fields
	TPostTemplate *pTemplate =
		(TPostTemplate *) GetDocument()->GetDocTemplate();
	if (pTemplate->TestFlag(TPT_FOLLOWUP)) 
	{
		CEdit *pEdit = (CEdit *) GetDlgItem(IDC_POST_EDITSUBJ);
		if (!pEdit)
			return;
		RECT winRct;  // stores orig height
		pEdit->GetWindowRect(&winRct);

		int newWidth;
		if (cx >= m_EditFieldLeft + m_EditFieldLen + 10)
			newWidth = cx - 10 - m_EditFieldLeft;
		else
			newWidth = m_EditFieldLen;

		int iCCWidth = 0;
		CButton *psCCButton = (CButton *) GetDlgItem(IDC_CC_TO_AUTHOR);
		if (psCCButton)
		{
			// this control was disabled and hidden by TPostHdrView::OnSize()
			psCCButton->EnableWindow(TRUE);
			psCCButton->ShowWindow(SW_SHOW);

			CRect rcTemp;
			psCCButton->GetClientRect(&rcTemp);
			iCCWidth = rcTemp.right - rcTemp.left;
			psCCButton->GetWindowRect(&rcTemp);
			CPoint sPoint(iSubjectRT - iCCToSelfWidth - 8 - iCCWidth, rcTemp.top);
			ScreenToClient(&sPoint);
			psCCButton->SetWindowPos(NULL, sPoint.x, sPoint.y,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		pEdit = (CEdit*) GetDlgItem(IDC_POST_CC);
		if (pEdit)
		{
			pEdit->GetWindowRect( &winRct );
			pEdit->SetWindowPos(NULL, NULL, NULL,
				newWidth - iCCWidth - 8 - iCCToSelfWidth - 8,
				winRct.bottom - winRct.top, SWP_NOZORDER | SWP_NOMOVE);
		}
	}
}

// -------------------------------------------------------------------------
//                         +------ Right Side
//                         v
//      [Edit box  ][Button]
//
void TPostHdrView::move_ebx_button(int iRightSide, int iCX, int idEbx, int idBut)
{
	CRect  winRct;
	CPoint ptButton;
	int iCGWidth = 0;    // CG == Choose Group
	CButton *psCGButton = (CButton *) GetDlgItem(idBut);
	if (psCGButton)
	{
		psCGButton->GetWindowRect(&winRct);
		iCGWidth = winRct.Width();

		ptButton.x = iRightSide - iCGWidth;
		ptButton.y = winRct.top;

		// back to client coords
		ScreenToClient( &ptButton );
		psCGButton->SetWindowPos(NULL, ptButton.x, ptButton.y,
			NULL, NULL, SWP_NOZORDER | SWP_NOSIZE);
	}

	CEdit * pEdit = (CEdit*) GetDlgItem(idEbx);
	if (pEdit)
	{
		pEdit->GetWindowRect( &winRct );
		pEdit->SetWindowPos(NULL, NULL, NULL, iCX - iCGWidth - 8,
			winRct.Height(), SWP_NOZORDER | SWP_NOMOVE);
	}
}

// -------------------------------------------------------------------------
void TPostHdrView::OnHdrMorefields()
{
	TPostDoc *pDoc = GetDocument();

	if (pDoc->m_bMailing) 
	{
		TEmailHeader *pHdr = GetDocument()->GetEmailHeader();
		TAdvMailHeaders sDlg(this, m_cpServer, pHdr, pDoc->m_lGroupID);

		if (IDOK == sDlg.DoModal())
			sDlg.ExportDlgData( pHdr );

	}
	else 
	{
		TAdvancedPostHdr sDlg(this, m_cpServer, pDoc->m_lGroupID);

		TArticleHeader* pHdr = GetDocument()->GetHeader();
		sDlg.m_dist         = pHdr->GetDistribution();
		sDlg.m_expires      = pHdr->GetExpires();
		sDlg.m_from         = pHdr->GetFrom();
		sDlg.m_followup     = pHdr->GetFollowup();
		sDlg.m_keywords     = pHdr->GetKeywords();
		sDlg.m_organization = pHdr->GetOrganization();
		sDlg.m_replyto      = pHdr->GetReplyTo();
		sDlg.m_sender       = pHdr->GetSender();
		sDlg.m_summary      = pHdr->GetSummary();
		CopyCStringList(sDlg.m_sCustomHeaders, pHdr->GetCustomHeaders());

		if (IDOK == sDlg.DoModal())
		{
			pHdr->SetDistribution( sDlg.m_dist );
			pHdr->SetExpires     ( sDlg.m_expires );
			pHdr->SetFollowup    ( sDlg.m_followup );
			pHdr->SetFrom        ( sDlg.m_from );
			pHdr->SetKeywords    ( sDlg.m_keywords );
			pHdr->SetOrganization( sDlg.m_organization );
			pHdr->SetReplyTo     ( sDlg.m_replyto );
			pHdr->SetSender      ( sDlg.m_sender );
			pHdr->SetSummary     ( sDlg.m_summary );
			pHdr->SetCustomHeaders( sDlg.m_sCustomHeaders );
		}
	}
}

// -------------------------------------------------------------------------
void TPostHdrView::OnInitialUpdate()
{
	m_bInitializing = FALSE;      // OK, now we can trust the document
	BOOL bMailing = GetDocument()->m_bMailing;

	// subclass this, so we can tab to the main RichEdit box
	m_sSubjectEbx.SubclassWindow(
		::GetDlgItem(m_hWnd, bMailing ? IDC_REPLY_EDITSUBJ : IDC_POST_EDITSUBJ));

	// subclass windows, to prevent SELECTING ALL THE TEXT when it GETS FOCUS
	m_sNewsGroupsEbx.SubclassWindow(
		::GetDlgItem(m_hWnd, bMailing ? IDC_REPLY_EDITTO : IDC_POST_EDITNGRP));

	CFormView::OnInitialUpdate();  // this does data-exchange

	// place keyboard focus... if "to" field is empty, put it there, otherwise,
	// if "subject" is empty, put it there, otherwise, put it in the body
	int iFieldID;
	if (m_Groups.IsEmpty())
		iFieldID = bMailing ? IDC_REPLY_EDITTO : IDC_POST_EDITNGRP;
	else
		iFieldID = bMailing ? IDC_REPLY_EDITSUBJ : IDC_POST_EDITSUBJ;
	GetDlgItem(iFieldID)->SetFocus();

	if (!m_Groups.IsEmpty() && !m_Subject.IsEmpty())
		GetParent()->PostMessage(WM_COMMAND, ID_TAB_TO_EDIT);

	// set the initial window title... if no subject yet, leave it alone
	if (!m_Subject.IsEmpty())
	{
		// ultra long subject lines cause a crash in MFC function
		// CFrameWnd::UpdateFrameTitleForDocument

		GetDocument()->SetTitle(  m_Subject.Left(155 + _MAX_PATH)  );
	}
}

// -------------------------------------------------------------------------
void TPostHdrView::OnChooseGroups()
{
	CWnd * pBut = GetDlgItem(IDM_CHOOSE_GROUP);

	// if button is hidden, this could be an e-mail window
	if (pBut && pBut->IsWindowVisible())
		choose_groups(IDC_POST_EDITNGRP);
}

// -------------------------------------------------------------------------
// does work for "Newsgroups" and "Followup-To"
void TPostHdrView::choose_groups(int idEbx)
{
	TStringList myList;
	CString bigCommaString;
	CEdit* pEdit = (CEdit*) GetDlgItem(idEbx);

	// fill a string list
	pEdit->GetWindowText(bigCommaString);
	myList.FillByCommaList(bigCommaString);

	TPickNewsgroupDlg dlg(&myList, this);
	if (IDOK == dlg.DoModal())
	{
		bigCommaString.Empty();
		myList.CommaList(bigCommaString);
		pEdit->SetWindowText( bigCommaString );
	}
}

// -------------------------------------------------------------------------
void TPostHdrView::SetWindowTitle(LPCTSTR pchSubject)
{
	CWnd *pWnd = GetTopLevelParent();
	if (!pWnd)
		return;

	pWnd ->SetWindowText(pchSubject);
}

// -------------------------------------------------------------------------
void TPostHdrView::OnChangePostEditsubj()
{
	CEdit *pEdit = (CEdit*) GetDlgItem(
		GetDocument()->m_bMailing ? IDC_REPLY_EDITSUBJ : IDC_POST_EDITSUBJ);
	ASSERT(pEdit);
	CString strSubject;
	pEdit->GetWindowText(strSubject);
	SetWindowTitle(strSubject);
}

// -------------------------------------------------------------------------
LRESULT TPostHdrView::OnVerifyPostHdr(WPARAM wParam, LPARAM lParam)
{
	// warn about no subject
	if (m_Subject.IsEmpty())
	{
		NewsMessageBox(this, IDS_WARN_NOSUBJECT, MB_OK | MB_ICONSTOP);
		GetDlgItem(IDC_POST_EDITSUBJ)->SetFocus();
		return 1;
	}

	// warn about no newsgroup
	if (m_Groups.IsEmpty())
	{
		NewsMessageBox(this, IDS_WARN_POSTGROUPS, MB_OK | MB_ICONSTOP);
		GetDlgItem(IDC_POST_EDITNGRP)->SetFocus();
		return 1;
	}

	return 0;
}

// -------------------------------------------------------------------------
//  This essentially handles accelerators for the Post, Reply, Followup
//   while in the PostBody pane.  These are specifically routed to us
//   by TPostPopFrame::OnCmdMsg
void TPostHdrView::OnJumpToEditBox(UINT nID)
{
	CWnd* pEbx = 0;
	switch(nID)
	{
	case ID_JUMPTO_SUBJ:
		pEbx = GetDlgItem(GetDocument()->m_bMailing
			? IDC_REPLY_EDITSUBJ : IDC_POST_EDITSUBJ);
		break;

	case ID_JUMPTO_TO:
		pEbx = GetDlgItem(IDC_REPLY_EDITTO);
		break;

	case ID_JUMPTO_NEWSGROUPS:
		pEbx = GetDlgItem(IDC_POST_EDITNGRP);
		break;

	case ID_JUMPTO_CC:
		pEbx = GetDlgItem(IDC_POST_CC);
		break;
	}

	// make sure it is visible(not hidden on purpose)
	if (pEbx && pEbx->IsWindowVisible())
	{
		if (GetParent()->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
			((CSplitterWnd*)GetParent())->SetActivePane(NULL, NULL, this) ;

		pEbx->SetFocus();
	}
}

// -------------------------------------------------------------------------
void TPostHdrView::OnDestroy()
{
	// save cc-to-self setting
	CButton *psCCToSelf = (CButton *) GetDlgItem(IDC_CC_TO_SELF);
	if (psCCToSelf) 
	{
		BOOL bCCToSelf = psCCToSelf->GetState() ? 1 : 0;
		gpGlobalOptions->GetRegSwitch()->m_fCCSelf = bCCToSelf;
		gpGlobalOptions->GetRegSwitch()->Save();
	}

	CFormView::OnDestroy();
}
