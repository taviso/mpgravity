/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tutlq.cpp,v $
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

// tutlq.cpp -- list of queued virtual jobs to go in the utility-dialog

#include "stdafx.h"
#include "tutlq.h"            // this file's prototypes
#include "tutlthrd.h"         // TUtilityThread
#include "tutljob.h"          // TUtilityJob

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// Fill -- fills the listbox with the current contents of the utility thread's
// job queue
void TUtilityQueue::Fill ()
{
   LV_ITEM lvi;
   ZeroMemory (&lvi, sizeof (lvi));
   lvi.mask = LVIF_TEXT;
   void *pJob;

   DeleteAllItems ();

   // go through the appropriate utility-thread's job queue
   if (m_iMyPosition == CURRENT) {
      if ((pJob = (void *) m_pUtilityThread -> GetCurrentJob ()) == NULL)
         return;
      FillRow (pJob, &lvi);
      return;
      }

   POSITION iPos;
   if (m_iMyPosition == WAIT_Q)
      iPos = m_pUtilityThread -> GetHeadPosition ();
   else
      iPos = m_pUtilityThread -> GetCompletedHeadPosition ();
   int i = 0;     // counts the items we're inserting
   while (iPos) {
      POSITION iThisItemsPos = iPos;
      if (m_iMyPosition == WAIT_Q)
         pJob = (void *) m_pUtilityThread -> GetNext (iPos);
      else
         pJob = (void *) m_pUtilityThread -> GetCompletedNext (iPos);
      ASSERT (pJob);
      // insert into CListCtrl
      lvi.iItem = i;
      FillRow (pJob, &lvi);
      // force the new item to be visible
      EnsureVisible (i, FALSE /* bPartialOK */);
      SetItemData (i, ((TUtilityJob *) pJob) -> m_iID);
      i++;
      }
}

// -------------------------------------------------------------------------
// InsertJob -- inserts a job into a particular index position
void TUtilityQueue::InsertJob (void *pJob, int iID, int iIndex /* = -1 */)
{
   ASSERT (pJob);

   LV_ITEM lvi;
   ZeroMemory (&lvi, sizeof (lvi));
   lvi.mask = LVIF_TEXT;

   // insert into CListCtrl
   if (iIndex == -1)
      iIndex = GetItemCount ();     // insert at end of list
   lvi.iItem = iIndex;
   FillRow (pJob, &lvi);
   SetItemData (lvi.iItem, (DWORD) iID); // remember the ID of this job
}

// -------------------------------------------------------------------------
// GetSelectedIndex -- gets the index of the currently-selected item, or -1
// if no item is selected
int TUtilityQueue::GetSelectedIndex ()
{
   int ret = GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

   // go through the listbox's items, and look for the first selected one
#if defined(_DEBUG)
   int iCount = GetItemCount ();
   int nVerify = -1;

   for (int i = 0; i < iCount; i++)
      if (GetItemState (i, LVIS_SELECTED))
         {
         nVerify = i;
         break;
         }

   ASSERT(ret == nVerify);
#endif

   return ret;
}

// -------------------------------------------------------------------------
BOOL TUtilityQueue::IsSelected (int iIndex)
{
   int iCount = GetItemCount ();
   if (iIndex >= iCount || iIndex < 0) {
      ASSERT (0);
      return 0;
      }
   return GetItemState (iIndex, LVIS_SELECTED) != 0;
}

