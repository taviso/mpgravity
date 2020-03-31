/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: boyerc.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:14  richard_wood
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
#include "boyerc.h"
//#include "mplib.h"
#include <tchar.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
TBoyerMoore::TBoyerMoore (LPCTSTR pattern, BOOL fCaseSensitive)

{
   pat = 0;
   m_fCaseSensitive = fCaseSensitive;
   SetPattern (pattern);
}

// -------------------------------------------------------------------------
// Destructor
TBoyerMoore::~TBoyerMoore ()
{
   delete [] pat;
}

// -------------------------------------------------------------------------
void TBoyerMoore::SetPattern (LPCTSTR pattern)

{
   // free the current pattern
   delete [] pat;

   int k;

   // copy the pattern
   m = _tcslen (pattern);
   pat = new TCHAR[m + 1];
   lstrcpy (pat, pattern);

   if (!m_fCaseSensitive)
      _tcsupr (pat);

   // set table values

   for (k = 0; k < kBoyerDeltaSize; k++)
       skip[k] = m;

   for (k = 0; k < m - 1; k++)
       skip[UCHAR( pat[k] )] = m - k - 1;

   // skip [m-1] is still set to m, this is by design

#if defined(_DEBUG)
   for (k=0; k < kBoyerDeltaSize; k++)
      ASSERT(skip[k] > 0);
#endif
}

#define UPR(x)  (_totupper(x))

// -------------------------------------------------------------------------
// Boyer-Moore-Horspool
//
BOOL  TBoyerMoore::Search (LPCTSTR text, int n, int& pos)

{
   register int j;
   int i, k;

   // m is the length of pat
   // n is the length of text

   if (m <= 0)
      return FALSE;

   if (m_fCaseSensitive)
      {
      for (k = m-1 ; k < n; k += skip[UCHAR(text[k])])
         {
         for (j = m-1, i = k; j >= 0 && text[i] == pat[j]; j--)
            i--;

         if (j < 0)
            {
            pos = i + 1;
            return TRUE;
            }
         }

      return FALSE;

      }
   else
      {
      // make sure you cast to UCHAR, so the index is not negative
      // the entire pat string is already uppercased

      for (k = m-1; k < n; k += skip[ UPR(UCHAR(text[k])) ])
         {
         for (j = m-1, i = k; j >= 0 && UPR(UCHAR(text[i])) == UCHAR(pat[j]); j--)
            i--;

         if (j < 0)
            {
            pos = i + 1;
            return TRUE;
            }

#if defined(_DEBUG)
         int delta = skip[  UPR(UCHAR(text[k]))];
         ASSERT(delta > 0);
#endif
         }

      return FALSE;
      }
}
