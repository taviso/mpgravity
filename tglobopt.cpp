/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tglobopt.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.2  2009/06/14 13:17:22  richard_wood
/*  Added side by side installation of Gravity.
/*  Adding (WORK IN PORGRESS!!!) DB export/import.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

#include "stdafx.h"
#include "resource.h"
#include "tglobopt.h"
#include "licutil.h"
#include "rgmgr.h"            // TRegistryManager
#include "gdiutil.h"          // setupSansSerifFont
#include "rgui.h"
#include "server.h"
#include "globals.h"

IMPLEMENT_SERIAL  (TGlobalOptions, PObject, 2)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern CFont* gpVariableFont;
extern CFont* gpHeaderCtrlFont;

TGlobalOptions::TGlobalOptions()
	: PObject(GLOB_VERSION_NUMBER)
	, m_dbCreateTime (time_t(0))
	, m_dbCreateTime2 (time_t(0))
	, m_createTimeVersion(0)
{
	m_pRegistryManager = NULL;

	// these are used for run-time storage
	ZeroMemory(&m_defTreeFont, sizeof(m_defTreeFont));  // not serialized
	ZeroMemory(&m_defNGFont, sizeof(m_defNGFont));      // not serialized

	m_pRegistryManager = new TRegistryManager(TRUE);
}

TGlobalOptions::~TGlobalOptions ()
{
	delete m_pRegistryManager;
}

extern TLicenseSystem *gpLicenseDope;

void TGlobalOptions::Serialize (CArchive & ar)
{
	PObject::Serialize (ar);
	if (ar.IsStoring())
	{
		m_sigList.Serialize ( ar );
		m_custView.Serialize ( ar );

#ifdef PRE_ANAWAVE
		// if the database create time hasn't been set,
		if (0 == m_dbCreateTime.GetTime ())
		{
			m_dbCreateTime = CTime::GetCurrentTime();
			m_dbCreateTime2 = LONG(m_dbCreateTime.GetTime());
			m_dbCreateTime2 ^= TRIAL_MASK;
			int   version;
			BOOL  beta;
			int   build;
			gpLicenseDope->GetVersionInt (&version, &beta, &build);
			m_createTimeVersion = version / 100;
		}
		ar << m_dbCreateTime;
#endif
		ar << m_subscribePassword;
#ifdef PRE_ANAWAVE
		ar << m_licensee;
		ar << m_licensedOrganization;
		ar << m_registrationKey;
		ar << m_dbCreateTime2;
		ar << m_createTimeVersion;
#endif
	}
	else
	{
		int curVersion = GetObjectVersion ();

		m_sigList.Serialize ( ar );
		m_custView.Serialize ( ar );

		if (curVersion == 1)
		{
			ar >> m_dbCreateTime;
		}
		ar >> m_subscribePassword;

		if (curVersion == 1)
		{
			ar >> m_licensee;
			ar >> m_licensedOrganization;
			ar >> m_registrationKey;
			ar >> m_dbCreateTime2;
			ar >> m_createTimeVersion;
		}
	}
}

void TGlobalOptions::GetPostingFont(LOGFONT* pLF, int* pPointSize)
{
	CopyMemory (pLF,
		&m_pRegistryManager->GetRegFonts()->m_postFont,
		sizeof(LOGFONT));
	*pPointSize = m_pRegistryManager->GetRegFonts()->m_postPtSize;
}

void TGlobalOptions::SetPostingFont(const LOGFONT* pLF, int PointSize)
{
	CopyMemory (&m_pRegistryManager->GetRegFonts()->m_postFont,
		pLF, sizeof(LOGFONT));
	m_pRegistryManager->GetRegFonts()->m_postPtSize = PointSize;
}

void TGlobalOptions::SaveToRegistry(void)
{
	m_pRegistryManager->GetRegLayoutMdi()->Save();

	m_pRegistryManager->GetRegFonts()->Save();
	m_pRegistryManager->GetRegCompose()->Save();
	m_pRegistryManager->GetRegWarn()->Save();

	m_pRegistryManager->m_webHelper.Save();
	m_pRegistryManager->m_ftpHelper.Save();
	m_pRegistryManager->m_gopherHelper.Save();
	m_pRegistryManager->m_telnetHelper.Save();
	m_pRegistryManager->GetRegURLSettings()->Save ();

	m_pRegistryManager->GetBackgroundColors()->Save ();

	m_pRegistryManager->GetRegDirs()->Save();
	m_pRegistryManager->GetRegSwitch()->Save();
	m_pRegistryManager->GetRegUI()->Save();
	m_pRegistryManager->GetRegLayoutWin()->Save();

	m_pRegistryManager->GetRegPurge()->Save();
	m_pRegistryManager->GetRegStorage()->Save();

	m_pRegistryManager->GetRegSystem()->Save();
}

// ------------------------------------------------------------------------
// As we create the global options for the first time, gather information
//   that may have been seeded there by the Install Script
void TGlobalOptions::PreLoadFromRegistry(void)
{
	m_pRegistryManager->GetRegDirs()->PreLoad();
}

void TGlobalOptions::LoadFromRegistry(void)
{
	// load from registry
	m_pRegistryManager->GetRegLayoutMdi()->Load();

	m_pRegistryManager->GetRegFonts()->Load();
	m_pRegistryManager->GetRegCompose()->Load();
	m_pRegistryManager->GetRegWarn()->Load();

	m_pRegistryManager->m_webHelper.Load();
	m_pRegistryManager->m_ftpHelper.Load();
	m_pRegistryManager->m_gopherHelper.Load();
	m_pRegistryManager->m_telnetHelper.Load();
	m_pRegistryManager->GetRegURLSettings()->Load ();
	m_pRegistryManager->GetBackgroundColors()->Load ();

	m_pRegistryManager->GetRegDirs()->Load();
	m_pRegistryManager->GetRegSwitch()->Load();
	m_pRegistryManager->GetRegUI()->Load();
	m_pRegistryManager->GetRegLayoutWin()->Load();

	m_pRegistryManager->GetRegPurge()->Load();
	m_pRegistryManager->GetRegStorage()->Load();

	m_pRegistryManager->GetRegSystem()->Load();

	PostProcessing();
}

void TGlobalOptions::PostProcessing()
{
	// fill in default font for the FlatLbx
	//   - the configure dialog box needs this (as a default)
	//   - the 3 pane layout, the flatlbx is never created
	HWND desktop = GetDesktopWindow();
	HDC hdc = GetDC( desktop );

	LOGFONT lf;

	if (gpVariableFont->m_hObject == NULL)
	{
		// this should give us that small font (10 point)
		int ptSize = m_pRegistryManager->GetRegUI()->GetDefaultPointSize();
		setupSansSerifFont ( ptSize, hdc, &lf );
		gpVariableFont->CreateFontIndirect ( &lf );
	}

	if (gpHeaderCtrlFont->m_hObject == NULL)
	{
		int ptSize = m_pRegistryManager->GetRegUI()->GetHdrCtrlPointSize();
		setupSansSerifFont ( ptSize, hdc, &lf );
		gpHeaderCtrlFont->CreateFontIndirect ( &lf );
	}

	ReleaseDC (desktop, hdc);
}

const LOGFONT * TGlobalOptions::GetTreeFont()
{
	return m_pRegistryManager->GetRegFonts()->GetTreeFont();
}

void TGlobalOptions::SetTreeFont(const LOGFONT * pLF)
{
	m_pRegistryManager->GetRegFonts()->SetTreeFont ( pLF );
}

const LOGFONT*
TGlobalOptions::GetNewsgroupFont()
{
	return m_pRegistryManager->GetRegFonts()->GetNewsGroupFont();
}

void
TGlobalOptions::SetNewsgroupFont(const LOGFONT* pLF)
{
	m_pRegistryManager->GetRegFonts()->SetNewsGroupFont( pLF );
}

BOOL TGlobalOptions::IncludingOriginalMail()
{return m_pRegistryManager->GetRegCompose()->m_fIncludeOriginalMail;}
void TGlobalOptions::SetIncludeOriginalMail (BOOL fInclude)
{ m_pRegistryManager->GetRegCompose()->m_fIncludeOriginalMail = fInclude;}

BOOL TGlobalOptions::IncludingOriginalFollowup()
{return m_pRegistryManager->GetRegCompose()->m_fIncludeOriginalFollowup;}
void TGlobalOptions::SetIncludeOriginalFollowup (BOOL fInclude)
{ m_pRegistryManager->GetRegCompose()->m_fIncludeOriginalFollowup = fInclude;}

BOOL TGlobalOptions::IsLimitingQuoteLines ()
{return m_pRegistryManager->GetRegCompose()->m_fLimitQuotedLines;}
void TGlobalOptions::SetLimitQuoteLines (BOOL fLimit)
{m_pRegistryManager->GetRegCompose()->m_fLimitQuotedLines = fLimit;}

BOOL TGlobalOptions::IsAttachmentsInSeparateArticle()
{ return  m_pRegistryManager->GetRegCompose()->m_fattachSeparateArt; }
void TGlobalOptions::SetAttachmentsInSeparateArticle(BOOL fSep)
{ m_pRegistryManager->GetRegCompose()->m_fattachSeparateArt = fSep; }

int  TGlobalOptions::GetArticleSplitLen (void)
{ return m_pRegistryManager->GetRegCompose()->m_encodeSplitLen; }
void TGlobalOptions::SetArticleSplitLen (int n)
{ m_pRegistryManager->GetRegCompose()->m_encodeSplitLen = n; }

int  TGlobalOptions::GetWordWrap()
{ return m_pRegistryManager->GetRegCompose()->m_WordWrap; }
void TGlobalOptions::SetWordWrap(int n)
{ m_pRegistryManager->GetRegCompose()->m_WordWrap = n; }

int  TGlobalOptions::GetIDsToRemember()
{ return m_pRegistryManager->GetRegCompose()->m_IDsToRemember; }
void TGlobalOptions::SetIDsToRemember(int n)
{ m_pRegistryManager->GetRegCompose()->m_IDsToRemember = n; }

LONG TGlobalOptions::GetMaxQuotedLines ()
{return m_pRegistryManager->GetRegCompose()->m_maxQuotedLines;}
void TGlobalOptions::SetMaxQuotedLines (LONG max)
{ m_pRegistryManager->GetRegCompose()->m_maxQuotedLines = max;}

LONG TGlobalOptions::GetMaxEncodedLines ()
{return m_pRegistryManager->GetRegCompose()->m_maxEncodedLines;}
void TGlobalOptions::SetMaxEncodedLines (LONG max)
{m_pRegistryManager->GetRegCompose()->m_maxEncodedLines = max;}

const CString & TGlobalOptions::GetFollowupIntro ()
{ return m_pRegistryManager->GetRegCompose()->m_folIntro; }
void            TGlobalOptions::SetFollowupIntro (LPCTSTR s)
{ m_pRegistryManager->GetRegCompose()->m_folIntro = s; }

const CString & TGlobalOptions::GetReplyIntro ()
{ return m_pRegistryManager->GetRegCompose()->m_rplyIntro; }
void            TGlobalOptions::SetReplyIntro (LPCTSTR s)
{ m_pRegistryManager->GetRegCompose()->m_rplyIntro = s; }

const CString & TGlobalOptions::GetCCIntro ()
{ return m_pRegistryManager->GetRegCompose()->m_CCIntro; }
void            TGlobalOptions::SetCCIntro (LPCTSTR s)
{ m_pRegistryManager->GetRegCompose()->m_CCIntro = s; }

void            TGlobalOptions::Set_MIME_TransferEncoding(LPCTSTR s)
{ m_pRegistryManager->GetRegCompose()->m_MIME_ContentTransferEncoding = s; }
const CString & TGlobalOptions::Get_MIME_TransferEncoding(void)
{ return m_pRegistryManager->GetRegCompose()->m_MIME_ContentTransferEncoding; }

void            TGlobalOptions::Set_MIME_ContentType(LPCTSTR s)
{ m_pRegistryManager->GetRegCompose()->m_MIME_ContentType = s; }
const CString & TGlobalOptions::Get_MIME_ContentType(void)
{ return m_pRegistryManager->GetRegCompose()->m_MIME_ContentType; }

const CString& TGlobalOptions::GetIndentString()
{ return m_pRegistryManager->GetRegCompose()->m_IndentString; }
void           TGlobalOptions::SetIndentString(const CString& s)
{ m_pRegistryManager->GetRegCompose()->m_IndentString = s; }

const CString& TGlobalOptions::GetSubjectTemplate ()
{return m_pRegistryManager->GetRegCompose()->m_attachSubjectTemplate;}
void    TGlobalOptions::SetSubjectTemplate (const CString & templ)
{m_pRegistryManager->GetRegCompose()->m_attachSubjectTemplate = templ;}

TGlobalDef::EEncoding TGlobalOptions::GetEncodingType ()
{return m_pRegistryManager->GetRegCompose()->m_kEncodingType;}
void       TGlobalOptions::SetEncodingType (TGlobalDef::EEncoding kEncoding)
{ m_pRegistryManager->GetRegCompose()->m_kEncodingType = kEncoding;}

BOOL TGlobalOptions::WarnOnCatchup () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnCatchup;}
void TGlobalOptions::SetWarnOnCatchup (BOOL fWarn) { m_pRegistryManager->GetRegWarn()->m_fWarnOnCatchup = fWarn;}
BOOL TGlobalOptions::WarnOnExitCompose () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnExitCompose;}
void TGlobalOptions::SetWarnOnExitCompose (BOOL fWarn) { m_pRegistryManager->GetRegWarn()->m_fWarnOnExitCompose = fWarn;}
BOOL TGlobalOptions::WarnOnExitNews32 () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnExitNews32;}
void TGlobalOptions::SetWarnOnExitNews32 (BOOL fWarn) { m_pRegistryManager->GetRegWarn()->m_fWarnOnExitNews32 = fWarn;}
BOOL TGlobalOptions::WarnOnMarkRead () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnMarkRead;}
void TGlobalOptions::SetWarnOnMarkRead (BOOL fWarn) { m_pRegistryManager->GetRegWarn()->m_fWarnOnMarkRead = fWarn;}
BOOL TGlobalOptions::WarnOnUnsubscribe () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnUnsubscribe;}
void TGlobalOptions::SetWarnOnUnsubscribe (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnUnsubscribe = fWarn;}
BOOL TGlobalOptions::WarnOnSending () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnSending;}
void TGlobalOptions::SetWarnOnSending (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnSending = fWarn;}
BOOL TGlobalOptions::WarnOnDeleteBinary () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnDeleteBinary;}
void TGlobalOptions::SetWarnOnDeleteBinary (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnDeleteBinary = fWarn;}
BOOL TGlobalOptions::WarnOnErrorDuringDecode () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnErrorDuringDecode;}
void TGlobalOptions::SetWarnOnErrorDuringDecode (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnErrorDuringDecode = fWarn;}
BOOL TGlobalOptions::WarnOnRunExe () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnRunExe;}
void TGlobalOptions::SetWarnOnRunExe (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnRunExe = fWarn;}
BOOL TGlobalOptions::WarnOnManualRuleOffline () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnManualRuleOffline;}
void TGlobalOptions::SetWarnOnManualRuleOffline (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnManualRuleOffline = fWarn;}
BOOL TGlobalOptions::WarnOnDeleteArticle () {return m_pRegistryManager->GetRegWarn()->m_fWarnOnDeleteArticle;}
void TGlobalOptions::SetWarnOnDeleteArticle (BOOL fWarn) {m_pRegistryManager->GetRegWarn()->m_fWarnOnDeleteArticle = fWarn;}

int  TGlobalOptions::FilenameConflict () {return m_pRegistryManager->GetRegSwitch()->GetFilenameConflict();}
void TGlobalOptions::SetFilenameConflict (int action) {m_pRegistryManager->GetRegSwitch()->SetFilenameConflict(action);}

const CString& TGlobalOptions::GetDecodeDirectory ()
{ return m_pRegistryManager->GetRegDirs()->m_DecodeDestination;}
void    TGlobalOptions::SetDecodeDirectory (const CString & dir)
{ m_pRegistryManager->GetRegDirs()->m_DecodeDestination = dir;}

const CString& TGlobalOptions::GetOtherViewer ()
{ return m_pRegistryManager->GetRegDirs()->m_strOtherViewer;}
void    TGlobalOptions::SetOtherViewer (const CString &strOtherViewer)
{ m_pRegistryManager->GetRegDirs()->m_strOtherViewer = strOtherViewer;}

const CString& TGlobalOptions::GetAttachmentDirectory ()
{ return m_pRegistryManager->GetRegDirs()->m_AttachmentSource;}
void    TGlobalOptions::SetAttachmentDirectory (const CString & dir)
{ m_pRegistryManager->GetRegDirs()->m_AttachmentSource = dir;}

const CString& TGlobalOptions::GetSaveFileDirectory ()
{ return m_pRegistryManager->GetRegDirs()->m_SaveArticleDestination;}
void    TGlobalOptions::SetSaveFileDirectory (const CString & dir)
{ m_pRegistryManager->GetRegDirs()->m_SaveArticleDestination = dir;}

const CString& TGlobalOptions::GetPGPDirectory ()
{ return m_pRegistryManager->GetRegDirs()->m_pgpHome;}
void    TGlobalOptions::SetPGPDirectory (const CString & dir)
{ m_pRegistryManager->GetRegDirs()->m_pgpHome = dir;}

BOOL TGlobalOptions::IsEnvelopeLebEnabled (ELebType eType)
{
	switch (eType)
	{
	case kLebFrom:          return m_pRegistryManager->GetRegSwitch()->m_fShowLebFrom;
	case kLebOrganization:  return m_pRegistryManager->GetRegSwitch()->m_fShowLebOrg;
	case kLebReply:         return m_pRegistryManager->GetRegSwitch()->m_fShowLebReply;
	case kLebFollowup:      return m_pRegistryManager->GetRegSwitch()->m_fShowLebFollowup;
	case kLebDistribution:  return m_pRegistryManager->GetRegSwitch()->m_fShowLebDistribution;
	}
	return FALSE;
}
void TGlobalOptions::SetEnvelopeLebEnabled (ELebType eType, BOOL fOn)
{
	switch (eType)
	{
	case kLebFrom:
		m_pRegistryManager->GetRegSwitch()->m_fShowLebFrom         = fOn;
		break;
	case kLebOrganization:
		m_pRegistryManager->GetRegSwitch()->m_fShowLebOrg          = fOn;
		break;
	case kLebReply:
		m_pRegistryManager->GetRegSwitch()->m_fShowLebReply        = fOn;
		break;
	case kLebFollowup:
		m_pRegistryManager->GetRegSwitch()->m_fShowLebFollowup     = fOn;
		break;
	case kLebDistribution:
		m_pRegistryManager->GetRegSwitch()->m_fShowLebDistribution = fOn;
		break;
	}
}

BOOL TGlobalOptions::DownloadFullHeader()
{return m_pRegistryManager->GetRegSwitch()->m_fDownloadFullheader; }
void TGlobalOptions::SetDlFullHeader(BOOL full)
{m_pRegistryManager->GetRegSwitch()->m_fDownloadFullheader = full; }

BOOL TGlobalOptions::UsingPopup ()
{ return m_pRegistryManager->GetRegSwitch()->m_fUsePopup; }
void TGlobalOptions::SetUsePopup (BOOL fUse)
{ m_pRegistryManager->GetRegSwitch()->m_fUsePopup = fUse; }

BOOL TGlobalOptions::IsCustomTreeFont(void)
{ return m_pRegistryManager->GetRegSwitch()->m_fCustomTreeFont; }
void TGlobalOptions::CustomTreeFont(BOOL fCustom)
{  m_pRegistryManager->GetRegSwitch()->m_fCustomTreeFont = fCustom; }

BOOL TGlobalOptions::IsCustomNGFont(void)
{  return m_pRegistryManager->GetRegSwitch()->m_fCustomNewsgroupFont; }
void TGlobalOptions::CustomNGFont(BOOL fCustom)
{ m_pRegistryManager->GetRegSwitch()->m_fCustomNewsgroupFont = fCustom; }

BOOL  TGlobalOptions::IsLaunchingViewerOnManual ()
{return m_pRegistryManager->GetRegSwitch()->m_fLaunchViewerOnManual;}
void  TGlobalOptions::SetLaunchViewerOnManual (BOOL fLaunch)
{m_pRegistryManager->GetRegSwitch()->m_fLaunchViewerOnManual = fLaunch;}

BOOL  TGlobalOptions::IsLaunchViewerByRules ()
{return m_pRegistryManager->GetRegSwitch()->m_fLaunchViewerByRules;}
void  TGlobalOptions::SetLaunchViewerByRules (BOOL fLaunch)
{m_pRegistryManager->GetRegSwitch()->m_fLaunchViewerByRules = fLaunch;}

BOOL  TGlobalOptions::IsUseOtherViewer ()
{return m_pRegistryManager->GetRegSwitch()->m_fUseOtherViewer;}
void  TGlobalOptions::SetUseOtherViewer (BOOL fUseOtherViewer)
{m_pRegistryManager->GetRegSwitch()->m_fUseOtherViewer = fUseOtherViewer;}

BOOL  TGlobalOptions::IsUseImageGallery ()
{return m_pRegistryManager->GetRegSwitch()->m_fUseImageGallery;}
void  TGlobalOptions::SetUseImageGallery (BOOL fUseImageGallery)
{m_pRegistryManager->GetRegSwitch()->m_fUseImageGallery = fUseImageGallery;}

BOOL TGlobalOptions::UsingPGP ()
{return m_pRegistryManager->GetRegSwitch()->m_fUsePGP;}
void TGlobalOptions::SetUsePgp (BOOL fUse)
{m_pRegistryManager->GetRegSwitch()->m_fUsePGP = fUse;}

TNewsGroup::EStorageOption TGlobalOptions::GetStorageMode()
{return m_pRegistryManager->GetRegStorage()->GetStorageMode();}
void  TGlobalOptions::SetStorageMode (TNewsGroup::EStorageOption kStorageMode)
{m_pRegistryManager->GetRegStorage()->SetStorageMode (kStorageMode);}

TGlobalDef::EThreadSort TGlobalOptions::GetThreadSort()
{ return m_pRegistryManager->GetRegUI()->GetThreadSort(); }

void TGlobalOptions::SetThreadSort (TGlobalDef::EThreadSort kSort)
{ m_pRegistryManager->GetRegUI()->SetThreadSort(kSort); }

void TGlobalOptions::SetThreadSortWithin (TGlobalDef::EThreadSortWithin kSort)
{ m_pRegistryManager->GetRegUI()->SetThreadSortWithin(kSort);}

TGlobalDef::EThreadViewType TGlobalOptions::GetThreadViewType ()
{ return m_pRegistryManager->GetRegUI()->GetThreadViewType(); }

void TGlobalOptions::SetThreadViewType (TGlobalDef::EThreadViewType kViewType)
{ m_pRegistryManager->GetRegUI()->SetThreadViewType(kViewType); }

void TGlobalOptions::SaveUISettings ()
{ m_pRegistryManager->GetRegUI()->Save(); }

const CString& TGlobalOptions::GetDistribution()
{ return m_pRegistryManager->GetRegCompose()->GetDistribution(); }

void  TGlobalOptions::SetDistribution (LPCTSTR dist)
{ m_pRegistryManager->GetRegCompose()->SetDistribution(dist); }

const CStringList& TGlobalOptions::GetCustomHeaders ()
{ return m_pRegistryManager->GetRegCompose()->GetCustomHeaders(); }

void TGlobalOptions::SetCustomHeaders (CStringList &sCustomHeaders)
{ m_pRegistryManager->GetRegCompose()->SetCustomHeaders(sCustomHeaders); }

BOOL TGlobalOptions::GetWriteDescription ()
{ return m_pRegistryManager->GetRegSwitch()->m_fWriteDescription; }
void TGlobalOptions::SetWriteDescription (BOOL fNew)
{ m_pRegistryManager->GetRegSwitch()->m_fWriteDescription = fNew; }

TRegLayoutMdi*  TGlobalOptions::GetRegLayoutMdi()
{ return m_pRegistryManager->GetRegLayoutMdi(); }

TRegFonts*      TGlobalOptions::GetRegFonts()
{ return m_pRegistryManager -> GetRegFonts(); }

TRegCompose*    TGlobalOptions::GetRegCompose()
{ return m_pRegistryManager -> GetRegCompose(); }

TRegWarn*       TGlobalOptions::GetRegWarn()
{ return m_pRegistryManager -> GetRegWarn(); }

TRegDirectory*  TGlobalOptions::GetRegDirs()
{ return m_pRegistryManager -> GetRegDirs(); }

TRegSystem*     TGlobalOptions::GetRegSystem()
{ return m_pRegistryManager -> GetRegSystem(); }

TRegSwitch*     TGlobalOptions::GetRegSwitch()
{ return m_pRegistryManager -> GetRegSwitch(); }

TRegPurge*      TGlobalOptions::GetRegPurge()
{ return m_pRegistryManager->GetRegPurge(); }

TRegUI*         TGlobalOptions::GetRegUI()
{ return m_pRegistryManager->GetRegUI(); }

TRegLayoutWin*  TGlobalOptions::GetRegLayoutWin()
{ return m_pRegistryManager->GetRegLayoutWin(); }

TUrlDde * TGlobalOptions::GetWebHelperPointer()
{return &m_pRegistryManager->m_webHelper; }
TUrlDde * TGlobalOptions::GetFtpHelperPointer()
{return &m_pRegistryManager->m_ftpHelper;}
TUrlDde * TGlobalOptions::GetGopherHelperPointer()
{return &m_pRegistryManager->m_gopherHelper;}
TUrlDde * TGlobalOptions::GetTelnetHelperPointer()
{return &m_pRegistryManager->m_telnetHelper;}
TURLSettings * TGlobalOptions::GetURLSettings()
{return m_pRegistryManager->GetRegURLSettings();}

TBackgroundColors* TGlobalOptions::GetBackgroundColors()
{return m_pRegistryManager->GetBackgroundColors(); }

TRegStorage* TGlobalOptions::GetRegStorage()
{
	return m_pRegistryManager->GetRegStorage();
}

int   TGlobalOptions::GetSendCharset ()
{
	return GetRegCompose()->GetSendCharset();
}

BOOL  TGlobalOptions::GetSend8Bit()
{
	return GetRegCompose()->GetAllow8Bit();
}

BOOL  TGlobalOptions::GetSend8BitHdrs()
{
	return GetRegCompose()->Get8BitHdrs();
}

