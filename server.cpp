/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: server.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.4  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.3  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/02/19 11:24:17  richard_wood
/*  Re-enabled optimisations in classes newssock.cpp, rulesdlg.cpp, server.cpp and tdecjob.cpp
/*
/*  Revision 1.2  2008/09/19 14:51:50  richard_wood
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

// server.cpp - News Server class

#include "stdafx.h"
#include "resource.h"
#include "mplib.h"
#include "newsdb.h"
#include "server.h"
#include "newsgrp.h"
#include "regutil.h"
#include "globals.h"             // TNewsServer
#include "grplist.h"
#include "ssutil.h"
#include "tmutex.h"
#include "tdecjob.h"             // TDecodeJobs
#include "tprnjob.h"             // TPrintJobs
#include "fileutil.h"
#include "servcp.h"
#include "security.h"            // SPA stuff
#include "dbutil.h"              // DBLoad, DBSave

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL (TServerIDs, PObject, 1)

// in class TNewsServer
CCriticalSection TNewsServer::m_critProtectsRefcount;

TServerIDs::TServerIDs()
{
	m_nextMailID   = 300000000;
	m_nextPostID   = 10000000;
	m_nextGroupID  = 1;
	m_reserved1    = 0;
	m_reserved2    = 0;
	m_reserved3    = 0;
}

/////////////////////////////////////////////////////////////////////////////
// TServerIDs - dtor
/////////////////////////////////////////////////////////////////////////////
TServerIDs::~TServerIDs()
{
}

/////////////////////////////////////////////////////////////////////////////
// Serialize - Write out or read in the IDs
/////////////////////////////////////////////////////////////////////////////
void TServerIDs::Serialize (CArchive & ar)
{
	PObject::Serialize (ar);
	if (ar.IsLoading())
	{
		ar >> m_nextGroupID;
		ar >> m_nextPostID;
		ar >> m_nextMailID;
		ar >> m_reserved1;
		ar >> m_reserved2;
		ar >> m_reserved3;
	}
	else
	{
		ar << m_nextGroupID;
		ar << m_nextPostID;
		ar << m_nextMailID;
		ar << m_reserved1;
		ar << m_reserved2;
		ar << m_reserved3;
	}
}

//#pragma optimize("",off)
/////////////////////////////////////////////////////////////////////////////
// TFreeDiskSpace
/////////////////////////////////////////////////////////////////////////////
class TFreeDiskSpace
{
public:
	TFreeDiskSpace();
	~TFreeDiskSpace();

	// Returns 0 for success.
	int GetFreeDiskSpace (LPCTSTR directory, DWORDLONG & quadSize);
private:
	bool        m_fUseExtended;    // use fancy GetDiskFreeSpaceEx  ?
	bool        m_fValid;
	HINSTANCE   m_hKernel;         // load kernel32.dll
	FARPROC     m_lpfn;
};

TFreeDiskSpace::TFreeDiskSpace()
{
	OSVERSIONINFO sVersion;
	m_hKernel = 0;
	m_lpfn    = NULL;
	m_fUseExtended = false;
	m_fValid = true;

	// on windows 95 before OSR2 (OEM Service Release 2), we can't use
	// GetDiskFreeSpaceEx().  Same for NT 3.x

	// find the windows version
	sVersion.dwOSVersionInfoSize = sizeof sVersion;
	if (!GetVersionEx (&sVersion))
	{
		m_fValid = false;
		return ;      // error... no go.
	}

	if (sVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
		LOWORD (sVersion.dwBuildNumber) > 1000)
		m_fUseExtended = true;
	if (sVersion.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		sVersion.dwMajorVersion >= 4)
		m_fUseExtended = true;

	if (m_fUseExtended)
	{
		m_hKernel = LoadLibrary ("KERNEL32.DLL");
		if (m_hKernel)
		{
			m_lpfn = GetProcAddress (m_hKernel, "GetDiskFreeSpaceExA");
		}
	}
}

TFreeDiskSpace::~TFreeDiskSpace()
{
	if (m_hKernel)
		FreeLibrary (m_hKernel);
}

// Returns 0 for success.
int TFreeDiskSpace::GetFreeDiskSpace (LPCTSTR directory, DWORDLONG & quadSize)
{
	if (!m_fValid)
		return 1;

	if (m_lpfn)
	{
		ULARGE_INTEGER sThreadBytes;              // bytes available to process
		ULARGE_INTEGER sTotalBytes, sFreeBytes;

		// this is a PASCAL calling convection function
		typedef BOOL (WINAPI *TYPEGetDiskFreeSpaceEx) (LPCTSTR, PULARGE_INTEGER,
			PULARGE_INTEGER, PULARGE_INTEGER);

		TYPEGetDiskFreeSpaceEx lpfnGetDiskFreeSpaceEx =
			(TYPEGetDiskFreeSpaceEx) m_lpfn;

		int nRet = lpfnGetDiskFreeSpaceEx (directory,
			&sThreadBytes,
			&sTotalBytes,
			&sFreeBytes);

		if ((0 == nRet) || (ERROR_CALL_NOT_IMPLEMENTED == nRet))
			return 1;

		quadSize = sThreadBytes.QuadPart;
		return 0;
	}

	// Use old fashioned function
	// running a windows 95 version prior to OSR2, so we have to use
	// GetDiskFreeSpace(), which can be inaccurate, especially for disks
	// larger than 2GB
	char rcRoot[10]; lstrcpy (rcRoot, "c:\\");
	rcRoot[0] = directory[0];

	DWORD dwSectorsPerCluster, dwBytesPerSector, dwFreeClusters, dwClusters;
	if (!GetDiskFreeSpace (rcRoot, &dwSectorsPerCluster, &dwBytesPerSector,
		&dwFreeClusters, &dwClusters))
		return 1;   // error

	int lRet = MulDiv ((int) dwSectorsPerCluster, (int) dwBytesPerSector, 1);
	if (-1 == lRet)
		return 1;   // error

	quadSize = UInt32x32To64 (dwFreeClusters, lRet);
	return 0;
}

#pragma optimize("",on)

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TNewsServer::TNewsServer()
	: m_subscribed("")
{
	m_pOutbox = 0;
	m_pDrafts = 0;
	m_fOpen = FALSE;
	m_pIStorage = 0;

	m_pDecodeJobs = new TDecodeJobs;       // uncompleted decode jobs
	m_pPrintJobs = new TPrintJobs;         // uncompleted print jobs

	m_hRangeMutex = CreateMutex(NULL, FALSE /*fLock*/, NULL /*name*/);
	m_iRefCount = 0;

	// true if we've updated counts during this session
	m_fUpdatedGroupCounts = false;

	InitializeCriticalSection (&m_critSPA);
	m_pSPA = 0;

	m_pDiskSpace = new TFreeDiskSpace;
}

