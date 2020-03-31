/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: tprnjob.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:15  richard_wood
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

// tprnjob.cpp -- printing job

#include "stdafx.h"              // standard stuff
#include <winspool.h>            // PRINTER_INFO_2
#include "tprnjob.h"             // this file's prototypes
#include "tprnthrd.h"            // gpsPrintDialog, gpPrintThread
#include "resource.h"            // IDS_*
#include "genutil.h"             // GetOS()
#include "tglobopt.h"            // gpGlobalOptions
#include "rgfont.h"              // TRegFonts
#include "rgui.h"                // TRegUI
#include "rgswit.h"              // TRegSwitch

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// -------------------------------------------------------------------------
// IMPLEMENT_SERIAL -- macro that defines several serialization functions
IMPLEMENT_SERIAL (TPrintJob, PObject, PRINT_JOB_VERSION_NUMBER)

// -------------------------------------------------------------------------
// TPrintJob -- default constructor
TPrintJob::TPrintJob () : TUtilityJob (PRINT_JOB_VERSION_NUMBER)
{
   m_hDC = 0;
}

// -------------------------------------------------------------------------
// ~TPrintJob -- default destructor
TPrintJob::~TPrintJob ()
{
   m_hDC = 0;
}

// -------------------------------------------------------------------------
// TPrintJob -- constructor
TPrintJob::TPrintJob (LONG lGroupID, LPCTSTR pchGroupName,
   LPCTSTR pchGroupNickname, TArticleHeader *psHdr, int iRuleBased, HDC hDC)
   : TUtilityJob (PRINT_JOB_VERSION_NUMBER, lGroupID, pchGroupName,
      pchGroupNickname, psHdr, iRuleBased)
{
   m_strJobType.LoadString (IDS_PRINT_STRING);
   m_hDC = hDC;
}

// -------------------------------------------------------------------------
static void OutputLine (CDC &sDC, int iLeft, int iRight, int iTop,
   int iLineHeight, int &iLineNumber, LPCTSTR pchLine)
{
   int iPaperWidth = sDC.GetDeviceCaps (HORZRES) - iRight - iLeft;
   // make sure it's not set too low
   if (iPaperWidth < 300)
      iPaperWidth = 300;

   int iCharsPrinted = 0;
   int iOriginalLineLength = strlen (pchLine);

   // print portions of the line until the whole line is printed
   do {
      CString strPortion = pchLine;

      // find the portion that will print within the margins
      int iPos = strPortion.GetLength ();
      CSize sSize = sDC.GetTextExtent (strPortion, iPos);
      int iLineWidth = sSize.cx;
      int iEscape = 0;
      while (iLineWidth > iPaperWidth && iEscape ++ < 100) {

         // work backward and stop when a space is found
         do {
            iPos --;
            } while (iPos > 0 && strPortion [iPos] != ' ');

         sSize = sDC.GetTextExtent (strPortion, iPos);
         iLineWidth = sSize.cx;
         }

      // if iPos is zero, there are no spaces
      if (!iPos)
         iPos = strPortion.GetLength ();

      // go forward and count the spaces as part of this line
      while (iPos < strPortion.GetLength () && strPortion [iPos] == ' ')
         iPos ++;

      strPortion = strPortion.Left (iPos);
      pchLine = &pchLine [0] + iPos;
      iCharsPrinted += iPos;

      sDC.TextOut (iLeft, iTop + iLineHeight * iLineNumber ++, strPortion,
         strPortion.GetLength ());
      } while (iCharsPrinted < iOriginalLineLength);
}

