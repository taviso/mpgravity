/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: attdoc.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.10  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.9  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.8  2008/10/03 08:20:37  richard_wood
/*  Fixed crash when emailing someone.
/*
/*  Revision 1.7  2008/09/19 14:51:12  richard_wood
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

// attdoc.cpp : implementation file
//
//
//       derived from CDocument, but has a TStringList for attachments

#include "stdafx.h"
#include "mplib.h"         // TPath

#include "attdoc.h"

#include "coding.h"
#include "tglobopt.h"
#include "msgid.h"
#include "custmsg.h"
#include "outbox.h"
#include "globals.h"
#include "fileutil.h"
#include "server.h"
#include "codepg.h"
#include "utilstr.h"
#include "tsearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static char grcBASE64[] = "Base64";
static char grcUU[] = "X-UUENCODE";
static char grcXX[] = "X-XXENCODE";

extern TGlobalOptions *gpGlobalOptions;

// separate routine for MIME handling of attachments
const int typOriginal     =  100;
const int typMulti_Mixed  =  101;
const int typMesg_Partial =  102;

const int iVery_Big_Number = 600000000;

/////////////////////////////////////////////////////////////////////////////
// TAttachmentDoc

IMPLEMENT_DYNCREATE(TAttachmentDoc, CDocument)

TAttachmentDoc::TAttachmentDoc()
{
	// m_cpServer is set to the current active server
	m_fSent = FALSE;
}

BOOL TAttachmentDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// take the encoding type from Global Options
	m_AttArray.m_eEncoding = gpGlobalOptions->GetEncodingType();

	return TRUE;
}

TAttachmentDoc::~TAttachmentDoc()
{
}

BEGIN_MESSAGE_MAP(TAttachmentDoc, CDocument)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TAttachmentDoc diagnostics

#ifdef _DEBUG
void TAttachmentDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void TAttachmentDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TAttachmentDoc serialization

void TAttachmentDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

//////////////////////////////////////////////////////////////////////////
//
void TAttachmentDoc::ChunkifyAttachments (int  AttCount)
{
	bool fMailing = m_fEmail;

	if (0 == AttCount)
		return;

	if (Att_UsingMime())
	{
		MIME_ChunkifyAttachments (fMailing);
		return;
	}

	CString origSubj = Message_GetSubject(fMailing);

	if ( gpGlobalOptions->IsAttachmentsInSeparateArticle() )
	{
		// multiple atts will be numbered 0/3, 1/3, 2/3, 3/3, 1/2, 2/2
		// we need the total size of first attachment
		int total = 0;
		const TAttachmentInfo& firstInfo  = m_AttArray[0];
		CalcEncodedParts ( firstInfo.m_att, total );

		CString origSubj = Message_GetSubject(fMailing);
		CString subj0;
		CalcSubjectByTemplate ( origSubj, firstInfo.m_att, 0, total, subj0);
		Message_SetSubject ( fMailing, subj0 );
		// attachments go into separate articles,
		//  thus TEXT article is marked as #0
		Finalize822Message (fMailing);
		Message_SetSubject ( fMailing, origSubj );

		for (int k = 0; k < AttCount; ++k)
		{
			const TAttachmentInfo& rInfo = m_AttArray[k];
			ChunkifyOneAttachment ( rInfo.m_att, origSubj, 1,
				FALSE,  // fBase64
				fMailing );
		}
	}
	else
	{
		// 1/3, 2/3, 3/3, 1/2, 2/2
		const CString& rbod = Message_GetBody (fMailing);
		BOOL fFirst = TRUE;
		for (int k = 0; k < AttCount; ++k)
		{
			const TAttachmentInfo& rInfo = m_AttArray[k];
			ChunkifyAllAttachments ( rbod, rInfo.m_att, origSubj,
				fFirst, fMailing );
			fFirst = FALSE;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
void TAttachmentDoc::ChunkifyOneAttachment (
	const CString& rAttName,
	const CString& origSubj,
	int   startNum,
	BOOL  fBase64,
	bool  fMailing
	)
{
	int  maxBytes = CalcSplitLen();

	unsigned long outBytes = 0;   // number of bytes in output
	unsigned long outLines = 0;   // number of lines in output

	CFile fl;
	TPath wholePath = rAttName;
	TPath fileName;
	wholePath.GetFile (fileName);

	CFileException e;

	if (FALSE == fl.Open (rAttName, CFile::modeRead | CFile::shareExclusive, &e))
	{
		report_fopen_error (rAttName, &e);
		return ;
	}

	DWORD dwFileLen = fl.GetLength();
	int partTotal;
	CalcEncodedParts (int(dwFileLen), partTotal);

	UINT  byteStart = 0;
	UINT  bytesRead = 0;
	UINT  totalRead = 0;
	int   partNum = startNum;
	while (totalRead < dwFileLen)
	{
		CMemFile memFile(4096);                // growth increment

		outBytes = 0;
		outLines = 0;

		bytesRead = m_codeMgr.Encode (
			(fBase64) ?
			TCodedBinBlock::CODE_BASE64 :
		TCodedBinBlock::CODE_UU,     // encoding type
			fl,                    // source file
			fileName,              // file name without Path
			byteStart,             // start encoding from here
			maxBytes,              // maximum size of chunk
			memFile,               // output file
			outBytes,
			outLines );
		byteStart += bytesRead;
		totalRead += bytesRead;

		CString subjn;

		// save this
		CString origBody = Message_GetBody (fMailing);

		CalcSubjectByTemplate ( origSubj, rAttName, partNum, partTotal, subjn);

		// overwrite two items
		Message_SetSubject ( fMailing, subjn );
		Message_SetBody ( fMailing, memFile );

		// dump to storage
		Finalize822Message (fMailing);

		// restore
		Message_SetSubject ( fMailing, origSubj );
		Message_SetBody ( fMailing, origBody );

		++ partNum;
	}
}

//////////////////////////////////////////////////////////////////////////
//
void TAttachmentDoc::ChunkifyAllAttachments (
	const CString& rbod,
	const CString& rAttName,
	const CString& origSubj,
	BOOL           fFirst,
	bool           fMailing
	)
{
	BOOL fFirstSave = fFirst;
	int  splitLen = CalcSplitLen();
	int  maxBytes;

	unsigned long outBytes = 0;   // number of bytes in output
	unsigned long outLines = 0;   // number of lines in output

	CFile fl;
	TPath wholePath = rAttName;
	TPath fileName;
	wholePath.GetFile (fileName);

	CFileException e;
	if (FALSE == fl.Open (rAttName, CFile::modeRead | CFile::shareExclusive, &e))
	{
		report_fopen_error(rAttName, &e);
		return;
	}

	DWORD dwFileLen = fl.GetLength();

	int partTotal;
	if (fFirst)
		CalcPlainParts (rbod.GetLength(), int(dwFileLen) , partTotal);
	else
		CalcEncodedParts (int(dwFileLen), partTotal);

	if (fFirst)
	{
		maxBytes = splitLen - rbod.GetLength();
	}
	else
	{
		maxBytes = splitLen;
	}

	UINT  byteStart = 0;
	UINT  bytesRead = 0;
	UINT  totalRead = 0;
	int   partNum = 1;
	while (totalRead < dwFileLen)
	{
		CMemFile memFile(4096);                // growth increment
		if (fFirstSave)
		{
			fFirstSave = FALSE;
			memFile.Write ( rbod, rbod.GetLength() );   // text-body
		}
		outBytes = 0;
		outLines = 0;

		bytesRead = m_codeMgr.Encode (
			TCodedBinBlock::CODE_UU,     // encoding type
			fl,                    // source file
			fileName,              // file name without Path
			byteStart,             // start encoding from here
			maxBytes,              // maximum size of chunk
			memFile,               // output file
			outBytes,
			outLines );

		maxBytes = splitLen;       // other messages don't have the Text-Body

		byteStart += bytesRead;
		totalRead += bytesRead;

		CString subjn;
		CalcSubjectByTemplate ( origSubj, rAttName, partNum, partTotal, subjn);
		Message_SetSubject ( fMailing, subjn );
		Message_SetBody ( fMailing, memFile );

		// dump to storage
		Finalize822Message (fMailing);

		Message_SetSubject (fMailing, origSubj );

		++ partNum;
	}
}

BOOL TAttachmentDoc::CalcEncodedParts (const CString & rAttName, int& parts)
{
	CFileStatus Status;
	if (!CFile::GetStatus ( rAttName, Status ))
		return FALSE;
	CalcEncodedParts ( Status.m_size, parts );
	return TRUE;
}

// Accounts for Expansion due to encoding
// ??? is this a good estimate? what about user signature?

int TAttachmentDoc::CalcEncodedParts (int origSize, int& partTotal)
{
	UINT maxBytes = CalcSplitLen();
	if (iVery_Big_Number == maxBytes)
	{
		partTotal = 1;
		return 0;
	}

	// assumes a 3 to 4 encoding
	origSize = 4 * origSize / 3;
	int lines = origSize / 60;     // LengthByte / CR /LF per line

	origSize += (origSize / 20);     // (N/60)*2   estimates \r\n for each line

	partTotal = (origSize / maxBytes) + 1;
	return 0;
}

int TAttachmentDoc::CalcPlainParts (int origSize, int attSize, int& partTotal)
{
	UINT maxBytes = CalcSplitLen();
	if ((UINT)iVery_Big_Number == maxBytes)
	{
		partTotal = 1;
		return 0;
	}

	// simple division
	attSize = 4 * attSize / 3;
	attSize += (attSize/20);

	partTotal =   ((origSize + attSize) / maxBytes) + 1;
	return 0;
}

void TAttachmentDoc::CalcSubjectByTemplate(
	const CString& origSub,
	const CString& attSpec,
	int   part,
	int   total,
	CString& subj)
{
	TCHAR* pBuf = new TCHAR[2048];
	TCHAR* pB_start;

	ZeroMemory (pBuf, 2048);

	TCHAR* pT;
	CString templ = gpGlobalOptions->GetSubjectTemplate();
	CString szTotal;
	szTotal.Format("%d", total);
	CString szPart;
	CString fmt;
	// width is number of digits
	fmt.Format ("%%%d.%dd", szTotal.GetLength(), szTotal.GetLength() );

	// %s == subject
	// %f == filename
	// %p == part number
	// %t == total parts
	// %% == percent-sign
	pT = templ.GetBuffer(templ.GetLength());
	pB_start = pBuf;

	while (*pT)
	{
		switch (*pT)
		{
		case '%':
			if (*(pT + 1) == 0)
				*pBuf++ = *pT++;
			else
			{
				switch (*(pT + 1))
				{
				case '%':
					*pBuf++ = *pT;
					pT += 2;
					break;

				case 'f':
					{
						TPath attFileName;
						TPath wholePath = attSpec;
						wholePath.GetFile (attFileName);

						lstrcpy (pBuf, attFileName);
						pBuf += attFileName.GetLength();
						pT += 2;
					}
					break;

				case 'p':
					szPart.Format (fmt, part);
					lstrcpy (pBuf, szPart);
					pBuf += szPart.GetLength();
					pT += 2;
					break;

				case 't':
					lstrcpy (pBuf, szTotal);
					pBuf += szTotal.GetLength();
					pT += 2;
					break;

				case 's':
					lstrcpy (pBuf, origSub);
					pBuf += origSub.GetLength();
					pT += 2;
					break;
				default:
					*pBuf++ = *pT++;
					break;
				}
			}
			break;

		default:
			*pBuf++ = *pT++;
			break;
		}
	}

	*pBuf = '\0';

	templ.ReleaseBuffer();
	pT = subj.GetBuffer(lstrlen(pB_start));
	lstrcpy (pT, pB_start);
	subj.ReleaseBuffer();

	delete [] pB_start;
}

int TAttachmentDoc::make_temp_filename(CString&  result)
{
	TCHAR rcFname[MAX_PATH];
	DWORD dwRet = GetTempPath(1, rcFname);      // see how much we need
	TCHAR* pPath = new TCHAR[dwRet + 2];
	if (pPath)
	{
		GetTempPath ( dwRet, pPath );

		UINT u = GetTempFileName ( pPath, "MPI", 0, rcFname );

		result = rcFname;

		delete [] pPath;
	}

	return 0;
}

int TAttachmentDoc::AppendBody_and_Attachment(
	const CString& rBody,
	const CString& attFile,
	CString&  result)
{
	make_temp_filename ( result );

	CFile fl;
	CFile flAtt;

	CFileException e;
	if (!flAtt.Open (attFile, CFile::modeRead, &e))
	{
		report_fopen_error(attFile, &e);
		return 0;
	}

	// extract the filename token
	TPath wholePath = attFile;
	TPath justFilename;
	wholePath.GetFile( justFilename );

	// shoot in the text-body
	fl.Open ( result, CFile::modeCreate | CFile::modeReadWrite );
	fl.Write ( rBody, rBody.GetLength() );
	fl.Write ("\r\n", 2);

	unsigned long outBytes = 0;   // number of bytes in output
	unsigned long outLines = 0;   // number of lines in output

	UINT byteStart = 0;
	UINT bytesRead = 0;

	// encode the 1st attachment
	bytesRead = m_codeMgr.Encode (
		TCodedBinBlock::CODE_UU,     // encoding type
		flAtt,                 // source file
		justFilename,          // file name without Path
		byteStart,             // start encoding from here
		0,                     // maximum size of chunk 0 = infinite
		fl,                    // output file
		outBytes,
		outLines );

	return int (2 + rBody.GetLength() + outBytes);
}

// returns reference INT
int TAttachmentDoc::Att_Add (
							 LPCTSTR att,
							 int     size,
							 LPCTSTR conType,
							 LPCTSTR conDesc,
							 TMime::ECode eCode)
{
	TAttachmentInfo one_attachment(att, size, conType, conDesc, eCode);
	m_AttArray.Add ( one_attachment );
	return 0;
}

void TAttachmentDoc::Att_RemoveAll (void)
{
#if defined(_DEBUG)
	int n = m_AttArray.GetSize();
#endif
	m_AttArray.RemoveAll ();
}

void TAttachmentDoc::Att_Remove (int n)
{
	m_AttArray.RemoveAt ( n );
}

void TAttachmentDoc::Att_GetName (int n, CString& att)
{
	const TAttachmentInfo& rInfo = m_AttArray.GetAt(n);
	att = rInfo.m_att;
}

BOOL TAttachmentDoc::Att_UsingMime()
{
	if (TGlobalDef::kMIME == m_AttArray.m_eEncoding)
		return TRUE;
	else
		return FALSE;
}

void TAttachmentDoc::Att_SetUsingMime(BOOL fOn)
{
	m_AttArray.m_eEncoding = fOn ? TGlobalDef::kMIME : TGlobalDef::kUUENCODE;
}

// go past the point of no return.  Prevent further switching
// between MIME and Normal.
void TAttachmentDoc::Att_MimeLock(BOOL fLock)
{
	m_AttArray.m_fEncodingInited = fLock;
}

BOOL TAttachmentDoc::Att_IsMimeLocked (void)
{
	return m_AttArray.m_fEncodingInited;
}

// ==========================================================================
// $2003$
void TAttachmentDoc::MIME_ChunkifyAttachments (bool fMailing)
{
	int curType;

	// ignore the gpGlobalOptions->IsAttachmentsInSeparateArticle()
	// and just build the structured mime-message

	int parts = MIME_CalcNumParts ( fMailing, curType );

	switch ( curType )
	{
	case typOriginal:
		MIME_Att_Original (parts, fMailing);
		break;

	case typMulti_Mixed:
		MIME_Att_Multipart (parts, fMailing);
		break;

	case typMesg_Partial:
		MIME_Att_Partial (parts, fMailing);
		break;
	}
}

int TAttachmentDoc::MIME_CalcNumParts (bool fMailing, int& curTyp)
{
	const CString& rbod = Message_GetBody(fMailing);

	// Generate a boundary line
	MakeABoundaryString (m_boundary);

	int attCount = m_AttArray.GetSize();
	if  (attCount > 0)
		Formalize_SubDescription();

	int splitLen = CalcSplitLen();

	// if body is null, Content-Type is as specified.
	// else, starts out as Multipart/Mixed
	if (rbod.IsEmpty() && (1 == attCount))
	{
		const TAttachmentInfo& rInfo = m_AttArray.GetAt(0);
		curTyp = typOriginal;
		AfxFormatString2(work_Type, IDS_MIME_HDR_CONTYPE_NAME,
			rInfo.m_contentType,
			LPCTSTR(rInfo.m_att) + rInfo.m_nameOffset);
		AfxFormatString1(work_Desc, IDS_MIME_HDR_CONDESC, rInfo.m_contentDesc);
		switch (rInfo.m_eCode)
		{
		case TMime::CODE_BASE64:
			AfxFormatString1(work_Encode, IDS_MIME_HDR_CONENCODE, grcBASE64);
			break;
		case TMime::CODE_UU:
			AfxFormatString1(work_Encode, IDS_MIME_HDR_CONENCODE, grcUU);
			break;
		case TMime::CODE_XX:
			AfxFormatString1(work_Encode, IDS_MIME_HDR_CONENCODE, grcXX);
			break;
		default:
			ASSERT(0);
			break;
		}
	}
	else
	{
		curTyp = typMulti_Mixed;   // mixed - body vs. att
		curTyp = typMulti_Mixed;   // mixed - 2 or more atts

		AfxFormatString2(work_Type, IDS_MIME_HDR_CONTYPE_BOUND,
			"multipart/mixed",
			LPCTSTR(m_boundary));
		// no global desc,  no global encoding
	}

	if (typOriginal == curTyp)
	{
		// no boundary line needed
		const TAttachmentInfo& rInfo = m_AttArray[0];
		int encoded_size = rInfo.m_size;
		encoded_size = encoded_size * 4 / 3;
		if (TMime::CODE_BASE64 == rInfo.m_eCode)
			encoded_size += encoded_size *2 / 60 ;       // CR/LF at every 60 char line
		else if (TMime::CODE_UU == rInfo.m_eCode ||
			TMime::CODE_XX == rInfo.m_eCode)
			encoded_size += encoded_size *3 / 60;    // CRLF + checksum

		if (encoded_size <= splitLen)
		{
			return 1;    // one part
		}
		else
		{
			curTyp = typMesg_Partial;
			// we must split into message partial
			// add in size of working-header
			if (!work_Type.IsEmpty())
				encoded_size += work_Type.GetLength() + 2;
			if (!work_Desc.IsEmpty())
				encoded_size += work_Desc.GetLength() + 2;
			if (!work_Encode.IsEmpty())
				encoded_size += work_Encode.GetLength() + 2;

			// blank line
			encoded_size += 2;
			int parts = (encoded_size / splitLen) + 1;
			return parts;
		}
	}
	else
	{
		// multipart/mixed
		int encoded_size = 0;

		if (!rbod.IsEmpty())
		{
			encoded_size += 2 + (m_boundary.GetLength()) + 2;  // --boundary\R\N

			CString strContentType;
			strContentType.Format("text/plain; charset=\"iso-8859-%d\"\r\n", gpGlobalOptions->GetSendCharset());

			CString contentDesc;
			AfxFormatString1(contentDesc, IDS_MIME_HDR_CONTYPE, LPCTSTR(strContentType));

			encoded_size += contentDesc.GetLength();

			if (gpGlobalOptions->GetSend8Bit())
				AfxFormatString1(contentDesc, IDS_MIME_HDR_CONENCODE, "8bit\r\n");
			else
				AfxFormatString1(contentDesc, IDS_MIME_HDR_CONENCODE, "7bit\r\n");
			encoded_size += contentDesc.GetLength() + 2;   // self + blankline

			encoded_size += rbod.GetLength();
		}

		for (int i = 0; i < m_AttArray.GetSize(); ++i)
		{
			const TAttachmentInfo& rInfo = m_AttArray.GetAt(i);
			if (i == 0)
				encoded_size += 2 + m_boundary.GetLength() + 2;   // --boundary\R\N
			else
				encoded_size += 4 + m_boundary.GetLength() + 2;   // \R\N--boundary\R\N

			encoded_size += rInfo.m_subDesc.GetLength() + 2;  // +CRLF

			int block = rInfo.m_size * 4 / 3;
			if (TMime::CODE_BASE64 == rInfo.m_eCode)
				block += ((block/60) + 1) *2 ;       // CR/LF at every 60 char line
			else if (TMime::CODE_UU == rInfo.m_eCode || TMime::CODE_XX == rInfo.m_eCode)
				block += ((block/60) + 1) *3;    // CRLF + checksum

			encoded_size += block;
		}

		// end boundary
		encoded_size += 4 + m_boundary.GetLength() + 4;

		if (encoded_size < splitLen)
			return 1;
		else
		{
			// Overflow! we must split into message/partial
			curTyp = typMesg_Partial;

			// add in size of working-header Multipart/mixed + CRLF
			if (!work_Type.IsEmpty())
				encoded_size += work_Type.GetLength() + 2;

			// each section has its own Description and Content-Encoding

			int parts = (encoded_size / splitLen) + 1;
			return parts;
		}
	}
}

void TAttachmentDoc::MakeABoundaryString(CString& bound)
{
	unsigned int uTime = time(NULL);
	srand(uTime);
	char rcLetters[11];
	int  n = sizeof(rcLetters);

	rcLetters[n-1] = 0;
	for (int i = 0; i < n-1; i++)
		rcLetters[i] = char((rand() % 26) + 97);    // lowercase a-z

	bound.Format("(%s=========__%d)", rcLetters, uTime);
}

// Generate the sub heading
void TAttachmentDoc::Formalize_SubDescription(void)
{
	for (int i = 0; i < m_AttArray.GetSize(); ++i)
	{
		CString contentType;
		CString contentDesc;
		CString contentEncode;

		TAttachmentInfo& rInfo = m_AttArray.ElementAt(i);

		AfxFormatString2(contentType, IDS_MIME_HDR_CONTYPE_NAME,
			rInfo.m_contentType,
			LPCTSTR(rInfo.m_att) + rInfo.m_nameOffset);

		switch (rInfo.m_eCode)
		{
		case TMime::CODE_BASE64:
			AfxFormatString1(contentEncode, IDS_MIME_HDR_CONENCODE, grcBASE64);
			break;
		case TMime::CODE_UU:
			AfxFormatString1(contentEncode, IDS_MIME_HDR_CONENCODE, grcUU);
			break;
		case TMime::CODE_XX:
			AfxFormatString1(contentEncode, IDS_MIME_HDR_CONENCODE, grcXX);
			break;
		default:
			ASSERT(0);
			break;
		}

		rInfo.m_subDesc = contentType + "\r\n";
		rInfo.m_subDesc += contentEncode + "\r\n";

		if (!rInfo.m_contentDesc.IsEmpty())
		{
			AfxFormatString1(contentDesc, IDS_MIME_HDR_CONDESC, rInfo.m_contentDesc);
			rInfo.m_subDesc += contentDesc + "\r\n";
		}
	}
}

// ==========================================================================
// there is no body, 1 attachment, fits in one article
void TAttachmentDoc::MIME_Att_Original(int parts, bool fMailing)
{
	// set the mime hdr into the Header.
	CString type;
	CString desc;
	CString enc;

	int i = work_Type.Find(' ');
	if (i != -1)
		type = work_Type.Mid(i+1);
	i = work_Desc.Find(' ');
	if (i != -1)
		desc = work_Desc.Mid(i+1);
	i = work_Encode.Find(' ');
	if (i != -1)
		enc = work_Encode.Mid(i+1);
	Message_SetMimeLines (fMailing, "1.0", type, enc, desc);

	const TAttachmentInfo& rInfo = m_AttArray[0];

	CString origSubj = Message_GetSubject (fMailing);

	CFile fl;
	TPath wholePath = rInfo.m_att;
	TPath fileName;
	wholePath.GetFile (fileName);

	CFileException e;
	if (!fl.Open (rInfo.m_att, CFile::modeRead | CFile::shareExclusive, &e))
	{
		report_fopen_error(rInfo.m_att, &e);
		return;
	}

	DWORD dwFileLen = fl.GetLength();

	unsigned long  outBytes = 0;
	unsigned long  outLines = 0;
	UINT  byteStart = 0;
	UINT  bytesRead = 0;
	UINT  totalRead = 0;
	int   partNum = 1;
	UINT  maxBytes = iVery_Big_Number;
	while (totalRead < dwFileLen)
	{
		CMemFile memFile(8192);                // growth increment

		outBytes = 0;
		outLines = 0;

		bytesRead = m_codeMgr.Encode (
			TCodedBinBlock::ECodeType(rInfo.m_eCode), // encoding type
			fl,                    // source file
			fileName,              // file name without Path
			byteStart,             // start encoding from here
			maxBytes,              // maximum size of chunk
			memFile,               // output file
			outBytes,
			outLines );
		byteStart += bytesRead;
		totalRead += bytesRead;

		CString subjn;
		CalcSubjectByTemplate ( origSubj, rInfo.m_att, 1, 1, subjn);
		Message_SetSubject ( fMailing, subjn );
		Message_SetBody ( fMailing, memFile );

		// dump to storage
		Finalize822Message (fMailing);

		Message_SetSubject (fMailing, origSubj);
	}
}

// ==========================================================================
// Multi-part, but it all fits into one article
void TAttachmentDoc::MIME_Att_Multipart(int parts, bool fMailing)
{
	unsigned long outBytes = 0;   // number of bytes in output
	unsigned long outLines = 0;   // number of lines in output

	UINT maxBytes = iVery_Big_Number;

	CMemFile memFile(8192);

	// set the mime hdr into the Header.
	CString type;
	CString desc;
	BOOL    fFirstBound = TRUE;

	int i = work_Type.Find(' ');
	if (i != -1)
		type = work_Type.Mid(i+1);

	i = work_Desc.Find(' ');
	if (i != -1)
		desc = work_Desc.Mid(i+1);
	Message_SetMimeLines (fMailing, "1.0", type, "", desc);

	CString origSubj =  Message_GetSubject(fMailing);

	// If there is a body,
	const CString& rbod = Message_GetBody (fMailing);
	if (!rbod.IsEmpty())
	{
		AddNormalBoundary(memFile, TRUE);
		fFirstBound = FALSE;

		insert_qp_entity (rbod, &memFile);
	}

	for (int j = 0 ; j < m_AttArray.GetSize(); ++j)
	{
		CFile fl;
		CFileException e;

		const TAttachmentInfo& rInfo = m_AttArray[j];
		AddNormalBoundary(memFile, fFirstBound);
		fFirstBound = FALSE;

		memFile.Write(rInfo.m_subDesc, rInfo.m_subDesc.GetLength());
		memFile.Write("\r\n", 2);

		if (!fl.Open (rInfo.m_att, CFile::modeRead | CFile::shareExclusive,&e))
		{
			report_fopen_error(rInfo.m_att, &e);
			continue;
		}

		DWORD dwFileLen = fl.GetLength();

		TPath wholePath = rInfo.m_att;
		TPath fileName;
		wholePath.GetFile (fileName);

		UINT  byteStart = 0;
		UINT  bytesRead = 0;
		UINT  totalRead = 0;
		while (totalRead < dwFileLen)
		{
			outBytes = 0;
			outLines = 0;

			bytesRead = m_codeMgr.Encode (
				TCodedBinBlock::ECodeType(rInfo.m_eCode),  // encoding type
				fl,                    // source file
				fileName,              // file name without Path
				byteStart,             // start encoding from here
				maxBytes,              // maximum size of chunk
				memFile,               // output file
				outBytes,
				outLines );
			byteStart += bytesRead;
			totalRead += bytesRead;
		}
	} // for loop thru attachments

	AddEndBoundary (memFile);

	CString subjn;
	CString nam;
	int attCount = m_AttArray.GetSize();
	if (1 == attCount)
	{
		const TAttachmentInfo& rInfo = m_AttArray[0];
		nam = LPCTSTR(rInfo.m_att) + rInfo.m_nameOffset;
	}
	else
		nam.Format("%d files", attCount);
	CalcSubjectByTemplate ( origSubj, nam, 1, 1, subjn);
	Message_SetSubject ( fMailing, subjn );
	Message_SetBody ( fMailing, memFile );

	// dump to storage
	Finalize822Message (fMailing);

	Message_SetSubject ( fMailing, origSubj );
}

///////////////////////////////////////////////////////////////////
// A multipart/mixed message that is broken up into Message/Partial
//
//  4-17-96 see part tagged with LOOK!
void TAttachmentDoc::MIME_Att_Partial(int parts, bool fMailing)
{
	unsigned long outBytes = 0;   // number of bytes in output
	unsigned long outLines = 0;   // number of lines in output
	CString composite_id;
	GenerateMessageID (m_cpServer->GetEmailAddress(),
		m_cpServer->GetNewsServerAddress(),
		0,
		composite_id);

	// start filling the Body
	CMemFile memFile(8192);

	// set the mime hdr into the Header.
	CString type;
	int     part = 1;
	Add_Partial_Headers ( fMailing, composite_id, part, parts );

	UINT splitLen = CalcSplitLen();

	UINT maxBytes = splitLen;

	CString origSubj = Message_GetSubject(fMailing);
	BOOL fFirstBound = TRUE;

	// header section declaring Multipart/mixed
	memFile.Write (work_Type, work_Type.GetLength()); memFile.Write ("\r\n", 2);
	// (4-17-96) I'm still not sure about this !LOOK!
	memFile.Write ("\r\n", 2);

	// If there is a body
	const CString& rbod = Message_GetBody(fMailing);
	if (!rbod.IsEmpty())
	{
		AddNormalBoundary(memFile, TRUE);
		fFirstBound = FALSE;

		insert_qp_entity (rbod, &memFile);
	}

	for (int j = 0 ; j < m_AttArray.GetSize(); ++j)
	{
		CFile fl;
		CFileException e;

		const TAttachmentInfo& rInfo = m_AttArray[j];

		AddNormalBoundary(memFile, fFirstBound); fFirstBound = FALSE;

		memFile.Write(rInfo.m_subDesc, rInfo.m_subDesc.GetLength());
		memFile.Write("\r\n", 2);

		maxBytes = splitLen - memFile.GetLength();

		// open source file
		if (!fl.Open (rInfo.m_att, CFile::modeRead | CFile::shareExclusive, &e))
		{
			report_fopen_error(rInfo.m_att, &e);
			continue;
		}
		DWORD dwFileLen = fl.GetLength();

		TPath wholePath = rInfo.m_att;
		TPath fileName;
		wholePath.GetFile (fileName);

		UINT  byteStart = 0;
		UINT  bytesRead = 0;
		UINT  totalRead = 0;
		while (totalRead < dwFileLen)
		{
			outBytes = 0;
			outLines = 0;

			bytesRead = m_codeMgr.Encode (
				TCodedBinBlock::ECodeType(rInfo.m_eCode), // encoding type
				fl,                    // source file
				fileName,              // file name without Path
				byteStart,             // start encoding from here
				maxBytes,              // maximum size of chunk
				memFile,               // output file
				outBytes,
				outLines );
			byteStart += bytesRead;
			totalRead += bytesRead;

			// we stopped. find out why
			if (totalRead < dwFileLen)
			{
				// this article is maxxed out. Save it out
				Message_SetBody (fMailing, memFile);

				CString subjn;
				CString n_files; n_files.Format ("%d files", m_AttArray.GetSize());
				CalcSubjectByTemplate (origSubj, n_files, part, parts, subjn);
				Message_SetSubject (fMailing, subjn);
				Finalize822Message (fMailing);
				Message_SetSubject (fMailing, origSubj );

				// new ball game
				memFile.SetLength(0);

				++part;
				Add_Partial_Headers (fMailing, composite_id, part, parts);

				maxBytes = splitLen;
			}
			else
			{
				// end of 1 source file. continue adding to this Article
			}
		} // while not EOF
	} // for loop thru attachments

	AddEndBoundary (memFile);

	CString subjn;
	CString nam;
	nam.Format("%d files",  m_AttArray.GetSize());
	CalcSubjectByTemplate ( origSubj, nam, part, parts, subjn);
	Message_SetSubject ( fMailing, subjn );
	Message_SetBody ( fMailing, memFile );

	// dump to storage
	Finalize822Message (fMailing);

	Message_SetSubject ( fMailing, origSubj );
}

void TAttachmentDoc::AddNormalBoundary(CFile& file,  BOOL fFirstBound)
{
	if (fFirstBound)
		file.Write("--", 2);
	else
		file.Write("\r\n--", 4);
	file.Write((LPCTSTR)m_boundary, m_boundary.GetLength());
	file.Write("\r\n", 2);
}

void TAttachmentDoc::AddEndBoundary(CFile& file)
{
	file.Write("\r\n--", 4);
	file.Write((LPCTSTR)m_boundary, m_boundary.GetLength());
	file.Write("--\r\n", 4);
}

void TAttachmentDoc::Add_Partial_Headers(bool fMailing,
										 const CString& composite_id,
										 int part, int total)

{
	CString fmt;
	fmt.LoadString(IDS_MIME_HDR_PARTIAL);

	CString content;
	content.Format(fmt, LPCTSTR(composite_id), part, total);

	// set the mime hdr into the Header.
	CString type;

	int i = content.Find(' ');
	if (i != -1)
		type = content.Mid(i+1);
	Message_SetMimeLines (fMailing, "1.0", type, "", "");
}

///////////////////////////////////////////////////////////////////////////
// Poll our views and save the data
//
//  4-22-96  amc Return a FALSE if any DDV_ routine fails.
BOOL TAttachmentDoc::UpdateData (void)
{
	BOOL fUpdateSucceeded = TRUE;

	POSITION pos = GetFirstViewPosition();
	while (pos)
	{
		CView* pView = GetNextView(pos);
		// get data from HdrView and BodyView
		if (FALSE == pView->UpdateData ( TRUE ))      // save and validate
		{
			// most likely a Data Validation Failure
			fUpdateSucceeded = FALSE;
			break;
		}
	}
	return fUpdateSucceeded;
}

void TAttachmentDoc::CloseFrameWnd (void)
{
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView (pos);
	CFrameWnd* pFrame = pView->GetParentFrame();
	if (pFrame)
	{
		// pFrame->DestroyWindow() is bad
		//   the parent window would be gone, and
		//    Toolbar::ButtonUp would be hosed.
		pFrame->PostMessage ( WM_CLOSE );
	}
}

// ==========================================================================
void TAttachmentDoc::PostArticle(
								 TArticleHeader* pArtHdr,
								 TArticleText* pArtText)
{
	m_fEmail = false;   // email msgs ignore Article split size

	if (FALSE == gpGlobalOptions->IsAttachmentsInSeparateArticle() &&
		FALSE == m_AttArray.IsUsingMIME() )
	{
		// attachment follows immediately.  Add a CRLF
		pArtText->Pad_CRLF();
	}

	// see how many attachments we have.
	int iAttCount = m_AttArray.GetSize();

	m_pArtHeader  = pArtHdr;
	m_pArtText    = pArtText;

	m_pEmailHeader = NULL;
	m_pEmailText   = NULL;

	if (0 == iAttCount)
	{
		// analyze the text
		use_qp_on_text ();

		FinalizeMessageForOutbox ();
	}
	else
	{
		ChunkifyAttachments ( iAttCount );
	}
}

/////////////////////////////////////////////////////////////////////////////
//  called from PostArticle
void TAttachmentDoc::FinalizeMessageForOutbox()
{
	// m_article is filled.
	CompleteArticle();

	TOutbox *pOutbox = m_cpServer->GetOutbox ();

	if (0 == pOutbox)
		throw(new TException (IDS_ERR_NO_OUTBOX, kFatal));

	pOutbox->Open ();

	// outbox will own the header. So Copy it
	TArticleHeader * pAHcopy = new TArticleHeader;
	*pAHcopy = (*m_pArtHeader);

	pOutbox->SaveArticle ( pAHcopy, m_pArtText );

	// make the status persistent
	m_cpServer->SaveOutboxState ();

	pOutbox->Close ();
}

void TAttachmentDoc::CompleteArticle()
{
	int iPostId = m_cpServer->NextPostID ();

	m_pArtText->SetNumber ( iPostId );

	// if you view the "outbox", the threading algorithm will hash via
	// the msg-id.  It must be unique, so we might as well make it a real
	// message-id
	CString msg_id;
	GenerateMessageID ( m_cpServer->GetEmailAddress(),
		m_cpServer->GetNewsServerAddress(),
		iPostId,
		msg_id );

	m_pArtHeader->SetMessageID ( msg_id );

	// save to "outbox" storage
	m_pArtHeader->SetArticleNumber ( iPostId );

	// can't leave the time uninitialized. 9-16-95.
	m_pArtHeader->StampCurrentTime ();
	m_pArtHeader->SetLines ( m_pArtText->GetBody() );
}

// ------------------------------------------------------------------------
// SendEmail --
void TAttachmentDoc::SendEmail (TEmailHeader* pEmHdr, TEmailText* pEmText)
{
	m_fEmail = true;           // email ignores Article split size

	m_pArtHeader = 0;
	m_pArtText = 0;

	m_pEmailHeader = pEmHdr;
	m_pEmailText   = pEmText;

	if (FALSE == gpGlobalOptions->IsAttachmentsInSeparateArticle() &&
		FALSE == m_AttArray.IsUsingMIME() )
	{
		// attachment follows immediately.  Add a CRLF
		pEmText->Pad_CRLF();

		// make sure the preamble is smaller than split size
		int  maxBytes = CalcSplitLen();
		if (pEmText->GetBody().GetLength() >= maxBytes)
		{
			ASSERT(0);
			MessageBeep(0);
			return;
		}
	}

	int iAttCount = m_AttArray.GetSize();
	if (0 == iAttCount)
	{
		// analyze the text
		use_qp_on_text ();

		FinalizeEmailForOutbox();
	}
	else
		ChunkifyAttachments ( iAttCount );
}

void TAttachmentDoc::FinalizeEmailForOutbox()
{
	ASSERT(m_pEmailText);
	ASSERT(m_pEmailHeader);

	int id = m_cpServer->NextMailID ();

	m_pEmailText->SetNumber ( id );

	// if you view the "outbox", the threading algorithm will hash via
	// the msg-id.  It must be unique, so we might as well make it a real
	// message-id
	CString msg_id;
	GenerateMessageID ( m_cpServer->GetEmailAddress(),
		m_cpServer->GetSmtpServer(),
		id,
		msg_id );

	m_pEmailHeader->SetMessageID ( msg_id );

	// save to "outbox" storage
	m_pEmailHeader->SetNumber ( id );

	// 9-15-95 can't leave time blank.
	m_pEmailHeader->StampCurrentTime ();

	const CString& body = m_pEmailText->GetBody();
	m_pEmailHeader->SetLines ( body );

	TOutbox *pOutbox = m_cpServer->GetOutbox();

	if (0 == pOutbox)
		throw(new TException(IDS_ERR_NO_OUTBOX, kFatal));

	pOutbox->Open ();

	// put the message into the outbox
	TEmailHeader * pEHcopy = new TEmailHeader;
	*pEHcopy = *m_pEmailHeader;
	pOutbox->SaveEmail ( pEHcopy, m_pEmailText );

	// save the outbox
	m_cpServer->SaveOutboxState ();

	pOutbox->Close ();
}

//  copy the attachment info to the article header
void TAttachmentDoc::SaveAttachmentInfo(TPersist822Header * p822Hdr)
{
	p822Hdr->SetAttachments (&m_AttArray);
}

// ------------------------------------------------------------------------
// Finalize822Message
void TAttachmentDoc::Finalize822Message (bool fMailing)
{
	if (!fMailing)
		FinalizeMessageForOutbox ();
	else
		FinalizeEmailForOutbox ();
}

LONG gl_Outbox_Locked;

int TAttachmentDoc::CalcSplitLen()
{
	if (m_fEmail)
		return iVery_Big_Number;

	int  iBytes = gpGlobalOptions->GetArticleSplitLen() * 1024;
	if (0 == iBytes)
		iBytes = iVery_Big_Number;       // zero is the same as Infinite
	return iBytes;
}

int TAttachmentDoc::report_fopen_error(const CString& rAttName, CFileException* pe)
{
	const int bufSz = 256;
	LPTSTR pBuf = new TCHAR[bufSz];
	POSITION pos  = GetFirstViewPosition();

	if (pe->GetErrorMessage(pBuf, bufSz-1))
		NewsMessageBox(GetNextView(pos), pBuf, MB_OK | MB_ICONEXCLAMATION);
	else
	{
		CString error; error.LoadString(IDS_ERR_FILEOPEN);
		CString msg = rAttName + "- " + error;
		NewsMessageBox(GetNextView(pos), msg, MB_OK | MB_ICONEXCLAMATION);
	}

	delete [] pBuf;
	return 0;
}

// ------------------------------------------------------------------------
void TAttachmentDoc::insert_qp_entity(const CString & rbod, CFile * pFile)
{
	int	iSendCharset = gpGlobalOptions->GetSendCharset();
	BOOL  fSend8Bit    = gpGlobalOptions->GetSend8Bit();

	// go from MultiByte charset to Unicode to ISO charset
	CString strBytes;

	ECP_OutputType eOutput = kCP_7BIT;

	CP_Outbound ( false,
		rbod,
		iSendCharset,
		fSend8Bit,
		strBytes,  // output really contains bytes
		eOutput );

	CString strContentType;
	strContentType.Format("text/plain; charset=\"iso-8859-%d\"", iSendCharset);

	CString sub;
	AfxFormatString1(sub, IDS_MIME_HDR_CONTYPE, LPCTSTR(strContentType));
	pFile->Write (sub, sub.GetLength());
	pFile->Write ("\r\n", 2);

	switch (eOutput)
	{
	case kCP_7BIT:
		AfxFormatString1(sub, IDS_MIME_HDR_CONENCODE, "7bit\r\n");
		break;
	case kCP_8BIT:
		AfxFormatString1(sub, IDS_MIME_HDR_CONENCODE, "8bit\r\n");
		break;
	case kCP_QP:
		AfxFormatString1(sub, IDS_MIME_HDR_CONENCODE, "quoted-printable\r\n");
		break;
	}

	pFile->Write (sub, sub.GetLength());
	pFile->Write ("\r\n", 2); // the blank line
	pFile->Write (strBytes, strBytes.GetLength());
}

class TEncodingFrom : public TEncodingYesNo
{
public:
	bool operator()(TCHAR c, CString & qpData)
	{
		if (needs_it(c))
		{
			if (' ' == c)
				qpData += '_';
			else
				add(c, qpData);
			return true;
		}
		return false;
	}

	bool needs_it (TCHAR c)
	{
		if ((0 <= c) && (c <= 31))  // 822 CTLs
		{
			return true;
		}

		switch (c)
		{
		case ' ':
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '\\':
		case '"':
		case '.':
		case '[':
		case ']':
		case '?':
			//         case '=':
			return true;
		}
		return false;
	}
};

class TEncodingSubj : public TEncodingYesNo
{
public:
	bool operator()(TCHAR c, CString & qpData)
	{
		if (' ' == c)
		{
			qpData += '_';
			return true;  // done
		}

		return false;
	}
};

// handles over-encoding, extremely picky
class TEncodingSubjEx : public TEncodingYesNo
{
public:
	bool operator()(TCHAR c, CString & qpData)
	{
		if (' ' == c)
		{
			qpData += '_';
			return true;  // done
		}
		if ('=' == c)
		{
			qpData += "=3D";
			return true;
		}
		if ('?' == c)
		{
			qpData += "=3F";
			return true;
		}

		return false;
	}
};

extern void sfnParse_from (const CString & fromIn, CString & phrase, CString & address);

// ------------------------------------------------------------------------
static int lastline_len (const CString & str)
{
	LPCTSTR pTrav = str;
	int n = 0;
	while (*pTrav)
	{
		n++;
		if ('\n' == *pTrav)
			n = 0;
		pTrav++;
	}
	return n;
}

// ------------------------------------------------------------------------
void TAttachmentDoc::use_qp_on_text()
{
	CString phrase, addr;
	CString phrase2;
	const CString& rbod = Message_GetBody (m_fEmail );

	int iSendCharset = gpGlobalOptions->GetSendCharset();
	BOOL fSend8Bit = gpGlobalOptions->GetSend8Bit();

	// go from MultiByte charset to Unicode to ISO charset
	CString strBytes;

	TEncodingFrom    sEncFrom;
	TEncodingSubj    sEncSubj;
	TEncodingSubjEx  sEncSubjExtreme;

	ECP_OutputType eOutput = kCP_7BIT;
	bool fUsedQP = false;
	bool fUsed8Bit = false;
	bool bContains8BitOrQPChars = false;

	CP_Outbound ( false,
		rbod,
		iSendCharset,
		fSend8Bit,
		strBytes,  // output really contains bytes
		eOutput );	// receives type

	// if fSend8Bit and message does contains 8bit chars, eOutput = kCP_8BIT
	// if fSend8Bit and message does NOT contains 8bit chars, eOutput = kCP_7BIT
	// if NOT fSend8Bit and message does contains 8bit chars, eOutput = kCP_QP and message body does NOT contain 8bit

	if (eOutput != kCP_7BIT)
		bContains8BitOrQPChars = true;

	// install bytes
	Message_SetBody (m_fEmail, strBytes);

	// at this point m_ArtText is still an object, but it contains QP "ready to go"
	CString ver;
	CString typ;
	CString encode;
	CString desc;

	if (kCP_QP == eOutput)
	{
		if (m_fEmail)
		{
			m_pEmailHeader->GetMimeLines (&ver, &typ, &encode, &desc);
			m_pEmailHeader->SetMimeLines (ver, typ, "quoted-printable", desc);
		}
		else
		{
			m_pArtHeader->GetMimeLines (&ver, &typ, &encode, &desc);
			m_pArtHeader->SetMimeLines (ver, typ, "quoted-printable", desc);
		}
	}
	else if (kCP_8BIT == eOutput)
	{
		if (m_fEmail)
		{
			m_pEmailHeader->GetMimeLines (&ver, &typ, &encode, &desc);
			m_pEmailHeader->SetMimeLines (ver, typ, "8bit", desc);
		}
		else
		{
			m_pArtHeader->GetMimeLines (&ver, &typ, &encode, &desc);
			m_pArtHeader->SetMimeLines (ver, typ, "8bit", desc);
		}
	}

	// phase 2  handle the Subject and FROM
	if (gpGlobalOptions->GetSend8BitHdrs())
	{
		// translate from CP to ISO , send eight-bit ISO
		CString strOutSubj;
		ECP_OutputType eOut = kCP_7BIT;

		// (1/3) do the subject
		CString subj = Message_GetSubject (m_fEmail);
		CP_Outbound (true, subj, iSendCharset, TRUE /*fSend8Bit*/, strOutSubj, eOut);
		if (eOut != kCP_7BIT)
			bContains8BitOrQPChars = true;
		Message_SetSubject (m_fEmail, strOutSubj);

		// (2/3) do the From
		if (true)
		{
			CString f, f2;
			if (m_fEmail)
				f = m_pEmailHeader->GetFrom();
			else
				f = m_pArtHeader->GetFrom();
			eOut = kCP_7BIT;
			CP_Outbound (true, f, iSendCharset, TRUE/*fSend8Bit*/, f2, eOut);
			if (eOut != kCP_7BIT)
				bContains8BitOrQPChars = true;
			if (m_fEmail)
				m_pEmailHeader->SetFrom(f2);
			else
				m_pArtHeader->SetFrom(f2);
		}

		// (3/3) do the ReplyTo
		if (true)
		{
			CString r, r2;
			if (m_fEmail)
				r = m_pEmailHeader->GetReplyTo();
			else
				r = m_pArtHeader->GetReplyTo();
			eOut = kCP_7BIT;
			CP_Outbound (true, r, iSendCharset, TRUE/*fSend8Bit*/, r2, eOut);
			if (eOut != kCP_7BIT)
				bContains8BitOrQPChars = true;
			if (m_fEmail)
				m_pEmailHeader->SetReplyTo(r2);
			else
				m_pArtHeader->SetReplyTo(r2);
		}
	}
	else
	{
		TSearch srch;
		srch.SetPattern ("=\\?.+\\?[qQbB]\\?.+\\?=", FALSE, TSearch::RE);

		// translate from CP to ISO, encode with QP and use funky RFC-2047 format
		CString subj = Message_GetSubject (m_fEmail);
		int iFoundLen=0;
		DWORD dwPos=0;

		bool hasHiBit = using_hibit(subj);
		BOOL patternMatch = srch.Search (subj, iFoundLen, dwPos, subj.GetLength ());
		if (hasHiBit || patternMatch)
		{
			CString strFinalSubj;
			CString strTail;
			int nOffset = 0;

			if (subj.Left(4).CompareNoCase("re: ") == 0)
				nOffset = 4;

			// we must not encode the "Re: " part.

			if (patternMatch)
				make_qp_hdr (iSendCharset, nOffset + ((LPCTSTR) subj), strTail, sEncSubjExtreme);
			else
				make_qp_hdr (iSendCharset, nOffset + ((LPCTSTR) subj), strTail, sEncSubj);

			strFinalSubj = subj.Left(nOffset) + strTail;
			Message_SetSubject (m_fEmail, strFinalSubj);
		}
		else
		{
			// leave alone
		}

		// ==== do the FROM ====
		CString strFrom;
		if (m_fEmail)
			strFrom = m_pEmailHeader->GetFrom();
		else
			strFrom = m_pArtHeader->GetOrigFrom();  // this w/o parseing

		if (using_hibit( strFrom ))
		{
			// just break it up, don't do any translation
			sfnParse_from (strFrom, phrase, addr);

			make_qp_hdr (iSendCharset, phrase, phrase2, sEncFrom);

			// calc funnylen - length of last line
			int n = lastline_len( phrase2 );

			CString tmp;
			// can we safely append?
			if ( ((6 /*"From: "*/) + n + (3) + addr.GetLength()) > 76)
				tmp.Format ("%s\r\n <%s>", LPCTSTR(phrase2), addr);
			else
				tmp.Format ("%s <%s>", LPCTSTR(phrase2), addr);

			Message_SetFrom (m_fEmail, tmp);
		}
		else
		{
			// leave alone
		}

		if (true)
		{
			CString replyTo, rplyPhrase, rplyAddr, rplyPhrase2;
			if (m_fEmail)
				replyTo = m_pEmailHeader->GetReplyTo();
			else
				replyTo = m_pArtHeader->GetReplyTo();
			if (FALSE==replyTo.IsEmpty())
			{
				if (!using_hibit(replyTo))
				{
					// leave alone
				}
				else
				{
					// just break it up, don't do any translation
					sfnParse_from (replyTo, rplyPhrase, rplyAddr);

					make_qp_hdr (iSendCharset, rplyPhrase, rplyPhrase2, sEncFrom);
					int n = lastline_len( rplyPhrase2 );

					CString tmp;
					if (lstrlen("Reply-To: ") + n + (3) + rplyAddr.GetLength() > 76)
						tmp.Format("%s\r\n <%s>", LPCTSTR(rplyPhrase2), LPCTSTR(rplyAddr));
					else
						tmp.Format("%s <%s>", LPCTSTR(rplyPhrase2), LPCTSTR(rplyAddr));

					if (m_fEmail)
						m_pEmailHeader->SetReplyTo(tmp);
					else
						m_pArtHeader->SetReplyTo(tmp);
				}
			}
		}
		// Everything is now 7BIT chars
	}

	if (!bContains8BitOrQPChars)
	{
		if (m_fEmail)
		{
			m_pEmailHeader->GetMimeLines (&ver, &typ, &encode, &desc);
			m_pEmailHeader->SetMimeLines (ver, "text/plain; charset=\"us-ascii\"", encode, desc);
		}
		else
		{
			m_pArtHeader->GetMimeLines (&ver, &typ, &encode, &desc);
			m_pArtHeader->SetMimeLines (ver, "text/plain; charset=\"us-ascii\"", encode, desc);
		}
	}
}

