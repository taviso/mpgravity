/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: sysclr.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:56  richard_wood
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
#include "sysclr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// construct
TSystem::TSystem(void)
{
}

// destruct
TSystem::~TSystem(void)
{
}

DWORD TSystem::Highlight(void)
{
   return GetSysColor (COLOR_HIGHLIGHT);
}

DWORD TSystem::HighlightText(void)
{
   return GetSysColor (COLOR_HIGHLIGHTTEXT);
}

DWORD TSystem::Window(void)
{
   return GetSysColor (COLOR_WINDOW);
}

DWORD TSystem::WindowText(void)
{
   return GetSysColor (COLOR_WINDOWTEXT);
}

DWORD TSystem::ColorScrollbar(void)
{
   return GetSysColor (COLOR_SCROLLBAR);
}

// ------------------------------------------------------------------------
// GetMachineInfo -- report on OS, RAM (cpu is kinda hard to get)
//
// Called by :  postbody.cpp
//
int  TSystem::GetMachineInfo (CString & strInfo)
{
   MEMORYSTATUS sMem;
   CString      strMemory;
   DWORD        nMem;
   WORD         wLowBuild;

   sMem.dwLength = sizeof(sMem);

   GlobalMemoryStatus (&sMem);

   nMem = sMem.dwTotalPhys;

   nMem /= 1048576;    // note this is 1024^2

   if (nMem % 2)       // round up
      ++nMem;

   strMemory.Format (_T("%d MB"), nMem);

   OSVERSIONINFO sVer;
   CString strOS;

   sVer.dwOSVersionInfoSize = sizeof(sVer);

   GetVersionEx (&sVer);

   switch (sVer.dwPlatformId)
      {
      case VER_PLATFORM_WIN32_NT:
         strOS.Format (_T("OS: NT %d.%02d build %d %s"),
                       sVer.dwMajorVersion,
                       sVer.dwMinorVersion,
                       sVer.dwBuildNumber,
                       sVer.szCSDVersion);
         break;

      case VER_PLATFORM_WIN32_WINDOWS:
         wLowBuild = LOWORD(sVer.dwBuildNumber);
         strOS.Format (_T("OS: Win9%c %d.%02d.%d %s"),
                       (wLowBuild > 1300) ? '8' : '5',
                       sVer.dwMajorVersion,
                       sVer.dwMinorVersion,
                       wLowBuild,
                       sVer.szCSDVersion);
         break;

      default:
         strOS.Format (_T("OS: Win32s %d.%02d build %d %s"),
                       sVer.dwMajorVersion,
                       sVer.dwMinorVersion,
                       sVer.dwBuildNumber,
                       sVer.szCSDVersion);
         break;
      }

   strInfo = _T("\r\n") + strOS + _T(" ") + strMemory;

   return 0;
}

