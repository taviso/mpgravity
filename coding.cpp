/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: coding.cpp,v $
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
/*  Revision 1.7  2009/01/28 14:53:36  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.6  2009/01/02 13:34:33  richard_wood
/*  Build 6 : BETA release
/*
/*    [-] Fixed bug in Follow up dialog - Quoted text should be coloured.
/*    [-] Fixed bug in New post/Follow up dialog - if more than 1 page of text
/*        and typing at or near top the text would jump around.
/*
/*  Revision 1.5  2008/09/24 19:53:55  richard_wood
/*  Fixed bug with articles which said they were quoted-printed but were not.
/*
/*  Revision 1.4  2008/09/19 14:51:15  richard_wood
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

// coding.cpp -- low-level encoding/decoding class

#include "stdafx.h"
#include "mplib.h"
#include "resource.h"
#include "coding.h"
#include "article.h"
//#include "subjscan.h"
#include "fileutil.h"

#include "strext.h"
#include "tdecthrd.h"            // gpsDecodeDialog

#include "autoprio.h"
#include "tglobopt.h"            // gpGlobalOptions
#include "crc32.h"

#include "zb.h"
#include "qprint.h"

// warning about CException has no copy-ctor
//#pragma warning( disable : 4671 4673 )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern HWND ghwndMainFrame;

/////////////////////////////////////////////////////////////////////
// defines

const int  CODINGTABLESIZE = 64;
const int  END_BLOCK = 5;

const int  ENCODE_LINE_LEN = 45;

const int  MAP_UNUSED = 127;
const int  FAIL       = -1;
const int  SUCCESS    = 0;

// sequence confidence levels
const int  LOW  = 1;
const int  HIGH = 2;

BOOL CallbackStream (DWORD dwCookie, BYTE* poutLine, UINT numDecoded);
BOOL CallbackBinBlock (DWORD dwCookie, BYTE* poutLine, UINT numDecoded);

/////////////////////////////////////////////////////////////////////
// tables - these don't change... others are allocated in TCodeMgr
/////////////////////////////////////////////////////////////////////////////

char xxTable[CODINGTABLESIZE + 1] =
"+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char uuTable[CODINGTABLESIZE + 1] =
"`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";
char base64Table[CODINGTABLESIZE + 1] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/////////////////////////////////////////////////////////////////////
//

// construct
//   add a bigger grow factor to the memfile

#define PAGELIMIT 2000
#define PAGESIZE  0x1000

TCodedBinBlock::TCodedBinBlock()
:  m_memSpot (TMemorySpot::GROW_DOUBLER, 131072)    // growth factor
{
	sequence      = -1;
	seqConfidence = 0;
	estNumLines   = 0;
	beginFlag = endFlag = FALSE;
	numLines = 0;
	hParentWnd = NULL;

	m_fYEncoded  = 0;
	m_fYEncPassCRC = 1;
	m_fYEncCorrectSize = 0;
	m_iYEncBeginLineSize = 0;
}

void TCodedBinBlock::ClearData (void)
{
	m_memSpot.Empty();
}

DWORD TCodedBinBlock::size()
{
	return m_memSpot.size();
}

TCodedBinBlock::~TCodedBinBlock()
{
}

BOOL TCodedBinBlock::AddData (BYTE* data, int dataLen)
{
	// let sub object handle it
	m_memSpot.Add ( (LPCTSTR) data, dataLen );

	return TRUE;
}

// construct
TCodedThread::TCodedThread(TCodedSequencer *pSeq)
{
	numBlocksWritten  = 0;           // number of blocks written to disk, so far
	mode              = 0;           // 2nd field of begin header, don't use in DOS
	numBlocks         = 0;           // number of coded blocks used in list
	expectedNumBlocks = 0;           // 0 if unknown, or a # if found in headers
	totalBytes        = 0;           // total # of bytes for entire file
	fFileCreated      = FALSE;
	pSequencer      = pSeq;

	m_fYEncoded        = 0;
	m_fYEncPassCRC     = 1;
	m_fYEncCorrectSize = 1;          // assume the best
}

TCodedThread::~TCodedThread()
{
	POSITION pos = m_BlockList.GetHeadPosition();

	while (pos)
	{
		TCodedBinBlock* pBlock = m_BlockList.GetNext(pos);
		delete pBlock;
	}
}

// construct
TCodedSequencer::TCodedSequencer()
{
}

// destruct
TCodedSequencer::~TCodedSequencer()
{
	POSITION pos = m_threadList.GetHeadPosition();
	while (pos)
	{
		delete m_threadList.GetNext ( pos );
	}
}

TCodeMgr::TCodeMgr()
{
	// Allocate the maps, etc...

	xxMap.SetSize ( 256 );
	uuMap.SetSize ( 256 );
	base64Map.SetSize ( 256 );
	customMap.SetSize ( 256 );
	customTable.SetSize ( CODINGTABLESIZE + 1 );

	m_seq.m_pMgr = this;
	m_pEventMonitor   = 0;

	InitializeMaps();

	// InitializeVars can be called more than once, so don't
	// initialize stuff that should be set once here
	InitializeVars (TCodedBinBlock::CODE_UNKNOWN);

	m_fArticleFromCache_NoSKIP = false;
}

void TCodeMgr::InitializeVars(TCodedBinBlock::ECodeType kCodeType)
{
	m_codingState     = STATE_DECODE_SKIPPING;
	m_UsingMIME       = FALSE;
	m_table_count     = 0;

	thisNumBlocks = 0;
	prevContentEncoding = kCodeType;
	SetContentEncoding ( TCodedBinBlock::CODE_UNKNOWN );
	memset (prevBlockIdent, 0 , sizeof(prevBlockIdent));
	memset (thisContentType, 0, sizeof(thisContentType));
	memset (thisContentDesc, 0, sizeof(thisContentDesc));
}

// destruct
TCodeMgr::~TCodeMgr()
{
}

/* ------------------------------------------------------------------------
*    Mappings for encoding/decoding
*/
void TCodeMgr::InitializeMaps(void)
{
	CreateCodingMap (uuMap, uuTable);
	uuMap[' '] = 0;            // decode both quote and space to 0 in UU

	CreateCodingMap (xxMap, xxTable);

	CreateCodingMap (base64Map, base64Table);

	// the custom map is just a pointer.  The map is defined if/when a
	// custom table is read in during decoding
}

/* ------------------------------------------------------------------------
*    Mappings for encoding/decoding
*/
int TCodeMgr::CreateCodingMap (CByteArray & bmap, LPCTSTR table)
{
	register unsigned int i;

	for (i = 0; i < 256; i++)
		bmap[i] = MAP_UNUSED;

	// if you see a table[i] character, decode it to i
	// i.e. UU, table[1] is a '!'.  So a '!' should decode to 1

	for (i = 0; i < CODINGTABLESIZE; i++)
		if (bmap[table[i]] != MAP_UNUSED)
			return (table[i]);    // table[i] is duplicate
		else
			bmap[table[i]] = i;

#ifdef DEBUG_MAP2
	wsprintf (debug, "\nCoding Map:\n");
	DebugLog ();
	for (i = 0; i < 128; i++)
	{
		wsprintf (debug, "%d ", bmap[i]);
		DebugLog ();
	}
#endif
	return (-1);
}

/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int TCodeMgr::CreateCodingMap2 (CByteArray & bmap, CByteArray & table)
{
	register unsigned int i;

	for (i = 0; i < 128; i++)
		bmap[i] = MAP_UNUSED;

	// if you see a table[i] character, decode it to i
	// i.e. UU, table[1] is a '!'.  So a '!' should decode to 1

	for (i = 0; i < CODINGTABLESIZE; i++)
		if (bmap[table[i]] != MAP_UNUSED)
			return (table[i]);    // table[i] is duplicate
		else
			bmap[table[i]] = i;

#ifdef DEBUG_MAP2
	wsprintf (debug, "\nCoding Map:\n");
	DebugLog ();
	for (i = 0; i < 128; i++) {
		wsprintf (debug, "%d ", bmap[i]);
		DebugLog ();
	}
#endif
	return (-1);
}

/////////////////////////////////////////////////////////////////////
CByteArray & TCodeMgr::GetCodingMap(TCodedBinBlock::ECodeType kType)
{
	switch (kType)
	{
	case TCodedBinBlock::CODE_BASE64:
		return base64Map;
		break;

	case TCodedBinBlock::CODE_UU:
		return uuMap;
		break;

	case TCodedBinBlock::CODE_XX:
		return xxMap;
		break;

	case TCodedBinBlock::CODE_CUSTOM:
		return customMap;
		break;

	case TCodedBinBlock::CODE_NONE:
		ASSERT(0);
		return uuMap;

	default:
		ASSERT(0);
		return uuMap;
	}
}

TCodedBinBlock* TCodeMgr::AllocNewBlock(void)
{
	return new TCodedBinBlock;
}

int TCodeMgr::DecodeOneLine (CString& line, TCodedBinBlock* pBlock)
{
	int result = DecodeLine ( line, pBlock );

	return result;
}

/////////////////////////////////////////////////////////////////////
// Decode -- main entry point
//
// Arguments:
//    HWND - anchor window for floating status wnd?
//
// Returns:
//    0 for success
//    1 for skip file
int
TCodeMgr::Decode (CString &        strSubjFilename,
				  POINT &          sFraction,
				  TArticleHeader * pArtHdr,
				  TArticleText *   pArtText,
				  HWND             hwnd,
				  const CString &  strRealSubject,
				  int              iAskOverwrite /* = TRUE */)
{
	ASSERT(!m_destinationDir.IsEmpty());

	CString strUserAgent;
	m_iAskOverwrite = iAskOverwrite;
	bool fHog = false;

	// parse the header - figure out if we are MIME
	pArtText->ParseFullHeader(&strUserAgent);

	if (strUserAgent.IsEmpty() == FALSE && (strUserAgent.Find("Hogwasher/") != -1))
		fHog = true;

	if (!fHog && pArtText->IsBodyMime())
	{
		const CString & rSubj = pArtHdr->GetSubject();
		CString identity = strSubjFilename;   // possible filename

		// this handles the case where
		// eg.  a guy sends a JPG file, but the content-type is INCORRECTLY
		//      tagged as TEXT/PLAIN

		CString dummy_body;
		BOOL fSectionExit = TRUE;
		TPersist822Text::ERenderStat eStat =
			pArtText->RenderBody_MIME (m_seq.GetDestinationDir(),
			dummy_body, // output not used
			kRenderDecode,        // decode == TRUE
			identity,    // possible filename
			m_pEventMonitor, // pass it down (deep)
			m_fArticleFromCache_NoSKIP
			);

		if (TPersist822Text::kNoContentType == eStat &&
			(-1 != dummy_body.Find("Encoder: Turnpike Version")))
		{
			// evil turnpike program. Probably UUENCODED
			fSectionExit = FALSE;
		}
		if (TPersist822Text::kTreatAsUU == eStat)
		{
			// Mime Thing which is UUencoded, but labeled as text/plain
			fSectionExit = FALSE;
		}

		if (fSectionExit)
			return 0;
	}

	TCodedBinBlock* pBlock = AllocNewBlock();

	// upper levels have already analyzed the subject line
	pBlock->sequence = sFraction.x;    // numerator
	thisNumBlocks    = sFraction.y;    // denominator
	pBlock->ident    = strSubjFilename;

	return DecodeRun ( pBlock, &(pArtText->GetBody()) );
} // Decode

