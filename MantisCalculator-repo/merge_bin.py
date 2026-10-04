#!/usr/bin/env python3
"""Merge PlatformIO ESP32 build artifacts into one flashable image.
Used only by GitHub Actions CI — not a local-dev tool.
"""
import argparse, os, subprocess, sys

p = argparse.ArgumentParser()
p.add_argument("firmware_dir", help="PlatformIO build dir, e.g. .pio/build/m5stack-core2")
p.add_argument("-o", "--output", default="MantisCalculator.bin")
a = p.parse_args()
base = a.firmware_dir

parts = [
    ("bootloader.bin", 0x1000),
    ("partitions.bin", 0x8000),
    ("boot_app0.bin", 0xE000),
    ("firmware.bin", 0x10000),
]
args = [
    sys.executable, "-m", "esptool",
    "--chip", "esp32", "merge_bin",
    "-o", a.output,
    "--flash_mode", "qio",
    "--flash_freq", "80m",
    "--flash_size", "16MB",
]
for fn, off in parts:
    path = os.path.join(base, fn)
    if not os.path.exists(path):
        raise SystemExit(f"Missing {path}")
    args += [hex(off), path]
print(" ".join(args), flush=True)
subprocess.check_call(args)
print(f"Wrote {a.output} ({os.path.getsize(a.output)} bytes)", flush=True)
