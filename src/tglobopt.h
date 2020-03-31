/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tglobopt.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.2  2008/09/19 14:52:04  richard_wood
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

#pragma once

#include "pobject.h"
#include "custview.h"
#include "custsig.h"
#include   "thelpapp.h"
#include "tglobdef.h"
#include   "newsgrp.h"

#define GLOB_VERSION_NUMBER 2

class TGlobalOptions : public PObject
{
public:
	DECLARE_SERIAL (TGlobalOptions)

	// defines optional lebs on the Envelope part of the Posting Window
	enum ELebType {
		kLebFrom         = 0x01,
		kLebOrganization = 0x02,
		kLebReply        = 0x04,
		kLebFollowup     = 0x08,
		kLebDistribution = 0x10
	} ;

public:
	TGlobalOptions();
	~TGlobalOptions();

	void        Serialize (CArchive & ar);
	void        SaveToRegistry();
	void        PreLoadFromRegistry ();
	void        LoadFromRegistry();

	void        SaveUISettings();

public:
	TGlobalDef::EEncoding   GetEncodingType ();
	void        SetEncodingType (TGlobalDef::EEncoding kEncoding);

	BOOL        UsingPGP ();
	void        SetUsePgp (BOOL fUse);

	void        SetSignatureList (TNewsSigList* pSigList)
	{
		m_sigList.Set (pSigList);
	}
	TNewsSigList*
		GetpSignatureList () {return &m_sigList;}

	TGlobalDef::EPurgeType  GetPurgeType ();
	void        SetPurgeType (TGlobalDef::EPurgeType kPurgeType);
	void        SetPurgeDays (LONG   purgeDays);
	LONG        GetPurgeDays ();

	TNewsGroup::EStorageOption GetStorageMode();
	void        SetStorageMode (TNewsGroup::EStorageOption kStorageMode);

	BOOL        UsingPopup ();
	void        SetUsePopup (BOOL fUse);

	TCustomArticleView&
		GetCustomView () { return m_custView; }
	void        SetCustomView (const TCustomArticleView& cv)
	{ m_custView = cv; }

	const CString& GetIndentString();
	void           SetIndentString(const CString& s);

	int     GetWordWrap();
	void    SetWordWrap(int n);

	int     GetIDsToRemember();
	void    SetIDsToRemember(int n);

	void    GetPostingFont(LOGFONT* pLF, int* pPointSize);
	void    SetPostingFont(const LOGFONT* pLF, int PointSize);

	BOOL    IncludingOriginalMail();
	void    SetIncludeOriginalMail (BOOL fInclude);

	BOOL    IncludingOriginalFollowup();
	void    SetIncludeOriginalFollowup (BOOL fInclude);

	LONG  GetMaxQuotedLines ();
	void  SetMaxQuotedLines (LONG max);

	const CString& GetSubjectTemplate ();
	void    SetSubjectTemplate (const CString & templ);

	const CString& GetDecodeDirectory ();
	void    SetDecodeDirectory (const CString & dir);

	const CString& GetOtherViewer ();
	void    SetOtherViewer (const CString &strOtherViewer);

	const CString& GetAttachmentDirectory ();
	void  SetAttachmentDirectory(const CString & dir);

	const CString& GetSaveFileDirectory ();
	void  SetSaveFileDirectory(const CString & dir);

	const CString& GetPGPDirectory ();
	void  SetPGPDirectory(const CString & dir);

	BOOL  IsLaunchingViewerOnManual ();
	void  SetLaunchViewerOnManual (BOOL fLaunch);

	BOOL  IsLaunchViewerByRules ();
	void  SetLaunchViewerByRules (BOOL fLaunch);

	BOOL  IsUseOtherViewer ();
	void  SetUseOtherViewer (BOOL fUseOtherViewer);

	BOOL  IsUseImageGallery ();
	void  SetUseImageGallery (BOOL fUseImageGallery);

	LONG  GetMaxEncodedLines ();
	void  SetMaxEncodedLines (LONG max);

	BOOL  IsLimitingQuoteLines ();
	void  SetLimitQuoteLines (BOOL fLimit);

	TGlobalDef::EThreadSort GetThreadSort ();
	void SetThreadSort (TGlobalDef::EThreadSort kSort);

	TGlobalDef::EThreadSortWithin GetThreadSortWithin ();
	void SetThreadSortWithin (TGlobalDef::EThreadSortWithin kSort);

	TGlobalDef::EThreadViewType GetThreadViewType ();
	void SetThreadViewType (TGlobalDef::EThreadViewType kViewType);

	const CString & GetFollowupIntro ();
	void            SetFollowupIntro (LPCTSTR s);

	const CString & GetReplyIntro ();
	void            SetReplyIntro (LPCTSTR s);

	const CString & GetCCIntro ();
	void            SetCCIntro (LPCTSTR s);

	void            Set_MIME_TransferEncoding(LPCTSTR s);
	const CString & Get_MIME_TransferEncoding(void);

	void            Set_MIME_ContentType(LPCTSTR s);
	const CString & Get_MIME_ContentType(void);

	BOOL  IsEnvelopeLebEnabled (ELebType eType) ;
	void  SetEnvelopeLebEnabled (ELebType eType, BOOL fOn);

	int   GetArticleSplitLen (void);
	void  SetArticleSplitLen (int n);
	BOOL  IsAttachmentsInSeparateArticle();
	void  SetAttachmentsInSeparateArticle(BOOL fSep);

	BOOL  DownloadFullHeader();
	void  SetDlFullHeader(BOOL full);

