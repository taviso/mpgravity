/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mailadr.cpp,v $
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
/*  Revision 1.4  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:30  richard_wood
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
#include "mailadr.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4706)    /* assignment within conditional expression */

#define PARSE 66

// Long TRUE
#define LONGT  (long) 1

/* Token delimiting special characters */

/* full RFC-822 specials */
const char *rspecials =  "()<>@,;:\\\"[].";
/* body token specials */
const char *tspecials = " ()<>@,;:\\\"[]./?=";

/* Once upon a time, CSnet had a mailer which assigned special semantics to
* dot in e-mail addresses.  For the sake of that mailer, dot was added to
* the RFC-822 definition of `specials', even though it had numerous bad side
* effects:
*   1) It broke mailbox names on systems which had dots in user names, such as
*     Multics and TOPS-20.  RFC-822's syntax rules require that `Admin . MRC'
*     be considered equivalent to `Admin.MRC'.  Fortunately, few people ever
*     tried this in practice.
*   2) It required that all personal names with an initial be quoted, a widely
*     detested user interface misfeature.
*   3) It made the parsing of host names be non-atomic for no good reason.
* To work around these problems, the following alternate specials lists are
* defined.  hspecials and wspecials are used in lieu of rspecials, and
* ptspecials are used in lieu of tspecials.  These alternate specials lists
* make the parser work a lot better in the real world.  It ain't politically
* correct, but it lets the users get their job done!
*/

/* parse-host specials */
const char *hspecials = " ()<>@,;:\\\"";

// parse-word specials by alchoy
// delimiters has been relaxed to accept
//        Al Choy, Jr.       (accept the comma)
//        Al Choy [VIP]
//
const char *wspecials = " ()<>@;:\\\"";

/* parse-token specials for parsing */
const char *ptspecials = " ()<>@,;:\\\"[]/?=";

// prototypes
char * cpystr (char * src);
MAIL_ADDRESS *rfc822_parse_routeaddr (char *string,char **ret,char *defaulthost,
									  CStringList * pErrList);
MAIL_ADDRESS *rfc822_parse_addrspec (char *string,char **ret,char *defaulthost,
									 CStringList * pErrList);
char *rfc822_skip_comment (char **s,long trim, CStringList * pErrList);
void rfc822_skipws (char **s, CStringList * pErrList);
char *rfc822_parse_word (char *s,const char *delimiters, CStringList * pErrList);
char *rfc822_parse_phrase (char *s, CStringList * pErrList);
char *rfc822_cpy (char *src);
char *rfc822_quote (char *src);
void FreeAddress (MAIL_ADDRESS * pAddr);

void mm_log(char * msg, int n, CStringList * pErrList);

#define NIL NULL
#define MAILTMPLEN 1024

/* Mail instantiate address
* Returns: new address
*/

MAIL_ADDRESS *mail_newaddr ()
{
	return new MAIL_ADDRESS;
}

MAIL_ADDRESS::MAIL_ADDRESS()
{
	personal = adl = mailbox = host = error = NIL;
	fUsedDefaultHost = FALSE;
	next = NIL;
}

MAIL_ADDRESS::~MAIL_ADDRESS()
{
	delete personal;
	delete adl;
	delete mailbox;
	delete host;
	delete error;
}

/* Parse RFC822 address list
* Accepts: address list to write to
*         input string
*         default host name
*/

