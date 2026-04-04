#pragma once
#ifndef WM_SEND_PACK_ACK
// Add a space between the macro name and parentheses
#define WM_SEND_PACK_ACK (WM_USER + 2)  
#endif

#include "afxdialogex.h"


// CWatchDialog dialog

class CWatchDialog : public CDialog
{
	DECLARE_DYNAMIC(CWatchDialog)

public:
	CWatchDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CWatchDialog();


// Dialog data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIG_WATCH };
#endif

public:
	int m_nObjWidth;
	int m_nObjHeight;
	CImage m_image;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	bool m_isFull;	// Whether the buffer contains data: true means buffered, false means empty
	DECLARE_MESSAGE_MAP()
public:
	CImage& GetImage()
	{
		return m_image;
	}
	void SetImageStatus(bool isFull = false)
	{
		m_isFull = isFull;
	}
	bool isFull() const
	{
		return m_isFull;
	}
	CPoint UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	virtual void OnOK();
	afx_msg void OnBnClickedBtnUnlock();
	afx_msg void OnBnClickedBtnLock();
};
 