/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ssutil.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:54  richard_wood
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

// ssutil.cpp - structured storage utility routines...

#include "stdafx.h"
#include "ssutil.h"
#include "mplib.h"
#include <winnls.h>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// StreamExist - Test if a stream exists within a storage...
/////////////////////////////////////////////////////////////////////////////

BOOL StreamExist (LPSTORAGE pIOpenStorage, LPCTSTR stream)

{
	BOOL              fExists;
	COleStreamFile    testStream;

	fExists = testStream.OpenStream (pIOpenStorage, stream);
	if (fExists)
		testStream.Close ();

	return fExists;
}

/////////////////////////////////////////////////////////////////////////////
// FormWideString - Given a LPCTSTR and a pointer to a wide character
//                  buffer, form the wide character version in
//                  preparation for handing to OLE
/////////////////////////////////////////////////////////////////////////////

void FormWideString (LPCTSTR  normal, LPWSTR wide, int width)

{

	// May 9 2002  change from CP_OEMCP to CP_ACP

	MultiByteToWideChar (CP_ACP,           // ANSI code page incoming...
		MB_PRECOMPOSED,   // translation flags...
		normal,           // multi-byte string...
		-1,               // calculate from NULL terminated
		wide,             // pointer to wide char buffer
		width);           // width in wide chars
}

/////////////////////////////////////////////////////////////////////////////
// LoadSSObject - Load an object from an structured storage stream (must
//                be in the root storage.
/////////////////////////////////////////////////////////////////////////////

void  LoadSSObject (LPSTORAGE    pIStorage,
					CObject *    pOb,
					LPCTSTR      name,
					CObject *    pObArray[])
{
	if (!pIStorage)
		throw(new TSSNoStorage (FormatString(IDS_ERR_OLE_NOSTORAGE, ""), kError));

	COleStreamFile fileToRead;
	CFileException sFE;

	if (!fileToRead.OpenStream (pIStorage,
		name,
		CFile::modeReadWrite | CFile::shareExclusive,
		&sFE))
	{
		CString errMsg;
		errMsg.Format (IDS_ERR_OPENSTREAM2, sFE.m_lOsError, name);

		throw(new TSSErrorOpening (errMsg, kError));
	}

	CArchive arcRead (&fileToRead, CArchive::load, 4096); // bufsize

	bool fArchiveOpen = true;
	try
	{
		if (pObArray)
		{
			int      i = 0;

			for (i = 0; pObArray[i] != NULL; i++)
				pObArray[i]->Serialize (arcRead);
		}
		else
			pOb->Serialize (arcRead);

		arcRead.Close ();
		fArchiveOpen = false;
		fileToRead.Close ();
	}
	catch (TException *te)
	{
		if (fArchiveOpen) 
			arcRead.Abort();
		CString str; str.Format (IDS_ERR_DESERIALIZING, name);
		te->PushError (str, kError);
		TException *ex = new TException(*te);
		te->Delete();
		throw(ex);
	}
	catch(...)
	{
		if (fArchiveOpen) 
			arcRead.Abort();
		CString str; str.Format (IDS_ERR_DESERIALIZING, name);
		throw(new TException (str, kError));
	}
}

/////////////////////////////////////////////////////////////////////////////
// SaveSSObject - Save an object to a structured storage stream (must
//                be in the root storage).
/////////////////////////////////////////////////////////////////////////////

void  SaveSSObject(LPSTORAGE  pIStorage,
				   CObject *   pOb,
				   LPCTSTR     name,
				   CObject *   pObArray[])

{
	if (!pIStorage)
		throw(new TSSNoStorage (FormatString(IDS_ERR_OLE_NOSTORAGE, ""), kError));

	COleStreamFile    newFile;
	CFileException    sFE;

	if (StreamExist (pIStorage, name))
	{
		if (!newFile.OpenStream (pIStorage,
			name,
			CFile::modeReadWrite | CFile::shareExclusive,
			&sFE))
		{
			CString errMsg;
			errMsg.Format (IDS_ERR_OPENSTREAM2, sFE.m_lOsError, name);

			throw(new TSSErrorOpening (errMsg, kError));
		}
	}
	else
	{
		if (!newFile.CreateStream (pIStorage,
			name,
			CFile::modeReadWrite | CFile::shareExclusive | CFile::modeCreate,
			&sFE))
		{
			CString errMsg;
			errMsg.Format (IDS_ERR_CREATESTREAM2, sFE.m_lOsError, name);

			throw(new TSSErrorCreating (errMsg, kError));
		}
	}

	CArchive arcWrite (&newFile, CArchive::store);
	bool fArcOpen = true;
	try
	{
		if (pObArray)
		{
			int      i = 0;

			for (i = 0; pObArray[i] != NULL; i++)
				pObArray[i]->Serialize (arcWrite);
		}
		else
			pOb->Serialize (arcWrite);
		arcWrite.Flush ();
		arcWrite.Close ();
		fArcOpen = false;
		newFile.SetLength (newFile.Seek(0, CFile::current));
		newFile.Close ();
	}
	catch(...)
	{
		if (fArcOpen)
			arcWrite.Abort();
		CString str; str.Format (IDS_ERR_SERIALIZING, name);
		throw(new TException (str, kError));
	}
}

/////////////////////////////////////////////////////////////////////////////
// DeleteStream - delete a stream from the structured storage file
//
void  DeleteStream ( LPSTORAGE      pIStorage,
					LPCTSTR        name )
{
	HRESULT hr;
	try
	{
		WCHAR wideName[512];
		FormWideString (name, wideName, sizeof(wideName)/sizeof(WCHAR));

		hr = pIStorage -> DestroyElement ( wideName );

		ASSERT(S_OK == hr);
	}
	catch(...)
	{

	}
}

