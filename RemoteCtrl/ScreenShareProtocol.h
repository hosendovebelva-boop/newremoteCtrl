#pragma once

#include <Windows.h>
#include <string>

namespace ScreenShareProtocol
{
constexpr UINT kDefaultPort = 9527;
constexpr DWORD kFrameIntervalMs = 500;
constexpr size_t kSessionCodeLength = 6;
constexpr UINT kMaxSessionCodeAttempts = 3;

enum Command : WORD
{
    SendScreen = 6,
    SubmitSessionCode = 1001,
    SessionStatus = 1002,
    EndSession = 1003,
};

enum Status : BYTE
{
    BadCode = 0,
    CodeAcceptedWaitingConsent = 1,
    ConsentDenied = 2,
    ConsentGranted = 3,
    Busy = 4,
    SessionEnded = 5,
};

inline bool IsValidSessionCode(const std::string& code)
{
    if (code.size() != kSessionCodeLength)
    {
        return false;
    }

    for (char ch : code)
    {
        if (ch < '0' || ch > '9')
        {
            return false;
        }
    }

    return true;
}
}  // namespace ScreenShareProtocol
