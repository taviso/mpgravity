/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgswit.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.4  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.3  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:46  richard_wood
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
#include "mplib.h"
#include "rgswit.h"
#include "regutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TRegSwitch::TRegSwitch()
{
   default_values();
   m_itemCount = 65;
   m_pTable = new TableItem[m_itemCount];
   SetItem (m_pTable + 0, "FldFrom",             kREGBool, &m_fShowLebFrom);
   SetItem (m_pTable + 1, "FldOrg",              kREGBool, &m_fShowLebOrg);
   SetItem (m_pTable + 2, "FldReply",            kREGBool, &m_fShowLebReply);
   SetItem (m_pTable + 3, "FldFollowup",         kREGBool, &m_fShowLebFollowup);
   SetItem (m_pTable + 4, "FldDist",             kREGBool, &m_fShowLebDistribution);
   SetItem (m_pTable + 5, "FullHdr",             kREGBool, &m_fDownloadFullheader);
   SetItem (m_pTable + 6, "Popup",               kREGBool, &m_fUsePopup);
   SetItem (m_pTable + 7, "CustomTreeFnt",       kREGBool, &m_fCustomTreeFont);
   SetItem (m_pTable + 8, "CustomNGFnt",         kREGBool, &m_fCustomNewsgroupFont);
   SetItem (m_pTable + 9, "CustomFlFnt",         kREGBool, &m_fCustomFlatFont);
   SetItem (m_pTable +10, "PGP",                 kREGBool, &m_fUsePGP);
   SetItem (m_pTable +11, "LaunchViewerManual",  kREGBool, &m_fLaunchViewerOnManual);
   SetItem (m_pTable +12, "LaunchViewerRules",   kREGBool, &m_fLaunchViewerByRules);
   SetItem (m_pTable +13, "UseOtherViewer",      kREGBool, &m_fUseOtherViewer);
   SetItem (m_pTable +14, "AskConnect",          kREGBool, &m_fAskBeforeConnect);
   SetItem (m_pTable +15, "CustomTreeBG",        kREGBool, &m_fCustomTreeBG);
   SetItem (m_pTable +16, "CustomNGBG",          kREGBool, &m_fCustomNewsgroupBG);
   SetItem (m_pTable +17, "CustomFlBG",          kREGBool, &m_fCustomFlatBG);
   SetItem (m_pTable +18, "CustomArtBG",         kREGBool, &m_fCustomArtBG);
   SetItem (m_pTable +19, "CatchUpLocSrv",       kREGBool, &m_fCatchUpLocSrv);
   SetItem (m_pTable +20, "CatchUpCPM",          kREGBool, &m_fCatchUpCPM);
   SetItem (m_pTable +21, "SpaceFullPgDn",       kREGBool, &m_fSpaceFullPgDn);
   SetItem (m_pTable +22, "ResumeDecode",        kREGBool, &m_fResumeDecJob);
   SetItem (m_pTable +23, "SrchGotoLd",          kREGBool, &m_fSrchGotoLoad);
   SetItem (m_pTable +24, "CatchUpOnRtrv",       kREGBool, &m_fCatchUpOnRtrv,  TRUE);
   SetItem (m_pTable +25, "CatchUpLoadNext",     kREGBool, &m_fCatchUpLoadNext,  TRUE);
   SetItem (m_pTable +26, "CatchUpKeepTags",     kREGBool, &m_fCatchUpKeepTags,  TRUE);
   SetItem (m_pTable +27, "TagMarksUnread",      kREGBool, &m_fTaggingMarksUnread, TRUE);
   SetItem (m_pTable +28, "SpellCheck",          kREGBool, &m_fSpellcheck, TRUE);
   SetItem (m_pTable +29, "DecodeMarksRead",     kREGInt,  &m_iDecodeMarksRead, TRUE);
   SetItem (m_pTable +30, "ViewIntoDB",          kREGBool, &m_fDBSaveMessages, TRUE);
   SetItem (m_pTable +31, "DeleteDecodedOnExit", kREGBool, &m_fDeleteOnExit, TRUE);
   SetItem (m_pTable +32, "WriteDescription",    kREGBool, &m_fWriteDescription, TRUE);
   SetItem (m_pTable +33, "SkipHTMLPart",        kREGBool, &m_fSkipHTML, TRUE);
   SetItem (m_pTable +34, "UseImageGallery",     kREGBool, &m_fUseImageGallery);
   SetItem (m_pTable +35, "AutoRescore",         kREGBool, &m_fAutoRescore);
   SetItem (m_pTable +36, "ScoreWholeWord",      kREGBool, &m_fScoreWholeWord, TRUE);
   SetItem (m_pTable +37, "ScoreType",           kREGInt,  &m_iScoreType, TRUE);
   SetItem (m_pTable +38, "ScoreWhere",          kREGInt,  &m_iScoreWhere, TRUE);
   SetItem (m_pTable +39, "AttNameAction",       kREGInt,  &m_iFilenameConflict, TRUE);
   SetItem (m_pTable +40, "PrintFullHeader",     kREGBool, &m_fPrintFullHeader, TRUE);
   SetItem (m_pTable +41, "PauseSpace",          kREGInt,  &m_iPauseSpace, TRUE);
   SetItem (m_pTable +42, "IsPauseSpace",        kREGBool, &m_fPauseSpace, TRUE);
   SetItem (m_pTable +43, "StripSig",            kREGBool, &m_fStripSignature, TRUE);
   SetItem (m_pTable +44, "CCSelf",              kREGBool, &m_fCCSelf, TRUE);
   SetItem (m_pTable +45, "ScoreExpires",        kREGBool, &m_fScoreExpires, TRUE);
   SetItem (m_pTable +46, "ScoreExpirationDays", kREGInt,  &m_iScoreExpirationDays, TRUE);
   SetItem (m_pTable +47, "BozoExpireMRU",       kREGBool, &m_fBozoExpireMRU, TRUE);
   SetItem (m_pTable +48, "BozoExpirationDaysMRU", kREGInt,  &m_iBozoExpirationDaysMRU, TRUE);
   SetItem (m_pTable +49, "IgnoreMoveNU",        kREGBool, &m_fIgnoreMNU, TRUE);
   SetItem (m_pTable +50, "TagMoveNU",           kREGBool, &m_fTagMNU,    TRUE);
   SetItem (m_pTable +51, "WatchMoveNU",         kREGBool, &m_fWatchMNU,  TRUE);
   SetItem (m_pTable +52, "SingleKeyRead",       kREGBool, &m_fSKRead,    TRUE);
   SetItem (m_pTable +53, "MButSingleKeyRead",   kREGBool, &m_fMiddleButtonSKR,     TRUE);
   SetItem (m_pTable +54, "PureThreading",       kREGBool, &m_fPureThreading,       TRUE);
   SetItem (m_pTable +55, "LeadFocusToArtpane",  kREGBool, &m_fReadPutFocusArticle, TRUE);
   SetItem (m_pTable +56, "LogGroupCmd",         kREGBool, &m_fLogGroupCmds,        TRUE);
   SetItem (m_pTable +57, "MR.Forward",          kREGBool, &m_fMR_fwd,              TRUE);
   SetItem (m_pTable +58, "MR.Followup",         kREGBool, &m_fMR_followup,         TRUE);
   SetItem (m_pTable +59, "MR.Reply",            kREGBool, &m_fMR_reply,            TRUE);
   SetItem (m_pTable +60, "MR.View",             kREGBool, &m_fMR_view,             TRUE);
   SetItem (m_pTable +61, "MR.FileSave",         kREGBool, &m_fMR_filesave,         TRUE);
   SetItem (m_pTable +62, "AutoSaveDecJobs",     kREGBool, &m_fAutoSaveDecJobs,     TRUE);
   SetItem (m_pTable +63, "AutoSaveDecJobsInterval",kREGInt, &m_iAutoSaveDecJobsInterval, TRUE);
   SetItem (m_pTable +64, "XFaces",              kREGBool, &m_fXFaces, TRUE);

   ASSERT(65 == m_itemCount);
}

