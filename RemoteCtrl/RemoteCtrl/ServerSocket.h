#pragma once
// ServerSocket.cpp
#include "pch.h"
#include "framework.h"

void Dump(BYTE* pData, size_t nSize);

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
	{
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}
		
		sSum = 0;
		for (size_t j = 0;j < strData.size();j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize)
	{
		size_t i = 0;
		for (;i < nSize;i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		// 包数据可能不全，或者包头未能全部接收到
		if (i + 4 + 2 + 2 > nSize)
		{
			nSize = 0;
			return;
		}

		nLength = *(DWORD*)(pData + i);
		i += 4;
		// 包没有完全接收到，就返回，此时解析失败
		if (nLength + i > nSize)
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0;j < strData.size();j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;	//head 2 length 4 data...
			return;
		}
		nSize = 0;
	}
	~CPacket(){}
	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size()	// 包数据的大小
	{
		return nLength + 6;
	}

	const char* Data()
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)(pData) = nLength;
		pData += 4;
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;

		return strOut.c_str();
	}

public:
	WORD sHead;			// 固定位 FE FF
	DWORD nLength;		// 包长度（从控制命令开始到和校验结束）
	WORD sCmd;			// 控制命令
	std::string strData;// 包数据
	WORD sSum;			// 和校验
	std::string strOut;	// 整个包的数据
};

#pragma pack(pop)

typedef struct MouseEvent{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	// 点击、移动、双击
	WORD nButton;	// 左键、右键、中键
	POINT ptXY;		// 坐标
}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	// 是否有效
	BOOL IsInvalid;
	// 是否为目录，0 否，1 是
	BOOL IsDirectory;
	// 是否还有后续 0 没有 1 有
	BOOL HasNext;
	// 文件名
	char szFileName[256];
}FILEINFO, * PFILEINFO;

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
		TRACE("enter AcceptClient\r\n");
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client=%d\r\n",m_client);
		if (m_client == -1)
			return false;
		return true;
	}

#define BUFFER_SIZE 4096
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
		if (((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4))||
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
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
private:
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
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