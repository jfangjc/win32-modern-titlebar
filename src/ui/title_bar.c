#include <windows.h>
#include <dwmapi.h>

#include "events.h"
#include "font.h"
#include "color.h"
#include "resource.h"
#include "menu.h"

#define TITLE_BAR_HEIGHT 35
#define TITLE_BAR_BORDER 2
#define TITLE_BAR_BUTTON_WIDTH 47

#define MENU_BORDER_OFFSET 4
#define MENU_HEIGHT_OFFSET 20
#define MENU_ITEM_OFFSET 6
#define MENU_FONT_SIZE 18

#define ICON_SIZE 25

static struct title_bar_rect title_bar_rect;
static struct menu_item menu_items[MENU_ITEM_COUNT];
static HFONT menu_font;

void cal_item_offset(HWND handle);

void init_title_bar(HWND handle) {
    int font_size = DPI_SCALE(MENU_FONT_SIZE, GetDpiForWindow(handle));
    menu_font = create_font(font_size, L"Segoe UI");
    init_menu(menu_items);
    cal_item_offset(handle);
}

void cal_item_offset(HWND handle) {
    GetClientRect(handle, &(title_bar_rect.title_bar));
    title_bar_rect.title_bar.bottom = TITLE_BAR_BORDER + DPI_SCALE(TITLE_BAR_HEIGHT, GetDpiForWindow(handle));
    HDC hdc = GetDC(handle);
    SelectObject(hdc, menu_font);
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);

    long top = (title_bar_rect.title_bar.bottom - tm.tmHeight) / 2 - 3;
    long bottom = top + tm.tmHeight + 3;
    int offset = DPI_SCALE(5, GetDpiForWindow(handle));

    int icon_size = DPI_SCALE(ICON_SIZE, GetDpiForWindow(handle));
    title_bar_rect.icon.top = (title_bar_rect.title_bar.bottom - icon_size) / 2 - 2;
    title_bar_rect.icon.left = offset;

    offset += 12 + icon_size;
    for (size_t i = 0; i < MENU_ITEM_COUNT; ++i) {
        SIZE text_size;
        GetTextExtentPoint32W(hdc, menu_items[i].text, (int)wcslen(menu_items[i].text), &text_size);
        title_bar_rect.menu[i].left = offset;
        title_bar_rect.menu[i].right = 20 + offset + text_size.cx;
        title_bar_rect.menu[i].top = top;
        title_bar_rect.menu[i].bottom = bottom;
        title_bar_rect.menu_hover[i].top = title_bar_rect.title_bar.top;
        title_bar_rect.menu_hover[i].bottom = title_bar_rect.title_bar.bottom;
        title_bar_rect.menu_hover[i].left = title_bar_rect.menu[i].left;
        title_bar_rect.menu_hover[i].right = title_bar_rect.menu[i].right;
        offset = title_bar_rect.menu[i].right + MENU_ITEM_OFFSET;
    }

    ReleaseDC(handle, hdc);

    int button_width = DPI_SCALE(TITLE_BAR_BUTTON_WIDTH, GetDpiForWindow(handle));
    title_bar_rect.close = title_bar_rect.title_bar;
    title_bar_rect.close.top += 1;
    title_bar_rect.close.left = title_bar_rect.close.right - button_width;

    title_bar_rect.maximize = title_bar_rect.close;
    title_bar_rect.maximize.left -= button_width;
    title_bar_rect.maximize.right -= button_width;

    title_bar_rect.minimize = title_bar_rect.maximize;
    title_bar_rect.minimize.left -= button_width;
    title_bar_rect.minimize.right -= button_width;
}

static void center_inner_rect(RECT *inner_rect, const RECT *outer_rect) {
    int width = inner_rect->right - inner_rect->left;
    int height = inner_rect->bottom - inner_rect->top;

    inner_rect->left = outer_rect->left + (outer_rect->right - outer_rect->left - width) / 2;
    inner_rect->top = outer_rect->top + (outer_rect->bottom - outer_rect->top - height) / 2;
    inner_rect->right = inner_rect->left + width;
    inner_rect->bottom = inner_rect->top + height;
}

