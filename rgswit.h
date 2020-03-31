/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgswit.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
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

#pragma once

#include "rgbase.h"

class TRegSwitch : public TRegistryBase {
public:
	enum EPostDecode { kNothing = 0, kReadAfterDecode = 100,
		kReadAfterView = 200 };
public:
	TRegSwitch();
	~TRegSwitch();
	int Load();
	int Save();

	BOOL IsDefaultTreeBG() { return !m_fCustomTreeBG; }
	BOOL IsDefaultNGBG()   { return !m_fCustomNewsgroupBG; }
	BOOL IsDefaultFlatBG() { return !m_fCustomFlatBG; }
	BOOL IsDefaultArticleBG() { return !m_fCustomArtBG; }

	void SetDefaultTreeBG(BOOL f) { m_fCustomTreeBG = !f; }
	void SetDefaultNGBG(BOOL f)   { m_fCustomNewsgroupBG = !f; }
	void SetDefaultFlatBG(BOOL f) { m_fCustomFlatBG = !f; }
	void SetDefaultArticleBG(BOOL f) { m_fCustomArtBG = !f; }

	BOOL GetCatchUpAll ()            { return m_fCatchUpLocSrv; }
	void SetCatchUpAll (BOOL all)    { m_fCatchUpLocSrv = all; }

	// cross-post management
	BOOL GetCatchUpCPM ()
	{
#ifdef LITE
		return FALSE;
#else
		return m_fCatchUpCPM;
#endif
	}
	void SetCatchUpCPM (BOOL cpm) { m_fCatchUpCPM = cpm; }

	BOOL GetCatchUpOnRetrieve ()  { return m_fCatchUpOnRtrv; }
	void SetCatchUpOnRetrieve (BOOL fCatchup) { m_fCatchUpOnRtrv = fCatchup; }

	BOOL GetCatchUpLoadNext ()    { return m_fCatchUpLoadNext; }
	void SetCatchUpLoadNext (BOOL fLoadNext) { m_fCatchUpLoadNext = fLoadNext; }

	BOOL GetCatchUpKeepTags ()    { return m_fCatchUpKeepTags; }
	void SetCatchUpKeepTags (BOOL fKeepTags) { m_fCatchUpKeepTags = fKeepTags; }

	BOOL GetSpaceBarFullPgDn ()   { return m_fSpaceFullPgDn; }

	// resume decoding after connection
	BOOL GetResumePausedDecJob () { return m_fResumeDecJob; }
	void SetResumePausedDecJob (BOOL fRestart) { m_fResumeDecJob = fRestart; }

	// "Go To Article" in Search dialog should load the article
	BOOL GetSearchGotoLoad ()  { return m_fSrchGotoLoad; }
	void SetSearchGotoLoad (BOOL fLoad)  { m_fSrchGotoLoad = fLoad; }

	// mark Art unread when you tag it
	BOOL GetTagDoesUnread () { return m_fTaggingMarksUnread; }
	void SetTagDoesUnread (BOOL f) { m_fTaggingMarksUnread = f; }

	// mark Art read after decoding.
	//   a)  do nothing
	//   b)  mark read after decoding
	//   c)  mark read after viewing
	EPostDecode  GetDecodeMarksRead ()
	{
		return static_cast<TRegSwitch::EPostDecode>(m_iDecodeMarksRead);
	}
	void SetDecodeMarksRead (TRegSwitch::EPostDecode eDec)
	{
		m_iDecodeMarksRead = (int) eDec;
	}

	// after downloading a message body to view it, do you want to
	//   cache/store it into the database?
	BOOL GetDBSaveDLMessages () { return m_fDBSaveMessages; }
	void SetDBSaveDLMessages (BOOL fSav) { m_fDBSaveMessages = fSav; }

	// delete decoded files on exit
	BOOL GetDeleteOnExit () { return m_fDeleteOnExit; }
	void SetDeleteOnExit (BOOL fDelete) { m_fDeleteOnExit = fDelete; }

	// for multipart/alternative, just show "text/plain" part, skip "text/html"
	BOOL GetSkipHTMLPart () { return m_fSkipHTML; }
	void SetSkipHTMLPart (BOOL fSkip) { m_fSkipHTML = fSkip; }

