#pragma once

namespace dialog
{
    static HINSTANCE ghInstance;
    static HWND ghDialog;

    static std::function<BOOL(HWND, UINT, WPARAM, LPARAM)> gDialogEventHandler;

    BOOL CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

    bool Create(HINSTANCE hInstance, HWND hParent);
    void Destroy();
}

BOOL CALLBACK dialog::DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch (iMessage)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_RADIO_PIXEL, IDC_RADIO_REGION, IDC_RADIO_PIXEL);
        SetDlgItemText(hDlg, IDC_EDIT_RADIUS, L"10");
        SetDlgItemText(hDlg, IDC_EDIT_TRANSLATION_X, L"1");
        SetDlgItemText(hDlg, IDC_EDIT_TRANSLATION_Y, L"0");
        SetDlgItemText(hDlg, IDC_EDIT_SCALING_X, L"1.5");
        SetDlgItemText(hDlg, IDC_EDIT_SCALING_Y, L"1.5");
        SetDlgItemText(hDlg, IDC_EDIT_ROTATION_SINX, L"0.1736");
        SetDlgItemText(hDlg, IDC_EDIT_ROTATION_COSX, L"0.9848");
        return TRUE;
    default:
        if (gDialogEventHandler)
        {
            return gDialogEventHandler(hDlg, iMessage, wParam, lParam);
        }
        break;
    }

    return FALSE;
}

bool dialog::Create(HINSTANCE hInstance, HWND hParent)
{
    ghDialog = CreateDialogW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_UTIL), hParent, (DLGPROC)&DialogProc);
    if (ghDialog == nullptr)
    {
        return false;
    }

    return true;
}

void dialog::Destroy()
{
    EndDialog(ghDialog, 0);
}