void rfc822_parse_adrlist (MAIL_ADDRESS **lst,char *string,char *host,
						   CStringList * pErrList)
{
	char tmp[MAILTMPLEN];
	char *p,*s;
	long n = 0;
	MAIL_ADDRESS *last = *lst;
	MAIL_ADDRESS *adr;
	/* run to tail of list */
	if (last)
		while (last->next)
			last = last->next;

	while (string)
	{              /* loop until string exhausted */
		rfc822_skipws (&string, pErrList);    /* skip leading WS */
		if (!*(p = string)) break;  /* trailing whitespace */
		/* see if start of group */
		while ((*p == ':') || (p = rfc822_parse_phrase (string, pErrList)))
		{
			s = p;                    /* end of phrase */
			rfc822_skipws (&s, pErrList); /* find delimiter */
			if (*s == ':')            /* really a group? */
			{
				n++;                     /* another level */
				*p = '\0';               /* tie off group name */
				p = ++s;         /* continue after the delimiter */
				rfc822_skipws (&p, pErrList);      /* skip subsequent whitespace */
				/* write as address */
				(adr = mail_newaddr ())->mailbox = rfc822_cpy (string);
				if (!*lst) *lst = adr;   /* first time through? */
				else last->next = adr;   /* no, append to the list */
				last = adr;              /* set for subsequent linking */
				string = p;              /* continue after this point */
			}
			else
				break;               /* bust out of this */
		}
		rfc822_skipws (&string, pErrList);    /* skip any following whitespace */
		if (!string) break;         /* punt if unterminated group */
		/* if not empty group */
		if (*string != ';' || n <= 0)
		{
			/* got an address? */
			if (adr = rfc822_parse_address (&string,host, pErrList))
			{
				if (!*lst) *lst = adr;   /* yes, first time through? */
				else last->next = adr;   /* no, append to the list */
				last = adr;              /* set for subsequent linking */
			}
			else if (string)          /* bad mailbox */
			{
				CString str; str.LoadString (IDS_BAD_MAILBOX);
				wsprintf (tmp,str,string);
				mm_log (tmp, PARSE, pErrList);
				break;
			}
		}

		/* handle end of group */
		if (string && *string == ';' && n >= 0)
		{
			n--;                      /* out of this group */
			string++;                 /* skip past the semicolon */
			/* append end of address mark to the list */
			last->next = (adr = mail_newaddr ());
			last = adr;               /* set for subsequent linking */
			rfc822_skipws (&string, pErrList);  /* skip any following whitespace */
			switch (*string)          /* see what follows */
			{
			case ',':                 /* another address? */
				++string;                /* yes, skip past the comma */
			case ';':                 /* another end of group? */
			case '\0':                /* end of string */
				break;
			default:
				{
					CString str; str.LoadString (IDS_ERR_UNEXPECTED_CHARS);
					wsprintf (tmp,str,string);
					mm_log (tmp, PARSE, pErrList);
				}
				break;
			}
		}
	}

	while (n-- > 0)               /* if unterminated groups */
	{
		last->next = (adr = mail_newaddr ());
		last = adr;                 /* set for subsequent linking */
	}
}

/* Parse RFC822 address
* Accepts: pointer to string pointer
*         default host
* Returns: address
*
* Updates string pointer
*/

MAIL_ADDRESS *rfc822_parse_address (char **string,char *defaulthost,
									CStringList * pErrList)
{
	char tmp[MAILTMPLEN];
	MAIL_ADDRESS *adr;
	char c,*s;
	char *phrase;
	if (!string)
		return NIL;
	rfc822_skipws (string, pErrList);       /* flush leading whitespace */

	/* This is much more complicated than it should be because users like
	* to write local addrspecs without "@localhost".  This makes it very
	* difficult to tell a phrase from an addrspec!
	* The other problem we must cope with is a route-addr without a leading
	* phrase.  Yuck!
	*/

	if (*(s = *string) == '<')    /* note start, handle case of phraseless RA */
		adr = rfc822_parse_routeaddr (s,string,defaulthost, pErrList);
	else                          /* get phrase if any */
	{
		if ((phrase = rfc822_parse_phrase (s, pErrList)) &&
			(adr = rfc822_parse_routeaddr (phrase,string,defaulthost, pErrList)))
		{
			*phrase = '\0';           /* tie off phrase */
			/* phrase is a personal name */
			adr->personal = rfc822_cpy (s);
		}
		else
			adr = rfc822_parse_addrspec (s,string,defaulthost, pErrList);
	}
	/* analyze what follows */
	if (*string)
		switch (c = **string)
	{
		case ',':                     /* comma? */
			++*string;                  /* then another address follows */
			break;
		case ';':                     /* possible end of group? */
			break;                      /* let upper level deal with it */
		default:
			{
				CString str;
				if (c < 0)
					str.LoadString(IDS_ERR_UNEXPECTED_CHARS_ADDRESS);
				else
					str.LoadString(isalnum(c) ? IDS_MUST_USE_COMMA : IDS_ERR_UNEXPECTED_CHARS_ADDRESS);
				s = (LPTSTR) (LPCTSTR) str;
				wsprintf (tmp,s,*string);
				mm_log (tmp,PARSE, pErrList);
			}
			/* falls through */
		case '\0':                    /* null-specified address? */
			*string = NIL;              /* punt remainder of parse */
			break;
	}
	return adr;                   /* return the address */
}

/* Parse RFC822 route-address
* Accepts: string pointer
*         pointer to string pointer to update
* Returns: address
*
* Updates string pointer
*/

