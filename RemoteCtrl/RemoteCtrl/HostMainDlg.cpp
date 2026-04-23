#include "pch.h"
#include "HostMainDlg.h"

#include "ConsentDialog.h"
#include "EdoyunTool.h"
#include "..\ScreenShareProtocol.h"

#include <memory>

CHostMainDlg::CHostMainDlg(CWnd* parent) : CDialogEx(IDD_DIALOG_HOST_MAIN, parent), m_icon(::LoadIcon(nullptr, IDI_APPLICATION))
{
    ZeroMemory(&m_trayData, sizeof(m_trayData));
}

void CHostMainDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CHostMainDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_icon, TRUE);
    SetIcon(m_icon, FALSE);

    SetWindowText(_T("Screen Share Host - Waiting for viewer"));
    SetDlgItemText(IDC_STATIC_HOST_IP_VALUE, CEdoyunTool::JoinLocalIpv4Addresses());

    CString portText;
    portText.Format(_T("%u"), ScreenShareProtocol::kDefaultPort);
    SetDlgItemText(IDC_STATIC_HOST_PORT_VALUE, portText);

    RefreshSessionCode();
    UpdateStatus(_T("Waiting for viewer"));
    GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);

    ConfigureTrayIcon();

    if (!m_server.Start(m_hWnd, m_sessionCode))
    {
        UpdateStatus(_T("Unable to bind the listening socket."));
    }

    return TRUE;
}

void CHostMainDlg::OnCancel()
{
    m_banner.HideBanner();
    RemoveTrayIcon();
    m_server.Stop();
    CDialogEx::OnCancel();
}

void CHostMainDlg::OnBnClickedStopSharing()
{
    m_server.EndSession();
}

LRESULT CHostMainDlg::OnServerEvent(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    std::unique_ptr<HostServerEventPayload> payload(reinterpret_cast<HostServerEventPayload*>(lParam));
    if (!payload)
    {
        return 0;
    }

    switch (payload->type)
    {
    case HostServerEventType::Listening:
        UpdateStatus(_T("Waiting for viewer"));
        GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
        break;
    case HostServerEventType::WaitingForConsent:
        UpdateStatus(_T("Waiting for local approval..."));
        break;
    case HostServerEventType::SharingStarted:
    {
        CString title;
        title.Format(_T("Screen Share Host - Sharing with %s"), payload->peerIp.GetString());
        SetWindowText(title);
        CString status;
        status.Format(_T("Sharing with %s"), payload->peerIp.GetString());
        UpdateStatus(status);
        GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(TRUE);
        m_banner.EnsureCreated(m_hWnd);
        m_banner.ShowBanner(payload->peerIp);
        break;
    }
    case HostServerEventType::SessionEnded:
        SetWindowText(_T("Screen Share Host - Waiting for viewer"));
        m_banner.HideBanner();
        GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
        RefreshSessionCode();
        UpdateStatus(_T("Waiting for viewer"));
        break;
    case HostServerEventType::Error:
        SetWindowText(_T("Screen Share Host - Waiting for viewer"));
        m_banner.HideBanner();
        GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
        RefreshSessionCode();
        UpdateStatus(payload->detail.IsEmpty() ? _T("Socket error.") : payload->detail);
        break;
    default:
        break;
    }

    return 0;
}

LRESULT CHostMainDlg::OnConsentRequest(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    ConsentRequest* request = reinterpret_cast<ConsentRequest*>(lParam);
    if (request == nullptr)
    {
        return 0;
    }

    CConsentDialog dialog(this);
    dialog.SetViewerIp(request->peerIp);
    request->allowed = (dialog.DoModal() == IDOK);
    ::SetEvent(request->completedEvent);
    return 0;
}

LRESULT CHostMainDlg::OnBannerEndSession(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    m_server.EndSession();
    return 0;
}

LRESULT CHostMainDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    switch (lParam)
    {
    case WM_RBUTTONUP:
        ShowTrayMenu();
        break;
    case WM_LBUTTONDBLCLK:
        ShowWindow(SW_SHOW);
        SetForegroundWindow();
        break;
    default:
        break;
    }

    return 0;
}

void CHostMainDlg::RefreshSessionCode()
{
    m_sessionCode = CEdoyunTool::GenerateSessionCode();
    SetDlgItemText(IDC_STATIC_SESSION_CODE_VALUE, m_sessionCode);
    m_server.SetSessionCode(m_sessionCode);
}

void CHostMainDlg::UpdateStatus(const CString& statusText)
{
    SetDlgItemText(IDC_STATIC_STATUS_VALUE, statusText);
}

void CHostMainDlg::ConfigureTrayIcon()
{
    m_trayData.cbSize = sizeof(m_trayData);
    m_trayData.hWnd = m_hWnd;
    m_trayData.uID = 1;
    m_trayData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_trayData.uCallbackMessage = WM_HOST_TRAYICON;
    m_trayData.hIcon = m_icon;
    _tcscpy_s(m_trayData.szTip, _T("Screen Share Host"));
    Shell_NotifyIcon(NIM_ADD, &m_trayData);
}

void CHostMainDlg::RemoveTrayIcon()
{
    if (m_trayData.hWnd != nullptr)
    {
        Shell_NotifyIcon(NIM_DELETE, &m_trayData);
    }
}

void CHostMainDlg::ShowTrayMenu()
{
    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, ID_TRAY_SHOW, _T("Show Host Window"));
    menu.AppendMenu(MF_STRING | (m_server.IsSharing() ? MF_ENABLED : MF_GRAYED), ID_TRAY_STOP, _T("Stop Sharing"));
    menu.AppendMenu(MF_STRING, ID_TRAY_EXIT, _T("Exit"));

    CPoint cursorPoint;
    GetCursorPos(&cursorPoint);
    SetForegroundWindow();
    const UINT command = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON, cursorPoint.x, cursorPoint.y, this);

    switch (command)
    {
    case ID_TRAY_SHOW:
        ShowWindow(SW_SHOW);
        SetForegroundWindow();
        break;
    case ID_TRAY_STOP:
        m_server.EndSession();
        break;
    case ID_TRAY_EXIT:
        OnCancel();
        break;
    default:
        break;
    }
}

BEGIN_MESSAGE_MAP(CHostMainDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_STOP_SHARING, &CHostMainDlg::OnBnClickedStopSharing)
    ON_MESSAGE(WM_HOST_SERVER_EVENT, &CHostMainDlg::OnServerEvent)
    ON_MESSAGE(WM_HOST_CONSENT_REQUEST, &CHostMainDlg::OnConsentRequest)
    ON_MESSAGE(WM_HOST_BANNER_END_SESSION, &CHostMainDlg::OnBannerEndSession)
    ON_MESSAGE(WM_HOST_TRAYICON, &CHostMainDlg::OnTrayIcon)
END_MESSAGE_MAP()
