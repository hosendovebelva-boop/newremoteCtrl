#include "pch.h"
#include "ShareBannerWnd.h"
#include "ServerSocket.h"

CShareBannerWnd::CShareBannerWnd()
    : m_ownerWnd(nullptr), m_screenActive(false), m_microphoneActive(false), m_remainingSeconds(0)
{
}

bool CShareBannerWnd::EnsureCreated(HWND owner)
{
    m_ownerWnd = owner;
    if (::IsWindow(m_hWnd))
    {
        return true;
    }

    CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_HAND));
    const CRect bounds(0, 0, GetSystemMetrics(SM_CXSCREEN), 44);
    return CreateEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className,
        _T("ScreenShareBanner"),
        WS_POPUP,
        bounds,
        CWnd::FromHandle(owner),
        0) != FALSE;
}

void CShareBannerWnd::ShowBanner(
    const CString& helperName,
    const CString& peerIp,
    const bool screenActive,
    const bool microphoneActive,
    const DWORD remainingSeconds)
{
    if (!EnsureCreated(m_ownerWnd))
    {
        return;
    }

    const CString displayName = helperName.IsEmpty() ? peerIp : helperName;
    m_text.Format(_T("Screen shared with %s (%s) - End session"), displayName.GetString(), peerIp.GetString());
    m_screenActive = screenActive;
    m_microphoneActive = microphoneActive;
    m_remainingSeconds = remainingSeconds;
    SetWindowPos(&wndTopMost, 0, 0, GetSystemMetrics(SM_CXSCREEN), 44, SWP_SHOWWINDOW);
    Invalidate();
}

void CShareBannerWnd::UpdateIndicators(const bool screenActive, const bool microphoneActive, const DWORD remainingSeconds)
{
    m_screenActive = screenActive;
    m_microphoneActive = microphoneActive;
    m_remainingSeconds = remainingSeconds;
    if (::IsWindow(m_hWnd) && IsWindowVisible())
    {
        Invalidate();
    }
}

void CShareBannerWnd::HideBanner()
{
    if (::IsWindow(m_hWnd))
    {
        ShowWindow(SW_HIDE);
    }
}

BOOL CShareBannerWnd::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    pDC->FillSolidRect(&rect, RGB(228, 105, 58));
    return TRUE;
}

void CShareBannerWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_ownerWnd != nullptr)
    {
        ::PostMessage(m_ownerWnd, WM_HOST_BANNER_END_SESSION, 0, 0);
    }

    CWnd::OnLButtonUp(nFlags, point);
}

void CShareBannerWnd::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(255, 255, 255));

    CRect textRect(rect);
    textRect.left = 210;
    textRect.right -= 160;
    dc.DrawText(m_text, &textRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

    DrawIndicator(dc, _T("Screen"), CPoint(18, 13), m_screenActive ? RGB(44, 188, 91) : RGB(135, 135, 135));
    DrawIndicator(dc, _T("Mic"), CPoint(112, 13), m_microphoneActive ? RGB(44, 188, 91) : RGB(135, 135, 135));

    CString remainingText;
    remainingText.Format(_T("%u min left"), (m_remainingSeconds + 59) / 60);
    CRect remainingRect(rect.right - 145, 0, rect.right - 12, rect.bottom);
    dc.DrawText(remainingText, &remainingRect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
}

void CShareBannerWnd::DrawIndicator(CDC& dc, const CString& label, const CPoint& origin, const COLORREF dotColor)
{
    CBrush brush(dotColor);
    CBrush* oldBrush = dc.SelectObject(&brush);
    CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
    CPen* oldPen = dc.SelectObject(&pen);
    dc.Ellipse(origin.x, origin.y, origin.x + 12, origin.y + 12);
    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    CRect labelRect(origin.x + 16, origin.y - 3, origin.x + 80, origin.y + 16);
    dc.DrawText(label, &labelRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
}

BEGIN_MESSAGE_MAP(CShareBannerWnd, CWnd)
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
END_MESSAGE_MAP()