MAIL_ADDRESS *rfc822_parse_routeaddr (char *string,char **ret,
									  char *defaulthost, CStringList * pErrList)
{
	char tmp[MAILTMPLEN];
	MAIL_ADDRESS *adr;
	char *adl = NIL;
	char *routeend = NIL;
	if (!string) return NIL;
	rfc822_skipws (&string, pErrList);      /* flush leading whitespace */
	/* must start with open broket */
	if (*string != '<')
		return NIL;
	if (string[1] == '@')        /* have an A-D-L? */
	{
		adl = ++string;             /* yes, remember that fact */
		while (*string != ':')      /* search for end of A-D-L */
		{
			/* punt if never found */
			if (!*string) return NIL;
			++string;                 /* Try next character */
		}
		*string = '\0';             /* tie off A-D-L */
		routeend = string;          /* remember in case need to put back */
	}
	/* parse address spec */
	if (!(adr = rfc822_parse_addrspec (++string,ret,defaulthost, pErrList)))
	{
		if (adl)
			*routeend = ':';   /* put colon back since parse barfed */
		return NIL;
	}
	/* have an A-D-L? */
	if (adl) adr->adl = cpystr (adl);
	/* make sure terminated OK */
	if (*ret)
		if (**ret == '>')
		{
			++*ret;                     /* skip past the broket */
			rfc822_skipws (ret, pErrList); /* flush trailing WS */
			if (!**ret)
				*ret = NIL;     /* wipe pointer if at end of string */
			return adr;                 /* return the address */
		}

		CString str; str.LoadString (IDS_ERR_UNTERMINATED_MAILBOX);
		wsprintf (tmp,str,adr->mailbox,
			*adr->host == '@' ? "<null>" : adr->host);
		mm_log (tmp, PARSE, pErrList);
		return adr;                   /* return the address */
}

/* Parse RFC822 address-spec
* Accepts: string pointer
*         pointer to string pointer to update
*         default host
* Returns: address
*
* Updates string pointer
*/

MAIL_ADDRESS *rfc822_parse_addrspec (char *string,char **ret,char *defaulthost,
									 CStringList * pErrList)
{
	MAIL_ADDRESS *adr;
	char *end;
	char c,*s,*t;
	if (!string) return NIL;
	rfc822_skipws (&string, pErrList);      /* flush leading whitespace */
	/* find end of mailbox */
	if (!(end = rfc822_parse_word (string,NIL, pErrList)))
		return NIL;
	adr = mail_newaddr ();        /* create address block */
	c = *end;                     /* remember delimiter */
	*end = '\0';                  /* tie off mailbox */
	/* copy mailbox */
	adr->mailbox = rfc822_cpy (string);
	*end = c;                     /* restore delimiter */
	t = end;                      /* remember end of mailbox for no host case */
	rfc822_skipws (&end, pErrList);         /* skip whitespace */
	if (*end == '@')             /* have host name? */
	{
		++end;                      /* skip delimiter */
		rfc822_skipws (&end, pErrList);       /* skip whitespace */
		*ret = end;                 /* update return pointer */
		/* search for end of host */
		if (end = rfc822_parse_word ((string = end),hspecials, pErrList))
		{
			c = *end;                 /* remember delimiter */
			*end = '\0';              /* tie off host */
			/* copy host */
			adr->host = rfc822_cpy (string);
			*end = c;                 /* restore delimiter */
		}
		else
		{
			CString str; str.LoadString (IDS_ERR_MISSING_HOSTNAME);
			mm_log ((LPTSTR) (LPCTSTR) str,PARSE, pErrList);
		}
	}
	else
		end = t;                 /* make person name default start after mbx */
	/* default host if missing */
	if (!adr->host)
	{
		adr->host = cpystr (defaulthost);
		adr->fUsedDefaultHost = TRUE;
	}
	if (end && !adr->personal)   /* Try person name in comments if missing */
	{
		while (*end == ' ') ++end;  /* see if we can find a person name here */
		if ((*end == '(') && (s = rfc822_skip_comment (&end,LONGT, pErrList)) && strlen (s))
			adr->personal = rfc822_cpy (s);
		rfc822_skipws (&end, pErrList);       /* skip any other WS in the normal way */
	}
	/* set return to end pointer */
	*ret = (end && *end) ? end : NIL;
	return adr;                   /* return the address we got */
}

/* Skips RFC822 comment
* Accepts: pointer to string pointer
*         trim flag
* Returns: pointer to first non-blank character of comment
*/

char *rfc822_skip_comment (char **s, long trim, CStringList * pErrList)
{
	char *ret,tmp[MAILTMPLEN];
	char *s1 = *s;
	char *t = NIL;
	/* skip past whitespace */
	for (ret = ++s1; *ret == ' '; ret++)
		;
	do
	{
		switch (*s1)              /* get character of comment */
		{
		case '(':                     /* nested comment? */
			if (!rfc822_skip_comment (&s1,(long) NIL, pErrList))
				return NIL;
			t = --s1;                   /* last significant char at end of comment */
			break;

		case ')':                     /* end of comment? */
			*s = ++s1;                  /* skip past end of comment */
			if (trim)                  /* if level 0, must trim */
			{
				if (t)
					t[1] = '\0';       /* tie off comment string */
				else
					*ret = '\0';         /* empty comment */
			}
			return ret;

		case '\\':                    /* quote next character? */
			if (*++s1)
				break;           /* drop in if null seen */

		case '\0':                    /* end of string */
			{
				CString str; str.LoadString (IDS_ERR_UNTERMINATED_COMMENT);
				wsprintf (tmp,str,*s);
				mm_log (tmp,PARSE, pErrList);
				**s = '\0';                 /* nuke duplicate messages in case reparse */
			}
			return NIL;                 /* this is weird if it happens */

		case ' ':                     /* whitespace isn't significant */
			break;
		default:                      /* random character */
			t = s1;                     /* update last significant character pointer */
			break;
		}
	} while (s1++);

	return t;
}

