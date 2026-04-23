#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"

#include "..\ScreenShareProtocol.h"

#include <atlconv.h>
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent)
    : CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent),
      m_hIcon(::LoadIcon(nullptr, IDI_APPLICATION)),
      m_serverAddress(0x7F000001),
      m_portText(_T("9527")),
      m_framePending(false),
      m_sessionActive(false),
      m_localCloseInProgress(false)
{
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_serverAddress);
    DDX_Text(pDX, IDC_EDIT_PORT, m_portText);
    DDX_Text(pDX, IDC_EDIT_SESSION_CODE, m_sessionCode);
    DDX_Text(pDX, IDC_EDIT_HELPER_NAME, m_helperName);
}

BOOL CRemoteClientDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    SetWindowText(_T("Remote Assist Viewer"));
    SetStatus(_T("Enter the host IP, port, a 6-digit session code, and your helper name."));
    GetDlgItem(IDC_EDIT_SESSION_CODE)->SendMessage(EM_SETLIMITTEXT, ScreenShareProtocol::kSessionCodeLength, 0);
    GetDlgItem(IDC_EDIT_HELPER_NAME)->SendMessage(EM_SETLIMITTEXT, ScreenShareProtocol::kHelperNameMaxLength, 0);
    UpdateData(FALSE);
    return TRUE;
}

void CRemoteClientDlg::OnCancel()
{
    m_watchDialog.HideWatch();
    m_localCloseInProgress = true;
    m_socket.Close();
    m_localCloseInProgress = false;
    CDialogEx::OnCancel();
}

void CRemoteClientDlg::OnBnClickedConnect()
{
    UpdateData(TRUE);

    m_sessionCode.Trim();
    m_helperName.Trim();
    UpdateData(FALSE);

    const UINT port = _ttoi(m_portText);
    const std::string sessionCode = CT2A(m_sessionCode);
    const CW2A helperNameUtf8(m_helperName, CP_UTF8);
    const std::string helperName(helperNameUtf8);
    std::string helloPayload;
    if (port == 0 || !ScreenShareProtocol::BuildHelloPayload(sessionCode, helperName, helloPayload))
    {
        SetStatus(_T("Please enter a valid host IP, port, 6-digit session code, and helper name."));
        return;
    }

    if (!m_socket.Connect(GetServerIpString(), port, m_hWnd))
    {
        SetStatus(_T("Unable to connect to the host."));
        return;
    }

    if (!m_socket.SendPacket(CPacket(ScreenShareProtocol::Hello, helloPayload)))
    {
        m_localCloseInProgress = true;
        m_socket.Close();
        m_localCloseInProgress = false;
        SetStatus(_T("Failed to send the connection hello."));
        return;
    }

    GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(FALSE);
    SetStatus(_T("Waiting for host approval..."));
}

LRESULT CRemoteClientDlg::OnViewerPacket(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    std::unique_ptr<CPacket> packet(reinterpret_cast<CPacket*>(lParam));
    if (!packet)
    {
        return 0;
    }

    if (packet->Command() == ScreenShareProtocol::ConsentResult && !packet->Payload().empty())
    {
        HandleSessionStatus(static_cast<BYTE>(packet->Payload()[0]));
        return 0;
    }

    if (packet->Command() == ScreenShareProtocol::FrameRequest)
    {
        m_framePending = false;
        if (m_sessionActive)
        {
            m_watchDialog.UpdateFrame(packet->Payload());
        }
        return 0;
    }

    return 0;
}

LRESULT CRemoteClientDlg::OnViewerSocketClosed(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    if (!m_localCloseInProgress)
    {
        const ViewerCloseReason reason = static_cast<ViewerCloseReason>(wParam);
        if (reason == ViewerCloseReason::ParseError)
        {
            HandleRemoteSessionEnded(_T("Connection closed after a protocol error."));
        }
        else
        {
            HandleRemoteSessionEnded(_T("Connection closed."));
        }
    }

    return 0;
}

