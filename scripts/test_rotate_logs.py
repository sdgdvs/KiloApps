#!/usr/bin/env python3
"""
scripts/test_rotate_logs.py
Unit tests for 0-Token Zero-Discard Log Rotation Engine
"""

import os
import shutil
import tempfile
import unittest
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from scripts.rotate_logs import (
    is_entry_start,
    normalize_entry,
    parse_execution_logs,
    rotate_next_work_logs,
    rotate_orchestrator_log,
)


class TestRotateLogs(unittest.TestCase):

    def setUp(self):
        self.temp_dir = Path(tempfile.mkdtemp())
        self.next_work = self.temp_dir / "next_work.md"
        self.archive = self.temp_dir / "fleet_execution_archive.md"
        self.log_file = self.temp_dir / "orchestrator.log"
        self.archive_log = self.temp_dir / "orchestrator_archive.log"

    def tearDown(self):
        shutil.rmtree(self.temp_dir, ignore_errors=True)

    def test_is_entry_start(self):
        self.assertTrue(is_entry_start("- **2026-10-08T15:31:00-07:00 — kilo-graphics: KMech"))
        self.assertTrue(is_entry_start("### Agent Run Log — Pass 5 QA"))
        self.assertTrue(is_entry_start("### Agent Run Log — kilo-expander (KChess)"))
        self.assertTrue(is_entry_start("- **[2026-10-08T15:31:00Z] kilo-graphics**"))
        # Sub-bullets should NOT match as entry start
        self.assertFalse(is_entry_start("  - Status: PASS ✅"))
        self.assertFalse(is_entry_start("- **Status:** 🟢 Completed"))
        self.assertFalse(is_entry_start("- Audited kasteroids.html"))
        self.assertFalse(is_entry_start("Regular prose line"))

    def test_normalize_entry(self):
        header_entry = (
            "### Agent Run Log — kilo-expander (KChess)\n"
            "- **Status:** 🟢 Completed\n"
            "- Added PGN export\n"
        )
        norm = normalize_entry(header_entry)
        self.assertTrue(norm.startswith("- **Agent Run Log — kilo-expander (KChess)**"))
        self.assertIn("  - **Status:** 🟢 Completed", norm)
        self.assertIn("  - Added PGN export", norm)

    def test_zero_discard_rotation_max_5(self):
        # Create a sample next_work.md with 8 entries
        sample_entries = [
            f"- **2026-10-08T{10+i}:00:00-07:00 — agent-{i}: App{i}**\n  - Status: PASS\n  - Item {i}"
            for i in range(8)
        ]
        # Reverse so newest is at the top
        sample_entries.reverse()

        content = (
            "---\ncurrent_agent: kilo-tester\n---\n\n"
            "# Master Plan\n\n"
            "## Directives\nSome directives\n\n"
            "## Recent Execution Logs (Max 5 Entries)\n\n"
            + "\n\n".join(sample_entries)
            + "\n"
        )
        self.next_work.write_text(content, encoding="utf-8")

        res = rotate_next_work_logs(
            max_entries=5,
            next_work_path=self.next_work,
            archive_path=self.archive,
        )

        self.assertEqual(res["retained"], 5)
        self.assertEqual(res["archived"], 3)
        self.assertEqual(res["status"], "success")

        # Verify next_work.md has exactly 5 entries
        new_nw_content = self.next_work.read_text(encoding="utf-8")
        _, _, remaining_entries, _ = parse_execution_logs(new_nw_content)
        self.assertEqual(len(remaining_entries), 5)
        self.assertIn("App7", remaining_entries[0])  # Newest kept
        self.assertIn("App3", remaining_entries[4])  # 5th kept

        # Verify archive has the 3 oldest entries (App2, App1, App0)
        archive_content = self.archive.read_text(encoding="utf-8")
        self.assertIn("App2", archive_content)
        self.assertIn("App1", archive_content)
        self.assertIn("App0", archive_content)
        # Verify newest are NOT in archive
        self.assertNotIn("App7", archive_content)

    def test_zero_discard_rotation_idempotent(self):
        sample_entries = [
            f"- **2026-10-08T{10+i}:00:00-07:00 — agent-{i}: App{i}**\n  - Status: PASS"
            for i in range(7)
        ]
        sample_entries.reverse()

        content = (
            "---\ncurrent_agent: kilo-tester\n---\n\n"
            "## Recent Execution Logs (Max 5 Entries)\n\n"
            + "\n\n".join(sample_entries)
            + "\n"
        )
        self.next_work.write_text(content, encoding="utf-8")

        res1 = rotate_next_work_logs(max_entries=5, next_work_path=self.next_work, archive_path=self.archive)
        self.assertEqual(res1["archived"], 2)

        # Second run should be within budget (5 <= 5)
        res2 = rotate_next_work_logs(max_entries=5, next_work_path=self.next_work, archive_path=self.archive)
        self.assertEqual(res2["archived"], 0)
        self.assertEqual(res2["status"], "within_budget")

    def test_mixed_header_and_bullet_rotation(self):
        entries = [
            "- **2026-10-08T12:00:00-07:00 — kilo-tester: K1**\n  - Status: PASS",
            "- **2026-10-08T11:00:00-07:00 — kilo-qa: K2**\n  - Status: PASS",
            "### Agent Run Log — Pass 5 QA (K3)\n- **Status:** 🟢 Done\n- Verified state",
            "### Agent Run Log — kilo-creator (K4)\n- **Status:** 🟢 Done",
        ]
        content = "## Recent Execution Logs\n\n" + "\n\n".join(entries) + "\n"
        self.next_work.write_text(content, encoding="utf-8")

        res = rotate_next_work_logs(max_entries=2, next_work_path=self.next_work, archive_path=self.archive)
        self.assertEqual(res["retained"], 2)
        self.assertEqual(res["archived"], 2)

        archive_content = self.archive.read_text(encoding="utf-8")
        self.assertIn("Pass 5 QA (K3)", archive_content)
        self.assertIn("kilo-creator (K4)", archive_content)

    def test_orchestrator_log_rotation(self):
        # Create a log file exceeding 10 KB
        lines = [f"[{i}] Log message line number {i} with some extra padding" for i in range(500)]
        self.log_file.write_text("\n".join(lines) + "\n", encoding="utf-8")

        res = rotate_orchestrator_log(
            max_bytes=5 * 1024,
            keep_lines=100,
            log_path=self.log_file,
            archive_path=self.archive_log,
        )

        self.assertTrue(res["rotated"])
        self.assertEqual(res["retained_lines"], 100)
        self.assertEqual(res["archived_lines"], 400)

        # Verify archive contains older lines
        archive_content = self.archive_log.read_text(encoding="utf-8")
        self.assertIn("[0] Log message", archive_content)
        self.assertIn("[399] Log message", archive_content)

        # Verify active log contains recent lines
        active_content = self.log_file.read_text(encoding="utf-8")
        self.assertIn("[400] Log message", active_content)
        self.assertIn("[499] Log message", active_content)
        self.assertNotIn("[0] Log message", active_content)


if __name__ == "__main__":
    unittest.main()
