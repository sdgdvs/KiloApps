# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
Generates high-fidelity cyberpunk sprites for KCyber:
1. Cyberdeck Console (player deck)
2. 5 Distinct ICE Guardian Constructs:
   - Basic ICE (Node 02)
   - Tracer ICE (Node 03)
   - Hunter ICE (Node 04)
   - Black ICE (Node 05)
   - Phantom ICE (Node 06)
3. Cryo-Stasis Lattice (frozen ICE)
4. Classified Data Core Cartridge (root node loot)
5. Darknet Contraband Vault (shop / black market)
"""

import json
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites" / "kcyber"
SPRITE_DIR.mkdir(parents=True, exist_ok=True)


def create_atlas(filename: str, w: int, h: int) -> dict:
    stem = Path(filename).stem
    return {
        "frames": {
            f"{stem}_0": {
                "frame": {"x": 0, "y": 0, "w": w, "h": h},
                "rotated": False,
                "trimmed": False,
                "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
                "sourceSize": {"w": w, "h": h},
                "pivot": {"x": 0.5, "y": 0.5}
            }
        },
        "meta": {
            "app": "KiloApps Asset Pipeline",
            "version": "1.0",
            "image": filename,
            "format": "RGBA8888",
            "size": {"w": w, "h": h},
            "scale": "1"
        }
    }


def save_sprite(img: Image.Image, filename: str):
    path = SPRITE_DIR / filename
    img.save(path, format="PNG", optimize=True)
    atlas = create_atlas(filename, img.width, img.height)
    atlas_path = SPRITE_DIR / f"{Path(filename).stem}.json"
    with open(atlas_path, "w", encoding="utf-8") as f:
        json.dump(atlas, f, indent=2)
    print(f"Generated {filename}: {img.width}x{img.height}")


def draw_cyberdeck_console() -> Image.Image:
    # 128x128 Cyberdeck Console sprite
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Holographic base pedestal glow
    d.ellipse([(20, 96), (108, 118)], fill=(0, 40, 20, 60), outline=(0, 255, 120, 110), width=1)

    # Braided neural interface cables grounding out
    d.line([(28, 88), (14, 114)], fill=(0, 180, 80, 180), width=2)
    d.line([(100, 88), (114, 114)], fill=(0, 180, 80, 180), width=2)

    # Cyberdeck Keyboard Base (Trapezoid)
    deck_poly = [(24, 76), (104, 76), (116, 102), (12, 102)]
    d.polygon(deck_poly, fill=(2, 36, 15, 245), outline=(0, 255, 100, 255))

    # Keyboard rows
    d.line([(28, 84), (100, 84)], fill=(0, 160, 60, 200), width=1)
    d.line([(22, 92), (106, 92)], fill=(0, 160, 60, 200), width=1)
    for kx in range(30, 99, 8):
        d.line([(kx, 79), (kx - 2, 83)], fill=(0, 200, 80, 180), width=1)
        d.line([(kx - 3, 85), (kx - 5, 91)], fill=(0, 200, 80, 180), width=1)

    # Front Status LEDs
    led_colors = [(0, 255, 100, 255), (255, 230, 0, 255), (0, 230, 255, 255), (255, 0, 220, 255)]
    for idx, col in enumerate(led_colors):
        lx = 38 + idx * 18
        d.ellipse([(lx - 2, 96), (lx + 2, 100)], fill=col)

    # Angled CRT Screen Housing
    screen_housing = [(32, 28), (96, 28), (96, 74), (32, 74)]
    d.polygon(screen_housing, fill=(0, 20, 8, 255), outline=(0, 255, 100, 255))
    d.rectangle([(36, 32), (92, 70)], fill=(0, 10, 4, 255), outline=(0, 140, 50, 180))

    # CRT Screen Content: Spectrum Bars + Sine Wave
    # Spectrum analyzer bars
    for b in range(6):
        bh = 6 + int(math.sin(b * 1.2) * 5 + 5)
        bx = 40 + b * 8
        d.rectangle([(bx, 68 - bh), (bx + 5, 68)], fill=(0, 220, 100, 200) if b < 4 else (255, 200, 0, 200))

    # Waveform line
    wave_pts = []
    for wx in range(38, 91, 2):
        wy = 46 + int(math.sin((wx - 38) * 0.4) * 6)
        wave_pts.append((wx, wy))
    for i in range(len(wave_pts) - 1):
        d.line([wave_pts[i], wave_pts[i+1]], fill=(0, 255, 140, 255), width=2)

    # Antenna / Neural Jack on deck side
    d.line([(32, 40), (18, 20)], fill=(0, 255, 120, 255), width=2)
    d.ellipse([(16, 18), (20, 22)], fill=(0, 255, 180, 255))
    d.arc([(10, 12), (26, 28)], start=-120, end=0, fill=(0, 255, 180, 160), width=1)
    d.arc([(6, 8), (30, 32)], start=-120, end=0, fill=(0, 255, 180, 90), width=1)

    return img


def draw_ice_basic_02() -> Image.Image:
    # Basic ICE (Node 02) — Crystalline Emerald Perimeter Firewall Construct
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Outer Hexagonal Shield Ring
    hex_outer = []
    for i in range(6):
        rad = math.radians(i * 60)
        hex_outer.append((cx + math.cos(rad) * 48, cy + math.sin(rad) * 48))
    d.polygon(hex_outer, fill=(2, 38, 24, 180), outline=(0, 255, 120, 255))

    # Internal Hex Grid
    hex_inner = []
    for i in range(6):
        rad = math.radians(i * 60 + 30)
        hex_inner.append((cx + math.cos(rad) * 32, cy + math.sin(rad) * 32))
    d.polygon(hex_inner, fill=(0, 60, 36, 220), outline=(0, 255, 180, 255))

    # Core firewall chip
    d.rectangle([(cx - 14, cy - 14), (cx + 14, cy + 14)], fill=(0, 24, 14, 255), outline=(0, 255, 140, 255), width=2)

    # Data bus nodes on spokes
    for i in range(6):
        rad = math.radians(i * 60)
        px = cx + math.cos(rad) * 48
        py = cy + math.sin(rad) * 48
        d.line([(cx, cy), (px, py)], fill=(0, 255, 150, 180), width=1)
        d.rectangle([(px - 3, py - 3), (px + 3, py + 3)], fill=(0, 255, 120, 255), outline=(255, 255, 255, 255))

    # Central pulsing glyph
    d.line([(cx - 8, cy), (cx + 8, cy)], fill=(0, 255, 220, 255), width=2)
    d.line([(cx, cy - 8), (cx, cy + 8)], fill=(0, 255, 220, 255), width=2)
    d.ellipse([(cx - 4, cy - 4), (cx + 4, cy + 4)], fill=(255, 255, 255, 255))

    return img


def draw_ice_tracer_03() -> Image.Image:
    # Tracer ICE (Node 03) — Corporate Tracking Drone in Amber & Warning Orange
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Outer Radar Targeting Field
    d.ellipse([(cx - 46, cy - 46), (cx + 46, cy + 46)], fill=(35, 20, 0, 160), outline=(255, 170, 0, 200), width=1)
    d.ellipse([(cx - 32, cy - 32), (cx + 32, cy + 32)], outline=(255, 140, 0, 180), width=1)

    # Crosshairs
    d.line([(cx - 52, cy), (cx + 52, cy)], fill=(255, 190, 0, 220), width=1)
    d.line([(cx, cy - 52), (cx, cy + 52)], fill=(255, 190, 0, 220), width=1)

    # Tri-Rotor Scanning Probes
    for i in range(3):
        rad = math.radians(i * 120 - 90)
        px = cx + math.cos(rad) * 38
        py = cy + math.sin(rad) * 38
        # Probe link
        d.line([(cx, cy), (px, py)], fill=(255, 160, 0, 255), width=2)
        # Probe sensor pod
        d.ellipse([(px - 8, py - 8), (px + 8, py + 8)], fill=(60, 32, 0, 255), outline=(255, 220, 0, 255), width=2)
        d.ellipse([(px - 3, py - 3), (px + 3, py + 3)], fill=(255, 60, 0, 255))

    # Central Core Chasis
    d.polygon([(cx, cy - 18), (cx + 18, cy + 12), (cx - 18, cy + 12)], fill=(40, 18, 0, 255), outline=(255, 200, 0, 255), width=2)
    # Scanning Eye
    d.ellipse([(cx - 6, cy - 3), (cx + 6, cy + 7)], fill=(255, 240, 50, 255), outline=(255, 80, 0, 255))
    d.rectangle([(cx - 2, cy - 1), (cx + 2, cy + 5)], fill=(120, 0, 0, 255))

    return img


def draw_ice_hunter_04() -> Image.Image:
    # Hunter ICE (Node 04) — Aggressive Crimson Raptor Construct
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Escort Razor Darts
    flanks = [(-42, -16), (42, -16), (-36, 24), (36, 24)]
    for fx, fy in flanks:
        px = cx + fx
        py = cy + fy
        d.line([(cx, cy), (px, py)], fill=(200, 20, 20, 140), width=1)
        dart = [(px, py - 9), (px + 6, py + 5), (px, py + 2), (px - 6, py + 5)]
        d.polygon(dart, fill=(160, 0, 20, 255), outline=(255, 140, 140, 255), width=1)

    # Angular Razor Wings / Chassis
    chassis = [
        (cx, cy - 38),
        (cx + 18, cy - 14),
        (cx + 38, cy + 2),
        (cx + 22, cy + 28),
        (cx, cy + 36),
        (cx - 22, cy + 28),
        (cx - 38, cy + 2),
        (cx - 18, cy - 14)
    ]
    d.polygon(chassis, fill=(50, 4, 8, 255), outline=(255, 30, 40, 255), width=2)

    # Inner armor plates
    inner_plate = [
        (cx, cy - 24),
        (cx + 14, cy),
        (cx, cy + 22),
        (cx - 14, cy)
    ]
    d.polygon(inner_plate, fill=(110, 10, 16, 255), outline=(255, 80, 90, 255), width=1)

    # Menacing Tri-Eye Visor
    d.rectangle([(cx - 18, cy - 5), (cx + 18, cy + 5)], fill=(15, 0, 2, 255), outline=(255, 60, 70, 255))
    d.ellipse([(cx - 12, cy - 3), (cx - 6, cy + 3)], fill=(255, 0, 0, 255))
    d.ellipse([(cx - 3, cy - 3), (cx + 3, cy + 3)], fill=(255, 255, 0, 255))
    d.ellipse([(cx + 6, cy - 3), (cx + 12, cy + 3)], fill=(255, 0, 0, 255))

    return img


def draw_ice_black_05() -> Image.Image:
    # Black ICE (Node 05) — Military Demonic Obsidian 8-Point Hazard Star Fortress
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Threat Matrix Halo
    d.ellipse([(cx - 48, cy - 48), (cx + 48, cy + 48)], fill=(30, 0, 4, 100), outline=(255, 30, 30, 120), width=1)

    # Heavy Barbed Escort Sentry Spikes
    interceptors = [(-46, -18), (46, -18), (-40, 26), (40, 26)]
    for ix, iy in interceptors:
        px = cx + ix
        py = cy + iy
        d.line([(cx, cy), (px, py)], fill=(255, 30, 30, 160), width=1)
        barb = [(px, py - 10), (px + 7, py + 6), (px, py + 2), (px - 7, py + 6)]
        d.polygon(barb, fill=(180, 0, 15, 255), outline=(255, 180, 180, 255), width=1)

    # 8-Point Demonic Hazard Star Main Core
    pts = []
    for i in range(8):
        rad = math.radians(i * 45)
        r = 38 if (i % 2 == 0) else 24
        pts.append((cx + math.cos(rad) * r, cy + math.sin(rad) * r))
    d.polygon(pts, fill=(28, 0, 4, 255), outline=(255, 35, 35, 255), width=3)

    # Lateral Armored Flange Shards
    d.polygon([(cx - 26, cy - 22), (cx - 40, cy), (cx - 26, cy + 22)], fill=(90, 0, 10, 255), outline=(255, 70, 70, 255), width=1)
    d.polygon([(cx + 26, cy - 22), (cx + 40, cy), (cx + 26, cy + 22)], fill=(90, 0, 10, 255), outline=(255, 70, 70, 255), width=1)

    # Visor Slit
    d.rectangle([(cx - 22, cy - 6), (cx + 22, cy + 6)], fill=(8, 0, 1, 255), outline=(255, 90, 90, 255), width=2)
    # Ominous Red Slit Eye
    d.ellipse([(cx - 8, cy - 4), (cx + 8, cy + 4)], fill=(255, 0, 0, 255))
    d.ellipse([(cx - 3, cy - 2), (cx + 3, cy + 2)], fill=(255, 255, 200, 255))

    return img


def draw_ice_phantom_06() -> Image.Image:
    # Phantom ICE (Node 06) — Void-Phase Stealth Sentinel in Violet / Magenta
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # 4 Phasing Diamond Stealth Shrouds
    shrouds = [(-44, -14), (44, -14), (-36, 24), (36, 24)]
    for sx, sy in shrouds:
        px = cx + sx
        py = cy + sy
        # Phase tether filament
        d.line([(cx, cy), (px, py)], fill=(180, 0, 240, 160), width=1)
        # Diamond Plate
        diamond = [(px, py - 9), (px + 7, py), (px, py + 9), (px - 7, py)]
        d.polygon(diamond, fill=(120, 0, 180, 255), outline=(255, 255, 255, 255), width=1)

    # Central Obsidian Polyhedral Armor Shell
    core = [
        (cx, cy - 38),
        (cx + 28, cy - 14),
        (cx + 28, cy + 18),
        (cx, cy + 38),
        (cx - 28, cy + 18),
        (cx - 28, cy - 14)
    ]
    d.polygon(core, fill=(22, 0, 32, 255), outline=(210, 0, 255, 255), width=2)

    # Lateral Shard Vanes
    d.polygon([(cx - 28, cy - 14), (cx - 38, cy - 4), (cx - 28, cy + 10)], fill=(90, 0, 140, 200), outline=(210, 50, 255, 255), width=1)
    d.polygon([(cx + 28, cy - 14), (cx + 38, cy - 4), (cx + 28, cy + 10)], fill=(90, 0, 140, 200), outline=(210, 50, 255, 255), width=1)

    # Quantum Optic Visor Slit
    d.rectangle([(cx - 20, cy - 5), (cx + 20, cy + 5)], fill=(10, 0, 16, 255), outline=(240, 100, 255, 255), width=1)
    d.ellipse([(cx - 6, cy - 3), (cx + 6, cy + 3)], fill=(255, 0, 230, 255))
    d.ellipse([(cx - 2, cy - 1), (cx + 2, cy + 1)], fill=(255, 255, 255, 255))

    return img


def draw_cryo_stasis_lattice() -> Image.Image:
    # Cryogenic Stasis Lattice (Blinded / Frozen ICE)
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Hexagonal Stasis Shield
    hex_stasis = []
    for i in range(6):
        rad = math.radians(i * 60)
        hex_stasis.append((cx + math.cos(rad) * 50, cy + math.sin(rad) * 50))
    d.polygon(hex_stasis, fill=(0, 30, 55, 160), outline=(0, 229, 255, 200), width=2)

    # 6 Radial Crystal Frost Spikes
    for i in range(6):
        rad = math.radians(i * 60)
        px = cx + math.cos(rad) * 46
        py = cy + math.sin(rad) * 46
        d.line([(cx, cy), (px, py)], fill=(0, 229, 255, 200), width=1)
        # Diamond spike at vertex
        spike = [(px, py - 6), (px + 4, py), (px, py + 6), (px - 4, py)]
        d.polygon(spike, fill=(0, 229, 255, 220), outline=(255, 255, 255, 255), width=1)

    # Frozen Inner Facets
    inner = []
    for i in range(6):
        rad = math.radians(i * 60 + 30)
        inner.append((cx + math.cos(rad) * 28, cy + math.sin(rad) * 28))
    d.polygon(inner, fill=(0, 48, 80, 240), outline=(0, 255, 255, 255), width=2)

    # Internal Frost Fracture Lines
    d.line([(cx - 18, cy - 10), (cx, cy + 4), (cx + 16, cy - 8)], fill=(255, 255, 255, 220), width=2)
    d.line([(cx, cy + 4), (cx - 5, cy + 20)], fill=(255, 255, 255, 220), width=2)

    return img


def draw_data_core_cartridge() -> Image.Image:
    # Classified Data Core Cartridge (root loot node)
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Holographic aura ring
    d.ellipse([(cx - 44, cy - 16), (cx + 44, cy + 16)], outline=(0, 255, 255, 140), width=1)

    # Cartridge Chassis
    cart_w, cart_h = 50, 68
    x0, y0 = cx - cart_w // 2, cy - cart_h // 2
    x1, y1 = x0 + cart_w, y0 + cart_h
    d.rectangle([(x0, y0), (x1, y1)], fill=(1, 28, 36, 250), outline=(0, 255, 255, 255), width=2)

    # Beveled metallic top rim
    d.line([(x0 + 3, y0 + 3), (x1 - 3, y0 + 3)], fill=(0, 255, 255, 200), width=2)

    # Gold connector pins at bottom
    for p in range(6):
        px = x0 + 7 + p * 6
        d.rectangle([(px, y1 - 6), (px + 4, y1)], fill=(255, 215, 0, 255))

    # Circuit traces
    d.line([(x0 + 8, y0 + 14), (x0 + 22, y0 + 14), (x0 + 22, y0 + 26)], fill=(0, 255, 200, 180), width=1)
    d.line([(x1 - 8, y0 + 20), (x1 - 22, y0 + 20), (x1 - 22, y0 + 32)], fill=(0, 255, 200, 180), width=1)

    # Central Microchip
    d.rectangle([(cx - 12, cy - 10), (cx + 12, cy + 10)], fill=(0, 65, 85, 255), outline=(0, 255, 200, 255), width=2)
    d.ellipse([(cx - 4, cy - 4), (cx + 4, cy + 4)], fill=(0, 255, 255, 255))

    return img


def draw_contraband_vault() -> Image.Image:
    # Darknet Contraband Vault (black market / shop)
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    cx, cy = 64, 64

    # Amber Holo-Pedestal Base
    d.ellipse([(cx - 48, cy + 30), (cx + 48, cy + 54)], fill=(30, 20, 0, 120), outline=(255, 183, 0, 180), width=1)

    # Gold Crypto-Credit Tokens
    for ox in [-26, 0, 26]:
        d.ellipse([(cx + ox - 5, cy + 38), (cx + ox + 5, cy + 48)], fill=(255, 215, 0, 255), outline=(160, 110, 0, 255), width=1)
        d.ellipse([(cx + ox - 2, cy + 41), (cx + ox + 2, cy + 45)], fill=(180, 120, 0, 255))

    # Octagonal Amber Vault Chassis
    r = 32
    oct_pts = []
    for i in range(8):
        rad = math.radians(i * 45 + 22.5)
        oct_pts.append((cx + math.cos(rad) * r, cy + math.sin(rad) * r - 4))
    d.polygon(oct_pts, fill=(35, 24, 0, 250), outline=(255, 183, 0, 255), width=2)

    # Inner Contraband Chip Slot
    d.rectangle([(cx - 12, cy - 12), (cx + 12, cy + 6)], fill=(65, 45, 0, 255), outline=(255, 220, 80, 255), width=2)
    # Gold Contact Teeth
    for p in range(-8, 9, 4):
        d.rectangle([(cx + p, cy + 1), (cx + p + 2, cy + 5)], fill=(255, 215, 0, 255))

    # Pulsing Encrypted Lock Emblem
    d.arc([(cx - 6, cy - 10), (cx + 6, cy + 2)], start=180, end=0, fill=(255, 215, 0, 255), width=2)
    d.rectangle([(cx - 5, cy - 5), (cx + 5, cy + 1)], fill=(255, 215, 0, 255))

    return img


def main():
    print("Generating KCyber High-Fidelity Cyberpunk Sprites...")
    assets = [
        ("cyberdeck_console.png", draw_cyberdeck_console()),
        ("ice_basic_02.png", draw_ice_basic_02()),
        ("ice_tracer_03.png", draw_ice_tracer_03()),
        ("ice_hunter_04.png", draw_ice_hunter_04()),
        ("ice_black_05.png", draw_ice_black_05()),
        ("ice_phantom_06.png", draw_ice_phantom_06()),
        ("cryo_stasis_lattice.png", draw_cryo_stasis_lattice()),
        ("data_core_cartridge.png", draw_data_core_cartridge()),
        ("contraband_vault.png", draw_contraband_vault()),
    ]
    for filename, img in assets:
        save_sprite(img, filename)
    print("All KCyber assets generated successfully.")


if __name__ == "__main__":
    main()
