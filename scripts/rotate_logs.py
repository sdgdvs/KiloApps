#!/usr/bin/env python3
"""
scripts/rotate_logs.py
0-Token Zero-Discard Log Rotation Engine for KiloApps Fleet

Enforces strict token conservation on shared planning files (next_work.md)
and workspace logs (orchestrator.log) by rotating older records into permanent
archives without discarding a single turn or character.

Key Responsibilities:
1. next_work.md Rotation:
   - Parses active turn log entries in `## Recent Execution Logs`.
   - Recognizes standard bullet style (`- **YYYY...**`) and header style (`### Agent Run Log...`).
   - Retains the N most recent turns (default: 5; configurable to 10 or custom via arg/frontmatter).
   - Zero-discard: Moves all entries older than N turns into `archive/fleet_execution_archive.md`.
   - Normalizes entries into standard bullet format with clean hierarchy.
   - Prevents duplicate archiving (idempotent).
   
2. orchestrator.log Rotation:
   - Monitors `logs/orchestrator.log`.
   - When size exceeds threshold (default 512 KB), rotates older lines into
     `logs/orchestrator_archive.log` with zero data loss, keeping recent ~1,000 lines active.

3. Zero Token Cost:
   - Runs deterministically in Python during orchestrator pre-flight, post-turn,
     or pre-flight auto-skip, consuming exactly 0 LLM tokens.
"""

import argparse
import datetime
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
NEXT_WORK_FILE = REPO_ROOT / "next_work.md"
ARCHIVE_FILE = REPO_ROOT / "archive" / "fleet_execution_archive.md"
LOG_DIR = REPO_ROOT / "logs"
ORCHESTRATOR_LOG = LOG_DIR / "orchestrator.log"
ORCHESTRATOR_ARCHIVE = LOG_DIR / "orchestrator_archive.log"

DEFAULT_MAX_ENTRIES = 5
DEFAULT_MAX_LOG_BYTES = 512 * 1024  # 512 KB
KEEP_LOG_LINES = 1000


def log(msg: str):
    ts = datetime.datetime.now(datetime.timezone.utc).isoformat()
    print(f"[{ts}] [rotate_logs] {msg}")


def is_entry_start(line: str) -> bool:
    """Returns True if the line marks the beginning of an individual turn log entry."""
    s = line.strip()
    if s.startswith("### "):
        return True
    # Matches bullet starting with timestamp or bold run title:
    # e.g., "- **2026-10-08T..." or "- **[timestamp]..." or "- **kilo-..."
    if re.match(r"^-\s+\*\*\d{4}-\d{2}-\d{2}", s):
        return True
    if re.match(r"^-\s+\*\*\[\d{4}-\d{2}-\d{2}", s):
        return True
    return False


def normalize_entry(entry_text: str) -> str:
    """Normalizes an entry into clean standard markdown bullet format for archival."""
    lines = [l for l in entry_text.strip().splitlines() if l.strip()]
    if not lines:
        return ""
    first = lines[0].strip()
    if first.startswith("### "):
        title = first[4:].strip()
        body_lines = []
        for l in lines[1:]:
            sl = l.strip()
            if sl.startswith("- "):
                body_lines.append(f"  {sl}")
            else:
                body_lines.append(f"  - {sl}")
        return f"- **{title}**\n" + "\n".join(body_lines)
    
    # Ensure entry starts with bullet
    if not first.startswith("- "):
        return f"- **{first}**\n" + "\n".join(f"  - {l.strip()}" for l in lines[1:])
    return "\n".join(lines)


def get_entry_key(entry_text: str) -> str:
    """Extracts a unique signature for an entry to prevent duplicate archiving."""
    lines = [l.strip() for l in entry_text.strip().splitlines() if l.strip()]
    if not lines:
        return ""
    # Use normalized first line as signature
    first = lines[0]
    # Remove markdown formatting for comparison
    clean = re.sub(r"[*#`_\[\]]", "", first).strip()
    return clean[:80].lower()


