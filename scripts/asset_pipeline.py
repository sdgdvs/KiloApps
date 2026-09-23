#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pillow>=10.0",
#     "numpy>=1.24",
# ]
# ///
"""
KiloApps 2D Game Asset Pipeline
Automates Stage 2 post-processing for Imagen 3 game assets:
1. Sprite processing:
   - Key out #FF00FF magenta background to alpha = 0 (32-bit RGBA)
   - Auto-crop and center sprite inside a uniform 128x128 bounding box
   - Pack 4 frames into a 512x128 horizontal sprite strip (run_strip4.png)
   - Generate Phaser / Unity compatible JSON coordinate atlas (run_strip4.json)
2. Seamless texture processing:
   - Run ImageChops.offset(512, 512) seam test
   - Quadrant seam blending to guarantee mathematical tileability
   - Export seamless albedo map (1024x1024) and offset diagnostic verification
"""

import argparse
import json
import os
import sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageChops


def key_out_magenta(
    image: Image.Image,
    target_color=(255, 0, 255),
    tolerance_hard: float = 85.0,
    fringe_tol: float = 55.0,
    despill: bool = True
) -> Image.Image:
    """
    Keys out solid magenta (#FF00FF) background to alpha = 0.
    Handles compression artifacts, fringe pixels near dark silhouettes,
    and despills residual magenta on boundary borders.
    """
    rgb_img = image.convert("RGB")
    arr = np.array(rgb_img, dtype=np.float32)

    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    tr, tg, tb = target_color

    # Distance to target chroma color
    dist_pure = np.sqrt((r - tr) ** 2 + (g - tg) ** 2 + (b - tb) ** 2)

    # Magenta prominence metric: (r and b dominant over g, and balanced)
    magenta_strength = np.maximum(0.0, np.minimum(r, b) - g * 1.1)
    color_balance = np.abs(r - b) / (np.maximum(r, b) + 1e-5)

    # Key background pixels (pure background + dark magenta compression border)
    is_bg = (dist_pure < tolerance_hard) | (
        (magenta_strength > fringe_tol) & (color_balance < 0.3) & (dist_pure < 220.0)
    )
    alpha = np.where(is_bg, 0, 255).astype(np.uint8)

    # Despill magenta fringe on edge pixels that remain opaque
    r_out = r.copy()
    b_out = b.copy()
    if despill:
        fringe = (magenta_strength > 25.0) & (color_balance < 0.4) & (alpha > 0)
        r_out = np.where(fringe, np.clip(r - magenta_strength * 0.9, 0, 255), r)
        b_out = np.where(fringe, np.clip(b - magenta_strength * 0.9, 0, 255), b)

    rgba_arr = np.dstack([
        r_out.astype(np.uint8),
        arr[:, :, 1].astype(np.uint8),
        b_out.astype(np.uint8),
        alpha
    ])

    return Image.fromarray(rgba_arr, mode="RGBA")


def resize_and_center_sprite(
    sprite: Image.Image,
    target_box: tuple[int, int] = (128, 128),
    padding: int = 6
) -> Image.Image:
    """
    Finds sprite silhouette bounding box, scales proportionally to fit
    within target_box (minus padding), and centers it in the target bounding box.
    """
    box_w, box_h = target_box
    bbox = sprite.getbbox()
    if not bbox:
        return Image.new("RGBA", target_box, (0, 0, 0, 0))

    # Crop to silhouette
    cropped = sprite.crop(bbox)
    cw, ch = cropped.size

    avail_w = max(1, box_w - 2 * padding)
    avail_h = max(1, box_h - 2 * padding)

    scale = min(avail_w / cw, avail_h / ch)
    new_w = max(1, int(round(cw * scale)))
    new_h = max(1, int(round(ch * scale)))

    # Use LANCZOS for high quality downsampling
    resized = cropped.resize((new_w, new_h), resample=Image.Resampling.LANCZOS)

    # Center in bounding box
    out_img = Image.new("RGBA", target_box, (0, 0, 0, 0))
    paste_x = (box_w - new_w) // 2
    paste_y = (box_h - new_h) // 2
    out_img.paste(resized, (paste_x, paste_y), mask=resized)

    return out_img


