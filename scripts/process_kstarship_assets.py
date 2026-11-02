# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
Process Imagen 3 generated sprites and complementary assets for KStarship:
1. hull_schematic (Imagen 3 top-down starship blueprint schematic)
2. nebula_backdrop (Volumetric cosmic nebula parallax backdrop)
3. cruiser_corsair (Fast pirate corsair raider silhouette)
4. cruiser_dreadnought (Imperial capital battlecruiser silhouette)
5. cruiser_syndicate (Syndicate stealth railgun frigate)
6. cruiser_xenon (Serrated bio-hunter predator vessel)
7. cruiser_leviathan (Eldritch void leviathan bioship)
8. ship_flagship (Player command starship)
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
OUT_DIR = SPRITE_DIR / "kstarship"
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
        w, h = cropped.size
        # Maintain aspect ratio within target_size
        scale = min((target_size[0] - padding * 2) / w, (target_size[1] - padding * 2) / h)
        nw, nh = max(1, int(w * scale)), max(1, int(h * scale))
        resized = cropped.resize((nw, nh), Image.Resampling.LANCZOS)
        sq = Image.new("RGBA", target_size, (0, 0, 0, 0))
        sq.paste(resized, ((target_size[0] - nw) // 2, (target_size[1] - nh) // 2))
        res = sq
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


# 1. Imagen 3 Generated Starship Subsystem Schematic
img1_path = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\a243ef9b-2c3d-4c74-ba5b-4ce413117849\starship_subsystem_schematic_1793637185967.jpg")
process_keyed_sprite(img1_path, "hull_schematic.png", (192, 128), padding=2)

# 2. Parallax Nebula Cosmic Cloud
copy_or_remix_sprite(SPRITE_DIR / "kstellar" / "anomaly_ionstorm.png", "nebula_backdrop.png", (0.8, 1.1, 1.4), (256, 256))

# 3. Enemy Cruisers Silhouettes
# Corsair Raider
copy_or_remix_sprite(SPRITE_DIR / "kstardredge" / "raider_corsair.png", "cruiser_corsair.png", (1.2, 0.7, 0.6), (96, 96))
# Imperial Dreadnought
copy_or_remix_sprite(SPRITE_DIR / "kstarforge" / "vessel_dreadnought.png", "cruiser_dreadnought.png", (1.3, 0.6, 0.6), (128, 128))
# Syndicate Stealth Frigate
copy_or_remix_sprite(SPRITE_DIR / "kstellar" / "ship_stealth_infiltrator.png", "cruiser_syndicate.png", (1.2, 1.0, 0.6), (96, 96))
# Xenon Bio-Hunter
copy_or_remix_sprite(SPRITE_DIR / "ksubmarine" / "squid_abyssal.png", "cruiser_xenon.png", (1.3, 0.6, 1.3), (96, 96))
# Void Leviathan
copy_or_remix_sprite(SPRITE_DIR / "ksubmarine" / "leviathan_trench.png", "cruiser_leviathan.png", (0.5, 1.3, 1.0), (128, 128))

# 4. Player Command Flagship
copy_or_remix_sprite(SPRITE_DIR / "kstellar" / "ship_player_cruiser.png", "ship_flagship.png", (0.9, 1.15, 1.3), (96, 96))

print("Asset processing completed for KStarship!")
