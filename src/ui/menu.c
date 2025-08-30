#include <windows.h>

#include "menu.h"
#include "title_bar.h"
#include "color.h"

static void append_menu(HMENU menu, UINT_PTR id, LPCWSTR name, LPCWSTR shortcut);
static void append_submenu(HMENU parent, HMENU menu, LPCWSTR name);
static UINT_PTR id_count;

void init_menu(struct menu_item* menu) {
    id_count = 2;
    HMENU file_menu = CreatePopupMenu();
    append_menu(file_menu, 0, NULL, NULL);
    append_menu(file_menu, id_count, L"New Text File", L"Ctrl+N");
    append_menu(file_menu, id_count, L"New File..", L"Ctrl+Alt+Windows+N");
    append_menu(file_menu, 1, NULL, NULL);
    append_menu(file_menu, id_count, L"Open File", NULL);
    append_menu(file_menu, id_count, L"Open Folder", L"Ctrl+K Ctrl+O");
    append_menu(file_menu, id_count, L"Open Workspace from File", NULL);
    append_menu(file_menu, 1, NULL, NULL);
    append_menu(file_menu, id_count, L"Save", L"Ctrl+S");
    append_menu(file_menu, id_count, L"Save As..", L"Ctrl+Shift+S");
    append_menu(file_menu, id_count, L"Save All", L"Ctrl+K S");
    append_menu(file_menu, 1, NULL, NULL);
    append_menu(file_menu, id_count, L"Close Editor", L"Ctrl+F4");
    append_menu(file_menu, id_count, L"Close Window", L"Ctrl+K F");
    append_menu(file_menu, id_count, L"Close File", L"Alt+F4");
    append_menu(file_menu, 1, NULL, NULL);
    append_menu(file_menu, id_count, L"Exit", NULL);
    append_menu(file_menu, 0, NULL, NULL);

    HMENU edit_menu = CreatePopupMenu();
    append_menu(edit_menu, 0, NULL, NULL);
    append_menu(edit_menu, id_count, L"Undo", L"Ctrl+Shift+Z");
    append_menu(edit_menu, id_count, L"Redo", L"Ctrl+Z");
    append_menu(edit_menu, 1, NULL, NULL);
    append_menu(edit_menu, id_count, L"Cut", L"Ctrl+X");
    append_menu(edit_menu, id_count, L"Copy", L"Ctrl+C");
    append_menu(edit_menu, id_count, L"Paste", L"Ctrl+V");
    append_menu(edit_menu, 1, NULL, NULL);
    append_menu(edit_menu, id_count, L"Find", L"Ctrl+F");
    append_menu(edit_menu, id_count, L"Replace", L"Ctrl+H");
    append_menu(edit_menu, 0, NULL, NULL);

    HMENU view_menu = CreatePopupMenu();
    append_menu(view_menu, 0, NULL, NULL);
    append_menu(view_menu, id_count, L"Command Palette", L"Ctrl+P");
    append_menu(view_menu, 1, NULL, NULL);
    append_menu(view_menu, id_count, L"Explorer", L"Ctrl+Shift+E");
    append_menu(view_menu, id_count, L"Search", L"Ctrl+Shift+F");
    append_menu(view_menu, 0, NULL, NULL);

    HMENU go_menu = CreatePopupMenu();
    append_menu(go_menu, 0, NULL, NULL);
    append_menu(go_menu, id_count, L"Back", L"Alt+LeftArrow");
    append_menu(go_menu, id_count, L"Forward", L"Alt+RightArrow");
    append_menu(go_menu, 0, NULL, NULL);

    HMENU run_menu = CreatePopupMenu();
    append_menu(run_menu, 0, NULL, NULL);
    append_menu(run_menu, id_count, L"Start Debugging", L"F5");
    append_menu(run_menu, id_count, L"Run Without Debugging", L"Ctrl+F5");
    append_menu(run_menu, 0, NULL, NULL);

    HMENU terminal_menu = CreatePopupMenu();
    append_menu(terminal_menu, 0, NULL, NULL);
    append_menu(terminal_menu, id_count, L"New Terminal", L"Ctrl+Shift+`");
    append_menu(terminal_menu, 0, NULL, NULL);

    HMENU help_menu = CreatePopupMenu();
    append_menu(help_menu, 0, NULL, NULL);
    append_menu(help_menu, id_count, L"Welcome", NULL);
    append_menu(help_menu, id_count, L"Show All Commands", NULL);
    append_menu(help_menu, id_count, L"Documentation", NULL);
    append_menu(help_menu, 1, NULL, NULL);
    append_menu(help_menu, id_count, L"About", NULL);
    append_menu(help_menu, 0, NULL, NULL);

    menu[0] = (struct menu_item){ L"File", file_menu, 0 };
    menu[1] = (struct menu_item){ L"Edit", edit_menu, 0 };
    menu[2] = (struct menu_item){ L"View", view_menu, 0 };
    menu[3] = (struct menu_item){ L"Go", go_menu, 0 };
    menu[4] = (struct menu_item){ L"Run", run_menu, 0 };
    menu[5] = (struct menu_item){ L"Terminal", terminal_menu, 0 };
    menu[6] = (struct menu_item){ L"Help", help_menu, 0 };

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        MENUINFO menu_info = { 0 };
        menu_info.cbSize = sizeof(MENUINFO);
        menu_info.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
        menu_info.hbrBack = CreateSolidBrush(get_colors().bg_menu);
        SetMenuInfo(menu[i].menu, &menu_info);
    }
}

static void append_menu(HMENU menu, UINT_PTR id, LPCWSTR name, LPCWSTR shortcut) {
    if (name == NULL) {
        AppendMenuW(menu, MF_SEPARATOR | MF_OWNERDRAW, id, name);
        return;
    }
    AppendMenuW(menu, MF_STRING | MF_OWNERDRAW, id, name);
    id_count += 1;
    struct menu_text* menu_text = malloc(sizeof(struct menu_text));
    if (menu_text) {
        menu_text->name = name;
        menu_text->shortcut = shortcut;
        MENUITEMINFOW menu_item_info = { 0 };
        menu_item_info.cbSize = sizeof(MENUITEMINFOW);
        menu_item_info.fMask = MIIM_DATA;
        menu_item_info.dwItemData = (ULONG_PTR)menu_text;
        SetMenuItemInfoW(menu, (UINT)id, 0, &menu_item_info);
    }
}

static void append_submenu(HMENU parent, HMENU menu, LPCWSTR name) {
    AppendMenuW(parent, MF_POPUP | MF_STRING | MF_OWNERDRAW, (UINT_PTR)menu, name);
    struct menu_text* menu_text = malloc(sizeof(struct menu_text));
    if (menu_text) {
        menu_text->name = name;
        menu_text->shortcut = L">";
        MENUITEMINFOW menu_item_info = { 0 };
        menu_item_info.cbSize = sizeof(MENUITEMINFOW);
        menu_item_info.fMask = MIIM_DATA;
        menu_item_info.dwItemData = (ULONG_PTR)menu_text;
        SetMenuItemInfoW(menu, (UINT)((UINT_PTR)menu), 0, &menu_item_info);
    }
}