TRegSwitch::~TRegSwitch()
{
   delete [] m_pTable;
}

int TRegSwitch::Load()
{
   return TRegistryBase::Load ( GetGravityRegKey()+"Switches" );
}

int TRegSwitch::Save()
{
   return TRegistryBase::Save ( GetGravityRegKey()+"Switches" );
}

void TRegSwitch::read()
{
   LONG lRet = rgReadTable (m_itemCount, m_pTable);
}

void TRegSwitch::write()
{
   LONG lRet = rgWriteTable (m_itemCount, m_pTable);
}

void TRegSwitch::default_values()
{
   m_fShowLebFrom          = FALSE;
   m_fShowLebOrg           = FALSE;
   m_fShowLebReply         = FALSE;
   m_fShowLebFollowup      = FALSE;
   m_fShowLebDistribution  = FALSE;

   m_fDownloadFullheader   = TRUE;
   m_fUsePopup             = FALSE;
   m_fCustomTreeFont       = FALSE;
   m_fCustomNewsgroupFont  = FALSE;
   m_fCustomFlatFont       = FALSE;

   m_fUsePGP               = FALSE;
   m_fLaunchViewerOnManual = FALSE;
   m_fLaunchViewerByRules  = FALSE;
   m_fUseOtherViewer       = FALSE;
   m_fUseImageGallery      = FALSE;
   m_fDeleteOnExit         = FALSE;

   m_fAskBeforeConnect     = TRUE;

   m_fCustomTreeBG         = FALSE;
   m_fCustomNewsgroupBG    = FALSE;
   m_fCustomFlatBG         = FALSE;
   m_fCustomArtBG          = FALSE;

   m_fCatchUpLocSrv        = FALSE;
   m_fCatchUpCPM           = TRUE;
   m_fCatchUpOnRtrv        = FALSE;
   m_fCatchUpLoadNext      = FALSE;
   m_fCatchUpKeepTags      = TRUE;

   m_fSpaceFullPgDn        = TRUE;
   m_fResumeDecJob         = TRUE;
   m_fSrchGotoLoad         = TRUE;
   m_fTaggingMarksUnread   = TRUE;
   m_fSpellcheck           = FALSE;
   m_iDecodeMarksRead      = 0;
   m_fDBSaveMessages       = FALSE;    // DL body to view it, save into DB
   m_fWriteDescription     = FALSE;
   m_fSkipHTML             = TRUE;
   m_fAutoRescore          = FALSE;
   m_fScoreWholeWord       = FALSE;
   m_iScoreType            = 0;
   m_iScoreWhere           = 2;
   m_fScoreExpires         = FALSE;
   m_iScoreExpirationDays  = 7;
   m_iFilenameConflict     = 1;        // default action = skip
   m_fPrintFullHeader      = FALSE;
   m_iPauseSpace           = 5;        // pause decoding if free space < 5 megs
   m_fPauseSpace           = TRUE;
   m_fStripSignature       = TRUE;     // strip after "-- \r\n"
   m_fCCSelf               = FALSE;
   m_fBozoExpireMRU        = TRUE;
   m_iBozoExpirationDaysMRU= 7;
   m_fIgnoreMNU            = TRUE;
   m_fTagMNU               = TRUE;
   m_fWatchMNU             = TRUE;
   m_fSKRead               = TRUE;
   m_fMiddleButtonSKR      = TRUE;
   m_fPureThreading        = FALSE;
   m_fReadPutFocusArticle  = TRUE;
   m_fLogGroupCmds         = FALSE;

   m_fMR_fwd               = FALSE;   // weird but true
   m_fMR_followup          = TRUE;
   m_fMR_reply             = TRUE;
   m_fMR_view              = TRUE;
   m_fMR_filesave          = TRUE;

   m_fAutoSaveDecJobs      = TRUE;
   m_iAutoSaveDecJobsInterval=30;     // save every 30 minutes
   m_fXFaces               = TRUE;
}
