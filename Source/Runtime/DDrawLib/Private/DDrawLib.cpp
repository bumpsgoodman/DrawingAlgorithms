#include "Precompiled.h"

DDraw::~DDraw()
{
    Cleanup();
}

bool DDraw::Initialize(const HWND hWnd)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd = {};

    mhWnd = hWnd;

    UpdateWindowPos();
    mWidth = mWindowRect.right - mWindowRect.left;
    mHeight = mWindowRect.bottom - mWindowRect.top;

    hr = DirectDrawCreate(nullptr, &mDD, nullptr);
    if (FAILED(hr))
    {
        AssertW(false, L"Failed to Create DDrawObject");
        goto EXIT;
    }

    hr = mDD->QueryInterface(IID_IDirectDraw7, (LPVOID*)&mDD7);
    if (FAILED(hr))
    {
        goto EXIT;
    }

    hr = mDD7->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        AssertW(false, L"Failed to SetCooperativeLevel");
        goto EXIT;
    }

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = mDD7->CreateSurface(&ddsd, &mPrimary, nullptr);
    if (FAILED(hr))
    {
        AssertW(false, L"Failed to CreateSurface");
        goto EXIT;
    }

    if (!CreateBackBuffer(mWidth, mHeight))
    {
        AssertW(false, L"Failed to CreateBackBuffer");
        goto EXIT;
    }

    hr = mDD7->CreateClipper(0, &mClipper, nullptr);
    if (FAILED(hr))
    {
        AssertW(false, L"Failed to CreateClipper");
        goto EXIT;
    }
    mClipper->SetHWnd(0, hWnd);
    mPrimary->SetClipper(mClipper);

    return true;

EXIT:
    Cleanup();
    return false;
}

void DDraw::Cleanup()
{
    SAFE_RELEASE(mClipper);
    SAFE_RELEASE(mBack);
    SAFE_RELEASE(mPrimary);
    SAFE_RELEASE(mDD7);
    SAFE_RELEASE(mDD);
}

void DDraw::UpdateWindowPos()
{
    GetClientRect(mhWnd, &mWindowRect);
    ClientToScreen(mhWnd, (POINT*)&mWindowRect.left);
    ClientToScreen(mhWnd, (POINT*)&mWindowRect.right);
}

void DDraw::UpdateWindowSize()
{
    SAFE_RELEASE(mBack);

    UpdateWindowPos();

    const uint32_t width = mWindowRect.right - mWindowRect.left;
    const uint32_t height = mWindowRect.bottom - mWindowRect.top;
    CreateBackBuffer(width, height);
}

bool DDraw::CreateBackBuffer(const uint32_t width, const uint32_t height)
{
    AssertW(mDD7 != nullptr, L"DDraw object is nullptr");

    HRESULT hr;
    DDSURFACEDESC2 ddsd = {};

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    ddsd.dwHeight = height;
    ddsd.dwWidth = width;

    hr = mDD7->CreateSurface(&ddsd, &mBack, nullptr);
    if (FAILED(hr))
    {
        goto EXIT;
    }

    mWidth = width;
    mHeight = height;

    return true;

EXIT:
    return false;
}

bool DDraw::BeginDraw()
{
    AssertW(mBack != nullptr, L"th back buffer is nullptr");

    HRESULT hr;
    DDSURFACEDESC2 ddsd = {};

    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = mBack->Lock(nullptr, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr);
    if (FAILED(hr))
    {
        goto EXIT;
    }
    
    AssertW(ddsd.dwWidth == mWidth, L"Mismatch width");
    AssertW(ddsd.dwHeight == mHeight, L"Mismatch height");

    mLockedBackBuffer = (char*)ddsd.lpSurface;
    mLockedBackBufferPitch = ddsd.lPitch;

    return true;

EXIT:
    return false;
}

void DDraw::EndDraw()
{
    AssertW(mBack != nullptr, L"The back buffer is nullptr");
    mBack->Unlock(nullptr);

    mLockedBackBuffer = nullptr;
    mLockedBackBufferPitch = 0;
}

void DDraw::Clear(const uint32_t color)
{
    AssertW(mLockedBackBuffer != nullptr, L"The back buffer is not locked");

    for (size_t y = 0; y < mHeight; ++y)
    {
        for (size_t x = 0; x < mWidth; ++x)
        {
            uint32_t* dest = (uint32_t*)(mLockedBackBuffer + y * mLockedBackBufferPitch + x * 4);
            *dest = color;
        }
    }
}

