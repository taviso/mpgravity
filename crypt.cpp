/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: crypt.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:51:16  richard_wood
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

/*--- crypt.cpp -- Simple encryption routines
 *
 *  These simple encryption/decryption routines are used to
 *  protect the user's NNTP server password.
 *  The lack of sophistication is meant to protect the
 *  exportability of this code.	For better stuff, see the
 *  non-anonymous FTP site ripem.msu.edu.
 */

#include "stdafx.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define ROTORSIZE 256
#define NSEEDBYTES 4
#define RANBUFSIZE 15
#define RANLAG1	 9
#define RANLAG2	 14

static unsigned char RanbufMaster[RANBUFSIZE] =
{
  0, 0, 0, 0, 56, 34, 199, 77, 245, 252, 17, 184, 130, 215, 102};
static unsigned char *Ranbuf;
static int Ranidx;

void MRRMakeRotors (unsigned char *ranbuf, unsigned char *rotorE,
                    unsigned char *rotorD);
int MRRRan (unsigned char *Ranbuf);
int prencode (unsigned char *bufin, unsigned int nbytes, char *bufcoded);
int prdecode (char *bufcoded, unsigned char *bufplain, int outbufsize);

/*--- function MRREncrypt ------------------------------------------
 *
 *  Encrypt and printably encode a sequence of bytes.
 *
 *  Entry:	plainText	is the input plaintext (binary bytes).
 * 			len			is the number of bytes in the above.
 * 			cipherText	is an output buffer that must be
 * 							2*len+6 bytes long.
 *
 *  Exit:	cipherText	is a zero-terminated ASCII printable
 * 							string of ciphertext.
 */
void
MRREncrypt (unsigned char *plainText, int len, char *cipherText)
{
  time_t mytime;
  int j;
  unsigned char prev = 0;
  unsigned char *cipbuf, *cipbuforig;
  unsigned char ranbuf[RANBUFSIZE], rotorE[ROTORSIZE], rotorD[ROTORSIZE];

  if (!len) {
	cipherText[0] = '\0';
	return;
  }
  cipbuf = cipbuforig = new BYTE[len + 4];

  time (&mytime);
  memcpy (ranbuf, RanbufMaster, RANBUFSIZE);
  for (j = 0; j < NSEEDBYTES; j++) {
	cipbuf[j] = ranbuf[j] = (unsigned char) (mytime & 0xff);
	mytime >>= 8;
  }

  MRRMakeRotors (ranbuf, rotorE, rotorD);

  for (j = 0; j < len; j++) {
	cipbuf[j + NSEEDBYTES] = prev = rotorE[(*(plainText++) + prev) % ROTORSIZE];
  }

  prencode (cipbuf, len + NSEEDBYTES, cipherText);
  delete [] cipbuforig;
}

int
MRRDecrypt (char *cipherText, unsigned char *plainText, int plainMax)
{
  unsigned char prev = 0;
  unsigned char *cipbuf, *cipbuforig;
  unsigned char ranbuf[RANBUFSIZE], rotorE[ROTORSIZE], rotorD[ROTORSIZE];
  int j, nbytes, ch;

  if (!strlen (cipherText)) {
	plainText[0] = '\0';
	return 0;
  }

  cipbuf = cipbuforig = new BYTE [plainMax + 4];

  nbytes = prdecode (cipherText, cipbuf, plainMax) - NSEEDBYTES;
  memcpy (ranbuf, RanbufMaster, RANBUFSIZE);
  memcpy (ranbuf, cipbuf, NSEEDBYTES);
  MRRMakeRotors (ranbuf, rotorE, rotorD);

  cipbuf += NSEEDBYTES;
  for (j = 0; j < nbytes; j++) {
	ch = (int) rotorD[*cipbuf] - (int) prev;
	if (ch < 0)
	  ch += ROTORSIZE;
	prev = *cipbuf;
	*(plainText++) = ch;
	cipbuf++;
  }

  delete [] cipbuforig;
  return nbytes;
}

