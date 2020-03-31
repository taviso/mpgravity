/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: sharesem.h,v $
/*  Revision 1.1  2010/07/21 17:14:57  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
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

#pragma once

class TSynchronizable
{
public:
   TSynchronizable ();
   ~TSynchronizable ();

   DWORD ReadLock (DWORD dwTimeout = INFINITE);
   void UnlockRead ();
   DWORD WriteLock (DWORD dwTimeout = INFINITE);
   void UnlockWrite ();

   int NumReaders () { return m_iNumReaders; }
#if defined(_DEBUG)
   int NumWriters () { return m_iNumWriters; }
#endif

private:
   // prevent copying
   TSynchronizable (const TSynchronizable &sem);
   TSynchronizable & operator=(const TSynchronizable &sem);

private:
   void adjust_reader_count (BOOL fIncrement);
   void win95_mutex_bug_workaround (BOOL fIncrement);

   int      m_iNumReaders;

   CRITICAL_SECTION csReaders;
   HANDLE           m_hEventNoReaders;
   HANDLE           m_hMutexExclusive;

#if defined(_DEBUG)
   DWORD    m_dwViewThreadId;
   int      m_iNumWriters;
#endif
};

class TShareSemaphore : public TSynchronizable
{
};

class TSyncReadLock
{
public:
   TSyncReadLock(TSynchronizable* pSync)
    : m_pSync(pSync) { m_pSync->ReadLock(); }
   ~TSyncReadLock()  { m_pSync->UnlockRead(); }

private:
   TSynchronizable* m_pSync;
};

class TSyncWriteLock
{
public:
   TSyncWriteLock(TSynchronizable* pSync)
    : m_pSync(pSync) { m_pSync->WriteLock(); }
   ~TSyncWriteLock()  { m_pSync->UnlockWrite(); }

private:
   TSynchronizable* m_pSync;
};

class TShareSemaphoreMgr
{
public:
   enum LockType { kReadLock, kWriteLock };
   TShareSemaphoreMgr (TShareSemaphore* pSem, LockType lockType = TShareSemaphoreMgr::kReadLock)
      : m_pSemaphore(pSem)
      {
      m_lockType = lockType;
      if (kWriteLock == m_lockType)
         m_pSemaphore->WriteLock();
      else
         m_pSemaphore->ReadLock();
      }

   ~TShareSemaphoreMgr ()
      {
      if (kWriteLock == m_lockType)
         m_pSemaphore->UnlockWrite();
      else
         m_pSemaphore->UnlockRead();
      }

private:
   TShareSemaphore* m_pSemaphore;
   LockType m_lockType;
};
