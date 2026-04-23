#include "pch.h"
#include "CWatchDialog.h"

IMPLEMENT_DYNAMIC(CWatchDialog, CDialogEx)

CWatchDialog::CWatchDialog(CWnd* parent)
    : CDialogEx(IDD_DIG_WATCH, parent),
      m_ownerWnd(nullptr),
      m_internalClose(false)
{
}

CWatchDialog::~CWatchDialog() = default;

bool CWatchDialog::EnsureCreated(CWnd* owner)
{
    m_ownerWnd = owner != nullptr ? owner->GetSafeHwnd() : nullptr;
    if (::IsWindow(m_hWnd))
    {
        return true;
    }

    return Create(IDD_DIG_WATCH, owner) != FALSE;
}

void CWatchDialog::ShowWatch()
{
    if (!::IsWindow(m_hWnd))
    {
        return;
    }

    SetWindowText(_T("Remote Assist Viewer - Active Session"));
    ShowWindow(SW_SHOW);
    SetForegroundWindow();
    SetTimer(1, ScreenShareProtocol::kFrameIntervalMs, nullptr);
}

void CWatchDialog::HideWatch()
{
    if (!::IsWindow(m_hWnd))
    {
        return;
    }

    m_internalClose = true;
    KillTimer(1);
    ShowWindow(SW_HIDE);
    m_internalClose = false;
}

void CWatchDialog::UpdateFrame(const std::string& pngBytes)
{
    if (pngBytes.empty())
    {
        return;
    }

    HGLOBAL memoryHandle = ::GlobalAlloc(GMEM_MOVEABLE, pngBytes.size());
    if (memoryHandle == nullptr)
    {
        return;
    }

    void* memory = ::GlobalLock(memoryHandle);
    if (memory == nullptr)
    {
        ::GlobalFree(memoryHandle);
        return;
    }

    memcpy(memory, pngBytes.data(), pngBytes.size());
    ::GlobalUnlock(memoryHandle);

    IStream* stream = nullptr;
    if (FAILED(::CreateStreamOnHGlobal(memoryHandle, TRUE, &stream)) || stream == nullptr)
    {
        ::GlobalFree(memoryHandle);
        return;
    }

    if ((HBITMAP)m_image != nullptr)
    {
        m_image.Destroy();
    }

    if (SUCCEEDED(m_image.Load(stream)))
    {
        RenderFrame();
        Invalidate(FALSE);
    }

    stream->Release();
}

bool CWatchDialog::IsWatching() const
{
    return ::IsWindow(m_hWnd) && IsWindowVisible();
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_WATCH_IMAGE, m_picture);
}

BOOL CWatchDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CWatchDialog::OnCancel()
{
    HideWatch();
    if (!m_internalClose && ::IsWindow(m_ownerWnd))
    {
        ::PostMessage(m_ownerWnd, WM_WATCH_END_SESSION, 0, 0);
    }
}

void CWatchDialog::OnBnClickedEndSession()
{
    OnCancel();
}

void CWatchDialog::OnPaint()
{
    CPaintDC dc(this);
    CDialogEx::OnPaint();
    RenderFrame();
}

void CWatchDialog::OnTimer(UINT_PTR eventId)
{
    if (eventId == 1 && ::IsWindowVisible(m_hWnd) && ::IsWindow(m_ownerWnd))
    {
        ::PostMessage(m_ownerWnd, WM_WATCH_REQUEST_FRAME, 0, 0);
    }

    CDialogEx::OnTimer(eventId);
}

void CWatchDialog::RenderFrame()
{
    if ((HBITMAP)m_image == nullptr || !::IsWindow(m_picture.GetSafeHwnd()))
    {
        return;
    }

    CRect rect;
    m_picture.GetClientRect(&rect);
    CDC* pictureDc = m_picture.GetDC();
    if (pictureDc == nullptr)
    {
        return;
    }

    pictureDc->FillSolidRect(&rect, RGB(0, 0, 0));
    m_image.StretchBlt(pictureDc->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
    m_picture.ReleaseDC(pictureDc);
}

BEGIN_MESSAGE_MAP(CWatchDialog, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_END_SESSION, &CWatchDialog::OnBnClickedEndSession)
    ON_WM_PAINT()
    ON_WM_TIMER()
END_MESSAGE_MAP()

