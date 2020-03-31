/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mpexcept.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:31  richard_wood
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

/////////////////////////////////////////////////////////////////////////////
// MicroPlanet Class Library Exception Implementation
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "mplib.h"
#include "resource.h"            // IDS_UNKNOWN

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Next - Get the next error on the error stack.
/////////////////////////////////////////////////////////////////////////////
int TErrorIterator::Next(CString & errString, ESeverity & eSeverity)
{
	int   size;

	size = m_pExcept->HowMany ();
	if (m_index != -1 && m_index < size)
	{
		m_pExcept->GetError(errString, eSeverity, m_index);
		m_index++;
		if (m_index == size)
			m_index = -1;
		return 1;
	}
	else
		return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Reset - Reset the iterator to the first error.
/////////////////////////////////////////////////////////////////////////////
int TErrorIterator::Reset()
{
	m_index = 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// TError - Construct a TError object.
/////////////////////////////////////////////////////////////////////////////
TError::TError()
{
	m_kSeverity = kError;
}

/////////////////////////////////////////////////////////////////////////////
// TError - Construct a TError object from a string and severity
/////////////////////////////////////////////////////////////////////////////
TError::TError(const CString &description, ESeverity kSeverity,
				EErrorClass eClass /*= kClassUnknown*/)
				: m_description(description)
{
	m_dwError   = 0;
	m_eClass    = eClass;
	m_kSeverity = kSeverity;
}

/////////////////////////////////////////////////////////////////////////////
TError::TError(ESeverity kSeverity, EErrorClass eClass, DWORD dwError)
{
	// m_description is empty
	m_dwError   = dwError;
	m_eClass    = eClass;
	m_kSeverity = kSeverity;
}

/////////////////////////////////////////////////////////////////////////////
TError::TError(int iStringID, ESeverity kSeverity, EErrorClass eClass)
{
	m_dwError = 0;
	m_description.LoadString(iStringID);
	m_eClass    = eClass;
	m_kSeverity = kSeverity;
}

/////////////////////////////////////////////////////////////////////////////
// TError (const TError &) - Copy constructor.
/////////////////////////////////////////////////////////////////////////////
TError::TError(const TError &error)
{
	m_kSeverity   = error.m_kSeverity;
	m_eClass      = error.m_eClass;
	m_dwError     = error.m_dwError;
	m_description = error.m_description;
}

/////////////////////////////////////////////////////////////////////////////
// operator= - Allows assigning of errors to each other.
/////////////////////////////////////////////////////////////////////////////
TError & TError::operator=(const TError &error)
{
	if (&error != this)
	{
		m_kSeverity   = error.m_kSeverity;
		m_eClass      = error.m_eClass;
		m_dwError     = error.m_dwError;
		m_description = error.m_description;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// ~TError - Destructor for a TError.
/////////////////////////////////////////////////////////////////////////////
TError::~TError()
{
	// currently empty
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TError::GetDescription(CString & description)
{
	description = m_description;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TError::GetSeverity(ESeverity & kSeverity)
{
	kSeverity = m_kSeverity;
}

/////////////////////////////////////////////////////////////////////////////
void TError::GetClass(EErrorClass & eClass)
{
	eClass = m_eClass;
}

/////////////////////////////////////////////////////////////////////////////
TErrorList::TErrorList()
{
}

// copy ctor
TErrorList::TErrorList(const TErrorList & src)
{
	*this = src;
}

// assignment
TErrorList & TErrorList::operator= (const TErrorList & rhs)
{
	int    size;

	if (&rhs != this)
	{
		size = rhs.GetSize();

		for (int i = 0; i < size; i++)
		{
			TError rError = rhs.m_errVec.GetAt(i);

			m_errVec.InsertAt(i, rError);
		}
	}
	return *this;
}

TErrorList::~TErrorList()
{
}

/////////////////////////////////////////////////////////////////////////////
int TErrorList::PushError(TError & sError)
{
	try
	{
		m_errVec.InsertAt(0, sError);
	}
	catch(...)
	{
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int TErrorList::ClearErrors()
{
	m_errVec.RemoveAll();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void TErrorList::GetErrorByIndex(int index, ESeverity & eSeverity,
								 EErrorClass & eClass,
								 DWORD & dwError, CString & description)
{
	TError & rError = m_errVec.GetAt(index);

	rError.GetSeverity(eSeverity);
	rError.GetClass(eClass);
	dwError = rError.GetDWError();
	rError.GetDescription(description);
}

/////////////////////////////////////////////////////////////////////////////
// TException - Default constructor for one of exceptions.
/////////////////////////////////////////////////////////////////////////////
TException::TException()
{
	// have to get rid of the crap that's in the list
}

/////////////////////////////////////////////////////////////////////////////
// TException (const TException &) - This allows construction of a new execption
//                          by copying from an existing one.
/////////////////////////////////////////////////////////////////////////////
TException::TException(const TException & except)
{
	// re-route to assignment operator

	*this = except;
}

/////////////////////////////////////////////////////////////////////////////
// Construct an exception while simultaneously pushing an error.
/////////////////////////////////////////////////////////////////////////////
TException::TException(const CString & description, const ESeverity & kSeverity)
{
	PushError(description, kSeverity);
}

TException::TException(int iStringID, const ESeverity & kSeverity)
{
	PushError(iStringID, kSeverity);
}

/////////////////////////////////////////////////////////////////////////////
// operator= This allows exceptions to be assigned to each other.
/////////////////////////////////////////////////////////////////////////////
TException & TException::operator=(const TException & rhs)
{
	if (&rhs != this)
		m_errorList = rhs.m_errorList;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// ~TException - Destructor for an exception.  In our case, the object list
//               full of error information is freed (deleted).
/////////////////////////////////////////////////////////////////////////////
TException::~TException()
{
}

/////////////////////////////////////////////////////////////////////////////
// PushError - Push a new error on the error stack.  Do this just before
//             re-throwing the exception.
/////////////////////////////////////////////////////////////////////////////
int TException::PushError(const CString & description, ESeverity kSeverity)
{
	try
	{
		TError error(description, kSeverity);

		return m_errorList.PushError(error);
	}
	catch(...)
	{
		// trap everything. Hopefully we can display the original error
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int TException::PushError(int iStringID, ESeverity kSeverity)
{
	try
	{
		CString str; str.LoadString(iStringID);

		// call sibling
		return PushError(str, kSeverity);
	}
	catch(...)
	{
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// ClearErrors - This will remove all of the errors that have been pushed.
//           Used just before pushing a new error and re-throwing an
//              exception, presumably because the error information that
//           came from below is not relevant.
/////////////////////////////////////////////////////////////////////////////
int TException::ClearErrors()
{
	return m_errorList.ClearErrors();
}

/////////////////////////////////////////////////////////////////////////////
void TException::GetError(CString & description, ESeverity & kSeverity, int index /* = 0 */)
{
	int size = m_errorList.GetSize();

	if (size > 0)
	{
		EErrorClass eDummyClass;
		DWORD       dwError;
		m_errorList.GetErrorByIndex(index, kSeverity, eDummyClass, dwError, description);
	}
}

/////////////////////////////////////////////////////////////////////////////
int TException::HowMany() const
{
	return m_errorList.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
TErrorIterator & TErrorIterator::operator=(const TErrorIterator &rhs)
{
	m_pExcept = rhs.m_pExcept;
	m_index = rhs.m_index;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// FormatString - Takes a resource ID and a string parameter and turns
//                it in to one string.
/////////////////////////////////////////////////////////////////////////////
CString FormatString(UINT nID, LPCTSTR parm)
{
	CString formatString;
	CString result;

	if (!formatString.LoadString(nID))
		return CString("");
	result.Format(formatString, parm);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// CreateIterator - Create an object to iterate over the exception stack.
/////////////////////////////////////////////////////////////////////////////
TErrorIterator TException::CreateIterator()
{
	return TErrorIterator(this);
}

TErrorIterator::~TErrorIterator()
{
}

// ------------------------------------------------------------------------
// Get description from the CException and fill a TException
void FillTException(CException * pCE, TException * pTE, UINT iStringIDOuter)
{
	CString strOuter;

	strOuter.LoadString(iStringIDOuter);

	FillTException(pCE, pTE, strOuter);
}

// ------------------------------------------------------------------------
// created  5-27-98
void FillTException(CException * pCE, TException * pTE, LPCTSTR pszDescOuter)
{
	TCHAR   rcMsg[512];
	CString desc;

	if (!pCE->GetErrorMessage(rcMsg, sizeof(rcMsg)))
	{
		CString str; str.LoadString(IDS_UNKNOWN);
		lstrcpy(rcMsg, str);
	}

	if (pCE->IsKindOf(RUNTIME_CLASS(CFileException)))
	{
		CFileException * pFE = (CFileException *) pCE;
		desc.Format("CFileException : %s (%d)", rcMsg, pFE->m_lOsError);
	}
	else if (pCE->IsKindOf(RUNTIME_CLASS(CArchiveException)))
	{
		CArchiveException * pAE = (CArchiveException *) pCE;
		desc.Format("CArchiveException : (%d)", pAE->m_cause);
	}
	else
		desc.Format("CException : %s", rcMsg);

	pTE->PushError(desc, kFatal);
	pTE->PushError(pszDescOuter, kFatal);
}

// ------------------------------------------------------------------------
// created  5-27-98
void ConvertCExceptionAndThrow(CException * pCE, UINT iStringIDOuter)
{
	TException *pTe = new TException("", kFatal);

	FillTException(pCE, pTe, iStringIDOuter);
	throw pTe;
}

// ------------------------------------------------------------------------
// created  5-27-98
void ConvertCExceptionAndThrow(CException * pCE, LPCTSTR pszOuter)
{
	TException *pTe = new TException("", kFatal);

	FillTException(pCE, pTe, pszOuter);
	throw pTe;
}
