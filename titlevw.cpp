/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: titlevw.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:08  richard_wood
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

// titlevw.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "titlevw.h"
#include "newsgrp.h"
#include "tglobopt.h"
#include "newsview.h"
#include "hints.h"
#include "tpoptmpl.h"
#include "tpopfrm.h"
#include "fileutil.h"
#include "uimem.h"
#include "globals.h"
#include "tasker.h"
#include "hourglas.h"
#include "custmsg.h"
#include "artview.h"
#include "ecpcomm.h"
#include "ngutil.h"
#include "critsect.h"
#include "fetchart.h"
#include "usrdisp.h"
#include "utilpump.h"
#include "utilrout.h"
#include "taglist.h"
#include "tnews3md.h"
#include "rgui.h"                // GetShowFullHeader
#include "rgswit.h"              // TRegSwitch, GetDBSaveDLMessages
#include "server.h"              // TNewsServer
#include "genutil.h"             // GetNewsView()
#include "vfilter.h"             // TViewFilter
#include "artchain.h"            // CArticleChain

extern TGlobalOptions* gpGlobalOptions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TNewsTasker * gpTasker;

/////////////////////////////////////////////////////////////////////////////
// TTitleView

IMPLEMENT_DYNCREATE(TTitleView, TITLEVIEW_BASE)

TTitleView::TTitleView()
{
	m_pArtView = NULL;
	m_pNewsView = NULL;
}

TTitleView::~TTitleView()
{
}

BEGIN_MESSAGE_MAP(TTitleView, TITLEVIEW_BASE)
	ON_COMMAND          (ID_ARTICLE_SAVE_AS,  OnArticleSaveAs)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_SAVE_AS,  OnUpdateArticleSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FORWARD_SELECTED, OnUpdateArticleForward)
	ON_COMMAND(ID_ARTICLE_CANCELMSG, OnArticleCancelmsg)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_CANCELMSG, OnUpdateArticleCancelmsg)
	ON_UPDATE_COMMAND_UI(ID_ARTICLE_MORE, OnUpdateArticleMore)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_BACKWARD, OnUpdateBackward)
	ON_COMMAND(ID_BACKWARD, OnBackward)
	ON_UPDATE_COMMAND_UI(ID_FORWARD, OnUpdateForward)
	ON_COMMAND(ID_FORWARD, OnForward)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TTitleView drawing

void TTitleView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// TTitleView diagnostics

#ifdef _DEBUG
void TTitleView::AssertValid() const
{
	TITLEVIEW_BASE::AssertValid();
}

