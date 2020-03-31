/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postdoc.cpp,v $
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
/*  Revision 1.7  2009/04/11 23:55:57  richard_wood
/*  Updates for bugs 2745988, 2546351, 2622598, 2637852, 2731453, 2674637.
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/09/19 14:51:40  richard_wood
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

// postdoc.cpp -- document class for posting/mailing windows

#include "stdafx.h"
#include "news.h"
#include "postdoc.h"
#include "tmutex.h"
#include "arttext.h"
#include "msgid.h"
#include "grplist.h"
#include "posttmpl.h"
#include "coding.h"
#include "tglobopt.h"
#include "warndlg.h"
#include "rgwarn.h"
#include "custmsg.h"
#include "fileutil.h"
#include "hints.h"               // SENDHINT_80CHARPERLINE
#include "postpfrm.h"            // call CancelWarning()
#include "server.h"
#include "servcp.h"              // TServerCountedPtr
#include "rgswit.h"              // TRegSwitch
#include "genutil.h"             // MsgResource()
#include "8859x.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

IMPLEMENT_DYNCREATE(TPostDoc, TAttachmentDoc)

// -------------------------------------------------------------------------
TPostDoc::TPostDoc()
{
	m_pArtHeader = 0;
	m_pEmailHeader = 0;

	// real values will be set in OnNewDocument()
	m_bMailing = FALSE;
	m_lGroupID = 0;

	m_pServer =  GetCountedActiveServer ();
	m_pServer->AddRef ();
}

// -------------------------------------------------------------------------
TPostDoc::~TPostDoc()
{
	if (m_pArtHeader)
		delete m_pArtHeader;
	if (m_pEmailHeader)
		delete m_pEmailHeader;

	m_pServer->Release ();
}

// -------------------------------------------------------------------------
BOOL TPostDoc::OnNewDocument()
{
	if (!TAttachmentDoc::OnNewDocument())
		return FALSE;

	m_pArtHeader = new TArticleHeader;
	m_pEmailHeader = new TEmailHeader;

	TPostTemplate* pTemplate = (TPostTemplate*) GetDocTemplate ();
	m_bMailing = pTemplate->TestFlag (TPT_MAIL);
	m_lGroupID = pTemplate->m_NewsGroupID;

	if (pTemplate->TestFlag (TPT_FOLLOWUP)) {
		CString strID = pTemplate->m_pArtHdr->GetMessageID ();
		ASSERT (!strID.IsEmpty ());
		m_pArtHeader->ConstructReferences (
			* pTemplate->m_pArtHdr->CastToArticleHeader (), strID);
	}

	// m_iCancelWarningID -- this is accessed by the frame class
	m_iCancelWarningID = pTemplate->m_iCancelWarningID;

	if (pTemplate->TestFlag (TPT_COPY_ARTICLE_HEADER))
	{
		// use the object's assignment operator
		*(m_pArtHeader) = *(pTemplate->m_pCopyArtHdr);
	}

	if (pTemplate->TestFlag (TPT_COPY_MAIL_HEADER)) {

		// marwan, 7/4/97 -- took out the assignment-operator usage and put
		// the individual assignments back in because of a problem with the
		// back-to-edit feature (changing the subject in the copy would change
		// the original message's subject in the outbox)

		TEmailHeader *&pHdr = m_pEmailHeader;
		TEmailHeader *&pSrc = pTemplate->m_pCopyMailHdr;
		pHdr->SetFrom     (pSrc->GetFrom ());
		pHdr->SetKeywords (pSrc->GetKeywords ());
		pHdr->SetReplyTo  (pSrc->GetReplyTo ());
		pHdr->SetSender   (pSrc->GetSender ());
		CString str;
		pSrc->GetCCText (str);
		pHdr->ParseCC (str);
		str = "";
		pSrc->GetBCCText (str);
		pHdr->ParseBCC (str);
		pHdr->SetSubject (pSrc->GetSubject ());
		pHdr->SetNumber (pSrc->GetNumber ());
	}

	return TRUE;
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(TPostDoc, TAttachmentDoc)
	ON_COMMAND(IDM_POST_SEND, OnPostSend)
	ON_COMMAND(IDM_DRAFT, OnSaveDraft)
	ON_COMMAND(IDM_CANCEL, OnPostCancel)
	ON_COMMAND(ID_CHECK_SPELLING, OnCheckSpelling)
	ON_COMMAND(ID_SELECTION_WRAP, OnSelectionWrap)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
#ifdef _DEBUG
void TPostDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void TPostDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// -------------------------------------------------------------------------
void TPostDoc::OnPostSend()
{
	SendTheMessage();
}

// -------------------------------------------------------------------------
void TPostDoc::OnSaveDraft()
{
	SaveTheDraftMessage();
}

// -------------------------------------------------------------------------
// gather data
// put data into PostDoc
// notify the other view to save data into the PostDoc
void TPostDoc::SendTheMessage ()
{
	POSITION pos = GetFirstViewPosition ();
	CView *pView = GetNextView (pos);
	ASSERT (pView);

	if (true)
	{
		// check spelling before confirming the send
		check_spelling ();

		// see if they really want to leave...
		if (gpGlobalOptions->WarnOnSending())
		{
			BOOL  fDisableWarning = FALSE;

			if (WarnWithCBX (IDS_WARNING_SEND, &fDisableWarning, pView))
			{
				if (fDisableWarning)
				{
					gpGlobalOptions->SetWarnOnSending(FALSE);
					TRegWarn *pRegWarning = gpGlobalOptions->GetRegWarn ();
					pRegWarning->Save();
				}
			}
			else
				return;
		}

		// check for lines longer 80 chars
		if (FALSE == check_format())
			return ;
	}

	// inherited
	if (FALSE == UpdateData ())
		return ;   // DDV_ failure?

	TOutbox *pOutbox = m_pServer->GetOutbox ();
	TDraftMsgs * pDrafts = m_pServer->GetDraftMsgs ();

	// Jan-10-2003
	CString strContentType;
	GravCharset* pCharset = gsCharMaster.findById( gpGlobalOptions->GetSendCharset() );
	if (NULL == pCharset) {
		CString msg; msg.Format("Could not find charset %d", gpGlobalOptions->GetSendCharset());
		NewsMessageBox (pView, msg, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	strContentType.Format("text/plain; charset=\"%s\"", (LPCTSTR) pCharset->GetName());

	LPCTSTR pszCTE =  "7bit";

	// this information can be updated after we call CP_Out.bound
	Message_SetMimeLines (m_bMailing ? true : false,
		"1.0",           // Mime-Version
		strContentType,  // Content-Type
		pszCTE,          // Content-Transfer-Encoding
		"");             // Content-Description

	if (m_bMailing)
	{
		// before sending, tell the outbox that we are sending this message so
		// it gets a chance to delete the original copy if it exists
		LONG lArticle = m_pEmailHeader->GetNumber ();
		pDrafts->SavingNewCopy (lArticle);

		// in base class
		SendEmail (m_pEmailHeader, &m_EmailText);
	}
	else
	{
		BOOL fStop;

		if (true)
		{
			if ((fStop = check_empty_fields ()) == TRUE)
				return;

			TStringList moderatedNGList;

			// check for newsgroups that don't allow posting ( do this
			//  before we start chunkifying )
			BOOL bSomeInvalidGroups = FALSE, bNoValidGroups = FALSE;
			allow_post (&moderatedNGList, bSomeInvalidGroups, bNoValidGroups);

			if (bNoValidGroups)
			{
				NewsMessageBox (pView, IDS_WARN_NOVALIDGROUPS, MB_OK | MB_ICONSTOP);
				return;
			}
			if (bSomeInvalidGroups)
				if (IDYES != NewsMessageBox (pView, IDS_WARN_SOMEINVALIDGROUPS,
					MB_YESNO | MB_ICONWARNING))
					return;
		}

		// before posting, tell the drafts that we are sending this article so
		// it gets a chance to delete the original copy if it exists
		pDrafts->SavingNewCopy (m_pArtHeader->GetNumber ());

		CDocTemplate* pDocTemplate = GetDocTemplate();
		TPostTemplate* pTemplate = (TPostTemplate*) pDocTemplate;
		CStringList groups;
		m_pArtHeader->GetNewsGroups (&groups);
		if (groups.GetCount() > 0)
		{
			// from TAttDoc
			PostArticle ( m_pArtHeader, &m_ArtText);
		}

		// if there exist addresses in the CC-by-mail recipient list, send email
		if (m_pEmailHeader->GetDestinationCount(TBase822Header::kAddrTo) > 0)
		{
			m_EmailText.SetBody (m_ArtText.GetBody ());
			// from TAttDoc
			SendEmail (m_pEmailHeader, &m_EmailText);
		}
	}

	SetSavedStatus (TRUE);
	AfxGetMainWnd ()->SendMessage (WMU_REFRESH_OUTBOX);
	AfxGetMainWnd ()->SendMessage (WMU_REFRESH_OUTBOX_BAR);

	// close the window
	CloseFrameWnd();
}

// -------------------------------------------------------------------------
// Fill an array with information on each newsgroup
// (PostingAllowed, PostingNotAllowed, PostingModerated)
//
// normal C-function: i didn't want newsgroup.h in postdoc.h
static int classify (const TStringList& nglist, TNewsGroup::EPosting* pStatus,
					 int count)
{
	TGroupList* pGroupList = 0;
	TNewsGroup* pNG;
	POSITION pos = nglist.GetHeadPosition();

	TServerCountedPtr cpNewsServer;            // smart pointer

	TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();
	TNewsGroupArrayReadLock ngMgr(vNewsGroups);
	try
	{
		for (int i = 0; i < count; ++i)
		{
			BOOL fUseLock;
			CString ngName = nglist.GetNext( pos );

			TNewsGroupUseLock useLock (cpNewsServer, ngName, &fUseLock, pNG);
			if (fUseLock)
			{
				pStatus[i] = pNG->GetPostingState();
			}
			else
			{
				if (0 == pGroupList)
				{
					// load this only if necessary
					pGroupList = cpNewsServer->LoadServerGroupList ();
				}
				BYTE result = pGroupList->GetGroupType( ngName );
				if (0 == result)  // not found!
					pStatus[i] = TNewsGroup::kPostInvalid;
				else
					pStatus[i] = TNewsGroup::EPosting(result);
			}
		}
	}
	catch(...)
	{
		delete pGroupList;
		throw;
		return 0;
	}
	delete pGroupList;
	return 0;
}

// -------------------------------------------------------------------------
int TPostDoc::allow_post(TStringList* pModeratedList, BOOL &bSomeInvalidGroups,
						 BOOL &bNoValidGroups)
{
	// powerful low-level func gives total access
	TStringList destNGList;
	m_pArtHeader->GetDestList( &destNGList );
	int count = destNGList.GetCount();
	TStringList killList;

	TNewsGroup::EPosting* pvStatus = 0;
	try
	{
		pvStatus = new TNewsGroup::EPosting[count];
		ZeroMemory(pvStatus, count*sizeof(TNewsGroup::EPosting));

		classify (destNGList, pvStatus, count);
		POSITION pos = destNGList.GetTailPosition();
		POSITION old;
		for (int i = count-1; i >= 0; --i)
		{
			old = pos;
			CString name = destNGList.GetPrev(pos);
			if (pvStatus[i] == TNewsGroup::kPostInvalid ||
				pvStatus[i] == TNewsGroup::kPostNotAllowed)
			{
				destNGList.RemoveAt(old);
				killList.AddHead (name);
			}
		}

		if (killList.GetCount() > 0)
		{
			CString end;
			killList.CommaList ( end );
			CString str; str.LoadString (IDS_REMOVED);
			CString st = str + ": " + end;
			POSITION pos = GetFirstViewPosition ();
			NewsMessageBox (GetNextView(pos), st);
			bSomeInvalidGroups = TRUE;
		}

		if (destNGList.IsEmpty ())
			bNoValidGroups = TRUE;
	}
	catch(...)
	{
		delete [] pvStatus;
		throw;
	}
	delete [] pvStatus;
	return 0;
}

// -------------------------------------------------------------------------
void TPostDoc::OnPostCancel()
{
	// from TAttachmentDoc
	CloseFrameWnd();
}

// -------------------------------------------------------------------------
CString TPostDoc::Message_GetFrom(bool fMailing)
{
	if (fMailing)
		return m_pEmailHeader->GetFrom ();
	else
		return m_pArtHeader->GetFrom ();
}

// -------------------------------------------------------------------------
void TPostDoc::Message_SetFrom(bool fMailing, const CString & from)
{
	if (fMailing)
		m_pEmailHeader->SetFrom (from);
	else
		m_pArtHeader->SetFrom (from);
}

// -------------------------------------------------------------------------
LPCTSTR TPostDoc::Message_GetSubject(bool fMailing)
{
	if (fMailing)
		return m_pEmailHeader->GetSubject ();
	else
		return m_pArtHeader->GetSubject ();
}

// -------------------------------------------------------------------------
void TPostDoc::Message_SetSubject(bool fMailing, LPCTSTR subj)
{
	if (fMailing)
		m_pEmailHeader->SetSubject (subj);
	else
		m_pArtHeader->SetSubject (subj);
}

// -------------------------------------------------------------------------
const CString& TPostDoc::Message_GetBody (bool fMailing)
{
	return fMailing ? m_EmailText.GetBody () : m_ArtText.GetBody ();
}

// -------------------------------------------------------------------------
// override a virtual
void TPostDoc::Message_SetBody (bool fMailing, CFile& body, int len /* = -1 */)
{
	if (fMailing)
		m_EmailText.SetBody (body, len);
	else
		m_ArtText.SetBody (body, len);
}

