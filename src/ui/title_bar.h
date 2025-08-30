#ifndef WINDOWS_TITLE_BAR
#define WINDOWS_TITLE_BAR

#include <windows.h>

#define MENU_ITEM_COUNT 7

struct menu_item {
    WCHAR* text;
    HMENU menu;
    int is_hovered;
};

struct title_bar_rect {
    RECT title_bar;
    RECT icon;
    RECT menu[MENU_ITEM_COUNT];
    RECT menu_hover[MENU_ITEM_COUNT];
    RECT close;
    RECT maximize;
    RECT minimize;
};

enum buttons {
    BUTTON_NONE = -1,
    BUTTON_MINIMIZE = 7,
    BUTTON_MAXIMIZE,
    BUTTON_CLOSE,
};

struct title_bar_rect* get_title_bar_rect();
struct menu_item* get_menu_items();

void init_title_bar(HWND handle);
void draw_title_bar(HWND handle, HDC hdc, LONG_PTR hovered_button);
void rounded_menu(HWND handle);
void measure_menu_item(HWND handle, LPMEASUREITEMSTRUCT lpmis);
void draw_menu_item(HWND handle, LPDRAWITEMSTRUCT lpdis);

#endif