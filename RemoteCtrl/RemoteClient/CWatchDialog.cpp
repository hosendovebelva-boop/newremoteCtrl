// CWatchDialog.cpp: implementation file
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "CWatchDialog.h"
#include "ClientController.h"

// CWatchDialog dialog

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIG_WATCH, pParent)
{
	m_isFull = false;
	m_nObjWidth = -1;
	m_nObjHeight = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_MESSAGE(WM_SEND_PACK_ACK,&CWatchDialog::OnSendPackAck)
END_MESSAGE_MAP()


// CWatchDialog message handlers
// Why do mouse click events send correctly when bool isScreen defaults to false?
CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen = false)
{
	CRect clientRect;
	if (!isScreen)
	{
		ClientToScreen(&point);		// Convert to coordinates relative to the upper-left corner of the screen (absolute screen coordinates)
	}
	m_picture.ScreenToClient(&point);	// Convert global coordinates to client-area coordinates
	TRACE("x=%d y=%d \r\n", point.x, point.y);
	// Map local coordinates to remote coordinates
	m_picture.GetWindowRect(clientRect);
	TRACE("x=%d y=%d \r\n", clientRect.Width(), clientRect.Height());
	return CPoint(point.x * m_nObjWidth / clientRect.Width(), point.y * m_nObjHeight / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_isFull = false;
	// TODO: Add extra initialization here
	// SetTimer(0, 45, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// Exception: OCX property pages should return FALSE
}

void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add message handler code here and/or call the default handler
	//if (nIDEvent == 0)
	//{
	//	CClientController* pParent = CClientController::getInstance();
	//	if (m_isFull)
	//	{
	//		CRect rect;
	//		m_picture.GetWindowRect(rect);
	//		m_nObjWidth = m_image.GetWidth();
	//		m_nObjHeight = m_image.GetHeight();

	//		m_image.StretchBlt(
	//			m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
	//		// ifFull still does not enter the condition at this point
	//		m_picture.InvalidateRect(NULL);
	//		m_image.Destroy();
	//		m_isFull = false;

	//	}
	//}
	CDialog::OnTimer(nIDEvent);
}

LRESULT CWatchDialog::OnSendPackAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || (lParam == -2))
	{
		//TODO: handle errors
	}
	else if (lParam == 1)
	{
		// The peer closed the socket
	}
	else
	{
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket != NULL)
		{
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (head.sCmd)
			{
			case 5:
				TRACE("mouse event ack\r\n");
				break;
			case 6:
			{
				CEdoyunTool::Bytes2Image(m_image, head.strData);
				CRect rect;
				m_picture.GetWindowRect(rect);
				m_nObjWidth = m_image.GetWidth();
				m_nObjHeight = m_image.GetHeight();

				m_image.StretchBlt(
					m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
				// ifFull still does not enter the condition at this point
				m_picture.InvalidateRect(NULL);
				TRACE("Image update completed %d %d %08X\r\n", m_nObjWidth, m_nObjHeight, (HBITMAP)m_image);
				m_image.Destroy();
				break;
			}
			case 7:
			case 8:
			default:
				break;
			}
		}
	}
	
	return LRESULT();
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;	// Left button
		event.nAction = 2;	// Double click
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		TRACE("remote:%d %d\r\n", remote.x, remote.y);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;	// Left button
		event.nAction = 2;	// Button down
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;	// Left button
		event.nAction = 3;	// Button up
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;	// Right button
		event.nAction = 1;	// Double click
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnRButtonDblClk(nFlags, point);
}

void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;	// Right button
		event.nAction = 2;	// Button down
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnRButtonDown(nFlags, point);
}

void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;	// Right button
		event.nAction = 3;	// Button up
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnRButtonUp(nFlags, point);
}

void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;	// No button
		event.nAction = 0;	// Move
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnMouseMove(nFlags, point);
}

void CWatchDialog::OnStnClickedWatch()
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1))
	{
		CPoint point;
		GetCursorPos(&point);

		// Coordinate conversion
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//Pack the event
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;	// Left button
		event.nAction = 0;	// Single click
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
}

void CWatchDialog::OnOK()
{
	// TODO: Add specialized code here and/or call the base class

	//CDialog::OnOK();
}

void CWatchDialog::OnBnClickedBtnLock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);

}

void CWatchDialog::OnBnClickedBtnUnlock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}