def pack_sprite_strip(
    frames: list[Image.Image],
    frame_size: tuple[int, int] = (128, 128)
) -> Image.Image:
    """
    Packs a sequence of N uniform sprite frames into an (N*w) x h horizontal sprite sheet.
    """
    fw, fh = frame_size
    n_frames = len(frames)
    strip_w = fw * n_frames
    strip_h = fh

    strip = Image.new("RGBA", (strip_w, strip_h), (0, 0, 0, 0))
    for i, frame in enumerate(frames):
        strip.paste(frame, (i * fw, 0), mask=frame)

    return strip


def generate_phaser_unity_atlas(
    num_frames: int,
    image_filename: str,
    frame_size: tuple[int, int] = (128, 128),
    prefix: str = "run"
) -> dict:
    """
    Generates a standard Phaser / Unity compatible JSON hash coordinate atlas.
    Compatible with Phaser's this.load.atlas() and Unity's TexturePacker JSON importer.
    """
    fw, fh = frame_size
    total_w = fw * num_frames
    total_h = fh

    frames_dict = {}
    for i in range(num_frames):
        key = f"{prefix}_{i}"
        frames_dict[key] = {
            "frame": {
                "x": i * fw,
                "y": 0,
                "w": fw,
                "h": fh
            },
            "rotated": False,
            "trimmed": False,
            "spriteSourceSize": {
                "x": 0,
                "y": 0,
                "w": fw,
                "h": fh
            },
            "sourceSize": {
                "w": fw,
                "h": fh
            },
            "pivot": {
                "x": 0.5,
                "y": 0.5
            }
        }

    atlas = {
        "frames": frames_dict,
        "meta": {
            "app": "KiloApps Asset Pipeline",
            "version": "1.0",
            "image": image_filename,
            "format": "RGBA8888",
            "size": {"w": total_w, "h": total_h},
            "scale": "1"
        }
    }
    return atlas


def process_sprites_pipeline(
    frame_paths: list[str],
    output_strip_path: str,
    output_atlas_path: str,
    output_individual_dir: str | None = None,
    box_size: int = 128
) -> tuple[Image.Image, dict]:
    """
    Executes the full sprite pipeline:
    - Loads all frame images
    - Keys out #FF00FF to alpha = 0 (saving 32-bit RGBA)
    - Crops and centers inside uniform (box_size, box_size)
    - Packs into horizontal sprite sheet
    - Writes coordinate JSON atlas
    """
    processed_frames = []
    if output_individual_dir:
        os.makedirs(output_individual_dir, exist_ok=True)

    for i, path in enumerate(frame_paths):
        raw = Image.open(path)
        keyed = key_out_magenta(raw)

        if output_individual_dir:
            indiv_path = Path(output_individual_dir) / f"frame_{i}_rgba.png"
            keyed.save(indiv_path, format="PNG")

        centered = resize_and_center_sprite(keyed, target_box=(box_size, box_size))
        processed_frames.append(centered)

    # Pack horizontal strip
    strip = pack_sprite_strip(processed_frames, frame_size=(box_size, box_size))
    os.makedirs(Path(output_strip_path).parent, exist_ok=True)
    strip.save(output_strip_path, format="PNG")

    # Generate Atlas
    atlas = generate_phaser_unity_atlas(
        num_frames=len(processed_frames),
        image_filename=os.path.basename(output_strip_path),
        frame_size=(box_size, box_size)
    )
    os.makedirs(Path(output_atlas_path).parent, exist_ok=True)
    with open(output_atlas_path, "w", encoding="utf-8") as f:
        json.dump(atlas, f, indent=2)

    return strip, atlas