/////////////////////////////////////////////////////////////////////////////
// TNewsServer Destructor - make sure all of our files are closed, etc...
/////////////////////////////////////////////////////////////////////////////
TNewsServer::~TNewsServer()
{
	// make sure all files are closed
	if (m_fOpen)
		Close ();

	delete m_pDecodeJobs;
	delete m_pPrintJobs;

	ASSERT (m_pOutbox    == 0);
	ASSERT (m_pDrafts    == 0);
	ASSERT (m_pIStorage  == 0);

	delete m_pSPA;
	DeleteCriticalSection (&m_critSPA);

	delete m_pDiskSpace;

	CloseHandle ( m_hRangeMutex );
}

/////////////////////////////////////////////////////////////////////////////
// CheckConvertServer - Check if the structured storage file needs to
//                      be exploded out and do it if necessary.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::CheckConvertServer ()
{
	TPath          idFile (GetServerDatabasePath(), NEW_SERVER_ID_STREAM);
	CFileStatus    status;

	// if we've already converted, just return
	if (CFile::GetStatus (idFile, status))
		return;

	ASSERT (m_pOutbox == 0);

	m_pOutbox = new TOutbox (this);

	// open the structured storage file...

	OpenStructuredFile ();

	// ??? - need to synchronize the object during this funny state...
	m_fOpen = TRUE;

	// tell embedded object what my name is
	AssignServerName ();

	// Check to see if the newsgroup list is in the server
	// settings file, and if so, move them to a separate file

	MoveNewsgroupList ();

	// Load the server IDs

	LoadServerIDs ();

	// load the outbox status information...

	LoadOutboxState ();

	// load the server read range set
	LoadReadRange ();

	// load the subscribed groups
	OldLoadSubscribedGroups ();

	RenameBodyFiles ();

	InitializeReadRange ();

	// load the persistent tags
	LoadPersistentTags ();

	LoadDecodeJobs ();
	LoadPrintJobs ();

	// save to new format
	SaveSubscribedGroups ();
	Close ();

	CloseStructuredFile();
}

/////////////////////////////////////////////////////////////////////////////
// Open - Open this news server up for business...
/////////////////////////////////////////////////////////////////////////////
TNewsServer * TNewsServer::Open ()
{
	if (IsOpen())
		return this;

	CheckConvertServer();

	ASSERT (m_pOutbox == 0);
	ASSERT (m_pDrafts == 0);

	m_pOutbox = new TOutbox (this);
	m_pDrafts = new TDraftMsgs (this);

	// ??? - need to synchronize the object during this funny state...
	m_fOpen = TRUE;

	// tell embedded object what my name is
	AssignServerName ();

	// Load the server IDs

	NewLoadServerIDs ();

	TPath pathDrafts;
	CFileStatus sFS;
	pathDrafts.FormPath (GetServerDatabasePath(), NEW_DRAFTS_FILE);

	if (CFile::GetStatus(pathDrafts, sFS) == FALSE)
	{
		CreateDraftStorage();   
	}

	// load the outbox status information...

	NewLoadOutboxState ();

	// load the drafts information...

	NewLoadDrafts ();

	// load the server read range set
	NewLoadReadRange ();

	// load the subscribed groups
	NewLoadSubscribedGroups ();

	InitializeReadRange ();

	// load the persistent tags
	NewLoadPersistentTags ();

	NewLoadDecodeJobs ();
	NewLoadPrintJobs ();

	// initialize SPASession
	m_pSPA = new TSPASession[2];

	return this;
}

