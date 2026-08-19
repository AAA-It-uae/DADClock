# SPDX-License-Identifier: Apache-2.0
# Copyright 2026 Mohammad Taghi Alavi

"""Validate that DADClock.exe matches the project's portability and security contract."""

from pathlib import Path
import sys
import pefile

# The DLL + function allowlist freezes the reviewed Win32 import surface.
# Any new imported API must be deliberately reviewed before it can enter the build.
ALLOWED_IMPORT_FUNCTIONS = {
    "kernel32.dll": {
        "ExitProcess",
        "GetLocalTime",
        "GetModuleFileNameW",
        "GetModuleHandleW",
    },
    "user32.dll": {
        "AppendMenuW",
        "BeginPaint",
        "ClientToScreen",
        "CreatePopupMenu",
        "CreateWindowExW",
        "DefWindowProcW",
        "DestroyMenu",
        "DestroyWindow",
        "DispatchMessageW",
        "EnableWindow",
        "EndPaint",
        "FillRect",
        "GetClientRect",
        "GetDC",
        "GetDlgItem",
        "GetMessageW",
        "GetMonitorInfoW",
        "GetSysColorBrush",
        "GetSystemMetrics",
        "GetWindowLongW",
        "GetWindowRect",
        "InvalidateRect",
        "KillTimer",
        "LoadCursorW",
        "LoadIconW",
        "MessageBoxIndirectW",
        "MessageBoxW",
        "MonitorFromRect",
        "MonitorFromWindow",
        "PostMessageW",
        "PostQuitMessage",
        "RedrawWindow",
        "RegisterClassExW",
        "ReleaseCapture",
        "ReleaseDC",
        "SendMessageW",
        "SetCursor",
        "SetFocus",
        "SetForegroundWindow",
        "SetLayeredWindowAttributes",
        "SetTimer",
        "SetWindowLongW",
        "SetWindowPos",
        "ShowWindow",
        "TrackPopupMenu",
        "TranslateMessage",
        "UpdateWindow",
    },
    "gdi32.dll": {
        "CreateFontW",
        "CreatePen",
        "CreateSolidBrush",
        "DeleteObject",
        "Ellipse",
        "EnumFontFamiliesExW",
        "GetStockObject",
        "GetTextExtentPoint32W",
        "Polygon",
        "SelectObject",
        "SetBkMode",
        "SetTextColor",
        "TextOutW",
    },
    "advapi32.dll": {
        "RegCloseKey",
        "RegCreateKeyExA",
        "RegCreateKeyExW",
        "RegDeleteValueW",
        "RegOpenKeyExA",
        "RegOpenKeyExW",
        "RegQueryValueExA",
        "RegQueryValueExW",
        "RegSetValueExA",
        "RegSetValueExW",
    },
    "shell32.dll": {
        "ShellExecuteW",
    },
}

FORBIDDEN_FRAGMENTS = ("msvcp", "vcruntime", "ucrtbase", "msvcr")
REQUIRED_DLL_CHARACTERISTICS = 0x0040 | 0x0100  # DYNAMIC_BASE | NX_COMPAT
EXPECTED_FILE_VERSION = (1, 0, 0, 0)


def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def version_tuple(ms: int, ls: int) -> tuple[int, int, int, int]:
    return (ms >> 16, ms & 0xFFFF, ls >> 16, ls & 0xFFFF)


def main() -> None:
    path = Path(sys.argv[1] if len(sys.argv) > 1 else "DADClock.exe")
    if not path.is_file():
        fail(f"missing {path}")

    pe = pefile.PE(str(path), fast_load=False)

    if pe.FILE_HEADER.Machine != 0x014C:
        fail(f"expected x86 machine 0x014C, got 0x{pe.FILE_HEADER.Machine:04X}")
    if pe.OPTIONAL_HEADER.Magic != 0x010B:
        fail(f"expected PE32 optional header, got 0x{pe.OPTIONAL_HEADER.Magic:04X}")
    if pe.OPTIONAL_HEADER.Subsystem != 2:
        fail(f"expected Windows GUI subsystem (2), got {pe.OPTIONAL_HEADER.Subsystem}")
    if (pe.OPTIONAL_HEADER.MajorSubsystemVersion, pe.OPTIONAL_HEADER.MinorSubsystemVersion) != (5, 1):
        fail(
            "expected subsystem version 5.01, got "
            f"{pe.OPTIONAL_HEADER.MajorSubsystemVersion}."
            f"{pe.OPTIONAL_HEADER.MinorSubsystemVersion:02d}"
        )

    dll_characteristics = pe.OPTIONAL_HEADER.DllCharacteristics
    if (dll_characteristics & REQUIRED_DLL_CHARACTERISTICS) != REQUIRED_DLL_CHARACTERISTICS:
        fail(
            "expected DYNAMIC_BASE and NX_COMPAT, got DllCharacteristics "
            f"0x{dll_characteristics:04X}"
        )

    imported_functions: dict[str, set[str]] = {}
    if hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            dll = entry.dll.decode("ascii", errors="replace").lower()
            functions: set[str] = set()
            for imported in entry.imports:
                if imported.name is None:
                    fail(f"ordinal import is not allowed: {dll}!#{imported.ordinal}")
                functions.add(imported.name.decode("ascii", errors="replace"))
            imported_functions[dll] = functions

    actual_dlls = set(imported_functions)
    allowed_dlls = set(ALLOWED_IMPORT_FUNCTIONS)
    unexpected_dlls = actual_dlls - allowed_dlls
    if unexpected_dlls:
        fail(f"unexpected imported DLLs: {', '.join(sorted(unexpected_dlls))}")

    for dll in actual_dlls:
        if any(fragment in dll for fragment in FORBIDDEN_FRAGMENTS):
            fail(f"runtime dependency detected: {dll}")
        unexpected_functions = imported_functions[dll] - ALLOWED_IMPORT_FUNCTIONS[dll]
        if unexpected_functions:
            fail(
                f"unreviewed imported APIs in {dll}: "
                + ", ".join(sorted(unexpected_functions))
            )

    resource_types = set()
    if hasattr(pe, "DIRECTORY_ENTRY_RESOURCE"):
        for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if entry.id is not None:
                resource_types.add(entry.id)
    if 3 not in resource_types or 14 not in resource_types:
        fail("embedded icon resources are missing")
    if 16 not in resource_types:
        fail("VERSIONINFO resource is missing")

    if not getattr(pe, "VS_FIXEDFILEINFO", None):
        fail("VERSIONINFO fixed-file data is missing")
    fixed = pe.VS_FIXEDFILEINFO[0]
    file_version = version_tuple(fixed.FileVersionMS, fixed.FileVersionLS)
    if file_version != EXPECTED_FILE_VERSION:
        fail(f"expected file version {EXPECTED_FILE_VERSION}, got {file_version}")

    if path.stat().st_size > 1024 * 1024:
        fail(f"EXE unexpectedly large: {path.stat().st_size} bytes")

    total_apis = sum(len(functions) for functions in imported_functions.values())
    print(f"OK: {path.name}")
    print(f"Size: {path.stat().st_size} bytes")
    print("Machine: x86 / PE32")
    print("Subsystem: Windows GUI 5.01")
    print(f"File version: {'.'.join(map(str, file_version))}")
    print(f"DllCharacteristics: 0x{dll_characteristics:04X} (DYNAMIC_BASE + NX_COMPAT verified)")
    print("Imports: " + ", ".join(sorted(actual_dlls)))
    print(f"Reviewed imported API functions: {total_apis}")
    print("Embedded icon: present")
    print("VERSIONINFO: present")


if __name__ == "__main__":
    main()
