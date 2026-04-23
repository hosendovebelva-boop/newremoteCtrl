#include "pch.h"
#include "ShareBannerWnd.h"
#include "ServerSocket.h"

CShareBannerWnd::CShareBannerWnd() : m_ownerWnd(nullptr)
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
    const CRect bounds(0, 0, GetSystemMetrics(SM_CXSCREEN), 36);
    return CreateEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className,
        _T("ScreenShareBanner"),
        WS_POPUP,
        bounds,
        CWnd::FromHandle(owner),
        0) != FALSE;
}

void CShareBannerWnd::ShowBanner(const CString& peerIp)
{
    if (!EnsureCreated(m_ownerWnd))
    {
        return;
    }

    m_text.Format(_T("Your screen is being viewed by %s. Click to end session."), peerIp.GetString());
    SetWindowPos(&wndTopMost, 0, 0, GetSystemMetrics(SM_CXSCREEN), 36, SWP_SHOWWINDOW);
    Invalidate();
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
    dc.DrawText(m_text, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

BEGIN_MESSAGE_MAP(CShareBannerWnd, CWnd)
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
END_MESSAGE_MAP()
