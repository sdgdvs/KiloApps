# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
import json
import os
import sys
from pathlib import Path
from PIL import Image, ImageEnhance, ImageOps
import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asset_pipeline import key_out_magenta

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites"
OUT_DIR = SPRITE_DIR / "kstarforge"
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

# 1. Process test_spacecraft_hull as module_hull.png
hull_src = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\f80dc727-c58a-4f86-a6f5-1d1b257c0060\test_spacecraft_hull_1793518306670.jpg")
if hull_src.exists():
    hull_img = Image.open(hull_src)
    keyed_hull = key_out_magenta(hull_img)
    bbox = keyed_hull.getbbox()
    if bbox:
        cropped = keyed_hull.crop(bbox)
        max_dim = max(cropped.width, cropped.height)
        sq = Image.new("RGBA", (max_dim, max_dim), (0, 0, 0, 0))
        sq.paste(cropped, ((max_dim - cropped.width) // 2, (max_dim - cropped.height) // 2))
        res = sq.resize((64, 64), Image.Resampling.LANCZOS)
        res.save(OUT_DIR / "module_hull.png", "PNG", optimize=True)
        write_atlas("module_hull.png", 64, 64)
        print("Processed module_hull.png")

# 2. Add atlas JSON for remaining modules
for mod_name in ["module_bridge", "module_reactor", "module_engine", "module_tactical", "module_shield", "module_weapon", "module_utility", "module_mining"]:
    p = OUT_DIR / f"{mod_name}.png"
    if p.exists():
        write_atlas(f"{mod_name}.png", 64, 64)

# 3. Asteroids from kspace_asteroid.png (Titanium, Ferrite, Deuterium)
ast_src = SPRITE_DIR / "kspace" / "kspace_asteroid.png"
if ast_src.exists():
    ast_img = Image.open(ast_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    
    # Titanium: Cool cyan-metallic tint
    arr = np.array(ast_img, dtype=np.float32)
    ti_arr = arr.copy()
    ti_arr[:, :, 0] *= 0.65  # R
    ti_arr[:, :, 1] *= 0.95  # G
    ti_arr[:, :, 2] *= 1.25  # B
    ti_arr = np.clip(ti_arr, 0, 255).astype(np.uint8)
    ti_img = Image.fromarray(ti_arr, "RGBA")
    ti_img.save(OUT_DIR / "asteroid_titanium.png", "PNG", optimize=True)
    write_atlas("asteroid_titanium.png", 128, 128)
    print("Saved asteroid_titanium.png")

    # Ferrite: Dense copper-orange iron tint
    fe_arr = arr.copy()
    fe_arr[:, :, 0] *= 1.35  # R
    fe_arr[:, :, 1] *= 0.75  # G
    fe_arr[:, :, 2] *= 0.40  # B
    fe_arr = np.clip(fe_arr, 0, 255).astype(np.uint8)
    fe_img = Image.fromarray(fe_arr, "RGBA")
    fe_img.save(OUT_DIR / "asteroid_ferrite.png", "PNG", optimize=True)
    write_atlas("asteroid_ferrite.png", 128, 128)
    print("Saved asteroid_ferrite.png")

    # Deuterium: Deep crystalline violet-ice tint
    de_arr = arr.copy()
    de_arr[:, :, 0] *= 0.90  # R
    de_arr[:, :, 1] *= 0.50  # G
    de_arr[:, :, 2] *= 1.45  # B
    de_arr = np.clip(de_arr, 0, 255).astype(np.uint8)
    de_img = Image.fromarray(de_arr, "RGBA")
    de_img.save(OUT_DIR / "asteroid_deuterium.png", "PNG", optimize=True)
    write_atlas("asteroid_deuterium.png", 128, 128)
    print("Saved asteroid_deuterium.png")

# 4. Salvage Pod from data_core_cartridge.png
sal_src = SPRITE_DIR / "kcyber" / "data_core_cartridge.png"
if sal_src.exists():
    sal_img = Image.open(sal_src).convert("RGBA").resize((64, 64), Image.Resampling.LANCZOS)
    sal_img.save(OUT_DIR / "salvage_pod.png", "PNG", optimize=True)
    write_atlas("salvage_pod.png", 64, 64)
    print("Saved salvage_pod.png")

# 5. Station & Drydock Gantry from orbital_station_facility.png
st_src = SPRITE_DIR / "kstellar" / "orbital_station_facility.png"
if st_src.exists():
    st_img = Image.open(st_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    st_img.save(OUT_DIR / "starforge_station.png", "PNG", optimize=True)
    write_atlas("starforge_station.png", 128, 128)
    print("Saved starforge_station.png")

    # Drydock gantry frame
    gantry_img = st_img.resize((256, 256), Image.Resampling.LANCZOS)
    gantry_img.save(OUT_DIR / "drydock_gantry.png", "PNG", optimize=True)
    write_atlas("drydock_gantry.png", 256, 256)
    print("Saved drydock_gantry.png")

# 6. Combat & Skirmish Vessels:
vessel_map = [
    ("vessel_dreadnought", SPRITE_DIR / "kstellar" / "ship_syndicate_dreadnought.png", 128),
    ("vessel_interceptor", SPRITE_DIR / "kstellar" / "ship_corsair_interceptor.png", 96),
    ("vessel_gunship", SPRITE_DIR / "kstellar" / "ship_corsair_marauder.png", 96),
    ("vessel_sentry", SPRITE_DIR / "kspace" / "kspace_enemy_armored.png", 80),
    ("vessel_rival", SPRITE_DIR / "kspace" / "kspace_ship_crimson.png", 112)
]

for vname, vsrc, sz in vessel_map:
    if vsrc.exists():
        vimg = Image.open(vsrc).convert("RGBA").resize((sz, sz), Image.Resampling.LANCZOS)
        vimg.save(OUT_DIR / f"{vname}.png", "PNG", optimize=True)
        write_atlas(f"{vname}.png", sz, sz)
        print(f"Saved {vname}.png ({sz}x{sz})")

print("All KStarForge sprites and atlases processed successfully!")
