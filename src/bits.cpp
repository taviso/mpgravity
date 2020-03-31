/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: bits.cpp,v $
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
#include "bits.h"

#include "mplib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

THeapBitmap::THeapBitmap (int lowIndex, int highIndex)
{
   ASSERT(highIndex >= lowIndex);
   ASSERT(lowIndex >= 0);

   m_low = lowIndex;
   m_hi  = highIndex;
   int count = highIndex - lowIndex + 1;
   count = (count / 8) + 1;
   m_bitmap = new BYTE[count];
   ZeroMemory( m_bitmap, count * sizeof(BYTE));
}

// the compiler messes it up if this is INLINE 11-16-95 amc
void THeapBitmap::SetBit(int whichBit)
{
   ASSERT(whichBit >= m_low);
   ASSERT(whichBit <= m_hi);
   whichBit -= m_low;

   m_bitmap[whichBit/8] |= (0x80 >> (whichBit % 8));
}

void THeapBitmap::ClearBit (int whichBit)
{
   ASSERT(whichBit >= m_low);
   ASSERT(whichBit <= m_hi);
   whichBit -= m_low;
   m_bitmap[whichBit/8] &= ~( 0x80 >> (whichBit % 8));
}

BOOL THeapBitmap::IsBitOn (int whichBit)
{
   ASSERT(whichBit >= m_low);
   ASSERT(whichBit <= m_hi);
   whichBit -= m_low;
   return (m_bitmap[whichBit/8] & ( 0x80 >> (whichBit % 8)));
}

THeapBitmap::~THeapBitmap ()
{
   delete [] m_bitmap;
}

// turn on any bits in the rangeset, that are within the Bounds (inclusive)
void THeapBitmap::TurnOn(const TRangeSet * pRange, const POINT * pptBounds)
{
   ASSERT(pRange);
   ASSERT(pptBounds);
   ASSERT(pptBounds->x >= m_low);
   ASSERT(pptBounds->y <= m_hi);
   ASSERT(pptBounds->x <= pptBounds->y);

   int rngCount = pRange->RangeCount();
   int i, j, low, hi;

   for (i = 0; i < rngCount; ++i)
      {
      pRange->GetRange(i, low, hi);
      for (j = low; j <= hi; ++j)
         {
         if (j >= pptBounds->x && j <= pptBounds->y)
            SetBit (j);
         }
      }
}

