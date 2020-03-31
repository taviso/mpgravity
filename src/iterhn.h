/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: iterhn.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.2  2008/09/19 14:51:28  richard_wood
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

#pragma once

// TArticleIterator - is passed to DeferredRuleActions
//
// TArticleIterator - returns PTR
//   TArticleDBIterator - src is GRPDB
//     TArticleNewIterator - adds filtering
//   TArticleBankIterator - src is TArticleBank
//

#include "newsdb.h"           // defined THeaderIterator

// forward class declarations
class TNewsGroup;
class TArticleHeader;
class TArticleBank;

// --------------------------------------------------------------------
// TArticleIterator - return type is definitely TArticle
//    This is an abstract base class
class TArticleIterator
{
public:
	virtual TArticleHeader * Next() = 0;
};

// -------------------------------------------------------------------------
// TArticleDBIterator
//    Private inheritance - means it's just an implementation detail
class TArticleDBIterator : public TArticleIterator, protected THeaderIterator
{
public:
	// collection based on contents of GrpDB
	TArticleDBIterator (TNewsGroup * pNG,
		THeaderIterator::ELockType kLockType = THeaderIterator::kReadLock);

	virtual TArticleHeader * Next ();

protected:
	TNewsGroup * m_pNG;
};

// --------------------------------------------------------------------
// Use some basic filtering
class TArticleNewIterator : public TArticleDBIterator
{
public:
	// two choices (New & Old) or (New)
	TArticleNewIterator (TNewsGroup* pGroup, BOOL fNewAndOld,
		THeaderIterator::ELockType kLockType = kReadLock);

	virtual TArticleHeader * Next ();

private:
	BOOL  m_fNewAndOld;
};
