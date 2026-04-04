#pragma once
class CEdoyunTool
{
public:
    static void Dump(BYTE* pData, size_t nSize)
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
    static bool IsAdmin()
	{
		HANDLE hToken = NULL;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			ShowError();
			return false;
		}
		TOKEN_ELEVATION eve;
		DWORD len = 0;
		if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE)
		{
			ShowError();
			return false;
		}
		CloseHandle(hToken);

		if (len == sizeof(eve))
		{
			return eve.TokenIsElevated;
		}
		printf("length of tokeninformation is %d\r\n", len);
		return false;
	}

	static void ShowError()
	{
		LPWSTR lpMessageBuf = NULL;
		//strerror(errno); standard
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
		OutputDebugString(lpMessageBuf);
		MessageBox(NULL, lpMessageBuf, _T("Error occurred"), 0);

		LocalFree(lpMessageBuf);
	}

	static bool RunAsAdmin()
	{
		// In Local Security Policy, enable the Administrator account and disable the policy that restricts blank-password accounts to local console logon only
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
		if (!ret)
		{
			ShowError();	//TODO: remove debug information
			MessageBox(NULL, sPath, _T("Program Error"), 0);	//TODO: remove debug information
			return false;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}

	static BOOL WriteStartupDir(const CString& strPath)
	{
		// Implement startup by modifying the startup folder
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		return CopyFile(sPath, strPath, FALSE);
		
	}

	// At startup, the program runs with the same privileges as the startup user
	// If the privilege levels do not match, the program may fail to start
	// Startup also affects environment variables, so if the program depends on DLLs (dynamic libraries), startup may fail
	// Solutions:
	// [Copy those DLLs into system32 or sysWOW64]
	// system32 usually contains 64-bit programs, while syswow64 usually contains 32-bit programs
	// [Use static libraries instead of dynamic libraries]
	static bool WriteRegisterTable(const CString& strPath)
	{
		// Implement startup by modifying the registry
		CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		BOOL ret = CopyFile(sPath, strPath, FALSE);
		if (ret == FALSE)
		{
			MessageBox(NULL, _T("Failed to copy the file. Is this due to insufficient privileges?\r\n"), _T("Error"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		HKEY hKey = NULL;
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
		if (ret != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			MessageBox(NULL, _T("Failed to configure startup at boot. Is this due to insufficient privileges?"), _T("Error"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}

		ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
		if (ret != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			MessageBox(NULL, _T("Failed to configure startup at boot. Is this due to insufficient privileges?"), _T("Error"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		RegCloseKey(hKey);
		return true;
	}

	static bool Init()
	{
		// Used to initialize MFC console applications (general-purpose)
		HMODULE hModule = ::GetModuleHandle(nullptr);
		if (hModule == nullptr)
		{
			wprintf(L"Error: GetModuleHandle failed\n");
			return false;
		}
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: Write code for the application behavior here.
			wprintf(L"Error: MFC initialization failed\n");
			return false;
		}
		return true;
	}

};

