// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Mohammad Taghi Alavi

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

#include <windows.h>

#include "resource.h"

extern "C" void *__cdecl memcpy(void *destination, const void *source, unsigned int count)
{
    BYTE *to = (BYTE *)destination;
    const BYTE *from = (const BYTE *)source;
    unsigned int i;

    for (i = 0; i < count; ++i)
        to[i] = from[i];
    return destination;
}

extern "C" HINSTANCE WINAPI ShellExecuteW(HWND, LPCWSTR, LPCWSTR,
                                           LPCWSTR, LPCWSTR, INT);

static const WCHAR kClockClassName[] = L"BABAClockWindow";
static const WCHAR kSettingsClassName[] = L"BABAClockSettingsWindow";
static const WCHAR kAboutClassName[] = L"BABAClockAboutWindow";
static const char kRegistryPath[] = "Software\\BABA Clock";
static const WCHAR kStartupKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const WCHAR kStartupValue[] = L"BABA Clock";
static const WCHAR kRepositoryUrl[] = L"https://github.com/AAA-It-uae/DADClock";
static const WCHAR kSourceText[] = L"Source: https://github.com/AAA-It-uae/DADClock";

static const UINT kTimerId = 1;
static const UINT kSettingsId = 100;
static const UINT kFormat24Id = 101;
static const UINT kFormat12Id = 102;
static const UINT kAboutId = 103;
static const UINT kExitId = 104;

static const UINT kShowSecondsControl = 201;
static const UINT kBlinkColonControl = 202;
static const UINT kStartupControl = 203;
static const UINT kOkControl = 204;
static const UINT kCancelControl = 205;
static const UINT kFont7SegmentControl = 206;
static const UINT kFontClassicControl = 207;
static const UINT kSourceLinkControl = 208;

static const int kFormat24 = 0;
static const int kFormat12 = 1;
static const int kFont7Segment = 0;
static const int kFontClassic = 1;
static const int kBaseHeight = 58;
static const int k24MinutesWidth = 156;
static const int k24SecondsWidth = 228;
static const int k12MinutesWidth = 224;
static const int k12SecondsWidth = 301;
static const int kSettingsWidth = 480;
static const int kSettingsHeight = 430;
static const int kAboutWidth = 520;
static const int kAboutHeight = 350;

static const WCHAR kFirstRunText[] = L"Made with love for my beloved father \u2665";
static const WCHAR kBrandingText[] =
    L"BABA Clock\r\n"
    L"Built by Mohammad Taghi Alavi\r\n"
    L"Idea by Abbas Alavi\r\n"
    L"Made with love \u2665\r\n\r\n"
    L"Compatibility: Windows XP, Vista, 7, 8, 8.1, 10, 11\r\n"
    L"Technology: C++ / Win32 API / GDI\r\n"
    L"Architecture: 32-bit x86\r\n"
    L"License: Apache License 2.0";

static const unsigned char kSegments[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static HINSTANCE g_instance;
static HICON g_icon;
static HFONT g_linkFont;
static HWND g_settingsWindow;
static HWND g_settingsOwner;
static HWND g_aboutWindow;
static HWND g_aboutOwner;
static BOOL g_showSeconds;
static BOOL g_blinkColon;
static BOOL g_runAtStartup;
static int g_timeFormat;
static int g_fontStyle;
static BOOL g_colonVisible = TRUE;
static char g_digits[9] = "00:00:00";
static char g_meridiem[3] = "AM";

static int LayoutWidth()
{
    if (g_timeFormat == kFormat12)
        return g_showSeconds ? k12SecondsWidth : k12MinutesWidth;
    return g_showSeconds ? k24SecondsWidth : k24MinutesWidth;
}

static int Scaled(int value, int scale)
{
    return (value * scale) / 1000;
}

static BOOL QueryDword(HKEY key, const char *name, DWORD *value)
{
    DWORD type;
    DWORD size = sizeof(*value);

    return RegQueryValueExA(key, name, NULL, &type, (BYTE *)value, &size) == ERROR_SUCCESS &&
           type == REG_DWORD && size == sizeof(*value);
}

static void WriteDword(HKEY key, const char *name, DWORD value)
{
    RegSetValueExA(key, name, 0, REG_DWORD, (const BYTE *)&value, sizeof(value));
}

static void LoadState(int *x, int *y, int *width, int *height)
{
    HKEY key;
    DWORD value;
    BOOL haveWidth = FALSE;

    *height = kBaseHeight;
    g_showSeconds = FALSE;
    g_blinkColon = TRUE;
    g_runAtStartup = FALSE;
    g_timeFormat = kFormat24;
    g_fontStyle = kFont7Segment;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        *width = LayoutWidth();
        return;
    }

    if (QueryDword(key, "X", &value))
        *x = (int)(LONG)value;
    if (QueryDword(key, "Y", &value))
        *y = (int)(LONG)value;
    if (QueryDword(key, "Width", &value) && value >= 1 && value <= 10000) {
        *width = (int)value;
        haveWidth = TRUE;
    }
    if (QueryDword(key, "Height", &value) && value >= 1 && value <= 10000)
        *height = (int)value;
    if (QueryDword(key, "ShowSeconds", &value))
        g_showSeconds = value != 0;
    if (QueryDword(key, "BlinkColon", &value))
        g_blinkColon = value != 0;
    if (QueryDword(key, "RunAtStartup", &value))
        g_runAtStartup = value != 0;
    if (QueryDword(key, "TimeFormat", &value) && value <= kFormat12)
        g_timeFormat = (int)value;
    if (QueryDword(key, "FontStyle", &value) && value <= kFontClassic)
        g_fontStyle = (int)value;

    RegCloseKey(key);
    if (!haveWidth)
        *width = LayoutWidth();
}

