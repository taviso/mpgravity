/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: sharesem.cpp,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.2  2009/08/25 20:04:25  richard_wood
/*  Updates for 2.9.9
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:51  richard_wood
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
#include <stdlib.h>
#include "sharesem.h"
#include "mplib.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// disable warning about CException has no copy-ctor.  When throwing
//   a TException, you must catch a pointer or a reference.
#pragma warning( disable : 4671 4673 )

/////////////////////////////////////////////////////////////////////////////
// TSynchronizable Contructor - Contruct a shared semaphore object.
//
// Current Behavior:
//   If same thread gets ReadLock, then WriteLock, thread STOPS
//         If same thread gets WriteLock, then attempts ReadLock, it continues
//
//  Notes this works:
//  Two threads 'Bob' 'Jane'
//     Bob gets read lock
//                              Jane waits on write lock
//     Bob gets read lock #2
//     Bob release
//     Bob release

/****************************************************************************
 The Win95 Mutex Bug:
    hEvent begins as non-signalled

    Thread A - locks hMutex
    Thread A - locks hMutex
                                 Thread B - waits on hEvent AND hMutex
    Thread A - Sets hEvent
                                 Thread B - Gets !! both!!

 Explanation:
    Each mutex has internal count of 1.  Each WaitForSingleObject call
    decreases it by 1; it becomes -1.  When the Event is signalled, the
    WaitForMultipleObjects call knows the hEvent is OK, and checks the
    hMutex.  There is a bug : the code assumes that if the internal
    count is !zero that the mutex is signalled.

 --------------------------------------
 Scenario #1
 With TShareSemaphore & the Win95bug this sequence is works basically OK

     Thread A - ReadLock
     Thread A - ReadLock
                                   Thread B - attempts WriteLock
     Thread A - UnlockRead
     Thread A - UnlockRead
 --------------------------------------
 Scenario #2
 With TShareSemaphore & the Win95bug this sequence fails miserably
     Thread A - WriteLock
     Thread A - WriteLock
                                   Thread B - attempts WriteLock
     Thread A - ReadLock
     Thread A - UnlockRead
                                   Thread B - Gets WriteLock (!?)

 It's because of this that I've added function win95_mutex_bug_workaround()
****************************************************************************/

TSynchronizable::TSynchronizable ()
{
#if defined(_DEBUG)
   m_iNumWriters = 0;
#endif

   m_hMutexExclusive = m_hEventNoReaders = 0;
   m_iNumReaders = 0;

   m_hMutexExclusive = CreateMutex (NULL, FALSE, NULL);
   if (NULL == m_hMutexExclusive)
      {
      DWORD dwErr = GetLastError ();
      CString pattern; pattern.LoadString (IDS_ERR_SEM_CREATE);
      CString str; str.Format(LPCTSTR(pattern), dwErr);
      throw(new TException (str, kError));
      }

   m_hEventNoReaders = CreateEvent (NULL, TRUE, TRUE, NULL);
   if (NULL == m_hEventNoReaders)
      {
      DWORD dwErr = GetLastError ();

      CloseHandle (m_hMutexExclusive);
      CString pattern; pattern.LoadString (IDS_ERR_SEM_CREATE);
      CString str; str.Format(LPCTSTR(pattern), dwErr);
      throw(new TException (str, kError));
      }

   InitializeCriticalSection ( &csReaders );
}

/////////////////////////////////////////////////////////////////////////////
// TShareSemaphore Destructor
/////////////////////////////////////////////////////////////////////////////

TSynchronizable::~TSynchronizable ()
{
   if (m_hMutexExclusive)
      CloseHandle (m_hMutexExclusive);

   if (m_hEventNoReaders)
      CloseHandle (m_hEventNoReaders);

   DeleteCriticalSection ( &csReaders );
}