	BOOL GetAutoRescore () { return m_fAutoRescore; }
	void SetAutoRescore (BOOL bSet) { m_fAutoRescore = bSet; }

	BOOL GetScoreWholeWord () { return m_fScoreWholeWord; }
	void SetScoreWholeWord (BOOL bSet) { m_fScoreWholeWord = bSet; }
	int GetScoreType () { return m_iScoreType; }
	void SetScoreType (int iVal) { m_iScoreType = iVal; }
	int GetScoreWhere () { return m_iScoreWhere; }
	void SetScoreWhere (int iVal) { m_iScoreWhere = iVal; }
	BOOL GetScoreExpires () { return m_fScoreExpires; }
	void SetScoreExpires (BOOL bSet) { m_fScoreExpires = bSet; }
	int GetScoreExpirationDays () { return m_iScoreExpirationDays; }
	void SetScoreExpirationDays (int iVal) { m_iScoreExpirationDays = iVal; }

	int GetPauseSpace () { return m_iPauseSpace; }
	void SetPauseSpace (int iVal) { m_iPauseSpace = iVal; }

	BOOL GetPausingLowSpace ()        { return m_fPauseSpace; }
	void SetPausingLowSpace (BOOL f)  { m_fPauseSpace = f; }

	int  GetFilenameConflict () { return m_iFilenameConflict; }
	void SetFilenameConflict (int action) { m_iFilenameConflict = action; }

	BOOL GetStripSignature () { return m_fStripSignature; }
	void SetStripSignature (BOOL fStrip) { m_fStripSignature = fStrip; }

	// Single
	BOOL GetSingleKeyDoesRead ()        { return m_fSKRead; }
	void SetSingleKeyDoesRead (BOOL sk) { m_fSKRead = sk; }

	// middle button maps to SKR
	BOOL GetMiddleButtonSKR ()          { return m_fMiddleButtonSKR; }
	void SetMiddleButtonSKR (BOOL sk)   { m_fMiddleButtonSKR = sk; }

	BOOL GetThreadPureThreading ()      { return m_fPureThreading; }
	void SetThreadPureThreading (BOOL p){ m_fPureThreading =p; }

	BOOL GetReadPutFocusToArtpane ()       { return m_fReadPutFocusArticle; }
	void SetReadPutFocusToArtpane (BOOL f) { m_fReadPutFocusArticle = f; }

	BOOL GetLogGroupCmds  ()               { return m_fLogGroupCmds; }
	void SetLogGroupCmds  (BOOL f)         { m_fLogGroupCmds = f; }

	BOOL GetMarkReadForward ()             { return m_fMR_fwd; }
	void SetMarkReadForward (BOOL f)       { m_fMR_fwd = f;    }

	BOOL GetMarkReadFollowup ()            { return m_fMR_followup; }
	void SetMarkReadFollowup (BOOL f)      { m_fMR_followup = f;    }

	BOOL GetMarkReadReply ()               { return m_fMR_reply; }
	void SetMarkReadReply (BOOL f)         { m_fMR_reply = f;    }

	BOOL GetMarkReadDisplay ()                { return m_fMR_view; }
	void SetMarkReadDisplay (BOOL f)          { m_fMR_view = f;    }

	BOOL GetMarkReadFileSave ()            { return m_fMR_filesave; }
	void SetMarkReadFileSave (BOOL f)      { m_fMR_filesave = f;    }

	BOOL GetAutoSaveDecodeJobs ()          { return m_fAutoSaveDecJobs; }
	void SetAutoSaveDecodeJobs (BOOL f)    { m_fAutoSaveDecJobs=f; }

	int  GetAutoSaveDecodeJobsInterval ()       { return m_iAutoSaveDecJobsInterval; }
	void SetAutoSaveDecodeJobsInterval (int i)  { m_iAutoSaveDecJobsInterval=i; }

	BOOL GetXFaces()        { return m_fXFaces; }
	void SetXFaces(BOOL f)  { m_fXFaces = f; }
public:
	BOOL  m_fShowLebFrom;            // optional lebs in Compose window
	BOOL  m_fShowLebOrg;
	BOOL  m_fShowLebReply;
	BOOL  m_fShowLebFollowup;
	BOOL  m_fShowLebDistribution;

