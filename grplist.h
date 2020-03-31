/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: grplist.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:26  richard_wood
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

// Group List Class
#include "pobject.h"
#include "tglobdef.h"

const int   kGroupTextExpand        = 64*1024;
const int   kSearchListExpand       = 64;
const int   kGroupOffsetExpand      = 5000;
const int   kGroupElemExpand        = 5000;
const TCHAR kGroupSeparator         = _TCHAR (' ');
#if defined(_DEBUG)
const int   kSimpleSearchThreshhold = 2;
#else
const int   kSimpleSearchThreshhold = 50;
#endif

class TSearchStackItem
{
public:
   TSearchStackItem();
   ~TSearchStackItem ();
   void Grow ();
   void AddResult (LONG offset);
public:
   LONG              *  m_pOffsets;
   int                  m_numItems;
   int                  m_arrayItems;
   TSearchStackItem  *  m_pNext;
   CString              m_pattern;
};

class TGroupList : public PObject

{
public:
   typedef struct
   {
      LONG lOffset;
      WORD wNumArticles;
      BYTE byNewsgroupType;         // Y, N, Moderated
   } GroupListElem;

public:
   TGroupList();
   ~TGroupList ();
   void AddGroup(LPCTSTR  group, WORD numArticles, TGlobalDef::ENewsGroupType eType);
   void Empty();
   void Serialize (CArchive & arc);
   void Print (CStdioFile & outfile);
   int  NumGroups ();
   int  GetItem (int iMutableIndex, CString& group,
                 WORD & numArticles, TGlobalDef::ENewsGroupType& eType) const;

   int  GetItem (int iMutableIndex, CString& group) const
        { return get_item (iMutableIndex, group); }

   void Search (LPCTSTR pSubString);
   void ClearSearch ();

   TGroupList & operator += (const TGroupList & rhs);

   BYTE GetGroupType(LPCTSTR group);
   BOOL GroupExist (LPCTSTR group, int * pIdx) const;

private:
   void PopSearch ();
   void GrowText();
   void GrowElements();
   BOOL FindGroup (LPCTSTR group, int & spot) const;
   int  Compare (int index, LPCTSTR group) const;

   // Get ptr to element, accounting for any existing gap
   GroupListElem* GetElemAt(int index) const
      {
      if (index < m_iGapIndex)
         return m_pElem + index;
      else
         return m_pElem + index + m_iGapSize;
      }

   GroupListElem* InsertElem(int index);
   void CompactData(void);

   int get_item (int iMutableIndex, CString & group) const;
   int get_offset (int iMutableIndex, GroupListElem ** ppElem = 0) const;
   int ExtractGroupName (int offset, CString& group) const;

private:

   LONG              m_numGroups;         // current number of groups
   LONG              m_nextInsert;        // offset of next insertion
   TCHAR *           m_pText;             // big block for holding text
   LONG              m_textSize;          // size of text region
   TSearchStackItem *m_pSearchStack;      // pointer to search stack
   GroupListElem *   m_pElem;             //
   LONG              m_iElemCount;        // items allocated
   LONG              m_iGapIndex;         // manages pElem
   LONG              m_iGapSize;          //

   BYTE              m_fEmptySet;         // return emptyset if looking for
                                          //   the space char

#ifdef _DEBUG
   WORD              m_checkSum;
#endif
};

typedef struct T_GROUPLISTDONE
{
   TGroupList * pGroupList;
   int          iNTPRet;
   CString      strAck;
} T_GROUPLISTDONE, * LPT_GROUPLISTDONE;