/* Skips RFC822 whitespace
* Accepts: pointer to string pointer
*/

void rfc822_skipws (char **s, CStringList * pErrList)
{
	/* while whitespace or start of comment */
	while ((**s == ' ') || (**s == '(')) 
	{
		if ((**s == '(') && !rfc822_skip_comment (s,(long) NIL, pErrList))
			return;
		if (s && *s && **s) ++*s;   /* skip past whitespace character */
	}
}

/* Parse RFC822 phrase
* Accepts: string pointer
* Returns: pointer to end of phrase
*/

char *rfc822_parse_phrase (char *s, CStringList * pErrList)
{
	char *curpos;
	if (!s) return NIL;           /* no-op if no string */
	/* find first word of phrase */
	curpos = rfc822_parse_word (s,NIL, pErrList);
	if (!curpos) return NIL;      /* no words means no phrase */
	if (!*curpos) return curpos;  /* check if string ends with word */
	s = curpos;                   /* sniff past the end of this word and WS */
	rfc822_skipws (&s, pErrList);           /* skip whitespace */
	/* recurse to see if any more */
	return (s = rfc822_parse_phrase (s, pErrList)) ? s : curpos;
}

/* Parse RFC822 word
* Accepts: string pointer
* Returns: pointer to end of word
*/

char *rfc822_parse_word (char *s,const char *delimiters, CStringList * pErrList)
{
	char *st,*str;
	if (!s) return NIL;           /* no-op if no string */
	rfc822_skipws (&s, pErrList); /* flush leading whitespace */
	if (!*s) return NIL;          /* end of string */
	/* default delimiters to standard */
	if (!delimiters) delimiters = wspecials;
	str = s;                      /* hunt pointer for strpbrk */
	while (TRUE)                    /* look for delimiter */
	{
		if (!(st = strpbrk (str,delimiters)))
		{
			while (*s) ++s;           /* no delimiter, hunt for end */
			return s;                 /* return it */
		}
		switch (*st)                /* dispatch based on delimiter */
		{
		case '"':                   /* quoted string */
			/* look for close quote */
			while (*++st != '"')
				switch (*st)
			{
				case '\0':                /* unbalanced quoted string */
					return NIL;              /* sick sick sick */
				case '\\':                /* quoted character */
					if (!*++st) return NIL;  /* skip the next character */
				default:                  /* ordinary character */
					break;                   /* no special action */
			}
			str = ++st;               /* continue parse */
			break;

		case '\\':                  /* quoted character */
			/* This is wrong; a quoted-pair can not be part of a word.  However,
			* domain-literal is parsed as a word and quoted-pairs can be used
			* *there*.  Either way, it's pretty pathological.
			*/
			if (st[1]) {              /* not on NUL though... */
				str = st + 2;            /* skip quoted character and go on */
				break;
			}
		default:                    /* found a word delimiter */
			return (st == s) ? NIL : st;
		}
	}
}

/* Copy an RFC822 format string
* Accepts: string
* Returns: copy of string
*/

char *rfc822_cpy (char *src)
{
	/* copy and unquote */
	return rfc822_quote (cpystr (src));
}

/* Unquote an RFC822 format string
* Accepts: string
* Returns: string
*/

char *rfc822_quote (char *src)
{
	char *ret = src;
	if (strpbrk (src,"\\\"")) {   /* any quoting in string? */
		char *dst = ret;
		while (*src) {              /* copy string */
			if (*src == '\"') src++;  /* skip double quote entirely */
			else {
				if (*src == '\\') src++;/* skip over single quote, copy next always */
				*dst++ = *src++; /* copy character */
			}
		}
		*dst = '\0';                /* tie off string */
	}
	return ret;                   /* return our string */
}

void mm_log(char * msg, int n, CStringList * pErrList)
{
	if (pErrList)
		pErrList->AddTail ( msg );
#if defined(_DEBUG)
	afxDump << msg << "\n";
#endif
}

char* cpystr (char* src)
{
	int iLen = _tcslen(src);
	char* pRet = new char[iLen + 1];

	_tcscpy (pRet, src);
	return pRet;
}

void FreeAddressList (MAIL_ADDRESS * pAddrList)
{
	MAIL_ADDRESS * pOne;
	MAIL_ADDRESS * lst = pAddrList;

	while (lst)
	{
		pOne = lst;
		lst = lst->next;
		delete pOne;
	}
}

