/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: coding.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.3  2009/01/28 14:53:37  richard_wood
/*  Tidying up formatting
/*
/*  Revision 1.2  2008/09/19 14:51:15  richard_wood
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

// coding.h
//
// UUEncoded Text->Binary bitmap    binar stored in a TCodedBinBlock

#pragma once

#include "declare.h"
#include "mplib.h"
#include "codeevt.h"
#include "memspot.h"
#include "superstl.h"            // istream, ...

#include "crc32.h"

const int  MAXINTERNALLINE = 180;
const int  MAXFILENAME = 256;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class TCodedBinBlock {
public:
   enum ECodeType {
     CODE_NONE    = 0,    // unused
     CODE_BASE64  = 1,
     CODE_UU      = 2,
     CODE_XX      = 3,
     CODE_CUSTOM  = 4,
     CODE_QP      = 5,    // unused
     CODE_UNKNOWN = 6,    // unused
     CODE_YENC    = 7
   } ;

   TCodedBinBlock();
   ~TCodedBinBlock();

   BOOL AddData (BYTE* data, int dataLen);
   void ClearData (void);
   DWORD size();

   int Write (CArchive& ar);
   int Write (CFile& fl);

public:
   CString  name;          // original file name from begin line
   CString  ident;         // based on article subject line + author
   BOOL     beginFlag;     // TRUE this block is begin of the complete binary
   BOOL     endFlag;       // =TRUE this block is end of the complete binary
   int      sequence;      // =-1 if unknown, or # in mult-part file
   int      seqConfidence; // confidence of sequence info
   int      estNumLines;   // info obtained from Lines: header during decode
   unsigned long numLines; // num lines read so far (for status window

   //unsigned long numBytes; // # of bytes used in this coded data block

   HWND hParentWnd;        // handle to group wnd that started this

   BYTE     m_fYEncoded;
   BYTE     m_fYEncPassCRC;
   BYTE     m_fYEncCorrectSize;
   int      m_iYEncBeginLineSize;

private:
   TMemorySpot m_memSpot;
};

class TCodedThread;
class TCodeMgr;

class TCodedSequencer {
public:
   typedef CTypedPtrList<CPtrList, TCodedThread*> ThreadList;
   TCodedSequencer();
   ~TCodedSequencer();

   BOOL AddBlock(TCodedBinBlock* pBlock, int expectedNumBlocks,
                 TCodedBinBlock::ECodeType type,
                 CODE_EVENT_MONITOR pEventMonitor = NULL);
   TCodedThread* FindThread (LPCTSTR ident);
   const CString & GetDestinationDir ();

public:
   ThreadList  m_threadList;
   TCodeMgr	 * m_pMgr;
};

typedef BOOL (*CODE_HELPER) (DWORD dwCookie, BYTE* poutLine, UINT numDecoded);

/////////////////////////////////////////////////////////////////////////////
// TCodeMgr - Manages decoding and encoding.
/////////////////////////////////////////////////////////////////////////////

// the top level dude
class TCodeMgr {
public:

   static BOOL uuencode_histogram(LPCTSTR lpszText, int iLen);

   bool m_fArticleFromCache_NoSKIP;
   TPath m_destinationDir;

   typedef enum {
     STATE_INACTIVE         = 0,
     STATE_DECODE_ACTIVE    = 1,
     STATE_DECODE_SKIPPING  = 2,
     STATE_DECODE_PROCESSING= 3,
     STATE_DECODE_GET_TABLE = 4,
     STATE_ATTACH_PROCESSING= 10,
     STATE_ATTACH_SENDING   = 11,
     STATE_ATTACH_READFILE  = 12,
     STATE_ATTACH_WAITING   = 13
   } ECodeState;

public:
   TCodeMgr();
   ~TCodeMgr();

   // Main entry point
   int  Decode (CString &        strSubjFilename,
                POINT &          sFraction,
                TArticleHeader * pArtHdr,
                TArticleText *   pArtText,
                HWND             hwnd,
                const CString &  strRealSubject,
                int              iAskOverwrite = TRUE);

   int DecodeIdentified(LPCTSTR filename, int iSequence, int iTotal,
      LPCTSTR lpszText, int iLen, int iAskOverwrite = TRUE);

   // Global Func: encoding an attachment
   unsigned long Encode (
      TCodedBinBlock::ECodeType kEncodingType,
      CFile&              fl,
      const CString &     fileName,
      unsigned long       startByte,
      unsigned long       maxBytes,
      CFile&              outFile,
      unsigned long&      numBytes,
      unsigned long&      numLines
   );

   void SetDestinationDirectory (CString& rString);
   const CString& GetDestinationDirectory (void);

   BOOL Decode_base64_to_binary(istream & in_stream,
                                unsigned long srcl,
                                ostream & out_stream,
                                unsigned long* uResultLen);

   BOOL Decode_QuotedPrintable_to_binary (istream& in_strm,
                                          unsigned long srcl,
                                          ostream& out_strm,
                                          unsigned long *len,
                                          BOOL*    pfIOError);

   BOOL Decode_UU_to_binary (istream& in_strm,
                             unsigned long srcl,
                             ostream& out_strm,
                             unsigned long *len,
                             BOOL*    pfIOError);

   BOOL Decode_XX_to_binary (istream& in_strm,
                             unsigned long srcl,
                             ostream& out_strm,
                             unsigned long *len,
                             BOOL*    pfIOError);

   BOOL Decode_YEnc_to_binary(istream& in_strm,
                             unsigned long srcl,
                             ostream& out_strm,
                             unsigned long *len,
                             BOOL*    pfIOError);

   BOOL DecodeDataLine (LPTSTR line,
                        TCodedBinBlock::ECodeType eCode,
                        CODE_HELPER pfnHelper,
                        DWORD dwUnknown);

   CByteArray &  GetCodingMap(TCodedBinBlock::ECodeType kType);

   BOOL TestDataLine(LPTSTR line, TCodedBinBlock::ECodeType eCode);

   BOOL  InstallEventMonitor (CODE_EVENT_MONITOR   pEventMonitor);

   int m_iAskOverwrite;

protected:
   void InitializeMaps();
   void InitializeVars(TCodedBinBlock::ECodeType kPrevCodeType);
   int  CreateCodingMap (CByteArray & map, LPCTSTR table);
   int  CreateCodingMap2 (CByteArray & map, CByteArray & table);
   int  GetLine(LPCTSTR str, int len, int& pos, CString& line);

   int  DecodeRun (TCodedBinBlock* pBlock, const CString* pText);

   int  DecodeLine     (CString & line, TCodedBinBlock* pBlock);
   int  Decode_Skipping(CString & line, TCodedBinBlock* pBlock);
   int  Decode_Skipping2(LPTSTR pszLine, TCodedBinBlock* pBlock);
   int  Decode_GetTable(LPTSTR line, TCodedBinBlock* pBlock);
   int  Decode_Processing(CString & line, TCodedBinBlock* pBlock);

   int  CompleteDecodeBlock (TCodedBinBlock* pBlock);
   void DecodeDone ();

   BOOL Decode_UUXX_to_binary (istream& in_strm,
                               unsigned long srcl,
                               ostream& out_strm,
                               unsigned long *len,
                               TCodedBinBlock::ECodeType eType,
                               BOOL * pfIOError);

   void NameWithoutPath(CString& dest, const CString& src);

   void ParseInfoLine (TCodedBinBlock* pBlock, LPTSTR line, BOOL fGuessIdentity);
   BOOL ParseAppSpecificLine (TCodedBinBlock* pBlock, LPTSTR line);
   BOOL IsDataLine(LPTSTR line, TCodedBinBlock* pBlock);

   BOOL ParseMIMEHeader (LPTSTR line, TCodedBinBlock* pBlock);

   BOOL GetPossiblyQuotedStr (char *dest, LPTSTR src, int maxLen);
   BOOL GetNumber (int *dest, LPTSTR src);
   BOOL SkipToNextClause (LPTSTR* ptr);
   BOOL SkipSpace (LPTSTR* ptr);
   TCodedBinBlock::ECodeType ThreadTable (CByteArray & dest, LPCTSTR ident);

   TCodedBinBlock* AllocNewBlock(void);
   int DecodeOneLine (CString& line, TCodedBinBlock* pBlock);

   // Two more global funcs
   void EncodeUnit (LPTSTR out, unsigned char *in, int num, TCodedBinBlock::ECodeType kEncodingType);
   unsigned long EncodeLine (LPTSTR outLine, unsigned char *line,
                             int start, int num, TCodedBinBlock::ECodeType kEncodingType);

   void SetContentEncoding(TCodedBinBlock::ECodeType kEncodingType);

   int TestFor_yEnc(const CString & strLine, TCodedBinBlock* pBlock);
   int Decode_YEnc_Line (CString & strLine,  CODE_HELPER pfnHelper,  TCodedBinBlock* pBlock);

protected:

   TCodedSequencer      m_seq;

   ECodeState           m_codingState;
   BOOL                 m_UsingMIME;
   int                  m_table_count;
   BOOL                 m_DumbDecode;
   CODE_EVENT_MONITOR   m_pEventMonitor;

   // stuff that used to be static to the class

   LPTSTR               m_encodingTable;
   CByteArray           xxMap;
   CByteArray           uuMap;
   CByteArray           base64Map;
   CByteArray           customMap;
   CByteArray           customTable;

   // MIME info bits
   char thisContentType[80];
   char thisContentDesc[MAXINTERNALLINE];
   char thisBoundary[MAXINTERNALLINE], thisBoundaryEnded[MAXINTERNALLINE];
   TCodedBinBlock::ECodeType thisContentEncoding;
   TCodedBinBlock::ECodeType prevContentEncoding;
   char prevBlockIdent[MAXFILENAME];
   int thisNumBlocks;

   CString              m_strNextLine;

   TObjectCRC  m_sCRC;
};

class TCodedThread {
public:
   typedef CTypedPtrList<CPtrList, TCodedBinBlock*> BlockList;
   TCodedThread(TCodedSequencer *pSequencer);
   ~TCodedThread();
   int AddBlock (TCodedBinBlock* pBlock);
   int Write();
   int WriteSequentialBlocks(BOOL fMemFree, BOOL* pfWroteEndBlock,
      CODE_EVENT_MONITOR pEventMonitor);

public:
  CString  name;                       // original file name from begin line
  CString  ident;                      // thread ID based on article subject line
  CString  dosFileName;                // workable DOS file name
  CString  contentType;                // MIME content type
  CString  contentDesc;                // MIME content description
  char customTable[100];               // custom coding table (if CODE_CUSTOM)
  TCodedBinBlock::ECodeType contentEncoding; // MIME content-transfer-encoding
  int numBlocksWritten;                // number of blocks written to disk, so far
  int mode;                            // 2nd field of begin header, don't use in DOS
  int numBlocks;                       // number of coded blocks used in list
  int expectedNumBlocks;               // 0 if unknown, or a # if found in headers
  unsigned long totalBytes;            // total # of bytes for entire file
  BlockList  m_BlockList;              // list of ptrs to decode objects
  BOOL fFileCreated;                   // created a file yet?
  TCodedSequencer	*pSequencer;
  int m_iAskOverwrite;                 // ask whether to overwrite existing output file?

  BYTE     m_fYEncoded;
  BYTE     m_fYEncPassCRC;
  BYTE     m_fYEncCorrectSize;

private:
  void write_util(CFile& fl);
};

inline const CString & TCodedSequencer::GetDestinationDir () {return m_pMgr->m_destinationDir;}

// this version is LPTSTR oriented, not file oriented.
ULONG String_base64_decode (LPTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz);
ULONG String_base64_encode (PBYTE pInBuf, DWORD nInSz, CString & output);

// this version is LPTSTR oriented, not file oriented.
ULONG String_QP_decode (LPCTSTR pInBuf, int nInSz, LPTSTR pOutBuf, int nOutSz);