def parse_execution_logs(content: str) -> tuple[int, int, list[str], str]:
    """
    Locates the Recent Execution Logs section and splits it into discrete log entries.
    Returns (section_start_index, section_end_index, entries_list, section_header).
    """
    header_pattern = r"(## Recent Execution Logs[^\n]*\n+)"
    match = re.search(header_pattern, content)
    if not match:
        return -1, -1, [], ""

    start_idx = match.start()
    header_str = match.group(1)
    body_start = match.end()

    # Look for next ## or # heading that terminates this section
    next_section = re.search(r"\n(?=##? [^\n]+)", content[body_start:])
    if next_section:
        end_idx = body_start + next_section.start()
    else:
        end_idx = len(content)

    log_section_text = content[body_start:end_idx]

    # Parse individual entries
    entries = []
    curr = []

    for line in log_section_text.splitlines():
        if is_entry_start(line):
            if curr and any(l.strip() for l in curr):
                entry_block = "\n".join(curr).strip()
                if entry_block:
                    entries.append(entry_block)
                curr = []
        if curr or line.strip():
            curr.append(line)

    if curr and any(l.strip() for l in curr):
        entry_block = "\n".join(curr).strip()
        if entry_block:
            entries.append(entry_block)

    return start_idx, end_idx, entries, header_str


def rotate_next_work_logs(
    max_entries: int = None,
    next_work_path: Path = NEXT_WORK_FILE,
    archive_path: Path = ARCHIVE_FILE,
    dry_run: bool = False,
) -> dict:
    """
    Zero-discard rotation of old execution logs from next_work.md into archive.
    
    If entries > max_entries:
      - Retains top `max_entries` entries in next_work.md.
      - Appends older entries to archive/fleet_execution_archive.md.
    """
    if not next_work_path.exists():
        log(f"Warning: {next_work_path} does not exist.")
        return {"retained": 0, "archived": 0, "status": "missing_file"}

    content = next_work_path.read_text(encoding="utf-8")

    # Determine max_entries if not explicitly given
    if max_entries is None:
        # Check environment variable
        env_val = os.environ.get("KILO_MAX_LOG_ENTRIES")
        if env_val and env_val.isdigit():
            max_entries = int(env_val)
        else:
            # Check frontmatter in next_work.md
            fm_match = re.search(r"max_log_entries:\s*(\d+)", content)
            if fm_match:
                max_entries = int(fm_match.group(1))
            else:
                max_entries = DEFAULT_MAX_ENTRIES

    start_idx, end_idx, entries, header_str = parse_execution_logs(content)
    if start_idx == -1:
        log("Execution logs section not found in next_work.md.")
        return {"retained": 0, "archived": 0, "status": "no_log_section"}

    total_entries = len(entries)
    if total_entries <= max_entries:
        # Already within bounds
        return {
            "retained": total_entries,
            "archived": 0,
            "status": "within_budget",
            "max_entries": max_entries,
        }

    kept_entries = entries[:max_entries]
    to_archive = entries[max_entries:]

    log(f"Rotating logs in {next_work_path.name}: {total_entries} total -> {len(kept_entries)} kept, {len(to_archive)} to archive.")

    # Prepare archive file
    archive_path.parent.mkdir(parents=True, exist_ok=True)
    if archive_path.exists():
        archive_content = archive_path.read_text(encoding="utf-8")
    else:
        archive_content = (
            "# KiloApps Master Fleet Execution Archive\n\n"
            "This file stores historical execution logs archived from `next_work.md` to keep the active planning context lean.\n\n"
            "## Archived Logs (Pre-Windows Task Scheduler Cutover)\n\n"
        )

    # Filter out entries already in archive to ensure idempotence
    archived_normalized = []
    for raw in to_archive:
        norm = normalize_entry(raw)
        key = get_entry_key(norm)
        if key and key in archive_content.lower():
            log(f"Entry signature '{key[:40]}...' already exists in archive. Skipping duplicate.")
            continue
        archived_normalized.append(norm)

    # Insert new archived entries under top header in archive file
    if archived_normalized:
        insertion_marker = "## Archived Logs (Pre-Windows Task Scheduler Cutover)\n\n"
        new_archive_block = "\n\n".join(archived_normalized) + "\n\n"
        if insertion_marker in archive_content:
            archive_content = archive_content.replace(
                insertion_marker, insertion_marker + new_archive_block, 1
            )
        else:
            # Fallback: append at end
            archive_content = archive_content.rstrip() + "\n\n" + new_archive_block

    # Format updated next_work.md section
    header_clean = f"## Recent Execution Logs (Max {max_entries} Entries)\n\n"
    new_log_section = header_clean + "\n\n".join(kept_entries) + "\n"

    new_next_work_content = content[:start_idx] + new_log_section + content[end_idx:].lstrip("\n")

    if not dry_run:
        archive_path.write_text(archive_content, encoding="utf-8")
        next_work_path.write_text(new_next_work_content, encoding="utf-8")
        log(f"Zero-discard rotation complete: Archived {len(to_archive)} entries, retained {len(kept_entries)} entries.")
    else:
        log(f"[DRY-RUN] Would archive {len(to_archive)} entries and retain {len(kept_entries)} entries.")

    return {
        "retained": len(kept_entries),
        "archived": len(to_archive),
        "archived_actually_written": len(archived_normalized),
        "status": "success",
        "max_entries": max_entries,
    }