void DDraw::Blt()
{
    mPrimary->Blt(&mWindowRect, mBack, nullptr, DDBLT_WAIT, nullptr);
}

bool DDraw::BeginGDI(HDC* outHdc)
{
    HRESULT hr;
    HDC hdc;

    hr = mBack->GetDC(&hdc);
    if (FAILED(hr))
    {
        AssertW(false, L"Failed to GetDC");
        return false;
    }

    *outHdc = hdc;

    return true;
}

void DDraw::EndGDI(const HDC hdc)
{
    mBack->ReleaseDC(hdc);
}

void DDraw::PrintText(const HDC hdc, const wchar_t* const text, const int32_t destX, const int32_t destY, const uint32_t length, const uint32_t color)
{
    const uint32_t r = (color & 0x00ff0000) >> 16;
    const uint32_t g = (color & 0x0000ff00) >> 8;
    const uint32_t b = (color & 0x000000ff);

    SetTextColor(hdc, RGB(r, g, b));
    SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, destX, destY, text, length);
}

void DDraw::DrawPixel(const uint32_t destX, const uint32_t destY, const uint32_t color)
{
    AssertW(mLockedBackBuffer != nullptr, L"The back buffer is not locked");
    //AssertW(destX <= mWidth && destY <= mHeight, L"Out of range x, y");

    if (destX >= mWidth || destY >= mHeight)
    {
        return;
    }

    //uint32_t* dest = (uint32_t*)(mLockedBackBuffer + destY * mLockedBackBufferPitch + destX * 4);
    //*dest = color;
    *(uint32_t*)(mLockedBackBuffer + destY * mLockedBackBufferPitch + destX * 4) = color;
}

void DDraw::DrawLineDDA(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const uint32_t color)
{
    uint32_t startX = MAX(x0, 0);
    uint32_t startY = MAX(y0, 0);
    uint32_t endX = MIN((uint32_t)x1, mWidth);
    uint32_t endY = MIN((uint32_t)y1, mHeight);

    startX = MIN((uint32_t)x0, mWidth);
    startY = MIN((uint32_t)y0, mHeight);
    endX = MAX(x1, 0);
    endY = MAX(y1, 0);

    float x = (float)startX;
    float y = (float)startY;

    int32_t dx = endX - startX;
    int32_t dy = endY - startY;
    uint32_t steps = (ABS(dx) > (ABS(dy))) ? ABS(dx) : ABS(dy);

    float xIncreament = dx / (float)steps;
    float yIncreament = dy / (float)steps;

    DrawPixel(startX, startY, color);

    for (uint32_t i = 0; i < steps; ++i)
    {
        x += xIncreament;
        y += yIncreament;

        DrawPixel(ROUND(x), ROUND(y), color);
    }
}

void DDraw::DrawLineBresenham(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const uint32_t color)
{
    uint32_t startX = MAX(x0, 0);
    uint32_t startY = MAX(y0, 0);
    uint32_t endX = MIN((uint32_t)x1, mWidth);
    uint32_t endY = MIN((uint32_t)y1, mHeight);

    startX = MIN((uint32_t)x0, mWidth);
    startY = MIN((uint32_t)y0, mHeight);
    endX = MAX(x1, 0);
    endY = MAX(y1, 0);

    int32_t width = x1 - x0;
    int32_t height = y1 - y0;
    bool bGradualSlope = (ABS(width) >= ABS(height)) ? true : false;

    int32_t dx = (width >= 0) ? 1 : -1;
    int32_t dy = (height >= 0) ? 1 : -1;
    int32_t w = dx * width;
    int32_t h = dy * height;

    int32_t p = (bGradualSlope) ? 2 * h - w : 2 * w - h;
    int32_t p1 = (bGradualSlope) ? 2 * h : 2 * w;
    int32_t p2 = (bGradualSlope) ? 2 * h - 2 * w : 2 * w - 2 * h;

    uint32_t x = startX;
    uint32_t y = startY;

    if (bGradualSlope)
    {
        while (x != endX)
        {
            DrawPixel(x, y, color);

            if (p < 0)
            {
                p += p1;
            }
            else
            {
                y += dy;
                p += p2;
            }

            x += dx;
        }
    }
    else
    {
        while (y != endY)
        {
            DrawPixel(x, y, color);

            if (p < 0)
            {
                p += p1;
            }
            else
            {
                x += dx;
                p += p2;
            }

            y += dy;
        }
    }

    DrawPixel(x, y, color);
}