/////////////////////////////////////////////////////////////////////////////
// Close - Close all of the files associated with a news server.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::Close ()
{
	int            count;
	TNewsGroup  *  pNG;

	// make sure we're really open...

	if (!m_fOpen)
		return;

	// make sure all of the groups are closed

	m_subscribed.WriteLock ();
	count = m_subscribed->GetSize ();
	for (int i = 0; i < count; i++)
	{
		pNG = m_subscribed[i];
		// forced close, must clean up...
		while (pNG->IsOpen ())
		{
#if defined(_DEBUG)
			CString groupName = pNG->GetBestname();
			ASSERT(0);
#endif
			pNG->Close ();
		}
		delete pNG;
	}

	m_subscribed.FreeMyMemory ();
	m_subscribed.UnlockWrite ();

	// remove the read range set from memory
	SaveReadRange ();

	SavePersistentTags();
	SaveDecodeJobs();
	SavePrintJobs();

	// outbox purging happens when we close the database
	m_pOutbox -> DoPurging ();
	SaveOutboxState ();
	delete m_pOutbox;
	m_pOutbox = 0;

	SaveDrafts();
	delete m_pDrafts;
	m_pDrafts = 0;

	SaveServerIDs();

	delete [] m_pSPA;
	m_pSPA = 0;

	m_fOpen = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Compact - Compact all of the groups in the server.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::Compact (BOOL fInPlace)
{
	int         count;

	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// ???? sort by smallest to biggest

	m_subscribed.WriteLock ();
	count = m_subscribed->GetSize ();

	for (int i = 0; i < count; i++)
		m_subscribed[i]->Compact (fInPlace);

	m_subscribed.UnlockWrite ();
}

/////////////////////////////////////////////////////////////////////////////
// SaveSettings - Save the server settings back to the registry.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveSettings ()
{
	Save (GetRegistryKey ());
}

/////////////////////////////////////////////////////////////////////////////
// SubscribeGroup - Create the necessary files for holding a group's
//                  headers and bodies.
/////////////////////////////////////////////////////////////////////////////
TNewsGroup * TNewsServer::SubscribeGroup (LPCTSTR  groupName,
										  TGlobalDef::ENewsGroupType eType,
										  BYTE     byStoreMode,
										  int      iGoBack,
										  BOOL     bSample /* = FALSE */)
{
	TNewsGroup * pNewsGroup;

	TNewsGroup::EStorageOption  eStorage;
	TNewsGroup::EStorageOption* peStorage = &eStorage;

	switch (byStoreMode)
	{
	case STORAGE_MODE_DEFAULT: eStorage = gpGlobalOptions->GetStorageMode(); break;
	case STORAGE_MODE_STORENOTHING:   eStorage = TNewsGroup::kNothing;    break;
	case STORAGE_MODE_STOREHEADERS:   eStorage = TNewsGroup::kHeadersOnly;break;
	case STORAGE_MODE_STOREBODIES:    eStorage = TNewsGroup::kStoreBodies;break;
	}

	// make sure the server has been opened...

	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// make sure that the group doesn't exist already

	if (m_subscribed.Exist (groupName))   // -amc
		throw(new TGroupExistsAlready(FormatString (IDS_ERR_GROUP_EXISTS, groupName), kError));

	// create a new newsgroup object

	pNewsGroup = new TNewsGroup (this,
		groupName,
		eType,
		peStorage,
		iGoBack);

	pNewsGroup -> Sample (bSample);

	// have to generate an ID for the group
	pNewsGroup->InstallGroupID ( NextGroupID () );

	// create the header and body files
	pNewsGroup->CreateDBFiles ();

	// get a mutex for changing the group list
	m_subscribed.WriteLock ();

	// put the object in the resident newsgroup array

	m_subscribed.Add (pNewsGroup);

	m_subscribed.UnlockWrite ();

	// maintain a list of subscribed groups
	TMutex sLock(m_hRangeMutex);
	m_spReadRange->AddSubscribedName (groupName);
	m_spReadRange->EraseGroupInfo ();

	return pNewsGroup;
}

/////////////////////////////////////////////////////////////////////////////
// UnsubscribeGroup - Remove files associated with a newsgroup.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::UnsubscribeGroup (LONG groupID)
{
	TNewsGroup  *pNewsGroup = 0;

	// make sure we're open

	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// Get exclusive rights to the array and
	// take the group out the subscribed group list

	{
		TNewsGroupArrayWriteLock ngMgr(m_subscribed);
		m_subscribed.RemoveGroup ( groupID, pNewsGroup );
	}

	if (pNewsGroup)
	{
		TMutex sLock(m_hRangeMutex);

		// take name out of list
		m_spReadRange->RemoveSubscribedName (pNewsGroup->GetName());

		// remove xpost rangeset
		UnreadGroup (pNewsGroup->GetName());
	}

	// if the group is open, close it

	while (pNewsGroup->IsOpen ())
		pNewsGroup->Close ();

	// kill the files...

	pNewsGroup->RemoveDBFiles ();

	delete pNewsGroup;
}

/////////////////////////////////////////////////////////////////////////////
// SaveIndividualGroup - Just save one group, w/o writing out all subscribed
//                       groups
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveIndividualGroup (TNewsGroup * pNG)
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TNewsGroupArrayReadLock ngMgr(m_subscribed);

	CSingleLock lock( &m_IDsMutex, TRUE );

	// save out to SS file
	pNG -> Serialize2 (true,  pNG);
}

/////////////////////////////////////////////////////////////////////////////
// SaveSubscribedGroups - Save the server subscribed group list out to the
//                        the subscribed groups file.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveSubscribedGroups ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TNewsGroupArrayReadLock ngMgr(m_subscribed);
	CSingleLock lock( &m_IDsMutex, TRUE );

	m_subscribed.Serialize2 ( true, GetServerDatabasePath() );
}

/////////////////////////////////////////////////////////////////////////////
// SaveDirtybitGroups -  save out only those groups that look 'dirty'
//
void TNewsServer::SaveDirtybitGroups ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TNewsGroupArrayReadLock ngMgr(m_subscribed);
	CSingleLock lock( &m_IDsMutex, TRUE );

	m_subscribed.SerializeDirty2 ( );
}

/////////////////////////////////////////////////////////////////////////////
// OldLoadSubscribedGroups - Load the server's subscribed group list from the
//                           subscribed groups file.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::OldLoadSubscribedGroups ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	try
	{
		LoadSSObject (m_pIStorage,
			&m_subscribed,
			SUBSCRIBED_FILE);
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_LOADING_SUB_GROUPS, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadSubscribedGroups - Load the server's subscribed group list from the
//                           subscribed groups file.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadSubscribedGroups ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	m_subscribed.Serialize2 ( false,  // fSave
		GetServerDatabasePath() );
}

