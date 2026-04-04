#pragma once
#include <map>
#include <atlimage.h>
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <list>
#include "Packet.h"
#include "EdoyunTool.h"
#include "ServerSocket.h"
#include "LockInfoDialog.h"
#include "resource.h"
#include "pch.h"
#include "framework.h"
#pragma warning(disable:4966)
class CCommand
{
public:
	CCommand();
	~CCommand() {}
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket);
	static void RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		CCommand* thiz = (CCommand*)arg;
		if (status > 0)
		{
			int ret = thiz->ExcuteCommand(status, lstPacket, inPacket);
			if (ret != 0)
			{
				TRACE("Command execution failed: %d ret=%d\r\n", status, ret);
			}
		}
		else
		{
			MessageBox(NULL, _T("Unable to connect to the user normally. Retrying automatically"), _T("User Connection Failed"), MB_OK | MB_ICONERROR);

		}

	}
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPacket); 	//member function pointer
	std::map<int, CMDFUNC> m_mapFunction;	//mapping from command ID to function
	CLockInfoDialog dlg;
	unsigned threadid;
protected:
	static unsigned __stdcall threadLockDlg(void* arg)
	{
		CCommand* thiz = (CCommand*)arg;
		thiz->threadLockDlgMain();
		_endthreadex(0);
		return 0;
	}

	void threadLockDlgMain()
	{
		TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
		dlg.Create(IDD_DIALOG_INFO, NULL);
		dlg.ShowWindow(SW_SHOW);
		CRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		rect.bottom = LONG(rect.bottom * 1.10);
		dlg.MoveWindow(rect);
		// Center the text
		CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
		if (pText)
		{
			CRect rtText;
			pText->GetWindowRect(rtText);
			int nWidth = rtText.Width();
			int x = (rect.right - nWidth) / 2;
			int nHeight = rtText.Height();
			int y = (rect.bottom - nHeight) / 2;
			pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
		}

		// Make the window topmost
		dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		// Restrict mouse functionality
		ShowCursor(false);
		// Hide the taskbar
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
		// Limit the mouse movement range
		dlg.GetWindowRect(rect);
		rect.left = 0;
		rect.top = 0;
		rect.right = 1;
		rect.bottom = 1;
		ClipCursor(rect);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_KEYDOWN)
			{
				TRACE("msg:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
				if (msg.wParam == 0x41)
					break;
			}
		}
		ClipCursor(NULL);
		// Restore the mouse cursor
		ShowCursor(true);
		// Restore the taskbar
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
		dlg.DestroyWindow();
	}

	// 1=>A 2=>B 3=>C
	int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		std::string result;
		for (int i = 1;i <= 26;i++)
		{
			if (_chdrive(i) == 0)
			{
				if (result.size() > 0)
					result += ',';
				result += 'A' + i - 1;
			}
		}
		lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
		return 0;
	}


	int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		if (_chdir(strPath.c_str()) != 0)
		{
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			OutputDebugString(_T("No permission to access the directory!!"));
			return -2;
		}
		_finddata_t fdata;
		intptr_t hfind = 0;
		if ((hfind = _findfirst("*", &fdata)) == -1)
		{
			OutputDebugString(_T("No files found!!"));
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			return -3;
		}
		int count = 0;
		do {
			FILEINFO finfo;
			finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
			memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
			//lstFileInfos.push_back(finfo);
			TRACE("%s\r\n", finfo.szFileName);
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			count++;
		} while (!_findnext(hfind, &fdata));
		TRACE("server:count = %d\r\n", count);
		// Send the information back to the controller
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
		return 0;
	}

	int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		lstPacket.push_back(CPacket(3, NULL, 0));
		return 0;
	}

	int DownloadFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		long long data = 0;
		FILE* pFile = NULL;
		errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
		if (err != 0)
		{
			lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
			return -1;
		}
		if (pFile != NULL)
		{
			fseek(pFile, 0, SEEK_END);
			data = _ftelli64(pFile);
			lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
			fseek(pFile, 0, SEEK_SET);
			char buffer[1024] = "";
			size_t rlen = 0;
			do {
				rlen = fread(buffer, 1, 1024, pFile);
				// The code reads from buffer but used to send data, which broke file downloads. Document the debugging process carefully in the notes
				//lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
				lstPacket.push_back(CPacket(4, (BYTE*)buffer, rlen));

			} while (rlen >= 1024);
			// [End of new code]

			fclose(pFile);
		}
		else
		{
			lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));

		}
		return 0;
	}

	int MouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		MOUSEEV mouse;
		memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));

		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0:// Left button
			nFlags = 1;
			break;
		case 1:// Right button
			nFlags = 2;
			break;
		case 2:// Middle button
			nFlags = 4;
			break;
		case 4:// No button
			nFlags = 8;
			break;
		default:
			break;
		}

		// Set the mouse position
		if (nFlags != 8) SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);

		switch (mouse.nAction)
		{
		case 0: // Single click
			nFlags |= 0x10;
			break;
		case 1: // Double click
			nFlags |= 0x20;
			break;
		case 2: // Button down
			nFlags |= 0x40;
			break;
		case 3: // Button up
			nFlags |= 0x80;
			break;
		default:
			break;
		}

		TRACE("mouse event : %08X x %d y %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);

		switch (nFlags)
		{
		case 0x21:  // Left-button double click
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11:  // Left-button single click
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

		case 0x41:  // Left-button down
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81:  // Left-button up
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

		case 0x22:  // Right-button double click
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12:  // Right-button single click
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42:  // Right-button down
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82:  // Right-button up
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

		case 0x24:  // Middle-button double click
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14:  // Middle-button single click
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44:  // Middle-button down
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84:  // Middle-button up
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08:  // Plain mouse move
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		default:
			break;
		}
		lstPacket.push_back(CPacket(5, NULL, 0));
		return 0;
	}

	int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		CImage screen;  //GDI
		HDC hScreen = ::GetDC(NULL);
		int nBitPerpixel = GetDeviceCaps(hScreen, BITSPIXEL);   // 8bit*4 ARGB
		int nWidth = GetDeviceCaps(hScreen, HORZRES);
		int nHeight = GetDeviceCaps(hScreen, VERTRES);
		screen.Create(nWidth, nHeight, nBitPerpixel);
		// Use the actual screen size to avoid black borders caused by hardcoding
		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hScreen);
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL)
			return -1;
		IStream* pStream = NULL;
		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (ret == S_OK)
		{
			screen.Save(pStream, Gdiplus::ImageFormatPNG);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			PBYTE pData = (PBYTE)GlobalLock(hMem);
			SIZE_T nSize = GlobalSize(hMem);
			lstPacket.push_back(CPacket(6, pData, nSize));
			GlobalUnlock(hMem);
		}
		pStream->Release();
		GlobalFree(hMem);
		screen.ReleaseDC();
		return 0;
	}

	int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
		{
			//_beginthread(threadLockDlg, 0, NULL);
			_beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);

			TRACE("threadid=%d\r\n", threadid);
		}
		lstPacket.push_back(CPacket(7, NULL, 0));
		return 0;
	}

	int UnlockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		//dlg.SendMessage(WM_KEYDOWN, 0x41, 0x001E001);
		//::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);
		PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
		lstPacket.push_back(CPacket(8, NULL, 0));
		return 0;
	}

	int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		lstPacket.push_back(CPacket(1981, NULL, 0));
		return 0;
	}

	int  DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		TCHAR sPath[MAX_PATH] = _T("");
		//mbstowcs(sPath, strPath.c_str(), strPath.size());
		MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(),
			sPath, sizeof(sPath) / sizeof(TCHAR));
		DeleteFileA(strPath.c_str());
		lstPacket.push_back(CPacket(9, NULL, 0));
		return 0;
	}
};

