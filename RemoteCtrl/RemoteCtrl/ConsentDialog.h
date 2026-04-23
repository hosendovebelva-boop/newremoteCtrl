#pragma once

#include "resource.h"
#include "..\ScreenShareProtocol.h"
#include "afxdialogex.h"

class CConsentDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CConsentDialog)

public:
    explicit CConsentDialog(CWnd* parent = nullptr);
    ~CConsentDialog() override;

    enum
    {
        IDD = IDD_DIALOG_CONSENT
    };

    void SetRequestDetails(const CString& peerIp, const CString& helperName, const CString& sessionCode, const CString& hostName);
    bool WasTimedOut() const;

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    afx_msg void OnTimer(UINT_PTR eventId);

    DECLARE_MESSAGE_MAP()

private:
    void UpdateCountdownText();

    CString m_viewerIp;
    CString m_helperName;
    CString m_sessionCode;
    CString m_hostName;
    bool m_timedOut;
    UINT m_secondsRemaining;
};
