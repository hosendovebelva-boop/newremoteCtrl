#include "pch.h"
#include "ScreenCapture.h"

#include <atlimage.h>

bool CapturePrimaryMonitorPng(std::string& pngBytes)
{
    pngBytes.clear();

    HDC screenDc = ::GetDC(nullptr);
    if (screenDc == nullptr)
    {
        return false;
    }

    const int width = GetSystemMetrics(SM_CXSCREEN);
    const int height = GetSystemMetrics(SM_CYSCREEN);

    CImage image;
    if (FAILED(image.Create(width, height, 32)))
    {
        ::ReleaseDC(nullptr, screenDc);
        return false;
    }

    BitBlt(image.GetDC(), 0, 0, width, height, screenDc, 0, 0, SRCCOPY);
    image.ReleaseDC();
    ::ReleaseDC(nullptr, screenDc);

    HGLOBAL memoryHandle = ::GlobalAlloc(GMEM_MOVEABLE, 0);
    if (memoryHandle == nullptr)
    {
        return false;
    }

    IStream* stream = nullptr;
    if (FAILED(::CreateStreamOnHGlobal(memoryHandle, FALSE, &stream)) || stream == nullptr)
    {
        ::GlobalFree(memoryHandle);
        return false;
    }

    bool success = false;
    if (SUCCEEDED(image.Save(stream, Gdiplus::ImageFormatPNG)))
    {
        HGLOBAL streamHandle = nullptr;
        if (SUCCEEDED(::GetHGlobalFromStream(stream, &streamHandle)) && streamHandle != nullptr)
        {
            const SIZE_T size = ::GlobalSize(streamHandle);
            void* data = ::GlobalLock(streamHandle);
            if (data != nullptr && size > 0)
            {
                pngBytes.assign(static_cast<const char*>(data), static_cast<const char*>(data) + size);
                success = true;
                ::GlobalUnlock(streamHandle);
            }
        }
    }

    stream->Release();
    ::GlobalFree(memoryHandle);
    return success;
}