/////////////////////////////////////////////////////////////////////////////
// GetSubscribedArray - Return a reference to the newsgroup array.
/////////////////////////////////////////////////////////////////////////////
TNewsGroupArray & TNewsServer::GetSubscribedArray()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	return m_subscribed;
}

/////////////////////////////////////////////////////////////////////////////
// LoadReadRange - Load the read range for the group.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::LoadReadRange()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TMutex lock(m_hRangeMutex);

	try
	{
		LoadSSObject (m_pIStorage,
			m_spReadRange.get(),
			READRANGE_FILENAME);
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_LOADING_RANGES, kError);
		except->Display();
		except->Delete();
		m_spReadRange.recreate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadReadRange - Load the read range for the group.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadReadRange()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TMutex lock(m_hRangeMutex);

	m_spReadRange.SafeLoad (false, GetServerDatabasePath(), NEW_READRANGE_FILENAME);
}
/////////////////////////////////////////////////////////////////////////////
// SaveReadRange - Save out the read range.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveReadRange ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TMutex lock(m_hRangeMutex);

	m_spReadRange.Save (GetServerDatabasePath(), NEW_READRANGE_FILENAME);
}

/////////////////////////////////////////////////////////////////////////////
// LoadServerIDs - Load the IDs for the server...
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::LoadServerIDs()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));
	try
	{
		LoadSSObject (m_pIStorage,
			m_spServerIDs.get(),
			SERVER_ID_STREAM);
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_LOADING_IDS, kError);
		except->Display();
		except->Delete();
		m_spServerIDs.recreate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadServerIDs - Load the IDs for the server...
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadServerIDs()
{
	m_spServerIDs.SafeLoad (false, GetServerDatabasePath(), NEW_SERVER_ID_STREAM);
}

/////////////////////////////////////////////////////////////////////////////
// SaveServerIDs - Save out the IDs for the server...
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveServerIDs ()
{
	m_spServerIDs.Save (GetServerDatabasePath(), NEW_SERVER_ID_STREAM);
}

/////////////////////////////////////////////////////////////////////////////
// NextPostID - Get the next post ID and save out the new value...
/////////////////////////////////////////////////////////////////////////////
LONG TNewsServer::NextPostID ()
{
	// this should all be atomic
	CSingleLock lock( &m_IDsMutex, TRUE );

	LONG  nextID = m_spServerIDs->NextPostID ();
	SaveServerIDs ();
	return nextID;
}

/////////////////////////////////////////////////////////////////////////////
// NextMailID - Get the next mail id and save out ID out to the ID file.
/////////////////////////////////////////////////////////////////////////////
LONG  TNewsServer::NextMailID ()
{
	// this should all be atomic
	CSingleLock lock( &m_IDsMutex, TRUE );

	LONG  nextID = m_spServerIDs->NextMailID ();
	SaveServerIDs ();
	return nextID;
}

/////////////////////////////////////////////////////////////////////////////
// NextGroupID - Get the next Group id and save out ID out to the ID file.
/////////////////////////////////////////////////////////////////////////////
LONG  TNewsServer::NextGroupID ()
{
	// this should all be atomic
	CSingleLock lock( &m_IDsMutex, TRUE );

	LONG  nextID = m_spServerIDs->NextGroupID ();
	SaveServerIDs ();
	return nextID;
}

/////////////////////////////////////////////////////////////////////////////
// SavePersistentTags - Save the persistent tags out to the database.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SavePersistentTags ()
{
	TEnterCSection sAuto(m_spTags->GetpCriticalSection());

	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	m_spTags.Save (GetServerDatabasePath(), NEW_PTAGS_FILE);
}

/////////////////////////////////////////////////////////////////////////////
// LoadPersistentTags - Load the persistent tags from the database.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::LoadPersistentTags ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	try
	{
		TEnterCSection sAuto(m_spTags->GetpCriticalSection());

		LoadSSObject (m_pIStorage, m_spTags.get(), PTAGS_FILE);
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_LOADING_TAGS, kError);
		except->Display();
		except->Delete();
		m_spTags.recreate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadPersistentTags - Load the persistent tags from the database.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadPersistentTags ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	TEnterCSection sAuto(m_spTags->GetpCriticalSection());

	if (false == m_spTags.SafeLoad (false, GetServerDatabasePath(), NEW_PTAGS_FILE))
	{
		TException te(IDS_ERR_LOADING_TAGS, kError);
		te.Display();
	}
}

/////////////////////////////////////////////////////////////////////////////
// SaveDecodeJobs - Save unfinished decode jobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveDecodeJobs ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBSave (m_pDecodeJobs, GetServerDatabasePath(), NEW_DECODE_FILE))
	{
		// ??? error recovery
	}
}

/////////////////////////////////////////////////////////////////////////////
// LoadDecodeJobs - Load unfinished decode jobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::LoadDecodeJobs ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// decode jobs are easily corrupted. There is a command line option to re-create
	if (gfSafeStart)
	{
		//  just leave m_pDecodeJobs as an empty collection
		return;
	}

	try
	{
		// this stream gets corrupted easily and there is a command
		// line option to re-generate it

		LoadSSObject (m_pIStorage, m_pDecodeJobs, DECODE_FILE);
	}
	catch (TException *e)
	{
		e->Display ();
		e->Delete();

		m_pDecodeJobs = new TDecodeJobs(GetNewsServerName());
	}
	catch (CException * pCE)
	{
		pCE->ReportError ();
		pCE->Delete();
		m_pDecodeJobs = new TDecodeJobs(GetNewsServerName());
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadDecodeJobs - Load unfinished decode jobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadDecodeJobs ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// decode jobs are easily corrupted. There is a command line option to re-create
	if (gfSafeStart)
	{
		//  just leave m_pDecodeJobs as an empty collection
		return;
	}

	if (!DBLoad (false, m_pDecodeJobs, GetServerDatabasePath(), NEW_DECODE_FILE))
	{
		try { delete m_pDecodeJobs; } catch(...) {}
		m_pDecodeJobs = new TDecodeJobs (GetNewsServerName());
	}
}

