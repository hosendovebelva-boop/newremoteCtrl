
// RemoteClient.cpp: defines the class behavior for the application.
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "ClientController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRemoteClientApp

BEGIN_MESSAGE_MAP(CRemoteClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CRemoteClientApp construction

CRemoteClientApp::CRemoteClientApp()
{
	// Support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: Add construction code here; new changes were added
	// Place all significant initialization work in InitInstance
}


// The one and only CRemoteClientApp object

CRemoteClientApp theApp;


// CRemoteClientApp initialization

BOOL CRemoteClientApp::InitInstance()
{
	// If the application uses one of the following controls, InitCommonControlsEx() is required on Windows XP
	// to use ComCtl32.dll version 6 or later for visual styles,
	//InitCommonControlsEx() is required. Otherwise, the window cannot be created.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all
	// common control classes used in the application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager in case the dialog contains
	// any shell tree-view controls or shell list-view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate the "Windows Native" visual manager so MFC controls can use themes
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you do not use these features and want to reduce
	// the size of the final executable, remove the following
	// unneeded specific initialization routines
	// Change the registry key used to store settings
	// TODO: This string should be changed appropriately,
	// for example, to your company or organization name
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CClientController::getInstance()->InitController();
	/*CClientController controller;
	controller.Invoke(m_pMainWnd);*/
	INT_PTR nResponse =  CClientController::getInstance()->Invoke(m_pMainWnd);
	//CRemoteClientDlg dlg;
	//m_pMainWnd = &dlg;
	//INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Add code here to handle when the dialog is dismissed with
		//  "OK" to close the dialog
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Add code here to handle when the dialog is dismissed with
		//  "Cancel" to close the dialog
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so the application will terminate unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: If you use MFC controls in dialogs, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
		TRACE("shell manager has deleted!\r\n");
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Because the dialog has been closed, FALSE is returned so the application exits,
	//  instead of starting the application message pump.
	return FALSE;
}

