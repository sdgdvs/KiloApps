#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# ///
"""
KiloApps Fleet Icon Uniqueness Auditor & Procedural Generator
Audits KiloOS/src/App.jsx and KiloOS/public/assets/icons/ to ensure
every application has a unique, visually distinctive 32x32 .ico file.

Checks:
1. Missing icon files referenced in App.jsx.
2. Multiple apps sharing the same icon path.
3. Multiple icon files sharing the same SHA256 hash (copied icons).

Usage:
  python scripts/check_icons.py           # Audit and report status
  python scripts/check_icons.py --fix     # Audit and procedurally generate unique icons for duplicates
"""

import argparse
import hashlib
import os
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
APP_JSX_PATH = REPO_ROOT / "KiloOS" / "src" / "App.jsx"
ICON_DIR = REPO_ROOT / "KiloOS" / "public" / "assets" / "icons"


# ---------------------------------------------------------------------------
# Pure Python 32x32 ICO Writer (No external dependencies)
# ---------------------------------------------------------------------------
def save_ico(filename: Path, pixels: list[list[tuple[int, int, int, int]]]):
    """Saves a 32x32 RGBA pixel grid to a valid Windows .ico file."""
    width = 32
    height = 32
    bpp = 24  # 24-bit color BMP with 1-bit transparency mask

    bmp_header = struct.pack(
        "<IiiHHIIIIII",
        40,          # biSize
        width,       # biWidth
        height * 2,  # biHeight (doubled for ICO: image + mask)
        1,           # biPlanes
        bpp,         # biBitCount
        0,           # biCompression (BI_RGB)
        0,           # biSizeImage
        0, 0, 0, 0   # resolution & color palette
    )

    row_size = ((width * bpp + 31) // 32) * 4
    img_data = b""
    # BMP scanlines are bottom-up
    for y in range(height - 1, -1, -1):
        row = b""
        for x in range(width):
            color = pixels[y][x]
            if len(color) == 4:
                r, g, b, a = color
            else:
                r, g, b = color
            row += struct.pack("<BBB", b, g, r)  # BGR
        row += b"\x00" * (row_size - len(row))
        img_data += row

    mask_row_size = ((width + 31) // 32) * 4
    mask_data = b""
    for y in range(height - 1, -1, -1):
        row_bits = 0
        for x in range(width):
            color = pixels[y][x]
            # 1 = transparent, 0 = opaque
            if len(color) == 4 and color[3] == 0:
                row_bits |= 1 << (7 - (x % 8))
            elif color == (0, 0, 0, 0):
                row_bits |= 1 << (7 - (x % 8))
            if (x % 8) == 7 or x == width - 1:
                mask_data += struct.pack("B", row_bits)
                row_bits = 0
        mask_data += b"\x00" * (mask_row_size - len(mask_data))

    full_data = bmp_header + img_data + mask_data
    ico_header = struct.pack("<HHH", 0, 1, 1)  # reserved, type (1=ico), count (1)
    direntry = struct.pack(
        "<BBBBHHII",
        width, height, 0, 0, 1, bpp, len(full_data), 22  # 6 (header) + 16 (entry)
    )

    filename.parent.mkdir(parents=True, exist_ok=True)
    with open(filename, "wb") as f:
        f.write(ico_header + direntry + full_data)


def new_canvas(bg=(0, 0, 0, 0)) -> list[list[tuple]]:
    return [[bg for _ in range(32)] for _ in range(32)]


def draw_rect(img, x1, y1, x2, y2, color):
    for y in range(max(0, y1), min(32, y2 + 1)):
        for x in range(max(0, x1), min(32, x2 + 1)):
            img[y][x] = color


def draw_frame(img, x1, y1, x2, y2, color, thickness=1):
    for t in range(thickness):
        for x in range(x1 + t, x2 - t + 1):
            if 0 <= y1 + t < 32 and 0 <= x < 32:
                img[y1 + t][x] = color
            if 0 <= y2 - t < 32 and 0 <= x < 32:
                img[y2 - t][x] = color
        for y in range(y1 + t, y2 - t + 1):
            if 0 <= y < 32 and 0 <= x1 + t < 32:
                img[y][x1 + t] = color
            if 0 <= y < 32 and 0 <= x2 - t < 32:
                img[y][x2 - t] = color


def draw_line(img, x0, y0, x1, y1, color):
    dx = abs(x1 - x0)
    dy = abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx - dy
    while True:
        if 0 <= x0 < 32 and 0 <= y0 < 32:
            img[y0][x0] = color
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x0 += sx
        if e2 < dx:
            err += dx
            y0 += sy


def draw_circle(img, cx, cy, r, color, fill=True):
    for y in range(32):
        for x in range(32):
            d2 = (x - cx) ** 2 + (y - cy) ** 2
            if fill:
                if d2 <= r ** 2:
                    img[y][x] = color
            else:
                if (r - 1) ** 2 <= d2 <= r ** 2:
                    img[y][x] = color


# ---------------------------------------------------------------------------
# Distinctive Procedural Icon Designers
# ---------------------------------------------------------------------------
def generate_distinctive_icon(app_id: str, out_path: Path):
    """Generates an authentic, period-accurate 1999 32x32 icon tailored to the app."""
    img = new_canvas()

    DARK_BLUE = (15, 23, 42, 255)
    CYAN = (6, 182, 212, 255)
    LIGHT_CYAN = (165, 243, 252, 255)
    GOLD = (234, 179, 8, 255)
    LIGHT_GOLD = (254, 240, 138, 255)
    DARK_GOLD = (161, 98, 7, 255)
    GREEN = (34, 197, 94, 255)
    LIGHT_GREEN = (187, 247, 208, 255)
    DARK_GREEN = (21, 128, 61, 255)
    PURPLE = (168, 85, 247, 255)
    LIGHT_PURPLE = (233, 213, 255, 255)
    DARK_PURPLE = (107, 33, 168, 255)
    RED = (239, 68, 68, 255)
    ORANGE = (249, 115, 22, 255)
    WHITE = (248, 250, 252, 255)
    GRAY = (148, 163, 184, 255)
    DARK_GRAY = (51, 65, 85, 255)

    if app_id == "knetmap":
        # Network subnet topology graph: 3 connected nodes with router ring
        draw_rect(img, 2, 2, 29, 29, (10, 15, 30, 255))
        draw_frame(img, 2, 2, 29, 29, CYAN, 1)
        # Center router node
        draw_circle(img, 15, 12, 4, CYAN, fill=True)
        draw_circle(img, 15, 12, 2, WHITE, fill=True)
        # Peripheral host nodes
        draw_circle(img, 7, 23, 3, GREEN, fill=True)
        draw_circle(img, 23, 23, 3, GOLD, fill=True)
        # Topology interconnect lines
        draw_line(img, 15, 15, 7, 21, LIGHT_CYAN)
        draw_line(img, 15, 15, 23, 21, LIGHT_GOLD)
        draw_line(img, 7, 23, 23, 23, GRAY)

    elif app_id == "kcipher":
        # Cryptographic cipher disc / key lock matrix
        draw_rect(img, 3, 3, 28, 28, (20, 20, 25, 255))
        draw_frame(img, 3, 3, 28, 28, GOLD, 1)
        # Outer cipher ring
        draw_circle(img, 15, 15, 9, DARK_GOLD, fill=False)
        draw_circle(img, 15, 15, 5, GOLD, fill=True)
        draw_circle(img, 15, 15, 2, (20, 20, 25, 255), fill=True)
        # Keyhole / Rotor notches
        draw_rect(img, 14, 15, 16, 21, GOLD)
        draw_rect(img, 7, 14, 9, 16, CYAN)
        draw_rect(img, 21, 14, 23, 16, CYAN)

    elif app_id == "ksteno":
        # Stenography: Pixel carrier envelope with hidden binary bit eye
        draw_rect(img, 3, 5, 28, 26, (15, 23, 42, 255))
        draw_frame(img, 3, 5, 28, 26, PURPLE, 1)
        # Envelope flap lines
        draw_line(img, 3, 5, 15, 16, LIGHT_PURPLE)
        draw_line(img, 28, 5, 16, 16, LIGHT_PURPLE)
        # Hidden pixel grid (LSB dots)
        for y in range(18, 24, 2):
            for x in range(6, 26, 3):
                img[y][x] = CYAN if (x + y) % 5 == 0 else DARK_GRAY
        # Covert eye in center
        draw_circle(img, 15, 16, 3, GOLD, fill=True)
        img[16][15] = (15, 23, 42, 255)

    elif app_id == "kanomaly":
        # Subterranean radar wave with glitch anomaly pulse
        draw_rect(img, 2, 2, 29, 29, (5, 5, 10, 255))
        draw_frame(img, 2, 2, 29, 29, (30, 41, 59, 255), 1)
        # Concentric sonar pulses
        draw_circle(img, 15, 15, 11, (16, 185, 129, 255), fill=False)
        draw_circle(img, 15, 15, 7, (52, 211, 153, 255), fill=False)
        draw_circle(img, 15, 15, 3, (110, 231, 183, 255), fill=True)
        # Red anomaly crosshair spike
        draw_line(img, 15, 4, 15, 26, RED)
        draw_line(img, 4, 15, 26, 15, RED)
        draw_circle(img, 19, 11, 2, (244, 63, 94, 255), fill=True)

    elif app_id == "kstardredge":
        # Space dredge mining claw & ore crystal
        draw_rect(img, 3, 3, 28, 28, (12, 10, 25, 255))
        draw_frame(img, 3, 3, 28, 28, ORANGE, 1)
        # Asteroid ore chunk (rough hexagon)
        draw_rect(img, 12, 13, 22, 23, (80, 70, 60, 255))
        # Glowing purple ore crystal inside
        draw_rect(img, 15, 16, 19, 20, PURPLE)
        draw_rect(img, 16, 17, 18, 19, LIGHT_PURPLE)
        # Dredge laser / claw prongs
        draw_line(img, 5, 7, 13, 14, GOLD)
        draw_line(img, 5, 23, 13, 16, GOLD)
        draw_circle(img, 5, 15, 3, CYAN, fill=True)

    elif app_id == "kstarforge":
        # Orbital station fabrication anvil / ring
        draw_rect(img, 3, 3, 28, 28, (8, 12, 28, 255))
        draw_frame(img, 3, 3, 28, 28, CYAN, 1)
        # Orbital ring
        draw_circle(img, 15, 15, 10, CYAN, fill=False)
        # Station core / hammer forge
        draw_rect(img, 11, 10, 19, 20, GOLD)
        draw_rect(img, 13, 8, 17, 10, WHITE)
        draw_line(img, 7, 15, 23, 15, LIGHT_GOLD)
        draw_circle(img, 15, 15, 2, RED, fill=True)

    elif app_id == "ksubmarine":
        # Submarine silhouette under sea waves
        draw_rect(img, 3, 3, 28, 28, (3, 25, 45, 255))
        draw_frame(img, 3, 3, 28, 28, (2, 132, 199, 255), 1)
        # Surface water wave lines
        draw_line(img, 4, 7, 27, 7, (56, 189, 248, 255))
        draw_line(img, 4, 10, 27, 10, (14, 165, 233, 255))
        # Sub hull
        draw_rect(img, 7, 16, 23, 22, (71, 85, 105, 255))
        draw_circle(img, 7, 19, 3, (71, 85, 105, 255), fill=True)
        draw_circle(img, 23, 19, 3, (71, 85, 105, 255), fill=True)
        # Conning tower & periscope
        draw_rect(img, 13, 12, 17, 16, (100, 116, 139, 255))
        draw_line(img, 15, 9, 15, 12, WHITE)
        draw_rect(img, 15, 8, 17, 9, WHITE)
        # Sonar ping dot
        draw_circle(img, 26, 19, 2, (34, 197, 94, 255), fill=True)

    elif app_id == "ksanctuary":
        # Biodome / ecological oasis shield
        draw_rect(img, 3, 3, 28, 28, (10, 25, 15, 255))
        draw_frame(img, 3, 3, 28, 28, GREEN, 1)
        # Dome outline
        draw_circle(img, 15, 16, 9, LIGHT_GREEN, fill=False)
        draw_line(img, 6, 23, 24, 23, GREEN)
        # Tree of life in center
        draw_rect(img, 14, 17, 16, 23, (120, 53, 15, 255))
        draw_circle(img, 15, 14, 4, GREEN, fill=True)
        draw_circle(img, 15, 13, 2, LIGHT_GREEN, fill=True)

    elif app_id == "kfortress":
        # Heavy stone castle parapet / bastion
        draw_rect(img, 3, 3, 28, 28, (30, 30, 35, 255))
        draw_frame(img, 3, 3, 28, 28, GRAY, 1)
        # Castle battlement top notches
        draw_rect(img, 6, 8, 9, 12, GRAY)
        draw_rect(img, 12, 8, 15, 12, GRAY)
        draw_rect(img, 18, 8, 21, 12, GRAY)
        draw_rect(img, 24, 8, 26, 12, GRAY)
        # Bastion wall body
        draw_rect(img, 6, 12, 26, 24, DARK_GRAY)
        # Gate arch
        draw_rect(img, 13, 18, 19, 24, (10, 10, 15, 255))
        draw_circle(img, 16, 18, 3, (10, 10, 15, 255), fill=True)
        # Banner flag
        draw_line(img, 7, 4, 7, 8, GOLD)
        draw_rect(img, 8, 4, 11, 6, RED)

    elif app_id == "kabyss":
        # Deep abyssal oceanic trench with glowing angler eye
        draw_rect(img, 2, 2, 29, 29, (2, 6, 23, 255))
        draw_frame(img, 2, 2, 29, 29, (30, 58, 138, 255), 1)
        # Angled rock canyon walls
        for y in range(4, 28):
            draw_line(img, 3, y, min(10, 3 + (y // 3)), y, (15, 23, 42, 255))
            draw_line(img, max(21, 28 - (y // 3)), y, 28, y, (15, 23, 42, 255))
        # Bioluminescent entity lure
        draw_line(img, 15, 10, 15, 16, CYAN)
        draw_circle(img, 15, 16, 3, (56, 189, 248, 255), fill=True)
        draw_circle(img, 15, 16, 1, WHITE, fill=True)
        # Bubbles
        draw_circle(img, 12, 8, 1, CYAN, fill=True)
        draw_circle(img, 18, 6, 1, CYAN, fill=True)

    elif app_id == "kcosmic":
        # Deep space galaxy spiral
        draw_rect(img, 2, 2, 29, 29, (5, 2, 15, 255))
        draw_frame(img, 2, 2, 29, 29, PURPLE, 1)
        # Spiral arms
        draw_circle(img, 15, 15, 9, (147, 51, 234, 255), fill=False)
        draw_circle(img, 15, 15, 5, (192, 132, 252, 255), fill=False)
        draw_circle(img, 15, 15, 2, WHITE, fill=True)
        # Background stars
        img[6][8] = GOLD
        img[22][23] = CYAN
        img[8][23] = WHITE
        img[21][7] = LIGHT_PURPLE

    elif app_id == "kdraw":
        # Artist easel & paintbrush palette
        draw_rect(img, 3, 3, 28, 28, (250, 250, 249, 255))
        draw_frame(img, 3, 3, 28, 28, (120, 113, 108, 255), 1)
        # Palette shape
        draw_circle(img, 14, 15, 8, (245, 222, 179, 255), fill=True)
        # Thumbhole
        draw_circle(img, 10, 17, 2, (250, 250, 249, 255), fill=True)
        # Paint color dabs
        img[11][13] = RED
        img[14][10] = GOLD
        img[18][12] = CYAN
        img[18][17] = GREEN
        # Brush crossing palette
        draw_line(img, 6, 25, 24, 7, (120, 53, 15, 255))
        draw_line(img, 23, 8, 26, 5, (200, 200, 200, 255))

    elif app_id == "kmatrix":
        # Falling digital green matrix rain
        draw_rect(img, 2, 2, 29, 29, (0, 10, 0, 255))
        draw_frame(img, 2, 2, 29, 29, GREEN, 1)
        # Matrix glyph rain columns
        cols = [5, 9, 13, 17, 21, 25]
        for idx, col in enumerate(cols):
            start_y = (idx * 3) % 8 + 4
            for y in range(start_y, min(27, start_y + 16), 3):
                img[y][col] = DARK_GREEN
                if y + 1 < 27:
                    img[y + 1][col] = GREEN
            # Bright lead head
            head_y = min(26, start_y + 15)
            img[head_y][col] = WHITE

    elif app_id == "kdirector":
        # Master Director golden pillar & eye console
        draw_rect(img, 2, 2, 29, 29, (15, 10, 0, 255))
        draw_frame(img, 2, 2, 29, 29, GOLD, 1)
        # Classical director pillar pediment
        draw_rect(img, 6, 6, 25, 8, GOLD)
        # Columns
        draw_rect(img, 8, 9, 10, 22, LIGHT_GOLD)
        draw_rect(img, 14, 9, 17, 22, LIGHT_GOLD)
        draw_rect(img, 21, 9, 23, 22, LIGHT_GOLD)
        # Base plinth
        draw_rect(img, 6, 23, 25, 25, GOLD)
        # Central eye
        draw_circle(img, 15, 15, 3, CYAN, fill=True)
        img[15][15] = WHITE

    elif app_id == "ksound":
        # Sound wave frequency bar speaker
        draw_rect(img, 3, 3, 28, 28, (15, 23, 42, 255))
        draw_frame(img, 3, 3, 28, 28, (99, 102, 241, 255), 1)
        # Equalizer bars
        bars = [(6, 12), (10, 18), (14, 23), (18, 15), (22, 20), (25, 9)]
        for x, h in bars:
            top_y = 25 - h
            for y in range(top_y, 25):
                color = GREEN if y > 17 else (GOLD if y > 12 else RED)
                img[y][x] = color
                img[y][x + 1] = color

    else:
        # Fallback procedural icon: colored tile with border and app initials
        draw_rect(img, 3, 3, 28, 28, (30, 41, 59, 255))
        draw_frame(img, 3, 3, 28, 28, CYAN, 1)
        draw_circle(img, 15, 15, 6, GOLD, fill=True)
        draw_circle(img, 15, 15, 3, DARK_BLUE, fill=True)

    save_ico(out_path, img)


# ---------------------------------------------------------------------------
# Fleet Icon Auditor
# ---------------------------------------------------------------------------
def parse_app_jsx() -> list[dict]:
    """Extracts all application definitions from App.jsx."""
    if not APP_JSX_PATH.exists():
        raise FileNotFoundError(f"App.jsx not found at {APP_JSX_PATH}")

    content = APP_JSX_PATH.read_text(encoding="utf-8")

    # Match objects inside APPS = [ ... ]
    apps_match = re.search(r"const APPS = \[(.*?)\];", content, re.DOTALL)
    if not apps_match:
        raise ValueError("Could not locate 'const APPS = [ ... ]' in App.jsx")

    apps_block = apps_match.group(1)
    # Parse individual app objects
    app_objects = []
    # Match each object { id: '...', title: '...', icon: '...', ... }
    for match in re.finditer(r"\{\s*([^}]+)\s*\}", apps_block):
        obj_text = match.group(1)
        id_m = re.search(r"id:\s*['\"]([^'\"]+)['\"]", obj_text)
        title_m = re.search(r"title:\s*['\"]([^'\"]+)['\"]", obj_text)
        icon_m = re.search(r"icon:\s*['\"]([^'\"]+)['\"]", obj_text)

        if id_m:
            app_id = id_m.group(1)
            title = title_m.group(1) if title_m else app_id
            icon = icon_m.group(1) if icon_m else ""
            app_objects.append({
                "id": app_id,
                "title": title,
                "icon": icon,
                "raw": obj_text,
            })

    return app_objects


def audit_icons(fix: bool = False) -> int:
    """Audits icon uniqueness across all KiloApps. Returns 0 if clean, 1 if issues found."""
    print("=" * 60)
    print("KiloApps Fleet Icon Uniqueness Audit")
    print(f"Icon Directory: {ICON_DIR}")
    print("=" * 60)

    apps = parse_app_jsx()
    print(f"Total apps registered in App.jsx: {len(apps)}")

    # 1. Check missing icon files and shared icon paths
    missing_icons = []
    icon_path_to_apps = defaultdict(list)
    for app in apps:
        icon_url = app["icon"]
        if not icon_url:
            missing_icons.append((app["id"], "No icon attribute defined"))
            continue

        icon_path_to_apps[icon_url].append(app["id"])
        # Resolve to local disk path
        if icon_url.startswith("/assets/icons/"):
            filename = icon_url.replace("/assets/icons/", "")
            local_path = ICON_DIR / filename
            if not local_path.exists():
                missing_icons.append((app["id"], f"File not found: {local_path.name}"))

    # 2. Check for identical icon paths in App.jsx
    shared_paths = {p: a for p, a in icon_path_to_apps.items() if len(a) > 1}

    # 3. Check SHA256 hashes of all icons in ICON_DIR
    hash_to_files = defaultdict(list)
    for ico_file in sorted(ICON_DIR.glob("*.ico")):
        try:
            h = hashlib.sha256(ico_file.read_bytes()).hexdigest()
            hash_to_files[h].append(ico_file.name)
        except Exception as e:
            print(f"Error reading {ico_file.name}: {e}")

    shared_hashes = {h: files for h, files in hash_to_files.items() if len(files) > 1}

    # Display findings
    print("\n[1] Missing Icon References:")
    if missing_icons:
        for app_id, reason in missing_icons:
            print(f"  ❌ {app_id}: {reason}")
    else:
        print("  ✅ All apps reference existing icon files.")

    print("\n[2] Shared Icon Paths in App.jsx (Multiple apps pointing to same file):")
    if shared_paths:
        for path, shared_apps in shared_paths.items():
            print(f"  ⚠️  {path} shared by: {', '.join(shared_apps)}")
    else:
        print("  ✅ Every app references a distinct icon path.")

    print("\n[3] Cloned / Duplicate Icon Hashes (Bit-for-bit identical .ico files):")
    if shared_hashes:
        for h, files in sorted(shared_hashes.items(), key=lambda x: -len(x[1])):
            print(f"  ⚠️  SHA256 {h[:10]}... shared by ({len(files)} files): {', '.join(files)}")
    else:
        print("  ✅ Every .ico file has a unique cryptographic hash.")

    total_issues = len(missing_icons) + len(shared_paths) + len(shared_hashes)

    if fix:
        print("\n" + "=" * 60)
        print("Applying Procedural Fixes for Cloned & Shared Icons...")
        print("=" * 60)

        # Fix 1: If an app shares an icon path (like knetmap pointing to knet.ico), assign unique file
        content = APP_JSX_PATH.read_text(encoding="utf-8")
        if "knetmap" in icon_path_to_apps.get("/assets/icons/knet.ico", []):
            print("  Fixing App.jsx: Re-pointing KNetMap to /assets/icons/knetmap.ico...")
            content = content.replace(
                "id: 'knetmap', title: 'KNetMap', url: '/apps/knetmap.html', exeUrl: '/exe/KNetMap.exe', icon: '/assets/icons/knet.ico'",
                "id: 'knetmap', title: 'KNetMap', url: '/apps/knetmap.html', exeUrl: '/exe/KNetMap.exe', icon: '/assets/icons/knetmap.ico'"
            )
            APP_JSX_PATH.write_text(content, encoding="utf-8")

        # Fix 2: Generate distinctive procedural icons for duplicated apps
        apps_to_differentiate = [
            ("knetmap", ICON_DIR / "knetmap.ico"),
            ("kcipher", ICON_DIR / "kcipher.ico"),
            ("ksteno", ICON_DIR / "ksteno.ico"),
            ("kanomaly", ICON_DIR / "kanomaly.ico"),
            ("kstardredge", ICON_DIR / "kstardredge.ico"),
            ("kstarforge", ICON_DIR / "kstarforge.ico"),
            ("ksubmarine", ICON_DIR / "ksubmarine.ico"),
            ("ksanctuary", ICON_DIR / "ksanctuary.ico"),
            ("kfortress", ICON_DIR / "kfortress.ico"),
            ("kabyss", ICON_DIR / "kabyss.ico"),
            ("kcosmic", ICON_DIR / "kcosmic.ico"),
            ("kdraw", ICON_DIR / "kdraw.ico"),
            ("kmatrix", ICON_DIR / "kmatrix.ico"),
            ("kdirector", ICON_DIR / "kdirector.ico"),
            ("ksound", ICON_DIR / "ksound.ico"),
        ]

        for app_id, target_file in apps_to_differentiate:
            print(f"  Generating distinctive icon for '{app_id}' -> {target_file.name}...")
            generate_distinctive_icon(app_id, target_file)

        print("\nFixes applied. Re-running audit...")
        return audit_icons(fix=False)

    print("\n" + "=" * 60)
    if total_issues == 0:
        print("✅ ALL ICONS UNIQUE AND VALID! Fleet icon audit passed.")
        print("=" * 60)
        return 0
    else:
        print(f"⚠️  FOUND {total_issues} ICON ISSUE(S). Run with --fix to resolve automatically.")
        print("=" * 60)
        return 1


def main():
    parser = argparse.ArgumentParser(description="KiloApps Fleet Icon Uniqueness Auditor")
    parser.add_argument("--fix", action="store_true", help="Procedurally generate unique icons for duplicate apps")
    args = parser.parse_args()

    sys.exit(audit_icons(fix=args.fix))


if __name__ == "__main__":
    main()