static void SaveWindowState(HWND window)
{
    RECT windowRect;
    HKEY key;
    DWORD unused;
    LONG x;
    LONG y;
    LONG width;
    LONG height;

    if (!GetWindowRect(window, &windowRect))
        return;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, kRegistryPath, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                        &key, &unused) != ERROR_SUCCESS)
        return;

    x = windowRect.left;
    y = windowRect.top;
    width = windowRect.right - windowRect.left;
    height = windowRect.bottom - windowRect.top;
    WriteDword(key, "X", (DWORD)x);
    WriteDword(key, "Y", (DWORD)y);
    WriteDword(key, "Width", (DWORD)width);
    WriteDword(key, "Height", (DWORD)height);
    WriteDword(key, "ShowSeconds", g_showSeconds ? 1 : 0);
    WriteDword(key, "BlinkColon", g_blinkColon ? 1 : 0);
    WriteDword(key, "RunAtStartup", g_runAtStartup ? 1 : 0);
    WriteDword(key, "TimeFormat", g_timeFormat == kFormat12 ? 1 : 0);
    WriteDword(key, "FontStyle", g_fontStyle == kFontClassic ? 1 : 0);
    RegCloseKey(key);
}

static BOOL IsFirstRun()
{
    HKEY key;
    DWORD value;
    BOOL firstRun = TRUE;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
        if (QueryDword(key, "FirstRunShown", &value))
            firstRun = value == 0;
        RegCloseKey(key);
    }
    return firstRun;
}

static void MarkFirstRunShown()
{
    HKEY key;
    DWORD unused;

    if (RegCreateKeyExA(HKEY_CURRENT_USER, kRegistryPath, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                        &key, &unused) == ERROR_SUCCESS) {
        WriteDword(key, "FirstRunShown", 1);
        RegCloseKey(key);
    }
}

static void ConfigureStartup(BOOL enabled)
{
    HKEY key;

    if (!enabled) {
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kStartupKey, 0, KEY_SET_VALUE, &key) == ERROR_SUCCESS) {
            RegDeleteValueW(key, kStartupValue);
            RegCloseKey(key);
        }
        return;
    }

    if (RegCreateKeyExW(HKEY_CURRENT_USER, kStartupKey, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                        &key, NULL) == ERROR_SUCCESS) {
        WCHAR path[MAX_PATH];
        WCHAR command[MAX_PATH + 4];
        DWORD length;
        DWORD i;

        length = GetModuleFileNameW(NULL, path, MAX_PATH);
        if (length != 0 && length < MAX_PATH - 1) {
            command[0] = L'"';
            for (i = 0; i < length; ++i)
                command[i + 1] = path[i];
            command[length + 1] = L'"';
            command[length + 2] = L'\0';
            RegSetValueExW(key, kStartupValue, 0, REG_SZ,
                           (const BYTE *)command, (length + 3) * sizeof(WCHAR));
        }
        RegCloseKey(key);
    }
}

static void UpdateDigits(HWND window)
{
    SYSTEMTIME now;
    char next[9];
    char nextMeridiem[3];
    WORD hour;
    BOOL nextColonVisible;
    BOOL changed = FALSE;
    int count;
    int i;

    GetLocalTime(&now);
    hour = now.wHour;
    nextMeridiem[0] = 'A';
    nextMeridiem[1] = 'M';
    if (g_timeFormat == kFormat12) {
        if (hour >= 12) {
            nextMeridiem[0] = 'P';
            hour = (WORD)(hour - 12);
        }
        if (hour == 0)
            hour = 12;
    }

    next[0] = (char)('0' + hour / 10);
    next[1] = (char)('0' + hour % 10);
    next[2] = ':';
    next[3] = (char)('0' + now.wMinute / 10);
    next[4] = (char)('0' + now.wMinute % 10);
    count = 5;
    if (g_showSeconds) {
        next[5] = ':';
        next[6] = (char)('0' + now.wSecond / 10);
        next[7] = (char)('0' + now.wSecond % 10);
        count = 8;
    }

    nextColonVisible = g_showSeconds ? TRUE : (!g_blinkColon || (now.wSecond % 2 == 0));
    if (g_colonVisible != nextColonVisible || g_meridiem[0] != nextMeridiem[0] ||
        g_meridiem[1] != nextMeridiem[1])
        changed = TRUE;
    for (i = 0; i < count; ++i) {
        if (g_digits[i] != next[i])
            changed = TRUE;
    }

    if (changed) {
        for (i = 0; i < count; ++i)
            g_digits[i] = next[i];
        g_meridiem[0] = nextMeridiem[0];
        g_meridiem[1] = nextMeridiem[1];
        g_colonVisible = nextColonVisible;
        InvalidateRect(window, NULL, FALSE);
    }
}

