/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgurl.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:47  richard_wood
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

#include "pobject.h" // for PObject
#include "mplib.h"   //  for TException
#include "regutil.h"
#include "rgbase.h"

class TURLSettings : public TRegistryBase
{
public:
	TURLSettings ();
	~TURLSettings ();
	TURLSettings & operator=(const TURLSettings &);

	LONG  MailUsingRegistry() {return m_fMailUseRegistry;}
	void  SetMailUsingRegistry (LONG fUseRegistry)
	{m_fMailUseRegistry = fUseRegistry;}

	LONG  WebUsingRegistry() {return m_fWebUseRegistry;}
	void  SetWebUsingRegistry (LONG  fUseRegistry)
	{m_fWebUseRegistry = fUseRegistry;}

	LONG  FtpUsingRegistry() {return m_fFtpUseRegistry;}
	void  SetFtpUsingRegistry (LONG  fUseRegistry)
	{m_fFtpUseRegistry = fUseRegistry;}

	//LONG  GopherUsingRegistry() {return m_fGopherUseRegistry;}
	//void  SetGopherUsingRegistry (LONG  fUseRegistry)
	//{m_fGopherUseRegistry = fUseRegistry;}

	//LONG  TelnetUsingRegistry() {return m_fTelnetUseRegistry;}
	//void  SetTelnetUsingRegistry (LONG  fUseRegistry)
	//{m_fTelnetUseRegistry = fUseRegistry;}

	COLORREF GetHotlinkColor() {return m_hotlinkColor;}
	void  SetHotlinkColor (COLORREF color)
	{m_hotlinkColor = color;}

	LONG  UnderliningLinks() { return m_fUnderlineLinks;}

	void  SetUnderliningLinks (LONG fUnderline)
	{m_fUnderlineLinks = fUnderline;}

	CString  GetWebPattern() {return m_webPattern;}
	CString  GetFtpPattern() {return m_ftpPattern;}
	//CString  GetGopherPattern() {return m_gopherPattern;}
	//CString  GetTelnetPattern() {return m_telnetPattern;}
	CString  GetMailToPattern() {return m_mailToPattern;}
	CString  GetNewsPattern()   {return m_newsPattern;}

	void  SetWebPattern   (LPCTSTR   pattern) {m_webPattern = pattern;}
	void  SetFtpPattern   (LPCTSTR   pattern) {m_ftpPattern = pattern;}
	//void  SetGopherPattern(LPCTSTR   pattern) {m_gopherPattern = pattern;}
	//void  SetTelnetPattern(LPCTSTR   pattern) {m_telnetPattern = pattern;}
	void  SetMailToPattern(LPCTSTR   pattern) {m_mailToPattern = pattern;}
	void  SetNewsPattern(LPCTSTR   pattern) {m_newsPattern = pattern;}

	LONG  HighlightNews() {return m_fHighlightNews;}
	LONG  HighlightMail() {return m_fHighlightMail;}
	LONG  HighlightWeb() {return m_fHighlightWeb;}
	LONG  HighlightFtp() {return m_fHighlightFtp;}
	//LONG  HighlightGopher() {return m_fHighlightGopher;}
	//LONG  HighlightTelnet() {return m_fHighlightTelnet;}

	void  SetHighlightMail(BOOL fHighlight) {m_fHighlightMail = fHighlight;}
	void  SetHighlightWeb(BOOL fHighlight) {m_fHighlightWeb = fHighlight;}
	void  SetHighlightFtp(BOOL fHighlight) {m_fHighlightFtp = fHighlight;}
	//void  SetHighlightGopher(BOOL fHighlight) {m_fHighlightGopher = fHighlight;}
	//void  SetHighlightTelnet(BOOL fHighlight) {m_fHighlightTelnet = fHighlight;}

	int Load();
	int Save();

protected:
	void read();
	void write();
	void default_values();

private:
	LONG        m_fMailUseRegistry;
	LONG        m_fWebUseRegistry;
	LONG        m_fFtpUseRegistry;
//	LONG        m_fGopherUseRegistry;
//	LONG        m_fTelnetUseRegistry;

	COLORREF    m_hotlinkColor;
	LONG        m_fUnderlineLinks;

	CString     m_webPattern;
	CString     m_ftpPattern;
//	CString     m_gopherPattern;
//	CString     m_telnetPattern;
	CString     m_mailToPattern;
	CString     m_newsPattern;

	LONG        m_fHighlightNews;
	LONG        m_fHighlightMail;
	LONG        m_fHighlightWeb;
	LONG        m_fHighlightFtp;
//	LONG        m_fHighlightGopher;
//	LONG        m_fHighlightTelnet;
};