void DDraw::DrawCircle(const int32_t centerX, const int32_t centerY, const uint32_t radius, const uint32_t color)
{
    int32_t x = 0;
    int32_t y = radius;
    int32_t p = 1 - radius;

    DrawPixel(centerX + radius, centerY, color);
    DrawPixel(centerX - radius, centerY, color);
    DrawPixel(centerX, centerY + radius, color);
    DrawPixel(centerX, centerY - radius, color);

    while (x < y)
    {
        ++x;
        if (p < 0)
        {
            p = p + 2 * (x + 1);
        }
        else
        {
            --y;
            p = p + 2 * (x - y) + 1;
        }

        DrawPixel(centerX + x, centerY + y, color);
        DrawPixel(centerX + y, centerY + x, color);
        DrawPixel(centerX + y, centerY - x, color);
        DrawPixel(centerX + x, centerY - y, color);
        DrawPixel(centerX - x, centerY - y, color);
        DrawPixel(centerX - y, centerY - x, color);
        DrawPixel(centerX - y, centerY + x, color);
        DrawPixel(centerX - x, centerY + y, color);
    }
}

void DDraw::DrawRectangle(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const uint32_t color)
{
    AssertW(mLockedBackBuffer != nullptr, L"The back buffer is not locked");

    const uint32_t startX = MAX(destX, 0);
    const uint32_t startY = MAX(destY, 0);
    const uint32_t endX = MIN(destX + width, mWidth);
    const uint32_t endY = MIN(destY + height, mHeight);

    for (uint32_t y = startY; y < endY; ++y)
    {
        for (uint32_t x = startX; x < endX; ++x)
        {
            DrawPixel(x, y, color);
        }
    }
}

void DDraw::DrawBitmap(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const char* const bitmap)
{
    AssertW(mLockedBackBuffer != nullptr, L"The back buffer is not locked");
    AssertW(bitmap != nullptr, L"bitmap is nullptr");

    const uint32_t srcX = (destX >= 0) ? 0 : -destX;
    const uint32_t srcY = (destY >= 0) ? 0 : -destY;

    const uint32_t startX = MAX(destX, 0);
    const uint32_t startY = MAX(destY, 0);
    const uint32_t endX = MIN(destX + (int32_t)width, (int32_t)mWidth);
    const uint32_t endY = MIN(destY + (int32_t)height, (int32_t)mHeight);

    const uint32_t destWidth = endX - startX;
    const uint32_t destHeight = endY - startY;
    if (destWidth > mWidth || destHeight > mHeight)
    {
        return;
    }

    const char* src = bitmap + (srcY * width + srcX) * 4;
    char* dest = mLockedBackBuffer + startY * mLockedBackBufferPitch + startX * 4;

    for (uint32_t y = startY; y < endY; ++y)
    {
        for (uint32_t x = startX; x < endX; ++x)
        {
            *(uint32_t*)dest = *(uint32_t*)src;

            src += 4;
            dest += 4;
        }

        src -= destWidth * 4;
        src += width * 4;
        dest -= destWidth * 4;
        dest += mLockedBackBufferPitch;
    }
}

void DDraw::DrawBitmapWithColorKey(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const char* const bitmap, const uint32_t colorKey)
{
    AssertW(mLockedBackBuffer != nullptr, L"The back buffer is not locked");
    AssertW(bitmap != nullptr, L"bitmap is nullptr");

    const uint32_t srcX = (destX >= 0) ? 0 : -destX;
    const uint32_t srcY = (destY >= 0) ? 0 : -destY;

    const uint32_t startX = MAX(destX, 0);
    const uint32_t startY = MAX(destY, 0);
    const uint32_t endX = MIN(destX + (int32_t)width, (int32_t)mWidth);
    const uint32_t endY = MIN(destY + (int32_t)height, (int32_t)mHeight);

    const uint32_t destWidth = endX - startX;
    const uint32_t destHeight = endY - startY;
    if (destWidth > mWidth || destHeight > mHeight)
    {
        return;
    }

    const char* src = bitmap + (srcY * width + srcX) * 4;
    char* dest = mLockedBackBuffer + startY * mLockedBackBufferPitch + startX * 4;

    for (uint32_t y = 0; y < destHeight; ++y)
    {
        for (uint32_t x = 0; x < destWidth; ++x)
        {
            if (*(uint32_t*)src != colorKey)
            {
                *(uint32_t*)dest = *(uint32_t*)src;
            }

            src += 4;
            dest += 4;
        }

        src -= destWidth * 4;
        src += width * 4;
        dest -= destWidth * 4;
        dest += mLockedBackBufferPitch;
    }
}