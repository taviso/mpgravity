/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vlsetcur.cpp,v $
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
/*  Revision 1.2  2008/09/19 14:52:26  richard_wood
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
#include "vlistint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

LONG vlbSetCurSel( PVLBOX pVLBox, int nOption, LONG lParam)
{
   int i;
   if ( pVLBox->wFlags & USEDATAVALUES ) {
       switch ( nOption) {

          case VLB_FIRST:
              VLBFirstPage(pVLBox);
              SendMessage(pVLBox->hwndList, LB_SETCURSEL, 0, 0L);
              SetSelectedItem(pVLBox);
          break;

          case VLB_PREV:
             if ( (i=vlbFindData(pVLBox, lParam)) == LB_ERR ) {
                if  ( VLBFindPage(pVLBox, (LONG)lParam, TRUE) ) {
                    return VLB_ERR;
                }
                if ( pVLBox->nCountInBox < pVLBox->nLines ) {
                   VLBLastPage(pVLBox);
                }
                else {
                    InvalidateRect(pVLBox->hwndList, NULL, TRUE);
                    UpdateWindow(pVLBox->hwndList);
                }
             }
             i=vlbFindData(pVLBox, lParam);
             if ( i == 0 ) {
                if ( VLBScrollUpLine(pVLBox) )
                    return VLB_ERR;
                else
                    SendMessage(pVLBox->hwndList, LB_SETCURSEL, 0, 0L);
                    SetSelectedItem(pVLBox);
             }
             else {
                SendMessage(pVLBox->hwndList, LB_SETCURSEL, i-1, 0L);
                SetSelectedItem(pVLBox);
             }
          break;

          case VLB_NEXT:
             if ( (i=vlbFindData(pVLBox, lParam)) == LB_ERR ) {
                if  ( VLBFindPage(pVLBox, (LONG)lParam, TRUE) ) {
                    return VLB_ERR;
                }
                UpdateWindow(pVLBox->hwndList);
                i=vlbFindData(pVLBox, lParam);
             }
             if ( i == (pVLBox->nCountInBox-1) ) {
                if ( VLBScrollDownLine(pVLBox) )
                    return VLB_ERR;
                else
                    SendMessage(pVLBox->hwndList, LB_SETCURSEL, pVLBox->nCountInBox-1, 0L);
                    SetSelectedItem(pVLBox);
             }
             else {
                if ( SendMessage(pVLBox->hwndList, LB_SETCURSEL, i+1, 0L) == -1L )
                    return VLB_ERR;
                SetSelectedItem(pVLBox);
             }
          break;

          case VLB_LAST:
              VLBLastPage(pVLBox);
              SendMessage(pVLBox->hwndList, LB_SETCURSEL, pVLBox->nCountInBox-1, 0L);
              SetSelectedItem(pVLBox);
          break;

          case VLB_FINDITEM:
             if ( (i=vlbFindData(pVLBox, lParam)) == LB_ERR ) {
                vlbRedrawOff(pVLBox);
                if  ( VLBFindPage(pVLBox, (LONG)lParam, TRUE) )
                    return VLB_ERR;
                else {
                    if ( pVLBox->nCountInBox < pVLBox->nLines ) {
                       VLBLastPage(pVLBox);
                    }
                    i=vlbFindData(pVLBox, lParam);
                    SendMessage(pVLBox->hwndList, LB_SETCURSEL, i, 0L);
                    pVLBox->lSelItem = (LONG) lParam;
                }
                vlbRedrawOn(pVLBox);
             }
             else {
                SendMessage(pVLBox->hwndList, LB_SETCURSEL, i, 0L);
                SetSelectedItem(pVLBox);
             }
          break;
       }
   }
   else {
       pVLBox->vlbStruct.lIndex = lParam;
       switch ( nOption) {
          case VLB_FIRST:
              VLBFirstPage(pVLBox);
              SendMessage(pVLBox->hwndList, LB_SETCURSEL, 0, 0L);
              SetSelectedItem(pVLBox);
          break;

          case VLB_LAST:
              VLBLastPage(pVLBox);
              SendMessage(pVLBox->hwndList, LB_SETCURSEL, pVLBox->nCountInBox-1, 0L);
              SetSelectedItem(pVLBox);
          break;

          case VLB_PREV:
             if ( pVLBox->vlbStruct.lIndex > pVLBox->lToplIndex &&
                  pVLBox->vlbStruct.lIndex <= (pVLBox->lToplIndex+(LONG)(pVLBox->nCountInBox)-1)) {
                if ( SendMessage(pVLBox->hwndList, LB_SETCURSEL, (int)(pVLBox->vlbStruct.lIndex-pVLBox->lToplIndex)-1, 0L) == -1L)
                    return VLB_ERR;
                else
                    SetSelectedItem(pVLBox);
             }
             else {
                if ( VLBScrollUpLine(pVLBox) )
                    return VLB_ERR;
                else {
                    SendMessage(pVLBox->hwndList, LB_SETCURSEL, 0, 0L);
                    SetSelectedItem(pVLBox);
                }
             }
          break;

          case VLB_NEXT:
             if ( pVLBox->vlbStruct.lIndex >= pVLBox->lToplIndex &&
                  pVLBox->vlbStruct.lIndex < (pVLBox->lToplIndex+(LONG)(pVLBox->nCountInBox)-1)) {
                if ( SendMessage(pVLBox->hwndList, LB_SETCURSEL, (int)(pVLBox->vlbStruct.lIndex-pVLBox->lToplIndex)+1, 0L) == -1L )
                    return VLB_ERR;
                else
                    SetSelectedItem(pVLBox);
             }
             else {
                if ( VLBScrollDownLine(pVLBox) )
                    return VLB_ERR;
                else {
                    SendMessage(pVLBox->hwndList, LB_SETCURSEL, pVLBox->nLines-1, 0L);
                    SetSelectedItem(pVLBox);
                }
             }
          break;

          case VLB_FINDITEM:
             if ( pVLBox->vlbStruct.lIndex >= pVLBox->lToplIndex &&
                  pVLBox->vlbStruct.lIndex <= (pVLBox->lToplIndex+(LONG)(pVLBox->nCountInBox)-1)) {
                SendMessage(pVLBox->hwndList, LB_SETCURSEL, (int)(pVLBox->vlbStruct.lIndex-pVLBox->lToplIndex), lParam);
                SetSelectedItem(pVLBox);
             }
             else {
                if ( VLBFindPage(pVLBox, pVLBox->vlbStruct.lIndex, TRUE) )
                    return VLB_ERR;
                else {
                    SendMessage(pVLBox->hwndList, LB_SETCURSEL, 0, 0L);
                    SetSelectedItem(pVLBox);
                }
             }
          break;
       }
   }
   return (LONG)VLB_OK;
}
