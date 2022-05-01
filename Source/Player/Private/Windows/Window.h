#pragma once

namespace window
{
    static HINSTANCE ghInstance;
    static HWND ghWindow;
    static wchar_t gAppName[128] = L"DrawingAlgorithms with DirectDraw 7 - ";
    static wchar_t gExtendedAppName[128] = L"20174067 강범수, 20194122 권하준";
    static std::function<void(void)> gUpdateWindowPosFunc;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool Create(HINSTANCE hInstance, const uint32_t windowWidth, const uint32_t windowHeight);
    void Destroy();
    bool Tick();
}

LRESULT CALLBACK window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        uint32_t x = GET_X_LPARAM(lParam);
        uint32_t y = GET_Y_LPARAM(lParam);
        event::mouse::OnClick(x, y);
        break;
    }
    case WM_MOVE:
        if (gUpdateWindowPosFunc)
        {
            gUpdateWindowPosFunc();
        }
        break;
    case WM_KEYDOWN:
        event::keyboard::KeyDown((uint8_t)wParam);
        break;
    case WM_KEYUP:
        event::keyboard::KeyUp((uint8_t)wParam);
        break;
    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

bool window::Create(HINSTANCE hInstance, const uint32_t windowWidth, const uint32_t windowHeight)
{
    ghInstance = hInstance;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = gAppName;
    wc.hIconSm = nullptr;

    RegisterClassExW(&wc);

    RECT windowRect = { 0, 0, (LONG)windowWidth, (LONG)windowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

    static_assert(sizeof(gAppName) == sizeof(wchar_t) * 128, "Mismatch size");
    static_assert(sizeof(gExtendedAppName) == sizeof(wchar_t) * 128, "Mismatch size");
    wchar_t appName[256];

    swprintf(appName, L"%s%s", gAppName, gExtendedAppName);

    ghWindow = CreateWindowExW(0, gAppName, appName, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (ghWindow == nullptr)
    {
        return false;
    }

    return true;
}

void window::Destroy()
{
    DestroyWindow(ghWindow);
}

bool window::Tick()
{
    MSG msg = {};

    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);

        if (msg.message == WM_QUIT)
        {
            return false;
        }
    }

    return true;
}