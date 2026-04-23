#pragma once

#include "..\\ScreenShareProtocol.h"
#include "..\\SharedPacket.h"

#include <afxdialogex.h>

#include <atomic>
#include <mutex>
#include <vector>

constexpr UINT WM_HOST_SERVER_EVENT = WM_APP + 0x100;
constexpr UINT WM_HOST_CONSENT_REQUEST = WM_APP + 0x101;
constexpr UINT WM_HOST_BANNER_END_SESSION = WM_APP + 0x102;
constexpr UINT WM_HOST_TRAYICON = WM_APP + 0x103;

enum class HostServerEventType
{
    Listening,
    WaitingForConsent,
    SharingStarted,
    SessionEnded,
    Error,
};

struct HostServerEventPayload
{
    HostServerEventType type;
    CString peerIp;
    CString helperName;
    CString detail;
};

struct ConsentRequest
{
    ConsentRequest(const CString& viewerIp, const CString& viewerName, const CString& codeText, const CString& hostMachineName)
        : peerIp(viewerIp),
          helperName(viewerName),
          sessionCode(codeText),
          hostName(hostMachineName),
          completedEvent(::CreateEvent(nullptr, TRUE, FALSE, nullptr)),
          result(ScreenShareProtocol::Denied)
    {
    }

    ~ConsentRequest()
    {
        if (completedEvent != nullptr)
        {
            ::CloseHandle(completedEvent);
            completedEvent = nullptr;
        }
    }

    CString peerIp;
    CString helperName;
    CString sessionCode;
    CString hostName;
    HANDLE completedEvent;
    ScreenShareProtocol::Status result;
};

class CServerSocket
{
public:
    CServerSocket();
    ~CServerSocket();

    bool Start(HWND owner, const CString& sessionCode, UINT port = ScreenShareProtocol::kDefaultPort);
    void Stop();
    void EndSession(const CString& detail = _T("Session ended locally."));
    void SetSessionCode(const CString& sessionCode);
    CString GetPeerIp() const;
    CString GetHelperName() const;
    bool IsSharing() const;

private:
    enum class SessionState
    {
        Idle,
        AwaitingHello,
        WaitingForConsent,
        Sharing,
    };

    static unsigned __stdcall ThreadEntry(void* context);
    unsigned Run();
    void AcceptClient();
    bool ReceiveFromClient();
    void HandlePacket(const CPacket& packet);
    ScreenShareProtocol::Status RequestConsent();
    void ResetClient();
    void CloseSocket(SOCKET& socket);
    bool SendPacket(SOCKET socket, const CPacket& packet);
    bool SendStatus(ScreenShareProtocol::Status status);
    CString CurrentSessionCode() const;
    CString CurrentHelperName() const;
    void PostEvent(
        HostServerEventType type,
        const CString& peerIp = CString(),
        const CString& helperName = CString(),
        const CString& detail = CString()) const;

    HWND m_ownerWnd;
    HANDLE m_thread;
    SOCKET m_listenSocket;
    SOCKET m_clientSocket;
    UINT m_port;
    std::atomic_bool m_running;
    SessionState m_sessionState;
    UINT m_wrongAttempts;
    std::atomic_bool m_endSessionRequested;
    CString m_peerIp;
    CString m_helperName;
    CString m_sessionCode;
    CString m_endSessionDetail;
    std::vector<char> m_receiveBuffer;
    mutable std::mutex m_stateMutex;
    std::mutex m_sendMutex;
};
