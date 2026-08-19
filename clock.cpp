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

static const WCHAR kClockClassName[] = L"DADClockWindow";
static const WCHAR kSettingsClassName[] = L"DADClockSettingsWindow";
static const WCHAR kAboutClassName[] = L"DADClockAboutWindow";
static const char kRegistryPath[] = "Software\\DAD Clock";
static const char kLegacyRegistryPath[] = "Software\\BABA Clock";
static const WCHAR kStartupKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const WCHAR kStartupValue[] = L"DAD Clock";
static const WCHAR kLegacyStartupValue[] = L"BABA Clock";
static const WCHAR kRepositoryUrl[] = L"https://github.com/AAA-It-uae/DADClock";
static const WCHAR kSourceText[] = L"Source: https://github.com/AAA-It-uae/DADClock";
static const WCHAR kBuiltIn7SegmentText[] = L"Built-in: 7-Segment";
static const WCHAR kBuiltInClassicText[] = L"Built-in: Digital Classic";

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
static const UINT kFontComboControl = 206;
static const UINT kSourceLinkControl = 208;
static const UINT kShowMeridiemControl = 209;
static const UINT kTransparentControl = 210;
static const UINT kColorComboControl = 211;

static const int kFormat24 = 0;
static const int kFormat12 = 1;
static const int kFont7Segment = 0;
static const int kFontClassic = 1;
static const int kFontSystem = 2;
static const int kBaseHeight = 58;
static const int k24MinutesWidth = 156;
static const int k24SecondsWidth = 228;
static const int k12MinutesWidth = 224;
static const int k12SecondsWidth = 301;
static const int kSettingsWidth = 500;
static const int kSettingsHeight = 475;
static const int kAboutWidth = 520;
static const int kAboutHeight = 350;

struct ClockColorOption {
    const WCHAR *name;
    COLORREF color;
};

static const ClockColorOption kClockColorOptions[] = {
    { L"Classic Green", RGB(0, 255, 0) },
    { L"Lime",          RGB(128, 255, 0) },
    { L"Cyan",          RGB(0, 255, 255) },
    { L"Yellow",        RGB(255, 255, 0) },
    { L"Orange",        RGB(255, 165, 0) },
    { L"Red",           RGB(255, 64, 64) },
    { L"White",         RGB(255, 255, 255) },
    { L"Blue",          RGB(64, 160, 255) },
    { L"Magenta",       RGB(255, 64, 255) }
};
static const int kClockColorCount =
    sizeof(kClockColorOptions) / sizeof(kClockColorOptions[0]);

static const WCHAR kFirstRunText[] = L"Made with love for my beloved father \u2665";
static const WCHAR kBrandingText[] =
    L"DAD Clock\r\n"
    L"Built by Mohammad Taghi Alavi\r\n"
    L"Idea by Abbas Alavi\r\n"
    L"Made with love \u2665\r\n\r\n"
    L"Compatibility: Windows XP, Vista, 7, 8, 8.1, 10, 11\r\n"
    L"Technology: C++ / Win32 API / GDI\r\n"
    L"Architecture: 32-bit x86\r\n"
    L"License: Apache License 2.0";
static const WCHAR kSettingsCreditText[] =
    L"DAD Clock - Built by Mohammad Taghi Alavi";
static const WCHAR kFontHintText[] =
    L"Built-in styles are always available. Other entries are fonts installed on this Windows PC.";
static const WCHAR kLivePreviewHintText[] =
    L"Font and color changes preview live on the clock. Cancel restores the previous look.";
static const WCHAR kTransparentHintText[] =
    L"Transparent mode uses the classic Windows layered-window color key. Drag from a visible digit to move the clock.";

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
static BOOL g_showMeridiem;
static BOOL g_transparentBackground;
static int g_timeFormat;
static int g_fontStyle;
static COLORREF g_clockColor = RGB(0, 255, 0);
static BOOL g_colonVisible = TRUE;
static char g_digits[9] = "00:00:00";
static char g_meridiem[3] = "AM";
static WCHAR g_fontName[LF_FACESIZE] = L"Arial";
static BOOL g_settingsPreviewActive;
static int g_previewFontStyle;
static WCHAR g_previewFontName[LF_FACESIZE];
static COLORREF g_previewClockColor;

