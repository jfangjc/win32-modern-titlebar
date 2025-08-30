#include "window.h"

#include <windows.h>
#include <dwmapi.h>

#include "events.h"
#include "resource.h"
#include "color.h"

static int register_window_class(HINSTANCE hinstance, LPCWSTR class_name) {
    WNDCLASSEXW wc = { 0 };

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = win_proc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = class_name;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = LoadIconW(hinstance, MAKEINTRESOURCEW(IDC_APP_ICON));

    if (!RegisterClassExW(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONERROR | MB_OK);
    }
    return 0;
}

int windows_create_window() {
    struct colors colors;
    colors.fg = 0xF2F8F8;
    colors.fg_no_focus = 0xA47262;
    colors.bg = 0x362a28;
    colors.bg_darker = 0x2C2221;
    colors.bg_hover = 0x483f3f;
    colors.bg_menu = 0x463734;
    colors.bg_menu_border = 0x463734;
    init_color(colors);

    LPCWSTR class_name = L"demo";
    HINSTANCE hinstance = 0;
    if (!GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        L"window_module", &hinstance)) {
        return 0;
    }

    register_window_class(hinstance, class_name);

    HWND handle;
    // WS_EX_COMPOSITED is required to fix flickering
    handle = CreateWindowExW(
        WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_COMPOSITED,
        class_name, L"demo",
        WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        HWND_DESKTOP, NULL, hinstance, NULL
    );

    if (handle == NULL) {
        return 0;
    }

    int light = is_light();
    DwmSetWindowAttribute(handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &light, sizeof(light));
    EnableDarkModeForApp();

    ShowWindow(handle, SW_MAXIMIZE);
    SetLayeredWindowAttributes(handle, 0x00000000, 255, LWA_ALPHA);
    UpdateWindow(handle);

    SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTProc, NULL, GetCurrentThreadId());
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}