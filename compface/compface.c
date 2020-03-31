/*
 *  Compface - 48x48x1 image compression and decompression
 *
 *  Copyright (c) James Ashton - Sydney University - June 1990.
 *
 *  Written 11th November 1989.
 *
 *  Permission is given to distribute these sources, as long as the
 *  copyright messages are not removed, and no monies are exchanged. 
 *
 *  No responsibility is taken for any errors on inaccuracies inherent
 *  either to the comments or the code of this program, but if reported
 *  to me, then an attempt will be made to fix them.
 */

/* prevent multiply defined symbols error message, because we have MAIN defined
 * in uncompface.c. This way we can have both uncompface() and compface() in 
 * one library.
 */
/*#define MAIN*/

#include "compface.h"

int
compface(fbuf)
char *fbuf;
{
	if (!(status = setjmp(comp_env)))
	{
		ReadFace(fbuf);
		GenFace();
		CompAll(fbuf);
	}
	return status;
}
