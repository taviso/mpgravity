/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: artrefs.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:11  richard_wood
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

// class used to simulate a string array
class TFlatStringArray : public CObject
{
public:
   DECLARE_SERIAL(TFlatStringArray);
   TFlatStringArray();
   ~TFlatStringArray();
   virtual void Serialize(CArchive& ar);

   TFlatStringArray& operator=(const TFlatStringArray& rhs);

   void AddString(LPCTSTR str);

   // return a string list
   void FillStringList(CStringList* pList) const;

   // tells how many strings are stored
   int GetSize () const;

   LPCTSTR get_string(int i) const;

   void RemoveAll () { cleanUp (); }

protected:
   void    cleanUp();
   int     get_offset(int i) const;

protected:
   LONG   m_count;
   LONG   m_dataSize;
   LPTSTR m_pText;             // start of the string data
   LPTSTR m_pData;             // int, int int... string\0string\0string\0
};

// class used to Flatten References
class T822References : public TFlatStringArray
{
public:
   DECLARE_SERIAL(T822References);
   T822References();
   ~T822References();
   virtual void Serialize(CArchive& ar);

   // operations
   void GetLastRef(CString& str);
   void GetFirstRef(CString& str);

   int  GetCount();
   BOOL Find(const CString & msgID);

   void CopyReferences(const T822References & src);
   void ConstructReferences(const T822References & src, const CString& msgid);

   // set the references from a Header line
   void SetReferences(const CString& inRefs);

   // <GreatGrandDad> <GrandDad> <Dad>
   void MakeWhiteSpaceList(CString& out);
};
