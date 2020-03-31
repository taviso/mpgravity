/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: custmsg.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.4  2009/05/08 14:43:37  richard_wood
/*  Added Auto Update Check.
/*  Removed "Using Help" menu command.
/*  Reversed "Connect to Server" button image, it shows connected when it is connected now.
/*
/*  Revision 1.3  2008/09/19 14:51:16  richard_wood
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

#define WMU_MODE1_HDRS_DONE            (WM_USER + 100)
#define WMU_PRIORITYARTICLE            (WM_USER + 101)
#define WMU_VCR_IDLE                   (WM_USER + 102)
#define WMU_VCR_GROUPDONE              (WM_USER + 103)
#define WMU_VCR_NEXTSTEP               (WM_USER + 104)
#define WMU_VCR_BADSERVER              (WM_USER + 129)
#define WMU_NGROUP_HDRS_DONE           (WM_USER + 105)
   // meaning - the headers for this newsgroup have all been downloaded
   // syntax LPARAM  is a CString*

#define WMU_PROGRESS_SETTEXT           (WM_USER + 120)
   // LPARAM - string LPCTSTR

#define WMU_PROGRESS_SETRANGE          (WM_USER + 121)
   // WPARAM - low
   // LPARAM - high
#define WMU_PROGRESS_STEP              (WM_USER + 122)

#define WMU_PROGRESS_POS               (WM_USER + 123)
   // LPARAM - pos

#define WMU_PROGRESS_END               (WM_USER + 124)
   // reset text and zeroout

#define WMU_CHILD_FOCUS                (WM_USER + 130)
   // let's a parent know what pane currently has focus
   // the child should send this message
   // lParam has [0-2]

#define WMU_CHILD_TAB                  (WM_USER + 131)
   // a child has received a VK_TAB
   //  the parent has responsibility of moving the focus
   //  wParam - 1 if ShiftKey is down
   //  lParam - index

#define WMU_CHILD_ESC                  (WM_USER + 132)

#define WMU_MAINFRAME_SAVE_STATUSMGR   (WM_USER + 142)
   // sent from ::OnIdle or when we have 10 or more status-deltas to save
   //   Action: check all the newsgroups, and save anything that is dirty

#define WMU_NEWSVIEW_NEWFONT           (WM_USER + 143)
   // sent from newsopt.cpp  The newsview will use the new font

#define WMU_TREEVIEW_NEWFONT           (WM_USER + 144)
   // sent from newsopt.cpp  The treectrl will apply the new font

#define WMU_NEWSMDI_NEWLAYOUT          (WM_USER + 145)
   // sent from newsopt.cpp  The mdi window will use a new layout

#define WMU_SEARCH_THREADTALK          (WM_USER + 146)
   // sent from the search worker thread to the modeless search dialog
   // to add results and notify the dialog that the search has
   // completed - see tsrchdlg.h for WPARAM and LPARAM
   // definitions

#define WMU_ARTHDR_DESTROYED           (WM_USER + 147)
   // LPARAM - oid of dead object

#define WMU_TREEVIEW_REFILL            (WM_USER + 148)
   // used to refill.  Toggle between sort by date sort-by-subject

#define WMU_FLATVIEW_NEWFONT           (WM_USER + 149)
#define WMU_PREPARE_TO_CLOSE           (WM_USER + 150)
#define WMU_NEWSVIEW_GOTOARTICLE       (WM_USER + 151)
#define WMU_RASDIAL_EVENT              (WM_USER + 152)
#define WMU_RASDIAL_BUSY               (WM_USER + 154)
#define WMU_ARTVIEW_NEWTEMPLATE        (WM_USER + 155)
#define WMU_CHECK_UIPIPE               (WM_USER + 156)

#define WMU_POST_INITIALUPDATE         (WM_USER + 157)
   // used as a WM_COMMAND id in postpfrm.cpp

#define WMC_DISPLAY_ARTCOUNT           (WM_USER + 158)
   // used as a WM_COMMAND id in newsview

#define WMU_NEWSVIEW_PROCESS_MAILTO    (WM_USER + 159)

// IMPORTANT
//    WM_USER + 160     are used by the Premia Tree Control
//    WM_USER + 170

#define WMU_STATUS_AUTOREFRESH         (WM_USER + 180)
#define WMU_ARTVIEW_UPDATEPOPMENU      (WM_USER + 181)
#define WMU_REFRESH_OUTBOX             (WM_USER + 182)
#define WMU_REFRESH_OUTBOX_BAR         (WM_USER + 183)
#define WMU_PUMP_ARTICLE               (WM_USER + 184)
#define WMU_PROCESS_PUMP_ARTICLE       (WM_USER + 185)
#define WMU_NONBLOCKING_CURSOR         (WM_USER + 186)
#define WMU_CONNECT_OK                 (WM_USER + 187)
#define WMU_TREEVIEW_RESETHDR          (WM_USER + 188)
#define WMU_FLATVIEW_RESETHDR          (WM_USER + 189)
#define WMC_DISPLAYALL_ARTCOUNT        (WM_USER + 190)
#define WMU_GETLIST_DONE               (WM_USER + 191)

// ui thread does work to purge articles
#define WMU_EXPIRE_ARTICLES            (WM_USER + 192)

// verify that the Article has a subject, etc...
#define WMU_VERIFY_POSTHDR             (WM_USER + 193)

#define WMU_THRDLVW_RESIZE             (WM_USER + 194)

// start compacting...
#define WMU_START_COMPACT              (WM_USER + 195)

// force UI to show messagebox
#define WMU_ERROR_FROM_SERVER          (WM_USER + 196)

// newsview send to itself
#define WMU_SELCHANGE_OPEN             (WM_USER + 198)

// first part of mode 1 newsgroup
#define WMU_MODE1_GOTPING              (WM_USER + 199)

// One Pane Mode cares about actions. wParam = TGlobalDef::EUserAction
#define WMU_USER_ACTION                (WM_USER + 200)

// send to mainfrm. from search dlg
#define WMU_FORCE_FILTER_CHANGE        (WM_USER + 201)

// NewsURL message from another instance
#define WMU_NEWSURL                    (WM_USER + 210)

#define WMU_SPLASH_GONE                (WM_USER + 211)

#define WMU_DOCK_DIR_BAR_ON_RIGHT_OF_TOOLBAR (WM_USER + 212)

// sent from the keyboard hook dll (gravmon.dll)
#define WMU_PANIC_KEY                  (WM_USER + 220)

// sent from middle list-control in image factory to main frame
#define WMU_RESET_FACTORY_COLUMNS      (WM_USER + 221)

#define WMU_SERVER_SWITCH              (WM_USER + 222)

// disk space low. scribe post msg to disconnect.  wParam(1) indicates E-pump
#define WMU_INTERNAL_DISCONNECT        (WM_USER + 223)

#define WMU_LOW_SPACE                  (WM_USER + 224)

#define WMU_GROUPRENUMBERED_WARNING    (WM_USER + 225)

#define WMU_BOZO_CONVERTED             (WM_USER + 226)

#define WMU_UPDATE_CHECKCOMPLETE       (WM_USER + 227)

#define WMU_MBUTTON_DOWN               (WM_USER + 228)

#define WMU_SHOW_CONNECT_RESULT        (WM_USER + 229)

#define WMU_READYTO_RUN                (WM_USER + 230)

#define WMU_CUSTOM_MOUSEUP             (WM_USER + 231)

#define WMU_DECODE                     (WM_USER + 232)

#define WMU_QUICK_FILTER               (WM_USER + 233)

#define WMU_REFRESH_DRAFT              (WM_USER + 234)

#define WMU_RTF_YIELD                  (WM_USER + 235)

#define WMU_CHECKFORUPDATE             (WM_USER + 236)