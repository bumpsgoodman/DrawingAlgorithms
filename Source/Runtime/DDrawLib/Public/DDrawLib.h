#pragma once

class DDraw final
{
public:
    DDraw() = default;
    DDraw(const DDraw&) = delete;
    DDraw(const DDraw&&) = delete;
    DDraw& operator=(const DDraw&) = delete;
    ~DDraw();

    bool Initialize(const HWND hWnd);
    void Cleanup();

    void UpdateWindowPos();
    void UpdateWindowSize();
    bool CreateBackBuffer(const uint32_t width, const uint32_t height);

    inline HWND GetHWnd() const { return mhWnd; }
    inline uint32_t GetWidth() const { return mWidth; }
    inline uint32_t GetHeight() const { return mHeight; }

    bool BeginDraw();
    void EndDraw();
    void Clear(const uint32_t color);
    void Blt();

    bool BeginGDI(HDC* outHdc);
    void EndGDI(const HDC hdc);
    void PrintText(const HDC hdc, const wchar_t* const text, const int32_t destX, const int32_t destY, const uint32_t length, const uint32_t color);

    void DrawPixel(const uint32_t destX, const uint32_t destY, const uint32_t color);
    void DrawLineDDA(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const uint32_t color);
    void DrawLineBresenham(const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1, const uint32_t color);
    void DrawCircle(const int32_t centerX, const int32_t centerY, const uint32_t radius, const uint32_t color);
    void DrawRectangle(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const uint32_t color);
    void DrawBitmap(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const char* const bitmap);
    void DrawBitmapWithColorKey(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const char* const bitmap, const uint32_t colorKey);

private:
    HWND mhWnd = 0;
    RECT mWindowRect = {};
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;

    char* mLockedBackBuffer = nullptr;
    uint32_t mLockedBackBufferPitch = 0;

    IDirectDraw* mDD = nullptr;
    IDirectDraw7* mDD7 = nullptr;
    IDirectDrawSurface7* mPrimary = nullptr;
    IDirectDrawSurface7* mBack = nullptr;
    IDirectDrawClipper* mClipper = nullptr;
};