/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: dbutil.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:20  richard_wood
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

// dbutil.cpp - routines for serializing objects

#include "stdafx.h"
//#include "ssutil.h"
#include "mplib.h"
#include "resource.h"
#include "dbutil.h"
#include "tmsgbx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern HWND ghwndMainFrame;

/////////////////////////////////////////////////////////////////////////////
// DBError1 - put message box constructed from string table and 1 parm
/////////////////////////////////////////////////////////////////////////////

static void DBError1 (UINT strID, LPCTSTR pArg1)
{
	CString error;
	error.Format (strID, pArg1);
	NewsMessageBox (ghwndMainFrame, error);
}

/////////////////////////////////////////////////////////////////////////////
// DBError1 - put message box constructed from string table and 2 parms
/////////////////////////////////////////////////////////////////////////////

static void DBError2 (UINT strID, LPCTSTR pArg1, LPCTSTR pArg2)
{
	CString error;
	error.Format (strID, pArg1, pArg2);
	NewsMessageBox (ghwndMainFrame, error);
}

/////////////////////////////////////////////////////////////////////////////
// ArcReadWrite - reads or writes one or more objects from an archive
/////////////////////////////////////////////////////////////////////////////

static bool ArcReadWrite (CArchive & arc,
						  CObject  * pOb,
						  CObject  * pObArray[])
{
	try
	{
		// write out the new file
		if (pObArray)
		{
			int i = 0;

			while (pObArray[i])
				pObArray[i++]->Serialize (arc);
		}
		else
			pOb->Serialize (arc);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// DBSave - Serialize one or more objects to a file.
/////////////////////////////////////////////////////////////////////////////

bool DBSave (CObject *       pOb,
			 const CString & path,
			 const CString & name,
			 CObject *       pObArray[])
{
	TPath             backup (path, name);
	TPath             filePath (path, name);
	CFile             newFile;
	CFileStatus       status;
	CFileException    sFE;
	bool              fBackup  = false;

	// add the .bak extension to the backup file name

	backup += ".bak";

	// if there is a backup file, remove it
	if (CFile::GetStatus ( backup, status ))
	{
		try
		{
			CFile::Remove (backup);
		}
		catch(...)
		{
			DBError1 (IDS_ERROR_REMOVING_BACKUP, backup);
			return false;
		}

	}

	// if a current file exists, rename it w/ the backup extension

	if (CFile::GetStatus (filePath, status))
	{
		try
		{
			CFile::Rename (filePath, backup);
			fBackup = true;
		}
		catch(...)
		{
			DBError1 (IDS_ERROR_RENAMING_BACKUP, filePath);
			return false;
		}
	}

	// create the new file - doesn't throw an exception

	BOOL fFileOpen = newFile.Open (filePath,
		CFile::modeCreate |
		CFile::modeReadWrite |
		CFile::shareExclusive,
		&sFE);

	if (!fFileOpen)
	{
		sFE.ReportError ();
		DBError1 (IDS_ERROR_CREATING_DBFILE, filePath);
		return false;
	}

	CArchive arcWrite (&newFile, CArchive::store);

	bool fWritten = ArcReadWrite (arcWrite, pOb, pObArray);

	if (fWritten)
	{
		try   // cleanup
		{
			// close the file
			arcWrite.Flush();
			arcWrite.Close ();
			newFile.Close();

		}
		catch(...)
		{
			DBError1 (IDS_ERROR_SAVING_OBJECTS_TO, filePath);
			return false;
		}
	}
	else
	{
		arcWrite.Abort();
		// ??? error message
	}

	// remove the backup file - we're good to go
	try
	{
		if (fBackup)
			CFile::Remove (backup);
	}
	catch(...)
	{
		DBError1 (IDS_ERROR_REMOVING_BACKUP_FINAL, backup);
		return false;
	}

	return fWritten;
}

/////////////////////////////////////////////////////////////////////////////
// DBLoad - De-serialize from a file.
/////////////////////////////////////////////////////////////////////////////

bool DBLoad (bool       fReportMissingAsTrue,
			 CObject *  pOb,
			 const CString & path,
			 const CString & name,
			 CObject *  pObArray[])
{
	TPath             backup (path, name);
	TPath             target (path, name);
	CFile             targetFile;
	CFileStatus       status;
	CFileException    sFE;
	BOOL              fFileOpen  = false;

	backup += ".bak";

	// if there is a backup file, we should
	// remove the target, rename the backup to the
	// target, and then open it

	if (CFile::GetStatus ( backup, status ))
	{
		DBError1 (IDS_BACKUP_FOUND_REVERTING, backup);
		try
		{
			if (CFile::GetStatus (target, status))
				CFile::Remove (target);
		}
		catch(...)
		{
			DBError1 (IDS_ERROR_REMOVING_DURING_REVERT, target);
			return false;
		}

		try
		{
			CFile::Rename (backup, target);
		}
		catch(...)
		{
			DBError2 (IDS_ERROR_RENAMING_DURING_REVERT, backup, target);
			return false;
		}
	}

	// open the file
	fFileOpen = targetFile.Open (target,
		CFile::modeRead|
		CFile::shareExclusive,
		&sFE);

	if (!CFile::GetStatus (target, status))
		if (fReportMissingAsTrue)
			return true;

	if (!fFileOpen)
	{
		sFE.ReportError ();
		DBError1 (IDS_ERROR_OPENING_OBJECTS, target);
		return false;
	}

	// hook an archive to it

	CArchive arcRead (&targetFile, CArchive::load);

	bool fFileRead = ArcReadWrite (arcRead, pOb, pObArray);

	if (fFileRead)
	{
		try
		{
			arcRead.Close();
			targetFile.Close();
		}
		catch(...)
		{
			DBError1 (IDS_ERROR_READING_OBJECTS, target);
			return false;
		}
	}
	else
	{
		arcRead.Abort();
		DBError1 (IDS_ERROR_READING_OBJECTS, target);
		return false;
	}

	return fFileRead;
}

/////////////////////////////////////////////////////////////////////////////
//

