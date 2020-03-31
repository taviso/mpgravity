#include "stdafx.h"
#include "SplashScreenEx.h"
#include "utilpump.h"
#include <process.h>
#include <math.h>
#include "PngImage.h"

#ifndef AW_HIDE
	#define AW_HIDE 0x00010000
	#define AW_BLEND 0x00080000
#endif

#ifndef CS_DROPSHADOW
	#define CS_DROPSHADOW   0x00020000
#endif

// CSplashScreenEx

UINT CSplashScreenExTimerThreadEntryPoint(LPVOID lpParameter);

IMPLEMENT_DYNAMIC(CSplashScreenEx, CWnd)
CSplashScreenEx::CSplashScreenEx()
{
	m_pWndParent=NULL;
	m_strText="";
	m_nBitmapWidth=0;
	m_nBitmapHeight=0;
	m_nxPos=0;
	m_nyPos=0;
	m_dwTimeout=2000;
	m_dwStyle=0;
	m_rcText.SetRect(0,0,0,0);
	m_crTextColor=RGB(1,1,1);
	m_uTextFormat=DT_CENTER | DT_VCENTER | DT_WORDBREAK;
	m_bThreadRunning = false;
	m_bAbortThread = true;

	SetTextDefaultFont();
}

CSplashScreenEx::~CSplashScreenEx()
{
}

