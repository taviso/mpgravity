/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutljob.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
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
/*  Revision 1.3  2008/09/19 14:52:21  richard_wood
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

// tutljob.cpp -- utility-thread job

#include "stdafx.h"              // standard stuff
#include "tutljob.h"             // this file's prototypes
#include "names.h"               // FORMAT_BODIES
#include "ruleutil.h"            // SaveToFileAction()
#include "globals.h"             // gpStore
#include "resource.h"            // IDS_*
#include "fetchart.h"            // BlockingFetchBody()
#include "tdecthrd.h"            // gpsDecodeDialog
#include "warndlg.h"             // WarnWithCBX()
#include "tglobopt.h"            // WarnOnErrorDuringDecode()
#include "nglist.h"              // TNewsGroupUseLock
#include "server.h"              // TNewsServer
#include "genutil.h"             // MsgResource()
#include "newsdb.h"              // we need gpStore->GetServerByName()
#include "tutlmsg.h"             // ID_CURRENT_CHANGE
#include "rgswit.h"              // TRegSwitch
#include "fileutil.h"
#include "tdecutil.h"            // EnsureDirExists

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TGlobalOptions *gpGlobalOptions;

extern  HWND fnAccessSafeFrameHwnd(BOOL fLock);

int TUtilityJob::s_iFetchStatus;
TArticleText *TUtilityJob::s_pFetchText;

// these are initailized by the Janitor object
HANDLE TUtilityJob::s_hFetchDone ;
HANDLE TUtilityJob::s_hMutex ;
TUtilityJobJanitor TUtilityJob::s_Janitor;  // calls Close on s_hMutex

int TUtilityJob::s_iIDCounter = 0;  // produces unique IDs

// wait 2 minutes for a lock-group operation
#define UTL_GROUP_LOCK_WAIT_TIME (120 * 1000)

TDecodeCache  gsTheDecodeCache;

TDecodeCache*  gpDecodeCache = &gsTheDecodeCache;

// -------------------------------------------------------------------------
TUtilityJob::TUtilityJob (BYTE bVersion) : PObject (bVersion)
{
	m_iID = s_iIDCounter++;
}

// -------------------------------------------------------------------------
TUtilityJob::TUtilityJob (BYTE bVersion, const TUtilityJob *pCopy)
: PObject (bVersion)
{
	m_iID = s_iIDCounter++;
	*this = *pCopy;
}

// -------------------------------------------------------------------------
// TUtilityJob -- constructor.  CString objects are passed in to take advantage
// of MFC reference counting
TUtilityJob::TUtilityJob (BYTE bVersion, LONG lGroupID,
						  const CString & rstrGroupName, const CString & rstrGroupNickname,
						  TArticleHeader *psHdr, int iRuleBased)
						  : PObject (bVersion),
						  m_strGroupName(rstrGroupName), m_strGroupNickname(rstrGroupNickname)
{
	m_iID = s_iIDCounter++;
	m_strSubject = psHdr -> GetSubject ();
	m_lGroupID = lGroupID;
	m_sHdr = *psHdr;
	m_iRuleBased = iRuleBased;
}

// -------------------------------------------------------------------------
TUtilityJob &TUtilityJob::operator= (const TUtilityJob &src)
{
	m_strPrefix = src.m_strPrefix;
	m_lGroupID = src.m_lGroupID;
	m_strGroupName = src.m_strGroupName;
	m_strGroupNickname = src.m_strGroupNickname;
	m_sHdr = src.m_sHdr;
	m_strStatus = src.m_strStatus;
	m_iRuleBased = src.m_iRuleBased;
	m_strJobType = src.m_strJobType;
	m_strSubject = src.m_strSubject;
	return *this;
}

// -------------------------------------------------------------------------
// WriteArticleToFile -- writes the article specified by this print job to a
// specific temporary file.  Returns 0 for success and non-0 for failure
int TUtilityJob::WriteArticleToFile (char * pchTempFile, HANDLE hKillEvent)
{
	TArticleText *pText = NULL;
	TError sErrorRet;
	EArticleSource eArticleSource = SRC_FROM_DB;

	// get the article's text
	pText = new TArticleText;
	if (!pText)
	{
		SetStatus (IDS_ERR_OUT_OF_MEMORY, ERROR_STATUS);
		return 1;
	}

	if (PlsFetchArticle (sErrorRet, eArticleSource, &m_sHdr, pText, false, hKillEvent))
	{
		// status already set by FetchArticle()
		delete pText;
		return 1;
	}

	SetStatus (IDS_UTIL_SAVING);

	// write to the file
	CString strFileName;
	strFileName = pchTempFile;
	TRegSwitch *pSwitches = gpGlobalOptions -> GetRegSwitch ();
	SaveToFileAction (&m_sHdr, pText, strFileName, FALSE /* bSeparator */,
		pSwitches -> m_fPrintFullHeader);

	delete pText;
	return 0;
}

