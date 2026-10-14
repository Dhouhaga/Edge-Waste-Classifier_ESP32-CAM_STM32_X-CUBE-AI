import os
import time
from datetime import datetime

import serial
from PIL import Image

PORT = "COM6"
BAUD = 115200
W, H = 96, 96
SCALE = 4                # saved image is upscaled for easier viewing
BYTE_SWAP = False        # True only if colors are still wrong after the STM32 fix
JPEG_QUALITY = 95
OUT_DIR = "captures"

MAGIC = b"IMG0"
HDR_LEN = 12             # magic(4) + width(2) + height(2) + length(4)
CS_LEN = 4
FRAME_BYTES = W * H * 2


def rgb565_to_image(raw):
    pixels = []
    for i in range(W * H):
        if BYTE_SWAP:
            p = (raw[2 * i + 1] << 8) | raw[2 * i]
        else:
            p = (raw[2 * i] << 8) | raw[2 * i + 1]     # big-endian, as the ESP sends it
        r = ((p >> 11) & 0x1F) * 255 // 31
        g = ((p >> 5) & 0x3F) * 255 // 63
        b = (p & 0x1F) * 255 // 31
        pixels.append((r, g, b))
    img = Image.new("RGB", (W, H))
    img.putdata(pixels)
    return img


def save_frame(raw):
    os.makedirs(OUT_DIR, exist_ok=True)
    img = rgb565_to_image(raw).resize((W * SCALE, H * SCALE), Image.NEAREST)
    name = datetime.now().strftime("frame_%Y%m%d_%H%M%S.jpg")
    path = os.path.join(OUT_DIR, name)
    img.save(path, "JPEG", quality=JPEG_QUALITY)
    return path


def handle_frame(buf):
    """Parse one frame at buf[0:]. Returns bytes consumed, 0 if more data is needed."""
    if len(buf) < HDR_LEN:
        return 0
    w = buf[4] | (buf[5] << 8)
    h = buf[6] | (buf[7] << 8)
    length = buf[8] | (buf[9] << 8) | (buf[10] << 16) | (buf[11] << 24)

    if w != W or h != H or length != FRAME_BYTES:
        print(f"[warn] bad frame header: {w}x{h}, {length} bytes - skipping")
        return 4

    total = HDR_LEN + length + CS_LEN
    if len(buf) < total:
        return 0

    raw = bytes(buf[HDR_LEN:HDR_LEN + length])
    rx_sum = int.from_bytes(buf[HDR_LEN + length:total], "little")
    calc_sum = sum(raw) & 0xFFFFFFFF

    if rx_sum != calc_sum:
        print(f"[warn] checksum mismatch (rx {rx_sum}, calc {calc_sum}) - frame dropped")
    else:
        path = save_frame(raw)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] frame saved: {path}")
    return total


def main():
    ser = serial.Serial(PORT, BAUD, timeout=0.05)
    print(f"Listening on {PORT} @ {BAUD} baud. Ctrl+C to stop.")

    buf = bytearray()
    try:
        while True:
            chunk = ser.read(max(1, ser.in_waiting))
            if chunk:
                buf += chunk

            while buf:
                i_img = buf.find(MAGIC)
                i_nl = buf.find(b"\n")

                if i_img < 0 and i_nl < 0:
                    if len(buf) > 3:
                        del buf[:-3]
                    break

                if i_nl >= 0 and (i_img < 0 or i_nl < i_img):
                    line = bytes(buf[:i_nl]).decode(errors="replace").strip()
                    del buf[:i_nl + 1]
                    if line:
                        print(f"[STM32] {line}")
                    continue

                if i_img > 0:
                    del buf[:i_img]
                used = handle_frame(buf)
                if used == 0:
                    break
                del buf[:used]

            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