/*--- function MRRMakeRotors -----------------------------------------
 *
 *  Set up the rotors used for encryption/decryption.
 *
 *  Entry:	Ranbuf is a buffer of "random" bytes.
 *
 *  Exit:	rotorE and rotorD are the encryption and
 * 			corresponding decryption rotors.
 */
void
MRRMakeRotors (unsigned char *ranbuf, unsigned char *rotorE,
			   unsigned char *rotorD)
{
  int j, k, idx;
  unsigned char ch;

  /* Make a standard reflexive rotor. */
  for (j = 0; j < ROTORSIZE; j++) {
	rotorE[j] = j;
  }
  Ranidx = 0;

  /* Permute the rotor */
  for (k = 2; k; k--)
	for (j = ROTORSIZE - 1; j; j--) {
	  idx = MRRRan (ranbuf);
	  /* This code omitted: idx = idx%(j+1); */
	  ch = rotorE[j];
	  rotorE[j] = rotorE[idx];
	  rotorE[idx] = ch;
	}

  /* Create rotorD as the inverse of rotorE */
  for (j = 0; j < ROTORSIZE; j++) {
	rotorD[rotorE[j]] = j;
  }

#ifdef DEBUGMRR
  printf ("rotorE:\n");
  for (j = 0; j < ROTORSIZE; j++) {
	printf ("%4d", rotorE[j]);
	if ((j + 1) % 16 == 0)
	  printf ("\n");
  }
  printf ("rotorD:\n");
  for (j = 0; j < ROTORSIZE; j++) {
	printf ("%4d", rotorD[j]);
	if ((j + 1) % 16 == 0)
	  printf ("\n");
  }
#endif
}

/*--- function MRRRan --------------------------------------------------
 *
 *  Return a pseudorandom number in the range 0-255.
 *  Uses an additive linear shift register.
 *
 *  Entry:	Ranbuf	is a buffer of RANBUFSIZE random bytes.
 * 			Ranidx	is an index into this buffer.
 *
 *  Exit:	Ranidx has been decremented.
 * 			Returns the next pseudorandom number.
 */
int
MRRRan (unsigned char *Ranbuf)
{
  int idx1, idx2;

  if (--Ranidx < 0)
	Ranidx = RANBUFSIZE - 1;
  idx1 = Ranidx + RANLAG1;
  if (idx1 >= RANBUFSIZE)
	idx1 -= RANBUFSIZE;
  idx2 = Ranidx + RANLAG2;
  if (idx2 >= RANBUFSIZE)
	idx2 -= RANBUFSIZE;

  Ranbuf[Ranidx] = 0xff & (Ranbuf[idx1] + Ranbuf[idx2] + Ranbuf[Ranidx]);

  return Ranbuf[Ranidx];
}

/*--- prencode.c -- File containing routines to convert a buffer
 *  of bytes to/from RFC 1113 printable encoding format.
 *
 *  This technique is similar to the familiar Unix uuencode
 *  format in that it maps 6 binary bits to one ASCII
 *  character (or more aptly, 3 binary bytes to 4 ASCII
 *  characters).  However, RFC 1113 does not use the same
 *  mapping to printable characters as uuencode.
 *
 *  Mark Riordan   12 August 1990 and 17 Feb 1991.
 *  This code is hereby placed in the public domain.
 */

static char six2pr[64] =
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

/*--- function prencode -----------------------------------------------
 *
 *   Encode a single line of binary data to a standard format that
 *   uses only printing ASCII characters (but takes up 33% more bytes).
 *
 *    Entry    bufin    points to a buffer of bytes.  If nbytes is not
 *                      a multiple of three, then the byte just beyond
 *                      the last byte in the buffer must be 0.
 *             nbytes   is the number of bytes in that buffer.
 *             bufcoded points to an output buffer.  Be sure that this
 *                      can hold at least 1 + (4*nbytes)/3 characters.
 *
 *    Exit     bufcoded contains the coded line.  The first 4*nbytes/3 bytes
 *                      contain printing ASCII characters representing
 *                      those binary bytes. This may include one or
 *                      two '=' characters used as padding at the end.
 *                      The last byte is a zero byte.
 *             Returns the number of ASCII characters in "bufcoded".
 */
