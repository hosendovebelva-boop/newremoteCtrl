#pragma once

class CShareBannerWnd : public CWnd
{
public:
    CShareBannerWnd();

    bool EnsureCreated(HWND owner);
    void ShowBanner(const CString& helperName, const CString& peerIp, bool screenActive, bool microphoneActive, DWORD remainingSeconds);
    void UpdateIndicators(bool screenActive, bool microphoneActive, DWORD remainingSeconds);
    void HideBanner();

protected:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPaint();

    DECLARE_MESSAGE_MAP()

private:
    void DrawIndicator(CDC& dc, const CString& label, const CPoint& origin, COLORREF dotColor);

    CString m_text;
    HWND m_ownerWnd;
    bool m_screenActive;
    bool m_microphoneActive;
    DWORD m_remainingSeconds;
};
