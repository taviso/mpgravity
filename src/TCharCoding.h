/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TCharCoding.h,v $
/*  Revision 1.1  2010/07/21 17:14:56  richard_wood
/*  Initial checkin of V3.0.0 source code and other resources.
/*  Initial code builds V3.0.0 RC1
/*
/*  Revision 1.1  2009/06/09 13:21:28  richard_wood
/*  *** empty log message ***
/*
/*  Revision 1.2  2008/09/19 14:51:08  richard_wood
/*  Updated for VS 2005
/*
/*                                                                           */
/*****************************************************************************/

#pragma once

// TCharCoding.h : header file
//

class GravCharset;

/////////////////////////////////////////////////////////////////////////////
// TCharCoding dialog

class TCharCoding : public CDialog
{
public:
	TCharCoding(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_CHARACTER_CODING };
	CListBox	m_lbxCharsets;
	GravCharset * m_pCharset;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnDblclkList1();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};
