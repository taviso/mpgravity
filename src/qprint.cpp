/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: qprint.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:42  richard_wood
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
#include "superstl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/********************************************************************
 * Convert 8BIT contents to QUOTED-PRINTABLE
 * Accepts: source
 *     length of source
 *        pointer to return destination length
 * Returns: destination as quoted-printable text
 *
 * honor maxBytes, but if we end a section, do it after a complete line
 */

//const int QP_MAXL = 75;      // 76th position only used by continuation =

unsigned long
Encode_8bit_QuotedPrintable ( istream& in_strm,
              int                 srcl,     // not used
              unsigned long       maxBytes,
              CFile&              outFile,
              unsigned long&      numBytes,
              unsigned long&      numLines,
              int                 maxCharsInLine )
{
int lp = 0;     /* characters on this line */
char *hex = "0123456789ABCDEF";
char c;
char peekr;
ULONG    numAdded = 0;
long     numRead = 0;

/* for each character */
while ( (numRead < srcl) && (!maxBytes || numAdded < maxBytes))
   {
   /* true line break? */
   in_strm.read(&c,1);  ++numRead;

   if ((c == '\015') && (in_strm.peek() == '\012') && !in_strm.eof())
      {
      in_strm.read(&peekr,1); ++numRead;
      outFile.Write("\r\n",2);   numAdded += 2; ++numLines;
      if (maxBytes > 0 && numAdded >= maxBytes)
         return numRead;  // this is maxxed-out
      lp = 0;        /* reset line count */
      }
   else          /* not a line break */
      {
      /* quoting required? */
      if (iscntrl (BYTE(c)) || (c == 0x7f) || (c & 0x80) || (c == '=') ||
          ((c == ' ') && (in_strm.peek() == '\015')))
          {
         TCHAR rcBuf[3];
         if ((lp += 3) > maxCharsInLine)  /* yes, would line overflow? */
            {
            outFile.Write("=\r\n",3);  numAdded += 3; numLines ++;
            if (maxBytes > 0 && numAdded >= maxBytes)
               {
               in_strm.putback (c);
               return --numRead;  // this is maxxed-out
               }
            lp = 3;     /* set line count */
            }
         /* quote character */

         BYTE  b = (BYTE) c;

         rcBuf[0] = '=';
         rcBuf[1] = hex[b >> 4];  /* high order 4 bits */
         rcBuf[2] = hex[b & 0xf]; /* low order 4 bits */
         outFile.Write(rcBuf,3);  numAdded += 3;

         }
      else          /* ordinary character */
         {
         if ((++lp) > maxCharsInLine)  /* would line overflow? */
            {
            outFile.Write("=\r\n",3); numAdded += 3; numLines++;
            if (maxBytes > 0 && numAdded >= maxBytes)
               {
               in_strm.putback (c);
               return --numRead;
               }
            lp = 1;     /* set line count */
            }
         outFile.Write(&c,1); /*ordinary character */
         ++numAdded;
         }
      }
}
numBytes = numAdded;
return numRead;
}