static int LayoutWidth()
{
    if (g_timeFormat == kFormat12 && g_showMeridiem)
        return g_showSeconds ? k12SecondsWidth : k12MinutesWidth;
    return g_showSeconds ? k24SecondsWidth : k24MinutesWidth;
}

static int Scaled(int value, int scale)
{
    return (value * scale) / 1000;
}

static BOOL WideEquals(const WCHAR *left, const WCHAR *right)
{
    int i = 0;

    while (left[i] || right[i]) {
        if (left[i] != right[i])
            return FALSE;
        ++i;
    }
    return TRUE;
}

static void CopyWideString(WCHAR *destination, const WCHAR *source, int capacity)
{
    int i = 0;

    if (capacity <= 0)
        return;
    while (i < capacity - 1 && source[i]) {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = L'\0';
}

static BOOL IsSupportedClockColor(COLORREF color)
{
    int i;

    for (i = 0; i < kClockColorCount; ++i) {
        if (kClockColorOptions[i].color == color)
            return TRUE;
    }
    return FALSE;
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

static BOOL QueryString(HKEY key, const WCHAR *name, WCHAR *value, DWORD capacity)
{
    DWORD type = 0;
    DWORD size = capacity * sizeof(WCHAR);

    if (!capacity)
        return FALSE;
    value[0] = L'\0';
    if (RegQueryValueExW(key, name, NULL, &type, (BYTE *)value, &size) != ERROR_SUCCESS ||
        type != REG_SZ || size < sizeof(WCHAR))
        return FALSE;
    value[capacity - 1] = L'\0';
    return TRUE;
}

static void WriteString(HKEY key, const WCHAR *name, const WCHAR *value)
{
    DWORD length = 0;

    while (value[length])
        ++length;
    RegSetValueExW(key, name, 0, REG_SZ, (const BYTE *)value,
                   (length + 1) * sizeof(WCHAR));
}

static BOOL BuildStartupCommand(WCHAR *command, DWORD capacity)
{
    WCHAR path[MAX_PATH];
    DWORD length;
    DWORD i;

    length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (!length || length >= MAX_PATH - 1 || capacity < length + 3)
        return FALSE;

    command[0] = L'"';
    for (i = 0; i < length; ++i)
        command[i + 1] = path[i];
    command[length + 1] = L'"';
    command[length + 2] = L'\0';
    return TRUE;
}

static BOOL StartupValueMatches(HKEY key, const WCHAR *valueName,
                                const WCHAR *expected)
{
    WCHAR stored[MAX_PATH + 4];
    DWORD type = 0;
    DWORD size = sizeof(stored);

    if (RegQueryValueExW(key, valueName, NULL, &type, (BYTE *)stored, &size) != ERROR_SUCCESS ||
        type != REG_SZ)
        return FALSE;
    stored[(MAX_PATH + 4) - 1] = L'\0';
    return WideEquals(stored, expected);
}

static BOOL IsStartupEnabled()
{
    HKEY key;
    WCHAR expected[MAX_PATH + 4];
    BOOL enabled;

    if (!BuildStartupCommand(expected, MAX_PATH + 4))
        return FALSE;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kStartupKey, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return FALSE;

    enabled = StartupValueMatches(key, kStartupValue, expected);
    if (!enabled)
        enabled = StartupValueMatches(key, kLegacyStartupValue, expected);
    RegCloseKey(key);
    return enabled;
}

static BOOL ConfigureStartup(BOOL enabled)
{
    HKEY key;
    LONG result;
    LONG legacyResult;

    if (!enabled) {
        result = RegOpenKeyExW(HKEY_CURRENT_USER, kStartupKey, 0, KEY_SET_VALUE, &key);
        if (result == ERROR_FILE_NOT_FOUND)
            return TRUE;
        if (result != ERROR_SUCCESS)
            return FALSE;
        result = RegDeleteValueW(key, kStartupValue);
        legacyResult = RegDeleteValueW(key, kLegacyStartupValue);
        RegCloseKey(key);
        return (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND) &&
               (legacyResult == ERROR_SUCCESS || legacyResult == ERROR_FILE_NOT_FOUND);
    }

    result = RegCreateKeyExW(HKEY_CURRENT_USER, kStartupKey, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                             &key, NULL);
    if (result != ERROR_SUCCESS)
        return FALSE;

    {
        WCHAR command[MAX_PATH + 4];
        DWORD length = 0;

        if (!BuildStartupCommand(command, MAX_PATH + 4)) {
            RegCloseKey(key);
            return FALSE;
        }
        while (command[length])
            ++length;
        result = RegSetValueExW(key, kStartupValue, 0, REG_SZ,
                                (const BYTE *)command, (length + 1) * sizeof(WCHAR));
        if (result == ERROR_SUCCESS)
            RegDeleteValueW(key, kLegacyStartupValue);
    }
    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}

static void ClampPositionToNearestMonitor(int *x, int *y, int width, int height)
{
    RECT rect;
    MONITORINFO info;
    HMONITOR monitor;
    int workWidth;
    int workHeight;

    rect.left = *x;
    rect.top = *y;
    rect.right = *x + width;
    rect.bottom = *y + height;
    monitor = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);
    if (!monitor)
        return;

    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info))
        return;

    workWidth = info.rcWork.right - info.rcWork.left;
    workHeight = info.rcWork.bottom - info.rcWork.top;

    if (width >= workWidth)
        *x = info.rcWork.left;
    else {
        if (*x < info.rcWork.left)
            *x = info.rcWork.left;
        if (*x + width > info.rcWork.right)
            *x = info.rcWork.right - width;
    }

    if (height >= workHeight)
        *y = info.rcWork.top;
    else {
        if (*y < info.rcWork.top)
            *y = info.rcWork.top;
        if (*y + height > info.rcWork.bottom)
            *y = info.rcWork.bottom - height;
    }
}

