/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: autodrw.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:13  richard_wood
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

class TAutoDraw {
public:
   TAutoDraw(CWnd* pWnd, BOOL fInvalidate);
   ~TAutoDraw();

   void SetRedrawOn();
protected:
   enum EAction { kDraw = 1, kInvalidate = 2 };

private:
   CWnd* m_pWnd;
   BYTE  m_byFlags;
};

class TLockDraw {

public:
   TLockDraw(CWnd* pWnd, BOOL fInvalidate)
      {
      m_pWnd = pWnd;
      m_fLocked = pWnd->LockWindowUpdate ();

      if (FALSE == m_fLocked)
         pWnd->SetRedraw (FALSE);

      m_fInvalidate = fInvalidate;
      }

   ~TLockDraw()
      {

      if (m_fLocked)
         {
         m_pWnd->UnlockWindowUpdate ();
         }
      else
         {
         m_pWnd->SetRedraw (TRUE);

         if (m_fInvalidate)
            m_pWnd->Invalidate ();
         }

      }

private:
   CWnd* m_pWnd;
   BOOL  m_fInvalidate;
   BOOL  m_fLocked;
};
