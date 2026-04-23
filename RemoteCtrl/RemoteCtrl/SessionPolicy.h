#pragma once

#include <array>

enum class AssistStreamId
{
    Screen = 0,
    Microphone = 1,
    Camera = 2,
    Count = 3,
};

enum class AssistStreamConsent
{
    NotRequested,
    Approved,
};

class CSessionPolicy
{
public:
    static constexpr UINT_PTR kTimerId = 10;
    static constexpr UINT kTimerIntervalMs = 1000;
    static constexpr DWORD kDefaultLimitSeconds = 60 * 60;
    static constexpr DWORD kExtendPromptSeconds = 55 * 60;
    static constexpr DWORD kExtensionSeconds = 15 * 60;

    CSessionPolicy();

    void Start();
    void Stop();
    void Extend();
    void MarkExtensionPrompted();
    bool IsActive() const;
    bool ShouldPromptForExtension() const;
    bool IsExpired() const;
    DWORD ElapsedSeconds() const;
    DWORD RemainingSeconds() const;
    void SetStreamConsent(AssistStreamId streamId, AssistStreamConsent consent);
    AssistStreamConsent GetStreamConsent(AssistStreamId streamId) const;

private:
    static size_t StreamIndex(AssistStreamId streamId);

    std::array<AssistStreamConsent, static_cast<size_t>(AssistStreamId::Count)> m_streamConsent;
    ULONGLONG m_startedTick;
    DWORD m_allowedSeconds;
    bool m_active;
    bool m_extensionPrompted;
};
