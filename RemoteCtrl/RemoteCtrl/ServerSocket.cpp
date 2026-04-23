#include "pch.h"
#include "ServerSocket.h"
#include "ScreenCapture.h"

#include <atlconv.h>
#include <process.h>

namespace
{
CString GetLocalHostName()
{
    WCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1] = {};
    DWORD length = _countof(buffer);
    if (::GetComputerNameW(buffer, &length))
    {
        return CString(buffer);
    }

    return _T("Unknown host");
}
}

CServerSocket::CServerSocket()
    : m_ownerWnd(nullptr),
      m_thread(nullptr),
      m_listenSocket(INVALID_SOCKET),
      m_clientSocket(INVALID_SOCKET),
      m_port(ScreenShareProtocol::kDefaultPort),
      m_running(false),
      m_sessionState(SessionState::Idle),
      m_wrongAttempts(0),
      m_endSessionRequested(false)
{
}

CServerSocket::~CServerSocket()
{
    Stop();
}

bool CServerSocket::Start(HWND owner, const CString& sessionCode, UINT port)
{
    Stop();

    m_ownerWnd = owner;
    m_port = port;
    m_sessionCode = sessionCode;
    m_running = true;
    m_sessionState = SessionState::Idle;
    m_wrongAttempts = 0;
    m_endSessionRequested = false;
    m_helperName.Empty();
    m_endSessionDetail = _T("Session ended locally.");
    m_receiveBuffer.clear();

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        m_running = false;
        return false;
    }

    BOOL exclusive = TRUE;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive), sizeof(exclusive));

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<u_short>(port));

    if (bind(m_listenSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
        listen(m_listenSocket, 1) == SOCKET_ERROR)
    {
        CloseSocket(m_listenSocket);
        m_running = false;
        return false;
    }

    unsigned threadId = 0;
    m_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &CServerSocket::ThreadEntry, this, 0, &threadId));
    if (m_thread == nullptr)
    {
        CloseSocket(m_listenSocket);
        m_running = false;
        return false;
    }

    PostEvent(HostServerEventType::Listening, CString(), CString(), _T("Waiting for viewer"));
    return true;
}

void CServerSocket::Stop()
{
    m_running = false;
    CloseSocket(m_clientSocket);
    CloseSocket(m_listenSocket);

    if (m_thread != nullptr)
    {
        ::WaitForSingleObject(m_thread, 2000);
        ::CloseHandle(m_thread);
        m_thread = nullptr;
    }

    ResetClient();
}

void CServerSocket::EndSession(const CString& detail)
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    if (m_clientSocket != INVALID_SOCKET)
    {
        m_endSessionRequested = true;
        m_endSessionDetail = detail;
    }
}

void CServerSocket::SetSessionCode(const CString& sessionCode)
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    m_sessionCode = sessionCode;
}

CString CServerSocket::GetPeerIp() const
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    return m_peerIp;
}

CString CServerSocket::GetHelperName() const
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    return m_helperName;
}

bool CServerSocket::IsSharing() const
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    return m_sessionState == SessionState::Sharing;
}

unsigned __stdcall CServerSocket::ThreadEntry(void* context)
{
    return static_cast<CServerSocket*>(context)->Run();
}

unsigned CServerSocket::Run()
{
    while (m_running)
    {
        fd_set readSet;
        FD_ZERO(&readSet);

        if (m_listenSocket != INVALID_SOCKET)
        {
            FD_SET(m_listenSocket, &readSet);
        }

        if (m_clientSocket != INVALID_SOCKET)
        {
            FD_SET(m_clientSocket, &readSet);
        }

        timeval timeout = {};
        timeout.tv_usec = 200000;
        const int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
        if (!m_running)
        {
            break;
        }

        if (selectResult == SOCKET_ERROR)
        {
            PostEvent(HostServerEventType::Error, GetPeerIp(), CString(), _T("Socket loop failed."));
            break;
        }

        if (m_endSessionRequested && m_clientSocket != INVALID_SOCKET)
        {
            const CString peerIp = GetPeerIp();
            const CString helperName = GetHelperName();
            CString detail;
            {
                std::lock_guard<std::mutex> guard(m_stateMutex);
                detail = m_endSessionDetail;
            }
            ResetClient();
            PostEvent(HostServerEventType::SessionEnded, peerIp, helperName, detail);
            continue;
        }

        if (m_listenSocket != INVALID_SOCKET && FD_ISSET(m_listenSocket, &readSet))
        {
            AcceptClient();
        }

        if (m_clientSocket != INVALID_SOCKET && FD_ISSET(m_clientSocket, &readSet))
        {
            if (!ReceiveFromClient())
            {
                const CString peerIp = GetPeerIp();
                const CString helperName = GetHelperName();
                ResetClient();
                PostEvent(HostServerEventType::SessionEnded, peerIp, helperName, _T("Viewer disconnected."));
            }
        }
    }

    CloseSocket(m_clientSocket);
    CloseSocket(m_listenSocket);
    return 0;
}

