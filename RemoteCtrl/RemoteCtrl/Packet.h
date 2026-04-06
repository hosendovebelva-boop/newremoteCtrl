#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
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
		// Packet data may be incomplete, or the packet header may not have been fully received
		if (i + 4 + 2 + 2 > nSize)
		{
			nSize = 0;
			return;
		}

		nLength = *(DWORD*)(pData + i);
		i += 4;
		// Return when the packet has not been fully received; parsing fails at this point
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
	~CPacket() {}
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

	int Size()	// size of the packet data
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
	WORD sHead;			// fixed marker FE FF
	DWORD nLength;		// packet length (from the control command to the checksum)
	WORD sCmd;			// control command
	std::string strData;// packet data
	WORD sSum;			// checksum
	std::string strOut;	// data of the entire packet
};

#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	// click, move, double-click
	WORD nButton;	// left, right, middle button
	POINT ptXY;		// coordinates
}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	// whether valid
	BOOL IsInvalid;
	// whether it is a directory, 0 = no, 1 = yes
	BOOL IsDirectory;
	// whether more entries follow, 0 = no, 1 = yes
	BOOL HasNext;
	// file name
	char szFileName[256];
}FILEINFO, * PFILEINFO;