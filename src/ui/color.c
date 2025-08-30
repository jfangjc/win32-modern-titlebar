#include "color.h"

static struct colors global_colors;

void init_color(struct colors colors) {
    global_colors = colors;
    if (!is_light()) {

    }
}

struct colors get_colors() {
    return global_colors;
}

void set_color(COLORREF color, int type) {
    (void)color;
    (void)type;
}

int is_light() {
    COLORREF color = get_colors().bg;
    return (((5 * ((color >> 8) & 0x0000ff)) + (2 * ((color >> 16) & 0x0000ff)) + (color & 0x0000ff)) > (8 * 128));
}

typedef int (WINAPI* fnShouldAppsUseDarkMode)();
typedef int (WINAPI* fnAllowDarkModeForWindow)(HWND hWnd, int allow);
typedef DWORD (WINAPI* fnSetPreferredAppMode)(DWORD appMode);

enum PreferredAppMode {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

fnShouldAppsUseDarkMode ShouldAppsUseDarkMode = NULL;
fnAllowDarkModeForWindow AllowDarkModeForWindow = NULL;
fnSetPreferredAppMode SetPreferredAppMode = NULL;

void InitDarkModeFunctions()
{
    HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxtheme)
    {
        ShouldAppsUseDarkMode = (fnShouldAppsUseDarkMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132));
        AllowDarkModeForWindow = (fnAllowDarkModeForWindow)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133));
        SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
    }
}

// Call this early in your WinMain or during window creation
void EnableDarkModeForApp() {
    InitDarkModeFunctions();
    if (SetPreferredAppMode) {
        SetPreferredAppMode(ForceDark);
    }
}