static void DrawScaledRect(HDC dc, int left, int top, int right, int bottom,
                           int offsetX, int offsetY, int scale, HBRUSH brush)
{
    RECT r;
    r.left = offsetX + Scaled(left, scale);
    r.top = offsetY + Scaled(top, scale);
    r.right = offsetX + Scaled(right, scale);
    r.bottom = offsetY + Scaled(bottom, scale);
    FillRect(dc, &r, brush);
}

static void DrawScaledPolygon(HDC dc, const int *coordinates, int count,
                              int offsetX, int offsetY, int scale)
{
    POINT points[6];
    int i;

    for (i = 0; i < count; ++i) {
        points[i].x = offsetX + Scaled(coordinates[i * 2], scale);
        points[i].y = offsetY + Scaled(coordinates[i * 2 + 1], scale);
    }
    Polygon(dc, points, count);
}

static void DrawScaledEllipse(HDC dc, int left, int top, int right, int bottom,
                              int offsetX, int offsetY, int scale)
{
    Ellipse(dc, offsetX + Scaled(left, scale), offsetY + Scaled(top, scale),
            offsetX + Scaled(right, scale), offsetY + Scaled(bottom, scale));
}

static void DrawClassicSegment(HDC dc, int x, int y, int segment,
                               int offsetX, int offsetY, int scale)
{
    int points[12];
    int top;
    int bottom;

    if (segment == 0 || segment == 3 || segment == 6) {
        top = y + (segment == 0 ? 0 : (segment == 6 ? 19 : 38));
        points[0] = x + 7;  points[1] = top;
        points[2] = x + 19; points[3] = top;
        points[4] = x + 22; points[5] = top + 3;
        points[6] = x + 19; points[7] = top + 6;
        points[8] = x + 7;  points[9] = top + 6;
        points[10] = x + 4; points[11] = top + 3;
    } else {
        top = y + ((segment == 1 || segment == 5) ? 4 : 25);
        bottom = top + 15;
        if (segment == 1 || segment == 2)
            x += 20;
        points[0] = x + 3; points[1] = top;
        points[2] = x + 6; points[3] = top + 3;
        points[4] = x + 6; points[5] = bottom - 3;
        points[6] = x + 3; points[7] = bottom;
        points[8] = x;     points[9] = bottom - 3;
        points[10] = x;    points[11] = top + 3;
    }
    DrawScaledPolygon(dc, points, 6, offsetX, offsetY, scale);
}

static void DrawSegment(HDC dc, int x, int y, int segment,
                        int offsetX, int offsetY, int scale, HBRUSH brush)
{
    int left;
    int top;
    int right;
    int bottom;

    if (g_fontStyle == kFontClassic) {
        DrawClassicSegment(dc, x, y, segment, offsetX, offsetY, scale);
        return;
    }

    switch (segment) {
    case 0: left = x + 5;  top = y;      right = x + 21; bottom = y + 6;  break;
    case 1: left = x + 20; top = y + 4;  right = x + 26; bottom = y + 19; break;
    case 2: left = x + 20; top = y + 25; right = x + 26; bottom = y + 40; break;
    case 3: left = x + 5;  top = y + 38; right = x + 21; bottom = y + 44; break;
    case 4: left = x;      top = y + 25; right = x + 6;  bottom = y + 40; break;
    case 5: left = x;      top = y + 4;  right = x + 6;  bottom = y + 19; break;
    default:left = x + 5;  top = y + 19; right = x + 21; bottom = y + 25; break;
    }
    DrawScaledRect(dc, left, top, right, bottom, offsetX, offsetY, scale, brush);
}

static void DrawDigit(HDC dc, int x, int y, int digit,
                      int offsetX, int offsetY, int scale, HBRUSH brush)
{
    unsigned char mask = kSegments[digit];
    int segment;

    for (segment = 0; segment < 7; ++segment) {
        if (mask & (1 << segment))
            DrawSegment(dc, x, y, segment, offsetX, offsetY, scale, brush);
    }
}

