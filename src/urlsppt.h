/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: urlsppt.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:24  richard_wood
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

// odb - our database header file

#pragma once

#include "rxsearch.h"

//#define  MAX_URL_STRING_SIZE  200
//#define  URL_DDE_TOPICSIZE    200
//#define  URL_DDE_MESSAGESIZE  200

class TGlobalOptions;

int IsURL(LPCTSTR urlString);							// Used in ArtDisp.cpp & TRichEd.cpp
BOOL LaunchURL(LPCTSTR url);							// Used in UpDate.cpp
void CompileURLExpression(TGlobalOptions * pOptions);	// Used in news.cpp & newsopt.cpp
BOOL AnyURLHighlighting();								// Used in artdisp.cpp
void Process2URL(int ich, LPCTSTR pLine, HWND hWnd);	// Used in TRichEd/cpp

// For the most part this matches the set of UNSAFE characters in
// RFC 1738, w/ the exception of the tilde.

#define URL_UNSAFE(ch) (ch == ' '  || \
	ch == '\t' || \
	ch == '\r' || \
	ch == '\n' || \
	ch == '\"' || \
	ch == '<'  || \
	ch == '>'  || \
	ch == '{'  || \
	ch == '}'  || \
	ch == '['  || \
	ch == ']'  || \
	ch == 0)



//
// Private stuff not referenced by any other code apart from URLSppt.cpp
//
class URLKiller
{
public:
	URLKiller();
	~URLKiller();
};
