#include <windows.h>

HFONT create_font(int font_size, WCHAR* font_name) {
    LOGFONTW lf = { 0 };
    lf.lfHeight = font_size;
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = DEFAULT_CHARSET;
    wcscpy_s(lf.lfFaceName, 32, font_name);
    return CreateFontIndirectW(&lf);
}