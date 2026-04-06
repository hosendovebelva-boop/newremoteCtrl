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
			// Manual allocation and manual release: an object created with new is not limited by scope lifetime.
			// Unless you explicitly call delete m_instance; in the code, its destructor will never be executed.
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527)
	{
		// 1. Controllable progress 2. Easier integration 3. Feasibility evaluation to expose risks early
		// TODO: socket, bind, listen, accept, read, write, close
		// Initialize the socket address structure
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
		// bind
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
			TRACE("Out of memory!\r\n");
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
			// TODO: handle commands
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

	// Why the Send function should not be removed even though nothing else references it yet
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
	// Private use only
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
			MessageBox(NULL, _T("Unable to initialize the socket environment. Please check the network settings!"), _T("Initialization Error!"), MB_OK | MB_ICONERROR);
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
		// Request to use Socket version 1.1
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			//TODO: handle the return value
			return  FALSE;
		}
		return TRUE;
	}

	static void releaseInstance()
	{
		// Defensive programming
		if (m_instance != NULL)
		{
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	// Static pointer note: m_instance is a static pointer.
	// When the program exits, the operating system reclaims the 4 / 8 bytes used by the pointer variable itself, but it does not destroy the heap object pointed to by that pointer.
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

// Declare an external variable so other files can use it by including this header
extern CServerSocket server;
