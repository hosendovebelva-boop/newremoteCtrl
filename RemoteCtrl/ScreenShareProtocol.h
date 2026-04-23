#pragma once

#include <Windows.h>
#include <string>

namespace ScreenShareProtocol
{
constexpr UINT kDefaultPort = 9527;
constexpr DWORD kFrameIntervalMs = 500;
constexpr size_t kSessionCodeLength = 6;
constexpr size_t kHelperNameMaxLength = 64;
constexpr UINT kConsentTimeoutSeconds = 30;
constexpr UINT kMaxSessionCodeAttempts = 3;

enum Command : WORD
{
    FrameRequest = 6,
    Hello = 1001,
    ConsentResult = 1002,
};

enum Status : BYTE
{
    BadCode = 0,
    WaitingForConsent = 1,
    Denied = 2,
    Approved = 3,
    Busy = 4,
    TimedOut = 5,
};

struct HelloPayload
{
    std::string sessionCode;
    std::string helperName;
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

inline bool IsValidHelperName(const std::string& helperName)
{
    if (helperName.empty() || helperName.find('\r') != std::string::npos || helperName.find('\n') != std::string::npos)
    {
        return false;
    }

    const int wideLength = ::MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        helperName.data(),
        static_cast<int>(helperName.size()),
        nullptr,
        0);
    return wideLength > 0 && wideLength <= static_cast<int>(kHelperNameMaxLength);
}

inline bool BuildHelloPayload(const std::string& sessionCode, const std::string& helperName, std::string& payload)
{
    if (!IsValidSessionCode(sessionCode) || !IsValidHelperName(helperName))
    {
        return false;
    }

    payload = sessionCode;
    payload.push_back('\n');
    payload += helperName;
    return true;
}

inline bool ParseHelloPayload(const std::string& payload, HelloPayload& hello)
{
    const size_t separator = payload.find('\n');
    if (separator == std::string::npos || payload.find('\n', separator + 1) != std::string::npos || payload.find('\r') != std::string::npos)
    {
        return false;
    }

    hello.sessionCode = payload.substr(0, separator);
    hello.helperName = payload.substr(separator + 1);
    return IsValidSessionCode(hello.sessionCode) && IsValidHelperName(hello.helperName);
}
}  // namespace ScreenShareProtocol