// -------------------------------------------------------------------------
// PlsFetchArticle -- fetches an article's text from the database, or the
// newsfeed if it is not in the database.
//
// Returns 0 for success, 1 otherwise
//
int TUtilityJob::PlsFetchArticle (TError        &  sErrorRet,
								  EArticleSource & eArticleSource,
								  TArticleHeader * pHdr,
								  TArticleText * & pText,
								  bool             fUseCache,
								  HANDLE           hKillEvent,
								  int iThis /* = 0 */,
								  int iTotal /* = 0 */)
{
	TServerCountedPtr cpNewsServer;  // smart pointer
	CString strStatus;

	if (iTotal)
		strStatus.Format (_T("(%d/%d) "), iThis, iTotal);
	SetStatus (strStatus, IDS_UTIL_FETCH_DB);

	int iArtInt = pHdr -> HashKey ();
	bool fGotBody = false;

	{
		BOOL fUseLock;
		TNewsGroup *psNG;
		TNewsGroupUseLock useLock(cpNewsServer, m_lGroupID, &fUseLock, psNG);

		if (fUseLock)
		{
			if (psNG -> TextRangeHave (iArtInt))
			{
				psNG -> Open ();
				try
				{
					psNG -> LoadBody (iArtInt, pText);
					fGotBody = true;
				}
				catch (TGroupDBErrors *pE)
				{
					pE->Delete();
				}
				catch (TException *pE)
				{
					pE->Delete();
				}
				catch (CException *e)
				{
					e -> Delete ();
				}
				psNG -> Close ();
			}
		}
		if (fGotBody)
		{
			eArticleSource = SRC_FROM_DB;
			return 0;
		}
	}

	if (fUseCache)
	{
		// 2nd Try -- Try to load from cache
		if (0 == gpDecodeCache->Load ( pHdr, pText ) )
		{
			eArticleSource = SRC_FROM_CACHE;
			return 0;
		}
	}

	// 3rd Try -- couldn't get text from database... Try getting from the newsfeed
	if (iTotal)
		strStatus.Format (_T("(%d/%d) "), iThis, iTotal);
	SetStatus (strStatus, IDS_UTIL_FETCH_NF);
	TArticleText *pPumpText = NULL;
	int RC;

	// pass in hKillEvent, so we can return when someone wants to
	//    terminate the decode / print thread

	CPoint ptPartID(iThis, iTotal);
	CString subject = pHdr->GetSubject();

	if ((RC = BlockingFetchBody (sErrorRet,
		m_strGroupName,
		m_lGroupID,
		subject,
		ptPartID,
		(int) pHdr -> GetNumber (),
		pHdr -> GetLines (),
		pPumpText,
		FALSE,         // fPrioPump
		hKillEvent,

		FALSE,         // bMarkAsRead
		FALSE          // bEngage
		)) != 0)
	{
		EErrorClass eClass ;  sErrorRet.GetClass ( eClass );

		CString desc;
		DWORD   dwErr  =  sErrorRet.GetDWError ( );

		if (dwErr)
			desc.Format (_T("Error %d"), dwErr);
		else
		{
			sErrorRet.GetDescription ( desc );

			if (desc.IsEmpty())
				desc = _T("Unknown error");
		}

		switch (eClass)
		{
		case kClassNNTP:
			if (411 == dwErr)
			{
				desc = "Invalid newsgroup";
				SetStatus ( desc, 0, ERROR_STATUS );
			}
			else if (dwErr > 420)
				SetStatus (IDS_ERR_UTIL_ART_NOTFOUND, ERROR_STATUS);
			else
				SetStatus ( desc, 0, ERROR_STATUS );
			break;

		case kClassWinsock:
			// error fetching text from newsfeed
			SetStatus (IDS_ERR_UTIL_FETCH_NF, ERROR_STATUS);
			break;

		case kClassUser:           // Whatever
		case kClassUnknown:
		case kClassInternal:
		case kClassResource:
		case kClassExternal:

			SetStatus ( desc, 0, ERROR_STATUS );
			break;

		default:
			// error fetching text from newsfeed
			SetStatus (IDS_ERR_UTIL_FETCH_NF, ERROR_STATUS);
			break;

			//case PUMP_NOT_CONNECTED:
			//   SetStatus (IDS_ERR_UTIL_NO_CONN, ERROR_STATUS);
			//   break;
		}	
		return 1;
	}

	// BlockingFetchBody() worked, so delete the text we were given, and
	// return the text given by BlockingFetchBody()
	delete pText;
	pText = pPumpText;

	if (fUseCache)
		gpDecodeCache->Save ( pHdr, pText );

	eArticleSource = SRC_FROM_NET;

	return 0;
}

