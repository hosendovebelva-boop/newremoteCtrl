#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "HostMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CRemoteCtrlApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CRemoteCtrlApp theApp;

CRemoteCtrlApp::CRemoteCtrlApp()
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

BOOL CRemoteCtrlApp::InitInstance()
{
    INITCOMMONCONTROLSEX initControls = {};
    initControls.dwSize = sizeof(initControls);
    initControls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&initControls);

    CWinApp::InitInstance();
    AfxEnableControlContainer();
    SetRegistryKey(_T("AssistHost"));

    WSADATA wsaData = {};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        AfxMessageBox(_T("Unable to initialize Winsock."));
        return FALSE;
    }

    CHostMainDlg dialog;
    m_pMainWnd = &dialog;
    dialog.DoModal();

    WSACleanup();
    return FALSE;
}
