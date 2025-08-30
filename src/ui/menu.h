#ifndef WINDOWS_MENU
#define WINDOWS_MENU

#include "title_bar.h"

#define ID_FILE_NEW_FILE 1
#define ID_FILE_OPEN_FILE 2
#define ID_EDIT_UNDO 3
#define ID_EDIT_REDO 4

struct menu_text {
    LPCWSTR name;
    LPCWSTR shortcut;
};

void init_menu(struct menu_item* menu);

#endif