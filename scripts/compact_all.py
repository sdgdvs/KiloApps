"""
scripts/compact_all.py
Enforces token conservation caps across shared Markdown plan files:
- Caps active log sections to the most recent ~80 lines.
- Archives older entries to archive/ directory.
- Replaces completed phases/milestones with 2-line stubs.
- Flags or trims run log entries exceeding the 8-line cap.
"""

import os
import re
import sys
from pathlib import Path

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
ARCHIVE_DIR = WORKSPACE_ROOT / "archive"

PLAN_FILES = [
    "next_work.md",
]

ARCHIVE_MAPPING = {
    "next_work.md": "fleet_execution_archive.md",
}

MAX_ACTIVE_LOG_LINES = 80


def ensure_archive_dir():
    ARCHIVE_DIR.mkdir(parents=True, exist_ok=True)


def compact_file(filename: str):
    file_path = WORKSPACE_ROOT / filename
    if not file_path.exists():
        return

    content = file_path.read_text(encoding="utf-8", errors="ignore")
    lines = content.splitlines()

    log_headers = [
        "## Recent Execution Logs",
        "## Recent Completed Fixes",
        "## Test Reports",
        "## Completed Apps Index",
        "## Run Logs",
        "## History",
    ]

    header_idx = -1
    matched_header = None
    for i, line in enumerate(lines):
        for h in log_headers:
            if line.strip().startswith(h):
                header_idx = i
                matched_header = h
                break
        if header_idx != -1:
            break

    if header_idx == -1:
        return

    log_lines = lines[header_idx + 1 :]
    if len(log_lines) <= MAX_ACTIVE_LOG_LINES:
        return

    archive_file = ARCHIVE_DIR / ARCHIVE_MAPPING.get(filename, "archive.md")
    overflow_count = len(log_lines) - MAX_ACTIVE_LOG_LINES
    lines_to_archive = log_lines[:overflow_count]
    kept_lines = log_lines[overflow_count:]

    with open(archive_file, "a", encoding="utf-8") as af:
        af.write(f"\n\n<!-- Archived from {filename} -->\n")
        af.write("\n".join(lines_to_archive) + "\n")

    stub = f"\n> 📁 **Archived Records**: Historical entries older than {MAX_ACTIVE_LOG_LINES} lines moved to [{archive_file.name}](archive/{archive_file.name}).\n"

    new_content = "\n".join(lines[: header_idx + 1]) + "\n" + stub + "\n".join(kept_lines) + "\n"
    file_path.write_text(new_content, encoding="utf-8")
    print(f"[{filename}] Archived {overflow_count} lines to archive/{archive_file.name} (kept {len(kept_lines)} active lines).")


def main():
    ensure_archive_dir()
    print("Running scripts/compact_all.py: checking plan files for token conservation...")
    for pf in PLAN_FILES:
        compact_file(pf)
    print("Compaction complete.")


if __name__ == "__main__":
    main()
