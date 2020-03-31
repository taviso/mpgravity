/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: LimitHdr.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:06  richard_wood
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

// LimitHdr.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LimitHdr.h"
#include "tglobopt.h"
#include "rgui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPromptLimitHeaders dialog

CPromptLimitHeaders::CPromptLimitHeaders(CWnd* pParent /*=NULL*/)
	: CDialog(CPromptLimitHeaders::IDD, pParent)
{

	m_iCount = 0;
}

void CPromptLimitHeaders::DoDataExchange(CDataExchange* pDX)
{
   // m_fGetAll is not used anymore
   if (FALSE == pDX->m_bSaveAndValidate)
      {
      int temp=10;
      gpGlobalOptions->GetRegUI()->GetLimitHeadersMRU (m_fGetAll, temp);
      m_iCount = abs(temp);
      }

	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EBX_HDRCOUNT, m_wndEbx);
	DDX_Text(pDX, IDC_EBX_HDRCOUNT, m_iCount);
	DDV_MinMaxUInt(pDX, m_iCount, 1, 10000000);


   if (pDX->m_bSaveAndValidate)
      {
      gpGlobalOptions->GetRegUI()->SetLimitHeadersMRU (m_fGetAll, m_iCount);

      gpGlobalOptions->GetRegUI()->Save ();
      }
}

BEGIN_MESSAGE_MAP(CPromptLimitHeaders, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPromptLimitHeaders message handlers

