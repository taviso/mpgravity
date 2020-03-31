/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tsubjct.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:19  richard_wood
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

// This is a C++ object pattern.  Subject & Observer pair.
// In short:
//    when the state of 'Subject' changes, it calls pObserver->Update
//    The observer then queries the 'Subject' to get the state.
//

#include "stdafx.h"
#include "tsubjct.h"
#include "tobserv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ------------------------------------------------------------------------
BOOL TSubject::AddObserver(TObserver * pObserver)
{
	pObserver -> ObserverSetSubject ( this );

	m_lstObservers.AddTail ( pObserver );
	return TRUE;
}

// ------------------------------------------------------------------------
BOOL TSubject::DeleteObserver(TObserver * pObserver)
{
	BOOL fRet = FALSE;
	m_lstObservers.Lock ();

	pObserver -> ObserverUnsetSubject ();

	// $ note we are comparing pointer values
	POSITION pos = m_lstObservers.Find ( pObserver );
	if (pos)
	{
		m_lstObservers.RemoveAt ( pos );
		fRet = TRUE;
	}

	m_lstObservers.Unlock ();

	ASSERT(fRet);
	return fRet;
}

// ------------------------------------------------------------------------
void TSubject::RemoveAllObservers ()
{
	m_lstObservers.RemoveAll ();
}

// ------------------------------------------------------------------------
void TSubject::UpdateAllObservers ()
{
	m_lstObservers.Lock ();
	POSITION pos = m_lstObservers.GetHeadPosition ();

	while (pos)
	{
		TObserver * pObs = static_cast<TObserver *>(m_lstObservers.GetNext(pos));

		try
		{
			// wake observer up
			pObs->UpdateObserver ();
		}
		catch(...)
		{
			// catch everything
		}
	}

	m_lstObservers.Unlock ();
}
