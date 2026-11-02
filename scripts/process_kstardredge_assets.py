# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
Process Imagen 3 generated sprites and complementary assets for KStarDredge:
1. derelict_hull (Imagen 3 top-down derelict starship wreckage)
2. pirate_wreckage (Imagen 3 top-down pirate corsair debris)
3. anomaly_geode (Cosmic geode resonance cluster)
4. anomaly_singularity (Cosmic dark matter vortex)
5. salvage_barge (Industrial dredge barge)
6. raider_corsair (Fast pirate skiff)
7. raider_gunship (Heavy pirate gunship)
8. raider_dread (Massive syndicate warship)
"""

import json
import os
import sys
from pathlib import Path
from PIL import Image, ImageChops
import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asset_pipeline import key_out_magenta

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites"
OUT_DIR = SPRITE_DIR / "kstardredge"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def write_atlas(filename: str, w: int, h: int):
    stem = Path(filename).stem
    atlas = {
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
    with open(OUT_DIR / f"{stem}.json", "w", encoding="utf-8") as f:
        json.dump(atlas, f, indent=2)


def process_keyed_sprite(src_path: Path, out_name: str, target_size=(128, 128), padding=4):
    if not src_path.exists():
        print(f"Skipping {src_path}, not found")
        return False
    img = Image.open(src_path)
    keyed = key_out_magenta(img)
    bbox = keyed.getbbox()
    if bbox:
        cropped = keyed.crop(bbox)
        # Pad to square
        w, h = cropped.size
        max_dim = max(w, h)
        sq = Image.new("RGBA", (max_dim + padding * 2, max_dim + padding * 2), (0, 0, 0, 0))
        sq.paste(cropped, (padding + (max_dim - w) // 2, padding + (max_dim - h) // 2))
        res = sq.resize(target_size, Image.Resampling.LANCZOS)
    else:
        res = keyed.resize(target_size, Image.Resampling.LANCZOS)
    res.save(OUT_DIR / out_name, "PNG", optimize=True)
    write_atlas(out_name, target_size[0], target_size[1])
    print(f"Processed {out_name} ({target_size[0]}x{target_size[1]})")
    return True


def copy_or_remix_sprite(src_path: Path, out_name: str, tint=(1.0, 1.0, 1.0), target_size=(128, 128)):
    if not src_path.exists():
        print(f"Skipping {src_path}, not found")
        return False
    img = Image.open(src_path).convert("RGBA").resize(target_size, Image.Resampling.LANCZOS)
    arr = np.array(img, dtype=np.float32)
    arr[:, :, 0] = np.clip(arr[:, :, 0] * tint[0], 0, 255)
    arr[:, :, 1] = np.clip(arr[:, :, 1] * tint[1], 0, 255)
    arr[:, :, 2] = np.clip(arr[:, :, 2] * tint[2], 0, 255)
    res = Image.fromarray(arr.astype(np.uint8), "RGBA")
    res.save(OUT_DIR / out_name, "PNG", optimize=True)
    write_atlas(out_name, target_size[0], target_size[1])
    print(f"Processed {out_name} ({target_size[0]}x{target_size[1]})")
    return True


# 1. Derelict Starship Wreckage (Imagen 3 generated)
img1_path = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\88a13f2c-132c-4ce0-bf40-7bc11914828a\derelict_hull_wreckage_1793597573270.jpg")
process_keyed_sprite(img1_path, "derelict_hull.png", (128, 128))

# 2. Pirate Corsair Wreckage Hulk (Imagen 3 generated)
img2_path = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\88a13f2c-132c-4ce0-bf40-7bc11914828a\pirate_wreckage_hulk_1793597588654.jpg")
process_keyed_sprite(img2_path, "pirate_wreckage.png", (96, 96))

# 3. Cosmic Anomaly - Radiant Geode
copy_or_remix_sprite(SPRITE_DIR / "kstellar" / "anomaly_ionstorm.png", "anomaly_geode.png", (1.2, 0.9, 1.4), (128, 128))

# 4. Cosmic Anomaly - Dark Singularity
copy_or_remix_sprite(SPRITE_DIR / "kstellar" / "anomaly_blackhole.png", "anomaly_singularity.png", (1.3, 0.7, 0.8), (128, 128))

# 5. Industrial Salvage Barge (player ship)
copy_or_remix_sprite(SPRITE_DIR / "kstarforge" / "module_mining.png", "salvage_barge.png", (1.1, 1.05, 0.9), (96, 96))

# 6. Pirate Raider vessels
copy_or_remix_sprite(SPRITE_DIR / "kstarforge" / "vessel_interceptor.png", "raider_corsair.png", (1.3, 0.7, 0.7), (96, 96))
copy_or_remix_sprite(SPRITE_DIR / "kstarforge" / "vessel_gunship.png", "raider_gunship.png", (1.3, 0.8, 0.7), (96, 96))
copy_or_remix_sprite(SPRITE_DIR / "kstarforge" / "vessel_dreadnought.png", "raider_dread.png", (1.4, 0.6, 0.6), (128, 128))

# 7. Spaceport Orbital Station
copy_or_remix_sprite(SPRITE_DIR / "kstarforge" / "starforge_station.png", "orbital_spaceport.png", (0.9, 1.1, 1.3), (160, 160))

print("Asset processing complete for KStarDredge!")
