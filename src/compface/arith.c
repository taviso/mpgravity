/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: arith.c,v $
/*  Revision 1.1  2010/07/21 17:16:08  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:22:20  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/01/02 13:34:57  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.3  2008/09/19 13:53:12  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 13:51:49  richard_wood
/*  Updated for VS 2005
/*  */
/*                                                                           */
/*****************************************************************************/

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

void
RevPush(p)
Prob *p;
{
	if (NumProbs >= PIXELS * 2 - 1)
		longjmp(comp_env, ERR_INTERNAL);
	ProbBuf[NumProbs++] = p;
}

void
BigPush(p)
Prob *p;
{
	static WORD tmp;

	BigDiv(p->p_range, &tmp);
	BigMul(0);
	BigAdd(tmp + p->p_offset);
}

int
BigPop(p)
register Prob *p; 
{
	static WORD tmp;
	register int i;

	BigDiv(0, &tmp);
	i = 0;
	while ((tmp < p->p_offset) || (tmp >= p->p_range + p->p_offset))
	{
		p++;
		i++;
	}
	BigMul(p->p_range);
	BigAdd(tmp - p->p_offset);
	return i;
}

#ifdef DEBUG
/* Print a BigInt in HexaDecimal
*/
void
BigPrint()
{
	register int i, c, count;
	register WORD *w;

	count = 0;
	w = B.b_word + (i = B.b_words);
	while (i--)
	{
		w--;
		c = *((*w >> 4) + HexDigits);
		putc(c, stderr);
		c = *((*w & 0xf) + HexDigits);
		putc(c, stderr);
		if (++count >= 36)
		{
			putc('\\', stderr);
			putc('\n', stderr);
			count = 0;
		}
	}
	putc('\n', stderr);
}
#endif

/* Divide B by a storing the result in B and the remainder in the word
* pointer to by r
*/
void
BigDiv(a, r)
register WORD a, *r;
{
	register int i;
	register WORD *w;
	register COMP c, d;

	a &= WORDMASK;
	if ((a == 1) || (B.b_words == 0))
	{
		*r = 0;
		return;
	}
	if (a == 0)	/* treat this as a == WORDCARRY */
	{			/* and just shift everything right a WORD */
		i = --B.b_words;
		w = B.b_word;
		*r = *w;
		while (i--)
		{
			*w = *(w + 1);
			w++;
		}
		*w = 0;
		return;
	}
	w = B.b_word + (i = B.b_words);
	c = 0;
	while (i--)
	{
		c <<= BITSPERWORD;
		c += (COMP)*--w;
		d = c / (COMP)a;
		c = c % (COMP)a;
		*w = (WORD)(d & WORDMASK);
	}
	*r = (unsigned char)c; // RLW 10/9/08 VS2005 change
	if (B.b_word[B.b_words - 1] == 0)
		B.b_words--;
}

/* Multiply a by B storing the result in B
*/
void
BigMul(a)
register WORD a;
{
	register int i;
	register WORD *w;
	register COMP c;

	a &= WORDMASK;
	if ((a == 1) || (B.b_words == 0))
		return;
	if (a == 0)	/* treat this as a == WORDCARRY */
	{			/* and just shift everything left a WORD */
		if ((i = B.b_words++) >= MAXWORDS - 1)
			longjmp(comp_env, ERR_INTERNAL);
		w = B.b_word + i;
		while (i--)
		{
			*w = *(w - 1);
			w--;
		}
		*w = 0;
		return;
	}
	i = B.b_words;
	w = B.b_word;
	c = 0;
	while (i--)
	{
		c += (COMP)*w * (COMP)a;
		*(w++) = (WORD)(c & WORDMASK);
		c >>= BITSPERWORD;
	}
	if (c)
	{
		if (B.b_words++ >= MAXWORDS)
			longjmp(comp_env, ERR_INTERNAL);
		*w = (unsigned char)(COMP)(c & WORDMASK);  // RLW 10/9/08 VS2005 change
	}
}

/* Add to a to B storing the result in B
*/
void
BigAdd(a)
WORD a;
{
	register int i;
	register WORD *w;
	register COMP c;

	a &= WORDMASK;
	if (a == 0)
		return;
	i = 0;
	w = B.b_word;
	c = a;
	while ((i < B.b_words) && c)
	{
		c += (COMP)*w;
		*w++ = (WORD)(c & WORDMASK);
		c >>= BITSPERWORD;
		i++;
	}
	if ((i == B.b_words) && c)
	{
		if (B.b_words++ >= MAXWORDS)
			longjmp(comp_env, ERR_INTERNAL);
		*w = (unsigned char)(COMP)(c & WORDMASK);  // RLW 10/9/08 VS2005 change
	}
}

void
BigClear()
{
	B.b_words = 0;
}