// -------------------------------------------------------------------------
// SetStatus -- sets the job's status string, and optionally prints an error
// message
void TUtilityJob::SetStatus (int iResourceID,
							 StatusCode iCode /* = NORMAL_STATUS */)
{
	SetStatus (_T(""), iResourceID, iCode);
}

void TUtilityJob::SetStatus (LPCTSTR pchStatus, int iResourceID,
							 StatusCode iCode /* = NORMAL_STATUS */)
{
	m_strStatus = pchStatus;
	CString strResource;
	if (iResourceID)
		strResource.LoadString (iResourceID);
	m_strStatus += strResource;

	CString str1; str1.LoadString (IDS_ERR_DECODE_CANCELLED);
	CString str2; str2.LoadString (IDS_ERR_UTIL_NO_CONN);

	if (iCode == ERROR_STATUS &&
		FALSE == m_iRuleBased &&
		gpGlobalOptions -> WarnOnErrorDuringDecode () &&
		// certain messages we don't want to show...
		m_strStatus.CompareNoCase (str1) &&
		m_strStatus.CompareNoCase (str2))
	{

		// special case for IDS_ERR_DECODE_SUBJECT since it has a long message
		CString strError;
		CString strSubject = Subject ();
		if (strSubject.GetLength () > 20)
			strSubject = strSubject.Left (20) + "...";

		if (iResourceID == IDS_ERR_DECODE_SUBJECT)
		{
			CString str; str.LoadString (IDS_SUBJECT_WAS);
			strError = m_strJobType + ": " + m_strStatus +
				". " + str + " \"" + strSubject + "\".";
		}
		else
		{
			CString str; str.LoadString (IDS_PROCESSING_ARTICLE);
			strError = m_strJobType + ": " + str + " \"" +
				strSubject + "\".  " + m_strStatus + ".";
		}

		HWND hWndMF = fnAccessSafeFrameHwnd(TRUE /* fLock */);

		if (hWndMF)
		{
			CWnd *pParent = gpsDecodeDialog ? gpsDecodeDialog : CWnd::FromHandle(hWndMF);
			BOOL bDoNotWarn;
			WarnWithCBX (strError, &bDoNotWarn, pParent, TRUE);
			gpGlobalOptions -> SetWarnOnErrorDuringDecode (!bDoNotWarn);
			gpStore -> SaveGlobalOptions ();
		}
		fnAccessSafeFrameHwnd (FALSE); // unlock
	}

	// send a message to the monitor-dialog, if it's present
	PostToMonitorDialog (ID_CURRENT_CHANGE);

	SetSubclassStatus (iResourceID);
}

// -------------------------------------------------------------------------
// PostToMonitorDialog -- posts a message to a dialog that may be monitoring
// this thread
void TUtilityJob::PostToMonitorDialog (int iMessage, WPARAM wParam /* = 0 */,
									   LPARAM lParam /* = 0 */)
{
	LockDialogPtr (TRUE);

	CDialog *pMonitorDlg = MonitorDialog ();
	if (pMonitorDlg && ::IsWindow (pMonitorDlg -> m_hWnd))
		pMonitorDlg -> PostMessage (iMessage, wParam, lParam);

	LockDialogPtr (FALSE);
}

