/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: nglist.cpp,v $
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
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:36  richard_wood
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
#include "nglist.h"
#include "newsgrp.h"
#include "pobject.h"
#include "critsect.h"
#include "server.h"        // TNewsServer
#include "dbutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//IMPLEMENT_SERIAL(TNewsGroupArray, PObject, 1)

///////////////////////////////////////////////////////////////////////////
// data structure is an element in 'TNewsGroupUseList'
//
TNewsGroupUseInfo::TNewsGroupUseInfo(TNewsGroup * pNG)
: m_pNG(pNG)
{
	m_iInUseCount = m_iDestroyCount = 0;
}

///////////////////////////////////////////////////////////////////////////
// ctor - name of server should be set as soon as possible
TNewsGroupArray::TNewsGroupArray(const CString & serverName)
: m_strServerName(serverName)
{
	pRep = new NGA;
	m_fSerializeTimeout = FALSE;
}

// dtor
TNewsGroupArray::~TNewsGroupArray(void)
{
	FreeMyMemory ();
	m_Share.WriteLock ();
	delete pRep;
	m_Share.UnlockWrite ();
}

void TNewsGroupArray::FreeMyMemory ()
{
	m_Share.WriteLock ();
	empty();
	m_Share.UnlockWrite ();
}

void TNewsGroupArray::empty()
{
	int tot = pRep->GetSize ();

	// delete UseInfo ptrs
	for (int i = 0; i < tot; ++i)
		delete pRep->GetAt(i);
	pRep->RemoveAll ();
}

