/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mpserial.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:31  richard_wood
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

//class MPSerialNumber
//{
//public:
//   enum EProductCode {kInvalidProductCode = -1,
//                      kNews32 = 0};
//
//   enum ELicenseType {kInvalidLicenseType = -1,
//                      kEducational = 0,
//                      kCommercial = 1,
//                      kSite = 2};
//
//   enum {kInvalidUserCount       = -1,
//         kMinVersionNumber       = 1,
//         kMaxVersionNumber       = 999,
//         kPrimeFactor            = 4289,
//         kMaxUserCount           = 65535,
//         kMaxSequenceNumber      = 1000000};
//   
//public:
//   MPSerialNumber (LPCTSTR pSerialNumber, LPCTSTR user, LPCTSTR org);
//   MPSerialNumber ();
//   BOOL IsValid ();
//   BOOL Set (LPCTSTR pCurrent, LPCTSTR user, LPCTSTR org);
//   BOOL Generate (LPTSTR  pResult);
//
//   void SetUserName (LPCTSTR userName);
//   CString GetUserName ();
//
//   void SetOrganization (LPCTSTR   organization);
//   CString GetOrganization ();
//
//   EProductCode GetProductCode ();
//   void SetProductCode (EProductCode kProductCode);
//
//   void SetUserCount (int count);
//   int GetUserCount ();
//
//   void SetLicenseType (ELicenseType kLicenseType);
//   ELicenseType GetLicenseType();
//
//   void SetVersionNumber (int version) {
//      m_version = version;
//      }
//
//   int  GetMajorVersion () {return m_version/100;}
//   int  GetMinorVersion () {return (m_version%100) / 10;}
//   int  GetTrivialVersion () {return m_version %10;}
//
//   void  SetSequenceNumber (DWORD sequenceNum);
//   DWORD GetSequenceNumber();
//
//private:
//   void SetBit (UCHAR * bitmap, int whichBit)
//      {(bitmap[whichBit/8] |= ( 1 << (whichBit % 8)));}
//   void ClearBit (UCHAR *bitmap, int whichBit)
//      {(bitmap[whichBit/8] &= ~( 1 << (whichBit % 8)));}
//   BOOL IsBitOn (UCHAR *bitmap, int whichBit)
//      {return (bitmap[whichBit/8] & ( 1 << (whichBit % 8)));}
//
//   long GetBits (UCHAR *bitSequence, int startBit, int numbits);
//   void SetBits (UCHAR *bitSequence, int startBit, int numbits, long value);
//
//   TCHAR GetChar (int      val);
//   int   GetVal  (TCHAR    c);
//
//   void EncryptDecrypt (UCHAR *pInputBits, UCHAR *pResultBits);
//   BOOL CheckMemberValidity();
//   int  CalcCheckSum ();
//
//private:
//   CString        m_userName;
//   CString        m_organization;
//   int            m_version;
//   ELicenseType   m_kLicenseType;
//   EProductCode   m_kProductCode;
//   DWORD          m_sequenceNumber;
//   int            m_userCount;
//   int            m_checkSum;
//};
