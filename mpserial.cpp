/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: mpserial.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.3  2010/04/11 13:47:40  richard_wood
/*  FIXED - Export custom headers does not work, they are lost
/*  FIXED - Foreign month names cause crash
/*  FIXED - Bozo bin not being exported / imported
/*  FIXED - Watch & ignore threads not being imported / exported
/*  FIXED - Save article (append to existing file) missing delimiters between existing text in file and new article
/*  ADDED - Add ability to customise signature font size + colour
/*  First build for 2.9.15 candidate.
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
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

#include "stdafx.h"
//#include "mpserial.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

//UCHAR serialMask[10] = {0x01, 0xde, 0xad, 0xfe, 0xed, 0xfa, 0xce, 0x45, 0x84, 0x35};

/////////////////////////////////////////////////////////////////////////////
// GetChar - Take a value between 0-31 and return a character that maps to
//           it.
/////////////////////////////////////////////////////////////////////////////
//TCHAR MPSerialNumber::GetChar (int val)
//{
//	if (val >= 0 && val <= 7)
//		return (TCHAR(val + '2'));
//
//	if (val >= 8 && val <= 18)
//		return (TCHAR (val - 8 + 'A'));
//
//	if (val == 19)
//		return TCHAR('M');
//
//	if (val == 20)
//		return TCHAR('N');
//
//	if (val >= 21 && val <= 31)
//		return (TCHAR (val - 21 + 'P'));
//
//	return TCHAR('0');
//}

/////////////////////////////////////////////////////////////////////////////
// GetVal - Take a character and map it to a value.
/////////////////////////////////////////////////////////////////////////////
//int MPSerialNumber::GetVal(TCHAR c)
//{
//	// if anyone sees 'I' as 1 or L, make the transform for them
//	if (c == '1' || c == 'L')
//		return 16;
//
//	if (c >= '2' && c <= '9')
//		return (c - '2');
//	if (c >= 'A' && c <= 'K')
//		return (8 + c - 'A');
//	if (c == 'M')
//		return 19;
//	if (c == 'N')
//		return 20;
//	if (c >= 'P' && c <= 'Z')
//		return (21 + c - 'P');
//	else return -1;
//}

/////////////////////////////////////////////////////////////////////////////
// GetBits - Get a range of bits and turn them into a long.
/////////////////////////////////////////////////////////////////////////////
//long MPSerialNumber::GetBits(UCHAR *pBitSequence, int start, int num)
//{
//	long  result = 0;
//	int i, j;
//
//	j = 0;
//
//	for (i = start; i < start + num; i++)
//	{
//		if (IsBitOn (pBitSequence, i))
//			SetBit ((UCHAR *) &result, j);
//		j++;
//	}
//	return result;
//}

/////////////////////////////////////////////////////////////////////////////
// SetBits - Stuff a bit sequence with bits from a long.
/////////////////////////////////////////////////////////////////////////////
//void MPSerialNumber::SetBits (UCHAR *pBitSequence, int startBit, int numBits, long value)
//{
//	for (int i = 0; i < numBits; i++)
//	{
//		ClearBit (pBitSequence, i + startBit);
//		if (IsBitOn ((UCHAR *) &value, i))
//			SetBit (pBitSequence, i + startBit);
//	}
//}
//
//MPSerialNumber::MPSerialNumber (LPCTSTR pSerialNumber, LPCTSTR user, LPCTSTR org)
//{
//	Set (pSerialNumber, user, org); 
//}
//
//MPSerialNumber::MPSerialNumber ()
//{
//	m_version          = -1;
//	m_kLicenseType    = kInvalidLicenseType;
//	m_kProductCode    = kInvalidProductCode;
//	m_sequenceNumber  = 0;
//	m_userCount       = kInvalidUserCount;
//}
//
//BOOL MPSerialNumber::CheckMemberValidity()
//{
//	if ((m_userName.GetLength () == 0) &&
//		(m_userCount == 1))
//		return FALSE;
//	if ((m_userCount > 1) &&
//		(m_organization.GetLength() == 0))
//		return FALSE;
//	if (m_version < kMinVersionNumber ||
//		m_version > kMaxVersionNumber)
//		return FALSE;
//	if (m_kLicenseType != kEducational &&
//		m_kLicenseType != kCommercial &&
//		m_kLicenseType != kSite)
//		return FALSE;
//	if (m_kProductCode != kNews32)
//		return FALSE;
//	if (m_sequenceNumber < 1000 ||
//		m_sequenceNumber > kMaxSequenceNumber)
//		return FALSE;
//	if (m_userCount < 1 ||
//		m_userCount > kMaxUserCount)
//		return FALSE;
//
//	return TRUE;
//}

//int MPSerialNumber::CalcCheckSum ()
//{
//	int i;
//	int sum = 0;
//
//	if (m_userCount == 1)
//	{
//		for (i = 0; i < m_userName.GetLength(); i++)
//			sum += (int (_totupper(m_userName[i])) * (i + 1));
//	}
//
//	if (m_userCount > 1)
//	{
//		for (i = 0; i < m_organization.GetLength(); i++)
//			sum += (int (_totupper(m_organization[i])) * (i + 1));
//	}
//	sum += m_sequenceNumber;
//	sum += m_version;
//	sum += m_userCount;
//
//	return sum;
//}

//BOOL MPSerialNumber::IsValid ()
//{
//	if (!CheckMemberValidity())
//		return FALSE;
//
//	if ((CalcCheckSum() & 0xFFF) != m_checkSum)
//		return FALSE;
//
//	return TRUE;
//}

//BOOL MPSerialNumber::Set (LPCTSTR   pCurrent, LPCTSTR   user, LPCTSTR   org)
//{
//	// parse the serial number and set the members accordingly
//	LPCTSTR  pCurr = pCurrent;
//	UCHAR    bits[10];
//	UCHAR    decryptedBits[10];
//	int      numDigits = 0;
//	long     temp;
//	int      currStart = 0;
//	int      i;
//
//	ZeroMemory (bits, sizeof (bits));
//
//	while (*pCurr)
//	{
//		temp = GetVal (_totupper (*pCurr));
//		if (temp != -1)
//		{
//			if (numDigits++ > 15)
//				return FALSE;
//			SetBits (bits, currStart, 5, temp);
//			currStart += 5;
//		}
//		pCurr++;
//	}
//
//	if (numDigits != 15)
//		return FALSE;
//
//	// decrypt
//	EncryptDecrypt (bits, decryptedBits);
//
//	// de-scramble
//	for (i = 0; i < sizeof (decryptedBits); i++)
//		decryptedBits[i] ^= serialMask[i%sizeof(serialMask)];
//
//	m_sequenceNumber = GetBits (decryptedBits, 0, 32) / kPrimeFactor;
//	m_kProductCode = (MPSerialNumber::EProductCode) GetBits (decryptedBits, 32, 3);
//	m_userCount = GetBits (decryptedBits, 35, 16);
//	m_kLicenseType = (MPSerialNumber::ELicenseType) GetBits (decryptedBits, 51, 2);
//	m_version = GetBits (decryptedBits, 53, 10);
//	m_checkSum = GetBits (decryptedBits, 63, 12);
//	m_userName = user;
//	m_organization = org;
//	return IsValid ();
//}

//void MPSerialNumber::EncryptDecrypt (UCHAR *pInputBits, UCHAR  *pResultBits)
//{
//	LONG  result;
//	int   i;
//
//	result = GetBits (pInputBits, 63, 12);
//	SetBits (pResultBits, 63, 12, result);
//	switch (result % 3)
//	{
//	case 0:
//		// all bits 0-61 get inverted
//		for (i = 0; i < 63; i++)
//		{
//			if (IsBitOn(pInputBits, i))
//				ClearBit (pResultBits, i);
//			else
//				SetBit (pResultBits, i);
//		}
//		break;
//	case 1:
//		// every fourth bit in 0-61 gets inverted
//		for (i = 0; i < 63; i++)
//		{
//			if ((i % 4) == 0)
//			{
//				if (IsBitOn(pInputBits, i))
//					ClearBit (pResultBits, i);
//				else
//					SetBit (pResultBits, i);
//			}
//			else
//			{
//				if (IsBitOn (pInputBits, i))
//					SetBit (pResultBits, i);
//				else
//					ClearBit (pResultBits, i);
//			}
//		}
//		break;
//	case 2:
//		// every third bit in 0-61 gets inverted
//		for (i = 0; i < 63; i++)
//		{
//			if ((i % 3) == 0)
//			{
//				if (IsBitOn(pInputBits, i))
//					ClearBit (pResultBits, i);
//				else
//					SetBit (pResultBits, i);
//			}
//			else
//			{
//				if (IsBitOn (pInputBits, i))
//					SetBit (pResultBits, i);
//				else
//					ClearBit (pResultBits, i);
//			}
//		}
//		break;
//	}
//}

//void MPSerialNumber::SetUserName (LPCTSTR userName)
//{
//	m_userName = userName;   
//}
//
//CString MPSerialNumber::GetUserName ()
//{
//	return m_userName;
//}

//void MPSerialNumber::SetOrganization (LPCTSTR   organization)
//{
//	m_organization = organization;   
//}

//CString MPSerialNumber::GetOrganization ()
//{
//	return m_organization;
//}

//MPSerialNumber::EProductCode MPSerialNumber::GetProductCode ()
//{
//	return m_kProductCode;
//}

//void MPSerialNumber::SetProductCode (EProductCode kProductCode)
//{
//	m_kProductCode = kProductCode;   
//}

//void MPSerialNumber::SetUserCount (int count)
//{
//	m_userCount = count;   
//}

//int MPSerialNumber::GetUserCount ()
//{
//	return m_userCount;
//}

//void MPSerialNumber::SetLicenseType (ELicenseType kLicenseType)
//{
//	m_kLicenseType = kLicenseType;   
//}

//MPSerialNumber::ELicenseType MPSerialNumber::GetLicenseType()
//{
//	return m_kLicenseType;
//}

//void MPSerialNumber::SetSequenceNumber (DWORD sequenceNum)
//{
//	m_sequenceNumber = sequenceNum;   
//}

//DWORD MPSerialNumber::GetSequenceNumber()
//{
//	return m_sequenceNumber;
//}