// -------------------------------------------------------------------------
// 10-17-96  amc  Lock all before starting serialize
void TNewsGroupArray::Serialize(CArchive& ar)
{
	// fTimeout is constant thru this function. m_fSerializeTimeout may change
	BOOL fTimeout = m_fSerializeTimeout;

	// maintain the original format
	//   original format was an array of newsgroup ptrs
	//   before we added the UseInfo structure
	CTypedPtrArray<CObArray, TNewsGroup*> nga;

	DWORD * pStati = 0;
	int     tot = 0;

	if (ar.IsStoring())
	{
		ReadLock ();
		BOOL fFailLock = FALSE;
		tot = pRep->GetSize();
		int i = 0;
		for (i = 0; i < tot; ++i)
			nga.Add ( pRep->GetAt(i)->m_pNG );

		if (fTimeout)
		{
			pStati = new DWORD[tot];
			for (i = 0; i < tot; ++i)
				pStati[i] = WAIT_OBJECT_0 + 40;

			// see if we can get a read lock on all NewsGroup object
			for (i = 0; i < tot; ++i)
			{
				pStati[i] = nga[i]->ReadLock ( 100 );
				if (WAIT_OBJECT_0 != pStati[i])
				{
					fFailLock = TRUE;
					break;
				}
			}
		}

		if (!fFailLock)
		{
			// the ptr array will call Serialize on each newsgroup object
			nga.Serialize ( ar );
		}

		if (fTimeout)
		{
			// unlock all
			for (i = 0; i < tot; ++i)
			{
				if (WAIT_OBJECT_0 == pStati[i])
					nga[i]->UnlockRead ();
			}

			delete [] pStati;
		}
		UnlockRead ();
	}
	else
	{
		empty ();

		nga.Serialize ( ar );

		tot = nga.GetSize ();
		for (int i = 0; i < tot; ++i)
		{
			nga[i]->SetServerName (m_strServerName);
			Add ( nga[i] );
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// Output:  strFilename (should be unique)
int fnGroupArray_MakeFilename (TPath & dir, CString & strFilename)
{
	CString test;

	for (int i = 1; i < 999999; i++)
	{
		test.Format ("G %06d.DAT", i);

		TPath full = dir;
		full.AddBackSlash ();
		full += test;

		// test for existence
		CFileStatus sFS;

		if (FALSE == CFile::GetStatus ( full, sFS ))
		{
			strFilename = test;
			return 0;
		}
	}

	ASSERT(0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////
//  TNewsGroupObject is saved out to the 'body' SS file.
//
//
void  TNewsGroupArray::Serialize2 (bool fSave, const CString & dir1)
{
	TPath dir(dir1);

	if (fSave)
	{
		ReadLock ();

		int tot = pRep->GetSize();

		for (int i = 0; i < tot; i++)
		{
			TNewsGroup * pNG =  pRep->GetAt(i)->m_pNG;

			try
			{
				// grpdb function

				pNG->Serialize2 ( true, pNG );
			}
			catch (TException *rTE)
			{
				rTE->Display ();
				rTE->Delete();
			}
			catch(...)
			{

			}
		} // for loop

		UnlockRead ();
	}
	else
	{
		empty ();

		CFileFind  sFinder;
		BOOL        bWorking, bLastResult = FALSE;

		dir.AddBackSlash();
		dir += "*. G";

		bWorking = sFinder.FindFile (dir);

		while (bWorking || bLastResult)
		{
			bLastResult = bWorking;
			bWorking = sFinder.FindNextFile();

			if (!(bWorking || bLastResult))
				break;

			if (sFinder.IsDirectory())
				continue;

			CString fnameG = sFinder.GetFileName();

			TNewsGroup * pNG = new TNewsGroup;
			pNG->SetServerName (m_strServerName);
			pNG->serialization_SetName (fnameG.Left (fnameG.GetLength() - 3));

			try
			{
				pNG->Serialize2 ( false, pNG );

				// finally !
				Add ( pNG ) ;
			}
			catch (TException *rTE)
			{
				rTE->Display ();
				rTE->Delete();
			}
			catch(...)
			{

			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// write out groups that seem dirty
void  TNewsGroupArray::SerializeDirty2 ()
{
	ReadLock ();

	int tot = pRep->GetSize();

	for (int i = 0; i < tot; i++)
	{
		TNewsGroup * pNG =  pRep->GetAt(i)->m_pNG;

		if (FALSE == pNG->GetDirty())
			continue;

		pNG->ClearDirty();

		try
		{
			// grpdb function

			pNG->Serialize2 ( true, pNG );
		}
		catch (TException *rTE)
		{
			rTE->Display ();
			rTE->Delete();
		}
		catch(...)
		{

		}
	} // for loop

	UnlockRead ();
}

///////////////////////////////////////////////////////////////////////////
//
//
void  TNewsGroupArray::Add(TNewsGroup* pNG)
{
	try
	{
		m_Share.WriteLock();

		TNewsGroupUseInfo* pInfo = new TNewsGroupUseInfo(pNG);

		pRep->Add ( pInfo );
	}
	catch(...)
	{
		m_Share.UnlockWrite();
		throw;
	}
	m_Share.UnlockWrite();
}

static BOOL fnMatch_id(TNewsGroup * pNG, void * p)
{
	return (pNG->m_GroupID == (LONG) p);
}

static BOOL fnMatch_name(TNewsGroup * pNG, void * pStr)
{
	const CString & name = pNG->GetName();
	return (0 == name.CompareNoCase ((LPCTSTR) pStr));
}

///////////////////////////////////////////////////////////////////////////
// Lookup by Name
// fUse = TRUE to use the newsgroup, FALSE to unsubscribe and delete it
//
BOOL TNewsGroupArray::Lookup(const CString& name, TNewsGroup*& pNG,
							 TNewsGroupArray::EMode eMode)
{
	// pass in compare function
	return lookup ( eMode, (void*)(LPCTSTR)name, pNG, fnMatch_name );
}

///////////////////////////////////////////////////////////////////////////
// Lookup by Number
// fUse = TRUE to use the newsgroup, FALSE to unsubscribe and delete it
//
BOOL TNewsGroupArray::Lookup(LONG groupID, TNewsGroup*& pNG,
							 TNewsGroupArray::EMode eMode)
{
	// pass in compare function
	return lookup ( eMode, (void*) groupID, pNG, fnMatch_id );
}

void TNewsGroupArray::UnlockUse (const CString& name)
{
	TNewsGroup * pDummy;
	lookup ( kUnlockUse, (void*)(LPCTSTR) name, pDummy, fnMatch_name );
}

void TNewsGroupArray::UnlockUse (LONG  groupID)
{
	TNewsGroup * pDummy;
	lookup ( kUnlockUse, (void*) groupID, pDummy, fnMatch_id );
}

///////////////////////////////////////////////////////////////////////////
// Worker bee
//
BOOL TNewsGroupArray::lookup (
							  TNewsGroupArray::EMode eMode,
							  void * pInputKey,
							  TNewsGroup*& pNG,
							  BOOL (*pfnMatch)(TNewsGroup*, void*)
							  )
{
	BOOL ret = FALSE;
	TNewsGroupUseInfo * pTest;

	pNG = 0;
	pRep->Lock();
	try
	{
		int Total = pRep->GetSize();
		for (int i = 0; i < Total; ++i)
		{
			pTest = pRep->GetAt(i);
			if (pfnMatch(pTest->m_pNG, pInputKey))
			{
				switch (eMode)
				{
				case kUseLock:
					if (0 == pTest->m_iDestroyCount)
					{
						pTest->m_iInUseCount++;
						pNG = pTest->m_pNG;
						ret = TRUE;
					}
					break;

				case kUnlockUse:
					ASSERT(0==pTest->m_iDestroyCount);
					ASSERT(pTest->m_iInUseCount > 0);
					pTest->m_iInUseCount--;
					ret = TRUE;
					break;

				case kAccess:
					if (0 == pTest->m_iDestroyCount)
					{
						pNG = pTest->m_pNG;
						ret = TRUE;
					}
					break;

				case kDestroy:
					if (0 == pTest->m_iInUseCount)
					{
						pNG = pTest->m_pNG;
						ret = TRUE;
					}
					break;
				} // switch
				break;
			}
		} // for
	} // try
	catch (CException *pE )
	{
		pE->Delete();
		pRep->Unlock();
		return ret;
	}
	pRep->Unlock();
	return ret;
}

BOOL TNewsGroupArray::GetNickname (const CString & name, CString& nickName)
{
	TNewsGroup* pNG;
	if (Lookup(name, pNG, kAccess))
	{
		nickName = pNG->GetNickname();
		return !nickName.IsEmpty();
	}
	return FALSE;
}

BOOL TNewsGroupArray::Exist (LONG groupID)
{
	TNewsGroup* pNG;
	if (Lookup ( groupID, pNG, kAccess ))
		return TRUE;
	else
		return FALSE;
}

BOOL TNewsGroupArray::Exist (const CString& name)
{
	TNewsGroup * pNG;
	if (Lookup ( name, pNG, kAccess ))
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////
// Put a Destroy Count on the NG. (if you can)
// Returns 0 on success
int TNewsGroupArray::ReserveGroup (LONG groupID)
{
	TNewsGroupUseInfo * pTest;
	int ret = 1;
	pRep->Lock();
	int tot = pRep->GetSize();
	for (int i = 0; i < tot; i++)
	{
		pTest = pRep->GetAt(i);

		if (pTest->m_pNG->m_GroupID == groupID)
		{
			if (0 == pTest->m_iInUseCount)
			{
				ret  = 0;
				pTest->m_iDestroyCount ++;
			}
			break;
		}
	}
	pRep->Unlock();
	return ret;
}

///////////////////////////////////////////////////////////////////////////
//
//
int TNewsGroupArray::RemoveGroup (LONG groupID, TNewsGroup*& rpNG)
{
	TNewsGroupUseInfo * pTest;
	pRep->Lock ();
	int  useCount;
	int tot = pRep->GetSize();
	for (int i = 0; i < tot; ++i)
	{
		pTest = pRep->GetAt(i);

		if (pTest->m_pNG->m_GroupID == groupID)
		{
			useCount = pTest->m_iInUseCount;
			if (0 == useCount)
			{
				// extract from Array
				pRep->RemoveAt(i);
				rpNG = pTest->m_pNG;
				delete pTest;
			}
			break;
		}
	}

	pRep->Unlock ();

	// 0 means it was removed
	return useCount;
}

DWORD TNewsGroupArray::ReadLock(DWORD dwTimeOut /* = INFINITE */)
{
	return m_Share.ReadLock(dwTimeOut);
}

void  TNewsGroupArray::UnlockRead()
{
	m_Share.UnlockRead();
}

DWORD TNewsGroupArray::WriteLock()
{
	return m_Share.WriteLock();
}

void  TNewsGroupArray::UnlockWrite()
{
	m_Share.UnlockWrite();
}

// pass in name or nickname
BOOL sort_group_nameLESSEQ(const CString& ngName1, const CString& ngName2)
{
	// return ngName1 <= ngName2;
	int result = _tcsicmp (ngName1, ngName2);
	return (result <= 0) ? TRUE : FALSE;
}

// ------------------------------------------------------------------------
// Get array of names (exclude Mode-1) and then sort them.
int TNewsGroupArray::GetSortedRetrieveArray (bool fHonorInactive, CStringArray* pArray)
{
	ASSERT(pArray);
	TNewsGroupUseInfo *pInfo1;
	int n = 0;
	m_Share.ReadLock();

	CStringArray vFriendly;

	try
	{
		n = pRep->GetSize();
		for (int i = 0; i < n; ++i)
		{
			pInfo1 = pRep->GetAt(i);

			bool fAddThisGroup = true;

			if (fHonorInactive)
				if (false == pInfo1->m_pNG->IsActiveGroup() )
					fAddThisGroup = false;

			if (fAddThisGroup)
			{
				pArray->Add (pInfo1->m_pNG->GetName());
				vFriendly.Add (pInfo1->m_pNG->GetBestname());
			}
		}
	}
	catch(...)
	{
		m_Share.UnlockRead ();
		throw;
	}
	m_Share.UnlockRead ();

	n = pArray->GetSize ();
	int i, j, gap;

	// ShellSort the output, sorting by friendly name

	for (gap = n/2; gap > 0; gap /= 2)
		for (i = gap; i < n; i++)
			for (j = i-gap; j>=0; j-= gap)
			{
				CString str1 = vFriendly[j];
				CString str2 = vFriendly[j+gap];

				if (sort_group_nameLESSEQ (str1, str2))
					break;

				vFriendly[j]     = vFriendly[j+gap];
				vFriendly[j+gap] = str1;

				// mimic movements
				str1 = pArray->GetAt(j);
				pArray->ElementAt(j)     = pArray->GetAt(j+gap);
				pArray->ElementAt(j+gap) = str1;
			}
			return 0;
}

// ------------------------------------------------------------------------
//
void TNewsGroupArray::UpgradeNeedsGlobalOptions (TGlobalOptions * pOptions)
{
	TSyncWriteLock sWriteLock(&m_Share);
	int idx = pRep->GetSize();
	for (--idx; idx >= 0; --idx)
	{
		(pRep->GetAt(idx)) -> m_pNG -> UpgradeNeedsGlobalOptions (pOptions);
	}
}

///////////////////////////////////////////////////////////////////////////
// Automatic locking object
//
TNewsGroupUseLock::TNewsGroupUseLock(
									 TNewsServer * pNewsServer,
									 LPCTSTR groupName,
									 BOOL* pfLock,                        // return TRUE if locked
									 TNewsGroup *& rpNG                   // return Newsgroup ptr
									 )
									 : m_ngArray(pNewsServer->GetSubscribedArray()),  m_GrpName(groupName)
{
	m_iGrpID = -1;
	m_ngArray.ReadLock ();

	// Find it and increment the Usage Count
	m_fLock = m_ngArray.UseLock ( groupName, rpNG );
	*pfLock = m_fLock;

	m_ngArray.UnlockRead ();
}

////////////////////////////////////////////////////////////////////////////
TNewsGroupUseLock::TNewsGroupUseLock(
									 TNewsServer * pNewsServer,
									 LONG grpID,
									 BOOL* pfLock,                        // return TRUE if locked
									 TNewsGroup *& rpNG                   // return Newsgroup ptr
									 )
									 : m_ngArray(pNewsServer->GetSubscribedArray())
{
	m_iGrpID = grpID;

	m_ngArray.ReadLock ();

	// Find it and increment the Usage Count
	m_fLock = m_ngArray.UseLock ( grpID, rpNG );
	*pfLock = m_fLock;

	m_ngArray.UnlockRead ();
}

////////////////////////////////////////////////////////////////////////////
TNewsGroupUseLock::~TNewsGroupUseLock()
{
	if (m_fLock)
	{
		if (m_iGrpID > -1)
			m_ngArray.UnlockUse( m_iGrpID );
		else
			m_ngArray.UnlockUse ( m_GrpName );
	}
}

