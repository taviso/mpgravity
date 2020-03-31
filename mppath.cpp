/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mppath.cpp,v $
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
/*  Revision 1.3  2008/09/19 14:51:31  richard_wood
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
// MicroPlanet Class Library Path Definitions
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mplib.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TPath - default constructor - just constructed from CString
/////////////////////////////////////////////////////////////////////////////
TPath::TPath ()
{
}

/////////////////////////////////////////////////////////////////////////////
// Copy constructor
/////////////////////////////////////////////////////////////////////////////
TPath::TPath (const TPath &rhs) : CString(rhs)
{
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
TPath::TPath (const CString & path) : CString(path)
{
}

/////////////////////////////////////////////////////////////////////////////
// assignment operator
/////////////////////////////////////////////////////////////////////////////
TPath & TPath::operator=(const TPath &rhs)
{
	CString::operator=(rhs);
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
TPath::~TPath ()
{
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
TPath::TPath (const CString & directory,
			  const CString & file) : CString(directory)
{
	// ???? doesn't handle MBCS
	TPath path(directory);
	path.AddTermChar (_T('\\'));
	path += file;
	*this = path;
}

/////////////////////////////////////////////////////////////////////////////
// replace existing path w/ new one
/////////////////////////////////////////////////////////////////////////////

void TPath::FormPath(const CString & directory,
					 const CString & file)
{
	*this = directory;
	AddBackSlash();
	*this += file;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void TPath::GetDirectory (TPath & directory)
{
	int i;      // ???? doesn't handle MBCS
	CString str;

	// search back until the last backslash and copy everything before it
	i = ReverseFind(_T('\\'));
	str = Left (i);                // copy everything to the left of backslash
	directory = str;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void TPath::GetFile (TPath &file)
{
	int      i;         // ???? doesn't handle MBCS
	CString  str;

	i = ReverseFind(_T('\\'));
	str = Mid (i + 1);      // copy everything to the right of backslash
	file = str;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void TPath::AddTermChar (TCHAR c)
{  // ???? doesn't handle MBCS yet
	// check to see if it is already there
	if (!IsEmpty())
	{
		if (GetAt(GetLength() - 1) != c)
			*this += c;
	}
	else
		*this += c;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void TPath::AddBackSlash ()
{  // ???? doesn't handle MBCS yet
	// check to see if it is already there
	if (!IsEmpty())
	{
		if (GetAt(GetLength() - 1) != _T('\\'))
			*this += _T('\\');
	}
	else
		*this += _T('\\');
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void TPath::RemoveTermChar (TCHAR c)
{
	if (IsEmpty()) return;
	CString str;
	int i = GetLength() - 1;
	if (GetAt(i) == _T('\\'))
		str= Left (i - 1);
	*this = str;
}

// ------------------------------------------------------------------------
// Class-wide global
// swaps out illegal longfilename chars
// illegal filename characters, must be replaced
static TCHAR   illegalChars[] = {'\\', '/', ':', '*', '?', '<', '>', '|'};
static TCHAR   replaceChars[] = {'$' , '%','\'', '-', '_', '@', '{', '}'};

// -------------------------------------------------------------------------
CString TPath::GenLegalFileName (LPCTSTR junk)
{
	// look for illegal characters and replace them
	CString  goodName = junk;
	int      pos;
	for (int i = 0; i < sizeof (illegalChars)/sizeof (illegalChars[0]); i++)
	{
		while ((pos = (goodName.Find (illegalChars[i]))) != -1)
			goodName.SetAt (pos, replaceChars[i]);
	}

	return goodName;
}
