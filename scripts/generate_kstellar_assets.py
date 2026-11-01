# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
Generates high-fidelity spacefaring sprites for KStellar:
1. ship_player_cruiser (Player Battlecruiser Flagship)
2. ship_corsair_interceptor (Corsair Interceptor Raider)
3. ship_corsair_marauder (Corsair Marauder Heavy Cruiser)
4. ship_syndicate_dreadnought (Syndicate Dreadnought Capital Battleship)
5. ship_stealth_infiltrator (Syndicate Stealth Infiltrator)
6. fleet_federation_icon (Roaming Federation Patrol Cruiser)
7. fleet_traders_icon (Roaming Traders Guild Merchant Hauler)
8. fleet_pirates_icon (Roaming Pirate Raider Skiff)
9. orbital_station_facility (High-Tech Orbital Space Station)
10. anomaly_monolith (Alien Precursor Monolith)
11. anomaly_blackhole (Supermassive Black Hole Singularity)
12. anomaly_ionstorm (Ion Storm & Relativistic Pulsar Anomaly)
"""

import json
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter

ROOT = Path(__file__).resolve().parent.parent
SPRITE_DIR = ROOT / "KiloOS" / "public" / "assets" / "sprites" / "kstellar"
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


def draw_player_cruiser() -> Image.Image:
    # 128x128 Player Battlecruiser facing right
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Sublight plasma thruster exhaust trails
    flame_pts_upper = [(18, 56), (4, 58), (18, 60)]
    flame_pts_lower = [(18, 68), (4, 70), (18, 72)]
    d.polygon(flame_pts_upper, fill=(0, 255, 255, 230))
    d.polygon(flame_pts_lower, fill=(0, 255, 255, 230))
    # Afterburner central flame
    d.polygon([(18, 62), (0, 64), (18, 66)], fill=(255, 180, 0, 240))

    # Port & starboard warp nacelle booms
    d.polygon([(20, 42), (48, 42), (54, 48), (20, 48)], fill=(12, 38, 68, 255), outline=(0, 220, 255, 255))
    d.polygon([(20, 80), (54, 80), (48, 86), (20, 86)], fill=(12, 38, 68, 255), outline=(0, 220, 255, 255))
    # Nacelle plasma cooling grills
    d.line([(24, 45), (46, 45)], fill=(0, 255, 255, 220), width=2)
    d.line([(24, 83), (46, 83)], fill=(0, 255, 255, 220), width=2)

    # Main Titanium Hull Armor Plates (Delta Keel)
    hull_pts = [
        (116, 64),   # Needle sensor prow
        (88, 46),    # Forward shoulder
        (46, 40),    # Midship sweep
        (22, 52),    # Aft upper flank
        (18, 56),    # Engine block top
        (18, 72),    # Engine block bottom
        (22, 76),    # Aft lower flank
        (46, 88),    # Midship sweep lower
        (88, 82),    # Forward shoulder lower
    ]
    d.polygon(hull_pts, fill=(10, 32, 54, 255), outline=(0, 255, 204, 255))

    # Reinforced ceramic armor plating (inner facets)
    inner_facet_upper = [(96, 64), (82, 52), (52, 48), (34, 58), (56, 64)]
    inner_facet_lower = [(96, 64), (56, 64), (34, 70), (52, 80), (82, 76)]
    d.polygon(inner_facet_upper, fill=(18, 52, 82, 255), outline=(0, 180, 220, 200))
    d.polygon(inner_facet_lower, fill=(14, 42, 68, 255), outline=(0, 180, 220, 200))

    # Elevated Command Bridge Citadel & Cockpit Canopy
    d.ellipse([(70, 58), (92, 70)], fill=(0, 20, 35, 255), outline=(0, 255, 255, 255), width=1)
    d.ellipse([(74, 60), (88, 68)], fill=(0, 240, 255, 240))
    d.line([(76, 62), (84, 62)], fill=(255, 255, 255, 255), width=1) # Glare line

    # Wingtip Laser Mounts & Targeting Sensors
    d.rectangle([(42, 36), (62, 39)], fill=(0, 255, 204, 255))
    d.rectangle([(42, 89), (62, 92)], fill=(0, 255, 204, 255))
    d.line([(62, 37), (70, 37)], fill=(0, 255, 255, 255), width=1)
    d.line([(62, 90), (70, 90)], fill=(0, 255, 255, 255), width=1)

    # Dorsal Sensor Array & Communications Mast
    d.rectangle([(50, 62), (58, 66)], fill=(255, 200, 0, 255))
    d.line([(54, 58), (54, 70)], fill=(0, 255, 204, 255), width=1)

    return img


def draw_corsair_interceptor() -> Image.Image:
    # 128x128 Corsair Interceptor facing left
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # High-temperature twin thruster flames
    d.polygon([(110, 50), (126, 52), (110, 54)], fill=(255, 120, 0, 240))
    d.polygon([(110, 74), (126, 76), (110, 78)], fill=(255, 120, 0, 240))
    d.polygon([(110, 62), (122, 64), (110, 66)], fill=(255, 200, 50, 255))

    # Swept dagger wings with raider teeth
    wing_pts = [
        (12, 64),    # Needle prow
        (48, 48),    # Forward chine
        (84, 30),    # Upper swept wingtip
        (104, 44),   # Upper engine pod
        (94, 58),    # Central notch
        (108, 64),   # Rear spine
        (94, 70),    # Central notch
        (104, 84),   # Lower engine pod
        (84, 98),    # Lower swept wingtip
        (48, 80),    # Forward chine lower
    ]
    d.polygon(wing_pts, fill=(120, 32, 18, 255), outline=(255, 90, 40, 255))

    # Armor plating facets (inner shaded panels)
    d.polygon([(26, 64), (52, 54), (80, 42), (76, 64)], fill=(160, 45, 25, 255), outline=(255, 140, 60, 200))
    d.polygon([(26, 64), (76, 64), (80, 86), (52, 74)], fill=(90, 24, 12, 255), outline=(255, 140, 60, 200))

    # Raider yellow/amber cockpit targeting slit
    d.rectangle([(36, 61), (56, 67)], fill=(255, 220, 40, 255), outline=(255, 140, 0, 255))
    d.line([(40, 64), (52, 64)], fill=(255, 255, 200, 255), width=1)

    # Serrated Wingtip Razor Blades
    d.polygon([(84, 30), (96, 22), (90, 34)], fill=(255, 80, 30, 255))
    d.polygon([(84, 98), (96, 106), (90, 94)], fill=(255, 80, 30, 255))

    return img


def draw_corsair_marauder() -> Image.Image:
    # 128x128 Corsair Marauder Heavy Cruiser facing left
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Triple heavy exhaust plume
    d.polygon([(112, 46), (127, 48), (112, 50)], fill=(255, 60, 20, 240))
    d.polygon([(112, 62), (128, 64), (112, 66)], fill=(255, 140, 0, 255))
    d.polygon([(112, 78), (127, 80), (112, 82)], fill=(255, 60, 20, 240))

    # Jagged Ramming Prow & Heavy Battleship Hull
    hull_pts = [
        (10, 64),    # Heavy ramming blade tip
        (28, 48),    # Upper tooth
        (40, 52),    # Barb notch
        (66, 32),    # Forward shoulder
        (92, 34),    # Midship armor ridge
        (110, 44),   # Upper engine block
        (104, 64),   # Central spine
        (110, 84),   # Lower engine block
        (92, 94),    # Midship armor ridge lower
        (66, 96),    # Forward shoulder lower
        (40, 76),    # Barb notch lower
        (28, 80),    # Lower tooth
    ]
    d.polygon(hull_pts, fill=(84, 16, 24, 255), outline=(255, 50, 60, 255))

    # Heavy blast plating sections
    d.polygon([(26, 64), (54, 52), (86, 50), (74, 64)], fill=(120, 24, 32, 255), outline=(255, 100, 80, 200))
    d.polygon([(26, 64), (74, 64), (86, 78), (54, 76)], fill=(64, 10, 16, 255), outline=(255, 100, 80, 200))

    # Dual plasma cannon turrets
    d.rectangle([(44, 40), (58, 45)], fill=(220, 40, 40, 255))
    d.rectangle([(44, 83), (58, 88)], fill=(220, 40, 40, 255))
    d.line([(32, 42), (44, 42)], fill=(255, 120, 50, 255), width=2)
    d.line([(32, 85), (44, 85)], fill=(255, 120, 50, 255), width=2)

    # Command Bridge Eye
    d.ellipse([(48, 60), (64, 68)], fill=(255, 0, 50, 255), outline=(255, 200, 50, 255))
    d.ellipse([(52, 62), (60, 66)], fill=(255, 255, 200, 255))

    return img


def draw_syndicate_dreadnought() -> Image.Image:
    # 128x128 Syndicate Dreadnought Flagship facing left
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Massive 4-manifold thermonuclear thrust plumes
    d.polygon([(114, 42), (127, 44), (114, 46)], fill=(255, 20, 60, 255))
    d.polygon([(116, 54), (128, 56), (116, 58)], fill=(255, 80, 20, 255))
    d.polygon([(116, 70), (128, 72), (116, 74)], fill=(255, 80, 20, 255))
    d.polygon([(114, 82), (127, 84), (114, 86)], fill=(255, 20, 60, 255))

    # Armored Heavy Monolithic Prow & Superstructure
    prow_pts = [
        (8, 54),     # Upper prow corner
        (8, 74),     # Lower prow corner
        (28, 90),    # Forward kinetic armor slab
        (68, 98),    # Broadside beam flank
        (104, 90),   # Rear armor block
        (114, 80),   # Engine casing lower
        (114, 48),   # Engine casing upper
        (104, 38),   # Rear armor block upper
        (68, 30),    # Broadside beam flank upper
        (28, 38),    # Forward kinetic armor slab upper
    ]
    d.polygon(prow_pts, fill=(48, 8, 16, 255), outline=(255, 30, 50, 255))

    # Thick internal armor facets
    d.rectangle([(28, 48), (96, 80)], fill=(32, 4, 10, 255), outline=(180, 20, 40, 255))
    d.polygon([(8, 54), (28, 48), (28, 80), (8, 74)], fill=(64, 12, 22, 255), outline=(255, 40, 60, 255))

    # Tiered Command Citadel Tower
    d.rectangle([(50, 56), (82, 72)], fill=(20, 2, 6, 255), outline=(255, 50, 50, 255))
    # Dual crimson sensor slit viewports
    d.rectangle([(32, 57), (44, 61)], fill=(255, 0, 40, 255))
    d.rectangle([(32, 67), (44, 71)], fill=(255, 0, 40, 255))

    # Broadside Railgun Battery Barrels
    for by in [34, 40, 88, 94]:
        d.rectangle([(52, by), (74, by + 2)], fill=(255, 80, 80, 255))

    # Glowing red antimatter containment core
    d.ellipse([(60, 61), (72, 67)], fill=(255, 50, 50, 255), outline=(255, 200, 200, 255))

    return img


def draw_stealth_infiltrator() -> Image.Image:
    # 128x128 Syndicate Stealth Infiltrator facing left
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Violet/cyan shielded ion exhaust
    d.polygon([(112, 62), (126, 64), (112, 66)], fill=(180, 50, 255, 220))
    d.polygon([(108, 54), (118, 55), (108, 56)], fill=(0, 240, 255, 180))
    d.polygon([(108, 72), (118, 73), (108, 74)], fill=(0, 240, 255, 180))

    # Faceted Stealth Chine Fuselage
    hull_pts = [
        (12, 64),    # Razor needle nose
        (42, 56),    # Forward chine
        (80, 26),    # Swept stealth wing
        (96, 38),    # Wing notch
        (108, 52),   # Nacelle upper
        (102, 64),   # Center keel
        (108, 76),   # Nacelle lower
        (96, 90),    # Wing notch lower
        (80, 102),   # Swept stealth wing lower
        (42, 72),    # Forward chine lower
    ]
    d.polygon(hull_pts, fill=(35, 10, 52, 255), outline=(190, 70, 255, 255))

    # Radar-absorbent angular facets
    d.polygon([(24, 64), (48, 58), (76, 44), (68, 64)], fill=(55, 16, 80, 255), outline=(140, 45, 200, 180))
    d.polygon([(24, 64), (68, 64), (76, 84), (48, 70)], fill=(22, 6, 34, 255), outline=(140, 45, 200, 180))

    # Violet active sensor slit
    d.rectangle([(32, 62), (52, 66)], fill=(255, 0, 255, 255), outline=(255, 180, 255, 255))
    d.line([(34, 64), (50, 64)], fill=(255, 255, 255, 255), width=1)

    # Cloaking emitter lattice nodes
    for cx in [60, 75, 90]:
        d.ellipse([(cx - 1, 46), (cx + 1, 48)], fill=(0, 255, 255, 220))
        d.ellipse([(cx - 1, 80), (cx + 1, 82)], fill=(0, 255, 255, 220))

    return img


def draw_fleet_federation_icon() -> Image.Image:
    # 64x64 Federation Fleet Icon
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Twin warp nacelle trails
    d.line([(48, 22), (58, 22)], fill=(0, 255, 255, 200), width=2)
    d.line([(48, 42), (58, 42)], fill=(0, 255, 255, 200), width=2)

    # Saucer prow + engineering hull
    d.ellipse([(8, 22), (32, 42)], fill=(16, 52, 92, 255), outline=(0, 220, 255, 255), width=1)
    d.polygon([(24, 28), (48, 26), (48, 38), (24, 36)], fill=(10, 36, 68, 255), outline=(255, 200, 0, 255))

    # Glowing deflector dish
    d.ellipse([(12, 28), (18, 36)], fill=(0, 255, 255, 255))

    # Nacelle struts
    d.line([(36, 28), (46, 22)], fill=(0, 200, 255, 255), width=2)
    d.line([(36, 36), (46, 42)], fill=(0, 200, 255, 255), width=2)
    d.rectangle([(42, 20), (52, 24)], fill=(255, 200, 0, 255))
    d.rectangle([(42, 40), (52, 44)], fill=(255, 200, 0, 255))

    return img


def draw_fleet_traders_icon() -> Image.Image:
    # 64x64 Traders Guild Merchant Hauler Icon
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Heavy cargo pods
    d.rectangle([(16, 20), (32, 30)], fill=(160, 90, 0, 255), outline=(255, 170, 0, 255), width=1)
    d.rectangle([(16, 34), (32, 44)], fill=(160, 90, 0, 255), outline=(255, 170, 0, 255), width=1)
    d.rectangle([(30, 22), (46, 42)], fill=(120, 68, 0, 255), outline=(255, 200, 50, 255), width=1)

    # Command bridge front
    d.polygon([(8, 32), (18, 24), (18, 40)], fill=(40, 80, 110, 255), outline=(0, 240, 255, 255))

    # Heavy industrial drive block
    d.rectangle([(46, 24), (54, 40)], fill=(70, 40, 10, 255), outline=(255, 140, 0, 255))
    d.line([(54, 28), (62, 28)], fill=(255, 140, 0, 255), width=2)
    d.line([(54, 36), (62, 36)], fill=(255, 140, 0, 255), width=2)

    return img


def draw_fleet_pirates_icon() -> Image.Image:
    # 64x64 Pirate Raider Skiff Icon
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Aggressive serrated hull
    pts = [(8, 32), (28, 16), (24, 26), (46, 22), (40, 32), (46, 42), (24, 38), (28, 48)]
    d.polygon(pts, fill=(90, 15, 25, 255), outline=(255, 60, 60, 255))

    # Central red optic eye
    d.ellipse([(20, 29), (26, 35)], fill=(255, 220, 0, 255))

    # Aft thruster fire
    d.polygon([(46, 30), (56, 32), (46, 34)], fill=(255, 100, 0, 240))

    return img


def draw_orbital_station() -> Image.Image:
    # 128x128 High-Tech Orbital Space Station
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Rotating habitat toroid ring
    d.ellipse([(14, 44), (114, 84)], outline=(0, 200, 255, 180), width=4)
    d.ellipse([(20, 48), (108, 80)], outline=(0, 255, 255, 255), width=2)

    # Station Central Hub Column
    d.rectangle([(54, 22), (74, 106)], fill=(12, 32, 54, 255), outline=(0, 255, 204, 255), width=2)

    # Connecting structural truss spokes
    d.line([(20, 64), (54, 64)], fill=(0, 255, 255, 255), width=2)
    d.line([(74, 64), (108, 64)], fill=(0, 255, 255, 255), width=2)
    d.line([(34, 52), (58, 60)], fill=(0, 200, 220, 200), width=1)
    d.line([(94, 52), (70, 60)], fill=(0, 200, 220, 200), width=1)
    d.line([(34, 76), (58, 68)], fill=(0, 200, 220, 200), width=1)
    d.line([(94, 76), (70, 68)], fill=(0, 200, 220, 200), width=1)

    # Photovoltaic Solar Array Wings
    d.rectangle([(10, 16), (46, 26)], fill=(0, 60, 120, 255), outline=(0, 255, 255, 255), width=1)
    d.rectangle([(82, 16), (118, 26)], fill=(0, 60, 120, 255), outline=(0, 255, 255, 255), width=1)
    for sx in range(16, 46, 6):
        d.line([(sx, 16), (sx, 26)], fill=(0, 200, 255, 180), width=1)
    for sx in range(88, 118, 6):
        d.line([(sx, 16), (sx, 26)], fill=(0, 200, 255, 180), width=1)
    # Solar boom supports
    d.line([(46, 21), (54, 30)], fill=(0, 255, 204, 255), width=2)
    d.line([(82, 21), (74, 30)], fill=(0, 255, 204, 255), width=2)

    # Communications Spire & Navigation Beacon at Apex
    d.line([(64, 6), (64, 22)], fill=(0, 255, 255, 255), width=2)
    d.ellipse([(62, 4), (66, 8)], fill=(255, 230, 0, 255))

    # Docking Pylons & Port Facilities at Base
    d.rectangle([(50, 96), (78, 104)], fill=(18, 52, 82, 255), outline=(0, 255, 204, 255))
    d.line([(48, 100), (40, 108)], fill=(0, 255, 255, 255), width=2)
    d.line([(80, 100), (88, 108)], fill=(0, 255, 255, 255), width=2)

    return img


def draw_anomaly_monolith() -> Image.Image:
    # 128x128 Alien Precursor Monolith Anomaly
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Gravitational distortion energy rings (static elliptical fields)
    d.ellipse([(20, 36), (108, 92)], outline=(0, 255, 136, 90), width=1)
    d.ellipse([(28, 44), (100, 84)], outline=(0, 255, 136, 140), width=1)

    # Monolith Obelisk (Obsidian 1:4:9 geometry with emerald runic carvings)
    mono_pts = [(48, 18), (80, 18), (76, 110), (44, 110)]
    d.polygon(mono_pts, fill=(6, 18, 14, 255), outline=(0, 255, 136, 255), width=2)

    # Facet shade for 3D depth
    d.polygon([(64, 18), (80, 18), (76, 110), (62, 110)], fill=(2, 10, 8, 255))

    # Glowing Precursor Glyphs
    glyphs = [
        [(52, 32), (60, 32), (56, 42)],
        [(54, 50), (60, 56), (52, 62)],
        [(56, 72), (62, 78), (54, 84), (60, 90)],
        [(68, 36), (72, 42), (66, 48)],
        [(66, 62), (72, 62), (68, 74)],
    ]
    for g in glyphs:
        d.line(g, fill=(0, 255, 136, 255), width=2)

    # Orbiting ancient resonance shards
    for (sx, sy) in [(30, 48), (96, 54), (28, 80), (98, 76)]:
        d.polygon([(sx, sy - 4), (sx + 4, sy), (sx, sy + 4), (sx - 4, sy)], fill=(0, 255, 180, 230), outline=(255, 255, 255, 255))

    return img


def draw_anomaly_blackhole() -> Image.Image:
    # 128x128 Supermassive Black Hole
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Gravitational Lensing & Relativistic Accretion Ring
    d.ellipse([(8, 42), (120, 86)], outline=(255, 0, 255, 220), width=4)
    d.ellipse([(14, 46), (114, 82)], outline=(0, 240, 255, 200), width=3)
    d.ellipse([(20, 50), (108, 78)], outline=(255, 200, 255, 180), width=2)

    # Vertical Relativistic Plasma Jets
    d.polygon([(62, 4), (66, 4), (68, 54), (60, 54)], fill=(0, 240, 255, 200))
    d.polygon([(62, 124), (66, 124), (68, 74), (60, 74)], fill=(0, 240, 255, 200))

    # Event Horizon Core (Absolute Void)
    d.ellipse([(44, 44), (84, 84)], fill=(0, 0, 0, 255), outline=(255, 0, 255, 255), width=2)
    # Photon Sphere Glow
    d.ellipse([(42, 42), (86, 86)], outline=(255, 255, 255, 160), width=1)

    return img


def draw_anomaly_ionstorm() -> Image.Image:
    # 128x128 Ion Storm / Pulsar Anomaly
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Swirling ionized plasma filaments
    d.ellipse([(16, 26), (112, 102)], outline=(51, 170, 255, 160), width=3)
    d.ellipse([(24, 34), (104, 94)], outline=(160, 80, 255, 180), width=2)

    # Dendritic electric arc lines
    arcs = [
        [(24, 40), (44, 52), (64, 48), (84, 38)],
        [(30, 80), (50, 72), (70, 80), (96, 76)],
        [(64, 20), (58, 44), (68, 84), (64, 108)],
        [(36, 64), (54, 60), (74, 68), (92, 64)],
    ]
    for arc in arcs:
        d.line(arc, fill=(0, 255, 255, 230), width=2)

    # Pulsating High-Density Magnetar Core
    d.ellipse([(52, 52), (76, 76)], fill=(200, 240, 255, 255), outline=(51, 170, 255, 255), width=2)
    d.ellipse([(58, 58), (70, 70)], fill=(255, 255, 255, 255))

    return img


def main():
    save_sprite(draw_player_cruiser(), "ship_player_cruiser.png")
    save_sprite(draw_corsair_interceptor(), "ship_corsair_interceptor.png")
    save_sprite(draw_corsair_marauder(), "ship_corsair_marauder.png")
    save_sprite(draw_syndicate_dreadnought(), "ship_syndicate_dreadnought.png")
    save_sprite(draw_stealth_infiltrator(), "ship_stealth_infiltrator.png")
    save_sprite(draw_fleet_federation_icon(), "fleet_federation_icon.png")
    save_sprite(draw_fleet_traders_icon(), "fleet_traders_icon.png")
    save_sprite(draw_fleet_pirates_icon(), "fleet_pirates_icon.png")
    save_sprite(draw_orbital_station(), "orbital_station_facility.png")
    save_sprite(draw_anomaly_monolith(), "anomaly_monolith.png")
    save_sprite(draw_anomaly_blackhole(), "anomaly_blackhole.png")
    save_sprite(draw_anomaly_ionstorm(), "anomaly_ionstorm.png")
    print("All KStellar spacefaring sprites successfully created.")


if __name__ == "__main__":
    main()