static void CenterOnOwnerMonitor(HWND owner, int width, int height, int *x, int *y)
{
    MONITORINFO info;
    HMONITOR monitor;
    int workWidth;
    int workHeight;

    monitor = MonitorFromWindow(owner, MONITOR_DEFAULTTONEAREST);
    info.cbSize = sizeof(info);
    if (monitor && GetMonitorInfoW(monitor, &info)) {
        workWidth = info.rcWork.right - info.rcWork.left;
        workHeight = info.rcWork.bottom - info.rcWork.top;
        *x = info.rcWork.left + (workWidth - width) / 2;
        *y = info.rcWork.top + (workHeight - height) / 2;
        ClampPositionToNearestMonitor(x, y, width, height);
        return;
    }

    *x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    *y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
}

static void MoveClockIntoVisibleWorkArea(HWND window)
{
    RECT rect;
    int x;
    int y;
    int width;
    int height;

    if (!GetWindowRect(window, &rect))
        return;
    x = rect.left;
    y = rect.top;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    ClampPositionToNearestMonitor(&x, &y, width, height);
    if (x != rect.left || y != rect.top)
        SetWindowPos(window, NULL, x, y, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static void LoadState(int *x, int *y, int *width, int *height)
{
    HKEY key;
    DWORD value;
    BOOL haveWidth = FALSE;
    LONG openResult;

    *height = kBaseHeight;
    g_showSeconds = FALSE;
    g_blinkColon = TRUE;
    g_runAtStartup = IsStartupEnabled();
    g_showMeridiem = TRUE;
    g_transparentBackground = FALSE;
    g_timeFormat = kFormat24;
    g_fontStyle = kFont7Segment;
    g_clockColor = RGB(0, 255, 0);
    g_fontName[0] = L'A';
    g_fontName[1] = L'r';
    g_fontName[2] = L'i';
    g_fontName[3] = L'a';
    g_fontName[4] = L'l';
    g_fontName[5] = L'\0';

    openResult = RegOpenKeyExA(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_QUERY_VALUE, &key);
    if (openResult != ERROR_SUCCESS)
        openResult = RegOpenKeyExA(HKEY_CURRENT_USER, kLegacyRegistryPath, 0, KEY_QUERY_VALUE, &key);
    if (openResult != ERROR_SUCCESS) {
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
    if (QueryDword(key, "TimeFormat", &value) && value <= kFormat12)
        g_timeFormat = (int)value;
    if (QueryDword(key, "FontStyle", &value) && value <= kFontSystem)
        g_fontStyle = (int)value;
    if (QueryDword(key, "ShowMeridiem", &value))
        g_showMeridiem = value != 0;
    if (QueryDword(key, "TransparentBackground", &value))
        g_transparentBackground = value != 0;
    if (QueryDword(key, "ClockColor", &value) && IsSupportedClockColor((COLORREF)value))
        g_clockColor = (COLORREF)value;
    QueryString(key, L"FontName", g_fontName, LF_FACESIZE);

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
    WriteDword(key, "FontStyle", (DWORD)g_fontStyle);
    WriteDword(key, "ShowMeridiem", g_showMeridiem ? 1 : 0);
    WriteDword(key, "TransparentBackground", g_transparentBackground ? 1 : 0);
    WriteDword(key, "ClockColor", (DWORD)g_clockColor);
    WriteString(key, L"FontName", g_fontName);
    RegCloseKey(key);
}

static BOOL IsFirstRun()
{
    HKEY key;
    DWORD value;
    BOOL firstRun = TRUE;
    BOOL found = FALSE;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
        if (QueryDword(key, "FirstRunShown", &value)) {
            firstRun = value == 0;
            found = TRUE;
        }
        RegCloseKey(key);
    }
    if (!found && RegOpenKeyExA(HKEY_CURRENT_USER, kLegacyRegistryPath, 0,
                                KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
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

static void ApplyTransparency(HWND window)
{
    LONG exStyle = GetWindowLongW(window, GWL_EXSTYLE);
    LONG desiredStyle = exStyle;

    if (g_transparentBackground)
        desiredStyle |= WS_EX_LAYERED;
    else
        desiredStyle &= ~WS_EX_LAYERED;

    if (desiredStyle != exStyle) {
        SetWindowLongW(window, GWL_EXSTYLE, desiredStyle);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    if (g_transparentBackground)
        SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_COLORKEY);

    RedrawWindow(window, NULL, NULL,
                 RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
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
    (void)brush;
    r.left = offsetX + Scaled(left, scale);
    r.top = offsetY + Scaled(top, scale);
    r.right = offsetX + Scaled(right, scale);
    r.bottom = offsetY + Scaled(bottom, scale);
    FillRect(dc, &r, (HBRUSH)GetCurrentObject(dc, OBJ_BRUSH));
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

static void DrawSegment(HWND window, HDC dc, int x, int y, int segment,
                        int offsetX, int offsetY, int scale, HBRUSH brush)
{
    int left;
    int top;
    int right;
    int bottom;
    (void)window;

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

static void DrawDigit(HWND window, HDC dc, int x, int y, int digit,
                      int offsetX, int offsetY, int scale, HBRUSH brush)
{
    unsigned char mask = kSegments[digit];
    int segment;

    for (segment = 0; segment < 7; ++segment) {
        if (mask & (1 << segment))
            DrawSegment(window, dc, x, y, segment, offsetX, offsetY, scale, brush);
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

static void DrawLetter(HWND window, HDC dc, int x, int y, char letter,
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
            DrawSegment(window, dc, x, y, segment, offsetX, offsetY, scale, brush);
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

static int BuildDisplayText(WCHAR *text, int capacity)
{
    int count = 0;
    int i;
    int digitCount = g_showSeconds ? 8 : 5;

    for (i = 0; i < digitCount && count < capacity - 1; ++i) {
        WCHAR character = (WCHAR)(unsigned char)g_digits[i];
        if (character == L':' && !g_colonVisible)
            character = L' ';
        text[count++] = character;
    }

    if (g_timeFormat == kFormat12 && g_showMeridiem && count < capacity - 4) {
        text[count++] = L' ';
        text[count++] = (WCHAR)(unsigned char)g_meridiem[0];
        text[count++] = (WCHAR)(unsigned char)g_meridiem[1];
    }
    text[count] = L'\0';
    return count;
}

static BOOL PaintSystemFontClock(HWND window, HDC dc, const RECT *client)
{
    WCHAR text[16];
    int textLength;
    int clientWidth = client->right - client->left;
    int clientHeight = client->bottom - client->top;
    int availableWidth = clientWidth > 8 ? clientWidth - 8 : clientWidth;
    int availableHeight = clientHeight > 4 ? clientHeight - 4 : clientHeight;
    int fontHeight = availableHeight * 9 / 10;
    int adjustedHeight;
    int x;
    int y;
    int oldBkMode;
    COLORREF oldColor;
    HFONT font;
    HFONT oldFont;
    SIZE textSize;
    BYTE quality = g_transparentBackground ? NONANTIALIASED_QUALITY : DEFAULT_QUALITY;
    (void)window;

    if (fontHeight < 8)
        fontHeight = 8;
    textLength = BuildDisplayText(text, 16);

    font = CreateFontW(-fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       quality, DEFAULT_PITCH | FF_DONTCARE, g_fontName);
    if (!font)
        return FALSE;

    oldFont = (HFONT)SelectObject(dc, font);
    if (!GetTextExtentPoint32W(dc, text, textLength, &textSize)) {
        SelectObject(dc, oldFont);
        DeleteObject(font);
        return FALSE;
    }

    if ((textSize.cx > availableWidth || textSize.cy > availableHeight) &&
        textSize.cx > 0 && textSize.cy > 0) {
        adjustedHeight = fontHeight;
        if (textSize.cx > availableWidth)
            adjustedHeight = (fontHeight * availableWidth) / textSize.cx;
        if (textSize.cy > availableHeight) {
            int byHeight = (fontHeight * availableHeight) / textSize.cy;
            if (byHeight < adjustedHeight)
                adjustedHeight = byHeight;
        }
        if (adjustedHeight < 6)
            adjustedHeight = 6;

        SelectObject(dc, oldFont);
        DeleteObject(font);
        fontHeight = adjustedHeight;
        font = CreateFontW(-fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           quality, DEFAULT_PITCH | FF_DONTCARE, g_fontName);
        if (!font)
            return FALSE;
        oldFont = (HFONT)SelectObject(dc, font);
        if (!GetTextExtentPoint32W(dc, text, textLength, &textSize)) {
            SelectObject(dc, oldFont);
            DeleteObject(font);
            return FALSE;
        }
    }

    x = (clientWidth - textSize.cx) / 2;
    y = (clientHeight - textSize.cy) / 2;
    oldBkMode = SetBkMode(dc, TRANSPARENT);
    oldColor = SetTextColor(dc, g_clockColor);
    TextOutW(dc, x, y, text, textLength);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldBkMode);
    SelectObject(dc, oldFont);
    DeleteObject(font);
    return TRUE;
}

static void PaintClock(HWND window, HDC dc)
{
    RECT client;
    HBRUSH clockBrush;
    HBRUSH black;
    HBRUSH oldBrush;
    HPEN clockPen;
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

    if (g_fontStyle == kFontSystem && PaintSystemFontClock(window, dc, &client))
        return;

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

    clockBrush = CreateSolidBrush(g_clockColor);
    clockPen = CreatePen(PS_SOLID, 1, g_clockColor);
    if (!clockBrush || !clockPen) {
        if (clockPen) DeleteObject(clockPen);
        if (clockBrush) DeleteObject(clockBrush);
        return;
    }
    oldBrush = (HBRUSH)SelectObject(dc, clockBrush);
    oldPen = (HPEN)SelectObject(dc, clockPen);

    DrawDigit(window, dc, 8, 7, g_digits[0] - '0', offsetX, offsetY, scale, clockBrush);
    DrawDigit(window, dc, 39, 7, g_digits[1] - '0', offsetX, offsetY, scale, clockBrush);
    if (g_colonVisible)
        DrawColon(dc, 72, offsetX, offsetY, scale, clockBrush);
    DrawDigit(window, dc, 85, 7, g_digits[3] - '0', offsetX, offsetY, scale, clockBrush);
    DrawDigit(window, dc, 116, 7, g_digits[4] - '0', offsetX, offsetY, scale, clockBrush);

    if (g_showSeconds) {
        if (g_colonVisible)
            DrawColon(dc, 149, offsetX, offsetY, scale, clockBrush);
        DrawDigit(window, dc, 162, 7, g_digits[6] - '0', offsetX, offsetY, scale, clockBrush);
        DrawDigit(window, dc, 193, 7, g_digits[7] - '0', offsetX, offsetY, scale, clockBrush);
    }

    if (g_timeFormat == kFormat12 && g_showMeridiem) {
        meridiemX = g_showSeconds ? 236 : 159;
        DrawLetter(window, dc, meridiemX, 7, g_meridiem[0], offsetX, offsetY, scale, clockBrush);
        DrawLetter(window, dc, meridiemX + 31, 7, g_meridiem[1], offsetX, offsetY, scale, clockBrush);
    }

    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(clockPen);
    DeleteObject(clockBrush);
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
    MoveClockIntoVisibleWorkArea(window);
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

static int CALLBACK FontEnumProc(const LOGFONTW *logFont, const TEXTMETRICW *,
                                 DWORD, LPARAM lParam)
{
    HWND combo = (HWND)lParam;
    const WCHAR *face = logFont->lfFaceName;

    if (!face[0] || face[0] == L'@')
        return 1;
    if (SendMessageW(combo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)face) == CB_ERR)
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)face);
    return 1;
}

static void PopulateFontCombo(HWND window)
{
    HWND combo = GetDlgItem(window, kFontComboControl);
    HDC dc;
    LOGFONTW logFont;
    int i;
    LRESULT selection;

    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)kBuiltIn7SegmentText);
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)kBuiltInClassicText);

    logFont.lfHeight = 0;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfWeight = 0;
    logFont.lfItalic = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 0;
    logFont.lfPitchAndFamily = 0;
    for (i = 0; i < LF_FACESIZE; ++i)
        logFont.lfFaceName[i] = L'\0';

    dc = GetDC(window);
    if (dc) {
        EnumFontFamiliesExW(dc, &logFont, FontEnumProc, (LPARAM)combo, 0);
        ReleaseDC(window, dc);
    }

    if (g_fontStyle == kFont7Segment)
        selection = 0;
    else if (g_fontStyle == kFontClassic)
        selection = 1;
    else {
        selection = SendMessageW(combo, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                 (LPARAM)g_fontName);
        if (selection == CB_ERR) {
            SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)g_fontName);
            selection = SendMessageW(combo, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                     (LPARAM)g_fontName);
        }
    }
    SendMessageW(combo, CB_SETCURSEL, (WPARAM)selection, 0);
}

static void PopulateColorCombo(HWND window)
{
    HWND combo = GetDlgItem(window, kColorComboControl);
    int i;
    int selection = 0;

    for (i = 0; i < kClockColorCount; ++i) {
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)kClockColorOptions[i].name);
        if (kClockColorOptions[i].color == g_clockColor)
            selection = i;
    }
    SendMessageW(combo, CB_SETCURSEL, (WPARAM)selection, 0);
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

static void ApplySelectedFont(HWND settingsWindow)
{
    HWND combo = GetDlgItem(settingsWindow, kFontComboControl);
    LRESULT selection = SendMessageW(combo, CB_GETCURSEL, 0, 0);

    if (selection == 0) {
        g_fontStyle = kFont7Segment;
        return;
    }
    if (selection == 1) {
        g_fontStyle = kFontClassic;
        return;
    }
    if (selection != CB_ERR) {
        g_fontStyle = kFontSystem;
        SendMessageW(combo, CB_GETLBTEXT, (WPARAM)selection, (LPARAM)g_fontName);
        g_fontName[LF_FACESIZE - 1] = L'\0';
        return;
    }
    g_fontStyle = kFont7Segment;
}

static void ApplySelectedColor(HWND settingsWindow)
{
    HWND combo = GetDlgItem(settingsWindow, kColorComboControl);
    LRESULT selection = SendMessageW(combo, CB_GETCURSEL, 0, 0);

    if (selection >= 0 && selection < kClockColorCount)
        g_clockColor = kClockColorOptions[(int)selection].color;
}

static void PreviewFontAndColor(HWND settingsWindow)
{
    if (!g_settingsOwner)
        return;
    ApplySelectedFont(settingsWindow);
    ApplySelectedColor(settingsWindow);
    InvalidateRect(g_settingsOwner, NULL, FALSE);
}

static void RestoreSettingsPreview()
{
    if (!g_settingsPreviewActive)
        return;

    g_fontStyle = g_previewFontStyle;
    CopyWideString(g_fontName, g_previewFontName, LF_FACESIZE);
    g_clockColor = g_previewClockColor;
    g_settingsPreviewActive = FALSE;
    if (g_settingsOwner)
        InvalidateRect(g_settingsOwner, NULL, FALSE);
}

static void ApplySettings(HWND settingsWindow)
{
    int oldBaseWidth = LayoutWidth();
    int oldWidth;
    int oldHeight;
    RECT oldRect;
    BOOL requestedStartup;

    if (!GetWindowRect(g_settingsOwner, &oldRect))
        return;
    oldWidth = oldRect.right - oldRect.left;
    oldHeight = oldRect.bottom - oldRect.top;

    g_showSeconds = SendMessageW(GetDlgItem(settingsWindow, kShowSecondsControl),
                                 BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_blinkColon = SendMessageW(GetDlgItem(settingsWindow, kBlinkColonControl),
                                BM_GETCHECK, 0, 0) == BST_CHECKED;
    requestedStartup = SendMessageW(GetDlgItem(settingsWindow, kStartupControl),
                                    BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_showMeridiem = SendMessageW(GetDlgItem(settingsWindow, kShowMeridiemControl),
                                  BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_transparentBackground = SendMessageW(GetDlgItem(settingsWindow, kTransparentControl),
                                           BM_GETCHECK, 0, 0) == BST_CHECKED;
    ApplySelectedFont(settingsWindow);
    ApplySelectedColor(settingsWindow);

    if (!ConfigureStartup(requestedStartup))
        MessageBoxW(settingsWindow,
                    L"Windows startup registration could not be changed. The other settings were saved.",
                    L"DAD Clock", MB_OK | MB_ICONWARNING);
    g_runAtStartup = IsStartupEnabled();

    ApplyTransparency(g_settingsOwner);
    ApplyLayoutSize(g_settingsOwner, oldBaseWidth, oldWidth, oldHeight);
    UpdateDigits(g_settingsOwner);
    SaveWindowState(g_settingsOwner);
    g_settingsPreviewActive = FALSE;
}

static LRESULT CALLBACK SettingsWindowProc(HWND window, UINT message,
                                           WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        CreateWindowExW(0, L"BUTTON", L"Show Seconds",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 14, 450, 24, window, (HMENU)kShowSecondsControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Blink Separator / Colon",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 42, 450, 24, window, (HMENU)kBlinkColonControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Run at Windows Startup",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 70, 450, 24, window, (HMENU)kStartupControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Show AM / PM in 12-Hour Mode",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 98, 450, 24, window, (HMENU)kShowMeridiemControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Transparent Background",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                        16, 126, 450, 24, window, (HMENU)kTransparentControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"STATIC", L"Clock Font",
                        WS_CHILD | WS_VISIBLE,
                        16, 158, 450, 20, window, NULL, g_instance, NULL);
        CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                        CBS_DROPDOWNLIST,
                        16, 179, 452, 210, window, (HMENU)kFontComboControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"STATIC", kFontHintText,
                        WS_CHILD | WS_VISIBLE | SS_LEFT,
                        16, 207, 452, 34, window, NULL, g_instance, NULL);
        CreateWindowExW(0, L"STATIC", L"Clock Color",
                        WS_CHILD | WS_VISIBLE,
                        16, 244, 450, 20, window, NULL, g_instance, NULL);
        CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                        CBS_DROPDOWNLIST,
                        16, 265, 452, 190, window, (HMENU)kColorComboControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"STATIC", kLivePreviewHintText,
                        WS_CHILD | WS_VISIBLE | SS_LEFT,
                        16, 294, 452, 20, window, NULL, g_instance, NULL);
        CreateWindowExW(0, L"STATIC", kTransparentHintText,
                        WS_CHILD | WS_VISIBLE | SS_LEFT,
                        16, 318, 452, 35, window, NULL, g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"OK",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                        176, 360, 64, 25, window, (HMENU)kOkControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"BUTTON", L"Cancel",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                        248, 360, 64, 25, window, (HMENU)kCancelControl,
                        g_instance, NULL);
        CreateWindowExW(0, L"STATIC", kSettingsCreditText,
                        WS_CHILD | WS_VISIBLE | SS_CENTER,
                        10, 395, 480, 20, window, NULL, g_instance, NULL);
        CreateSourceLink(window, 10, 419, 480, 22);

        SendMessageW(GetDlgItem(window, kShowSecondsControl), BM_SETCHECK,
                     g_showSeconds ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kBlinkColonControl), BM_SETCHECK,
                     g_blinkColon ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kStartupControl), BM_SETCHECK,
                     g_runAtStartup ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kShowMeridiemControl), BM_SETCHECK,
                     g_showMeridiem ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kTransparentControl), BM_SETCHECK,
                     g_transparentBackground ? BST_CHECKED : BST_UNCHECKED, 0);
        PopulateFontCombo(window);
        PopulateColorCombo(window);
        SetFocus(GetDlgItem(window, kOkControl));
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE &&
            (LOWORD(wParam) == kFontComboControl || LOWORD(wParam) == kColorComboControl)) {
            PreviewFontAndColor(window);
            return 0;
        }
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
        RestoreSettingsPreview();
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
    int x;
    int y;

    if (g_settingsWindow) {
        SetForegroundWindow(g_settingsWindow);
        return;
    }

    CenterOnOwnerMonitor(owner, kSettingsWidth, kSettingsHeight, &x, &y);
    g_settingsOwner = owner;
    g_previewFontStyle = g_fontStyle;
    CopyWideString(g_previewFontName, g_fontName, LF_FACESIZE);
    g_previewClockColor = g_clockColor;
    g_settingsPreviewActive = TRUE;
    EnableWindow(owner, FALSE);
    g_settingsWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
                                       kSettingsClassName, L"DAD Clock Settings",
                                       WS_POPUP | WS_CAPTION | WS_SYSMENU,
                                       x, y, kSettingsWidth, kSettingsHeight,
                                       owner, NULL, g_instance, NULL);
    if (!g_settingsWindow) {
        g_settingsPreviewActive = FALSE;
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
    int x;
    int y;

    if (g_aboutWindow) {
        SetForegroundWindow(g_aboutWindow);
        return;
    }

    CenterOnOwnerMonitor(owner, kAboutWidth, kAboutHeight, &x, &y);
    g_aboutOwner = owner;
    EnableWindow(owner, FALSE);
    g_aboutWindow = CreateWindowExW(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
                                    kAboutClassName, L"About DAD Clock",
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
        ApplyTransparency(window);
        UpdateDigits(window);
        SetTimer(window, kTimerId, 1000, NULL);
        return 0;

    case WM_TIMER:
        if (wParam == kTimerId)
            UpdateDigits(window);
        return 0;

    case WM_DISPLAYCHANGE:
        MoveClockIntoVisibleWorkArea(window);
        SaveWindowState(window);
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
    parameters.lpszIcon = MAKEINTRESOURCEW(IDI_DAD_CLOCK);
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
    g_icon = LoadIconW(g_instance, MAKEINTRESOURCEW(IDI_DAD_CLOCK));
    x = (GetSystemMetrics(SM_CXSCREEN) - k24MinutesWidth) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - kBaseHeight) / 2;
    LoadState(&x, &y, &width, &height);
    ClampPositionToNearestMonitor(&x, &y, width, height);
    if (!RegisterWindowClasses())
        return 1;

    window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                             kClockClassName, L"DAD Clock",
                             WS_POPUP | WS_THICKFRAME,
                             x, y, width, height, NULL, NULL, instance, NULL);
    if (!window)
        return 2;

    SetWindowPos(window, HWND_TOPMOST, x, y, width, height,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);
    if (IsFirstRun()) {
        MarkFirstRunShown();
        ShowIconMessage(window, kFirstRunText, L"DAD Clock");
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
