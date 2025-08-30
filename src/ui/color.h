#ifndef WINDOWS_COLOR
#define WINDOWS_COLOR

#include <windows.h>

struct colors {
    COLORREF fg;
    COLORREF fg_no_focus;
    COLORREF bg;
    COLORREF bg_darker;
    COLORREF bg_hover;
    COLORREF bg_menu;
    COLORREF bg_menu_border;
};

void init_color(struct colors colors);
struct colors get_colors();
int is_light();
void EnableDarkModeForApp();

#endif