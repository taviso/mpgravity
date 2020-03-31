/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: taglist.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:57  richard_wood
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

#include "declare.h"
#include "pobject.h"

#include "sortlst.h"
class TTagArt : public PObject
{
public:
   DECLARE_SERIAL(TTagArt)
   virtual void Serialize(CArchive & ar);
   virtual void SerializeVer464(CArchive& ar);

   TTagArt();
   TTagArt(int iArtNum, int iLines, TArticleHeader * pHdr);
   ~TTagArt();

   LONG m_artInt;
   LONG m_iLines;
   BYTE m_fSubmitted;
   TArticleHeader * m_pArtHdr;
};

class TTagElement : public PObject
{
public:
   DECLARE_SERIAL(TTagElement)
   virtual void Serialize(CArchive& ar);
   virtual void SerializeVer464(CArchive& ar);

   static int fnSortCompare(const CObject* pObj1, const CObject* pObj2);
   static int fnSearchCompare(void * pVoid, const CObject * pObj1);

   TTagElement();
   TTagElement(const CString & groupName, int groupID);
   ~TTagElement();
   void Clean();
   BOOL FindTag(int artInt);
   int Add (int artInt, int iLines, TArticleHeader* pHdr);

   int InitializeSubmit ();

   CString          m_groupName;
   LONG             m_GroupID;
   TSortedPtrArray<CObArray, TTagArt*> m_rArt;
   BOOL             m_fSorted;
};

class TPersistentTags : public PObject
{
public:
   DECLARE_SERIAL(TPersistentTags)
   virtual void Serialize(CArchive& ar);
   virtual void SerializeVer464(CArchive& ar, LONG items);

   TPersistentTags();
   ~TPersistentTags();

   int AddTag (TNewsGroup *pNG, int artInt, int iLines,
               TArticleHeader* pHdr = 0);
   int DeleteTagCancelJob (const CString & groupName, int groupID, int artInt,
                           BOOL fCancelDownloadJob);

   BOOL FindTag (int groupID, int artInt);

   int RetrieveTagged (bool fAllGroups, CDWordArray & vecGroupIDs);
   int InitializeSubmit ();

   // used only by server.cpp
   CRITICAL_SECTION * GetpCriticalSection () { return &m_CritSect; }

protected:
   int delete_tag(int groupID, int artInt, int * piSubmitted = 0);

   CTypedPtrArray<CPtrArray,TTagElement*> m_ptrArray;
   CRITICAL_SECTION m_CritSect;
};
