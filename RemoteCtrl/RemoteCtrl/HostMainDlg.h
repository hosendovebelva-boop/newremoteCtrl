#pragma once

#include "resource.h"
#include "ServerSocket.h"
#include "ShareBannerWnd.h"

#include "afxdialogex.h"

class CHostMainDlg : public CDialogEx
{
public:
    explicit CHostMainDlg(CWnd* parent = nullptr);

    enum
    {
        IDD = IDD_DIALOG_HOST_MAIN
    };

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    void OnCancel() override;

    afx_msg void OnBnClickedStopSharing();
    afx_msg LRESULT OnServerEvent(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnConsentRequest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnBannerEndSession(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
    enum
    {
        ID_TRAY_SHOW = 40001,
        ID_TRAY_STOP = 40002,
        ID_TRAY_EXIT = 40003,
    };

    void RefreshSessionCode();
    void UpdateStatus(const CString& statusText);
    void ConfigureTrayIcon();
    void RemoveTrayIcon();
    void ShowTrayMenu();

    HICON m_icon;
    CServerSocket m_server;
    CShareBannerWnd m_banner;
    NOTIFYICONDATA m_trayData;
    CString m_sessionCode;
};
