#pragma once

#include "resource.h"
#include "..\ScreenShareProtocol.h"

#include "afxdialogex.h"
#include <atlimage.h>

constexpr UINT WM_WATCH_REQUEST_FRAME = WM_APP + 0x220;
constexpr UINT WM_WATCH_END_SESSION = WM_APP + 0x221;

class CWatchDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CWatchDialog)

public:
    explicit CWatchDialog(CWnd* parent = nullptr);
    ~CWatchDialog() override;

    enum
    {
        IDD = IDD_DIG_WATCH
    };

    bool EnsureCreated(CWnd* owner);
    void ShowWatch();
    void HideWatch();
    void UpdateFrame(const std::string& pngBytes);
    bool IsWatching() const;

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    void OnCancel() override;

    afx_msg void OnBnClickedEndSession();
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR eventId);

    DECLARE_MESSAGE_MAP()

private:
    void RenderFrame();

    CStatic m_picture;
    CImage m_image;
    HWND m_ownerWnd;
    bool m_internalClose;
};
