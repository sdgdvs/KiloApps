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

# 0-Token Zero-Discard Log Rotation Engine
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))
try:
    from scripts.rotate_logs import rotate_next_work_logs, rotate_orchestrator_log
except ImportError:
    from rotate_logs import rotate_next_work_logs, rotate_orchestrator_log

try:
    from scripts.pack_tiered_context import main as pack_context
except ImportError:
    try:
        from pack_tiered_context import main as pack_context
    except ImportError:
        pack_context = None

# Multi-PC Node Profiles: Dedicated, non-colliding dispatch windows based on account model tier:
# - PC A: sdgdvs (Gemini Pro) - 1 turn/hr at :12 past each hour (Window :10 - :18)
# - PC B: anonymous2 (Gemini Pro) - 1 turn/hr at :30 past each hour (Window :28 - :36)
# - PC C: This PC / anonymous1 (Gemini Ultra) - 4 turns/hr at :02, :20, :38, :48 past each hour
NODE_PROFILES = {
    "pc_a": {
        "node_id": "pc_a",
        "name": "PC A (sdgdvs)",
        "account": "sdgdvs",
        "model_tier": "Gemini Pro",
        "minutes": [12],
        "allowed_window": lambda m: 10 <= m <= 18,
        "safe_window": ":10 - :18",
        "collision_desc": "Reserved for PC B (Pro: :28-:36) & PC C (Ultra: Outside :10-:18, :28-:36)",
    },
    "pc_b": {
        "node_id": "pc_b",
        "name": "PC B (anonymous2)",
        "account": "anonymous2",
        "model_tier": "Gemini Pro",
        "minutes": [30],
        "allowed_window": lambda m: 28 <= m <= 36,
        "safe_window": ":28 - :36",
        "collision_desc": "Reserved for PC A (Pro: :10-:18) & PC C (Ultra: Outside :10-:18, :28-:36)",
    },
    "pc_c": {
        "node_id": "pc_c",
        "name": "PC C (This PC / anonymous1)",
        "account": "anonymous1",
        "hostname_hint": "12900K",
        "model_tier": "Gemini Ultra",
        "minutes": [2, 20, 38, 48],
        "allowed_window": lambda m: not ((10 <= m <= 18) or (28 <= m <= 36)),
        "safe_window": "Outside Pro Windows (:00-:10, :18-:28, :36-:60 excluding :28-:36)",
        "collision_desc": "Reserved for PC A (Pro: :10-:18) & PC B (Pro: :28-:36)",
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


def clean_task_triggers_boundary(task_name: str = TASK_NAME):
    """Ensures Windows Scheduled Task triggers have no EndBoundary or Duration limit (Continuous Mode)."""
    if sys.platform != "win32":
        return
    ps_cmd = (
        f"$task = Get-ScheduledTask -TaskName '{task_name}' -ErrorAction SilentlyContinue; "
        f"if ($task) {{ "
        f"  $updated = $false; "
        f"  foreach ($trig in $task.Triggers) {{ "
        f"    if ($trig.EndBoundary -or $trig.Repetition.Duration) {{ "
        f"      $trig.EndBoundary = $null; "
        f"      $trig.Repetition.Duration = $null; "
        f"      $updated = $true; "
        f"    }} "
        f"  }}; "
        f"  if ($updated) {{ "
        f"    Set-ScheduledTask -TaskName '{task_name}' -Trigger $task.Triggers | Out-Null; "
        f"    Write-Host 'CONTINUOUS_MIGRATED'; "
        f"  }} "
        f"}}"
    )
    try:
        res = subprocess.run(
            ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", ps_cmd],
            capture_output=True,
            text=True,
            stdin=subprocess.DEVNULL,
            creationflags=CREATE_NO_WINDOW,
            timeout=10,
        )
        if "CONTINUOUS_MIGRATED" in res.stdout:
            log(f"[MIGRATION] Neutralized EndBoundary and Duration limit on Task '{task_name}' triggers (continuous repetition).")
    except Exception as e:
        log(f"Warning checking task triggers boundary: {e}")


def atomic_write_json(path: Path, data: dict):
    """Writes JSON atomically via temp file + fsync + os.replace to prevent NTFS NUL-byte corruption on power loss."""
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp_path = path.with_suffix(path.suffix + ".tmp")
    payload = json.dumps(data, indent=2)
    with open(tmp_path, "w", encoding="utf-8") as f:
        f.write(payload)
        f.flush()
        os.fsync(f.fileno())
    os.replace(tmp_path, path)


def _detect_fallback_node_id() -> str:
    """Detects node identity without reading SESSION_FILE (used for self-healing SESSION_FILE)."""
    cfg_file = REPO_ROOT / ".agents" / "dashboard_config.json"
    if cfg_file.exists():
        try:
            raw = cfg_file.read_text(encoding="utf-8-sig", errors="replace").replace("\x00", "").strip()
            if raw:
                cfg = json.loads(raw)
                if cfg.get("node_id") in NODE_PROFILES:
                    return cfg["node_id"]
        except Exception:
            pass

    import socket
    h = socket.gethostname().upper()
    if "DESKTOP-HSSS709" in h:
        return "pc_b"
    if "12900K" in h:
        return "pc_c"
    if "P10700" in h:
        return "pc_a"

    try:
        res = subprocess.run(["git", "config", "user.name"], cwd=str(REPO_ROOT), capture_output=True, text=True, timeout=2)
        if res.returncode == 0:
            uname = res.stdout.strip().lower()
            if "anonymous1" in uname or "kiloapps" in uname:
                return "pc_c"
            if "sdgdvs" in uname:
                return "pc_a"
            if "anonymous2" in uname:
                return "pc_b"
    except Exception:
        pass

    return "pc_b"


def load_session_data(node_id: str = "", auto_heal: bool = True) -> dict:
    """Reads SESSION_FILE safely and auto-heals if corrupted (e.g. NTFS NUL bytes after unclean shutdown)."""
    if not SESSION_FILE.exists():
        return {}
    try:
        raw = SESSION_FILE.read_text(encoding="utf-8-sig", errors="replace").replace("\x00", "").strip()
        if raw:
            data = json.loads(raw)
            if isinstance(data, dict):
                return data
    except Exception as e:
        if not auto_heal:
            return {}
        log(f"[SELF-HEAL] Detected corrupted session file ({e}). Reconstructing from fleet node state...")

    if not auto_heal:
        return {}

    resolved_node = node_id or _detect_fallback_node_id()
    profile = NODE_PROFILES.get(resolved_node, NODE_PROFILES["pc_c"])
    node_file = FLEET_NODES_DIR / f"node_{resolved_node.replace('pc_', '')}.json"
    node_data = {}
    if node_file.exists():
        try:
            n_raw = node_file.read_text(encoding="utf-8-sig", errors="replace").replace("\x00", "").strip()
            if n_raw:
                node_data = json.loads(n_raw)
        except Exception:
            pass

    now_iso = datetime.datetime.now(datetime.timezone.utc).isoformat()
    today_str = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d")
    recovered = {
        "node_id": resolved_node,
        "session_start": node_data.get("session_start") or now_iso,
        "session_end": node_data.get("session_end"),
        "session_limit_enabled": bool(node_data.get("session_limit_enabled", False)),
        "timer_enabled": bool(node_data.get("timer_enabled", False)),
        "duration_hours": 0.0,
        "max_turns_estimate": 0,
        "turns_executed": int(node_data.get("turns_executed", 0)),
        "turns_today": int(node_data.get("turns_today", 0)),
        "last_turn_date": today_str,
        "minutes": node_data.get("schedule_minutes") or profile["minutes"],
        "task_name": TASK_NAME,
        "status": node_data.get("status", "active"),
        "last_turn_timestamp": node_data.get("last_turn_timestamp") or now_iso,
    }
    try:
        atomic_write_json(SESSION_FILE, recovered)
        log(f"[SELF-HEAL] Reconstructed clean {SESSION_FILE.name} for {resolved_node} (turns_executed={recovered['turns_executed']}).")
    except Exception as write_err:
        log(f"Warning: Failed to write self-healed session file: {write_err}")
    return recovered


def ensure_continuous_scheduler(node_id: str):
    """Respects session limits when user_managed is set; only auto-heals if explicitly in continuous mode."""
    if SESSION_FILE.exists():
        try:
            s_data = load_session_data(node_id=node_id, auto_heal=True)
            if not s_data:
                return
            # If the session is user-managed (quota-controlled), do NOT override limits
            if s_data.get("user_managed", False):
                log("[SESSION] User-managed quota mode active. Skipping continuous self-healing.")
                return
            # Only auto-heal if session_limit_enabled is explicitly False (legacy continuous mode)
            if s_data.get("session_limit_enabled") is False and not s_data.get("user_managed", False):
                needs_save = False
                if s_data.get("status") == "expired":
                    s_data["status"] = "active"
                    needs_save = True
                if needs_save:
                    atomic_write_json(SESSION_FILE, s_data)
                    log("[MIGRATION] Reset expired status to active (continuous mode, no user quota).")
        except Exception as e:
            log(f"Warning verifying session continuous state: {e}")


def check_session_timer() -> bool:
    """Verifies whether a session limit is active and has expired.
    
    Defaults to Continuous Mode (unlimited fleet execution) unless session_limit_enabled is explicitly True.
    """
    if not SESSION_FILE.exists():
        log("[SESSION ACTIVE] Continuous mode (unlimited fleet execution, no session file).")
        return True

    try:
        data = load_session_data(auto_heal=True)
        
        # If timer is not explicitly enabled, fleet runs continuously
        if not data.get("session_limit_enabled", False):
            log("[SESSION ACTIVE] Continuous mode enabled (no auto-stop timer).")
            return True

        end_str = data.get("session_end")
        if not end_str:
            log("[SESSION ACTIVE] Continuous mode enabled (session_end is null).")
            return True

        ts_str = str(end_str).strip()
        if ts_str.endswith("Z"):
            ts_str = ts_str[:-1] + "+00:00"
        end_dt = datetime.datetime.fromisoformat(ts_str)
        now_dt = datetime.datetime.now(datetime.timezone.utc)

        if now_dt >= end_dt:
            log(f"[SESSION EXPIRED] Timed session ended at {end_str}. Halting execution until user requests more time.")
            data["status"] = "expired"
            try:
                atomic_write_json(SESSION_FILE, data)
            except Exception:
                pass
            disable_scheduled_task()
            return False

        remaining_sec = (end_dt - now_dt).total_seconds()
        log(f"[SESSION ACTIVE] {remaining_sec / 3600:.1f}h remaining in timed session (expires {end_str}).")
        return True
    except Exception as e:
        log(f"Warning: Failed to read/validate session timer file: {e}")
        return True


def record_turn_in_session():
    """Increments turns_executed and turns_today in session file upon successful turn."""
    if not SESSION_FILE.exists():
        return
    try:
        data = load_session_data(auto_heal=True)
        data["turns_executed"] = data.get("turns_executed", 0) + 1
        data["turns_today"] = data.get("turns_today", 0) + 1
        data["last_turn_timestamp"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        data["last_turn_date"] = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d")
        atomic_write_json(SESSION_FILE, data)
        log(f"[SESSION] Turn #{data['turns_executed']} recorded (today: {data['turns_today']}).")
    except Exception as e:
        log(f"Warning: Failed to update session timer turn count: {e}")


def check_daily_turn_cap() -> bool:
    """Checks if daily turn cap has been reached. Resets counter at midnight UTC."""
    if not SESSION_FILE.exists():
        return True
    try:
        data = load_session_data(auto_heal=True)
        max_per_day = data.get("max_turns_per_day", 0)
        if max_per_day <= 0:
            return True  # No cap configured

        today_str = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d")
        last_date = data.get("last_turn_date", "")

        # Reset daily counter at midnight UTC
        if last_date != today_str:
            data["turns_today"] = 0
            data["last_turn_date"] = today_str
            atomic_write_json(SESSION_FILE, data)
            log(f"[QUOTA] Daily counter reset for {today_str}.")

        turns_today = data.get("turns_today", 0)
        if turns_today >= max_per_day:
            log(f"[QUOTA] Daily turn cap reached ({turns_today}/{max_per_day}). Skipping dispatch until tomorrow.")
            return False

        remaining = max_per_day - turns_today
        log(f"[QUOTA] Daily budget: {turns_today}/{max_per_day} used, {remaining} remaining.")
        return True
    except Exception as e:
        log(f"Warning: Failed to check daily turn cap: {e}")
        return True


def get_current_node_id() -> str:
    """Identifies which physical PC / account is executing (pc_a, pc_b, or pc_c)."""
    # 1. Local session file
    if SESSION_FILE.exists():
        try:
            s_data = load_session_data(auto_heal=False)
            if s_data.get("node_id") in NODE_PROFILES:
                return s_data["node_id"]
        except Exception:
            pass

    return _detect_fallback_node_id()


def check_and_handle_remote_commands(node_id: str) -> bool:
    """Checks if another PC has commanded this PC to stop or start."""
    node_file = FLEET_NODES_DIR / f"node_{node_id.replace('pc_', '')}.json"
    if not node_file.exists():
        return True
    try:
        raw = node_file.read_text(encoding="utf-8-sig", errors="replace").replace("\x00", "").strip()
        if not raw:
            return True
        data = json.loads(raw)
        cmd = data.get("remote_command", "none")
        if cmd == "stop":
            log(f"[REMOTE CONTROL] Received remote 'stop' command. Disabling Windows Scheduled Task on {node_id}.")
            disable_scheduled_task()
            data["status"] = "stopped"
            data["remote_command"] = "none"
            data["last_heartbeat"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
            atomic_write_json(node_file, data)
            if SESSION_FILE.exists():
                try:
                    s_data = load_session_data(node_id=node_id, auto_heal=True)
                    s_data["status"] = "stopped"
                    atomic_write_json(SESSION_FILE, s_data)
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
            try:
                n_raw = node_file.read_text(encoding="utf-8-sig", errors="replace").replace("\x00", "").strip()
                if n_raw:
                    data = json.loads(n_raw)
            except Exception:
                data = {}
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
            s_data = load_session_data(node_id=node_id, auto_heal=True)
            if s_data:
                data["turns_executed"] = s_data.get("turns_executed", data.get("turns_executed", 0))
                data["session_end"] = s_data.get("session_end")
                data["session_start"] = s_data.get("session_start", data.get("session_start"))
                data["session_limit_enabled"] = s_data.get("session_limit_enabled", False)
                data["timer_enabled"] = s_data.get("session_limit_enabled", False)
        else:
            data["session_end"] = None
            data["session_limit_enabled"] = False
            data["timer_enabled"] = False
        atomic_write_json(node_file, data)
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

    # Fallback line-by-line parser for simple key-value and list YAML
    data = {}
    current_key = None
    for line in raw_yaml.splitlines():
        line = line.rstrip()
        if not line or line.startswith("#"):
            continue
        if line.lstrip().startswith("- ") and current_key:
            item = line.lstrip()[2:].strip().strip("'\"")
            if isinstance(data.get(current_key), dict) and not data[current_key]:
                data[current_key] = []
            if isinstance(data.get(current_key), list):
                data[current_key].append(item)
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

    log("Running git pull --rebase --autostash in repo root...")
    res = subprocess.run(
        ["git", "pull", "--rebase", "--autostash"],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
    )
    if res.returncode == 0:
        out = res.stdout.strip()
        log(f"Git sync clean: {out or 'Already up to date.'}")
        if "scripts/orchestrate.py" in out and os.environ.get("ORCHESTRATOR_REEXEC") != "1":
            log("[AUTO-UPDATE] Detected update to orchestrator.py from git pull. Re-executing freshly pulled script...")
            os.environ["ORCHESTRATOR_REEXEC"] = "1"
            release_lock()
            re_res = subprocess.run([sys.executable, str(Path(__file__).resolve())] + sys.argv[1:], cwd=str(REPO_ROOT))
            sys.exit(re_res.returncode)
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
        content = path.read_text(encoding="utf-8", errors="replace")
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
                content = path.read_text(encoding="utf-8", errors="replace")
            except Exception as e:
                return False, f"Failed to re-read {path}: {e}"
        else:
            return False, f"File {path} contains unresolved git conflict markers and could not be restored."

    return True, content


TOKEN_EFFICIENCY_SUFFIX = (
    " QUALITY & FAST VERIFICATION DIRECTIVE: "
    "Complete this turn in <= 32 tool calls. "
    "COMMIT CHECKPOINT: Complete all Mission Spec items across both Web HTML and Win32 C (<App>/main.c if present), then wrap up by ~16 tool calls: "
    "1) Run ONE-COMMAND post-edit verification ONCE (do NOT run verify_turn.py before editing): `python scripts/verify_turn.py --app <target> --build-native` (validates size <999KB, native C build, startup check, visual probe sheet.png, Vite build if shell changed, and security lint in 1 call), and inspect `docs/gallery/review/<target>/sheet.png` via `view_file` to confirm zero layout clipping. "
    "2) Run ONE-COMMAND queue handoff & logging: `python scripts/advance_turn.py --agent <agent> --app <target> --title \"<Summary>\" --bullet \"<Status>\" --bullet \"<Key change>\"` (or pass `--quarantine-mature \"<Summary>\" --category \"<Category>\"` if promoting to Mature). "
    "3) Commit (`git add -A && git commit -m ... && git push origin main`) and STOP immediately. "
    "Do NOT read historical archive files (archive/) or receipts."
)

INAPPROPRIATE_IMAGEN_TARGETS = {
    "kasteroids", "kchess", "kgo", "kreversi", "ktowers", "kconnect4",
    "kterm", "kscript", "khex", "kdragon", "kcolony", "kwizard", "kfarm",
    "kwords", "khangman", "ksudoku", "kcalc", "kmine", "kmines", "knetmap",
    "kmaze", "kmech", "kabyss", "kcolosseum", "kspace", "kfortress", "kquest"
}

# Pure-utility / text-only apps where procedural soundscape composition is not appropriate.
INAPPROPRIATE_SOUNDSCAPE_TARGETS = {
    "kcalc", "kcalendar", "kclock", "kcontacts", "kconverter", "kdb",
    "kfont", "khash", "kimage", "kjournal", "kmail", "knote", "kpad",
    "kpass", "krss", "kscript", "ksettings", "ksteno", "ksys", "ktask",
    "ktaskmgr", "kterm", "ktimer", "ktodo", "kzip", "kbookmark", "kclip",
    "kcolor", "kexplorer", "kfleet", "kgraph", "khex", "kread",
    "kchatserver", "kcontribute", "kpomodoro"
}


def extract_target_mission_spec(target: str, agent: str = "") -> str:
    """Extracts the 1-line mission spec for `target` from next_work.md pipeline tables (scoped to `agent`'s section first)."""
    if not target or not NEXT_WORK_FILE.exists():
        return ""
    try:
        import re
        content = NEXT_WORK_FILE.read_text(encoding="utf-8", errors="replace")
        escaped = re.escape(target.strip())
        row_pat = rf"^\|\s*[`*]*{escaped}[`*]*\s*\|\s*([^|\n]+)\|\s*([^|\n]+)\|"

        # 1. Search inside `agent`'s dedicated section first so per-agent [ARGON REVIEW] specs are matched
        section_headers = {
            "kilo-creator": r"### 1\.",
            "kilo-graphics": r"### 2\.",
            "kilo-soundscape": r"### 3\.",
            "kilo-tester": r"### 4\.",
            "kilo-usability": r"### 5\.",
            "kilo-qa": r"### 6\.",
            "kilo-expander": r"### 7\.",
            "kilo-review": r"### 8\.",
        }
        if agent in section_headers:
            sec_m = re.search(rf"{section_headers[agent]}[^\n]*\n(.*?)(?=\n### \d|\n---|\n## |\Z)", content, re.DOTALL)
            if sec_m:
                m_sec = re.search(row_pat, sec_m.group(1), re.MULTILINE | re.IGNORECASE)
                if m_sec:
                    return f" [Target Files: {m_sec.group(1).strip()} | Mission Spec: {m_sec.group(2).strip()}]"

        # 2. Fallback to global search
        m = re.search(row_pat, content, re.MULTILINE | re.IGNORECASE)
        if m:
            files_col = m.group(1).strip()
            spec_col = m.group(2).strip()
            return f" [Target Files: {files_col} | Mission Spec: {spec_col}]"
    except Exception:
        pass
    return ""


def build_agent_prompt(agent: str, targets: dict) -> str:
    target = str(targets.get(agent.replace("-", "_"), "the next app in queue"))
    spec_hint = extract_target_mission_spec(target, agent)
    if agent == "kilo-tester":
        return (
            f"Activate skill 'kilo-tester'. "
            f"Perform interactive UI audit, VFS/IPC check, and inline fixes for target app '{target}' per next_work.md.{spec_hint} "
            f"Verify with `python scripts/verify_turn.py --app {target}`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-qa":
        return (
            f"Activate skill 'kilo-qa'. "
            f"Perform Pass 5 audit, F5/F9 persistence, and dual Win32 C parity fixes for target app '{target}' per next_work.md.{spec_hint} "
            f"Verify with `python scripts/verify_turn.py --app {target} --build-native`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-creator":
        if "kweb://" in target or "Virtual Web" in target or "public/web" in target:
            return (
                f"Activate skill 'kilo-creator'. "
                f"Design and implement Virtual 1999 Web destination or deep expansion for '{target}' in KiloOS/public/web/ per next_work.md and arg_plan.md (Anti-Potemkin standard: fully functioning interactive Web 1.0 experience, <999KB).{spec_hint} "
                f"Link in KNet/portal/webring, verify with `python scripts/verify_turn.py --app {target}`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 target only then STOP."
                + TOKEN_EFFICIENCY_SUFFIX
            )
        return (
            f"Activate skill 'kilo-creator'. "
            f"Design and implement new application or deep expansion for '{target}' per next_work.md.{spec_hint} "
            f"Verify builds (<999KB), register in App.jsx, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-graphics":
        return (
            f"Activate skill 'kilo-graphics'. "
            f"Replace programmer art with Imagen 3 generated sprites and backgrounds for '{target}' per next_work.md.{spec_hint} "
            f"If Imagen 3 asset replacement is not appropriate for '{target}', skip turn cleanly with `python scripts/advance_turn.py`. "
            f"Search for and remove any rotating specular glints or traveling perimeter border dots. "
            f"Verify with `python scripts/verify_turn.py --app {target}`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-soundscape":
        return (
            f"Activate skill 'kilo-soundscape'. "
            f"Compose unique procedural FM synthesis audio (YM2612 2-Op / SPC700) for '{target}' per next_work.md.{spec_hint} "
            f"Replace existing generic FM presets with genre-appropriate 6-channel music: melodic themes, contrary counterpoint, shell voicings, swing-tick groove, UI earcons, and action stingers. "
            f"Verify with `python scripts/verify_turn.py --app {target}`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-usability":
        return (
            f"Activate skill 'kilo-usability'. "
            f"Perform UI/UX, responsive layout, HiDPI canvas, and keyboard/VFS shortcut usability pass for target app '{target}' per next_work.md.{spec_hint} "
            f"Verify with `python scripts/verify_turn.py --app {target}`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-expander":
        return (
            f"Activate skill 'kilo-expander'. "
            f"Perform deep feature expansion (KiloOS VFS/IPC integration, RFMS online multiplayer, and domain workstation depth) for target app '{target}' per next_work.md.{spec_hint} "
            f"Verify with `python scripts/verify_turn.py --app {target}`, advance queue with `python scripts/advance_turn.py`, and git commit/push. Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-review":
        return (
            f"Activate skill 'kilo-review'. "
            f"Perform Gemini 4 Argon visual, feature, gameplay, and usability critic pass for target app '{target}' per next_work.md.{spec_hint} "
            f"Run `uv run scripts/visual_probe.py --app {target}`, inspect `docs/gallery/review/{target.lower()}/sheet.png` via `view_file`, audit the app's HTML/JS for visual clipping, dead/Potemkin features, gameplay flaws, and UX friction, and inject surgical fix specs into Flash queues via `python scripts/advance_turn.py --agent kilo-review --app {target} --inject-fix \"<target_agent>:{target}:<Surgical Fix Spec>\" ...`, then git commit/push. Do NOT edit app source directly—diagnose and dispatch to Flash! Process 1 app only then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    elif agent == "kilo-planner":
        return (
            "Activate skill 'kilo-planner'. "
            "Perform 24-hour fleet planning: evaluate project velocity, review queue health, "
            "rework the daily agent rotation schedule and active targets in next_work.md, "
            "compact execution logs to archive, update last_planner_run timestamp, git commit/push, then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )
    else:
        return (
            f"Activate skill '{agent}'. "
            f"Execute scheduled task per instructions in next_work.md, git commit/push, then STOP."
            + TOKEN_EFFICIENCY_SUFFIX
        )


def handle_preflight_skip(agent: str, targets: dict, frontmatter: dict, node_id: str) -> bool:
    """
    Pre-flight check in Python to auto-advance past Mature or inappropriate targets with 0 LLM tokens.
    Instead of wasting the scheduled turn slot, advances `targets[fm_target_key]` in-place to the
    next valid target in `next_work.md` and returns False so `orchestrate.py` immediately dispatches
    the agent on the valid target!
    Note: `kilo-review` intentionally audits Mature apps, so Mature quarantine is not skipped for `kilo-review`.
    """
    try:
        from scripts.advance_turn import advance_section_and_pick_next, get_mature_apps
    except ImportError:
        try:
            from advance_turn import advance_section_and_pick_next, get_mature_apps
        except ImportError:
            return False

    fm_target_key = agent.replace("-", "_")
    content = NEXT_WORK_FILE.read_text(encoding="utf-8", errors="replace")
    mature_set = get_mature_apps(content)

    advanced_any = False
    for _ in range(8):
        target = str(targets.get(fm_target_key, "")).strip()
        if not target:
            break
        t_low = target.lower()
        skip_reason = ""

        spec_hint = extract_target_mission_spec(target, agent)
        is_argon_fix = "[ARGON REVIEW]" in spec_hint
        if t_low in mature_set and agent != "kilo-review" and not is_argon_fix:
            skip_reason = f"{target} is already in Mature Applications quarantine"
        elif agent == "kilo-graphics" and not is_argon_fix and (t_low in INAPPROPRIATE_IMAGEN_TARGETS or "(skip)" in t_low):
            skip_reason = f"Imagen 3 asset replacement not appropriate for {target}"
        elif agent == "kilo-soundscape" and not is_argon_fix and (t_low in INAPPROPRIATE_SOUNDSCAPE_TARGETS or "(skip)" in t_low):
            skip_reason = f"Procedural soundscape not appropriate for {target}"

        if not skip_reason:
            break

        log(f"[PRE-FLIGHT FAST-FORWARD] Advancing {agent} past '{target}' ({skip_reason})...")
        content, next_target = advance_section_and_pick_next(
            content=content,
            agent=agent,
            completed_app=target,
            mature_set=mature_set,
            quarantined=(t_low in mature_set and agent != "kilo-review"),
        )
        if not next_target or next_target.lower() == t_low:
            break
        targets[fm_target_key] = next_target
        advanced_any = True

    if advanced_any:
        try:
            parts = content.split("---", 2)
            if len(parts) >= 3:
                import yaml
                fm_data = yaml.safe_load(parts[1]) or {}
                if "current_targets" not in fm_data or not isinstance(fm_data["current_targets"], dict):
                    fm_data["current_targets"] = {}
                fm_data["current_targets"][fm_target_key] = targets[fm_target_key]
                new_fm_str = yaml.dump(fm_data, sort_keys=False, allow_unicode=True).strip()
                content = f"---\n{new_fm_str}\n---" + parts[2]
            NEXT_WORK_FILE.write_text(content, encoding="utf-8")
            log(f"[PRE-FLIGHT FAST-FORWARD] Updated {agent} active target to '{targets[fm_target_key]}'. Proceeding to dispatch in same turn!")
        except Exception as e:
            log(f"Warning persisting pre-flight target fast-forward: {e}")

    return False


def rotate_past_failed_argon_turn(failed_agent: str, failed_target: str) -> tuple[str, dict]:
    """
    When `kilo-review` (running on Gemini 4 Argon) fails (e.g., Pro node Argon quota exhausted),
    auto-rotates `current_agent` in `next_work.md` to the next Flash agent in `agent_rotation`
    while keeping `failed_target` at the top of `kilo-review`'s queue for the next cycle.
    Returns (next_flash_agent, updated_frontmatter).
    """
    try:
        subprocess.run(["git", "checkout", "--", "."], cwd=str(REPO_ROOT), capture_output=True)
        content = NEXT_WORK_FILE.read_text(encoding="utf-8", errors="replace")
        parts = content.split("---", 2)
        if len(parts) >= 3:
            import yaml
            fm = yaml.safe_load(parts[1]) or {}
            rotation = [a for a in (fm.get("agent_rotation") or [
                "kilo-creator", "kilo-graphics",
                "kilo-tester", "kilo-usability", "kilo-qa", "kilo-expander", "kilo-review",
            ]) if a != "kilo-soundscape"]
            curr_idx = rotation.index(failed_agent) if failed_agent in rotation else 0
            next_agent = rotation[(curr_idx + 1) % len(rotation)]
            if next_agent == failed_agent and len(rotation) > 1:
                next_agent = rotation[(curr_idx + 2) % len(rotation)]
            next_idx = rotation.index(next_agent) if next_agent in rotation else 0
            after_next = rotation[(next_idx + 1) % len(rotation)]

            fm["current_agent"] = next_agent
            fm["next_agent"] = after_next
            fm["status"] = "ready"
            new_fm_str = yaml.dump(fm, sort_keys=False, allow_unicode=True).strip()
            new_content = f"---\n{new_fm_str}\n---" + parts[2]
            NEXT_WORK_FILE.write_text(new_content, encoding="utf-8")
            subprocess.run(["git", "add", "next_work.md"], cwd=str(REPO_ROOT), capture_output=True)
            subprocess.run(
                ["git", "commit", "-m", f"chore(orchestrator): auto-rotate past failed {failed_agent} Argon turn ({failed_target})"],
                cwd=str(REPO_ROOT),
                capture_output=True,
            )
            log(f"[ARGON FALLBACK] Auto-rotated current_agent '{failed_agent}' -> '{next_agent}' (kept '{failed_target}' queued for next Argon rotation).")
            return next_agent, fm
    except Exception as e:
        log(f"Error in rotate_past_failed_argon_turn: {e}")
    return "kilo-tester", {}


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
    parser.add_argument("--force-agent", type=str, help="Override active agent (e.g. kilo-tester, kilo-qa, kilo-creator, kilo-review)")
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

        # 2. Git Sync: pull remote changes first so this machine runs with latest logic & signals
        if not args.dry_run:
            if not run_git_pull():
                log("Proceeding with local state despite git sync warning.")
        else:
            check_and_recover_git_state()

        # 3. Continuous Mode Self-Healing: neutralize 24h timer and task trigger boundaries
        ensure_continuous_scheduler(node_id)

        # 4. Check for remote commands from fleet dashboard
        if not check_and_handle_remote_commands(node_id):
            return

        # 5. Enforce session limit (Continuous mode by default unless explicitly configured)
        if not args.ignore_session and not check_session_timer():
            broadcast_node_heartbeat(node_id, status="expired")
            return

        # 5b. Enforce daily turn cap (quota management)
        if not args.ignore_session and not check_daily_turn_cap():
            broadcast_node_heartbeat(node_id, status="quota_paused")
            return

        # 6. Enforce collision-free window guard for this specific node
        if not args.ignore_window and not args.dry_run and is_in_collision_window(node_id):
            return

        is_valid, content_or_err = validate_next_work_file(NEXT_WORK_FILE)
        if not is_valid:
            log(f"Queue validation error: {content_or_err}. Skipping dispatch.")
            return

        # 6b. 0-Token Zero-Discard Log Rotation: Enforce lean context before agent spawn
        if not args.dry_run:
            try:
                rot_res = rotate_next_work_logs(next_work_path=NEXT_WORK_FILE)
                if rot_res.get("archived", 0) > 0:
                    log(f"[PRE-FLIGHT] 0-token rotation: archived {rot_res['archived']} old turn(s) to archive/fleet_execution_archive.md (retained {rot_res['retained']}).")
                    subprocess.run(["git", "add", "next_work.md", "archive/fleet_execution_archive.md"], cwd=str(REPO_ROOT), capture_output=True)
                    subprocess.run(["git", "commit", "-m", "chore(logs): zero-token rotate old work to archive"], cwd=str(REPO_ROOT), capture_output=True)
                rotate_orchestrator_log()
                # Re-validate and refresh content after rotation
                is_valid, content_or_err = validate_next_work_file(NEXT_WORK_FILE)
                if not is_valid:
                    log(f"Queue validation error after rotation: {content_or_err}. Skipping dispatch.")
                    return
            except Exception as e:
                log(f"Warning in pre-flight log rotation: {e}")

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

        # Up to 2 attempts: if attempt 0 is an Argon turn (`kilo-review`) that fails due to quota exhaustion,
        # auto-rotate `current_agent` to the next Flash agent and immediately execute that Flash agent in attempt 1!
        for dispatch_attempt in range(2):
            status = frontmatter.get("status", "ready")
            fm_agent_models = frontmatter.get("agent_models")
            if os.environ.get("AGY_MODEL"):
                model = os.environ.get("AGY_MODEL")
            elif isinstance(fm_agent_models, dict) and agent in fm_agent_models:
                model = str(fm_agent_models[agent]).strip()
            else:
                model = frontmatter.get("model", "gemini-3.8-flash-high")

            # High effort across all active Flash & Argon agents to maximize code/UX/fix quality
            HIGH_EFFORT_AGENTS = {
                "kilo-creator", "kilo-expander", "kilo-soundscape", "kilo-tester",
                "kilo-usability", "kilo-qa", "kilo-graphics", "kilo-planner", "kilo-review"
            }
            fm_effort = frontmatter.get("effort", "high")
            if os.environ.get("AGY_EFFORT"):
                effort = os.environ.get("AGY_EFFORT")
            elif isinstance(frontmatter.get("agent_efforts"), dict) and agent in frontmatter["agent_efforts"]:
                effort = frontmatter["agent_efforts"][agent]
            elif fm_effort in ("high", "medium", "low"):
                effort = fm_effort
            else:
                effort = "high" if agent in HIGH_EFFORT_AGENTS else "medium"

            # Align model identifier with effort for Gemini 3.8 Flash & Gemini 4 Argon to prevent CLI conflict
            if model.startswith("gemini-3.8-flash") and effort in ("high", "medium", "low"):
                model = f"gemini-3.8-flash-{effort}"
            elif model.startswith("gemini-4-argon") and effort in ("high", "medium", "low"):
                model = f"gemini-4-argon-{effort}"

            timeout_min = int(
                os.environ.get("AGY_TIMEOUT")
                or (frontmatter.get("agent_timeouts", {}).get(agent) if isinstance(frontmatter.get("agent_timeouts"), dict) else None)
                or frontmatter.get("timeout_minutes", 10)
            )
            targets = frontmatter.get("current_targets", {})
            if isinstance(targets, str):
                targets = {}

            log(f"Queue State: agent='{agent}', status='{status}', model='{model}', effort='{effort}', timeout={timeout_min}m")

            if status != "ready" and not args.force_agent and not should_run_planner:
                log(f"Task status is '{status}' (not 'ready'). Skipping dispatch.")
                return

            # Pre-flight auto-skip check: avoid spending millions of tokens on known skip targets
            if not args.dry_run and not args.force_agent and handle_preflight_skip(agent, targets, frontmatter, node_id):
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
                "--effort",
                effort,
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

            # Check if an Argon review turn failed (e.g., Pro node ran out of Argon quota)
            is_argon_turn = (agent == "kilo-review" or "argon" in model.lower())
            argon_did_not_advance = False
            if is_argon_turn and process.returncode == 0:
                try:
                    post_fm = parse_frontmatter(NEXT_WORK_FILE.read_text(encoding="utf-8", errors="replace"))
                    if post_fm.get("current_agent") == agent:
                        argon_did_not_advance = True
                        log(f"Warning: {agent} exited 0 but did not advance current_agent in next_work.md.")
                except Exception:
                    pass

            if is_argon_turn and (process.returncode != 0 or argon_did_not_advance) and dispatch_attempt == 0 and not args.force_agent:
                log(
                    f"[ARGON QUOTA/FAILURE FALLBACK] {agent} ({model}) failed to complete (code={process.returncode}). "
                    f"Auto-rotating to next Flash agent and dispatching immediately in the same tick!"
                )
                agent, new_fm = rotate_past_failed_argon_turn(agent, target_app)
                if new_fm:
                    frontmatter = new_fm
                    continue

            if process.returncode != 0:
                log(f"Agent turn exited with non-zero code {process.returncode}. Investigation required.")
                broadcast_node_heartbeat(node_id, status="error", target=target_app)
            else:
                log("Agent turn completed successfully.")
                record_turn_in_session()
                broadcast_node_heartbeat(node_id, status="active", target=target_app)
            break

        # Post-agent Git check: ensure unpushed commits and node state are synchronized to remote
        if process.returncode == 0 and not args.dry_run:
            try:
                # Post-turn Uncommitted Work Salvage: if agent finished edits right as --print-timeout fired
                nw_status = subprocess.run(
                    ["git", "status", "--porcelain", "next_work.md"],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                )
                if nw_status.returncode == 0 and nw_status.stdout.strip():
                    log(f"[POST-TURN SALVAGE] Detected completed uncommitted turn in working tree for {agent} ({target_app}). Validating security lint...")
                    lint_res = subprocess.run([sys.executable, "scripts/security_lint.py"], cwd=str(REPO_ROOT), capture_output=True)
                    if lint_res.returncode == 0:
                        subprocess.run(["git", "add", "-u"], cwd=str(REPO_ROOT), capture_output=True)
                        subprocess.run(["git", "commit", "-m", f"feat({agent}): auto-salvage completed turn for {target_app}"], cwd=str(REPO_ROOT), capture_output=True)
                        log(f"[POST-TURN SALVAGE] Committed completed turn for {agent} ({target_app}).")
                elif target_app:
                    app_status = subprocess.run(
                        ["git", "status", "--porcelain", "KiloOS/public"],
                        cwd=str(REPO_ROOT),
                        capture_output=True,
                        text=True,
                    )
                    if app_status.returncode == 0 and app_status.stdout.strip():
                        log(f"[POST-TURN SALVAGE] Detected uncommitted app edits for {agent} ({target_app}) before handoff. Running verify_turn.py...")
                        v_res = subprocess.run(
                            [sys.executable, "scripts/verify_turn.py", "--app", target_app, "--build-native"],
                            cwd=str(REPO_ROOT),
                            capture_output=True,
                            text=True,
                            stdin=subprocess.DEVNULL,
                            timeout=90,
                        )
                        if v_res.returncode == 0:
                            adv_res = subprocess.run(
                                [
                                    sys.executable, "scripts/advance_turn.py",
                                    "--agent", agent,
                                    "--app", target_app,
                                    "--title", "Verified Turn (Auto-Salvaged at Timeout)",
                                    "--bullet", "Status: PASS ✅ (verified by orchestrator post-turn salvage).",
                                    "--bullet", f"Completed {agent} updates for {target_app} across web and native targets.",
                                ],
                                cwd=str(REPO_ROOT),
                                capture_output=True,
                                text=True,
                                stdin=subprocess.DEVNULL,
                            )
                            if adv_res.returncode == 0:
                                subprocess.run(["git", "add", "-u"], cwd=str(REPO_ROOT), capture_output=True)
                                subprocess.run(["git", "commit", "-m", f"feat({agent}): auto-salvage verified turn for {target_app}"], cwd=str(REPO_ROOT), capture_output=True)
                                log(f"[POST-TURN SALVAGE] Verified, advanced queue, and committed turn for {agent} ({target_app})!")

                # Post-turn 0-token rotation: ensure new logs written by agent don't exceed cap
                rot_res = rotate_next_work_logs(next_work_path=NEXT_WORK_FILE)
                if rot_res.get("archived", 0) > 0:
                    log(f"[POST-TURN] 0-token rotation: archived {rot_res['archived']} old turn(s) to archive/fleet_execution_archive.md.")
                    subprocess.run(["git", "add", "next_work.md", "archive/fleet_execution_archive.md"], cwd=str(REPO_ROOT), capture_output=True)
                    subprocess.run(["git", "commit", "-m", "chore(logs): zero-token rotate old work to archive"], cwd=str(REPO_ROOT), capture_output=True)
                rotate_orchestrator_log()

                # Auto-sync Tiered Context exports to Google Drive
                if pack_context:
                    try:
                        pack_context()
                    except Exception as gdrive_err:
                        log(f"[POST-TURN] Google Drive context sync note: {gdrive_err}")

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
                        push_env = os.environ.copy()
                        push_env["GIT_TERMINAL_PROMPT"] = "0"
                        push_env["GCM_INTERACTIVE"] = "never"
                        push_res = subprocess.run(
                            ["git", "push", "origin", "main"],
                            cwd=str(REPO_ROOT),
                            capture_output=True,
                            text=True,
                            timeout=60,
                            env=push_env,
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
