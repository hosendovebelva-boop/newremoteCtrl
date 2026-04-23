#include "pch.h"
#include "ConsentDialog.h"

IMPLEMENT_DYNAMIC(CConsentDialog, CDialogEx)

CConsentDialog::CConsentDialog(CWnd* parent)
    : CDialogEx(IDD_DIALOG_CONSENT, parent), m_timedOut(false), m_secondsRemaining(ScreenShareProtocol::kConsentTimeoutSeconds)
{
}

CConsentDialog::~CConsentDialog() = default;

void CConsentDialog::SetRequestDetails(const CString& peerIp, const CString& helperName, const CString& sessionCode, const CString& hostName)
{
    m_viewerIp = peerIp;
    m_helperName = helperName;
    m_sessionCode = sessionCode;
    m_hostName = hostName;
}

bool CConsentDialog::WasTimedOut() const
{
    return m_timedOut;
}

void CConsentDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CConsentDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(_T("Remote Assist Consent"));
    SetDlgItemText(IDC_STATIC_CONSENT_MESSAGE, _T("A viewer is requesting read-only screen access."));
    SetDlgItemText(IDC_STATIC_CONSENT_VIEWER_IP_VALUE, m_viewerIp);
    SetDlgItemText(IDC_STATIC_CONSENT_HELPER_NAME_VALUE, m_helperName);
    SetDlgItemText(IDC_STATIC_CONSENT_SESSION_CODE_VALUE, m_sessionCode);
    SetDlgItemText(IDC_STATIC_CONSENT_HOST_NAME_VALUE, m_hostName);
    UpdateCountdownText();
    SetTimer(1, 1000, nullptr);
    return TRUE;
}

void CConsentDialog::OnTimer(UINT_PTR eventId)
{
    if (eventId == 1)
    {
        if (m_secondsRemaining > 0)
        {
            --m_secondsRemaining;
            UpdateCountdownText();
        }

        if (m_secondsRemaining == 0)
        {
            m_timedOut = true;
            KillTimer(1);
            EndDialog(IDCANCEL);
            return;
        }
    }

    CDialogEx::OnTimer(eventId);
}

void CConsentDialog::UpdateCountdownText()
{
    CString countdown;
    countdown.Format(_T("Auto-deny in %u seconds"), m_secondsRemaining);
    SetDlgItemText(IDC_STATIC_CONSENT_COUNTDOWN, countdown);
}

BEGIN_MESSAGE_MAP(CConsentDialog, CDialogEx)
    ON_WM_TIMER()
END_MESSAGE_MAP()
