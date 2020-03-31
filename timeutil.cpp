/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: timeutil.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:08  richard_wood
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
#include "time.h"
#include "timeutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static BYTE gfTimeZoneInit;
static TIME_ZONE_INFORMATION sZone;
static CTimeSpan spanBias;

static void InitTimeZone(void)
{
   gfTimeZoneInit = TRUE;

   DWORD dwRet = GetTimeZoneInformation ( &sZone );
   if (dwRet != -1)
      {
      // this is a count of minutes
      LONG minutes;
      if (TIME_ZONE_ID_STANDARD == dwRet)
         minutes = sZone.StandardBias + sZone.Bias;
      else if (TIME_ZONE_ID_DAYLIGHT == dwRet)
         minutes = sZone.DaylightBias + sZone.Bias;
      else
         minutes = sZone.Bias;

      // utc = local + bias
      // local = utc - bias;

      CTimeSpan tmpBias(0, minutes/60, minutes%60, 0);
      spanBias = tmpBias;
      }
   else
      ASSERT(0);
}

void GetGmtTime(CTime& time)
{
   if (!gfTimeZoneInit)
      InitTimeZone();

#if defined(_DEBUG_EXTREME)
      CString res1 = time.Format ("%m %d %Y %H:%M:%S");
      afxDump << res1 << "\n";
#endif

   // utc = local + bias
   time += spanBias;
}

// Pass in a GMT time, and you get a local time
void GetLocalTime(
  const CTime& gmtTime,
  CTime& localTime            // output
)
{
   if (!gfTimeZoneInit)
      InitTimeZone();

#if defined(_DEBUG_EXTREME)
      afxDump << gmtTime.Format ("%m %d %Y %H:%M:%S") << "\n";
#endif
   // local = utc - bias
   localTime = gmtTime - spanBias;

#if defined(_DEBUG_EXTREME)
      afxDump << localTime.Format ("%m %d %Y %H:%M:%S") << "\n\n";
#endif
}

//   enum EDateFormat { kFormatDate=0, kFormatDateTime24=1, kFormatDateTimePM=2 };
//  3-31 remove leading zero on Month - use '#'
void GetCustomFormat(const CString& fmt,
                     const CTime& time, CString& output)
{
   LPCTSTR lpszFmt = fmt;

   output = time.Format(lpszFmt);
}

