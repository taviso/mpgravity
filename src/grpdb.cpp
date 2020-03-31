/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: grpdb.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.5  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.4  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 14:51:26  richard_wood
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
#include "server.h"
#include "servcp.h"            // TServerCountedPtr
#include "newsdb.h"
#include "grpdb.h"
#include "tmutex.h"
#include "critsect.h"
#include "primes.h"
#include "ssutil.h"
#include "newsgrp.h"
#include "dbutil.h"
#include "utilpump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(THeaderMap, PObject, 1);
IMPLEMENT_SERIAL(TBodyDescriptor, CObject, 1);

extern TNewsDB*      gpStore;

/////////////////////////////////////////////////////////////////////////////
// Copy constructor for TBodyDescriptor...
/////////////////////////////////////////////////////////////////////////////

TBodyDescriptor::TBodyDescriptor (const TBodyDescriptor &rhs)

{
	m_articleNum = rhs.m_articleNum;
	m_offset     = rhs.m_offset;
	m_size       = rhs.m_size;
	m_fPurged    = rhs.m_fPurged;
}

/////////////////////////////////////////////////////////////////////////////
// Assignment operator...
/////////////////////////////////////////////////////////////////////////////

const TBodyDescriptor & TBodyDescriptor::operator=(const TBodyDescriptor &rhs)