// -------------------------------------------------------------------------
// PrintPage -- prints one page. Returns zero when the last page has been
// printed
static int PrintPage (CDC &sDC, int iLinesPerPage, HANDLE hPrintFile,
   int iLineHeight, int iRight, int iLeft, int iTop, BOOL bPrintBar)
{
   int iLineNumber = 0;
   int iFileFinished = FALSE;
   BOOL fSuccess;
   int i;

   if (bPrintBar) {
      // print header bar
      COLORREF clrOld = sDC.GetBkColor ();
      int iPaperWidth = sDC.GetDeviceCaps (HORZRES) - iRight - iLeft;
      sDC.FillSolidRect (iLeft, iTop, iPaperWidth, 15 /* pixels high */,
         0 /* black */);
      sDC.SetBkColor (clrOld);
      }

   // print lines from the file until the end of this page is reached
   do {

      // read a line from the file... keep reading until we hit a linefeed or eof
      char rchLine [256];
      int iLinePos = 0;
      DWORD dwCharsRead;
      char ch;
      do {
         fSuccess = ReadFile (hPrintFile, &ch, 1, &dwCharsRead, NULL);

         if (dwCharsRead != 1 || !fSuccess) {
            iFileFinished = TRUE;
            break;
            }

         switch (ch) {
            case '\n':
            case '\r':
               break;   // linefeeds and carriage returns don't go to the line
            case '\t':
               // tabs get expanded into 8 chars
               for (i = 0; i < 8; i++)
                  if (iLinePos < sizeof (rchLine) - 1)
                     rchLine [iLinePos++] = ' ';
               break;
            default:
               // regular character (presumably)
               if (iLinePos < sizeof (rchLine) - 1)
                  rchLine [iLinePos++] = ch;
               break;
            }
         } while (ch != '\n');
      rchLine [iLinePos] = '\0';

      OutputLine (sDC, iLeft, iRight, iTop + (bPrintBar ? 50 : 0),
         iLineHeight, iLineNumber, rchLine);

      } while (iLineNumber < iLinesPerPage && !iFileFinished);

   return !iFileFinished;
}

