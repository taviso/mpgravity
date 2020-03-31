/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: comptool.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:15  richard_wood
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

// comptool.h -- toolbar used by posting/mailing windows
// This 'Compose-Window' toolbar manages the combobox with the
//   multiple signatures.  I figure this can be used for the
//   Post, Reply and Followup toolbars

#pragma once

#include "sigcomb.h"
#include "btnlock.h"

// fwd reference
class TAttachmentDoc;

// -------------------------------------------------------------------------
class TCompToolBar : public CToolBar
{
public:
   TSigComboBox m_comboBox;
   CComboBox m_cbxMIME;
   CFont m_font;

   BOOL GetSignatureText(int idx, CString& str);
   void GetCurrent(int& iCurrent);
   void SelectSig (const CString & str);
   virtual ~TCompToolBar() {};
   BOOL MimeButtonChecked(void);

protected:
	// Generated message map functions
	//{{AFX_MSG(TCompToolBar)
	afx_msg void OnDestroy();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()

   TCompToolBar()      // only derived classes, please
      {}

   // Override
   // LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

   BOOL Create(CWnd* pParent, int ComboBox_PlacementIndex,
               int cbxMIME_Index);

protected:
   BYTE m_byMimeLocked;
   int  m_iMimeIndex;
};

const int idCompTool_Combo = 102;
const int idCompTool_cbxMime = 103;