{
	m_articleNum = rhs.m_articleNum;
	m_offset     = rhs.m_offset;
	m_size       = rhs.m_size;
	m_fPurged    = rhs.m_fPurged;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// TBodyDescriptor SerializeElements function.  This is used when the body
// descriptor is read into and spit out of the descriptor array to the disk.
/////////////////////////////////////////////////////////////////////////////

template <> void AFXAPI
SerializeElements<TBodyDescriptor>(CArchive &ar, TBodyDescriptor *pDesc, int nCount)
{
	for (int i=0; i < nCount; i++)
		pDesc[i].Serialize ( ar );
}

/////////////////////////////////////////////////////////////////////////////
// TBodyDescriptor::Serialize - Serialize a body descriptor.  Always a part
//                              of a descriptor array.
/////////////////////////////////////////////////////////////////////////////

void TBodyDescriptor::Serialize (CArchive & ar)

{
	LONG temp;
	if (ar.IsStoring())
	{
		ar << m_articleNum;
		ar << m_offset;
		ar << m_size;
		ar << (LONG) m_fPurged;
	}
	else
	{
		ar >> m_articleNum;
		ar >> m_offset;
		ar >> m_size;
		ar >> temp;
		m_fPurged = (BOOL) temp;
	}
}

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupDB Constructor ctor
/////////////////////////////////////////////////////////////////////////////

TNewsGroupDB::TNewsGroupDB ()
{
	// m_strServerName is set just before serialization. (and basic usage)

	m_fOpen           = FALSE;
	m_pHeaderMap      = 0;
	m_pBodyMap        = 0;
	m_pDescriptors    = 0;
	m_pServer         = 0;
	m_fHeadersDirty   = FALSE;
	m_fBodiesDirty    = FALSE;
	InitializeCriticalSection (&m_openSection);
	m_openCount       = 0;
	m_pIStorage       = 0;
	m_pBodyFile       = new COleStreamFile;
	m_bRuleInfoDirty  = 0;
}

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupDB Destructor
/////////////////////////////////////////////////////////////////////////////

TNewsGroupDB::~TNewsGroupDB ()
{
	ASSERT (0 == m_openCount);
	ASSERT (!m_fOpen);
	ASSERT (!m_fBodiesDirty);
	ASSERT (!m_fHeadersDirty);

	if (m_pHeaderMap)
		delete m_pHeaderMap;

	if (m_pBodyMap)
		delete m_pBodyMap;

	if (m_pDescriptors)
		delete m_pDescriptors;

	delete m_pBodyFile;     // get rid of the COleStreamFile...

	DeleteCriticalSection (&m_openSection);
	ASSERT (m_pIStorage == 0);
}

/////////////////////////////////////////////////////////////////////////////
// TNewsGroupDB
/////////////////////////////////////////////////////////////////////////////

BOOL TNewsGroupDB::IsOpen ()

{
	return m_fOpen;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::GetWideDBPath (LPWSTR  pWidePath, int size)

{
	// open the structured storage file...

	TPath    storePath;

	BuildNarrowDBPath ( storePath );

	FormWideString (storePath, pWidePath, size);
}

/////////////////////////////////////////////////////////////////////////////
CString TNewsGroupDB::OldGetNarrowDBPath ()

{
	TPath    sPathOut;

	sPathOut = CacheServerPointer ()->GetServerDatabasePath();
	sPathOut.AddBackSlash ();
	sPathOut += TNewsDB::GenFileName (GetName ());

	return sPathOut;
}

/////////////////////////////////////////////////////////////////////////////

CString TNewsGroupDB::GetNarrowDBPath ()

{
	TPath    storePath;

	BuildNarrowDBPath (storePath);

	return storePath;
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupDB::BuildNarrowDBPath (TPath& rPathOut)
{
	rPathOut = CacheServerPointer ()->GetServerDatabasePath();
	rPathOut.AddBackSlash ();

	CString name = GetName();

	ASSERT(FALSE==name.IsEmpty());

	if (0 == GetType().Compare("TNewsGroup"))
	{
		name.MakeLower();

		if (-1 == name.Find (". g"))
			name += ". g";
	}
	else
	{
		// probably the Outbox
	}

	rPathOut += TNewsDB::GenFileName ( name );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::Open ()

{
	TEnterCSection crit(&m_openSection);

	if (m_fOpen)
	{
		m_openCount++;
		return;
	}

	ASSERT (0 == m_openCount);

	try
	{
		WCHAR    wideString[256];
		HRESULT  hr;

		GetWideDBPath (wideString, sizeof (wideString)/sizeof (WCHAR));

		hr = StgOpenStorage (wideString,
			NULL,
			STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			NULL,
			0,
			&m_pIStorage);

		// if the it wasn't there, we just create it since they probably
		// deleted it...

		if (FAILED(hr))
		{
			CreateDBFiles ();
			hr = StgOpenStorage (wideString,
				NULL,
				STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
				NULL,
				0,
				&m_pIStorage);
			if (FAILED(hr))
			{
				CString temp;

				CString str; str.LoadString (IDS_ERR_OPEN_NG_DB);
				temp.Format (str, GetName(), hr);
				throw(new TErrorOpeningNewsgroupFile (temp, kError));
			}

			// ???? This is bad practice, but there's really no other way
			//      of doing this since we would have to trap dozens of Open
			//      calls to fix this...
			if (0 == GetType().Compare("TNewsGroup"))
			{
				TNewsGroup *pNG = static_cast<TNewsGroup *> (this);
				if (pNG)
				{
					pNG->WriteLock ();
					pNG->HdrRangeEmpty();
					pNG->TextRangeEmpty ();
					pNG->ReadRangeEmpty ();
					pNG->MissingRangeEmpty();
					pNG->SetServerBounds (0, 0);
					pNG->StatusDestroyAll ();

					if (IsActiveServer())
					{
						TServerCountedPtr cpNewsServer;
						cpNewsServer->UnreadGroup (GetName());
					}
					pNG->SetLowwaterMark(-1);
					pNG->SetDirty ();
					pNG->UnlockWrite ();
				}
			}
		}

		try
		{
			LoadHeaders();
		}
		catch(TException *pE)
		{
			pE->Display();
			pE->Delete();
			// and muddle on
		}
		catch(CException *pE)
		{
			pE->ReportError();
			pE->Delete();
			// and muddle on
		}

		LoadHeaderRuleInfo();

		LoadBodyDescriptors();

		OpenBodyFile ();

		m_fOpen = TRUE;
		m_openCount++;
	}
	catch(TException *pE)
	{
		pE->PushError(IDS_FUNC_GRPDBOPEN, kError);
		TException *ex = new TException(*pE);
		pE->Delete();
		throw (ex);
	}
	catch(CException *pCE)
	{
		ConvertCExceptionAndThrow (pCE, IDS_FUNC_GRPDBOPEN);
	}
	//TRACE ("group %s opened\n", GetName ());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::Close (DWORD wait)

{
	try
	{
		TEnterCSection crit (&m_openSection);

		if (!m_fOpen)
		{
			CString temp;
			CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
			temp.Format (str, GetName());
			throw(new TGroupIsNotOpen (temp, kError));
		}

		ASSERT (m_openCount > 0);

		// check to see if we're the only one...

		m_openCount--;

		if (m_openCount > 0)
			return;

		if (m_fBodiesDirty)
			FlushBodies ();

		if (m_bRuleInfoDirty)
			SaveHeaderRuleInfo ();

		// save out the headers if they are dirty...

		if (m_fHeadersDirty)
			SaveHeaders ();

		PreDeleteHeaders();

		delete m_pHeaderMap;
		m_pHeaderMap = 0;

		delete m_pBodyMap;
		m_pBodyMap = 0;

		delete m_pDescriptors;
		m_pDescriptors = 0;

		m_pBodyFile->Close ();

		// close the server structured storage file...
		// commit and close the structured storage file...

		HRESULT  hr;
		if (m_pIStorage)
			hr = m_pIStorage->Commit (STGC_DEFAULT);

		if (FAILED(hr))
		{
			CString temp;
			CString str; str.LoadString (IDS_ERR_GROUP_DB_CLOSE);
			temp.Format (str, GetName());
			throw(new TErrorClosingGlobalFile (temp, kError));
		}

		m_pIStorage->Release ();
		m_pIStorage = 0;

		//TRACE ("Newsgroup %s closed\n", GetName ());
		m_fOpen = FALSE;
	}
	catch(TException *e)
	{
		e->PushError (IDS_FUNC_GRPDBCLOSE, kError);
		TException *ex = new TException(*e);
		e->Delete();
		throw(ex);
	}
	catch(CException *pCE)
	{
		ConvertCExceptionAndThrow(pCE, IDS_FUNC_GRPDBCLOSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
//  this function is virtual

void TNewsGroupDB::PreDeleteHeaders (void)
{
	// base class does nothing
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::SaveHeaders ()

{
	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	ASSERT (0 != m_pHeaderMap);
	ASSERT (m_pServer);

	try
	{
		SaveSSObject (m_pIStorage, m_pHeaderMap, HEADERS_STREAM);
	}
	catch(TException *pE)
	{
		CString temp;
		temp.Format (IDS_ERR_SAVING_HEADERS, LPCTSTR(GetName ()));
		pE->PushError (temp, kError);
		TException *ex = new TException(*pE);
		pE->Delete();
		throw(ex);
		return;
	}
	catch(CException *pCE)
	{
		CString temp;
		temp.Format (IDS_ERR_SAVING_HEADERS, LPCTSTR(GetName ()));

		ConvertCExceptionAndThrow (pCE, temp);
	}

	//TRACE ("Headers Saved For %s\n", GetName ());
	m_fHeadersDirty = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// SaveHeaderRuleInfo -- save out the group's rule-information if dirty
void TNewsGroupDB::SaveHeaderRuleInfo ()
{
	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	if (!m_bRuleInfoDirty)
		return;

	try
	{
		SaveSSObject (m_pIStorage, &m_sRuleInfoMap, HEADER_RULE_INFO_STREAM);
	}
	catch(TException *except)
	{
		CString temp;
		temp.Format (IDS_ERR_HDR_RULE_INFO, LPCTSTR(GetName ()));
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
		return;
	}
	catch(CException *pCE)
	{
		CString temp;
		temp.Format (IDS_ERR_HDR_RULE_INFO, LPCTSTR(GetName ()));

		ConvertCExceptionAndThrow (pCE, temp);
	}

	m_bRuleInfoDirty = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::AddHeader (int artNum, TPersist822Header * pHeader)

{
	// get a write lock while this is happening...

	TSyncWriteLock writeLock(this);

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}
	LONG lNumber = artNum;

	m_fHeadersDirty   = TRUE;

	THeaderDescriptor* pNewHD = new THeaderDescriptor(pHeader);
	m_pHeaderMap->SetAt (lNumber, pNewHD);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

TPersist822Header * TNewsGroupDB::GetHeader (int artNum)

{
	THeaderDescriptor* pHD;

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	LONG  fartNum = artNum;

	if (m_pHeaderMap->Lookup (fartNum, pHD))
		return pHD->GetpHeader();
	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::GetHeaderCount (int & nCount)

{

	nCount  = 0;

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	nCount = m_pHeaderMap->GetCount ();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::PurgeHeader (int artNum)

{
	// get a write lock while this is happening...

	TSyncWriteLock writeLock(this);

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	LONG fartNum = artNum;

	THeaderDescriptor* pHD;
	if (m_pHeaderMap->Lookup (fartNum, pHD))
	{
		m_pHeaderMap->RemoveKey (fartNum);
		delete pHD;
		m_fHeadersDirty   = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::SaveBody (int artNum, TPersist822Text *pText)

{
	try
	{
		DWORD    offset;

		TSyncWriteLock writeLoc( this );

		if (!m_fOpen)
		{
			CString temp;
			CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
			temp.Format (str, GetName());
			throw(new TGroupIsNotOpen (temp, kError));
		}

		// make sure the body file is open

		ASSERT (m_pBodyMap);
		ASSERT (m_pDescriptors);

		ASSERT (m_pBodyFile);

		// find the end of the file
		offset = m_pBodyFile->SeekToEnd();

		// write to the end of the file
		CArchive arcWrite (m_pBodyFile, CArchive::store);

		try
		{
			pText->Serialize (arcWrite);
		}
		catch(CException *pCE)
		{                               // some really careful clean up
			pCE->ReportError();
			pCE->Delete();

			arcWrite.Abort(); // prevent further exceptions during carchive::close

			for (int n = m_pDescriptors->GetSize() - 1;  n >= 0; --n)
			{
				TBodyDescriptor sBD = m_pDescriptors->GetAt( n );

				try
				{
					m_pBodyFile->SetLength (sBD.m_offset + sBD.m_size);         // truncate down

					// if successful, we survive to come here
					return;
				}
				catch(CException *pCE2)       // setLength failed!
				{
					m_pBodyMap->RemoveKey ( sBD.m_articleNum );
					m_pDescriptors->RemoveAt (n);
					//TRACE0("Here internal doggy\r\n");
					pCE2->Delete();
				}
			}
		}

		arcWrite.Flush ();
		arcWrite.Close ();

		// add a descriptor for the body
		TBodyDescriptor   bodyDesc;

		bodyDesc.m_articleNum = artNum;
		bodyDesc.m_offset     = offset;
		bodyDesc.m_size       = m_pBodyFile->Seek (0, CFile::current) - offset;
		bodyDesc.m_fPurged    = FALSE;

		LONG idx = m_pDescriptors->Add (bodyDesc);

		// add a map entry for the body...

		LONG  lNumber = artNum;

		m_fBodiesDirty = TRUE;

		m_pBodyMap->SetAt (lNumber, idx);
	}
	catch(TException  *rTE)
	{
		rTE->PushError (IDS_FUNC_GRPDBSAVEBODY, kError);
		TException *ex = new TException(*rTE);
		rTE->Delete();
		throw(ex);
	}
	catch(CFileException *pFE)
	{
		// pFE = pFE;
		throw pFE;
	}
	catch(CException *pCE)
	{
		ConvertCExceptionAndThrow (pCE, IDS_FUNC_GRPDBSAVEBODY);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::LoadBody (int artNum, TPersist822Text *pText)

{
	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	TSyncReadLock readLoc(this);

	// find the map item...
	LONG idx;

	if (!CheckBodyExist (artNum, idx))
	{
		CString str; str.LoadString (IDS_ERR_NO_BODY);
		throw(new TBodyDoesNotExist (str, kError));
	}

	TBodyDescriptor bodyDesc;

	bodyDesc = m_pDescriptors->GetAt (idx);

	m_pBodyFile->Seek (bodyDesc.m_offset, CFile::begin);

	CArchive readArc (m_pBodyFile, CArchive::load);

	try
	{
		pText->Serialize (readArc);
	}
	catch(CException *pCE)
	{
		readArc.Abort();  // prevent further exceptions (from CArchive==>Cfile::close)
		throw pCE;
	}

	readArc.Close ();
}

/////////////////////////////////////////////////////////////////////////////
void TNewsGroupDB::GetBodyCount (int & iCount)
{
	iCount  = 0;
	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	TSyncReadLock readLoc(this);
	//TRACE0("--GetBodyCount\n");
	int tot = m_pDescriptors->GetSize();
	for (int i = 0; i < tot; i++)
	{
		const TBodyDescriptor & bd = m_pDescriptors->GetAt(i);
		if (!bd.m_fPurged)
		{
			++iCount;
			//TRACE1("#%d\n",bd.m_articleNum);
		}
	}
}

#if defined(_DEBUG)

void TNewsGroupDB::GetBodyPrintable (CString & strOut)
{
	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	TRangeSet     rs;
	TSyncReadLock readLoc(this);

	int tot = m_pDescriptors->GetSize();
	for (int i = 0; i < tot; i++)
	{
		TBodyDescriptor & sBD = m_pDescriptors->GetAt (i);
		if (!sBD.m_fPurged)
			rs.Add (sBD.m_articleNum);
	}

	rs.toString ( strOut );
}

#endif

/////////////////////////////////////////////////////////////////////////////
//  Protected function for now.  Used for outbox verification  -amc
//
/////////////////////////////////////////////////////////////////////////////
BOOL TNewsGroupDB::CheckBodyExist(int iArtInt, LONG & idx)
{
	LONG fartNum = iArtInt;
	return m_pBodyMap->Lookup (fartNum, idx);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::PurgeBody (int artNum)

{
	LONG idx;
	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	// replaces 2 and 3
	TSyncWriteLock writeLoc(this);

	{
		// (2) TSyncReadLock readLoc(this);

		// find the map item...
		LONG  fartNum = artNum;

		if (!m_pBodyMap->Lookup (fartNum, idx))
			return;
	}

	// (3) TSyncWriteLock writeLoc(this);
	TBodyDescriptor bodyDesc = m_pDescriptors->GetAt (idx);
	bodyDesc.m_fPurged = TRUE;

	m_pDescriptors->SetAt (idx, bodyDesc);

	m_fBodiesDirty = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::Compact (BOOL fInPlace)

{
	ASSERT(m_openCount == 1);

	TNewsDBStats   stats;

	GetDBStats (&stats);

	if (!stats.CompactingWorthWhile ())
	{
		Close ();   // close the database that was opened in compactdlg
		return;
	}

	// open and get read lock

	ReadLock ();  // the open is done by the compact dialog

	TNewsGroupDB *pGroupDB = new TNewsGroupDB ();
	pGroupDB->SetServerName(m_strServerName);

	TPath newPath;
	newPath.LoadString (IDS_COMPRESSED);
	newPath += " ";
	newPath += GetName ();

	pGroupDB->SetDBName (newPath);
	pGroupDB->CreateDBFiles ();
	pGroupDB->Open ();

	// save the newsgroup object

	// ???? This is bad practice, but there's really no other way
	if (0 == GetType().Compare("TNewsGroup"))
	{
		try
		{
			TNewsGroup *pNG = static_cast<TNewsGroup *> (this);
			SaveSSObject (pGroupDB->m_pIStorage, pNG, TNEWSGROUP_STREAM);
		}
		catch(TException *except)
		{
			CString temp;
			CString str; str.LoadString (IDS_ERR_SAVING_NEWSGROUP_OBJECT);
			temp.Format (str, GetName ());
			except->PushError (temp, kError);
			TException *ex = new TException(*except);
			except->Delete();
			throw(ex);
			return;
		}
	}

	// save the headers from the newsgroup to the new group db...
	try
	{
		SaveSSObject (pGroupDB->m_pIStorage, m_pHeaderMap, HEADERS_STREAM);
	}
	catch(TException *except)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_SAVE_HEADERS);
		temp.Format (str, GetName ());
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		throw(ex);
		return;
	}

	// read through the body descriptors and add bodies to the news database...

	TBodyDescriptor bodyDescriptor;
	TBodyDescriptor newDescriptor;
	DWORD currPos = 0;
	TCHAR buff[2048];

	UINT  sizeToTransfer;
	LONG  idx;

	THeaderDescriptor* pHD;    // not used

	// loop over the body descriptors looking for dead space...
	for (int i = 0; i < m_pDescriptors->GetSize (); i++)
	{

		bodyDescriptor = m_pDescriptors->GetAt (i);

		// read from the old body file and write to the new one...
		if (!bodyDescriptor.m_fPurged  &&
			m_pHeaderMap->Lookup (bodyDescriptor.m_articleNum, pHD))
		{
			newDescriptor.m_articleNum = bodyDescriptor.m_articleNum;
			newDescriptor.m_offset     = currPos;
			newDescriptor.m_size       = bodyDescriptor.m_size;
			newDescriptor.m_fPurged    = FALSE;

			m_pBodyFile->Seek (bodyDescriptor.m_offset, CFile::begin);
			sizeToTransfer = bodyDescriptor.m_size;

			while (sizeToTransfer > 0)
			{
				if (sizeToTransfer > sizeof (buff))
				{
					m_pBodyFile->Read ((LPVOID) buff, UINT (sizeof (buff)));
					pGroupDB->m_pBodyFile->Write ((LPVOID) buff, UINT (sizeof (buff)));
					sizeToTransfer -= sizeof (buff);
				}
				else
				{
					m_pBodyFile->Read ((LPVOID) buff, sizeToTransfer);
					pGroupDB->m_pBodyFile->Write ((LPVOID) buff, sizeToTransfer);
					sizeToTransfer = 0;
				}
			}

			// add the new body descriptor...
			idx = pGroupDB->m_pDescriptors->Add (newDescriptor);
			pGroupDB->m_pBodyMap->SetAt (newDescriptor.m_articleNum, idx);

			currPos += bodyDescriptor.m_size;
		}
	}

	// save the rule information....
	try
	{
		// $$ Sanity check to clean up bloated datafiles
		LONG           lArtNum;
		TArtRuleInfo * pRecord;
		POSITION       pos = m_sRuleInfoMap.GetStartPosition ();

		while (pos)
		{
			pHD = NULL;
			m_sRuleInfoMap.GetNextAssoc (pos, lArtNum, pRecord);

			if (FALSE == m_pHeaderMap->Lookup (lArtNum, pHD))
			{
				// hey, this is an orphan!
				m_sRuleInfoMap.RemoveKey ( lArtNum );
			}
		}

		SaveSSObject (pGroupDB->m_pIStorage, &m_sRuleInfoMap, HEADER_RULE_INFO_STREAM);
	}
	catch(TException *except)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_SAVE_GROUP_RULE_INFO);
		temp.Format (str, GetName ());
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
	}

	// save the body meta data...

	CObject *   obArray[3];

	// SaveSSObject can write out multiple objects...

	obArray[0] = pGroupDB->m_pBodyMap;
	obArray[1] = pGroupDB->m_pDescriptors;
	obArray[2] = 0;

	try
	{
		SaveSSObject (pGroupDB->m_pIStorage, NULL, BODY_DESC_STREAM, obArray);
	}
	catch(TException *except)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_SAVE_BODY_DESC);
		temp.Format (str, GetName ());
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
	}

	// !!!! - do not mark the header, bodies, or rule information dirty, or
	// the stuff we just wrote would be overwritten...

	pGroupDB->Close ();

	UnlockRead ();
	Close ();      // we close here something that was opened in the compact dlg

	ASSERT (!m_fOpen);

	// we succeeded, get rid of the old, non-compacted file and rename new one...
	TPath oldPath;
	BuildNarrowDBPath (oldPath);

	if (TRUE != DeleteFile (oldPath))
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_REMOVING);
		temp.Format (str, oldPath);
		throw(new TErrorRemovingServerFiles (temp, kError));
	}

	pGroupDB->BuildNarrowDBPath (newPath);

	if (TRUE != MoveFile (newPath, oldPath))
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_RENAMING);
		temp.Format (str, oldPath, newPath);
		throw(new TErrorRenamingDatabaseFile (temp, kError));
	}
	delete pGroupDB;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::GetDBStats (TNewsDBStats *pStats)

{
	// get size of compound file for currentsize parm

	CFileStatus    fileStatus;
	int            rc;
	BOOL           fWeOpenedFile = FALSE;

	// make sure no one is opening or closing this while we're checking it out...
	TEnterCSection crit(&m_openSection);

	// get a read lock on the object...
	TSyncReadLock readLoc(this);

	TPath    storePath;

	BuildNarrowDBPath ( storePath );

	rc = CFile::GetStatus (storePath, fileStatus);
	pStats->SetCurrentSize (fileStatus.m_size);

	// if we're not open, we need to open the compound file
	// so that we can look at the body meta data...

	if (!IsOpen ())
	{
		WCHAR    wideString[256];
		HRESULT  hr;

		GetWideDBPath (wideString, sizeof (wideString)/sizeof (WCHAR));

		hr = StgOpenStorage (wideString,
			NULL,
			STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			NULL,
			0,
			&m_pIStorage);
		if (FAILED(hr))
		{
			CString temp;
			CString str; str.LoadString (IDS_ERR_OPENING);
			temp.Format (str, GetName ());
			throw(new TErrorOpeningNewsgroupFile(temp, kError));
		}

		// load the body descriptors
		LoadBodyDescriptors();
		fWeOpenedFile = TRUE;
	}

	TBodyDescriptor bodyDescriptor;

	LONG           lGroupTotal = 0;
	int            sizeDead = 0;

	// loop over the body descriptors looking for dead space...
	for (int i = 0; i < m_pDescriptors->GetSize (); i++)
	{
		bodyDescriptor = m_pDescriptors->GetAt (i);
		if (bodyDescriptor.m_fPurged)
			sizeDead += bodyDescriptor.m_size;
	}

	// get the actual stream size for each stream and round up to
	// the nearest 512 bytes...

	COleStreamFile bodyStream;
	COleStreamFile descriptorStream;
	COleStreamFile headerStream;
	COleStreamFile ruleStream;

	if (bodyStream.OpenStream (m_pIStorage, BODIES_STREAM))
	{
		lGroupTotal += (((bodyStream.GetLength() - sizeDead) / 512) + 1) * 512;
		bodyStream.Close();
	}

	if (descriptorStream.OpenStream (m_pIStorage, BODY_DESC_STREAM))
	{
		lGroupTotal += ((descriptorStream.GetLength()/512) + 1) * 512;
		descriptorStream.Close ();
	}

	if (headerStream.OpenStream (m_pIStorage, HEADERS_STREAM))
	{
		lGroupTotal += ((headerStream.GetLength()/512) + 1) * 512;
		headerStream.Close ();
	}

	if (ruleStream.OpenStream (m_pIStorage, HEADER_RULE_INFO_STREAM))
	{
		lGroupTotal += ((ruleStream.GetLength()/512) + 1) * 512;
		ruleStream.Close ();
	}

	// add in an extra 512 for good measure
	pStats->SetCompressibleSize (lGroupTotal + 512);
	pStats->SetExtraSpaceNeeded (lGroupTotal + 512);

	if (fWeOpenedFile)
	{
		m_pIStorage->Release ();
		m_pIStorage = 0;
		delete m_pDescriptors;
		m_pDescriptors = NULL;
		delete m_pBodyMap;
		m_pBodyMap = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::CreateDBFiles ()

{
	// we'll play a trick here, create some empty things and then call save...

	// ???? - syncrhonization - this m_fOpen thing is dangerous

	//TRACE ("Creating newsgroup files for %s\n", GetName ());

	WCHAR       wideString[256];
	LPSTORAGE   pIStorage;
	HRESULT     hr;

	CString     dbPath;

	dbPath = GetNarrowDBPath ();

	CFileStatus stat;

	if (CFile::GetStatus (dbPath, stat))
		DeleteFile (dbPath);

	GetWideDBPath (wideString, sizeof (wideString)/sizeof (WCHAR));

	hr = StgCreateDocfile (wideString,
		STGM_DIRECT |
		STGM_READWRITE |
		STGM_SHARE_EXCLUSIVE,
		0,
		&pIStorage);

	if (FAILED(hr))
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_CREATING);
		temp.Format (str, GetName ());
		throw(new TErrorCreatingGlobalsFile (temp, kError));
	}
	else
		m_pIStorage = pIStorage;

	m_fOpen = TRUE;

	// set m_bRuleInfoDirty so that its file is created
	m_bRuleInfoDirty = TRUE;
	SaveHeaderRuleInfo();

	m_pHeaderMap = new THeaderMap;
	SaveHeaders();

	m_pBodyMap     = new TBodyMap;
	m_pDescriptors = new TBodyDescriptors;

	SaveBodyMetaData ();
	m_fOpen = FALSE;

	delete m_pHeaderMap;
	m_pHeaderMap = 0;

	delete m_pBodyMap;
	m_pBodyMap = 0;

	delete m_pDescriptors;
	m_pDescriptors = 0;

	if (!m_pBodyFile->CreateStream (m_pIStorage, BODIES_STREAM))
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_CREATING_BODY);
		temp.Format (str, GetName ());
		throw(new TErrorCreatingBodiesFile(temp, kError));
	}

	m_pBodyFile->Close ();

	m_pIStorage->Commit (STGC_DEFAULT);
	m_pIStorage->Release ();
	m_pIStorage = 0;
}

/////////////////////////////////////////////////////////////////////////////
// RemoveDBFiles - Removes database files associated with a newsgroup.
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::RemoveDBFiles ()

{
	// construct the directory and the filename...

	CString     filePath = GetNarrowDBPath();

	//TRACE ("Removing newsgroup files for %s\n", GetName ());

	if (!DeleteFile (filePath))
	{
		CString str;
		str.Format (IDS_ERR_REMOVING_FILE, LPCTSTR(filePath));
		AfxMessageBox (str);
	}
}

/////////////////////////////////////////////////////////////////////////////
// LoadHeaders - Load the headers for the group into the object...
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::LoadHeaders ()

{
	ASSERT (0 == m_pHeaderMap);
	ASSERT (m_pServer);

	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	//TRACE ("Loading headers for %s\n", GetName ());
	try
	{
		m_pHeaderMap = new THeaderMap;

		LoadSSObject (m_pIStorage,
			m_pHeaderMap,
			HEADERS_STREAM);
	}
	catch(TException *except)
	{
		CString temp;
		temp.Format (IDS_ERR_LOADING, LPCTSTR(GetName ()));
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
		return;
	}
	catch(CException *pCE)
	{
		ConvertCExceptionAndThrow (pCE, IDS_FUNC_LOADHDRS);
		return;
	}
	//TRACE ("Headers loaded for %s\n", GetName ());
}

/////////////////////////////////////////////////////////////////////////////
// LoadHeaderRuleInfo - Load the per-header rule info
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::LoadHeaderRuleInfo ()
{
	ASSERT (m_pServer);
	try
	{
		LoadSSObject (m_pIStorage, &m_sRuleInfoMap, HEADER_RULE_INFO_STREAM);
	}
	catch(TException *except)
	{
		CString temp;
		temp.Format (IDS_ERR_LOADING_NG_RULE_INFO, LPCTSTR(GetName ()));
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
	}
	catch(CException *pCE)
	{
		CString temp;
		temp.Format (IDS_ERR_LOADING_NG_RULE_INFO, LPCTSTR(GetName ()));
		ConvertCExceptionAndThrow (pCE, temp);
	}
}

/////////////////////////////////////////////////////////////////////////////
// LoadBodyDescriptors - Load the data structures that describe what is in
//                       the body file for the group.
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::LoadBodyDescriptors()

{
	CObject *   obArray[3];

	ASSERT (m_pBodyMap == NULL);
	ASSERT (m_pDescriptors == NULL);

	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	//TRACE ("Loading body descriptors for %s\n", GetName ());

	// allocate new objects to hold the body information...

	m_pBodyMap     = new TBodyMap;
	m_pDescriptors = new TBodyDescriptors;

	obArray[0] = m_pBodyMap;
	obArray[1] = m_pDescriptors;
	obArray[2] = 0;

	try
	{
		LoadSSObject (m_pIStorage, NULL, BODY_DESC_STREAM, obArray);
	}
	catch(TException *except)
	{
		CString temp;
		temp.Format (IDS_ERR_LOADING_BODY_DESC, LPCTSTR(GetName ()));
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
	}
	catch(CException *pCE)
	{
		CString temp;
		temp.Format (IDS_ERR_LOADING_BODY_DESC, LPCTSTR(GetName ()));
		ConvertCExceptionAndThrow (pCE, temp);
	}
}

/////////////////////////////////////////////////////////////////////////////
// OpenBodyFile - open up the body file for processing...
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::OpenBodyFile ()

{
	TPath       directory;
	TPath       fileName;

	// get a read lock while this is happening...

	TSyncReadLock readLoc(this);

	try
	{
		ASSERT (m_pBodyFile);

		if (!m_pBodyFile->OpenStream (m_pIStorage,
			BODIES_STREAM))
		{
			CString temp;
			temp.Format (IDS_ERR_OPENING_BODY, LPCTSTR(GetName ()));
			throw(new TErrorOpeningBodyFile(temp, kError));
		}
	}
	catch(CException *pCE)
	{
		CString temp;
		temp.Format (IDS_FUNC_GRPDBOBODY, LPCTSTR(GetName ()));

		ConvertCExceptionAndThrow (pCE, temp);
	}
}

/////////////////////////////////////////////////////////////////////////////
// FlushBodies - Flush the bodies out to disk.
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::FlushBodies ()

{
	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	try
	{
		// read lock is acquired in SaveBodyMetaData

		//TRACE ("Flushing bodies...\n");
		SaveBodyMetaData ();
		m_pBodyFile->Flush ();
	}
	catch(TException *except)
	{
		except->PushError (FormatString(IDS_FUNC_FLUSHBODIES, ""), kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
	}
	catch(CException *pCE)
	{
		ConvertCExceptionAndThrow (pCE, IDS_FUNC_FLUSHBODIES);
	}
}

/////////////////////////////////////////////////////////////////////////////
// SaveBodyMetaData - Save data structures associated with the group bodies.
/////////////////////////////////////////////////////////////////////////////

void TNewsGroupDB::SaveBodyMetaData ()

{
	CObject *   obArray[3];

	if (!m_fOpen)
	{
		CString temp;
		CString str; str.LoadString (IDS_ERR_GROUP_NOT_OPEN);
		temp.Format (str, GetName());
		throw(new TGroupIsNotOpen (temp, kError));
	}

	ASSERT (m_pBodyMap != NULL);
	ASSERT (m_pDescriptors != NULL);

	TSyncReadLock readLoc(this);

	try
	{
		// SaveSSObject can write out multiple objects...

		obArray[0] = m_pBodyMap;
		obArray[1] = m_pDescriptors;
		obArray[2] = 0;

		SaveSSObject (m_pIStorage, NULL, BODY_DESC_STREAM, obArray);
	}
	catch(TException *except)
	{
		CString temp;
		temp.Format (IDS_ERR_SAVING_BODY_DESC, GetName ());
		except->PushError (temp, kError);
		TException *ex = new TException(*except);
		except->Delete();
		throw(ex);
		return;
	}
	catch(CException *pCE)
	{
		CString temp;
		temp.Format (IDS_ERR_SAVING_BODY_DESC, LPCTSTR(GetName ()));
		ConvertCExceptionAndThrow (pCE, temp);
		return;
	}

	m_fBodiesDirty = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator Constructor
/////////////////////////////////////////////////////////////////////////////

THeaderIterator::THeaderIterator (TNewsGroupDB *   pGroup,
								  ELockType        kLockType)

{
	m_pNG          = pGroup;
	m_kLockType    = kLockType;
	m_fLocked      = FALSE;

	Lock ();
	m_currPos = m_pNG->m_pHeaderMap->GetStartPosition();

	// we unlock automatically at the end
	if (m_currPos == NULL)
		Unlock ();
}

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator Destructor
/////////////////////////////////////////////////////////////////////////////

THeaderIterator::~THeaderIterator ()

{
	Unlock ();
}

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator::Next - Get the next header in the iteration...
/////////////////////////////////////////////////////////////////////////////

BOOL THeaderIterator::Next (TPersist822Header *&pHeader)

{
	LONG             key;
	THeaderDescriptor* pHD;

	if (m_currPos == NULL)
	{
		Unlock ();
		return FALSE;
	}
	else
	{
		m_pNG->m_pHeaderMap->GetNextAssoc (m_currPos, key, pHD);
		pHeader = pHD->GetpHeader();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator::Close - Close the iteration.  Do this if you want to
//                          unlock the group before the iterator goes out
//                          of scope.
/////////////////////////////////////////////////////////////////////////////

void THeaderIterator::Close ()

{
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator::Lock - Internal function for doing appropriate type
//                         of locking.
/////////////////////////////////////////////////////////////////////////////

void THeaderIterator::Lock ()

{
	ASSERT (!m_fLocked);
	switch (m_kLockType)
	{
	case kNoLocking:
		return;
		break;
	case kReadLock:
		m_pNG->ReadLock ();
		break;
	case kWriteLock:
		m_pNG->WriteLock ();
		break;
	}

	m_fLocked = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// THeaderIterator::Unlock - Unlock with the appropriate type of unlock.
/////////////////////////////////////////////////////////////////////////////

void THeaderIterator::Unlock ()

{
	if (!m_fLocked)
		return;

	switch (m_kLockType)
	{
	case kNoLocking:
		return;
		break;
	case kReadLock:
		m_pNG->UnlockRead ();
		break;
	case kWriteLock:
		m_pNG->UnlockWrite ();
		break;
	}
	m_fLocked = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

THeaderMap::THeaderMap()
: m_map(200)
{
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

THeaderMap::~THeaderMap()
{
	LONG artInt;
	THeaderDescriptor* pHD;
	POSITION pos = m_map.GetStartPosition();
	while (pos)
	{
		m_map.GetNextAssoc(pos, artInt, pHD);
		delete pHD;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

BOOL THeaderMap::Lookup(LONG artInt, THeaderDescriptor*& rpHD)
{
	return m_map.Lookup(artInt, rpHD);
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

POSITION  THeaderMap::GetStartPosition(void)
{
	return m_map.GetStartPosition();
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void THeaderMap::GetNextAssoc(POSITION& rNextPosition, LONG& artInt, THeaderDescriptor*& rpHD)
{
	m_map.GetNextAssoc(rNextPosition, artInt, rpHD);
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void THeaderMap::Serialize(CArchive& ar)
{
	POSITION           pos;
	LONG               artLong;
	THeaderDescriptor* pHD;
	LONG               total;

	extern DWORD       gdwThreadId;  // Thread ID of UI thread
	bool fMsgLoop      = (GetCurrentThreadId() == gdwThreadId);

	if (ar.IsStoring())
	{
		total = m_map.GetCount();
		ar << total;
		pos = m_map.GetStartPosition();
		while (pos)
		{
			if (fMsgLoop)
				PumpAUsefulMessageLITE ();

			m_map.GetNextAssoc(pos, artLong, pHD);
			pHD->Serialize( ar );
		}
	}
	else
	{
		ar >> total;
		m_map.InitHashTable(prime_larger_than(total));

		for (int i = 0; i < total; ++i)
		{
			if (fMsgLoop)
				PumpAUsefulMessageLITE ();

			pHD = new THeaderDescriptor;
			pHD->Serialize( ar );
			artLong = pHD->m_pPersist822Hdr->GetNumber();
			m_map.SetAt(artLong, pHD);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void THeaderMap::SetAt(LONG artInt, THeaderDescriptor*& rpHD)
{
	m_map.SetAt(artInt, rpHD);
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

BOOL THeaderMap::RemoveKey(LONG artInt)
{
	return m_map.RemoveKey(artInt);
}

///////////////////////////////////////////////////////////////////////////
TArtRuleInfo *TNewsGroupDB::GetHeaderRuleInfo (LONG lArtNum)
{
	TArtRuleInfo *pRecord;

	TSyncReadLock  sRLock (&m_sRuleInfoMap);

	if (!m_sRuleInfoMap.Lookup (lArtNum, pRecord))
		return NULL;
	return pRecord;
}

///////////////////////////////////////////////////////////////////////////
TArtRuleInfo *TNewsGroupDB::CreateHeaderRuleInfo (LONG lArtNum)
{
	TArtRuleInfo *pFoundRecord;

	TSyncWriteLock  sWLock (&m_sRuleInfoMap);

	if (m_sRuleInfoMap.Lookup (lArtNum, pFoundRecord))
	{
		return pFoundRecord;
	}
	else
	{
		TArtRuleInfo *pEmptyRecord = new TArtRuleInfo;

		if (!pEmptyRecord)
			throw(new TException (IDS_ERR_OUT_OF_MEMORY, kError));

		m_sRuleInfoMap.SetAt (lArtNum, pEmptyRecord);
		return pEmptyRecord;
	}
}

///////////////////////////////////////////////////////////////////////////
bool  TNewsGroupDB::RemoveHeaderRuleInfo (LONG lArtNum)
{
	TSyncWriteLock  sWLock (&m_sRuleInfoMap);

	BOOL fRet = m_sRuleInfoMap.RemoveKey ( lArtNum ) ;

	if (fRet)
	{
		RuleInfoDirty ();
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////
// GetParentServer -- this is public
TNewsServer * TNewsGroupDB::GetParentServer ()
{
	return CacheServerPointer ();
}

///////////////////////////////////////////////////////////////////////////
// CacheServerPointer -- this is private
TNewsServer * TNewsGroupDB::CacheServerPointer ()
{
	if (0 == m_pServer)
	{
		ASSERT(gpStore);
		m_pServer = gpStore->GetServerByName (m_strServerName);
	}

	ASSERT(m_pServer);
	return m_pServer;
}

/////////////////////////////////////////////////////////////////////////////
int TNewsGroupDB::GetFreeDiskSpace (DWORDLONG & quadSize)
{
	return CacheServerPointer ()->GetFreeDiskSpace (quadSize);
}

/////////////////////////////////////////////////////////////////////////////
// save out to individual file
bool TNewsGroupDB::DBSave (PObject * pObj, const CString & fname)
{
	return ::DBSave (pObj,
		CacheServerPointer ()->GetServerDatabasePath(),
		fname);
}

/////////////////////////////////////////////////////////////////////////////
// save out to individual file
bool TNewsGroupDB::DBLoad (PObject * pObj, bool  fReportMissingAsTrue, const CString & fname)
{
	return ::DBLoad (fReportMissingAsTrue,
		pObj,
		CacheServerPointer ()->GetServerDatabasePath(),
		fname);
}

/////////////////////////////////////////////////////////////////////////////
// Serialize2
//
void TNewsGroupDB::Serialize2 (bool fSave, PObject * pNG)
{
	TEnterCSection crit(&m_openSection);

	if (m_fOpen)
	{
		ASSERT(m_pIStorage);

		if (fSave)
			SaveSSObject ( m_pIStorage,
			pNG,
			TNEWSGROUP_STREAM );
		else
			LoadSSObject ( m_pIStorage,
			pNG,
			TNEWSGROUP_STREAM );
	}
	else
	{
		// open this enough to write out the TNewsGroup object

		LPSTORAGE pIStorage  = 0;
		WCHAR    wideString[512];
		HRESULT  hr;
		try
		{
			GetWideDBPath (wideString, sizeof (wideString)/sizeof (WCHAR));

			hr = StgOpenStorage (wideString,
				NULL,
				STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
				NULL,
				0,
				&pIStorage);

			if (FAILED(hr))
			{
				CString temp;

				CString str; str.LoadString (IDS_ERR_OPEN_NG_DB);
				temp.Format (str, GetName(), hr);
				throw(new TErrorOpeningNewsgroupFile (temp, kError));
			}

			try
			{
				if (fSave)
					SaveSSObject ( pIStorage,
					pNG,
					TNEWSGROUP_STREAM );
				else
					LoadSSObject ( pIStorage,
					pNG,
					TNEWSGROUP_STREAM );

				hr = pIStorage->Release ();
				ASSERT(!FAILED(hr));
			}
			catch(TException *rTE)
			{
				CString strNarrowPath = GetNarrowDBPath();
				CString msg; msg.Format (IDS_ERROR_READ_PNG, strNarrowPath);
				rTE->PushError (msg, kError);
				TException *ex = new TException(*rTE);
				rTE->Delete();
				throw(ex);
			}
			catch(...)
			{
				ASSERT(0);
				pIStorage->Release ();
				throw;
			}
		}
		catch(TException *rTE)
		{
			rTE->PushError (IDS_FUNC_GRPDBSER2, kError);
			TException *ex = new TException(*rTE);
			rTE->Delete();
			throw(ex);
		}
		catch(CException *pCE)
		{
			ConvertCExceptionAndThrow (pCE, IDS_FUNC_GRPDBSER2);
		}
		catch(...)
		{
			throw(new TException (IDS_FUNC_GRPDBSER2, kError));
		}
	}
}