// ------------------------------------------------------------------------
void TAttachmentDoc::make_qp_hdr (
								  int iSendCharset,
								  const CString & subj,
								  CString & strFinalSubj,
								  TEncodingYesNo & sEnc)
{
	CString strOutSubj;
	CString qpData;

	ECP_OutputType eOutput = kCP_7BIT;

	// convert to iso-bytes
	CP_Outbound (true, subj, iSendCharset, false, strOutSubj, eOutput);

	// final pass
	CString tmp;
	for (LPCTSTR psz = strOutSubj; *psz; psz++)
	{
		if (!sEnc(*psz, tmp))
			tmp += *psz;
	}
	strOutSubj = tmp;

	GravCharset * pCharset = gsCharMaster.findById( iSendCharset );
	if (NULL == pCharset)
	{
		strFinalSubj = subj;
		return;
	}

	// we may have added =XX, so preserve equals signs
	// we may have added ??, so hex them

	// we have to use Quoted-printable on the header lines (a la RFC 2047)

	// we want something like  =?iso8859-15?Q?XXXXXX?=
	// requirements  76 chars or less

	// Subject: Re: =?ISO-8859-15?Q??=
	// 			 =?ISO-8859-15?Q??=

	int nMax = 76 - 13 - 7 - pCharset->GetName().GetLength();
	LPCSTR pszTrav = LPCTSTR(strOutSubj);
	while (*pszTrav)
	{
		TCHAR c = *pszTrav++;

		qpData += c;
		if ('=' == c)
		{
			qpData += *pszTrav++;  // must include entire octet in one encoded word
			qpData += *pszTrav++;
		}

		if (qpData.GetLength() >= nMax)
		{
			CString tmp; tmp.Format ("=?%s?Q?%s?=", LPCTSTR(pCharset->GetName()), (LPCTSTR) qpData);
			if (strFinalSubj.IsEmpty())
			{
				strFinalSubj += tmp;
				// 2nd line can contain more, since "Subject: " is done with
				// 76 - (foldspace) - len(=?ISO-8859-15?Q??=) - fudgefactor(2)

				nMax = 76 - 3 - 7 - pCharset->GetName().GetLength();
			}
			else
			{
				strFinalSubj += "\r\n ";      // line folding
				strFinalSubj += tmp;
			}
			qpData.Empty();
		}
	}

	if (FALSE==qpData.IsEmpty())
	{
		CString tmp; tmp.Format ("=?%s?Q?%s?=", LPCTSTR(pCharset->GetName()), (LPCTSTR) qpData);
		if (strFinalSubj.IsEmpty())
			strFinalSubj += tmp;
		else
		{
			strFinalSubj += "\r\n ";         // line folding
			strFinalSubj += tmp;
		}
	}
}