///////////////////////////////////////////////////////////////////////////
// Another entry point - used when You think you have the filename,
//   sequence and total.
//
//
// Hacker Note: MIME sections that are labelled as text but are probably
//              UUENCODED sections come in via here
//
//  Returns 0 for success,
//          1 to abort decode sequence (skip filename)
//          2 no-op
//
int TCodeMgr::DecodeIdentified(LPCTSTR filename, int iSequence, int iTotal,
							   LPCTSTR lpszText, int iLen, int iAskOverwrite /* = TRUE */)
{
	if (iLen <= 0)
		return 2;

	if (FALSE == uuencode_histogram(lpszText, iLen))
		return 2;

	ASSERT(!m_destinationDir.IsEmpty());

	// based on line length I think it is something encoded

	m_iAskOverwrite = iAskOverwrite;

	// argh this is stupid.
	CString text(lpszText, iLen);

	TCodedBinBlock* pBlock = AllocNewBlock();

	pBlock->name     = filename;
	pBlock->sequence = iSequence;
	thisNumBlocks    = iTotal;

	if (pBlock->ident.IsEmpty())
		pBlock->ident = filename;

	return DecodeRun ( pBlock, &text );
}

///////////////////////////////////////////////////////////////////////////
//  Input - pBlock should have the sequence and total and identity filled
//        - pText is the data
//
//  Returns 0 for success,
//          1 to abort decode sequence (skip filename)
int  TCodeMgr::DecodeRun (TCodedBinBlock* pBlock, const CString* pText)
{
	int pos = 0;                   // position in srcdata
	int iResult;
	int iLineLen;
	int iLen2;
	int iStat = 0;
	CString line;

	TStringEx smartStr(pText);

	while (1)
	{
		if (pos == 0)
		{
			if ((iLineLen = smartStr.GetLine ( pos, line )) == 0)
				line = "~EOF~";  // make sure wrap up occurs ('~' is not in any char map)

			if ((iLen2 = smartStr.GetLine ( pos, m_strNextLine )) == 0)
				m_strNextLine = "~EOF~";
		}
		else
		{
			line = m_strNextLine;
			iLineLen = iLen2;

			if ((iLen2 = smartStr.GetLine ( pos, m_strNextLine )) == 0)
				m_strNextLine = "~EOF~";
		}

		iResult = DecodeOneLine ( line, pBlock );

		if (FAIL == iResult)
		{
			iStat = 1;
			goto abortDecodeFile;
		}

		if (END_BLOCK == iResult)
		{
			if (FAIL == CompleteDecodeBlock (pBlock))
			{
				iStat = 1;
				goto abortDecodeFile;
			}

			// we need a new block
			pBlock = AllocNewBlock ();
		}

		if (0 == iLineLen)
			break;
	}

	// Finish last block
	CompleteDecodeBlock ( pBlock );

abortDecodeFile:
	DecodeDone ();
	return iStat;
}

// wrap up loose threads
void TCodeMgr::DecodeDone()
{
	// scaffold

	InitializeVars(TCodedBinBlock::CODE_UNKNOWN);
}

