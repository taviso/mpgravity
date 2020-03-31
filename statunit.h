/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: statunit.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:56  richard_wood
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

typedef struct tagStatusUnitI
{
   LONG   m_articleInt;
   time_t m_timeRead; // This is a 32 bit integer on VS V6, and a 64 bit integer on VS .NET, 2005 & 2008
   WORD   m_status;
} StatusUnitI;

class TStatusUnit
{
public:
   enum EStatus {kOBSOLETE  = 0x0001,  // OBSOLETE! used to be TStatusUnit::kRead
                 kNew       = 0x0002,  // user hasn't read (bitfield)
                 kQDecode   = 0x0004,  // queued for decoding
                 kPermanent = 0x0008,  // impervious to purging..
                 kImportant = 0x0010,  // important
                 kDecodeErr = 0x0020,  // decoding error
                 kOut       = 0x0040,  // waiting in Outbox
                 kSending   = 0x0080,  // in the process of going out
                 kSent      = 0x0100,  // successfully transmitted
                 kSendErr   = 0x0200,  // transmission failure
                 kDecoded   = 0x0400,  // decoded - success
                 kWatch     = 0x0800,  // watch article
                 kIgnore    = 0x1000,  // article is being ignored
                 kDraft     = 0x2000   // draft saved in outbox
                 };

   enum EStatusFilter {
                 kFilterNew       = 0x0002,  // user hasn't read
                 kFilterQDecode   = 0x0004,  // queued for decoding
                 kFilterPermanent = 0x0008,  // impervious to purging..
                 kFilterImportant = 0x0010,  // important
                 kFilterDecodeErr = 0x0020,  // decoding error
                 kFFOut       = 0x0040,  // waiting in Outbox
                 kFFSending   = 0x0080,  // in the process of going out
                 kFFSent      = 0x0100,  // successfully transmitted
                 kFFSendErr   = 0x0200,  // transmission failure
                 kFilterDecoded   = 0x0400,  // decoded - success
                 kFilterWatch     = 0x0800,  // watch article
                 kFilterIgnore    = 0x1000,  // article is being ignored
                 kFilterTag   = 0x4000,  // filter on Tagged
                 kFilterLocal = 0x8000   // filter on Local
                 };

   enum ETriad { kYes, kNo, kMaybe };

public:
   TStatusUnit();
   TStatusUnit(int artInt);
   ~TStatusUnit() {}
   TStatusUnit(const TStatusUnit& src);
   TStatusUnit& operator=(const TStatusUnit& rhs);

public:
   LONG  m_articleInt;
   CTime m_timeRead;
   WORD  m_status;
};
