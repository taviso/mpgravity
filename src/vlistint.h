/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: vlistint.h,v $
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

#include "vlist.h"

#ifdef WIN32
#define EXPORT
#else
#define EXPORT _export
#endif

typedef struct tagVLISTBox
   {
      HWND      hwnd;             // hwnd of this VLIST box
      int       nId;              // Id of Control
      HINSTANCE hInstance;        // Instance of parent
      HWND      hwndParent;       // hwnd of parent of VLIST box
      HWND      hwndList;         // hwnd of List box
      WNDPROC   lpfnLBWndProc;    // Window procedure of list box
      int       nchHeight;        // Height of text line
      int       nLines;           // Number of lines in listbox
      LONG      styleSave;        // Save the Style Bits
      WORD      VLBoxStyle;       // List Box Style
      HANDLE    hFont;            // Font for List box
      LONG      lToplIndex;      // Top logical record number;
      int       nCountInBox;      // Number of Items in box.
      LONG      lNumLogicalRecs;  // Number of logical records
      VLBSTRUCT vlbStruct;        // Buffer to communicate to app
      WORD      wFlags;           // Various flags fot the VLB
                                  //
                                  // 0x01 - HasStrings
                                  // 0x02 - Use Data Values
                                  // 0x04 - Multiple Selections
                                  // 0x08 - Ok for parent to have focus
                                  // 0x10 - Control has focus

      LONG      lSelItem;         // List of selected items
      int       nvlbRedrawState;  // Redraw State
      BOOL      bHScrollBar;      // Does it have a H Scroll
} VLBOX;

typedef VLBOX NEAR *PVLBOX;
typedef VLBOX FAR  *LPVLBOX;

#define IDS_VLBOXNAME  1
#define VLBLBOXID      100
#define VLBEDITID      101

#define HASSTRINGS     0x01       // List box stores strings
#define USEDATAVALUES  0x02       // Use Data Values to talk to parent
#define MULTIPLESEL    0x04       // VLB has extended or multiple selection
#define PARENTFOCUS    0x08       // Ok for parent to have focus
#define HASFOCUS       0x10       // 0x10 - Control has focus

LRESULT EXPORT WINAPI VListBoxWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT WINAPI LBSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL /*WINAPI*/ RegisterVListBox(HINSTANCE hInstance);
LONG VLBMessageItemHandler( PVLBOX pVLBox,  UINT message, LPSTR lpfoo);
LONG VLBParentMessageHandler( PVLBOX pVLBox, UINT message, WPARAM wParam, LPARAM lParam);
LONG VLBNcCreateHandler( HWND hwnd, LPCREATESTRUCT lpcreateStruct);
LONG VLBCreateHandler( PVLBOX pVListBox, HWND hwnd, LPCREATESTRUCT lpcreateStruct);
void VLBNcDestroyHandler(HWND hwnd,  PVLBOX pVListBox, WPARAM wParam, LPARAM lParam);
void VLBSetFontHandler( PVLBOX pVListBox, HANDLE hFont, BOOL fRedraw);
int  VLBScrollDownLine( PVLBOX pVLBox);
int  VLBScrollUpLine( PVLBOX pVLBox);
int  VLBScrollDownPage( PVLBOX pVLBox, int nAdjustment);
int  VLBScrollUpPage( PVLBOX pVLBox, int nAdjustment);
void UpdateVLBWindow( PVLBOX pVLBox, LPRECT lpRect);
int  VLBFindPage( PVLBOX pVLBox, LONG lFindRecNum, BOOL bUpdateTop);
int  VLBFindPos( PVLBOX pVLBox, int nPos);
void VLBFirstPage( PVLBOX pVLBox);
void VLBLastPage( PVLBOX pVLBox);
LONG vlbSetCurSel( PVLBOX pVLBox, int nOption, LONG lParam);
int  vlbFindData( PVLBOX pVLBox, LONG lData);
void VLBSizeHandler( PVLBOX pVLBox, int nItemHeight);
int  vlbInVLB( PVLBOX pVLBox, LONG lData);
void VLBCountLines( PVLBOX pVLBox);

void vlbRedrawOff(PVLBOX pVLBox);
void vlbRedrawOn(PVLBOX pVLBox);

BOOL TestSelectedItem(PVLBOX pVLBox, VLBSTRUCT vlbStruct);
void SetSelectedItem(PVLBOX pVLBox);

void vlbPGDN(PVLBOX pVLBox);
void vlbPGUP(PVLBOX pVLBox);

void vlbLineDn(PVLBOX pVLBox);
void vlbLineUp(PVLBOX pVLBox);

extern HANDLE  hInstance;              // Global instance handle for  DLL