///////////////////////////////////////////////////////////////////////////
//    Decode a line in article.  Store in pBlock
//
int TCodeMgr::DecodeLine (CString & line, TCodedBinBlock* pBlock)
{
	/* ignore blank lines */
	int len = line.GetLength();

	if ((0 == len) ||
		((1 == len) && (line[0] == '\n')) ||
		((2 == len) && (line[0] == '\r') && line[1] == '\n'))
		return (SUCCESS);

	pBlock->numLines++;

	int  ret = END_BLOCK;

	switch (m_codingState)
	{
	case STATE_DECODE_SKIPPING:
		ret = Decode_Skipping (line, pBlock);
		break;

	case STATE_DECODE_GET_TABLE:
		{
			int ln = line.GetLength();

			LPTSTR pszLine = line.GetBuffer(ln);
			ret = Decode_GetTable (pszLine, pBlock);
			line.ReleaseBuffer(ln);
		}
		break;

	case STATE_DECODE_PROCESSING:
		ret = Decode_Processing(line, pBlock);
		break;

	default:
		ASSERT(0);   // unknown
		break;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////
// ExtractFileName -- takes a line of the form "begin 777 filename.jpg" and
// extracts the filename.  Returns 0 for success, non-0 for failure
static int ExtractFileName (LPCTSTR pchLine, LPTSTR pchFile, int iSize)
{
	CString strFile;
	int iBegin = 5;   // length of "begin "
	int iLineLen = strlen (pchLine);

	// skip white space
	while (iBegin < iLineLen && pchLine [iBegin] == ' ')
		iBegin ++;

	// skip the mode
	while (iBegin < iLineLen && pchLine [iBegin] >= '0' && pchLine [iBegin] <= '9')
		iBegin ++;

	// copy the rest of the line
	strFile = &pchLine [iBegin];

	// trim whitespace and quotes
	strFile.TrimLeft ();
	strFile.TrimRight ();
	if (strFile.Left (1) == "\"")
		strFile = strFile.Right (strFile.GetLength () - 1);
	if (strFile.Right (1) == "\"")
		strFile = strFile.Left (strFile.GetLength () - 1);

	// even after all this we still might have dir/dir2/filename
	int iFwdslash = strFile.ReverseFind ('/');
	if (iFwdslash != -1)
	{
		CString strTemp = strFile.Mid (iFwdslash + 1);

		if (strTemp.IsEmpty())
			strFile.Replace ('/', '-');
		else
			strFile = strTemp;
	}
	strncpy (pchFile, strFile, iSize);
	// strncpy() is not guaranteed to terminate the dest. string with a null
	pchFile [iSize - 1] = 0;
	return 0;
}

//////////////////////////////////////////////////////////////////////
//
int TCodeMgr::Decode_Skipping(
							  CString &         line,
							  TCodedBinBlock*   pBlock
							  )
{
	int      len;
	int      ret;
	LPTSTR   pszLine;

	if (line.GetLength() < 3)     // if we're skipping, ignore any really short line
		return (SUCCESS);

	// test for the new yEnc scheme

	if (SUCCESS == TestFor_yEnc ( line, pBlock ))
		return (SUCCESS);

	len = line.GetLength();
	pszLine = line.GetBuffer(len);

	ret = Decode_Skipping2 (pszLine, pBlock);

	line.ReleaseBuffer(len);
	return ret;
}

//////////////////////////////////////////////////////////////////////
//
int TCodeMgr::Decode_Skipping2(
							   LPTSTR            line,
							   TCodedBinBlock*   pBlock
							   )
{
	int   mode;
	char  thisBlockName[MAXINTERNALLINE];
	LPCTSTR ptr;

	// encoded UU/XX/Custom data always starts with line
	// like 'begin <mode> <file-name>'
	if (!strncmp (line, "begin", 5) &&
		sscanf (line, "%*s %d %s", &mode, thisBlockName) == 2 &&
		!ExtractFileName (line, thisBlockName, sizeof thisBlockName))
	{
#ifdef DEBUG_DECODE
		wsprintf (debug, "\nStart of new encoded file %s, mode %d", thisBlockName, mode);
		DebugLog ();
#endif
		if (thisBlockName[0] != '\0')
		{
			NameWithoutPath (pBlock->name, thisBlockName);
		}
		else
		{
			NameWithoutPath (pBlock->name, pBlock->ident);
		}

		pBlock->sequence = 1;
		pBlock->beginFlag = TRUE;

		//PreAllocDataByLines (pBlock);
		m_codingState = STATE_DECODE_PROCESSING;
		return (SUCCESS);
	}

	// Check for keywords in the non-data lines which may aid in describing
	// the name for this data, the sequence, a custom table, etc

	else if (!m_UsingMIME && !_strnicmp (line, "MIME-Version:", 13))
		m_UsingMIME = TRUE;
	else if (!m_DumbDecode && ParseAppSpecificLine (pBlock, line))
		return (SUCCESS);
	else if (!m_DumbDecode && !_strnicmp (line, "lines:", 6))
	{
		for (ptr = &line[6]; isspace (*ptr); ptr++); /* skip spaces */
		pBlock->estNumLines = atol (ptr);
		return (SUCCESS);
	}
	else if (!strncmp (line, "table", 5))     // next two lines are table
	{
		m_codingState = STATE_DECODE_GET_TABLE;
		m_table_count = 0;
		return (SUCCESS);
	}

	// Skipping to find new block of data.
	// Skip until a data line (a line with the correct length) is found
	else
	{
		if (IsDataLine (line, pBlock))
		{
			// found good line, process it and continue processing
#ifdef DEBUG_DECODE
			wsprintf (debug, "\nFound start of next data section", line);
			DebugLog ();
#endif
			//PreAllocDataByLines (pBlock);
			m_codingState = STATE_DECODE_PROCESSING;
			return (DecodeDataLine (line, thisContentEncoding,
				CallbackBinBlock, DWORD(pBlock)));
		}
	}
#ifdef DEBUG_DECODE
	wsprintf (debug, "\nskipped: %s", line);
	DebugLog ();
#endif
	return (SUCCESS);       // continue skipping
} // Decode_Skipping

/////

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
int TCodeMgr::Decode_GetTable(
							  LPTSTR           line,
							  TCodedBinBlock*   pBlock
							  )
{
	int i;

	// This is one of two lines containing table info
	for (int k = 0; k < 32; k++)
		customTable[ (m_table_count * 32) + k ] = line[k];

	if (++m_table_count == 2)
	{
#ifdef DEBUG_MAP
		strncpy (str, customTable, 64);
		str[64] = '\0';
		wsprintf (debug, "\nfound table: %s", str);
		DebugLog ();
#endif
		m_codingState = STATE_DECODE_SKIPPING;

		i = CreateCodingMap2 (GetCodingMap(TCodedBinBlock::CODE_CUSTOM), customTable);
		if (i != -1)
		{
#ifdef DEBUG_MAP
			wsprintf (debug, "Invalid decoding table in block.  Duplicate character %c.", i);
			DebugLog ();
#endif
			customTable[0] = '\0';  // ditch the table
		}
		else
			SetContentEncoding (TCodedBinBlock::CODE_CUSTOM);
	}

	return (SUCCESS);
} //  Decode_GetTable

int TCodeMgr::Decode_Processing(
								CString &         line,
								TCodedBinBlock*   pBlock
								)
{
	if (TCodedBinBlock::CODE_YENC == thisContentEncoding)
	{
		return Decode_YEnc_Line ( line,
			CallbackBinBlock,
			pBlock );
	}

	// Some encoders place an END line at end of a section, and length it like
	// a proper encoded line
	// Make sure we catch it, don't decode it as data, and switch back to skipping
	// Note the only End statement which means End of entire file is lower case
	// "end".  The other tags are ignored, i.e. "END", "End_of_section", etc
	//
	if (!_strnicmp (line, "end", 3))
	{
		goto label_big_if;
	}
	if (true)
	{
		int iRet ;
		int len = line.GetLength();
		LPTSTR pszLine = line.GetBuffer(len);
		iRet  = IsDataLine (pszLine, pBlock) ;

		line.ReleaseBuffer (len);
		if (0 == iRet)
			goto label_big_if;
	}

	if (m_UsingMIME &&
		*thisBoundary &&
		(!strcmp (line.Mid(2), thisBoundary) || !strcmp (line.Mid(2), thisBoundaryEnded)))
	{
		goto label_big_if;
	}

	goto label_end_big_if;
	{

label_big_if:

#ifdef DEBUG_DECODE
		wsprintf (debug, "\nSwitching back to skipping\nskipped: %s", line);
		DebugLog ();
#endif
		m_codingState = STATE_DECODE_SKIPPING;

		// If we are switching back to skipping and haven't really received any
		// data yet (< about a line's worth), then the line which gave us the coding type
		// was bogus (regular text line where first char happened to = encoded line
		// length, Here's one: 'From: marnold@cwis.unomaha.edu (Matthew Eldon Arnold)'
		// Ditch all data so far and the current table  and reprocess
		// this line.  Note if we have a custom table, then we trust it.
		//
		bool fExactEnd = false;

		if (0 == strncmp (line, "end", 3))
		{
			fExactEnd = true;
			pBlock->endFlag = TRUE;
		}

		if (! (pBlock->beginFlag && pBlock->endFlag) )
		{
			if (thisContentEncoding != TCodedBinBlock::CODE_CUSTOM && pBlock->size() < 80)
			{
				pBlock->ClearData();

				SetContentEncoding (TCodedBinBlock::CODE_UNKNOWN);
#ifdef DEBUG_DECODE
				wsprintf (debug, "\nBogus table!  Scrapping so far, reprocessing line: %s", line);
				DebugLog ();
#endif

				TRACE0("Bogus info!  Scrapping so far\n");
				pBlock->numLines--;
				pBlock->ClearData();
				return (DecodeLine (line, pBlock));
			}
		}
		strcpy (prevBlockIdent, pBlock->ident);

		if (fExactEnd)
		{
			pBlock->endFlag = TRUE;
			if (thisNumBlocks == -1 && pBlock->sequence != -1)
				thisNumBlocks = pBlock->sequence;
#ifdef DEBUG_DECODE
			wsprintf (debug, "\nFound end of file %s", pBlock->ident);
			DebugLog ();
#endif
		}
		return (END_BLOCK);
	}

label_end_big_if:

	int ln = line.GetLength();
	LPTSTR pszLine = line.GetBuffer(ln);

	int stat = DecodeDataLine (pszLine,
		thisContentEncoding,
		CallbackBinBlock,
		DWORD(pBlock));

	line.ReleaseBuffer(ln);
	return stat;
} // Decode_Processing

/* ------------------------------------------------------------------------
* ParseAppSpecificLine
* returns TRUE if it handles the line, else FALSE
*
* Example info lines handled by this function:
*  part=n
*  file=abc.def
*  pfile=xyz.abc
*  Archive-name: fileident/part0n      // no extension included for filename
*
*  section N of uuencode 5.10 of file abcd.efg   by R.E.M.
*  section n/N   file abcd.efg   [ Wincode v2.3 ]
*  [ Section: n/N  File: abcd.efg  Encoder: Wincode v1.4 ]
*  section n/N abcd.efg  [EnUU 2.1]
*  abcd.efg    section  n/N   UUXFER ver 2.0 by David M. Read
*  POST V2.0.0 abcd.efg (Part n/N)
*  Interim MIME support
*  Content-Type: application/octet-stream; [tokens]; name="abcd.efg"; [tokens]
*  name=abcd.efg
*
*  AL : i changed this to compare against lowercase
*/
BOOL
TCodeMgr::ParseAppSpecificLine (TCodedBinBlock* pBlock, LPTSTR line)
{
	LPTSTR orig;
	LPTSTR ptr;
	int seq, num, len;
	float ver;
	char name[MAXINTERNALLINE];
	char copy[MAXINTERNALLINE];

	for (orig = line; *orig && isspace (*orig); orig++);  // skip leading spaces

	if (*orig == '\0')         // blank line (or all spaces)

		return (SUCCESS);

	len = min (MAXINTERNALLINE - 1, strlen (orig));
	//strntcpy (copy, orig, len); // make a lower-case copy
	strncpy (copy, orig, len); // make a lower-case copy
	copy[len] = '\0';

	_strlwr (copy);

	if (0 == strncmp (orig, "BEGIN", 5)) // specifically comparing case-sensitive
	{
		/////ParseInfoLine (pBlock, copy, FALSE);
		return (TRUE);
	}

	if (sscanf (copy, "file=%s", name) == 1)
	{
		pBlock->ident = LPCTSTR(name);
		return (TRUE);
	}

	if (*copy == 'p')          // info headers starting with 'p'
	{
		if (sscanf (copy, "part=%d", &seq) == 1)
		{
			pBlock->sequence = seq;
			if (!pBlock->seqConfidence)
				pBlock->seqConfidence = LOW;  // still don't have number of parts
			return (TRUE);
		}
		if (sscanf (copy, "pfile=%s", name) == 1)
		{
			pBlock->ident = LPCTSTR(name);
			return (TRUE);
		}
		/* POST */
		if (sscanf (copy, "post v%*s %s (part %d/%d)", name, &seq, &num) == 3)
		{
			pBlock->sequence = seq;
			thisNumBlocks = num;
			pBlock->seqConfidence = HIGH;
			pBlock->ident = LPCTSTR(name);
			return (TRUE);
		}
		return (FALSE);            // started with 'p' but we didn't have a template
	}

	/* note some of these look for a version number, then don't use it.  the
	version number is scanned simply to add to our confidence that this is
	a valid info line.  we want it to be included in the count - so we know
	if the line fits the template (hence don't use %*f)
	*/

	if (m_UsingMIME && ParseMIMEHeader (orig, pBlock))
	{
		return TRUE;
	}

	/* R.E.M., EnUU, and WinCode v2.x all start with "section " */
	if (!strncmp (copy, "section", 7))
	{
		ptr = &copy[7];
		/* R.E.M. */
		if (sscanf (ptr, " %d of uuencode %f of file %s", &seq, &ver, name) == 3) {
			pBlock->sequence = seq;
			if (!pBlock->seqConfidence)
				pBlock->seqConfidence = LOW;  // still don't have number of parts

			pBlock->ident = LPCTSTR(name);
			SetContentEncoding (TCodedBinBlock::CODE_UU);
			return (TRUE);
		}
		/* EnUU */
		if (sscanf (ptr, " %d/%d %s [enuu %f]", &seq, &num, name, &ver) == 4) {
			pBlock->sequence = seq;
			thisNumBlocks = num;
			pBlock->seqConfidence = HIGH;
			pBlock->ident = LPCTSTR(name);
			SetContentEncoding (TCodedBinBlock::CODE_UU);
			return (TRUE);
		}
		/* Wincode v2.x */
		if (sscanf (ptr, " %d/%d  file %s [ wincode v%f ]", &seq, &num, name, &ver) == 4) {
			pBlock->sequence = seq;
			thisNumBlocks = num;
			pBlock->seqConfidence = HIGH;
			pBlock->ident = LPCTSTR(name);
			return (TRUE);
		}
		return (FALSE);            // started with 'section' but we didn't have a template
	}

	/* Wincode v1.x */
	if (sscanf (copy, "[ section: %d/%d  file: %s  encoder: wincode v%f", &seq, &num, name, &ver) == 4)
	{
		pBlock->sequence = seq;
		thisNumBlocks = num;
		pBlock->seqConfidence = HIGH;
		pBlock->ident = LPCTSTR(name);
		return (TRUE);
	}

	/* UULite */
	if (sscanf (copy, "[section: %d/%d file: %s  uulite v%f", &seq, &num, name, &ver) == 4)
	{
		pBlock->sequence = seq;
		thisNumBlocks = num;
		pBlock->seqConfidence = HIGH;
		pBlock->ident = LPCTSTR(name);
		SetContentEncoding (TCodedBinBlock::CODE_UU);
		return (TRUE);
	}

	/* UUXFER */
	if (sscanf (copy, "%s section %d/%d  uuxfer ver %f", name, &seq, &num, &ver) == 4)
	{
		pBlock->sequence = seq;
		thisNumBlocks = num;
		pBlock->seqConfidence = HIGH;
		pBlock->ident = LPCTSTR(name);
		SetContentEncoding (TCodedBinBlock::CODE_UU);
		return (TRUE);
	}

	if (sscanf (copy, "archive-name: %[^/]/part%d", name, &seq) == 2)    // %[^/] reads a string up to a slash
	{
		pBlock->ident = LPCTSTR(name);
		pBlock->sequence = seq;
		if (!pBlock->seqConfidence)
			pBlock->seqConfidence = LOW;  // still don't have number of parts

		return (TRUE);
	}

	return (FALSE);
}

/* ParseMIMEHeader
* this a very simplisitc approach
* returns TRUE if it was a MIME header (regardless of whether we handled it ok)
*/
BOOL TCodeMgr::ParseMIMEHeader (LPTSTR line, TCodedBinBlock* pBlock)
{
	LPTSTR ptr;
	int result, num;
	char temp[MAXINTERNALLINE];

	ptr = line;
	result = FALSE;

	if (thisContentEncoding == TCodedBinBlock::CODE_UNKNOWN &&
		!_strnicmp (ptr, "Content-Transfer-Encoding:", 26)) {
			ptr += 26;
			if (SkipSpace (&ptr) &&
				GetPossiblyQuotedStr (temp, ptr, MAXINTERNALLINE - 1)) {
					if (!_stricmp (temp, "base64")) {
						SetContentEncoding (TCodedBinBlock::CODE_BASE64);
						pBlock->sequence = 1;
					}
					else if (!_strnicmp (temp, "x-uu", 4)) {
						SetContentEncoding ( TCodedBinBlock::CODE_UU );
					}
					else if (!_strnicmp (temp, "x-xx", 4)) {
						SetContentEncoding ( TCodedBinBlock::CODE_XX );
					}
					else if (!_stricmp (temp, "quoted-printable"))
						SetContentEncoding ( TCodedBinBlock::CODE_QP );
			}
			return TRUE;
	}
	while (*ptr) {
		if (!_strnicmp (ptr, "name=", 5)) {
			char rcIdent[MAXINTERNALLINE];

			if (GetPossiblyQuotedStr (rcIdent, &ptr[5], MAXINTERNALLINE - 1)) {
				pBlock->ident = LPCTSTR(rcIdent);
				result = TRUE;
			}
		}
		if (!_strnicmp (ptr, "number=", 7)) {
			if (GetNumber (&(pBlock->sequence), &ptr[7])) {
				result = TRUE;
				pBlock->seqConfidence = HIGH;
			}
		}
		if (!_strnicmp (ptr, "total=", 6)) {
			if (GetNumber (&num, &ptr[6])) {
				result = TRUE;
				thisNumBlocks = num;
				pBlock->seqConfidence = HIGH;
			}
		}
		if (!_strnicmp (ptr, "boundary=", 9)) {
			/* this is no good for encapsulated multipart sections. but ok for now */
			if (GetPossiblyQuotedStr (thisBoundary, &ptr[9], MAXINTERNALLINE - 1)) {
				_snprintf (thisBoundaryEnded, MAXINTERNALLINE, "%s--", thisBoundary);
				result = TRUE;
			}
		}
		if (!SkipToNextClause (&ptr)) {
			break;
		}
	}
	return result;
}

/* ------------------------------------------------------------------------
*  4 to 3 line decoder
*/
BOOL
TCodeMgr::DecodeDataLine (
						  LPTSTR                    line,
						  TCodedBinBlock::ECodeType eEncoding,
						  CODE_HELPER               pfnHelper,
						  DWORD                     dwCookie)
{
	register unsigned int i, j;
	unsigned int          decodedCount, numEncoded, numDecoded;
#ifdef CHECKSUM
	UINT                  checkSum;
#endif
	UINT                  startNum, stop;
	unsigned char         buf[4];
	static const int      cbOutLine = 120;
	unsigned char         outLine[cbOutLine];
	int                   offset;
	unsigned char *       pOutLine;   // points to array or dynamic memory
	unsigned char *       pDynamic = 0;

	//if (TCodedBinBlock::CODE_YENC  ==  eEncoding)
	//      return Decode_YEnc_Line (line, pfnHelper, dwCookie);

	CByteArray & thisMap = GetCodingMap (eEncoding);

	switch (eEncoding)
	{
	case TCodedBinBlock::CODE_QP:
		// not implemented yet
		return (FAIL);

	case TCodedBinBlock::CODE_UU:
	case TCodedBinBlock::CODE_XX:
	case TCodedBinBlock::CODE_CUSTOM:
		{
			startNum = 1;

			// these are 4-to-3 decodings which have the line length encoded
			// as the first char (count is # of chars after decoding)
			decodedCount = thisMap[ (UCHAR) line[0]];

			if (MAP_UNUSED == decodedCount)
			{
				decodedCount = 0;
			}

			else
			{
				// to precisely determine the numEncoded from this count char,
				// add (4 * the number of full 3-byte units) and a remainder offset
				// the remainder offset is 2 if 1 byte over, or 3 if 2 bytes over

				if ((offset = (decodedCount % 3)) != 0)
				{
					offset++;
				}
				numEncoded = 4 * (decodedCount / 3) + offset;

				LPCTSTR pBadLine = (line + 1);
				int nLen = lstrlen (pBadLine);

				if (nLen >= 2 && '\r' == pBadLine[nLen-2] && '\n' == pBadLine[nLen-1])
					nLen -= 2;

				// we should have numEncoded characters on this line

				if (nLen >= numEncoded)
				{
					// ok
				}
				else
				{
					// this is a short line!
					decodedCount = nLen * 3 / 4;
					numEncoded = nLen;
				}
			}
			break;
		}

	case TCodedBinBlock::CODE_BASE64:
		// base64 is 4-to-3 decoding with nonexplicit line lengths
		numEncoded = strlen (line);
		decodedCount = 3 * (numEncoded / 4);
		startNum = 0;
		break;

	}

	// setup the ptr one way or the other
	if (decodedCount + 10 <= cbOutLine)
		pOutLine = &outLine[0];
	else
		pOutLine = pDynamic = new UCHAR[decodedCount + 10];

	for (i = startNum, numDecoded = 0; numDecoded < decodedCount;)
	{
		// Get the next group of four characters
		// Handle last group of characters in a base64 encoding specially -
		// padding '=' at end means we have fewer than 24 bits after decoding
		// Base64 end scenarios:
		//  'xx==' decodes to 8 bits
		//  'xxx=' decodes to 16 bits
		//  'xxxx' decodes to 24 bits
		//  'x===' can't happen
		if ((stop = numEncoded - i + 1) < 4)
		{
			memset (buf, 0, 4);
		}
		else
		{
			stop = 4;
		}

		for (j = 0; j < stop; j++, i++)
		{
			if (eEncoding == TCodedBinBlock::CODE_BASE64 && line[i] == '=')
			{
				buf[j] = 0;
				j--;
				break;
			}
			else
			{
				buf[j] = thisMap[(UCHAR)line[i]];
			}
			//  checkSum += buf[j];
		}

		pOutLine[numDecoded++] = buf[0] << 2 | buf[1] >> 4;
		if (j == 1 || numDecoded == decodedCount)
			break;

		pOutLine[numDecoded++] = buf[1] << 4 | buf[2] >> 2;
		if (j == 2 || numDecoded == decodedCount)
			break;

		pOutLine[numDecoded++] = buf[2] << 6 | buf[3];

	} // outer for loop

	if (numDecoded && pfnHelper(dwCookie, pOutLine, numDecoded) == FAIL)
	{
		if (pDynamic)
			delete pDynamic;
		return (FAIL);
	}

#ifdef CHECKSUM
	if (checkSum%64 != line[i])
	{
		printf("\nChecksum error: %d vs. %d", checkSum%64, line[i]);
		break;
	}
	else
		printf("\nChecksum ok: %c vs. %c", checkSum%64, line[i]);
#endif

	if (pDynamic)
		delete pDynamic;
	return (SUCCESS);
}

// --------------------------------------------------------------------------
BOOL CallbackBinBlock (DWORD dwCookie, BYTE* poutLine, UINT numDecoded)
{
	TCodedBinBlock * pBlock = (TCodedBinBlock *) dwCookie;

	if (!pBlock->AddData (poutLine, numDecoded))
		return FAIL;

	return TRUE;
}

BOOL CallbackStream (DWORD dwCookie, BYTE* poutLine, UINT numDecoded)
{
	ostream* pOutStream = (ostream*) dwCookie;
	pOutStream->write ( (char*) poutLine, numDecoded);
	if (pOutStream->fail())
		return FAIL;
	return TRUE;
}

// ------------------------------------------------------------------------
//    Complete a decode block
//  Adds to thread list
//  Writes block to disk if possible (and any others now in sequence)
//  If it ends a thread, make sure everything is written, and
//  free up the thread
//
//  Returns SUCCESS or FAIL

int TCodeMgr::CompleteDecodeBlock (TCodedBinBlock* pBlock)
{
#ifdef DEBUG_DECODE
	wsprintf (debug, "\nCompletepBlock for file %s, block %d",
		currentCoded->ident, currentCoded->sequence);
	DebugLog ();
#endif

	m_codingState = STATE_DECODE_SKIPPING;

	if (pBlock->size() == 0)
	{
#ifdef DEBUG_DECODE
		wsprintf (debug, "\nNon-data block.  Discarding it.");
		DebugLog ();
#endif
		delete pBlock;
		return (SUCCESS);
	}

	BOOL ret = m_seq.AddBlock (pBlock, thisNumBlocks, thisContentEncoding, m_pEventMonitor);

	// prevEncoding is thisContentEncoding
	InitializeVars (thisContentEncoding);

	return ret ? SUCCESS : FAIL;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void TCodeMgr::NameWithoutPath(CString& dest, const CString& src)
{
	TPath path = src;
	TPath file;
	path.GetFile ( file );

	CString temp = file;
	dest.Empty();

	for (int i = 0; i < temp.GetLength(); i++)
	{
		TCHAR c = temp[i];

		switch (c)
		{
		case '\\':
		case '/':
		case '?':
		case '*':
		case '"':
		case '<':
		case '>':
		case '|':
			break;
		default:
			dest += c;
			break;
		}
	}
}

/* ------------------------------------------------------------------------
*    Determine if the line is a data line
*/
BOOL
TCodeMgr::IsDataLine (LPTSTR line, TCodedBinBlock* pBlock)
{
	TCodedBinBlock::ECodeType type;

	if (thisContentEncoding == TCodedBinBlock::CODE_UNKNOWN)
	{
		/* If this block has same ident as prev block, use same decode table
		* If this block has same ident as any existing threads, use same decode table
		* as matching thread
		* Otherwise, test line for UU, XX, or Base64, returning if find a fit
		*/
		if (!pBlock->ident.IsEmpty()  &&  !strcmp (pBlock->ident, prevBlockIdent))
		{
			TRACE0("Using same table as prev decode block\n");

			SetContentEncoding ( prevContentEncoding );

#ifdef DEBUG_MAP
			strcpy (debug, "\nUsing same table as prev decode block");
			DebugLog ();
#endif
		}
		else if ((type = ThreadTable (customTable, pBlock->ident)) != TCodedBinBlock::CODE_UNKNOWN)
		{
			SetContentEncoding (type);
			if (thisContentEncoding == TCodedBinBlock::CODE_CUSTOM)
				CreateCodingMap2 (GetCodingMap(TCodedBinBlock::CODE_CUSTOM), customTable);
			TRACE0("Using stored thread table\n");
#ifdef DEBUG_MAP
			strcpy (debug, "\nUsing stored thread table");
			DebugLog ();
#endif
		}
		else
		{
			SetContentEncoding (TCodedBinBlock::CODE_UU);
			if (TestDataLine (line, TCodedBinBlock::CODE_UU))
			{
#ifdef DEBUG_MAP
				wsprintf (debug, "\nUsing UU table: %s", line);
				DebugLog ();
#endif
				TRACE1("Using UU table: %s\n", line);
				return (TRUE);
			}
			SetContentEncoding (TCodedBinBlock::CODE_XX);
			if (TestDataLine (line, TCodedBinBlock::CODE_XX))
			{
#ifdef DEBUG_MAP
				wsprintf (debug, "\nUsing XX table: %s", line);
				DebugLog ();
#endif
				TRACE1("Using XX table: %s\n", line);
				return (TRUE);
			}
			SetContentEncoding (TCodedBinBlock::CODE_BASE64);
			if (TestDataLine (line, TCodedBinBlock::CODE_BASE64))
			{
#ifdef DEBUG_MAP
				wsprintf (debug, "\nUsing Base64 table: %s", line);
				DebugLog ();
#endif
				TRACE1("Using Base64 table: %s\n", line);
				if (0 == thisNumBlocks)
				{
					// 1-19-96 still not set. Assume One of One
					pBlock->seqConfidence = LOW;
					pBlock->sequence = 1;
					thisNumBlocks = 1;
				}
				return (TRUE);
			}
			SetContentEncoding (TCodedBinBlock::CODE_UNKNOWN);
			return (FALSE);
		}
	}
	return (TestDataLine (line, thisContentEncoding));
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
BOOL TCodeMgr::TestDataLine (
							 LPTSTR line,
							 TCodedBinBlock::ECodeType eContentEncoding
							 )
{
	unsigned int expectedLineLen, dataLen, lineLen;
	unsigned int decodedCount, offset, tolerance;
	register unsigned int i;
	int          iIndex;

	CByteArray & thisMap = GetCodingMap ( eContentEncoding );

	lineLen = strlen (line);
	switch (eContentEncoding) {
  case TCodedBinBlock::CODE_UU:
  case TCodedBinBlock::CODE_XX:
  case TCodedBinBlock::CODE_CUSTOM:
	  /*
	  * The first char of a good data UU/XX/Custom line is the encoded character
	  * count for the line.
	  * example M = ascii 77, decodes to 45.  This line will decode to 45 characters,
	  * so the # encoded chars should be (int)(4 * ((n+2) / 3)) - (round up and re-encode)
	  * So if line[0] is 'M', there should be 60 characters on the line
	  * The count does not include the first count char, so the expected line
	  * length is count+1 (i.e. 61).
	  * If the line length is actually 2 + count, then we have a checksum (?)
	  */
	  //  expectedLineLen = 4 * (thisMap[line[0]] + 2) / 3) + 1;

	  // validate the index
	  iIndex = (UCHAR) line[0];

	  if (iIndex >= 128)
		  return FALSE;

	  decodedCount = thisMap[iIndex];
	  /* to precisely determine the numEncoded from this count char,
	  * add (4 * the number of full 3-byte units) and a remainder offset
	  * the remainder offset is 2 if 1 byte over, or 3 if 2 bytes over.
	  * Not all encoders are smart enough to do this though, and just leave
	  * the last (4-offset) bytes as junk (usually bytes found in the same
	  * location on the previous line).  So allow a tolerance of (4-offset)
	  * bytes
	  */
	  if ((offset = (decodedCount % 3)) != 0) {
		  offset++;
		  tolerance = 4 - offset;
	  }
	  else {
		  tolerance = 0;
	  }
	  expectedLineLen = 4 * (decodedCount / 3) + offset + 1;   // add 1 for start char

	  /*
	  * Count the number of encoded data characters on the line (up to any whitespace)
	  * (Some encoders pad with spaces at the end).  Don't forget, a space may be
	  * a valid encoded char, so a proper line may very well end with a space.
	  * Don't blindly remove all trailing white space!
	  */
	  // first strip trailing non-spaces (i.e. \n\r)
	  for (dataLen = lineLen;
		  isspace (line[dataLen - 1]) && (thisMap[' '] == MAP_UNUSED || line[dataLen - 1] != ' ') && dataLen > 0;
		  --dataLen);

	  // now deal with spaces
	  if (thisMap[' '] != MAP_UNUSED) {
		  for (; line[dataLen - 1] == ' ' && dataLen > 0 && dataLen != expectedLineLen;
			  --dataLen);
	  }

	  if (TCodedBinBlock::CODE_UU == eContentEncoding && m_strNextLine == "`\r\n" )
	  {
		  // skip tolerance test

	  }
	  else
	  {
		  // perform tolerance test

		  // May have a checksum character, so allow for one extra char
		  // also allow for encoders which include the count char in the line count
		  if (expectedLineLen != dataLen && expectedLineLen + tolerance != dataLen &&
			  expectedLineLen + 1 != dataLen && expectedLineLen + tolerance + 1 != dataLen &&
			  expectedLineLen - 1 != dataLen && expectedLineLen + tolerance - 1 != dataLen)
			  return (FALSE);
	  }

	  line[dataLen] = '\0';      // permanently chop off the white space
	  // It's the right length, now check content for match w/ table

	  for (i = 0; i < dataLen; i++)
	  {
		  if (thisMap[ (UCHAR) line[i]] == MAP_UNUSED)
			  return (FALSE);
	  }
	  return (TRUE);

  case TCodedBinBlock::CODE_BASE64:
	  // permanently remove all trailing space
	  while (lineLen > 0 && isspace (line[lineLen - 1]))
		  line[--lineLen] = '\0';

	  // for base 64, just check if all chars in coding map (allow pad '=')
	  for (i = 0; i < lineLen; i++)
	  {
		  iIndex = (UCHAR) line[i];
		  if (iIndex >= 128)
			  return FALSE;

		  if (thisMap[iIndex] == MAP_UNUSED && line[i] != '=')
			  return (FALSE);
	  }
	  return (TRUE);

  case TCodedBinBlock::CODE_QP:
	  // not implemented yet
  default:
	  return (FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// install an event monitor in the encode/decode manager.
/////////////////////////////////////////////////////////////////////////////

BOOL  TCodeMgr::
InstallEventMonitor (CODE_EVENT_MONITOR   pEventMonitor)

{
	if (m_pEventMonitor)
		return FALSE;

	m_pEventMonitor = pEventMonitor;
	return TRUE;
}

/* ------------------------------------------------------------------------
* simple MIME header handling - not very smart!
*/
BOOL TCodeMgr::
SkipSpace (LPTSTR* ptr)
{
	while (**ptr && isspace (**ptr)) {
		(*ptr)++;
	}

	return (**ptr != '\0');
}

BOOL TCodeMgr::
SkipToNextClause (LPTSTR * ptr)
{
	while (**ptr && **ptr != ';') {   // skip to semi-colon or end

		(*ptr)++;
	}
	if (**ptr == ';') {         // skip semi-colon

		(*ptr)++;
	}
	if (!SkipSpace (ptr)) {     // skip space after semi-colon

		return FALSE;
	}
	return TRUE;
}

BOOL TCodeMgr::
GetNumber (int *dest, LPTSTR src)
{
	char temp[20];
	char *ptr, *destPtr;

	ptr = src;
	destPtr = temp;
	*destPtr = '\0';
	while (*ptr && isdigit (*ptr)) {
		*destPtr++ = *ptr++;
	}
	if (*temp == '\0')
		return FALSE;

	*dest = atoi (temp);
	return TRUE;
}

BOOL TCodeMgr::
GetPossiblyQuotedStr (char *dest, LPTSTR src, int maxLen)
{
	char *ptr, *destPtr;
	int len;
	BOOL quoted = FALSE;

	ptr = src;
	destPtr = dest;
	*destPtr = '\0';
	len = 0;
	if (*ptr == '"' || *ptr == '\'') {
		ptr++;
		quoted = TRUE;
	}

	// if quoted, seek to end quote (allow spaces and semicolons inside quotes)
	// if not in quotes, read up to space or semicolon
	while (*ptr && len < maxLen) {
		if (quoted) {
			if (*ptr == '"' || *ptr == '\'') {
				break;
			}
		}
		else {
			if (isspace (*ptr) || *ptr == ';') {
				break;
			}
		}
		len++;
		*destPtr++ = *ptr++;
	}
	*destPtr = '\0';

	if (*dest == '\0')
		return FALSE;

	//  if (quoted && *ptr != '"' && *ptr != '\'')  // missing end-quot
	//      return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
BOOL TCodedSequencer::AddBlock(
							   TCodedBinBlock* pBlock,
							   int expectedBlocks,
							   TCodedBinBlock::ECodeType type,
							   CODE_EVENT_MONITOR   pEventMonitor
							   )
{
	BOOL fMatch = FALSE;

	TCodedThread* pThread = FindThread ( pBlock->ident );

	// Add new thread
	if (0 == pThread)
	{
		pThread = new TCodedThread(this);
		pThread->ident = pBlock->ident;

		pThread->m_iAskOverwrite = m_pMgr->m_iAskOverwrite;

		if (!pBlock->name.IsEmpty())
			pThread->name  = pBlock->name;
		pThread->contentEncoding = type;

		m_threadList.AddTail ( pThread );
	}

	// thread has no name
	if (pThread->name.IsEmpty() && !pBlock->name.IsEmpty())
		pThread->name = pBlock->name;

	if ((pBlock->beginFlag || pBlock->sequence == 1) && pThread->name.IsEmpty())
	{
		// use the ident as the name
		pThread->name = pBlock->ident;
		TRACE1("using ident %s as name\n", LPCTSTR(pBlock->ident));
	}

	if (expectedBlocks != 0 && pThread->expectedNumBlocks == 0)
		pThread->expectedNumBlocks = expectedBlocks;
	pThread->AddBlock ( pBlock );

	//// WRITE OUT WHAT WE CAN

	if (pThread->contentEncoding == TCodedBinBlock::CODE_BASE64 &&
		pThread->expectedNumBlocks != 0 &&
		pBlock->sequence == pThread->expectedNumBlocks)
	{
		/* base-64 end flag if last block */
		pBlock->endFlag = TRUE;
	}

	BOOL  singleBlockDone = pBlock->beginFlag && pBlock->endFlag;

	BOOL fWroteEndBlock = FALSE;
	if (FALSE == singleBlockDone)
	{
		if (pThread->WriteSequentialBlocks(TRUE, &fWroteEndBlock, pEventMonitor))
			return FALSE;
	}

	/* If this block is both begin and end, then go straight to WriteDecodeThread
	* If currentCoded is in sequence, then it was added as block 0 in the
	* thread's block list, and may have caused other blocks to now be in
	* sequence as well.  Write all blocks which are in sequence now.
	*/
	bool fNumericallyComplete =
		(pThread->expectedNumBlocks > 0) &&
		(pThread->numBlocksWritten  >= pThread->expectedNumBlocks);

	if (singleBlockDone || (fWroteEndBlock && fNumericallyComplete) )
	{
		TRACE1("Thread complete: file %s\n", pThread->name);

		// write it out!  non-zero return is an error condition
		int writeStat = pThread->Write();

		POSITION pos = m_threadList.Find ( pThread );
		m_threadList.RemoveAt ( pos );
		if (pEventMonitor)
		{
			TCodeEvent   event;

			if (0==writeStat)
			{
				if (pThread->m_fYEncoded)
				{
					if (0 == pThread->m_fYEncPassCRC)
					{
						event.m_kEventType = TCodeEvent::kFileFailedCRC;
					}
					else if (0 == pThread->m_fYEncCorrectSize)
					{
						event.m_kEventType = TCodeEvent::kFileFailedYEncSize;
					}
					else
						event.m_kEventType = TCodeEvent::kFileFinished;
				}
				else
					event.m_kEventType = TCodeEvent::kFileFinished;
			}
			else if (1==writeStat)
				event.m_kEventType = TCodeEvent::kFileError;
			else
				event.m_kEventType = TCodeEvent::kFileSkipped;

			event.m_file = pThread->dosFileName;
			pEventMonitor (&event);
		}
		delete pThread;
		return TRUE;
	}

	// numerically complete, but we didn't see a real 'end' line
	if (fNumericallyComplete)
	{
		TRACE1("Thread parts completed: file %s\n", pThread->name);

		// write it out!  non-zero return is an error condition
		int writeStat = pThread->Write();

		POSITION pos = m_threadList.Find ( pThread );
		m_threadList.RemoveAt ( pos );
		if (pEventMonitor)
		{
			TCodeEvent   event;

			if (0==writeStat)
			{
				if (pThread->m_fYEncoded)
				{
					if (0 == pThread->m_fYEncPassCRC)
					{
						event.m_kEventType = TCodeEvent::kFileFailedCRC;
					}
					else if (0 == pThread->m_fYEncCorrectSize)
					{
						event.m_kEventType = TCodeEvent::kFileFailedYEncSize;
					}
					else
						event.m_kEventType = TCodeEvent::kFileFinished;
				}
				else
					event.m_kEventType = TCodeEvent::kFileSeemsIncomplete;
			}
			else if (1==writeStat)
				event.m_kEventType = TCodeEvent::kFileError;
			else
				event.m_kEventType = TCodeEvent::kFileSkipped;

			event.m_file = pThread->dosFileName;
			pEventMonitor (&event);
		}
		delete pThread;
		return TRUE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// Returns 0 for success
//         1 for fail
//         2 for skip
int TCodedThread::Write(void)
{
	try
	{
		int total = m_BlockList.GetCount();
		if (total == 0)
			return total;

		CFile fl;

		TPath bestName;

		if (!dosFileName.IsEmpty())      // this is Path + filename
			bestName = dosFileName;
		else
			bestName.FormPath (pSequencer->GetDestinationDir(), this->name);

		dosFileName = bestName;  // tony added 9/12

		bool fArticleFromCache_NoSKIP = pSequencer->m_pMgr->m_fArticleFromCache_NoSKIP;

		int iRet = 0;
		{
			auto_prio sPrio(auto_prio::kNormal);  // boost priority for dlgbox

			TYP_GETOUTPUTFILE sMode;         // various file choices

			CWnd *pParent = gpsDecodeDialog
				? gpsDecodeDialog
				: CWnd::FromHandle (ghwndMainFrame);

			iRet = TFileUtil::CreateOutputFile (pParent,
				bestName,                    // 1st Try
				pSequencer->GetDestinationDir(),  // default dir
				IDS_SAVE_DECODED_AS,
				fl,
				&sMode,
				fArticleFromCache_NoSKIP,
				IDS_FILTER_DECODE);
		}

		if (iRet)
			return iRet;

		// file is open...  if the name was changed within CreateOutputFile(),
		// update our filename
		if (dosFileName != bestName)
			dosFileName = bestName;

		write_util ( fl );
	}
	catch(CFileException *pExcept)
	{
		NewsShowCException (NULL, pExcept, MB_OK | MB_ICONSTOP);
		pExcept->Delete();
		return 1;
	}
	return 0;
}

// 12-31-95  handle the disk full condition
void TCodedThread::write_util (CFile& fl)
{
	POSITION pos = m_BlockList.GetHeadPosition();
	TCodedBinBlock* pBlock;

	try
	{
		while (pos)
		{
			pBlock = m_BlockList.GetNext(pos);

			pBlock->Write ( fl );
		}

		fl.Flush();
		fl.Close();
	}
	catch(...)
	{
		fl.Flush();
		fl.Close();
		throw;
	}
}

// ------------------------------------------------------------------
int TCodedBinBlock::Write ( CFile& fl )
{
	try
	{
		int numBytes = m_memSpot.size();

		fl.Write ( m_memSpot.getData(), numBytes );
		return numBytes;
	}
	catch(CFileException *pExcept)
	{
		NewsShowCException (NULL, pExcept, MB_OK | MB_ICONSTOP);
		pExcept->Delete();
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////
// Returns 0 for success
//         1 for error
//         2 for skip file
int TCodedThread::WriteSequentialBlocks(BOOL fMemFree, BOOL* pfWroteEndBlock,
										CODE_EVENT_MONITOR pEventMonitor)
{
	TCodedBinBlock* pBlock;

	if (m_BlockList.IsEmpty())
		return 0;
	pBlock = m_BlockList.GetHead();

	// skip repeats
	if (numBlocksWritten == pBlock->sequence)
	{
		delete pBlock;
		m_BlockList.RemoveHead();
		return 0;
	}

	if (numBlocksWritten + 1 != pBlock->sequence)
		return 0;

	// we have a candidate
	CFile fl;

	TPath bestName;

	bool fArticleFromCache_NoSKIP = pSequencer->m_pMgr->m_fArticleFromCache_NoSKIP;

	if (!dosFileName.IsEmpty())      // this is Path + filename
		bestName = dosFileName;
	else
		bestName.FormPath (pSequencer->GetDestinationDir(), this->name);

	if (fFileCreated)
	{
		fl.Open (bestName, CFile::modeWrite | CFile::shareExclusive);
		fl.SeekToEnd();
	}
	else
	{
		// 1-3-96 bump up priority
		auto_prio sBoost(auto_prio::kNormal);

		TYP_GETOUTPUTFILE sMode;         // various file choices

		int iRet =
			TFileUtil::CreateOutputFile (
			gpsDecodeDialog ? gpsDecodeDialog : CWnd::FromHandle(ghwndMainFrame),
			bestName,                    // 1st Try
			pSequencer->GetDestinationDir(),  // default dir
			IDS_SAVE_DECODED_AS,
			fl,
			&sMode,
			fArticleFromCache_NoSKIP,
			IDS_FILTER_DECODE);

		if (1 == iRet)
		{
			if (pEventMonitor)
			{
				TCodeEvent event;
				event.m_kEventType = TCodeEvent::kFileError;
				pEventMonitor (&event);
			}
			return 1;     // failure
		}

		// save the result.
		dosFileName = bestName;

		if (2 == iRet)   // filename conflict - skip. This is success
		{
			// as soon as we know the file name, tell the event monitor
			if (pEventMonitor)
			{
				TCodeEvent event;
				event.m_kEventType = TCodeEvent::kFileSkipped;
				event.m_file = dosFileName;
				pEventMonitor (&event);
			}

			return 2;
		}

		// as soon as we know the file name, tell the event monitor
		if (pEventMonitor)
		{
			TCodeEvent event;
			event.m_kEventType = TCodeEvent::kFileName;
			event.m_file = dosFileName;
			pEventMonitor (&event);
		}

		fFileCreated = TRUE;
	}

	*pfWroteEndBlock = FALSE;

	for (;;)
	{
		if (m_BlockList.IsEmpty())
			break;
		pBlock = m_BlockList.GetHead();

		// write out this block
		if (numBlocksWritten + 1 == pBlock->sequence)
		{
			// here it is
			int wret = pBlock->Write ( fl );

			numBlocksWritten ++;
			if (pBlock->endFlag)
				*pfWroteEndBlock = TRUE;
			m_BlockList.RemoveHead();
			delete pBlock;
			if (0 == wret)
			{
				if (pEventMonitor)
				{
					TCodeEvent event;
					event.m_kEventType = TCodeEvent::kIOError;
					pEventMonitor (&event);
				}
				return 1;
			}
		}
		else if (numBlocksWritten == pBlock->sequence)
		{
			// this is a repeat
			m_BlockList.RemoveHead();
			delete pBlock;
		}
		else
			break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////
int TCodedThread::AddBlock (TCodedBinBlock* pBlock)
{
	m_fYEncoded = pBlock->m_fYEncoded;

	if (m_fYEncoded)
	{
		if (0 == pBlock->m_fYEncPassCRC)
			m_fYEncPassCRC = 0;

		if (0 == pBlock->m_fYEncCorrectSize)
			m_fYEncCorrectSize = 0;
	}

	if (m_BlockList.IsEmpty())
		m_BlockList.AddTail ( pBlock );
	else
	{
		POSITION pos = m_BlockList.GetHeadPosition ();
		POSITION prev;
		TCodedBinBlock* pHaystack;
		while (pos)
		{
			prev = pos;
			pHaystack = m_BlockList.GetNext ( pos );

			if (pHaystack->sequence == -1 ||
				pHaystack->sequence >= pBlock->sequence)
			{
				m_BlockList.InsertBefore (prev, pBlock);
				return 0;
			}
		}
		// fell off end
		m_BlockList.AddTail ( pBlock );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
TCodedBinBlock::ECodeType TCodeMgr::ThreadTable (CByteArray & dest, LPCTSTR ident)
{
	TCodedThread* pThread = m_seq.FindThread ( ident );

	if (0 == pThread)          // no thread by that name
		return (TCodedBinBlock::CODE_UNKNOWN);
	else
	{
		if (pThread->contentEncoding == TCodedBinBlock::CODE_CUSTOM)
		{
			int  n = sizeof (pThread->customTable) / sizeof(pThread->customTable[0]);
			for (int j = 0; j < n; j++)
				dest[j] = pThread->customTable[j];
		}
		return (pThread->contentEncoding);
	}
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
TCodedThread* TCodedSequencer::FindThread (LPCTSTR ident)
{
	POSITION pos;
	pos = m_threadList.GetHeadPosition();
	TCodedThread* pThread;

	while (pos)
	{
		pThread = m_threadList.GetNext ( pos );

		if (0 == pThread->ident.CompareNoCase (ident))
		{
			return pThread;
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------------
*    Encode a file to file, net, or textblock
*  Assumes currentCoded is initialized
*    startByte is file byte to start at (0 == beginning)
*    maxBytes is # encoded bytes to generate before stopping (0 == no restriction)
*    AddLine is a pointer to a function which will be used to deal with each
*      encoded line appropriately (i.e. add to file, send to net...)
* returns # bytes read from file or 0 on failure
*/

/////////////////////////////////////////////////////////////////////
//
// takes an open Cfile (the input data)
//
// puts encoded bytes into a CMemFile
//
unsigned long
TCodeMgr::Encode (
				  TCodedBinBlock::ECodeType kEncodingType,
				  CFile&              fl,
				  const CString &     fileName,
				  unsigned long       startByte,
				  unsigned long       maxBytes,
				  CFile&              outFile,
				  unsigned long&      numBytes,
				  unsigned long&      numLines
				  )
{
	int numRead, lineLen, start;
	register int i;
	BYTE inBuf[ENCODE_LINE_LEN];
	char outLine[MAXINTERNALLINE];
	DWORD totalNumAdded, totalNumRead;
	BOOL done;
	int len;

	totalNumAdded = totalNumRead = 0;

	// Set up encoding map
	switch (kEncodingType)
	{
	case TCodedBinBlock::CODE_BASE64:
		m_encodingTable = base64Table;
		break;
	case TCodedBinBlock::CODE_UU:
		m_encodingTable = uuTable;
		break;
	case TCodedBinBlock::CODE_XX:
		//  Note, XX table is hard coded in declarations for simplicity
		m_encodingTable = xxTable;
		break;
	}

	// Prepare for encoding
	switch (kEncodingType)
	{
	case TCodedBinBlock::CODE_UU:
	case TCodedBinBlock::CODE_XX:
	case TCodedBinBlock::CODE_CUSTOM:
		// these are 3-to-4 encodings with 1st char indicating line len
		// and block starts with 'begin' line and ends with 'end' line
		// add 'begin' line
		if (startByte == 0)
		{
			CString str;
			str.Format ("begin 755 %s\r\n", LPCTSTR(fileName));
			len = str.GetLength();
			outFile.Write (str, len);

			totalNumAdded += len;
			numBytes += len;
			numLines++;
		}
		// set 1st char to appropriate line length value
		outLine[0] = m_encodingTable[ENCODE_LINE_LEN];
		start = 1;              // count includes the count char itself
		break;

	case TCodedBinBlock::CODE_BASE64:
		// base64 is a 3-to-4 encoding, with no line len indicator and
		// no 'begin' or 'end' lines
		start = 0;
		break;

	case TCodedBinBlock::CODE_QP:
		start = 0;
		break;
	}
	lineLen = ENCODE_LINE_LEN;

	// seek the file and have at it
	fl.Seek(startByte, CFile::begin);

	done = FALSE;
	while (!done && (!maxBytes || totalNumAdded < maxBytes))
	{
		numRead = fl.Read (inBuf, lineLen);
		if (numRead < lineLen)    // change last line length value
		{
			if (kEncodingType == TCodedBinBlock::CODE_UU ||
				kEncodingType == TCodedBinBlock::CODE_XX)
			{
				outLine[0] = m_encodingTable[numRead];
			}
			for (i = numRead; i < lineLen; i++)
			{
				inBuf[i] = 0;
			}
			done = TRUE;
		}
		EncodeLine (outLine, inBuf, start, numRead, kEncodingType);
		strcat (outLine, "\r\n");
		len = strlen (outLine);
		outFile.Write (outLine, len);
		totalNumAdded += len;
		numLines++;
		numBytes += numRead;
		totalNumRead += numRead;

		//if (currentCoded->numLines % STATUS_UPDATE_FREQ == 0)
		//   UpdateBlockStatus ();
	}

	if (done)
	{
		switch (kEncodingType)
		{
		case TCodedBinBlock::CODE_UU:
		case TCodedBinBlock::CODE_XX:
		case TCodedBinBlock::CODE_CUSTOM:
			// add zero length line
			outLine[0] = m_encodingTable[0];
			outLine[1] = '\r';
			outLine[2] = '\n';
			outLine[3] = '\0';
			outFile.Write (outLine, 3);
			outFile.Write ("end\r\n",5);
			numLines += 2;
		}
	}
	//UpdateBlockStatus ();

	return (totalNumRead);
}

/* ------------------------------------------------------------------------
*    Encode num chars from line
*  Put data in outLine starting at start
*/
unsigned long
TCodeMgr::EncodeLine (
					  LPTSTR outLine,
					  unsigned char *line,
					  int start,
					  int num,
					  TCodedBinBlock::ECodeType kEncodingType)
{
	register int i, j;

	for (j = start, i = 0; i < num; j += 4, i += 3)
		EncodeUnit (&outLine[j],
		&line[i],
		(i + 3 > num) ? num - i : 3,
		kEncodingType);

	outLine[j] = '\0';
	return (j);
}

/////////////////////////////////////////////////////////////////////
// encode 3-octets into 4 (6bit) chars
void TCodeMgr::
EncodeUnit (
			LPTSTR out,
			unsigned char *in,
			int num,
			TCodedBinBlock::ECodeType kEncodingType)
{
	out[0] = m_encodingTable[((in[0] >> 2) & 63)];
	out[1] = m_encodingTable[(((in[0] << 4) | (in[1] >> 4)) & 63)];
	if (num == 1)
	{
		if (kEncodingType == TCodedBinBlock::CODE_BASE64)
		{
			strcpy (&out[2], "==");
		}
		else
		{
			out[2] = 0;
		}
		return;
	}

	out[2] = m_encodingTable[(((in[1] << 2) | (in[2] >> 6)) & 63)];
	if (num == 2)
	{
		if (kEncodingType == TCodedBinBlock::CODE_BASE64)
		{
			strcpy (&out[3], "=");
		}
		else
		{
			out[3] = 0;
		}
		return;
	}

	out[3] = m_encodingTable[(in[2] & 63)];
}

void TCodeMgr::SetDestinationDirectory (CString& rString)
{
	m_destinationDir = rString;
}

const CString& TCodeMgr::GetDestinationDirectory (void)
{
	return m_destinationDir;
}

void TCodeMgr::SetContentEncoding(TCodedBinBlock::ECodeType kEncodingType)
{
	thisContentEncoding = kEncodingType;
}

///////////////////////////////////////////////////////////////////
//
//
BOOL
TCodeMgr::Decode_QuotedPrintable_to_binary (istream& in_strm,
											unsigned long srcl,
											ostream& out_strm,
											unsigned long *len,
											BOOL * pfIOError)
{
	BOOL fRet = TRUE;
	streampos strm_pos;
	char c;
	while (in_strm.read(&c,1)) /* until run out of characters */
	{
		switch (c)     /* what type of character is it? */
		{
		case '=':         /* quoting character */
			in_strm.read (&c,1);
			switch ( c )  /* what does it quote? */
			{
			case '\0':     /* end of data */
				in_strm.putback (c);  /* back up pointer */
				break;
			case '\015':      /* non-significant line break */
				{
					char peekr;
					if ('\012' == in_strm.peek())
						in_strm.read(&peekr,1);
					break;
				}

			default:       /* two hex digits then */
				{
					BYTE e;
					if (!isxdigit (c))   /* must be hex! */
						return FALSE;

					if (isdigit (c))
						e = c - '0';
					else
						e = c - (isupper (c) ? 'A' - 10 : 'a' - 10);
					in_strm.read(&c,1);  /* snarf next character */
					if (!isxdigit (c))   /* must be hex! */
						return FALSE;

					if (isdigit (c))
						c -= '0';
					else
						c -= (isupper (c) ? 'A' - 10 : 'a' - 10);
					out_strm << BYTE(c + (e << 4)); /* merge the two hex digits */
					/* note point of non-space */
					strm_pos = out_strm.tellp();
					break;
				}
			}
			break;
		case ' ':        /* space, possibly bogus */
			out_strm << c;    /* stash the space but don't update s */
			break;
		case '\015':     /* end of line */
			out_strm.seekp ( strm_pos ); /* slide back to last non-space, drop in */
		default:
			out_strm << c;  /* stash the character */
			strm_pos = out_strm.tellp();
		}
	}

	out_strm.flush();
	if (out_strm.fail())
	{
		*pfIOError = TRUE;
		fRet = FALSE;
	}
	else
		(*len) = (ULONG) out_strm.tellp();

	return fRet;
} // hacked version

/*********************************************************************
* Convert BASE64 contents to binary
* Used by:  decoding a MIME body-part
* Accepts: source
*         length of source
*         pointer to return destination length
* Returns: destination as binary
*/
BOOL
TCodeMgr::Decode_base64_to_binary(
								  istream& in_stream,
								  unsigned long srcl,
								  ostream& out_stream0,
								  unsigned long* uResultLen
								  )
{
	BYTE three[3];
	char c;
	short e = 0;
	streampos strm_pos = out_stream0.tellp();

	bofstream out_stream(out_stream0);

	while (in_stream.read(&c,1))   /* until run out of characters */
	{                            /* simple-minded decode */
		if (isupper (c))
			c -= 'A';
		else if (islower (c))
			c -= 'a' - 26;
		else if (isdigit (c))
			c -= '0' - 52;
		else if (c == '+')
			c = 62;
		else if (c == '/')
			c = 63;
		else if (c == '=')          /* padding */
		{
			//  'xx==' decodes to 8 bits
			//  'xxx=' decodes to 16 bits
			//  'xxxx' decodes to 24 bits
			//  'x===' can't happen

			switch (e++)              /* check quantum position */
			{
			case 2:
				// this is ok
				if (in_stream.peek() != '=')
					return FALSE;
				break;
			case 3:
				// this is ok
				e = 0;
				break;
			default:                  /* impossible quantum position */
				return FALSE;
			}
			continue;
		}
		else
			continue;              /* junk character */
		switch (e++)              /* install based on quantum position */
		{
		case 0:
			three[0] = c << 2;        /* byte 1: high 6 bits */
			break;
		case 1:
			three[0] |= c >> 4;       /* byte 1: low 2 bits */
			out_stream.putc (three[0]);
			three[1] =  c << 4;       /* byte 2: high 4 bits */
			break;
		case 2:
			three[1] |= c >> 2;       /* byte 2: low 4 bits */
			out_stream.putc (three[1]);
			three[2] =  c << 6;       /* byte 3: high 2 bits */
			break;
		case 3:
			three[2] |= c;            /* byte 3: low 6 bits */
			out_stream.putc (three[2]);
			e = 0;                    /* reinitialize mechanism */
			break;
		}
	}
	out_stream.flush();

	*uResultLen = ULONG(out_stream0.tellp() - strm_pos);

	return TRUE;
}

// ------------------------------------------------------------------------
// Returns length of the final binary.  The output buffer should be at least
//    as big as the input buffer.
ULONG String_base64_decode (LPTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz)
{
	static TCodeMgr sMgr; // make this static, so we don't have to construct it
	// over and over again
	ULONG uResultLen = 0;

	istrstream in_stream(pInBuf);
	ostrstream out_stream(pOutBuf, nOutSz);

	sMgr.Decode_base64_to_binary (in_stream, nInSz, out_stream, &uResultLen);

	return uResultLen;
}

// ------------------------------------------------------------------------
// String_base64_encode -- created for use with Secure Password Authentication
//
ULONG String_base64_encode (PBYTE pInBuf, DWORD nInSz, CString & strOutput)
{
	// the encode member function likes to operate with the CFile object,
	//   so convert to a memfile
	CMemFile fl1;

	fl1.Write (pInBuf, nInSz);         // load up the memfile
	fl1.SeekToBegin();

	TCodeMgr sMgr;

	ULONG uStart = 0;
	ULONG uMax = 0;
	ULONG uNumBytes = 0;               // shows bytes read from input
	ULONG uNumLines = 0;               // shows # lines created in output

	CMemFile flOutput;

	// encode to base64
	sMgr.Encode (TCodedBinBlock::CODE_BASE64,
		fl1,  // input
		"",   // name of image
		uStart,
		uMax,
		flOutput,  // output file
		uNumBytes,
		uNumLines);

	// convert from a memfile back to a CString
	flOutput.SeekToBegin ();

	int fileLen = int(flOutput.GetLength());
	LPTSTR pStr = strOutput.GetBuffer(fileLen);

	// the 'Encode' member function creates data with CRLF after every 66 bytes
	//   so strip them out here, to get pure base64 data
	for (int i = 0; i < fileLen; i++)
	{
		char c;
		flOutput.Read (&c, 1);

		if (c != '\r' && c != '\n')
			*pStr++ = c;
	}

	*pStr = '\0';     // cap off the string

	strOutput.ReleaseBuffer ();
	return 0;
}

// ------------------------------------------------------------------------
// Returns length of the final binary.  The output buffer should be at least
//    as big as the input buffer.
ULONG String_QP_decode (LPCTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz)
{
	ULONG uResultLen = 0;

	{
		istrstream in_stream(pInBuf, nInSz);
		ostrstream out_stream(pOutBuf, nOutSz);
		BOOL   fIOError = FALSE;

		static TCodeMgr sMgr;

		if (sMgr.Decode_QuotedPrintable_to_binary (in_stream, nInSz,
			out_stream,
			&uResultLen,
			&fIOError) == FALSE)
		{
			uResultLen = -1;
		}
	}

	if (uResultLen == -1)
	{
		// RLW - Error decoding
		// Means the text is not actually QP after all.
		// Copy pInBuf to pOutBuf, upto max of nOutSz-1 chars
		uResultLen = nInSz;
		if (uResultLen > nOutSz-1)
			uResultLen = nOutSz-1;
		memcpy_s(pOutBuf, nOutSz, pInBuf, uResultLen);
		pOutBuf[uResultLen] = 0;
	}

	return uResultLen;
}

BOOL TCodeMgr::Decode_UU_to_binary (istream& in_strm,
									unsigned long srcl,
									ostream& out_strm,
									unsigned long *len,
									BOOL * pfIOError)
{
	return Decode_UUXX_to_binary (in_strm, srcl, out_strm, len,
		TCodedBinBlock::CODE_UU, pfIOError);
}

BOOL TCodeMgr::Decode_XX_to_binary (istream& in_strm,
									unsigned long srcl,
									ostream& out_strm,
									unsigned long *len,
									BOOL * pfIOError)
{
	return Decode_UUXX_to_binary (in_strm, srcl, out_strm, len,
		TCodedBinBlock::CODE_XX, pfIOError);
}

BOOL TCodeMgr::Decode_UUXX_to_binary (istream& in_strm,
									  unsigned long srcl,
									  ostream& out_strm,
									  unsigned long *len,
									  TCodedBinBlock::ECodeType eType,
									  BOOL * pfIOError)
{
	BOOL fRet = TRUE;
	const int bufsz = 512;
	TCHAR* buf= new TCHAR[bufsz];
	BOOL   fDataLine;
	try
	{
		*pfIOError = FALSE;

		buf[0] = '\0';
		while (in_strm.getline (buf, bufsz, '\r'))
		{
			if (in_strm.peek() == '\n')
				in_strm.get();

			fDataLine = TRUE;

			// test for empty line
			if ('\0' == buf[0])
			{
				fDataLine = FALSE;
			}

			// look for 'begin' and 'end' lines
			else if ( ('b' == buf[0] || 'B' == buf[0] || 'e' == buf[0] || 'E' == buf[0]))
			{
				if (!TestDataLine(buf, eType))
					fDataLine = FALSE;
			}

			if (fDataLine)
			{
				// decode this line. Send result to callback-func
				int nDecodeRet =
					DecodeDataLine ( buf, eType,
					CallbackStream, DWORD(&out_strm) );
				if (FAIL == nDecodeRet)
				{
					*pfIOError = TRUE;
					fRet = FALSE;
					break;
				}
			}

			// reset line
			buf[0] = '\0';
		} // while loop
	}
	catch(...)
	{
		delete [] buf;
		throw;
	}
	delete [] buf;
	out_strm.flush();
	return fRet;
}

BOOL TCodeMgr::uuencode_histogram(LPCTSTR lpszText, int iLen)
{
	LPCTSTR ptr = lpszText;
	int     i   = 0;
	int     iLongLineCount = 0;
	int     iTotalLineCount = 0;
	int     iThisLineCount = 0;

	while (i < iLen)
	{
		if ('\n' != ptr[i])
		{
			++iThisLineCount;
		}
		else
		{
			if ((iThisLineCount > 58) && (iThisLineCount <= 62))
				iLongLineCount++;
			iTotalLineCount++;

			iThisLineCount = 0;
		}
		++i;
	}

	return (iLongLineCount >= ((9 * iTotalLineCount) / 10));
}

// --------------------------------------------------------------------------
//
//
//
int TCodeMgr::TestFor_yEnc(
						   const CString &   strLine,
						   TCodedBinBlock*   pBlock)
{
	LPTSTR cp;
	TCHAR  szAttname[4096];
	int    iYline;
	int    iYpart;
	int    iYsize;
	int    fStat = FAIL;

	LPCTSTR line = strLine;
	do
	{
		if (strncmp(line,"=ybegin ",8) != 0)
			break;

		cp = (LPSTR)strstr(line,"name=");
		if (NULL == cp)
			break;   // Error - filename not found

		strcpy (szAttname,cp+5);  // Store the filename

		cp = (LPSTR)strstr(line,"size=");
		if (NULL == cp)
			break;   // Error - size not found

		iYsize = atol(cp+5);
		pBlock->m_iYEncBeginLineSize = iYsize;

		cp = (LPSTR)strstr (line, "line=");
		if (NULL == cp)
			break;   // Error - linelength not found

		iYline = atol(cp+5);

		iYpart = 0;

		cp = (LPSTR)strstr(line,"part=");   // Check if this is a multipart message
		if (cp)
		{
			iYpart = atol(cp+5);

			pBlock->sequence = iYpart;

			if (iYpart == thisNumBlocks)
				pBlock->endFlag = TRUE;
		}
		else
		{
			pBlock->sequence = 1;

			if (1 == thisNumBlocks)
				pBlock->endFlag = TRUE;
		}

		// looks valid
		fStat = SUCCESS;

		pBlock->m_fYEncoded = 1;

		bool trimmed = false;
		int len = lstrlen(szAttname);
		if (len > 2)
		{
			if (szAttname[len-2]  == '\r'  &&
				szAttname[len-1]  == '\n')
			{
				len -= 2;
				szAttname[len] = 0;
				trimmed = true;
			}
		}

		// some versions of yPost(ver 0.20) don't follow the CRLF convention. it just has LF
		if (!trimmed)
		{
			if ((len > 1) && ('\n' == szAttname[len-1]))  
			{
				szAttname[len-1] = 0;
			}
		}

		NameWithoutPath (pBlock->name, szAttname);

		SetContentEncoding (TCodedBinBlock::CODE_YENC);

		m_codingState = STATE_DECODE_PROCESSING;

		m_sCRC.Initialize ();

	} while (FALSE);

	return fStat;
}

// --------------------------------------------------------------------------
unsigned long hex_to_ulong(char * text)  // Because strtol() does not deliver 32 bit on my C-Compiler
{
	unsigned long res;
	unsigned char c;

	if (text==NULL) return(-1);

	res=0;
loop:
	c=*text; text++;
	if ((c>='0')&(c<='9'))
	{
		res=(res<<4)+((long)(c-48) & 0x0F);
		goto loop;
	}
	if ((c>='A')&(c<='F'))
	{
		res=(res<<4)+((long)(c-55) & 0x0F);
		goto loop;
	}
	if ((c>='a')&(c<='f'))
	{
		res=(res<<4)+((long)(c-87) & 0x0F);
		goto loop;
	}
	return(res);
}

// --------------------------------------------------------------------------
// Returns  SUCCESS, FAIL, END_BLOCK
//
//
int TCodeMgr::Decode_YEnc_Line (
								CString &                 line,
								CODE_HELPER               pfnHelper,
								TCodedBinBlock*           pBlock )

{
	unsigned char desbuf[4100];
	unsigned char * srcp;
	unsigned char * desp;
	int deslen;
	unsigned char c;
	char * cp;
	long decolen;
	unsigned long included_crc;
	unsigned long calculated_crc;

	PBYTE  srcbuf = 0;

	int slen = line.GetLength();

	if ((slen >= 2)  && ('\r' == line[slen-2]) && ('\n' == line[slen-1]))
	{
		slen -= 2;
	}
	// some versions of yPost(ver 0.20) don't follow the CRLF convention. it just has LF
	else if ((slen >= 1) && ('\n' == line[slen-1]))
	{
		slen --;
	}

	srcbuf = PBYTE(LPCTSTR(line));

	decolen = 0;
	deslen = 0;
	desp = desbuf;

	if (strncmp ((LPCTSTR)srcbuf, "=ypart ", 7) == 0)
	{
		// skip this shit, most of it anyways
		LPTSTR pBegin;
		LPTSTR pEnd;

		// the size marker should always be there
		pBegin = (LPSTR)strstr ((LPCTSTR)srcbuf, "begin=");
		pEnd   = (LPSTR)strstr ((LPCTSTR)srcbuf, "end=");

		if (pBegin && pEnd)
		{
			pBlock->m_iYEncBeginLineSize = atol(pEnd + 4) - atol(pBegin + 6) + 1;
		}

		return SUCCESS;
	}

	if (strncmp((LPCTSTR)srcbuf, "=yend ", 6) == 0)
	{
		// the crc marker is optional
		cp = (LPSTR)strstr((LPCTSTR)srcbuf, "crc32=");
		if (cp)
		{
			included_crc   = hex_to_ulong((char*)(cp+6));         // included CRC
			calculated_crc = m_sCRC.GetResult ();                 // calculated CRC

			if (included_crc == calculated_crc)
				pBlock->m_fYEncPassCRC = 1;   // very good sign
			else
			{
				pBlock->m_fYEncPassCRC = 0;   // bad CRC found
				ASSERT(0);
			}
		}

		// the size marker should always be there
		cp = (LPSTR)strstr ((LPCTSTR)srcbuf, "size=");
		if (cp)
		{
			int endline_size = atol(cp+5);

			if ((pBlock->size() == (DWORD) endline_size) &&
				(pBlock->m_iYEncBeginLineSize == endline_size))
			{
				pBlock->m_fYEncCorrectSize = 1;
			}
			else
			{
				ASSERT(0);  // size mismatch found
			}
		}

		return END_BLOCK;
	}

	srcp = srcbuf;

	c = *srcp++;

	while (--slen >= 0)
	{

		if ('=' == c)
		{
			--slen;
			if (slen < 0)
				return FAIL;   // last char cannot be escape char!

			c = *srcp;  srcp++;

			c = (unsigned char) (c - 64);
		}

		c = (unsigned char) (c - 42);

		*desp = c;
		desp ++;
		deslen ++;
		decolen++;

		// keep running CRC total
		m_sCRC.Add(c);

		// buffer full, write it out
		if (deslen >= 4096)
		{
			if (FAIL == pfnHelper (DWORD(pBlock), desbuf, deslen))
				return (FAIL);

			deslen = 0; desp = desbuf;
		}

		c = *srcp++;
	}  // end while

	// dump out remaining stuff in buffer
	if (deslen > 0)
	{
		if (FAIL == pfnHelper (DWORD(pBlock), desbuf, deslen))
		{
			return FAIL;
		}
	}
	return SUCCESS;      // end of line reached
}

// --------------------------------------------------------------------------
//
BOOL TCodeMgr::Decode_YEnc_to_binary(istream& in_strm,
									 unsigned long srcl,
									 ostream& out_strm,
									 unsigned long *len,
									 BOOL*    pfIOError)
{
	BOOL fRet = TRUE;
	const int bufsz = 2048;
	TCHAR *buf = new TCHAR[bufsz];
	BYTE  c;

	auto_ptr<TCHAR> deleter(buf);

	streampos strm_pos = out_strm.tellp();

	BOOL   fDataLine;

	*pfIOError = FALSE;
	buf[0] = '\0';

	CString strTest;

	while (in_strm.getline (buf, bufsz, '\r'))
	{
		fDataLine = TRUE;

		if (in_strm.peek() == '\n')
		{
			in_strm.get();
			//fDataLine = FALSE;
		}

		// test for empty line
		if ('\0' == buf[0])
			fDataLine = FALSE;

		if (fDataLine)
		{
			if ( (strncmp(buf,"=ybegin ",8) == 0) ||
				(strncmp(buf,"=yend ",6) == 0)   ||
				(strncmp(buf,"=ypart ",7) == 0) )
				fDataLine = FALSE;
		}

		if (fDataLine)
		{
			PBYTE cp = PBYTE(buf);

			while ((c = *cp) != NULL)
			{
				if ('=' == c)
				{
					if (cp[1] != 0)
					{
						c = *(++cp);

						c = (BYTE) (c - 64);
					}
					else
					{
						cp++;
						continue;
					}
				}

				c = (BYTE) (c - 42);

				out_strm << c;
				cp++;
			}
		}

		// reset line
		buf[0] = '\0';

	} // while

	out_strm.flush();
	if (out_strm.fail())
	{
		*pfIOError = TRUE;
		fRet = FALSE;
	}
	else
		(*len) = ULONG(out_strm.tellp() - strm_pos);

	return fRet;
}
