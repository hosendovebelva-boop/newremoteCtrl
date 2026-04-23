#include "pch.h"
#include "HostMainDlg.h"

#include "ConsentDialog.h"
#include "EdoyunTool.h"
#include "SessionLog.h"
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

    SetWindowText(_T("Remote Assist Host - Waiting for viewer"));
    SetDlgItemText(IDC_STATIC_HOST_IP_VALUE, CEdoyunTool::JoinLocalIpv4Addresses());

    CString portText;
    portText.Format(_T("%u"), ScreenShareProtocol::kDefaultPort);
    SetDlgItemText(IDC_STATIC_HOST_PORT_VALUE, portText);

    RefreshSessionCode();
    UpdateStatus(_T("Waiting for viewer"));
    GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_EXTEND_SESSION)->EnableWindow(FALSE);
    RefreshRecentSessions();

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
    KillTimer(CSessionPolicy::kTimerId);
    RemoveTrayIcon();
    m_server.Stop();
    CDialogEx::OnCancel();
}

void CHostMainDlg::OnBnClickedStopSharing()
{
    m_server.EndSession(_T("Session ended locally."));
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
        if (!payload->peerIp.IsEmpty())
        {
            AppendSessionLog(_T("connect_attempt"), payload->detail, payload->helperName, payload->peerIp);
        }
        if (!m_sessionPolicy.IsActive())
        {
            UpdateStatus(_T("Waiting for viewer"));
            GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_EXTEND_SESSION)->EnableWindow(FALSE);
        }
        break;
    case HostServerEventType::WaitingForConsent:
        AppendSessionLog(_T("consent_shown"), payload->detail, payload->helperName, payload->peerIp);
        UpdateStatus(_T("Waiting for local approval..."));
        break;
    case HostServerEventType::SharingStarted:
        StartSession(*payload);
        break;
    case HostServerEventType::SessionEnded:
        StopSession(*payload);
        break;
    case HostServerEventType::Error:
        AppendSessionLog(_T("socket_error"), payload->detail, payload->helperName, payload->peerIp);
        SetWindowText(_T("Remote Assist Host - Waiting for viewer"));
        m_banner.HideBanner();
        GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_EXTEND_SESSION)->EnableWindow(FALSE);
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
    dialog.SetRequestDetails(request->peerIp, request->helperName, request->sessionCode, request->hostName);
    request->result = (dialog.DoModal() == IDOK) ? ScreenShareProtocol::Approved
                                                 : (dialog.WasTimedOut() ? ScreenShareProtocol::TimedOut : ScreenShareProtocol::Denied);
    AppendSessionLog(
        request->result == ScreenShareProtocol::Approved ? _T("consent_approved")
                                                         : (request->result == ScreenShareProtocol::TimedOut ? _T("consent_timed_out")
                                                                                                             : _T("consent_denied")),
        request->result == ScreenShareProtocol::Approved ? _T("Screen stream approved.")
                                                         : (request->result == ScreenShareProtocol::TimedOut ? _T("Consent dialog timed out.")
                                                                                                             : _T("Host denied screen sharing.")),
        request->helperName,
        request->peerIp);
    ::SetEvent(request->completedEvent);
    return 0;
}

