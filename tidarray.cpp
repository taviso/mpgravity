/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tidarray.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:06  richard_wood
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

// tidarray.cpp -- array of message IDs

#include "stdafx.h"           // standard stuff
#include "pobject.h"          // for tidarray.h
#include "tidarray.h"         // this file's prototypes
#include "globals.h"          // gpStore
#include "names.h"            // NAME_POSTED_ARTICLE_IDS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const int TIDArray::m_iGrowthIncrement = 10;

// -------------------------------------------------------------------------
#define TIDARRAY_VERSION_NUMBER 1
IMPLEMENT_SERIAL  (TIDArray, PObject, TIDARRAY_VERSION_NUMBER)

// -------------------------------------------------------------------------
// constructor
TIDArray::TIDArray () : PObject (TIDARRAY_VERSION_NUMBER)
{
   m_iDirty = FALSE;
   m_iNextInsertion = 0;

   // if the program is starting without an existing database, we must set
   // the m_iNumToSave member here
   m_iNumToSave = 20;      // should pull this constant from somewhere else??
}

// -------------------------------------------------------------------------
// Serialize -- called by database code
void TIDArray::Serialize (CArchive &archive)
{
   PObject::Serialize (archive);
   if (archive.IsStoring()) {
      archive << (unsigned short) m_iNextInsertion;
      m_rstrIDs.Serialize (archive);
      }
   else {
      if (GetObjectVersion () < TIDARRAY_VERSION_NUMBER) {
         // it's an older version, here we need to read the object in its
         // original format then convert it...
         ASSERT (0);
         }
      else {
         archive >> (unsigned short) m_iNextInsertion;
         m_rstrIDs.Serialize (archive);

         // build the initial aggregate string
         m_strAggregate = "";
         for (int i = 0; i < m_rstrIDs.GetSize(); i++)
            m_strAggregate += m_rstrIDs [i];
         }
      }

   m_iDirty = FALSE;
}

// -------------------------------------------------------------------------
// RememberMessageID -- records a message ID string in the list that
// we're keeping
void TIDArray::RememberMessageID (const CString &strMessageID)
{
   if (!m_iNumToSave)
      return;

   // destroy the old CString sitting in our spot, if it exists
   if (m_iNextInsertion <= m_rstrIDs.GetUpperBound() && m_rstrIDs [m_iNextInsertion])
      m_rstrIDs.RemoveAt (m_iNextInsertion);

   // add the new CString to the CStringArray
   m_rstrIDs.SetAtGrow (m_iNextInsertion, strMessageID);

   // increment the next-insertion pointer
   m_iNextInsertion = (m_iNextInsertion + 1) % m_iNumToSave;

   // add the new CString to the aggregate
   m_strAggregate += strMessageID;

   m_iDirty = TRUE;
}

// -------------------------------------------------------------------------
// IsMessageIDInSet -- is a particular string in our global list of
// message IDs?
int TIDArray::IsMessageIDInSet (LPCTSTR pchMessageID)
{
   // first, use the fast search on the big CString.  If that produces a
   // possible match, use the slower search on the string array, since
   // searching the big string could give a false positive result

   // case-sensitive search OK
   if (m_strAggregate.Find (pchMessageID) == -1)
      return FALSE;

   // now look at the string array
   for (int i = 0; i < m_rstrIDs.GetSize (); i++)
      if (m_rstrIDs [i] == pchMessageID)
         return TRUE;

   return FALSE;
}

// -------------------------------------------------------------------------
// IsSomeMessageIDInSet -- takes a message ID list and tells whether any of
// its IDs exist in our global list of message IDs
int TIDArray::IsSomeMessageIDInSet (const CStringList * pMessageIDList)
{
   POSITION pos = pMessageIDList -> GetHeadPosition();
   while (pos)
      if (IsMessageIDInSet (pMessageIDList -> GetNext (pos)))
         return TRUE;

   return FALSE;
}

// -------------------------------------------------------------------------
// SetMessageIDsSaved -- sets the number of message IDs to save
void TIDArray::SetMessageIDsSaved (unsigned short iNumToSave,
   int iAdjustArray)
{
   if (m_iNumToSave == iNumToSave)
      return;

   // shrink the array if needed
   if (iAdjustArray && iNumToSave < m_iNumToSave) {
      m_rstrIDs.SetSize (iNumToSave, m_iGrowthIncrement);
      m_iDirty = TRUE;
      }

   // if the next-insertion pointer is now invalid, adjust
   if (iNumToSave <= m_iNextInsertion) {
      m_iNextInsertion = 0;
      m_iDirty = TRUE;
      }

   m_iNumToSave = iNumToSave;
}