/////////////////////////////////////////////////////////////////////////////
// RecreateDecodeJobs - create an empty but valid decode jobs stream
//   does not affect m_pDecodeJobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::RecreateDecodeJobs (BOOL bErrorMsg /* = TRUE */)
{
	// make a fresh one
	TDecodeJobs sFreshJobs(GetNewsServerName());

	if (!DBSave (&sFreshJobs, GetServerDatabasePath(), NEW_DECODE_FILE))
	{
		// ????
	}
}

/////////////////////////////////////////////////////////////////////////////
// SavePrintJobs - Save unfinished print jobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SavePrintJobs ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBSave(m_pPrintJobs, GetServerDatabasePath(), NEW_PRINT_FILE))
	{
		// ?? error recovery
	}
}

/////////////////////////////////////////////////////////////////////////////
// LoadPrintJobs - Load unfinished print jobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::LoadPrintJobs ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));
	try
	{
		LoadSSObject (m_pIStorage, m_pPrintJobs, PRINT_FILE);
	}
	catch (TException *except)
	{
		except->PushError (IDS_ERR_LOADING_PRINT_JOBS, kError);
		except->Display();
		except->Delete();
		m_pPrintJobs = new TPrintJobs (GetNewsServerName());
	}
	catch (CFileException *pFE)
	{
		pFE->ReportError();
		pFE->Delete();
		m_pPrintJobs = new TPrintJobs (GetNewsServerName());
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadPrintJobs - Load unfinished print jobs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadPrintJobs ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBLoad (false, m_pPrintJobs, GetServerDatabasePath(), NEW_PRINT_FILE))
	{
		try { delete m_pPrintJobs; } catch(...) {}
		m_pPrintJobs = new TPrintJobs(GetNewsServerName());
	}
}

/////////////////////////////////////////////////////////////////////////////
// LoadServerGroupList - Load the list of newsgroups at the server from the
//                       server group list file.
/////////////////////////////////////////////////////////////////////////////
TGroupList *TNewsServer::LoadServerGroupList (BOOL fOldStyle)
{
	TGroupList *pAllGroups = new TGroupList;

	if (fOldStyle)
	{
		if (!m_fOpen)
			throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

		try
		{
			LoadSSObject (m_pIStorage, pAllGroups, ALLGROUPS_FILENAME);
		}
		catch (TException *except)
		{
			except->PushError (IDS_ERR_LOADING_NG_LIST, kError);
			TException *ex = new TException(*except);
			except->Delete();
			throw(ex);
			return NULL;
		}

		return pAllGroups;
	}
	else
	{
		if (!m_fOpen)
			throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

		try
		{
			TPath    storePath (GetServerDatabasePath(), ALLGROUPS_FILENAME);
			CFile    groupsFile;

			if (!groupsFile.Open (storePath, CFile::modeRead|CFile::shareExclusive))
				throw(new TException (IDS_ERR_LOADING_NG_LIST, kError));

			CArchive    readArc (&groupsFile, CArchive::load);
			pAllGroups->Serialize (readArc);
			readArc.Close ();
			groupsFile.Close ();
		}
		catch (TException *except)
		{
			except->PushError (IDS_ERR_LOADING_NG_LIST, kError);
			TException *ex = new TException(*except);
			except->Delete();
			throw(ex);
			return NULL;
		}
	}

	return pAllGroups;
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadServerGroupList - Load the list of newsgroups at the server from the
//                          server group list file.
/////////////////////////////////////////////////////////////////////////////
TGroupList *TNewsServer::NewLoadServerGroupList (BOOL fOldStyle)
{
	TGroupList *pAllGroups = new TGroupList;

	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBLoad (false, pAllGroups, GetServerDatabasePath(), ALLGROUPS_FILENAME))
	{
		// ???
	}

	return pAllGroups;
}
/////////////////////////////////////////////////////////////////////////////
// MoveNewsgroupList - If the list of newsgroups is in the Server Settings
//                     file, then move it out to a separate file and
//                     remove that stream.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::MoveNewsgroupList ()
{
	// check if the newsgroup list stream exists in the database...

	if (StreamExist (m_pIStorage, ALLGROUPS_FILENAME))
	{
		TGroupList  *pGroupList;

		// load the old one from the server settings file

		pGroupList = LoadServerGroupList (TRUE);

		// save the list to the separate file

		SaveServerGroupList (pGroupList);
		delete pGroupList;

		// remove the stream from the Server Settings file

		HRESULT hr;
		WCHAR wideStream[256];

		FormWideString (ALLGROUPS_FILENAME, wideStream, sizeof (wideStream));

		hr =  m_pIStorage->DestroyElement (wideStream);

		if (FAILED(hr))
		{
			throw (FormatString (IDS_ERR_DESTROY_GROUPLIST_STREAM, ""), kError);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// SaveServerGroupList - Save the list of server groups out to the server
//                       list file.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveServerGroupList (TGroupList *pGroups)
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBSave (pGroups, GetServerDatabasePath(), ALLGROUPS_FILENAME))
	{
		// ????
	}
}

/////////////////////////////////////////////////////////////////////////////
// GetRangeSet - Get the range set for the news server.  The caller can
//               then use the pointer to get the rangeset for a given
//               group.
/////////////////////////////////////////////////////////////////////////////
TRangeSet *TNewsServer::GetRangeSetReadLock (LPCTSTR groupName)
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	WaitForSingleObject (m_hRangeMutex, INFINITE);

	return (m_spReadRange->GetRangeSet (groupName));
}

