#include "pch.h"
#include "SessionPolicy.h"

CSessionPolicy::CSessionPolicy()
    : m_startedTick(0), m_allowedSeconds(kDefaultLimitSeconds), m_active(false), m_extensionPrompted(false)
{
    m_streamConsent.fill(AssistStreamConsent::NotRequested);
}

void CSessionPolicy::Start()
{
    m_startedTick = ::GetTickCount64();
    m_allowedSeconds = kDefaultLimitSeconds;
    m_active = true;
    m_extensionPrompted = false;
    m_streamConsent.fill(AssistStreamConsent::NotRequested);
    SetStreamConsent(AssistStreamId::Screen, AssistStreamConsent::Approved);
}

void CSessionPolicy::Stop()
{
    m_active = false;
    m_extensionPrompted = false;
    m_streamConsent.fill(AssistStreamConsent::NotRequested);
}

void CSessionPolicy::Extend()
{
    if (m_active)
    {
        m_allowedSeconds += kExtensionSeconds;
        m_extensionPrompted = false;
    }
}

void CSessionPolicy::MarkExtensionPrompted()
{
    m_extensionPrompted = true;
}

bool CSessionPolicy::IsActive() const
{
    return m_active;
}

bool CSessionPolicy::ShouldPromptForExtension() const
{
    return m_active && !m_extensionPrompted && ElapsedSeconds() >= kExtendPromptSeconds;
}

bool CSessionPolicy::IsExpired() const
{
    return m_active && ElapsedSeconds() >= m_allowedSeconds;
}

DWORD CSessionPolicy::ElapsedSeconds() const
{
    if (!m_active)
    {
        return 0;
    }

    return static_cast<DWORD>((::GetTickCount64() - m_startedTick) / 1000);
}

DWORD CSessionPolicy::RemainingSeconds() const
{
    const DWORD elapsedSeconds = ElapsedSeconds();
    return elapsedSeconds >= m_allowedSeconds ? 0 : m_allowedSeconds - elapsedSeconds;
}

void CSessionPolicy::SetStreamConsent(AssistStreamId streamId, AssistStreamConsent consent)
{
    m_streamConsent[StreamIndex(streamId)] = consent;
}

AssistStreamConsent CSessionPolicy::GetStreamConsent(AssistStreamId streamId) const
{
    return m_streamConsent[StreamIndex(streamId)];
}

size_t CSessionPolicy::StreamIndex(AssistStreamId streamId)
{
    return static_cast<size_t>(streamId);
}
