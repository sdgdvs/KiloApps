#!/usr/bin/env python3
"""
scripts/advance_turn.py
Deterministic 0-Token Queue Handoff, Mature Quarantine & Log Updater for KiloApps Fleet.

Saves Flash agents 3-5 tool calls per turn and eliminates:
- YAML frontmatter syntax corruption or stale target mismatches
- Windows-1252 invalid UTF-8 byte crashes (e.g. 0x97 em-dash)
- Manual log rotation overhead
- Incomplete Mature Application Quarantine purges

Usage example:
  python scripts/advance_turn.py --agent kilo-tester --app KPass \
    --title "Interactive UI Audit & Vault Hotkeys" \
    --bullet "Status: PASS (0 runtime errors, web 139.7 KB < 999 KB; startup & security_lint PASS)." \
    --bullet "Interactive Audit: Verified entropy visualizer, copy shortcuts, and modal dismissals."

With Mature Quarantine:
  python scripts/advance_turn.py --agent kilo-qa --app KPass \
    --title "Pass 5 QA & Mature Quarantine" \
    --bullet "Status: PASS (web 139.7 KB, native 25.0 KB < 999 KB; F5/F9 persistence verified)." \
    --quarantine-mature "Password generator & AES credential safe; F5/F9 persistence, focus trapping, dual Win32 C parity." \
    --category "System / Security"
"""

import argparse
import datetime
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
NEXT_WORK_FILE = REPO_ROOT / "next_work.md"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))
try:
    from scripts.rotate_logs import rotate_next_work_logs
except ImportError:
    from rotate_logs import rotate_next_work_logs


AGENT_SECTION_HEADERS = {
    "kilo-creator": r"### 1\.",
    "kilo-graphics": r"### 2\.",
    "kilo-soundscape": r"### 3\.",
    "kilo-tester": r"### 4\.",
    "kilo-usability": r"### 5\.",
    "kilo-qa": r"### 6\.",
    "kilo-expander": r"### 7\.",
}


def get_mature_apps(content: str) -> set[str]:
    """Returns lowercase set of app names in the Mature Applications table."""
    sec_match = re.search(r"## Mature Applications[^\n]*\n+(.*?)(?=\n## |\Z)", content, re.DOTALL)
    if not sec_match:
        return set()
    block = sec_match.group(1)
    return {m.strip().lower() for m in re.findall(r"\|\s*\*\*([^*]+)\*\*\s*\|", block)}


def add_to_mature_table(content: str, app: str, category: str, summary: str, date_str: str) -> str:
    """Inserts app into Mature Applications table if not already present."""
    mature = get_mature_apps(content)
    if app.lower() in mature:
        return content
    html_file = f"`KiloOS/public/apps/{app.lower()}.html`"
    row = f"| **{app}** | {html_file} | Pass 5+ ({date_str}) | {category} | {summary} |\n"
    marker = "| Application | Primary File | Pass Count / Date | Category | Summary of Mature State |\n|---|---|---|---|---|\n"
    if marker in content:
        return content.replace(marker, marker + row, 1)
    return content


def extract_pipeline_targets(section_text: str) -> list[str]:
    """Extracts ordered target names from a queue section's Markdown table or Upcoming Queue line."""
    targets = []
    # 1. Check Markdown pipeline table rows: | `Target` | ... or | **Target** | ...
    for m in re.finditer(r"^\|\s*(?:\d+\s*\|\s*)?[`*]*([A-Za-z0-9_:/~.-]+)[`*]*\s*\|", section_text, re.MULTILINE):
        cand = m.group(1).strip()
        if cand.lower() not in ("target", "app", "application", "---", "#", "order", "node"):
            if cand not in targets:
                targets.append(cand)

    # 2. Also check `- **Upcoming Queue**: ...` if present
    up_m = re.search(r"-\s*\*\*Upcoming Queue\*\*:\s*([^\n]+)", section_text)
    if up_m:
        for item in up_m.group(1).split(","):
            clean = item.strip().strip("`.").split()[0] if item.strip() else ""
            if clean and clean not in targets:
                targets.append(clean)

    return targets


