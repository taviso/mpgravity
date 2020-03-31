/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: lay3lbx.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:29  richard_wood
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

// lay3lbx.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "lay3lbx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// map ENUM to the picture position in the CImageList
static  LAYSYNC vLayoutSync[] = {
   { 0, TUIMemory::LAY3_2TOP},
   { 1, TUIMemory::LAY3_2BOT},
   { 2, TUIMemory::LAY3_2LEFT},
   { 3, TUIMemory::LAY3_2RIGHT},
   { 4, TUIMemory::LAY3_3HIGH},
   };
/////////////////////////////////////////////////////////////////////////////
// TLayout3Lbx

TLayout3Lbx::TLayout3Lbx()
   : TLayoutLbx(IDB_PANE3DESC)
{
}

TLayout3Lbx::~TLayout3Lbx()
{
}

BEGIN_MESSAGE_MAP(TLayout3Lbx, TLayoutLbx)
			// NOTE - the ClassWizard will add and remove mapping macros here.
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TLayout3Lbx message handlers

int
TLayout3Lbx::GetLaySync(const LAYSYNC* & pSync, int& total)
{
   total = sizeof(vLayoutSync)/sizeof(vLayoutSync[0]);
   pSync = vLayoutSync;
   return 0;
}
