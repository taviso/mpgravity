///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author: Elm� (http://netcult.ch/elmue)
// Date: 5-12-2007
//
// Filename: File.hpp
//
// Classes:
// - CFile
//
// Purpose: Some utilities for file and directoy operations
//

#pragma once


#include <shlobj.h> // IShellLink, Ole
#include "Defines.h"
#include "String.hpp"

#pragma warning(disable: 4996)

namespace Cabinet
{

// -------------------------------------------------------------
// The following defines are used for compression AND extraction
// -------------------------------------------------------------

class CFile
{
public:
	static UINT CreateFolderTreeW(WCHAR* u16_Folder)
	{
		if (CheckDirectoryExistsW(u16_Folder))
			return 0;

		WCHAR  u16_Parent[1000];
		wcscpy(u16_Parent, u16_Folder);

		RemovePathTerminationW(u16_Parent);

		// Cut the last Folder from the tree
		WCHAR* u16_Pos = wcsrchr(u16_Parent, '\\');
		if (u16_Pos > 0) u16_Pos[0] = 0;

		// recursively create all the parent folders (if necessary)
		UINT u32_Error = CreateFolderTreeW(u16_Parent);
		if  (u32_Error)
			return u32_Error;

		if (!CreateDirectoryW(u16_Folder, 0))
			return GetLastError();

		return 0;
	}

	static BOOL CheckDirectoryExistsW(WCHAR* u16_Folder)
	{
		UINT u32_Attr = GetFileAttributesW(u16_Folder);

		if (u32_Attr == 0xFFFFFFFF)
			return FALSE;

		return (u32_Attr & FILE_ATTRIBUTE_DIRECTORY) > 0;
	}

	static void TerminatePathW(CStrW& s_Path)
	{
		int s32_Len = s_Path.Len();
		if (s32_Len > 1 && s_Path[s32_Len-1] != '\\')
		{
			s_Path += L"\\";
		}
	}

	static void RemovePathTerminationW(WCHAR* u16_Path)
	{
		int s32_Len = (int)wcslen(u16_Path);

		if (s32_Len > 1 && u16_Path[s32_Len-1] == '\\')
		{
			u16_Path[s32_Len-1] = 0;
		}
	}

	// Splits a path like "C:\Test\File.cab" into 
	// the folder "C:\Test\" and
	// the file   "File.cab"
	// optionally both returned strings may be null
	static void SplitPathW(CStrW s_Path, CStrW* ps_Folder, CStrW* ps_File)
	{
		WCHAR* u16_Slash = wcsrchr(s_Path, '\\');
		if (u16_Slash == 0)
		{
			if (ps_Folder) *ps_Folder = L"";
			if (ps_File)   *ps_File   = s_Path;
		}
		else
		{
			u16_Slash ++;
			if (ps_File) *ps_File = u16_Slash;
			
			u16_Slash[0] = 0;
			if (ps_Folder) *ps_Folder = s_Path;
		}
	}

	// returns the target file which is stored in a shortcut ".LNK"
	// returns error code or 0
	static HRESULT ResolveLnkShortcutW(WCHAR* u16_LinkFile, WCHAR* u16_Target, DWORD u32_BufSize)
	{
		// CoInitializeEx returns S_FALSE or RPC_E_CHANGED_MODE if Ole is already initialized
		HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE)
			return hr;

		IShellLinkW*  i_ShLink;	
		IPersistFile* i_Persist;

		// Get a pointer to the IShellLink interface.
		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&i_ShLink);
		if (SUCCEEDED(hr))
		{
			hr = i_ShLink->QueryInterface(IID_IPersistFile, (void**)&i_Persist);
			if (SUCCEEDED(hr))
			{
				hr = i_Persist->Load(u16_LinkFile, STGM_READ);
				if (SUCCEEDED(hr))
				{
					hr = i_ShLink->Resolve(0, SLR_NO_UI|SLR_NOSEARCH|SLR_NOTRACK|SLR_NOUPDATE|SLR_NOLINKINFO);
					if (SUCCEEDED(hr))
					{
						hr = i_ShLink->GetPath(u16_Target, u32_BufSize, 0, 0);
					}
				}
				i_Persist->Release();
			}
			i_ShLink->Release();
		}
		return hr;
	}

	// returns the target file which is stored in a shortcut ".URL"
	// ".URL" files have the format of a Windows INI file
	// returns error code or 0
	static DWORD ResolveUrlShortcutW(WCHAR* u16_LinkFile, WCHAR* u16_Url, DWORD u32_BufSize)
	{
		DWORD u32_Len = GetPrivateProfileStringW(L"InternetShortcut", L"URL", L"", u16_Url, u32_BufSize, u16_LinkFile);
		if (u32_Len >= u32_BufSize - 3)
			return ERROR_BUFFER_OVERFLOW;

		return 0;
	}
};

} // Namespace Cabinet

