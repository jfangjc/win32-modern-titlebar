#include <windows.h>
#include <wingdi.h>

#include "color.h"
#include "title_bar.h"
#include "events.h"

#define HTICON 22

void handle_paint(HWND handle);
void handle_nc_mousemove(HWND handle);
void handle_system_menu(HWND handle, POINT pos);

static UINT dpi;
static LONG_PTR hovered_button;
static HHOOK menu_hook = NULL;
static int update_popup, curr_popup;

// Placeholder for now
LRESULT CALLBACK CBTProc(int code, WPARAM w_param, LPARAM l_param) {
    return CallNextHookEx(0, code, w_param, l_param);
}

static LRESULT CALLBACK menu_mouse_down_proc(int code, WPARAM w_param, LPARAM l_param) {
    if (code == MSGF_MENU) {
        PMSG msg = (PMSG)l_param;
        if (msg->message == WM_MOUSEMOVE) {
            handle_nc_mousemove(msg->hwnd);
            if (curr_popup != hovered_button && hovered_button > -1 && hovered_button < 7) {
                update_popup = 1;
                PostMessageW(msg->hwnd, WM_CANCELMODE, 0, 0);
            }
        }
    }
    return CallNextHookEx(NULL, code, w_param, l_param);
}

LRESULT CALLBACK win_proc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param) {
    hovered_button = GetWindowLongPtrW(handle, GWLP_USERDATA);
    int x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
    int y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
    int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

    switch (msg) {
    case WM_NCCALCSIZE:
        if (!w_param) {
            break;
        }
        RECT* rect = ((NCCALCSIZE_PARAMS*)l_param)->rgrc;
        rect->right -= x + padding;
        rect->left += x + padding;
        rect->bottom -= y + padding;
        if (IsZoomed(handle)) {
            rect->top += y + padding - 1;
        }
        return 0;
    case WM_CREATE:
        SetWindowPos(handle, NULL, 0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
        init_title_bar(handle);
        dpi = GetDpiForWindow(handle);
        break;
    case WM_ACTIVATE:
        InvalidateRect(handle, &(get_title_bar_rect()->title_bar), 0);
        break;
    case WM_DPICHANGED:
        init_title_bar(handle);
        InvalidateRect(handle, &(get_title_bar_rect()->title_bar), 0);
        dpi = GetDpiForWindow(handle);
        break;
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProcW(handle, msg, w_param, l_param);
        switch (hit) {
        case HTNOWHERE:
        case HTRIGHT:
        case HTLEFT:
        case HTTOPLEFT:
        case HTTOP:
        case HTTOPRIGHT:
        case HTBOTTOMRIGHT:
        case HTBOTTOM:
        case HTBOTTOMLEFT: { return hit; }
        }
        if (hovered_button == BUTTON_MAXIMIZE) {
            return HTMAXBUTTON;
        }

        POINT cursor_point = { 0 };
        cursor_point.x = LOWORD(l_param);
        cursor_point.y = HIWORD(l_param);
        ScreenToClient(handle, &cursor_point);
        if (!IsZoomed(handle) && cursor_point.y > 0 && cursor_point.y < y + padding) {
            return HTTOP;
        }

        if (cursor_point.y < get_title_bar_rect()->title_bar.bottom) {
            if (cursor_point.x < get_title_bar_rect()->icon.right) {
                return HTICON;
            }
            return HTCAPTION;
        }

        return HTCLIENT;
    }
    case WM_PAINT:
        handle_paint(handle);
        break;
    case WM_INITMENUPOPUP:
        rounded_menu((HWND)w_param);
        return 1;
    case WM_MEASUREITEM:
        measure_menu_item(handle, (LPMEASUREITEMSTRUCT)l_param);
        return 1;
    case WM_DRAWITEM:
        if (((LPDRAWITEMSTRUCT)l_param)->CtlType == ODT_MENU) {
            draw_menu_item(handle, (LPDRAWITEMSTRUCT)l_param);
        }
        return 1;
    case WM_LBUTTONDOWN:
        break;
    case WM_NCMOUSEMOVE:
        handle_nc_mousemove(handle);
        break;
    case WM_MOUSEMOVE:
        if (hovered_button > -1) {
            InvalidateRect(handle, &(get_title_bar_rect()->title_bar), 0);
            SetWindowLongPtrW(handle, GWLP_USERDATA, (LONG_PTR)BUTTON_NONE);
        }
        break;
    case WM_SIZE: { // Resolve issue when resize, restore button stay hovered
        RECT window_rect = { 0 };
        GetWindowRect(handle, &window_rect);
        POINT point;
        GetCursorPos(&point);

        if (point.x < window_rect.left || point.x > window_rect.right ||
            point.y < window_rect.top || point.y > window_rect.bottom) {
            SetWindowLongPtrW(handle, GWLP_USERDATA, (LONG_PTR)BUTTON_NONE);
            InvalidateRect(handle, &(get_title_bar_rect()->maximize), 0);
        }
        break;
    }
    case WM_NCLBUTTONDOWN:
        if (hovered_button > -1) {
            if (hovered_button < 7) {
                menu_hook = SetWindowsHookExW(WH_MSGFILTER, menu_mouse_down_proc, NULL, GetCurrentThreadId());
                update_popup = 1;
                while (update_popup) {
                    update_popup = 0;
                    curr_popup = (int)hovered_button;
                    POINT point = (POINT){ get_title_bar_rect()->menu[hovered_button].left,
                        get_title_bar_rect()->menu[hovered_button].bottom + 1 };
                    ClientToScreen(handle, &point);
                    TrackPopupMenu(get_menu_items()[hovered_button].menu, TPM_TOPALIGN | TPM_LEFTALIGN,
                        point.x, point.y, 0, handle, NULL);
                }
                if (menu_hook) {
                    UnhookWindowsHookEx(menu_hook);
                    menu_hook = NULL;
                }
            }
            return 0;
        }
        break;
    case WM_NCLBUTTONUP:
        if (hovered_button == BUTTON_CLOSE) {
            PostMessageW(handle, WM_CLOSE, 0, 0);
        }
        else if (hovered_button == BUTTON_MINIMIZE) {
            ShowWindow(handle, SW_MINIMIZE);
        }
        else if (hovered_button == BUTTON_MAXIMIZE) {
            int mode = IsZoomed(handle) ? SW_NORMAL : SW_MAXIMIZE;
            ShowWindow(handle, mode);
        }
        return 0;
    case WM_NCRBUTTONUP:
        if (w_param == HTICON) {
            handle_system_menu(handle, (POINT) { LOWORD(l_param), HIWORD(l_param) });
        }
        break;
    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(handle, msg, w_param, l_param);
}

void handle_paint(HWND handle) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(handle, &ps);
    // Just filling the background of the application
    FillRect(hdc, &(ps.rcPaint), CreateSolidBrush(get_colors().bg));
    // Now draw the title bar
    draw_title_bar(handle, hdc, hovered_button);
    EndPaint(handle, &ps);
}