LRESULT CRemoteClientDlg::OnWatchRequestFrame(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (!m_sessionActive || m_framePending || !m_socket.IsConnected())
    {
        return 0;
    }

    if (m_socket.SendPacket(CPacket(ScreenShareProtocol::FrameRequest, nullptr, 0)))
    {
        m_framePending = true;
    }
    else
    {
        HandleRemoteSessionEnded(_T("Unable to request the next frame."));
    }

    return 0;
}

LRESULT CRemoteClientDlg::OnWatchEndSession(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    m_localCloseInProgress = true;
    m_socket.Close();
    m_localCloseInProgress = false;
    m_watchDialog.HideWatch();
    m_sessionActive = false;
    m_framePending = false;
    GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
    SetStatus(_T("Session ended."));
    return 0;
}

void CRemoteClientDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        const int iconWidth = GetSystemMetrics(SM_CXICON);
        const int iconHeight = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        const int x = (rect.Width() - iconWidth + 1) / 2;
        const int y = (rect.Height() - iconHeight + 1) / 2;
        dc.DrawIcon(x, y, m_hIcon);
        return;
    }

    CDialogEx::OnPaint();
}

HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

CString CRemoteClientDlg::GetServerIpString() const
{
    CString ipAddress;
    ipAddress.Format(
        _T("%u.%u.%u.%u"),
        FIRST_IPADDRESS(m_serverAddress),
        SECOND_IPADDRESS(m_serverAddress),
        THIRD_IPADDRESS(m_serverAddress),
        FOURTH_IPADDRESS(m_serverAddress));
    return ipAddress;
}

void CRemoteClientDlg::SetStatus(const CString& statusText)
{
    SetDlgItemText(IDC_EDIT_STATUS, statusText);
}

void CRemoteClientDlg::HandleSessionStatus(BYTE status)
{
    switch (status)
    {
    case ScreenShareProtocol::BadCode:
        m_localCloseInProgress = true;
        m_socket.Close();
        m_localCloseInProgress = false;
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
        SetStatus(_T("Incorrect session code."));
        break;
    case ScreenShareProtocol::WaitingForConsent:
        SetStatus(_T("Waiting for host approval..."));
        break;
    case ScreenShareProtocol::Denied:
        m_localCloseInProgress = true;
        m_socket.Close();
        m_localCloseInProgress = false;
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
        SetStatus(_T("Host denied the request."));
        break;
    case ScreenShareProtocol::Approved:
        m_sessionActive = true;
        m_framePending = false;
        m_watchDialog.EnsureCreated(this);
        m_watchDialog.ShowWatch();
        SetStatus(_T("Screen sharing active."));
        break;
    case ScreenShareProtocol::Busy:
        m_localCloseInProgress = true;
        m_socket.Close();
        m_localCloseInProgress = false;
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
        SetStatus(_T("The host is already sharing with another viewer."));
        break;
    case ScreenShareProtocol::TimedOut:
        m_localCloseInProgress = true;
        m_socket.Close();
        m_localCloseInProgress = false;
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
        SetStatus(_T("Host did not respond before the consent timer expired."));
        break;
    default:
        break;
    }
}

void CRemoteClientDlg::HandleRemoteSessionEnded(const CString& message)
{
    m_watchDialog.HideWatch();
    m_sessionActive = false;
    m_framePending = false;

    if (m_socket.IsConnected())
    {
        m_localCloseInProgress = true;
        m_socket.Close();
        m_localCloseInProgress = false;
    }

    GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
    SetStatus(message);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CONNECT, &CRemoteClientDlg::OnBnClickedConnect)
    ON_MESSAGE(WM_VIEWER_PACKET, &CRemoteClientDlg::OnViewerPacket)
    ON_MESSAGE(WM_VIEWER_SOCKET_CLOSED, &CRemoteClientDlg::OnViewerSocketClosed)
    ON_MESSAGE(WM_WATCH_REQUEST_FRAME, &CRemoteClientDlg::OnWatchRequestFrame)
    ON_MESSAGE(WM_WATCH_END_SESSION, &CRemoteClientDlg::OnWatchEndSession)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()
