/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: turldde.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:20  richard_wood
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

#include "pobject.h"     // for PObject
#include "mplib.h"   //  for TException
#include "regutil.h"
#include "rgbase.h"

// URL exception class -
class TUrlException : public TException
{
public:
	TUrlException (const CString &description,
		const ESeverity &kSeverity)
	{
		PushError (description, kSeverity);
	}
};

class TUrlDde : public TRegistryBase
{
public:
	// these correspond to the indexes in a combo box/radio group
	// don't reorder these

	enum TransactionType {kExecute = 0, kPoke = 1, kRequest = 2};

	TUrlDde (LPCTSTR subkey);
	~TUrlDde () {}

	CString & GetApplication () {return m_application;}
	void  SetApplication (LPCTSTR app) {m_application = app;}

	LONG  UseDDE () {return m_fUseDDE;}
	void  SetUseDDE (LONG fUseDDE ) {m_fUseDDE = fUseDDE;}

	CString & GetTopic () {return m_topic;}
	void  SetTopic (LPCTSTR topic) {m_topic = topic;}

	CString & GetService () {return m_service;}
	void  SetService (LPCTSTR service) {m_service = service;}

	CString & GetItem () {return m_item;}
	void  SetItem (LPCTSTR item) {m_item = item;}

	CString & GetData () {return m_data;}
	void  SetData (LPCTSTR data) {m_data = data;}

	LONG  GetTransactionType () {return m_transactionType;}
	void  SetTransactionType (LONG type) {m_transactionType = type;}

	TUrlDde & operator=(const TUrlDde &);

	int Load();
	int Save();

protected:
	void read();
	void write();
	void default_values();

private:
	CString     m_regSubKey;

	LONG        m_fUseDDE;
	LONG        m_transactionType;

	CString     m_application;
	CString     m_service;
	CString     m_topic;
	CString     m_item;
	CString     m_data;
};
