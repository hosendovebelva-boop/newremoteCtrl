#include "pch.h"
#include "SessionLog.h"

namespace
{
CString FormatTimestamp()
{
    SYSTEMTIME localTime = {};
    ::GetLocalTime(&localTime);

    CString timestamp;
    timestamp.Format(
        _T("%04u-%02u-%02u %02u:%02u:%02u"),
        localTime.wYear,
        localTime.wMonth,
        localTime.wDay,
        localTime.wHour,
        localTime.wMinute,
        localTime.wSecond);
    return timestamp;
}
}

void CSessionLog::AppendEvent(const CString& helperName, const CString& peerIp, const CString& eventType, const CString& detail)
{
    const CString logPath = GetLogPath();
    const int slashIndex = logPath.ReverseFind(_T('\\'));
    if (slashIndex > 0)
    {
        ::CreateDirectory(logPath.Left(slashIndex), nullptr);
    }

    CStdioFile file;
    if (!file.Open(logPath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::typeText))
    {
        return;
    }

    file.SeekToEnd();
    CString line;
    line.Format(
        _T("%s\t%s\t%s\t%s\t%s\n"),
        FormatTimestamp().GetString(),
        EscapeField(helperName).GetString(),
        EscapeField(peerIp).GetString(),
        EscapeField(eventType).GetString(),
        EscapeField(detail).GetString());
    file.WriteString(line);
}

CString CSessionLog::ReadRecentSessions()
{
    CStdioFile file;
    if (!file.Open(GetLogPath(), CFile::modeRead | CFile::typeText))
    {
        return _T("No session history yet.");
    }

    CString content;
    CString line;
    while (file.ReadString(line))
    {
        content += line;
        content += _T("\r\n");
    }

    return content.IsEmpty() ? _T("No session history yet.") : content;
}

CString CSessionLog::GetLogPath()
{
    TCHAR localAppData[MAX_PATH] = {};
    const DWORD length = ::GetEnvironmentVariable(_T("LOCALAPPDATA"), localAppData, _countof(localAppData));
    CString basePath = (length > 0 && length < _countof(localAppData)) ? CString(localAppData) : CString(_T("."));
    if (!basePath.IsEmpty() && basePath[basePath.GetLength() - 1] != _T('\\'))
    {
        basePath += _T("\\");
    }

    return basePath + _T("AssistHost\\sessions.log");
}

CString CSessionLog::EscapeField(const CString& value)
{
    CString escaped(value);
    escaped.Replace(_T("\r"), _T(" "));
    escaped.Replace(_T("\n"), _T(" "));
    escaped.Replace(_T("\t"), _T(" "));
    return escaped;
}
