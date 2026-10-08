"""
scripts/compact_all.py
Enforces token conservation caps across shared Markdown plan files:
- Calls rotate_logs.py to execute 0-token zero-discard rotation of next_work.md.
- Retains active execution logs to the most recent 5 entries (or configured limit).
- Safely moves older entries to archive/fleet_execution_archive.md without data loss.
- Rotates workspace logs (logs/orchestrator.log) to prevent unbounded growth.
"""

import os
import sys
from pathlib import Path

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
if str(WORKSPACE_ROOT) not in sys.path:
    sys.path.insert(0, str(WORKSPACE_ROOT))

from scripts.rotate_logs import rotate_next_work_logs, rotate_orchestrator_log


def main():
    print("Running scripts/compact_all.py: checking plan files for token conservation...")
    
    # Enforce zero-discard rotation on next_work.md
    res = rotate_next_work_logs(repo_root=WORKSPACE_ROOT)
    if res.get("archived", 0) > 0:
        print(f"[next_work.md] Archived {res['archived']} entries to archive/fleet_execution_archive.md (kept {res['retained']} active entries).")
    else:
        print(f"[next_work.md] Within token budget ({res.get('retained', 0)} active entries).")

    # Enforce zero-discard rotation on orchestrator.log
    res_orch = rotate_orchestrator_log()
    if res_orch.get("rotated"):
        print(f"[orchestrator.log] Rotated {res_orch.get('archived_lines', 0)} lines to logs/orchestrator_archive.log.")

    print("Compaction complete.")


if __name__ == "__main__":
    main()
