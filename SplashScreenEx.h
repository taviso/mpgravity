#pragma once

#define CSS_FADEIN		0x0001
#define CSS_FADEOUT		0x0002
#define CSS_FADE		CSS_FADEIN | CSS_FADEOUT
#define CSS_SHADOW		0x0004
#define CSS_CENTERSCREEN	0x0008
#define CSS_CENTERAPP		0x0010
#define CSS_HIDEONCLICK		0x0020

#define CSS_TEXT_NORMAL		0x0000
#define CSS_TEXT_BOLD		0x0001
#define CSS_TEXT_ITALIC		0x0002
#define CSS_TEXT_UNDERLINE	0x0004

// CSplashScreenEx

class CSplashScreenEx : public CWnd
{
	DECLARE_DYNAMIC(CSplashScreenEx)

public:
	CSplashScreenEx();
	virtual ~CSplashScreenEx();

	BOOL Create(CWnd *pWndParent,LPCTSTR szText=NULL,DWORD dwTimeout=2000,DWORD dwStyle=CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW);
	BOOL SetBMPImage(UINT uiResourceID);
	BOOL SetPNGImage(UINT uiResourceID);
	void SetStyle(DWORD dwStyle=CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW) {m_dwStyle = dwStyle;};

	void Show();
	void Hide();

	void SetText(LPCTSTR szText);
	void SetTextFont(LPCTSTR szFont,int nSize,int nStyle);
	void SetTextDefaultFont();
	void SetTextColor(COLORREF crTextColor);
	void SetTextRect(CRect& rcText);
	void SetTextFormat(UINT uTextFormat);

	static UINT WINAPI _ThreadEntry(LPVOID pParam);
	void ThreadEntry();
	
protected:	
	void GetImage();
	virtual void PostNcDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	void Fade(bool bFadeIn, int nDuration);

	CWnd *m_pWndParent;
	CFont m_myFont;
	
	DWORD m_dwStyle;
	DWORD m_dwTimeout;
	CString m_strText;
	CRect m_rcText;
	UINT m_uTextFormat;
	COLORREF m_crTextColor;

	int m_nBitmapWidth;
	int m_nBitmapHeight;
	int m_nxPos;
	int m_nyPos;

	DWORD m_dwStartTime;

	HANDLE m_hThread;

	CBitmap m_Image;

	bool m_bThreadRunning;
	bool m_bAbortThread;
};