/////////////////////////////////////////////////////////////////////////////
// GetRangeSetWriteLock - Get the range set for a given group.  Holds onto
//               a Write Lock
/////////////////////////////////////////////////////////////////////////////
DWORD gdwRSThreadId;
HANDLE ghRSThread;
TRangeSet *TNewsServer::GetRangeSetWriteLock (LPCTSTR groupName)
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// client wants exclusive access, but the server also needs a lock on it
	// when we want to serialize it.
	WaitForSingleObject (m_hRangeMutex, INFINITE);

#if defined(_DEBUG)
	gdwRSThreadId = GetCurrentThreadId();
	ghRSThread = GetCurrentThread();
#endif
	return (m_spReadRange->GetRangeSet (groupName));
}

/////////////////////////////////////////////////////////////////////////////
//  Release a read or write lock. It remembers internally
void TNewsServer::UnlockReadRangeSet(void)
{
	ReleaseMutex (m_hRangeMutex);
}

/////////////////////////////////////////////////////////////////////////////
void TNewsServer::UnlockWriteRangeSet(void)
{
#if defined(_DEBUG)
	gdwRSThreadId = 0;
	ghRSThread = 0;
#endif

	ReleaseMutex (m_hRangeMutex);
}

/////////////////////////////////////////////////////////////////////////////
// UnreadGroup - Removes the range set for a newsgroup.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::UnreadGroup (LPCTSTR group)
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));
	TMutex lock(m_hRangeMutex);

	m_spReadRange->RemoveSubscribedName (group);
	m_spReadRange->RemoveRangeSet (group);
}

/////////////////////////////////////////////////////////////////////////////
// GetDBStats - Get the database stats for the groups for this server.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::GetDBStats (TNewsDBStats *pStats)
{
	int            count;
	TNewsDBStats   groupStats;

	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// loop through the groups and get the stats...
	m_subscribed.ReadLock ();
	count = m_subscribed->GetSize ();

	//

	for (int i = 0; i < (count + 1); i++)
	{
		// either get it for the newsgroup, or at the very end
		// get the stats for the outbox...

		if (i < count)
			m_subscribed[i]->GetDBStats (&groupStats);
		else
			GetOutbox()->GetDBStats (&groupStats);

		pStats->SetCurrentSize (pStats->GetCurrentSize () +
			groupStats.GetCurrentSize());
		pStats->SetCompressibleSize (pStats->GetCompressibleSize() +
			groupStats.GetCompressibleSize());
		if (groupStats.GetExtraSpaceNeeded() > pStats->GetExtraSpaceNeeded())
			pStats->SetExtraSpaceNeeded(groupStats.GetExtraSpaceNeeded());
	}

	m_subscribed.UnlockRead ();
}

/////////////////////////////////////////////////////////////////////////////
// RemoveRegistryKeys - Remove the registry keys for a given server.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::RemoveRegistryKeys ()
{
	TPath serverKey;

	serverKey += GetGravityRegKey()+"Servers";
	serverKey.AddBackSlash ();
	serverKey += GetRegistryKey ();
	if (!UtilRegDelKeyTree (HKEY_CURRENT_USER, serverKey))
	{
		CString str; str.LoadString (IDS_ERR_REMOVING_KEYS);
		throw(new TDatabaseRegistryError(str, kError));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CreateDatabaseFiles - Create the database files for the server.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::CreateDatabaseFiles ()
{
	// create the following files:
	//    1) subscribed groups
	//    2) outbox
	//    3) read range file
	//    4) persistent tags
	//    5) empty newsgroup list...

	TPath serverPath;

	serverPath = GetServerDatabasePath ();

	m_spServerIDs.Save (GetServerDatabasePath(), NEW_SERVER_ID_STREAM);

	TOutbox sTmpOutbox(this);

	// save the outbox manager...
	if (!DBSave (&sTmpOutbox, GetServerDatabasePath(), NEW_OUTBOX_FILE))
	{
		// ???
	}

	// create the other outbox files...
	sTmpOutbox.CreateDBFiles ();

	// create the draft-msgs storage
	CreateDraftStorage();

	m_spReadRange.Save (GetServerDatabasePath(), NEW_READRANGE_FILENAME);

	// create the persistent tags file...

	m_spTags.Save (GetServerDatabasePath(), NEW_PTAGS_FILE);

	// create the saved-decode-jobs file...
	RecreateDecodeJobs ( TRUE );

	// create the saved-print-jobs file...
	TPrintJobs sTmpPrintJobs(GetNewsServerName());
	if (!DBSave(&sTmpPrintJobs, GetServerDatabasePath(), NEW_PRINT_FILE))
	{
	}

	m_pIStorage = 0;  // ??? don't need, but who knows?
}

/////////////////////////////////////////////////////////////////////////////
// GetOutbox - Get a pointer to the server outbox.
/////////////////////////////////////////////////////////////////////////////
TOutbox * TNewsServer::GetOutbox ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	return m_pOutbox;
}

/////////////////////////////////////////////////////////////////////////////
// GetDraftMsgs - Get a pointer to the draft messages
/////////////////////////////////////////////////////////////////////////////
TDraftMsgs * TNewsServer::GetDraftMsgs()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	return m_pDrafts;
}

/////////////////////////////////////////////////////////////////////////////
// SaveOutboxState - Save the outbox object irrespective of whether
//                   it is open or closed or whatever...
//
//  4-21-96  amc Added a mutex around entire process
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveOutboxState ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// I arbitrarily dictate that 'm_OutboxMutex' has the highest priority
	//   so get lesser priority locks first
	TSyncReadLock readLock (m_pOutbox);

	// Get exclusive access while we make a backup copy, write, destroy backup
	CSingleLock lock( &m_OutboxMutex, TRUE );

	DBSave (m_pOutbox, GetServerDatabasePath(), NEW_OUTBOX_FILE);
}

