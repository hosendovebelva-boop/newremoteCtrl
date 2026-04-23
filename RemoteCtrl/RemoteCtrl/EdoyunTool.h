#pragma once

#include <atlconv.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <random>
#include <set>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

class CEdoyunTool
{
public:
    static CString GenerateSessionCode()
    {
        std::random_device seed;
        std::mt19937 engine(seed());
        std::uniform_int_distribution<int> distribution(0, 9);

        CString code;
        for (int index = 0; index < 6; ++index)
        {
            code.AppendChar(static_cast<TCHAR>('0' + distribution(engine)));
        }

        return code;
    }

    static CString JoinLocalIpv4Addresses()
    {
        const std::vector<CString> addresses = GetLocalIpv4Addresses();
        CString joined;
        for (size_t index = 0; index < addresses.size(); ++index)
        {
            if (index > 0)
            {
                joined += _T(", ");
            }
            joined += addresses[index];
        }

        return joined;
    }

private:
    static std::vector<CString> GetLocalIpv4Addresses()
    {
        std::vector<CString> result;
        char hostName[256] = {};
        if (gethostname(hostName, sizeof(hostName)) != 0)
        {
            result.push_back(_T("127.0.0.1"));
            return result;
        }

        addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        addrinfo* info = nullptr;
        if (getaddrinfo(hostName, nullptr, &hints, &info) != 0)
        {
            result.push_back(_T("127.0.0.1"));
            return result;
        }

        std::set<std::wstring> unique;
        for (addrinfo* current = info; current != nullptr; current = current->ai_next)
        {
            const sockaddr_in* address = reinterpret_cast<const sockaddr_in*>(current->ai_addr);
            WCHAR ipBuffer[INET_ADDRSTRLEN] = {};
            if (InetNtopW(AF_INET, const_cast<IN_ADDR*>(&address->sin_addr), ipBuffer, _countof(ipBuffer)) == nullptr)
            {
                continue;
            }

            CString ip(ipBuffer);
            if (ip == _T("127.0.0.1"))
            {
                continue;
            }

            unique.insert(std::wstring(ip.GetString()));
        }

        freeaddrinfo(info);

        for (const std::wstring& address : unique)
        {
            result.push_back(address.c_str());
        }

        if (result.empty())
        {
            result.push_back(_T("127.0.0.1"));
        }

        return result;
    }
};