/////////////////////////////////////////////////////////////////////////////
// ReadLock - Get non-exclusive read lock
//
DWORD TSynchronizable::ReadLock (DWORD dwTimeout)
{
   DWORD dwRet, dwMyTime;

   if (INFINITE == dwTimeout)
      dwMyTime = 10000;
   else
      dwMyTime = dwTimeout;

   rdlock_top:

   // this blocks if there is an active writer, or a writer waiting for
   //   active readers to exit
   dwRet = WaitForSingleObject (m_hMutexExclusive, dwMyTime);
   if (WAIT_TIMEOUT == dwRet)
      {
      if (INFINITE == dwTimeout)
         {
         TRACE0("Help me - I'm stuck!\n");
         goto rdlock_top;
         }
      else
         {
         return dwRet;
         }
      }
   else
      {
      ASSERT (WAIT_OBJECT_0 == dwRet);

      // increment reader count
      adjust_reader_count ( TRUE );

      // release access to exclusive.  This enables other Readers
      // to come through, or helps the next waiting Writer
      VERIFY(ReleaseMutex (m_hMutexExclusive));
      }

   return dwRet;
}

/////////////////////////////////////////////////////////////////////////////
// UnlockRead -
//
void TSynchronizable::UnlockRead ()
{
   // decrement
   adjust_reader_count ( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// adjust the reader count, and change m_hEventNoReaders when needed
void TSynchronizable::adjust_reader_count (BOOL fIncrement)
{
   EnterCriticalSection ( &csReaders );
   if (fIncrement)
      {
      ASSERT(m_iNumReaders >= 0);
      // the first reader toggles the Event off. Says there's an active reader
      if (m_iNumReaders++ == 0)
         ResetEvent ( m_hEventNoReaders );
      }
   else
      {
      ASSERT(m_iNumReaders > 0);
      // the last reader signals the Event. Says the last reader is gone
      if (--m_iNumReaders == 0)
         SetEvent ( m_hEventNoReaders );
      }
   LeaveCriticalSection ( &csReaders );
}

/////////////////////////////////////////////////////////////////////////////
//  Called from WriteLock, UnlockWrite
//    This is implementation specific. The artificial increment essentially
//    'masks out' any changes to m_hEventNoReaders and fixes Scenario #2
//
//   1-8-97  amc Created
void TSynchronizable::win95_mutex_bug_workaround (BOOL fIncrement)
{
   EnterCriticalSection ( &csReaders );

   if (fIncrement)
      {
      ASSERT(m_iNumReaders >= 0);
      ++m_iNumReaders;
      }
   else
      {
      ASSERT(m_iNumReaders > 0);
      --m_iNumReaders;
      }

   LeaveCriticalSection ( &csReaders );
}

/////////////////////////////////////////////////////////////////////////////
// WriteLock - Gets exclusive write access
//
DWORD TSynchronizable::WriteLock (DWORD  dwTimeout)
{
   HANDLE rHandles[2];

   DWORD dwRet = 0;
   rHandles[0] = m_hMutexExclusive;
   rHandles[1] = m_hEventNoReaders;

   dwRet = WaitForMultipleObjects ( 2, rHandles, TRUE, dwTimeout );

   if (WAIT_FAILED == dwRet)
      {
      ASSERT(0);
      return dwRet;
      }
   else if (WAIT_TIMEOUT == dwRet)
      {
      return dwRet;
      }
   else if ((WAIT_OBJECT_0 + 0) != dwRet && (WAIT_OBJECT_0 + 1) != dwRet)
      {
      ASSERT(0);
      return dwRet;
      }
   else // WAIT_OBJECT_0, or  WAIT_OBJECT_0 +1
      {
      // we have the Write Lock

      // use workaround, increment
      win95_mutex_bug_workaround ( TRUE );

#if defined(_DEBUG)
      ++ m_iNumWriters;

      DWORD dwThreadId = GetCurrentThreadId ();
      if (m_iNumWriters > 1)
         ASSERT(dwThreadId == m_dwViewThreadId);
      m_dwViewThreadId = dwThreadId;
      ASSERT(m_dwViewThreadId);
#endif
      return dwRet;
      }
}

/////////////////////////////////////////////////////////////////////////////
// UnlockWrite
//
void TSynchronizable::UnlockWrite ()
{
#if defined(_DEBUG)
   ASSERT(m_iNumWriters > 0);

   // see who is unlocking us.
   DWORD dwThreadId = GetCurrentThreadId ();

   // should match the thread that Acquired
   //  the write lock
   ASSERT(m_dwViewThreadId == dwThreadId);

   if (0==(m_iNumWriters -1))
      {
      m_dwViewThreadId = 0;
      }

   m_iNumWriters --;

#endif
   // use workaround, decrement
   win95_mutex_bug_workaround ( FALSE );

   VERIFY(ReleaseMutex (m_hMutexExclusive));
}
