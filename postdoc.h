/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: postdoc.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:40  richard_wood
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

// postdoc.h -- document class for posting/mailing windows

#pragma once

#include "attdoc.h"
#include "article.h"

class TStringList;
class TNewsServer;

// -------------------------------------------------------------------------
class TPostDoc : public TAttachmentDoc
{
protected:
	TPostDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TPostDoc)

public:
   void FinalizeMessageForOutbox();
   LPCTSTR Message_GetSubject(bool fMailing);
   void Message_SetSubject(bool fMailing, LPCTSTR subj);
   CString Message_GetFrom(bool fMailing);
   void Message_SetFrom(bool fMailing, const CString & from);

   const CString& Message_GetBody (bool fMailing);
   void Message_SetBody (bool fMailing, CFile& body, int len = -1);
   void Message_SetBody (bool fMailing, const CString& str);
   void Message_GetMimeLines(bool fMailing, CString* pVer, CString* pTyp, CString* pEnc, CString* pDesc);
   void Message_SetMimeLines(bool fMailing, LPCTSTR  ver,  LPCTSTR  typ,  LPCTSTR  enc,  LPCTSTR  desc);
   TArticleHeader* GetHeader () { return m_pArtHeader; }
	TArticleText* GetText () { return &m_ArtText; }
   TEmailHeader* GetEmailHeader () { return m_pEmailHeader; }
	TEmailText* GetEmailText () { return &m_EmailText; }

   int m_iCancelWarningID;
   BOOL m_bMailing;              // are we posting or mailing?
   BOOL m_fBodyFormatValid;      // verify format of body
   BOOL m_fShowSpellComplete;    // when spell checking over, display message?
   LONG m_lGroupID;              // ID of group we're posting to or forwarding
                                 // from

public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TPostDoc)
	public:
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

public:
	~TPostDoc();
   #ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
   #endif

protected:
	//{{AFX_MSG(TPostDoc)
   afx_msg void OnPostSend();
   afx_msg void OnSaveDraft();
   afx_msg void OnPostCancel();
	afx_msg void OnCheckSpelling();
	afx_msg void OnSelectionWrap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
   int allow_post (TStringList* pModeratedList, BOOL &bSomeInvalidGroups,
      BOOL &bNoValidGroups);
   int send_moderated_mail (TStringList* pModeratedList);
   BOOL check_format ();
   void check_spelling ();
   BOOL check_empty_fields ();
   void SendTheMessage ();
   void SaveTheDraftMessage ();

protected:
   TArticleHeader *m_pArtHeader;
	TArticleText m_ArtText;
   TEmailHeader *m_pEmailHeader;
   TEmailText m_EmailText;

   TNewsServer * m_pServer;   // ptr to current news server
};
