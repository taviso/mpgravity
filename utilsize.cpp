/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: utilsize.cpp,v $
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
/*  Revision 1.3  2009/01/29 17:22:35  richard_wood
/*  Tidying up source code.
/*  Removing dead classes.
/*
/*  Revision 1.2  2008/09/19 14:52:25  richard_wood
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
#include "utilsize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////
// Inset a window w.r.t. a parent's client area
// Inset margin has a default value
int Utility_InsetWindow (CWnd* pWnd, int insetMargin)
{
	if (NULL == pWnd) {
		ASSERT(0);
		return 0;
	}

	CWnd* pParent = pWnd->GetParent();
	if (NULL == pParent) {
		ASSERT(0);
		return 0;
	}

	RECT rct;

	pParent->GetClientRect ( &rct );

	// don't shrink down to nothing
	if (rct.bottom <= 2*insetMargin)
		rct.bottom = 2 * insetMargin + 2;

	if (rct.right <= 2*insetMargin)
		rct.right = 2 * insetMargin + 2;

	pWnd->MoveWindow (insetMargin, insetMargin,
		rct.right - 2*insetMargin, rct.bottom - 2*insetMargin, TRUE);

	//return the width
	return rct.right - 2 * insetMargin;
}

int Utility_GetPosParent(CWnd* pDad, int id, CRect& rRect)
{
	CWnd* pChild = pDad->GetDlgItem(id);
	return Utility_GetPosParent ( pDad, pChild, rRect );
}

int Utility_GetPosParent(CWnd* pDad, CWnd* pChild, CRect& rRect)
{
	pChild->GetClientRect(&rRect);
	pChild->MapWindowPoints ( pDad, &rRect );
	return 0;
}

// pPos must be zeroed
int Utility_PlaceHdrCtrl(WINDOWPOS* pPos, int iParentCX, CHeaderCtrl* pHdr)
{
	RECT rct;
	rct.top = rct.left = 0;
	rct.right = iParentCX;
	rct.bottom = 17;

	HD_LAYOUT sLayout;
	sLayout.prc = &rct;
	sLayout.pwpos = pPos;
	pHdr->Layout( &sLayout );

	pHdr->SetWindowPos(NULL, pPos->x, pPos->y, pPos->cx,
		pPos->cy, pPos->flags | SWP_NOZORDER);
	return 0;
}

// --------------------------------------------------------------------------
void Utility_SetWindowPlacementPreCreate (WINDOWPLACEMENT & wp,
										  CWnd * pWnd)
{
	int left  = wp.rcNormalPosition.left;
	int top   = wp.rcNormalPosition.top;

	// restore width and height
	int cx = wp.rcNormalPosition.right -  left;
	int cy = wp.rcNormalPosition.bottom - top;

	// the following correction is needed when the taskbar is
	// at the left or top and it is not "auto-hidden"
	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
	left += workArea.left;
	top  += workArea.top;

	// make sure the window is not completely out of sight
	int max_x = GetSystemMetrics(SM_CXSCREEN) -
		GetSystemMetrics(SM_CXICON);
	int max_y = GetSystemMetrics(SM_CYSCREEN) -
		GetSystemMetrics(SM_CYICON);

	pWnd->SetWindowPos (NULL,
		min(left, max_x),
		min(top, max_y),
		cx,
		cy,
		SWP_NOZORDER);
}

