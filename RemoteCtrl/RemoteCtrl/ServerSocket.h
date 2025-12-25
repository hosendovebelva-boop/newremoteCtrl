#pragma once
#include "pch.h"
#include "framework.h"

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL)
		{
			// 手动申请，手动释放：使用 new 创建的对象，其生命周期不受作用域限制。
			// 除非你在代码中显式调用 delete m_instance;，否则它的析构函数永远不会被执行。
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket()
	{
		
		//TODO: 校验
		if (m_sock == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);

		//绑定
		if (bind(m_sock, reinterpret_cast<sockaddr*>(&serv_adr), sizeof(serv_adr)) == -1)
			return false;
		
		//TODO
		if (listen(m_sock, 1) == -1)
			return false;

		return true;
	}

	bool AcceptClient()
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)
			return false;
		return true;
	}

	int DealCommand()
	{
		if (m_client = -1)
			return false;
		char buffer[1024] = "";
		while (true)
		{
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0)
				return -1;
			// TODO:处理命令

		}
	}

	bool Send(const char* pData, size_t nSize)
	{
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}

private:
	SOCKET m_client;
	SOCKET m_sock;
	// 只允许私人使用
	CServerSocket& operator=(const CServerSocket& ss){}
	CServerSocket(const CServerSocket& ss)
	{
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket(){
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置！"),_T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket()
	{
		closesocket(m_sock);
		WSACleanup();
	}
	BOOL InitSockEnv() 
	{
		WSADATA data;
		// 申请使用的 Socket 版本是 1.1
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) 
		{
			//TODO:返回值处理
			return  FALSE;
		}
		return TRUE;
	}

	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	// 静态指针的特性： m_instance 是一个静态指针。
	// 当程序退出时，操作系统会回收指针变量本身占用的 4 / 8 字节内存，但并不会顺着指针去销毁它指向的堆内存对象。
	static CServerSocket* m_instance;

	class CHelper 
	{
	public:
		CHelper()
		{
			CServerSocket::getInstance();
		}
		~CHelper()
		{
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

// 声明一个外部的变量，这样别的文件只需要引用了此头文件就可以使用这个变量了
extern CServerSocket server;