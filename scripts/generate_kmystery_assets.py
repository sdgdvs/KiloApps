# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
Generates high-fidelity atmospheric noir sprites and backgrounds for KMystery.
Includes 5 suspect portraits, 5 atmospheric location backdrops, and 11 forensic clues.
"""

import os
import math
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites" / "kmystery"
BG_DIR = ROOT / "KiloOS" / "public" / "assets" / "backgrounds" / "kmystery"

SPRITE_DIR.mkdir(parents=True, exist_ok=True)
BG_DIR.mkdir(parents=True, exist_ok=True)


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


def save_sprite_with_atlas(img: Image.Image, filename: str):
    path = SPRITE_DIR / filename
    img.save(path, format="PNG", optimize=True)
    atlas = create_atlas(filename, img.width, img.height)
    atlas_path = SPRITE_DIR / f"{Path(filename).stem}.json"
    with open(atlas_path, "w", encoding="utf-8") as f:
        json.dump(atlas, f, indent=2)
    print(f"Saved sprite: {path} ({img.width}x{img.height})")


# =============================================================================
# 1. SUSPECT PORTRAITS (128x128 RGBA)
# =============================================================================

def generate_suspect_mr_black() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Dramatic back rim glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(212, 175, 55, alpha))

    # Overcoat silhouette (charcoal/black wool)
    d.polygon([(18, 128), (34, 82), (54, 76), (64, 84), (74, 76), (94, 82), (110, 128)], fill=(16, 17, 20, 255))
    # Lapels
    d.polygon([(40, 84), (58, 80), (54, 114), (36, 128)], fill=(28, 30, 36, 255))
    d.polygon([(88, 84), (70, 80), (74, 114), (92, 128)], fill=(24, 25, 30, 255))
    # Silk tie & collar
    d.polygon([(60, 82), (68, 82), (66, 120), (62, 120)], fill=(120, 20, 25, 255))
    d.polygon([(56, 78), (64, 84), (60, 92)], fill=(200, 195, 185, 255))
    d.polygon([(72, 78), (64, 84), (68, 92)], fill=(180, 175, 165, 255))

    # Head / jaw (warm shadow skin tone)
    d.ellipse([46, 42, 82, 78], fill=(145, 118, 96, 255))
    d.polygon([(48, 60), (80, 60), (70, 80), (58, 80)], fill=(130, 104, 84, 255))

    # Shadow across eyes / upper face
    d.rectangle([46, 48, 82, 63], fill=(20, 18, 22, 230))
    # Intense sharp eyes piercing from shadow
    d.rectangle([53, 55, 58, 58], fill=(240, 235, 220, 255))
    d.rectangle([70, 55, 75, 58], fill=(240, 235, 220, 255))
    d.rectangle([55, 55, 57, 58], fill=(40, 70, 100, 255))
    d.rectangle([72, 55, 74, 58], fill=(40, 70, 100, 255))

    # Stubble / jaw shadow
    d.line([(54, 74), (74, 74)], fill=(80, 65, 55, 180), width=2)
    d.line([(58, 70), (70, 70)], fill=(90, 75, 65, 200), width=1)

    # Fedora hat (classic gangster/detective trilby)
    # Crown
    d.polygon([(42, 44), (46, 20), (54, 16), (74, 16), (82, 20), (86, 44)], fill=(18, 19, 22, 255))
    # Crown crease / indent
    d.polygon([(54, 16), (64, 24), (74, 16)], fill=(10, 10, 12, 255))
    # Crimson silk hat band
    d.rectangle([43, 38, 85, 44], fill=(130, 24, 30, 255))
    d.line([(43, 44), (85, 44)], fill=(212, 175, 55, 200), width=1)
    # Wide curved brim tilted noir style
    d.ellipse([26, 38, 102, 52], fill=(22, 23, 27, 255))
    d.arc([26, 38, 102, 52], 10, 170, fill=(60, 62, 70, 255), width=2)

    # Cigarette with curling smoke
    d.line([(68, 73), (84, 76)], fill=(230, 225, 215, 255), width=2)
    d.point([(85, 76)], fill=(255, 80, 20, 255))
    # Smoke wisp
    smoke_pts = [(85, 76), (90, 70), (88, 62), (94, 54), (91, 44), (97, 34)]
    for i in range(len(smoke_pts) - 1):
        alpha = int(140 * (1 - i / len(smoke_pts)))
        d.line([smoke_pts[i], smoke_pts[i+1]], fill=(200, 205, 215, alpha), width=2)

    return im


def generate_suspect_miss_scarlet() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Warm crimson noir glow behind
    for r in range(48, 10, -4):
        alpha = int(40 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(180, 20, 40, alpha))

    # Crimson evening gown with sweetheart neckline & fur stole
    d.polygon([(20, 128), (38, 86), (52, 94), (64, 102), (76, 94), (90, 86), (108, 128)], fill=(140, 16, 32, 255))
    # Dark silk folds
    d.polygon([(46, 98), (64, 106), (82, 98), (76, 128), (52, 128)], fill=(110, 10, 24, 255))
    # Black mink fur stole around shoulders
    d.ellipse([22, 78, 56, 114], fill=(24, 20, 22, 255))
    d.ellipse([72, 78, 106, 114], fill=(24, 20, 22, 255))
    d.ellipse([34, 72, 94, 96], fill=(30, 25, 28, 255))

    # Elegant porcelain neck and face
    d.ellipse([46, 42, 82, 80], fill=(225, 185, 168, 255))
    d.polygon([(52, 72), (76, 72), (68, 88), (60, 88)], fill=(215, 172, 154, 255))

    # Raven black wavy bob hair
    d.ellipse([40, 36, 60, 78], fill=(18, 15, 20, 255))
    d.ellipse([68, 36, 88, 78], fill=(18, 15, 20, 255))
    d.ellipse([42, 28, 86, 56], fill=(18, 15, 20, 255))

    # Sultry eyes with winged liner and long lashes
    d.arc([51, 52, 61, 58], 190, 350, fill=(20, 10, 15, 255), width=2)
    d.ellipse([53, 54, 59, 58], fill=(40, 120, 70, 255)) # Emerald green eyes
    d.arc([67, 52, 77, 58], 190, 350, fill=(20, 10, 15, 255), width=2)
    d.ellipse([69, 54, 75, 58], fill=(40, 120, 70, 255))
    # Delicate arched eyebrows
    d.arc([49, 47, 61, 53], 190, 340, fill=(35, 25, 25, 255), width=2)
    d.arc([67, 47, 79, 53], 200, 350, fill=(35, 25, 25, 255), width=2)

    # Vivid ruby lips
    d.polygon([(58, 71), (64, 69), (70, 71), (67, 76), (61, 76)], fill=(195, 18, 38, 255))
    d.point([(64, 71)], fill=(255, 120, 140, 255))

    # Ornate vintage cloche hat tilted with veil
    d.ellipse([34, 24, 94, 52], fill=(160, 20, 42, 255))
    d.arc([34, 24, 94, 52], 10, 170, fill=(200, 40, 65, 255), width=2)
    # Gold brooch & emerald feather pin
    d.ellipse([76, 32, 84, 40], fill=(212, 175, 55, 255))
    d.polygon([(82, 34), (96, 20), (90, 38)], fill=(20, 140, 80, 255))
    # Fishnet veil pattern across brow
    for vx in range(48, 80, 6):
        d.line([(vx, 44), (vx + 6, 56)], fill=(20, 15, 25, 120), width=1)
        d.line([(vx + 6, 44), (vx, 56)], fill=(20, 15, 25, 120), width=1)

    # Emerald drop earrings
    d.ellipse([43, 68, 47, 76], fill=(20, 180, 90, 255))
    d.point([(45, 69)], fill=(200, 255, 220, 255))
    d.ellipse([81, 68, 85, 76], fill=(20, 180, 90, 255))

    return im


def generate_suspect_colonel_mustard() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Amber rim glow
    for r in range(48, 10, -4):
        alpha = int(40 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(212, 160, 40, alpha))

    # Khaki/Olive military service dress uniform
    d.polygon([(18, 128), (34, 82), (54, 76), (64, 80), (74, 76), (94, 82), (110, 128)], fill=(78, 70, 44, 255))
    # High standing collar with gold piping
    d.polygon([(52, 74), (76, 74), (72, 86), (56, 86)], fill=(62, 56, 34, 255))
    d.line([(52, 74), (76, 74)], fill=(212, 175, 55, 255), width=2)
    # Gold braided epaulets on shoulders
    d.ellipse([26, 80, 46, 92], fill=(200, 165, 45, 255))
    d.ellipse([82, 80, 102, 92], fill=(200, 165, 45, 255))
    # Medal ribbons on chest (red, blue, gold stripes)
    medals = [(38, 98, 46, 104, (180, 30, 30)), (48, 98, 56, 104, (30, 80, 160)), (58, 98, 66, 104, (212, 175, 55)),
              (38, 106, 46, 112, (20, 120, 60)), (48, 106, 56, 112, (220, 180, 50)), (58, 106, 66, 112, (120, 30, 140))]
    for x1, y1, x2, y2, c in medals:
        d.rectangle([x1, y1, x2, y2], fill=c + (255,))
        d.rectangle([x1, y1, x2, y2], outline=(30, 30, 30, 180), width=1)

    # Weathered square jaw, ruddy complexion
    d.ellipse([46, 42, 82, 78], fill=(195, 155, 130, 255))
    d.polygon([(48, 62), (80, 62), (74, 82), (54, 82)], fill=(180, 140, 115, 255))

    # Stern narrow gray eyes
    d.line([(51, 56), (61, 56)], fill=(40, 35, 30, 255), width=2)
    d.ellipse([54, 55, 58, 58], fill=(90, 110, 120, 255))
    d.line([(67, 56), (77, 56)], fill=(40, 35, 30, 255), width=2)
    d.ellipse([70, 55, 74, 58], fill=(90, 110, 120, 255))
    # Bushy furrowed brows
    d.polygon([(48, 52), (62, 54), (60, 51)], fill=(210, 205, 195, 255))
    d.polygon([(80, 52), (66, 54), (68, 51)], fill=(210, 205, 195, 255))

    # Magnificent waxed handlebar mustache (silver-white)
    d.polygon([(52, 67), (64, 68), (76, 67), (82, 63), (76, 73), (64, 74), (52, 73), (46, 63)], fill=(235, 230, 220, 255))
    d.line([(46, 63), (54, 70)], fill=(245, 240, 230, 255), width=2)
    d.line([(82, 63), (74, 70)], fill=(245, 240, 230, 255), width=2)

    # Officer peaked service cap
    # Cap crown
    d.polygon([(40, 42), (44, 22), (64, 18), (84, 22), (88, 42)], fill=(86, 78, 50, 255))
    # Gold officer chin strap & cap badge
    d.rectangle([41, 38, 87, 43], fill=(212, 175, 55, 255))
    d.ellipse([60, 26, 68, 36], fill=(225, 185, 60, 255)) # Crown crest
    # Shiny black patent leather visor
    d.ellipse([34, 40, 94, 54], fill=(15, 15, 18, 255))
    d.arc([36, 42, 92, 52], 20, 160, fill=(180, 180, 200, 255), width=2) # Visor specular sheen

    # Monocle over left eye
    d.ellipse([68, 52, 77, 61], outline=(212, 175, 55, 255), width=2)
    d.line([(77, 58), (86, 84)], fill=(212, 175, 55, 200), width=1) # Monocle chain

    return im


def generate_suspect_mrs_white() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Pale silvery noir back glow
    for r in range(48, 10, -4):
        alpha = int(35 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(160, 170, 190, alpha))

    # Severe Victorian black mourning dress
    d.polygon([(20, 128), (38, 86), (54, 78), (64, 82), (74, 78), (90, 86), (108, 128)], fill=(18, 17, 20, 255))
    # Stiff pleated white lace bib collar
    d.polygon([(52, 80), (76, 80), (70, 104), (64, 110), (58, 104)], fill=(235, 230, 225, 255))
    d.polygon([(52, 80), (76, 80), (70, 104), (64, 110), (58, 104)], outline=(180, 175, 170, 255), width=1)
    # Jet black mourning cameo brooch at throat
    d.ellipse([60, 84, 68, 94], fill=(10, 10, 12, 255))
    d.ellipse([60, 84, 68, 94], outline=(212, 175, 55, 255), width=1)
    d.ellipse([62, 87, 66, 91], fill=(220, 215, 205, 255)) # Cameo silhouette

    # Triple strand pearl necklace
    for py, r in [(98, 18), (103, 22), (108, 26)]:
        for deg in range(30, 150, 15):
            rad = math.radians(deg)
            px = 64 + math.cos(rad) * r
            p_y = 80 + math.sin(rad) * r * 0.7
            d.ellipse([px - 2, p_y - 2, px + 2, p_y + 2], fill=(245, 245, 250, 255))

    # Pale aristocratic austere face
    d.ellipse([46, 40, 82, 78], fill=(215, 190, 175, 255))
    d.polygon([(48, 62), (80, 62), (72, 82), (56, 82)], fill=(200, 175, 160, 255))

    # Severe silver-gray hair tightly coiled into Victorian bun
    d.ellipse([40, 30, 88, 56], fill=(160, 155, 160, 255))
    d.ellipse([54, 18, 74, 34], fill=(145, 140, 145, 255)) # Top chignon bun
    d.arc([54, 18, 74, 34], 0, 360, fill=(190, 185, 190, 255), width=1)

    # Cold hooded observant eyes
    d.arc([50, 52, 60, 58], 190, 350, fill=(30, 25, 30, 255), width=2)
    d.ellipse([53, 54, 57, 58], fill=(70, 85, 105, 255))
    d.arc([68, 52, 78, 58], 190, 350, fill=(30, 25, 30, 255), width=2)
    d.ellipse([71, 54, 75, 58], fill=(70, 85, 105, 255))

    # Thin pinched pursed lips
    d.line([(58, 72), (70, 72)], fill=(160, 110, 110, 255), width=2)
    d.line([(56, 73), (58, 72)], fill=(130, 90, 90, 255), width=1)
    d.line([(70, 72), (72, 73)], fill=(130, 90, 90, 255), width=1)

    # Black sheer mourning veil draped over forehead
    d.polygon([(42, 34), (86, 34), (88, 48), (40, 48)], fill=(25, 20, 28, 170))
    for vx in range(42, 86, 5):
        d.line([(vx, 36), (vx + 4, 46)], fill=(40, 35, 45, 220), width=1)

    return im


def generate_suspect_professor_plum() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Violet/Indigo academic noir glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(90, 40, 120, alpha))

    # Brown herringbone tweed professor jacket
    d.polygon([(18, 128), (34, 82), (54, 76), (64, 82), (74, 76), (94, 82), (110, 128)], fill=(65, 52, 42, 255))
    # Herringbone weave texture accents
    for y in range(86, 126, 8):
        for x in range(24, 104, 12):
            d.line([(x, y), (x + 4, y + 4)], fill=(85, 70, 58, 160), width=1)
            d.line([(x + 4, y + 4), (x + 8, y)], fill=(85, 70, 58, 160), width=1)

    # Plum velvet waistcoat & crisp white shirt
    d.polygon([(46, 88), (64, 96), (82, 88), (78, 128), (50, 128)], fill=(75, 25, 80, 255))
    d.polygon([(56, 78), (64, 84), (72, 78), (68, 92), (60, 92)], fill=(230, 225, 220, 255))
    # Ornate plum silk butterfly bowtie
    d.polygon([(54, 82), (64, 86), (54, 90)], fill=(120, 30, 110, 255))
    d.polygon([(74, 82), (64, 86), (74, 90)], fill=(120, 30, 110, 255))
    d.ellipse([62, 84, 66, 88], fill=(150, 40, 140, 255))

    # Scholarly gaunt face, high intellectual forehead
    d.ellipse([46, 38, 82, 78], fill=(210, 175, 150, 255))
    d.polygon([(48, 62), (80, 62), (72, 82), (56, 82)], fill=(195, 160, 135, 255))

    # Receding wavy brown/gray academic hair, wild at temples
    d.ellipse([38, 34, 52, 68], fill=(70, 55, 45, 255))
    d.ellipse([76, 34, 90, 68], fill=(70, 55, 45, 255))
    d.ellipse([44, 26, 84, 46], fill=(85, 70, 58, 255))

    # Calculating, scheming eyes magnified behind thick circular spectacles
    # Round tortoiseshell & gold spectacles
    d.ellipse([48, 50, 62, 64], outline=(212, 175, 55, 255), width=2)
    d.ellipse([66, 50, 80, 64], outline=(212, 175, 55, 255), width=2)
    d.line([(62, 57), (66, 57)], fill=(212, 175, 55, 255), width=2) # Bridge
    # Glass refraction highlight
    d.arc([49, 51, 61, 63], 210, 310, fill=(230, 245, 255, 220), width=1)
    d.arc([67, 51, 79, 63], 210, 310, fill=(230, 245, 255, 220), width=1)
    # Dark sharp pupils
    d.ellipse([53, 55, 57, 59], fill=(30, 20, 25, 255))
    d.ellipse([71, 55, 75, 59], fill=(30, 20, 25, 255))

    # Smug knowing smirk
    d.arc([56, 68, 72, 76], 10, 160, fill=(140, 85, 80, 255), width=2)

    return im


# =============================================================================
# 2. FORENSIC CLUE SPRITES (128x128 RGBA)
# =============================================================================

def generate_clue_cufflink() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Warm gold forensic light pool
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(212, 175, 55, alpha))

    # Heavy gold octagonal setting
    cx, cy, radius = 64, 64, 42
    pts_outer = []
    for i in range(8):
        angle = math.radians(i * 45 + 22.5)
        pts_outer.append((cx + math.cos(angle) * radius, cy + math.sin(angle) * radius))
    d.polygon(pts_outer, fill=(212, 175, 55, 255), outline=(245, 215, 95, 255))

    # Inner beveled gold rim
    pts_inner = []
    for i in range(8):
        angle = math.radians(i * 45 + 22.5)
        pts_inner.append((cx + math.cos(angle) * 36, cy + math.sin(angle) * 36))
    d.polygon(pts_inner, fill=(160, 125, 30, 255))

    # Deep glossy polished onyx facet
    pts_onyx = []
    for i in range(8):
        angle = math.radians(i * 45 + 22.5)
        pts_onyx.append((cx + math.cos(angle) * 32, cy + math.sin(angle) * 32))
    d.polygon(pts_onyx, fill=(18, 18, 22, 255))

    # Specular glass reflection bar across onyx
    d.polygon([(42, 48), (62, 38), (86, 62), (66, 72)], fill=(255, 255, 255, 55))
    d.line([(44, 44), (58, 38)], fill=(255, 255, 255, 180), width=2)

    # Silver engraved monogram crest (Initials "B")
    d.line([(56, 50), (56, 78)], fill=(225, 225, 235, 255), width=3)
    d.arc([54, 50, 72, 64], 270, 90, fill=(225, 225, 235, 255), width=2)
    d.arc([54, 64, 74, 78], 270, 90, fill=(225, 225, 235, 255), width=2)

    # Gold swivel toggle post visible behind
    d.rectangle([58, 102, 70, 114], fill=(180, 140, 35, 255), outline=(120, 90, 20, 255))
    d.line([(64, 96), (64, 106)], fill=(212, 175, 55, 255), width=4)

    return im


def generate_clue_perfume() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Rose red ambient glow
    for r in range(48, 10, -4):
        alpha = int(40 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(220, 30, 70, alpha))

    # Cut-crystal flacon bottle (faceted perfume decanter)
    body_pts = [(40, 108), (88, 108), (96, 68), (84, 50), (64, 46), (44, 50), (32, 68)]
    d.polygon(body_pts, fill=(180, 25, 60, 180), outline=(235, 200, 210, 220))
    # Perfume liquid level inside
    liquid_pts = [(42, 106), (86, 106), (92, 76), (36, 76)]
    d.polygon(liquid_pts, fill=(195, 20, 50, 230))

    # Crystal facet cut lines & light refraction
    d.line([(40, 108), (64, 46)], fill=(255, 255, 255, 120), width=2)
    d.line([(88, 108), (64, 46)], fill=(255, 255, 255, 80), width=2)
    d.line([(64, 108), (64, 46)], fill=(255, 255, 255, 160), width=2)
    d.line([(32, 68), (96, 68)], fill=(255, 255, 255, 90), width=1)

    # Ornate vintage brass neck & nozzle
    d.rectangle([58, 34, 70, 48], fill=(212, 175, 55, 255), outline=(245, 215, 95, 255))
    d.line([(54, 38), (74, 38)], fill=(245, 215, 95, 255), width=2)

    # Atomizer pump tube & burgundy woven silk squeeze bulb
    tube_pts = [(70, 38), (86, 36), (98, 44), (102, 58)]
    for i in range(len(tube_pts) - 1):
        d.line([tube_pts[i], tube_pts[i+1]], fill=(180, 140, 40, 255), width=3)
    # Silk atomizer bulb with fringe tassel
    d.ellipse([92, 54, 114, 82], fill=(130, 20, 36, 255), outline=(90, 10, 24, 255))
    # Tassel
    d.polygon([(103, 82), (97, 104), (109, 104)], fill=(160, 25, 45, 255))

    # Gold spray nozzle mist droplets
    d.point([(48, 36), (42, 34), (40, 40), (34, 36), (32, 44)], fill=(255, 200, 220, 240))

    return im


def generate_clue_button() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Brass forensic glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(200, 150, 40, alpha))

    # Heavy convex brass military button
    d.ellipse([26, 26, 102, 102], fill=(160, 120, 35, 255), outline=(225, 190, 75, 255))
    # Outer rope-pattern border rim
    d.ellipse([32, 32, 96, 96], outline=(100, 75, 20, 255), width=3)
    d.ellipse([36, 36, 92, 92], fill=(185, 140, 45, 255))

    # Raised embossed fouled-anchor military crest
    # Anchor stock (crossbar)
    d.rectangle([46, 52, 82, 58], fill=(235, 205, 95, 255), outline=(90, 70, 20, 255))
    # Anchor ring at top
    d.ellipse([58, 40, 70, 52], outline=(235, 205, 95, 255), width=3)
    # Anchor shank (vertical post)
    d.rectangle([61, 52, 67, 88], fill=(235, 205, 95, 255))
    # Anchor arms & flukes (curved bottom)
    d.arc([42, 60, 86, 92], 20, 160, fill=(235, 205, 95, 255), width=5)
    d.polygon([(40, 64), (44, 60), (48, 66)], fill=(235, 205, 95, 255))
    d.polygon([(88, 64), (84, 60), (80, 66)], fill=(235, 205, 95, 255))
    # Fouled rope spiraling anchor
    d.arc([56, 50, 72, 66], 40, 220, fill=(120, 95, 35, 255), width=2)
    d.arc([56, 68, 72, 84], 40, 220, fill=(120, 95, 35, 255), width=2)

    # Frayed dark wool threads torn from uniform
    threads = [(64, 104), (58, 114), (66, 120), (74, 112), (70, 124)]
    for i in range(len(threads) - 1):
        d.line([threads[i], threads[i+1]], fill=(60, 55, 35, 255), width=2)
    d.line([(60, 104), (52, 118)], fill=(45, 40, 25, 255), width=2)

    return im


def generate_clue_glove() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Silvery noir glow
    for r in range(48, 10, -4):
        alpha = int(35 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(200, 210, 225, alpha))

    # White lace evening glove (wrist to fingers)
    # Palm and wrist
    d.polygon([(46, 116), (82, 116), (84, 76), (44, 76)], fill=(240, 238, 235, 240))
    # Ruffled lace wrist cuff
    for rx in range(42, 86, 6):
        d.ellipse([rx, 112, rx + 8, 122], fill=(255, 255, 255, 255), outline=(190, 185, 180, 255))

    # Thumb
    d.polygon([(44, 82), (32, 64), (36, 56), (48, 74)], fill=(240, 238, 235, 240), outline=(200, 195, 190, 255))
    # Index finger
    d.polygon([(48, 74), (48, 38), (56, 38), (56, 74)], fill=(240, 238, 235, 240), outline=(200, 195, 190, 255))
    # Middle finger (Torn fingertip showing forensic blood trace)
    d.polygon([(57, 74), (57, 30), (66, 30), (66, 74)], fill=(240, 238, 235, 240), outline=(200, 195, 190, 255))
    # Torn jagged hole at tip of middle finger
    d.polygon([(56, 32), (61, 26), (67, 33), (62, 36)], fill=(160, 20, 30, 255)) # Blood trace
    # Ring finger
    d.polygon([(67, 74), (67, 36), (75, 36), (75, 74)], fill=(240, 238, 235, 240), outline=(200, 195, 190, 255))
    # Little finger
    d.polygon([(76, 76), (76, 46), (83, 46), (83, 76)], fill=(240, 238, 235, 240), outline=(200, 195, 190, 255))

    # Intricate lace embroidery pattern across palm
    for ly in range(80, 112, 6):
        for lx in range(48, 80, 8):
            d.line([(lx, ly), (lx + 4, ly + 4)], fill=(215, 210, 205, 200), width=1)
            d.line([(lx + 4, ly + 4), (lx + 8, ly)], fill=(215, 210, 205, 200), width=1)

    return im


def generate_clue_spectacles() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Amber gold glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(212, 175, 55, alpha))

    # Vintage round horn-rimmed / gold wire spectacles
    # Right intact lens
    d.ellipse([70, 42, 110, 82], outline=(212, 175, 55, 255), width=3)
    d.ellipse([72, 44, 108, 80], fill=(210, 235, 255, 60))
    d.arc([74, 46, 106, 78], 200, 300, fill=(255, 255, 255, 180), width=2) # Glass reflection

    # Left broken / cracked lens
    d.ellipse([18, 42, 58, 82], outline=(212, 175, 55, 255), width=3)
    d.ellipse([20, 44, 56, 80], fill=(210, 235, 255, 50))
    # Spiderweb hairline fracture lines
    crack_hub = (38, 62)
    crack_endpoints = [(20, 50), (28, 44), (48, 46), (56, 56), (52, 76), (36, 80), (22, 70)]
    for ep in crack_endpoints:
        d.line([crack_hub, ep], fill=(255, 255, 255, 220), width=1)
    # Secondary splinter lines
    d.line([(28, 54), (34, 48)], fill=(255, 255, 255, 180), width=1)
    d.line([(46, 68), (52, 62)], fill=(255, 255, 255, 180), width=1)

    # Gold nose bridge & nose pads
    d.arc([54, 54, 74, 70], 200, 340, fill=(212, 175, 55, 255), width=3)
    d.line([(57, 64), (56, 70)], fill=(245, 215, 95, 255), width=2)
    d.line([(71, 64), (72, 70)], fill=(245, 215, 95, 255), width=2)

    # Folded wire ear temple arms
    d.line([(18, 62), (6, 74)], fill=(180, 140, 40, 255), width=2)
    d.line([(110, 62), (122, 74)], fill=(180, 140, 40, 255), width=2)

    return im


def generate_clue_shell() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Gunmetal amber glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(200, 140, 30, alpha))

    # Spent .38 Special brass cartridge casing (isometric tilted view)
    # Cylindrical case body
    d.polygon([(46, 28), (82, 28), (82, 94), (46, 94)], fill=(195, 150, 45, 255))
    # Metallic specular cylinder gradient lines
    d.line([(52, 28), (52, 94)], fill=(250, 220, 110, 255), width=3)
    d.line([(74, 28), (74, 94)], fill=(140, 100, 25, 255), width=3)

    # Rim extractor groove
    d.rectangle([44, 94, 84, 100], fill=(150, 110, 30, 255))
    # Cartridge rim base
    d.ellipse([42, 98, 86, 112], fill=(215, 170, 55, 255), outline=(130, 95, 25, 255))

    # Case mouth open at top with gunpowder scorch
    d.ellipse([46, 22, 82, 34], fill=(25, 25, 28, 255), outline=(130, 95, 25, 255))
    # Black burnt soot powder residue around lip
    d.ellipse([44, 20, 84, 38], outline=(15, 15, 18, 230), width=2)

    # Headstamp caliber text embossed on casing body
    d.text((50, 54), ".38", fill=(110, 75, 20, 255))
    d.text((49, 66), "SPL", fill=(110, 75, 20, 255))

    return im


def generate_clue_vial() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Eerie toxic green chemical glow
    for r in range(48, 10, -4):
        alpha = int(50 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(40, 220, 100, alpha))

    # Dark apothecary glass vial body
    d.rectangle([46, 44, 82, 112], fill=(25, 45, 30, 160), outline=(180, 240, 200, 200))
    # Rounded base
    d.ellipse([46, 104, 82, 116], fill=(20, 40, 25, 200), outline=(180, 240, 200, 200))

    # Glowing toxic luminous green liquid inside
    d.rectangle([48, 64, 80, 108], fill=(35, 215, 95, 220))
    d.ellipse([48, 60, 80, 68], fill=(50, 240, 120, 255)) # Liquid meniscus
    # Rising bubbles inside venom
    d.ellipse([54, 92, 58, 96], fill=(180, 255, 210, 255))
    d.ellipse([68, 80, 74, 86], fill=(180, 255, 210, 255))
    d.ellipse([60, 70, 64, 74], fill=(180, 255, 210, 255))

    # Aged paper poison label with skull symbol
    d.rectangle([49, 72, 79, 98], fill=(235, 225, 205, 255), outline=(130, 100, 60, 255))
    # Skull
    d.ellipse([60, 75, 68, 83], fill=(20, 20, 20, 255))
    d.rectangle([62, 82, 66, 86], fill=(20, 20, 20, 255))
    d.line([(58, 88), (70, 80)], fill=(20, 20, 20, 255), width=2)
    d.line([(58, 80), (70, 88)], fill=(20, 20, 20, 255), width=2)

    # Vial neck & rim
    d.rectangle([54, 28, 74, 44], fill=(25, 45, 30, 200), outline=(180, 240, 200, 200))
    d.ellipse([52, 24, 76, 32], fill=(20, 40, 25, 220), outline=(180, 240, 200, 200))

    # Weathered cork stopper
    d.polygon([(56, 14), (72, 14), (70, 26), (58, 26)], fill=(175, 125, 75, 255), outline=(110, 75, 40, 255))

    return im


def generate_clue_pipe() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Cold steel crime scene glow
    for r in range(48, 10, -4):
        alpha = int(40 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(140, 150, 170, alpha))

    # Heavy industrial lead pipe segment (diagonal angled bar)
    p_pts = [(24, 96), (92, 28), (106, 42), (38, 110)]
    d.polygon(p_pts, fill=(65, 68, 75, 255), outline=(120, 125, 135, 255))
    # Cylindrical metallic reflection line
    d.line([(30, 94), (96, 28)], fill=(160, 165, 178, 255), width=3)
    d.line([(34, 106), (102, 38)], fill=(40, 42, 48, 255), width=3)

    # Threaded coupler collar at top right
    c_pts = [(82, 24), (102, 14), (114, 34), (96, 46)]
    d.polygon(c_pts, fill=(90, 94, 102, 255), outline=(140, 145, 155, 255))
    d.line([(86, 22), (100, 40)], fill=(45, 48, 55, 255), width=2)
    d.line([(92, 18), (106, 36)], fill=(45, 48, 55, 255), width=2)

    # Dark dried suspicious bloodstain on blunt striking end
    b_pts = [(22, 94), (32, 84), (46, 98), (36, 112)]
    d.polygon(b_pts, fill=(110, 15, 20, 230))
    d.line([(34, 110), (32, 122)], fill=(90, 10, 15, 255), width=2) # Drip
    d.point([(32, 124)], fill=(120, 20, 25, 255))

    return im


def generate_clue_letter() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Sepia parchment glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(212, 180, 120, alpha))

    # Crumpled, partially burned blackmail letter
    p_pts = [(30, 22), (98, 18), (108, 98), (84, 108), (44, 102), (22, 78)]
    d.polygon(p_pts, fill=(235, 225, 200, 255), outline=(170, 150, 120, 255))

    # Charred burned ragged right edge
    burn_pts = [(98, 18), (108, 38), (102, 54), (112, 74), (108, 98), (92, 86), (90, 40)]
    d.polygon(burn_pts, fill=(35, 28, 25, 240))
    d.line([(98, 18), (108, 38), (102, 54), (112, 74), (108, 98)], fill=(190, 60, 20, 255), width=2) # Glowing ember fringe

    # Typed typewriter courier text lines
    for y in range(32, 92, 8):
        w_len = 54 if y < 70 else 38
        d.line([(34, y), (34 + w_len, y)], fill=(45, 40, 38, 220), width=2)

    # Red exclamation / stamp mark "PAY BY MIDNIGHT"
    d.line([(34, 76), (72, 76)], fill=(180, 25, 30, 255), width=3)
    d.text((36, 68), "DEADLINE", fill=(180, 25, 30, 255))

    # Diagonal creases across paper
    d.line([(30, 22), (84, 108)], fill=(190, 175, 150, 140), width=1)
    d.line([(22, 78), (98, 18)], fill=(190, 175, 150, 140), width=1)

    return im


def generate_clue_will() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Wealthy amber legal glow
    for r in range(48, 10, -4):
        alpha = int(45 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(212, 175, 55, alpha))

    # Formal parchment document
    d.rectangle([28, 20, 100, 108], fill=(245, 240, 225, 255), outline=(180, 160, 130, 255))
    # Fancy legal document border frame
    d.rectangle([32, 24, 96, 104], outline=(212, 175, 55, 200), width=1)

    # Header Gothic Title "LAST WILL"
    d.rectangle([42, 28, 86, 36], fill=(30, 25, 25, 255))
    d.text((44, 26), "LAST WILL", fill=(245, 240, 225, 255))

    # Testament text lines
    for y in range(42, 82, 6):
        d.line([(38, y), (90, y)], fill=(60, 55, 50, 180), width=1)

    # Signature line
    d.line([(38, 88), (68, 88)], fill=(40, 35, 30, 255), width=1)
    d.arc([42, 82, 64, 88], 0, 180, fill=(30, 30, 60, 255), width=1) # Cursive quill signature

    # Heavy crimson wax seal with silk ribbon
    # Blue silk notary ribbon tails
    d.polygon([(78, 88), (74, 116), (82, 112)], fill=(30, 60, 140, 255))
    d.polygon([(82, 88), (86, 116), (78, 112)], fill=(40, 80, 180, 255))
    # Red wax seal stamp
    d.ellipse([70, 82, 92, 104], fill=(160, 20, 28, 255), outline=(210, 30, 40, 255))
    d.ellipse([75, 87, 87, 99], fill=(130, 15, 22, 255))
    d.ellipse([78, 90, 84, 96], fill=(200, 40, 50, 255)) # Embossed crest

    return im


def generate_clue_photo() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Noir vintage glow
    for r in range(48, 10, -4):
        alpha = int(40 * (1 - r / 48))
        d.ellipse([64 - r, 64 - r, 64 + r, 64 + r], fill=(180, 160, 140, alpha))

    # Gelatin silver torn photograph (vintage 1940s border)
    # White card border
    photo_pts = [(24, 24), (92, 20), (96, 52), (104, 62), (94, 84), (100, 106), (28, 104)]
    d.polygon(photo_pts, fill=(240, 235, 225, 255), outline=(190, 180, 165, 255))

    # Inner image area (sepia tones)
    img_pts = [(30, 30), (86, 27), (90, 54), (98, 64), (88, 82), (92, 98), (34, 98)]
    d.polygon(img_pts, fill=(65, 52, 42, 255))

    # Sepia backdrop: balcony railing overlooking city skyline
    d.rectangle([34, 76, 88, 96], fill=(45, 36, 28, 255)) # Balcony
    d.line([(34, 76), (88, 76)], fill=(120, 100, 80, 255), width=2) # Railing
    for bx in range(38, 86, 8):
        d.line([(bx, 76), (bx, 96)], fill=(100, 85, 68, 255), width=1)

    # Silhouette of victim & suspect embracing on balcony
    d.ellipse([46, 46, 58, 60], fill=(25, 20, 16, 255)) # Head 1
    d.polygon([(42, 60), (62, 60), (64, 76), (40, 76)], fill=(20, 16, 12, 255))
    d.ellipse([64, 48, 76, 62], fill=(25, 20, 16, 255)) # Head 2 (torn right near seam!)
    d.polygon([(62, 62), (78, 62), (80, 76), (60, 76)], fill=(20, 16, 12, 255))

    # Violent red "X" scratched across victim's face
    d.line([(44, 44), (60, 62)], fill=(220, 30, 40, 240), width=3)
    d.line([(60, 44), (44, 62)], fill=(220, 30, 40, 240), width=3)

    # Jagged torn zigzag edge on right side of photograph
    d.line([(92, 20), (86, 38), (96, 52), (88, 66), (104, 82), (94, 96), (100, 106)], fill=(255, 255, 255, 255), width=2)

    return im


# =============================================================================
# 3. NOIR LOCATION BACKDROPS (640x360 RGB PNG)
# =============================================================================

def generate_bg_noir_office() -> Image.Image:
    w, h = 640, 360
    im = Image.new("RGB", (w, h), (14, 12, 10))
    d = ImageDraw.Draw(im)

    # 1. Dark room gradient
    for y in range(h):
        factor = y / h
        r = int(18 * (1 - factor * 0.6))
        g = int(14 * (1 - factor * 0.6))
        b = int(12 * (1 - factor * 0.5))
        d.line([(0, y), (w, y)], fill=(r, g, b))

    # 2. Window to rainy nocturnal cityscape
    win_x1, win_y1, win_x2, win_y2 = int(w * 0.15), 20, int(w * 0.85), int(h * 0.72)
    d.rectangle([win_x1, win_y1, win_x2, win_y2], fill=(6, 8, 14))

    # Distant skyscrapers through rain
    d.rectangle([win_x1 + 40, win_y1 + 40, win_x1 + 100, win_y2], fill=(12, 15, 22))
    d.rectangle([win_x1 + 120, win_y1 + 20, win_x1 + 200, win_y2], fill=(16, 20, 28))
    d.rectangle([win_x1 + 220, win_y1 + 50, win_x1 + 280, win_y2], fill=(10, 14, 20))
    d.rectangle([win_x1 + 310, win_y1 + 10, win_x1 + 390, win_y2], fill=(18, 22, 32))
    # Distant yellow window dots in buildings
    for wy in range(win_y1 + 40, win_y2 - 20, 14):
        for wx in range(win_x1 + 130, win_x1 + 190, 16):
            d.rectangle([wx, wy, wx + 6, wy + 8], fill=(220, 180, 60))

    # Heavy mahogany window frame
    d.rectangle([win_x1 - 8, win_y1 - 8, win_x2 + 8, win_y2 + 8], outline=(28, 22, 16), width=10)
    d.line([(win_x1, (win_y1 + win_y2) // 2), (win_x2, (win_y1 + win_y2) // 2)], fill=(28, 22, 16), width=6)
    d.line([((win_x1 + win_x2) // 2, win_y1), ((win_x1 + win_x2) // 2, win_y2)], fill=(28, 22, 16), width=6)

    # 3. Venetian Blinds casting diagonal shadow slats across the entire room
    for sy in range(0, h, 20):
        poly = [(0, sy + 30), (w, sy - 40), (w, sy - 26), (0, sy + 44)]
        d.polygon(poly, fill=(212, 175, 55, 30))

    # 4. Detective's polished mahogany desk
    desk_y = int(h * 0.70)
    d.rectangle([0, desk_y, w, h], fill=(22, 18, 14))
    d.line([(0, desk_y), (w, desk_y)], fill=(48, 38, 28), width=4)

    # 5. Banker's Lamp (emerald green glass shade casting glowing green light pool)
    lamp_x, lamp_y = int(w * 0.22), desk_y
    # Cast light cone
    light_cone = [(lamp_x, lamp_y - 35), (lamp_x - 120, h), (lamp_x + 120, h)]
    d.polygon(light_cone, fill=(35, 120, 60))
    # Brass base & stem
    d.rectangle([lamp_x - 4, lamp_y - 35, lamp_x + 4, lamp_y], fill=(190, 150, 40))
    d.ellipse([lamp_x - 18, lamp_y - 6, lamp_x + 18, lamp_y + 4], fill=(160, 120, 30))
    # Green glass shade
    d.ellipse([lamp_x - 28, lamp_y - 48, lamp_x + 28, lamp_y - 32], fill=(20, 140, 50), outline=(50, 200, 90))
    # Glowing yellow bulb filament inside
    d.ellipse([lamp_x - 8, lamp_y - 38, lamp_x + 8, lamp_y - 28], fill=(255, 240, 180))

    # 6. Props on desk
    # Manila case file folder stack with red tape
    d.rectangle([int(w * 0.44), desk_y + 10, int(w * 0.58), desk_y + 45], fill=(175, 145, 95), outline=(100, 80, 50))
    d.rectangle([int(w * 0.46), desk_y + 6, int(w * 0.57), desk_y + 40], fill=(195, 165, 110), outline=(100, 80, 50))
    d.rectangle([int(w * 0.48), desk_y + 16, int(w * 0.52), desk_y + 22], fill=(180, 30, 40)) # "CONFIDENTIAL" red tape

    # Brass magnifying glass resting on files
    mg_x, mg_y = int(w * 0.52), desk_y + 24
    d.ellipse([mg_x - 14, mg_y - 14, mg_x + 14, mg_y + 14], outline=(212, 175, 55), width=3)
    d.ellipse([mg_x - 12, mg_y - 12, mg_x + 12, mg_y + 12], fill=(200, 230, 255))
    d.line([(mg_x + 10, mg_y + 10), (mg_x + 28, mg_y + 28)], fill=(120, 90, 30), width=4) # Wooden handle

    # Heavy black rotary telephone
    phone_x = int(w * 0.72)
    d.rectangle([phone_x - 25, desk_y + 10, phone_x + 25, desk_y + 40], fill=(16, 16, 18), outline=(35, 35, 40))
    d.ellipse([phone_x - 14, desk_y + 14, phone_x + 14, desk_y + 36], fill=(220, 220, 220), outline=(16, 16, 18))
    # Handset
    d.ellipse([phone_x - 32, desk_y - 2, phone_x + 32, desk_y + 12], fill=(22, 22, 25))

    # Ashtray with glowing cigarette
    ash_x = int(w * 0.86)
    d.ellipse([ash_x - 16, desk_y + 16, ash_x + 16, desk_y + 32], fill=(30, 30, 34), outline=(70, 70, 78))
    d.line([(ash_x - 4, desk_y + 20), (ash_x + 14, desk_y + 22)], fill=(230, 230, 230), width=2)
    d.point([(ash_x + 15, desk_y + 22)], fill=(255, 80, 20))

    return im


def generate_bg_noir_manor() -> Image.Image:
    w, h = 640, 360
    im = Image.new("RGB", (w, h), (10, 8, 14))
    d = ImageDraw.Draw(im)

    # 1. Dark stormy wallpaper with damask pattern
    for y in range(h):
        factor = y / h
        r = int(14 * (1 - factor * 0.5))
        g = int(12 * (1 - factor * 0.5))
        b = int(20 * (1 - factor * 0.5))
        d.line([(0, y), (w, y)], fill=(r, g, b))

    # 2. Tall gothic mullioned arched windows on left
    for wx in [60, 140]:
        d.polygon([(wx, 160), (wx + 45, 160), (wx + 45, 60), (wx + 22, 30), (wx, 60)], fill=(25, 35, 55))
        # Mullion panes
        d.line([(wx + 22, 30), (wx + 22, 160)], fill=(12, 14, 20), width=2)
        for wy in range(50, 160, 22):
            d.line([(wx, wy), (wx + 45, wy)], fill=(12, 14, 20), width=2)
        # Lightning reflection flash
        d.polygon([(wx + 5, 50), (wx + 25, 40), (wx + 40, 120), (wx + 10, 140)], fill=(80, 110, 160))

    # 3. Grand Stone Fireplace in center
    fp_x1, fp_x2 = int(w * 0.38), int(w * 0.62)
    # Carved stone mantle
    d.rectangle([fp_x1 - 15, 80, fp_x2 + 15, 100], fill=(55, 50, 48), outline=(80, 75, 72))
    # Stone pillars
    d.rectangle([fp_x1 - 10, 100, fp_x1 + 10, int(h * 0.78)], fill=(45, 42, 40))
    d.rectangle([fp_x2 - 10, 100, fp_x2 + 10, int(h * 0.78)], fill=(45, 42, 40))
    # Hearth cavity (soot black)
    d.polygon([(fp_x1 + 10, int(h * 0.78)), (fp_x1 + 10, 130), ((fp_x1 + fp_x2)//2, 115), (fp_x2 - 10, 130), (fp_x2 - 10, int(h * 0.78))], fill=(10, 8, 8))
    # Glowing embers & crackling fireplace logs
    d.ellipse([(fp_x1 + fp_x2)//2 - 35, int(h * 0.78) - 30, (fp_x1 + fp_x2)//2 + 35, int(h * 0.78)], fill=(240, 100, 20))
    d.ellipse([(fp_x1 + fp_x2)//2 - 20, int(h * 0.78) - 45, (fp_x1 + fp_x2)//2 + 20, int(h * 0.78) - 10], fill=(255, 210, 50))
    # Cast warm firelight glow across hearthstone
    d.polygon([((fp_x1 + fp_x2)//2, int(h * 0.78)), (fp_x1 - 80, h), (fp_x2 + 80, h)], fill=(70, 30, 10))

    # 4. Antique Grandfather Clock on right
    clk_x = int(w * 0.78)
    d.rectangle([clk_x - 16, 70, clk_x + 16, int(h * 0.78)], fill=(38, 24, 16), outline=(65, 42, 28))
    # Clock face (illuminated cream)
    d.ellipse([clk_x - 12, 80, clk_x + 12, 104], fill=(240, 235, 215), outline=(212, 175, 55))
    d.line([(clk_x, 92), (clk_x, 84)], fill=(20, 20, 20), width=2)
    d.line([(clk_x, 92), (clk_x + 6, 94)], fill=(20, 20, 20), width=2)
    # Brass pendulum swinging in glass door
    d.ellipse([clk_x - 5, 140, clk_x + 5, 150], fill=(212, 175, 55))

    # 5. Polished parquet wood floor
    d.rectangle([0, int(h * 0.78), w, h], fill=(28, 20, 14))
    for fx in range(0, w, 40):
        d.line([(fx, int(h * 0.78)), (fx + 20, h)], fill=(45, 32, 22), width=1)

    # 6. Yellow Police Cordon Tape blocking the study door
    door_x = int(w * 0.26)
    d.rectangle([door_x - 20, 90, door_x + 20, int(h * 0.78)], fill=(18, 14, 12), outline=(40, 30, 25))
    # Stretched yellow crime scene tape
    d.polygon([(0, int(h * 0.82)), (w, int(h * 0.76)), (w, int(h * 0.79)), (0, int(h * 0.85))], fill=(240, 210, 35))
    # Text on police tape
    for tx in range(20, w - 80, 160):
        d.text((tx, int(h * 0.80)), "POLICE LINE - DO NOT CROSS", fill=(10, 10, 10))

    return im


def generate_bg_noir_docks() -> Image.Image:
    w, h = 640, 360
    im = Image.new("RGB", (w, h), (8, 12, 18))
    d = ImageDraw.Draw(im)

    # 1. Dark nocturnal harbor sky & sea mist
    for y in range(h):
        factor = y / h
        r = int(6 + factor * 8)
        g = int(10 + factor * 14)
        b = int(18 + factor * 22)
        d.line([(0, y), (w, y)], fill=(r, g, b))

    # 2. Murky harbor water with rippling oily reflections
    water_y = int(h * 0.62)
    d.rectangle([0, water_y, w, h], fill=(8, 14, 22))
    for ry in range(water_y + 10, h, 8):
        wave_w = int(w * 0.8)
        d.line([(w - wave_w, ry), (w, ry)], fill=(15, 28, 42), width=2)

    # 3. Heavy timber pier with weathered wooden planks on right
    pier_x1 = int(w * 0.35)
    d.polygon([(pier_x1, h), (int(w * 0.55), water_y - 10), (w, water_y - 10), (w, h)], fill=(22, 18, 14))
    for py in range(water_y, h, 14):
        d.line([(pier_x1 + (py - water_y) * 0.5, py), (w, py)], fill=(36, 28, 22), width=2)
    # Timber pylons extending down into water
    for px in [int(w * 0.45), int(w * 0.65), int(w * 0.85)]:
        d.rectangle([px - 8, water_y - 15, px + 8, h], fill=(16, 13, 10))
        # Seaweed / barnacle waterline
        d.rectangle([px - 10, water_y + 10, px + 10, water_y + 25], fill=(20, 35, 25))

    # 4. Cast Iron Mooring Bollard wrapped with coiled rope
    boll_x = int(w * 0.58)
    d.rectangle([boll_x - 12, water_y + 20, boll_x + 12, water_y + 55], fill=(30, 32, 36), outline=(60, 64, 70))
    d.ellipse([boll_x - 16, water_y + 14, boll_x + 16, water_y + 26], fill=(45, 48, 54))
    # Coiled hemp hawser rope
    d.ellipse([boll_x - 22, water_y + 40, boll_x + 22, water_y + 60], outline=(180, 150, 95), width=4)

    # 5. Heavy Cargo Crane & Warehouse Silhouettes in background
    crane_x = int(w * 0.18)
    # Warehouse
    d.polygon([(0, water_y), (crane_x - 20, water_y), (crane_x - 20, water_y - 80), (crane_x - 60, water_y - 110), (0, water_y - 80)], fill=(12, 16, 22))
    # Lattice crane boom
    d.line([(crane_x, water_y), (crane_x + 80, 60)], fill=(20, 26, 36), width=4)
    d.line([(crane_x + 80, 60), (crane_x + 140, 90)], fill=(20, 26, 36), width=3)
    d.line([(crane_x + 140, 90), (crane_x + 140, water_y - 20)], fill=(40, 48, 60), width=2) # Crane cable & hook

    # 6. Gas Streetlamp on pier casting glowing amber light & water reflection
    lamp_x, lamp_y = int(w * 0.78), water_y + 30
    # Light pool
    d.ellipse([lamp_x - 80, lamp_y - 140, lamp_x + 80, lamp_y + 20], fill=(255, 180, 50))
    # Cast iron Victorian lamp post
    d.rectangle([lamp_x - 3, lamp_y - 100, lamp_x + 3, lamp_y], fill=(24, 25, 28))
    # Glass lantern housing
    d.polygon([(lamp_x - 12, lamp_y - 115), (lamp_x + 12, lamp_y - 115), (lamp_x + 8, lamp_y - 95), (lamp_x - 8, lamp_y - 95)], fill=(255, 220, 110))

    # Dense swirling sea fog layer across pier and water
    for fy in range(water_y - 30, h, 18):
        alpha = int(40 * (1 - abs(fy - water_y) / (h - water_y + 30)))
        d.line([(0, fy), (w, fy)], fill=(160, 185, 210))

    return im


def generate_bg_noir_casino() -> Image.Image:
    w, h = 640, 360
    im = Image.new("RGB", (w, h), (18, 8, 12))
    d = ImageDraw.Draw(im)

    # 1. Art deco underground speakeasy wallpaper (deep burgundy & gold fan motifs)
    for y in range(h):
        factor = y / h
        r = int(28 * (1 - factor * 0.4))
        g = int(12 * (1 - factor * 0.4))
        b = int(16 * (1 - factor * 0.4))
        d.line([(0, y), (w, y)], fill=(r, g, b))

    # Art deco fan arches on back wall
    for ax in range(60, w, 110):
        for r in range(40, 10, -8):
            d.arc([ax - r, 30, ax + r, 30 + r * 2], 180, 360, fill=(75, 45, 25), width=2)

    # 2. Rich draped velvet stage curtains on flanks
    d.polygon([(0, 0), (70, 0), (45, h), (0, h)], fill=(65, 12, 22))
    d.polygon([(w, 0), (w - 70, 0), (w - 45, h), (w, h)], fill=(65, 12, 22))
    # Gold curtain tassels
    d.line([(55, int(h * 0.45)), (55, int(h * 0.55))], fill=(212, 175, 55), width=4)
    d.line([(w - 55, int(h * 0.45)), (w - 55, int(h * 0.55))], fill=(212, 175, 55), width=4)

    # 3. Gilded Art Deco Chandelier casting golden glow overhead
    chan_x = w // 2
    d.ellipse([chan_x - 120, 0, chan_x + 120, 160], fill=(255, 210, 80)) # Glow pool
    # Tiered crystal brass chandelier
    d.polygon([(chan_x - 50, 30), (chan_x + 50, 30), (chan_x + 25, 60), (chan_x - 25, 60)], fill=(212, 175, 55))
    d.polygon([(chan_x - 25, 60), (chan_x + 25, 60), (chan_x, 85)], fill=(245, 225, 130))

    # 4. Massive Roulette Table in foreground
    table_y = int(h * 0.64)
    # Green baize felt betting layout
    d.ellipse([int(w * 0.08), table_y - 20, int(w * 0.92), h + 100], fill=(16, 68, 38), outline=(80, 40, 18))
    # Mahogany table rim
    d.arc([int(w * 0.08), table_y - 20, int(w * 0.92), h + 100], 0, 180, fill=(95, 45, 20), width=12)

    # 5. Authentic Art Deco Roulette Wheel Bowl (mahogany & brass turret)
    wheel_cx, wheel_cy = w // 2, table_y + 45
    # Outer mahogany bowl
    d.ellipse([wheel_cx - 95, wheel_cy - 45, wheel_cx + 95, wheel_cy + 45], fill=(50, 22, 10), outline=(212, 175, 55))
    # Number ring (red and black alternating pocket sectors)
    d.ellipse([wheel_cx - 75, wheel_cy - 35, wheel_cx + 75, wheel_cy + 35], fill=(18, 18, 20))
    for deg in range(0, 360, 20):
        rad = math.radians(deg)
        x1 = wheel_cx + math.cos(rad) * 60
        y1 = wheel_cy + math.sin(rad) * 28
        col = (180, 25, 30) if (deg // 20) % 2 == 0 else (20, 20, 25)
        d.line([(wheel_cx, wheel_cy), (x1, y1)], fill=col, width=4)

    # Static brass center turret hub (Zero traveling dots)
    d.ellipse([wheel_cx - 24, wheel_cy - 12, wheel_cx + 24, wheel_cy + 12], fill=(212, 175, 55))
    d.line([(wheel_cx - 18, wheel_cy), (wheel_cx + 18, wheel_cy)], fill=(255, 230, 120), width=3)
    d.line([(wheel_cx, wheel_cy - 8), (wheel_cx, wheel_cy + 8)], fill=(255, 230, 120), width=3)

    # 6. Clay betting chip stacks on felt
    for cx, cy, col in [(int(w * 0.22), table_y + 20, (190, 25, 35)),
                        (int(w * 0.28), table_y + 10, (30, 90, 190)),
                        (int(w * 0.74), table_y + 15, (212, 175, 55)),
                        (int(w * 0.80), table_y + 25, (20, 20, 25))]:
        for stack_h in range(0, 18, 4):
            d.ellipse([cx - 12, cy - stack_h, cx + 12, cy - stack_h + 8], fill=col, outline=(240, 240, 240))

    # Smoky haze curling in air
    for sy in range(40, table_y, 25):
        d.line([(w * 0.2, sy), (w * 0.8, sy - 5)], fill=(180, 160, 170))

    return im


def generate_bg_noir_station() -> Image.Image:
    w, h = 640, 360
    im = Image.new("RGB", (w, h), (8, 10, 16))
    d = ImageDraw.Draw(im)

    # 1. Nocturnal atmospheric terminal concourse
    for y in range(h):
        factor = y / h
        r = int(8 + factor * 8)
        g = int(10 + factor * 12)
        b = int(18 + factor * 18)
        d.line([(0, y), (w, y)], fill=(r, g, b))

    # 2. Grand Vaulted Victorian Iron Truss Arches overhead
    for tx in range(0, w + 80, 80):
        # Arched iron girders
        d.arc([tx - 120, -60, tx + 120, 220], 0, 180, fill=(35, 42, 54), width=4)
        # Lattice struts
        d.line([(tx, 0), (tx, 140)], fill=(28, 34, 45), width=2)
        d.line([(tx - 40, 0), (tx, 60)], fill=(24, 30, 40), width=1)
        d.line([(tx + 40, 0), (tx, 60)], fill=(24, 30, 40), width=1)

    # 3. Massive Steam Locomotive Boiler & Cabin on Platform 4
    train_x = int(w * 0.15)
    # Dark iron boiler body
    d.rectangle([0, int(h * 0.42), train_x + 160, int(h * 0.78)], fill=(16, 17, 20), outline=(40, 44, 50))
    d.ellipse([train_x + 130, int(h * 0.42), train_x + 190, int(h * 0.78)], fill=(22, 24, 28), outline=(60, 65, 75)) # Front smokebox door
    # Smokestack funnel
    d.rectangle([train_x + 110, int(h * 0.32), train_x + 136, int(h * 0.42)], fill=(16, 17, 20))
    # Massive glowing headlight beam cutting through steam
    hl_x, hl_y = train_x + 175, int(h * 0.54)
    d.polygon([(hl_x, hl_y), (w, hl_y - 70), (w, hl_y + 110)], fill=(255, 245, 190))
    d.ellipse([hl_x - 14, hl_y - 14, hl_x + 14, hl_y + 14], fill=(255, 255, 230), outline=(212, 175, 55))

    # Billowing white clouds of steam from engine
    for sx, sy, sr in [(train_x + 120, int(h * 0.28), 35),
                       (train_x + 150, int(h * 0.22), 48),
                       (train_x + 200, int(h * 0.18), 65),
                       (train_x + 260, int(h * 0.15), 80)]:
        d.ellipse([sx - sr, sy - sr, sx + sr, sy + sr], fill=(210, 220, 235))

    # 4. Elevated Suspended Platform Clock in center
    clk_x = int(w * 0.58)
    d.line([(clk_x, 0), (clk_x, 60)], fill=(40, 45, 55), width=3) # Iron hanger
    d.ellipse([clk_x - 24, 60, clk_x + 24, 108], fill=(245, 240, 225), outline=(212, 175, 55))
    d.line([(clk_x, 84), (clk_x, 70)], fill=(20, 20, 20), width=3)
    d.line([(clk_x, 84), (clk_x + 10, 90)], fill=(20, 20, 20), width=2)

    # 5. Wet cobblestone platform in foreground
    plat_y = int(h * 0.76)
    d.rectangle([0, plat_y, w, h], fill=(20, 22, 28))
    # Cobblestones with wet rain sheen
    for cy in range(plat_y, h, 12):
        for cx in range(0, w, 24):
            offset = 12 if (cy // 12) % 2 == 1 else 0
            d.rectangle([cx + offset, cy, cx + offset + 22, cy + 10], outline=(38, 42, 52), width=1)
            d.line([(cx + offset + 2, cy + 2), (cx + offset + 12, cy + 2)], fill=(80, 95, 120), width=1) # Wet reflection

    # 6. Stacks of leather luggage trunks on platform
    lug_x = int(w * 0.76)
    d.rectangle([lug_x - 30, plat_y - 26, lug_x + 30, plat_y], fill=(70, 42, 24), outline=(40, 24, 12))
    d.rectangle([lug_x - 24, plat_y - 48, lug_x + 24, plat_y - 26], fill=(95, 58, 32), outline=(55, 32, 16))
    # Brass locks & corner brackets
    d.rectangle([lug_x - 4, plat_y - 38, lug_x + 4, plat_y - 32], fill=(212, 175, 55))
    d.rectangle([lug_x - 4, plat_y - 16, lug_x + 4, plat_y - 10], fill=(212, 175, 55))

    return im


def main():
    print("Generating KMystery Suspect Portraits...")
    save_sprite_with_atlas(generate_suspect_mr_black(), "suspect_mr_black.png")
    save_sprite_with_atlas(generate_suspect_miss_scarlet(), "suspect_miss_scarlet.png")
    save_sprite_with_atlas(generate_suspect_colonel_mustard(), "suspect_colonel_mustard.png")
    save_sprite_with_atlas(generate_suspect_mrs_white(), "suspect_mrs_white.png")
    save_sprite_with_atlas(generate_suspect_professor_plum(), "suspect_professor_plum.png")

    print("Generating KMystery Forensic Clue Sprites...")
    save_sprite_with_atlas(generate_clue_cufflink(), "clue_cufflink.png")
    save_sprite_with_atlas(generate_clue_perfume(), "clue_perfume.png")
    save_sprite_with_atlas(generate_clue_button(), "clue_button.png")
    save_sprite_with_atlas(generate_clue_glove(), "clue_glove.png")
    save_sprite_with_atlas(generate_clue_spectacles(), "clue_spectacles.png")
    save_sprite_with_atlas(generate_clue_shell(), "clue_shell.png")
    save_sprite_with_atlas(generate_clue_vial(), "clue_vial.png")
    save_sprite_with_atlas(generate_clue_pipe(), "clue_pipe.png")
    save_sprite_with_atlas(generate_clue_letter(), "clue_letter.png")
    save_sprite_with_atlas(generate_clue_will(), "clue_will.png")
    save_sprite_with_atlas(generate_clue_photo(), "clue_photo.png")

    print("Generating KMystery Noir Location Backdrops...")
    bgs = [
        ("bg_noir_office.png", generate_bg_noir_office()),
        ("bg_noir_manor.png", generate_bg_noir_manor()),
        ("bg_noir_docks.png", generate_bg_noir_docks()),
        ("bg_noir_casino.png", generate_bg_noir_casino()),
        ("bg_noir_station.png", generate_bg_noir_station())
    ]
    for filename, bg_img in bgs:
        path = BG_DIR / filename
        bg_img.save(path, format="PNG", optimize=True)
        print(f"Saved backdrop: {path} ({bg_img.width}x{bg_img.height})")

    print("All KMystery visual assets generated successfully!")


if __name__ == "__main__":
    main()