static void DrawClassicVerticalBar(HDC dc, int x, int top, int bottom,
                                    int offsetX, int offsetY, int scale)
{
    int points[12];

    points[0] = x + 3; points[1] = top;
    points[2] = x + 6; points[3] = top + 3;
    points[4] = x + 6; points[5] = bottom - 3;
    points[6] = x + 3; points[7] = bottom;
    points[8] = x;     points[9] = bottom - 3;
    points[10] = x;    points[11] = top + 3;
    DrawScaledPolygon(dc, points, 6, offsetX, offsetY, scale);
}

static void DrawLetter(HDC dc, int x, int y, char letter,
                       int offsetX, int offsetY, int scale, HBRUSH brush)
{
    unsigned char mask;
    int segment;

    if (letter == 'A')
        mask = 0x77;
    else if (letter == 'P')
        mask = 0x73;
    else
        mask = 0;

    for (segment = 0; segment < 7; ++segment) {
        if (mask & (1 << segment))
            DrawSegment(dc, x, y, segment, offsetX, offsetY, scale, brush);
    }

    if (letter == 'M') {
        if (g_fontStyle == kFontClassic) {
            DrawClassicVerticalBar(dc, x, y + 4, y + 40,
                                   offsetX, offsetY, scale);
            DrawClassicVerticalBar(dc, x + 20, y + 4, y + 40,
                                   offsetX, offsetY, scale);
            DrawClassicVerticalBar(dc, x + 10, y + 4, y + 20,
                                   offsetX, offsetY, scale);
        } else {
            DrawScaledRect(dc, x, y + 4, x + 6, y + 40,
                           offsetX, offsetY, scale, brush);
            DrawScaledRect(dc, x + 20, y + 4, x + 26, y + 40,
                           offsetX, offsetY, scale, brush);
            DrawScaledRect(dc, x + 10, y + 4, x + 16, y + 20,
                           offsetX, offsetY, scale, brush);
        }
    }
}

static void DrawColon(HDC dc, int x, int offsetX, int offsetY, int scale, HBRUSH brush)
{
    if (g_fontStyle == kFontClassic) {
        DrawScaledEllipse(dc, x, 17, x + 6, 23, offsetX, offsetY, scale);
        DrawScaledEllipse(dc, x, 35, x + 6, 41, offsetX, offsetY, scale);
    } else {
        DrawScaledRect(dc, x, 17, x + 6, 23, offsetX, offsetY, scale, brush);
        DrawScaledRect(dc, x, 35, x + 6, 41, offsetX, offsetY, scale, brush);
    }
}

static void PaintClock(HWND window, HDC dc)
{
    RECT client;
    HBRUSH green;
    HBRUSH black;
    HBRUSH oldBrush;
    HPEN greenPen;
    HPEN oldPen;
    int clientWidth;
    int clientHeight;
    int scaleX;
    int scaleY;
    int scale;
    int drawWidth;
    int drawHeight;
    int offsetX;
    int offsetY;
    int meridiemX;

    GetClientRect(window, &client);
    black = (HBRUSH)GetStockObject(BLACK_BRUSH);
    FillRect(dc, &client, black);

    clientWidth = client.right - client.left;
    clientHeight = client.bottom - client.top;
    scaleX = (clientWidth * 1000) / LayoutWidth();
    scaleY = (clientHeight * 1000) / kBaseHeight;
    scale = scaleX < scaleY ? scaleX : scaleY;
    if (scale < 1)
        scale = 1;

    drawWidth = Scaled(LayoutWidth(), scale);
    drawHeight = Scaled(kBaseHeight, scale);
    offsetX = (clientWidth - drawWidth) / 2;
    offsetY = (clientHeight - drawHeight) / 2;

    green = CreateSolidBrush(RGB(0, 255, 0));
    greenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    oldBrush = (HBRUSH)SelectObject(dc, green);
    oldPen = (HPEN)SelectObject(dc, greenPen);
    DrawDigit(dc, 8, 7, g_digits[0] - '0', offsetX, offsetY, scale, green);
    DrawDigit(dc, 39, 7, g_digits[1] - '0', offsetX, offsetY, scale, green);
    if (g_colonVisible)
        DrawColon(dc, 70, offsetX, offsetY, scale, green);
    DrawDigit(dc, 85, 7, g_digits[3] - '0', offsetX, offsetY, scale, green);
    DrawDigit(dc, 116, 7, g_digits[4] - '0', offsetX, offsetY, scale, green);

    if (g_showSeconds) {
        if (g_colonVisible)
            DrawColon(dc, 147, offsetX, offsetY, scale, green);
        DrawDigit(dc, 162, 7, g_digits[6] - '0', offsetX, offsetY, scale, green);
        DrawDigit(dc, 193, 7, g_digits[7] - '0', offsetX, offsetY, scale, green);
    }

    if (g_timeFormat == kFormat12) {
        meridiemX = g_showSeconds ? 236 : 159;
        DrawLetter(dc, meridiemX, 7, g_meridiem[0], offsetX, offsetY, scale, green);
        DrawLetter(dc, meridiemX + 31, 7, g_meridiem[1], offsetX, offsetY, scale, green);
    }
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(greenPen);
    DeleteObject(green);
}

