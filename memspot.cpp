/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: memspot.cpp,v $
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
#include "memspot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
TMemorySpot::TMemorySpot(EGrowType eGrowType, int InitialBytes)
{
	common_construct (GROW_DOUBLER, 32768, InitialBytes);
}

// ------------------------------------------------------------------------
TMemorySpot::TMemorySpot(int GrowFactor, int InitialBytes)
{
	common_construct (GROW_NUM, GrowFactor, InitialBytes);
}

// ------------------------------------------------------------------------
TMemorySpot::TMemorySpot(int GrowFactor)
{
	common_construct (GROW_NUM, GrowFactor, 1024);
}

// ------------------------------------------------------------------------
void TMemorySpot::common_construct (
									EGrowType eGrowType,
									int GrowFactor,
									int InitialBytes)
{
	m_eGrowType = eGrowType;
	m_iGrowFactor = GrowFactor;
	m_iCurSize = 0;
	m_iMaxSize = max(InitialBytes, 16);

	m_pData = (LPTSTR) malloc ( m_iMaxSize );
	if (0 == m_pData)
		AfxThrowMemoryException();
#if defined(_DEBUG)
	init_size = m_iMaxSize;
	resize_count = 0;
#endif
}

// ------------------------------------------------------------------------
TMemorySpot::~TMemorySpot()
{
	free (m_pData);

#if defined(_DEBUG)
	if (GROW_DOUBLER == m_eGrowType)

		TRACE("Dbler MemSpot (%d) resized %d\n", init_size, resize_count );
	else
		TRACE("Normal MemSpot (%d) resized %d\n", init_size, resize_count );
#endif
}

// ------------------------------------------------------------------------
void TMemorySpot::Add(LPCTSTR data, int bytes)
{
top:
	int iNeed = m_iCurSize + bytes;

	if (iNeed <= m_iMaxSize)
	{
		CopyMemory(m_pData + m_iCurSize, data, bytes);
		m_iCurSize += bytes;
	}
	else
	{
		int iReSize = m_iMaxSize;

		while (iReSize < iNeed)
		{
			if (GROW_DOUBLER == m_eGrowType)
				iReSize += iReSize;           // more aggressive
			else
				iReSize += m_iGrowFactor;
#if defined(_DEBUG)
			resize_count ++;
#endif
		}

		// better resize
		LPTSTR pNew = (LPTSTR) realloc (m_pData, iReSize);

		if (NULL == pNew)
		{
			free (m_pData);
			AfxThrowMemoryException();
		}

		m_pData = pNew;
		m_iMaxSize = iReSize;
		goto top;
	}
}