// -------------------------------------------------------------------------
// override a virtual
void TPostDoc::Message_SetBody (bool fMailing, const CString& str)
{
	if (fMailing)
		m_EmailText.SetBody (str);
	else
		m_ArtText.SetBody (str);
}

// -------------------------------------------------------------------------
void TPostDoc::Message_GetMimeLines (bool     fMailing,
									 CString* pVer,
									 CString* pTyp,
									 CString* pEnc,
									 CString* pDesc)
{
	if (fMailing)
		m_pEmailHeader->GetMimeLines (pVer, pTyp, pEnc, pDesc);
	else
		m_pArtHeader->GetMimeLines (pVer, pTyp, pEnc, pDesc);
}

// -------------------------------------------------------------------------
void TPostDoc::Message_SetMimeLines(bool     fMailing,
									LPCTSTR  ver,
									LPCTSTR  typ,
									LPCTSTR  enc,
									LPCTSTR  desc)
{
	if (fMailing)
		m_pEmailHeader->SetMimeLines (ver, typ, enc, desc);
	else
		m_pArtHeader->SetMimeLines (ver, typ, enc, desc);
}

// -------------------------------------------------------------------------
// Check body for lines longer than 80 chars. User may wish to cancel sending
BOOL TPostDoc::check_format ()
{
	if (m_bMailing)
		return TRUE;

	m_fBodyFormatValid = FALSE;
	UpdateAllViews ( NULL, SENDHINT_80CHARPERLINE, this );
	return m_fBodyFormatValid;
}

