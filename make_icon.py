# SPDX-License-Identifier: Apache-2.0
# Copyright 2026 Mohammad Taghi Alavi

"""Create the small embedded BABA Clock ICO without third-party packages."""

import struct
import zlib


GREEN = (0, 255, 80, 255)
GREEN_DARK = (0, 100, 35, 255)
BLACK = (4, 7, 6, 255)
EDGE = (0, 35, 18, 255)


def rect(pixels, size, left, top, right, bottom, color):
    left = max(0, int(left))
    top = max(0, int(top))
    right = min(size, int(right))
    bottom = min(size, int(bottom))
    for y in range(top, bottom):
        row = y * size
        for x in range(left, right):
            pixels[row + x] = color


def digital_icon(size):
    pixels = [BLACK] * (size * size)
    border = max(1, size // 16)
    rect(pixels, size, border, border, size - border, size - border, EDGE)
    rect(pixels, size, border * 2, border * 2, size - border * 2, size - border * 2, BLACK)

    frame = max(1, size // 12)
    rect(pixels, size, size // 7, size // 5, size - size // 7, size // 5 + frame, GREEN_DARK)
    rect(pixels, size, size // 7, size // 5, size - size // 7, size // 5 + max(1, frame // 2), GREEN)

    digit_w = max(3, size // 6)
    digit_h = max(6, size // 2)
    thick = max(1, size // 14)
    gap = max(1, size // 32)
    start_x = (size - (digit_w * 3 + gap * 2)) // 2
    start_y = size // 3

    def seg(x, y, which):
        if which in (0, 3, 6):
            yy = y + (0 if which == 0 else digit_h // 2 if which == 6 else digit_h - thick)
            rect(pixels, size, x, yy, x + digit_w, yy + thick, GREEN)
        else:
            xx = x if which in (1, 2) else x + digit_w - thick
            yy = y + (0 if which in (1, 5) else digit_h // 2)
            rect(pixels, size, xx, yy, xx + thick, yy + digit_h // 2 + thick, GREEN)

    for digit_x, mask in ((start_x, 0x3F), (start_x + digit_w + gap, 0x06)):
        for which in range(7):
            if mask & (1 << which):
                seg(digit_x, start_y, which)

    colon_x = start_x + digit_w * 2 + gap * 2
    dot = max(1, size // 12)
    rect(pixels, size, colon_x, start_y + digit_h // 3, colon_x + dot, start_y + digit_h // 3 + dot, GREEN)
    rect(pixels, size, colon_x, start_y + digit_h * 2 // 3, colon_x + dot, start_y + digit_h * 2 // 3 + dot, GREEN)
    return pixels


def dib_icon(size):
    pixels = digital_icon(size)
    data = bytearray()
    data += struct.pack("<IiiHHIIiiII", 40, size, size * 2, 1, 32, 0,
                        size * size * 4, 0, 0, 0, 0)
    for y in range(size - 1, -1, -1):
        for red, green, blue, alpha in pixels[y * size:(y + 1) * size]:
            data += bytes((blue, green, red, alpha))
    mask_row = ((size + 31) // 32) * 4
    data += bytes(mask_row * size)
    return bytes(data)


def png_icon(size):
    pixels = digital_icon(size)

    palette = [BLACK, EDGE, GREEN_DARK, GREEN]
    indexes = {color: index for index, color in enumerate(palette)}

    def chunk(kind, data):
        return (struct.pack(">I", len(data)) + kind + data +
                struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF))

    raw = bytearray()
    for y in range(size):
        raw.append(0)
        for color in pixels[y * size:(y + 1) * size]:
            raw.append(indexes[color])
    return (b"\x89PNG\r\n\x1a\n" +
            chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 3, 0, 0, 0)) +
            chunk(b"PLTE", b"".join(bytes(color[:3]) for color in palette)) +
            chunk(b"IDAT", zlib.compress(bytes(raw), 9)) +
            chunk(b"IEND", b""))


def main():
    sizes = (16, 32, 48, 256)
    images = [(size, png_icon(size) if size == 256 else dib_icon(size)) for size in sizes]
    offset = 6 + len(images) * 16
    directory = bytearray(struct.pack("<HHH", 0, 1, len(images)))
    body = bytearray()
    for size, image in images:
        dimension = 0 if size == 256 else size
        directory += struct.pack("<BBBBHHII", dimension, dimension, 0, 0, 1, 32,
                                 len(image), offset)
        body += image
        offset += len(image)
    with open("BABAClock.ico", "wb") as icon:
        icon.write(directory + body)


if __name__ == "__main__":
    main()