/////////////////////////////////////////////////////////////////////////////
// SaveDrafts - Save the outbox object irrespective of whether
//                   it is open or closed or whatever...
//
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SaveDrafts ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	// I arbitrarily dictate that 'm_OutboxMutex' has the highest priority
	//   so get lesser priority locks first
	TSyncReadLock readLock (m_pDrafts);

	// Get exclusive access while we make a backup copy, write, destroy backup
	CSingleLock lock( &m_OutboxMutex, TRUE );

	DBSave (m_pDrafts, GetServerDatabasePath(), NEW_DRAFTS_FILE);
}

/////////////////////////////////////////////////////////////////////////////
// GetGroupListExist - Checks whether there is a group list file has
//                     been saved.
//
// wae 4/28/98 - removed 100 byte size check because some servers just
//               have a group or two.
/////////////////////////////////////////////////////////////////////////////
BOOL TNewsServer::GetGroupListExist (BOOL fOldStyle)
{
	if (!fOldStyle)
	{
		TPath groupList;

		groupList += GetServerDatabasePath ();
		groupList.AddBackSlash ();

		groupList += ALLGROUPS_FILENAME;

		CFileStatus fileStatus;

		if (CFile::GetStatus (groupList, fileStatus))
			return TRUE;

		return FALSE;
	}
	else
	{
		// open the the group stream and get the size...

		WCHAR    wide[256];
		BOOL     fGroupsHere = FALSE;

		FormWideString (ALLGROUPS_FILENAME, wide, sizeof (wide));

		LPSTREAM pIStream;
		HRESULT  hr;

		hr = m_pIStorage->OpenStream (wide,
			NULL,
			STGM_DIRECT |
			STGM_READ |
			STGM_SHARE_EXCLUSIVE,
			DWORD(0),
			&pIStream);

		if (S_OK != hr)
			return FALSE;

		// get the statistics so we can check the size...

		STATSTG  stats;

		hr = pIStream->Stat (&stats, STATFLAG_NONAME);

		if (S_OK == hr)
			fGroupsHere = TRUE;

		// close the stream...
		pIStream->Release();
		return fGroupsHere;
	}
}

/////////////////////////////////////////////////////////////////////////////
// LoadOutboxState - Load outbox object.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::LoadOutboxState ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	try
	{
		LoadSSObject (m_pIStorage, m_pOutbox, OUTBOX_FILE);
	}
	catch(...)
	{
		m_pOutbox = new TOutbox(this);
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadOutboxState - Load outbox object.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadOutboxState ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBLoad(false, m_pOutbox, GetServerDatabasePath(), NEW_OUTBOX_FILE))
	{
		try
		{
			delete m_pOutbox;
		}
		catch(...) { }

		m_pOutbox = new TOutbox(this);
	}
}

/////////////////////////////////////////////////////////////////////////////
// NewLoadDrafts - Load draft msgs
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::NewLoadDrafts ()
{
	if (!m_fOpen)
		throw(new TServerNotOpen (FormatString (IDS_ERR_SERVER_NOTOPEN, ""), kError));

	if (!DBLoad(false, m_pDrafts, GetServerDatabasePath(), NEW_DRAFTS_FILE))
	{
		try
		{
			delete m_pDrafts;
		}
		catch(...) { }

		m_pDrafts = new TDraftMsgs(this);
	}
}

/////////////////////////////////////////////////////////////////////////////
// SetServerRename - poke stuff in the registry to cause a server rename.
/////////////////////////////////////////////////////////////////////////////
void TNewsServer::SetServerRename (LPCTSTR newName,
								   BOOL    fRemoveAndCreate)
{
	CString  regKey;
	HKEY     hRegKey;
	DWORD    dwRemCreate = fRemoveAndCreate;
	DWORD    disp;

	regKey = GetGravityRegKey()+"Servers";
	regKey += "\\Rename";

	UtilRegCreateKey (regKey, &hRegKey, &disp);

	// add values for the old name, the new name, and whether
	// to remove and create...

	if (
		(ERROR_SUCCESS != RegSetValueEx (hRegKey,
		OLD_SERVER_NAME,
		DWORD(0),
		REG_SZ,
		(CONST BYTE *) LPCTSTR (GetNewsServerName()),
		DWORD (GetNewsServerName().GetLength() + 1)))  ||
		(ERROR_SUCCESS != RegSetValueEx (hRegKey,
		NEW_SERVER_NAME,
		DWORD(0),
		REG_SZ,
		(CONST BYTE *) newName,
		DWORD(_tcslen (newName) + 1))) ||

		(ERROR_SUCCESS != RegSetValueEx (hRegKey,
		REMOVE_AND_CREATE,
		DWORD(0),
		REG_DWORD,
		(CONST BYTE *) &dwRemCreate,
		DWORD(sizeof(dwRemCreate))))
		)
		throw(new TDatabaseRegistryError (FormatString (IDS_ERR_SERV_REG_RENAME,""),kError));

	RegCloseKey (hRegKey);
}

/////////////////////////////////////////////////////////////////////////////
TPersistentTags & TNewsServer::GetPersistentTags()
{
	return *(m_spTags.get());
}

/////////////////////////////////////////////////////////////////////////////
TDecodeJobs * TNewsServer::GetDecodeJobs()
{
	return m_pDecodeJobs;
}

/////////////////////////////////////////////////////////////////////////////
TPrintJobs * TNewsServer::GetPrintJobs()
{
	return m_pPrintJobs;
}

