/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: nglist.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
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

//////////////////////////////////////////////////////////////////////
// A global array of newsgroup Ptrs.
// Purpose:  - it's a handy thing
//           - during shutdown, freeing each newsgroup, in turn frees
//             the threads and frees the pArticleHeaders
//           - the order in the array is the order in which we shovel
//             groups into the "Tasker-NewsPump-NewsScribe" subsystem
//
//
//  See Coplien Pg. 68 Counted String Pointers aka "Smart Pointers"
//
#pragma once

// warning about CException has no copy-ctor
#pragma warning( disable : 4671 4673 )

#include "bucket.h"
#include "pobject.h"
#include "sharesem.h"
#include "mplib.h"                  // TException

class TNewsGroup;
class TNewsServer;
class TGlobalOptions;

///////////////////////////////////////////////////////////////////////////
// object lives in an array - ultimate arbiter of whether you can
//   access a newsgroup to destroy it
class TNewsGroupUseInfo : public CObject
{
public:
	TNewsGroupUseInfo(TNewsGroup* pNG);
	~TNewsGroupUseInfo() {}

	TNewsGroup* m_pNG;

	int  m_iInUseCount;
	int  m_iDestroyCount;
};

class TNewsGroupArray : public PObject
{
public:
	typedef TSyncTypedPtrArray<CObArray, TNewsGroupUseInfo*> NGA;

protected:
	enum EMode { kUseLock, kUnlockUse, kAccess, kDestroy };

public:
	//   DECLARE_SERIAL(TNewsGroupArray)
	TNewsGroupArray(const CString & serverName);
	~TNewsGroupArray();

	virtual void Serialize(CArchive & ar);
	void  Serialize2 (bool fSave, const CString & dir);
	void  SerializeDirty2 ();

	NGA* operator->() { return pRep; }

	TNewsGroup* operator[](int i) const
	{
		return pRep->GetAt(i)->m_pNG;
	}

	BOOL UseLock(const CString& name, TNewsGroup*& pNG)
	{ return Lookup(name, pNG, TNewsGroupArray::kUseLock); }

	BOOL UseLock(LONG groupID, TNewsGroup*& pNG)
	{ return Lookup(groupID, pNG, TNewsGroupArray::kUseLock); }

	void UnlockUse (const CString& name);
	void UnlockUse (LONG  groupID);

	BOOL Exist (LONG groupID);
	BOOL Exist (const CString& name);

	// returns usage count
	int ReserveGroup (LONG groupID);
	int RemoveGroup (LONG groupID, TNewsGroup*& rpNG);

	BOOL GetNickname (const CString& name, CString& nickName);

	void Add(TNewsGroup* pNG);

	// synchronization
	DWORD ReadLock(DWORD dwTimeOut = INFINITE);
	void  UnlockRead();
	DWORD WriteLock();
	void  UnlockWrite();

	//void SortByName(void);
	int  GetSortedRetrieveArray (bool fHonorInactive, CStringArray* pArray);

	void FreeMyMemory ();

	// controls lock mode in Serialize
	void AllowSerializeTimeout (bool fTimeout)
	{ m_fSerializeTimeout = fTimeout; }

	void UpgradeNeedsGlobalOptions (TGlobalOptions * pOptions);

	void SetNewsServerName (const CString& strServerName)
	{
		m_strServerName = strServerName;
	}

protected:
	BOOL Lookup(const CString& name, TNewsGroup*& pNG,
		TNewsGroupArray::EMode eMode);
	BOOL Lookup(LONG groupID, TNewsGroup*& pNG,
		TNewsGroupArray::EMode eMode);

	BOOL lookup (TNewsGroupArray::EMode eMode,
		void * pInputKey,
		TNewsGroup*& pNG,
		BOOL (*pfnMatch)(TNewsGroup*, void*));

	void empty();
	NGA*  pRep ;  // representation;

	TShareSemaphore   m_Share;
	bool              m_fSerializeTimeout;

	CString  m_strServerName;
};

// automatic locking object
class TNewsGroupArrayReadLock
{
public:
	TNewsGroupArrayReadLock(TNewsGroupArray& array, DWORD dwTimeOut = INFINITE)
		: m_rArray(array)
	{
		if (WAIT_TIMEOUT == m_rArray.ReadLock(dwTimeOut))
			throw (new TException("NewsGroupArray read lock", kInfo));
	}

	~TNewsGroupArrayReadLock()
	{
		m_rArray.UnlockRead();
	}

private:
	TNewsGroupArray& m_rArray;
};

// automatic locking object
class TNewsGroupArrayWriteLock
{
public:
	TNewsGroupArrayWriteLock(TNewsGroupArray& array)
		: m_rArray(array)
	{
		m_rArray.WriteLock();
	}

	~TNewsGroupArrayWriteLock()
	{
		m_rArray.UnlockWrite();
	}

private:
	TNewsGroupArray& m_rArray;
};

// automatic locking object
class TNewsGroupUseLock
{
public:
	TNewsGroupUseLock(TNewsServer * pNewsServer,
		LPCTSTR groupName,
		BOOL* pfLock,
		TNewsGroup*& rpNG);

	TNewsGroupUseLock(TNewsServer * pNewsServer,
		LONG grpID,
		BOOL* pfLock,
		TNewsGroup*& rpNG);

	~TNewsGroupUseLock();

private:
	TNewsGroupArray& m_ngArray;
	int m_iGrpID;
	CString m_GrpName;
	BOOL m_fLock;
};