def blend_seamless_texture(
    image: Image.Image,
    transition_ratio: float = 0.45
) -> tuple[Image.Image, Image.Image]:
    """
    Guarantees mathematical tileability by offset seam blending:
    1. Computes B = ImageChops.offset(A, w // 2, h // 2).
       In B, the borders are continuous/seamless because they originate
       from the center of A. The center of B contains the former borders.
    2. Builds a smooth 2D transition mask M:
       M = 1.0 at outer boundaries (where B is continuous)
       M = 0.0 at center (where A is continuous)
    3. Composite = (1 - M) * A + M * B.
    4. Applies ImageChops.offset(512, 512) to produce the diagnostic verification.
    """
    w, h = image.size
    img_rgb = image.convert("RGB")

    # Half offset
    offset_half = ImageChops.offset(img_rgb, w // 2, h // 2)

    # 2D smoothstep mask
    y = np.linspace(-1.0, 1.0, h, dtype=np.float32)[:, None]
    x = np.linspace(-1.0, 1.0, w, dtype=np.float32)[None, :]
    d = np.maximum(np.abs(x), np.abs(y))  # 0 at center, 1 at boundaries

    # Smoothstep taper
    t_start = 0.35
    t_end = min(0.90, t_start + transition_ratio)
    t = np.clip((d - t_start) / (t_end - t_start), 0.0, 1.0)
    smooth_mask = t * t * (3.0 - 2.0 * t)

    mask_l = Image.fromarray((smooth_mask * 255.0).astype(np.uint8), mode="L")

    # Composite: mask=0 selects img_rgb (center), mask=255 selects offset_half (borders)
    seamless = Image.composite(offset_half, img_rgb, mask_l)

    # Diagnostic offset test: should show zero visible seams anywhere
    offset_test = ImageChops.offset(seamless, w // 2, h // 2)

    return seamless, offset_test


def process_texture_pipeline(
    input_texture_path: str,
    output_seamless_path: str,
    output_offset_path: str | None = None,
    quantize_colors: int | None = None
) -> tuple[Image.Image, Image.Image]:
    """
    Executes seamless texture processing:
    - Loads 1024x1024 albedo texture
    - Blends quadrant seams
    - Optionally quantizes colors (e.g. 256 for retro <999KB ceiling)
    - Saves verified seamless texture
    - Generates offset test verification
    """
    raw_img = Image.open(input_texture_path)
    seamless, offset_test = blend_seamless_texture(raw_img)

    os.makedirs(Path(output_seamless_path).parent, exist_ok=True)
    if quantize_colors and quantize_colors > 0:
        paletted = seamless.quantize(colors=quantize_colors)
        paletted.save(output_seamless_path, format="PNG", optimize=True)
    else:
        seamless.save(output_seamless_path, format="PNG", optimize=True)

    if output_offset_path:
        os.makedirs(Path(output_offset_path).parent, exist_ok=True)
        offset_test.save(output_offset_path, format="PNG", optimize=True)

    return seamless, offset_test


def main():
    parser = argparse.ArgumentParser(description="KiloApps 2D Game Asset Pipeline")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # process-sprites
    sp = subparsers.add_parser("process-sprites", help="Key magenta, crop, center, pack strip and generate atlas")
    sp.add_argument("--frames", nargs="+", required=True, help="List of input frame paths in order")
    sp.add_argument("--out-strip", default="run_strip4.png", help="Output horizontal sprite sheet path")
    sp.add_argument("--out-atlas", default="run_strip4.json", help="Output JSON coordinate atlas path")
    sp.add_argument("--save-frames-dir", default=None, help="Optional directory to save individual 32-bit RGBA frames")
    sp.add_argument("--box-size", type=int, default=128, help="Bounding box size for each frame (default 128)")

    # process-texture
    tp = subparsers.add_parser("process-texture", help="Blend seams and test offset for seamless tiling")
    tp.add_argument("--input", required=True, help="Input albedo texture path")
    tp.add_argument("--out-seamless", default="cobblestone_seamless.png", help="Output seamless texture path")
    tp.add_argument("--out-offset-test", default="cobblestone_offset.png", help="Output offset test image path")
    tp.add_argument("--quantize", type=int, default=None, help="Optional color quantization (e.g. 256 for <999KB retro budget)")

    args = parser.parse_args()

    if args.command == "process-sprites":
        print(f"[*] Processing {len(args.frames)} sprite frames into {args.box_size}x{args.box_size} boxes...")
        strip, atlas = process_sprites_pipeline(
            frame_paths=args.frames,
            output_strip_path=args.out_strip,
            output_atlas_path=args.out_atlas,
            output_individual_dir=args.save_frames_dir,
            box_size=args.box_size
        )
        print(f"[+] Saved sprite strip: {args.out_strip} ({strip.size[0]}x{strip.size[1]} RGBA)")
        print(f"[+] Saved atlas: {args.out_atlas} ({len(atlas['frames'])} frames mapped)")

    elif args.command == "process-texture":
        print(f"[*] Processing texture {args.input} for seamless tileability...")
        seamless, offset_test = process_texture_pipeline(
            input_texture_path=args.input,
            output_seamless_path=args.out_seamless,
            output_offset_path=args.out_offset_test,
            quantize_colors=args.quantize
        )
        print(f"[+] Saved seamless texture: {args.out_seamless} ({seamless.size[0]}x{seamless.size[1]})")
        if args.out_offset_test:
            print(f"[+] Saved offset test image: {args.out_offset_test}")


if __name__ == "__main__":
    main()
