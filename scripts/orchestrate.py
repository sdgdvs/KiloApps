#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "pyyaml>=6.0",
# ]
# ///
"""
KiloApps Fleet Master Orchestrator
Triggered periodically by Windows Task Scheduler (e.g. every 2 hours).

Responsibilities:
1. Concurrency control: Enforces single-instance execution via PID lockfile.
2. Git Sync: Runs `git pull --rebase` to fetch remote updates.
3. Queue Parsing: Reads YAML frontmatter from `next_work.md`.
4. Agent Dispatch: Invokes `agy` CLI in headless mode with the assigned Gemini Skill.
5. Logging: Records execution details in `logs/orchestrator.log`.
"""

import argparse
import datetime
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

# Paths
REPO_ROOT = Path(__file__).resolve().parent.parent
LOCK_FILE = REPO_ROOT / ".agents" / ".orchestrator.lock"
NEXT_WORK_FILE = REPO_ROOT / "next_work.md"
LOG_DIR = REPO_ROOT / "logs"
LOG_FILE = LOG_DIR / "orchestrator.log"

DEFAULT_AGY_PATH = (
    Path(os.environ.get("LOCALAPPDATA", "")) / "agy" / "bin" / "agy.exe"
    if os.environ.get("LOCALAPPDATA")
    else Path(r"C:\Users\mrbos\AppData\Local\agy\bin\agy.exe")
)