LRESULT CHostMainDlg::OnBannerEndSession(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    m_server.EndSession(_T("Session ended locally."));
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

void CHostMainDlg::OnBnClickedExtendSession()
{
    if (!m_sessionPolicy.IsActive())
    {
        return;
    }

    m_sessionPolicy.Extend();
    AppendSessionLog(_T("session_extended"), _T("Host extended the session by 15 minutes."));
    RefreshSessionTimerUi();
    RefreshRecentSessions();
}

void CHostMainDlg::OnTimer(UINT_PTR eventId)
{
    if (eventId == CSessionPolicy::kTimerId)
    {
        if (m_sessionPolicy.IsExpired())
        {
            AppendSessionLog(_T("session_expired"), _T("60-minute session limit reached."));
            m_server.EndSession(_T("Session expired."));
            return;
        }

        if (m_sessionPolicy.ShouldPromptForExtension())
        {
            m_sessionPolicy.MarkExtensionPrompted();
            GetDlgItem(IDC_BTN_EXTEND_SESSION)->EnableWindow(TRUE);
            UpdateStatus(_T("Session will expire soon. Host can extend by 15 minutes."));
        }

        RefreshSessionTimerUi();
        return;
    }

    CDialogEx::OnTimer(eventId);
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

void CHostMainDlg::StartSession(const HostServerEventPayload& payload)
{
    m_activeHelperName = payload.helperName;
    m_activePeerIp = payload.peerIp;
    m_sessionPolicy.Start();

    const CString displayName = m_activeHelperName.IsEmpty() ? m_activePeerIp : m_activeHelperName;
    CString title;
    title.Format(_T("Remote Assist Host - Sharing with %s"), displayName.GetString());
    SetWindowText(title);

    CString status;
    status.Format(_T("Sharing with %s (%s)"), displayName.GetString(), m_activePeerIp.GetString());
    UpdateStatus(status);
    GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(TRUE);
    GetDlgItem(IDC_BTN_EXTEND_SESSION)->EnableWindow(FALSE);

    m_banner.EnsureCreated(m_hWnd);
    m_banner.ShowBanner(
        m_activeHelperName,
        m_activePeerIp,
        m_sessionPolicy.GetStreamConsent(AssistStreamId::Screen) == AssistStreamConsent::Approved,
        false,
        m_sessionPolicy.RemainingSeconds());
    SetTimer(CSessionPolicy::kTimerId, CSessionPolicy::kTimerIntervalMs, nullptr);

    AppendSessionLog(_T("session_started"), payload.detail);
    RefreshRecentSessions();
}

void CHostMainDlg::StopSession(const HostServerEventPayload& payload)
{
    const CString detail = payload.detail.IsEmpty() ? _T("Session ended.") : payload.detail;
    const CString eventType = detail == _T("Session expired.") ? _T("session_expired")
                                                              : (detail == _T("Session ended locally.") ? _T("session_ended_locally")
                                                                                                      : _T("session_disconnected"));
    AppendSessionLog(eventType, detail, payload.helperName, payload.peerIp);

    KillTimer(CSessionPolicy::kTimerId);
    m_sessionPolicy.Stop();
    m_activeHelperName.Empty();
    m_activePeerIp.Empty();
    SetWindowText(_T("Remote Assist Host - Waiting for viewer"));
    m_banner.HideBanner();
    GetDlgItem(IDC_BTN_STOP_SHARING)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_EXTEND_SESSION)->EnableWindow(FALSE);
    RefreshSessionCode();
    UpdateStatus(_T("Waiting for viewer"));
    RefreshRecentSessions();
}

void CHostMainDlg::RefreshRecentSessions()
{
    SetDlgItemText(IDC_EDIT_RECENT_SESSIONS, CSessionLog::ReadRecentSessions());
}

void CHostMainDlg::RefreshSessionTimerUi()
{
    if (!m_sessionPolicy.IsActive())
    {
        return;
    }

    m_banner.UpdateIndicators(
        m_sessionPolicy.GetStreamConsent(AssistStreamId::Screen) == AssistStreamConsent::Approved,
        false,
        m_sessionPolicy.RemainingSeconds());
}

void CHostMainDlg::AppendSessionLog(const CString& eventType, const CString& detail, const CString& helperName, const CString& peerIp)
{
    const CString logHelperName = helperName.IsEmpty() ? m_activeHelperName : helperName;
    const CString logPeerIp = peerIp.IsEmpty() ? m_activePeerIp : peerIp;
    CSessionLog::AppendEvent(logHelperName, logPeerIp, eventType, detail);
}

void CHostMainDlg::ConfigureTrayIcon()
{
    m_trayData.cbSize = sizeof(m_trayData);
    m_trayData.hWnd = m_hWnd;
    m_trayData.uID = 1;
    m_trayData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_trayData.uCallbackMessage = WM_HOST_TRAYICON;
    m_trayData.hIcon = m_icon;
    _tcscpy_s(m_trayData.szTip, _T("Remote Assist Host"));
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
        m_server.EndSession(_T("Session ended locally."));
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
    ON_BN_CLICKED(IDC_BTN_EXTEND_SESSION, &CHostMainDlg::OnBnClickedExtendSession)
    ON_MESSAGE(WM_HOST_SERVER_EVENT, &CHostMainDlg::OnServerEvent)
    ON_MESSAGE(WM_HOST_CONSENT_REQUEST, &CHostMainDlg::OnConsentRequest)
    ON_MESSAGE(WM_HOST_BANNER_END_SESSION, &CHostMainDlg::OnBannerEndSession)
    ON_MESSAGE(WM_HOST_TRAYICON, &CHostMainDlg::OnTrayIcon)
    ON_WM_TIMER()
END_MESSAGE_MAP()
