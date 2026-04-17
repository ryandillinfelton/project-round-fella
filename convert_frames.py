from PIL import Image
import numpy as np
import os
import sys

# Converts PNG frames to raw RGB565 binary files for use with LittleFS.
# Usage: python3 convert_frames.py [data_dir]
# Defaults to ./sketch_apr16b/data if no argument given.

data_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "sketch_apr16b", "data")

for filename in os.listdir(data_dir):
    if not filename.lower().endswith(".png"):
        continue
    name = os.path.splitext(filename)[0]
    png_path = os.path.join(data_dir, filename)
    bin_path = os.path.join(data_dir, f"{name}.bin")

    img = Image.open(png_path).convert("RGBA")
    bg = Image.new("RGBA", img.size, (0, 0, 0, 255))
    bg.paste(img, mask=img.split()[3])
    img = bg.convert("RGB")

    arr = np.array(img, dtype=np.uint16)
    r = (arr[:, :, 0] >> 3).astype(np.uint16)
    g = (arr[:, :, 1] >> 2).astype(np.uint16)
    b = (arr[:, :, 2] >> 3).astype(np.uint16)
    rgb565 = (r << 11) | (g << 5) | b
    rgb565.astype("<u2").tofile(bin_path)

    print(f"{filename} -> {name}.bin ({os.path.getsize(bin_path):,} bytes)")
