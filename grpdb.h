/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: grpdb.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:26  richard_wood
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

#include "article.h"
#include "pobject.h"
#include "sharesem.h"
#include "artrule.h"             // TArtRuleInfoMap
#include "hd.h"                  // THeaderDescriptor

class TNewsServer;
class TNewsDBStats;
class THeaderIterator;

/////////////////////////////////////////////////////////////////////////////
// Some exceptions...
/////////////////////////////////////////////////////////////////////////////

class TGroupDBErrors : public TException
{
public:
	TGroupDBErrors (const CString &description, const ESeverity &kSeverity) :
	  TException (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TGroupIsNotOpen : public TGroupDBErrors
{
public:
	TGroupIsNotOpen (const CString &description, const ESeverity &kSeverity) :
	  TGroupDBErrors (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TErrorPurgingHeader : public TGroupDBErrors
{
public:
	TErrorPurgingHeader (const CString &description, const ESeverity &kSeverity) :
	  TGroupDBErrors (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TErrorOpeningBodyFile : public TGroupDBErrors
{
public:
	TErrorOpeningBodyFile (const CString &description, const ESeverity &kSeverity) :
	  TGroupDBErrors (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TBodyDoesNotExist : public TGroupDBErrors
{
public:
	TBodyDoesNotExist (const CString &description, const ESeverity &kSeverity) :
	  TGroupDBErrors (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TNotImplemented : public TGroupDBErrors
{
public:
	TNotImplemented (const CString &description, const ESeverity &kSeverity) :
	  TGroupDBErrors (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////

class TErrorRemovingFile : public TGroupDBErrors
{
public:
	TErrorRemovingFile (const CString &description, const ESeverity &kSeverity) :
	  TGroupDBErrors (description, kSeverity)
	  {
	  }
};

/////////////////////////////////////////////////////////////////////////////
// TBodyDescriptor - The body descriptor describes an article or email
//                   body within the news database.
/////////////////////////////////////////////////////////////////////////////

class TBodyDescriptor : public CObject
{
public:
	DECLARE_SERIAL (TBodyDescriptor)
	TBodyDescriptor() {}
	~TBodyDescriptor(){}
	TBodyDescriptor (const TBodyDescriptor &rhs);
	const TBodyDescriptor & operator=(const TBodyDescriptor &rhs);
	void Serialize (CArchive & ar);

	LONG  m_articleNum;                 // article number of the body
	LONG  m_offset;                     // offset of article in the file
	LONG  m_size;                       // size of the body
	BOOL  m_fPurged;                    // has the article been purged?
};

// ConstructElements and DestructElements done automatically
//
template <> void AFXAPI SerializeElements<TBodyDescriptor>
(CArchive &ar, TBodyDescriptor *pHD, int nCount);

class THeaderMap : public PObject
{
public:
	typedef CMap <LONG, LONG &, THeaderDescriptor*, THeaderDescriptor*&> THeaderCMap;

public:
	DECLARE_SERIAL(THeaderMap)
	virtual void Serialize(CArchive& archive);
	THeaderMap();
	~THeaderMap();

	BOOL        Lookup(LONG artInt, THeaderDescriptor*& rpHD);
	POSITION    GetStartPosition(void);
	void        GetNextAssoc(POSITION& rNextPosition, LONG& artInt, THeaderDescriptor*& rpHD);
	void        SetAt(LONG artInt, THeaderDescriptor*& rpHD);
	BOOL        RemoveKey(LONG artInt);
	int         GetCount() { return m_map.GetCount(); }

protected:
	THeaderCMap m_map;
};

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupDB - This class manages the database aspects of a
//                subscribed newsgroup.
/////////////////////////////////////////////////////////////////////////////

class TNewsGroupDB : public TSynchronizable
{
public:
	typedef CMap <LONG, LONG &, LONG, LONG &> TBodyMap;
	typedef CArray <TBodyDescriptor, TBodyDescriptor &> TBodyDescriptors;

	friend class THeaderIterator;

protected:
	TNewsGroupDB ();

public:

	~TNewsGroupDB ();

	BOOL IsOpen ();                  // is the group open?
	virtual void Open ();            // open the group up for processing
	virtual void Close (DWORD wait = 500);   // wait half second before giving up...

	void HeadersDirty () { m_fHeadersDirty = TRUE; }
	void BodiesDirty () {m_fBodiesDirty = TRUE;}
	void SaveHeaders ();
	void SaveHeaderRuleInfo ();

	void AddHeader (int artNum, TPersist822Header * pHeader);
	TPersist822Header * GetHeader (int artNum);
	virtual void PurgeHeader (int artNum);
	void   GetHeaderCount (int & iCount);

	void SaveBody (int artNum, TPersist822Text *pText);
	void LoadBody (int artNum, TPersist822Text *pText);
	void PurgeBody (int artNum);
	void GetBodyCount (int & iCount);
	void GetBodyPrintable (CString & txt);

	void Compact (BOOL fInPlace);
	void GetDBStats (TNewsDBStats *pStats);

	virtual void MonthlyMaintenance () {  }

	// this is done for newsgroups but is no-op for outbox...
	virtual void PurgeByDate (bool fWriteOutSubscribedGroups = true) {}

	void CreateDBFiles ();                // create initial db files...
	void RemoveDBFiles ();                // remove the db files when unsubscribed

	// if we're just a TNewsGroupDB, then we'll return
	// this, otherwise, the derived class will return the
	// name...

	virtual const CString & GetName () {return m_dbName;}
	void SetDBName (LPCTSTR szName) {
		m_dbName = szName;
	}

	void FlushBodies ();

	void SaveBodyMetaData ();

	void RuleInfoDirty () { m_bRuleInfoDirty = TRUE; }
	TArtRuleInfo *GetHeaderRuleInfo (LONG lArtNum);
	TArtRuleInfo *CreateHeaderRuleInfo (LONG lArtNum);
	bool          RemoveHeaderRuleInfo (LONG lArtNum);

	virtual CString GetType(){
		return CString("TNewsGroupDB");
	}

	// caller must do refcounting
	TNewsServer * GetParentServer ();

	virtual void SetServerName (const CString& strServerName)
	{
		m_strServerName = strServerName;
	}

	// disk space checking for the server DB path.
	int GetFreeDiskSpace (DWORDLONG & quadSize);

public:  // used by nglist.h
	bool DBSave (PObject * pObj, const CString & fname);
	bool DBLoad (PObject * pObj,
		bool    fReportMissingAsTrue,
		const CString & fname);

	void Serialize2 (bool fSave, PObject * pNG);

public:
	CString GetNarrowDBPath ();
	CString OldGetNarrowDBPath ();

private:
	void LoadHeaders ();
	void LoadHeaderRuleInfo ();
	void LoadBodyDescriptors();
	void OpenBodyFile ();
	void GetWideDBPath (LPWSTR  pWidePath, int size);
	void  BuildNarrowDBPath (TPath& rPathOut);
	TNewsServer * CacheServerPointer ();

	// Try making this private
private:
	TNewsServer      *   m_pServer;
	CString              m_strServerName;

protected:
	virtual void PreDeleteHeaders ();
	BOOL  CheckBodyExist (int iArtInt, LONG & idx);  // could be public..

private:
	BOOL                 m_fOpen;
	THeaderMap       *   m_pHeaderMap;
	TBodyMap         *   m_pBodyMap;
	TBodyDescriptors *   m_pDescriptors;
	COleStreamFile   *   m_pBodyFile;
	// TShareSemaphore      m_pinSem;
	BOOL                 m_fHeadersDirty;
	BOOL                 m_fBodiesDirty;
	int                  m_openCount;
	CRITICAL_SECTION     m_openSection;
	LPSTORAGE            m_pIStorage;
	TArtRuleInfoMap      m_sRuleInfoMap;
	BOOL                 m_bRuleInfoDirty;
	CString              m_dbName;
};

class TAutoClose
{
	// Goal : open and close a newsgroup automatically
public:
	TAutoClose(TNewsGroupDB * pNG) : m_pDB(pNG)
	{ m_pDB->Open(); }

	~TAutoClose()
	{ m_pDB->Close(); }

private:
	TNewsGroupDB * m_pDB;
};
