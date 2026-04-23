#pragma once

class CShareBannerWnd : public CWnd
{
public:
    CShareBannerWnd();

    bool EnsureCreated(HWND owner);
    void ShowBanner(const CString& peerIp);
    void HideBanner();

protected:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPaint();

    DECLARE_MESSAGE_MAP()

private:
    CString m_text;
    HWND m_ownerWnd;
};
