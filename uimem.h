/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: uimem.h,v $
/*  Revision 1.1  2010/07/21 17:14:58  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:29  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:52:23  richard_wood
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

#include "mplib.h"
#include "pobject.h"
#include "wndstat.h"

////////////////////////////////////////////////////////////////////////////
//  Class to store things that should be remembered.
//
class TUIMemory {
public:
	TUIMemory();
	~TUIMemory();

	enum EDirType {
		DIR_DECODE_DEST,          // decode destination dir
		DIR_ATTACH_SRC,           // src dir for attachments
		DIR_ARTICLE_DEST,         // save article dest dir
		DIR_PGP_HOME              // home of pgp.exe
	};

	enum EWinType {
		WND_POST          = 0,
		WND_REPLY         = 1,
		WND_VIEW          = 2,
		WND_MAIN          = 3,
		WND_MDI           = 4,
		WND_BUGREPORT     = 5,
		WND_PRODUCTIDEA   = 6,
		WND_SENDTOFRIEND  = 7
	};

	enum EMdiLayout {
		LAY3_2TOP,
		LAY3_2BOT,
		LAY3_2RIGHT,
		LAY3_2LEFT,
		LAY3_3HIGH,
		LAY3_3WIDE
	};

	enum EPaneType {
		PANE_NEWSGROUPS,
		PANE_THREADVIEW,
		PANE_ARTVIEW
	};

	static int  LayoutPaneCount(EMdiLayout eLayout);

	void GetPath (EDirType eDirType, TPath& tpath);
	void SetPath (EDirType eDirType, const TPath& tpath);

	BOOL GetWindowStatus (EWinType eWinType, TWindowStatus& wndStatus);
	void SetWindowStatus (EWinType eWinType, const TWindowStatus& wndStatus);

	//WORD GetViewFilter();             //obsolete
	//void SetViewFilter(WORD filter);  //obsolete

	const CString & Deprecated_GetViewFilterName();
	void Obsolete_SetViewFilterName(const CString& s);

	int  GetViewFilterID ();
	void SetViewFilterID (int iFilterID);

	void Save(void);
	BOOL Load(void);

	void SetLastGroup(int gid);

protected:
	BOOL FormulatePath (TPath& path);
	enum EUIMem_Dirty { DIRTY_DIR       = 0x1,
		DIRTY_LAYOUTWIN = 0x2,
		DIRTY_UI        = 0x4 };

protected:
	BYTE            m_byDirtyBits;
};

extern TUIMemory* gpUIMemory;
