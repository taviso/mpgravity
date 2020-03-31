/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: usrdisp.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:24  richard_wood
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

class TStatusWnd;

typedef struct
{
	BOOL m_fPrio;
	int  m_iLow;
	int  m_iHigh;
	int  m_iCurrent;
	CString m_text;
	CString m_uiStatus;
	CPoint  m_ptFilter;
} USRDISP_INFO, * LPUSRDISP_INFO;

// automatic object
class TUserDisplay_Auto {
public:
	enum EAction   { /*kExit, */ kClearDisplay };
	enum EPriority { kNormal, kPriority };

	TUserDisplay_Auto(EAction   eAction,
		EPriority ePriority);

	~TUserDisplay_Auto();

private:
	TUserDisplay_Auto::EAction   m_eAction;
	TUserDisplay_Auto::EPriority m_ePriority;
};

// -------------------------------------------------------
// another automatic object
class TUserDisplay_UIPane
{
public:
	TUserDisplay_UIPane(const CString & status);
	~TUserDisplay_UIPane();
};

// another automatic object
class TUserDisplay_Reset
{
public:
	TUserDisplay_Reset(const CString & status, BOOL fPrio);
	~TUserDisplay_Reset();

private:
	BOOL    m_fPrio;
	CString m_str;
};

/**************************************************************************
A wrapper around Status Bar updates
**************************************************************************/
class TUserDisplay : public CObject {
	friend TUserDisplay_Auto;

protected:
	typedef struct {
		int m_iLow;
		int m_iHigh;
		int m_iCurrent;
		int m_iLowBound;
		CString m_text;

		void reset ()
		{
			m_iLow = m_iLowBound = m_iCurrent = 0;
			m_iHigh = 5;
			m_text.Empty();
		}
		void setRange (int low, int high)
		{
			m_iCurrent = 0;
			m_iLow = 0;
			m_iLowBound = low;
			m_iHigh = high - low;
		}
		void setPos (int pos)
		{
			m_iCurrent = pos - m_iLowBound;
		}
	} UD_STATE;

public:
	TUserDisplay(void);
	~TUserDisplay(void);
	void SetWindow(HWND hwndMainFrame, HWND hWndStatus);

	// setting the text is independent of adjusting the progress
	void SetText(const CString& rText, BOOL fPriority = FALSE);
	void SetText(UINT stringID, BOOL fPriority = FALSE);

	void GetText (CString & strOut, BOOL fPriority = FALSE);

	// set range
	void SetRange(int low, int high, BOOL fPriority = FALSE);

	// update progress ctrl
	void StepIt (BOOL fDraw = TRUE, BOOL fPriority = FALSE, int incr = 1);
	void SetPos (int pos, BOOL fDraw = TRUE, BOOL fPriority = FALSE);

	void AutoRefresh(BOOL fOn);
	BOOL IsAutoRefresh();

	BOOL GetAllCurrentInfo (LPUSRDISP_INFO pDI);

	// functions to control cursor in FrameWindow
	// specifically the 'IDC_APP_STARTING' cursor

	void AddActiveCursor();
	void RemoveActiveCursor();

	// --- for Pane 0
	void SetUIStatus   (const CString & rText);
	void ClearUIStatus (void);

	// ------ for Pane 1
	void SetCountFilter (int iCountFilter);
	void SetCountTotal  (int iCountTotal);

protected:
	// end of activity - clear display
	void Done(BOOL fPrio = FALSE);

private:
	void SendToStatuswnd (UINT message, WPARAM wDraw);

protected:
	void FlushMessages(UINT msg, WPARAM wP, LPARAM lP);

	UD_STATE m_sNorm;
	UD_STATE m_sPrio;

	BYTE m_iAutoRefreshStack;

	BYTE m_fShowPrioString;
	CRITICAL_SECTION m_critSect;

	HWND   m_hWndStatus;
	HWND   m_hwndMainFrame;
	int    m_iCursorCount;

	CString m_strUIStatus;

	CPoint  m_ptFilter;
};
