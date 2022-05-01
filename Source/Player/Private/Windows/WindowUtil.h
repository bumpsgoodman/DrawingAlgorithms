#pragma once

namespace window_util
{
    void ShowWindow(HWND hWnd);
    void CenterWindow(HWND hWnd);
    void CenterWindow(HWND hWnd, HWND hDlg);
}

void window_util::ShowWindow(HWND hWnd)
{
    ::ShowWindow(hWnd, SW_SHOW);
}

void window_util::CenterWindow(HWND hWnd)
{
    const uint32_t SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
    const uint32_t SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
    const uint32_t CENTER_SCREEN_X = SCREEN_WIDTH / 2;
    const uint32_t CENTER_SCREEN_Y = SCREEN_HEIGHT / 2;

    RECT windowRect;

    GetWindowRect(hWnd, &windowRect);
    const uint32_t WINDOW_WIDTH = windowRect.right - windowRect.left;
    const uint32_t WINDOW_HEIGHT = windowRect.bottom - windowRect.top;

    MoveWindow(hWnd, CENTER_SCREEN_X - WINDOW_WIDTH / 2, CENTER_SCREEN_Y - WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT, TRUE);
}

void window_util::CenterWindow(HWND hWnd, HWND hDlg)
{
    const uint32_t SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
    const uint32_t SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
    const uint32_t CENTER_SCREEN_X = SCREEN_WIDTH / 2;
    const uint32_t CENTER_SCREEN_Y = SCREEN_HEIGHT / 2;

    RECT windowRect;
    RECT dialogRect;

    GetWindowRect(hWnd, &windowRect);
    GetWindowRect(hDlg, &dialogRect);

    const uint32_t WINDOW_WIDTH = windowRect.right - windowRect.left;
    const uint32_t WINDOW_HEIGHT = windowRect.bottom - windowRect.top;
    const uint32_t DIALOG_WIDTH = dialogRect.right - dialogRect.left;
    const uint32_t DIALOG_HEIGHT = dialogRect.bottom - dialogRect.top;

    const uint32_t WIDTH = WINDOW_WIDTH + DIALOG_WIDTH;
    const uint32_t HEIGHT = WINDOW_HEIGHT;

    MoveWindow(hWnd, CENTER_SCREEN_X - WIDTH / 2, CENTER_SCREEN_Y - HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT, TRUE);
    MoveWindow(hDlg, CENTER_SCREEN_X - WIDTH / 2 + WINDOW_WIDTH, CENTER_SCREEN_Y - HEIGHT / 2, DIALOG_WIDTH, DIALOG_HEIGHT, TRUE);
}