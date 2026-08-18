<div align="center">

# BABA Clock

**A tiny, portable, always-on-top digital clock for Windows XP through Windows 11.**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
![Platform](https://img.shields.io/badge/platform-Windows%20XP--11-0078D6)
![Architecture](https://img.shields.io/badge/architecture-x86-lightgrey)
![Language](https://img.shields.io/badge/language-C%2B%2B-00599C)
![API](https://img.shields.io/badge/API-Win32%20%2B%20GDI-4B4B4B)

*Made with love for my beloved father ♥*

</div>

---

## Overview

**BABA Clock** is a minimal digital clock overlay designed to stay visible above other Windows applications while using almost no system resources.

It is built as a native **32-bit Win32 application** with **C++ and GDI**, with no .NET, Electron, Qt, external runtime, external fonts, or required companion files. The goal is simple: one small `BABAClock.exe` that can be copied and run directly.

## Features

- Always-on-top digital clock overlay
- Bright green 7-segment digits on a black background
- Borderless, lightweight native window
- Mouse-drag repositioning
- Mouse resizing with a fixed aspect ratio
- Proportional scaling without deforming the digits
- Optional seconds display
- Optional blinking separator / colon
- Optional **Run at Windows Startup**
- Saves window position, size, and settings in the current user's registry
- Right-click menu with **Settings**, **About**, and **Exit**
- First-run dedication message
- Extremely low CPU and memory usage
- Single portable executable with no required external files

## Compatibility

| Operating System | Support |
|---|---|
| Windows XP | Yes |
| Windows Vista | Yes |
| Windows 7 | Yes |
| Windows 8 / 8.1 | Yes |
| Windows 10 | Yes |
| Windows 11 | Yes |

The application targets **x86 / 32-bit Windows** for maximum backward compatibility. It can also run on supported 64-bit Windows versions through the Windows 32-bit compatibility layer.

## Technology

- **Language:** C++
- **UI / Windowing:** Win32 API
- **Rendering:** GDI
- **Architecture:** x86 / 32-bit
- **Target subsystem:** Windows XP compatible
- **Dependencies:** None at runtime
- **Packaging:** Single portable EXE

No external frameworks are required.

## User Interface

The main window is intentionally minimal:

- black rectangular background
- bright green digital digits
- no title bar
- no unnecessary controls
- always visible above normal application windows

The window can be moved and resized directly with the mouse while preserving the clock's aspect ratio.

## Settings

BABA Clock provides only the settings needed for daily use:

- **Show Seconds** — show or hide seconds beside the hour and minute digits
- **Blink Separator / Colon** — enable or disable separator blinking
- **Run at Windows Startup** — start BABA Clock automatically when the current user signs in

Settings, window size, and window position are persisted in the current user's Windows Registry.

## First Run

On the first launch only, BABA Clock displays:

> Made with love for my beloved father ♥

The first-run state is then saved so the message is not shown automatically again.

## About

The About dialog identifies the original project and its technology:

```text
BABA Clock
Built by Mohammad Taghi Alavi
Idea by Abbas Alavi
Made with love ♥

Compatibility: Windows XP, Vista, 7, 8, 8.1, 10, 11
Technology: C++ / Win32 API / GDI
Architecture: 32-bit x86
```

## Build

The project is intended to build with the Microsoft C++ toolchain as a native **x86** Windows application while avoiding APIs newer than Windows XP.

Typical build flow:

```bat
build.bat
```

The final release artifact should be:

```text
BABAClock.exe
```

The executable must remain self-contained and require no installer, runtime package, external DLL, font, configuration file, or asset beside the EXE itself.

## Design Principles

BABA Clock follows a few strict rules:

1. **One executable** — everything required at runtime belongs inside the EXE.
2. **Native Windows** — no heavyweight cross-platform framework.
3. **Backward compatible** — keep Windows XP compatibility wherever technically possible.
4. **Low overhead** — avoid unnecessary polling, allocations, background activity, and dependencies.
5. **Simple UI** — the clock should stay out of the user's way.

## Open Source

BABA Clock is open source under the **Apache License 2.0**.

See [LICENSE](LICENSE) for the license terms and [NOTICE](NOTICE) for the original project attribution.

When redistributing this project or derivative works, preserve the required license and attribution notices in accordance with the Apache License 2.0.

## Attribution

**Original author:** Mohammad Taghi Alavi  
**Idea:** Abbas Alavi  
**Original project:** https://github.com/AAA-It-uae/DADClock

---

<div align="center">

**BABA Clock**  
Built by **Mohammad Taghi Alavi**  
Idea by **Abbas Alavi**  
Made with love ♥

</div>
