/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mplib.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/18 22:05:02  richard_wood
/*  Refactored XOVER and XHDR commands so they fetch item data in batches of 300 (or less) if we want > 300 articles.
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
// MicroPlanet Class Library Definitions
//
//   TError
//   TErrorIterator
//   TErrorList
//   TException
//      TMemoryException
//      TSocketException
//   TPath
//   TRange
//   TRangeSet
//
//   TRelevance
//   TRelevanceSet
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <afxtempl.h>

// disable warnings about "unreferenced formal parameter"
#pragma warning ( disable : 4100 )

#define LINE_TRACE() TRACE("%s : %d\n", __FILE__, __LINE__)

enum ESeverity {kInfo, kWarning, kError, kFatal};

//                  Example
// kClassUnknown  - default
// kClassInternal - argument is null when it shouldn't be/LoadString failed
// kClassResource - out of memory, out of window handles.
// kClassFile     - File error/ out of disk space
// kClassExternal - for some reason an Win32 function failed
// kClassNNTP     - NNTP error code
// kClassSMTP     - SMTP error code
// kClassWinsock  - line dropped, couldn't connect etc.
// kClassUser     - group suddenly unsubscribed
// kClassSecurity - SSL security error

enum EErrorClass
{
	kClassUnknown,
	kClassInternal,
	kClassResource,
	kClassFile,
	kClassExternal,
	kClassNNTP,
	kClassSMTP,
	kClassWinsock,
	kClassUser,
	kClassSecurity
};

/////////////////////////////////////////////////////////////////////////////
// TError - Error type that contains an error string and a severity code.
/////////////////////////////////////////////////////////////////////////////

class TError : public CObject
{
public:
	TError ();
	TError (const TError &);
	TError &operator=(const TError &);
	~TError ();
	TError (const CString & description, ESeverity kSeverity,
		EErrorClass eClass = kClassUnknown);

	TError (int iStringID, ESeverity kSeverity, EErrorClass kClass);
	TError (ESeverity kSeverity, EErrorClass kClass, DWORD dwError);

	// accessors
	void GetDescription (CString & description);
	void GetSeverity (ESeverity & kSeverity);
	void GetClass (EErrorClass & kClass);
	DWORD GetDWError (void) { return m_dwError; }
	void SetDWError (DWORD err) { m_dwError = err; }

	void Description (const CString & desc) { m_description = desc; }

	// ???? add dialog options when we figure out what they are
private:
	ESeverity   m_kSeverity;
	EErrorClass m_eClass;

	// if m_dwError==0 , then m_description must be filled
	DWORD       m_dwError;
	CString     m_description;
};

class TException;

class TErrorIterator
{
public:
	TErrorIterator (TException *pExcept) {m_index = 0; m_pExcept = pExcept;}
	~TErrorIterator ();
	TErrorIterator & operator=(const TErrorIterator &);
	int Next (CString & errString, ESeverity & kSeverity);
	int Reset ();
private:
	int         m_index;
	TException *m_pExcept;
};

/////////////////////////////////////////////////////////////////////////////
// TErrorList - holds a bunch of errors. A utility class. Kind of Abstract
class TErrorList : public CObject
{
public:
	TErrorList();
	~TErrorList();
	TErrorList (const TErrorList & src);
	TErrorList & operator= (const TErrorList & rhs);

	int PushError (TError & sError);
	int ClearErrors();
	int GetSize ()    const
	{ return m_errVec.GetSize(); }

	void GetErrorByIndex (int index, ESeverity & kSeverity, EErrorClass & eClass,
		DWORD & dwError, CString & description);

protected:
	CArray<TError, TError &> m_errVec;     // error Vector
};

/////////////////////////////////////////////////////////////////////////////
// TException - Exception type derived from the MFC CException
//              type.  Contains a list of error information that is passed
//              in via Push.
/////////////////////////////////////////////////////////////////////////////
//void DestructElements( TError * pItems, int nCount );
//void ConstructElements( TError * pItems, int nCount );

// 4673 -- throwing 'class TException' the following types will not be considered at the catch site
// 4671 -- 'TException::CException' : the copy constructor is inaccessible
#pragma warning (disable : 4673 4671)

class TException : public CException
{
	friend class TErrorIterator;

	// ???? - fix leak

public:

	TException ();
	TException (const TException &);
	TException &operator=(const TException &);
	~TException();
	TException (const CString &description, const ESeverity &kSeverity);
	TException (int iStringID, const ESeverity &kSeverity);

	// need stuff for taking the work out of handling resource stuff ????

	virtual TErrorIterator CreateIterator ();
	virtual int Display ();
	virtual int PushError (const CString & error, ESeverity kSeverity);
	virtual int PushError (int iStringID, ESeverity kSeverity);
	virtual int ClearErrors();
	virtual int HowMany ()  const;
	virtual void GetError (CString & description, ESeverity & kSeverity, int index = 0);

private:

	TErrorList  m_errorList;
};

CString FormatString (UINT nID, LPCTSTR parameter);
void FillTException (CException * pCE, TException * pTE, UINT iStringIDOuter);
void FillTException (CException * pCE, TException * pTE, LPCTSTR pszOuter);
void ConvertCExceptionAndThrow (CException * pCE, UINT iStringIDOuter);
void ConvertCExceptionAndThrow (CException * pCE, LPCTSTR pszOuter);

/////////////////////////////////////////////////////////////////////////////
// TMemoryException
/////////////////////////////////////////////////////////////////////////////
class TMemoryException : public TException {
public:
	TMemoryException (void) { }
	TMemoryException (const CString &description,
		const ESeverity & kSeverity = kFatal)
		: TException(description, kFatal)
	{
	}

	TMemoryException (const TMemoryException &e)
		: TException(e)
	{
	}

	TMemoryException &operator=(const TMemoryException &e)
	{
		TException::operator=(e);
		return *this;
	}

	~TMemoryException()  { }
};

class TSocketException : public TException {
public:
	TSocketException (void) { }
	TSocketException (int iLastError,
		const CString &description,
		const ESeverity & kSeverity = kFatal)
		: TException(description, kFatal), m_iLastError(iLastError)
	{
	}

	TSocketException (const TSocketException &src)
		: TException(src)
	{
		m_iLastError = src.m_iLastError;
	}

	TSocketException &operator=(const TSocketException &rhs)
	{
		TException::operator=(rhs);
		m_iLastError = rhs.m_iLastError;
		return *this;
	}

	~TSocketException()  { }

	int m_iLastError;
};

class TSocketInitPostException : public TSocketException {
public:
	TSocketInitPostException (CString & description, ESeverity & kSeverity)
		: TSocketException(2000, description, kSeverity) { }
};

/////////////////////////////////////////////////////////////////////////////
// TPath - Type for dealing with paths - break them into dir + file, etc...
/////////////////////////////////////////////////////////////////////////////

class TPath : public CString
{
public:
	TPath ();                                 // default constructor
	TPath (const TPath & rhs);                // copy constructor
	TPath (const CString & path);             // construct from cstring
	TPath &operator=(const TPath & rhs);      // assignment operator
	~TPath ();
	TPath (const CString & directory,         // construct from directory and file
		const CString & file);
	void GetDirectory (TPath & directory);    // get the directory from the path
	void GetFile (TPath & file);              // get the file from the path
	void AddTermChar (TCHAR   c);             // add a terminating character
	void AddBackSlash ();                     // add a terminating character
	void RemoveTermChar (TCHAR c);            // remove a terminating character
	void FormPath(const CString & directory,  // replace existing path w/ new one
		const CString & file);

	// handles multiple file names from struct OPENFILENAME
	static void MakeList(LPCTSTR string,  CStringList& list);

	// swaps out illegal longfilename chars
	static CString GenLegalFileName (LPCTSTR junk);

private:
	// nothing goes here, it's all private to CString
};

/////////////////////////////////////////////////////////////////////////////
// TRange - Used to represent a range of integers.
/////////////////////////////////////////////////////////////////////////////

typedef LONG   RANGEINT;

class TRange : public CObject
{
public:
	DECLARE_SERIAL(TRange)
	TRange () {}
	TRange (RANGEINT lowerAndUpper) {m_lower = m_upper = lowerAndUpper;}
	TRange (RANGEINT lower, RANGEINT upper) {m_lower = lower; m_upper = upper;}
	TRange & operator=(const TRange &);
	TRange (const TRange& src);

	~TRange () {}

	RANGEINT Upper () const
	{return m_upper;}

	RANGEINT Lower () const
	{return m_lower;}

	void SetUpper (RANGEINT upper) {m_upper = upper;}
	void SetLower (RANGEINT lower) {m_lower = lower;}
	void Serialize (CArchive & archive);

#if defined(_DEBUG)
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	RANGEINT m_lower;
	RANGEINT m_upper;
};

/////////////////////////////////////////////////////////////////////////////
// TRangeSet - Used to hold ranges of integers.
/////////////////////////////////////////////////////////////////////////////

template <> void AFXAPI
SerializeElements <TRange> (CArchive & ar, TRange *pItem, int nCount);

typedef CArray<TRange, TRange &> RangeList;

class TRangeSet : public CObject
{
public:
	TRangeSet& operator= (const TRangeSet& rhs);

	BOOL Have (RANGEINT iNum);
	void Add (RANGEINT iNum);
	void Add (RANGEINT iLow, RANGEINT iHigh);
	void Serialize (CArchive & ar);
	void Format (CString & str);
	void Subtract (TRangeSet& sub, TRangeSet& result);
	void Subtract (RANGEINT iNum);
	int  RangeCount () const;
	void GetRange (int idx, int& low, int& hi) const;
	BOOL IsEmpty ();
	BOOL RemoveHead (RANGEINT *piReturn);
	void Empty ();
	int  CountItems ();

	// erase any range sets that are below iLimit.  Does not split a range
	void DeleteRangesBelow (int iLimit);

	// get printable human readable form
	int  toString (CString & txt);

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
//	void DebugPrint();

private:
	void Sort ();
	void MergeAll ();
	int FallsIn (RANGEINT iNum);
	int FallsBelow (RANGEINT iNum);
	void Merge (int iIndex);

	RangeList m_rangeList;
};

BOOL RangeSetOK (TRangeSet &sRangeSet);   // for debugging

/////////////////////////////////////////////////////////////////////////////
// Relevance - An association between a string and an integer value.
/////////////////////////////////////////////////////////////////////////////

typedef LONG REL_VALUE;

class TRelevance : public CObject
{
public:
	DECLARE_SERIAL(TRelevance)
	TRelevance () {m_value = 0;}
	TRelevance & operator=(const TRelevance &);
	~TRelevance () {}
	void Serialize (CArchive & archive);

	REL_VALUE GetValue () {return m_value;}
	CString & GetKeyword () {return m_keyword;}

	void Set (const CString & keyword,
		REL_VALUE       value) {m_keyword = keyword; m_value = value;}

private:
	CString     m_keyword;
	REL_VALUE   m_value;
};

template <> void AFXAPI SerializeElements <TRelevance>
(CArchive & ar, TRelevance *pItem, int nCount);

typedef CArray<TRelevance, TRelevance &> RelevanceList;

/////////////////////////////////////////////////////////////////////////////
// TRelevanceSet - Used to hold Relevances of integers.
/////////////////////////////////////////////////////////////////////////////

class TRelevanceSet : public CObject
{
public:
	TRelevanceSet () {}
	~TRelevanceSet () {}

	void        Add (const CString & keyword,
		REL_VALUE       value);
	void        Del (const CString & keyword);
	REL_VALUE   Get (const CString & keyword);
	void        Set (const CString & keyword, REL_VALUE value);
	TRelevance  &operator[] (int pos);
	int         GetSize () {return m_set.GetSize();}

private:
	RelevanceList m_set;
};

//*************************************************************************//
// TRelevanceIterator - Iterate over a relevance set.
//*************************************************************************//

class TRelevanceIterator : public CObject
{
public:
	TRelevanceIterator (TRelevanceSet & relset):
	  m_index(0), m_relSet (relset) {}
	  BOOL operator() (TRelevance *relevance);

private:
	int               m_index;
	TRelevanceSet  &  m_relSet;
};

#if defined(GOOBER)
class TOSVersion
{
public:
	typedef enum {kWin32s, kWin95, kWinNT} EOSType;

public:
	// returns TRUE for
	static TOSVersion::EOSType Get(UINT* major, UINT*  minor, UINT* build);
};
#endif

typedef const LOGFONT * LPCLOGFONT;
