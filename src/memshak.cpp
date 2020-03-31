/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: memshak.cpp,v $
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
#include "memshak.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

struct TDumbAss
{
	BYTE * pNext;
};

TMemShack::TMemShack (size_t defSZ, const CString & name)
: m_ObjSZ(defSZ),
m_name(name),
m_pHeadOfFreeList(0),
m_nObjects(0)
{
}

TMemShack::~TMemShack ()
{
	deallocate();
}

void TMemShack::deallocate ()
{

	while (!m_BlockList.IsEmpty())
	{
		PBYTE pBlock = (PBYTE) m_BlockList.RemoveHead();
		delete [] pBlock;
	}
}

ULONG TMemShack::Shrink ()
{
	if (0 == m_nObjects)
	{
		deallocate ();
		m_pHeadOfFreeList = NULL;
	}
	return 0;
}

#define  XXBLOC   512

void * TMemShack::myalloc (size_t sz)
{
	// send requests of "wrong" size to ::new
	if (sz != m_ObjSZ)
		return ::new char[sz];

	void * p = m_pHeadOfFreeList;

	if (p)
	{
		m_pHeadOfFreeList =  ((TDumbAss*) p)->pNext;
	}
	else
	{
		BYTE * newBlock = ::new BYTE[XXBLOC * m_ObjSZ];

		if (0 == newBlock)
			return 0;

		BYTE * p1;
		BYTE * p2;

		for (int i = 0; i < (XXBLOC-1); i++)
		{
			p1 = newBlock + (i * m_ObjSZ);
			p2 = p1 + m_ObjSZ;

			((TDumbAss*) p1)->pNext = p2;
		}

		((TDumbAss*)p2)->pNext = NULL;

		// return 1st elem of newBlock
		p = newBlock;

		// freelist begins at 2nd elem of newBlock
		m_pHeadOfFreeList = newBlock + m_ObjSZ;

		m_BlockList.AddTail ( newBlock );
	}

	m_nObjects++;

	return p;
}

void TMemShack::myfree (void * pMem)
{
	// add carcass to our FreeList
	TDumbAss*  pDeadObj = (TDumbAss*) pMem;

	pDeadObj->pNext = m_pHeadOfFreeList;

	m_pHeadOfFreeList = (BYTE*)  pDeadObj;

	m_nObjects--;
}