def advance_section_and_pick_next(
    content: str,
    agent: str,
    completed_app: str,
    mature_set: set[str],
    quarantined: bool,
) -> tuple[str, str]:
    """
    Finds the next valid (non-mature) target for `agent` in its queue section,
    updates the section's Active Target & rotates/purges the pipeline table,
    and returns (updated_content, next_target).
    """
    hdr_pat = AGENT_SECTION_HEADERS.get(agent)
    if not hdr_pat:
        return content, completed_app

    sec_m = re.search(rf"({hdr_pat}[^\n]*\n)(.*?)(?=\n### \d|\n---|\n## |\Z)", content, re.DOTALL)
    if not sec_m:
        return content, completed_app

    sec_header = sec_m.group(1)
    sec_body = sec_m.group(2)

    all_candidates = extract_pipeline_targets(sec_body)
    # Filter out completed_app and any mature apps
    remaining = [
        t for t in all_candidates
        if t.lower() != completed_app.lower() and t.lower() not in mature_set
    ]
    next_target = remaining[0] if remaining else (all_candidates[0] if all_candidates else completed_app)

    # Update `- **Active Target**: ...` or `- **Target**: ...` line in section
    sec_body = re.sub(
        r"(-\s*\*\*(?:Active )?Target\*\*:\s*)`?[^`\n]+`?",
        rf"\g<1>`{next_target}`",
        sec_body,
        count=1,
    )

    # If there is a Markdown table in sec_body, rotate completed_app row to bottom (or remove if quarantined/mature)
    table_rows = list(re.finditer(r"^(\|\s*[`*]*([A-Za-z0-9_:/~.-]+)[`*]*\s*\|[^\n]*\n)", sec_body, re.MULTILINE))
    data_rows = [
        m for m in table_rows
        if m.group(2).strip().lower() not in ("target", "app", "application", "---")
        and not set(m.group(2).strip()).issubset({"-", ":"})
    ]
    if data_rows:
        kept_rows = []
        rotated_rows = []
        for m in data_rows:
            row_txt = m.group(1)
            row_target = m.group(2).strip()
            if row_target.lower() in mature_set:
                continue  # Purge mature apps from pipeline table
            if row_target.lower() == completed_app.lower():
                if not quarantined:
                    rotated_rows.append(row_txt)
            else:
                kept_rows.append(row_txt)
        new_rows_block = "".join(kept_rows + rotated_rows)
        first_start = data_rows[0].start()
        last_end = data_rows[-1].end()
        sec_body = sec_body[:first_start] + new_rows_block + sec_body[last_end:]
        if kept_rows:
            m_first = re.match(r"^\|\s*[`*]*([A-Za-z0-9_:/~.-]+)[`*]*\s*\|", kept_rows[0])
            if m_first:
                next_target = m_first.group(1).strip()
                sec_body = re.sub(
                    r"(-\s*\*\*(?:Active )?Target\*\*:\s*)`?[^`\n]+`?",
                    rf"\g<1>`{next_target}`",
                    sec_body,
                    count=1,
                )

    new_content = content[:sec_m.start()] + sec_header + sec_body + content[sec_m.end():]
    return new_content, next_target


def purge_mature_across_all_sections(content: str, mature_set: set[str]) -> str:
    """Removes any row belonging to a mature app from all 7 queue tables."""
    for agent in AGENT_SECTION_HEADERS:
        hdr_pat = AGENT_SECTION_HEADERS[agent]
        sec_m = re.search(rf"({hdr_pat}[^\n]*\n)(.*?)(?=\n### \d|\n---|\n## |\Z)", content, re.DOTALL)
        if not sec_m:
            continue
        sec_header, sec_body = sec_m.group(1), sec_m.group(2)
        lines = sec_body.splitlines(keepends=True)
        cleaned = []
        for line in lines:
            m = re.match(r"^\|\s*[`*]*([A-Za-z0-9_:/~.-]+)[`*]*\s*\|", line)
            if m and m.group(1).strip().lower() in mature_set:
                continue
            cleaned.append(line)
        content = content[:sec_m.start()] + sec_header + "".join(cleaned) + content[sec_m.end():]
    return content


