# SPDX-License-Identifier: Apache-2.0
# Copyright 2026 Mohammad Taghi Alavi

"""Validate that BABAClock.exe matches the project's portability contract."""

from pathlib import Path
import sys
import pefile

ALLOWED_IMPORTS = {
    "kernel32.dll",
    "user32.dll",
    "gdi32.dll",
    "advapi32.dll",
    "shell32.dll",
}
FORBIDDEN_FRAGMENTS = ("msvcp", "vcruntime", "ucrtbase", "msvcr")


def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def main() -> None:
    path = Path(sys.argv[1] if len(sys.argv) > 1 else "BABAClock.exe")
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

    imports = set()
    if hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            imports.add(entry.dll.decode("ascii", errors="replace").lower())

    unexpected = imports - ALLOWED_IMPORTS
    if unexpected:
        fail(f"unexpected imported DLLs: {', '.join(sorted(unexpected))}")
    for dll in imports:
        if any(fragment in dll for fragment in FORBIDDEN_FRAGMENTS):
            fail(f"runtime dependency detected: {dll}")

    resource_types = set()
    if hasattr(pe, "DIRECTORY_ENTRY_RESOURCE"):
        for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if entry.id is not None:
                resource_types.add(entry.id)
    if 3 not in resource_types or 14 not in resource_types:
        fail("embedded icon resources are missing")

    if path.stat().st_size > 1024 * 1024:
        fail(f"EXE unexpectedly large: {path.stat().st_size} bytes")

    print(f"OK: {path.name}")
    print(f"Size: {path.stat().st_size} bytes")
    print("Machine: x86 / PE32")
    print("Subsystem: Windows GUI 5.01")
    print("Imports: " + ", ".join(sorted(imports)))
    print("Embedded icon: present")


if __name__ == "__main__":
    main()