def log(msg: str):
    timestamp = datetime.datetime.now(datetime.timezone.utc).isoformat()
    line = f"[{timestamp}] {msg}"
    print(line)
    try:
        LOG_DIR.mkdir(parents=True, exist_ok=True)
        with open(LOG_FILE, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception as e:
        print(f"Warning: Failed to write to log file: {e}", file=sys.stderr)


def acquire_lock() -> bool:
    """Acquires single-instance execution lock using a PID file."""
    LOCK_FILE.parent.mkdir(parents=True, exist_ok=True)
    if LOCK_FILE.exists():
        try:
            pid_str = LOCK_FILE.read_text(encoding="utf-8").strip()
            if pid_str:
                pid = int(pid_str)
                # Check if process is currently alive on Windows
                import ctypes
                kernel32 = ctypes.windll.kernel32
                SYNCHRONIZE = 0x00100000
                process = kernel32.OpenProcess(SYNCHRONIZE, False, pid)
                if process:
                    kernel32.CloseHandle(process)
                    log(f"Lock active: another orchestrator instance (PID {pid}) is running. Exiting.")
                    return False
        except Exception:
            pass  # Stale lock or invalid PID; override

    # Write current PID
    try:
        LOCK_FILE.write_text(str(os.getpid()), encoding="utf-8")
        return True
    except Exception as e:
        log(f"Error creating lock file: {e}")
        return False


def release_lock():
    try:
        if LOCK_FILE.exists():
            LOCK_FILE.unlink()
    except Exception as e:
        log(f"Warning: Failed to remove lock file: {e}")


def parse_frontmatter(content: str) -> dict:
    """Parses YAML frontmatter between the first two '---' markers."""
    parts = content.split("---", 2)
    if len(parts) < 3:
        raise ValueError("Invalid format: next_work.md missing YAML frontmatter markers (---).")
    raw_yaml = parts[1].strip()

    # Try PyYAML if installed
    try:
        import yaml
        parsed = yaml.safe_load(raw_yaml)
        if isinstance(parsed, dict):
            return parsed
    except ImportError:
        pass

    # Fallback line-by-line parser for simple key-value YAML
    data = {}
    current_dict = data
    current_key = None
    for line in raw_yaml.splitlines():
        line = line.rstrip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("  ") and current_key:
            # Sub-key
            sub = line.strip()
            if ":" in sub:
                k, v = sub.split(":", 1)
                k = k.strip()
                v = v.strip().strip("'\"")
                if isinstance(data.get(current_key), dict):
                    data[current_key][k] = v
            continue

        if ":" in line:
            k, v = line.split(":", 1)
            k = k.strip()
            v = v.strip().strip("'\"")
            if not v:
                data[k] = {}
                current_key = k
            else:
                data[k] = v
                current_key = None
    return data


def find_agy_executable() -> str:
    which_agy = shutil.which("agy") or shutil.which("agy.exe")
    if which_agy:
        return which_agy
    if DEFAULT_AGY_PATH.exists():
        return str(DEFAULT_AGY_PATH)
    raise FileNotFoundError("Could not locate agy.exe. Please ensure Antigravity CLI is installed.")


def run_git_pull():
    log("Running git pull --rebase in repo root...")
    res = subprocess.run(
        ["git", "pull", "--rebase"],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
    )
    if res.returncode != 0:
        log(f"Git pull rebase failed (code {res.returncode}):\n{res.stderr}")
        return False
    log(f"Git sync clean: {res.stdout.strip() or 'Already up to date.'}")
    return True


def build_agent_prompt(agent: str, targets: dict) -> str:
    if agent == "kilo-tester":
        target = targets.get("kilo_tester", "the next app in queue")
        return (
            f"Activate skill 'kilo-tester'. "
            f"Perform interactive UI audit and inline fixes for target app '{target}' per next_work.md. "
            f"Verify builds, advance queue, update next_work.md, and git commit/push. Process 1 app only then STOP."
        )
    elif agent == "kilo-qa":
        target = targets.get("kilo_qa", "the next app in queue")
        return (
            f"Activate skill 'kilo-qa'. "
            f"Perform Pass 5 audit and fixes for target app '{target}' per next_work.md. "
            f"Verify builds, advance queue, update next_work.md, and git commit/push. Process 1 app only then STOP."
        )
    elif agent == "kilo-planner":
        return (
            "Activate skill 'kilo-planner'. "
            "Perform daily fleet maintenance, compact execution logs in next_work.md to archive, "
            "verify queue health, git commit/push, then STOP."
        )
    else:
        return (
            f"Activate skill '{agent}'. "
            f"Execute scheduled task per instructions in next_work.md, git commit/push, then STOP."
        )


def main():
    parser = argparse.ArgumentParser(description="KiloApps Fleet Master Orchestrator")
    parser.add_argument("--dry-run", action="store_true", help="Inspect and validate without executing agy")
    parser.add_argument("--force-agent", type=str, help="Override active agent (e.g. kilo-tester, kilo-qa)")
    args = parser.parse_args()

    log("=" * 60)
    log("Orchestrator tick initiated.")

    if not acquire_lock():
        sys.exit(0)

    try:
        if not args.dry_run:
            if not run_git_pull():
                log("Proceeding with local state despite git sync warning.")

        if not NEXT_WORK_FILE.exists():
            log(f"Error: {NEXT_WORK_FILE} does not exist!")
            sys.exit(1)

        content = NEXT_WORK_FILE.read_text(encoding="utf-8")
        frontmatter = parse_frontmatter(content)

        agent = args.force_agent or frontmatter.get("current_agent", "kilo-tester")
        status = frontmatter.get("status", "ready")
        model = frontmatter.get("model", "gemini-3.8-flash-high")
        timeout_min = int(frontmatter.get("timeout_minutes", 15))
        targets = frontmatter.get("current_targets", {})
        if isinstance(targets, str):
            targets = {}

        log(f"Queue State: agent='{agent}', status='{status}', model='{model}', timeout={timeout_min}m")

        if status != "ready" and not args.force_agent:
            log(f"Task status is '{status}' (not 'ready'). Skipping dispatch.")
            return

        prompt = build_agent_prompt(agent, targets)
        agy_bin = find_agy_executable()

        cmd = [
            agy_bin,
            "-p",
            prompt,
            "--model",
            model,
            "--dangerously-skip-permissions",
            f"--print-timeout={timeout_min}m",
        ]

        log(f"Prepared Command: {' '.join(cmd)}")

        if args.dry_run:
            log("Dry-run mode enabled: Skipping CLI execution.")
            return

        log(f"Spawning agent execution via agy (timeout: {timeout_min}m)...")
        start_time = datetime.datetime.now()

        # Execute agy
        process = subprocess.run(
            cmd,
            cwd=str(REPO_ROOT),
            text=True,
            capture_output=True,
        )

        duration = (datetime.datetime.now() - start_time).total_seconds()
        log(f"Agent finished in {duration:.1f}s with exit code {process.returncode}.")

        if process.stdout:
            stdout_sample = process.stdout.strip()[-500:]
            log(f"Stdout (tail):\n{stdout_sample}")
        if process.stderr:
            stderr_sample = process.stderr.strip()[-500:]
            log(f"Stderr (tail):\n{stderr_sample}")

        if process.returncode != 0:
            log(f"Agent turn exited with non-zero code {process.returncode}. Investigation required.")
        else:
            log("Agent turn completed successfully.")

    finally:
        release_lock()
        log("Orchestrator tick finished. Lock released.")
        log("=" * 60)


if __name__ == "__main__":
    main()