	BOOL  WarnOnCatchup ();
	void  SetWarnOnCatchup (BOOL fWarn);
	BOOL  WarnOnExitCompose ();
	void  SetWarnOnExitCompose (BOOL fWarn);
	BOOL  WarnOnExitNews32 ();
	void  SetWarnOnExitNews32 (BOOL fWarn);
	BOOL  WarnOnMarkRead ();
	void  SetWarnOnMarkRead (BOOL fWarn);
	BOOL  WarnOnUnsubscribe ();
	void  SetWarnOnUnsubscribe (BOOL fWarn);
	BOOL  WarnOnSending ();
	void  SetWarnOnSending (BOOL fWarn);
	BOOL  WarnOnDeleteBinary ();
	void  SetWarnOnDeleteBinary (BOOL fWarn);
	BOOL  WarnOnErrorDuringDecode ();
	void  SetWarnOnErrorDuringDecode (BOOL fWarn);
	BOOL  WarnOnRunExe ();
	void  SetWarnOnRunExe (BOOL fWarn);
	BOOL  WarnOnManualRuleOffline ();
	void  SetWarnOnManualRuleOffline (BOOL fWarn);
	BOOL  WarnOnDeleteArticle();
	void  SetWarnOnDeleteArticle(BOOL fWarn);

	int   FilenameConflict ();
	void  SetFilenameConflict (int action);

	BOOL  IsCustomTreeFont(void);
	void  CustomTreeFont(BOOL fCustom);

	const LOGFONT*  GetTreeFont ();
	void  SetTreeFont (const LOGFONT* pLF);

	BOOL  GetDefaultIcontitleFont(LPLOGFONT pLF, int* piPointSize);

	BOOL  IsCustomNGFont(void);
	void  CustomNGFont(BOOL fCustom);

	const LOGFONT* GetNewsgroupFont();
	void  SetNewsgroupFont(const LOGFONT* pLF);

	BOOL  GetWriteDescription ();
	void  SetWriteDescription (BOOL fNew);

	int   GetSendCharset ();
	BOOL  GetSend8Bit();
	BOOL  GetSend8BitHdrs();

	// URL stuff
	TUrlDde  *GetWebHelperPointer();
	TUrlDde  *GetFtpHelperPointer();
	TUrlDde  *GetGopherHelperPointer();
	TUrlDde  *GetTelnetHelperPointer();
	TURLSettings *GetURLSettings();

	TBackgroundColors *GetBackgroundColors();

	// registry layout
	TRegLayoutMdi*  GetRegLayoutMdi();
	//TRegConnection* GetRegConnection();
	TRegFonts*      GetRegFonts();
	TRegCompose*    GetRegCompose();
	TRegWarn*       GetRegWarn();
	TRegDirectory*  GetRegDirs();
	TRegSystem*     GetRegSystem();
	TRegSwitch*     GetRegSwitch();
	TRegPurge*      GetRegPurge();
	TRegUI*         GetRegUI();
	TRegLayoutWin*  GetRegLayoutWin();
	TRegStorage*    GetRegStorage();
	/// --obsolete--   TAllViewFilter* GetAllViewFilters();

#ifdef PRE_ANAWAVE
	CTime           GetDBCreateTime () { return m_dbCreateTime; }
	void            SetDBCreateTime (CTime time) { m_dbCreateTime = time;}

	LONG            GetDBCreateTime2() { return m_dbCreateTime2;}
	void            SetDBCreateTime2 (long time) { m_dbCreateTime2 = time;}

	LONG            GetCreateTimeVersion () {return m_createTimeVersion;}
	void            SetCreateTimeVersion (LONG version)
	{m_createTimeVersion = version;}
#endif PRE_ANAWAVE

	const CString&  GetSubscribePassword () {return m_subscribePassword;}
	void            SetSubscribePassword (LPCTSTR pwd) {m_subscribePassword = pwd;}

#ifdef PRE_ANAWAVE
	const CString & GetLicensee () {return m_licensee;}
	void            SetLicensee (LPCTSTR pLicensee) {m_licensee = pLicensee;}

	const CString & GetLicensedOrganization () {return m_licensedOrganization;}
	void            SetLicensedOrganization (LPCTSTR pLicensedOrg) {
		m_licensedOrganization = pLicensedOrg;
	}

	const CString & GetRegistrationKey () {return m_registrationKey;}
	void            SetRegistrationKey (LPCTSTR pRegKey)
	{m_registrationKey = pRegKey;}
#endif

	const CString&  GetDistribution ();
	void            SetDistribution (LPCTSTR dist);

	const CStringList& GetCustomHeaders ();
	void               SetCustomHeaders (CStringList &sCustomHeaders);

	void  PostProcessing();

public:
	LOGFONT             m_defTreeFont;   // not serialized
	LOGFONT             m_defNGFont;     // not serialized

private:
	// User information
	TNewsSigList m_sigList;             // a pool of signatures

	// Display stuff
	TCustomArticleView  m_custView;     // appearance of Subject:, Date: etc

	// various things that shouldn't be in the registry
	CTime    m_dbCreateTime;         // date of creation
	CString  m_subscribePassword;    // kiddy condom
	CString  m_licensee;             // licensee name
	CString  m_licensedOrganization; // licensed organization
	CString  m_registrationKey;      // registration key
	LONG     m_dbCreateTime2;        // tamper proofing
	LONG     m_createTimeVersion;    // what major version is this 30 day eval?

	TRegistryManager*  m_pRegistryManager;  // indirection
};

extern TGlobalOptions * gpGlobalOptions;   // declaration. defined in NEWS.CPP