void CServerSocket::AcceptClient()
{
    sockaddr_in clientAddress = {};
    int clientLength = sizeof(clientAddress);
    SOCKET acceptedSocket = accept(m_listenSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength);
    if (acceptedSocket == INVALID_SOCKET)
    {
        return;
    }

    if (m_clientSocket != INVALID_SOCKET)
    {
        const BYTE busy = static_cast<BYTE>(ScreenShareProtocol::Busy);
        CPacket busyPacket(ScreenShareProtocol::ConsentResult, &busy, sizeof(busy));
        SendPacket(acceptedSocket, busyPacket);
        CloseSocket(acceptedSocket);
        return;
    }

    char ipBuffer[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &clientAddress.sin_addr, ipBuffer, sizeof(ipBuffer));

    {
        std::lock_guard<std::mutex> guard(m_stateMutex);
        m_clientSocket = acceptedSocket;
        m_peerIp = CA2T(ipBuffer);
        m_helperName.Empty();
        m_sessionState = SessionState::AwaitingHello;
        m_wrongAttempts = 0;
        m_receiveBuffer.clear();
    }

    PostEvent(HostServerEventType::Listening, GetPeerIp(), CString(), _T("Incoming viewer connection"));
}

bool CServerSocket::ReceiveFromClient()
{
    char receiveBuffer[8192] = {};
    const int receivedLength = recv(m_clientSocket, receiveBuffer, sizeof(receiveBuffer), 0);
    if (receivedLength <= 0)
    {
        return false;
    }

    m_receiveBuffer.insert(m_receiveBuffer.end(), receiveBuffer, receiveBuffer + receivedLength);

    while (!m_receiveBuffer.empty())
    {
        CPacket packet;
        size_t consumed = 0;
        const PacketParseResult result = CPacket::TryParse(m_receiveBuffer.data(), m_receiveBuffer.size(), packet, consumed);
        if (result == PacketParseResult::NeedMoreData)
        {
            break;
        }

        if (result == PacketParseResult::Invalid)
        {
            return false;
        }

        m_receiveBuffer.erase(m_receiveBuffer.begin(), m_receiveBuffer.begin() + static_cast<std::ptrdiff_t>(consumed));
        HandlePacket(packet);
        if (m_clientSocket == INVALID_SOCKET)
        {
            break;
        }
    }

    return true;
}

void CServerSocket::HandlePacket(const CPacket& packet)
{
    if (m_sessionState == SessionState::AwaitingHello)
    {
        if (packet.Command() != ScreenShareProtocol::Hello)
        {
            ResetClient();
            PostEvent(HostServerEventType::SessionEnded, CString(), CString(), _T("Viewer sent an unexpected command."));
            return;
        }

        ScreenShareProtocol::HelloPayload hello;
        if (!ScreenShareProtocol::ParseHelloPayload(packet.Payload(), hello))
        {
            const CString peerIp = GetPeerIp();
            ResetClient();
            PostEvent(HostServerEventType::SessionEnded, peerIp, CString(), _T("Viewer sent an invalid hello payload."));
            return;
        }

        const CString expectedCode = CurrentSessionCode();
        const CString submittedCode(CA2T(hello.sessionCode.c_str()));
        if (submittedCode != expectedCode)
        {
            ++m_wrongAttempts;
            ::Sleep(500);
            SendStatus(ScreenShareProtocol::BadCode);
            if (m_wrongAttempts >= ScreenShareProtocol::kMaxSessionCodeAttempts)
            {
                const CString peerIp = GetPeerIp();
                const CString helperName(CA2W(hello.helperName.c_str(), CP_UTF8));
                ResetClient();
                PostEvent(HostServerEventType::SessionEnded, peerIp, helperName, _T("Too many incorrect session codes."));
            }
            return;
        }

        const CString helperName(CA2W(hello.helperName.c_str(), CP_UTF8));
        {
            std::lock_guard<std::mutex> guard(m_stateMutex);
            m_helperName = helperName;
            m_sessionState = SessionState::WaitingForConsent;
        }
        SendStatus(ScreenShareProtocol::WaitingForConsent);
        PostEvent(HostServerEventType::WaitingForConsent, GetPeerIp(), helperName, _T("Waiting for local approval"));

        const ScreenShareProtocol::Status consentStatus = RequestConsent();
        if (consentStatus == ScreenShareProtocol::Approved)
        {
            {
                std::lock_guard<std::mutex> guard(m_stateMutex);
                m_sessionState = SessionState::Sharing;
            }
            SendStatus(ScreenShareProtocol::Approved);
            PostEvent(HostServerEventType::SharingStarted, GetPeerIp(), helperName, _T("Screen sharing active"));
        }
        else
        {
            SendStatus(consentStatus);
            const CString peerIp = GetPeerIp();
            ResetClient();
            const CString detail =
                (consentStatus == ScreenShareProtocol::TimedOut) ? _T("Consent request timed out.") : _T("Local user denied the request.");
            PostEvent(HostServerEventType::SessionEnded, peerIp, helperName, detail);
        }
        return;
    }

    if (m_sessionState == SessionState::Sharing)
    {
        if (packet.Command() == ScreenShareProtocol::FrameRequest && packet.Payload().empty())
        {
            std::string pngBytes;
            if (CapturePrimaryMonitorPng(pngBytes))
            {
                SendPacket(m_clientSocket, CPacket(ScreenShareProtocol::FrameRequest, pngBytes));
            }
            return;
        }

        const CString peerIp = GetPeerIp();
        const CString helperName = GetHelperName();
        ResetClient();
        PostEvent(HostServerEventType::SessionEnded, peerIp, helperName, _T("Viewer sent an unexpected command."));
    }
}

