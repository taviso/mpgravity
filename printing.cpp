/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: printing.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:41  richard_wood
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

// printing.cpp -- handles print messages from the menus

#include "stdafx.h"              // standard stuff
#include "printing.h"            // this file's prototypes
#include "resource.h"            // something below wants this, IDS_ERR...
#include "newsgrp.h"             // TNewsGroup
#include "thrdlvw.h"             // TThreadListView
#include "mlayout.h"             // TMdiLayout
#include "tprnthrd.h"            // gpPrintThread, ...
#include "tprnjob.h"             // TPrintJob
#include "genutil.h"             // GetThreadView()
#include "rgui.h"                // TRegUI
#include "tglobopt.h"            // gpGlobalOptions
#include "globals.h"             // gpStore
#include "server.h"              // TNewsServer
#include "newsdb.h"              // TNewsDB
#include "rgfont.h"              // TRegFonts
#include "rgswit.h"              // TRegSwitch

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// QueueArticleForPrinting -- takes an article and a flag that says whether
// it's selected (in one of the article views), and queues it for printing.
// Returns nonzero if an error occurs, otherwise returns zero
int QueueArticleForPrinting (TArticleHeader *pHdr, int iSelected,
   TNewsGroup *pNG, DWORD dwData)
{
   if (!iSelected)
      return 0;

   // queue the article
   CString strGroupNickname = pNG -> GetBestname ();

   TPrintJob *pJob = new TPrintJob (pNG -> m_GroupID, pNG -> GetName (),
      strGroupNickname, pHdr, FALSE /* iRuleBased */, (HDC) dwData);
   gpPrintThread -> AddJob (pJob);
   return 0;
}

// -------------------------------------------------------------------------
CPrintSetup::CPrintSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintSetup::IDD, pParent)
{

	m_fBottom = 0.0f;
	m_fLeft = 0.0f;
	m_fRight = 0.0f;
	m_fTop = 0.0f;
	m_bFullHeader = FALSE;
}

// -------------------------------------------------------------------------
void CPrintSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_BOTTOM_MARGIN, m_fBottom);
	DDX_Text(pDX, IDC_LEFT_MARGIN, m_fLeft);
	DDX_Text(pDX, IDC_RIGHT_MARGIN, m_fRight);
	DDX_Text(pDX, IDC_TOP_MARGIN, m_fTop);
	DDX_Check(pDX, IDC_FULL_HEADER, m_bFullHeader);
}

// -------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CPrintSetup, CDialog)
		ON_BN_CLICKED(IDC_FONT, OnFont)

END_MESSAGE_MAP()

// -------------------------------------------------------------------------
BOOL CPrintSetup::OnInitDialog() 
{
   TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
   int iTop, iBottom, iLeft, iRight;
   pRegUI -> GetPrintMargins (iLeft, iRight, iTop, iBottom);
   m_fTop = (float) iTop / 100;
   m_fBottom = (float) iBottom / 100;
   m_fRight = (float) iRight / 100;
   m_fLeft = (float) iLeft / 100;

   TRegSwitch *pSwitch = gpGlobalOptions -> GetRegSwitch ();
   m_bFullHeader = pSwitch -> m_fPrintFullHeader;

   TRegFonts *pFonts = gpGlobalOptions -> GetRegFonts ();
   m_dwColor = pFonts -> GetPrintTextColor ();
   m_iPointSize = pFonts -> GetPrintPointSize ();
   m_sFont = * (pFonts -> GetPrintFont ());

	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
}

// -------------------------------------------------------------------------
void CPrintSetup::OnOK() 
{
   if (!UpdateData ())
      return;  // data validation error
   int iTop, iBottom, iLeft, iRight;
   iTop = (int) (m_fTop * 100 + .1);   // + .1 to make sure it rounds up
   iBottom = (int) (m_fBottom * 100 + .1);
   iLeft = (int) (m_fLeft * 100 + .1);
   iRight = (int) (m_fRight * 100 + .1);
   TRegUI *pRegUI = gpGlobalOptions -> GetRegUI ();
   pRegUI -> SetPrintMargins (iLeft, iRight, iTop, iBottom);

   TRegSwitch *pSwitch = gpGlobalOptions -> GetRegSwitch ();
   pSwitch -> m_fPrintFullHeader = m_bFullHeader;

   TRegFonts *pFonts = gpGlobalOptions -> GetRegFonts ();
   pFonts -> SetPrintTextColor (m_dwColor);
   pFonts -> SetPrintPointSize (m_iPointSize);
   pFonts -> SetPrintFont (&m_sFont);

   gpStore -> SaveGlobalOptions ();

   CDialog::OnOK();
}

// -------------------------------------------------------------------------
void CPrintSetup::OnFont() 
{
   CFontDialog sDlg;
   sDlg.m_cf.lpLogFont = &m_sFont;
   sDlg.m_cf.iPointSize = m_iPointSize * 10;
   sDlg.m_cf.rgbColors = m_dwColor;
   sDlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
   if (sDlg.DoModal () != IDOK)
      return;
   sDlg.GetCurrentFont (&m_sFont);
   m_dwColor = sDlg.GetColor ();
   m_iPointSize = sDlg.GetSize () / 10;
}
