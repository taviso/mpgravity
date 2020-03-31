/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vlist.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

#pragma once

#define VLB_OK            0
#define VLB_ERR           -1
#define VLB_ENDOFFILE     -1

#define VLBS_USEDATAVALUES     0x8000L  
#define VLBS_3DFRAME           0x4000L
#define VLBS_NOTIFY            0x0001L
#define VLBS_NOREDRAW          0x0004L
#define VLBS_OWNERDRAWFIXED    0x0010L
#define VLBS_HASSTRINGS        0x0040L
#define VLBS_USETABSTOPS       0x0080L
#define VLBS_NOINTEGRALHEIGHT  0x0100L
#define VLBS_WANTKEYBOARDINPUT 0x0400L
#define VLBS_DISABLENOSCROLL   0x1000L

// Application->VLIST messages               
// Corresponding to LB_ messages
#define VLB_MSGMIN              (WM_USER+500)
#define VLB_RESETCONTENT        (WM_USER+500)
#define VLB_SETCURSEL           (WM_USER+501)
#define VLB_GETCURSEL           (WM_USER+502)
#define VLB_GETTEXT             (WM_USER+503)
#define VLB_GETTEXTLEN          (WM_USER+504)
#define VLB_GETCOUNT            (WM_USER+505)
#define VLB_SELECTSTRING        (WM_USER+506)
#define VLB_FINDSTRING          (WM_USER+507)
#define VLB_GETITEMRECT         (WM_USER+508)
#define VLB_GETITEMDATA         (WM_USER+509)
#define VLB_SETITEMDATA         (WM_USER+510)
#define VLB_SETITEMHEIGHT       (WM_USER+511)
#define VLB_GETITEMHEIGHT       (WM_USER+512)
#define VLB_FINDSTRINGEXACT     (WM_USER+513)
#define VLB_INITIALIZE          (WM_USER+514)
#define VLB_SETTABSTOPS         (WM_USER+515)
#define VLB_GETTOPINDEX         (WM_USER+516)
#define VLB_SETTOPINDEX         (WM_USER+517)
#define VLB_GETHORIZONTALEXTENT (WM_USER+518)
#define VLB_SETHORIZONTALEXTENT (WM_USER+519)

// Unique to VLIST
#define VLB_UPDATEPAGE          (WM_USER+520)
#define VLB_GETLINES            (WM_USER+521)
#define VLB_PAGEDOWN            (WM_USER+522)
#define VLB_PAGEUP              (WM_USER+523)
#define VLB_MSGMAX				(WM_USER+523)

// VLIST->Application messages  
// Conflicts with VLB_
#define VLBR_FINDSTRING         (WM_USER+600) 
#define VLBR_FINDSTRINGEXACT    (WM_USER+601) 
#define VLBR_SELECTSTRING       (WM_USER+602) 
#define VLBR_GETITEMDATA        (WM_USER+603)
#define VLBR_GETTEXT            (WM_USER+604)
#define VLBR_GETTEXTLEN         (WM_USER+605)

// Unique Messages
//
#define VLB_FIRST               (WM_USER+606)
#define VLB_PREV                (WM_USER+607)
#define VLB_NEXT                (WM_USER+608)
#define VLB_LAST                (WM_USER+609)
#define VLB_FINDITEM            (WM_USER+610)
#define VLB_RANGE               (WM_USER+611)
#define VLB_FINDPOS             (WM_USER+612)

// VLIST->Application Notifications
#define VLBN_FREEITEM            (WM_USER+700)
#define VLBN_FREEALL             (WM_USER+701)

#define IDS_VLBOXNAME         1

typedef struct _VLBStruct {
   int   nCtlID;
   int   nStatus;
   LONG  lData;
   LONG  lIndex;
   LPTSTR lpTextPointer;
   LPTSTR lpFindString;
} VLBSTRUCT;

typedef VLBSTRUCT FAR*  LPVLBSTRUCT;

#define VLIST_CLASSNAME _T("VList")

#ifdef __cplusplus
extern "C" {
#endif

   BOOL /*WINAPI*/ RegisterVListBox(HINSTANCE);

#ifdef __cplusplus
}
#endif
