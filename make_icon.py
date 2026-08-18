# SPDX-License-Identifier: Apache-2.0
# Copyright 2026 Mohammad Taghi Alavi

"""Regenerate the Windows ICO from the canonical BABA Clock 11:11 artwork."""

from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "assets" / "BABAClock-11-11.png"
TARGET = ROOT / "BABAClock.ico"
SIZES = [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]


def main() -> None:
    image = Image.open(SOURCE).convert("RGBA")
    image.save(TARGET, format="ICO", sizes=SIZES)
    print(f"Generated {TARGET.name} from {SOURCE.as_posix()}")


if __name__ == "__main__":
    main()
