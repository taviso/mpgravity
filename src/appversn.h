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





#if !defined(APPVERSN_H)
#define  APPVERSN_H

/////////////////////////////////////////////////////////////////////////////
// Gravity versions

#ifdef OEM
#define NO_EXPIRE
#define GRAVITY_VERSION " (OEM)"
#endif

#ifdef LITE
#define NO_EXPIRE
#define GRAVITY_VERSION " (Lite)"
#endif

#ifdef KISS
#define NO_EXPIRE
#define GRAVITY_VERSION " (Kiss)"
#endif

#ifdef KISS_TRIAL
#define NO_REGISTER
#define GRAVITY_VERSION " (Kiss Trial)"
#endif

#ifdef PROXICOM
#define XRETRIEVEICONS
#define PROTECT_ROOT_MSGS
#define FORCE_ERROR_ENTERGROUP
#define PRECHECK_POSTING_STATUS
#define NO_CMD_NEWGROUPS
#define GRAVITY_VERSION " (Proxicom)"
#endif

// regular version
#ifndef GRAVITY_VERSION
#define GRAVITY_VERSION ""
#endif


#endif      // APPVERSN_H