BEGIN_MESSAGE_MAP(CSplashScreenEx, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CSplashScreenEx::Create(CWnd *pWndParent,LPCTSTR szText,DWORD dwTimeout,DWORD dwStyle)
{
	// m_pWndParent is only used if we want to center the splash to the app window.
	if (dwStyle & CSS_CENTERAPP)
		ASSERT(pWndParent!=NULL);
	m_pWndParent = pWndParent;
	
	if (szText!=NULL)
		m_strText = szText;
	else
		m_strText = "";

	m_dwTimeout = dwTimeout;
	m_dwStyle = dwStyle;
	
	WNDCLASSEX wcx; 

	wcx.cbSize = sizeof(wcx);
	wcx.lpfnWndProc = AfxWndProc;
	wcx.style = CS_DBLCLKS|CS_SAVEBITS;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = AfxGetInstanceHandle();
	wcx.hIcon = NULL;
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground=::GetSysColorBrush(COLOR_WINDOW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = "SplashScreenExClass";
	wcx.hIconSm = NULL;

	if (m_dwStyle & CSS_SHADOW)
		wcx.style |= CS_DROPSHADOW;

	ATOM classAtom = RegisterClassEx(&wcx);
      
	// didn't work? try not using dropshadow (may not be supported)

	if (classAtom==NULL)
	{
		if (m_dwStyle & CSS_SHADOW)
		{
			wcx.style &= ~CS_DROPSHADOW;
			classAtom = RegisterClassEx(&wcx);
		}
		else
			return FALSE;
	}

	if (!CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED, "SplashScreenExClass", NULL, WS_POPUP,
//	if (!CreateEx(WS_EX_TOOLWINDOW, "SplashScreenExClass", NULL, WS_POPUP,
		0,0,0,0, pWndParent == NULL ? NULL : pWndParent->m_hWnd, NULL))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CSplashScreenEx::SetBMPImage(UINT uiResourceID)
{
	if (m_Image.m_hObject)
		m_Image.DeleteObject();
	if (!m_Image.LoadBitmap(uiResourceID)) return FALSE;

	BITMAP bitmap;
	m_Image.GetBitmap(&bitmap);

	m_nBitmapWidth = bitmap.bmWidth;
	m_nBitmapHeight = bitmap.bmHeight;
	m_rcText.SetRect(0, 0, m_nBitmapWidth, m_nBitmapHeight);
	
	if (m_dwStyle & CSS_CENTERSCREEN)
	{
		m_nxPos=(GetSystemMetrics(SM_CXFULLSCREEN)-m_nBitmapWidth)/2;
		m_nyPos=(GetSystemMetrics(SM_CYFULLSCREEN)-m_nBitmapHeight)/2;
	}
	else if ((m_dwStyle & CSS_CENTERAPP) && m_pWndParent)
	{
		CRect rcParentWindow;
		m_pWndParent->GetWindowRect(&rcParentWindow);
		m_nxPos=rcParentWindow.left+(rcParentWindow.right-rcParentWindow.left-m_nBitmapWidth)/2;
		m_nyPos=rcParentWindow.top+(rcParentWindow.bottom-rcParentWindow.top-m_nBitmapHeight)/2;
	}
	else
	{
		m_nxPos=(GetSystemMetrics(SM_CXFULLSCREEN)-m_nBitmapWidth)/2;
		m_nyPos=(GetSystemMetrics(SM_CYFULLSCREEN)-m_nBitmapHeight)/2;
	}

	return TRUE;
}

BOOL CSplashScreenEx::SetPNGImage(UINT uiResourceID)
{
	if (m_Image.m_hObject)
		m_Image.DeleteObject();
	// Use the png library to load it
	PngImage image;
	if (!image.load(uiResourceID))
		return FALSE;

	image.GetBitmap(m_Image, this);

	BITMAP bitmap;
	m_Image.GetBitmap(&bitmap);

	m_nBitmapWidth = bitmap.bmWidth;
	m_nBitmapHeight = bitmap.bmHeight;
	m_rcText.SetRect(0, 0, m_nBitmapWidth, m_nBitmapHeight);
	
	if (m_dwStyle & CSS_CENTERSCREEN)
	{
		m_nxPos=(GetSystemMetrics(SM_CXFULLSCREEN)-m_nBitmapWidth)/2 - 50;
		m_nyPos=(GetSystemMetrics(SM_CYFULLSCREEN)-m_nBitmapHeight)/2;
	}
	else if ((m_dwStyle & CSS_CENTERAPP) && m_pWndParent)
	{
		CRect rcParentWindow;
		m_pWndParent->GetWindowRect(&rcParentWindow);
		m_nxPos=rcParentWindow.left+(rcParentWindow.right-rcParentWindow.left-m_nBitmapWidth)/2;
		m_nyPos=rcParentWindow.top+(rcParentWindow.bottom-rcParentWindow.top-m_nBitmapHeight)/2;
	}
	else
	{
		m_nxPos=(GetSystemMetrics(SM_CXFULLSCREEN)-m_nBitmapWidth)/2;
		m_nyPos=(GetSystemMetrics(SM_CYFULLSCREEN)-m_nBitmapHeight)/2;
	}

	return TRUE;
}

void CSplashScreenEx::SetTextFont(LPCTSTR szFont,int nSize,int nStyle)
{
	LOGFONT lf;
	m_myFont.DeleteObject();
	m_myFont.CreatePointFont(nSize,szFont);
	m_myFont.GetLogFont(&lf);
	
	if (nStyle & CSS_TEXT_BOLD)
		lf.lfWeight = FW_BOLD;
	else
		lf.lfWeight = FW_NORMAL;
	
	if (nStyle & CSS_TEXT_ITALIC)
		lf.lfItalic=TRUE;
	else
		lf.lfItalic=FALSE;
	
	if (nStyle & CSS_TEXT_UNDERLINE)
		lf.lfUnderline=TRUE;
	else
		lf.lfUnderline=FALSE;

	m_myFont.DeleteObject();
	m_myFont.CreateFontIndirect(&lf);
}

void CSplashScreenEx::SetTextDefaultFont()
{
	LOGFONT lf;
	CFont *myFont=CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	myFont->GetLogFont(&lf);
	m_myFont.DeleteObject();
	m_myFont.CreateFontIndirect(&lf);
}

void CSplashScreenEx::SetText(LPCTSTR szText)
{
	m_strText=szText;
	RedrawWindow();
}

void CSplashScreenEx::SetTextColor(COLORREF crTextColor)
{
	// Proper black is treated as a transparent colour so the text cannot use true black.
	if (crTextColor == RGB(0,0,0))
		crTextColor = RGB(1,1,1);

	m_crTextColor=crTextColor;
	RedrawWindow();
}

void CSplashScreenEx::SetTextRect(CRect& rcText)
{
	m_rcText=rcText;
	RedrawWindow();
}

void CSplashScreenEx::SetTextFormat(UINT uTextFormat)
{
	m_uTextFormat=uTextFormat;
}

void CSplashScreenEx::Show()
{
	// If fade in or fade out, fire off a worker thread to deal with the fading
	if ((m_dwStyle & CSS_FADEIN) || (m_dwStyle & CSS_FADEOUT))
	{
		unsigned int nDummy;
		m_hThread = (HANDLE) _beginthreadex(NULL, 0, _ThreadEntry, this, CREATE_SUSPENDED, &nDummy);
		if (m_hThread)
			ResumeThread(m_hThread);
		else 
			TRACE(_T("Draw: Couldn't start timer thread\n"));
	}
	else
	{
		// Else just pop up the window
		SetLayeredWindowAttributes(0x00000000, 255, LWA_COLORKEY|LWA_ALPHA);
		SetWindowPos(NULL, m_nxPos, m_nyPos, m_nBitmapWidth, m_nBitmapHeight,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		ShowWindow(SW_SHOW);
	}
}

void CSplashScreenEx::Hide()
{
	if (m_hThread)
	{
		// Thread was, or is, running.
		// Tell thread to stop waiting and hide window now
		m_bAbortThread = true;
		while (m_bThreadRunning)
		{
			PumpDialogMessages();
			Sleep(0);
		}
		m_hThread = NULL;
	}
	else
	{
		// No thread running, hide it ourselves
		if (m_dwStyle & CSS_FADEOUT)
			Fade(false, 200);
	}
	ShowWindow(SW_HIDE);
}

UINT WINAPI CSplashScreenEx::_ThreadEntry(LPVOID pParam)
{
	ASSERT(pParam);
	CSplashScreenEx *pPic = reinterpret_cast<CSplashScreenEx *> (pParam);

	pPic->ThreadEntry();

	return 0;
}

void CSplashScreenEx::ThreadEntry()
{
	m_bThreadRunning = true;
	m_bAbortThread = false;
	SetWindowPos(NULL, m_nxPos, m_nyPos, m_nBitmapWidth, m_nBitmapHeight,
		SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);

	if (m_dwStyle & CSS_FADEIN)
		Fade(true, 400);

	UINT nCount = 0;
	while(!m_bAbortThread && (nCount < m_dwTimeout))
	{
		Sleep(25);
		nCount += 25;
	}

	if (m_dwStyle & CSS_FADEOUT)
		Fade(false, 200);

	m_bThreadRunning = false;
}

BOOL CSplashScreenEx::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CSplashScreenEx::OnPaint()
{
	CPaintDC dc(this);
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap *pbmOld = memDC.SelectObject(&m_Image);
	dc.BitBlt(0, 0, m_nBitmapWidth, m_nBitmapHeight, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pbmOld);

	// Draw Text
	CFont *pOldFont;
	pOldFont=dc.SelectObject(&m_myFont);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(m_crTextColor);
	dc.DrawText(m_strText,-1,m_rcText,m_uTextFormat);
	dc.SelectObject(pOldFont);
}

void CSplashScreenEx::PostNcDestroy()
{
	CWnd::PostNcDestroy();
}

BOOL CSplashScreenEx::PreTranslateMessage(MSG* pMsg)
{
	if (m_dwStyle & CSS_HIDEONCLICK)
	{
		if (pMsg)
		{
			if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
			{
				Hide();
			}
			if (pMsg->message == WM_LBUTTONUP ||
				pMsg->message == WM_LBUTTONDBLCLK)
			{
				Hide();
			}
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CSplashScreenEx::Fade(bool bFadeIn, int nDuration)
{
	// Make sure the window is faded in our out completely (according to what we're doing)
	// and show the window.
	SetLayeredWindowAttributes(0x00000000, bFadeIn ? 0 : 255, LWA_COLORKEY|LWA_ALPHA);
	ShowWindow(SW_SHOW);

	DWORD dwStart = ::GetTickCount(), dwNow;
	double dPercentage = 0;
	while(dPercentage < 100.0)
	{
		Sleep(0);
		if (bFadeIn)
			SetLayeredWindowAttributes(0x00000000, (int)(dPercentage*2.55), LWA_COLORKEY|LWA_ALPHA);
		else
			SetLayeredWindowAttributes(0x00000000, 255-(int)(dPercentage*2.55), LWA_COLORKEY|LWA_ALPHA);
		UpdateWindow();

		dwNow = ::GetTickCount();
		dPercentage = ((double(dwNow) - double(dwStart)) / double(nDuration)) * 100;
	}

	SetLayeredWindowAttributes(0x00000000, bFadeIn ? 255 : 0, LWA_COLORKEY|LWA_ALPHA);
	UpdateWindow();

	// If fading out, last thing to do is hide the window
	if (!bFadeIn)
		ShowWindow(SW_HIDE);
}