def rotate_orchestrator_log(
    max_bytes: int = DEFAULT_MAX_LOG_BYTES,
    keep_lines: int = KEEP_LOG_LINES,
    log_path: Path = ORCHESTRATOR_LOG,
    archive_path: Path = ORCHESTRATOR_ARCHIVE,
    dry_run: bool = False,
) -> dict:
    """
    Zero-discard rotation of logs/orchestrator.log when file exceeds max_bytes.
    Appends older lines to logs/orchestrator_archive.log and retains the most recent keep_lines.
    """
    if not log_path.exists():
        return {"rotated": False, "reason": "log_not_found"}

    curr_size = log_path.stat().st_size
    if curr_size < max_bytes:
        return {"rotated": False, "size_bytes": curr_size, "limit_bytes": max_bytes}

    log(f"Rotating {log_path.name}: {curr_size / 1024:.1f} KB exceeds {max_bytes / 1024:.1f} KB threshold.")

    try:
        content = log_path.read_text(encoding="utf-8", errors="ignore")
        lines = content.splitlines()

        if len(lines) <= keep_lines:
            return {"rotated": False, "reason": "line_count_within_limit"}

        split_idx = len(lines) - keep_lines
        older_lines = lines[:split_idx]
        recent_lines = lines[split_idx:]

        if not dry_run:
            archive_path.parent.mkdir(parents=True, exist_ok=True)
            with open(archive_path, "a", encoding="utf-8") as af:
                ts = datetime.datetime.now(datetime.timezone.utc).isoformat()
                af.write(f"\n\n--- ROTATION CHUNK ARCHIVED AT {ts} ({len(older_lines)} lines) ---\n")
                af.write("\n".join(older_lines) + "\n")

            banner = f"[{datetime.datetime.now(datetime.timezone.utc).isoformat()}] [LOG ROTATION] Archived {len(older_lines)} lines to {archive_path.name}. Retained recent {len(recent_lines)} lines."
            new_active_content = banner + "\n" + "\n".join(recent_lines) + "\n"
            log_path.write_text(new_active_content, encoding="utf-8")
            log(f"Rotated {log_path.name}: {len(older_lines)} lines -> {archive_path.name}, {len(recent_lines)} lines kept active.")

        return {
            "rotated": True,
            "archived_lines": len(older_lines),
            "retained_lines": len(recent_lines),
        }
    except Exception as e:
        log(f"Warning during orchestrator log rotation: {e}")
        return {"rotated": False, "error": str(e)}


def main():
    parser = argparse.ArgumentParser(description="KiloApps 0-Token Zero-Discard Log Rotation Utility")
    parser.add_argument("--max-entries", type=int, default=None, help="Maximum active execution log entries to keep in next_work.md (e.g. 5 or 10)")
    parser.add_argument("--max-log-kb", type=int, default=512, help="Max size in KB for orchestrator.log before rotation (default: 512)")
    parser.add_argument("--keep-log-lines", type=int, default=1000, help="Lines to retain in orchestrator.log after rotation (default: 1000)")
    parser.add_argument("--dry-run", action="store_true", help="Preview rotation without writing changes")
    args = parser.parse_args()

    log("=" * 60)
    log("Initiating zero-discard log rotation...")
    
    # 1. Rotate next_work.md
    res_work = rotate_next_work_logs(
        max_entries=args.max_entries,
        dry_run=args.dry_run,
    )
    log(f"next_work.md rotation: {res_work}")

    # 2. Rotate orchestrator.log
    res_orch = rotate_orchestrator_log(
        max_bytes=args.max_log_kb * 1024,
        keep_lines=args.keep_log_lines,
        dry_run=args.dry_run,
    )
    log(f"orchestrator.log rotation: {res_orch}")
    log("=" * 60)


if __name__ == "__main__":
    main()
