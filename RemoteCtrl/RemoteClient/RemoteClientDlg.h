#pragma once

#include "resource.h"
#include "CClientSocket.h"
#include "CWatchDialog.h"

class CRemoteClientDlg : public CDialogEx
{
public:
    explicit CRemoteClientDlg(CWnd* pParent = nullptr);

    enum
    {
        IDD = IDD_REMOTECLIENT_DIALOG
    };

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    void OnCancel() override;

    afx_msg void OnBnClickedConnect();
    afx_msg LRESULT OnViewerPacket(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnViewerSocketClosed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWatchRequestFrame(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWatchEndSession(WPARAM wParam, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();

    DECLARE_MESSAGE_MAP()

private:
    CString GetServerIpString() const;
    void SetStatus(const CString& statusText);
    void HandleSessionStatus(BYTE status);
    void HandleRemoteSessionEnded(const CString& message);

    HICON m_hIcon;
    DWORD m_serverAddress;
    CString m_portText;
    CString m_sessionCode;
    CClientSocket m_socket;
    CWatchDialog m_watchDialog;
    bool m_framePending;
    bool m_sessionActive;
    bool m_localCloseInProgress;
};
