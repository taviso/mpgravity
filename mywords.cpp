/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mywords.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.5  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.4  2009/08/16 21:05:38  richard_wood
/*  Changes for V2.9.7
/*
/*  Revision 1.3  2009/07/08 18:32:32  richard_wood
/*  Fixed lots of new installer bugs, spell checker dialog bug, updated the vcredist file to 2008 SP1 version, plus lots of other bug fixes.
/*
/*  Revision 1.2  2009/06/11 21:10:12  richard_wood
/*  Upgraded to VS2008.
/*  Changed from Inno installer to MS VS2008 installer.
/*  Added online installer + offline installer.
/*  Improved spell checker.
/*  Bug fix for initial setup dialog.
/*  Improvements to ROT13.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2008/09/19 14:51:32  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

#include "stdafx.h"

#include "mywords.h"
#include "mplib.h"
#include "genutil.h"
#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TMyWords::TMyWords()
{
	m_fNeedSave = false;

	CString strDir = TFileUtil::GetAppData()+"\\Gravity\\";
	TPath   p0(strDir, "spell");
	TPath   path(p0, "mywords.lst");

	CFileStatus sFS;
	if (CFile::GetStatus(path, sFS))
	{
		CStdioFile fl(path, CFile::modeRead | CFile::typeText);
		CString word;
		while ( fl.ReadString(word))
		{
			word.TrimRight();
			if (word.IsEmpty() == FALSE)
				m_set.insert(word);
		}
		fl.Close();
	}
}

TMyWords::~TMyWords()
{
	Save();
}

void TMyWords::Save()
{
	try
	{
		if (m_fNeedSave)
		{
			CString strDir = TFileUtil::GetAppData()+"\\Gravity\\";
			TPath   p0(strDir, "spell");
			TPath   path(p0, "mywords.lst");

			TPath   path2(p0, "mywords.lst.bak");

			CFileStatus sFS;
			if (CFile::GetStatus(path2, sFS))
				DeleteFile (path2);

			if (CFile::GetStatus(path, sFS))
				MoveFile (path, path2);

			CStdioFile fl(path, CFile::modeCreate |  CFile::modeWrite | CFile::typeText);

			set<CString>::iterator it = m_set.begin();
			for (; it != m_set.end(); it++)
			{
				fl.WriteString ( (*it) + "\n" );
			}

			fl.Close();

			if (CFile::GetStatus(path2, sFS))
				DeleteFile (path2);

			m_fNeedSave = false;
		}
	}
	catch(...)
	{
	}
}

bool TMyWords::Lookup (const CString & word)
{
	if (m_set.end() == m_set.find (word))
	{
		return false;
	}
	return true;
}

void TMyWords::AddWord (const CString & word)
{
	m_set.insert ( word );
	m_fNeedSave = true;
}

void TMyWords::RemoveWord (const CString & word)
{
	m_set.erase ( word );
	m_fNeedSave = true;
}

void TMyWords::FillListbox (CListBox & lbx)
{
	set<CString>::iterator it = m_set.begin();
	for (; it != m_set.end(); it++)
	{
		lbx.AddString ( *it );
	}
}
