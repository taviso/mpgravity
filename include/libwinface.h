/* Originally written by Matthijs Laan (matthijsln@xs4all.nl) this file
 * released to the public domain.
 */

/* $Id: libwinface.h,v 1.1 2010/07/21 17:16:16 richard_wood Exp $ */

#ifndef __LIBWINFACE_H
#define __LIBWINFACE_H

/* error codes copied from compface.h */
#define ERR_OK		0	/* successful completion */
#define ERR_EXCESS	1	/* completed OK but some input was ignored */
#define ERR_INSUFF	-1	/* insufficient input.  Bad face format? */
#define ERR_INTERNAL	-2;	/* arithmetic overflow or buffer overflow */

/* compface functions. See the compface(3) manpage in the source */
int compface(char *);
int uncompface(char *);

/* libwinface added functions: */

/* Converts an X-Face string to a bitmap.
 *
 * face
 *   X-Face string (max MAXFACESTRLEN-1 characters long)
 * pixels
 *   pointer to a buffer receiving the bitmap. The buffer should be rowbytes*48
 *   bytes long. A byte represents 8 pixels.
 * rowbytes
 *   amount of bytes in a row of pixels (scanline). With no padding, this would
 *   be 48/8=6 bytes. For a 32-bit scanline aligned bitmap set this to 8.
 *
 * return value
 *   ERR_OK for success, ERR_INSUFF or ERR_INTERNALL on error.
 *
 */
int face_to_bitmap(const char *face, void *pixels, int rowbytes);

/* Converts a bitmap to an X-Face string.
 *
 * pixels
 *   pointer to buffer containing the source bitmap to convert.
 * face
 *   pointer buffer to which the X-Face string is written. This buffer must be
 *   at least MAXFACESTRLEN bytes long.
 */
void bitmap_to_face(const void *pixels, int rowbytes, char *face);

/* maximum size of a X-Face string including the null-terminator. 2K is enough
 * according to the compface(3) man page...
 */
#define MAXFACESTRLEN 2048

#endif/*__LIBWINFACE_H*/