	BOOL  m_fDownloadFullheader;
	BOOL  m_fUsePopup;
	BOOL  m_fCustomTreeFont;
	BOOL  m_fCustomNewsgroupFont;
	BOOL  m_fCustomFlatFont;

	BOOL  m_fCustomTreeBG;            // using Custom background color
	BOOL  m_fCustomNewsgroupBG;
	BOOL  m_fCustomFlatBG;
	BOOL  m_fCustomArtBG;

	BOOL  m_fUsePGP;
	BOOL  m_fLaunchViewerOnManual;
	BOOL  m_fLaunchViewerByRules;
	BOOL  m_fUseOtherViewer;
	BOOL  m_fUseImageGallery;
	BOOL  m_fDeleteOnExit;           // delete decoded files on exit
	BOOL  m_fAskBeforeConnect;
	BOOL  m_fSpaceFullPgDn;          // space bar , full page down
	BOOL  m_fResumeDecJob;           // resume paused decode job
	BOOL  m_fSrchGotoLoad;           // Load art after "go to"
	BOOL  m_fTaggingMarksUnread;     // mark Art unread when you tag it
	BOOL  m_fSpellcheck;             // run spellchecker before sending
	BOOL  m_fDBSaveMessages;         // store viewed msgs into the dbase
	BOOL  m_fWriteDescription;       // write 4dos-compatible description
	BOOL  m_fSkipHTML;               // skip multipart/alternative "text/html"
	BOOL  m_fPrintFullHeader;        // for printing
	BOOL  m_fCCSelf;                 // cc to self when posting/following-up
	BOOL  m_fBozoExpireMRU;          // purge bozo bin entry?
	int   m_iBozoExpirationDaysMRU;  // days before purging bozo entry

protected:
	// TRUE  - catchup all articles (local and server)
	// FALSE - catchup local articles
	BOOL  m_fCatchUpLocSrv;

	// TRUE  - articles marked read via catchup also do Cross-Post-Mgmt
	// FALSE - articles are marked read in this newsgroup
	BOOL  m_fCatchUpCPM;

	BOOL  m_fCatchUpOnRtrv;          // catchup the group when retrieving?
	BOOL  m_fCatchUpLoadNext;        // Load next group after catching up this
	BOOL  m_fCatchUpKeepTags;        // Don't mark tagged read on catchup

	int   m_iDecodeMarksRead;

	BOOL  m_fAutoRescore;            // rescore after closing quick-score dialog

	BOOL  m_fScoreWholeWord;         // MRU value
	int   m_iScoreType;              // MRU value
	int   m_iScoreWhere;             // MRU value
	BOOL  m_fScoreExpires;           // MRU value
	int   m_iScoreExpirationDays;    // MRU value

	int   m_iPauseSpace;             // pause decoding if free space less than this
	BOOL  m_fPauseSpace;

	// what to do if decode filename is in use
	int   m_iFilenameConflict;       // 0=ask, 1=skip, 2=overwrite, 3=new name

	BOOL  m_fStripSignature;         // if TRUE, strip after "-- \r\n"

	BOOL  m_fIgnoreMNU;              // after Ignore, move to next unread
	BOOL  m_fTagMNU;                 // after Tag,    move to next unread
	BOOL  m_fWatchMNU;               // after Watch,  move to next unread

	BOOL  m_fSKRead;                 // single-key-read Reads next msg
	BOOL  m_fMiddleButtonSKR;

	BOOL  m_fPureThreading;          // pure means not thread by subject
	BOOL  m_fReadPutFocusArticle;    // after double-click put focus in artpane? like agent?

	BOOL  m_fLogGroupCmds;           // log output of GROUP NNTP cmd

	BOOL  m_fMR_fwd     ;
	BOOL  m_fMR_followup;
	BOOL  m_fMR_reply   ;
	BOOL  m_fMR_view    ;            // mark read when View article
	BOOL  m_fMR_filesave;

	BOOL  m_fAutoSaveDecJobs;
	int   m_iAutoSaveDecJobsInterval;//in minutes
	BOOL  m_fXFaces;

protected:
	void read();
	void write();
	void default_values();
};