// -------------------------------------------------------------------------
void TPostDoc::check_spelling ()
{
	if (!gpGlobalOptions->GetRegSwitch()->m_fSpellcheck)
		return;

	m_fShowSpellComplete = FALSE;
	UpdateAllViews ( NULL, SENDHINT_SPELLCHECK, this );
}

// return TRUE to stop sending
BOOL TPostDoc::check_empty_fields ()
{
	LPARAM lRet;
	POSITION pos = GetFirstViewPosition ();
	CView * pView;
	while (pos)
	{
		pView = GetNextView (pos);
		// verify and set focus to problem field
		lRet = pView->SendMessage ( WMU_VERIFY_POSTHDR );
		if (lRet)
			return TRUE;
	}

	// check if the body is empty
	if (m_ArtText.GetBody ().IsEmpty())/*& (0 == Att_Count())*/
	{
		pos = GetFirstViewPosition ();
		NewsMessageBox(GetNextView(pos),
			IDS_ERR_NO_MESSAGE_BODY, MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}

	// seems OK
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
// Ask if the user really wants to discard this compose window.
//
// This is called in two cases
//   1) The user is closing this compose window
//   2) The app is shutting down and we are querying any open compose windows
//
BOOL TPostDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	if ( HasBeenSaved () )
		return TRUE;

	ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(TPostPopFrame)));
	TPostPopFrame * pPostFrame = static_cast<TPostPopFrame*>(pFrame);

	BOOL RC = pPostFrame->CancelWarning ();

	if (RC) {
		// dialog is closing without sending or saving a draft,
		// so notify the Drafts folder

		TDraftMsgs * pDrafts = m_pServer->GetDraftMsgs();
		LONG lArticle = m_bMailing
			? m_pEmailHeader->GetNumber ()
			: m_pArtHeader->GetNumber ();
		pDrafts->AbortingEdit (lArticle);
	}

	return RC;
}