void draw_title_bar(HWND handle, HDC hdc, LONG_PTR hovered_button) {
    HBRUSH bg_brush = CreateSolidBrush(get_colors().bg);
    HBRUSH bg_darker_brush = CreateSolidBrush(get_colors().bg_darker);
    HBRUSH bg_hover_brush = CreateSolidBrush(get_colors().bg_hover);

    cal_item_offset(handle);
    FillRect(hdc, &(title_bar_rect.title_bar), bg_darker_brush);

    { // Painting icon
        HINSTANCE hinstance = 0;
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            L"window_module", &hinstance);
        HICON icon = LoadIcon(hinstance, MAKEINTRESOURCE(IDC_APP_ICON));
        int width = DPI_SCALE(ICON_SIZE, GetDpiForWindow(handle));
        DrawIconEx(hdc, title_bar_rect.icon.left, title_bar_rect.icon.top, icon,
        width, width, 10, NULL, DI_NORMAL);
    }

    { // Painting menu items
        SelectObject(hdc, menu_font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetForegroundWindow() == handle ? get_colors().fg : get_colors().fg_no_focus);
        int round_radius = DPI_SCALE(10, GetDpiForWindow(handle));
        if (hovered_button >= 0 && hovered_button < 7) {
            SelectObject(hdc, bg_hover_brush);
            SelectObject(hdc, GetStockObject(NULL_PEN));
            RoundRect(hdc, title_bar_rect.menu[hovered_button].left, title_bar_rect.menu[hovered_button].top,
                title_bar_rect.menu[hovered_button].right, title_bar_rect.menu[hovered_button].bottom,
                round_radius, round_radius);
        }
        for (int i = 0; i < 7; i++) {
            DrawTextW(hdc, menu_items[i].text, -1, (LPRECT)&title_bar_rect.menu[i],
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
    }

    // Painting caption buttons
    int icon_size = DPI_SCALE(10, GetDpiForWindow(handle));
    HFONT icon_font = create_font(icon_size, L"Segoe Fluent Icons");
    HFONT old_font = SelectObject(hdc, icon_font);
    {
        if (hovered_button == BUTTON_MINIMIZE) {
            FillRect(hdc, &title_bar_rect.minimize, bg_hover_brush);
        }

        COLORREF old_fg = SetTextColor(hdc, get_colors().fg);
        COLORREF old_bg = SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, L"\uE921", -1, &title_bar_rect.minimize, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, old_fg);
        SetBkColor(hdc, old_bg);
    }

    {
        if (hovered_button == BUTTON_MAXIMIZE) {
            FillRect(hdc, &title_bar_rect.maximize, bg_hover_brush);
        }

        COLORREF old_fg = SetTextColor(hdc, get_colors().fg);
        COLORREF old_bg = SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, IsZoomed(handle) ? L"\uE923" : L"\uE922", -1, &title_bar_rect.maximize, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, old_fg);
        SetBkColor(hdc, old_bg);
    }

    {
        COLORREF old_fg;
        if (hovered_button == BUTTON_CLOSE) {
            HBRUSH brush = CreateSolidBrush(0x2409e9);
            FillRect(hdc, &title_bar_rect.close, brush);
            DeleteObject(brush);
            old_fg = SetTextColor(hdc, 0xfffcff);
        }
        else {
            old_fg = SetTextColor(hdc, get_colors().fg);
        }
        COLORREF old_bg = SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, L"\uE8BB", -1, &title_bar_rect.close, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, old_fg);
        SetBkColor(hdc, old_bg);
    }

    SelectObject(hdc, old_font);

    DeleteObject(bg_brush);
    DeleteObject(bg_darker_brush);
    DeleteObject(bg_hover_brush);
    DeleteObject(icon_font);
}

struct menu_item* get_menu_items() {
    return menu_items;
}

struct title_bar_rect* get_title_bar_rect() {
    return &title_bar_rect;
}

