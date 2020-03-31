/*****************************************************************************/
/*  SOURCE CONTROL VERSIONS                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Version Date  Time  Author / Comment (optional)                          */
/*                                                                           */
/*  $Log: TCharCoding.cpp,v $
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

// TCharCoding.cpp : implementation file
//

#include "stdafx.h"
#include "news.h"
#include "TCharCoding.h"
#include "8859x.h"
#include "tglobopt.h"
#include "rgsys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TCharCoding dialog

TCharCoding::TCharCoding(CWnd* pParent /*=NULL*/)
	: CDialog(TCharCoding::IDD, pParent)
{


	m_pCharset = 0;
}

void TCharCoding::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST1, m_lbxCharsets);


	if (FALSE==pDX->m_bSaveAndValidate)
		{
      int currentId = gpGlobalOptions -> GetRegSystem() -> GetViewingCharsetID();

		list<GravCharset*>::iterator it = gsCharMaster.m_sCharsets.begin();
		for (; it != gsCharMaster.m_sCharsets.end(); it++)
			{
			GravCharset* pCharset = *it;
			int iAt = m_lbxCharsets.AddString ( pCharset->GetName() );
			m_lbxCharsets.SetItemDataPtr (iAt, pCharset);
         if (pCharset->GetId() == currentId)
            m_lbxCharsets.SetCurSel (iAt);

			}
		}
   else
      {
      gpGlobalOptions -> GetRegSystem() -> SetViewingCharsetID( m_pCharset->GetId() );
      }
}

BEGIN_MESSAGE_MAP(TCharCoding, CDialog)
		ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TCharCoding message handlers

void TCharCoding::OnDblclkList1()
{
	OnOK();
}

void TCharCoding::OnOK()
{
	int idx = m_lbxCharsets.GetCurSel();
	if (LB_ERR == idx)
		m_pCharset =  NULL;
	else
		m_pCharset = static_cast<GravCharset*>(m_lbxCharsets.GetItemDataPtr(idx));

	CDialog::OnOK();
}
