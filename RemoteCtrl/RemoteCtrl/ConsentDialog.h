#pragma once

#include "resource.h"
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

    void SetViewerIp(const CString& peerIp);

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;

    DECLARE_MESSAGE_MAP()

private:
    CString m_message;
};
