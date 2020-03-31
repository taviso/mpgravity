/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: facebmp.c,v $
/*  Revision 1.1  2010/07/21 17:16:16  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:55  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:22:20  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:42:03  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

/* Originally written by Matthijs Laan (matthijsln@xs4all.nl) this file
 * released to the public domain.
 */

/* Implements the functions from libwinface.h using the compface library */

/* $Id: facebmp.c,v 1.1 2010/07/21 17:16:16 richard_wood Exp $ */

#include <string.h>
#include <stdio.h>

#include "..\include\libwinface.h"

int face_to_bitmap(const char *face, void *pixels, int rowbytes)
{
	int ret;
	int row;
	char *f, *p;

	/* takes some stack, but that shouldn't hurt much */
	char face_buffer[MAXFACESTRLEN];

	/* make a copy of the string to our buffer which is large enough
	 * for uncompface() which reuses the input buffer for output
	 *
	 * don't copy more than MAXFACESTRLEN bytes though -- we could be
	 * getting a string directly from a header of a malicious message
	 * and we wouldn't want a buffer overflow
	 */
	strncpy_s(face_buffer, MAXFACESTRLEN, face, MAXFACESTRLEN - 1);  // RLW 10/9/08 VS2005 change
	face_buffer[MAXFACESTRLEN-1] = '\0';

	if((ret = uncompface(face_buffer)) < 0)
	{
		return ret;
	}

	/* face_buffer now contains something like this (ASCII):
         * 0xDFFF,0xFFFF,0xFFFF,
         * 0x7ADD,0xFFFF,0xFFFF,
         * 0xD7AA,0xAAFF,0xFFFF,
         * ...and so on, for a total of 48 rows,
         * with a LF after each row,
         * terminated with 0
         *
         * now convert that to this: (HEX)
         *
         * DFFFFFFFFFFF[padding]7ADDFFFFFFFF[padding]D7AAAAFFFFFF[padding]...
	 */

	f = face_buffer;
	p = pixels;
	for(row=0;row<48;row++)
	{
		int word;
		for(word=0;word<3;word++)
		{
			unsigned int w;
			sscanf_s(f, "%X", &w);  // RLW 10/9/08 VS2005 change
			*p++ = w >> 8;
			*p++ = w;
			f += 7; /* skip to next word */
		}
		f++; /* skip LF */
		p += rowbytes-6;
	}

	return 0;
}

char hexchars[] = "0123456789ABCDEF";

void bitmap_to_face(const void *pixels, int rowbytes, char *face)
{
	/* convert this: (HEX)
         * 
         * DFFFFFFFFFFF[padding]7ADDFFFFFFFF[padding]D7AAAAFFFFFF...
         * 
         * to: (ASCII)
         * 
         * DFFFFFFFFFFF7ADDFFFFFFFFD7AAAAFFFFFF...
	 */
	int row;
	char *f = face;
	const unsigned char *p = pixels;

	for(row=0;row<48;row++)
	{
		int i;
		for(i=0;i<6;i++)
		{
			unsigned char c = *p++;
			*f++ = hexchars[c >> 4];
			*f++ = hexchars[c & 0xF];
		}
		p += rowbytes-6;
	}
	*f++ = '\0';

	compface(face);	
}
