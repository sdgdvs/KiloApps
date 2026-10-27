# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
Generates high-fidelity Roman Flavian Amphitheatre arena backdrop and 2-stage gladiator/boss sprites for KColosseum.
Includes:
- Flavian Colosseum arena backdrop (800x320 PNG)
- 4 Player/Opponent Gladiator Class Sprites + JSON Atlases (128x128)
- 5 Arena Boss Sprites + JSON Atlases (Lion, Chariot, Behemoth, Praetorian, Dimachaerus)
- 4 Class Mini-Portraits (48x48)
"""

import os
import json
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites" / "kcolosseum"
BG_DIR = ROOT / "KiloOS" / "public" / "assets" / "backgrounds" / "kcolosseum"

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
# 1. FLAVIAN AMPHITHEATRE ARENA BACKDROP (800x320 RGBA)
# =============================================================================

def generate_colosseum_arena_bg() -> Image.Image:
    w, h = 800, 320
    im = Image.new("RGBA", (w, h), (0, 0, 0, 255))
    d = ImageDraw.Draw(im)

    # Sky gradient: Mediterranean Roman azure / dusk amber gradient
    for y in range(int(h * 0.4)):
        t = y / (h * 0.4)
        r = int(24 + (180 - 24) * t * 0.8)
        g = int(36 + (140 - 36) * t * 0.8)
        b = int(72 + (80 - 72) * t * 0.5)
        d.line([(0, y), (w, y)], fill=(r, g, b, 255))

    # Roman Sun glow
    sun_x, sun_y = 650, 45
    for r in range(40, 0, -2):
        alpha = int(70 * (1 - r / 40))
        d.ellipse([sun_x - r, sun_y - r, sun_x + r, sun_y + r], fill=(255, 230, 160, alpha))
    d.ellipse([sun_x - 14, sun_y - 14, sun_x + 14, sun_y + 14], fill=(255, 250, 220, 240))

    # Upper Colosseum Attics & Velarium Awning Brackets (y: 20 to 65)
    for y in range(25, 65):
        d.line([(0, y), (w, y)], fill=(95, 70, 50, 255))
    # Cornice trim
    d.line([(0, 24), (w, 24)], fill=(150, 120, 85, 255), width=2)
    d.line([(0, 64), (w, 64)], fill=(130, 100, 70, 255), width=2)
    # Velarium masts / poles
    for x in range(15, w, 35):
        d.line([(x, 10), (x, 30)], fill=(65, 45, 30, 255), width=3)
        # Red fabric fringe
        d.polygon([(x - 8, 25), (x + 8, 25), (x, 34)], fill=(160, 25, 25, 220))

    # Second Tier Arches & Columns (y: 65 to 135)
    for y in range(65, 135):
        d.line([(0, y), (w, y)], fill=(120, 92, 68, 255))
    arch_spacing = 58
    for x in range(10, w, arch_spacing):
        # Column
        d.rectangle([x, 65, x + 10, 135], fill=(155, 125, 95, 255))
        d.rectangle([x + 1, 65, x + 3, 135], fill=(185, 155, 125, 255))
        # Arch opening (dark shadow with silhouettes)
        ax = x + 12
        aw = arch_spacing - 14
        d.pieslice([ax, 70, ax + aw, 110], 180, 360, fill=(35, 18, 12, 255))
        d.rectangle([ax, 90, ax + aw, 135], fill=(35, 18, 12, 255))
        # Shadow crowd inside arch
        for cx in range(ax + 4, ax + aw - 4, 7):
            d.ellipse([cx, 102, cx + 5, 112], fill=(20, 8, 6, 230))
            d.rectangle([cx - 1, 112, cx + 6, 135], fill=(22, 10, 8, 230))

    # Lower Tier Cornice & Podium Wall (y: 135 to 195)
    d.rectangle([0, 135, w, 142], fill=(175, 145, 110, 255))
    d.line([(0, 135), (w, 135)], fill=(215, 185, 145, 255), width=2)
    d.rectangle([0, 142, w, 195], fill=(110, 85, 62, 255))

    # Dense cheering spectators along podium rail (y: 130 to 142)
    for x in range(4, w, 6):
        bob = int(math.sin(x * 0.45) * 3)
        head_y = 126 + bob
        # Head
        d.ellipse([x, head_y, x + 5, head_y + 6], fill=(215, 165, 130, 255))
        # Toga / tunic
        tunic_col = (180, 30, 30, 255) if (x % 18 == 0) else ((220, 215, 200, 255) if (x % 12 == 0) else (140, 80, 50, 255))
        d.rectangle([x - 1, head_y + 6, x + 6, 142], fill=tunic_col)
        # Cheering arm
        if x % 24 == 0:
            d.line([(x + 2, head_y + 4), (x + 6, head_y - 4)], fill=(215, 165, 130, 255), width=2)

    # Imperial Banners hanging from podium (SPQR crimson silk with gold embroidery)
    for bx in [90, 250, 550, 710]:
        d.polygon([(bx, 142), (bx + 32, 142), (bx + 32, 185), (bx + 16, 196), (bx, 185)], fill=(145, 15, 25, 255))
        d.line([(bx, 142), (bx + 32, 142), (bx + 32, 185), (bx + 16, 196), (bx, 185), (bx, 142)], fill=(212, 175, 55, 255), width=2)
        # Gold eagle / laurel motif
        d.ellipse([bx + 10, 154, bx + 22, 166], outline=(212, 175, 55, 255), width=2)
        d.polygon([(bx + 8, 170), (bx + 24, 170), (bx + 16, 178)], fill=(212, 175, 55, 255))

    # Podia wall stone blocks pattern
    for y in range(145, 195, 12):
        d.line([(0, y), (w, y)], fill=(80, 60, 42, 200), width=1)
    for x in range(20, w, 40):
        d.line([(x, 145), (x, 195)], fill=(80, 60, 42, 180), width=1)

    # Iron Arena Gates (Porta Triumphalis / Libitinensis)
    for gx in [160, 480]:
        d.rectangle([gx, 152, gx + 50, 195], fill=(30, 20, 15, 255))
        # Iron bars
        for bar_x in range(gx + 4, gx + 48, 6):
            d.line([(bar_x, 152), (bar_x, 195)], fill=(120, 115, 110, 255), width=2)
        d.line([(gx, 170), (gx + 50, 170)], fill=(90, 85, 80, 255), width=2)
        d.line([(gx, 185), (gx + 50, 185)], fill=(90, 85, 80, 255), width=2)
        # Arch top
        d.arc([gx, 144, gx + 50, 160], 180, 360, fill=(160, 130, 95, 255), width=3)

    # Arena Sand Floor (y: 195 to 320)
    for y in range(195, h):
        t = (y - 195) / (h - 195)
        # Sand gradient: golden ochre at barrier fading to bright sunlit arena sand
        sr = int(185 + (220 - 185) * t)
        sg = int(145 + (185 - 145) * t)
        sb = int(85 + (115 - 85) * t)
        d.line([(0, y), (w, y)], fill=(sr, sg, sb, 255))

    # Sand texture & dunes
    for y in range(200, h, 6):
        d.line([(0, y), (w, y)], fill=(int(sr * 0.95), int(sg * 0.93), int(sb * 0.9), 120), width=1)

    # Chariot wheel ruts in the sand
    d.arc([80, 215, 720, 260], 0, 180, fill=(160, 125, 75, 140), width=3)
    d.arc([75, 218, 725, 263], 0, 180, fill=(160, 125, 75, 140), width=3)

    # Dramatic blood stains on the sand
    for (bsx, bsy, bsw, bsh) in [(280, 240, 35, 14), (520, 255, 42, 16), (390, 285, 28, 10)]:
        d.ellipse([bsx, bsy, bsx + bsw, bsy + bsh], fill=(110, 15, 15, 160))
        d.ellipse([bsx + 4, bsy + 2, bsx + bsw - 6, bsy + bsh - 3], fill=(85, 10, 10, 180))

    # Flavian Amphitheatre Braziers & Torches (left and right pillars)
    for tx in [45, w - 55]:
        # Iron torch bracket
        d.rectangle([tx - 4, 150, tx + 4, 180], fill=(45, 42, 40, 255))
        d.polygon([(tx - 10, 150), (tx + 10, 150), (tx + 5, 160), (tx - 5, 160)], fill=(75, 70, 65, 255))
        # Flame glow
        for r in range(22, 4, -2):
            d.ellipse([tx - r, 145 - r, tx + r, 145 + r], fill=(255, 120, 20, int(60 * (1 - r / 22))))
        # Inner flame
        d.polygon([(tx - 6, 150), (tx + 6, 150), (tx, 132)], fill=(255, 80, 10, 255))
        d.polygon([(tx - 3, 148), (tx + 3, 148), (tx, 136)], fill=(255, 220, 60, 255))

    return im


# =============================================================================
# 2. GLADIATOR SPRITES (128x128 RGBA)
# =============================================================================

def generate_gladiator_murmillo() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Shadow on sand
    d.ellipse([34, 112, 94, 124], fill=(70, 45, 25, 140))

    # Legs & Caligae (Roman sandals)
    d.rectangle([48, 86, 57, 114], fill=(160, 115, 80, 255))
    d.rectangle([68, 86, 77, 114], fill=(160, 115, 80, 255))
    # Bronze Ocrea (greaves)
    d.polygon([(47, 92), (58, 92), (56, 112), (48, 112)], fill=(195, 145, 40, 255))
    d.line([(52, 93), (52, 111)], fill=(240, 205, 90, 255), width=1)
    d.polygon([(67, 92), (78, 92), (76, 112), (68, 112)], fill=(195, 145, 40, 255))

    # Subligaculum (loincloth) & Balteus (heavy belt)
    d.polygon([(44, 76), (82, 76), (76, 92), (50, 92)], fill=(150, 25, 25, 255))
    d.rectangle([43, 74, 83, 80], fill=(85, 45, 20, 255))
    d.rectangle([59, 74, 67, 80], fill=(220, 180, 50, 255))

    # Muscular Torso & Chest
    d.polygon([(46, 50), (80, 50), (78, 76), (48, 76)], fill=(195, 140, 100, 255))
    # Muscle definition
    d.line([(63, 52), (63, 72)], fill=(160, 105, 70, 200), width=2)
    d.arc([50, 56, 62, 66], 0, 180, fill=(160, 105, 70, 200), width=1)
    d.arc([64, 56, 76, 66], 0, 180, fill=(160, 105, 70, 200), width=1)

    # Manica (segmented arm guard on right weapon arm)
    d.polygon([(78, 52), (94, 62), (88, 72), (76, 58)], fill=(175, 130, 35, 255))
    for segment in range(4):
        sy = 54 + segment * 4
        d.line([(78 + segment * 3, sy), (86 + segment * 2, sy + 6)], fill=(230, 195, 80, 255), width=2)

    # Large Rectangular Scutum Shield (left arm, facing forward)
    # Curved red shield body
    d.polygon([(22, 54), (52, 50), (52, 104), (22, 100)], fill=(170, 20, 20, 255))
    d.polygon([(24, 56), (50, 52), (50, 102), (24, 98)], fill=(190, 28, 28, 255))
    # Bronze Shield Rim
    d.line([(22, 54), (52, 50), (52, 104), (22, 100), (22, 54)], fill=(212, 175, 55, 255), width=2)
    # Golden Thunderbolt (Fulmen) motif on shield
    d.ellipse([33, 72, 41, 80], fill=(225, 190, 60, 255))
    d.polygon([(26, 76), (37, 68), (34, 76), (45, 66)], fill=(240, 210, 80, 255))
    d.polygon([(26, 76), (37, 84), (34, 76), (45, 86)], fill=(240, 210, 80, 255))

    # Cassis Crista Helmet (Murmillo helmet with broad brim and marine fish crest)
    # Head & Neck
    d.rectangle([58, 42, 68, 52], fill=(185, 130, 90, 255))
    # Helmet skull
    d.ellipse([50, 26, 76, 48], fill=(195, 145, 45, 255))
    # Wide brim
    d.polygon([(44, 38), (82, 38), (78, 44), (48, 44)], fill=(225, 180, 60, 255))
    # Pierced grill face visor
    d.rectangle([54, 38, 72, 48], fill=(130, 95, 25, 255))
    for vx in range(56, 72, 3):
        d.line([(vx, 40), (vx, 46)], fill=(40, 28, 8, 255), width=1)
    # Murmillo stylized fin crest / crista
    d.polygon([(52, 28), (62, 12), (72, 16), (74, 28)], fill=(215, 170, 50, 255))
    d.polygon([(54, 26), (62, 14), (70, 18), (72, 26)], fill=(245, 210, 80, 255))
    # Red horsehair plume flowing back
    d.arc([60, 10, 90, 36], 180, 340, fill=(190, 20, 20, 255), width=4)

    # Gladius Sword (right hand)
    d.rectangle([90, 64, 94, 72], fill=(110, 70, 30, 255)) # Hilt
    d.ellipse([88, 62, 96, 66], fill=(220, 180, 50, 255)) # Pommel
    d.rectangle([87, 72, 97, 75], fill=(220, 180, 50, 255)) # Crossguard
    d.polygon([(89, 75), (95, 75), (93, 105), (92, 110), (91, 105)], fill=(235, 235, 245, 255)) # Steel blade
    d.line([(92, 75), (92, 108)], fill=(255, 255, 255, 255), width=1)

    return im


def generate_gladiator_thraex() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Shadow on sand
    d.ellipse([34, 112, 94, 124], fill=(70, 45, 25, 140))

    # Legs & high Thracian greaves (reaching to midthigh)
    d.rectangle([48, 80, 57, 114], fill=(210, 160, 45, 255))
    d.line([(52, 80), (52, 112)], fill=(245, 210, 90, 255), width=1)
    d.rectangle([68, 80, 77, 114], fill=(210, 160, 45, 255))
    d.line([(72, 80), (72, 112)], fill=(245, 210, 90, 255), width=1)

    # Subligaculum & Bronze Belt
    d.polygon([(46, 72), (80, 72), (76, 84), (50, 84)], fill=(30, 90, 140, 255))
    d.rectangle([45, 70, 81, 75], fill=(212, 175, 55, 255))

    # Lean Muscular Torso
    d.polygon([(48, 48), (78, 48), (76, 72), (50, 72)], fill=(205, 150, 110, 255))
    d.line([(63, 50), (63, 68)], fill=(170, 115, 80, 200), width=2)

    # Small Round Shield (Parmula)
    d.ellipse([24, 56, 56, 88], fill=(185, 135, 35, 255))
    d.ellipse([27, 59, 53, 85], fill=(25, 80, 130, 255))
    d.ellipse([36, 68, 44, 76], fill=(235, 195, 60, 255)) # Central boss

    # Thraex Griffin Crest Helmet
    d.rectangle([58, 40, 68, 50], fill=(195, 140, 100, 255))
    d.ellipse([50, 24, 76, 46], fill=(200, 150, 45, 255))
    # Visor
    d.rectangle([54, 36, 72, 45], fill=(120, 85, 20, 255))
    d.line([(54, 40), (72, 40)], fill=(30, 20, 5, 255), width=2)
    # Griffin head crest
    d.polygon([(58, 24), (66, 10), (74, 16), (70, 24)], fill=(230, 185, 55, 255))
    d.polygon([(66, 10), (62, 4), (68, 8)], fill=(250, 220, 80, 255)) # Griffin beak
    # Blue / gold feathers on helmet side
    d.line([(50, 26), (42, 16)], fill=(40, 120, 200, 255), width=2)
    d.line([(76, 26), (84, 16)], fill=(40, 120, 200, 255), width=2)

    # Curved Sica Sword (Thracian forward-curving blade)
    d.rectangle([88, 62, 92, 70], fill=(90, 50, 20, 255))
    d.ellipse([87, 60, 93, 64], fill=(212, 175, 55, 255))
    # Sica blade curving aggressively forward
    d.line([(90, 70), (94, 88)], fill=(230, 230, 240, 255), width=3)
    d.line([(94, 88), (108, 96)], fill=(245, 245, 255, 255), width=3)
    d.polygon([(106, 94), (112, 98), (104, 98)], fill=(255, 255, 255, 255))

    return im


def generate_gladiator_retiarius() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Shadow on sand
    d.ellipse([34, 112, 94, 124], fill=(70, 45, 25, 140))

    # Bare legs with leather wraps
    d.rectangle([48, 82, 57, 114], fill=(210, 155, 115, 255))
    d.rectangle([68, 82, 77, 114], fill=(210, 155, 115, 255))
    for y in range(96, 114, 4):
        d.line([(48, y), (57, y)], fill=(120, 70, 35, 255), width=1)
        d.line([(68, y), (77, y)], fill=(120, 70, 35, 255), width=1)

    # White & Sea-Green Subligaculum
    d.polygon([(46, 72), (80, 72), (74, 86), (52, 86)], fill=(40, 140, 120, 255))
    d.rectangle([45, 70, 81, 74], fill=(212, 175, 55, 255))

    # Muscular Bare Torso & Head (Unhelmeted, athletic)
    d.polygon([(48, 48), (78, 48), (76, 72), (50, 72)], fill=(210, 155, 115, 255))
    # Head & Roman hair
    d.ellipse([54, 28, 72, 48], fill=(210, 155, 115, 255))
    d.arc([52, 26, 74, 42], 180, 360, fill=(60, 35, 15, 255), width=4) # Dark curls
    d.line([(58, 38), (62, 38)], fill=(40, 20, 10, 255), width=1) # Eye
    d.line([(64, 38), (68, 38)], fill=(40, 20, 10, 255), width=1)

    # Galerus (Bronze shoulder guard standing upright on left shoulder)
    d.polygon([(36, 32), (48, 36), (46, 56), (34, 52)], fill=(215, 165, 45, 255))
    d.polygon([(38, 34), (46, 38), (44, 54), (36, 50)], fill=(245, 205, 80, 255))
    # Manica padding on left arm
    d.rectangle([34, 54, 44, 76], fill=(185, 135, 40, 255))

    # Casting Net (Rete) draped over left forearm and hanging down
    net_pts = [(36, 74), (20, 86), (16, 110), (38, 116), (42, 92)]
    d.polygon(net_pts, fill=(160, 140, 110, 120))
    # Net mesh lines & lead sinkers
    for ny in range(86, 116, 6):
        d.line([(18, ny), (40, ny)], fill=(90, 80, 65, 200), width=1)
    for nx in range(18, 40, 5):
        d.line([(nx, 86), (nx, 114)], fill=(90, 80, 65, 200), width=1)
    # Lead weights
    for wx, wy in [(16, 110), (22, 113), (30, 115), (38, 116)]:
        d.ellipse([wx - 2, wy - 2, wx + 2, wy + 2], fill=(120, 125, 130, 255))

    # Steel Trident (Fuscina) held in right hand
    d.line([(88, 14), (88, 118)], fill=(120, 80, 45, 255), width=3) # Ash wood shaft
    # Trident head
    d.line([(80, 20), (96, 20)], fill=(225, 225, 235, 255), width=3) # Crossbar
    d.line([(80, 6), (80, 20)], fill=(245, 245, 255, 255), width=2)  # Left prong
    d.line([(88, 2), (88, 20)], fill=(255, 255, 255, 255), width=3)  # Center long prong
    d.line([(96, 6), (96, 20)], fill=(245, 245, 255, 255), width=2)  # Right prong
    # Barbs
    d.line([(79, 10), (81, 13)], fill=(220, 220, 230, 255), width=2)
    d.line([(97, 10), (95, 13)], fill=(220, 220, 230, 255), width=2)

    return im


def generate_gladiator_hoplomachus() -> Image.Image:
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Shadow on sand
    d.ellipse([34, 112, 94, 124], fill=(70, 45, 25, 140))

    # Greaves & subligaculum
    d.rectangle([48, 80, 57, 114], fill=(195, 145, 45, 255))
    d.rectangle([68, 80, 77, 114], fill=(195, 145, 45, 255))
    d.polygon([(46, 72), (80, 72), (76, 84), (50, 84)], fill=(130, 40, 20, 255))
    d.rectangle([45, 70, 81, 74], fill=(212, 175, 55, 255))

    # Muscular Torso with quilted belt
    d.polygon([(48, 48), (78, 48), (76, 72), (50, 72)], fill=(200, 145, 105, 255))

    # Round Concave Bronze Shield (Aspis / Parma)
    d.ellipse([22, 54, 58, 90], fill=(212, 175, 55, 255))
    d.ellipse([26, 58, 54, 86], fill=(175, 135, 35, 255))
    d.ellipse([36, 68, 44, 76], fill=(245, 215, 90, 255)) # Gilded center

    # Crested Helmet with side feathers
    d.ellipse([50, 24, 76, 46], fill=(200, 150, 45, 255))
    d.rectangle([54, 36, 72, 45], fill=(120, 85, 20, 255))
    # Tall bronze crest
    d.polygon([(56, 24), (63, 8), (70, 24)], fill=(225, 180, 55, 255))
    # Twin side feathers
    d.line([(52, 24), (44, 10)], fill=(230, 230, 240, 255), width=2)
    d.line([(74, 24), (82, 10)], fill=(230, 230, 240, 255), width=2)

    # Thrusting Spear (Hasta) held upright in right hand
    d.line([(92, 10), (92, 118)], fill=(130, 85, 45, 255), width=3)
    # Leaf-shaped spearhead
    d.polygon([(88, 20), (92, 6), (96, 20)], fill=(235, 235, 245, 255))
    d.line([(92, 6), (92, 22)], fill=(255, 255, 255, 255), width=1)

    return im


# =============================================================================
# 3. ARENA BOSS SPRITES (Lion, Chariot, Behemoth, Praetorian, Dimachaerus)
# =============================================================================

def generate_boss_lion() -> Image.Image:
    w, h = 160, 128
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Ground shadow
    d.ellipse([25, 104, 145, 122], fill=(60, 35, 18, 140))

    # Hindquarters & Tail
    d.ellipse([100, 56, 142, 96], fill=(195, 140, 60, 255))
    # Tail curving up with dark tuft
    d.arc([115, 30, 155, 80], 180, 330, fill=(185, 130, 55, 255), width=4)
    d.ellipse([142, 34, 152, 46], fill=(65, 35, 15, 255))

    # Muscular Hind Legs & Paws
    d.polygon([(112, 75), (128, 75), (124, 112), (110, 112)], fill=(180, 128, 50, 255))
    d.ellipse([108, 108, 126, 116], fill=(190, 135, 55, 255)) # Paw with claws
    d.polygon([(126, 72), (138, 72), (136, 108), (125, 108)], fill=(165, 115, 45, 255))

    # Muscular Flank & Chest
    d.ellipse([50, 52, 115, 96], fill=(205, 150, 65, 255))

    # Front Legs planted aggressive
    d.polygon([(46, 68), (58, 68), (56, 112), (44, 112)], fill=(190, 135, 55, 255))
    d.ellipse([42, 108, 58, 116], fill=(205, 150, 65, 255))
    for cx in range(43, 56, 4):
        d.line([(cx, 114), (cx - 2, 118)], fill=(240, 240, 240, 255), width=2) # Claws digging

    d.polygon([(62, 68), (74, 68), (72, 112), (60, 112)], fill=(175, 120, 48, 255))

    # Magnificent Dark Barbary Lion Mane (Deep bronze & dark umber)
    for r in range(32, 14, -3):
        d.ellipse([26 - r * 0.4, 30 - r * 0.4, 76 + r * 0.4, 82 + r * 0.4], fill=(85, 45, 18, 180))
    d.ellipse([28, 32, 74, 84], fill=(70, 35, 15, 255))
    # Mane tufts
    for angle in range(0, 360, 30):
        rad = math.radians(angle)
        tx = 51 + math.cos(rad) * 28
        ty = 56 + math.sin(rad) * 28
        d.polygon([(51, 56), (tx, ty), (tx + 4, ty + 4)], fill=(105, 55, 22, 255))

    # Lion Head & Snout (Facing Left in Ferocious Roar)
    d.ellipse([24, 40, 54, 72], fill=(205, 150, 65, 255))
    # Snout
    d.polygon([(16, 52), (32, 46), (32, 66), (18, 64)], fill=(185, 130, 55, 255))
    # Piercing Amber Eye
    d.ellipse([30, 46, 36, 51], fill=(255, 200, 30, 255))
    d.ellipse([32, 47, 34, 50], fill=(20, 10, 5, 255))

    # Roaring Jaws & Bared Fangs
    d.polygon([(14, 56), (28, 56), (26, 68), (14, 66)], fill=(130, 20, 20, 255)) # Open mouth pink/red
    # Upper fangs
    d.polygon([(16, 56), (19, 56), (17, 62)], fill=(250, 250, 250, 255))
    d.polygon([(23, 56), (25, 56), (24, 60)], fill=(250, 250, 250, 255))
    # Lower fangs
    d.polygon([(16, 66), (19, 66), (17, 60)], fill=(250, 250, 250, 255))

    # Whiskers
    d.line([(22, 58), (8, 54)], fill=(230, 220, 200, 200), width=1)
    d.line([(22, 60), (6, 62)], fill=(230, 220, 200, 200), width=1)
    d.line([(22, 62), (10, 68)], fill=(230, 220, 200, 200), width=1)

    return im


def generate_boss_chariot() -> Image.Image:
    w, h = 192, 128
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Sand shadow
    d.ellipse([15, 106, 185, 124], fill=(60, 35, 18, 140))

    # Warhorses (Charging left)
    # Horse 2 (Back Horse)
    d.ellipse([50, 50, 100, 84], fill=(90, 55, 30, 255))
    d.polygon([(40, 35), (60, 48), (48, 62), (32, 48)], fill=(90, 55, 30, 255))
    # Horse 1 (Front Warhorse)
    d.ellipse([35, 54, 90, 90], fill=(135, 80, 40, 255))
    # Muscular neck & head
    d.polygon([(25, 36), (48, 50), (38, 66), (18, 50)], fill=(135, 80, 40, 255))
    d.polygon([(18, 50), (8, 54), (12, 64), (22, 62)], fill=(135, 80, 40, 255))
    # Horse eye & red war plume
    d.ellipse([20, 46, 24, 50], fill=(20, 10, 5, 255))
    d.polygon([(24, 34), (20, 20), (28, 28)], fill=(190, 20, 20, 255)) # Plume

    # Horse front legs galloping
    d.line([(32, 75), (16, 104)], fill=(125, 75, 35, 255), width=5)
    d.line([(48, 75), (38, 110)], fill=(115, 68, 30, 255), width=5)
    # Rear legs
    d.line([(78, 75), (68, 110)], fill=(125, 75, 35, 255), width=5)

    # Leather harness & brass studs connecting horse to chariot
    d.line([(35, 60), (120, 75)], fill=(185, 140, 45, 255), width=2)
    d.line([(15, 58), (110, 82)], fill=(140, 30, 30, 255), width=2)

    # Chariot Carriage (Curved bronze chassis with golden eagles)
    d.polygon([(105, 56), (155, 56), (152, 92), (112, 92)], fill=(185, 135, 45, 255))
    d.polygon([(108, 58), (152, 58), (150, 90), (114, 90)], fill=(145, 25, 25, 255))
    # Gilded chariot rim & Roman Eagle motif
    d.line([(105, 56), (155, 56)], fill=(225, 190, 60, 255), width=3)
    d.ellipse([124, 68, 136, 80], fill=(225, 190, 60, 255))

    # Charioteer standing inside chassis
    # Torso in bronze cuirass
    d.rectangle([122, 42, 138, 62], fill=(200, 150, 50, 255))
    # Helmet with red horsehair crest
    d.ellipse([124, 26, 136, 40], fill=(212, 175, 55, 255))
    d.polygon([(122, 28), (130, 16), (138, 28)], fill=(200, 20, 20, 255))
    # Holding whip / spear
    d.line([(135, 46), (160, 22)], fill=(220, 220, 230, 255), width=2)

    # Chariot Wheel with Scythed Hub Blade
    d.ellipse([118, 78, 150, 110], fill=(130, 80, 35, 255))
    d.ellipse([124, 84, 144, 104], fill=(70, 40, 18, 255)) # Inner rim
    d.ellipse([130, 90, 138, 98], fill=(212, 175, 55, 255)) # Bronze axle hub
    # Spokes
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        sx = 134 + math.cos(rad) * 15
        sy = 94 + math.sin(rad) * 15
        d.line([(134, 94), (sx, sy)], fill=(190, 140, 50, 255), width=2)
    # Scythed Wheel Blade (spinning razor blade on axle)
    d.polygon([(134, 94), (164, 90), (160, 96)], fill=(240, 240, 250, 255))
    d.polygon([(134, 94), (104, 98), (108, 92)], fill=(240, 240, 250, 255))

    return im


def generate_boss_behemoth() -> Image.Image:
    w, h = 160, 160
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Ground shadow
    d.ellipse([28, 136, 132, 154], fill=(55, 30, 15, 150))

    # Massive Tree-Trunk Legs
    d.rectangle([50, 100, 68, 142], fill=(175, 120, 80, 255))
    d.rectangle([92, 100, 110, 142], fill=(175, 120, 80, 255))
    # Fur-lined spiked boots
    d.rectangle([48, 134, 70, 146], fill=(75, 45, 25, 255))
    d.rectangle([90, 134, 112, 146], fill=(75, 45, 25, 255))

    # Heavy Fur Kilt / War-skirt
    d.polygon([(44, 82), (116, 82), (122, 108), (38, 108)], fill=(65, 40, 25, 255))
    # Spiked iron war-belt
    d.rectangle([42, 80, 118, 88], fill=(95, 95, 100, 255))
    for bx in range(46, 116, 12):
        d.polygon([(bx, 80), (bx + 4, 76), (bx + 8, 80)], fill=(200, 200, 210, 255))

    # Giant Scarred Bare Torso
    d.polygon([(40, 46), (120, 46), (112, 82), (48, 82)], fill=(185, 128, 85, 255))
    # Battle Scars (White / crimson jagged slashes)
    d.line([(55, 52), (75, 68)], fill=(140, 30, 30, 255), width=2)
    d.line([(57, 54), (73, 66)], fill=(225, 195, 185, 255), width=1)
    d.line([(95, 56), (82, 74)], fill=(140, 30, 30, 255), width=2)
    # Pectoral & ab definition
    d.arc([52, 54, 76, 68], 0, 180, fill=(145, 95, 60, 255), width=2)
    d.arc([84, 54, 108, 68], 0, 180, fill=(145, 95, 60, 255), width=2)

    # Horned Barbarian Skull-Helm
    d.ellipse([64, 20, 96, 48], fill=(90, 92, 98, 255))
    d.rectangle([70, 36, 90, 44], fill=(30, 20, 15, 255)) # Visor slot with burning red eyes
    d.ellipse([73, 38, 77, 42], fill=(255, 40, 20, 255))
    d.ellipse([83, 38, 87, 42], fill=(255, 40, 20, 255))
    # Giant Curved Beast Horns
    d.arc([42, 6, 76, 36], 120, 280, fill=(225, 215, 190, 255), width=5)
    d.arc([84, 6, 118, 36], 260, 420, fill=(225, 215, 190, 255), width=5)

    # Massive Iron Spiked Maul (Held across body)
    # Thick oak handle
    d.line([(24, 120), (124, 24)], fill=(110, 68, 35, 255), width=5)
    # Huge heavy iron maul head
    d.rectangle([14, 110, 46, 138], fill=(70, 72, 78, 255))
    d.rectangle([18, 114, 42, 134], fill=(105, 108, 115, 255))
    # Heavy steel spikes protruding from maul
    for sx, sy in [(12, 116), (12, 132), (48, 116), (48, 132), (30, 140)]:
        d.ellipse([sx - 3, sy - 3, sx + 3, sy + 3], fill=(215, 215, 225, 255))

    return im


def generate_boss_praetorian() -> Image.Image:
    w, h = 144, 144
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Ground shadow
    d.ellipse([30, 122, 114, 138], fill=(55, 30, 15, 150))

    # Flowing Imperial Purple Cape in Background
    d.polygon([(45, 46), (30, 120), (52, 124), (65, 52)], fill=(85, 15, 75, 255))
    d.polygon([(46, 48), (32, 118), (50, 122), (64, 54)], fill=(110, 22, 95, 255))

    # Segmented Greaves & Boots
    d.rectangle([54, 94, 65, 126], fill=(215, 175, 55, 255))
    d.rectangle([78, 94, 89, 126], fill=(215, 175, 55, 255))
    d.polygon([(50, 80), (94, 80), (88, 98), (56, 98)], fill=(155, 20, 25, 255)) # Pteruges leather strips
    for px in range(54, 90, 6):
        d.line([(px, 80), (px, 98)], fill=(212, 175, 55, 255), width=2)

    # Lorica Segmentata (Polished Imperial Steel/Bronze Banded Armor)
    d.polygon([(52, 50), (92, 50), (90, 82), (54, 82)], fill=(195, 150, 45, 255))
    for band_y in range(54, 80, 5):
        d.line([(52, band_y), (92, band_y)], fill=(245, 210, 80, 255), width=2)
    # Golden Gorgon Medallion on chest
    d.ellipse([67, 58, 77, 68], fill=(255, 225, 90, 255))

    # Ornate Oval Praetorian Shield (Left arm)
    d.ellipse([22, 60, 58, 108], fill=(110, 15, 30, 255))
    d.ellipse([24, 62, 56, 106], fill=(145, 20, 40, 255))
    d.ellipse([22, 60, 58, 108], outline=(225, 190, 60, 255), width=2)
    # Golden Praetorian Winged Thunderbolt
    d.polygon([(28, 84), (40, 72), (36, 84), (52, 74)], fill=(255, 220, 80, 255))
    d.polygon([(28, 84), (40, 96), (36, 84), (52, 94)], fill=(255, 220, 80, 255))

    # Attic Praetorian Helmet with Transverse Horsehair Crest
    d.ellipse([62, 28, 82, 48], fill=(220, 175, 55, 255))
    d.polygon([(60, 36), (84, 36), (82, 46), (62, 46)], fill=(140, 95, 20, 255)) # Cheek pieces
    # Transverse Purple & Crimson Crest (Side-to-side centurion plume)
    d.arc([52, 14, 92, 36], 180, 360, fill=(185, 20, 40, 255), width=6)
    d.arc([54, 16, 90, 34], 180, 360, fill=(120, 20, 90, 255), width=3)

    # Gleaming Spatha Sword (Right hand)
    d.line([(96, 68), (118, 100)], fill=(240, 240, 250, 255), width=3)
    d.ellipse([94, 66, 98, 70], fill=(212, 175, 55, 255)) # Pommel

    return im


def generate_boss_dimachaerus() -> Image.Image:
    w, h = 144, 144
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Ground shadow
    d.ellipse([30, 122, 114, 138], fill=(55, 30, 15, 150))

    # Agile Stance: Legs wide & flexed
    d.rectangle([46, 90, 58, 126], fill=(195, 140, 95, 255))
    d.rectangle([86, 90, 98, 126], fill=(195, 140, 95, 255))
    # Greaves with bronze banding
    for y in range(96, 124, 6):
        d.line([(46, y), (58, y)], fill=(212, 175, 55, 255), width=2)
        d.line([(86, y), (98, y)], fill=(212, 175, 55, 255), width=2)

    # Black leather subligaculum & crossed bandoliers
    d.polygon([(44, 76), (100, 76), (94, 92), (50, 92)], fill=(28, 26, 32, 255))
    # Crossed chest straps
    d.line([(48, 50), (96, 80)], fill=(75, 45, 25, 255), width=3)
    d.line([(96, 50), (48, 80)], fill=(75, 45, 25, 255), width=3)
    # Studs
    d.ellipse([70, 63, 74, 67], fill=(212, 175, 55, 255))

    # Hooded / Visored Helmet
    d.ellipse([62, 28, 82, 48], fill=(45, 42, 50, 255))
    # Bronze face mask with horizontal slit
    d.polygon([(64, 36), (80, 36), (78, 46), (66, 46)], fill=(185, 140, 45, 255))
    d.line([(65, 40), (79, 40)], fill=(10, 8, 12, 255), width=2)

    # Twin Crossed Sicae Swords (Dual-wield lethal pose)
    # Right Hand Sica (slashing down-left)
    d.line([(46, 68), (18, 48)], fill=(240, 240, 250, 255), width=3)
    d.line([(18, 48), (14, 28)], fill=(255, 255, 255, 255), width=3) # Forward hook blade
    # Left Hand Sica (slashing down-right)
    d.line([(98, 68), (126, 48)], fill=(240, 240, 250, 255), width=3)
    d.line([(126, 48), (130, 28)], fill=(255, 255, 255, 255), width=3) # Forward hook blade

    return im


# =============================================================================
# 4. GLADIATOR MINI-PORTRAITS (48x48 RGBA)
# =============================================================================

def generate_portrait(gladiator_type: str) -> Image.Image:
    im = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    # Circular background badge with gilded border
    d.ellipse([2, 2, 45, 45], fill=(45, 10, 10, 255))
    d.ellipse([2, 2, 45, 45], outline=(212, 175, 55, 255), width=2)

    # Torso & Neck
    d.ellipse([14, 26, 34, 46], fill=(185, 130, 85, 255))

    if gladiator_type == "murmillo":
        # Cassis crista helmet & red plume
        d.ellipse([16, 12, 32, 28], fill=(195, 145, 45, 255))
        d.rectangle([18, 18, 30, 24], fill=(80, 50, 15, 255))
        d.line([(18, 20), (30, 20)], fill=(20, 10, 5, 255), width=1)
        d.polygon([(20, 12), (24, 4), (28, 12)], fill=(200, 25, 25, 255))
    elif gladiator_type == "thraex":
        # Griffin helmet with blue feathers
        d.ellipse([16, 12, 32, 28], fill=(210, 160, 45, 255))
        d.rectangle([18, 18, 30, 24], fill=(90, 60, 20, 255))
        d.polygon([(22, 12), (24, 6), (26, 12)], fill=(245, 215, 80, 255))
        d.line([(16, 14), (12, 8)], fill=(30, 100, 180, 255), width=2)
        d.line([(32, 14), (36, 8)], fill=(30, 100, 180, 255), width=2)
    elif gladiator_type == "retiarius":
        # Bare head with dark curls & golden galerus on shoulder
        d.ellipse([17, 12, 31, 26], fill=(210, 155, 115, 255))
        d.arc([16, 10, 32, 20], 180, 360, fill=(60, 35, 15, 255), width=3)
        d.rectangle([10, 24, 18, 34], fill=(215, 165, 45, 255)) # Galerus shoulder guard
    elif gladiator_type == "hoplomachus":
        # Crested helmet with white side plumes
        d.ellipse([16, 12, 32, 28], fill=(195, 145, 45, 255))
        d.polygon([(20, 12), (24, 5), (28, 12)], fill=(230, 190, 60, 255))
        d.line([(16, 14), (12, 8)], fill=(240, 240, 245, 255), width=2)
        d.line([(32, 14), (36, 8)], fill=(240, 240, 245, 255), width=2)

    return im


def main():
    print("Generating KColosseum Flavian Amphitheatre arena backdrop...")
    bg = generate_colosseum_arena_bg()
    bg_path = BG_DIR / "colosseum_arena_bg.png"
    bg.save(bg_path, format="PNG", optimize=True)
    print(f"Saved background: {bg_path} ({bg.width}x{bg.height})")

    print("\nGenerating KColosseum gladiator class sprites...")
    save_sprite_with_atlas(generate_gladiator_murmillo(), "kcolosseum_gladiator_murmillo.png")
    save_sprite_with_atlas(generate_gladiator_thraex(), "kcolosseum_gladiator_thraex.png")
    save_sprite_with_atlas(generate_gladiator_retiarius(), "kcolosseum_gladiator_retiarius.png")
    save_sprite_with_atlas(generate_gladiator_hoplomachus(), "kcolosseum_gladiator_hoplomachus.png")

    print("\nGenerating KColosseum arena boss sprites...")
    save_sprite_with_atlas(generate_boss_lion(), "kcolosseum_boss_lion.png")
    save_sprite_with_atlas(generate_boss_chariot(), "kcolosseum_boss_chariot.png")
    save_sprite_with_atlas(generate_boss_behemoth(), "kcolosseum_boss_behemoth.png")
    save_sprite_with_atlas(generate_boss_praetorian(), "kcolosseum_boss_praetorian.png")
    save_sprite_with_atlas(generate_boss_dimachaerus(), "kcolosseum_boss_dimachaerus.png")

    print("\nGenerating KColosseum mini-portraits...")
    for gtype in ["murmillo", "thraex", "retiarius", "hoplomachus"]:
        p = generate_portrait(gtype)
        p_path = SPRITE_DIR / f"kcolosseum_portrait_{gtype}.png"
        p.save(p_path, format="PNG", optimize=True)
        print(f"Saved portrait: {p_path} ({p.width}x{p.height})")

    print("\nAll KColosseum visual assets generated successfully!")


if __name__ == "__main__":
    main()
