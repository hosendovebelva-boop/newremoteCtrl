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
#include "ESocket.h"
#include "ENetWork.h"
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



void udp_server();
void udp_client(bool ishost = true);

void initsock()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
}

void clearsock()
{
	WSACleanup();
}

//int wmain(int argc, TCHAR* argv[]);
//int _tmain(int argc, TCHAR* argv[]);
int main(int argc, char* argv[])
{
	if (!CEdoyunTool::Init()) return 1;

	// server code
	if (argc == 1)
	{
		char wstrDir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, wstrDir);
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		string strCmd = argv[0];
		initsock();
		strCmd += " 1";
		BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
		if (bRet)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			TRACE("Process ID:%d\r\n", pi.dwProcessId);
			TRACE("Thread ID:%d\r\n", pi.dwThreadId);

			strCmd += "2";
			CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
			if (bRet)
			{
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				TRACE("Process ID:%d\r\n", pi.dwProcessId);
				TRACE("Thread ID:%d\r\n", pi.dwThreadId);
				udp_server();	// server code
			}
		}
	}
	else if (argc == 2)	// this is main client
	{
		udp_client();
	}
	else// this is child client
	{
		udp_client(false);
	}
	clearsock();
	//iocp();

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
}

/**
* 1. Usability
*	a simplified parameter
*	b Adaptation (Parameter Adaptation)
*	c process simplification
* 2 Portability (high cohesion, low coupling)
*	a What is the core function?
*	b What is the business logic?
*/
int RecvFromCB(void* arg, const EBuffer& buffer, ESockaddrIn& addr)
{
	EServer* server = (EServer*)arg;
	return server->Sendto(addr, buffer);
}
int SendToCB(void* arg, const ESockaddrIn& addr, int ret)
{
	EServer* server = (EServer*)arg;
	printf("sendto done!%p\r\n", server);
	return 0;
}
void udp_server()
{
	std::list<ESockaddrIn> lstclients;
	printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	EServerParameter param(
		"127.0.0.1",20000,
		ETYPE::ETypeUDP,
		NULL,NULL,NULL,
		RecvFromCB,SendToCB
	);
	EServer server(param);
	server.Invoke(&server);
	printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	getchar();
	return;
	//SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
	
}
void udp_client(bool ishost)
{
	Sleep(2000);
	sockaddr_in server, client;
	int len = sizeof(client);
	server.sin_family = AF_INET;
	server.sin_port = htons(20000);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	// main client 
	if (ishost)
	{
		printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		EBuffer msg = "hello world!\n";
		int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
		printf("%s(%d):%s ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, ret);
		if (ret > 0)
		{
			msg.resize(1024);
			memset((char*)msg.c_str(), 0, msg.size());
			ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
			printf("host %s(%d):%s ERROR(%d)!!! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			if (ret > 0)
			{
				printf("%s(%d):%s ip %08X port %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
				printf("%s(%d):%s msg = %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
			}

			ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
			printf("host %s(%d):%s ERROR(%d)!!! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			if (ret > 0)
			{
				printf("%s(%d):%s ip %08X port %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
				printf("%s(%d):%s msg = %s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());
			}
		}
	}
	else
	{
		printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		std::string msg = "hello world!\n";
		int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
		printf("%s(%d):%s ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, ret);
		if (ret > 0)
		{
			msg.resize(1024);
			memset((char*)msg.c_str(), 0, msg.size());
			ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
			printf("client %s(%d):%s ERROR(%d)!!! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			if (ret > 0)
			{
				sockaddr_in addr;
				memcpy(&addr, msg.c_str(), sizeof(addr));
				sockaddr_in* paddr = (sockaddr_in*)msg.c_str();
				printf("%s(%d):%s ip %08X port %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, client.sin_port);
				printf("%s(%d):%s msg = %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
				printf("%s(%d):%s ip %08X port %d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
				msg = "hello,i am client!\r\n";
				ret = sendto(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)paddr, sizeof(sockaddr_in));
				printf("%s(%d):%s ip %08X port %d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
				printf("client %s(%d):%s ERROR(%d)!!! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);

			}
		}
	}
	closesocket(sock);
}
