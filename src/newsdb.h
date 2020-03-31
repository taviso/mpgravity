/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: newsdb.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.4  2009/07/26 15:54:59  richard_wood
/*  Added import / export of news server.
/*  Refactored import / export of database / settings.
/*  Added command line import of news server.
/*  Fixed crash on trace file use.
/*  Tidied up source code in a few files.
/*
/*  Revision 1.3  2009/06/21 22:45:35  richard_wood
/*  Added Import on first "new install" first run sceen.
/*  Fixed bugs in Import/Export.
/*  Upped version to 2.9.2
/*  Tidied up crap source code formatting.
/*
/*  Revision 1.2  2009/06/12 16:28:07  richard_wood
/*  Added new registry branch for each new minor version of gravity (with upgrade/import code)
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:33  richard_wood
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

// NewsDB.H - Third generation news database, without transaction support

#pragma once

#include "rgserv.h"
#include "servrang.h"
#include "grplist.h"
#include "tglobopt.h"
#include "tidarray.h"            // TIDArray
#include "thrdact.h"             // TThreadAction
#include "tbozobin.h"            // TBozoList
#include "vfilter.h"             // TAllViewFilter
#include "dbutil.h"

class Rules;
class ScoreSets;

// pre 2.2 file names

#define  OPTIONS_FILENAME     ((LPCTSTR) "Global Options")
#define  RULES_FILENAME       ((LPCTSTR) "Rules")
#define  SCORES_FILENAME      ((LPCTSTR) "Scores")
#define  SCORE_COLORS_FILENAME ((LPCTSTR) "Score Colors")
#define  IDARRAY_FILENAME     ((LPCTSTR) "Article IDs")
#define  READRANGE_FILENAME   ((LPCTSTR) "Crosspost Management")
#define  ALLGROUPS_FILENAME   ((LPCTSTR) "Newsgroup List")
#define  SUBSCRIBED_FILE      ((LPCTSTR) "Subscribed Groups")
#define  PTAGS_FILE           ((LPCTSTR) "Persistent Tags")
#define  DECODE_FILE          ((LPCTSTR) "Decode Jobs")
#define  PRINT_FILE           ((LPCTSTR) "Printing")
#define  OUTBOX_FILE          ((LPCTSTR) "Outbox Management")
#define  OLD_SERVER_NAME      ((LPCTSTR) "OldServerName")
#define  NEW_SERVER_NAME      ((LPCTSTR) "NewServerName")
#define  REMOVE_AND_CREATE    ((LPCTSTR) "RemoveAndCreate")
#define  GLOBALS_FILE         ((LPCTSTR) "Global Settings")
#define  SERVER_FILE          ((LPCTSTR) "Server Settings")
#define  HEADERS_STREAM       ((LPCTSTR) "Headers")
#define  HEADER_RULE_INFO_STREAM ((LPCTSTR) "Header Rule Info")
#define  BODIES_STREAM        ((LPCTSTR) "Bodies")
#define  BODY_DESC_STREAM     ((LPCTSTR) "Body Descriptors")
#define  HEADER_RULES_STREAM  ((LPCTSTR) "Header Rules")
#define  SERVER_ID_STREAM     ((LPCTSTR) "Server IDs")
#define  IGNORE_FILENAME      ((LPCTSTR) "Ignore List")
#define  WATCH_FILENAME       ((LPCTSTR) "Watch List")
#define  BOZO_FILENAME        ((LPCTSTR) "Bozo List")
#define  TNEWSGROUP_STREAM    ((LPCTSTR) "NG")

// 2.2 file names

#define  NEW_OPTIONS_FILENAME      ((LPCTSTR) "Options.dat")
#define  NEW_RULES_FILENAME        ((LPCTSTR) "Rules.dat")
#define  NEW_SCORES_FILENAME       ((LPCTSTR) "Scores.dat")
#define  NEW_SCORE_COLORS_FILENAME ((LPCTSTR) "Score Colors.dat")
#define  NEW_IDARRAY_FILENAME      ((LPCTSTR) "ArtIDs.dat")
#define  NEW_READRANGE_FILENAME    ((LPCTSTR) "Crosspost.dat")
#define  NEW_ALLGROUPS_FILENAME    ((LPCTSTR) "Newsgroup List.dat")
#define  NEW_PTAGS_FILE            ((LPCTSTR) "Tags.dat")
#define  NEW_DECODE_FILE           ((LPCTSTR) "Decode.dat")
#define  NEW_PRINT_FILE            ((LPCTSTR) "Print.dat")
#define  NEW_OUTBOX_FILE           ((LPCTSTR) "Outbox.dat")
#define  NEW_DRAFTS_FILE           ((LPCTSTR) "Drafts.dat")
#define  NEW_SERVER_ID_STREAM      ((LPCTSTR) "ServerIDs.dat")
#define  NEW_IGNORE_FILENAME       ((LPCTSTR) "Ignore.dat")
#define  NEW_WATCH_FILENAME        ((LPCTSTR) "Watch.dat")
#define  NEW_BOZO_FILENAME         ((LPCTSTR) "Bozo.dat")
#define  NEW_FILTER_V1_FILENAME    ((LPCTSTR) "Filters.dat")
#define  NEW_FILTER_V2_FILENAME    ((LPCTSTR) "Filters2.dat")

/////////////////////////////////////////////////////////////////////////////
// Exceptions returned by the news database classes.
/////////////////////////////////////////////////////////////////////////////

class TNewsDBError : public TException
{
public:
	TNewsDBError(const CString &description, const ESeverity &kSeverity) :
	  TException(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TDatabaseAlreadyExists : public TNewsDBError
{
public:
	TDatabaseAlreadyExists(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TDatabaseRegistryError : public TNewsDBError

{
public:
	TDatabaseRegistryError(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TServerNotOpen : public TNewsDBError
{
public:
	TServerNotOpen(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TGroupExistsAlready : public TNewsDBError
{
public:
	TGroupExistsAlready(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TGroupIsNotSubscribed : public TNewsDBError
{
public:
	TGroupIsNotSubscribed(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TNoBaseDirectory : public TNewsDBError
{
public:
	TNoBaseDirectory(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TDatabaseDoesNotExist : public TNewsDBError
{
public:
	TDatabaseDoesNotExist(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TDatabaseNotOpen : public TNewsDBError
{
public:
	TDatabaseNotOpen(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TDatabaseCreateDirectory : public TNewsDBError
{
public:
	TDatabaseCreateDirectory(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TServerDoesNotExist : public TNewsDBError
{
public:
	TServerDoesNotExist(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorRemovingServerFiles : public TNewsDBError
{
public:
	TErrorRemovingServerFiles(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorRenamingDatabaseFile : public TNewsDBError
{
public:
	TErrorRenamingDatabaseFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TBadDatabasePath : public TNewsDBError
{
public:
	TBadDatabasePath(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorClosingGlobalFile : public TNewsDBError
{
public:
	TErrorClosingGlobalFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorCreatingGlobalsFile : public TNewsDBError
{
public:
	TErrorCreatingGlobalsFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorOpeningGlobalsFile : public TNewsDBError
{
public:
	TErrorOpeningGlobalsFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorDestroyingStream : public TNewsDBError
{
public:
	TErrorDestroyingStream(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorOpeningStream : public TNewsDBError
{
public:
	TErrorOpeningStream(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorClosingNGFile : public TNewsDBError
{
public:
	TErrorClosingNGFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorCreatingBodiesFile : public TNewsDBError
{
public:
	TErrorCreatingBodiesFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorOpeningNewsgroupFile : public TNewsDBError
{
public:
	TErrorOpeningNewsgroupFile(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
class TErrorCantCompactWhileOpen : public TNewsDBError
{
public:
	TErrorCantCompactWhileOpen(const CString &description, const ESeverity &kSeverity) :
	  TNewsDBError(description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
// TNewsDBStats - statistics for a group or a database as a whole
/////////////////////////////////////////////////////////////////////////////
class TNewsDBStats
{
public:
	int   GetCurrentSize() {return m_currentSize;}
	int   GetCompressibleSize() {return m_compressibleSize;}
	int   GetExtraSpaceNeeded() {return m_extraSpaceNeeded;}
	void  SetCurrentSize(int size) {m_currentSize = size;}
	void  SetCompressibleSize(int size){m_compressibleSize = size;}
	void  SetExtraSpaceNeeded(int size){m_extraSpaceNeeded = size;}
	TNewsDBStats() {
		m_currentSize = m_compressibleSize = m_extraSpaceNeeded = 0;
	}
	~TNewsDBStats(){}
	virtual BOOL CompactingWorthWhile();

public:
	int   m_currentSize;          // current size of the database
	int   m_compressibleSize;     // size if group or database was compressed
	int   m_extraSpaceNeeded;     // how much space is needed for compact
};

/////////////////////////////////////////////////////////////////////////////
// TNewsDB - News Database object.  Spans registry entries, directories,
//           and files that make up the conceptual persistent state of
//           the news system.
/////////////////////////////////////////////////////////////////////////////

class TNewsDB
{
public:
	// friends...

	friend class TServerIterator;

public:
	typedef CMap <CString, LPCTSTR, TNewsServer *, TNewsServer * &> TServerMap;

	TNewsDB();
	~TNewsDB();

	void Create(LPCTSTR path);
	void Open(BOOL fLoadGlobalOptions = TRUE);
	void Destroy();
	BOOL Exists();
	void SaveData();
	BOOL IsOpen() { return (m_fOpen == TRUE); };

	void Close();
	void Compact(BOOL fInPlace);
	void GetDBStats(TNewsDBStats *pStats);

	bool AnyServers();
	TNewsServer * GetFirstServer();

	TNewsServer * CreateServer(LPCTSTR server);
	void RemoveServer(LPCTSTR server);
	BOOL ServerExist(LPCTSTR serverName);

	TNewsServer *GetServerByName(const CString & serverName);
	TNewsServer *OpenNewsServer(LPCTSTR name);

	void SaveRules();
	Rules *GetRuleList();

	static CString GenFileName(LPCTSTR  groupName);

	void SaveScoreSets();
	ScoreSets *GetScoreSets();

	void SaveScoreColors();

	void SaveGlobalOptions();

	TGlobalOptions * GetGlobalOptions()
		{return m_spGlobalOptions.get();}

	////void LockGlobalOptions();
	////void UnlockGlobalOptions();

	TIDArray &GetIDArray() { return *(m_spIdArray.get()); }
	TThreadActionList &GetIgnoreList() { return *(m_spIgnoreList.get()); }
	TThreadActionList &GetWatchList() { return *(m_spWatchList.get()); }
	TAllViewFilter * GetAllViewFilters() { return m_spAllFilters.get(); }

	TBozoList &GetBozoList() { return *(m_spBozoList.get()); }

	static BOOL GetDatabasePath(CString & databasePath);
	void ExpireThreadActions();
	void     CheckServerRename();

	void     CreateDBRegistryKeys(LPCTSTR fullPath, DWORD minimumFree);

	void     RemoveDBRegistryKeys();
	CString  GenServerRegKey();

private:

	void     CheckConversion(BOOL fLoadGlobalOptions);
	void     OldLoadGlobalOptions(BOOL fLoadRegistrySettings = TRUE);
	void     NewLoadGlobalOptions(BOOL fLoadRegistrySettings = TRUE);
	void     OldLoadRules();
	void     NewLoadRules();
	void     OldLoadScoreSets();
	void     NewLoadScoreSets();
	void     OldLoadScoreColors();
	void     NewLoadScoreColors();
	void     OldLoadIDArray();
	void     NewLoadIDArray();
	void     OldLoadThreadActions();
	void     NewLoadThreadActions();
	void     OldLoadBozoList();
	void     NewLoadBozoList();
	void     SaveBozoList();
	void     SaveIDArray();
	void     SaveThreadActions();
	void     LoadServerDefinitions();
	int      LoadViewFilters();
	int      SaveViewFilters();
	int      CreateDefaultViewFilters();

	void     SaveGlobalObject(  CObject *   pOb,
		LPCTSTR     name,
		CObject *   pObArray[] = NULL);

	void     LoadGlobalObject(  CObject *   pOb,
		LPCTSTR     name,
		CObject *   pObArray[] = NULL);

private:
	int            m_fOpen;          // is this database open yet?
	CString        m_databasePath;   // full path to the database directory
	DWORD          m_minimumFree;    // minimum space in megs before warning

	DBLoadMgr<Rules> m_spRules;      // the global list of rules
	DBLoadMgr<ScoreSets>
		m_spScoreSets;    // the global list of score sets

	DBLoadMgr<TGlobalOptions>
		m_spGlobalOptions;  // global options object for odds and ends

	TServerMap     m_servers;        // collection of servers in this database
	HANDLE         m_optionsMutex;   // mutex for the global options
	HANDLE         m_serversMutex;   // guards the server list

	DBLoadMgr<TIDArray> m_spIdArray; // array of IDs of articles user has sent

	LPSTORAGE      m_pIStorage;      // pointer to storage interface

	DBLoadMgr<TThreadActionList>
		m_spIgnoreList; // threads to ignore

	DBLoadMgr<TThreadActionList>
		m_spWatchList;  // threads to watch

	DBLoadMgr<TBozoList>
		m_spBozoList;   // list of names to ignore

	DBLoadMgr<TAllViewFilter>
		m_spAllFilters; // view filters
};

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator - Iterate over the headers for a group.
/////////////////////////////////////////////////////////////////////////////

class THeaderIterator
{
public:
	enum ELockType {kNoLocking, kReadLock, kWriteLock};
	THeaderIterator(TNewsGroupDB *pGroup, ELockType kLockType = kReadLock);
	~THeaderIterator();
	BOOL Next(TPersist822Header *& pHeader);
	void Close();

private:
	void Lock();
	void Unlock();

	POSITION          m_currPos;
	TNewsGroupDB   *  m_pNG;
	BOOL              m_fLocked;
	ELockType         m_kLockType;
};

/////////////////////////////////////////////////////////////////////////////
// TOutboxIterator - Iterate over the email outbox or the outgoing articles
//                   for the current server.
/////////////////////////////////////////////////////////////////////////////

class TOutboxIterator
{
public:
	TOutboxIterator(TNewsServer *);   // iterate articles in server outbox
	TOutboxIterator(TNewsDB *);       // iterate over the email outbox
};

/////////////////////////////////////////////////////////////////////////////
// TRuleIterator - Iterate over the rules.
/////////////////////////////////////////////////////////////////////////////

enum LockType { ReadLock, WriteLock, NoLock };
class TRuleIterator
{
public:
	TRuleIterator(Rules &sRules, LockType iLockType);
	TRuleIterator(LockType iLockType);
	~TRuleIterator();
	Rule *Next();

private:
	void CommonConstruct(Rules &sRules, LockType iLockType);

	POSITION m_iPosition;
	Rules *m_psRules;
	LockType iType;
};
