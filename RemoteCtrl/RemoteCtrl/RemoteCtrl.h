#pragma once

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH generation"
#endif

#include "resource.h"

class CRemoteCtrlApp : public CWinApp
{
public:
    CRemoteCtrlApp();

    BOOL InitInstance() override;

    DECLARE_MESSAGE_MAP()
};

extern CRemoteCtrlApp theApp;
