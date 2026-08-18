# SPDX-License-Identifier: Apache-2.0
# Copyright 2026 Mohammad Taghi Alavi

"""Convert the supplied BABA Clock artwork to a compact XP-compatible ICO.

Usage:
    python make_icon.py path\\to\\source.png [path\\to\\BABAClock.ico]

The application does not load the PNG at runtime. Only the generated ICO is
embedded by the Windows resource compiler.
"""

import struct
import sys
import zlib


PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def read_png(path):
    data = open(path, "rb").read()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("the input is not a PNG file")

    width = height = bit_depth = color_type = interlace = None
    compressed = bytearray()
    offset = len(PNG_SIGNATURE)
    while offset < len(data):
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        kind = data[offset + 4:offset + 8]
        chunk = data[offset + 8:offset + 8 + length]
        offset += 12 + length
        if kind == b"IHDR":
            width, height, bit_depth, color_type, _, _, interlace = struct.unpack(
                ">IIBBBBB", chunk
            )
        elif kind == b"IDAT":
            compressed += chunk
        elif kind == b"IEND":
            break

    if (width is None or bit_depth != 8 or color_type not in (2, 6) or
            interlace != 0):
        raise ValueError("only non-interlaced 8-bit RGB/RGBA PNG files are supported")

    channels = 3 if color_type == 2 else 4
    stride = width * channels
    raw = zlib.decompress(bytes(compressed))
    expected = height * (stride + 1)
    if len(raw) != expected:
        raise ValueError("the PNG scanline data is incomplete")

    pixels = []
    previous = bytearray(stride)
    cursor = 0
    for _ in range(height):
        filter_type = raw[cursor]
        cursor += 1
        scanline = bytearray(raw[cursor:cursor + stride])
        cursor += stride
        for index in range(stride):
            left = scanline[index - channels] if index >= channels else 0
            above = previous[index]
            upper_left = previous[index - channels] if index >= channels else 0
            if filter_type == 1:
                scanline[index] = (scanline[index] + left) & 255
            elif filter_type == 2:
                scanline[index] = (scanline[index] + above) & 255
            elif filter_type == 3:
                scanline[index] = (scanline[index] + ((left + above) // 2)) & 255
            elif filter_type == 4:
                estimate = left + above - upper_left
                distance_left = abs(estimate - left)
                distance_above = abs(estimate - above)
                distance_upper_left = abs(estimate - upper_left)
                predictor = left
                if distance_above < distance_left and distance_above <= distance_upper_left:
                    predictor = above
                elif distance_upper_left < distance_left and distance_upper_left < distance_above:
                    predictor = upper_left
                scanline[index] = (scanline[index] + predictor) & 255
            elif filter_type != 0:
                raise ValueError("unsupported PNG filter")
        for index in range(0, stride, channels):
            alpha = scanline[index + 3] if channels == 4 else 255
            pixels.append((scanline[index], scanline[index + 1],
                           scanline[index + 2], alpha))
        previous = scanline
    return width, height, pixels


def resize(pixels, source_width, source_height, size):
    result = []
    for y in range(size):
        top = y * source_height // size
        bottom = max(top + 1, (y + 1) * source_height // size)
        for x in range(size):
            left = x * source_width // size
            right = max(left + 1, (x + 1) * source_width // size)
            red = green = blue = alpha = count = 0
            for source_y in range(top, bottom):
                row = source_y * source_width
                for source_x in range(left, right):
                    pixel = pixels[row + source_x]
                    red += pixel[0]
                    green += pixel[1]
                    blue += pixel[2]
                    alpha += pixel[3]
                    count += 1
            result.append((red // count, green // count, blue // count,
                           alpha // count))
    return result


def dib_icon(pixels, size):
    data = bytearray()
    data += struct.pack("<IiiHHIIiiII", 40, size, size * 2, 1, 32, 0,
                        size * size * 4, 0, 0, 0, 0)
    for y in range(size - 1, -1, -1):
        for red, green, blue, alpha in pixels[y * size:(y + 1) * size]:
            data += bytes((blue, green, red, alpha))
    mask_row = ((size + 31) // 32) * 4
    data += bytes(mask_row * size)
    return bytes(data)


def png_icon(pixels, size):
    def chunk(kind, value):
        return (struct.pack(">I", len(value)) + kind + value +
                struct.pack(">I", zlib.crc32(kind + value) & 0xFFFFFFFF))

    raw = bytearray()
    for y in range(size):
        raw.append(0)
        for red, green, blue, alpha in pixels[y * size:(y + 1) * size]:
            raw += bytes((red, green, blue, alpha))
    header = struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0)
    return (PNG_SIGNATURE + chunk(b"IHDR", header) +
            chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b""))


def write_ico(source_path, output_path):
    source_width, source_height, source_pixels = read_png(source_path)
    sizes = (16, 32, 48, 256)
    images = []
    for size in sizes:
        pixels = resize(source_pixels, source_width, source_height, size)
        images.append((size, png_icon(pixels, size) if size == 256
                       else dib_icon(pixels, size)))

    offset = 6 + len(images) * 16
    directory = bytearray(struct.pack("<HHH", 0, 1, len(images)))
    body = bytearray()
    for size, image in images:
        dimension = 0 if size == 256 else size
        directory += struct.pack("<BBBBHHII", dimension, dimension, 0, 0,
                                 1, 32, len(image), offset)
        body += image
        offset += len(image)
    with open(output_path, "wb") as icon:
        icon.write(directory + body)


def main():
    if len(sys.argv) < 2:
        raise SystemExit("usage: python make_icon.py source.png [BABAClock.ico]")
    source_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else "BABAClock.ico"
    write_ico(source_path, output_path)


if __name__ == "__main__":
    main()