// -------------------------------------------------------------------------
void TUtilityQueue::Select (int iIndex, BOOL bSelect /* = TRUE */)
{
   int iCount = GetItemCount ();
   if (iIndex >= iCount || iIndex < 0) {
      ASSERT (0);
      return;
      }
   SetItemState (iIndex, bSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

// -------------------------------------------------------------------------
// DeleteJob -- delete a specific element from the queue, given its item-data.
// returns the index of the deleted item, or -1 for failure
int TUtilityQueue::DeleteJob (int iID)
{
   int iLen = GetItemCount ();
   for (int i = 0; i < iLen; i++)
      if ((int) GetItemData (i) == iID) {
         DeleteLine (i);
         return i;
         }
   return -1;
}

// -------------------------------------------------------------------------
// DeleteJob -- delete a specific element from the queue, given its item-data.
// returns the index of the deleted item, or -1 for failure
int TUtilityQueue::DeleteJob (PTYP_UTIL_DELINFO psDeleteInfo)
{
   int iLen = GetItemCount ();
   if (0 == iLen)
      return -1;

   // see if the hint index exists and that it matches
   if ((psDeleteInfo->iLbxHintIndex > -1) &&
       ((int) GetItemData (psDeleteInfo->iLbxHintIndex) == psDeleteInfo->iID)) {
      DeleteLine (psDeleteInfo->iLbxHintIndex);
      return psDeleteInfo->iLbxHintIndex;
      }

   // ok, do it the hard way
   return DeleteJob (psDeleteInfo->iID);
}

// -------------------------------------------------------------------------
// DeleteLine -- deletes a line from list control and resets selection
void TUtilityQueue::DeleteLine (int iIndex, bool fAdjustSelection /* =true */)
{
   // if this item is selected, select another nearby one
   if (fAdjustSelection && GetItemState (iIndex, LVIS_SELECTED)) {
      int iSelect = iIndex;
      if (iSelect == GetItemCount() - 1)
         iSelect --;
      else
         iSelect ++;
      SetItemState (iSelect, LVIS_SELECTED, LVIS_SELECTED);
      EnsureVisible (iSelect, FALSE);
      }
   DeleteItem (iIndex);
}

// -------------------------------------------------------------------------
// MoveItemUp -- moves an item up given its item-data
void TUtilityQueue::MoveItemUp (int iID)
{
   int iLen = GetItemCount ();
   for (int i = 0; i < iLen; i++)
      if ((int) GetItemData (i) == iID) {
         if (i > 0) {
            CopyItem (i, i-1);
            DeleteItem (i+1);    // used to be `i'
            SetItemState (i - 1, LVIS_SELECTED, LVIS_SELECTED);
            EnsureVisible (i - 1, FALSE);
            }
         break;
         }
}

// -------------------------------------------------------------------------
// MoveItemDown -- moves an item up given its item-data
void TUtilityQueue::MoveItemDown (int iID)
{
   int iLen = GetItemCount ();
   for (int i = 0; i < iLen; i++)
      if ((int) GetItemData (i) == iID) {
         if (i < GetItemCount () - 1) {   // if it's not the last one already
            CopyItem (i, i+2);
            DeleteItem (i);
            SetItemState (i + 1, LVIS_SELECTED, LVIS_SELECTED);
            EnsureVisible (i + 1, FALSE);
            }
         break;
         }
}

// -------------------------------------------------------------------------
// CopyItem -- copies a queue item to a given index
void TUtilityQueue::CopyItem (int iIndex, int iNewIndex)
{
   // insert a new item at the new index
   InsertItem (iNewIndex, (char *) (LPCTSTR) GetItemText (iIndex, 0));

   // iIndex may have changed
   if (iIndex > iNewIndex)
      iIndex++;

   // give the new item the item-data of the old item
   SetItemData (iNewIndex, GetItemData (iIndex));

   // copy text from subsequent columns to the new item
   for (int i = 0; i < m_iNumColumns; i++)
      SetItemText (iNewIndex, i, (char *) (LPCTSTR) GetItemText (iIndex, i));
}

// -------------------------------------------------------------------------
// GetJobID -- return id stored as the item-data
int TUtilityQueue::GetJobID (int iIndex)
{
   return (int) GetItemData (iIndex);
}

// -------------------------------------------------------------------------
BOOL TUtilityQueue::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
   NMHDR *pHdr = (NMHDR *) lParam;

   // for middle list, if user has dragged the columns so that the total
   // column widths exceed the control's width, reset the column widths
   if (m_iMyPosition == CURRENT &&
       (pHdr -> code == HDN_ENDTRACKA || pHdr -> code == HDN_ENDTRACKW))
      CheckForHorizontalScrollbar ();

	return COwnerDrawListView::OnNotify(wParam, lParam, pResult);
}