static void ApplyLayoutSize(HWND window, int oldBaseWidth, int oldWidth, int oldHeight)
{
    int scaleX;
    int scaleY;
    int scale;
    int width;
    int height;
    RECT windowRect;

    if (!GetWindowRect(window, &windowRect))
        return;
    scaleX = (oldWidth * 1000) / oldBaseWidth;
    scaleY = (oldHeight * 1000) / kBaseHeight;
    scale = scaleX < scaleY ? scaleX : scaleY;
    if (scale < 500)
        scale = 500;

    width = Scaled(LayoutWidth(), scale);
    height = Scaled(kBaseHeight, scale);
    SetWindowPos(window, HWND_TOPMOST, windowRect.left, windowRect.top,
                 width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(window, NULL, FALSE);
}

static void KeepAspectRatio(RECT *windowRect, WPARAM edge)
{
    int baseWidth = LayoutWidth();
    int width = windowRect->right - windowRect->left;
    int height = windowRect->bottom - windowRect->top;
    int newWidth;
    int newHeight;
    int minWidth = (baseWidth + 1) / 2;
    int minHeight = (kBaseHeight + 1) / 2;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    if (width * kBaseHeight >= height * baseWidth) {
        newHeight = height;
        newWidth = (height * baseWidth + kBaseHeight / 2) / kBaseHeight;
    } else {
        newWidth = width;
        newHeight = (width * kBaseHeight + baseWidth / 2) / baseWidth;
    }
    if (newWidth < minWidth) {
        newWidth = minWidth;
        newHeight = (newWidth * kBaseHeight + baseWidth / 2) / baseWidth;
    }
    if (newHeight < minHeight) {
        newHeight = minHeight;
        newWidth = (newHeight * baseWidth + kBaseHeight / 2) / kBaseHeight;
    }

    switch (edge) {
    case WMSZ_LEFT:
        windowRect->left = windowRect->right - newWidth;
        windowRect->bottom = windowRect->top + newHeight;
        break;
    case WMSZ_RIGHT:
        windowRect->right = windowRect->left + newWidth;
        windowRect->bottom = windowRect->top + newHeight;
        break;
    case WMSZ_TOP:
        windowRect->top = windowRect->bottom - newHeight;
        windowRect->right = windowRect->left + newWidth;
        break;
    case WMSZ_BOTTOM:
        windowRect->bottom = windowRect->top + newHeight;
        windowRect->right = windowRect->left + newWidth;
        break;
    case WMSZ_TOPLEFT:
        windowRect->left = windowRect->right - newWidth;
        windowRect->top = windowRect->bottom - newHeight;
        break;
    case WMSZ_TOPRIGHT:
        windowRect->right = windowRect->left + newWidth;
        windowRect->top = windowRect->bottom - newHeight;
        break;
    case WMSZ_BOTTOMLEFT:
        windowRect->left = windowRect->right - newWidth;
        windowRect->bottom = windowRect->top + newHeight;
        break;
    default:
        windowRect->right = windowRect->left + newWidth;
        windowRect->bottom = windowRect->top + newHeight;
        break;
    }
}

static void OpenRepositoryLink(HWND owner)
{
    ShellExecuteW(owner, L"open", kRepositoryUrl, NULL, NULL, SW_SHOWNORMAL);
}

static void EnsureLinkFont()
{
    if (!g_linkFont)
        g_linkFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Arial");
}

static HWND CreateSourceLink(HWND parent, int x, int y, int width, int height)
{
    HWND link;

    link = CreateWindowExW(0, L"STATIC", kSourceText,
                           WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOTIFY,
                           x, y, width, height, parent,
                           (HMENU)kSourceLinkControl, g_instance, NULL);
    EnsureLinkFont();
    SendMessageW(link, WM_SETFONT, (WPARAM)g_linkFont, TRUE);
    return link;
}

static LRESULT LinkColor(HWND window, WPARAM wParam, LPARAM lParam)
{
    if ((HWND)lParam == GetDlgItem(window, kSourceLinkControl)) {
        SetTextColor((HDC)wParam, RGB(0, 0, 180));
        SetBkMode((HDC)wParam, TRANSPARENT);
    }
    return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
}

static void ShowClockMenu(HWND window, int x, int y)
{
    HMENU menu;
    HMENU formatMenu;

    menu = CreatePopupMenu();
    formatMenu = CreatePopupMenu();
    if (!menu || !formatMenu) {
        if (menu) DestroyMenu(menu);
        if (formatMenu) DestroyMenu(formatMenu);
        return;
    }

    AppendMenuW(menu, MF_STRING, kSettingsId, L"Settings");
    AppendMenuW(formatMenu, MF_STRING | (g_timeFormat == kFormat24 ? MF_CHECKED : 0),
                kFormat24Id, L"24-Hour");
    AppendMenuW(formatMenu, MF_STRING | (g_timeFormat == kFormat12 ? MF_CHECKED : 0),
                kFormat12Id, L"12-Hour");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)formatMenu, L"Time Format");
    AppendMenuW(menu, MF_STRING, kAboutId, L"About");
    AppendMenuW(menu, MF_STRING, kExitId, L"Exit");
    SetForegroundWindow(window);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, x, y, 0, window, NULL);
    PostMessageW(window, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

static void SelectTimeFormat(HWND window, int format)
{
    int oldBaseWidth;
    int oldWidth;
    int oldHeight;
    RECT oldRect;

    if (format == g_timeFormat || !GetWindowRect(window, &oldRect))
        return;
    oldBaseWidth = LayoutWidth();
    oldWidth = oldRect.right - oldRect.left;
    oldHeight = oldRect.bottom - oldRect.top;
    g_timeFormat = format;
    ApplyLayoutSize(window, oldBaseWidth, oldWidth, oldHeight);
    UpdateDigits(window);
    SaveWindowState(window);
}

static void ApplySettings(HWND settingsWindow)
{
    int oldBaseWidth = LayoutWidth();
    int oldWidth;
    int oldHeight;
    RECT oldRect;

    if (!GetWindowRect(g_settingsOwner, &oldRect))
        return;
    oldWidth = oldRect.right - oldRect.left;
    oldHeight = oldRect.bottom - oldRect.top;
    g_showSeconds = SendMessageW(GetDlgItem(settingsWindow, kShowSecondsControl),
                                 BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_blinkColon = SendMessageW(GetDlgItem(settingsWindow, kBlinkColonControl),
                                BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_runAtStartup = SendMessageW(GetDlgItem(settingsWindow, kStartupControl),
                                  BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_fontStyle = SendMessageW(GetDlgItem(settingsWindow, kFontClassicControl),
                               BM_GETCHECK, 0, 0) == BST_CHECKED ? kFontClassic : kFont7Segment;

    ConfigureStartup(g_runAtStartup);
    ApplyLayoutSize(g_settingsOwner, oldBaseWidth, oldWidth, oldHeight);
    UpdateDigits(g_settingsOwner);
    SaveWindowState(g_settingsOwner);
}

static LRESULT CALLBACK SettingsWindowProc(HWND window, UINT message,
                                           WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        CreateWindowExW(0, L"BUTTON", L"Show Seconds",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 14, 420, 24, window, (HMENU)kShowSecondsControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Blink Separator/Colon",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 43, 420, 24, window, (HMENU)kBlinkColonControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Run at Windows Startup",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 72, 420, 24, window, (HMENU)kStartupControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"STATIC", L"Clock Font",
                        WS_CHILD | WS_VISIBLE,
                        16, 105, 420, 20, window, NULL, g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"7-Segment",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | WS_GROUP,
                        16, 126, 180, 24, window, (HMENU)kFont7SegmentControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Digital Classic",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON,
                        205, 126, 220, 24, window, (HMENU)kFontClassicControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"OK",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                        170, 171, 64, 25, window, (HMENU)kOkControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Cancel",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                        242, 171, 64, 25, window, (HMENU)kCancelControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"STATIC", kBrandingText,
                        WS_CHILD | WS_VISIBLE | SS_CENTER,
                        10, 215, 460, 145, window, NULL, g_instance, NULL);
        CreateSourceLink(window, 10, 365, 460, 22);
        SendMessageW(GetDlgItem(window, kShowSecondsControl), BM_SETCHECK,
                     g_showSeconds ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kBlinkColonControl), BM_SETCHECK,
                     g_blinkColon ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kStartupControl), BM_SETCHECK,
                     g_runAtStartup ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kFont7SegmentControl), BM_SETCHECK,
                     g_fontStyle == kFont7Segment ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kFontClassicControl), BM_SETCHECK,
                     g_fontStyle == kFontClassic ? BST_CHECKED : BST_UNCHECKED, 0);
        SetFocus(GetDlgItem(window, kOkControl));
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == kOkControl) {
            ApplySettings(window);
            DestroyWindow(window);
            return 0;
        }
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == kCancelControl) {
            DestroyWindow(window);
            return 0;
        }
        if (HIWORD(wParam) == STN_CLICKED && LOWORD(wParam) == kSourceLinkControl) {
            OpenRepositoryLink(window);
            return 0;
        }
        return 0;

    case WM_CTLCOLORSTATIC:
        return LinkColor(window, wParam, lParam);

    case WM_SETCURSOR:
        if ((HWND)wParam == GetDlgItem(window, kSourceLinkControl)) {
            SetCursor(LoadCursorW(NULL, MAKEINTRESOURCEW(32649)));
            return TRUE;
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(window);
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(window);
        return 0;

    case WM_NCDESTROY:
        g_settingsWindow = NULL;
        if (g_settingsOwner) {
            EnableWindow(g_settingsOwner, TRUE);
            SetForegroundWindow(g_settingsOwner);
            g_settingsOwner = NULL;
        }
        break;

    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

static void ShowSettings(HWND owner)
{
    RECT ownerRect;
    int x;
    int y;

    if (g_settingsWindow) {
        SetForegroundWindow(g_settingsWindow);
        return;
    }
    if (!GetWindowRect(owner, &ownerRect))
        return;

    x = ownerRect.left + ((ownerRect.right - ownerRect.left) - kSettingsWidth) / 2;
    y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - kSettingsHeight) / 2;
    g_settingsOwner = owner;
    EnableWindow(owner, FALSE);
    g_settingsWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
                                       kSettingsClassName, L"BABA Clock Settings",
                                       WS_POPUP | WS_CAPTION | WS_SYSMENU,
                                       x, y, kSettingsWidth, kSettingsHeight,
                                       owner, NULL, g_instance, NULL);
    if (!g_settingsWindow) {
        EnableWindow(owner, TRUE);
        g_settingsOwner = NULL;
        return;
    }
    ShowWindow(g_settingsWindow, SW_SHOWNORMAL);
    UpdateWindow(g_settingsWindow);
}

