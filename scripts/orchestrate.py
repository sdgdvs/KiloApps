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
RECEIPTS_DIR = REPO_ROOT / ".agents" / "receipts"
SESSION_FILE = REPO_ROOT / ".agents" / "scheduler_session.json"
FLEET_NODES_DIR = REPO_ROOT / ".agents" / "fleet_nodes"
TASK_NAME = "KiloApps-Fleet-Orchestrator"

# Multi-PC Node Profiles: Each computer has dedicated, non-colliding dispatch windows
NODE_PROFILES = {
    "pc_a": {
        "node_id": "pc_a",
        "name": "PC A (anonymous1 / 12900K)",
        "account": "anonymous1",
        "minutes": [2, 17],
        "allowed_window": lambda m: 0 <= m <= 20,
        "collision_desc": "Reserved for PC B (:30-:50) and PC C (:20-:30, :50-:60)",
    },
    "pc_b": {
        "node_id": "pc_b",
        "name": "PC B (sdgdvs)",
        "account": "sdgdvs",
        "minutes": [32, 47],
        "allowed_window": lambda m: 30 <= m <= 50,
        "collision_desc": "Reserved for PC A (:00-:20) and PC C (:20-:30, :50-:60)",
    },
    "pc_c": {
        "node_id": "pc_c",
        "name": "PC C (anonymous2)",
        "account": "anonymous2",
        "minutes": [22, 52],
        "allowed_window": lambda m: (20 <= m <= 30) or (50 <= m <= 60),
        "collision_desc": "Reserved for PC A (:00-:20) and PC B (:30-:50)",
    },
}


CREATE_NO_WINDOW = 0x08000000 if sys.platform == "win32" else 0


def disable_scheduled_task(task_name: str = TASK_NAME):
    """Disables the scheduled task in Windows Task Scheduler when session expires."""
    try:
        res = subprocess.run(
            ["schtasks.exe", "/change", "/tn", task_name, "/disable"],
            capture_output=True,
            text=True,
            stdin=subprocess.DEVNULL,
            creationflags=CREATE_NO_WINDOW,
        )
        if res.returncode == 0:
            log(f"Windows Scheduled Task '{task_name}' successfully disabled.")
        else:
            log(f"Note: schtasks disable returned code {res.returncode}: {res.stderr.strip() or res.stdout.strip()}")
    except Exception as e:
        log(f"Warning: Could not disable scheduled task via schtasks: {e}")


def check_session_timer() -> bool:
    """Verifies that the 24-hour fleet contribution session has not expired."""
    if not SESSION_FILE.exists():
        return True

    try:
        data = json.loads(SESSION_FILE.read_text(encoding="utf-8"))
        end_str = data.get("session_end")
        if not end_str:
            return True

        ts_str = str(end_str).strip()
        if ts_str.endswith("Z"):
            ts_str = ts_str[:-1] + "+00:00"
        end_dt = datetime.datetime.fromisoformat(ts_str)
        now_dt = datetime.datetime.now(datetime.timezone.utc)

        if now_dt >= end_dt:
            log(f"[SESSION EXPIRED] Active 24-hour contribution period ended at {end_str}. Halting execution until user requests more time.")
            data["status"] = "expired"
            try:
                SESSION_FILE.write_text(json.dumps(data, indent=2), encoding="utf-8")
            except Exception:
                pass
            disable_scheduled_task()
            return False

        remaining_sec = (end_dt - now_dt).total_seconds()
        log(f"[SESSION ACTIVE] {remaining_sec / 3600:.1f}h remaining in 24-hour session (expires {end_str}).")
        return True
    except Exception as e:
        log(f"Warning: Failed to read/validate session timer file: {e}")
        return True


