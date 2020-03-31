/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: rgmgr.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:45  richard_wood
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

// objects that write to the Registry
#include "rglaymdi.h"
#include "rgfont.h"
#include "rgcomp.h"
#include "rgdir.h"
#include "rgswit.h"
#include "rgwarn.h"
#include "rgpurg.h"
#include "rgui.h"
#include "rglaywn.h"
#include "rgstor.h"
#include "rgsys.h"
#include "rgurl.h"               // TURLSettings
#include "rgbkgrnd.h"
#include "turldde.h"
#include "vfilter.h"

//  TGlobalOptions just keeps a pointer to this big guy
//    The indirection should save us some compile time.
class TRegistryManager {
public:
   TRegistryManager(BOOL fFillDefaults);
   ~TRegistryManager();

   TRegLayoutMdi* GetRegLayoutMdi()   { return &m_regLayoutMdi;}
   TRegFonts*      GetRegFonts()      { return &m_regFonts; }
   TRegCompose*    GetRegCompose()    { return &m_regCompose; }
   TRegWarn*       GetRegWarn()       { return &m_regWarnings; }
   TRegDirectory*  GetRegDirs()       { return &m_regDirectories; }
   TRegSystem*     GetRegSystem()     { return &m_regSystem; }
   TRegSwitch*     GetRegSwitch()     { return &m_regSwitches; }
   TRegPurge*      GetRegPurge()      { return &m_regPurge; }
   TRegUI*         GetRegUI()         { return &m_regUI; }
   TRegLayoutWin*  GetRegLayoutWin()  { return &m_regLayWin; }
   TRegStorage*    GetRegStorage()    { return &m_regStorage; }
   TURLSettings*   GetRegURLSettings(){ return &m_regUrlSettings; }

   // URL stuff

   TUrlDde  *GetWebHelperPointer() {return &m_webHelper;}
   TUrlDde  *GetFtpHelperPointer() {return &m_ftpHelper;}
   TUrlDde  *GetGopherHelperPointer() {return &m_gopherHelper;}
   TUrlDde  *GetTelnetHelperPointer() {return &m_telnetHelper;}

   TBackgroundColors *GetBackgroundColors() { return &m_backgroundColors; }

   // URL Hotlink Support
   TUrlDde           m_webHelper;
   TUrlDde           m_ftpHelper;
   TUrlDde           m_gopherHelper;
   TUrlDde           m_telnetHelper;
   TURLSettings      m_regUrlSettings;

   TBackgroundColors m_backgroundColors;

private:
   TRegLayoutMdi     m_regLayoutMdi;     // mdi layout stored in registry
   TRegFonts         m_regFonts;         // logfonts
   TRegCompose       m_regCompose;       // compose options
   TRegWarn          m_regWarnings;      // warnings
   TRegDirectory     m_regDirectories;
   TRegSwitch        m_regSwitches;      // booleans
   TRegPurge         m_regPurge;
   TRegUI            m_regUI;            // more UI settings, filters
   TRegLayoutWin     m_regLayWin;        // positions of Post, Main wnds
   TRegStorage       m_regStorage;       // global storage mode
   TRegSystem        m_regSystem;        // system scalars
};
