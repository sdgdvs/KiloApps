# /// script
# dependencies = ["google-genai"]
# ///
"""
KiloApps Vision Audit — AI-powered visual quality scoring (HUMAN-ONLY FALLBACK).

NOTE: For autonomous agent usage, prefer the kilo-vision-audit agent skill in
.agents/skills/kilo-vision-audit/ which requires NO API key or external dependencies.
This script is preserved for humans who want to run vision audits manually outside
the agent system.

Sends app screenshots to Gemini Flash vision model for structured
quality evaluation. Outputs JSON scores and flags apps below threshold.

Usage:
    uv run --with google-genai scripts/vision_audit.py
    uv run --with google-genai scripts/vision_audit.py --threshold 6.0
    uv run --with google-genai scripts/vision_audit.py --app ksnake
    uv run --with google-genai scripts/vision_audit.py --baseline-compare

Requires: GEMINI_API_KEY environment variable.
"""

import argparse
import json
import os
import sys
import time
from pathlib import Path

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
SCREENSHOTS_DIR = WORKSPACE_ROOT / "docs" / "gallery" / "screenshots"
SCORES_PATH = WORKSPACE_ROOT / "docs" / "gallery" / "vision_scores.json"

EVAL_PROMPT = """You are evaluating a retro-themed (1999 aesthetic) single-file web application screenshot.

Rate each dimension from 1-10:
- LAYOUT: Is content properly arranged? No overlapping elements? Readable text? Proper spacing?
- CONTRAST: Can you read all text? Are interactive elements visually distinct from background?
- COMPLETENESS: Does the UI look finished? Are there blank areas that should have content? Are labels present?
- CONSISTENCY: Does the visual style match a coherent retro/90s theme?
- FUNCTIONALITY: Based on visible UI elements, does this look like a working, interactive application?

Return ONLY valid JSON (no markdown fencing, no explanation):
{"layout": N, "contrast": N, "completeness": N, "consistency": N, "functionality": N, "overall": N, "issues": ["issue1", "issue2"]}
where overall is the average of the 5 scores, rounded to 1 decimal."""


def evaluate_screenshot(client, image_path: Path) -> dict:
    """Send a screenshot to Gemini Flash and get structured scores."""
    from google.genai import types

    image_data = image_path.read_bytes()

    for attempt in range(3):
        try:
            response = client.models.generate_content(
                model="gemini-2.0-flash",
                contents=[
                    types.Content(parts=[
                        types.Part.from_text(EVAL_PROMPT),
                        types.Part.from_bytes(data=image_data, mime_type="image/png"),
                    ])
                ],
            )
            text = response.text.strip()
            # Strip markdown fencing if model includes it despite instruction
            if text.startswith("```"):
                text = text.split("\n", 1)[1].rsplit("```", 1)[0].strip()
            return json.loads(text)
        except (json.JSONDecodeError, Exception) as e:
            if attempt < 2:
                time.sleep(1 * (attempt + 1))
                continue
            return {
                "layout": 0, "contrast": 0, "completeness": 0,
                "consistency": 0, "functionality": 0, "overall": 0,
                "issues": [f"Vision API error: {str(e)[:100]}"],
                "error": True,
            }


def load_previous_scores() -> dict:
    """Load previous vision_scores.json if it exists."""
    if SCORES_PATH.exists():
        try:
            data = json.loads(SCORES_PATH.read_text())
            return {r["app"]: r for r in data.get("results", [])}
        except (json.JSONDecodeError, KeyError):
            pass
    return {}