static LRESULT CALLBACK AboutWindowProc(HWND window, UINT message,
                                        WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        CreateWindowExW(0, L"STATIC", kBrandingText,
                        WS_CHILD | WS_VISIBLE | SS_CENTER,
                        14, 20, 492, 185, window, NULL, g_instance, NULL);
        CreateSourceLink(window, 14, 222, 492, 22);
        CreateWindowExW(0, L"BUTTON", L"OK",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                        228, 264, 64, 25, window, (HMENU)kOkControl,
                        g_instance, NULL);
        SetFocus(GetDlgItem(window, kOkControl));
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == kOkControl) {
            DestroyWindow(window);
            return 0;
        }
        if (HIWORD(wParam) == STN_CLICKED && LOWORD(wParam) == kSourceLinkControl) {
            OpenRepositoryLink(window);
            return 0;
        }
        return 0;

    case WM_CTLCOLORSTATIC:
        return LinkColor(window, wParam, lParam);

    case WM_SETCURSOR:
        if ((HWND)wParam == GetDlgItem(window, kSourceLinkControl)) {
            SetCursor(LoadCursorW(NULL, MAKEINTRESOURCEW(32649)));
            return TRUE;
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(window);
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(window);
        return 0;

    case WM_NCDESTROY:
        g_aboutWindow = NULL;
        if (g_aboutOwner) {
            EnableWindow(g_aboutOwner, TRUE);
            SetForegroundWindow(g_aboutOwner);
            g_aboutOwner = NULL;
        }
        break;

    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

static void ShowAbout(HWND owner)
{
    RECT ownerRect;
    int x;
    int y;

    if (g_aboutWindow) {
        SetForegroundWindow(g_aboutWindow);
        return;
    }
    if (!GetWindowRect(owner, &ownerRect))
        return;

    x = ownerRect.left + ((ownerRect.right - ownerRect.left) - kAboutWidth) / 2;
    y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - kAboutHeight) / 2;
    g_aboutOwner = owner;
    EnableWindow(owner, FALSE);
    g_aboutWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
                                    kAboutClassName, L"About BABA Clock",
                                    WS_POPUP | WS_CAPTION | WS_SYSMENU,
                                    x, y, kAboutWidth, kAboutHeight,
                                    owner, NULL, g_instance, NULL);
    if (!g_aboutWindow) {
        EnableWindow(owner, TRUE);
        g_aboutOwner = NULL;
        return;
    }
    ShowWindow(g_aboutWindow, SW_SHOWNORMAL);
    UpdateWindow(g_aboutWindow);
}

