#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pyyaml>=6.0",
# ]
# ///
"""
KiloApps Fleet Receipt Reconciler
Ingests pending turn receipts from `.agents/receipts/` and reconciles them into `next_work.md`.
Eliminates Git merge conflicts by allowing distributed runners to write atomic receipt files
rather than concurrently editing `next_work.md`.
"""

import argparse
import datetime
import json
import os
import shutil
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
RECEIPTS_DIR = REPO_ROOT / ".agents" / "receipts"
ARCHIVE_DIR = RECEIPTS_DIR / "archive"
NEXT_WORK_FILE = REPO_ROOT / "next_work.md"
EXECUTION_ARCHIVE_FILE = REPO_ROOT / "archive" / "fleet_execution_archive.md"


def log(msg: str):
    timestamp = datetime.datetime.now(datetime.timezone.utc).isoformat()
    print(f"[{timestamp}] [reconcile] {msg}")


def parse_frontmatter(content: str) -> tuple[dict, str, str]:
    """Splits next_work.md into (frontmatter_dict, raw_frontmatter, body)."""
    parts = content.split("---", 2)
    if len(parts) < 3:
        raise ValueError("Invalid format: next_work.md missing YAML frontmatter markers (---).")
    raw_yaml = parts[1]
    body = parts[2]

    try:
        import yaml
        data = yaml.safe_load(raw_yaml) or {}
    except ImportError:
        # Minimal fallback parser
        data = {}
        for line in raw_yaml.splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if ":" in line:
                k, v = line.split(":", 1)
                data[k.strip()] = v.strip().strip('"').strip("'")

    return data, raw_yaml, body


def dump_frontmatter(data: dict) -> str:
    """Serializes frontmatter dict back to clean YAML."""
    try:
        import yaml
        # Custom dumper to keep clean formatting
        class CleanDumper(yaml.SafeDumper):
            pass
        return yaml.dump(data, Dumper=CleanDumper, sort_keys=False, default_flow_style=False)
    except ImportError:
        # Fallback simple serializer
        lines = []
        for k, v in data.items():
            if isinstance(v, list):
                lines.append(f"{k}:")
                for item in v:
                    lines.append(f"  - {item}")
            elif isinstance(v, dict):
                lines.append(f"{k}:")
                for sub_k, sub_v in v.items():
                    lines.append(f"  {sub_k}: {sub_v}")
            else:
                lines.append(f"{k}: {v}")
        return "\n".join(lines) + "\n"


def advance_rotation(current_agent: str, rotation: list[str]) -> tuple[str, str]:
    """Given current agent and rotation list, returns (next_current, next_upcoming)."""
    if not rotation:
        return current_agent, current_agent
    try:
        idx = rotation.index(current_agent)
    except ValueError:
        idx = 0
    next_idx = (idx + 1) % len(rotation)
    subsequent_idx = (next_idx + 1) % len(rotation)
    return rotation[next_idx], rotation[subsequent_idx]


def reconcile_receipts(dry_run: bool = False) -> int:
    if not RECEIPTS_DIR.exists():
        log(f"Receipts directory {RECEIPTS_DIR} does not exist. Nothing to reconcile.")
        return 0

    receipt_files = sorted(
        [p for p in RECEIPTS_DIR.glob("receipt_*.json") if p.is_file()],
        key=lambda p: p.name
    )

    if not receipt_files:
        log("No pending turn receipts found.")
        return 0

    log(f"Found {len(receipt_files)} pending receipt(s) to reconcile.")

    if not NEXT_WORK_FILE.exists():
        log(f"Error: {NEXT_WORK_FILE} does not exist.")
        return 1

    content = NEXT_WORK_FILE.read_text(encoding="utf-8")
    fm, raw_fm, body = parse_frontmatter(content)

    rotation = fm.get("agent_rotation", [
        "kilo-creator",
        "kilo-graphics",
        "kilo-tester",
        "kilo-usability",
        "kilo-qa",
        "kilo-expander",
    ])

    new_log_entries = []

    for rf in receipt_files:
        try:
            data = json.loads(rf.read_text(encoding="utf-8"))
        except Exception as e:
            log(f"Warning: Failed to parse receipt {rf.name}: {e}. Skipping.")
            continue

        agent = data.get("agent", "unknown")
        model = data.get("model", "unknown")
        target = data.get("target", "unspecified")
        timestamp = data.get("timestamp", datetime.datetime.now(datetime.timezone.utc).isoformat())
        duration = data.get("duration_seconds", 0)
        success = data.get("success", True)
        summary = data.get("summary", "")

        status_icon = "PASS" if success else "FAIL"
        log(f"Processing receipt: {rf.name} ({agent} on '{target}', status: {status_icon})")

        # Update last_run
        fm["last_run"] = {
            "agent": agent,
            "app": target,
            "timestamp": timestamp,
        }

        # Advance rotation
        next_curr, next_up = advance_rotation(agent, rotation)
        fm["current_agent"] = next_curr
        fm["next_agent"] = next_up

        entry = (
            f"- **[{timestamp}] {agent}** (`{model}`, {duration}s) - Target: `{target}` [{status_icon}]\n"
        )
        if summary:
            entry += f"  - {summary}\n"
        new_log_entries.append(entry)

    log(f"Advanced queue to current_agent='{fm.get('current_agent')}', next_agent='{fm.get('next_agent')}'")

    # Format updated next_work.md content
    new_yaml = dump_frontmatter(fm).strip()
    
    # Inject new log entries into body if execution log header exists
    log_header = "## Recent Execution Log"
    if log_header in body and new_log_entries:
        parts = body.split(log_header, 1)
        # Prepend new entries right after the header
        body = parts[0] + log_header + "\n\n" + "".join(new_log_entries) + parts[1].lstrip("\n")

    new_full_content = f"---\n{new_yaml}\n---\n{body}"

    if dry_run:
        log("Dry-run enabled. Changes were NOT written to disk.")
        log(f"Preview of updated frontmatter:\n{new_yaml}")
        return 0

    # Write updated next_work.md
    NEXT_WORK_FILE.write_text(new_full_content, encoding="utf-8")
    log(f"Successfully updated {NEXT_WORK_FILE.name}.")

    # Move processed receipts to archive
    ARCHIVE_DIR.mkdir(parents=True, exist_ok=True)
    for rf in receipt_files:
        dest = ARCHIVE_DIR / rf.name
        shutil.move(str(rf), str(dest))
        log(f"Archived receipt: {rf.name} -> archive/")

    log("Reconciliation complete.")
    return 0


def main():
    parser = argparse.ArgumentParser(description="KiloApps Turn Receipt Reconciler")
    parser.add_argument("--dry-run", action="store_true", help="Preview reconciliation without modifying files")
    args = parser.parse_args()
    sys.exit(reconcile_receipts(dry_run=args.dry_run))


if __name__ == "__main__":
    main()
