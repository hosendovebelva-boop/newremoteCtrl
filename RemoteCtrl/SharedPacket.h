#pragma once

#include <Windows.h>

#include <cstring>
#include <string>
#include <vector>

enum class PacketParseResult
{
    NeedMoreData,
    Complete,
    Invalid,
};

class CPacket
{
public:
    static constexpr WORD kMagic = 0xFEFF;

    CPacket() : m_command(0) {}

    CPacket(WORD command, const void* data, size_t size) : m_command(command)
    {
        if (data != nullptr && size > 0)
        {
            m_payload.assign(static_cast<const char*>(data), static_cast<const char*>(data) + size);
        }
    }

    explicit CPacket(WORD command, const std::string& payload) : m_command(command), m_payload(payload) {}

    WORD Command() const
    {
        return m_command;
    }

    const std::string& Payload() const
    {
        return m_payload;
    }

    std::vector<char> Serialize() const
    {
        const DWORD payloadLength = static_cast<DWORD>(m_payload.size());
        const DWORD bodyLength = payloadLength + sizeof(WORD) + sizeof(WORD);
        std::vector<char> bytes(sizeof(WORD) + sizeof(DWORD) + bodyLength, 0);
        char* cursor = bytes.data();

        const WORD magic = kMagic;
        memcpy(cursor, &magic, sizeof(magic));
        cursor += sizeof(WORD);
        memcpy(cursor, &bodyLength, sizeof(bodyLength));
        cursor += sizeof(DWORD);
        memcpy(cursor, &m_command, sizeof(m_command));
        cursor += sizeof(WORD);

        if (!m_payload.empty())
        {
            memcpy(cursor, m_payload.data(), m_payload.size());
            cursor += m_payload.size();
        }

        const WORD checksum = CalculateChecksum(m_payload.data(), m_payload.size());
        memcpy(cursor, &checksum, sizeof(checksum));
        return bytes;
    }

    static PacketParseResult TryParse(const char* buffer, size_t bufferSize, CPacket& packet, size_t& consumed)
    {
        consumed = 0;
        if (buffer == nullptr || bufferSize < HeaderSize())
        {
            return PacketParseResult::NeedMoreData;
        }

        WORD magic = 0;
        memcpy(&magic, buffer, sizeof(magic));
        if (magic != kMagic)
        {
            return PacketParseResult::Invalid;
        }

        DWORD bodyLength = 0;
        memcpy(&bodyLength, buffer + sizeof(WORD), sizeof(bodyLength));
        const size_t packetSize = sizeof(WORD) + sizeof(DWORD) + bodyLength;
        if (bodyLength < sizeof(WORD) + sizeof(WORD))
        {
            return PacketParseResult::Invalid;
        }

        if (bufferSize < packetSize)
        {
            return PacketParseResult::NeedMoreData;
        }

        WORD command = 0;
        memcpy(&command, buffer + sizeof(WORD) + sizeof(DWORD), sizeof(command));
        const size_t payloadLength = bodyLength - sizeof(WORD) - sizeof(WORD);
        const char* payload = buffer + sizeof(WORD) + sizeof(DWORD) + sizeof(WORD);
        WORD checksum = 0;
        memcpy(&checksum, payload + payloadLength, sizeof(checksum));
        const WORD actualChecksum = CalculateChecksum(payload, payloadLength);
        if (checksum != actualChecksum)
        {
            consumed = packetSize;
            return PacketParseResult::Invalid;
        }

        packet = CPacket(command, payload, payloadLength);
        consumed = packetSize;
        return PacketParseResult::Complete;
    }

    static size_t HeaderSize()
    {
        return sizeof(WORD) + sizeof(DWORD) + sizeof(WORD) + sizeof(WORD);
    }

private:
    static WORD CalculateChecksum(const char* payload, size_t payloadLength)
    {
        WORD checksum = 0;
        for (size_t index = 0; index < payloadLength; ++index)
        {
            checksum = static_cast<WORD>(checksum + static_cast<BYTE>(payload[index]));
        }
        return checksum;
    }

    WORD m_command;
    std::string m_payload;
};