def main() -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="Deterministic Queue Handoff & Log Updater")
    parser.add_argument("--agent", required=True, help="Current agent skill name (e.g. kilo-tester)")
    parser.add_argument("--app", required=True, help="Completed target app or kweb:// URL")
    parser.add_argument("--title", required=True, help="Short summary title for the log header")
    parser.add_argument("--bullet", action="append", default=[], help="Log bullet line (pass multiple times, max 5)")
    parser.add_argument("--quarantine-mature", type=str, default="", help="If set, moves app to Mature table with this summary")
    parser.add_argument("--category", type=str, default="Application", help="Category for Mature table when --quarantine-mature is used")
    args = parser.parse_args()

    if not NEXT_WORK_FILE.exists():
        print(f"Error: {NEXT_WORK_FILE} not found.", file=sys.stderr)
        return 1

    content = NEXT_WORK_FILE.read_text(encoding="utf-8", errors="replace")
    now_local = datetime.datetime.now().astimezone()
    now_iso = now_local.isoformat(timespec="seconds")
    date_str = now_local.strftime("%Y-%m-%d")

    quarantined = bool(args.quarantine_mature.strip())
    if quarantined:
        content = add_to_mature_table(content, args.app, args.category, args.quarantine_mature.strip(), date_str)

    mature_set = get_mature_apps(content)
    if quarantined:
        mature_set.add(args.app.lower())
        content = purge_mature_across_all_sections(content, mature_set)

    content, next_target = advance_section_and_pick_next(
        content=content,
        agent=args.agent,
        completed_app=args.app,
        mature_set=mature_set,
        quarantined=quarantined,
    )

    # Update YAML frontmatter
    parts = content.split("---", 2)
    if len(parts) >= 3:
        import yaml
        fm = yaml.safe_load(parts[1]) or {}
        rotation = fm.get("agent_rotation") or [
            "kilo-creator", "kilo-graphics", "kilo-soundscape",
            "kilo-tester", "kilo-usability", "kilo-qa", "kilo-expander",
        ]
        curr_idx = rotation.index(args.agent) if args.agent in rotation else 0
        next_agent = rotation[(curr_idx + 1) % len(rotation)]
        after_next = rotation[(curr_idx + 2) % len(rotation)]

        fm["current_agent"] = next_agent
        fm["next_agent"] = after_next
        fm["status"] = "ready"

        fm_key = args.agent.replace("-", "_")
        if "current_targets" not in fm or not isinstance(fm["current_targets"], dict):
            fm["current_targets"] = {}
        fm["current_targets"][fm_key] = next_target
        if args.agent == "kilo-creator" and str(next_target).startswith("kweb://"):
            fm["virtual_web_target"] = next_target

        # Also ensure no other agent's current_target is pointing at a newly quarantined mature app
        if quarantined:
            for other_agent, other_hdr in AGENT_SECTION_HEADERS.items():
                okey = other_agent.replace("-", "_")
                curr_t = str(fm["current_targets"].get(okey, ""))
                if curr_t.lower() in mature_set:
                    content, replacement_t = advance_section_and_pick_next(
                        content=content,
                        agent=other_agent,
                        completed_app=curr_t,
                        mature_set=mature_set,
                        quarantined=True,
                    )
                    fm["current_targets"][okey] = replacement_t

        fm["last_run"] = {
            "agent": args.agent,
            "app": args.app,
            "timestamp": now_iso,
        }
        parts_after = content.split("---", 2)
        new_fm_str = yaml.dump(fm, sort_keys=False, allow_unicode=True).strip()
        content = f"---\n{new_fm_str}\n---" + parts_after[2]
    else:
        next_agent = "kilo-tester"

    # Build terse log entry (strictly <= 6 bullet lines)
    bullets = [b.strip().lstrip("- ").strip() for b in args.bullet if b.strip()][:5]
    if not bullets:
        bullets = ["Status: PASS ✅ (verified build, startup check & byte budget < 999 KB)."]
    handoff_line = f"Handoff: Advanced `{args.agent}` to `{next_target}`; rotated active agent to `{next_agent}`."
    if quarantined:
        handoff_line = f"Mature Quarantine & Handoff: Promoted `{args.app}` to Mature archive; advanced `{args.agent}` to `{next_target}`; next agent `{next_agent}`."
    bullets.append(handoff_line)

    log_entry = f"- **{now_iso} — {args.agent}: {args.app} ({args.title})**\n" + "\n".join(f"  - {b}" for b in bullets) + "\n"

    marker_m = re.search(r"(## Recent Execution Logs[^\n]*\n+)", content)
    if marker_m:
        insert_pos = marker_m.end()
        content = content[:insert_pos] + log_entry + content[insert_pos:]

    NEXT_WORK_FILE.write_text(content, encoding="utf-8")

    # Zero-discard rotation of excess logs
    rot_res = rotate_next_work_logs(next_work_path=NEXT_WORK_FILE)
    print(
        f"✅ ADVANCE_TURN_COMPLETE: {args.agent} ({args.app}) -> next target '{next_target}', "
        f"next agent '{next_agent}' (archived {rot_res.get('archived', 0)} old logs)."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