ScreenShareProtocol::Status CServerSocket::RequestConsent()
{
    if (!::IsWindow(m_ownerWnd))
    {
        return ScreenShareProtocol::Denied;
    }

    ConsentRequest* request = new ConsentRequest(GetPeerIp(), CurrentHelperName(), CurrentSessionCode(), GetLocalHostName());
    if (!::PostMessage(m_ownerWnd, WM_HOST_CONSENT_REQUEST, 0, reinterpret_cast<LPARAM>(request)))
    {
        delete request;
        return ScreenShareProtocol::Denied;
    }

    ::WaitForSingleObject(request->completedEvent, INFINITE);
    const ScreenShareProtocol::Status result = request->result;
    delete request;
    return result;
}

void CServerSocket::ResetClient()
{
    CloseSocket(m_clientSocket);
    std::lock_guard<std::mutex> guard(m_stateMutex);
    m_peerIp.Empty();
    m_helperName.Empty();
    m_sessionState = SessionState::Idle;
    m_wrongAttempts = 0;
    m_endSessionRequested = false;
    m_endSessionDetail = _T("Session ended locally.");
    m_receiveBuffer.clear();
}

void CServerSocket::CloseSocket(SOCKET& socket)
{
    if (socket != INVALID_SOCKET)
    {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
}

bool CServerSocket::SendPacket(SOCKET socket, const CPacket& packet)
{
    if (socket == INVALID_SOCKET)
    {
        return false;
    }

    const std::vector<char> bytes = packet.Serialize();
    size_t sentLength = 0;
    std::lock_guard<std::mutex> guard(m_sendMutex);
    while (sentLength < bytes.size())
    {
        const int chunkLength = send(socket, bytes.data() + sentLength, static_cast<int>(bytes.size() - sentLength), 0);
        if (chunkLength <= 0)
        {
            return false;
        }

        sentLength += static_cast<size_t>(chunkLength);
    }

    return true;
}

bool CServerSocket::SendStatus(ScreenShareProtocol::Status status)
{
    const BYTE rawStatus = static_cast<BYTE>(status);
    return SendPacket(m_clientSocket, CPacket(ScreenShareProtocol::ConsentResult, &rawStatus, sizeof(rawStatus)));
}

CString CServerSocket::CurrentSessionCode() const
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    return m_sessionCode;
}

CString CServerSocket::CurrentHelperName() const
{
    std::lock_guard<std::mutex> guard(m_stateMutex);
    return m_helperName;
}

void CServerSocket::PostEvent(const HostServerEventType type, const CString& peerIp, const CString& helperName, const CString& detail) const
{
    if (!::IsWindow(m_ownerWnd))
    {
        return;
    }

    HostServerEventPayload* payload = new HostServerEventPayload{type, peerIp, helperName, detail};
    ::PostMessage(m_ownerWnd, WM_HOST_SERVER_EVENT, 0, reinterpret_cast<LPARAM>(payload));
}
