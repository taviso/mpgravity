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

#include "compface.h"

#define GEN(g) F[h] ^= G.g[k]; break

static void Gen P((char *)) ;

static void
Gen(f)
register char *f;
{
	register int m, l, k, j, i, h;

	for (j = 0; j < HEIGHT;  j++)
	{
		for (i = 0; i < WIDTH;  i++)
		{
			h = i + j * WIDTH;
			k = 0;
			for (l = i - 2; l <= i + 2; l++)
				for (m = j - 2; m <= j; m++)
				{
					if ((l >= i) && (m == j))
						continue;
					if ((l > 0) && (l <= WIDTH) && (m > 0))
						k = *(f + l + m * WIDTH) ? k * 2 + 1 : k * 2;
				}
			switch (i)
			{
				case 1 :
					switch (j)
					{
						case 1 : GEN(g_22);
						case 2 : GEN(g_21);
						default : GEN(g_20);
					}
					break;
				case 2 :
					switch (j)
					{
						case 1 : GEN(g_12);
						case 2 : GEN(g_11);
						default : GEN(g_10);
					}
					break;
				case WIDTH - 1 :
					switch (j)
					{
						case 1 : GEN(g_42);
						case 2 : GEN(g_41);
						default : GEN(g_40);
					}
					break;
				case WIDTH :
					switch (j)
					{
						case 1 : GEN(g_32);
						case 2 : GEN(g_31);
						default : GEN(g_30);
					}
					break;
				default :
					switch (j)
					{
						case 1 : GEN(g_02);
						case 2 : GEN(g_01);
						default : GEN(g_00);
					}
					break;
			}
		}
	}
}

void
GenFace()
{
	static char new[PIXELS];
	register char *f1;
	register char *f2;
	register int i;

	f1 = new;
	f2 = F;
	i = PIXELS;
	while (i-- > 0)
		*(f1++) = *(f2++);
	Gen(new);
}

void
UnGenFace()
{
	Gen(F);
}

