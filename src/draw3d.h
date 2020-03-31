/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: draw3d.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:23  richard_wood
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

/*-----------------------------------------------------------------------
|   Draw3d - Routines to help add 3D effects to Windows
-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

// Index Color Table
// WARNING: change mpicvSysColors if you change the icv order
typedef int ICV;
#define ICVBTNHILITE   0
#define ICVBTNFACE     1
#define ICVBTNSHADOW   2
#define ICVBRUSHMAX    3

#define ICVBTNTEXT     3
#define ICVWINDOW      4
#define ICVWINDOWTEXT  5
#define ICVGRAYTEXT    6
#define ICVWINDOWFRAME 7
#define ICVMAX         8

  
// DrawRec3d flags
#define DR3LEFT  0x0001
#define DR3TOP   0x0002
#define DR3RIGHT 0x0004
#define DR3BOT   0x0008
#define DR3HACKBOTRIGHT 0x1000  // code size is more important than aesthetics
#define DR3ALL    0x000f

typedef WORD DR3;     

// isomorphic to windows RECT
typedef struct
    {
    int xLeft;
    int yTop;
    int xRight;
    int yBot;
    } RC;
 
BOOL   FAR PASCAL Draw3dEnabled(void);
HBRUSH FAR PASCAL Draw3dCtlColor(UINT wm, WPARAM wParam, LPARAM lParam);
BOOL   FAR PASCAL Draw3dColorChange(void);

BOOL   FAR PASCAL Draw3dRegister(void);
BOOL   FAR PASCAL Draw3dUnregister(void); 

VOID Draw3dRec(HDC hdc, RC FAR *lprc, ICV icvUpperLeft, ICV icvLowerRight, DR3 rdr3);
VOID Draw3dInsetRect(HDC hdc, RC FAR *prc, DR3 dr3);

#ifdef __cplusplus
}
#endif
