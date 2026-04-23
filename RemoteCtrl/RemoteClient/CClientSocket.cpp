#include "pch.h"
#include "CClientSocket.h"

#include <atlconv.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

CClientSocket::CClientSocket()
    : m_socket(INVALID_SOCKET),
      m_thread(nullptr),
      m_threadId(0),
      m_ownerWnd(nullptr),
      m_running(false),
      m_suppressCloseNotification(false)
{
}

CClientSocket::~CClientSocket()
{
    Close();
}

bool CClientSocket::Connect(const CString& hostIp, UINT port, HWND ownerWnd)
{
    Close();

    m_ownerWnd = ownerWnd;
    m_suppressCloseNotification = false;
    m_receiveBuffer.clear();

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(static_cast<u_short>(port));

    CT2A hostIpA(hostIp);
    if (InetPtonA(AF_INET, hostIpA, &serverAddress.sin_addr) != 1)
    {
        CloseSocket();
        return false;
    }

    if (connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
    {
        CloseSocket();
        return false;
    }

    m_running = true;
    m_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &CClientSocket::ThreadEntry, this, 0, &m_threadId));
    if (m_thread == nullptr)
    {
        CloseSocket();
        m_running = false;
        return false;
    }

    return true;
}

void CClientSocket::Close()
{
    m_running = false;
    m_suppressCloseNotification = true;
    CloseSocket();

    if (m_thread != nullptr)
    {
        if (GetCurrentThreadId() != m_threadId)
        {
            WaitForSingleObject(m_thread, 2000);
        }

        CloseHandle(m_thread);
        m_thread = nullptr;
        m_threadId = 0;
    }
}

bool CClientSocket::SendPacket(const CPacket& packet)
{
    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    const std::vector<char> bytes = packet.Serialize();
    std::lock_guard<std::mutex> guard(m_sendMutex);
    size_t sentLength = 0;
    while (sentLength < bytes.size())
    {
        const int chunkLength = send(m_socket, bytes.data() + sentLength, static_cast<int>(bytes.size() - sentLength), 0);
        if (chunkLength <= 0)
        {
            return false;
        }

        sentLength += static_cast<size_t>(chunkLength);
    }

    return true;
}

bool CClientSocket::IsConnected() const
{
    return m_socket != INVALID_SOCKET;
}

unsigned __stdcall CClientSocket::ThreadEntry(void* context)
{
    return static_cast<CClientSocket*>(context)->Run();
}

unsigned CClientSocket::Run()
{
    ViewerCloseReason closeReason = ViewerCloseReason::RemoteClosed;
    while (m_running)
    {
        char buffer[8192] = {};
        const int receivedLength = recv(m_socket, buffer, sizeof(buffer), 0);
        if (receivedLength <= 0)
        {
            break;
        }

        m_receiveBuffer.insert(m_receiveBuffer.end(), buffer, buffer + receivedLength);
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
                closeReason = ViewerCloseReason::ParseError;
                m_running = false;
                break;
            }

            m_receiveBuffer.erase(m_receiveBuffer.begin(), m_receiveBuffer.begin() + static_cast<std::ptrdiff_t>(consumed));
            if (::IsWindow(m_ownerWnd))
            {
                ::PostMessage(m_ownerWnd, WM_VIEWER_PACKET, 0, reinterpret_cast<LPARAM>(new CPacket(packet)));
            }
        }
    }

    const bool notifyClose = !m_suppressCloseNotification;
    m_running = false;
    CloseSocket();

    if (notifyClose && ::IsWindow(m_ownerWnd))
    {
        ::PostMessage(m_ownerWnd, WM_VIEWER_SOCKET_CLOSED, static_cast<WPARAM>(closeReason), 0);
    }

    return 0;
}

void CClientSocket::CloseSocket()
{
    if (m_socket != INVALID_SOCKET)
    {
        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}
