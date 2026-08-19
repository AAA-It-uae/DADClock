# SPDX-License-Identifier: Apache-2.0
# Copyright 2026 Mohammad Taghi Alavi

"""Generate DAD Clock PNG and multi-size ICO from a reproducible 11:11 design."""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent
PNG_TARGET = ROOT / "assets" / "DADClock-11-11.png"
ICO_TARGET = ROOT / "DADClock.ico"
CANVAS = 512
GREEN = (132, 255, 53, 255)
GREEN_SOFT = (103, 225, 49, 255)


def font(size: int, bold: bool = False):
    candidates = [
        Path("C:/Windows/Fonts/arialbd.ttf" if bold else "C:/Windows/Fonts/arial.ttf"),
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size=size)
    return ImageFont.load_default()


def draw_digit_one(draw: ImageDraw.ImageDraw, x: int) -> None:
    draw.rounded_rectangle((x, 182, x + 15, 252), radius=7, fill=GREEN)
    draw.rounded_rectangle((x, 260, x + 15, 330), radius=7, fill=GREEN)


def build_image() -> Image.Image:
    image = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 255))
    draw = ImageDraw.Draw(image)

    draw.rounded_rectangle((10, 10, 502, 502), radius=92, fill=(18, 21, 20, 255), outline=(82, 86, 84, 255), width=3)
    draw.ellipse((49, 49, 463, 463), fill=(2, 6, 4, 255), outline=(13, 53, 26, 255), width=5)

    glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_draw.ellipse((58, 58, 454, 454), outline=(108, 255, 40, 210), width=12)
    glow_draw.ellipse((63, 63, 449, 449), outline=(132, 255, 53, 150), width=6)
    glow = glow.filter(ImageFilter.GaussianBlur(9))
    image = Image.alpha_composite(image, glow)
    draw = ImageDraw.Draw(image)
    draw.ellipse((58, 58, 454, 454), outline=GREEN, width=7)

    digit_glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    gd = ImageDraw.Draw(digit_glow)
    for x in (128, 205, 292, 369):
        gd.rounded_rectangle((x, 182, x + 15, 252), radius=7, fill=(120, 255, 45, 220))
        gd.rounded_rectangle((x, 260, x + 15, 330), radius=7, fill=(120, 255, 45, 220))
    gd.ellipse((248, 214, 264, 230), fill=(120, 255, 45, 220))
    gd.ellipse((248, 282, 264, 298), fill=(120, 255, 45, 220))
    digit_glow = digit_glow.filter(ImageFilter.GaussianBlur(8))
    image = Image.alpha_composite(image, digit_glow)
    draw = ImageDraw.Draw(image)

    for x in (128, 205, 292, 369):
        draw_digit_one(draw, x)
    draw.ellipse((248, 214, 264, 230), fill=GREEN)
    draw.ellipse((248, 282, 264, 298), fill=GREEN)

    dad_font = font(34, bold=True)
    clock_font = font(15, bold=False)
    dad = "D A D"
    clock = "C L O C K"
    bbox = draw.textbbox((0, 0), dad, font=dad_font)
    draw.text(((CANVAS - (bbox[2] - bbox[0])) / 2, 365), dad, font=dad_font, fill=GREEN)
    bbox = draw.textbbox((0, 0), clock, font=clock_font)
    draw.text(((CANVAS - (bbox[2] - bbox[0])) / 2, 408), clock, font=clock_font, fill=GREEN_SOFT)

    return image


def main() -> None:
    image = build_image()
    PNG_TARGET.parent.mkdir(parents=True, exist_ok=True)
    preview = image.resize((256, 256), Image.Resampling.LANCZOS)
    preview.save(PNG_TARGET, format="PNG", optimize=True)
    preview.save(
        ICO_TARGET,
        format="ICO",
        sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)],
    )
    print(f"Generated {PNG_TARGET.relative_to(ROOT)}")
    print(f"Generated {ICO_TARGET.name}")


if __name__ == "__main__":
    main()
