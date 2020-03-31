/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: netcfg.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

/////////////////////////////////////////////////////////////////////////////
// TNetConfiguration -  This object manages an ini file that is in the same
//                      directory as the news32 executable.  It allows an
//                      administrator to set certain configuration settings
//                      for a group of users who run News32 from a network
//                      directory.  This object is meant to be a global and
//                      prevents more than one copy from existing.
/////////////////////////////////////////////////////////////////////////////

class TNetConfiguration
{
public:
   TNetConfiguration();

   BOOL     ReadConfigFile();
   BOOL     ConfigFileExists() {return m_configExists;}
   CString  GetLicenseKey() {return m_licenseKey;}
   CString  GetOrganization() {return m_organization;}
   CString  GetDefaultDBLocation() {return m_defaultDBLocation;}
   BOOL     PromptForDBLocation(){return m_promptForDBLocation;}
   CString  GetSmtpServer () {return m_smtpServer;}
   CString  GetNewsServer () {return m_newsServer;}
private:
   CString  m_licenseKey;
   CString  m_organization;
   CString  m_newsServer;
   CString  m_smtpServer;
   BOOL     m_promptForDBLocation;
   CString  m_defaultDBLocation;
   static   int m_numCopies;
   BOOL     m_configExists;
};
