#include "pch.h"
#include "ConsentDialog.h"

IMPLEMENT_DYNAMIC(CConsentDialog, CDialogEx)

CConsentDialog::CConsentDialog(CWnd* parent) : CDialogEx(IDD_DIALOG_CONSENT, parent)
{
}

CConsentDialog::~CConsentDialog() = default;

void CConsentDialog::SetViewerIp(const CString& peerIp)
{
    m_message.Format(_T("Viewer at %s wants to see your screen."), peerIp.GetString());
}

void CConsentDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CConsentDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetDlgItemText(IDC_STATIC_CONSENT_MESSAGE, m_message);
    return TRUE;
}

BEGIN_MESSAGE_MAP(CConsentDialog, CDialogEx)
END_MESSAGE_MAP()
