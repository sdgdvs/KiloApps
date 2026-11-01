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
from PIL import Image
import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asset_pipeline import key_out_magenta

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites"
OUT_DIR = SPRITE_DIR / "ksubmarine"
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

def process_keyed_sprite(src_path: Path, out_name: str, target_size=(128, 128)):
    if not src_path.exists():
        print(f"Skipping {src_path}, not found")
        return False
    img = Image.open(src_path)
    keyed = key_out_magenta(img)
    bbox = keyed.getbbox()
    if bbox:
        cropped = keyed.crop(bbox)
        max_dim = max(cropped.width, cropped.height)
        sq = Image.new("RGBA", (max_dim, max_dim), (0, 0, 0, 0))
        sq.paste(cropped, ((max_dim - cropped.width) // 2, (max_dim - cropped.height) // 2))
        res = sq.resize(target_size, Image.Resampling.LANCZOS)
    else:
        res = keyed.resize(target_size, Image.Resampling.LANCZOS)
    res.save(OUT_DIR / out_name, "PNG", optimize=True)
    write_atlas(out_name, target_size[0], target_size[1])
    print(f"Processed {out_name} ({target_size[0]}x{target_size[1]})")
    return True

# 1. Bathysphere Submarine (Imagen 3 generated)
sub_src = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\7da61514-76f8-4a12-9608-f9e8b70d3085\bathysphere_topdown_sprite_1793543516661.jpg")
process_keyed_sprite(sub_src, "sub_bathysphere.png", (128, 128))

# 2. Leviathan Sea Serpent (Imagen 3 generated)
lev_src = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\7da61514-76f8-4a12-9608-f9e8b70d3085\trench_leviathan_sprite_1793543546327.jpg")
process_keyed_sprite(lev_src, "leviathan_trench.png", (128, 128))

# 3. Abyssal Giant Squid (Imagen 3 generated)
squid_src = Path(r"C:\Users\mrbos\.gemini\antigravity-cli\brain\7da61514-76f8-4a12-9608-f9e8b70d3085\abyssal_squid_sprite_1793543615149.jpg")
process_keyed_sprite(squid_src, "squid_abyssal.png", (128, 128))

# 4. Salvage Pod from kstarforge/salvage_pod.png with yellow-hazard & ocean tint
salv_src = SPRITE_DIR / "kstarforge" / "salvage_pod.png"
if salv_src.exists():
    salv_img = Image.open(salv_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    salv_arr = np.array(salv_img, dtype=np.float32)
    # Brighten gold/hazard trim and deep sea rust
    salv_arr[:, :, 0] = np.clip(salv_arr[:, :, 0] * 1.25, 0, 255)
    salv_arr[:, :, 1] = np.clip(salv_arr[:, :, 1] * 1.15, 0, 255)
    salv_arr[:, :, 2] = np.clip(salv_arr[:, :, 2] * 0.85, 0, 255)
    salv_res = Image.fromarray(salv_arr.astype(np.uint8), "RGBA")
    salv_res.save(OUT_DIR / "salvage_pod.png", "PNG", optimize=True)
    write_atlas("salvage_pod.png", 128, 128)
    print("Processed salvage_pod.png")

# 5. Combat Drone from kstarforge/vessel_sentry.png
sentry_src = SPRITE_DIR / "kstarforge" / "vessel_sentry.png"
if sentry_src.exists():
    sen_img = Image.open(sentry_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    sen_arr = np.array(sen_img, dtype=np.float32)
    # Enhance crimson targeting ocular & stealth steel
    sen_arr[:, :, 0] = np.clip(sen_arr[:, :, 0] * 1.3, 0, 255)
    sen_arr[:, :, 1] = np.clip(sen_arr[:, :, 1] * 0.7, 0, 255)
    sen_arr[:, :, 2] = np.clip(sen_arr[:, :, 2] * 0.8, 0, 255)
    sen_res = Image.fromarray(sen_arr.astype(np.uint8), "RGBA")
    sen_res.save(OUT_DIR / "combat_drone.png", "PNG", optimize=True)
    write_atlas("combat_drone.png", 128, 128)
    print("Processed combat_drone.png")

# 6. Benthic Vent Flora from kalchemy/reagent_water.png
flora_src = SPRITE_DIR / "kalchemy" / "reagent_water.png"
if flora_src.exists():
    flo_img = Image.open(flora_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    flo_arr = np.array(flo_img, dtype=np.float32)
    # Bioluminescent cyan/emerald shift
    flo_arr[:, :, 0] = np.clip(flo_arr[:, :, 0] * 0.4, 0, 255)
    flo_arr[:, :, 1] = np.clip(flo_arr[:, :, 1] * 1.3, 0, 255)
    flo_arr[:, :, 2] = np.clip(flo_arr[:, :, 2] * 1.4, 0, 255)
    flo_res = Image.fromarray(flo_arr.astype(np.uint8), "RGBA")
    flo_res.save(OUT_DIR / "flora_benthic.png", "PNG", optimize=True)
    write_atlas("flora_benthic.png", 128, 128)
    print("Processed flora_benthic.png")

# 7. Abyssal Anglerfish Trench Fauna from kdragon/enemy_shadow.png
fauna_src = SPRITE_DIR / "kdragon" / "enemy_shadow.png"
if fauna_src.exists():
    fau_img = Image.open(fauna_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    fau_arr = np.array(fau_img, dtype=np.float32)
    # Violet/cyan bioluminescent shift
    fau_arr[:, :, 0] = np.clip(fau_arr[:, :, 0] * 0.9, 0, 255)
    fau_arr[:, :, 1] = np.clip(fau_arr[:, :, 1] * 0.8, 0, 255)
    fau_arr[:, :, 2] = np.clip(fau_arr[:, :, 2] * 1.3, 0, 255)
    fau_res = Image.fromarray(fau_arr.astype(np.uint8), "RGBA")
    fau_res.save(OUT_DIR / "fauna_abyssal.png", "PNG", optimize=True)
    write_atlas("fauna_abyssal.png", 128, 128)
    print("Processed fauna_abyssal.png")

# 8. Hydrothermal Black Smoker Chimney from kstarforge/asteroid_ferrite.png
vent_src = SPRITE_DIR / "kstarforge" / "asteroid_ferrite.png"
if vent_src.exists():
    vent_img = Image.open(vent_src).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)
    vent_arr = np.array(vent_img, dtype=np.float32)
    # Dark volcanic basalt & sulfur highlights
    vent_arr[:, :, 0] = np.clip(vent_arr[:, :, 0] * 0.6, 0, 255)
    vent_arr[:, :, 1] = np.clip(vent_arr[:, :, 1] * 0.65, 0, 255)
    vent_arr[:, :, 2] = np.clip(vent_arr[:, :, 2] * 0.55, 0, 255)
    vent_res = Image.fromarray(vent_arr.astype(np.uint8), "RGBA")
    vent_res.save(OUT_DIR / "vent_smoker.png", "PNG", optimize=True)
    write_atlas("vent_smoker.png", 128, 128)
    print("Processed vent_smoker.png")

print("All KSubmarine assets generated and indexed successfully.")
