// RemoteCtrl.cpp : This file contains the "main" function. Program execution starts and ends here.
#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "EdoyunTool.h"
#include "Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define INVOKE_PATH_T _T("C:\\Windows\\system32\\RemoteCtrl.exe")
#define INVOKE_PATH_T _T("C:\\Users\\49522\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")

// The one and only application object
CWinApp theApp;
using namespace std;
// At startup, the program runs with the privileges of the startup user.
// If the privilege levels do not match, the program may fail to start.
// Startup also affects environment variables, so if the program depends on DLLs (dynamic libraries), startup may fail.
// Copy those DLLs into system32.
bool ChooseAutoInvoke(const CString& strPath)
{
	TCHAR wcsSystem[MAX_PATH] = _T("");
	//CString strPath = CString();
	if (PathFileExists(strPath))
	{
		return;
	}
	CString strInfo = _T("This program may only be used for lawful purposes!\n");
	strInfo += _T("Continuing to run this program will place this machine under remote monitoring.\n");
	strInfo += _T("If you do not want that, click \"Cancel\" to exit the program.\n");
	strInfo += _T("Press \"Yes\" to copy this program to your machine and start it automatically with the system.\n");
	strInfo += _T("Press \"No\" to allow this program to run only once without leaving anything on the system.\n");
	strInfo += _T("\n");
	int ret = MessageBox(NULL, strInfo, _T("Warning"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
	if (ret == IDYES)
	{
		//WriteRegisterTable(strPath);
		if (!CEdoyunTool::WriteStartupDir(strPath))
		{
			MessageBox(NULL, _T("Failed to copy the file. Is this due to insufficient privileges?\r\n"), _T("Error"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
	}
	else if (ret == IDCANCEL)
	{
		return false;
	}
	return true;
}

int main()
{
	if (CEdoyunTool::IsAdmin())
	{
		if (!CEdoyunTool::Init()) return 1;
		if (ChooseAutoInvoke(INVOKE_PATH_T))
		{
			CCommand cmd;
			int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
			switch (ret)
			{
			case -1:
				MessageBox(NULL, _T("Network initialization failed. Please check the network status."), _T("Network Initialization Failed"), MB_OK | MB_ICONERROR);
				break;
			case -2:
				MessageBox(NULL, _T("Unable to connect to the user normally. Retrying automatically."), _T("User Connection Failed"), MB_OK | MB_ICONERROR);
				break;
			}
		}
	}
	else
	{
		if (CEdoyunTool::RunAsAdmin() == false)
		{
			CEdoyunTool::ShowError();
			return 1;
		}
	}
	return 0;
}
