#pragma once

class CSessionLog
{
public:
    static void AppendEvent(const CString& helperName, const CString& peerIp, const CString& eventType, const CString& detail);
    static CString ReadRecentSessions();

private:
    static CString GetLogPath();
    static CString EscapeField(const CString& value);
};
