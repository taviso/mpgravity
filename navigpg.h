/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: navigpg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/06/16 16:47:42  richard_wood
/*  Fixed spell checker bug.
/*  Fixed missing space in registry key (compared to installer key)
/*  Removed Gopher and Telnet URL checking.
/*  Added in checks for reg keys being deleted.
/*  Changed some default settings to suit modern usage.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:33  richard_wood
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

// navigpg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TOptionsNavigatePage dialog

class TOptionsNavigatePage : public CPropertyPage
{
   DECLARE_DYNCREATE(TOptionsNavigatePage)

public:
   TOptionsNavigatePage();
   ~TOptionsNavigatePage();

   enum { IDD = IDD_OPTIONS_NAVIGATE };
   CTabCtrl m_tab;
   CComboBox   m_cmb1KeyRead_Threaded;    // index 0
   CComboBox   m_cmb1KeyRead_Sorted;
   CComboBox   m_cmbIgnore_Threaded;
   CComboBox   m_cmbIgnore_Sorted;
   CComboBox   m_cmbK_Threaded;
   CComboBox   m_cmbK_Sorted;
   CComboBox   m_cmbKThread_Threaded;
   CComboBox   m_cmbKThread_Sorted;
   CComboBox   m_cmbTag_Threaded;
   CComboBox   m_cmbTag_Sorted;
   CComboBox   m_cmbWatch_Threaded;
   CComboBox   m_cmbWatch_Sorted;         // index 11
   BOOL        m_fMButSKR;

   int   m_vPref[12][2];                  // 2nd dimension is Offline/Online

protected:
   void initialize ();
   void ShowOnlineData (bool fOnline);
   void SaveData (bool fOnline);
   void fill_and_select (CComboBox & cmb, int iAction);
   void save_cmb_action (CComboBox & cmb, int & iAction);

   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnPSNHelp (NMHDR *pNotifyStruct, LRESULT *result);
   DECLARE_MESSAGE_MAP()
};
