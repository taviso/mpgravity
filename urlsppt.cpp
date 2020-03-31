/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: urlsppt.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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
/*  Revision 1.6  2009/02/17 14:28:23  richard_wood
/*  Deleted old commented out code.
/*
/*  Revision 1.5  2009/02/15 18:05:31  richard_wood
/*  Build 2.7.1b.10. Fixed SF bugs:
/*    2603050 Problem when adjusting columns in Thread View
/*    2551581 Invalid Argument Error using Wrap Lines
/*    2546368 HTTPS link recognition
/*    2538282 Twisted column widths when thread windows is maximized
/*    2533528 Missing Separators in Thread Pane Headings
/*    1219993 Clicking second URL on a line does nothing
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:52:24  richard_wood
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

// urlsppt.cpp : Support for URL recognition and execution
//

#include "stdafx.h"
#include "news.h"
#include "tglobopt.h"
#include "urlsppt.h"
#include "turldde.h"
#include <ddeml.h>
#include "mplib.h"
#include "topenurl.h"
#include "rgurl.h"
#include "globals.h"
#include "posttmpl.h"
#include "rxsearch.h"
#include "newsdoc.h"       // HandleNewsUrl
#include "DlgQueryMsgID.h"

#ifdef NOT_WORKING_YET
//#include "ml2tmpl.h"
#endif

#include "custmsg.h"
#include "tnews3md.h"
#include "utlmacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// suppress warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

static int ParseExecuteString (LPTSTR pCommand, CString * pstrOutput);
void HandleMailTo(LPCTSTR address);
void HttpExec(LPCTSTR parsed, HWND hWnd);
void FtpExec(LPCTSTR parsed, HWND hWnd);
//void GopherExec (LPCTSTR parsed, HWND hWnd);
void MailExec(LPCTSTR parsed, HWND hWnd);
void NewsExec(LPCTSTR parsed, HWND hWnd);
//void TelnetExec (LPCTSTR parsed, HWND hWnd);
void AppExec(const CString & application, LPCTSTR url);

extern TGlobalOptions *gpGlobalOptions;
extern DWORD gidDdeServerInst;
extern HSZ ghszServerName;
extern HSZ ghszTopic;

HSZ hszService = 0;
HSZ hszTopic   = 0;
HSZ hszItem    = 0;

RxSearch urlSearch;     // compiled regular expression for URL searching

#define  SUPPORT_HTTP   0
#define  SUPPORT_FTP    1
#define  SUPPORT_NEWS   2
#define  SUPPORT_MAILTO 3
//#define  SUPPORT_GOPHER 4
//#define  SUPPORT_TELNET 5
#define NUM_SUPPORTED_URLS 4

typedef void (*pfURLExec) (LPCTSTR  parsed, HWND hWnd);

struct SupportedURL
{
	pfURLExec   pfExec;
	BOOL        fHighlight;
	RxSearch    *pSearch;
	CString     m_strOrigPattern;

	void CompilePattern (const CString & strPat)
	{
		m_strOrigPattern = strPat;
		pSearch->Compile (strPat);
	}

	SupportedURL()
	{
		pSearch = 0;
		fHighlight = TRUE;
		pfExec = NULL;
	} 

	~SupportedURL()
	{
		if (pSearch)
			delete pSearch;
	}
};

SupportedURL supportedURLs[NUM_SUPPORTED_URLS];

URLKiller  freeTableMem;

URLKiller::URLKiller()
{
	supportedURLs[SUPPORT_HTTP].pfExec = HttpExec;
	supportedURLs[SUPPORT_FTP].pfExec = FtpExec;
	supportedURLs[SUPPORT_NEWS].pfExec = NewsExec;
	supportedURLs[SUPPORT_MAILTO].pfExec = MailExec;
	//supportedURLs[SUPPORT_GOPHER].pfExec = GopherExec;
	//supportedURLs[SUPPORT_TELNET].pfExec = TelnetExec;
}

URLKiller::~URLKiller()
{
}

/////////////////////////////////////////////////////////////////////////////
// Function for launching URLs that have registry associations.
/////////////////////////////////////////////////////////////////////////////
BOOL LaunchURL (LPCTSTR url)
{
	SHELLEXECUTEINFO  shellParms;

	ZeroMemory (&shellParms, sizeof (shellParms));
	shellParms.cbSize       = sizeof (shellParms);
	shellParms.fMask        = 0;
	shellParms.hwnd         = GetDesktopWindow ();
	shellParms.lpVerb       = "Open";
	shellParms.lpFile       = url;
	shellParms.lpParameters = url;
	shellParms.lpDirectory  = "";
	shellParms.nShow        = SW_SHOWNORMAL;

	if (!ShellExecuteEx (&shellParms))
	{
		CString  error;
		CString str; str.LoadString (IDS_ERR_LAUNCH_URL);
		error.Format (str, shellParms.hInstApp);
		AfxMessageBox (error);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CompileURLExpression - Compile new expression for searching URLs.  This
//                        is called once in InitInstance after the global
//                        settings have been loaded, and if the user changed
//                        the global settings through the property sheet.
//
//  8-07-97  amc  Pass in pOptions explicitly.  It's more readable in the
//                dreaded InitInstance.
/////////////////////////////////////////////////////////////////////////////
void CompileURLExpression(TGlobalOptions* pOptions)
{
	CString        pattern;
	TURLSettings   *pSettings = pOptions->GetURLSettings();
	BOOL           fDoneFirst = FALSE;

	// remove the regular expressions
	for (int i = 0; i < ELEM(supportedURLs); i++)
	{
		if (supportedURLs[i].pSearch)
		{
			delete supportedURLs[i].pSearch;
			supportedURLs[i].pSearch = 0;
		}
		supportedURLs[i].pSearch = new RxSearch;
	}

	supportedURLs[SUPPORT_HTTP].CompilePattern (pSettings->GetWebPattern());
	supportedURLs[SUPPORT_FTP].CompilePattern (pSettings->GetFtpPattern());
	supportedURLs[SUPPORT_NEWS].CompilePattern (pSettings->GetNewsPattern());
	supportedURLs[SUPPORT_MAILTO].CompilePattern (pSettings->GetMailToPattern());
//	supportedURLs[SUPPORT_GOPHER].CompilePattern (pSettings->GetGopherPattern());
//	supportedURLs[SUPPORT_TELNET].CompilePattern (pSettings->GetTelnetPattern());

	if (!AnyURLHighlighting())
		return;

	if (pSettings->HighlightWeb())
	{
		pattern = '(' + pSettings->GetWebPattern() + ')';
		fDoneFirst = TRUE;
	}

	if (pSettings->HighlightFtp())
	{
		if (fDoneFirst)
			pattern += '|';
		pattern += '(' + pSettings->GetFtpPattern() + ')';
	}

	//if (pSettings->HighlightGopher())
	//{
	//	if (fDoneFirst)
	//		pattern += '|';
	//	pattern += '(' + pSettings->GetGopherPattern() + ')';
	//}

	//if (pSettings->HighlightTelnet())
	//{
	//	if (fDoneFirst)
	//		pattern += '|';
	//	pattern += '(' + pSettings->GetTelnetPattern() + ')';
	//}

	if (pSettings->HighlightMail())
	{
		if (fDoneFirst)
			pattern += '|';
		pattern += '(' + pSettings->GetMailToPattern() + ')';
	}

	if (pSettings->HighlightNews())
	{
		if (fDoneFirst)
			pattern += '|';
		pattern += '(' + pSettings->GetNewsPattern() + ')';
	}

	urlSearch.Compile(pattern);
}

BOOL AnyURLHighlighting()
{
	TURLSettings   *pSettings = gpGlobalOptions->GetURLSettings();
	return (pSettings->HighlightWeb()     ||
		pSettings->HighlightFtp()     ||
		//pSettings->HighlightGopher () ||
		//pSettings->HighlightTelnet () ||
		pSettings->HighlightMail ()   ||
		pSettings->HighlightNews() );
}

/////////////////////////////////////////////////////////////////////////////
// IsURL - High level function to tell whether a particular string is a URL
/////////////////////////////////////////////////////////////////////////////
int IsURL(LPCTSTR urlString)
{
	int len;
	for (int i = 0; i < ELEM(supportedURLs); i++)
	{
		if (supportedURLs[i].pSearch->Search(urlString, &len) != NULL)
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Process2URL - High level function to handle the processing of a URL.
//
// Merged patch to parse multiple URLs on single line.
// Patch from Raf Guns (SourceForge ID : rguns)
//
/////////////////////////////////////////////////////////////////////////////
void Process2URL (int ich, LPCTSTR pLine, HWND hWnd)
{
	int      i;
	int      iLen;
	int      iLenParsed = 0; // Length of current line already parsed
	LPCTSTR  pFound;

	for (i = 0; i < ELEM(supportedURLs) ; i++)
	{
		SupportedURL *pURL = &supportedURLs[i];

		TRACE("Checking %s %d\r\n", LPCTSTR(pURL->m_strOrigPattern), i);

		while ((pFound = pURL->pSearch->Search (pLine, &iLen)) != NULL)
		{
			// Start and end of current URL
			int start = pFound - pLine + iLenParsed;
			int end   = start + iLen + iLenParsed;

			if ((ich >= start) && (ich <= end))
			{
				CString parsed;
				LPTSTR p = parsed.GetBuffer(iLen);
				_tcsncpy (p, pFound, iLen);
				parsed.ReleaseBuffer(iLen);

				gptrApp -> BeginWaitCursor ();
				try
				{
					pURL->pfExec (parsed, hWnd);
				}
				catch(...)
				{
					gptrApp -> EndWaitCursor ();
					throw;
				}
				gptrApp -> EndWaitCursor ();
				return;
			}
			pLine      += end;
			iLenParsed += iLen;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// AppExec - Used for non-DDE URL support.  Basically performs URL
//           substitution in the into the application command line and
//           calls CreateProcess to execute the helper app.
/////////////////////////////////////////////////////////////////////////////
void AppExec (const CString & application, LPCTSTR url)
{
	CString  app = application;
	CString  formatted;
	int      loc;

	if (!app.GetLength())
	{
		// pop up property sheet to allow definition ?????
		return;
	}

	// if there is a %1, then we need to substitute the parsed
	// html for the %1, otherwise append it
	if ((loc = app.Find ("%1")) >= 0)
	{
		// substitute so we can use format
		app.SetAt (loc+1, 's');
		formatted.Format (app, url);
	}
	else
	{
		app += " ";
		formatted = app + url;
	}

	STARTUPINFO    startup;
	ZeroMemory (&startup, sizeof (startup));
	startup.cb = sizeof (startup);
	startup.dwFlags = STARTF_USESHOWWINDOW;
	startup.wShowWindow = SW_SHOWNORMAL;

	PROCESS_INFORMATION  procInfo;

	CreateProcess (NULL,
		(char *) LPCTSTR(formatted),
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&startup,
		&procInfo);
}

/////////////////////////////////////////////////////////////////////////////
// DdeCallback - Required callback for use with the DDEML.
/////////////////////////////////////////////////////////////////////////////
HDDEDATA CALLBACK DdeCallback(
							  UINT     uType,         // transaction type
							  UINT     uFmt,          // clipboard data format
							  HCONV    hConv,         // handle of conversation
							  HSZ      hsz1,          // handle of string
							  HSZ      hsz2,          // handle of string
							  HDDEDATA hdata,         // handle of global memory object
							  DWORD    dwData1,       // transaction-specific data
							  DWORD    dwData2        // transaction-specific data
							  )
{
	// Basically, we're just gonna report errors, we don't
	// really Try to maintain any real illusion of interactivity
	TRACE ("DDE txt utype = %u\n", uType);
	switch (uType)
	{
	case XTYP_CONNECT:
		// this stuff is to handle the news tag registry association

		// if a client request connection withmy server Name and Topic
		// grant connection.
		if (ghszServerName == hsz2 && ghszTopic == hsz1)
			return (HDDEDATA)TRUE;
		return (0);

	case XTYP_EXECUTE :
		{
			// query the size
			DWORD dwSize = DdeGetData (hdata, (LPBYTE)NULL, 0, 0L);
			LPTSTR pExecute = new TCHAR[dwSize];

			// retrieves the execution string
			DdeGetData (hdata, (LPBYTE)pExecute, dwSize, 0L);
			TRACE1("Execute $%s$\n", pExecute);

			HDDEDATA ret = (HDDEDATA) DDE_FNOTPROCESSED;
			CString * pstrExec = new CString;
			if (0 == ParseExecuteString (pExecute, pstrExec))
			{
				CString str; str.Format("Execute PARAMETER IS $%s$\n", (LPCTSTR)(*pstrExec));
				AfxMessageBox(str);

				if (gptrApp -> GetGlobalNewsDoc())
					gptrApp -> GetGlobalNewsDoc() -> HandleNewsUrl ( pstrExec );

				ret = (HDDEDATA) DDE_FACK;
			}
			delete [] pExecute;
			delete pstrExec;

			DdeFreeDataHandle ( hdata );
			return ret;
		}

		break;

	case XTYP_REGISTER:
		TRACE("DDE Register\n");
		return (HDDEDATA) NULL;
		break;

	case XTYP_UNREGISTER:
		TRACE("DDE Unregister\n");
		return (HDDEDATA) NULL;
		break;

	case XTYP_ADVDATA:
		TRACE ("DDE Advise data!!!\n");
		return (HDDEDATA) DDE_FACK;
		break;

	case XTYP_XACT_COMPLETE:
		// disconnect ...
		if (FALSE == DdeDisconnect(hConv))
		{
			CString str; str.LoadString (IDS_ERR_DISCONNECT);
			throw(new TUrlException (str, kError));
		}
		if (hszService)
			DdeFreeStringHandle (gidDdeServerInst, hszService);
		if (hszTopic)
			DdeFreeStringHandle (gidDdeServerInst, hszTopic);
		if (hszItem)
			DdeFreeStringHandle (gidDdeServerInst, hszItem);
		hszService = 0;
		hszTopic = 0;
		hszItem = 0;
		hConv = 0;
		return (HDDEDATA) NULL;
		break;

	case XTYP_ERROR:
		TRACE0("DDEML Error encountered in Callback function\n");
		return (0);

	case XTYP_DISCONNECT:
		TRACE ("DDE Disconnect\n");
		return (HDDEDATA) NULL;
		break;

	default:
		TRACE ("Default handler in DDE callback\n");
		return (HDDEDATA) NULL;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// ProcessDdeUrl - This function takes a TUrlDde and tries to contact the
//                 application, establish the transaction, and do whatever
//                 the user specified in the dialog.  It does everything
//                 that is necessary to initialize DDE, shutdown, etc...
/////////////////////////////////////////////////////////////////////////////
void ProcessDdeUrl (TUrlDde * pURL, LPCTSTR urlString)
{
	// initialize dde
	HCONV       hConv = 0;

	try
	{
		// initialize DDE if it hasn't been
		if (!gidDdeServerInst)
			if (DMLERR_NO_ERROR !=
				DdeInitialize(&gidDdeServerInst,                   /* receives instance identifier */
				(PFNCALLBACK) DdeCallback,  /* address of callback function */
				APPCMD_CLIENTONLY,          /* filter notifications         */
				0))
			{
				CString str; str.LoadString (IDS_ERR_DDE_INIT);
				throw(new TUrlException (str, kWarning));
			}

			// Try to connect to the server

			if (!hszService)
				hszService = DdeCreateStringHandle (gidDdeServerInst,
				pURL->GetService(),
				CP_WINANSI);

			if (0 == hszService)
			{
				CString str; str.LoadString (IDS_ERR_STRING_HANDLE);
				throw(new TUrlException (str, kError));
			}

			if (!hszTopic)
				hszTopic = DdeCreateStringHandle (gidDdeServerInst,
				pURL->GetTopic(),
				CP_WINANSI);

			if (0 == hszTopic)
			{
				CString str; str.LoadString (IDS_ERR_STRING_HANDLE);
				throw(new TUrlException (str, kError));
			}

			// ???? should we set up conversation context structure,
			//      or is NULL really okay?

			CONVCONTEXT context;

			ZeroMemory (&context, sizeof (context));
			context.cb = sizeof (context);
			context.iCodePage = CP_WINANSI;

			hConv = DdeConnect (gidDdeServerInst, hszService, hszTopic, &context);

			// the DDE server is not in memory ?
			if (0 == hConv)
			{
				UINT  lastError = DdeGetLastError (gidDdeServerInst);
				if (lastError == DMLERR_NO_CONV_ESTABLISHED)
				{
					// Try to start the app... ??? this could be factored
					STARTUPINFO startup;
					ZeroMemory (&startup, sizeof (startup));
					startup.cb = sizeof (startup);
					CString  app = pURL->GetApplication ();

					PROCESS_INFORMATION  procInfo;

					if (TRUE != CreateProcess (NULL,
						(char *) LPCTSTR(app),
						NULL,
						NULL,
						FALSE,
						CREATE_NEW_PROCESS_GROUP |
						NORMAL_PRIORITY_CLASS,
						NULL,
						NULL,
						&startup,
						&procInfo))
					{
						CString str; str.LoadString (IDS_ERR_LAUNCH_DDE);
						throw(new TUrlException (str, kError));
					}

					// Try to re-establish the conversation ...
					WaitForInputIdle (procInfo.hProcess, 5000);

					hConv = DdeConnect (gidDdeServerInst, hszService, hszTopic, &context);
					if (0 == hConv)
					{
						CString str; str.LoadString (IDS_ERR_DDE_EST);
						throw(new TUrlException (str, kError));
					}

				}
				else
				{
					// something we don't know how to deal with
					CString str; str.LoadString (IDS_ERR_DDE_CONNECT);
					throw(new TUrlException (str, kError));
				}
			}

			CString  item;
			CString  data;
			CString  temp;

			item = pURL->GetItem ();

			// replace all occurences of %1 with the real URL in both
			// the item and the data

			int pos;

			while ((pos = item.Find ("%1")) >= 0)
			{
				temp = item;
				temp.SetAt (pos + 1, 's');
				item.Format (temp, urlString);
			}

			data = pURL->GetData ();

			// replace all occurences of %1 with the real URL in both
			// the item and the data, should we put quotes around the
			// URL for the user if they aren't there?

			while ((pos = data.Find ("%1")) >= 0)
			{
				temp = data;
				temp.SetAt (pos + 1, 's');
				data.Format (temp, urlString);
			}

			HDDEDATA hReturnData;
			int      cbData;
			LPBYTE   lpbData;
			HGLOBAL  hData;
			cbData = data.GetLength() + 1;
			hData = GlobalAlloc (GMEM_DDESHARE,
				cbData);
			lpbData = (LPBYTE) GlobalLock (hData);
			_tcscpy (LPTSTR (lpbData), data);
			if (!hszItem)
				hszItem = DdeCreateStringHandle (gidDdeServerInst,
				item,
				CP_WINANSI);

			switch (pURL->GetTransactionType())
			{
			case TUrlDde::kPoke:
				hReturnData = DdeClientTransaction (lpbData,
					cbData,
					hConv,
					hszItem,
					CF_TEXT,
					XTYP_POKE,
					TIMEOUT_ASYNC,
					NULL);
				break;
			case TUrlDde::kExecute:
				hReturnData = DdeClientTransaction (lpbData,
					cbData,
					hConv,
					0,
					0,
					XTYP_EXECUTE,
					TIMEOUT_ASYNC,
					NULL);
				break;
			case TUrlDde::kRequest:
				hReturnData = DdeClientTransaction (NULL,
					0,
					hConv,
					hszItem,
					CF_TEXT,
					XTYP_REQUEST,
					TIMEOUT_ASYNC,
					NULL);
				break;
			}

			if (0 == hReturnData)
			{
				UINT  error = DdeGetLastError (gidDdeServerInst);
				CString str; str.LoadString (IDS_ERR_DDE_EXEC);
				throw(new TUrlException (str, kError));
			}

	}
	catch (TException *pE) // we catch most general case and rethrow....
	{
		if (hConv)
			DdeDisconnect (hConv);        // end the conversation

		if (hszService)
		{
			DdeFreeStringHandle (gidDdeServerInst, hszService);
			hszService = 0;
		}

		if (hszTopic)
		{
			DdeFreeStringHandle (gidDdeServerInst, hszTopic);
			hszTopic = 0;
		}

		if (hszItem)
		{
			DdeFreeStringHandle (gidDdeServerInst, hszItem);
			hszItem = 0;
		}

		TException *ex = new TException(*pE);
		pE->Delete();
		throw(ex);
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// HttpExec - Gets configuration information about the Http helper application
//            and then uses that to launch the associated viewer.
/////////////////////////////////////////////////////////////////////////////
void HttpExec (LPCTSTR parsed, HWND hWnd)
{
	TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();
	CString strURL(parsed);

	// either we're using registry associations or a custom definition
	if (pURLSettings->WebUsingRegistry() && (gdwOSMajor >= 4))
	{
		// If the URL doesn't start with http then add it
		if (strURL.Find("http") == -1)
			strURL = "http://" + strURL;
		LaunchURL(strURL);
	}
	else
	{
		TUrlDde  *pURL = gpGlobalOptions->GetWebHelperPointer();
		if (pURL->UseDDE ())
			ProcessDdeUrl (pURL, parsed);
		else
			AppExec(pURL->GetApplication (), parsed);
	}
}

/////////////////////////////////////////////////////////////////////////////
// FtpExec - Gets configuration information about the Ftp helper application
//         and then uses that to launch the associated viewer.
/////////////////////////////////////////////////////////////////////////////
void FtpExec (LPCTSTR parsed, HWND hWnd)
{
	TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();

	if (pURLSettings->FtpUsingRegistry() && (gdwOSMajor >= 4))
	{
		LaunchURL (parsed);
	}
	else
	{
		TUrlDde *pURL = gpGlobalOptions->GetFtpHelperPointer ();
		if (pURL->UseDDE ())
			ProcessDdeUrl (pURL, parsed);
		else
			AppExec(pURL->GetApplication (), parsed);
	}
}

// RLW - Gopher and Telnet removed 16/6/09
///////////////////////////////////////////////////////////////////////////////
//// GopherExec - Gets configuration information about the Gopher helper application
////              and then uses that to launch the associated viewer.
///////////////////////////////////////////////////////////////////////////////
//void GopherExec (LPCTSTR parsed, HWND hWnd)
//{
//	TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();
//
//	if (pURLSettings->GopherUsingRegistry() && gdwOSMajor >= 4)
//	{
//		LaunchURL (parsed);
//	}
//	else
//	{
//		TUrlDde *pURL = gpGlobalOptions->GetFtpHelperPointer ();
//		if (pURL->UseDDE ())
//			ProcessDdeUrl (pURL, parsed);
//		else
//			AppExec(pURL->GetApplication (), parsed);
//	}
//}
//
///////////////////////////////////////////////////////////////////////////////
//// TelnetExec - Gets configuration information about the Telnet helper application
////              and then uses that to launch the associated viewer.
///////////////////////////////////////////////////////////////////////////////
//void TelnetExec (LPCTSTR parsed, HWND hWnd)
//{
//	TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();
//
//	if (pURLSettings->TelnetUsingRegistry()  && gdwOSMajor >= 4)
//	{
//		LaunchURL (parsed);
//	}
//	else
//	{
//		TUrlDde *pURL = gpGlobalOptions->GetFtpHelperPointer ();
//		if (pURL->UseDDE ())
//			ProcessDdeUrl (pURL, parsed);
//		else
//			AppExec(pURL->GetApplication (), parsed);
//	}
//}

/////////////////////////////////////////////////////////////////////////////
// MailExec - Gets configuration information about the Mail helper application
//            and then uses that to launch the associated viewer.
/////////////////////////////////////////////////////////////////////////////
void MailExec (LPCTSTR parsed, HWND hWnd)
{
	/// these mail url matches are of a generic nature usually  a111@xxx.yyy
	/// so they can really be either an email address or a msg-id
	//  Check state of ctrl key. (forces treatment as msg-id, fwded to google

	CDlgQueryMsgID sDlg(CWnd::FromHandle(hWnd));

	if (sDlg.DoModal() == IDCANCEL)
	{
		return;
	}

	if (sDlg.m_fGoogle)
	{
		NewsExec (parsed, hWnd);
		return;
	}

	TURLSettings *pURLSettings = gpGlobalOptions->GetURLSettings();

	if (pURLSettings->MailUsingRegistry() && gdwOSMajor >= 4)
	{
		CString  temp = parsed;
		CString  url;

		temp.MakeUpper();
		if (temp.Find("MAILTO") == 0)
			url = parsed;
		else
		{
			url = "mailto:";
			url += parsed;
		}
		LaunchURL (url);
	}
	else
	{
		HandleMailTo(parsed);
	}
}

/////////////////////////////////////////////////////////////////////////////
// HandleMailTo - Pop up a new mail message with us as the from and
//                the double-clicked user as the recipient.
/////////////////////////////////////////////////////////////////////////////
void HandleMailTo (LPCTSTR address)
{
	// send a message off to the newsview to go to the article
	CMDIFrameWnd *pMainWnd = (CMDIFrameWnd *) AfxGetMainWnd ();
	TNews3MDIChildWnd *pChildWnd  = (TNews3MDIChildWnd *) pMainWnd->MDIGetActive();

	pChildWnd->SendMessage (WMU_NEWSVIEW_PROCESS_MAILTO, 0, LPARAM(address));
}

// ------------------------------------------------------------------------
// Removes blank spaces from a given string.
void URL_SkipWhiteSpace (LPTSTR pIn)
{
	TCHAR * pNewString;
	int i, j;
	BOOL fInsideParen = FALSE;

	// allocate temporary string
	pNewString = new TCHAR[lstrlen(pIn) + 1];
	i = j = 0;

	while ('\0' != pIn[i])
	{
		// skip character if it is blank, but keep all characters
		//   within the parens

		TCHAR c = pIn[i];
		if ( '(' == c )
			fInsideParen = TRUE;
		else if ( ')' == c)
			fInsideParen = FALSE;

		if (fInsideParen || ' ' != pIn[i])
			pNewString[j++] = pIn[i];
		++i;
	}
	pNewString[j] = '\0';

	// copy it back
	lstrcpy (pIn, pNewString);

	// free temporary
	delete [] pNewString;
}

// ------------------------------------------------------------------------
// Purpose: extract the command, and the parameters passed with it
//
// Syntax: "[<action> (<param>)]"
//
// Supported Actions:  open
//
static int ParseExecuteString (LPTSTR pCommand, CString * pstrOutput)
{
	int iRet = -1;
	LPTSTR pCmd = pCommand;

	// check validity
	if (NULL == pCmd)
		return -1;

	URL_SkipWhiteSpace ( pCmd );

	// very first character must be [
	if (*pCmd != '[')
		return -1;

	pCmd = CharNext(pCmd);
	if (_strnicmp(pCmd, "open", 4) == 0)
	{
		iRet = 0;
		pCmd += 4 * sizeof(TCHAR);
	}
	if (iRet != 0)
		return -1;

	// skip open paren
	CString str = pCmd + 1;

	int idx = str.Find ( ")" );
	if (-1 == idx)
		return -1;

	// next char should be ]
	if (str.GetLength() <= idx+1 || str[idx+1] != ']')
		return -1;

	// return goodies in 'pOutput'
	*pstrOutput = str.SpanExcluding ( ")" );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// NewsExec - Gets configuration information about the News helper application
//            and then uses that to launch the associated viewer.
/////////////////////////////////////////////////////////////////////////////
void NewsExec (LPCTSTR parsed, HWND hWnd)
{
	CString strLower = parsed;
	strLower.MakeLower();

	CString strSearch;

	// hack off  "news:"
	if (strLower.Find("news:") == 0)
	{
		strSearch = parsed;
		strSearch = strSearch.Mid (5);
	}
	else
		strSearch = parsed;

	// trim off any elipsis
	while (strSearch.GetLength() > 1  &&  strSearch[strSearch.GetLength() - 1] == '.')
		strSearch = strSearch.Left(strSearch.GetLength() - 1);

	int ln = strSearch.GetLength();

	if (ln >= 2)
	{
		if ('<' == strSearch[0] && '>' == strSearch[ln-1])
			strSearch = strSearch.Mid (1, ln - 2);
	}

	CString strGoogle =
		"http://groups.google.com/groups?safe=off&ie=UTF-8&oe=UTF-8&as_umsgid=";

	strGoogle += strSearch;

	LaunchURL (strGoogle);
}
