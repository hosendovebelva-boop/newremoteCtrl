#pragma once
#include "CClientSocket.h"
#include "CWatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "resource.h"
#include "EdoyunTool.h"
#include <map>

//#define WM_SEND_DATA (WM_USER+2)	// Send data
#define WM_SHOW_STATUS (WM_USER+3)	// Show status
#define WM_SHOW_WATCH (WM_USER+4)	// Remote monitor
#define WM_SEND_MESSAGE (WM_USER+0x1000)	// Custom message handling

class CClientController
{
public:
	// Get the singleton instance
	static CClientController* getInstance();
	// Initialize
	int InitController();
	// Start
	int Invoke(CWnd*& pMainWnd);
	// Update the network server address
	void UpdateAddress(int nIP, int nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand()
	{
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket()
	{
		return CClientSocket::getInstance()->CloseSocket();
	}
	

	// 1 View drive partitions
	// 2 View files under the specified directory
	// 3 Open file
	// 4 Download file
	// 5 Mouse operation
	// 6 Send screen content
	// 7 Lock machine
	// 8 Unlock machine
	// 9 Delete file
	// 1981 Test connection
	// Return value: true on success; false on failure
	bool SendCommandPacket(
		HWND hWnd,	//Window that should receive the reply after the packet arrives
		int nCmd,
		bool bAutoClose = true,
		BYTE* pData = NULL,
		size_t nLength = 0,
		WPARAM wParam = 0);

	int GetImage(CImage& image)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		return CEdoyunTool::Bytes2Image(image, pClient->GetPacket().strData);
		
	}

	void DownloadEnd();
	int DownFile(CString strPath);

	void StartWatchScreen();

protected:
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);
	CClientController() :
		m_statusDlg(&m_remoteDlg), 
		m_watchDlg(&m_remoteDlg)
	{
		m_isClosed = true;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController()
	{
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance()
	{
		TRACE("CClientSocket has been called!\r\n");
		if (m_instance != NULL)
		{
			delete m_instance;
			m_instance = NULL;
			TRACE("CClientController has released!\r\n");
		}
	}
	
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:
	typedef	struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m)
		{
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo()
		{
			result = 0;
			memcpy(&msg, 0, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m)
		{
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}

		MsgInfo& operator=(const MsgInfo& m)
		{
			if (this != &m)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}

	} MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	// UUID: ensure each created value is unique
	CWatchDialog m_watchDlg;		// Message packets may cause a memory leak after the dialog closes
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadWatch;
	bool m_isClosed;	//whether monitoring is closed
	// Remote path of the file to download
	CString m_strRemote;
	// Local save path of the downloaded file
	CString m_strLocal;
	unsigned m_nThreadID;
	static CClientController* m_instance;

	class CHelper
	{
	public:
		CHelper()
		{
			// A global variable or a static class member is initialized before main(), so there is no multithreading issue before main()
			// Why release it here? Because it exists before main()
			//CClientController::getInstance();
		}
		~CHelper()
		{
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