void rounded_menu(HWND handle) {
    (void)handle;
    HWND menu = FindWindowW(L"#32768", NULL);
    if (menu) {
        DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUNDSMALL;
        DwmSetWindowAttribute(menu, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
        COLORREF color = get_colors().bg_menu_border;
        DwmSetWindowAttribute(menu, DWMWA_BORDER_COLOR, &color, sizeof(color));
    }
}

void measure_menu_item(HWND handle, LPMEASUREITEMSTRUCT lpmis) {
    if (lpmis->itemID == 0) { // Fake padding on top and bottom
        lpmis->itemWidth = GetSystemMetrics(SM_CXSMICON) + 5 + 100;
        lpmis->itemHeight = MENU_BORDER_OFFSET;
        return;
    }
    else if (lpmis->itemID == 1) { // Sepeator
        lpmis->itemWidth = GetSystemMetrics(SM_CXSMICON) + 5 + 100;
        lpmis->itemHeight = 11;
        return;
    }
    HDC hdc = GetDC(handle);
    struct menu_text* menu_text = (struct menu_text*)lpmis->itemData;
    if (menu_text != NULL) {
        SIZE name_size = { 0 };
        SelectObject(hdc, menu_font);
        GetTextExtentPoint32W(hdc, menu_text->name, (int)wcslen(menu_text->name), &name_size);
        lpmis->itemWidth = name_size.cx + GetSystemMetrics(SM_CXSMICON) + 5 + 100;
        lpmis->itemHeight = name_size.cy + MENU_HEIGHT_OFFSET;
        if (menu_text->shortcut != NULL) {
            SIZE shortcut_size = { 0 };
            GetTextExtentPoint32W(hdc, menu_text->shortcut, (int)wcslen(menu_text->shortcut), &shortcut_size);
            lpmis->itemWidth += shortcut_size.cx;
        }
        ReleaseDC(handle, hdc);
    }
}

void draw_menu_item(HWND handle, LPDRAWITEMSTRUCT lpdis) {
    (void)handle;
    if (lpdis->itemID == 0) { // Fake padding
        RECT rect = lpdis->rcItem;
        FillRect(lpdis->hDC, &(rect), CreateSolidBrush(get_colors().bg_menu));
        return;
    }
    else if (lpdis->itemID == 1) { // Sepeator
        RECT rect = lpdis->rcItem;
        FillRect(lpdis->hDC, &(rect), CreateSolidBrush(get_colors().bg_menu));
        rect.top += 5;
        rect.bottom = rect.top + 1;
        FillRect(lpdis->hDC, &(rect), CreateSolidBrush(get_colors().fg_no_focus));
        return;
    }

    SetCursor(LoadCursor(NULL, IDC_HAND));
    FillRect(lpdis->hDC, &(lpdis->rcItem), CreateSolidBrush(get_colors().bg_menu));
    if (lpdis->itemState & ODS_SELECTED) { // If hovered
        HBRUSH brush = SelectObject(lpdis->hDC, CreateSolidBrush(get_colors().bg_hover));
        HPEN pen = SelectObject(lpdis->hDC, GetStockObject(NULL_PEN));
        int radius = 10;

        RoundRect(lpdis->hDC, lpdis->rcItem.left + MENU_BORDER_OFFSET, lpdis->rcItem.top,
            lpdis->rcItem.right - MENU_BORDER_OFFSET, lpdis->rcItem.bottom, radius, radius);

        SelectObject(lpdis->hDC, brush);
        SelectObject(lpdis->hDC, pen);
    }

    COLORREF fg = SetTextColor(lpdis->hDC, get_colors().fg);
    COLORREF bg = SetBkMode(lpdis->hDC, TRANSPARENT);
    SelectObject(lpdis->hDC, menu_font);

    struct menu_text* menu_text = (struct menu_text*)lpdis->itemData;

    RECT rect = lpdis->rcItem;
    rect.left += GetSystemMetrics(SM_CXSMICON) + 5;
    rect.right -= GetSystemMetrics(SM_CXSMICON) + 5;
    DrawTextW(lpdis->hDC, menu_text->name, -1, &(rect), DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
    if (menu_text->shortcut != NULL) {
        SetTextColor(lpdis->hDC, get_colors().fg_no_focus);
        DrawTextW(lpdis->hDC, menu_text->shortcut, -1, &(rect), DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
    }

    SetTextColor(lpdis->hDC, fg);
    SetBkColor(lpdis->hDC, bg);
}