void TTitleView::Dump(CDumpContext& dc) const
{
	TITLEVIEW_BASE::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TTitleView message handlers

// --------------------------------------------------------------------------
// Returns 0 for success.  If status unit is not found, display error box.
int  TTitleView::LoadFullArticle(
								 TNewsGroup*        pNG,
								 TPersist822Header* pArtHdr,
								 DWORD              dwRedrawCookie,
								 BOOL               bRemember /* = TRUE */)
{
	TPersist822Text* pBaseText = pArtHdr->AllocText();
	BOOL fFilePresent = FALSE;

	try
	{
		fFilePresent = LoadFullArticleHelper (pNG, pArtHdr, pBaseText);
	}
	catch (TException *pE )
	{
		pE->Delete();
		delete pBaseText;
		return -1;
	}

	if (fFilePresent)
	{
		// allow the layout manager to see this action
		CFrameWnd * pDadFrame = GetParentFrame();
		if (pDadFrame)
			pDadFrame->SendMessage (WMU_USER_ACTION, TGlobalDef::kActionOpenArticle);

		CNewsView* pNewsView = GetNewsView();

		pNewsView->SetBrowseHeader ( pArtHdr );

		// NewsDoc owns the ptr.
		pNewsView->SetBrowseText ( pBaseText );
		{
			// the browse header ptr must be protected
			TSyncReadLock sync(pNG);

			// We have a Hdr and Text
			GetDocument()->UpdateAllViews (this, VIEWHINT_SHOWARTICLE);

			// redraw 'New Article' counter on the Newsview
			pNewsView->PostMessage(WMC_DISPLAY_ARTCOUNT, pNG->m_GroupID);
		}

		// derived classes define this virtual function
		RedrawItem ( dwRedrawCookie );
		if (bRemember)
			m_chain.New (pNG -> m_GroupID, pArtHdr -> GetNumber ());
		return 0;
	}
	else
	{  // body is not present in database

		delete pBaseText;

		if (pArtHdr->IsArticle() == FALSE)
		{
			throw(new TException(IDS_ERR_NO_EMAIL_MESSAGE, kError));
			return -1;
		}

		// ask the high Prio pump
		WORD wStatus ;
		if (pNG->iStatusDirect ( pArtHdr->GetNumber(), wStatus ))
		{
			NewsMessageBox (this, IDS_UNIT_DELETED);
			return -1;
		}

		if (bRemember)
			m_chain.New (pNG -> m_GroupID, pArtHdr -> GetNumber ());
		return priority_article (pNG->GetName(),
			pNG->m_GroupID,
			gpGlobalOptions->GetRegSwitch()->GetDBSaveDLMessages(),
			pArtHdr->CastToArticleHeader(),
			dwRedrawCookie, wStatus);
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Sets the database status to kRead. Sets the pArtHdr to kRead.  Caller must
//  force the redraw
//
//  7-14-96 amc   Trap exception TBodyDoesNotExist explicitly
BOOL TTitleView::LoadFullArticleHelper(
									   TNewsGroup*        pNG,
									   TPersist822Header* pBaseHdr,
									   TPersist822Text*   pBaseText
									   )
{
	TArticleHeader* pArticleHdr = pBaseHdr->CastToArticleHeader();

	int artInt = pArticleHdr->GetArticleNumber();

	if (FALSE == pNG->TextRangeHave (artInt) )
		return FALSE;

	TAutoClose  sAutoOpenClose(pNG);

	try
	{
		// An entire article (separate from the m_stage)
		//   hdr is marked as READ
		//   pass in the NG - write out the ReadRangeSet
		pNG->LoadBody (artInt, pBaseText);

		// fill header with cross-posting info
		pBaseText->CastToArticleText()->TransferXRef (pArticleHdr);

		if ( gpGlobalOptions->GetRegSwitch()->GetMarkReadDisplay() )
		{
			// set status unit, do cross-post-mgmt
			pNG->ReadRangeAdd (pArticleHdr);
		}
	}
	catch (TBodyDoesNotExist *pE/* a TGroupDBErrors exception */)
	{
		pE->Delete();
		// correct ourselves
		pNG->TextRangeSubtract (artInt);
		return FALSE;
	}
	catch (TException *te)
	{

		te->PushError (IDS_UTIL_LOADFULLH, kError);
		te->Display ();
		te->Delete();
		return FALSE;
	}
	catch(...)
	{
		throw;
	}

	return TRUE;
}

// -------------------------------------------------------------------------
int TTitleView::priority_article(
								 LPCTSTR          newsgroup,
								 LONG             groupID,
								 BOOL             fSaveBody,
								 TArticleHeader * pArtHdr,
								 DWORD            dwRedrawCookie,
								 WORD             wHeaderStatus
								 )
{
	TServerCountedPtr cpNewsServer;     // smart ptr

	try
	{
		bool fIsConnecting ;
		if (FALSE == gpTasker->IsConnected ( &fIsConnecting ))
		{

			// part of Absolute Online / Absolute Offline mode

			if (false == fIsConnecting)
				return -1;

			// if we are connect-ing, then proceed
		}

		int artInt = pArtHdr->GetNumber();
		TFetchArticle* pObjFetch = 0;
		int ret;

		if (FALSE == gpTasker->NormalPumpBusy() )
		{
			pObjFetch = new TFetchArticle(newsgroup, groupID, artInt, wHeaderStatus);

			// pass in the header.  we may have to evaluate rules with it.
			ret = gpTasker->NonPriorityArticle ( newsgroup, groupID, fSaveBody,
				pArtHdr, pObjFetch );
		}
		else
		{
			int  iTry;
			BOOL fConnected;
			BOOL fContinueConnect = FALSE;
			fConnected = gpTasker->SecondIsConnected( &fContinueConnect );
			if (!fConnected)
			{
				if (!fContinueConnect)
				{
					// we are either "In The Process Of" connecting or disconnecting
					TRACE0("titlevw - reject connect\n");
					return -1;
				}

				gpUserDisplay->AddActiveCursor();
				iTry = gpTasker->SecondConnect( cpNewsServer->GetNewsServerAddress() );
				gpUserDisplay->RemoveActiveCursor();
				if (0 != iTry)
					return -1;
			}
			pObjFetch = new TFetchArticle(newsgroup, groupID, artInt, wHeaderStatus);

			// pass in the header.  we may have to evaluate rules with it.
			ret = gpTasker->PriorityArticle ( newsgroup,
				groupID,
				fSaveBody,
				pArtHdr,
				pObjFetch );
			if (0 != ret)
			{
				pObjFetch->DestroySelf();
				pObjFetch = 0;
			}
			iTry = 676;
		}

		if (0 == ret)
			return 0;
		else
			pObjFetch->DestroySelf();

		return -1;
	}
	catch(...)
	{
		return -1;
		throw;
	}
}

////////////////////////////////////////////////////////////
// there is still some UI involved so I didn't move this
// function to the CNewsDoc.
//
// this helper function is also used by the popup View window
// caller owns the ptrs.
BOOL
TTitleView::SaveArticle (CFile & file,                   // opened CFile
						 TPersist822Header* pBaseHdr,
						 TPersist822Text*   pBaseText,
						 BOOL fAppend)
{
	BOOL  fRet = TRUE;
	if (fAppend)
		file.SeekToEnd();

	// the file is open - yay!

	BOOL fFullHdr = gpGlobalOptions->GetRegUI()->GetShowFullHeader();

	try
	{
		GetDocument()->SaveToFile (pBaseHdr, pBaseText, fFullHdr, file);
	}
	catch (CFileException * pfe)
	{
		pfe->ReportError (MB_OK | MB_ICONSTOP);
		pfe->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL
TTitleView::SaveOneArticle (CWnd* pParentWnd, TPersist822Header* pBaseHdr,
							TPersist822Text*   pBaseText)
{
	// Ask for a filename
	CString fileName;
	CFile   file;

	CNewsApp* pApp = (CNewsApp*) AfxGetApp();

	// this function gives us an open file
	BOOL fRet = pApp->GetSpec_SaveArticle (pParentWnd, &fileName, &file);
	if (fRet)
	{
		BOOL fFullHdr = gpGlobalOptions->GetRegUI()->GetShowFullHeader();
		// the file is open - yay!

		try
		{
			GetDocument()->SaveToFile (pBaseHdr, pBaseText, fFullHdr, file);
		}
		catch (CFileException * pfe)
		{
			pfe->ReportError (MB_OK | MB_ICONSTOP);
			pfe->Delete ();
			return FALSE;
		}
	}
	return fRet;
}

//////////////////////////////////////////////////////////////////////
//
BOOL TTitleView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// 9-3-95  The tnews3md.cpp window does the 4way dispatch.

	return TITLEVIEW_BASE::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/////////////////////////////////////////////////////////////////////////////
//  Ask for a filename.
//   we will have a separate function to append to standard file "ALT.TV.X-FILES"
//  Questions:
//      Save as Text or as RTF?
//      Save Full Header?  or Save according to the ViewTemplate?
//      Ask about append?  Do we need a separator?
//
//
void TTitleView::OnArticleSaveAs()
{
	// [Part 1] Check if all articles are local
	int non_local_count = 0;
	BOOL fAllLocal = vSelectedArticlesAvailable( non_local_count );
	BOOL fRetrieveNonLocal = TRUE;

	// [Part 2] ask if user wants to actively retrieve
	//   non-local articles (Y/NO/Cancel)
	if (FALSE == fAllLocal)
	{
		int ret;
		int strid = non_local_count <= 1 ?
IDS_RETRIEVE_NOTLOCAL1 : IDS_RETRIEVE_NOTLOCAL2;
		CString s;
		s.LoadString(strid);
		CString msg;
		msg.Format((LPCTSTR)s, non_local_count);
		ret = NewsMessageBox(this, msg, MB_ICONQUESTION | MB_YESNOCANCEL);
		if (IDCANCEL == ret)
			return;

		fRetrieveNonLocal = (IDYES == ret);
	}

	// [Part 3] Ask for a filename
	CString fileName;
	CFile   fl;
	CNewsApp* pApp = (CNewsApp*) AfxGetApp();
	if (FALSE == pApp->GetSpec_SaveArticle(this, &fileName, &fl))
		return;

	// [Part 4] SaveSelectedArticles
	vSaveSelectedArticles( fileName, fl, fRetrieveNonLocal );
}

void TTitleView::OnUpdateArticleSaveAs(CCmdUI* pCmdUI)
{
	// let derived class to its thing
	int selCount = vAnythingSelected();
	pCmdUI->Enable ( selCount > 0 );
}

// make this inline?
void TTitleView::SetItemStatus(
							   TNewsGroup* pNewsGrp,
							   int         msgInt,
							   TStatusUnit::EStatus eStatus)
{
	pNewsGrp->StatusBitSet ( msgInt, eStatus, TRUE );
}

///////////////////////////////////////////////////////////////////////////
// Disable items in the popup menu.  The usual MFC support for mainmenus
// doesn't apply.
//
void TTitleView::UpdateContextMenu (CMenu* pSubmenu)
{
	ProbeCmdUI ( this, pSubmenu );      // utility C function
}

int  TTitleView::SaveArticleDriver (LPTITLSaveArt pSaveInfo,
									TArticleHeader* pHdr, int fSelected, TNewsGroup* pNG)
{
	if (!fSelected)
		return 0;
	int artInt = pHdr->GetArticleNumber();
	BOOL fAppend = TRUE;

	CNewsView* pNV = pSaveInfo->pNV;
	TPersist822Text* pText = pNV->GetBrowseText();

	int ret = 0;
	if (pText && artInt == pText->GetNumber())
	{
		SaveArticle (*pSaveInfo->pFile, pHdr, pText, fAppend );
		return 0;
	}
	if (pNG->TextRangeHave(artInt))
	{
		TPersist822Text*    pBaseText = 0;
		pBaseText = pHdr->AllocText();
		// load the text part
		BOOL fFilePresent = LoadFullArticleHelper (pNG, pHdr, pBaseText);
		if (fFilePresent)
		{
			if (FALSE == pSaveInfo->pSelf->SaveArticle (*pSaveInfo->pFile, pHdr,
				pBaseText, fAppend))
				ret = 1;
		}
		delete pBaseText;
		return ret;
	}

	if (!pSaveInfo->fRetrieve)
		return 0;

	TArticleText* pServerText = 0;

	BOOL fMarkAsRead = gpGlobalOptions->GetRegSwitch()->GetMarkReadFileSave();
	try
	{
		TError  sErrorRet;
		CPoint ptPartID(0,0);
		CString strEmpty;
		int ret = BlockingFetchBody (sErrorRet,
			pNG->GetName(),
			pNG->m_GroupID,
			strEmpty,
			ptPartID,
			pHdr->GetArticleNumber(),
			pHdr->GetLines(),
			pServerText,
			TRUE,             // fPrioPump
			(HANDLE) NULL,    // hKillEvent
			fMarkAsRead,      // fMarkAsRead
			TRUE);            // fForceConnection

		if (0 == ret)
		{
			if (!pSaveInfo->pSelf->SaveArticle (*pSaveInfo->pFile, pHdr,
				pServerText, fAppend))
				ret = 1;
		}
		else
		{
			CString str;

			EErrorClass eClass;   sErrorRet.GetClass( eClass );
			switch (eClass )
			{
			case kClassNNTP:
				NewsMessageBox ( this, IDS_ERR_ARTEXPIRED, MB_OK | MB_ICONEXCLAMATION );
				ret = 1;
				break;

			case kClassWinsock:
				str.LoadString (IDS_ERR_ONLINE);
				NewsMessageBox(this, str);
				ret = 1;
				break;

			case kClassUser:
				str.LoadString (IDS_UNSUBSCRIBED);
				NewsMessageBox(this, str);
				ret = 1;
				pSaveInfo->fErrorShown = TRUE;
				break;

			case kClassUnknown:
			case kClassInternal:
			case kClassResource:
			case kClassExternal:
				{
					DWORD dwRet =  sErrorRet.GetDWError();

					if (dwRet)
						str.Format ("Error %d", dwRet);
					else
						sErrorRet.GetDescription( str );

					if (str.IsEmpty())
						str = "Unknown error";

					ret = 1;
					if (!pSaveInfo->fErrorShown)
					{
						NewsMessageBox(this, str);
						pSaveInfo->fErrorShown = TRUE;
					}
				}
				break;

			default:
				ASSERT(0);
				break;
			}
		}
	}
	catch(...)
	{
		delete pServerText;
		throw;
		return 0;
	}
	delete pServerText;
	return ret;
}

void TTitleView::OnUpdateArticleForward(CCmdUI* pCmdUI)
{
	TServerCountedPtr cpNewsServer;

	CString host = cpNewsServer->GetSmtpServer();
	if (host.IsEmpty() || 0==m_pNewsView->GetCurNewsGroupID())
	{
		pCmdUI->Enable ( FALSE );
		return;
	}
	// allow derived class to do its thing
	int selCount = vAnythingSelected();
	pCmdUI->Enable ( selCount > 0 );
}

BOOL TTitleView::IsArticleTagged(int artInt)
{
	TServerCountedPtr cpNewsServer;

	CNewsView* pNewsView = GetNewsView();
	int GroupID = pNewsView->GetCurNewsGroupID();
	return cpNewsServer->GetPersistentTags().FindTag ( GroupID, artInt );
}

void TTitleView::OnArticleCancelmsg()
{
	// TODO: Add your command handler code here
	vCancelArticle();
}

void TTitleView::OnUpdateArticleCancelmsg(CCmdUI* pCmdUI)
{
	int  selCount = vAnythingSelected();
	// allow 1 and only 1
	pCmdUI->Enable ( 1 == selCount );
}

void TTitleView::doCancelArticle(TArticleHeader* pArtHdr)
{
	TServerCountedPtr cpNewsServer;
	CString phrase;

	CString strMyAddress = cpNewsServer -> GetEmailAddress ();
	if (!cpNewsServer -> GetEmailAddressForPost ().IsEmpty ())
		strMyAddress = cpNewsServer -> GetEmailAddressForPost ();

	CString addr;
	pArtHdr->ParseFrom ( phrase, addr );
	if (0 != addr.CompareNoCase (strMyAddress))
	{
		NewsMessageBox(NULL, IDS_ERR_NOCANCELMSG, MB_OK | MB_ICONSTOP);
		return;
	}
	if (IDNO == NewsMessageBox(NULL, IDS_WARN_CANCELMSG, MB_YESNO | MB_ICONQUESTION))
		return;

	CString curNGname;
	{
		LONG gid = GetNewsView()->GetCurNewsGroupID();
		BOOL fUseLock;
		TNewsGroup* pNG = 0;
		TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNG);
		if (fUseLock)
			curNGname = pNG->GetName();
	}
	GetDocument()->CancelMessage ( curNGname,
		pArtHdr->GetMessageID(),
		pArtHdr->GetSubject() );
}

// tnews3md.cpp does the real work. Each active view must have a
// handler for this

void TTitleView::OnUpdateArticleMore(CCmdUI* pCmdUI)
{
	CWnd* pMdiChild = ((CFrameWnd*) AfxGetMainWnd())->GetActiveFrame();
	BOOL fOK = ((TNews3MDIChildWnd*) pMdiChild)->CanArticleMore();
	pCmdUI->Enable (fOK);
}

//-------------------------------------------------------------------------
void TTitleView::ProcessPumpArticle (TFetchArticle* pFetch)
{
	TServerCountedPtr cpNewsServer;
	LONG fetch_gid;
	int  fetch_artint;
	WORD fetch_hdrstatus;

	pFetch->GetUIContext ( &fetch_gid, &fetch_artint, &fetch_hdrstatus );
	if ( pFetch->GetSuccess() )
	{
		// get the header, set as current header - virt func
		if (FALSE == vMarkBrowseHeader (fetch_artint, pFetch->GetArtText()))
		{
			delete pFetch->GetArtText();
			gpUserDisplay->RemoveActiveCursor();
		}
		else
		{
			// allow the layout manager to see this action
			CFrameWnd * pDadFrame = GetParentFrame();
			if (pDadFrame)
				pDadFrame->SendMessage (WMU_USER_ACTION, TGlobalDef::kActionOpenArticle);

			// newsview owns the ptr.
			GetNewsView()->SetBrowseText   ( pFetch->GetArtText() );

			{
				BOOL fUseLock;
				TNewsGroup* pNG = 0;
				TNewsGroupUseLock useLock(cpNewsServer, fetch_gid, &fUseLock, pNG);
				if (fUseLock)
				{
					WORD wStatusBits = 0;

					// TStatusUnit::kSendErr was set before, turn it off.
					if (pNG->IsStatusBitOn (fetch_artint, TStatusUnit::kSendErr))
						pNG->StatusBitSet(fetch_artint, TStatusUnit::kSendErr, FALSE);

					// lock thread list while we fill the artview
					TSyncReadLock sync(pNG);
					GetDocument()->UpdateAllViews ( this, VIEWHINT_SHOWARTICLE );
				}
			}

			gpUserDisplay->RemoveActiveCursor();

			// redraw 'New Article' counter
			GetNewsView()->PostMessage (WMC_DISPLAY_ARTCOUNT, fetch_gid);
		}
	}
	else
	{
		gpUserDisplay->RemoveActiveCursor();

		EErrorClass eClass;  pFetch->m_mplibError.GetClass ( eClass );
		CString desc;        pFetch->m_mplibError.GetDescription ( desc );
		DWORD  dwError  =    pFetch->m_mplibError.GetDWError ();

		bool fHandled = false;

		switch (eClass)
		{
		case kClassWinsock:

			if (dwError == WSAEINTR)
				fHandled = true;

			break;

		case kClassNNTP:
			{
				fHandled = true;
				bool fArticleGone = false;
				if (dwError)
				{
					if (423 == dwError)
						fArticleGone = true;
					else if (dwError < 500)
					{
						NewsMessageBox (this, IDS_ARTICLE_NOTAVAIL,
							MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						// some weirdo NNTP error
						CString str; str.LoadString (IDS_ERR_RETRIEVE_ART);
						CString msg; msg.Format("%s\n\nError - %d", str, dwError );
						NewsMessageBox(this, msg, MB_OK | MB_ICONINFORMATION);
					}
				}
				else
				{
					int nntpRet = atoi(LPCTSTR(desc));
					if (423 == nntpRet)
						fArticleGone = true;
					else if (nntpRet < 500)
					{
						NewsMessageBox (this, IDS_ARTICLE_NOTAVAIL,
							MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						// some weirdo NNTP error
						CString str; str.LoadString (IDS_ERR_RETRIEVE_ART);
						CString msg; msg.Format("%s\n\n%s", str, (LPCTSTR)desc );
						NewsMessageBox(this, msg, MB_OK | MB_ICONINFORMATION);
					}
				}

				if (fArticleGone)
				{

					// change status to kSendError - will display an X token
					// in the document

					{
						BOOL fUseLock;
						TNewsGroup* pNG = 0;
						TNewsGroupUseLock useLock(cpNewsServer, fetch_gid, &fUseLock, pNG);
						if (fUseLock)
							pNG->StatusBitSet(fetch_artint, TStatusUnit::kSendErr, TRUE);
						// who forces the redraw?
					}

					fHandled = true;
				}

				break;
			}

		case kClassUser:
			fHandled = true;
			break;
		} // end switch

		if (!fHandled)
		{
			CString str; str.LoadString (IDS_ERR_RETRIEVE_ART);
			NewsMessageBox(this, str, MB_OK | MB_ICONINFORMATION);
		}
	}
	pFetch->DestroySelf();
}

void TTitleView::Util_ArticleNGread (TArticleHeader* pHdr)
{
	if ( gpGlobalOptions->GetRegSwitch()->GetMarkReadDisplay() == FALSE)
		return;

	if (0 == pHdr)
		throw(new TException ("util_articlengread", kFatal));

	TServerCountedPtr cpNewsServer;
	int artInt = pHdr->GetNumber();
	CNewsView*  pNewsView = GetNewsView();
	LONG gid = pNewsView->GetCurNewsGroupID();
	BOOL fUseLock;
	TNewsGroup* pNewsGrp  = 0;
	TNewsGroupUseLock useLock(cpNewsServer, gid, &fUseLock, pNewsGrp);
	if (fUseLock)
	{
		pNewsGrp->StatusBitSet ( artInt, TStatusUnit::kNew, FALSE );
		pNewsGrp->ReadRangeAdd ( pHdr );
		pNewsGrp->SetDirty();
	}
}

void TTitleView::NonBlockingCursor(BOOL fOn)
{
	HWND hwndLbx = GetInternalLBX();

	HCURSOR cursor = LoadCursor(NULL, fOn ? IDC_APPSTARTING : IDC_ARROW);
	SetClassLong(hwndLbx, GCL_HCURSOR, (LONG) cursor);
	SetCursor ( cursor );
}

///////////////////////////////////////////////////////////////////////////
// Used by flatview and thread view, thus this is a protected function
//
void TTitleView::EmptyBrowse_ClearArtPane ()
{
	GetNewsView()->SetBrowseHeader( NULL );
	GetNewsView()->SetBrowseText( NULL );
	GetArtView()->Clear ();
	m_chain.Clear();
}

TArtHdrDestroyedInfo::TArtHdrDestroyedInfo(int count)
:  m_total(count)
{
	ASSERT(count > 0);
	m_vID = new int[m_total];
}

TArtHdrDestroyedInfo::~TArtHdrDestroyedInfo()
{
	delete [] m_vID;
}

// ------------------------------------------------------------------------
//
int TTitleView::ArtPaneSummarize ( TArticleHeader * pHdr )
{
	m_pNewsView->SetBrowseHeader ( pHdr );
	m_pNewsView->SetBrowseText   (NULL);

	// i know we only have the header, but show a summary in artpane
	GetDocument()->UpdateAllViews (this, VIEWHINT_SHOWARTICLE);
	return 0;
}

int TTitleView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (TITLEVIEW_BASE::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Strip CS_HREDRAW and CS_VREDRAW. We don't need to repaint everytime we
	//  are sized.  (Tip from 'PC Magazine' May 6,1997)
	DWORD dwStyle = GetClassLong (m_hWnd, GCL_STYLE);
	SetClassLong (m_hWnd, GCL_STYLE, dwStyle & ~(CS_HREDRAW | CS_VREDRAW));

	return 0;
}

// --------------------------------------------------------------------------
void TTitleView::NotifySwitchServers ()
{
	m_chain.Empty ();
}

// --------------------------------------------------------------------------
void TTitleView::OnUpdateBackward(CCmdUI* pCmdUI)
{
	pCmdUI -> Enable (!m_chain.AtBegin ());
}

// --------------------------------------------------------------------------
void TTitleView::OnUpdateForward(CCmdUI* pCmdUI)
{
	pCmdUI -> Enable (!m_chain.AtEnd ());
}

// --------------------------------------------------------------------------
void TTitleView::OnBackward()
{
	ASSERT (!m_chain.AtBegin ());
	long lGroup, lArticle;
	m_chain.Back (lGroup, lArticle);
	if (GoToArticle (lGroup, lArticle)) {
		CString strTemplate;
		strTemplate.LoadString (IDS_ERR_UNAVAILABLE);
		CString strTemp;
		strTemp.LoadString (IDS_PREV);
		CString strError;
		strError.Format (strTemplate, strTemp);
		MessageBox (strError);
	}
}

// --------------------------------------------------------------------------
void TTitleView::OnForward()
{
	ASSERT (!m_chain.AtEnd ());
	long lGroup, lArticle;
	m_chain.Forward (lGroup, lArticle);
	if (GoToArticle (lGroup, lArticle)) {
		CString strTemplate;
		strTemplate.LoadString (IDS_ERR_UNAVAILABLE);
		CString strTemp;
		strTemp.LoadString (IDS_NEXT);
		CString strError;
		strError.Format (strTemplate, strTemp);
		MessageBox (strError);
	}
}

// --------------------------------------------------------------------------
// GoToArticle -- returns 0 for success, non-0 for failure
int TTitleView::GoToArticle (long lGroup, long lArticle,
							 BOOL bRemember /* = FALSE */)
{
	// open group if it's not the current group
	long lCurrentGroup = m_pNewsView -> GetCurNewsGroupID ();
	if (lCurrentGroup != lGroup) {
		int RC = m_pNewsView -> OpenNewsgroup (
			CNewsView::kOpenNormal /* eMode */,
			CNewsView::kPreferredFilter, lGroup);
		if (RC < 0)
			return 1;
	}

	TServerCountedPtr cpNewsServer;
	BOOL fUseLock;
	TNewsGroup *pNG;
	TNewsGroupUseLock useLock (cpNewsServer, lGroup, &fUseLock, pNG);
	if (!fUseLock)
		return 1;

	TGotoArticle sGoto;
	sGoto.m_articleNumber = lArticle;
	sGoto.m_groupNumber = lGroup;
	sGoto.m_byLoad = TRUE;
	sGoto.m_byRemember = bRemember ? 1 : 0;

	if (!IsArticleInThreadPane (lArticle)) {
		// switch to "all articles" filter
		if (AfxGetMainWnd () -> SendMessage (WMU_FORCE_FILTER_CHANGE))
			return 1;

		// open the group again, to utilize new filter
		int RC = m_pNewsView -> OpenNewsgroup (
			CNewsView::kOpenNormal /* eMode */, CNewsView::kCurrentFilter, lGroup);
		if (RC < 0)
			return 1;

		if (!IsArticleInThreadPane (lArticle))
			return 1;
	}

	// open the article
	TNews3MDIChildWnd *pMDIWnd = (TNews3MDIChildWnd *)
		((CMDIFrameWnd *) AfxGetMainWnd ()) -> MDIGetActive ();
	pMDIWnd -> SendMessage (WMU_NEWSVIEW_GOTOARTICLE, 0, (LPARAM) &sGoto);

	return 0;
}

