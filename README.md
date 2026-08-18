<div align="center">

<img src="assets/BABAClock-11-11.png" width="160" alt="BABA Clock 11:11 icon">

# BABA Clock

**A tiny, native, always-on-top digital clock for Windows XP through Windows 11.**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
![Windows](https://img.shields.io/badge/Windows-XP%20%E2%86%92%2011-0078D6)
![Architecture](https://img.shields.io/badge/architecture-x86-lightgrey)
![C++](https://img.shields.io/badge/C%2B%2B-Win32%20%2B%20GDI-00599C)
![Portable](https://img.shields.io/badge/runtime-single%20EXE-success)

*Made with love for my beloved father ♥*

</div>

---

## What is BABA Clock?

**BABA Clock** is a minimal desktop clock overlay built for one job: keep the time visible without getting in the way.

It is a native **32-bit C++ / Win32 / GDI** application. There is no .NET, Qt, Electron, browser engine, external font, configuration file, or application runtime to install. The checked-in release is a single portable `BABAClock.exe`.

> Repository name: `DADClock`  
> Product name: **BABA Clock**

## Download and run

The current portable build is included in the repository:

**[BABAClock.exe](BABAClock.exe)**

Copy the EXE anywhere and run it. No installer is required.

Windows may show a security warning for an unsigned executable. The complete source and build script are included so the binary can also be built locally.

## Features

The following features are implemented in the current source:

- **Always on top** — the clock stays above normal application windows.
- **Borderless black clock surface** with bright green digital digits.
- **Native GDI rendering** — digits are drawn directly; no external font is required.
- **Two clock styles**
  - `7-Segment`
  - `Digital Classic`
- **12-hour and 24-hour formats**
  - 24-hour is the default.
  - Switching from the right-click menu updates immediately.
  - 12-hour mode displays a proportional digital `AM` / `PM` indicator.
  - The active format is check-marked in the menu.
- **Optional seconds display**.
- **Optional blinking colon** when seconds are hidden.
- **Mouse dragging** — left-drag anywhere on the clock to move it.
- **Mouse resizing** from the window edges and corners.
- **Fixed aspect ratio** while resizing, so the digits do not stretch or deform.
- **Persistent size and position** across launches.
- **Run at Windows Startup** option for the current user.
- **Settings and About windows** with a clickable link back to this repository.
- **First-run dedication message**, shown only once.
- **Low-overhead 1-second update timer** with redraws only when the displayed state changes.
- **Single-file runtime** — no companion application files are required.

## Controls

| Action | Result |
|---|---|
| Left-drag the clock | Move it |
| Drag an edge or corner | Resize with aspect ratio preserved |
| Right-click | Open the clock menu |
| `Settings` | Open display/startup settings |
| `Time Format → 24-Hour` | Switch immediately to 24-hour time |
| `Time Format → 12-Hour` | Switch immediately to 12-hour time with AM/PM |
| `About` | Show project, author, compatibility and license information |
| `Exit` | Close BABA Clock |

## Settings

### Show Seconds

Shows or hides seconds beside hours and minutes.

### Blink Separator / Colon

When seconds are **hidden**, the hour/minute colon can blink once per second.

When seconds are visible, the separators remain visible so the full `HH:MM:SS` display stays stable.

### Run at Windows Startup

Adds or removes BABA Clock from the current user's Windows startup registry entry.

If the EXE is moved after startup has been enabled, disable and re-enable this option so Windows stores the new path.

### Clock Font

Choose between:

- **7-Segment** — rectangular digital segments.
- **Digital Classic** — a more traditional shaped digital-segment style.

Both styles are rendered directly with GDI.

## Time formats

### 24-hour

```text
23:57
23:57:42
```

### 12-hour

```text
11:57 PM
11:57:42 PM
```

The selected format is saved automatically and restored on the next launch.

## First run

On the first launch only, BABA Clock displays:

> **Made with love for my beloved father ♥**

A registry flag records that the message has already been shown.

## Saved state

BABA Clock stores its application state under the current Windows user:

```text
HKEY_CURRENT_USER\Software\BABA Clock
```

The implementation persists:

- window X/Y position
- window width/height
- show-seconds setting
- blink-colon setting
- startup preference
- 12/24-hour format
- selected clock style
- first-run state

The startup command itself is stored under:

```text
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
```

No separate `.ini`, `.json`, database, or configuration file is required.

## Compatibility

BABA Clock is intentionally built as an **x86 / 32-bit** Windows application and targets the **Windows 5.01 subsystem**.

| Windows version | Project target |
|---|---|
| Windows XP | Yes |
| Windows Vista | Yes |
| Windows 7 | Yes |
| Windows 8 | Yes |
| Windows 8.1 | Yes |
| Windows 10 | Yes |
| Windows 11 | Yes |

On 64-bit Windows, the x86 executable runs through the normal Windows 32-bit compatibility layer.

The code defines:

```cpp
_WIN32_WINNT 0x0501
WINVER       0x0501
```

and intentionally uses Win32 APIs available to the XP-era target.

## Technology

| Layer | Implementation |
|---|---|
| Language | C++ |
| Windowing | Win32 API |
| Rendering | GDI |
| Architecture | x86 / 32-bit |
| Runtime UI framework | None |
| C/C++ runtime dependency | None by design |
| External application assets at runtime | None |
| State | Windows Registry |
| Build | Microsoft C++ toolchain + `build.bat` |
| License | Apache License 2.0 |

The build links only against standard Windows system libraries used by the application:

```text
kernel32.lib
user32.lib
gdi32.lib
advapi32.lib
shell32.lib
```

## Build

### Requirements

Install Visual Studio 2019 or 2022 / Build Tools with the **x86 C++ toolchain**.

Then run:

```bat
build.bat
```

The script:

1. locates an installed Visual Studio C++ toolchain,
2. initializes the 32-bit compiler environment,
3. compiles `clock.cpp`,
4. compiles the Windows resource file,
5. links the executable for `WINDOWS,5.01`,
6. links with `/NODEFAULTLIB`,
7. uses the custom `EntryPoint`,
8. removes temporary object/resource files,
9. leaves `BABAClock.exe`.

The build deliberately avoids a C/C++ runtime dependency and uses only the Windows system libraries required by the program.

## Project structure

```text
DADClock/
├─ assets/
│  └─ BABAClock-11-11.png   # 11:11 project artwork
├─ BABAClock.exe             # Current portable build
├─ BABAClock.ico             # Compact icon embedded by the current build
├─ BABAClock.rc              # Windows resource definition
├─ build.bat                 # x86 / XP-targeted build script
├─ clock.cpp                 # Application source
├─ make_icon.py              # Compact embedded-icon generator
├─ resource.h                # Resource identifiers
├─ LICENSE                   # Apache License 2.0
├─ NOTICE                    # Project attribution
└─ README.md
```

### Artwork

The `assets/` directory contains the new **11:11 BABA Clock** artwork used for the repository identity and README.

The current executable still uses the deliberately compact `BABAClock.ico` resource. Keeping the high-detail artwork separate avoids silently changing the checked-in executable without rebuilding it and preserves the project's very small runtime footprint.

## Implementation notes

A few details are intentional:

- The main clock uses `WS_EX_TOPMOST`.
- The window has no visible non-client frame.
- `WM_NCHITTEST` provides resize zones around the edges.
- `WM_SIZING` enforces the clock's current layout ratio.
- Time is read using `GetLocalTime`.
- A 1-second timer updates the display.
- GDI primitives draw digits, separators, and AM/PM.
- Settings and position are persisted in `HKCU`.
- Startup registration uses the current executable path.
- The project URL in Settings/About opens with the user's default browser.

## About dialog

The application identifies itself with:

```text
BABA Clock
Built by Mohammad Taghi Alavi
Idea by Abbas Alavi
Made with love ♥

Compatibility: Windows XP, Vista, 7, 8, 8.1, 10, 11
Technology: C++ / Win32 API / GDI
Architecture: 32-bit x86
License: Apache License 2.0
Source: https://github.com/AAA-It-uae/DADClock
```

## Open source

BABA Clock is released under the **Apache License 2.0**.

See:

- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

Redistributions and derivative works must preserve the applicable license and attribution notices as required by the license.

## Credits

**Built by:** Mohammad Taghi Alavi  
**Idea by:** Abbas Alavi  
**Source:** https://github.com/AAA-It-uae/DADClock

<div align="center">

### BABA Clock

**Small. Native. Portable.**

*Made with love ♥*

</div>
