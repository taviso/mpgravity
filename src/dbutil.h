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





// structured storage utility functions...

#if !defined (DBUTIL_H)
#define DBUTIL_H
#include "mplib.h"

bool DBSave (CObject *  pOb,
			 const CString & path,
			 const CString & name,
			 CObject *  pObArray[] = NULL);

bool DBLoad (bool       fReportMissingAsTrue,
			 CObject *  pOb,
			 const CString & path,
			 const CString & name,
			 CObject *  pObArray[] = NULL);


template<class T>
class DBLoadMgr {

public:
	DBLoadMgr()
	{
		m_pObj = new T;
	}

	~DBLoadMgr()
	{
		delete m_pObj;
	}

	T* get()
	{
		return m_pObj;
	}

	T* operator->()
	{
		return m_pObj;
	}

	void recreate ()
	{
		try
		{
			delete m_pObj;
		}
		catch (...)
		{

		}

		// make a fresh copy
		m_pObj = new T;
	}

	bool SafeLoad(bool            fReportMissingAsTrue,
		const CString & path,
		const CString & name)
	{

		bool fRet =  DBLoad (fReportMissingAsTrue,
			m_pObj,
			path,
			name);
		if (!fRet)
		{
			ASSERT(0);
			// serialize failed, so toss out this half-serialized object
			recreate ();
		}
		return fRet;
	}

	// this is not incredibly useful, but is here for completeness.
	bool Save (  const CString & path,
		const CString & name)
	{
		return DBSave (m_pObj, path, name);
	}

private:
	T* m_pObj;
};

#endif

