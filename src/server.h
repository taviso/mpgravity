/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: server.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

#pragma once

#include "afxmt.h"                     // CMutex
#include "nglist.h"
#include "outbox.h"
#include "grplist.h"
#include "rgserv.h"
#include "servrang.h"

// codes for SubscribeGroup()
#define STORAGE_MODE_DEFAULT         1
#define STORAGE_MODE_STORENOTHING    2
#define STORAGE_MODE_STOREHEADERS    3
#define STORAGE_MODE_STOREBODIES     4

#include "ngstat.h"
#include "taglist.h"             // TPersistentTags
#include "servcp.h"

#include "dbutil.h"

// forward declarations
class TDecodeJobs;
class TPrintJobs;
class TSPASession;
class TFreeDiskSpace;

/////////////////////////////////////////////////////////////////////////////
// TServerIDs - used to encapsulate some of the ids that are stored in the
//              server.
/////////////////////////////////////////////////////////////////////////////

class TServerIDs : public PObject
{
public:
	DECLARE_SERIAL (TServerIDs)
	TServerIDs();
	~TServerIDs();
	void Serialize (CArchive & ar);
	LONG NextPostID () {return m_nextPostID++;}
	LONG NextMailID () {return m_nextMailID++;}
	LONG NextGroupID () {return m_nextGroupID++;}

private:
	LONG  m_nextGroupID;
	LONG  m_nextPostID;
	LONG  m_nextMailID;
	LONG  m_reserved1;
	LONG  m_reserved2;
	LONG  m_reserved3;
};

/////////////////////////////////////////////////////////////////////////////
// TNewsServer - Encapsulation of a news server.
/////////////////////////////////////////////////////////////////////////////

class TNewsServer : public TRegServer
{
private:
	// just 1 for all instances of this class
	static CCriticalSection m_critProtectsRefcount;

public:
	friend class TNewsDB;
	friend class TRangeSetReadLock;
	friend class TRangeSetWriteLock;

	TNewsServer ();
	~TNewsServer ();

	void CheckConvertServer ();
	TNewsServer *Open ();
	void Close ();
	void Compact (BOOL fInPlace);

	BOOL IsOpen () {return m_fOpen;}

	void SaveSettings();                 // saves registry values ...

	TNewsGroup * SubscribeGroup (LPCTSTR      groupName,
		TGlobalDef::ENewsGroupType eType,
		BYTE        byStoreMode,
		int         iGoBack,
		BOOL        bSample = FALSE);

	void UnsubscribeGroup (LONG      groupID);
	void SaveSubscribedGroups ();
	void SaveIndividualGroup (TNewsGroup * pNG);
	void SaveDirtybitGroups ();

	void SaveReadRange ();
	void SavePersistentTags ();
	void SaveDecodeJobs ();
	void SavePrintJobs ();

	// get a pointer to the email and article outboxes...
	TOutbox * GetOutbox();
	TDraftMsgs * GetDraftMsgs();

	// save the current outbox state...

	void SaveOutboxState ();
	void SaveDrafts ();

	TGroupList *LoadServerGroupList (BOOL fOldStyle = FALSE);
	TGroupList *NewLoadServerGroupList (BOOL fOldStyle = FALSE);
	void SaveServerGroupList (TGroupList *pGroups);

	void UnreadGroup (LPCTSTR group);

	void GetDBStats (TNewsDBStats *pStats);

	void RemoveRegistryKeys ();

	const CString& GetRegistryKey () {return m_registryKey;}
	void SetRegistryKey (LPCTSTR key) {m_registryKey = key;}

	void CreateDatabaseFiles ();

	TNewsGroupArray & GetSubscribedArray();
	TNewsGroupStatus & GetServerState();
	TPersistentTags & GetPersistentTags();
	TDecodeJobs * GetDecodeJobs();
	TPrintJobs * GetPrintJobs();

	LONG NextPostID ();
	LONG NextMailID ();

	BOOL GetGroupListExist (BOOL fOldStyle = FALSE);

	void SetServerRename (LPCTSTR newName, BOOL fRemoveAndCreate);

	void SaveServerIDs ();

	void BackupVitalData ();
	void MoveNewsgroupList ();

	// for refcounting and server switching
	void AddRef ();
	void Release ();
	int  GetRefcount ();

	// for 'update group counts' only once per session
	void SetGroupCountsUpdated (bool flag) { m_fUpdatedGroupCounts = flag; }
	bool GetGroupCountsUpdated (void) { return m_fUpdatedGroupCounts; }

	// MSN password stuff
	TSPASession * LockSPASession () {
		EnterCriticalSection (&m_critSPA);
		return m_pSPA;
	}

	void UnlockSPASession () {
		LeaveCriticalSection (&m_critSPA);
	}

