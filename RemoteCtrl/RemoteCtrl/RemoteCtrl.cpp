// RemoteCtrl.cpp : This file contains the "main" function. Program execution starts and ends here.
#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "EdoyunTool.h"
#include "Command.h"
#include <conio.h>
#include "CEdoyunQueue.h"

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

#define IOCP_LIST_EMPTY 0
#define IOCP_LIST_PUSH 1
#define IOCP_LIST_POP 2

enum
{
	IocpListEmpty,
	IocpListPush,
	IocpListPop
};

typedef struct IocpParam
{
	int nOperator;			//operator
	std::string strData;	//data
	_beginthread_proc_type cbFunc;	//callback
	IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL)
	{
		nOperator = op;
		strData = sData;
		cbFunc = cb;
	}
	IocpParam()
	{
		nOperator = -1;
	}
}IOCP_PARAM;

void threadmain(HANDLE hIOCP)
{
	std::list<std::string> lstString;
	DWORD dwTransferred = 0;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* pOverlapped = NULL;
	int count = 0, count0 = 0, total = 0;
	while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE))
	{
		if (dwTransferred == 0 || (CompletionKey == NULL))
		{
			printf("thread is prepare to exit!\r\n");
			break;
		}
		IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKey;
		if (pParam->nOperator == IocpListPush)
		{
			lstString.push_back(pParam->strData);
			printf("push size %d %p\r\n", lstString.size(), pOverlapped);
			count0++;
		}
		else if (pParam->nOperator == IocpListPop)
		{
			printf("%p size=%d \r\n", pParam->cbFunc, lstString.size());
			std::string str;
			if (lstString.size() > 0)
			{
				str = lstString.front();
				lstString.pop_front();
			}
			if (pParam->cbFunc)
			{
				pParam->cbFunc(&str);
			}
			count++;

		}
		else if (pParam->nOperator == IocpListEmpty)
		{
			lstString.clear();
		}
		delete pParam;
		printf("total %d\r\n", ++total);
	}
	lstString.clear();
	printf("thread exit count %d count0 %d \r\n", count, count0);
}

void threadQueueEntry(HANDLE hIOCP)
{
	threadmain(hIOCP);
	_endthread();	//The code stops here, which will prevent the local object from being able to call the destructor, thereby causing memory leakage.
}

void func(void* arg)
{
	std::string* pstr = (std::string*)arg;
	if (pstr != NULL)
	{
		printf("pop from list:%s \r\n", pstr->c_str());
		//delete pstr;
	}
	else
	{
		printf("list is empty,no data!\r\n");
	}

}

int main()
{
	if (!CEdoyunTool::Init()) return 1;
	printf("press any key to exit...\r\n");
	CEdoyunQueue<std::string> lstStrings;
	ULONGLONG tick0 = GetTickCount(), tick = GetTickCount64();
	// This bug is bound to happen, and it will seriously affect the program.
	while (_kbhit() == 0)	//Core:The completion port separates the request from the implementation.
	{
		if (GetTickCount64() - tick0 > 1300)
		{
			lstStrings.PushBack("hello world");
			tick0 = GetTickCount64();
		}
		if (GetTickCount64() - tick > 2000)
		{
			std::string str;
			lstStrings.PopFront(str);
			tick = GetTickCount64();
			printf("pop from queue:%s\r\n", str.c_str());
		}
		Sleep(1);
	}
	
	printf("exit done!size=%d\r\n", lstStrings.Size());
	lstStrings.Clear();
	printf("exit done!size=%d\r\n", lstStrings.Size());
	::exit(0);
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