static LRESULT CALLBACK ClockWindowProc(HWND window, UINT message,
                                        WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_NCCALCSIZE:
        return 0;

    case WM_NCPAINT:
        return 0;

    case WM_CREATE:
        UpdateDigits(window);
        SetTimer(window, kTimerId, 1000, NULL);
        return 0;

    case WM_TIMER:
        if (wParam == kTimerId)
            UpdateDigits(window);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        PaintClock(window, dc);
        EndPaint(window, &paint);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_NCHITTEST:
    {
        RECT windowRect;
        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        int border = 6;
        BOOL left;
        BOOL right;
        BOOL top;
        BOOL bottom;

        GetWindowRect(window, &windowRect);
        left = x >= windowRect.left && x < windowRect.left + border;
        right = x < windowRect.right && x >= windowRect.right - border;
        top = y >= windowRect.top && y < windowRect.top + border;
        bottom = y < windowRect.bottom && y >= windowRect.bottom - border;
        if (left && top) return HTTOPLEFT;
        if (right && top) return HTTOPRIGHT;
        if (left && bottom) return HTBOTTOMLEFT;
        if (right && bottom) return HTBOTTOMRIGHT;
        if (left) return HTLEFT;
        if (right) return HTRIGHT;
        if (top) return HTTOP;
        if (bottom) return HTBOTTOM;
        return HTCLIENT;
    }

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *info = (MINMAXINFO *)lParam;
        info->ptMinTrackSize.x = (LayoutWidth() + 1) / 2;
        info->ptMinTrackSize.y = (kBaseHeight + 1) / 2;
        return 0;
    }

    case WM_SIZING:
        KeepAspectRatio((RECT *)lParam, wParam);
        return TRUE;

    case WM_LBUTTONDOWN:
        ReleaseCapture();
        SendMessageW(window, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return 0;

    case WM_RBUTTONUP:
    {
        POINT point;
        point.x = (short)LOWORD(lParam);
        point.y = (short)HIWORD(lParam);
        ClientToScreen(window, &point);
        ShowClockMenu(window, point.x, point.y);
        return 0;
    }

    case WM_NCRBUTTONUP:
        ShowClockMenu(window, (short)LOWORD(lParam), (short)HIWORD(lParam));
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kSettingsId:
            ShowSettings(window);
            return 0;
        case kFormat24Id:
            SelectTimeFormat(window, kFormat24);
            return 0;
        case kFormat12Id:
            SelectTimeFormat(window, kFormat12);
            return 0;
        case kAboutId:
            ShowAbout(window);
            return 0;
        case kExitId:
            DestroyWindow(window);
            return 0;
        default:
            break;
        }
        break;

    case WM_EXITSIZEMOVE:
        SaveWindowState(window);
        return 0;

    case WM_CLOSE:
        DestroyWindow(window);
        return 0;

    case WM_DESTROY:
        if (g_settingsWindow)
            DestroyWindow(g_settingsWindow);
        if (g_aboutWindow)
            DestroyWindow(g_aboutWindow);
        if (g_linkFont) {
            DeleteObject(g_linkFont);
            g_linkFont = NULL;
        }
        KillTimer(window, kTimerId);
        SaveWindowState(window);
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