def record_turn_in_session():
    """Increments turns_executed in session file upon successful turn."""
    if not SESSION_FILE.exists():
        return
    try:
        data = json.loads(SESSION_FILE.read_text(encoding="utf-8"))
        data["turns_executed"] = data.get("turns_executed", 0) + 1
        data["last_turn_timestamp"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        SESSION_FILE.write_text(json.dumps(data, indent=2), encoding="utf-8")
        log(f"[SESSION] Turn #{data['turns_executed']} recorded in session timer state.")
    except Exception as e:
        log(f"Warning: Failed to update session timer turn count: {e}")


def get_current_node_id() -> str:
    """Identifies which physical PC / account is executing (pc_a, pc_b, or pc_c)."""
    # 1. Local session file
    if SESSION_FILE.exists():
        try:
            s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8-sig"))
            if s_data.get("node_id") in NODE_PROFILES:
                return s_data["node_id"]
        except Exception:
            pass

    # 2. Local dashboard config
    cfg_file = REPO_ROOT / ".agents" / "dashboard_config.json"
    if cfg_file.exists():
        try:
            cfg = json.loads(cfg_file.read_text(encoding="utf-8-sig"))
            if cfg.get("node_id") in NODE_PROFILES:
                return cfg["node_id"]
        except Exception:
            pass

    # 3. Git user configuration
    try:
        res = subprocess.run(["git", "config", "user.name"], cwd=str(REPO_ROOT), capture_output=True, text=True, timeout=2)
        if res.returncode == 0:
            uname = res.stdout.strip().lower()
            if "sdgdvs" in uname:
                return "pc_b"
            if "anonymous2" in uname:
                return "pc_c"
            if "anonymous1" in uname or "kiloapps" in uname:
                return "pc_a"
    except Exception:
        pass

    # 4. Hostname hint
    import socket
    h = socket.gethostname().upper()
    if "12900K" in h:
        return "pc_a"

    return "pc_a"


def check_and_handle_remote_commands(node_id: str) -> bool:
    """Checks if another PC has commanded this PC to stop or start."""
    node_file = FLEET_NODES_DIR / f"node_{node_id.replace('pc_', '')}.json"
    if not node_file.exists():
        return True
    try:
        data = json.loads(node_file.read_text(encoding="utf-8-sig"))
        cmd = data.get("remote_command", "none")
        if cmd == "stop":
            log(f"[REMOTE CONTROL] Received remote 'stop' command. Disabling Windows Scheduled Task on {node_id}.")
            disable_scheduled_task()
            data["status"] = "stopped"
            data["remote_command"] = "none"
            data["last_heartbeat"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
            node_file.write_text(json.dumps(data, indent=2), encoding="utf-8")
            if SESSION_FILE.exists():
                try:
                    s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8-sig"))
                    s_data["status"] = "stopped"
                    SESSION_FILE.write_text(json.dumps(s_data, indent=2), encoding="utf-8")
                except Exception:
                    pass
            return False
    except Exception as e:
        log(f"Warning reading remote commands: {e}")
    return True


def is_in_collision_window(node_id: str) -> bool:
    """Checks if current minute is outside this computer's designated dispatch window."""
    now = datetime.datetime.now()
    minute = now.minute
    profile = NODE_PROFILES.get(node_id, NODE_PROFILES["pc_a"])
    is_allowed = profile["allowed_window"](minute)
    if not is_allowed:
        log(
            f"[WINDOW GUARD] Current minute (:{minute:02d}) is outside {profile['name']}'s "
            f"designated dispatch window. {profile['collision_desc']}. Deferring turn."
        )
        return True
    return False


def broadcast_node_heartbeat(node_id: str, status: str = "active", target: str = ""):
    """Updates fleet node status file so other computers see this PC's state."""
    node_file = FLEET_NODES_DIR / f"node_{node_id.replace('pc_', '')}.json"
    FLEET_NODES_DIR.mkdir(parents=True, exist_ok=True)
    try:
        import socket
        data = {}
        if node_file.exists():
            data = json.loads(node_file.read_text(encoding="utf-8-sig"))
        profile = NODE_PROFILES.get(node_id, NODE_PROFILES["pc_a"])
        data["node_id"] = node_id
        data["name"] = profile["name"]
        data["account"] = profile["account"]
        data["hostname"] = socket.gethostname().upper()
        data["status"] = status
        data["schedule_minutes"] = profile["minutes"]
        data["last_heartbeat"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        if target:
            data["current_target"] = target
        if SESSION_FILE.exists():
            s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8-sig"))
            data["turns_executed"] = s_data.get("turns_executed", 0)
            data["session_end"] = s_data.get("session_end")
            data["session_start"] = s_data.get("session_start")
        node_file.write_text(json.dumps(data, indent=2), encoding="utf-8")
    except Exception as e:
        log(f"Warning updating fleet node status: {e}")


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
    candidates = [
        Path(os.environ.get("LOCALAPPDATA", "")) / "agy" / "bin" / "agy.exe",
        Path(os.environ.get("USERPROFILE", "")) / ".local" / "bin" / "agy.exe",
        Path(os.environ.get("USERPROFILE", "")) / "AppData" / "Local" / "agy" / "bin" / "agy.exe",
        Path(r"C:\Users\mrbos\AppData\Local\agy\bin\agy.exe"),
        Path(r"C:\Users\M\AppData\Local\agy\bin\agy.exe"),
    ]
    for c in candidates:
        if c.exists():
            return str(c)
    raise FileNotFoundError("Could not locate agy.exe. Please ensure Antigravity CLI is installed.")


def check_and_recover_git_state() -> bool:
    """Detects and cleans up incomplete/interrupted Git operations (stuck rebase, merge, locks)."""
    git_dir = REPO_ROOT / ".git"
    if not git_dir.exists():
        return True

    # 1. Clear stale index.lock if present (> 2 minutes old)
    index_lock = git_dir / "index.lock"
    if index_lock.exists():
        try:
            mtime = index_lock.stat().st_mtime
            age_sec = datetime.datetime.now().timestamp() - mtime
            if age_sec > 120:
                index_lock.unlink()
                log(f"Removed stale git index.lock (age: {age_sec:.0f}s).")
        except Exception as e:
            log(f"Warning: Failed to check/remove index.lock: {e}")

    # 2. Check if a rebase is stuck in progress
    rebase_merge = git_dir / "rebase-merge"
    rebase_apply = git_dir / "rebase-apply"
    if rebase_merge.exists() or rebase_apply.exists():
        log("Detected stuck Git rebase in progress. Aborting rebase to restore clean tree...")
        res = subprocess.run(
            ["git", "rebase", "--abort"],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
        )
        if res.returncode == 0:
            log("Stuck rebase aborted successfully.")
        else:
            log(f"Warning: git rebase --abort exited with code {res.returncode}: {res.stderr.strip()}")

    # 3. Check if a merge is stuck in progress
    merge_head = git_dir / "MERGE_HEAD"
    if merge_head.exists():
        log("Detected stuck Git merge in progress. Aborting merge...")
        subprocess.run(
            ["git", "merge", "--abort"],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
        )

    # 4. Check for unmerged files (conflict status)
    try:
        status_res = subprocess.run(
            ["git", "status", "--porcelain"],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
        )
        if status_res.returncode == 0:
            lines = status_res.stdout.splitlines()
            has_unmerged = any(
                line.startswith(("UU", "AA", "UD", "DU", "DD", "AU", "UA")) or (len(line) >= 2 and line[0] == "U")
                for line in lines
            )
            if has_unmerged:
                log("Detected unmerged conflict files in working tree! Attempting recovery via git checkout HEAD -- . ...")
                subprocess.run(
                    ["git", "checkout", "HEAD", "--", "."],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                )
    except Exception as e:
        log(f"Warning: Failed to verify git status during recovery check: {e}")

    return True


def run_git_pull() -> bool:
    """Pulls latest remote changes with automatic rebase and safety rollback on conflict."""
    # 1. Clean up any stuck rebase, merge, or lockfile before pulling
    check_and_recover_git_state()

    log("Running git pull --rebase in repo root...")
    res = subprocess.run(
        ["git", "pull", "--rebase"],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
    )
    if res.returncode == 0:
        log(f"Git sync clean: {res.stdout.strip() or 'Already up to date.'}")
        return True

    err_output = res.stderr.strip() or res.stdout.strip()
    log(f"Git pull rebase failed (code {res.returncode}):\n{err_output}")

    # CRITICAL: If git pull --rebase stopped mid-rebase with conflicts, immediately abort
    # so conflict markers are never left in working tree files.
    git_dir = REPO_ROOT / ".git"
    if (git_dir / "rebase-merge").exists() or (git_dir / "rebase-apply").exists():
        log("Pull left rebase in progress with conflicts. Aborting rebase immediately to preserve file integrity...")
        abort_res = subprocess.run(
            ["git", "rebase", "--abort"],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
        )
        if abort_res.returncode == 0:
            log("Rebase aborted cleanly. Local working files preserved without conflict markers.")
        else:
            log(f"Warning: git rebase --abort failed (code {abort_res.returncode}): {abort_res.stderr.strip()}")

    return False


def validate_next_work_file(path: Path) -> tuple[bool, str]:
    """Validates that next_work.md exists and contains no Git conflict markers."""
    if not path.exists():
        return False, f"File {path} does not exist."
    try:
        content = path.read_text(encoding="utf-8")
    except Exception as e:
        return False, f"Failed to read {path}: {e}"

    # Check for git conflict markers
    conflict_markers = ["<<<<<<<", "=======", ">>>>>>>"]
    has_conflict = any(marker in content for marker in conflict_markers)
    if has_conflict:
        log(f"CRITICAL: Git conflict markers found in {path.name}! Attempting self-healing from HEAD...")
        restore_res = subprocess.run(
            ["git", "checkout", "HEAD", "--", path.name],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
        )
        if restore_res.returncode == 0:
            log(f"Successfully self-healed {path.name} from HEAD.")
            try:
                content = path.read_text(encoding="utf-8")
            except Exception as e:
                return False, f"Failed to re-read {path}: {e}"
        else:
            return False, f"File {path} contains unresolved git conflict markers and could not be restored."

    return True, content


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
    elif agent == "kilo-creator":
        target = targets.get("kilo_creator", "the next virtual web / ARG concept per next_work.md")
        if "kweb://" in target or "Virtual Web" in target or "public/web" in target:
            return (
                f"Activate skill 'kilo-creator'. "
                f"Design and implement Virtual 1999 Web destination or deep expansion for '{target}' in KiloOS/public/web/ per next_work.md and arg_plan.md (Anti-Potemkin standard: fully functioning interactive Web 1.0 experience, <999KB). "
                f"Link in KNet/portal/webring, verify builds, advance queue, update next_work.md, and git commit/push. Process 1 target only then STOP."
            )
        return (
            f"Activate skill 'kilo-creator'. "
            f"Design and implement new application or deep expansion for '{target}' per next_work.md. "
            f"Verify builds (<999KB), register in App.jsx, advance queue, update next_work.md, and git commit/push. Process 1 app only then STOP."
        )
    elif agent == "kilo-graphics":
        target = targets.get("kilo_graphics", "the next game in queue")
        return (
            f"Activate skill 'kilo-graphics'. "
            f"Perform game content, visual polish, and balance pass for '{target}' per next_work.md. "
            f"Search for and remove any rotating specular glints or traveling perimeter border dots. "
            f"Verify builds, advance queue, update next_work.md, and git commit/push. Process 1 app only then STOP."
        )
    elif agent == "kilo-usability":
        target = targets.get("kilo_usability", "the next app in queue")
        return (
            f"Activate skill 'kilo-usability'. "
            f"Perform UI/UX and usability pass for target app '{target}' per next_work.md. "
            f"Verify builds, advance queue, update next_work.md, and git commit/push. Process 1 app only then STOP."
        )
    elif agent == "kilo-expander":
        target = targets.get("kilo_expander", "the next app in queue")
        return (
            f"Activate skill 'kilo-expander'. "
            f"Perform deep feature expansion for target app '{target}' per next_work.md. "
            f"Verify builds, advance queue, update next_work.md, and git commit/push. Process 1 app only then STOP."
        )
    elif agent == "kilo-planner":
        return (
            "Activate skill 'kilo-planner'. "
            "Perform 24-hour fleet planning: evaluate project velocity, review queue health, "
            "rework the daily agent rotation schedule and active targets in next_work.md, "
            "compact execution logs to archive, update last_planner_run timestamp, git commit/push, then STOP."
        )
    else:
        return (
            f"Activate skill '{agent}'. "
            f"Execute scheduled task per instructions in next_work.md, git commit/push, then STOP."
        )


def write_turn_receipt(agent: str, model: str, duration: float, returncode: int, target: str = "") -> Path:
    """Emits an atomic, immutable JSON turn receipt to avoid git merge conflicts."""
    RECEIPTS_DIR.mkdir(parents=True, exist_ok=True)
    now_utc = datetime.datetime.now(datetime.timezone.utc)
    ts_slug = now_utc.strftime("%Y%m%d_%H%M%S")
    receipt_file = RECEIPTS_DIR / f"receipt_{agent}_{ts_slug}.json"
    provenance = None
    if os.environ.get("GITHUB_ACTIONS"):
        provenance = {
            "run_id": os.environ.get("GITHUB_RUN_ID", ""),
            "run_number": os.environ.get("GITHUB_RUN_NUMBER", ""),
            "actor": os.environ.get("GITHUB_ACTOR", ""),
            "workflow": os.environ.get("GITHUB_WORKFLOW", ""),
            "sha": os.environ.get("GITHUB_SHA", ""),
            "repository": os.environ.get("GITHUB_REPOSITORY", ""),
        }

    data = {
        "timestamp": now_utc.isoformat(),
        "agent": agent,
        "model": model,
        "target": target,
        "duration_seconds": round(duration, 2),
        "returncode": returncode,
        "success": returncode == 0,
        "provenance": provenance,
    }
    try:
        receipt_file.write_text(json.dumps(data, indent=2), encoding="utf-8")
        log(f"Emitted turn receipt: {receipt_file.name}")
        return receipt_file
    except Exception as e:
        log(f"Warning: Failed to write turn receipt: {e}")
        return None


def main():
    parser = argparse.ArgumentParser(description="KiloApps Fleet Master Orchestrator")
    parser.add_argument("--dry-run", action="store_true", help="Inspect and validate without executing agy")
    parser.add_argument("--force-agent", type=str, help="Override active agent (e.g. kilo-tester, kilo-qa, kilo-creator)")
    parser.add_argument("--no-receipt", action="store_true", help="Skip emitting turn receipt")
    parser.add_argument("--ignore-window", action="store_true", help="Bypass remote fleet contributor window guard")
    parser.add_argument("--ignore-session", action="store_true", help="Bypass 24-hour session timer check")
    args = parser.parse_args()

    log("=" * 60)
    log("Orchestrator tick initiated.")

    if not acquire_lock():
        sys.exit(0)

    try:
        # 1. Detect current node identity (PC A, PC B, or PC C)
        node_id = get_current_node_id()
        node_name = NODE_PROFILES.get(node_id, {}).get("name", node_id)
        log(f"Operating Node Identity: {node_name} [{node_id}]")

        # 2. Check for remote commands from fleet dashboard
        if not check_and_handle_remote_commands(node_id):
            return

        # 3. Enforce 24-hour session limit
        if not args.ignore_session and not check_session_timer():
            broadcast_node_heartbeat(node_id, status="expired")
            return

        # 4. Enforce collision-free window guard for this specific node
        if not args.ignore_window and not args.dry_run and is_in_collision_window(node_id):
            return

        if not args.dry_run:
            if not run_git_pull():
                log("Proceeding with local state despite git sync warning.")
        else:
            check_and_recover_git_state()

        is_valid, content_or_err = validate_next_work_file(NEXT_WORK_FILE)
        if not is_valid:
            log(f"Queue validation error: {content_or_err}. Skipping dispatch.")
            return

        try:
            frontmatter = parse_frontmatter(content_or_err)
        except Exception as e:
            log(f"Error parsing frontmatter in {NEXT_WORK_FILE.name}: {e}. Skipping dispatch.")
            return

        # Check if 24 hours have elapsed since last planner run
        last_planner_str = frontmatter.get("last_planner_run")
        should_run_planner = False
        if not args.force_agent and last_planner_str:
            try:
                ts_str = str(last_planner_str).strip()
                if ts_str.endswith("Z"):
                    ts_str = ts_str[:-1] + "+00:00"
                last_planner_dt = datetime.datetime.fromisoformat(ts_str)
                now_dt = datetime.datetime.now(datetime.timezone.utc)
                elapsed_sec = (now_dt - last_planner_dt).total_seconds()
                if elapsed_sec >= 24 * 3600:
                    should_run_planner = True
                    log(f"Daily Planner interval reached ({elapsed_sec / 3600:.1f}h >= 24h since {last_planner_str}). Triggering kilo-planner.")
            except Exception as e:
                log(f"Warning parsing last_planner_run timestamp '{last_planner_str}': {e}")

        if should_run_planner:
            agent = "kilo-planner"
        else:
            agent = args.force_agent or frontmatter.get("current_agent", "kilo-tester")

        status = frontmatter.get("status", "ready")
        model = os.environ.get("AGY_MODEL") or frontmatter.get("model", "gemini-3.8-flash-high")
        timeout_min = int(frontmatter.get("timeout_minutes", 15))
        targets = frontmatter.get("current_targets", {})
        if isinstance(targets, str):
            targets = {}

        log(f"Queue State: agent='{agent}', status='{status}', model='{model}', timeout={timeout_min}m")

        if status != "ready" and not args.force_agent and not should_run_planner:
            log(f"Task status is '{status}' (not 'ready'). Skipping dispatch.")
            return

        prompt = build_agent_prompt(agent, targets)
        agy_bin = find_agy_executable()

        cmd = [
            agy_bin,
            "--add-dir",
            str(REPO_ROOT),
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

        target_app = str(targets.get(agent.replace("-", "_"), ""))
        if not args.no_receipt:
            write_turn_receipt(
                agent=agent,
                model=model,
                duration=duration,
                returncode=process.returncode,
                target=target_app,
            )

        if process.returncode != 0:
            log(f"Agent turn exited with non-zero code {process.returncode}. Investigation required.")
            broadcast_node_heartbeat(node_id, status="error", target=target_app)
        else:
            log("Agent turn completed successfully.")
            record_turn_in_session()
            broadcast_node_heartbeat(node_id, status="active", target=target_app)

        # Post-agent Git check: ensure unpushed commits and node state are synchronized to remote
        if process.returncode == 0 and not args.dry_run:
            try:
                # Stage fleet node telemetry
                subprocess.run(
                    ["git", "add", ".agents/fleet_nodes"],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                )
                rev_res = subprocess.run(
                    ["git", "rev-list", "@{u}..HEAD", "--count"],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                )
                if rev_res.returncode == 0 and rev_res.stdout.strip():
                    unpushed_count = int(rev_res.stdout.strip())
                    if unpushed_count > 0:
                        log(f"Detected {unpushed_count} unpushed commit(s). Pushing to origin/main...")
                        push_res = subprocess.run(
                            ["git", "push", "origin", "main"],
                            cwd=str(REPO_ROOT),
                            capture_output=True,
                            text=True,
                        )
                        if push_res.returncode == 0:
                            log("Unpushed commit(s) successfully pushed to origin/main.")
                        else:
                            log(f"Warning: git push exited with code {push_res.returncode}: {push_res.stderr.strip()}")
            except Exception as e:
                log(f"Warning: Post-agent push check encountered error: {e}")

    except Exception as e:
        import traceback
        log(f"CRITICAL: Unhandled exception in orchestrator tick: {e}")
        log(traceback.format_exc())
    finally:
        release_lock()
        log("Orchestrator tick finished. Lock released.")
        log("=" * 60)


if __name__ == "__main__":
    main()
