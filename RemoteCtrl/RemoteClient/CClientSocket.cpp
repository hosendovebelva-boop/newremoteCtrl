#include "pch.h"
#include "CClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

// 为什么在头文件中不做实现
std::string GetErrInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

void Dump(BYTE* pData, size_t nSize)
{
	std::string strOut;
	for (size_t i = 0;i < nSize;i++)
	{
		char buf[8] = "";
		if (i > 0 && (i % 16 == 0))
			strOut += '/n';
		snprintf(buf, sizeof(buf), "%02X", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += '\n';
	OutputDebugStringA(strOut.c_str());
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
{
	// 发包的时候才去判断是否有效
	if (m_sock == INVALID_SOCKET)
	{
		/*if (InitSocket() == false)
		{
			return false;
		}*/

		_beginthread(&CClientSocket::threadEntry, 0, this);
	}
	auto pr = m_mapAck.insert(std::pair<HANDLE,
		std::list<CPacket>&>(pack.hEvent, lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	m_lstSend.push_back(pack);
	TRACE("cmd:%d event %IX thread id %lu\r\n",
		pack.sCmd, (UINT_PTR)pack.hEvent, GetCurrentThreadId());
	m_lstSend.push_back(pack);
	WaitForSingleObject(pack.hEvent, INFINITE);
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end())
	{
		m_mapAck.erase(it);
		return true;
	}
	return false;
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_sock != INVALID_SOCKET)
	{
		if (m_lstSend.size() > 0)
		{
			TRACE("lstSend size: %d\r\n", m_lstSend.size());
			CPacket& head = m_lstSend.front();
			if (Send(head) == false)
			{
				TRACE("send failed!\r\n");
				continue;
			}
			std::map<HANDLE, std::list<CPacket>&>::iterator it;
			it = m_mapAck.find(head.hEvent);
			if (it != m_mapAck.end())
			{
				std::map<HANDLE, bool>::iterator it0 = m_mapAutoClosed.find(head.hEvent);
				do {
					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
					if (length > 0 || index > 0)
					{
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);
						if (size > 0)
						{
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							if (it0->second)
							{
								SetEvent(head.hEvent);
							}
						}
					}
					else if (length <= 0 && index <= 0)
					{
						CloseSocket();
						SetEvent(head.hEvent);	// 等到服务器关闭命令之后，在通知事件完成
						m_mapAutoClosed.erase(it0);
						break;
					}
				} while (it0->second == false);
			}
			m_lstSend.pop_front();
			if (InitSocket() == false)
			{
				InitSocket();
			}
		}
	}
	CloseSocket();
}

bool CClientSocket::Send(const CPacket& pack)
{
	TRACE("m_sock = %d\r\n", m_sock);
	if (m_sock == -1)
		return false;
	std::string strOut;
	pack.Data(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}