void handle_nc_mousemove(HWND handle) {
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(handle, &cursor_point);
    struct title_bar_rect* title_bar_rect = get_title_bar_rect();
    int new_hovered_button = BUTTON_NONE;

    if (PtInRect(&(title_bar_rect->close), cursor_point)) {
        new_hovered_button = BUTTON_CLOSE;
    }
    else if (PtInRect(&title_bar_rect->minimize, cursor_point)) {
        new_hovered_button = BUTTON_MINIMIZE;
    }
    else if (PtInRect(&title_bar_rect->maximize, cursor_point)) {
        new_hovered_button = BUTTON_MAXIMIZE;
    }
    else {
        for (int i = 0; i < 7; i++) {
            if (PtInRect(&(title_bar_rect->menu_hover[i]), cursor_point)) {
                new_hovered_button = i;
            }
        }
    }
    if (new_hovered_button != hovered_button) {
        if (hovered_button > -1 && hovered_button < 7
        || new_hovered_button > -1 && new_hovered_button < 7) {
            InvalidateRect(handle, &(get_title_bar_rect()->menu[hovered_button]), 0);
            InvalidateRect(handle, &(get_title_bar_rect()->menu[new_hovered_button]), 0);
        }
        if (hovered_button == BUTTON_MINIMIZE
        || new_hovered_button == BUTTON_MINIMIZE) {
            InvalidateRect(handle, &(get_title_bar_rect()->minimize), 0);
        }
        if (hovered_button == BUTTON_MAXIMIZE
        || new_hovered_button == BUTTON_MAXIMIZE) {
            InvalidateRect(handle, &(get_title_bar_rect()->maximize), 0);
        }
        if (hovered_button == BUTTON_CLOSE
        || new_hovered_button == BUTTON_CLOSE) {
            InvalidateRect(handle, &(get_title_bar_rect()->close), 0);
        }
    }
    SetWindowLongPtrW(handle, GWLP_USERDATA, (LONG_PTR)new_hovered_button);
}

void handle_system_menu(HWND handle, POINT pos) {
    const int maximize = IsZoomed(handle);
    MENUITEMINFO menu_item_info = { 0 };
    menu_item_info.cbSize = sizeof(menu_item_info);
    menu_item_info.fMask = MIIM_STATE;
    HMENU sys_menu = GetSystemMenu(handle, 0);

    menu_item_info.fState = maximize ? MF_ENABLED : MF_DISABLED;
    SetMenuItemInfo(sys_menu, SC_RESTORE, 0, &menu_item_info);

    menu_item_info.fState = !maximize ? MF_ENABLED : MF_DISABLED;
    SetMenuItemInfo(sys_menu, SC_MOVE, 0, &menu_item_info);
    SetMenuItemInfo(sys_menu, SC_SIZE, 0, &menu_item_info);
    SetMenuItemInfo(sys_menu, SC_MAXIMIZE, 0, &menu_item_info);

    int result = TrackPopupMenu(sys_menu, TPM_RETURNCMD, pos.x, pos.y, 0, handle, NULL);
    if (result != 0) {
        PostMessage(handle, WM_SYSCOMMAND, result, 0);
    }
}