	int GetFreeDiskSpace (DWORDLONG & quadSize);
	int GetFreeDiskSpaceEx (DWORDLONG & quadSize, LPCTSTR pchDir);

	// for multi-server downloads
	int GetSubscribedNames (CStringList & lst);

protected:
	// Used by friend class TRangeSetReadLock, TRangeSetWriteLock
	//   almost private.

	// Protect the global Read rangeset
	TRangeSet *GetRangeSetReadLock (LPCTSTR  groupName);
	TRangeSet *GetRangeSetWriteLock (LPCTSTR  groupName);
	void UnlockReadRangeSet ();
	void UnlockWriteRangeSet ();

private:
	LONG NextGroupID ();              // next ID for a group
	void OldLoadSubscribedGroups ();    // load the list of subscribed groups
	void NewLoadSubscribedGroups ();    // load the list of subscribed groups
	void LoadReadRange();               // load in the read range
	void NewLoadReadRange();            // load in the read range
	void InitializeReadRange();         // init read range
	void LoadPersistentTags();
	void NewLoadPersistentTags();
	void LoadOutboxState ();
	void NewLoadOutboxState ();
	void NewLoadDrafts ();
	void LoadDecodeJobs ();
	void NewLoadDecodeJobs ();
	void RecreateDecodeJobs (BOOL bErrorMsg = TRUE);   // called if Load fails
	void LoadPrintJobs ();
	void NewLoadPrintJobs ();
	void LoadServerIDs ();
	void NewLoadServerIDs ();
	void AssignServerName ();
	void OpenStructuredFile ();
	void CloseStructuredFile ();

	void RenameBodyFiles ();

	void CreateDraftStorage();

private:
	CString           m_registryKey;    // name of the server - comes from reg
	TNewsGroupArray   m_subscribed;     // array of subscribed newsgroups
	CMutex            m_OutboxMutex;    // protects outbox
	TOutbox *         m_pOutbox;        // email outbox
	TDraftMsgs *      m_pDrafts;        // draft msgs
	DBLoadMgr<TServerRangeSet> m_spReadRange;  // server read range...
	DBLoadMgr<TPersistentTags> m_spTags;       // persistent tags...
	TDecodeJobs *     m_pDecodeJobs;    // uncompleted decode jobs
	TPrintJobs *      m_pPrintJobs;     // uncompleted print jobs
	BOOL              m_fOpen;          // is this server ready to roll?
	TNewsGroupStatus  m_GroupStats;     // incrementing integers
	LPSTORAGE         m_pIStorage;      // pointer to root structured storage
	CMutex            m_IDsMutex;       // protects IDs
	HANDLE            m_hRangeMutex;    // protects ReadRange
	DBLoadMgr<TServerIDs>      m_spServerIDs;  // IDs for this server...
	int               m_iRefCount;      // not serialized, for server switching

	// true if we've updated counts during this session
	bool              m_fUpdatedGroupCounts;
	CRITICAL_SECTION  m_critSPA;
	TSPASession     * m_pSPA;           // stuff for SecPasswordAuthentication
	TFreeDiskSpace  * m_pDiskSpace;     // to check free disk space
};

/////////////////////////////////////////////////////////////////////////////
// TServerIterator - Iterate over the servers in a news database.
/////////////////////////////////////////////////////////////////////////////

class TServerIterator
{
public:
	TServerIterator (TNewsDB *pDatabase);
	~TServerIterator ();
	TNewsServer *Next ();

private:
	POSITION m_currPos;
	TNewsDB *m_pDB;
};

/////////////////////////////////////////////////////////////////////////////
// TRangeSetReadLock - control access to global range set
/////////////////////////////////////////////////////////////////////////////
class TRangeSetReadLock
{
public:
	TRangeSet * m_pRangeSet;
	TRangeSetReadLock (TNewsServer * pServer, LPCTSTR groupName)
		: m_pServer(pServer)
	{
		m_pRangeSet = m_pServer->GetRangeSetReadLock (groupName);
	}

	~TRangeSetReadLock()
	{
		m_pServer->UnlockReadRangeSet ();
	}
private:
	TNewsServer * m_pServer;
};

/////////////////////////////////////////////////////////////////////////////
// TRangeSetWriteLock - control access to global range set
/////////////////////////////////////////////////////////////////////////////
class TRangeSetWriteLock
{
public:
	TRangeSet * m_pRangeSet;
	TRangeSetWriteLock (TNewsServer * pServer, LPCTSTR groupName)
		: m_pServer(pServer)
	{
		m_pRangeSet = m_pServer->GetRangeSetWriteLock (groupName);
	}

	~TRangeSetWriteLock()
	{
		m_pServer->UnlockWriteRangeSet ();
	}
private:
	TNewsServer * m_pServer;
};
