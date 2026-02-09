#pragma once
// ServerSocket.cpp
#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"


typedef void (*SOCKET_CALLBACK)(void* , int , std::list<CPacket>& ,CPacket&);

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
	

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527)
	{
		// 1.进度的可控性 2.对接的方便性 3.可行性评估，提早暴露风险
		// TODO:  socket、bind、listen、accept、read、write、close
		// 套接字结构体初始化
		bool ret = InitSocket(port);
		if (ret == false)return -1;
		std::list<CPacket> lstPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true)
		{
			if (AcceptClient() == false)
			{
				if (count >= 3)
				{
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0)
			{
				m_callback(m_arg, ret, lstPackets, m_packet);
				while (lstPackets.size() > 0)
				{
					Send(lstPackets.front());
					lstPackets.pop_front();
				}
			}
			CloseClient();
		}

		return 0;
	}
protected:
	bool InitSocket(short port)
	{
		if (m_sock == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
		//绑定
		if (bind(m_sock, reinterpret_cast<sockaddr*>(&serv_adr), sizeof(serv_adr)) == -1)
			return false;

		if (listen(m_sock, 1) == -1)
			return false;

		return true;
	}

	bool AcceptClient()
	{
		TRACE("enter AcceptClient\r\n");
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client=%d\r\n", m_client);
		if (m_client == -1)
			return false;
		return true;
	}

#define BUFFER_SIZE 4096000
	int DealCommand()
	{
		if (m_client == -1)
			return -1;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL)
		{
			TRACE("内存不足！\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				delete[]buffer;
				return -1;
			}
			TRACE("recv %d\r\n", len);
			// TODO:处理命令
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}

	bool Send(const char* pData, size_t nSize)
	{
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack)
	{
		if (m_client == -1)
			return false;
		//Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;

	}
	bool GetFilePath(std::string& strPath)
	{
		if (((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) ||
			(m_packet.sCmd == 9))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& GetPacket()
	{
		return m_packet;
	}

	void CloseClient()
	{
		if (m_client != INVALID_SOCKET)
		{
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}
private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
	// 只允许私人使用
	CServerSocket& operator=(const CServerSocket& ss) {}
	CServerSocket(const CServerSocket& ss)
	{
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
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