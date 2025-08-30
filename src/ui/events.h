#ifndef WINDOWS_EVENTS
#define WINDOWS_EVENTS

#include <windows.h>

#define DPI_SCALE(value, dpi) (int)((float)value * dpi / 96);

LRESULT CALLBACK CBTProc(int code, WPARAM w_param, LPARAM l_param);
LRESULT CALLBACK win_proc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param);

#endif
