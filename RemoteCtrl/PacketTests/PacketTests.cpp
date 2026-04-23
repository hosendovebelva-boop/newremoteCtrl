#include <iostream>
#include <string>
#include <vector>

#include "..\ScreenShareProtocol.h"
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

std::string BuildHello()
{
    std::string payload;
    const bool ok = ScreenShareProtocol::BuildHelloPayload("123456", "Helper One", payload);
    Expect(ok, "valid hello should be constructed");
    return payload;
}

void TestHelloPayload()
{
    const std::string payload = BuildHello();
    ScreenShareProtocol::HelloPayload hello;
    const bool parsed = ScreenShareProtocol::ParseHelloPayload(payload, hello);

    Expect(parsed, "valid hello payload should parse");
    Expect(hello.sessionCode == "123456", "hello payload should preserve the session code");
    Expect(hello.helperName == "Helper One", "hello payload should preserve the helper name");
}

void TestHelloMalformedPayloads()
{
    ScreenShareProtocol::HelloPayload hello;

    Expect(!ScreenShareProtocol::ParseHelloPayload("12345\nHelper", hello), "hello should reject short session codes");
    Expect(!ScreenShareProtocol::ParseHelloPayload("12A456\nHelper", hello), "hello should reject non-digit session codes");
    Expect(!ScreenShareProtocol::ParseHelloPayload("123456\n", hello), "hello should reject empty helper names");
    Expect(!ScreenShareProtocol::ParseHelloPayload("123456\nHelper\nAgain", hello), "hello should reject helper names with newlines");
}

void TestCompletePacket()
{
    const std::string payload = BuildHello();
    const CPacket original(ScreenShareProtocol::Hello, payload);
    const std::vector<char> bytes = original.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Complete, "complete packet should parse");
    Expect(consumed == bytes.size(), "complete packet should consume all bytes");
    Expect(parsed.Command() == ScreenShareProtocol::Hello, "complete packet should preserve command");
    Expect(parsed.Payload() == payload, "complete packet should preserve payload");
}

void TestFragmentedHeader()
{
    const std::string payload = BuildHello();
    const CPacket packet(ScreenShareProtocol::Hello, payload);
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), 4, parsed, consumed);

    Expect(result == PacketParseResult::NeedMoreData, "fragmented header should require more data");
    Expect(consumed == 0, "fragmented header should not consume bytes");
}

void TestFragmentedHelloPayload()
{
    const std::string payload = BuildHello();
    const CPacket packet(ScreenShareProtocol::Hello, payload);
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size() - 2, parsed, consumed);

    Expect(result == PacketParseResult::NeedMoreData, "fragmented hello payload should require more data");
    Expect(consumed == 0, "fragmented hello payload should not consume bytes");
}

void TestFragmentedFramePayload()
{
    const CPacket packet(ScreenShareProtocol::FrameRequest, std::string("png-bytes"));
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size() - 2, parsed, consumed);

    Expect(result == PacketParseResult::NeedMoreData, "fragmented frame payload should require more data");
    Expect(consumed == 0, "fragmented frame payload should not consume bytes");
}

void TestConcatenatedPackets()
{
    const CPacket first(ScreenShareProtocol::Hello, BuildHello());
    const CPacket second(ScreenShareProtocol::ConsentResult, std::string(1, static_cast<char>(ScreenShareProtocol::Approved)));
    std::vector<char> bytes = first.Serialize();
    const std::vector<char> secondBytes = second.Serialize();
    bytes.insert(bytes.end(), secondBytes.begin(), secondBytes.end());

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Complete, "first packet in concatenated buffer should parse");
    Expect(consumed == first.Serialize().size(), "concatenated buffer should consume only the first packet");
    Expect(parsed.Command() == ScreenShareProtocol::Hello, "first packet command should be preserved");
}

void TestBadChecksum()
{
    const CPacket packet(ScreenShareProtocol::Hello, BuildHello());
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
    const CPacket packet(ScreenShareProtocol::FrameRequest, nullptr, 0);
    const std::vector<char> bytes = packet.Serialize();

    CPacket parsed;
    size_t consumed = 0;
    const PacketParseResult result = CPacket::TryParse(bytes.data(), bytes.size(), parsed, consumed);

    Expect(result == PacketParseResult::Complete, "empty payload packet should parse");
    Expect(parsed.Command() == ScreenShareProtocol::FrameRequest, "empty payload packet should preserve command");
    Expect(parsed.Payload().empty(), "empty payload packet should keep an empty payload");
}
}  // namespace

int main()
{
    TestHelloPayload();
    TestHelloMalformedPayloads();
    TestCompletePacket();
    TestFragmentedHeader();
    TestFragmentedHelloPayload();
    TestFragmentedFramePayload();
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
