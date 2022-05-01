#include "Precompiled.h"
#include "Dialog.h"
#include "Window.h"
#include "WindowUtil.h"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    constexpr uint32_t WINDOW_WIDTH = 800;
    constexpr uint32_t WINDOW_HEIGHT = 600;

    Renderer* renderer = nullptr;

    if (!window::Create(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        AssertW(false, L"Failed to Create Window");
        goto EXIT;
    }

    if (!dialog::Create(hInstance, window::ghWindow))
    {
        AssertW(false, L"Failed to Create Dialog");
        goto EXIT;
    }

    window_util::ShowWindow(window::ghWindow);
    window_util::ShowWindow(dialog::ghDialog);

    window_util::CenterWindow(window::ghWindow, dialog::ghDialog);
    window_util::CenterWindow(window::ghWindow, dialog::ghDialog);

    renderer = new Renderer;
    if (!renderer->Initialize(window::ghWindow))
    {
        AssertW(false, L"Failed to Initialize renderer");
        goto EXIT;
    }

    {
        window::gUpdateWindowPosFunc = [&renderer]() {
            renderer->UpdateWindowPos();
        };

        dialog::gDialogEventHandler = [&renderer](HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) -> BOOL {
            return renderer->GetCanvas()->DialogEventHandler(hDlg, iMessage, wParam, lParam);
        };
    }

    while (window::Tick())
    {
        renderer->Tick();
    }

EXIT:
    renderer->Shutdown();
    delete renderer;

    dialog::Destroy();
    window::Destroy();

    return 0;
}