static BOOL RegisterWindowClasses()
{
    WNDCLASSEXW windowClass;

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ClockWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = g_instance;
    windowClass.hIcon = g_icon;
    windowClass.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = kClockClassName;
    windowClass.hIconSm = g_icon;
    if (!RegisterClassExW(&windowClass))
        return FALSE;

    windowClass.lpfnWndProc = SettingsWindowProc;
    windowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    windowClass.lpszClassName = kSettingsClassName;
    if (!RegisterClassExW(&windowClass))
        return FALSE;

    windowClass.lpfnWndProc = AboutWindowProc;
    windowClass.lpszClassName = kAboutClassName;
    return RegisterClassExW(&windowClass) != 0;
}

static int ShowIconMessage(HWND owner, const WCHAR *text, const WCHAR *caption)
{
    MSGBOXPARAMSW parameters;

    parameters.cbSize = sizeof(parameters);
    parameters.hwndOwner = owner;
    parameters.hInstance = g_instance;
    parameters.lpszText = text;
    parameters.lpszCaption = caption;
    parameters.dwStyle = MB_OK | MB_USERICON;
    parameters.lpszIcon = MAKEINTRESOURCEW(IDI_BABA_CLOCK);
    parameters.dwContextHelpId = 0;
    parameters.lpfnMsgBoxCallback = NULL;
    parameters.dwLanguageId = 0;
    return MessageBoxIndirectW(&parameters);
}

static int RunClock(HINSTANCE instance)
{
    HWND window;
    MSG message;
    int x;
    int y;
    int width;
    int height;

    g_instance = instance;
    g_icon = LoadIconW(g_instance, MAKEINTRESOURCEW(IDI_BABA_CLOCK));
    x = (GetSystemMetrics(SM_CXSCREEN) - k24MinutesWidth) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - kBaseHeight) / 2;
    LoadState(&x, &y, &width, &height);
    if (!RegisterWindowClasses())
        return 1;

    window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                             kClockClassName, L"BABA Clock",
                             WS_POPUP | WS_THICKFRAME,
                             x, y, width, height, NULL, NULL, instance, NULL);
    if (!window)
        return 2;

    SetWindowPos(window, HWND_TOPMOST, x, y, width, height,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);
    if (IsFirstRun()) {
        MarkFirstRunShown();
        ShowIconMessage(window, kFirstRunText, L"BABA Clock");
    }

    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return (int)message.wParam;
}

extern "C" __declspec(noreturn) void EntryPoint()
{
    int result = RunClock((HINSTANCE)GetModuleHandleW(NULL));
    ExitProcess((UINT)result);
}