int
prencode (
	 unsigned char *bufin,
	 unsigned int nbytes,
	 char *bufcoded
)
{
/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) six2pr[c]

  register char *outptr = bufcoded;
  unsigned int i;

  for (i = 0; i < nbytes; i += 3) {
	*(outptr++) = ENC (*bufin >> 2);	/* c1 */
	*(outptr++) = ENC (((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017));	/* c2 */
	*(outptr++) = ENC (((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03));	/* c3 */
	*(outptr++) = ENC (bufin[2] & 077);		/* c4 */

	bufin += 3;
  }

  /* If nbytes was not a multiple of 3, then we have encoded too
   * many characters.  Adjust appropriately.
   */
  if (i == nbytes + 1) {
	/* There were only 2 bytes in that last group */
	outptr[-1] = '=';
  }
  else if (i == nbytes + 2) {
	/* There was only 1 byte in that last group */
	outptr[-1] = '=';
	outptr[-2] = '=';
  }
  *outptr = '\0';
  return (outptr - bufcoded);
}

/*--- function prdecode ------------------------------------------------
 *
 *  Decode an ASCII-encoded buffer back to its original binary form.
 *
 * 	Entry 	bufcoded 	points to a encoded string.	It is
 *                         terminated by any character not in
 *                         the printable character table six2pr, but
 *                         leading whitespace is stripped.
 *             bufplain    points to the output buffer; must be big
 *                         enough to hold the decoded string (generally
 *                         shorter than the encoded string) plus
 *                         as many as two extra bytes used during
 *                         the decoding process.
 *             outbufsize  is the maximum number of bytes that
 *                         can fit in bufplain.
 *
 *    Exit     Returns the number of binary bytes decoded.
 *             bufplain    contains these bytes.
 */
int
prdecode (
	 char *bufcoded,
	 unsigned char *bufplain,
	 int outbufsize
)
{
/* single character decode */
#define DEC(c) pr2six[(int)c]
#define MAXVAL 63

  int nbytesdecoded, j;
  register char *bufin = bufcoded;
  register unsigned char *bufout = bufplain;
  register int nprbytes;
  unsigned char pr2six[256];

  /* Initialize the mapping table.
     * This code should work even on non-ASCII machines.
   */
  for (j = 0; j < 256; j++)
	pr2six[j] = MAXVAL + 1;

  for (j = 0; j < 64; j++)
	pr2six[(int) six2pr[j]] = (unsigned char) j;

  /* Strip leading whitespace. */

  while (*bufcoded == ' ' || *bufcoded == '\t')
	bufcoded++;

  /* Figure out how many characters are in the input buffer.
   * If this would decode into more bytes than would fit into
   * the output buffer, adjust the number of input bytes downwards.
   */
  bufin = bufcoded;
  while (pr2six[(int) *(bufin++)] <= MAXVAL);
  nprbytes = bufin - bufcoded - 1;
  nbytesdecoded = ((nprbytes + 3) / 4) * 3;
  if (nbytesdecoded > outbufsize) {
	nprbytes = (outbufsize * 4) / 3;
  }

  bufin = bufcoded;

  while (nprbytes > 0) {
	*(bufout++) = (unsigned char) (DEC (*bufin) << 2 | DEC (bufin[1]) >> 4);
	*(bufout++) = (unsigned char) (DEC (bufin[1]) << 4 | DEC (bufin[2]) >> 2);
	*(bufout++) = (unsigned char) (DEC (bufin[2]) << 6 | DEC (bufin[3]));
	bufin += 4;
	nprbytes -= 4;
  }

  if (nprbytes & 03) {
	if (pr2six[(int) bufin[-2]] > MAXVAL) {
	  nbytesdecoded -= 2;
	}
	else {
	  nbytesdecoded -= 1;
	}
  }

  return (nbytesdecoded);
}
