/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutlq.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:21  richard_wood
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

// tutlq.h -- list of queued virtual jobs to go in the utility-dialog

#pragma once

#include "odlist.h"              // COwnerDrawListView

class TUtilityThread;

#define SCROLL_BAR_WIDTH 20   // for leaving room at the end of a listbox row

// -------------------------------------------------------------------------
typedef struct tagTYP_UTIL_DELINFO
{
   int iID;             // unique job id
   int iLbxHintIndex;   // index to check first
} TYP_UTIL_DELINFO, * PTYP_UTIL_DELINFO;

// -------------------------------------------------------------------------
class TUtilityQueue : public COwnerDrawListView
{
public:
   TUtilityQueue() {};
   void Fill ();
   void SetThread (TUtilityThread *pUtilityThread)
      { m_pUtilityThread = pUtilityThread; }
   int GetSelectedIndex ();
   static enum Position {WAIT_Q, CURRENT, COMPLETED};
   void SetPosition (Position iPos) { m_iMyPosition = iPos; }
   virtual void MakeColumns () = 0;
   void InsertJob (void *pJob, int iID, int iIndex = -1);
   int DeleteJob (int iID);
   int DeleteJob (PTYP_UTIL_DELINFO psDeleteInfo);
   void DeleteLine (int iIndex, bool fAdjustSelection = true);
   void MoveItemUp (int iID);
   void MoveItemDown (int iID);
   virtual void GetCurrentLineStatus (CString &str) = 0;
   BOOL IsSelected (int iIndex);
   void Select (int iIndex, BOOL bSelect = TRUE);
   int  GetJobID (int iIndex);
   virtual void SetDefaultWidths () = 0;
   virtual void CheckForHorizontalScrollbar () = 0;

public:
	//{{AFX_VIRTUAL(TUtilityQueue)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

public:
   virtual ~TUtilityQueue() {};

protected:
   virtual void FillRow (void *pJob, LV_ITEM *pLVI) = 0;
   void CopyItem (int iIndex, int iNewIndex);

   Position m_iMyPosition;
   TUtilityThread *m_pUtilityThread;
   int m_iNumColumns;
};
