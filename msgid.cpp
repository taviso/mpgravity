/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: msgid.cpp,v $
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
/*  Revision 1.3  2009/01/28 14:53:38  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:32  richard_wood
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
#include "msgid.h"
#include "globals.h"
#include "tidarray.h"               // RememberMessageID ()
#include <time.h>
#include "server.h"     // TNewsServer
#include "newsdb.h"     // TNewsDB

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

unsigned int CRC32(unsigned char * data, int len);

BOOL GenerateMessageID (
   LPCTSTR szUserName,
   LPCTSTR hostName,
   int     id,
   CString& result
)
{
   TServerCountedPtr cpNewsServer;
   CString hexTime;
   CString userName = szUserName;

   // '<' MPG.{time}{crc}{counter}@{hostname} '>'
   CTime now = CTime::GetCurrentTime();
   CTime bday(1989, 12, 25, 10, 0, 0);
   time_t  uNow = now.GetTime();
   time_t  uBday = bday.GetTime();

   // hack off some hi digits?
   if (uNow > uBday)
      uNow -= uBday;

   hexTime.Format("%x", uNow);

   result = "<MPG.";
   result += hexTime;

   CString stub = hexTime + userName.SpanExcluding("@");
   UINT uCRC = CRC32((LPBYTE)(LPCTSTR) stub, stub.GetLength());
   stub.Format("%x", uCRC);

   result += stub;

   // get counter - convert to hex
   int count = id;
   if (0 == count)
      count = cpNewsServer->NextPostID ();
   userName.Format("%x", count);
   result += userName;

   result += '@';
   result += hostName;
   result += '>';

   // add this article's message ID to the global list
   (gpStore->GetIDArray ()).RememberMessageID (result);

   return TRUE;
}