def main():
    parser = argparse.ArgumentParser(description="KiloApps Vision Audit")
    parser.add_argument("--threshold", type=float, default=5.0,
                        help="Minimum acceptable overall score (default: 5.0)")
    parser.add_argument("--app", type=str, default=None,
                        help="Audit a single app by name (e.g. ksnake)")
    parser.add_argument("--baseline-compare", action="store_true",
                        help="Show score deltas vs previous run")
    parser.add_argument("--interact", action="store_true",
                        help="Audit post-interaction screenshots (*_interact.png) instead of initial")
    args = parser.parse_args()

    # Check API key
    api_key = os.environ.get("GEMINI_API_KEY")
    if not api_key:
        print("ERROR: GEMINI_API_KEY environment variable is not set.")
        print("")
        print("To set it:")
        print('  Windows:  $env:GEMINI_API_KEY = "your-key-here"')
        print('  Linux:    export GEMINI_API_KEY="your-key-here"')
        print("")
        print("Get a free key at: https://aistudio.google.com/apikey")
        sys.exit(1)

    from google import genai
    client = genai.Client(api_key=api_key)

    # Find screenshots
    if not SCREENSHOTS_DIR.exists():
        print(f"ERROR: Screenshots directory not found: {SCREENSHOTS_DIR}")
        print("Run the headless test suite first: node scripts/test_web_apps.js")
        sys.exit(1)

    if args.interact:
        screenshots = sorted(SCREENSHOTS_DIR.glob("*_interact.png"))
        scores_dest = WORKSPACE_ROOT / "docs" / "gallery" / "vision_scores_interact.json"
    else:
        screenshots = sorted(
            p for p in SCREENSHOTS_DIR.glob("*.png")
            if "_interact" not in p.name  # Skip post-interaction screenshots
        )
        scores_dest = SCORES_PATH

    if args.app:
        screenshots = [p for p in screenshots if p.stem == args.app]
        if not screenshots:
            print(f"ERROR: No screenshot found for app '{args.app}'")
            sys.exit(1)

    print(f"=== KiloApps Vision Quality Audit ===")
    print(f"Model: gemini-2.0-flash")
    print(f"Threshold: {args.threshold}")
    print(f"Apps to audit: {len(screenshots)}")
    print()

    previous = load_previous_scores() if args.baseline_compare else {}
    results = []
    flagged = 0

    # Table header
    print(f"{'App':<20} {'Overall':>7} {'Layout':>7} {'Contrast':>8} {'Complete':>9} {'Consist':>8} {'Funct':>6}  {'Delta':>6}  Issues")
    print("-" * 110)

    for i, ss_path in enumerate(screenshots):
        app_name = ss_path.stem
        print(f"  [{i+1}/{len(screenshots)}] {app_name}...", end="", flush=True)

        scores = evaluate_screenshot(client, ss_path)
        scores["app"] = app_name
        results.append(scores)

        overall = scores.get("overall", 0)
        is_flagged = overall < args.threshold or scores.get("error")

        if is_flagged:
            flagged += 1

        # Delta from previous run
        delta_str = ""
        if app_name in previous:
            prev_overall = previous[app_name].get("overall", 0)
            delta = overall - prev_overall
            if delta > 0:
                delta_str = f"+{delta:.1f} ↑"
            elif delta < 0:
                delta_str = f"{delta:.1f} ↓"
            else:
                delta_str = "  0  ="

        flag = " ⚠️" if is_flagged else " ✅"
        issues_str = "; ".join(scores.get("issues", [])[:2])
        if len(issues_str) > 40:
            issues_str = issues_str[:37] + "..."

        # Clear the progress text and print the result line
        print(f"\r{'App':<20} " if False else "", end="")
        print(
            f"\r  {app_name:<18} {overall:>7.1f} {scores.get('layout',0):>7} "
            f"{scores.get('contrast',0):>8} {scores.get('completeness',0):>9} "
            f"{scores.get('consistency',0):>8} {scores.get('functionality',0):>6}"
            f"  {delta_str:>6}  {issues_str}{flag}"
        )

        # Rate limiting: ~0.5s between calls to be polite
        if i < len(screenshots) - 1:
            time.sleep(0.5)

    # Summary
    print()
    print("-" * 110)
    avg_overall = sum(r.get("overall", 0) for r in results) / len(results) if results else 0
    print(f"  Fleet average: {avg_overall:.1f}/10")
    print(f"  Flagged (below {args.threshold}): {flagged}/{len(results)}")

    # Save report
    report = {
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "model": "gemini-2.0-flash",
        "threshold": args.threshold,
        "summary": {
            "total": len(results),
            "flagged": flagged,
            "average_overall": round(avg_overall, 1),
        },
        "results": results,
    }
    scores_dest.parent.mkdir(parents=True, exist_ok=True)
    scores_dest.write_text(json.dumps(report, indent=2))
    print(f"\n  Full report saved to {scores_dest.relative_to(WORKSPACE_ROOT)}")

    if flagged > 0:
        print(f"\n  ⚠️  {flagged} app(s) scored below {args.threshold} — review recommended.")
        sys.exit(1)
    else:
        print(f"\n  ✅ All apps passed the {args.threshold} threshold.")


if __name__ == "__main__":
    main()