// -------------------------------------------------------------------------
void TUtilityJob::Serialize (CArchive& sArchive)
{
	if (sArchive.IsStoring()) {
		sArchive << s_iIDCounter;  // ugly, I know
		sArchive << m_iID;
		sArchive << m_strPrefix;
		sArchive << m_lGroupID;
		sArchive << m_strGroupName;
		sArchive << m_strStatus;
		sArchive << m_iRuleBased;
		sArchive << m_strJobType;
		m_sHdr.Serialize (sArchive);
	}
	else {
		int iSavedCounter;         // ugly, I know
		sArchive >> iSavedCounter;
		if (iSavedCounter > s_iIDCounter)
			s_iIDCounter = iSavedCounter;

		sArchive >> m_iID;
		sArchive >> m_strPrefix;
		sArchive >> m_lGroupID;
		sArchive >> m_strGroupName;
		sArchive >> m_strStatus;
		sArchive >> m_iRuleBased;
		sArchive >> m_strJobType;
		m_sHdr.Serialize (sArchive);
	}
}

// -------------------------------------------------------------------------
// Serialize -- NEW_UTILITY_JOB_FORMAT_MARKER at the start of the stream marks
// a stream of utility jobs in our "new" format which is versionable
void TUtilityJobs::Serialize (CArchive& sArchive)
{
	CString strCachedName, strCachedNickname;
#define NEW_UTILITY_JOB_FORMAT_MARKER ((UINT) 0xaef83711)

	TUtilityJob *pJob;

	if (sArchive.IsStoring()) {
		sArchive << (UINT) NEW_UTILITY_JOB_FORMAT_MARKER;
		sArchive << (UINT) GetCount ();

		POSITION pos = GetHeadPosition();

		while (pos)
		{
			pJob = (TUtilityJob *) GetNext (pos);
			pJob -> Serialize (sArchive);
		}
	}
	else {
		TNewsServer* pServer = gpStore->GetServerByName (m_strServerName);
		TServerCountedPtr cpNewsServer(pServer);

		UINT uiMarker;
		sArchive >> uiMarker;
		if (uiMarker != NEW_UTILITY_JOB_FORMAT_MARKER) {
			// old format kept the count word first
			if (uiMarker)
				MsgResource (IDS_OLD_FORMAT_DISCARDED);
			return;
		}

		ASSERT(IsEmpty());

		delete_all();

		UINT iCount;
		sArchive >> iCount;
		TUtilityJob *pJob;
		for (UINT i = 0; i < iCount; i++) {
			pJob = CreateJob ();
			pJob -> Serialize (sArchive);

			// use m_strGroupName to compute the nickname
			if (pJob -> m_strGroupName == strCachedName)
				pJob -> m_strGroupNickname = strCachedNickname;
			else {
				strCachedName = pJob -> m_strGroupName;
				BOOL fUseLock;
				TNewsGroup *pNG;
				TNewsGroupUseLock useLock (cpNewsServer, pJob -> m_lGroupID,
					&fUseLock, pNG);
				if (fUseLock)
					strCachedNickname = pNG -> GetNickname ();
				if (strCachedNickname.IsEmpty ())
					strCachedNickname = strCachedName;
				pJob -> m_strGroupNickname = strCachedNickname;
			}

			AddHead (pJob);
		}
	}
}

// -------------------------------------------------------------------------
// destructor
TUtilityJobs::~TUtilityJobs()
{
	try
	{
		delete_all();
	}
	catch(...)
	{
	}
}

// -------------------------------------------------------------------------
// public
void TUtilityJobs::delete_all()
{
	while (FALSE==IsEmpty())
	{
		TUtilityJob * pJob = (TUtilityJob*) RemoveHead();

		delete pJob;
	}
}

// -------------------------------------------------------------------------
LPCTSTR TUtilityJob::Subject ()
{
	// very ugly... need to consolidate m_strSubject with the subject in m_sHdr
	if (!m_strSubject.IsEmpty ())
		return m_strSubject;
	return m_sHdr.GetSubject ();
}