// -------------------------------------------------------------------------
// DoYourJob -- prints a job's article
//
// return true to re-queue the job.  We always return false
//
bool TPrintJob::DoYourJob (HANDLE hKillEvent)
{
   HANDLE hPrintFile = INVALID_HANDLE_VALUE;
   char rchTempPath [MAX_PATH + 1];
   char rchTempFile [MAX_PATH + 1];
   int iLinesPerPage;
   int iLineHeight;
   int iStartedDoc = FALSE;
   int iKeepPrinting = TRUE;
   int cHeightPels;
   DWORD dwPathLength;
   CString strJobName;
   CString str;
   CFont sFont;

   SetStatus (IDS_PRINT_OPEN_TEMP);

   CDC sDC;
   sDC.Attach (m_hDC);

   rchTempFile [0] = '\0';
   rchTempPath [0] = '\0';

   // save the article to a temporary file
   dwPathLength = GetTempPath (sizeof (rchTempPath) - 1, rchTempPath);
   if (!dwPathLength || dwPathLength > (sizeof (rchTempPath) - 1)) {
      str.LoadString (IDS_ERR_TEMP_FILE_DIR);
		SetStatus (str, ERROR_STATUS);
      goto end;
      }

   if (!GetTempFileName (rchTempPath, "", 0, rchTempFile)) {
      str.LoadString (IDS_ERR_TEMP_FILE);
		SetStatus (str, ERROR_STATUS);
      goto end;
      }

   if (WriteArticleToFile (rchTempFile, hKillEvent))
      // error message has been displayed by WriteArticleToFile()
      goto end;

   SetStatus (IDS_PRINT_STARTING);

   hPrintFile = CreateFile (rchTempFile, GENERIC_READ | GENERIC_WRITE, 0,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY, 0);
   if (hPrintFile == INVALID_HANDLE_VALUE) {
		SetStatus (IDS_ERR_OPEN_TEMP, ERROR_STATUS);
      goto end;
      }

   // select the right font
   TRegFonts *pFonts;
   pFonts = gpGlobalOptions -> GetRegFonts ();
   LOGFONT sLogFont;
   const LOGFONT *pLogFont;
   pLogFont = pFonts -> GetPrintFont ();
   memcpy (&sLogFont, pLogFont, sizeof LOGFONT);
   COLORREF iColor;
   iColor = (COLORREF) pFonts -> GetPrintTextColor ();
   int iPoints;
   iPoints = pFonts -> GetPrintPointSize ();

   HFONT hOldFont;
   hOldFont = 0;
   sLogFont.lfHeight = -MulDiv (iPoints, GetDeviceCaps (m_hDC, LOGPIXELSY), 72);
   if (sFont.CreateFontIndirect (&sLogFont))
      hOldFont = (HFONT) SelectObject (m_hDC, (HFONT) sFont);

   // print the file
   strJobName.Format ("%s (%s)", m_sHdr.GetSubject (), m_strGroupName);
   // one DOCINFO help page (the mfc one) said the job name must not be more
   // than 32 chars including the NULL terminator
   if (strJobName.GetLength () > 27)
      strJobName = strJobName.Left (27) + "...";

   DOCINFO di;
   memset (&di, 0, sizeof (DOCINFO));
   di.cbSize = sizeof (DOCINFO);
   di.lpszDocName = strJobName;
   di.lpszOutput = (LPTSTR) NULL;
   di.lpszDatatype = "";
   if (StartDoc (m_hDC, &di) == SP_ERROR) {
      dwPathLength = GetLastError();
		SetStatus (IDS_ERR_PRINT_JOB, ERROR_STATUS);
      goto end;
      }
   iStartedDoc = TRUE;

   // get margins
   int iTop, iBottom, iRight, iLeft;
   TRegUI *pRegUI;
   pRegUI = gpGlobalOptions -> GetRegUI ();
   pRegUI -> GetPrintMargins (iLeft, iRight, iTop, iBottom);
   // now the margins are in hundredths of inches
   int iHorizPixelsPerInch; iHorizPixelsPerInch = sDC.GetDeviceCaps (LOGPIXELSX);
   int iVertPixelsPerInch; iVertPixelsPerInch = sDC.GetDeviceCaps (LOGPIXELSY);
   iTop = iTop * iHorizPixelsPerInch / 100;
   iBottom = iBottom * iHorizPixelsPerInch / 100;
   iLeft = iLeft * iHorizPixelsPerInch / 100;
   iRight = iRight * iHorizPixelsPerInch / 100;

   // get lines-per-page
   TEXTMETRIC sMetrics;
   GetTextMetrics (m_hDC, &sMetrics);
   iLineHeight = sMetrics.tmHeight + sMetrics.tmInternalLeading +
      sMetrics.tmExternalLeading;
   cHeightPels = GetDeviceCaps (m_hDC, VERTRES);
   cHeightPels -= (iTop + iBottom);
   iLinesPerPage = cHeightPels / iLineHeight;

   SetStatus (IDS_PRINTING);

   BOOL bFirstPage;
   bFirstPage = TRUE;

   // keep printing pages until PrintPage() returns nonzero
   do {
      if (StartPage (m_hDC) <= 0) {
		   SetStatus (IDS_ERR_PRINT_PAGE, ERROR_STATUS);
         goto end;
         }

      sDC.SetTextColor (iColor);
      sDC.SelectObject (&sFont);

      iKeepPrinting = PrintPage (sDC, iLinesPerPage, hPrintFile, iLineHeight,
         iRight, iLeft, iTop, bFirstPage);

      if (EndPage (m_hDC) <= 0) {
		   SetStatus (IDS_ERR_PRINT_PAGE, ERROR_STATUS);
         goto end;
         }

      bFirstPage = FALSE;

      } while (iKeepPrinting && !gpPrintThread -> m_bImBeingKilled);

   SetStatus (IDS_UTIL_OK);

end:

   // close and delete the temporary file
   if (iStartedDoc)
      EndDoc (m_hDC);
   if (hPrintFile != INVALID_HANDLE_VALUE)
      CloseHandle (hPrintFile);
   if (rchTempFile [0])
      DeleteFile (rchTempFile);

   // free our font
   if (hOldFont) {
      SelectObject (m_hDC, hOldFont);
      sFont.DeleteObject ();
      }

   sDC.Detach ();

   return false;
}

// -------------------------------------------------------------------------
// CopyVisualFields -- copies fields that appear in the decode-jobs dialog
void TPrintJob::CopyVisualFields (TPrintJob *pSource)
{
   m_strGroupName = pSource -> m_strGroupName;
   m_strGroupNickname = pSource -> m_strGroupNickname;
   m_strStatus = pSource -> m_strStatus;
   m_sHdr.SetSubject (pSource -> m_sHdr.GetSubject());
}

// -------------------------------------------------------------------------
// MonitorDialog -- returns a dialog that is monitoring us, or NULL
CDialog *TPrintJob::MonitorDialog ()
{
   return gpsPrintDialog;
}

// -------------------------------------------------------------------------
// LockDialogPtr -- get exclusive access to critical section that protects
//     gpsPrintDialog
void TPrintJob::LockDialogPtr(BOOL fLock)
{
   if (fLock)
      EnterCriticalSection (&gcritPrintDialog);
   else
      LeaveCriticalSection (&gcritPrintDialog);
}

// -------------------------------------------------------------------------
void TPrintJob::Serialize (CArchive& sArchive)
{
   PObject::Serialize (sArchive);
   TUtilityJob::Serialize (sArchive);
}