// ------------------------------------------------------------------------
// Init read range with names of the subscribed groups.  We no longer
//  store x-post info for groups that are Not subscribed.
void TNewsServer::InitializeReadRange()
{
	m_subscribed.ReadLock ();

	int count = m_subscribed->GetSize ();
	for (int i = 0; i < count; ++i)
	{
		TNewsGroup * pNG = m_subscribed[i];

		m_spReadRange -> AddSubscribedName ( pNG->GetName() );
	}

	m_subscribed.UnlockRead ();
}

// -----------------------------------------------------------------------
// we are refcounting so we can safely do server switching
void TNewsServer::AddRef (void)
{
	m_critProtectsRefcount.Lock ();
	ASSERT(m_iRefCount >= 0);
	m_iRefCount++;
	m_critProtectsRefcount.Unlock ();
}

// ------------------------------------------------------------------------
// Release - decrement the reference count
void TNewsServer::Release (void)
{
	m_critProtectsRefcount.Lock ();
	ASSERT(m_iRefCount > 0);
	m_iRefCount--;
	ASSERT(m_iRefCount >= 0);
	m_critProtectsRefcount.Unlock ();
}

// ------------------------------------------------------------------------
int TNewsServer::GetRefcount ()
{
	// no locking, this is not meant to be an airtight function
	return m_iRefCount;
}

// ------------------------------------------------------------------------
// For all the components that need to know who we are, tell them now.
void TNewsServer::AssignServerName ()
{
	CString name = GetNewsServerName();

	m_pDecodeJobs -> m_strServerName = name;
	m_pPrintJobs -> m_strServerName = name;
	m_pOutbox -> SetServerName (name);

	// gotta let the newsgroups know what the server name is.
	m_subscribed.SetNewsServerName (name);
}

// ------------------------------------------------------------------------
// Specifically checks the directory of the database. 0 for success
int TNewsServer::GetFreeDiskSpace (DWORDLONG & quadSize)
{
	return m_pDiskSpace->GetFreeDiskSpace (GetServerDatabasePath(), quadSize);
}

// ------------------------------------------------------------------------
int TNewsServer::GetFreeDiskSpaceEx (DWORDLONG & quadSize, LPCTSTR pchDir)
{
	return m_pDiskSpace->GetFreeDiskSpace (pchDir, quadSize);
}

// ------------------------------------------------------------------------
void TNewsServer::OpenStructuredFile ()
{
	TPath    storePath (GetServerDatabasePath(), SERVER_FILE);
	HRESULT  hr;

	WCHAR wideString[256];

	FormWideString (storePath, wideString, sizeof (wideString));

	hr = StgOpenStorage (wideString,
		NULL,
		STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		NULL,
		0,
		&m_pIStorage);

	if (FAILED(hr))
		throw(new TErrorOpeningGlobalsFile (FormatString (IDS_ERR_DB_OPEN, storePath),kError));
}

// ------------------------------------------------------------------------
void TNewsServer::CloseStructuredFile ()
{
	HRESULT  hr;
	if (m_pIStorage)
		hr = m_pIStorage->Commit (STGC_DEFAULT);

	if (FAILED(hr))
	{
		CString str; str.LoadString (IDS_ERR_CLOSING_SETTINGS);
		throw(new TErrorClosingGlobalFile (str, kError));
	}

	m_pIStorage->Release ();
	m_pIStorage = 0;
}

// ------------------------------------------------------------------------
// this info is displayed during the VCR mode
int TNewsServer::GetSubscribedNames (CStringList & lstGroups)
{
	try
	{
		if (IsOpen())
		{
			TServerCountedPtr cpNewsServer(this);

			TNewsGroupArray& vNewsGroups = cpNewsServer->GetSubscribedArray();

			TNewsGroupArrayReadLock readLock(vNewsGroups, INFINITE);

			for (int i = 0; i < vNewsGroups->GetSize(); i++)
			{
				TNewsGroup * pNG = vNewsGroups[i];
				lstGroups.AddTail (pNG->GetName());
			}

		}
		else
		{
			TPath          idFile (GetServerDatabasePath(), NEW_SERVER_ID_STREAM);
			CFileStatus    status;

			TNewsGroupArray  temp(GetNewsServerName());

			// if file does Not exist, this is an unconverted server
			if (FALSE == CFile::GetStatus (idFile, status))
			{
				OpenStructuredFile ();

				LoadSSObject (m_pIStorage,
					&temp,
					SUBSCRIBED_FILE);

				CloseStructuredFile ();
			}
			else
			{
				// new format
				temp.Serialize2 ( false, GetServerDatabasePath () );
			}

			// both cases come back to here
			for (int i = 0; i < temp->GetSize(); i++)
			{
				TNewsGroup * pNG = temp[i];

				lstGroups.AddTail ( pNG->GetName() );

				delete pNG;
			}
		}
		return 0;
	}
	catch (TException *except)
	{
		except->Display ();
		except->Delete();
		return 1;
	}
}

// ------------------------------------------------------------------------
void TNewsServer::RenameBodyFiles ()
{
	for (int i = 0; i < m_subscribed->GetSize(); i++)
	{
		TNewsGroup * pNG = m_subscribed[i];

		CString oldPath = pNG->OldGetNarrowDBPath();

		CString newPath = pNG->GetNarrowDBPath();

		try
		{
			CFile::Rename ( oldPath, newPath );
		}
		catch (CFileException * pFE)
		{
			pFE->ReportError ();
			pFE->Delete();
		}
		catch(...)
		{
			ASSERT(0);
		}
	}
}

// ------------------------------------------------------------------------
void TNewsServer::CreateDraftStorage()
{
	TDraftMsgs sTmpDraft(this);

	if (!DBSave (&sTmpDraft, GetServerDatabasePath(), NEW_DRAFTS_FILE))
	{
		// ???
		ASSERT(0);
	}

	// create the other outbox files...
	sTmpDraft.CreateDBFiles ();
}