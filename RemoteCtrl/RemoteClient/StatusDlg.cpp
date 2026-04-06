// StatusDlg.cpp: implementation file
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "StatusDlg.h"


// CStatusDlg dialog

IMPLEMENT_DYNAMIC(CStatusDlg, CDialog)

CStatusDlg::CStatusDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_STATUS, pParent)
{

}

CStatusDlg::~CStatusDlg()
{
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INFO, m_info);
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_INFO, &CStatusDlg::OnEnChangeEditInfo)
END_MESSAGE_MAP()


// CStatusDlg message handlers

void CStatusDlg::OnEnChangeEditInfo()
{
	// TODO: If this control is a RICHEDIT control, it will not
	// send this notification unless the CDialog::OnInitDialog() function is overridden and CRichEditCtrl().SetEventMask() is called,
	// and the ENM_CHANGE flag is "OR"ed into the mask. 
	// TODO:  Add the code for the control notification handler here
}
