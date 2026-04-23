#include <iostream>
#include <string>
#include <vector>

#include "..\SharedPacket.h"

namespace
{
int g_failures = 0;

void Expect(bool condition, const char* message)
{
    if (!condition)
    {
        ++g_failures;
        std::cerr << "[FAIL] " << message << std::endl;
    }
}

void TestCompletePacket()
{
    const std::string payload = "123456";
    const CPacket original(1001, payload);
    const std::vector<char> bytes = original.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Complete, "complete packet should parse");
    Expect(consumed == bytes.size(), "complete packet should consume all bytes");
    Expect(parsed.Command() == 1001, "complete packet should preserve command");
    Expect(parsed.Payload() == payload, "complete packet should preserve payload");
}

void TestFragmentedHeader()
{
    const std::string payload = "123456";
    const CPacket packet(1001, payload);
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), 4, parsed, consumed);

    Expect(result == PacketParseResult::NeedMoreData, "fragmented header should require more data");
    Expect(consumed == 0, "fragmented header should not consume bytes");
}

void TestFragmentedPayload()
{
    const std::string payload = "payload";
    const CPacket packet(6, payload);
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size() - 2, parsed, consumed);

    Expect(result == PacketParseResult::NeedMoreData, "fragmented payload should require more data");
    Expect(consumed == 0, "fragmented payload should not consume bytes");
}

void TestConcatenatedPackets()
{
    const CPacket first(1001, std::string("111111"));
    const CPacket second(1002, std::string(1, static_cast<char>(3)));
    std::vector<char> bytes = first.Serialize();
    const std::vector<char> secondBytes = second.Serialize();
    bytes.insert(bytes.end(), secondBytes.begin(), secondBytes.end());

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Complete, "first packet in concatenated buffer should parse");
    Expect(consumed == first.Serialize().size(), "concatenated buffer should consume only the first packet");
    Expect(parsed.Command() == 1001, "first packet command should be preserved");
}

void TestBadChecksum()
{
    const CPacket packet(1001, std::string("123456"));
    std::vector<char> bytes = packet.Serialize();
    bytes.back() ^= 0xFF;

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Invalid, "bad checksum should be rejected");
    Expect(consumed == bytes.size(), "bad checksum should consume the malformed packet");
}

void TestEmptyPayload()
{
    const CPacket packet(1003, nullptr, 0);
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Complete, "empty payload packet should parse");
    Expect(parsed.Command() == 1003, "empty payload packet should preserve command");
    Expect(parsed.Payload().empty(), "empty payload packet should keep an empty payload");
}
}  // namespace

int main()
{
    TestCompletePacket();
    TestFragmentedHeader();
    TestFragmentedPayload();
    TestConcatenatedPackets();
    TestBadChecksum();
    TestEmptyPayload();

    if (g_failures == 0)
    {
        std::cout << "PacketTests passed." << std::endl;
        return 0;
    }

    std::cerr << "PacketTests failed: " << g_failures << std::endl;
    return 1;
}
