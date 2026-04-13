// RemoteCtrl.cpp : This file contains the "main" function. Program execution starts and ends here.
#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "EdoyunTool.h"
#include "Command.h"
#include <conio.h>
#include "CEdoyunQueue.h"
#include <MSWSock.h>
#include "EdoyunServer.h"

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
		return true;
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

void iocp();

int main()
{
	if (!CEdoyunTool::Init()) return 1;

	iocp();

	return 0;
	/*if (CEdoyunTool::IsAdmin())
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
	return 0;*/
}

class COverlapped
{
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;
	char m_buffer[4096];
	COverlapped()
	{
		m_operator = 0;
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		memset(m_buffer, 0, sizeof(m_buffer));
	}
};

void iocp()
{

	EdoyunServer server;
	server.StartService();
	getchar();

	/*
	//SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);	//TCP
	//With overlapping structure, non-blocking
	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
	{
		CEdoyunTool::ShowError();
		return;
	}

	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, sock, 4);
	SOCKET client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort((HANDLE)sock, hIOCP, 0, 0);

	sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	addr.sin_port = htons(9527);

	bind(sock, (sockaddr*)&addr, sizeof(addr));
	listen(sock, 5);
	COverlapped overlapped;
	overlapped.m_operator = 1;	// accept
	memset(&overlapped, 0, sizeof(OVERLAPPED));

	//Compared to accept() a socket that has already been created in advance, this can increase the concurrency level.
	char buffer[4096] = "";
	DWORD received = 0;
	if (AcceptEx(sock, client, overlapped.m_buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped) == FALSE)
	{
		CEdoyunTool::ShowError();
	}
	overlapped.m_operator = 2;	// accept
	WSASend();
	overlapped.m_operator = 3;	// accept
	WSARecv();

	// Start the thread
	while (true)
	{
		LPOVERLAPPED pOverlapped = NULL;
		DWORD transferred = 0;
		DWORD key = 0;
		// Asynchronous operation, obtaining the pointer to the parent object through the address of the member variable
		// Represents a thread
		if (GetQueuedCompletionStatus(hIOCP, &transferred, &key, &pOverlapped, INFINITY))
		{
			COverlapped* pO = CONTAINING_RECORD(pOverlapped, COverlapped, m_overlapped);
			switch (pO->m_operator)
			{
			case 1:
				// Handling the Accept operation

			default:
				break;
			}
		}
	}
	*/

}