// -------------------------------------------------------------------------
void TPostDoc::OnCheckSpelling()
{
	m_fShowSpellComplete = TRUE;
	UpdateAllViews (NULL, SENDHINT_SPELLCHECK, this);
}

// -------------------------------------------------------------------------
// gather data
// put data into PostDoc
// notify the other view to save data into the PostDoc
void TPostDoc::SaveTheDraftMessage ()
{
	POSITION pos = GetFirstViewPosition ();
	CView *pView = GetNextView (pos);
	ASSERT (pView);

	// inherited
	if (FALSE == UpdateData ())
		return ;   // DDV_ failure?

	TDraftMsgs *pDrafts = m_pServer->GetDraftMsgs ();

	if (NULL == pDrafts)
		return ;

	TAutoClose sCloser(pDrafts);

	if (m_bMailing)
	{
		LONG iNumber = m_pEmailHeader->GetNumber ();

		if (iNumber <= 0)
		{
			int iPostId = m_pServer->NextPostID ();
			m_pEmailHeader->SetNumber ( iPostId );
			m_EmailText.SetNumber ( iPostId );
		}

		// before sending, tell the draftbox that we are saving this message so
		// it gets a chance to delete the original copy if it exists
		iNumber = m_pEmailHeader->GetNumber ();

		pDrafts->SavingNewCopy (iNumber);

		// TAttachmentDoc
		SaveAttachmentInfo( m_pEmailHeader );

		pDrafts->SaveDraftMessage ( m_pEmailHeader , &m_EmailText );
		m_pEmailHeader = 0;
	}
	else
	{
		// The time in the final posted message will get set when we send it, but lets
		// populate the time/date now so the user sees when this draft was created.
		m_pArtHeader->StampCurrentTime ();
		LONG iNumber = m_pArtHeader->GetNumber ();

		if (iNumber <= 0)
		{
			int iPostId = m_pServer->NextPostID ();

			m_pArtHeader->SetArticleNumber ( iPostId );
			m_ArtText.SetNumber ( iPostId );
		}

		iNumber = m_pArtHeader->GetNumber ();

		pDrafts->SavingNewCopy (iNumber);

		// TAttachmentDoc
		SaveAttachmentInfo( m_pArtHeader );

		pDrafts->SaveDraftMessage ( m_pArtHeader , &m_ArtText );

		m_pArtHeader = 0;
	}

	m_pServer->SaveDrafts ();

	SetSavedStatus (true);

	// close the window
	CloseFrameWnd();
}

void TPostDoc::OnSelectionWrap()
{
	UpdateAllViews (NULL, SENDHINT_SELECTIONWRAP, this);
}
