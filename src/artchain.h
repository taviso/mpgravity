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

#include <vector>
using namespace std;

const long kInvalidGroup = -1;

struct CGroupArt
{
   long m_groupID;
   long m_artNum;
};

class CArticleChain
{
   public:
      CArticleChain () {m_it = m_chain.begin();}
      ~CArticleChain () {}

      bool Curr(long & groupID, long & artNum) 
         {
         if (m_chain.size())
            {
            groupID = (*m_it).m_groupID;
            artNum  = (*m_it).m_artNum;
            return true;
            }
         else
            return false;
         }


      bool Forward(long & groupID, long & artNum) // move forward one
         {
         if ((m_it == m_chain.end()) ||
             m_it == (m_chain.end() - 1))
             return false;

         if ((*m_it).m_groupID == kInvalidGroup)
            m_it = m_chain.erase (m_it);

         ++m_it;

         CGroupArt & ga = *m_it;
         groupID = ga.m_groupID;
         artNum  = ga.m_artNum;

         return true;
         }

      bool Back (long & groupID, long & artNum) // move back one
         {
         if (m_chain.begin() == m_it)
             return false;

         if ((*m_it).m_groupID == kInvalidGroup)
            m_chain.erase (m_it);
         --m_it;
         CGroupArt & ga = *m_it;
         groupID = ga.m_groupID;
         artNum  = ga.m_artNum;
         return true;
         }

      void Empty() // clear out everything
         {
         m_chain.clear();
         m_it = m_chain.begin();
         }

      void Clear() // insert a empty view marker if there isn't one 
         {
         if (!m_chain.size())
            return;
         if ((*m_it).m_groupID != kInvalidGroup)
            {
            CGroupArt ga;
            ga.m_groupID = kInvalidGroup;
            ga.m_artNum = 0;
            InsertAfter (ga);
            }
         }

      void New (long groupID, long artNum)
         {
         // if we're not at the end, lop off everything after us...
         CGroupArt groupArt;
         groupArt.m_groupID = groupID;
         groupArt.m_artNum  = artNum;

         if (m_chain.size()  &&
             (m_chain.end() != (m_it + 1)))
            {
            m_chain.erase (++m_it, m_chain.end());
            m_it = m_chain.insert (m_chain.end(), groupArt);
            }
         else
            {
            InsertAfter ( groupArt );
            }
         }

      void InsertAfter (CGroupArt & GroupArt)
         {
            if (m_it == m_chain.end())
               m_it = m_chain.insert (m_chain.end(), GroupArt);
            else
               m_it = m_chain.insert (++m_it, GroupArt);
         }

      bool AtEnd()
         {
         if (!m_chain.size() ||
             (m_it == (m_chain.end() - 1)) ||
             m_it == m_chain.end())
             return true;
         return false;
         }

      bool AtBegin ()
         {
         if (!m_chain.size() ||
            (m_it == m_chain.begin()))
            return true;
         return false;
         }

   private:
      vector<CGroupArt>             m_chain;
      vector<CGroupArt>::iterator   m_it;
};

