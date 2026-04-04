#pragma once
#include "afxdialogex.h"


// CLockDialog dialog

class CLockInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(CLockInfoDialog)

public:
	CLockInfoDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLockInfoDialog();

// Dialog data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
