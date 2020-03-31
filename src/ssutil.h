/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: ssutil.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:54  richard_wood
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

// structured storage utility functions...

#pragma once

#include "mplib.h"

BOOL StreamExist (LPSTORAGE pIOpenStorage, LPCTSTR subStorage);
void FormWideString (LPCTSTR  normal, LPWSTR wide, int width);

//---------------------------------------------------------------------//

class TSSException : public TException
{
public:

TSSException (const CString &description, const ESeverity &kSeverity) :
   TException (description, kSeverity)
   {
   }
};
//---------------------------------------------------------------------//

class TSSNoStorage : public TSSException

{
public:

TSSNoStorage (const CString &description, const ESeverity &kSeverity) :
   TSSException (description, kSeverity)
   {
   }
};

//---------------------------------------------------------------------//

class TSSErrorDeleting : public TSSException

{
public:

TSSErrorDeleting (const CString &description, const ESeverity &kSeverity) :
   TSSException (description, kSeverity)
   {
   }
};

//---------------------------------------------------------------------//

class TSSErrorRenaming : public TSSException
{
public:

TSSErrorRenaming (const CString &description, const ESeverity &kSeverity) :
   TSSException (description, kSeverity)
   {
   }
};

//---------------------------------------------------------------------//

class TSSErrorOpening : public TSSException
{
public:

TSSErrorOpening (const CString &description, const ESeverity &kSeverity) :
   TSSException (description, kSeverity)
   {
   }
};

//---------------------------------------------------------------------//

class TSSErrorCreating : public TSSException

{
public:

TSSErrorCreating (const CString &description, const ESeverity &kSeverity) :
   TSSException (description, kSeverity)
   {
   }
};

//---------------------------------------------------------------------//

void  SaveSSObject(LPSTORAGE  pIStorage,
                   CObject *   pOb,
                   LPCTSTR     name,
                   CObject *   pObArray[] = NULL);

void  LoadSSObject ( LPSTORAGE      pIStorage,
                     CObject *      pOb,
                     LPCTSTR        name,
                     CObject *      pObArray[] = NULL);

void  DeleteStream ( LPSTORAGE      pIStorage,
                     LPCTSTR        name );