// -------------------------------------------------------------------------
// TUtilityJobJanitor -- use as an embedded member of TUtilityJob.
// Called at startup... initialize mutex handle at Global-Var-init-time.
TUtilityJobJanitor::TUtilityJobJanitor (void)
{
	TUtilityJob::s_hFetchDone = CreateEvent (NULL /* no security */,
		TRUE /* manual reset */, FALSE /* start out unsignaled */,
		NULL /* no name */);

	TUtilityJob::s_hMutex = CreateMutex (NULL, FALSE, NULL);
}

// -------------------------------------------------------------------------
// called at shutdown... deinitialize s_hFetchDone, which is initialized
// at startup
TUtilityJobJanitor::~TUtilityJobJanitor (void)
{
	VERIFY (CloseHandle (TUtilityJob::s_hFetchDone));
	VERIFY (CloseHandle (TUtilityJob::s_hMutex));
}

////////////////////////////////////////////////////////////////////////////

TDecodeCache::TDecodeCache()
{
	CString strDir = TFileUtil::GetAppData()+"\\Gravity\\";
	m_fReady = false;

	m_path.FormPath (strDir, "parts");

		if (0 == EnsureDirExists ( m_path ))
			m_fReady = true;
	}

TDecodeCache::~TDecodeCache()
{
}

// -----------------------------------------------------------
TPath TDecodeCache::GenerateFilename ( TArticleHeader * pHdr )
{
	CString msgid =  pHdr->GetMessageID();

	msgid = TPath::GenLegalFileName( msgid.Left(255) );

	TPath spec (m_path, msgid);

	return spec;
}

// -----------------------------------------------------------
int TDecodeCache::Save (TArticleHeader * pHdr, TArticleText * pText)
{
	if (!m_fReady)
		return 1;

	TPath  spec = GenerateFilename (pHdr);

	CFileStatus rStat;

	CFileException sFE;

	CFile          sFile;

	int ret = 1;
	bool fErase = false;

	if (sFile.Open (spec, CFile::modeCreate | CFile::modeWrite, &sFE))
	{
		{
			CArchive  ar(&sFile, CArchive::store);

			try
			{
				pText->Serialize ( ar );

				ret = 0;
			}
			catch(...)
			{
				ret = 1;
				fErase = true;
			}
			// destruct the archive
		}

		sFile.Close ();
	}

	if (fErase)
	{
		try
		{
			CFile::Remove (spec);
		}
		catch(...)
		{
			// catch all
		}
	}

	return ret;
}

// -----------------------------------------------------------
int TDecodeCache::Load (TArticleHeader * pHdr, TArticleText *& pText)
{
	if (!m_fReady)
		return 1;

	TPath  spec = GenerateFilename (pHdr);

	CFileStatus  rStat;

	if (!CFile::GetStatus (spec, rStat))
		return 1;

	CFile  sFile;
	CFileException sFE;

	TArticleText * pTEXT = new TArticleText;

	if (sFile.Open (spec, CFile::modeRead, &sFE))
	{
		CArchive ar(&sFile, CArchive::load);

		pTEXT->Serialize ( ar );

		delete pText;

		pText = pTEXT;

		return 0;
	}

	delete pTEXT;

	return 1;
}

// --------------------------------------------------------------------------
int TDecodeCache::EraseAll ()
{

	HANDLE            thisDirectory;
	WIN32_FIND_DATA   findData;
	TCHAR             buff[512];
	int               len;

	_tcscpy (buff, (LPCTSTR) m_path);

	len = _tcslen (buff);

	if (buff[len - 1] != '\\')
	{
		buff[len] = '\\';
		buff[len + 1] = 0;
	}

	_tcscat (buff, _T("*.*"));

	thisDirectory = FindFirstFile (buff, &findData);

	if (thisDirectory == INVALID_HANDLE_VALUE)
		return 1;

	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_tcscmp (findData.cFileName, _T(".")) &&
				_tcscmp (findData.cFileName, _T("..")))
			{
				// nothing
			}
		}
		else
		{
			_tcscpy (buff, (LPCTSTR)m_path);
			if (buff[_tcslen (buff) - 1] != '\\')
				_tcscat (buff, _T("\\"));
			_tcscat (buff, findData.cFileName);
			//TRACE ("Removing file %s\n", buff);
			if (!DeleteFile (buff))
			{
				FindClose (thisDirectory);
				return 1;
			}
		}

	}
	while (FindNextFile (thisDirectory, &findData));

	if (!FindClose (thisDirectory))
		return 1;

	return 0;
}

